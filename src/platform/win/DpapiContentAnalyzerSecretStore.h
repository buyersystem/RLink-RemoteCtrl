// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QString>

#include <string>
#include <string_view>

#include "media_intelligence/remote/ISecretProvider.h"

namespace remote {

class DpapiContentAnalyzerSecretStore final
    : public media_intelligence::ISecretProvider {
public:
    explicit DpapiContentAnalyzerSecretStore(QString directory = {});

    bool LoadSecret(
        std::string_view credentialId,
        media_intelligence::SecretValue* secret,
        std::string* error) override;

    bool SaveSecret(
        std::string_view credentialId,
        std::string_view secret,
        std::string* error = nullptr);
    bool DeleteSecret(
        std::string_view credentialId,
        std::string* error = nullptr);
    [[nodiscard]] bool HasSecret(std::string_view credentialId) const;

    [[nodiscard]] const QString& Directory() const noexcept
    {
        return directory_;
    }

    [[nodiscard]] static QString DefaultDirectory();

private:
    [[nodiscard]] QString CredentialPath(
        std::string_view credentialId) const;

    QString directory_;
};

}  // namespace remote
