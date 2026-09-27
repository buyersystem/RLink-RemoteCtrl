// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <string>

#include "src/core/DesktopCaptureTypes.h"
#include "src/webrtc/VideoDecoderRuntimeStatus.h"
#include "src/webrtc/VideoEncoderRuntimeStatus.h"

namespace remote::app {

inline std::string EncoderPreferenceName(VideoEncoderPreference preference)
{
    switch (preference) {
    case VideoEncoderPreference::kHardwareOnly:
        return "hardware";
    case VideoEncoderPreference::kFfmpegHardware:
        return "ffmpeg_hardware";
    case VideoEncoderPreference::kSoftwareOnly:
        return "software";
    case VideoEncoderPreference::kFfmpegX264Only:
        return "ffmpeg";
    case VideoEncoderPreference::kAutomatic:
    default:
        return "auto";
    }
}

inline std::string DecoderPreferenceName(VideoDecoderPreference preference)
{
    switch (preference) {
    case VideoDecoderPreference::kHardwareOnly:
        return "hardware";
    case VideoDecoderPreference::kSoftwareOnly:
        return "software";
    case VideoDecoderPreference::kAutomatic:
    default:
        return "auto";
    }
}

inline std::string DesktopCaptureImplementationName(
    DesktopCaptureImplementation implementation)
{
    switch (implementation) {
    case DesktopCaptureImplementation::kLibWebRtc:
        return "libwebrtc";
    case DesktopCaptureImplementation::kNativeDxgi:
    default:
        return "native_dxgi";
    }
}

}  // namespace remote::app
