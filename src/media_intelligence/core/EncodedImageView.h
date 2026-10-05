// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

namespace remote::media_intelligence {

enum class EncodedImageFormat : std::uint8_t {
    kJpeg,
    kPng,
    kWebp,
};

struct EncodedImageView {
    std::span<const std::uint8_t> bytes;
    EncodedImageFormat format = EncodedImageFormat::kJpeg;
    std::uint32_t width = 0;
    std::uint32_t height = 0;

    [[nodiscard]] bool IsValid() const noexcept
    {
        return !bytes.empty() && width > 0 && height > 0;
    }
};

// Owns encoded bytes through shared immutable storage. Passing a copy across
// the component boundary does not copy the image payload.
class EncodedImage final {
public:
    using Storage = std::vector<std::uint8_t>;

    EncodedImage() = default;

    EncodedImage(
        std::shared_ptr<const Storage> storage,
        EncodedImageFormat format,
        std::uint32_t width,
        std::uint32_t height) noexcept
        : storage_(std::move(storage)),
          format_(format),
          width_(width),
          height_(height)
    {
    }

    [[nodiscard]] EncodedImageView View() const noexcept
    {
        return {
            storage_ ? std::span<const std::uint8_t>(*storage_)
                     : std::span<const std::uint8_t>{},
            format_,
            width_,
            height_};
    }

    [[nodiscard]] bool IsValid() const noexcept { return View().IsValid(); }

private:
    std::shared_ptr<const Storage> storage_;
    EncodedImageFormat format_ = EncodedImageFormat::kJpeg;
    std::uint32_t width_ = 0;
    std::uint32_t height_ = 0;
};

[[nodiscard]] inline std::string_view EncodedImageMimeType(
    EncodedImageFormat format) noexcept
{
    switch (format) {
    case EncodedImageFormat::kJpeg:
        return "image/jpeg";
    case EncodedImageFormat::kPng:
        return "image/png";
    case EncodedImageFormat::kWebp:
        return "image/webp";
    }
    return {};
}

}  // namespace remote::media_intelligence
