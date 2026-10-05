// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QObject>

#include <functional>
#include <memory>
#include <string>

#include "media_intelligence/core/IRemoteSemanticClassifier.h"
#include "media_intelligence/remote/VisionApiTypes.h"

namespace remote::app {

class VisionApiSettingsController final : public QObject {
public:
    using TestCompletion =
        std::function<void(media_intelligence::RemoteSemanticResult)>;

    explicit VisionApiSettingsController(QObject* parent = nullptr);
    ~VisionApiSettingsController() override;

    bool SaveCredential(
        std::string_view credentialId,
        std::string_view secret,
        std::string* error = nullptr);
    bool DeleteCredential(
        std::string_view credentialId,
        std::string* error = nullptr);
    [[nodiscard]] bool HasCredential(std::string_view credentialId) const;

    bool TestConnection(
        media_intelligence::VisionApiEndpointConfig endpoint,
        int maximumImageDimension,
        int jpegQuality,
        TestCompletion completion,
        std::string* error = nullptr);
    bool TestConnectionWithImage(
        media_intelligence::VisionApiEndpointConfig endpoint,
        media_intelligence::EncodedImage testImage,
        TestCompletion completion,
        std::string* error = nullptr);
    void CancelTest();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace remote::app
