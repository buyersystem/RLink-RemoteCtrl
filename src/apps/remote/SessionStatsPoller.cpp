// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "SessionStatsPoller.h"

#include <utility>

namespace remote::app {

SessionStatsPoller::~SessionStatsPoller()
{
    Stop();
}

void SessionStatsPoller::Start(
    PollAction action,
    const std::chrono::milliseconds interval)
{
    Stop();
    worker_ = std::jthread(
        [this, action = std::move(action), interval](
            const std::stop_token stopToken) {
            while (!stopToken.stop_requested()) {
                action();
                std::unique_lock waitLock(waitMutex_);
                wake_.wait_for(
                    waitLock, stopToken, interval, [] { return false; });
            }
        });
}

void SessionStatsPoller::Stop()
{
    if (!worker_.joinable()) {
        return;
    }
    worker_.request_stop();
    wake_.notify_all();
    worker_.join();
}

}  // namespace remote::app
