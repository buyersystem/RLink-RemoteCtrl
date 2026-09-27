// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <array>
#include <cstdint>

namespace remote::controller {

inline constexpr auto kRemoteScreenQualitySetting =
    "remoteSession/qualityTier";
inline constexpr auto kDragPointerSampleRateSetting =
    "remoteSession/dragPointerSampleRateHz";
inline constexpr std::array<std::uint32_t, 5>
    kSupportedDragPointerSampleRates = {60u, 80u, 120u, 170u, 240u};

}  // namespace remote::controller
