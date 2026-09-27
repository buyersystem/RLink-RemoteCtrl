// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <string>

#include "src/platform/win/WindowsDesktopCaptureSource.h"
#include "src/webrtc/VideoEncoderRuntimeStatus.h"

namespace remote::app {

std::string DesktopCaptureBackendName(
    WindowsDesktopCaptureSource::CaptureBackend backend);

std::string DescribeEncoderRuntimeInstance(
    const VideoEncoderInstanceRuntimeStatus& status);

}  // namespace remote::app
