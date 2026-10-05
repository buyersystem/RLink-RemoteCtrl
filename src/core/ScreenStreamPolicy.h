// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>

namespace remote {

inline constexpr std::uint32_t kDefaultScreenVideoBitrateBppHundredths = 15;
inline constexpr std::uint32_t kMinimumScreenVideoBitrateBppHundredths = 3;
inline constexpr std::uint32_t kMaximumScreenVideoBitrateBppHundredths = 50;

// Invalid persisted or external values fall back to the existing default.
std::uint32_t NormalizeScreenVideoBitrateBppHundredths(std::uint32_t value);

struct ScreenStreamPolicyRequest {
    std::uint32_t maxWidth = 0;
    std::uint32_t maxHeight = 0;
    std::uint32_t framesPerSecond = 60;
    // Video ceiling coefficient. Connection headroom is derived internally
    // as 105% of this video ceiling; neither is a forced sending rate.
    // This is not a measured quality requirement or an encoder QP setting.
    std::uint32_t videoBitrateBppHundredths =
        kDefaultScreenVideoBitrateBppHundredths;
};

struct ScreenStreamPolicyResult {
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t framesPerSecond = 60;
    std::uint32_t startBitrateBps = 0;
    std::uint32_t maxBitrateBps = 0;
    std::uint32_t networkProbeMaxBitrateBps = 0;
};

// Resolves the encoded desktop dimensions and bitrate without depending on
// capture, signaling, Qt, or WebRTC state.
ScreenStreamPolicyResult ResolveScreenStreamPolicy(
    std::uint32_t sourceWidth,
    std::uint32_t sourceHeight,
    const ScreenStreamPolicyRequest& request);

}  // namespace remote
