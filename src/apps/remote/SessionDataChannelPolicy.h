// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>

namespace remote::app {

inline constexpr std::uint64_t kMaximumFileTransferBufferedBytes =
    8 * 1024 * 1024;

// Clipboard paste shares the PeerConnection with the live desktop video.
// Keep its cancellable in-flight tail small: libwebrtc cannot retract bytes
// already accepted by an ordered reliable SCTP stream.
inline constexpr std::uint64_t kMaximumClipboardBufferedBytes =
    512 * 1024;

inline bool IsClipboardWarmupPayload(
    std::span<const std::uint8_t> payload)
{
    constexpr std::array<std::uint8_t, 4> kWarmupMagic = {
        'R', 'C', 'W', '1'};
    return payload.size() >= kWarmupMagic.size() &&
        std::equal(kWarmupMagic.begin(), kWarmupMagic.end(), payload.begin());
}

}  // namespace remote::app
