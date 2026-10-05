// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "src/apps/remote/InProcessSessionEngineInternal.h"
#include "src/protocol/DataChannelCatalog.h"
#include "src/protocol/ScreenShareControlProtocol.h"

#include <QCoreApplication>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <stdexcept>

namespace remote::testing {
namespace {
using namespace std::chrono_literals;
int failures = 0;
void Check(bool value, const char* description)
{
    (value ? std::cout : std::cerr) << (value ? "PASS " : "FAIL ")
        << description << '\n';
    if (!value) ++failures;
}

class FakeSession final : public IWebRtcSession {
public:
    void SetObserver(IWebRtcSessionObserver*) override {}
    OperationId Start(const WebRtcSessionConfig&) override { return 1; }
    OperationId CreateOffer() override { return 2; }
    OperationId CreateIceRestartOffer() override { return 3; }
    OperationId CreateAnswer() override { return 4; }
    OperationId ApplyRemoteDescription(const SessionDescription&) override { return 5; }
    OperationId AddRemoteIceCandidate(const IceCandidate&) override { return 6; }
    OperationId CreateDataChannels(const std::vector<DataChannelSpec>&) override { return 7; }
    std::optional<std::uint64_t> DataChannelBufferedAmount(const std::string&) const override
    { return 0; }
    void Close() override { Release(SendResult::kSessionNotStarted); }
    SendResult SendData(const std::string&, std::span<const std::uint8_t> payload,
                        bool binary) override
    {
        std::unique_lock lock(mutex_);
        ScreenStreamPreferenceRequest decoded;
        metadataValid_ = metadataValid_ && binary &&
            DecodeScreenStreamPreferenceRequest(payload, &decoded) &&
            decoded.senderDeviceId == "viewer" && decoded.sequence != 0;
        if (!blocked_) return SendResult::kSent;
        entered_ = true;
        condition_.notify_all();
        if (!condition_.wait_for(lock, 5s, [this] { return !blocked_; })) {
            return SendResult::kSendFailed;
        }
        return result_;
    }
    void Arm()
    {
        std::lock_guard lock(mutex_);
        entered_ = false;
        blocked_ = true;
    }
    void WaitEntered()
    {
        std::unique_lock lock(mutex_);
        if (!condition_.wait_for(lock, 5s, [this] { return entered_; })) {
            throw std::runtime_error("fake send did not enter");
        }
    }
    void Release(SendResult result)
    {
        { std::lock_guard lock(mutex_); result_ = result; blocked_ = false; }
        condition_.notify_all();
    }
    bool MetadataValid() const
    { std::lock_guard lock(mutex_); return metadataValid_; }
private:
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    bool blocked_ = false, entered_ = false, metadataValid_ = true;
    SendResult result_ = SendResult::kSent;
};

class FakeSignaling final : public ISessionSignalingSender {
public:
    bool SendDescription(const SessionDescription&) override { return true; }
    bool SendIceCandidate(const IceCandidate&) override { return true; }
};

struct Completion {
    std::promise<SessionCommandResult> promise;
    std::future<SessionCommandResult> future = promise.get_future();
    auto Callback() { return [this](SessionCommandResult result) { promise.set_value(result); }; }
    SessionCommandResult Wait()
    {
        if (future.wait_for(5s) != std::future_status::ready) {
            throw std::runtime_error("completion timed out");
        }
        return future.get();
    }
};
ScreenStreamPreferenceRequest Preference(std::uint32_t fps = 60)
{
    ScreenStreamPreferenceRequest request;
    request.framesPerSecond = fps;
    return request;
}
}  // namespace

// Reuses the engine's existing bounded test-access friend. Neither the engine
// nor WebRTC runtime is started; only real queue/dispatch logic uses the fake.
class InProcessSessionEngineTestAccess {
public:
    static void Run()
    {
        FakeSession directTransport, roomTransport;
        FakeSignaling signaling;
        app::InProcessSessionEngine engine;
        {
            std::lock_guard lock(engine.mutex_);
            auto& s = engine.snapshot_;
            s.state = SessionEngineState::kActive;
            s.localDeviceId = "viewer";
            s.sessionId = "direct";
            s.peerDeviceId = "sharer";
            s.purpose = SessionPurpose::kRemoteControl;
            s.remoteControlRole = RemoteControlRole::kController;
            s.direct.screenMaximumFrameRate = 120;
            s.direct.remoteScreenShareGeneration = 4;
            engine.directSession_.openDataChannels[std::string(kControlReliableChannel)] = true;
            engine.sessionController_ =
                std::make_unique<ControllerSessionController>(directTransport, signaling);
        }
        directTransport.Arm();
        Completion first;
        const auto begin = std::chrono::steady_clock::now();
        Check(engine.QueueDirectScreenStreamPreference(Preference(), first.Callback()).accepted,
              "direct engine accepts nonblocking request");
        const auto elapsed = std::chrono::steady_clock::now() - begin;
        directTransport.WaitEntered();
        Check(elapsed < 100ms && first.future.wait_for(0ms) != std::future_status::ready,
              "direct queue acceptance is distinct from actual send completion");
        directTransport.Release(SendResult::kSent);
        Check(first.Wait().accepted && engine.Snapshot().direct.screenPreferencePending,
              "direct real send succeeds but waits for remote ACK");
        AckDirect(engine, true);
        Check(!engine.Snapshot().direct.screenPreferencePending &&
              engine.Snapshot().direct.screenFramesPerSecond == 60,
              "direct existing ACK still applies FPS and clears wait");

        directTransport.Arm();
        Completion rejected;
        engine.QueueDirectScreenStreamPreference(Preference(120), rejected.Callback());
        directTransport.WaitEntered();
        directTransport.Release(SendResult::kSendFailed);
        Check(rejected.Wait().errorCode == "direct_screen_stream_send_failed" &&
              !engine.Snapshot().direct.screenPreferencePending,
              "direct transport failure clears pending and reports exact failure");

        directTransport.Arm();
        Completion older, newer;
        engine.QueueDirectScreenStreamPreference(Preference(30), older.Callback());
        directTransport.WaitEntered();
        engine.QueueDirectScreenStreamPreference(Preference(60), newer.Callback());
        directTransport.Release(SendResult::kSendFailed);
        Check(older.Wait().errorCode == "screen_stream_request_stale" && newer.Wait().accepted &&
              engine.Snapshot().direct.screenPreferencePending,
              "older direct failure cannot clear newer pending preference");
        AckDirect(engine, false);
        Check(engine.Snapshot().error.code == "direct_screen_preference_rejected",
              "direct remote rejection retains existing error behavior");
        Completion afterReject;
        engine.QueueDirectScreenStreamPreference(Preference(), afterReject.Callback());
        Check(afterReject.Wait().accepted && engine.Snapshot().error.code.empty(),
              "new direct pending request clears only obsolete rejection status");

        directTransport.Arm();
        Completion oldGeneration;
        engine.QueueDirectScreenStreamPreference(Preference(), oldGeneration.Callback());
        directTransport.WaitEntered();
        SharedDisplayCatalog catalog;
        catalog.roomId = "direct";
        catalog.senderDeviceId = "sharer";
        catalog.layoutVersion = 2;
        catalog.screenShareGeneration = 5;
        DisplayDescriptor display;
        display.stableDisplayKey = "test-display";
        display.sessionDisplayId = 1;
        display.width = 1920;
        display.height = 1080;
        display.scalePercent = 100;
        catalog.displays.push_back(display);
        std::vector<std::uint8_t> bytes;
        Check(EncodeSharedDisplayCatalog(catalog, &bytes), "encode new-generation display catalog");
        engine.DispatchDirectScreenData(std::string(kControlReliableChannel), bytes);
        directTransport.Release(SendResult::kSent);
        Check(oldGeneration.Wait().errorCode == "screen_stream_request_stale" &&
              !engine.Snapshot().direct.screenPreferencePending,
              "catalog generation change releases wait and invalidates old direct completion");

        directTransport.Arm();
        Completion oldSession;
        engine.QueueDirectScreenStreamPreference(Preference(), oldSession.Callback());
        directTransport.WaitEntered();
        { std::lock_guard lock(engine.mutex_); ++engine.directSessionGeneration_; }
        directTransport.Release(SendResult::kSent);
        Check(oldSession.Wait().errorCode == "screen_stream_request_stale",
              "same direct controller address cannot bypass session-generation guard");

        constexpr auto pairId = "pair";
        auto pair = std::make_shared<app::InProcessSessionEngine::RoomPairRuntime>();
        pair->pairId = pairId;
        pair->peerDeviceId = "sharer";
        pair->openDataChannels[std::string(kControlReliableChannel)] = true;
        pair->controller = std::make_unique<ControllerSessionController>(roomTransport, signaling);
        {
            std::lock_guard lock(engine.mutex_);
            engine.roomPairs_[pairId] = pair;
            auto& s = engine.snapshot_;
            s.room.roomId = "room";
            s.room.membership = RoomMembershipState::kActive;
            s.room.screenShareState = RoomScreenShareState::kActive;
            s.room.screenSharerDeviceId = "sharer";
            s.room.screenShareEpoch = 10;
            s.roomActivity.peerConnections.push_back({.pairId = pairId, .peerDeviceId = "sharer"});
        }
        roomTransport.Arm();
        Completion room;
        Check(engine.QueueRoomScreenStreamPreference(pairId, Preference(), room.Callback()).accepted,
              "room engine accepts queued stream preference");
        roomTransport.WaitEntered();
        Check(room.future.wait_for(0ms) != std::future_status::ready,
              "room API no longer waits for blocked transport");
        roomTransport.Release(SendResult::kSent);
        Check(room.Wait().accepted && engine.Snapshot().roomActivity.peerConnections.front().screenPreferencePending,
              "room real successful send retains remote ACK wait");
        AckRoom(engine, pairId, false);
        roomTransport.Arm();
        Completion roomFailure;
        engine.QueueRoomScreenStreamPreference(pairId, Preference(), roomFailure.Callback());
        roomTransport.WaitEntered();
        Check(engine.Snapshot().roomActivity.peerConnections.front().errorCode.empty(),
              "new room request clears obsolete remote rejection only");
        roomTransport.Release(SendResult::kChannelNotOpen);
        Check(roomFailure.Wait().errorCode == "screen_stream_channel_not_open" &&
              !engine.Snapshot().roomActivity.peerConnections.front().screenPreferencePending,
              "room failure clears current pending and reports exact transport result");

        roomTransport.Arm();
        Completion oldRoom, newRoom;
        engine.QueueRoomScreenStreamPreference(pairId, Preference(30), oldRoom.Callback());
        roomTransport.WaitEntered();
        engine.QueueRoomScreenStreamPreference(pairId, Preference(), newRoom.Callback());
        roomTransport.Release(SendResult::kSendFailed);
        Check(oldRoom.Wait().errorCode == "screen_stream_request_stale" && newRoom.Wait().accepted &&
              engine.Snapshot().roomActivity.peerConnections.front().screenPreferencePending,
              "older room failure cannot clear newer pending request");
        AckRoom(engine, pairId, true);
        Check(!engine.Snapshot().roomActivity.peerConnections.front().screenPreferencePending,
              "room existing generation-bound ACK still clears pending");

        roomTransport.Arm();
        Completion roomEpoch;
        engine.QueueRoomScreenStreamPreference(pairId, Preference(), roomEpoch.Callback());
        roomTransport.WaitEntered();
        { std::lock_guard lock(engine.mutex_); ++engine.snapshot_.room.screenShareEpoch; }
        roomTransport.Release(SendResult::kSent);
        Check(roomEpoch.Wait().errorCode == "screen_stream_request_stale",
              "room share epoch invalidates old completion");

        roomTransport.Arm();
        Completion roomBinding;
        engine.QueueRoomScreenStreamPreference(pairId, Preference(), roomBinding.Callback());
        roomTransport.WaitEntered();
        {
            std::lock_guard lock(engine.mutex_);
            engine.roomPairs_[pairId] = std::make_shared<app::InProcessSessionEngine::RoomPairRuntime>();
        }
        roomTransport.Release(SendResult::kSent);
        Check(roomBinding.Wait().errorCode == "screen_stream_request_stale",
              "same room pair ID cannot bypass replaced-runtime ownership guard");
        Check(directTransport.MetadataValid() && roomTransport.MetadataValid(),
              "both queue paths keep original protocol and sender/sequence metadata");
        pair->controller.reset();
        engine.Stop();
    }
private:
    static ScreenStreamPreferenceApplied Ack(std::string room, std::uint64_t sequence,
                                             std::uint64_t generation, bool accepted)
    {
        ScreenStreamPreferenceApplied applied;
        applied.roomId = std::move(room);
        applied.senderDeviceId = "sharer";
        applied.requestSequence = sequence;
        applied.screenShareGeneration = generation;
        applied.accepted = accepted;
        applied.width = 1920;
        applied.height = 1080;
        applied.framesPerSecond = 60;
        applied.error = accepted ? "" : "test rejection";
        return applied;
    }
    static void AckDirect(app::InProcessSessionEngine& engine, bool accepted)
    {
        const auto s = engine.Snapshot();
        auto applied = Ack("direct", s.direct.screenPreferenceSequence,
                           s.direct.remoteScreenShareGeneration, accepted);
        std::vector<std::uint8_t> bytes;
        Check(EncodeScreenStreamPreferenceAppliedV2(applied, &bytes), "encode direct ACK");
        engine.DispatchDirectScreenData(std::string(kControlReliableChannel), bytes);
    }
    static void AckRoom(app::InProcessSessionEngine& engine, const std::string& pair, bool accepted)
    {
        const auto s = engine.Snapshot();
        auto applied = Ack("room", s.roomActivity.peerConnections.front().screenPreferenceSequence,
                           s.room.screenShareEpoch, accepted);
        std::vector<std::uint8_t> bytes;
        Check(EncodeScreenStreamPreferenceAppliedV2(applied, &bytes), "encode room ACK");
        engine.DispatchRoomPairScreenData(pair, bytes);
    }
};
}  // namespace remote::testing

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    try { remote::testing::InProcessSessionEngineTestAccess::Run(); }
    catch (const std::exception& error) {
        std::cerr << "FAIL " << error.what() << '\n';
        return 1;
    }
    std::cout << "RESULT stream_preference_engine " << remote::testing::failures << " failures\n";
    return remote::testing::failures == 0 ? 0 : 1;
}
