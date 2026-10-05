// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <string>
#include <string_view>

#include "media_intelligence/remote/VisionApiTypes.h"

namespace remote::media_intelligence {

class ISecretProvider {
public:
    virtual ~ISecretProvider() = default;

    virtual bool LoadSecret(
        std::string_view credentialId,
        SecretValue* secret,
        std::string* error) = 0;
};

}  // namespace remote::media_intelligence
