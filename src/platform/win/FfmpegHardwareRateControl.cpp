// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "FfmpegHardwareRateControl.h"

#include <algorithm>
#include <cmath>

namespace remote {
namespace {

bool HasElapsed(std::int64_t nowMs, std::int64_t startMs, std::int64_t durationMs) noexcept
{
    // Unsigned subtraction after the ordered comparison handles the full
    // signed timestamp domain without signed-overflow undefined behaviour.
    return nowMs >= startMs && static_cast<std::uint64_t>(nowMs) -
        static_cast<std::uint64_t>(startMs) >= static_cast<std::uint64_t>(durationMs);
}

} // namespace

FfmpegHardwareRateControl::FfmpegHardwareRateControl(FfmpegHardwareRateControlConfig config) noexcept
    : config_(config)
{
    config_.maximumFrameRate = (std::clamp)(config_.maximumFrameRate, 1u, 120u);
    if (!std::isfinite(config_.minimumMaterialFrameRateDelta)) {
        config_.minimumMaterialFrameRateDelta = 5.0;
    }
    config_.minimumMaterialFrameRateDelta = (std::clamp)(config_.minimumMaterialFrameRateDelta, 1.0, 120.0);
    if (!std::isfinite(config_.materialFrameRateRatio)) {
        config_.materialFrameRateRatio = 0.25;
    }
    config_.materialFrameRateRatio = (std::clamp)(config_.materialFrameRateRatio, 0.0, 1.0);
    if (!std::isfinite(config_.minimumActiveFrameRateForNominalChange)) {
        config_.minimumActiveFrameRateForNominalChange = 10.0;
    }
    config_.minimumActiveFrameRateForNominalChange = (std::clamp)(
        config_.minimumActiveFrameRateForNominalChange, 1.0, 120.0);
    config_.downStableMs = (std::max)(config_.downStableMs, std::int64_t{0});
    config_.minimumDownChangeIntervalMs = (std::max)(config_.minimumDownChangeIntervalMs, std::int64_t{0});
}

void FfmpegHardwareRateControl::Reset(std::uint32_t nominalFrameRate,
    std::uint32_t requestedBitrateBps, std::int64_t nowMs) noexcept
{
    nominalFrameRate_ = (std::clamp)(nominalFrameRate, 1u, config_.maximumFrameRate);
    activeFrameRate_ = nominalFrameRate_;
    requestedBitrateBps_ = (std::min)(requestedBitrateBps, config_.maximumRequestedBitrateBps);
    pendingDownFrameRate_ = 0;
    pendingDownSinceMs_ = nowMs;
    lastNominalChangeMs_ = nowMs;
}

double FfmpegHardwareRateControl::MaterialDelta(double frameRate) const noexcept
{
    return (std::max)(config_.minimumMaterialFrameRateDelta, frameRate * config_.materialFrameRateRatio);
}

void FfmpegHardwareRateControl::Observe(std::uint32_t requestedBitrateBps,
    double activeFrameRate, std::int64_t nowMs) noexcept
{
    requestedBitrateBps_ = (std::min)(requestedBitrateBps, config_.maximumRequestedBitrateBps);
    if (std::isfinite(activeFrameRate) && activeFrameRate > 0.0) {
        activeFrameRate_ = (std::clamp)(activeFrameRate, 1.0, static_cast<double>(config_.maximumFrameRate));
    }
    if (requestedBitrateBps_ == 0 || activeFrameRate_ < config_.minimumActiveFrameRateForNominalChange ||
        static_cast<double>(nominalFrameRate_) - activeFrameRate_ < MaterialDelta(nominalFrameRate_)) {
        pendingDownFrameRate_ = 0;
        return;
    }
    const auto candidate = static_cast<std::uint32_t>(std::lround(activeFrameRate_));
    if (pendingDownFrameRate_ == 0 || nowMs < pendingDownSinceMs_ ||
        std::abs(static_cast<double>(candidate) - pendingDownFrameRate_) >= MaterialDelta(pendingDownFrameRate_)) {
        pendingDownFrameRate_ = candidate;
        pendingDownSinceMs_ = nowMs;
    }
}

std::uint32_t FfmpegHardwareRateControl::ClampSubmittedBitrate(
    std::uint32_t requestedBitrateBps, std::uint32_t maximumSubmittedBitrateBps) noexcept
{
    return (std::min)(requestedBitrateBps, maximumSubmittedBitrateBps);
}

FfmpegHardwareRateControlDecision FfmpegHardwareRateControl::Recommend(std::int64_t nowMs) const noexcept
{
    FfmpegHardwareRateControlDecision result;
    result.requestedBitrateBps = requestedBitrateBps_;
    result.activeFrameRate = activeFrameRate_;
    result.nominalFrameRate = result.proposedNominalFrameRate = nominalFrameRate_;
    result.submittedBitrateBps = ClampSubmittedBitrate(requestedBitrateBps_, config_.maximumSubmittedBitrateBps);
    if (requestedBitrateBps_ == 0 || activeFrameRate_ < config_.minimumActiveFrameRateForNominalChange) {
        return result;
    }
    if (activeFrameRate_ - nominalFrameRate_ >= MaterialDelta(nominalFrameRate_)) {
        result.proposedNominalFrameRate = static_cast<std::uint32_t>(std::lround(activeFrameRate_));
    } else if (pendingDownFrameRate_ != 0 &&
        HasElapsed(nowMs, pendingDownSinceMs_, config_.downStableMs) &&
        HasElapsed(nowMs, lastNominalChangeMs_, config_.minimumDownChangeIntervalMs)) {
        result.proposedNominalFrameRate = static_cast<std::uint32_t>(std::lround(activeFrameRate_));
    }
    result.nominalFrameRateChangeNeeded = result.proposedNominalFrameRate != nominalFrameRate_;
    return result;
}

void FfmpegHardwareRateControl::CommitNominalFrameRate(std::uint32_t nominalFrameRate,
    std::int64_t nowMs) noexcept
{
    nominalFrameRate_ = (std::clamp)(nominalFrameRate, 1u, config_.maximumFrameRate);
    lastNominalChangeMs_ = nowMs;
    pendingDownFrameRate_ = 0;
    pendingDownSinceMs_ = nowMs;
}

} // namespace remote
