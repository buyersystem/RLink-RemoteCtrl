// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

#include "media_intelligence/backends/openai_compatible/OpenAiCompatibleVisionProtocol.h"
#include "media_intelligence/remote/VisionApiTypes.h"

int main()
{
    using namespace remote::media_intelligence;

    VisionApiEndpointConfig endpoint = DeepSeekVisionApiPreset();
    endpoint.credentialId = "host-managed-credential";

    auto bytes = std::make_shared<const EncodedImage::Storage>(
        EncodedImage::Storage{0xFF, 0xD8, 0xFF});
    const EncodedImage image(
        bytes,
        EncodedImageFormat::kJpeg,
        1,
        1);

    OpenAiCompatibleVisionProtocol protocol;
    const VisionProtocolBuildResult request =
        protocol.BuildRequest(endpoint, image.View());
    if (!request.success) {
        std::cerr << "request_error=" << request.error << '\n';
        return 1;
    }

    const std::string sampleResponse =
        "{\"choices\":[{\"message\":{\"content\":\"{\\\"scene\\\":"
        "\\\"code_terminal\\\",\\\"confidence\\\":0.95}\"}}]}";
    const VisionProtocolParseResult result =
        protocol.ParseResponse(endpoint, sampleResponse);
    if (!result.success) {
        std::cerr << "response_error=" << result.error << '\n';
        return 1;
    }

    std::cout << "url=" << request.request.url << '\n'
              << "scene="
              << ScreenSceneName(result.classification.scene)
              << '\n'
              << "confidence=" << result.classification.confidence << '\n';
    return 0;
}
