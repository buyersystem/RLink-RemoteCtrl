// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include "media_intelligence/core/EncodedImageView.h"
#include "media_intelligence/remote/VisionApiTypes.h"

namespace remote::media_intelligence {

class IVisionApiProtocol {
public:
    virtual ~IVisionApiProtocol() = default;

    [[nodiscard]] virtual VisionProtocolBuildResult BuildRequest(
        const VisionApiEndpointConfig& config,
        EncodedImageView image) const = 0;
    [[nodiscard]] virtual VisionProtocolParseResult ParseResponse(
        const VisionApiEndpointConfig& config,
        std::string_view responseBody) const = 0;
};

}  // namespace remote::media_intelligence
