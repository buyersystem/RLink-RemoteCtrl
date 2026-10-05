// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <limits>

namespace remote {

struct FfmpegHardwareRateControlConfig {
    std::uint32_t maximumRequestedBitrateBps = std::numeric_limits<std::uint32_t>::max();
    std::uint32_t maximumSubmittedBitrateBps = std::numeric_limits<std::uint32_t>::max();
    std::uint32_t maximumFrameRate = 120;
    double minimumMaterialFrameRateDelta = 5.0;
    double materialFrameRateRatio = 0.25;
    double minimumActiveFrameRateForNominalChange = 10.0;
    std::int64_t downStableMs = 2000;
    std::int64_t minimumDownChangeIntervalMs = 5000;
};

struct FfmpegHardwareRateControlDecision {
    std::uint32_t requestedBitrateBps = 0;
    // Never amplifies the requested rate using a possibly stale FPS estimate.
    std::uint32_t submittedBitrateBps = 0;
    double activeFrameRate = 1.0;
    std::uint32_t nominalFrameRate = 1;
    std::uint32_t proposedNominalFrameRate = 1;
    bool nominalFrameRateChangeNeeded = false;
};

// Serial caller-owned state. No Qt, FFmpeg, hardware access or allocation.
// A recommendation never commits a backend change: the caller commits only
// after its selected update/reopen mechanism has succeeded.
class FfmpegHardwareRateControl final {
public:
    explicit FfmpegHardwareRateControl(FfmpegHardwareRateControlConfig config = {}) noexcept;

    void Reset(std::uint32_t nominalFrameRate, std::uint32_t requestedBitrateBps,
        std::int64_t nowMs) noexcept;
    void Observe(std::uint32_t requestedBitrateBps, double activeFrameRate,
        std::int64_t nowMs) noexcept;
    [[nodiscard]] FfmpegHardwareRateControlDecision Recommend(std::int64_t nowMs) const noexcept;
    void CommitNominalFrameRate(std::uint32_t nominalFrameRate, std::int64_t nowMs) noexcept;

    [[nodiscard]] static std::uint32_t ClampSubmittedBitrate(
        std::uint32_t requestedBitrateBps, std::uint32_t maximumSubmittedBitrateBps) noexcept;

private:
    [[nodiscard]] double MaterialDelta(double frameRate) const noexcept;
    FfmpegHardwareRateControlConfig config_;
    std::uint32_t requestedBitrateBps_ = 0;
    double activeFrameRate_ = 1.0;
    std::uint32_t nominalFrameRate_ = 1;
    std::uint32_t pendingDownFrameRate_ = 0;
    std::int64_t pendingDownSinceMs_ = 0;
    std::int64_t lastNominalChangeMs_ = 0;
};

} // namespace remote
