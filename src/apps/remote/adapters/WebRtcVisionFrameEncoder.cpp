// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "src/apps/remote/adapters/WebRtcVisionFrameEncoder.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <limits>
#include <utility>

#include "media_intelligence/backends/turbojpeg/TurboJpegI420Encoder.h"

namespace remote::app {
namespace {

std::uint32_t ElapsedMicroseconds(
    std::chrono::steady_clock::time_point startedAt) noexcept
{
    return static_cast<std::uint32_t>((std::min<std::int64_t>)(
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - startedAt).count(),
        std::numeric_limits<std::uint32_t>::max()));
}

}  // namespace

media_intelligence::EncodedImage EncodeWebRtcFrameAsJpeg(
    webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame,
    std::uint32_t maximumDimension,
    int quality,
    VisionFrameEncodingMetrics* metrics)
{
    if (metrics) {
        *metrics = {};
    }
    if (!frame || frame->width() <= 0 || frame->height() <= 0 ||
        maximumDimension < 64) {
        return {};
    }

    const auto scaleConvertStartedAt = std::chrono::steady_clock::now();
    const int sourceWidth = frame->width();
    const int sourceHeight = frame->height();
    const int sourceMaximum = (std::max)(sourceWidth, sourceHeight);
    const double scale = sourceMaximum > static_cast<int>(maximumDimension)
        ? static_cast<double>(maximumDimension) /
              static_cast<double>(sourceMaximum)
        : 1.0;
    const int outputWidth = (std::max)(
        2, static_cast<int>(sourceWidth * scale) & ~1);
    const int outputHeight = (std::max)(
        2, static_cast<int>(sourceHeight * scale) & ~1);

    auto sampledFrame = frame;
    if (outputWidth != sourceWidth || outputHeight != sourceHeight) {
        sampledFrame = frame->CropAndScale(
            0, 0, sourceWidth, sourceHeight,
            outputWidth, outputHeight);
    }
    if (!sampledFrame) {
        return {};
    }
    const auto i420 = sampledFrame->ToI420();
    if (!i420 || i420->width() != outputWidth ||
        i420->height() != outputHeight) {
        return {};
    }
    if (metrics) {
        metrics->scaleConvertTimeUs =
            ElapsedMicroseconds(scaleConvertStartedAt);
    }

    const auto jpegStartedAt = std::chrono::steady_clock::now();
    auto encoded = media_intelligence::EncodeI420WithTurboJpeg(
        {
            i420->DataY(), i420->StrideY(),
            i420->DataU(), i420->StrideU(),
            i420->DataV(), i420->StrideV(),
            outputWidth, outputHeight,
        },
        quality);
    if (metrics) {
        metrics->jpegEncodeTimeUs = ElapsedMicroseconds(jpegStartedAt);
        metrics->jpegBytes = encoded.View().bytes.size();
    }
    return encoded;
}

}  // namespace remote::app
