// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <functional>

namespace remote::media_intelligence {

class IExecutor {
public:
    virtual ~IExecutor() = default;
    virtual void Post(std::function<void()> task) = 0;

    // The task must run no earlier than delayMs and must not run inline.
    // Implementations may drop queued work during host shutdown.
    virtual void PostAfter(
        std::uint32_t delayMs,
        std::function<void()> task) = 0;
};

}  // namespace remote::media_intelligence
