// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

#include "media_intelligence/core/ContentAwareStreamPolicy.h"

namespace remote::media_intelligence {

struct StreamEncoderProfile {
    std::string_view codec;
    std::string_view encoder;
    std::string_view profile;
};

// The importer is responsible for measuring visual quality against its stated
// offline criterion. QP, a seed formula and current sent bitrate are not such
// evidence. This table does not certify the current session's visual quality.
struct StreamQualityCalibrationSample {
    ScreenScene scene = ScreenScene::kUnknown;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t frameRate = 0;
    std::uint64_t measuredRequiredBitrateBps = 0;
    std::uint32_t measuredFrameCount = 0;
    bool measuredVisualQualityPassed = false;
    // Optional boundary from the SAME offline visual-quality measurement.
    // Zero supplies no QP evidence. This calibrated proxy does not constitute
    // a per-frame perceptual guarantee or a universal codec QP threshold.
    std::uint32_t maximumVerifiedAverageQp = 0;
};

struct StreamCurrentQualityVerification {
    bool available = false;
    bool acceptable = false;
};

enum class StreamCalibrationError : std::uint8_t {
    kNone,
    kInvalidProfile,
    kMissingEvidence,
    kInvalidSampleCount,
    kInvalidScene,
    kInvalidDimensions,
    kInvalidFrameRate,
    kInvalidBitrate,
    kInsufficientMeasurement,
    kVisualQualityNotPassed,
    kInvalidQualityBoundary,
    kDuplicateSpecification,
};

class CalibratedStreamQualityModel {
public:
    static constexpr std::size_t kMaximumSamples = 128;
    static constexpr std::size_t kMaximumProfileText = 63;
    static constexpr std::size_t kMaximumEvidenceText = 127;
    static constexpr std::uint32_t kMinimumMeasuredFrames = 30;
    static constexpr std::uint32_t kMaximumDimension = 16384;
    static constexpr std::uint64_t kMaximumBitrateBps = 100'000'000;

    // Replaces the whole model, including on failure. Callers should publish a
    // new immutable instance; Build must not race concurrent estimates.
    StreamCalibrationError Build(
        std::span<const StreamQualityCalibrationSample> samples,
        StreamEncoderProfile profile, std::string_view qualityEvidenceId) noexcept;

    bool IsValid() const noexcept { return sampleCount_ != 0; }
    std::size_t SampleCount() const noexcept { return sampleCount_; }
    StreamEncoderProfile Profile() const noexcept;
    std::string_view QualityEvidenceId() const noexcept;
    bool MatchesProfile(StreamEncoderProfile activeProfile) const noexcept;

    // No extrapolation beyond measured sizes/FPS, no interpolation, and no
    // cross-aspect-ratio inference (only common-scale even-pixel rounding is
    // allowed). A covering measured specification bounds
    // the request. Its envelope includes every dominated sample of that scene
    // and aspect ratio, so a non-monotonic higher requirement is never hidden.
    StreamQualityEstimate Estimate(const StreamQualityRequest& request,
        StreamEncoderProfile activeProfile) const noexcept;
    StreamCurrentQualityVerification VerifyCurrentQuality(
        const StreamQualityRequest& request, StreamEncoderProfile activeProfile,
        double windowAverageQp) const noexcept;

private:
    StreamQualityEstimate EstimateMatched(const StreamQualityRequest& request) const noexcept;
    StreamCurrentQualityVerification VerifyQualityMatched(
        const StreamQualityRequest& request, double windowAverageQp) const noexcept;
    std::array<StreamQualityCalibrationSample, kMaximumSamples> samples_{};
    // Built once on import; each hot-path query visits at most 128 entries.
    std::array<std::uint64_t, kMaximumSamples> bitrateEnvelopes_{};
    std::array<std::uint32_t, kMaximumSamples> qpBoundaries_{};
    std::size_t sampleCount_ = 0;
    std::array<char, kMaximumProfileText + 1> codec_{};
    std::array<char, kMaximumProfileText + 1> encoder_{};
    std::array<char, kMaximumProfileText + 1> profile_{};
    std::array<char, kMaximumEvidenceText + 1> evidence_{};

    friend class CalibratedStreamQualityContext;
};

// Owns a fixed-size copy of the model. The host can retain this context with
// shared ownership when publishing the estimator, without any hot-path
// allocation or a dangling pointer to an imported table. Recreate it whenever
// the encoder, codec or encoder profile changes.
class CalibratedStreamQualityContext {
public:
    CalibratedStreamQualityContext() noexcept = default;
    CalibratedStreamQualityContext(const CalibratedStreamQualityModel& model,
        StreamEncoderProfile activeProfile) noexcept;

    bool MatchesActiveProfile() const noexcept { return profileMatches_; }
    static StreamQualityEstimate Estimate(const StreamQualityRequest& request,
        const void* context) noexcept;
    StreamCurrentQualityVerification VerifyCurrentQuality(
        const StreamQualityRequest& request, double windowAverageQp) const noexcept;

private:
    CalibratedStreamQualityModel model_;
    bool profileMatches_ = false;
};

const char* StreamCalibrationErrorName(StreamCalibrationError error) noexcept;

}  // namespace remote::media_intelligence
