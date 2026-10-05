// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>

namespace remote::media_intelligence {

class IClock {
public:
    virtual ~IClock() = default;
    [[nodiscard]] virtual std::uint64_t NowMs() const noexcept = 0;
};

}  // namespace remote::media_intelligence
