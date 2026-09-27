// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace remote::app {

class SessionStatsPoller final {
public:
    using PollAction = std::function<void()>;

    SessionStatsPoller() = default;
    ~SessionStatsPoller();

    SessionStatsPoller(const SessionStatsPoller&) = delete;
    SessionStatsPoller& operator=(const SessionStatsPoller&) = delete;

    void Start(
        PollAction action,
        std::chrono::milliseconds interval = std::chrono::seconds(1));
    void Stop();

private:
    std::jthread worker_;
    std::condition_variable_any wake_;
    std::mutex waitMutex_;
};

}  // namespace remote::app
