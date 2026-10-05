// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>

#include "media_intelligence/core/EncodedImageView.h"

namespace remote::media_intelligence {

struct I420ImageView {
    const std::uint8_t* y = nullptr;
    int strideY = 0;
    const std::uint8_t* u = nullptr;
    int strideU = 0;
    const std::uint8_t* v = nullptr;
    int strideV = 0;
    int width = 0;
    int height = 0;

    [[nodiscard]] bool IsValid() const noexcept;
};

// Compresses planar I420 directly to a 4:2:0 JPEG. The returned storage owns
// its bytes, so no TurboJPEG lifetime crosses the component boundary.
[[nodiscard]] EncodedImage EncodeI420WithTurboJpeg(
    const I420ImageView& image,
    int quality);

}  // namespace remote::media_intelligence
