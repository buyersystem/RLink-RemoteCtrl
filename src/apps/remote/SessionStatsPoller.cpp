// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "SessionStatsPoller.h"

#include <algorithm>
#include <utility>

namespace remote::app {

SessionStatsPoller::~SessionStatsPoller()
{
    Stop();
}

void SessionStatsPoller::Start(
    PollAction action,
    const std::chrono::milliseconds interval,
    PollAction tickAction,
    const std::chrono::milliseconds tickInterval)
{
    Stop();
    if (!action) {
        return;
    }
    const auto pollPeriod = (std::max)(interval, std::chrono::milliseconds(1));
    const auto tickPeriod = (std::max)(tickInterval, std::chrono::milliseconds(1));
    worker_ = std::jthread(
        [this, action = std::move(action), tickAction = std::move(tickAction),
            pollPeriod, tickPeriod](
            const std::stop_token stopToken) {
            using Clock = std::chrono::steady_clock;
            auto nextPollAt = Clock::now();
            auto nextTickAt = nextPollAt;
            const auto advanceDeadline = [](auto& deadline, auto period) {
                deadline += period;
                // A slow callback does not create a backlog of catch-up work.
                const auto now = Clock::now();
                if (deadline <= now) {
                    deadline = now + period;
                }
            };
            while (!stopToken.stop_requested()) {
                if (Clock::now() >= nextPollAt) {
                    action();
                    advanceDeadline(nextPollAt, pollPeriod);
                }
                if (stopToken.stop_requested()) {
                    break;
                }
                if (tickAction && Clock::now() >= nextTickAt) {
                    tickAction();
                    advanceDeadline(nextTickAt, tickPeriod);
                }
                const auto nextWakeAt = tickAction
                    ? (std::min)(nextPollAt, nextTickAt) : nextPollAt;
                std::unique_lock waitLock(waitMutex_);
                wake_.wait_until(
                    waitLock, stopToken, nextWakeAt, [] { return false; });
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
