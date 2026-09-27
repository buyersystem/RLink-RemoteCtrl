// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>

namespace remote {

enum class DesktopCaptureImplementation : std::uint8_t {
    kLibWebRtc,
    kNativeDxgi,
};

}  // namespace remote
