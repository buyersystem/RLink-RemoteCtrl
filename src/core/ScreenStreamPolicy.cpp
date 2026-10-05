// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ScreenStreamPolicy.h"

#include <algorithm>
#include <cstdint>

namespace remote {

std::uint32_t NormalizeScreenVideoBitrateBppHundredths(std::uint32_t value)
{
    return value >= kMinimumScreenVideoBitrateBppHundredths &&
        value <= kMaximumScreenVideoBitrateBppHundredths
        ? value : kDefaultScreenVideoBitrateBppHundredths;
}

ScreenStreamPolicyResult ResolveScreenStreamPolicy(
    std::uint32_t sourceWidth,
    std::uint32_t sourceHeight,
    const ScreenStreamPolicyRequest& request)
{
    ScreenStreamPolicyResult result;
    result.framesPerSecond = request.framesPerSecond;
    sourceWidth = (std::max)(sourceWidth, 2u);
    sourceHeight = (std::max)(sourceHeight, 2u);
    if (request.maxWidth == 0 || request.maxHeight == 0 ||
        (sourceWidth <= request.maxWidth &&
         sourceHeight <= request.maxHeight)) {
        result.width = sourceWidth;
        result.height = sourceHeight;
    } else {
        const double scale = (std::min)(
            static_cast<double>(request.maxWidth) / sourceWidth,
            static_cast<double>(request.maxHeight) / sourceHeight);
        result.width =
            static_cast<std::uint32_t>(sourceWidth * scale);
        result.height =
            static_cast<std::uint32_t>(sourceHeight * scale);
    }
    result.width = (std::max)(2u, result.width & ~1u);
    result.height = (std::max)(2u, result.height & ~1u);

    constexpr std::uint64_t kMaximumBitrateBps = 100'000'000;
    const std::uint64_t pixels = static_cast<std::uint64_t>(result.width) * result.height;
    const auto estimate = [pixels](std::uint32_t fps, std::uint64_t coefficient,
                                  std::uint64_t minimum, std::uint64_t maximum) {
        const std::uint64_t factor = static_cast<std::uint64_t>(fps) * coefficient;
        if (factor == 0) return static_cast<std::uint32_t>(minimum);
        // Clamp before multiplying; even adversarial uint32 dimensions and
        // frame rates cannot overflow the intermediate pixel-rate product.
        if (pixels > maximum * 100 / factor) return static_cast<std::uint32_t>(maximum);
        return static_cast<std::uint32_t>(std::clamp(pixels * factor / 100, minimum, maximum));
    };
    const auto targetFps = std::clamp(result.framesPerSecond, 1u, 120u);
    const auto videoBpp = NormalizeScreenVideoBitrateBppHundredths(
        request.videoBitrateBppHundredths);
    result.maxBitrateBps = estimate(targetFps, videoBpp, 1, kMaximumBitrateBps);
    // Remove the old 4-Mbps policy floor and fixed 0.15 media multiplier.
    // 5% connection headroom is not a guaranteed DataChannel reservation.
    result.networkProbeMaxBitrateBps = static_cast<std::uint32_t>(
        static_cast<std::uint64_t>(result.maxBitrateBps) * 105 / 100);
    // The one-shot startup prior still reflects the requested media workload.
    // Ordinary FPS changes do not reapply this prior or restart GoogCC.
    result.startBitrateBps = estimate(targetFps, 8, 1, result.maxBitrateBps);
    return result;
}

}  // namespace remote
