// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)
#pragma once

#include <Windows.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <stop_token>

namespace remote::desktop_capture_timing {

using Clock = std::chrono::steady_clock;
enum class WaitResult { kDeadlineReached, kScheduleChanged, kStopped, kFailed };
inline constexpr auto kSpinThreshold = std::chrono::microseconds(200);

inline auto FrameInterval(std::uint32_t fps) noexcept
{
    const auto rate = (std::max)(fps, 1u);
    return std::chrono::microseconds((1'000'000ULL + rate - 1) / rate);
}

inline Clock::time_point RetuneDeadline(Clock::time_point deadline,
    Clock::time_point captureStarted, std::uint32_t previousFps,
    std::uint32_t requestedFps) noexcept
{
    // A refresh or an input wake is not permission to capture an extra frame.
    return previousFps == requestedFps ? deadline
                                      : captureStarted + FrameInterval(requestedFps);
}

inline Clock::time_point AdvanceDeadline(Clock::time_point deadline,
    Clock::time_point captureStarted, Clock::time_point captureFinished,
    std::uint32_t fps) noexcept
{
    const auto interval = FrameInterval(fps);
    deadline += interval;
    // Keep phase under ordinary jitter. After a long pause, skip historical
    // slots; the next capture cannot be an immediate duplicate of this one.
    if (captureFinished - deadline >= interval)
        deadline = (std::max)(captureFinished, captureStarted + interval);
    return deadline;
}

inline DWORD DeadlineTimeout(Clock::time_point deadline) noexcept
{
    const auto remaining = std::chrono::duration_cast<std::chrono::microseconds>(
        deadline - Clock::now()).count();
    const auto roundedMs = (std::max<std::int64_t>)(0, remaining + 999) / 1000;
    return static_cast<DWORD>((std::min<std::int64_t>)(roundedMs + 2, MAXDWORD - 1));
}

inline WaitResult WaitForDeadline(HANDLE timer, HANDLE stopEvent,
    HANDLE scheduleWakeEvent, Clock::time_point deadline, std::stop_token stopToken)
{
    if (!stopEvent || !scheduleWakeEvent) return WaitResult::kFailed;
    if (stopToken.stop_requested()) return WaitResult::kStopped;
    const auto coarseDeadline = deadline - kSpinThreshold;
    auto now = Clock::now();
    if (now < coarseDeadline) {
        LARGE_INTEGER due{};
        due.QuadPart = -(std::max<std::int64_t>)(1,
            std::chrono::duration_cast<std::chrono::nanoseconds>(coarseDeadline - now).count() / 100);
        DWORD result = WAIT_FAILED;
        if (timer && SetWaitableTimerEx(timer, &due, 0, nullptr, nullptr, nullptr, 0)) {
            const HANDLE waits[] = {stopEvent, scheduleWakeEvent, timer};
            // The stop event remains interruptible, and a missed timer signal
            // cannot leave capture waiting indefinitely after network recovery.
            result = WaitForMultipleObjects(3, waits, FALSE, DeadlineTimeout(deadline));
            if (result == WAIT_OBJECT_0) return WaitResult::kStopped;
            if (result == WAIT_OBJECT_0 + 1) {
                CancelWaitableTimer(timer);
                return WaitResult::kScheduleChanged;
            }
        }
        if (result == WAIT_FAILED) {
            // Creation/arming/wait failures use a finite event wait instead
            // of an entire frame interval of busy spinning.
            const HANDLE waits[] = {stopEvent, scheduleWakeEvent};
            result = WaitForMultipleObjects(2, waits, FALSE, DeadlineTimeout(coarseDeadline));
            if (result == WAIT_FAILED) return WaitResult::kFailed;
            if (result == WAIT_OBJECT_0) return WaitResult::kStopped;
            if (result == WAIT_OBJECT_0 + 1) return WaitResult::kScheduleChanged;
        }
    }
    while (!stopToken.stop_requested() && Clock::now() < deadline) {
        const HANDLE waits[] = {stopEvent, scheduleWakeEvent};
        const auto result = WaitForMultipleObjects(2, waits, FALSE, 0);
        if (result == WAIT_FAILED) return WaitResult::kFailed;
        if (result == WAIT_OBJECT_0) return WaitResult::kStopped;
        if (result == WAIT_OBJECT_0 + 1) return WaitResult::kScheduleChanged;
        YieldProcessor();
    }
    return stopToken.stop_requested() ? WaitResult::kStopped : WaitResult::kDeadlineReached;
}

} // namespace remote::desktop_capture_timing
