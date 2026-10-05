// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>

#include "api/video/video_frame_buffer.h"
#include "media_intelligence/core/EncodedImageView.h"
#include "media_intelligence/remote/VisionApiTypes.h"

namespace remote::app {

struct VisionFrameEncodingMetrics {
    std::uint32_t scaleConvertTimeUs = 0;
    std::uint32_t jpegEncodeTimeUs = 0;
    std::uint64_t jpegBytes = 0;
};

media_intelligence::EncodedImage EncodeWebRtcFrameAsJpeg(
    webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame,
    std::uint32_t maximumDimension =
        media_intelligence::kDefaultVisionApiMaximumImageDimension,
    int quality = 60,
    VisionFrameEncodingMetrics* metrics = nullptr);

}  // namespace remote::app
