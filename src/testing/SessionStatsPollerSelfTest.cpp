// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "src/apps/remote/SessionStatsPoller.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

namespace {
using Clock = std::chrono::steady_clock;
using namespace std::chrono_literals;

bool Check(bool value, const char* name)
{
    std::cout << name << '=' << (value ? "PASS" : "FAIL") << '\n';
    return value;
}
}

int main()
{
    remote::app::SessionStatsPoller poller;
    bool passed = true;
    std::mutex samplesMutex;
    std::condition_variable samplesChanged;
    std::vector<Clock::time_point> polls;
    std::vector<Clock::time_point> ticks;
    poller.Start([&] {
        std::lock_guard lock(samplesMutex);
        polls.push_back(Clock::now());
        samplesChanged.notify_all();
    }, 1s, [&] {
        std::lock_guard lock(samplesMutex);
        ticks.push_back(Clock::now());
        samplesChanged.notify_all();
    }, 50ms);
    bool enough = false;
    {
        std::unique_lock lock(samplesMutex);
        enough = samplesChanged.wait_for(lock, 2500ms,
            [&] { return polls.size() >= 2 && ticks.size() >= 22; });
    }
    const auto stopStarted = Clock::now();
    poller.Stop();
    const auto stopDuration = Clock::now() - stopStarted;
    bool independent = enough && polls.size() == 2 && ticks.size() >= 22;
    if (polls.size() >= 2) {
        const auto interval = polls[1] - polls[0];
        independent = independent && interval >= 950ms && interval <= 1500ms;
    }
    passed &= Check(independent, "FIFTY_MS_TICK_DOES_NOT_INCREASE_FULL_STATS_FREQUENCY");
    passed &= Check(stopDuration < 250ms, "STOP_INTERRUPTS_PENDING_WAIT");
    const auto stoppedPolls = polls.size();
    const auto stoppedTicks = ticks.size();
    std::this_thread::sleep_for(120ms);
    passed &= Check(polls.size() == stoppedPolls && ticks.size() == stoppedTicks,
        "STOP_JOINS_ALL_CALLBACKS");

    ticks.clear();
    polls.clear();
    bool slowCallbackFinished = false;
    Clock::time_point slowFinishedAt;
    poller.Start([&] {
        std::lock_guard lock(samplesMutex);
        polls.push_back(Clock::now());
    }, 1s, [&] {
        bool isFirst;
        {
            std::lock_guard lock(samplesMutex);
            ticks.push_back(Clock::now());
            isFirst = ticks.size() == 1;
        }
        if (isFirst) {
            std::this_thread::sleep_for(230ms);
            std::lock_guard lock(samplesMutex);
            slowFinishedAt = Clock::now();
            slowCallbackFinished = true;
        }
        samplesChanged.notify_all();
    }, 50ms);
    {
        std::unique_lock lock(samplesMutex);
        enough = samplesChanged.wait_for(lock, 1500ms,
            [&] { return slowCallbackFinished && ticks.size() >= 4; });
    }
    poller.Stop();
    bool noCatchup = enough && polls.size() == 1 && ticks.size() >= 4
        && ticks[1] - slowFinishedAt >= 40ms;
    for (std::size_t index = 2; index < ticks.size(); ++index) {
        noCatchup = noCatchup && ticks[index] - ticks[index - 1] >= 40ms;
    }
    passed &= Check(noCatchup, "SLOW_CALLBACK_SKIPS_MISSED_TICKS_WITHOUT_BURST");

    std::atomic<unsigned> restartCalls{0};
    poller.Start([&] { ++restartCalls; }, 50ms);
    std::this_thread::sleep_for(130ms);
    poller.Stop();
    passed &= Check(restartCalls.load() >= 2, "RESTART_AND_FULL_POLL_ONLY_MODE");
    poller.Stop();
    poller.Start({}, 0ms, [&] { ++restartCalls; }, 0ms);
    const auto beforeEmptyStart = restartCalls.load();
    std::this_thread::sleep_for(30ms);
    poller.Stop();
    passed &= Check(restartCalls.load() == beforeEmptyStart,
        "EMPTY_POLL_ACTION_CANNOT_START_BUSY_WORKER");
    std::cout << "SESSION_STATS_POLLER=" << (passed ? "PASS" : "FAIL") << '\n';
    return passed ? 0 : 1;
}
