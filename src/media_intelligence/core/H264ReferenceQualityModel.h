// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <array>
#include <string_view>

#include "media_intelligence/core/CalibratedStreamQualityModel.h"

namespace remote::media_intelligence {

// Product starting values, NOT measurements or an external encoder benchmark.
// All supported H264 implementations share one reference curve. Applications
// may tune these values, or replace this reference with measured calibration.
struct H264ReferenceQualityConfig {
    std::array<double, 10> sceneBitsPerPixelPerFrame{};
    std::array<double, 10> sceneMaximumAverageQp{};
    double bitrateSafetyMargin = 1.15;
    // Global additional ceiling; per-scene thresholds may be more strict.
    double maximumAverageQp = 51.0;

    H264ReferenceQualityConfig() noexcept;
};

// Fixed-size, allocation-free reference context. The profile is retained for
// session provenance, not used to pretend to predict a particular GPU's visual
// quality. Recreate the context when the active codec/encoder/profile changes.
// QP verifies only a configurable heuristic; it is not a visual certification.
class H264ReferenceQualityContext {
public:
    static constexpr std::size_t kMaximumProfileText = 63;
    static constexpr std::uint32_t kMaximumDimension = 16384;

    H264ReferenceQualityContext() noexcept = default;
    explicit H264ReferenceQualityContext(StreamEncoderProfile activeProfile,
        const H264ReferenceQualityConfig& config = {}) noexcept;

    bool IsValid() const noexcept { return valid_; }
    bool MatchesActiveProfile() const noexcept { return valid_; }
    bool MatchesProfile(StreamEncoderProfile activeProfile) const noexcept;
    StreamEncoderProfile Profile() const noexcept;
    static StreamQualityEstimate Estimate(const StreamQualityRequest& request,
        const void* context) noexcept;
    StreamCurrentQualityVerification VerifyCurrentQuality(
        const StreamQualityRequest& request, double windowAverageQp) const noexcept;

private:
    bool ValidRequest(const StreamQualityRequest& request) const noexcept;
    H264ReferenceQualityConfig config_;
    std::array<char, kMaximumProfileText + 1> codec_{};
    std::array<char, kMaximumProfileText + 1> encoder_{};
    std::array<char, kMaximumProfileText + 1> profile_{};
    bool valid_ = false;
};

}  // namespace remote::media_intelligence
