// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <functional>

#include "media_intelligence/remote/VisionApiTypes.h"

namespace remote::media_intelligence {

class IHttpTransport {
public:
    using RequestId = std::uint64_t;
    using Completion = std::function<void(HttpResponse)>;

    virtual ~IHttpTransport() = default;

    // Completion must not run inline before Start returns. The transport owns
    // the request and authorization values after this call.
    virtual RequestId Start(
        HttpRequest request,
        HttpAuthorization authorization,
        Completion completion) = 0;
    virtual void Cancel(RequestId requestId) = 0;
};

}  // namespace remote::media_intelligence
