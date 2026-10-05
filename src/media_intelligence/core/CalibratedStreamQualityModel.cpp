// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "media_intelligence/core/CalibratedStreamQualityModel.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace remote::media_intelligence {
namespace {

bool ValidText(std::string_view text, std::size_t maximum) noexcept
{
    return !text.empty() && text.size() <= maximum &&
        text.find('\0') == std::string_view::npos;
}

bool ValidScene(ScreenScene scene) noexcept
{
    return scene > ScreenScene::kUnknown && scene <= ScreenScene::kMixed;
}

bool ValidDimensions(std::uint32_t width, std::uint32_t height) noexcept
{
    return width >= 2 && height >= 2 &&
        width <= CalibratedStreamQualityModel::kMaximumDimension &&
        height <= CalibratedStreamQualityModel::kMaximumDimension &&
        width % 2 == 0 && height % 2 == 0;
}

bool SameAspect(std::uint32_t width, std::uint32_t height,
    std::uint32_t otherWidth, std::uint32_t otherHeight) noexcept
{
    // Candidates scale both axes together, then round each down to an even
    // pixel. The scale intervals must overlap; a fixed ratio tolerance would
    // incorrectly admit different aspect ratios at small resolutions.
    const double lower = std::max(static_cast<double>(width) / otherWidth,
        static_cast<double>(height) / otherHeight);
    const double upper = std::min(static_cast<double>(width + 2) / otherWidth,
        static_cast<double>(height + 2) / otherHeight);
    return lower < upper && lower <= 1.0;
}

}  // namespace

StreamCalibrationError CalibratedStreamQualityModel::Build(
    std::span<const StreamQualityCalibrationSample> samples,
    StreamEncoderProfile profile, std::string_view qualityEvidenceId) noexcept
{
    *this = {};
    if (!ValidText(profile.codec, kMaximumProfileText) ||
        !ValidText(profile.encoder, kMaximumProfileText) ||
        !ValidText(profile.profile, kMaximumProfileText)) {
        return StreamCalibrationError::kInvalidProfile;
    }
    if (!ValidText(qualityEvidenceId, kMaximumEvidenceText)) {
        return StreamCalibrationError::kMissingEvidence;
    }
    if (samples.empty() || samples.size() > kMaximumSamples) {
        return StreamCalibrationError::kInvalidSampleCount;
    }
    for (std::size_t index = 0; index < samples.size(); ++index) {
        const auto& sample = samples[index];
        if (!ValidScene(sample.scene)) return StreamCalibrationError::kInvalidScene;
        if (!ValidDimensions(sample.width, sample.height)) {
            return StreamCalibrationError::kInvalidDimensions;
        }
        if (sample.frameRate == 0 || sample.frameRate > 120) {
            return StreamCalibrationError::kInvalidFrameRate;
        }
        if (sample.measuredRequiredBitrateBps == 0 ||
            sample.measuredRequiredBitrateBps > kMaximumBitrateBps) {
            return StreamCalibrationError::kInvalidBitrate;
        }
        if (sample.measuredFrameCount < kMinimumMeasuredFrames) {
            return StreamCalibrationError::kInsufficientMeasurement;
        }
        if (!sample.measuredVisualQualityPassed) {
            return StreamCalibrationError::kVisualQualityNotPassed;
        }
        if (sample.maximumVerifiedAverageQp > 51) {
            return StreamCalibrationError::kInvalidQualityBoundary;
        }
        for (std::size_t previous = 0; previous < index; ++previous) {
            const auto& other = samples[previous];
            if (other.scene == sample.scene && other.width == sample.width &&
                other.height == sample.height && other.frameRate == sample.frameRate) {
                return StreamCalibrationError::kDuplicateSpecification;
            }
        }
    }
    std::copy(profile.codec.begin(), profile.codec.end(), codec_.begin());
    std::copy(profile.encoder.begin(), profile.encoder.end(), encoder_.begin());
    std::copy(profile.profile.begin(), profile.profile.end(), profile_.begin());
    std::copy(qualityEvidenceId.begin(), qualityEvidenceId.end(), evidence_.begin());
    std::copy(samples.begin(), samples.end(), samples_.begin());
    sampleCount_ = samples.size();
    for (std::size_t index = 0; index < sampleCount_; ++index) {
        const auto& covering = samples_[index];
        bitrateEnvelopes_[index] = covering.measuredRequiredBitrateBps;
        qpBoundaries_[index] = covering.maximumVerifiedAverageQp;
        for (std::size_t child = 0; child < sampleCount_; ++child) {
            const auto& sample = samples_[child];
            if (sample.scene != covering.scene || sample.width > covering.width ||
                sample.height > covering.height || sample.frameRate > covering.frameRate ||
                !SameAspect(sample.width, sample.height, covering.width, covering.height)) continue;
            bitrateEnvelopes_[index] = std::max(bitrateEnvelopes_[index], sample.measuredRequiredBitrateBps);
            if (qpBoundaries_[index] != 0 && sample.maximumVerifiedAverageQp != 0) {
                qpBoundaries_[index] = std::min(qpBoundaries_[index], sample.maximumVerifiedAverageQp);
            }
        }
    }
    return StreamCalibrationError::kNone;
}

StreamEncoderProfile CalibratedStreamQualityModel::Profile() const noexcept
{
    return {codec_.data(), encoder_.data(), profile_.data()};
}

std::string_view CalibratedStreamQualityModel::QualityEvidenceId() const noexcept
{
    return evidence_.data();
}

bool CalibratedStreamQualityModel::MatchesProfile(StreamEncoderProfile active) const noexcept
{
    const auto expected = Profile();
    return IsValid() && expected.codec == active.codec &&
        expected.encoder == active.encoder && expected.profile == active.profile;
}

StreamQualityEstimate CalibratedStreamQualityModel::Estimate(
    const StreamQualityRequest& request, StreamEncoderProfile active) const noexcept
{
    return MatchesProfile(active) ? EstimateMatched(request) : StreamQualityEstimate{};
}

StreamQualityEstimate CalibratedStreamQualityModel::EstimateMatched(
    const StreamQualityRequest& request) const noexcept
{
    if (!ValidScene(request.scene) || !ValidDimensions(request.width, request.height) ||
        request.frameRate == 0 || request.frameRate > 120) return {};
    std::uint64_t requirement = std::numeric_limits<std::uint64_t>::max();
    for (std::size_t index = 0; index < sampleCount_; ++index) {
        const auto& covering = samples_[index];
        if (covering.scene != request.scene || covering.width < request.width ||
            covering.height < request.height || covering.frameRate < request.frameRate ||
            !SameAspect(request.width, request.height, covering.width, covering.height)) continue;
        requirement = std::min(requirement, bitrateEnvelopes_[index]);
    }
    if (requirement == std::numeric_limits<std::uint64_t>::max()) return {};
    // Processing feasibility is independently gated by the host's measured
    // encoder/receiver feedback; this model imposes no extra processing veto.
    return {requirement, true, true, true};
}

StreamCurrentQualityVerification CalibratedStreamQualityModel::VerifyCurrentQuality(
    const StreamQualityRequest& request, StreamEncoderProfile active,
    double windowAverageQp) const noexcept
{
    return MatchesProfile(active) ? VerifyQualityMatched(request, windowAverageQp) :
        StreamCurrentQualityVerification{};
}

StreamCurrentQualityVerification CalibratedStreamQualityModel::VerifyQualityMatched(
    const StreamQualityRequest& request, double windowAverageQp) const noexcept
{
    if (!std::isfinite(windowAverageQp) || windowAverageQp < 0 || windowAverageQp > 51 ||
        !ValidScene(request.scene) || !ValidDimensions(request.width, request.height) ||
        request.frameRate == 0 || request.frameRate > 120) return {};
    std::uint32_t boundary = 52;
    for (std::size_t index = 0; index < sampleCount_; ++index) {
        const auto& covering = samples_[index];
        if (covering.scene != request.scene || covering.width < request.width ||
            covering.height < request.height || covering.frameRate < request.frameRate ||
            covering.maximumVerifiedAverageQp == 0 ||
            !SameAspect(request.width, request.height, covering.width, covering.height)) continue;
        boundary = std::min(boundary, qpBoundaries_[index]);
    }
    if (boundary == 52) return {};
    return {true, windowAverageQp <= boundary};
}

CalibratedStreamQualityContext::CalibratedStreamQualityContext(
    const CalibratedStreamQualityModel& model, StreamEncoderProfile active) noexcept
    : model_(model), profileMatches_(model.MatchesProfile(active))
{
}

StreamQualityEstimate CalibratedStreamQualityContext::Estimate(
    const StreamQualityRequest& request, const void* context) noexcept
{
    if (!context) return {};
    const auto& bound = *static_cast<const CalibratedStreamQualityContext*>(context);
    return bound.profileMatches_ ? bound.model_.EstimateMatched(request) : StreamQualityEstimate{};
}

StreamCurrentQualityVerification CalibratedStreamQualityContext::VerifyCurrentQuality(
    const StreamQualityRequest& request, double windowAverageQp) const noexcept
{
    return profileMatches_ ? model_.VerifyQualityMatched(request, windowAverageQp) :
        StreamCurrentQualityVerification{};
}

const char* StreamCalibrationErrorName(StreamCalibrationError error) noexcept
{
    switch (error) {
    case StreamCalibrationError::kNone: return "none";
    case StreamCalibrationError::kInvalidProfile: return "invalid_profile";
    case StreamCalibrationError::kMissingEvidence: return "missing_evidence";
    case StreamCalibrationError::kInvalidSampleCount: return "invalid_sample_count";
    case StreamCalibrationError::kInvalidScene: return "invalid_scene";
    case StreamCalibrationError::kInvalidDimensions: return "invalid_dimensions";
    case StreamCalibrationError::kInvalidFrameRate: return "invalid_frame_rate";
    case StreamCalibrationError::kInvalidBitrate: return "invalid_bitrate";
    case StreamCalibrationError::kInsufficientMeasurement: return "insufficient_measurement";
    case StreamCalibrationError::kVisualQualityNotPassed: return "visual_quality_not_passed";
    case StreamCalibrationError::kInvalidQualityBoundary: return "invalid_quality_boundary";
    case StreamCalibrationError::kDuplicateSpecification: return "duplicate_specification";
    }
    return "unknown";
}

}  // namespace remote::media_intelligence
