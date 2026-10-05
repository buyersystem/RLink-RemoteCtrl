// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "media_intelligence/core/ScreenScene.h"

namespace remote::media_intelligence {

enum class StreamProtection : std::uint8_t {
    kSpatialDetail,
    kCurrentFrameRate,
};

enum class StreamUpgradeOrder : std::uint8_t {
    kResolutionThenFrameRate,
    kFrameRateThenResolution,
};

struct StreamSceneProfile {
    StreamProtection protection = StreamProtection::kSpatialDetail;
    // Linear scale relative to source/caps. These are configurable seed
    // constraints, not measured perceptual guarantees.
    double minimumSpatialScale = 0.5;
    double maximumSpatialReductionFromAnchor = 0.10;
    double seedBitsPerPixelPerFrame = 0.15;
    // Explicit product reference values; applications can replace them with
    // encoder/scene measurements. They are not calibrated quality promises.
    double desiredSpatialScale = 1.0;
    std::uint32_t minimumUsefulActiveFps = 15;
    double maximumReferenceAverageQp = 28.0;
    double maximumEncodeFrameBudgetRatio = 0.80;
    double maximumDecodeFrameBudgetRatio = 0.80;
    double maximumReceiverProcessingMs = 50.0;
    StreamUpgradeOrder upgradeOrder = StreamUpgradeOrder::kResolutionThenFrameRate;
    // Zero inherits the host hysteresis; a nonzero value adds a scene floor.
    std::uint64_t upgradeMinimumMs = 0;
    std::uint64_t resolutionResidenceMs = 0;
};

struct StreamQualityRequest {
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t frameRate = 0;
    ScreenScene scene = ScreenScene::kUnknown;
};

struct StreamQualityEstimate {
    std::uint64_t requiredVideoBitrateBps = 0;
    bool available = false;
    // The host supplies calibration by codec/encoder/profile. The default
    // pixel-rate seed never claims to be calibrated.
    bool calibrated = false;
    bool processingFeasible = true;
    // Explicit approximate codec/encoder reference, never measured calibration.
    bool reference = false;
};

using StreamQualityEstimator = StreamQualityEstimate (*)(
    const StreamQualityRequest&, const void* context) noexcept;

struct ContentAwareStreamConfig {
    std::array<StreamSceneProfile, 10> profiles{};
    // Zero entries are unused. Capacity is fixed; callers may replace seed
    // tiers without allocating or changing the selection algorithm.
    std::array<std::uint32_t, 16> referenceHeights{
        720, 810, 900, 990, 1080, 1170, 1260,
        1350, 1440, 1620, 1800, 1980, 2160};
    std::array<std::uint32_t, 12> frameRateCandidates{
        120, 100, 90, 75, 60, 50, 45, 30, 24, 20, 15};
    StreamQualityEstimator qualityEstimator = nullptr;
    const void* qualityEstimatorContext = nullptr;
    std::uint64_t burstAllowanceBps = 0;
    std::uint64_t maximumNetworkSampleAgeMs = 3000;
    double upgradeHeadroomRatio = 1.15;
    std::uint32_t alignment = 2;
    // The bound also limits work and arithmetic. Requests beyond this are
    // rejected instead of silently overflowing a pixel-rate calculation.
    std::uint32_t maximumDimension = 16384;
    // Standalone hosts must explicitly opt in to approximate reference models.
    // This does not turn a reference into calibration. Original-user baseline
    // restoration is separately authorized by the host's fresh network evidence.
    bool allowReferenceModel = false;

    ContentAwareStreamConfig() noexcept;
};

struct ContentAwareStreamInput {
    std::uint32_t sourceWidth = 0;
    std::uint32_t sourceHeight = 0;
    std::uint32_t maximumWidth = 0;   // zero means source bound
    std::uint32_t maximumHeight = 0;
    std::uint32_t maximumFrameRate = 120;
    std::uint32_t currentWidth = 0;
    std::uint32_t currentHeight = 0;
    std::uint32_t currentFrameRate = 0;
    std::uint64_t currentDesiredVideoBitrateBps = 0;
    std::uint64_t currentSenderMaxBitrateBps = 0;

    bool resolutionManual = false;
    std::uint32_t requestedWidth = 0;
    std::uint32_t requestedHeight = 0;
    bool frameRateManual = false;
    std::uint32_t requestedFrameRate = 0;

    ScreenScene scene = ScreenScene::kUnknown;
    bool activityAvailable = false;
    bool active = false;
    // Safe VIDEO budget, already reconciled by the host with its connection
    // limit/reservations. Never substitute actual sent rate for this value.
    bool networkBudgetAvailable = false;
    // Host evidence of actual network pressure, independent of the model's
    // theoretical demand. A reference estimate alone never authorizes R/F loss.
    bool networkConstrained = false;
    // Original user ceiling, not the current adapted sender ceiling. The host
    // grants restoration only from fresh GCC recovery evidence with no current
    // congestion pressure. This restores the user's baseline, never preference
    // upgrades beyond it or a claim that the quality model has been calibrated.
    std::uint64_t userVideoBitrateCeilingBps = 0;
    bool userSpecificationRecoveryAllowed = false;
    double safeVideoBudgetBps = 0.0;
    std::uint64_t nowMs = 0;
    std::uint64_t networkTimestampMs = 0;
    std::uint64_t generation = 0;
    std::uint64_t networkGeneration = 0;

    bool qualityAvailable = false;
    bool qualityAcceptable = false;
    // Optional same-scene/user-specification measured demand. The host must
    // qualify it using sustained active, good-QP, adequate delivery windows and
    // invalidate it on route/scene/user-spec changes. It replaces only the
    // approximate reference at the exact user's maximum R/F, never calibrated
    // estimates or intermediate candidates. Existing quality/time confirmation
    // still gates recovery; zero leaves reference qualification unchanged.
    std::uint64_t userSpecificationVerifiedBitrateBps = 0;
    // A host may request a bounded bitrate recovery step from fresh bad-QP
    // evidence (for example current budget * 1.15). Used only for repairs, never
    // upgrades or an instruction to fill all spare capacity. Candidate pressure
    // scales by pixel rate; actual target remains owned by congestion control.
    std::uint64_t qualityRecoveryBitrateBps = 0;
    // Diagnostic only; neither missing timing nor a timing threshold overrun
    // authorizes R/F loss or blocks adaptation within the user's bounds.
    bool processingAvailable = false;
    bool processingHealthy = false;
    // The anchor must survive successive downgrades. Zero uses the current
    // specification; stateful callers should pass a previously stable anchor.
    std::uint32_t stableAnchorWidth = 0;
    std::uint32_t stableAnchorHeight = 0;
};

enum class ContentAwareStreamReason : std::uint8_t {
    kInvalidInput,
    kIdleHold,
    kActivityUnavailable,
    kCapacityUnavailable,
    kStaleNetwork,
    kGenerationMismatch,
    kUnknownSceneHold,
    kHold,
    kQualityLimited,
    kManualQualityLimited,
    kProtectResolution,
    kProtectFrameRate,
    kResolutionUpgrade,
    kFrameRateUpgrade,
    kBitrateRecovery,
    kProcessingLimited,
    kActivityFrameRateLimited,
    kAwaitingQualityEvidence,
    kHealthyHold,
    kEmergencyNetworkReduction,
    kUserSpecificationRestore,
};

struct ContentAwareStreamDecision {
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t senderMaxFps = 0;
    std::uint64_t requiredVideoBitrateBps = 0;
    std::uint64_t desiredVideoBitrateBps = 0;
    std::uint64_t senderMaxBitrateBps = 0;
    bool hasRecommendation = false;
    bool estimatedFeasible = false;
    bool modelCalibrated = false;
    bool modelReference = false;
    // True only for a qualified calibrated/explicitly allowed reference model
    // and acceptable current quality evidence. For a reference this remains a
    // runtime proxy check, not a measured visual-quality guarantee.
    bool qualityRequirementMet = false;
    // Execution requires qualified model and feasible budget, plus either live
    // quality approval or a bounded quality/GoogCC network repair. Processing
    // feedback is diagnostic and cannot veto execution.
    bool automaticControlEligible = false;
    // A repair is permission to attempt a bounded correction using a qualified
    // model, NOT confirmation that the new quality already passed. It requires
    // available bad quality evidence and rejects any R/F upgrade.
    bool qualityRepairRequired = false;
    // Fresh GoogCC pressure may authorize a feasible non-upgrading R/F repair
    // before QP feedback arrives or deteriorates. This is permission to attempt
    // a scene allocation, not confirmation of its resulting perceptual quality.
    bool networkRepairRequired = false;
    bool userSpecificationRestoreRequired = false;
    bool processingRepairRequired = false;
    // A real budget shortage may require an emergency FPS below the scene's
    // useful activity floor. The host must report that limitation explicitly.
    bool activeFrameRateRequirementMet = false;
    ContentAwareStreamReason reason = ContentAwareStreamReason::kInvalidInput;
};

// Pure, bounded, allocation-free single-step recommendation. It neither
// schedules capture nor writes encoder/sender settings. Network safety wins
// over manual preferences during a confirmed pressure episode; infeasible manual requests stay explicit in the
// recommendation while new bitrate actions remain bounded by the budget.
// Healthy holds preserve an existing ceiling (not a forced sending rate).
ContentAwareStreamDecision RecommendContentAwareStream(
    const ContentAwareStreamInput& input,
    const ContentAwareStreamConfig& config = {}) noexcept;

// Direct 95% share of the reported outgoing capacity, bounded by the user's
// video ceiling. Media/data traffic is not subtracted again from this budget.
std::uint64_t ContentAwareVideoBudgetBps(std::uint64_t availableOutgoingBitrateBps,
    std::uint64_t maximumVideoBitrateBps) noexcept;

const char* ContentAwareStreamReasonName(ContentAwareStreamReason reason) noexcept;

// Optional session-local guard. It accumulates only fresh active samples,
// resets on generation changes and retains the spatial anchor through a
// downgrade episode. A caller must confirm successful execution separately.
struct ContentAwareStreamPolicyState {
    ScreenScene lastScene = ScreenScene::kUnknown;
    std::uint64_t generation = 0;
    std::uint64_t lastSampleTimestampMs = 0;
    std::uint64_t pendingSinceMs = 0;
    std::uint64_t lastAppliedMs = 0;
    std::uint64_t lastResolutionChangeMs = 0;
    std::uint32_t anchorWidth = 0;
    std::uint32_t anchorHeight = 0;
    std::uint32_t appliedWidth = 0;
    std::uint32_t appliedHeight = 0;
    std::uint32_t pendingWidth = 0;
    std::uint32_t pendingHeight = 0;
    std::uint32_t pendingFrameRate = 0;
    std::uint64_t pendingDesiredVideoBitrateBps = 0;
    std::uint64_t pendingSenderMaxBitrateBps = 0;
    std::uint64_t pendingSafeVideoBudgetBps = 0;
    std::uint32_t pendingSamples = 0;
    bool initialized = false;
    bool hasSample = false;
    bool pendingConfirmed = false;
    bool resolutionChangeApplied = false;
    bool pendingQualityRepairRequired = false;
    bool pendingNetworkRepairRequired = false;
    bool pendingUserSpecificationRestoreRequired = false;
    bool pendingProcessingRepairRequired = false;
};

struct ContentAwareStreamHysteresis {
    std::uint32_t downgradeSamples = 2;
    std::uint32_t upgradeSamples = 3;
    std::uint64_t upgradeMinimumMs = 2000;
    std::uint64_t downgradeMinimumIntervalMs = 1000;
    std::uint64_t resolutionResidenceMs = 3000;
};

enum class ContentAwareStreamConfirmationBlock {
    kNotApplicable, kAwaitingFreshSample, kAwaitingEvidence, kAwaitingSamples,
    kAwaitingStableTime, kAwaitingResidence, kAwaitingApplyInterval, kReady
};

const char* ContentAwareStreamConfirmationBlockName(
    ContentAwareStreamConfirmationBlock block) noexcept;

struct ContentAwareStreamEvaluation {
    ContentAwareStreamDecision recommendation;
    bool confirmed = false;
    std::uint32_t confirmObservedSamples = 0;
    std::uint32_t confirmRequiredSamples = 0;
    // Internal confirmation timers only; this does not predict network recovery.
    std::uint64_t confirmationRemainingMs = 0;
    ContentAwareStreamConfirmationBlock confirmationBlock =
        ContentAwareStreamConfirmationBlock::kNotApplicable;
};

ContentAwareStreamEvaluation EvaluateContentAwareStream(
    ContentAwareStreamPolicyState& state,
    const ContentAwareStreamInput& input,
    const ContentAwareStreamConfig& config = {},
    const ContentAwareStreamHysteresis& hysteresis = {}) noexcept;

// Called only for a confirmed candidate after effective settings are observed.
// qualityVerified determines whether an upgraded size can become the anchor.
// Recommendations alone never move the stable anchor or execution timestamps.
void ConfirmContentAwareStreamApplied(
    ContentAwareStreamPolicyState& state,
    const ContentAwareStreamInput& effective,
    bool qualityVerified) noexcept;

}  // namespace remote::media_intelligence
