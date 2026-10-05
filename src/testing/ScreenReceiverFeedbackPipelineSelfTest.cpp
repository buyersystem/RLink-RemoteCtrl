// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <thread>
#include <vector>

#include "api/make_ref_counted.h"
#include "api/video/adapted_video_track_source.h"
#include "api/video/i420_buffer.h"
#include "rtc_base/time_utils.h"
#include "src/core/SessionController.h"
#include "src/protocol/DataChannelCatalog.h"
#include "src/protocol/ScreenReceiverFeedbackProtocol.h"
#include "src/webrtc/LibWebRtcSession.h"
#include "src/webrtc/WebRtcRuntime.h"

namespace remote {
namespace {
using namespace std::chrono_literals;
constexpr int kWidth = 320;
constexpr int kHeight = 180;
constexpr int kFps = 30;
constexpr auto kOperationTimeout = 10s;

bool Check(bool value, const char* name)
{
    std::cout << name << '=' << (value ? "PASS" : "FAIL") << '\n';
    return value;
}

class Source : public webrtc::AdaptedVideoTrackSource {
public:
    SourceState state() const override { return kLive; }
    bool remote() const override { return false; }
    bool is_screencast() const override { return true; }
    std::optional<bool> needs_denoising() const override { return false; }
    void Push(int index)
    {
        auto buffer = webrtc::I420Buffer::Create(kWidth, kHeight);
        for (int row = 0; row < kHeight; ++row) {
            std::memset(buffer->MutableDataY() + row * buffer->StrideY(),
                32 + (index * 3 + row) % 180, kWidth);
        }
        for (int row = 0; row < kHeight / 2; ++row) {
            std::memset(buffer->MutableDataU() + row * buffer->StrideU(),
                90 + index % 60, kWidth / 2);
            std::memset(buffer->MutableDataV() + row * buffer->StrideV(),
                160 - index % 60, kWidth / 2);
        }
        OnFrame(webrtc::VideoFrame::Builder().set_video_frame_buffer(buffer)
            .set_timestamp_us(webrtc::TimeMicros())
            .set_rotation(webrtc::kVideoRotation_0).build());
    }
};

class Sink final : public webrtc::VideoSinkInterface<webrtc::VideoFrame> {
public:
    void OnFrame(const webrtc::VideoFrame& frame) override
    {
        width.store(frame.width());
        height.store(frame.height());
        frames.fetch_add(1);
    }
    std::atomic<int> frames{0}, width{0}, height{0};
};

class Signaling final : public ISessionSignalingSender {
public:
    SessionControllerBase* peer = nullptr;
    bool SendDescription(const SessionDescription& description) override
    {
        if (!peer) return false;
        peer->HandleRemoteDescription(description);
        return true;
    }
    bool SendIceCandidate(const IceCandidate& candidate) override
    {
        if (!peer) return false;
        peer->HandleRemoteIceCandidate(candidate);
        return true;
    }
};

class Observer final : public ISessionControllerObserver {
public:
    explicit Observer(LibWebRtcSession* feedbackRecipient = nullptr)
        : recipient_(feedbackRecipient) {}

    void OnControllerSnapshot(const SessionControllerSnapshot& snapshot) override
    {
        std::lock_guard lock(mutex_);
        snapshot_ = snapshot;
        condition_.notify_all();
    }
    void OnDataChannelStateChanged(const DataChannelInfo& channel) override
    {
        if (channel.label != kTelemetryChannel) return;
        std::lock_guard lock(mutex_);
        telemetry_ = channel;
        condition_.notify_all();
    }
    void OnRemoteTrackAdded(const RemoteTrackInfo&) override {}
    void OnDataMessage(const std::string& label,
        std::span<const std::uint8_t> payload, bool binary) override
    {
        if (!recipient_ || label != kTelemetryChannel) return;
        ScreenReceiverFeedback feedback;
        const bool valid = binary &&
            DecodeScreenReceiverFeedback(payload, &feedback);
        // This callback stands in for the authenticated engine dispatch;
        // identities are deliberately fixed for this isolated local pair.
        const bool accepted = valid && feedback.roomId == "TEST/local-room" &&
            feedback.senderDeviceId == "TEST/viewer" &&
            recipient_->AcceptScreenReceiverFeedback(feedback);
        std::lock_guard lock(mutex_);
        ++messages_;
        allMetadata_ = allMetadata_ && valid &&
            payload.size() == kScreenReceiverFeedbackHeaderBytes +
                feedback.roomId.size() + feedback.senderDeviceId.size() &&
            payload.size() <= kMaximumScreenReceiverFeedbackMessageBytes;
        if (accepted) accepted_.push_back(feedback);
        else ++rejected_;
        condition_.notify_all();
    }

    bool WaitState(SessionControllerState expected)
    {
        std::unique_lock lock(mutex_);
        const bool ready = condition_.wait_for(lock, kOperationTimeout, [&] {
            return snapshot_.state == expected || !snapshot_.errorCode.empty();
        });
        if (!ready || snapshot_.state != expected) {
            std::cerr << "CONTROLLER_ERROR=" << snapshot_.errorCode << ':'
                << snapshot_.errorMessage << '\n';
            return false;
        }
        return true;
    }
    bool WaitTelemetry()
    {
        std::unique_lock lock(mutex_);
        return condition_.wait_for(lock, kOperationTimeout, [&] {
            return telemetry_ && telemetry_->state == DataChannelState::kOpen;
        });
    }
    bool TelemetryBestEffort()
    {
        std::lock_guard lock(mutex_);
        return telemetry_ && !telemetry_->ordered &&
            telemetry_->maxRetransmits == 0;
    }
    std::vector<ScreenReceiverFeedback> Accepted()
    {
        std::lock_guard lock(mutex_);
        return accepted_;
    }
    std::size_t Rejected()
    {
        std::lock_guard lock(mutex_);
        return rejected_;
    }
    bool WaitRejected(std::size_t count)
    {
        std::unique_lock lock(mutex_);
        return condition_.wait_for(lock, 3s, [&] { return rejected_ >= count; });
    }
    bool AllMetadata()
    {
        std::lock_guard lock(mutex_);
        return messages_ > 0 && allMetadata_;
    }
private:
    LibWebRtcSession* recipient_;
    std::mutex mutex_;
    std::condition_variable condition_;
    SessionControllerSnapshot snapshot_;
    std::optional<DataChannelInfo> telemetry_;
    std::vector<ScreenReceiverFeedback> accepted_;
    std::size_t messages_ = 0, rejected_ = 0;
    bool allMetadata_ = true;
};

bool Pump(Source& source, LibWebRtcSession& receiver,
    Observer& observer, std::size_t wanted, int& frameIndex)
{
    const auto end = std::chrono::steady_clock::now() + 12s;
    auto nextStats = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() < end) {
        source.Push(frameIndex++);
        const auto now = std::chrono::steady_clock::now();
        if (now >= nextStats) {
            receiver.RequestStats();
            nextStats = now + 1050ms;
        }
        if (observer.Accepted().size() >= wanted) return true;
        std::this_thread::sleep_for(1000ms / kFps);
    }
    const auto snapshot = receiver.StatsSnapshot();
    for (const auto& stream : snapshot.rtpStreams) {
        std::cerr << "RECEIVER_STATS=" << stream.slot << ',' << stream.kind
            << ",decoded=" << stream.framesDecoded
            << ",counters=" << stream.receiverFrameCountersAvailable
            << ",size=" << stream.frameWidth << 'x' << stream.frameHeight << '\n';
    }
    return false;
}

bool SendFixture(LibWebRtcSession& receiver, Observer& observer,
    const ScreenReceiverFeedback& report)
{
    std::vector<std::uint8_t> wire;
    if (!EncodeScreenReceiverFeedback(report, &wire)) return false;
    const auto rejected = observer.Rejected();
    return receiver.SendData(std::string(kTelemetryChannel), wire, true) ==
        SendResult::kSent && observer.WaitRejected(rejected + 1);
}

bool Run(webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory)
{
    Sink sink;
    LibWebRtcSession sender(factory), receiver(factory);
    Observer senderObserver(&sender), receiverObserver;
    Signaling senderSignaling, receiverSignaling;
    ControllerSessionController senderController(sender, senderSignaling);
    AgentSessionController receiverController(receiver, receiverSignaling);
    senderSignaling.peer = &receiverController;
    receiverSignaling.peer = &senderController;
    senderController.SetObserver(&senderObserver);
    receiverController.SetObserver(&receiverObserver);
    receiver.SetRemoteVideoSink(&sink);
    receiverController.SetAnswerPreparation([&]() -> std::optional<OperationError> {
        const auto result = receiver.BindNegotiatedVideoTransceiverSlots(
            {kScreenMainVideoSlot});
        if (result.ok()) return std::nullopt;
        return OperationError{"TEST_BIND_SCREEN", result.message()};
    });
    SessionControllerConfig config;
    config.webRtc.includeLoopbackAdapter = true;
    config.webRtc.fastDesktopBweStartup = false;
    senderController.Start(config);
    receiverController.Start(config);
    if (!Check(senderObserver.WaitState(SessionControllerState::kReady) &&
        receiverObserver.WaitState(SessionControllerState::kReady),
        "REAL_SESSIONS_READY")) return false;
    if (!Check(sender.PrepareVideoTransceiverSlot(kScreenMainVideoSlot).ok(),
        "REAL_SCREEN_TRANSCEIVER")) return false;
    senderController.Connect(DefaultRemoteControlDataChannels());
    if (!Check(senderObserver.WaitState(SessionControllerState::kConnected) &&
        receiverObserver.WaitState(SessionControllerState::kConnected),
        "LOCAL_ICE_CONNECTED")) return false;
    if (!Check(senderObserver.WaitTelemetry() && receiverObserver.WaitTelemetry() &&
        senderObserver.TelemetryBestEffort() && receiverObserver.TelemetryBestEffort(),
        "TELEMETRY_UNORDERED_ZERO_RETRANSMITS")) return false;

    auto source = webrtc::make_ref_counted<Source>();
    auto track = factory->CreateVideoTrack(source, "TEST/synthetic-screen");
    if (!Check(sender.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot,
        kFps, kWidth, kHeight).ok() &&
        sender.SetVideoSlotTrack(kScreenMainVideoSlot, track).ok() &&
        sender.SetVideoSlotSendingActive(kScreenMainVideoSlot, true).ok(),
        "REAL_VIDEO_SOURCE_ACTIVE")) return false;
    ScreenReceiverFeedback context;
    context.roomId = "TEST/local-room";
    context.senderDeviceId = "TEST/viewer";
    context.screenShareGeneration = 7;
    context.preferenceSequence = 23;
    sender.SetScreenSenderFeedbackContract(7, 23);
    receiver.SetScreenReceiverFeedbackContext(context);
    int frameIndex = 0;
    if (!Check(Pump(*source, receiver, senderObserver, 2, frameIndex),
        "REAL_STATS_TELEMETRY_ACCEPTED_TWO_WINDOWS")) return false;
    bool ok = Check(sink.frames.load() > 0 && sink.width.load() == kWidth &&
        sink.height.load() == kHeight, "ACTUAL_VIDEO_DECODED");
    const auto first = senderObserver.Accepted();
    ok &= Check(std::all_of(first.begin(), first.end(), [](const auto& feedback) {
        return feedback.screenShareGeneration == 7 && feedback.preferenceSequence == 23 &&
            feedback.decodedFrames > 0 && feedback.decodedFrames <= 100 &&
            feedback.frameWidth == kWidth && feedback.frameHeight == kHeight &&
            feedback.sampleWindowMs >= 900 && feedback.sampleWindowMs <= 3000 &&
            feedback.decodeTimeAvailable && feedback.processingTimeAvailable;
    }), "REAL_WINDOW_COUNTERS_DIMENSIONS_AND_TIMINGS");
    ok &= Check(first.back().sequence > first.front().sequence,
        "FEEDBACK_SEQUENCE_MONOTONIC");
    ok &= Check(SendFixture(receiver, senderObserver, first.back()),
        "TRANSPORT_REPLAY_REJECTED");
    auto old = first.back();
    old.sequence += 1000;
    old.screenShareGeneration = 6;
    ok &= Check(SendFixture(receiver, senderObserver, old),
        "TRANSPORT_OLD_GENERATION_REJECTED");
    old.screenShareGeneration = 7;
    old.preferenceSequence = 22;
    ok &= Check(SendFixture(receiver, senderObserver, old),
        "TRANSPORT_OLD_PREFERENCE_REJECTED");

    // Context can disappear while a preference request is in flight. Resume
    // must keep a higher sequence even if generation/preference did not change.
    receiver.SetScreenReceiverFeedbackContext({});
    std::this_thread::sleep_for(200ms);
    const auto pausedCount = senderObserver.Accepted().size();
    receiver.RequestStats();
    std::this_thread::sleep_for(1200ms);
    receiver.RequestStats();
    std::this_thread::sleep_for(200ms);
    ok &= Check(senderObserver.Accepted().size() == pausedCount,
        "PAUSED_CONTEXT_SENDS_NO_REPORTS");
    receiver.SetScreenReceiverFeedbackContext(context);
    const bool resumed = Pump(*source, receiver, senderObserver,
        pausedCount + 1, frameIndex);
    ok &= Check(resumed && senderObserver.Accepted().back().sequence >
        first.back().sequence, "SAME_CONTRACT_RESUME_ACCEPTS_HIGHER_SEQUENCE");
    if (!resumed) return false;

    const auto beforeRestart = senderObserver.Accepted();
    context.screenShareGeneration = 8;
    context.preferenceSequence = 0; // Initial preference of the new share.
    sender.SetScreenSenderFeedbackContract(8, 0);
    receiver.SetScreenReceiverFeedbackContext(context);
    const bool restarted = Pump(*source, receiver, senderObserver,
        beforeRestart.size() + 1, frameIndex);
    const auto final = senderObserver.Accepted();
    ok &= Check(restarted && final.back().screenShareGeneration == 8 &&
        final.back().preferenceSequence == 0 &&
        final.back().sequence > beforeRestart.back().sequence,
        "NEW_SHARE_DEFAULT_PREFERENCE_ACCEPTED");
    ok &= Check(senderObserver.AllMetadata(), "ALL_REPORTS_BOUNDED_METADATA_NO_IMAGES");
    std::cout << "DECODED_FRAMES=" << sink.frames.load() << '\n'
        << "ACCEPTED_REPORTS=" << final.size() << '\n';
    senderController.Close();
    receiverController.Close();
    ok &= Check(senderObserver.WaitState(SessionControllerState::kClosed) &&
        receiverObserver.WaitState(SessionControllerState::kClosed),
        "FORMAL_SESSIONS_CLOSED");
    return ok;
}
} // namespace
} // namespace remote

int main()
{
    remote::WebRtcRuntime runtime(remote::VideoEncoderPreference::kSoftwareOnly,
        remote::VideoDecoderPreference::kSoftwareOnly);
    if (!remote::Check(runtime.Initialize(), "WEBRTC_RUNTIME_INITIALIZED")) return 1;
    const bool ok = remote::Run(runtime.PeerConnectionFactory());
    runtime.Shutdown();
    std::cout << "SCREEN_RECEIVER_FEEDBACK_PIPELINE=" << (ok ? "PASS" : "FAIL") << '\n';
    return ok ? 0 : 1;
}
