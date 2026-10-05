// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "media_intelligence/core/ContentAwareStreamPolicy.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace remote::media_intelligence {
namespace {

constexpr std::size_t kMaximumSizes = 20;
constexpr std::size_t kMaximumFrameRates = 14;

struct Size {
    std::uint32_t width = 0;
    std::uint32_t height = 0;
};

struct Candidate {
    Size size;
    std::uint32_t fps = 0;
    StreamQualityEstimate quality;
    bool feasible = false;
};

std::size_t SceneIndex(ScreenScene scene) noexcept
{
    const auto value = static_cast<std::size_t>(scene);
    return value < 10 ? value : 0;
}

bool ValidConfig(const ContentAwareStreamConfig& config) noexcept
{
    if (config.alignment == 0 || config.alignment > 64 ||
        config.maximumDimension < config.alignment ||
        config.maximumDimension > 65536 ||
        !std::isfinite(config.upgradeHeadroomRatio) ||
        config.upgradeHeadroomRatio < 1.0 || config.upgradeHeadroomRatio > 4.0) {
        return false;
    }
    for (const auto& profile : config.profiles) {
        if (!std::isfinite(profile.minimumSpatialScale) ||
            profile.minimumSpatialScale <= 0.0 || profile.minimumSpatialScale > 1.0 ||
            !std::isfinite(profile.maximumSpatialReductionFromAnchor) ||
            profile.maximumSpatialReductionFromAnchor < 0.0 ||
            profile.maximumSpatialReductionFromAnchor > 0.5 ||
            !std::isfinite(profile.seedBitsPerPixelPerFrame) ||
            profile.seedBitsPerPixelPerFrame <= 0.0 ||
            profile.seedBitsPerPixelPerFrame > 100.0 ||
            !std::isfinite(profile.desiredSpatialScale) ||
            profile.desiredSpatialScale < profile.minimumSpatialScale ||
            profile.desiredSpatialScale > 1.0 ||
            !profile.minimumUsefulActiveFps || profile.minimumUsefulActiveFps > 120 ||
            !std::isfinite(profile.maximumReferenceAverageQp) ||
            profile.maximumReferenceAverageQp < 0.0 || profile.maximumReferenceAverageQp > 51.0 ||
            (profile.upgradeOrder != StreamUpgradeOrder::kResolutionThenFrameRate &&
             profile.upgradeOrder != StreamUpgradeOrder::kFrameRateThenResolution) ||
            (profile.protection != StreamProtection::kSpatialDetail &&
             profile.protection != StreamProtection::kCurrentFrameRate)) {
            return false;
        }
    }
    for (const auto height : config.referenceHeights) {
        if (height > config.maximumDimension) {
            return false;
        }
    }
    for (const auto fps : config.frameRateCandidates) {
        if (fps > 120) {
            return false;
        }
    }
    return true;
}

std::uint64_t Pixels(Size size) noexcept
{
    return static_cast<std::uint64_t>(size.width) * size.height;
}

bool SameSize(Size left, Size right) noexcept
{
    return left.width == right.width && left.height == right.height;
}

// One scale factor; floor-to-alignment introduces at most alignment-1 pixels
// of rounding per axis. Very narrow sources unable to form a legal size fail.
Size FitSize(Size source, double scale, std::uint32_t alignment) noexcept
{
    if (!std::isfinite(scale) || scale <= 0.0 || scale > 1.0) {
        return {};
    }
    const auto width = static_cast<std::uint32_t>(std::floor(source.width * scale));
    const auto height = static_cast<std::uint32_t>(std::floor(source.height * scale));
    return {width / alignment * alignment, height / alignment * alignment};
}

Size FitBounds(Size source, Size bounds, std::uint32_t alignment) noexcept
{
    if (!source.width || !source.height || !bounds.width || !bounds.height) {
        return {};
    }
    const double scale = std::min({1.0,
        static_cast<double>(bounds.width) / source.width,
        static_cast<double>(bounds.height) / source.height});
    return FitSize(source, scale, alignment);
}

bool LegalSize(Size size, Size cap, Size source, std::uint32_t alignment) noexcept
{
    if (!size.width || !size.height || size.width > cap.width ||
        size.height > cap.height || size.width % alignment || size.height % alignment) {
        return false;
    }
    // Both aligned axes must be obtainable from the SAME scale factor.
    // Comparing aspect ratios with a fixed pixel tolerance on the long axis
    // incorrectly rejects ultrawide sizes after rounding the short axis.
    const double lower = std::max(static_cast<double>(size.width) / source.width,
        static_cast<double>(size.height) / source.height);
    const double upper = std::min(
        static_cast<double>(size.width + alignment) / source.width,
        static_cast<double>(size.height + alignment) / source.height);
    return lower < upper && lower <= 1.0;
}

template <typename T, std::size_t N, typename Equal>
void AddUnique(std::array<T, N>& values, std::size_t& count,
    T value, Equal equal) noexcept
{
    for (std::size_t i = 0; i < count; ++i) {
        if (equal(values[i], value)) {
            return;
        }
    }
    if (count < N) {
        values[count++] = value;
    }
}

StreamQualityEstimate Estimate(const StreamQualityRequest& request,
    const StreamSceneProfile& profile, const ContentAwareStreamConfig& config) noexcept
{
    if (config.qualityEstimator) {
        auto normalized = request;
        if (SceneIndex(normalized.scene) == 0) {
            normalized.scene = ScreenScene::kUnknown;
        }
        auto estimate = config.qualityEstimator(normalized, config.qualityEstimatorContext);
        if (!estimate.requiredVideoBitrateBps) {
            estimate.available = false;
            estimate.calibrated = false;
            estimate.reference = false;
        }
        return estimate;
    }
    const long double raw = static_cast<long double>(request.width) * request.height *
        request.frameRate * profile.seedBitsPerPixelPerFrame + config.burstAllowanceBps;
    // Strictly less avoids undefined float-to-uint64 conversion near 2^64.
    if (!std::isfinite(raw) || raw <= 0.0L ||
        raw >= static_cast<long double>(std::numeric_limits<std::uint64_t>::max())) {
        return {};
    }
    return {static_cast<std::uint64_t>(std::ceil(raw)), true, false, true};
}

bool Upgrades(const Candidate& value, Size current, std::uint32_t fps) noexcept
{
    return Pixels(value.size) > Pixels(current) || value.fps > fps;
}

bool Better(const Candidate& value, const Candidate& previous,
    const StreamSceneProfile& profile, std::uint32_t currentFps,
    std::uint32_t usefulFps) noexcept
{
    // Useful activity is a common constraint, ahead of the preference order.
    // If no candidate meets it, retain an explicitly limited emergency action.
    if ((value.fps >= usefulFps) != (previous.fps >= usefulFps)) {
        return value.fps >= usefulFps;
    }
    if (profile.protection == StreamProtection::kCurrentFrameRate) {
        // Preserve existing FPS first. Once that is met, restore spatial
        // detail before contemplating a new FPS peak.
        const auto maintained = std::min(value.fps, currentFps);
        const auto oldMaintained = std::min(previous.fps, currentFps);
        if (maintained != oldMaintained) {
            return maintained > oldMaintained;
        }
    }
    if (profile.upgradeOrder == StreamUpgradeOrder::kFrameRateThenResolution &&
        value.fps != previous.fps) {
        return value.fps > previous.fps;
    }
    if (Pixels(value.size) != Pixels(previous.size)) {
        return Pixels(value.size) > Pixels(previous.size);
    }
    return value.fps > previous.fps;
}

void SetBudget(ContentAwareStreamDecision& result,
    const ContentAwareStreamInput& input, std::uint64_t budget) noexcept
{
    result.desiredVideoBitrateBps = std::min(budget,
        std::max(result.requiredVideoBitrateBps, input.currentDesiredVideoBitrateBps));
    result.senderMaxBitrateBps = std::min(budget,
        std::max(result.desiredVideoBitrateBps, input.currentSenderMaxBitrateBps));
}

void HoldEstimate(ContentAwareStreamDecision& result,
    const ContentAwareStreamInput& input, const StreamSceneProfile& profile,
    const ContentAwareStreamConfig& config) noexcept
{
    const auto quality = Estimate({result.width, result.height, result.senderMaxFps,
        input.scene}, profile, config);
    result.requiredVideoBitrateBps = quality.requiredVideoBitrateBps;
    result.modelCalibrated = quality.available && quality.calibrated;
    result.modelReference = quality.available && quality.reference;
    // No network data means feasibility has not been established.
    result.estimatedFeasible = false;
    result.qualityRequirementMet = false;
    result.automaticControlEligible = false;
}

}  // namespace

std::uint64_t ContentAwareVideoBudgetBps(std::uint64_t capacity,
    std::uint64_t maximumVideoBitrateBps) noexcept
{
    // Split the arithmetic so even UINT64_MAX capacity cannot overflow.
    return std::min(capacity / 100 * 95 + capacity % 100 * 95 / 100,
        maximumVideoBitrateBps);
}

ContentAwareStreamConfig::ContentAwareStreamConfig() noexcept
{
    // Seed-only configuration. Each scene is addressable independently even
    // when sharing a protection order; real calibration lives in the model.
    const auto set = [&](ScreenScene scene, double scale, double bpp,
        std::uint32_t activeFps, double qp, double receiverMs,
        StreamProtection protection) noexcept {
        auto& value = profiles[SceneIndex(scene)];
        value.minimumSpatialScale = scale;
        value.seedBitsPerPixelPerFrame = bpp;
        value.minimumUsefulActiveFps = activeFps;
        value.maximumReferenceAverageQp = qp;
        value.maximumReceiverProcessingMs = receiverMs;
        value.protection = protection;
    };
    // These starting points implement the scene contract, NOT a benchmark.
    // Shared vendor reference does not imply shared scene quality thresholds.
    set(ScreenScene::kCodeTerminal, .65, .15, 15, 25, 50, StreamProtection::kSpatialDetail);
    set(ScreenScene::kDocument, .65, .145, 15, 26, 65, StreamProtection::kSpatialDetail);
    set(ScreenScene::kSpreadsheet, .75, .16, 15, 25, 50, StreamProtection::kSpatialDetail);
    set(ScreenScene::kWebApp, .60, .15, 20, 28, 45, StreamProtection::kSpatialDetail);
    set(ScreenScene::kPhotoGraphics, .65, .18, 15, 24, 65, StreamProtection::kSpatialDetail);
    set(ScreenScene::kCadDiagram, .75, .17, 20, 25, 45, StreamProtection::kSpatialDetail);
    set(ScreenScene::kVideo, .60, .14, 24, 30, 45, StreamProtection::kCurrentFrameRate);
    set(ScreenScene::kGame3d, .60, .16, 30, 30, 30, StreamProtection::kCurrentFrameRate);
    set(ScreenScene::kMixed, .65, .16, 20, 27, 45, StreamProtection::kSpatialDetail);
    // Classification can change rapidly for mixed UI; require longer recovery.
    profiles[SceneIndex(ScreenScene::kMixed)].upgradeMinimumMs = 3000;
    profiles[SceneIndex(ScreenScene::kMixed)].resolutionResidenceMs = 4000;
    profiles[SceneIndex(ScreenScene::kGame3d)].maximumEncodeFrameBudgetRatio = .70;
    profiles[SceneIndex(ScreenScene::kGame3d)].maximumDecodeFrameBudgetRatio = .70;
}

ContentAwareStreamDecision RecommendContentAwareStream(
    const ContentAwareStreamInput& input, const ContentAwareStreamConfig& config) noexcept
{
    ContentAwareStreamDecision result;
    if (!ValidConfig(config) || !input.sourceWidth || !input.sourceHeight ||
        input.sourceWidth > config.maximumDimension ||
        input.sourceHeight > config.maximumDimension || !input.maximumFrameRate) {
        return result;
    }
    const Size source{input.sourceWidth, input.sourceHeight};
    const Size bound{input.maximumWidth ? input.maximumWidth : source.width,
        input.maximumHeight ? input.maximumHeight : source.height};
    const Size cap = FitBounds(source, bound, config.alignment);
    if (!cap.width || !cap.height) {
        return result;
    }
    const std::uint32_t fpsCap = std::min(input.maximumFrameRate, 120u);
    Size current{input.currentWidth, input.currentHeight};
    if (!LegalSize(current, cap, source, config.alignment)) {
        current = input.currentWidth && input.currentHeight
            ? FitBounds(source, {std::min(cap.width, input.currentWidth),
                std::min(cap.height, input.currentHeight)}, config.alignment)
            : cap;
    }
    if (!current.width || !current.height) {
        return result;
    }
    std::uint32_t currentFps = input.currentFrameRate
        ? std::min(input.currentFrameRate, fpsCap) : fpsCap;
    Size requested;
    if (input.resolutionManual) {
        if (!input.requestedWidth || !input.requestedHeight) {
            return result;
        }
        requested = FitBounds(source, {std::min(cap.width, input.requestedWidth),
            std::min(cap.height, input.requestedHeight)}, config.alignment);
        if (!requested.width || !requested.height) {
            return result;
        }
        current = requested;
    }
    if (input.frameRateManual) {
        if (!input.requestedFrameRate) {
            return result;
        }
        currentFps = std::min(input.requestedFrameRate, fpsCap);
    }
    result.width = current.width;
    result.height = current.height;
    result.senderMaxFps = currentFps;
    result.desiredVideoBitrateBps = input.currentDesiredVideoBitrateBps;
    result.senderMaxBitrateBps = input.currentSenderMaxBitrateBps;
    result.hasRecommendation = true;
    const auto& profile = config.profiles[SceneIndex(input.scene)];
    const auto usefulFps = std::min(profile.minimumUsefulActiveFps, fpsCap);
    HoldEstimate(result, input, profile, config);

    // Validate capacity before idle/activity holds so even idle diagnostics
    // cannot accidentally show permission to exceed a known budget.
    if (!input.networkBudgetAvailable || !std::isfinite(input.safeVideoBudgetBps) ||
        input.safeVideoBudgetBps < 0.0 ||
        input.safeVideoBudgetBps >= static_cast<long double>(
            std::numeric_limits<std::uint64_t>::max())) {
        result.reason = ContentAwareStreamReason::kCapacityUnavailable;
        return result;
    }
    if (input.generation != input.networkGeneration) {
        result.reason = ContentAwareStreamReason::kGenerationMismatch;
        return result;
    }
    if (input.networkTimestampMs > input.nowMs ||
        input.nowMs - input.networkTimestampMs > config.maximumNetworkSampleAgeMs) {
        result.reason = ContentAwareStreamReason::kStaleNetwork;
        return result;
    }
    const auto budget = static_cast<std::uint64_t>(input.safeVideoBudgetBps);
    SetBudget(result, input, budget);
    if (!input.activityAvailable) {
        result.reason = ContentAwareStreamReason::kActivityUnavailable;
        return result;
    }
    if (!input.active) {
        result.reason = ContentAwareStreamReason::kIdleHold;
        return result;
    }

    const bool preserveSpecification = !input.networkConstrained;
    // Successful delivery is stronger evidence for the CURRENT specification
    // than an approximate pixel-rate curve. The curve still gates recovery to
    // higher specifications after a genuine downgrade.
    if (preserveSpecification && current.width == cap.width &&
        current.height == cap.height && currentFps == fpsCap &&
        input.qualityAvailable && input.qualityAcceptable) {
        result.desiredVideoBitrateBps = input.currentDesiredVideoBitrateBps;
        result.senderMaxBitrateBps = input.currentSenderMaxBitrateBps;
        result.estimatedFeasible = true;
        const bool modelQualified = result.modelCalibrated ||
            (result.modelReference && config.allowReferenceModel);
        result.qualityRequirementMet = modelQualified;
        result.activeFrameRateRequirementMet = currentFps >= usefulFps;
        result.automaticControlEligible = modelQualified && input.scene != ScreenScene::kUnknown;
        result.reason = ContentAwareStreamReason::kHealthyHold;
        return result;
    }

    const auto userRestoreBudget = ContentAwareVideoBudgetBps(
        input.userVideoBitrateCeilingBps, std::numeric_limits<std::uint64_t>::max());
    if (input.userSpecificationRecoveryAllowed && input.userVideoBitrateCeilingBps > 0 &&
        budget >= userRestoreBudget && SceneIndex(input.scene) != 0 &&
        SameSize(current, cap) && currentFps == fpsCap) {
        // The pressure episode remains open while GCC confirms stable normal.
        // Do not undo a just-restored user baseline because its approximate
        // quality curve exceeds the user's cap. QP remains diagnostic here.
        result.desiredVideoBitrateBps = input.currentDesiredVideoBitrateBps;
        result.senderMaxBitrateBps = input.currentSenderMaxBitrateBps;
        result.estimatedFeasible = result.requiredVideoBitrateBps <= budget &&
            (result.modelCalibrated || result.modelReference);
        result.qualityRequirementMet = result.estimatedFeasible &&
            (result.modelCalibrated || (result.modelReference && config.allowReferenceModel)) &&
            input.qualityAvailable && input.qualityAcceptable;
        result.activeFrameRateRequirementMet = currentFps >= usefulFps;
        result.reason = ContentAwareStreamReason::kHealthyHold;
        return result;
    }
    if (input.userSpecificationRecoveryAllowed && input.userVideoBitrateCeilingBps > 0 &&
        budget >= userRestoreBudget && SceneIndex(input.scene) != 0 &&
        (!input.resolutionManual || SameSize(requested, cap)) &&
        (!input.frameRateManual || currentFps == fpsCap) &&
        (!SameSize(current, cap) || currentFps != fpsCap)) {
        // GCC can now sustain the video ceiling the user selected. Restoring
        // that original specification must not deadlock behind an approximate
        // reference curve (or a strict QP limit) above that ceiling. Report the
        // unchanged model/quality result honestly and remeasure after applying.
        result.width = cap.width;
        result.height = cap.height;
        result.senderMaxFps = fpsCap;
        const auto quality = Estimate({cap.width, cap.height, fpsCap, input.scene},
            profile, config);
        result.requiredVideoBitrateBps = quality.requiredVideoBitrateBps;
        result.modelCalibrated = quality.available && quality.calibrated;
        result.modelReference = quality.available && quality.reference;
        result.estimatedFeasible = quality.available && quality.requiredVideoBitrateBps <= budget;
        result.qualityRequirementMet = result.estimatedFeasible &&
            (result.modelCalibrated || (result.modelReference && config.allowReferenceModel)) &&
            input.qualityAvailable && input.qualityAcceptable;
        result.desiredVideoBitrateBps = result.senderMaxBitrateBps =
            std::min(budget, input.userVideoBitrateCeilingBps);
        result.activeFrameRateRequirementMet = fpsCap >= usefulFps;
        result.userSpecificationRestoreRequired = true;
        result.automaticControlEligible = true;
        result.reason = ContentAwareStreamReason::kUserSpecificationRestore;
        return result;
    }

    Size anchor{input.stableAnchorWidth, input.stableAnchorHeight};
    if (!LegalSize(anchor, cap, source, config.alignment)) {
        anchor = current;
    }
    const double minimumScale = profile.protection == StreamProtection::kCurrentFrameRate
        ? std::max(profile.minimumSpatialScale,
            std::min(static_cast<double>(anchor.width) / cap.width,
                static_cast<double>(anchor.height) / cap.height) *
                (1.0 - profile.maximumSpatialReductionFromAnchor))
        : profile.minimumSpatialScale;
    const Size minimum = FitBounds(source,
        {static_cast<std::uint32_t>(cap.width * minimumScale),
         static_cast<std::uint32_t>(cap.height * minimumScale)}, config.alignment);

    std::array<Size, kMaximumSizes> sizes{};
    std::size_t sizeCount = 0;
    const auto addSize = [&](Size size) noexcept {
        if (LegalSize(size, cap, source, config.alignment) &&
            size.width >= minimum.width && size.height >= minimum.height) {
            AddUnique(sizes, sizeCount, size, SameSize);
        }
    };
    if (input.resolutionManual) {
        // Manual dimensions remain explicit even below the scene seed floor.
        sizes[sizeCount++] = requested;
    } else {
        addSize(cap);
        addSize(current);
        addSize(anchor);
        addSize(minimum);
        for (const auto height : config.referenceHeights) {
            if (!height) {
                continue;
            }
            Size reference{height * 16u / 9u, height};
            if (source.height > source.width) {
                std::swap(reference.width, reference.height);
            }
            addSize(FitBounds(source, reference, config.alignment));
        }
    }
    std::array<std::uint32_t, kMaximumFrameRates> frameRates{};
    std::size_t frameRateCount = 0;
    std::uint32_t nextLowerFps = currentFps;
    if (!input.frameRateManual && input.networkConstrained) {
        std::uint32_t nearestLower = 0;
        for (const auto fps : config.frameRateCandidates) {
            if (fps < currentFps) nearestLower = std::max(nearestLower, fps);
        }
        if (nearestLower) nextLowerFps = nearestLower;
    }
    const auto addFps = [&](std::uint32_t fps) noexcept {
        // Real sender recommendations take one downward step at a time.
        // A severe reference quality shortfall must not jump directly from
        // 120 or 60 FPS to the minimum. Every next step needs fresh evidence,
        // so recovering networking can stop the sequence between steps.
        if (fps && fps <= fpsCap && fps >= nextLowerFps) {
            AddUnique(frameRates, frameRateCount, fps,
                [](std::uint32_t a, std::uint32_t b) noexcept { return a == b; });
        }
    };
    addFps(currentFps);
    if (!input.frameRateManual) {
        addFps(fpsCap); // preserve exact non-standard requests (e.g. 75 FPS)
        for (const auto fps : config.frameRateCandidates) {
            addFps(fps);
        }
    }

    Candidate best;
    Candidate emergency;
    bool found = false;
    bool emergencyFound = false;
    const bool qualityBad = input.qualityAvailable && !input.qualityAcceptable;
    const bool qualityAllowsUpgrade = input.qualityAvailable && input.qualityAcceptable;
    const auto desiredSize = FitSize(cap, profile.desiredSpatialScale, config.alignment);
    for (std::size_t s = 0; s < sizeCount; ++s) {
        for (std::size_t f = 0; f < frameRateCount; ++f) {
            Candidate candidate{sizes[s], frameRates[f],
                Estimate({sizes[s].width, sizes[s].height, frameRates[f], input.scene},
                    profile, config), false};
            if (preserveSpecification &&
                (Pixels(candidate.size) < Pixels(current) || candidate.fps < currentFps)) {
                continue;
            }
            if (!candidate.quality.available) {
                continue;
            }
            const bool verifiedUserSpecification = qualityAllowsUpgrade &&
                candidate.quality.reference && !candidate.quality.calibrated &&
                input.userSpecificationVerifiedBitrateBps != 0 &&
                SameSize(candidate.size, cap) && candidate.fps == fpsCap;
            if (verifiedUserSpecification) {
                // Sustained delivery of this exact user specification is
                // stronger evidence than the approximate pixel-rate curve.
                // This does not promote the reference to measured calibration.
                candidate.quality.requiredVideoBitrateBps =
                    input.userSpecificationVerifiedBitrateBps;
            }
            if (qualityBad && input.qualityRecoveryBitrateBps) {
                const auto currentPixelRate = static_cast<long double>(Pixels(current)) * currentFps;
                const auto candidatePixelRate = static_cast<long double>(Pixels(candidate.size)) * candidate.fps;
                const auto recoveryRequirement = std::ceil(
                    static_cast<long double>(input.qualityRecoveryBitrateBps) *
                    candidatePixelRate / currentPixelRate);
                if (!std::isfinite(recoveryRequirement) || recoveryRequirement <= 0.0L ||
                    recoveryRequirement >= static_cast<long double>(
                        std::numeric_limits<std::uint64_t>::max())) {
                    continue;
                }
                // Measured model demand remains a lower bound. Fresh bad
                // quality can ask for more B at the same R/F before sacrificing
                // detail or fluidity; smaller candidates ease that pressure.
                candidate.quality.requiredVideoBitrateBps = std::max(
                    candidate.quality.requiredVideoBitrateBps,
                    static_cast<std::uint64_t>(recoveryRequirement));
            }
            // Never trade sustainable resolution for a new FPS peak, even
            // in motion scenes. Restore a degraded stable anchor first.
            if (candidate.fps > currentFps &&
                (Pixels(candidate.size) < Pixels(current) ||
                 Pixels(candidate.size) < Pixels(anchor))) {
                continue;
            }
            const bool upgrade = Upgrades(candidate, current, currentFps);
            if (Pixels(candidate.size) > Pixels(current) &&
                Pixels(candidate.size) > Pixels(desiredSize)) {
                continue;
            }
            if (upgrade && (SceneIndex(input.scene) == 0 || !qualityAllowsUpgrade ||
                static_cast<long double>(
                    candidate.quality.requiredVideoBitrateBps) *
                    (verifiedUserSpecification ? 1.0 : config.upgradeHeadroomRatio) > budget)) {
                continue;
            }
            // Processing time is diagnostic only. It is not proof of weak
            // networking and cannot veto a lower-load scene allocation.
            // Bad observed quality forbids accepting an unchanged pixel rate
            // unless opening the model's required budget can address it.
            if (qualityBad && Pixels(candidate.size) >= Pixels(current) &&
                candidate.fps >= currentFps &&
                candidate.quality.requiredVideoBitrateBps <=
                    input.currentDesiredVideoBitrateBps) {
                continue;
            }
            const bool reducedLoad = !Upgrades(candidate, current, currentFps) &&
                (Pixels(candidate.size) < Pixels(current) || candidate.fps < currentFps);
            const bool previousReducedLoad = emergencyFound &&
                !Upgrades(emergency, current, currentFps) &&
                (Pixels(emergency.size) < Pixels(current) || emergency.fps < currentFps);
            if (!emergencyFound || (reducedLoad && !previousReducedLoad) ||
                (reducedLoad == previousReducedLoad &&
                    (reducedLoad ? Better(candidate, emergency, profile, currentFps, usefulFps) :
                        candidate.quality.requiredVideoBitrateBps < emergency.quality.requiredVideoBitrateBps))) {
                emergency = candidate;
                emergencyFound = true;
            }
            candidate.feasible = candidate.quality.requiredVideoBitrateBps <= budget;
            if (candidate.feasible && (!found || Better(candidate, best,
                profile, currentFps, usefulFps))) {
                best = candidate;
                found = true;
            }
        }
    }
    if (!found && !emergencyFound) {
        result.reason = ContentAwareStreamReason::kQualityLimited;
        return result;
    }
    if (preserveSpecification && !found) {
        result.desiredVideoBitrateBps = input.currentDesiredVideoBitrateBps;
        result.senderMaxBitrateBps = input.currentSenderMaxBitrateBps;
        result.reason = ContentAwareStreamReason::kHealthyHold;
        return result;
    }
    const auto& choice = found ? best : emergency;
    result.width = choice.size.width;
    result.height = choice.size.height;
    result.senderMaxFps = choice.fps;
    result.requiredVideoBitrateBps = choice.quality.requiredVideoBitrateBps;
    result.modelCalibrated = choice.quality.calibrated;
    result.modelReference = choice.quality.reference;
    result.estimatedFeasible = found;
    const bool modelQualified = result.modelCalibrated ||
        (result.modelReference && config.allowReferenceModel);
    // A reference quality floor is a quality estimate, not a prohibition on
    // easing confirmed network overload. A scene-prioritized downward step
    // can reduce load even when its budget cannot meet the quality estimate.
    // Preserve the failed quality/feasibility flags and never use this path to
    // upgrade, change locked dimensions, or act without fresh network evidence.
    const bool emergencyNetworkReduction = !found && budget > 0 && modelQualified &&
        input.networkConstrained && SceneIndex(input.scene) != 0 &&
        !Upgrades(choice, current, currentFps) &&
        (Pixels(choice.size) < Pixels(current) || choice.fps < currentFps);
    result.qualityRequirementMet = found && modelQualified &&
        input.qualityAvailable && input.qualityAcceptable;
    result.qualityRepairRequired = found && modelQualified && qualityBad &&
        !Upgrades(choice, current, currentFps) &&
        (Pixels(choice.size) < Pixels(current) || choice.fps < currentFps ||
         choice.quality.requiredVideoBitrateBps > input.currentDesiredVideoBitrateBps);
    result.networkRepairRequired = (found || emergencyNetworkReduction) && modelQualified && input.networkConstrained &&
        SceneIndex(input.scene) != 0 && !Upgrades(choice, current, currentFps) &&
        (Pixels(choice.size) < Pixels(current) || choice.fps < currentFps);
    result.activeFrameRateRequirementMet = choice.fps >= usefulFps;
    result.automaticControlEligible = result.qualityRequirementMet || result.qualityRepairRequired ||
        result.networkRepairRequired;
    SetBudget(result, input, budget);
    if (preserveSpecification && !qualityBad &&
        choice.size.width == current.width && choice.size.height == current.height &&
        choice.fps == currentFps) {
        result.desiredVideoBitrateBps = input.currentDesiredVideoBitrateBps;
        result.senderMaxBitrateBps = input.currentSenderMaxBitrateBps;
        result.reason = ContentAwareStreamReason::kHealthyHold;
        return result;
    }
    if (emergencyNetworkReduction) {
        result.reason = ContentAwareStreamReason::kEmergencyNetworkReduction;
    } else if (!found) {
        result.reason = input.resolutionManual || input.frameRateManual
            ? ContentAwareStreamReason::kManualQualityLimited
            : ContentAwareStreamReason::kQualityLimited;
    } else if (!result.activeFrameRateRequirementMet) {
        result.reason = ContentAwareStreamReason::kActivityFrameRateLimited;
    } else if (Pixels(choice.size) > Pixels(current)) {
        result.reason = ContentAwareStreamReason::kResolutionUpgrade;
    } else if (choice.fps > currentFps) {
        result.reason = ContentAwareStreamReason::kFrameRateUpgrade;
    } else if (Pixels(choice.size) < Pixels(current) && choice.fps >= currentFps) {
        result.reason = ContentAwareStreamReason::kProtectFrameRate;
    } else if (choice.fps < currentFps) {
        result.reason = ContentAwareStreamReason::kProtectResolution;
    } else if (result.desiredVideoBitrateBps > input.currentDesiredVideoBitrateBps) {
        result.reason = ContentAwareStreamReason::kBitrateRecovery;
    } else if (SceneIndex(input.scene) == 0) {
        result.reason = ContentAwareStreamReason::kUnknownSceneHold;
    } else if (!qualityAllowsUpgrade) {
        result.reason = ContentAwareStreamReason::kAwaitingQualityEvidence;
    } else {
        result.reason = ContentAwareStreamReason::kHold;
    }
    return result;
}

const char* ContentAwareStreamReasonName(ContentAwareStreamReason reason) noexcept
{
    switch (reason) {
    case ContentAwareStreamReason::kInvalidInput: return "invalid_input";
    case ContentAwareStreamReason::kIdleHold: return "idle_hold";
    case ContentAwareStreamReason::kActivityUnavailable: return "activity_unavailable";
    case ContentAwareStreamReason::kCapacityUnavailable: return "capacity_unavailable";
    case ContentAwareStreamReason::kStaleNetwork: return "stale_network";
    case ContentAwareStreamReason::kGenerationMismatch: return "generation_mismatch";
    case ContentAwareStreamReason::kUnknownSceneHold: return "unknown_scene_hold";
    case ContentAwareStreamReason::kHold: return "hold";
    case ContentAwareStreamReason::kQualityLimited: return "quality_limited";
    case ContentAwareStreamReason::kManualQualityLimited: return "manual_quality_limited";
    case ContentAwareStreamReason::kProtectResolution: return "protect_resolution";
    case ContentAwareStreamReason::kProtectFrameRate: return "protect_frame_rate";
    case ContentAwareStreamReason::kResolutionUpgrade: return "resolution_upgrade";
    case ContentAwareStreamReason::kFrameRateUpgrade: return "frame_rate_upgrade";
    case ContentAwareStreamReason::kBitrateRecovery: return "bitrate_recovery";
    case ContentAwareStreamReason::kProcessingLimited: return "processing_limited";
    case ContentAwareStreamReason::kActivityFrameRateLimited: return "activity_frame_rate_limited";
    case ContentAwareStreamReason::kAwaitingQualityEvidence: return "awaiting_quality_evidence";
    case ContentAwareStreamReason::kHealthyHold: return "healthy_hold";
    case ContentAwareStreamReason::kEmergencyNetworkReduction: return "emergency_network_reduction";
    case ContentAwareStreamReason::kUserSpecificationRestore: return "user_specification_restore";
    }
    return "invalid_input";
}

const char* ContentAwareStreamConfirmationBlockName(
    ContentAwareStreamConfirmationBlock block) noexcept
{
    switch (block) {
    case ContentAwareStreamConfirmationBlock::kNotApplicable: return "not_applicable";
    case ContentAwareStreamConfirmationBlock::kAwaitingFreshSample: return "awaiting_fresh_sample";
    case ContentAwareStreamConfirmationBlock::kAwaitingEvidence: return "awaiting_evidence";
    case ContentAwareStreamConfirmationBlock::kAwaitingSamples: return "awaiting_samples";
    case ContentAwareStreamConfirmationBlock::kAwaitingStableTime: return "awaiting_stable_time";
    case ContentAwareStreamConfirmationBlock::kAwaitingResidence: return "awaiting_residence";
    case ContentAwareStreamConfirmationBlock::kAwaitingApplyInterval: return "awaiting_apply_interval";
    case ContentAwareStreamConfirmationBlock::kReady: return "ready";
    }
    return "not_applicable";
}

ContentAwareStreamEvaluation EvaluateContentAwareStream(
    ContentAwareStreamPolicyState& state, const ContentAwareStreamInput& input,
    const ContentAwareStreamConfig& config,
    const ContentAwareStreamHysteresis& hysteresis) noexcept
{
    if (!state.initialized || state.generation != input.generation) {
        state = {};
        state.initialized = true;
        state.generation = input.generation;
        state.lastScene = input.scene;
        state.anchorWidth = input.currentWidth;
        state.anchorHeight = input.currentHeight;
        state.appliedWidth = input.currentWidth;
        state.appliedHeight = input.currentHeight;
        state.lastAppliedMs = input.nowMs;
        state.lastResolutionChangeMs = input.nowMs;
    }
    if (state.lastScene != input.scene) {
        // Equal R/F/B from different scene contracts is not consecutive proof.
        // Retain the pre-downgrade spatial anchor and applied timestamps.
        state.lastScene = input.scene;
        state.pendingSamples = 0;
        state.pendingConfirmed = false;
    }
    auto anchored = input;
    anchored.stableAnchorWidth = state.anchorWidth;
    anchored.stableAnchorHeight = state.anchorHeight;
    ContentAwareStreamEvaluation result{
        RecommendContentAwareStream(anchored, config), false};
    auto& decision = result.recommendation;
    const bool validSample = decision.hasRecommendation && input.activityAvailable &&
        input.active && input.networkBudgetAvailable &&
        input.networkGeneration == state.generation &&
        input.networkTimestampMs <= input.nowMs &&
        input.nowMs - input.networkTimestampMs <= config.maximumNetworkSampleAgeMs &&
        std::isfinite(input.safeVideoBudgetBps) && input.safeVideoBudgetBps >= 0.0;
    if (!validSample) {
        state.pendingSamples = 0;
        state.pendingConfirmed = false;
        result.confirmationBlock = ContentAwareStreamConfirmationBlock::kAwaitingEvidence;
        return result;
    }
    if (state.hasSample &&
        input.networkTimestampMs <= state.lastSampleTimestampMs) {
        // Duplicate/late windows neither confirm an action nor contaminate
        // the consecutive fresh-window counter.
        result.confirmObservedSamples = state.pendingSamples;
        result.confirmationBlock = ContentAwareStreamConfirmationBlock::kAwaitingFreshSample;
        return result;
    }
    if (state.hasSample && input.networkTimestampMs - state.lastSampleTimestampMs >
        config.maximumNetworkSampleAgeMs) {
        // Windows separated by a long feedback gap are not consecutive proof.
        state.pendingSamples = 0;
        state.pendingConfirmed = false;
    }
    state.lastSampleTimestampMs = input.networkTimestampMs;
    state.hasSample = true;
    if (!decision.automaticControlEligible) {
        state.pendingSamples = 0;
        state.pendingConfirmed = false;
        result.confirmationBlock = ContentAwareStreamConfirmationBlock::kAwaitingEvidence;
        return result;
    }
    const bool changed = decision.width != input.currentWidth ||
        decision.height != input.currentHeight ||
        decision.senderMaxFps != input.currentFrameRate ||
        decision.desiredVideoBitrateBps != input.currentDesiredVideoBitrateBps ||
        decision.senderMaxBitrateBps != input.currentSenderMaxBitrateBps;
    if (!changed) {
        state.pendingSamples = 0;
        state.pendingConfirmed = false;
        return result;
    }
    const bool spatialOrFpsUpgrade = Pixels({decision.width, decision.height}) >
        Pixels({input.currentWidth, input.currentHeight}) ||
        decision.senderMaxFps > input.currentFrameRate;
    const auto userCap = FitBounds({input.sourceWidth, input.sourceHeight},
        {input.maximumWidth ? input.maximumWidth : input.sourceWidth,
         input.maximumHeight ? input.maximumHeight : input.sourceHeight}, config.alignment);
    const bool verifiedUserSpecification = decision.modelReference &&
        !decision.modelCalibrated && input.userSpecificationVerifiedBitrateBps > 0 &&
        decision.requiredVideoBitrateBps == input.userSpecificationVerifiedBitrateBps &&
        decision.width == userCap.width && decision.height == userCap.height &&
        decision.senderMaxFps == std::min(input.maximumFrameRate, 120u);
    const auto restoreMinimumBudget = ContentAwareVideoBudgetBps(
        input.userVideoBitrateCeilingBps, std::numeric_limits<std::uint64_t>::max());
    const long double requiredSafeBudget = decision.userSpecificationRestoreRequired ?
        static_cast<long double>(restoreMinimumBudget) :
        static_cast<long double>(decision.requiredVideoBitrateBps) *
            (spatialOrFpsUpgrade && !verifiedUserSpecification ? config.upgradeHeadroomRatio : 1.0);
    const auto currentSafeBudget = input.safeVideoBudgetBps >= static_cast<double>(
        std::numeric_limits<std::uint64_t>::max()) ?
        std::numeric_limits<std::uint64_t>::max() :
        static_cast<std::uint64_t>(input.safeVideoBudgetBps);
    const auto conservativeSafeBudget = std::min(currentSafeBudget,
        state.pendingSafeVideoBudgetBps);
    const auto conservativeDesired = std::min(decision.desiredVideoBitrateBps,
        state.pendingDesiredVideoBitrateBps);
    const auto conservativeMaximum = std::min(decision.senderMaxBitrateBps,
        state.pendingSenderMaxBitrateBps);
    const bool emergencyReduction = decision.reason ==
        ContentAwareStreamReason::kEmergencyNetworkReduction && decision.networkRepairRequired &&
        !spatialOrFpsUpgrade;
    const bool compatibleBudget =
        (emergencyReduction ? conservativeSafeBudget > 0 :
            conservativeSafeBudget >= requiredSafeBudget) &&
        (emergencyReduction ? conservativeDesired > 0 :
            conservativeDesired >= (decision.userSpecificationRestoreRequired ?
                restoreMinimumBudget : decision.requiredVideoBitrateBps)) &&
        conservativeMaximum >= conservativeDesired &&
        (decision.desiredVideoBitrateBps > input.currentDesiredVideoBitrateBps) ==
            (state.pendingDesiredVideoBitrateBps > input.currentDesiredVideoBitrateBps) &&
        (decision.senderMaxBitrateBps > input.currentSenderMaxBitrateBps) ==
            (state.pendingSenderMaxBitrateBps > input.currentSenderMaxBitrateBps);
    if (decision.width != state.pendingWidth || decision.height != state.pendingHeight ||
        decision.senderMaxFps != state.pendingFrameRate ||
        !compatibleBudget ||
        decision.qualityRepairRequired != state.pendingQualityRepairRequired ||
        decision.networkRepairRequired != state.pendingNetworkRepairRequired ||
        decision.userSpecificationRestoreRequired != state.pendingUserSpecificationRestoreRequired ||
        decision.processingRepairRequired != state.pendingProcessingRepairRequired ||
        !state.pendingSamples) {
        state.pendingWidth = decision.width;
        state.pendingHeight = decision.height;
        state.pendingFrameRate = decision.senderMaxFps;
        state.pendingDesiredVideoBitrateBps = decision.desiredVideoBitrateBps;
        state.pendingSenderMaxBitrateBps = decision.senderMaxBitrateBps;
        state.pendingSafeVideoBudgetBps = currentSafeBudget;
        state.pendingQualityRepairRequired = decision.qualityRepairRequired;
        state.pendingNetworkRepairRequired = decision.networkRepairRequired;
        state.pendingUserSpecificationRestoreRequired = decision.userSpecificationRestoreRequired;
        state.pendingProcessingRepairRequired = decision.processingRepairRequired;
        state.pendingSamples = 1;
        state.pendingConfirmed = false;
        state.pendingSinceMs = input.nowMs;
    } else if (state.pendingSamples < std::numeric_limits<std::uint32_t>::max()) {
        ++state.pendingSamples;
        // GCC can ramp by far more than 5%. Confirm the same feasible action
        // using the minimum budget across all windows; changing the budget
        // alone must not repeatedly restart recovery or delay a safe reduction.
        state.pendingSafeVideoBudgetBps = conservativeSafeBudget;
        decision.desiredVideoBitrateBps = state.pendingDesiredVideoBitrateBps = conservativeDesired;
        decision.senderMaxBitrateBps = state.pendingSenderMaxBitrateBps = conservativeMaximum;
    }
    const bool upgrade = !decision.qualityRepairRequired && !decision.networkRepairRequired &&
        !decision.processingRepairRequired &&
        (Pixels({decision.width, decision.height}) >
        Pixels({input.currentWidth, input.currentHeight}) ||
        decision.senderMaxFps > input.currentFrameRate ||
        decision.desiredVideoBitrateBps > input.currentDesiredVideoBitrateBps ||
        decision.senderMaxBitrateBps > input.currentSenderMaxBitrateBps);
    const auto& profile = config.profiles[SceneIndex(input.scene)];
    const auto upgradeMinimumMs = std::max(hysteresis.upgradeMinimumMs, profile.upgradeMinimumMs);
    const auto residenceMs = std::max(hysteresis.resolutionResidenceMs, profile.resolutionResidenceMs);
    const bool resolutionChanged = decision.width != input.currentWidth ||
        decision.height != input.currentHeight;
    const auto remaining = [now = input.nowMs](std::uint64_t since,
        std::uint64_t interval) noexcept {
        return now < since ? interval :
            (now - since >= interval ? 0 : interval - (now - since));
    };
    result.confirmObservedSamples = state.pendingSamples;
    result.confirmRequiredSamples = std::max(1u,
        upgrade ? hysteresis.upgradeSamples : hysteresis.downgradeSamples);
    const auto stableRemaining = upgrade ? remaining(state.pendingSinceMs, upgradeMinimumMs) : 0;
    const auto intervalRemaining = upgrade ? 0 :
        remaining(state.lastAppliedMs, hysteresis.downgradeMinimumIntervalMs);
    // Spatial residence prevents recovery oscillation. A fresh, qualified
    // congestion repair must not sit behind that recovery timer.
    const auto residenceRemaining = resolutionChanged &&
        (upgrade || (state.resolutionChangeApplied && !decision.networkRepairRequired)) ?
        remaining(state.lastResolutionChangeMs, residenceMs) : 0;
    result.confirmationRemainingMs = std::max({stableRemaining,
        intervalRemaining, residenceRemaining});
    if (state.pendingSamples < result.confirmRequiredSamples) {
        result.confirmationBlock = ContentAwareStreamConfirmationBlock::kAwaitingSamples;
    } else if (residenceRemaining && residenceRemaining >= stableRemaining &&
        residenceRemaining >= intervalRemaining) {
        result.confirmationBlock = ContentAwareStreamConfirmationBlock::kAwaitingResidence;
    } else if (stableRemaining && stableRemaining >= intervalRemaining) {
        result.confirmationBlock = ContentAwareStreamConfirmationBlock::kAwaitingStableTime;
    } else if (intervalRemaining) {
        result.confirmationBlock = ContentAwareStreamConfirmationBlock::kAwaitingApplyInterval;
    } else {
        result.confirmed = true;
        result.confirmationBlock = ContentAwareStreamConfirmationBlock::kReady;
    }
    state.pendingConfirmed = result.confirmed;
    return result;
}

void ConfirmContentAwareStreamApplied(ContentAwareStreamPolicyState& state,
    const ContentAwareStreamInput& effective, bool qualityVerified) noexcept
{
    if (!state.initialized || state.generation != effective.generation ||
        effective.nowMs < state.lastAppliedMs) {
        return;
    }
    if (effective.currentWidth == 0 || effective.currentHeight == 0 ||
        effective.currentFrameRate == 0) {
        return;
    }
    if (effective.currentWidth != state.pendingWidth ||
        effective.currentHeight != state.pendingHeight ||
        effective.currentFrameRate != state.pendingFrameRate ||
        effective.currentDesiredVideoBitrateBps != state.pendingDesiredVideoBitrateBps ||
        effective.currentSenderMaxBitrateBps != state.pendingSenderMaxBitrateBps ||
        !state.pendingSamples || !state.pendingConfirmed) {
        return;
    }
    if (state.appliedWidth != effective.currentWidth ||
        state.appliedHeight != effective.currentHeight) {
        state.lastResolutionChangeMs = effective.nowMs;
        state.resolutionChangeApplied = true;
    }
    state.appliedWidth = effective.currentWidth;
    state.appliedHeight = effective.currentHeight;
    state.lastAppliedMs = effective.nowMs;
    // A verified smaller output is NOT a new anchor: cumulative loss remains
    // relative to the pre-downgrade stable specification.
    if (qualityVerified && Pixels({effective.currentWidth, effective.currentHeight}) >=
        Pixels({state.anchorWidth, state.anchorHeight})) {
        state.anchorWidth = effective.currentWidth;
        state.anchorHeight = effective.currentHeight;
    }
    state.pendingSamples = 0;
    state.pendingConfirmed = false;
}

}  // namespace remote::media_intelligence
