// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QObject>

#include "media_intelligence/remote/IExecutor.h"

namespace remote::app {

class QtVisionApiExecutor final
    : public QObject,
      public media_intelligence::IExecutor {
public:
    explicit QtVisionApiExecutor(QObject* parent = nullptr);

    void Post(std::function<void()> task) override;
    void PostAfter(
        std::uint32_t delayMs,
        std::function<void()> task) override;
};

}  // namespace remote::app
