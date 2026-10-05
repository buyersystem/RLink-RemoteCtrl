// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QObject>

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_set>

#include "media_intelligence/remote/IHttpTransport.h"

class QNetworkAccessManager;

namespace remote::app {

class QtVisionApiHttpTransport final
    : public QObject,
      public media_intelligence::IHttpTransport {
public:
    explicit QtVisionApiHttpTransport(QObject* parent = nullptr);
    ~QtVisionApiHttpTransport() override;

    RequestId Start(
        media_intelligence::HttpRequest request,
        media_intelligence::HttpAuthorization authorization,
        Completion completion) override;
    void Cancel(RequestId requestId) override;

private:
    struct Impl;

    std::unique_ptr<Impl> impl_;
    std::atomic<RequestId> nextRequestId_{1};
    std::mutex canceledMutex_;
    std::unordered_set<RequestId> pendingStartIds_;
    std::unordered_set<RequestId> canceledBeforeStart_;
};

}  // namespace remote::app
