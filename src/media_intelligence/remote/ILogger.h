// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <string_view>

namespace remote::media_intelligence {

enum class VisionLogLevel {
    kDebug,
    kInfo,
    kWarning,
    kError,
};

class ILogger {
public:
    virtual ~ILogger() = default;

    // Messages are event codes or sanitized diagnostics. Implementations must
    // never receive request bodies, image data, credentials, or response text.
    virtual void Log(VisionLogLevel level, std::string_view message) = 0;
};

}  // namespace remote::media_intelligence
