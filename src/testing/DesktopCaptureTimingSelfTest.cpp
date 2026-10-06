// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "src/platform/win/DesktopCaptureTiming.h"
#include "src/platform/win/DxgiDesktopFramePolicy.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <stop_token>
#include <string>
#include <thread>

namespace {
namespace timing = remote::desktop_capture_timing;
using namespace std::chrono_literals;

int failures = 0;
void Check(bool ok, const std::string& name)
{
    std::cout << name << '=' << (ok ? "PASS" : "FAIL") << '\n';
    if (!ok) ++failures;
}

class Handle {
public:
    explicit Handle(HANDLE value) : value_(value) {}
    ~Handle() { if (value_) CloseHandle(value_); }
    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;
    HANDLE get() const { return value_; }
private:
    HANDLE value_;
};

void DeadlineModelTests()
{
    Check(!remote::HasDxgiDesktopImageUpdate(0, 0),
        "DXGI_POINTER_ONLY_UPDATE_REUSES_VALID_FRAME");
    Check(!remote::HasDxgiDesktopImageUpdate(0, 1) &&
          !remote::HasDxgiDesktopImageUpdate(1, 0),
        "DXGI_INCOMPLETE_IMAGE_METADATA_NEVER_COPIES_SURFACE");
    Check(remote::HasDxgiDesktopImageUpdate(1, 1) &&
          remote::HasDxgiDesktopImageUpdate(1000000, 5),
        "DXGI_REAL_DESKTOP_UPDATE_STILL_COPIES_SURFACE");
    const auto origin = timing::Clock::time_point{} + 10s;
    Check(timing::FrameInterval(60) == 16667us &&
        timing::FrameInterval(80) == 12500us &&
        timing::FrameInterval(120) == 8334us,
        "FRAME_INTERVAL_ROUNDS_UP_TO_AVOID_EXCEEDING_USER_RATE");
    Check(timing::FrameInterval(0) == 1s,
        "ZERO_RATE_DOES_NOT_DIVIDE_BY_ZERO");

    const auto original = origin + timing::FrameInterval(60);
    auto deadline = original;
    for (int wake = 0; wake != 100000; ++wake)
        deadline = timing::RetuneDeadline(deadline, origin, 60, 60);
    Check(deadline == original, "SAME_RATE_WAKE_STORM_PRESERVES_DEADLINE");
    // Forced-refresh and input messages use the same retune operation: their
    // delivery flags must not turn a wake into permission for another frame.
    for (int refresh = 0; refresh != 10000; ++refresh)
        deadline = timing::RetuneDeadline(deadline, origin, 60, 60);
    Check(deadline == original, "FORCED_REFRESH_PRESERVES_USER_FRAME_INTERVAL");

    const auto faster = timing::RetuneDeadline(original, origin, 60, 120);
    Check(faster == origin + 8334us && faster < original,
        "TARGET_INCREASE_RECOMPUTES_FROM_LAST_CAPTURE_START");
    const auto slower = timing::RetuneDeadline(faster, origin, 120, 30);
    Check(slower == origin + 33334us && slower > faster,
        "TARGET_DECREASE_EXTENDS_EXISTING_DEADLINE");
    Check(timing::RetuneDeadline(slower, origin, 30, 30) == slower,
        "REPEATED_SLOW_TARGET_DOES_NOT_RESTART_OR_ADVANCE_PHASE");

    for (const std::uint32_t fps : {60u, 80u, 120u}) {
        const auto period = timing::FrameInterval(fps);
        auto slot = origin;
        constexpr std::chrono::microseconds jitter[] = {0us, 120us, 500us, 250us};
        bool phaseStable = true;
        bool noImmediateDuplicate = true;
        constexpr int frameCount = 2400;
        timing::Clock::time_point lastStart{};
        for (int frame = 0; frame != frameCount; ++frame) {
            const auto started = slot + jitter[frame % 4];
            const auto finished = started + 300us;
            const auto next = timing::AdvanceDeadline(slot, started, finished, fps);
            phaseStable = phaseStable && next == origin + period * (frame + 1);
            noImmediateDuplicate = noImmediateDuplicate && next > finished;
            slot = timing::RetuneDeadline(next, started, fps, fps);
            lastStart = started;
        }
        const double observedFps = (frameCount - 1) /
            std::chrono::duration<double>(lastStart - origin).count();
        const auto suffix = std::to_string(fps);
        Check(phaseStable && noImmediateDuplicate,
            "JITTER_" + suffix + "_KEEPS_PHASE_WITHOUT_DUPLICATE_FRAMES");
        Check(observedFps <= fps + 0.01 && observedFps >= fps - 0.1,
            "JITTER_" + suffix + "_NEITHER_ALIASES_NOR_OVERSHOOTS");

        const auto resumedAt = origin + 5s;
        const auto resumedNext = timing::AdvanceDeadline(
            origin, resumedAt, resumedAt + 300us, fps);
        Check(resumedNext == resumedAt + period && resumedNext > resumedAt,
            "LONG_PAUSE_" + suffix + "_SKIPS_HISTORY_WITHOUT_CATCHUP_BURST");
        auto resumedSlot = resumedNext;
        bool resumesAtRequestedRate = true;
        for (int frame = 0; frame != 100; ++frame) {
            const auto next = timing::AdvanceDeadline(
                resumedSlot, resumedSlot, resumedSlot + 200us, fps);
            resumesAtRequestedRate = resumesAtRequestedRate &&
                next - resumedSlot == period;
            resumedSlot = next;
        }
        Check(resumesAtRequestedRate,
            "LONG_PAUSE_" + suffix + "_RECOVERS_TARGET_WITHOUT_STALE_LOW_RATE");
    }
}

void RealDeadlineWait(HANDLE timer, const std::string& name)
{
    Handle stopped(CreateEventW(nullptr, TRUE, FALSE, nullptr));
    Handle schedule(CreateEventW(nullptr, FALSE, FALSE, nullptr));
    if (!stopped.get() || !schedule.get()) {
        Check(false, name + "_EVENT_CREATION");
        return;
    }
    const auto started = timing::Clock::now();
    const auto deadline = started + 30ms;
    const auto result = timing::WaitForDeadline(
        timer, stopped.get(), schedule.get(), deadline, {});
    const auto finished = timing::Clock::now();
    Check(result == timing::WaitResult::kDeadlineReached && finished >= deadline &&
        finished - started < 300ms, name);
}

void Win32WaitTests()
{
    Handle highResolution(CreateWaitableTimerExW(nullptr, nullptr,
        CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_MODIFY_STATE | SYNCHRONIZE));
    Handle ordinary(CreateWaitableTimerExW(nullptr, nullptr, 0,
        TIMER_MODIFY_STATE | SYNCHRONIZE));
    Check(ordinary.get() != nullptr, "ORDINARY_WAITABLE_TIMER_CREATED");
    if (highResolution.get())
        RealDeadlineWait(highResolution.get(), "HIGH_RESOLUTION_TIMER_REACHES_DEADLINE");
    else
        std::cout << "HIGH_RESOLUTION_TIMER=UNAVAILABLE_USING_FALLBACK\n";
    if (ordinary.get())
        RealDeadlineWait(ordinary.get(), "ORDINARY_TIMER_REACHES_DEADLINE");
    RealDeadlineWait(nullptr, "MISSING_TIMER_FINITE_EVENT_FALLBACK");
    RealDeadlineWait(INVALID_HANDLE_VALUE, "FAILED_TIMER_ARM_FINITE_EVENT_FALLBACK");

    Handle stopped(CreateEventW(nullptr, TRUE, FALSE, nullptr));
    Handle schedule(CreateEventW(nullptr, FALSE, FALSE, nullptr));
    if (!stopped.get() || !schedule.get()) {
        Check(false, "INTERRUPT_EVENT_CREATION");
        return;
    }

    {
        const auto started = timing::Clock::now();
        std::jthread signal([&] { std::this_thread::sleep_for(10ms); SetEvent(schedule.get()); });
        const auto result = timing::WaitForDeadline(ordinary.get(), stopped.get(),
            schedule.get(), started + 200ms, {});
        Check(result == timing::WaitResult::kScheduleChanged &&
            timing::Clock::now() - started < 150ms,
            "SCHEDULE_WAKE_INTERRUPTS_WAIT_WITHOUT_CAPTURING_EARLY");
    }
    {
        const auto started = timing::Clock::now();
        std::jthread signal([&] { std::this_thread::sleep_for(10ms); SetEvent(stopped.get()); });
        const auto result = timing::WaitForDeadline(ordinary.get(), stopped.get(),
            schedule.get(), started + 200ms, {});
        Check(result == timing::WaitResult::kStopped &&
            timing::Clock::now() - started < 150ms,
            "STOP_EVENT_INTERRUPTS_WAIT_PROMPTLY");
    }
    ResetEvent(stopped.get());

    if (ordinary.get()) {
        const auto started = timing::Clock::now();
        const auto deadline = started + 80ms;
        std::atomic<bool> complete{false};
        std::jthread canceller([&] {
            std::this_thread::sleep_for(15ms);
            CancelWaitableTimer(ordinary.get());
            // A watchdog prevents an implementation regression from hanging
            // the self-test; a watchdog stop is a failure, never a pass.
            const auto limit = started + 400ms;
            while (!complete.load() && timing::Clock::now() < limit)
                std::this_thread::sleep_for(1ms);
            if (!complete.load()) SetEvent(stopped.get());
        });
        const auto result = timing::WaitForDeadline(ordinary.get(), stopped.get(),
            schedule.get(), deadline, {});
        const auto finished = timing::Clock::now();
        complete.store(true);
        Check(result == timing::WaitResult::kDeadlineReached && finished >= deadline &&
            finished - started < 300ms,
            "LOST_TIMER_SIGNAL_HAS_FINITE_BACKUP_DEADLINE");
    }
    ResetEvent(stopped.get());
    ResetEvent(schedule.get());

    {
        std::stop_source token;
        token.request_stop();
        const auto started = timing::Clock::now();
        Check(timing::WaitForDeadline(ordinary.get(), stopped.get(), schedule.get(),
            started + 200ms, token.get_token()) == timing::WaitResult::kStopped &&
            timing::Clock::now() - started < 100ms,
            "ALREADY_STOPPED_TOKEN_RETURNS_WITHOUT_WAITING");
    }
    {
        std::stop_source token;
        std::stop_callback wake(token.get_token(), [&] { SetEvent(stopped.get()); });
        const auto started = timing::Clock::now();
        std::jthread request([&] { std::this_thread::sleep_for(10ms); token.request_stop(); });
        const auto result = timing::WaitForDeadline(ordinary.get(), stopped.get(),
            schedule.get(), started + 200ms, token.get_token());
        Check(result == timing::WaitResult::kStopped &&
            timing::Clock::now() - started < 150ms,
            "PRODUCTION_STOP_TOKEN_CALLBACK_INTERRUPTS_KERNEL_WAIT");
    }
    ResetEvent(stopped.get());
    {
        std::stop_source token;
        const auto started = timing::Clock::now();
        std::jthread request([&] { std::this_thread::sleep_for(10ms); token.request_stop(); });
        const auto result = timing::WaitForDeadline(nullptr, stopped.get(), schedule.get(),
            started + 60ms, token.get_token());
        Check(result == timing::WaitResult::kStopped &&
            timing::Clock::now() - started < 250ms,
            "TOKEN_WITHOUT_EXTERNAL_EVENT_CALLBACK_STILL_FINISHES_BY_DEADLINE");
    }

    {
        const auto started = timing::Clock::now();
        const auto result = timing::WaitForDeadline(nullptr, INVALID_HANDLE_VALUE,
            schedule.get(), started + 60ms, {});
        Check(result == timing::WaitResult::kFailed &&
            timing::Clock::now() - started < 100ms,
            "INVALID_STOP_HANDLE_FAILS_WITHOUT_SPIN_OR_HANG");
        Check(timing::WaitForDeadline(nullptr, stopped.get(), nullptr,
            started + 60ms, {}) == timing::WaitResult::kFailed,
            "MISSING_SCHEDULE_HANDLE_FAILS_EXPLICITLY");
    }

    // Exercise the exact wait/retune contract used by both capture loops.
    // Repeated input/refresh events must not keep resetting the capture slot.
    ResetEvent(schedule.get());
    const auto captureStarted = timing::Clock::now();
    auto deadline = captureStarted + 60ms;
    const auto expected = deadline;
    int wakeCount = 0;
    bool reached = false;
    std::jthread storm([&](std::stop_token stop) {
        while (!stop.stop_requested()) {
            SetEvent(schedule.get());
            std::this_thread::sleep_for(1ms);
        }
    });
    while (timing::Clock::now() - captureStarted < 300ms) {
        const auto result = timing::WaitForDeadline(ordinary.get(), stopped.get(),
            schedule.get(), deadline, {});
        if (result == timing::WaitResult::kScheduleChanged) {
            ++wakeCount;
            deadline = timing::RetuneDeadline(deadline, captureStarted, 60, 60);
            continue;
        }
        reached = result == timing::WaitResult::kDeadlineReached;
        break;
    }
    storm.request_stop();
    storm.join();
    Check(reached && wakeCount > 0 && deadline == expected &&
        timing::Clock::now() >= expected && timing::Clock::now() - captureStarted < 300ms,
        "REAL_WAKE_STORM_NEITHER_CAPTURES_EARLY_NOR_PREVENTS_RESUMPTION");
}
} // namespace

int main()
{
    DeadlineModelTests();
    Win32WaitTests();
    Check(failures == 0, "DESKTOP_CAPTURE_TIMING");
    return failures == 0 ? 0 : 1;
}
