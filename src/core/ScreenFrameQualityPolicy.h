// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace remote {

inline constexpr std::uint32_t kDefaultScreenQualityDeficitShareHundredths = 50;
inline constexpr std::uint32_t NormalizeScreenQualityDeficitShareHundredths(
    std::uint32_t value) noexcept
{ return (std::min)(value, 100u); }

struct ScreenFrameQualityDecision {
    bool protecting = false;
    bool qualityLimited = false;
    std::uint32_t networkBudgetBps = 0;
    // Diagnostic-only values supplied by the encoder integration. Neither is
    // used as the native FrameDropper's budget or the quality blend input.
    std::uint32_t encoderAdjustedBudgetBps = 0;
    std::uint32_t bandwidthAllocationBps = 0;
    // Encoder rate at its ORIGINAL nominal cadence. This is not a send rate
    // or a network ceiling: native FrameDropper still receives the GCC budget.
    std::uint32_t encoderReferenceBps = 0;
    std::uint32_t referenceBps = 0;
    std::uint32_t deficitShareHundredths = kDefaultScreenQualityDeficitShareHundredths;
};

// Caller-serialized, allocation-free encoder-side quality reference. It does
// not estimate bandwidth, drop frames, change capture, or change RTP limits.
// Native FrameDropper remains the only additional rate-budget frame dropper.
class ScreenFrameQualityPolicy final {
public:
    void Reset(std::uint32_t fps, std::uint32_t capBps,
               std::uint32_t startBps) noexcept
    {
        fps_ = (std::clamp)(fps, 1u, 120u);
        capBps_ = capBps;
        referenceBps_ = (std::min)(startBps, capBps_);
        healthyBudgetBps_ = 0;
        frameBits_ = 0;
        goodFrames_ = 0;
        protecting_ = false;
    }

    ScreenFrameQualityDecision Update(std::uint32_t budgetBps,
                                     bool enabled, bool gccPressure,
                                     std::uint32_t deficitShareHundredths =
                                         kDefaultScreenQualityDeficitShareHundredths) noexcept
    {
        if (!enabled) {
            protecting_ = false;
        } else if (budgetBps != 0) {
            if (gccPressure) protecting_ = true;
            // Full budget recovery releases protection immediately. There is
            // no FPS ladder, fixed bpp curve, or per-step recovery timer.
            // A healthy network allocation may exceed the actual per-frame
            // reference. Recover as soon as that reference fits; requiring the
            // previous (larger) allocation would unnecessarily hold protection.
            const auto recoveryBudget = healthyBudgetBps_ != 0 && referenceBps_ != 0
                ? (std::min)(healthyBudgetBps_, referenceBps_)
                : (std::max)(healthyBudgetBps_, referenceBps_);
            if (!gccPressure && protecting_ && recoveryBudget != 0 &&
                budgetBps >= recoveryBudget * 0.98) protecting_ = false;
            if (!protecting_) healthyBudgetBps_ = budgetBps;
        }
        ScreenFrameQualityDecision result;
        result.networkBudgetBps = budgetBps;
        result.referenceBps = referenceBps_;
        result.deficitShareHundredths = NormalizeScreenQualityDeficitShareHundredths(deficitShareHundredths);
        result.encoderReferenceBps = budgetBps;
        if (!enabled || !protecting_ || budgetBps == 0 || capBps_ == 0) return result;
        const auto reference = (std::min)(referenceBps_, capBps_);
        // C = A - (A - B) * k, only while A > B and GCC proves pressure.
        // A is the unblended reference, so repeated updates cannot compound
        // the discount. Native FrameDropper/pacer still receive B unchanged.
        if (reference > budgetBps) {
            const auto reduction = (static_cast<std::uint64_t>(reference - budgetBps) *
                result.deficitShareHundredths + 50) / 100;
            result.encoderReferenceBps = reference - static_cast<std::uint32_t>(reduction);
        }
        result.protecting = result.encoderReferenceBps > budgetBps;
        result.qualityLimited = result.encoderReferenceBps < reference;
        return result;
    }

    void Retarget(std::uint32_t fps, std::uint32_t capBps) noexcept
    {
        fps = (std::clamp)(fps, 1u, 120u);
        referenceBps_ = static_cast<std::uint32_t>((std::min<std::uint64_t>)(capBps,
            static_cast<std::uint64_t>(referenceBps_) * fps / fps_));
        healthyBudgetBps_ = static_cast<std::uint32_t>((std::min<std::uint64_t>)(capBps,
            static_cast<std::uint64_t>(healthyBudgetBps_) * fps / fps_));
        fps_ = fps;
        capBps_ = capBps;
    }

    void ObserveEncoded(std::uint64_t bytes, int qp, bool keyFrame) noexcept
    {
        if (keyFrame || bytes == 0 || qp < 0 || qp > 51) return;
        if (protecting_) {
            // The blend intentionally spends a share of the deficit on image
            // quality. Its higher QP cannot be treated as evidence that A must
            // grow: that creates positive feedback and delays FPS recovery.
            // Learn a changed scene again from healthy, unconstrained frames.
            return;
        }
        if (qp > 35) return; // Never learn a blurry frame as the desired quality.
        const double bits = static_cast<double>(bytes) * 8;
        frameBits_ = goodFrames_ == 0 ? bits : frameBits_ * 0.9 + bits * 0.1;
        if (goodFrames_ < 8) ++goodFrames_;
        if (goodFrames_ >= 8) {
            const double rate = (std::min)(static_cast<double>(capBps_),
                                         std::ceil(frameBits_ * fps_));
            referenceBps_ = static_cast<std::uint32_t>((std::max)(1.0, rate));
        }
    }

private:
    std::uint32_t fps_ = 1, capBps_ = 0, referenceBps_ = 0;
    std::uint32_t healthyBudgetBps_ = 0, goodFrames_ = 0;
    double frameBits_ = 0;
    bool protecting_ = false;
};
} // namespace remote
