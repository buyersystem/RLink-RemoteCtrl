// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "media_intelligence/backends/turbojpeg/TurboJpegI420Encoder.h"

#include <turbojpeg.h>

#include <algorithm>
#include <cstddef>
#include <memory>

namespace remote::media_intelligence {
namespace {

struct TurboJpegHandleDeleter {
    void operator()(tjhandle handle) const noexcept { tj3Destroy(handle); }
};

struct TurboJpegBufferDeleter {
    void operator()(unsigned char* buffer) const noexcept { tj3Free(buffer); }
};

}  // namespace

bool I420ImageView::IsValid() const noexcept
{
    return y && u && v && width > 0 && height > 0 &&
        strideY >= width && strideU >= (width + 1) / 2 &&
        strideV >= (width + 1) / 2;
}

EncodedImage EncodeI420WithTurboJpeg(
    const I420ImageView& image,
    int quality)
{
    if (!image.IsValid()) {
        return {};
    }

    std::unique_ptr<void, TurboJpegHandleDeleter> compressor(
        tj3Init(TJINIT_COMPRESS));
    if (!compressor ||
        tj3Set(compressor.get(), TJPARAM_SUBSAMP, TJSAMP_420) != 0 ||
        tj3Set(
            compressor.get(), TJPARAM_QUALITY,
            std::clamp(quality, 30, 90)) != 0) {
        return {};
    }

    const unsigned char* planes[] = {image.y, image.u, image.v};
    const int strides[] = {image.strideY, image.strideU, image.strideV};
    unsigned char* rawBuffer = nullptr;
    std::size_t encodedSize = 0;
    if (tj3CompressFromYUVPlanes8(
            compressor.get(), planes, image.width, strides, image.height,
            &rawBuffer, &encodedSize) != 0 ||
        !rawBuffer || encodedSize == 0) {
        tj3Free(rawBuffer);
        return {};
    }
    std::unique_ptr<unsigned char, TurboJpegBufferDeleter> encoded(rawBuffer);
    auto storage = std::make_shared<const EncodedImage::Storage>(
        encoded.get(), encoded.get() + encodedSize);
    return EncodedImage(
        std::move(storage),
        EncodedImageFormat::kJpeg,
        static_cast<std::uint32_t>(image.width),
        static_cast<std::uint32_t>(image.height));
}

}  // namespace remote::media_intelligence
