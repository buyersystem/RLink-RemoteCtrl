// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include "media_intelligence/remote/IVisionApiProtocol.h"

namespace remote::media_intelligence {

class OpenAiCompatibleVisionProtocol final : public IVisionApiProtocol {
public:
    [[nodiscard]] VisionProtocolBuildResult BuildRequest(
        const VisionApiEndpointConfig& config,
        EncodedImageView image) const override;
    [[nodiscard]] VisionProtocolParseResult ParseResponse(
        const VisionApiEndpointConfig& config,
        std::string_view responseBody) const override;
};

}  // namespace remote::media_intelligence
