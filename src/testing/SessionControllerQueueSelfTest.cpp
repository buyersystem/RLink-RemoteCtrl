// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

// Compile the implementation once in this probe so its private serial executor
// can be tested deterministically without adding a production test hook. Do not
// also compile SessionController.cpp or link rlink_core into this test target.
#include "src/core/SessionController.cpp"

#include <atomic>
#include <iostream>

namespace {
using namespace remote;
using namespace std::chrono_literals;

int failures = 0;

void Check(bool condition, const char* message)
{
    (condition ? std::cout : std::cerr)
        << (condition ? "PASS " : "FAIL ") << message << '\n';
    if (!condition) ++failures;
}

class FakeSession final : public IWebRtcSession {
public:
    struct Send {
        std::string channel;
        std::vector<std::uint8_t> payload;
        bool binary;
    };

    void SetObserver(IWebRtcSessionObserver*) override {}
    OperationId Start(const WebRtcSessionConfig&) override { return 1; }
    OperationId CreateOffer() override { return 2; }
    OperationId CreateIceRestartOffer() override { return 3; }
    OperationId CreateAnswer() override { return 4; }
    OperationId ApplyRemoteDescription(const SessionDescription&) override { return 5; }
    OperationId AddRemoteIceCandidate(const IceCandidate&) override { return 6; }
    OperationId CreateDataChannels(const std::vector<DataChannelSpec>&) override { return 7; }
    std::optional<std::uint64_t> DataChannelBufferedAmount(const std::string&) const override
    {
        return 0;
    }
    void Close() override
    {
        std::lock_guard lock(mutex_);
        closed_ = true;
    }

    SendResult SendData(const std::string& channel, std::span<const std::uint8_t> payload,
                        bool binary) override
    {
        std::unique_lock lock(mutex_);
        if (closed_) return SendResult::kSessionNotStarted;
        sends_.push_back({channel, {payload.begin(), payload.end()}, binary});
        sendThread_ = std::this_thread::get_id();
        if (sends_.size() == 1) {
            entered_ = true;
            condition_.notify_all();
            condition_.wait(lock, [this] { return released_; });
        }
        switch (payload.empty() ? 0 : payload.front()) {
        case 2: return SendResult::kSendFailed;
        case 3: return SendResult::kChannelNotOpen;
        default: return SendResult::kSent;
        }
    }

    bool WaitEntered()
    {
        std::unique_lock lock(mutex_);
        return condition_.wait_for(lock, 5s, [this] { return entered_; });
    }
    void Release()
    {
        {
            std::lock_guard lock(mutex_);
            released_ = true;
        }
        condition_.notify_all();
    }
    std::vector<Send> Sends() const
    {
        std::lock_guard lock(mutex_);
        return sends_;
    }
    std::thread::id SendThread() const
    {
        std::lock_guard lock(mutex_);
        return sendThread_;
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    bool entered_ = false;
    bool released_ = false;
    bool closed_ = false;
    std::vector<Send> sends_;
    std::thread::id sendThread_;
};

class FakeSignaling final : public ISessionSignalingSender {
public:
    bool SendDescription(const SessionDescription&) override { return true; }
    bool SendIceCandidate(const IceCandidate&) override { return true; }
};

class Completions {
public:
    void Add(SendResult result)
    {
        {
            std::lock_guard lock(mutex_);
            results_.push_back(result);
            threads_.push_back(std::this_thread::get_id());
        }
        condition_.notify_all();
    }
    bool Wait(std::size_t count)
    {
        std::unique_lock lock(mutex_);
        return condition_.wait_for(lock, 5s, [this, count] { return results_.size() >= count; });
    }
    std::vector<SendResult> Results() const
    {
        std::lock_guard lock(mutex_);
        return results_;
    }
    bool AllOn(std::thread::id expected) const
    {
        std::lock_guard lock(mutex_);
        return std::all_of(threads_.begin(), threads_.end(),
                           [expected](auto thread) { return thread == expected; });
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::vector<SendResult> results_;
    std::vector<std::thread::id> threads_;
};

void TestQueueData()
{
    FakeSession session;
    FakeSignaling signaling;
    Completions completions;
    ControllerSessionController controller(session, signaling);
    auto completed = [&completions](SendResult result) { completions.Add(result); };
    const std::vector<std::uint8_t> first{1};
    Check(controller.QueueData("control", first, true, completed), "first queued send accepted");
    Check(session.WaitEntered(), "fake transport holds the executor in its first send");

    std::vector<std::uint8_t> payload{2, 42};
    const auto begin = std::chrono::steady_clock::now();
    const bool accepted = controller.QueueData("preference", payload, false, completed);
    const auto elapsed = std::chrono::steady_clock::now() - begin;
    payload.assign({99, 99});
    const std::vector<std::uint8_t> third{3};
    Check(controller.QueueData("preference", third, true, completed), "following preference queued in order");
    Check(accepted && completions.Results().empty() && session.Sends().size() == 1,
          "QueueData returns while the transport is blocked without reporting false success");
    std::cout << "TIMING queue_while_executor_blocked "
              << std::chrono::duration<double, std::milli>(elapsed).count() << " ms\n";
    session.Release();
    Check(completions.Wait(3), "all queued sends eventually deliver real completion");
    const auto results = completions.Results();
    Check(results == std::vector<SendResult>{SendResult::kSent, SendResult::kSendFailed,
                                            SendResult::kChannelNotOpen},
          "completion preserves exact transport success/failure and send order");
    const auto sends = session.Sends();
    Check(sends.size() == 3 && sends[1].payload == std::vector<std::uint8_t>{2, 42} &&
          sends[1].channel == "preference" && !sends[1].binary,
          "queued send owns its payload and preserves channel/binary metadata");
    Check(session.SendThread() != std::this_thread::get_id() &&
          completions.AllOn(session.SendThread()),
          "normal send completion executes on the serial executor, not the caller");
    const std::vector<std::uint8_t> fourth{4};
    Check(controller.QueueData("optional-completion", fourth, true),
          "queued sends may omit the completion callback");
    Check(controller.SendData("barrier", fourth, true) == SendResult::kSent &&
          session.Sends().size() == 5,
          "existing synchronous API still works and follows queued sends");
    controller.Close();
    Check(controller.QueueData("after-close", fourth, true, completed) && completions.Wait(4),
          "send queued behind Close still completes rather than leaving its caller pending");
    const auto closedResults = completions.Results();
    Check(closedResults.size() == 4 && closedResults.back() == SendResult::kSessionNotStarted,
          "closed transport completion reports not-started instead of false send success");
}

void TestLatestTransientData()
{
    FakeSession session;
    FakeSignaling signaling;
    ControllerSessionController controller(session, signaling);
    const std::vector<std::uint8_t> initial{1};
    Check(controller.QueueLatestData("telemetry", "cursor-position", initial, true),
          "first transient cursor send accepted");
    Check(session.WaitEntered(), "blocked transport holds the in-flight cursor");
    const auto begin = std::chrono::steady_clock::now();
    bool accepted = true;
    for (int i = 0; i < 1200; ++i) {
        const std::vector<std::uint8_t> update{4,
            static_cast<std::uint8_t>(i >> 8), static_cast<std::uint8_t>(i)};
        accepted = controller.QueueLatestData("telemetry", "cursor-position", update, true)
            && accepted;
    }
    std::vector<std::uint8_t> other{5, 42};
    Check(controller.QueueLatestData("telemetry", "receiver-feedback", other, false),
          "other transient key is not overwritten by cursor positions");
    other.assign({99});
    Check(controller.QueueLatestData("other-channel", "cursor-position", initial, true),
          "transient keys are isolated between channels");
    Check(controller.QueueData("control-reliable", initial, true),
          "reliable shape/reset events keep their normal queue");
    Check(accepted && session.Sends().size() == 1,
          "1200 cursor positions return without blocking or sending obsolete backlog");
    std::cout << "TIMING coalesce_1200_cursor_positions "
              << std::chrono::duration<double, std::milli>(
                     std::chrono::steady_clock::now() - begin).count() << " ms\n";
    session.Release();
    Check(controller.SendData("barrier", initial, true) == SendResult::kSent,
          "queued transient sends complete before the transport barrier");
    const auto sends = session.Sends();
    Check(sends.size() == 6 && sends[1].channel == "telemetry" &&
          sends[1].payload == std::vector<std::uint8_t>{4, 4, 175},
          "busy cursor transport sends only in-flight and newest pending position");
    Check(sends.size() == 6 && sends[2].payload == std::vector<std::uint8_t>{5, 42} &&
          !sends[2].binary && sends[3].channel == "other-channel" &&
          sends[4].channel == "control-reliable",
          "coalescing owns payloads, separates keys and preserves reliable events");
    const std::vector<std::uint8_t> afterDrain{6};
    Check(controller.QueueLatestData("telemetry", "cursor-position", afterDrain, true) &&
          controller.SendData("barrier", initial, true) == SendResult::kSent,
          "drained cursor key schedules the next update without a lost wakeup");
    const auto finalSends = session.Sends();
    Check(finalSends.size() == 8 && finalSends[6].payload == afterDrain,
          "newest-only queue resumes after it becomes empty");
}

void TestExecutorCancellation()
{
    SerialExecutor executor;
    std::mutex gateMutex;
    std::condition_variable gateCondition;
    bool entered = false;
    bool release = false;
    std::atomic<int> executed{0};
    std::atomic<int> runningCanceled{0};
    std::atomic<int> queuedCanceled{0};
    std::atomic<bool> reentrantPostRejected{false};
    std::promise<void> cancellationNotified;
    auto notified = cancellationNotified.get_future();
    Check(executor.PostWithCancellation([&] {
        std::unique_lock lock(gateMutex);
        entered = true;
        gateCondition.notify_all();
        gateCondition.wait(lock, [&] { return release; });
        ++executed;
    }, [&] { ++runningCanceled; }), "running task accepted with cancellation notification");
    {
        std::unique_lock lock(gateMutex);
        Check(gateCondition.wait_for(lock, 5s, [&] { return entered; }),
              "executor test barrier entered");
    }
    Check(executor.PostWithCancellation([&] { ++executed; }, [&] {
        ++queuedCanceled;
        reentrantPostRejected = !executor.Post([] {});
        cancellationNotified.set_value();
    }), "pending send accepted before shutdown");
    Check(executor.PostWithCancellation([&] { ++executed; }, [&] { ++queuedCanceled; }),
          "second pending send accepted before shutdown");
    std::thread stopper([&] { executor.Stop(); });
    Check(notified.wait_for(5s) == std::future_status::ready,
          "shutdown reports discarded sends before the running transport returns");
    {
        std::lock_guard lock(gateMutex);
        release = true;
    }
    gateCondition.notify_all();
    stopper.join();
    Check(executed == 1 && runningCanceled == 0 && queuedCanceled == 2,
          "running task finishes normally; each unsent accepted task is canceled exactly once");
    Check(reentrantPostRejected, "cancellation may reenter Post without lock deadlock");
    int rejectedCancellation = 0;
    Check(!executor.PostWithCancellation([] {}, [&] { ++rejectedCancellation; }) &&
          rejectedCancellation == 0,
          "shutdown rejects new work without invoking an unaccepted-task callback");
    executor.Stop();
    Check(queuedCanceled == 2, "repeated Stop does not repeat completion notifications");
}
}  // namespace

int main()
{
    TestQueueData();
    TestLatestTransientData();
    TestExecutorCancellation();
    std::cout << "RESULT session_controller_queue " << failures << " failures\n";
    return failures == 0 ? 0 : 1;
}
