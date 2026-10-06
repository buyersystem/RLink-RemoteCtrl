// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <atomic>
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <deque>
#include <iomanip>
#include <iostream>
#include <limits>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "api/make_ref_counted.h"
#include "api/video/adapted_video_track_source.h"
#include "api/video/i420_buffer.h"
#include "rtc_base/time_utils.h"
#include "rtc_base/logging.h"
#include "src/core/SessionController.h"
#include "src/core/ScreenStreamPolicy.h"
#include "src/core/ScreenRecoveryProbePolicy.h"
#include "src/protocol/DataChannelCatalog.h"
#include "src/webrtc/LibWebRtcSession.h"
#include "src/webrtc/WebRtcRuntime.h"

namespace remote {
namespace {
using namespace std::chrono_literals;
constexpr int kWidth = 320, kHeight = 180, kFps = 30;

bool Check(bool passed, const char* name)
{
    std::cout << name << '=' << (passed ? "PASS" : "FAIL") << std::endl;
    return passed;
}

// Sleep(1) can wake only every ~15.6 ms in a background Windows process.
// That both caps a synthetic source at ~64 FPS and turns the relay's 5-ms
// credit cap into a fictitious network bottleneck. Keep this test's clock
// independent of other applications' timer-resolution requests.
class HighResolutionTick final {
public:
    HighResolutionTick() : timer_(CreateWaitableTimerExW(nullptr, nullptr,
        CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_MODIFY_STATE | SYNCHRONIZE)) {}
    ~HighResolutionTick() { if (timer_) CloseHandle(timer_); }
    HighResolutionTick(const HighResolutionTick&) = delete;
    HighResolutionTick& operator=(const HighResolutionTick&) = delete;
    bool Valid() const { return timer_ != nullptr; }
    bool Wait() const
    {
        if (!timer_) return false;
        LARGE_INTEGER due;
        due.QuadPart = -10'000; // One millisecond, in 100-ns units.
        return SetWaitableTimerEx(timer_, &due, 0, nullptr, nullptr, nullptr, 0) &&
            WaitForSingleObject(timer_, INFINITE) == WAIT_OBJECT_0;
    }
private:
    HANDLE timer_ = nullptr;
};

class Source : public webrtc::AdaptedVideoTrackSource {
public:
    explicit Source(int width = kWidth, int height = kHeight, bool cacheFrames = false)
        : width_(width), height_(height)
    {
        // Prepare synthetic full-HD frames before timing starts. This keeps
        // CPU pattern generation out of the hardware-encoder FPS comparison.
        if (cacheFrames) for (int index = 0; index < 32; ++index) cached_.push_back(Generate(index));
    }
    SourceState state() const override { return kLive; }
    bool remote() const override { return false; }
    bool is_screencast() const override { return true; }
    std::optional<bool> needs_denoising() const override { return false; }
    void Push(int index)
    {
        auto buffer = cached_.empty() ? Generate(index) : cached_[index % cached_.size()];
        OnFrame(webrtc::VideoFrame::Builder().set_video_frame_buffer(buffer)
            .set_timestamp_us(webrtc::TimeMicros())
            .set_rotation(webrtc::kVideoRotation_0).build());
    }
private:
    webrtc::scoped_refptr<webrtc::I420Buffer> Generate(int index) const
    {
        auto buffer = webrtc::I420Buffer::Create(width_, height_);
        for (int y = 0; y < height_; ++y) {
            auto* row = buffer->MutableDataY() + y * buffer->StrideY();
            for (int x = 0; x < width_; ++x)
                row[x] = static_cast<std::uint8_t>(16 +
                    ((x * 11 + y * 7 + index * 13) % 220));
        }
        for (int y = 0; y < height_ / 2; ++y) {
            auto* u = buffer->MutableDataU() + y * buffer->StrideU();
            auto* v = buffer->MutableDataV() + y * buffer->StrideV();
            for (int x = 0; x < width_ / 2; ++x) {
                u[x] = static_cast<std::uint8_t>(96 + (x + index) % 64);
                v[x] = static_cast<std::uint8_t>(96 + (y + index) % 64);
            }
        }
        return buffer;
    }
    int width_;
    int height_;
    std::vector<webrtc::scoped_refptr<webrtc::I420Buffer>> cached_;
};

class Sink final : public webrtc::VideoSinkInterface<webrtc::VideoFrame> {
public:
    void OnFrame(const webrtc::VideoFrame&) override { ++frames; }
    std::atomic<int> frames{0};
};

// Do not send any ICE checks to another machine, even if the host has physical
// NICs or VPNs. No STUN/TURN servers or persisted user settings are involved.
bool IsLoopbackCandidate(const std::string& candidate)
{
    std::istringstream fields(candidate);
    std::string foundation, component, protocol, priority, address;
    fields >> foundation >> component >> protocol >> priority >> address;
    return address == "::1" || address.rfind("127.", 0) == 0;
}

// Test-only UDP relays keep real ICE, DTLS, RTP, RTCP and transport feedback.
// The limited direction has one shared token bucket and a bounded FIFO. It
// therefore creates actual queueing/loss at the packet transport, rather than
// injecting bandwidth estimates or encoder budgets. It never routes to a NIC.
class LoopbackShaper final {
public:
    LoopbackShaper() : thread_([this] { Pump(); }) {}
    ~LoopbackShaper()
    {
        stopping_.store(true);
        thread_.join();
        for (const auto& endpoint : endpoints_) closesocket(endpoint.socket);
    }
    void SetCapacity(std::uint32_t bitsPerSecond) { capacity_.store(bitsPerSecond); }
    std::uint64_t Dropped() const { return dropped_.load(); }
    std::uint64_t ForwardedBytes() const { return forwarded_.load(); }
    std::uint64_t PeakQueuedBytes() const { return peakQueued_.load(); }
    bool TimingHealthy() const { return tick_.Valid() && timingHealthy_.load(); }
    std::string Rewrite(const std::string& candidate, bool ownerIsSender)
    {
        std::istringstream fields(candidate);
        std::vector<std::string> words;
        for (std::string field; fields >> field;) words.push_back(std::move(field));
        if (words.size() < 8 || words[2] != "udp" || words[4] != "127.0.0.1") return {};
        const int originalPort = std::stoi(words[5]);
        std::lock_guard lock(mutex_);
        auto found = std::find_if(endpoints_.begin(), endpoints_.end(), [&](const auto& value) {
            return value.ownerIsSender == ownerIsSender && ntohs(value.target.sin_port) == originalPort;
        });
        if (found == endpoints_.end()) {
            const SOCKET socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (socket == INVALID_SOCKET) return {};
            sockaddr_in bound{};
            bound.sin_family = AF_INET;
            bound.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            if (bind(socket, reinterpret_cast<sockaddr*>(&bound), sizeof(bound)) != 0) {
                closesocket(socket); return {};
            }
            int size = sizeof(bound);
            getsockname(socket, reinterpret_cast<sockaddr*>(&bound), &size);
            u_long nonblocking = 1;
            ioctlsocket(socket, FIONBIO, &nonblocking);
            Endpoint endpoint;
            endpoint.socket = socket;
            endpoint.target = bound;
            endpoint.target.sin_port = htons(static_cast<u_short>(originalPort));
            endpoint.proxyPort = ntohs(bound.sin_port);
            endpoint.ownerIsSender = ownerIsSender;
            endpoints_.push_back(endpoint);
            found = std::prev(endpoints_.end());
        }
        words[5] = std::to_string(found->proxyPort);
        std::string result;
        for (const auto& word : words) {
            if (!result.empty()) result += ' ';
            result += word;
        }
        return result;
    }
private:
    using Clock = std::chrono::steady_clock;
    struct Endpoint {
        SOCKET socket = INVALID_SOCKET;
        sockaddr_in target{};
        sockaddr_in other{};
        bool hasOther = false;
        bool ownerIsSender = false;
        unsigned proxyPort = 0;
    };
    struct Datagram {
        SOCKET socket;
        sockaddr_in destination;
        std::vector<char> bytes;
        Clock::time_point ready;
    };
    void Deliver(const Datagram& datagram, bool shaped)
    {
        const int sent = sendto(datagram.socket, datagram.bytes.data(),
            static_cast<int>(datagram.bytes.size()), 0,
            reinterpret_cast<const sockaddr*>(&datagram.destination), sizeof(datagram.destination));
        if (sent > 0 && shaped) forwarded_.fetch_add(static_cast<std::uint64_t>(sent));
    }
    void Pump()
    {
        if (!tick_.Valid()) { timingHealthy_.store(false); return; }
        auto previous = Clock::now();
        double credit = 3000;
        std::size_t queuedBytes = 0;
        while (!stopping_.load()) {
            const auto now = Clock::now();
            const auto capacity = capacity_.load();
            // Retain five milliseconds of serialization credit to tolerate
            // host scheduling jitter without silently lowering a 40 Mbps
            // link to the throughput of a fixed 3 KB / pump-tick ceiling.
            const double burst = (std::max)(3000.0, capacity / 1600.0);
            credit = (std::min)(burst, credit +
                std::chrono::duration<double>(now - previous).count() * capacity / 8.0);
            previous = now;
            {
                std::lock_guard lock(mutex_);
                for (auto& endpoint : endpoints_) {
                    for (int packet = 0; packet < 128; ++packet) {
                        char bytes[65536];
                        sockaddr_in from{};
                        int fromSize = sizeof(from);
                        const int received = recvfrom(endpoint.socket, bytes, sizeof(bytes), 0,
                            reinterpret_cast<sockaddr*>(&from), &fromSize);
                        if (received <= 0) break;
                        const bool fromOwner = from.sin_addr.s_addr == endpoint.target.sin_addr.s_addr &&
                            from.sin_port == endpoint.target.sin_port;
                        if (!fromOwner) { endpoint.other = from; endpoint.hasOther = true; }
                        if (fromOwner && !endpoint.hasOther) continue;
                        const bool shaped = fromOwner ? endpoint.ownerIsSender : !endpoint.ownerIsSender;
                        Datagram datagram{endpoint.socket, fromOwner ? endpoint.other : endpoint.target,
                            std::vector<char>(bytes, bytes + received), now + 10ms};
                        if (shaped) {
                            // At most 250 ms of the current link capacity is queued.
                            const auto maximumQueue = (std::max)(std::size_t{3000},
                                static_cast<std::size_t>(capacity / 32));
                            if (queuedBytes + datagram.bytes.size() > maximumQueue) {
                                ++dropped_; continue;
                            }
                            queuedBytes += datagram.bytes.size();
                            auto peak = peakQueued_.load();
                            while (queuedBytes > peak && !peakQueued_.compare_exchange_weak(peak, queuedBytes)) {}
                            outgoing_.push_back(std::move(datagram));
                        } else reverse_.push_back(std::move(datagram));
                    }
                }
                while (!outgoing_.empty() && outgoing_.front().ready <= now &&
                    credit >= outgoing_.front().bytes.size()) {
                    const auto size = outgoing_.front().bytes.size();
                    Deliver(outgoing_.front(), true);
                    outgoing_.pop_front();
                    queuedBytes -= size;
                    credit -= size;
                }
                while (!reverse_.empty() && reverse_.front().ready <= now) {
                    Deliver(reverse_.front(), false);
                    reverse_.pop_front();
                }
            }
            if (!tick_.Wait()) { timingHealthy_.store(false); return; }
        }
    }
    std::mutex mutex_;
    std::vector<Endpoint> endpoints_;
    std::deque<Datagram> outgoing_, reverse_;
    std::atomic<std::uint32_t> capacity_{20'000'000};
    std::atomic<std::uint64_t> dropped_{0}, forwarded_{0}, peakQueued_{0};
    std::atomic<bool> stopping_{false};
    HighResolutionTick tick_;
    std::atomic<bool> timingHealthy_{true};
    std::thread thread_;
};

class Signaling final : public ISessionSignalingSender {
public:
    SessionControllerBase* peer = nullptr;
    LoopbackShaper* shaper = nullptr;
    bool ownerIsSender = false;
    bool SendDescription(const SessionDescription& description) override
    {
        if (!peer) return false;
        auto filtered = description;
        filtered.sdp.clear();
        std::istringstream lines(description.sdp);
        std::string line;
        while (std::getline(lines, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.rfind("a=candidate:", 0) == 0) {
                if (!IsLoopbackCandidate(line.substr(2))) continue;
                if (shaper) {
                    const auto rewritten = shaper->Rewrite(line.substr(2), ownerIsSender);
                    if (rewritten.empty()) continue;
                    line = "a=" + rewritten;
                }
            }
            filtered.sdp += line + "\r\n";
        }
        peer->HandleRemoteDescription(filtered);
        return true;
    }
    bool SendIceCandidate(const IceCandidate& candidate) override
    {
        if (!peer) return false;
        if (IsLoopbackCandidate(candidate.candidate)) {
            auto forwarded = candidate;
            if (shaper) forwarded.candidate = shaper->Rewrite(candidate.candidate, ownerIsSender);
            if (!forwarded.candidate.empty()) peer->HandleRemoteIceCandidate(forwarded);
        }
        return true;
    }
};

class Observer final : public ISessionControllerObserver {
public:
    void OnControllerSnapshot(const SessionControllerSnapshot& value) override
    {
        std::lock_guard lock(mutex_);
        snapshot_ = value;
        condition_.notify_all();
    }
    void OnDataChannelStateChanged(const DataChannelInfo&) override {}
    void OnDataMessage(const std::string&, std::span<const std::uint8_t>, bool) override {}
    void OnRemoteTrackAdded(const RemoteTrackInfo&) override {}
    bool Wait(SessionControllerState expected)
    {
        std::unique_lock lock(mutex_);
        const bool ready = condition_.wait_for(lock, 8s, [&] {
            return snapshot_.state == expected || !snapshot_.errorCode.empty();
        });
        if (!ready || snapshot_.state != expected) {
            std::cerr << "SESSION_ERROR=" << snapshot_.errorCode << ':'
                << snapshot_.errorMessage << std::endl;
            return false;
        }
        return true;
    }
private:
    std::mutex mutex_;
    std::condition_variable condition_;
    SessionControllerSnapshot snapshot_;
};

bool Run(webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory)
{
    Sink sink;
    LibWebRtcSession sender(factory), receiver(factory), idle(factory);
    Observer senderObserver, receiverObserver, idleObserver;
    Signaling senderSignaling, receiverSignaling, idleSignaling;
    ControllerSessionController senderController(sender, senderSignaling);
    AgentSessionController receiverController(receiver, receiverSignaling);
    ControllerSessionController idleController(idle, idleSignaling);
    senderSignaling.peer = &receiverController;
    receiverSignaling.peer = &senderController;
    senderController.SetObserver(&senderObserver);
    receiverController.SetObserver(&receiverObserver);
    idleController.SetObserver(&idleObserver);
    receiver.SetRemoteVideoSink(&sink);
    receiverController.SetAnswerPreparation([&]() -> std::optional<OperationError> {
        auto result = receiver.BindNegotiatedVideoTransceiverSlots({kScreenMainVideoSlot});
        if (result.ok()) return std::nullopt;
        return OperationError{"TEST_BIND_SCREEN", result.message()};
    });
    // Declared after the controllers, so cleanup also runs on every early exit.
    struct Cleanup {
        SessionControllerBase& sender;
        SessionControllerBase& receiver;
        SessionControllerBase& idle;
        ~Cleanup() { sender.Close(); receiver.Close(); idle.Close(); }
    } cleanup{senderController, receiverController, idleController};
    SessionControllerConfig config;
    config.webRtc.includeLoopbackAdapter = true;
    config.webRtc.fastDesktopBweStartup = false;
    config.webRtc.adaptiveDesktopNetworkFrameRate = true;
    config.negotiationTimeout = 8s;
    senderController.Start(config);
    receiverController.Start(config);
    idleController.Start(config);
    if (!Check(senderObserver.Wait(SessionControllerState::kReady) &&
        receiverObserver.Wait(SessionControllerState::kReady) &&
        idleObserver.Wait(SessionControllerState::kReady), "REAL_PC_FACTORY_READY")) return false;
    if (!Check(sender.PrepareVideoTransceiverSlot(kScreenMainVideoSlot).ok(),
        "REAL_SCREEN_SLOT_PREPARED")) return false;
    senderController.Connect(DefaultRemoteControlDataChannels());
    if (!Check(senderObserver.Wait(SessionControllerState::kConnected) &&
        receiverObserver.Wait(SessionControllerState::kConnected),
        "LOOPBACK_ICE_AND_DTLS_CONNECTED")) return false;
    const auto idleBefore = idle.StatsSnapshot().transport.googCc;
    auto source = webrtc::make_ref_counted<Source>();
    auto track = factory->CreateVideoTrack(source, "TEST/googcc-live-video");
    if (!Check(sender.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot,
        kFps, kWidth, kHeight).ok() &&
        sender.SetVideoSlotTrack(kScreenMainVideoSlot, track).ok() &&
        sender.SetVideoSlotSendingActive(kScreenMainVideoSlot, true).ok(),
        "REAL_VIDEO_SENDER_ENABLED")) return false;

    // No synthetic controller callbacks, target values or stats are injected.
    // The controller factory must be installed in the real Call and observe
    // actual RTP acknowledgements for this assertion to become true.
    const auto deadline = std::chrono::steady_clock::now() + 10s;
    GoogCcNetworkDiagnostics gcc;
    int frameIndex = 0;
    bool ready = false;
    while (std::chrono::steady_clock::now() < deadline) {
        source->Push(frameIndex++);
        if (frameIndex % 15 == 1) sender.RequestStats();
        const auto actual = sender.StatsSnapshot();
        gcc = actual.transport.googCc;
        ready = gcc.controllerObserved && gcc.targetRateBps > 0 &&
            gcc.effectiveTargetRateBps > 0 && gcc.feedbackFresh &&
            gcc.feedbackAtMs > 0 && gcc.delayObserved && sink.frames.load() >= 5 &&
            std::any_of(actual.rtpStreams.begin(), actual.rtpStreams.end(), [](const auto& stream) {
                return stream.direction == RtpStreamDirection::kOutbound && stream.kind == "video" &&
                    stream.slot == kScreenMainVideoSlot && stream.userVideoBitrateLimitBps != 0;
            });
        if (ready) break;
        std::this_thread::sleep_for(1000ms / kFps);
    }
    std::cout << "ACTUAL_GCC_TARGET_BPS=" << gcc.targetRateBps
        << "\nACTUAL_GCC_EFFECTIVE_BPS=" << gcc.effectiveTargetRateBps
        << "\nACTUAL_GCC_DELAY=" << gcc.delayState
        << "\nACTUAL_GCC_FEEDBACK_AGE_MS=" << gcc.feedbackAgeMs
        << "\nACTUAL_DECODED_FRAMES=" << sink.frames.load() << std::endl;
    bool ok = Check(ready, "REAL_GCC_TARGET_AND_PACKET_FEEDBACK_OBSERVED");
    const auto senderStats = sender.StatsSnapshot();
    const auto screen = std::find_if(senderStats.rtpStreams.begin(), senderStats.rtpStreams.end(),
        [](const auto& stream) { return stream.direction == RtpStreamDirection::kOutbound &&
            stream.kind == "video" && stream.slot == kScreenMainVideoSlot; });
    const auto expected = ResolveScreenStreamPolicy(kWidth, kHeight, {kWidth, kHeight, kFps});
    ok &= Check(screen != senderStats.rtpStreams.end() &&
        screen->userVideoBitrateBppHundredths == kDefaultScreenVideoBitrateBppHundredths &&
        screen->userVideoBitrateLimitBps == expected.maxBitrateBps,
        "REAL_SCREEN_DIAGNOSTICS_REPORT_USER_BPP_AND_ORIGINAL_SPECIFICATION_CEILING");
    ok &= Check(screen != senderStats.rtpStreams.end() && screen->screenQualityProtectionAvailable,
        "REAL_ENCODER_ENVIRONMENT_BINDS_ITS_OWN_QUALITY_CONTROL");
    const auto idleAfter = idle.StatsSnapshot().transport.googCc;
    ok &= Check(idleBefore.feedbackAtMs == 0 && idleAfter.feedbackAtMs == 0 &&
        !idleAfter.feedbackFresh && idleAfter.targetRateBps == idleBefore.targetRateBps &&
        idleAfter.routeRevision == idleBefore.routeRevision,
        "UNCONNECTED_PC_DOES_NOT_INHERIT_ACTIVE_PC_TELEMETRY");
    senderController.Close();
    receiverController.Close();
    idleController.Close();
    ok &= Check(senderObserver.Wait(SessionControllerState::kClosed) &&
        receiverObserver.Wait(SessionControllerState::kClosed) &&
        idleObserver.Wait(SessionControllerState::kClosed), "ALL_REAL_PC_SESSIONS_CLOSED");
    return ok;
}

struct RecoveryMeasurement {
    bool measured = false;
    int bweMs = -1, budgetMs = -1, fpsMs = -1;
};

bool RunShortOutage(webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory,
    bool protection, unsigned outageMilliseconds, const std::string& backend,
    const std::string& profile, RecoveryMeasurement& measurement, bool enableRecoveryHints)
{
    const bool fullHd = profile == "1080p80";
    const int width = fullHd ? 1920 : 640, height = fullHd ? 1080 : 360, fps = fullHd ? 80 : 120;
    const std::uint32_t bppHundredths = fullHd ? 15 : 29;
    const std::uint32_t normalCapacity = fullHd ? 40'000'000 : 20'000'000, limitedCapacity = 1'600'000;
    const auto userPolicy = ResolveScreenStreamPolicy(width, height,
        {static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height),
            static_cast<std::uint32_t>(fps), bppHundredths});
    const char* mode = protection ? "quality" : enableRecoveryHints ? "native" : "native_gcc";
    LoopbackShaper shaper;
    HighResolutionTick sourceTick;
    if (!Check(shaper.TimingHealthy() && sourceTick.Valid(),
        "OUTAGE_HIGH_RESOLUTION_SOURCE_AND_RELAY_TIMERS_READY")) return false;
    // Warm-up and restoration must use the SAME declared healthy capacity.
    // Otherwise the full-HD startup probes see 20 Mbps but recovery sees 40.
    shaper.SetCapacity(normalCapacity);
    Sink sink;
    LibWebRtcSession sender(factory), receiver(factory);
    Observer senderObserver, receiverObserver;
    Signaling senderSignaling, receiverSignaling;
    senderSignaling.shaper = receiverSignaling.shaper = &shaper;
    senderSignaling.ownerIsSender = true;
    ControllerSessionController senderController(sender, senderSignaling);
    AgentSessionController receiverController(receiver, receiverSignaling);
    senderSignaling.peer = &receiverController;
    receiverSignaling.peer = &senderController;
    senderController.SetObserver(&senderObserver);
    receiverController.SetObserver(&receiverObserver);
    receiver.SetRemoteVideoSink(&sink);
    receiverController.SetAnswerPreparation([&]() -> std::optional<OperationError> {
        auto result = receiver.BindNegotiatedVideoTransceiverSlots({kScreenMainVideoSlot});
        if (result.ok()) return std::nullopt;
        return OperationError{"TEST_BIND_SCREEN", result.message()};
    });
    struct Cleanup {
        SessionControllerBase& sender;
        SessionControllerBase& receiver;
        ~Cleanup() { sender.Close(); receiver.Close(); }
    } cleanup{senderController, receiverController};
    SessionControllerConfig config;
    config.webRtc.includeLoopbackAdapter = true;
    config.webRtc.fastDesktopBweStartup = false;
    config.webRtc.adaptiveDesktopNetworkFrameRate = protection;
    config.negotiationTimeout = 8s;
    senderController.Start(config);
    receiverController.Start(config);
    if (!Check(senderObserver.Wait(SessionControllerState::kReady) &&
        receiverObserver.Wait(SessionControllerState::kReady), "OUTAGE_REAL_PC_READY")) return false;
    sender.SetScreenVideoBitrateBpp(bppHundredths);
    sender.SetScreenQualityDeficitShare(kDefaultScreenQualityDeficitShareHundredths);
    if (!sender.PrepareVideoTransceiverSlot(kScreenMainVideoSlot).ok()) return false;
    senderController.Connect(DefaultRemoteControlDataChannels());
    if (!Check(senderObserver.Wait(SessionControllerState::kConnected) &&
        receiverObserver.Wait(SessionControllerState::kConnected), "OUTAGE_RELAY_ICE_CONNECTED")) return false;
    auto source = webrtc::make_ref_counted<Source>(width, height, fullHd);
    auto track = factory->CreateVideoTrack(source, std::string("TEST/short-outage/") + mode);
    if (!Check(sender.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot, fps, width, height).ok() &&
        sender.SetVideoSlotTrack(kScreenMainVideoSlot, track).ok() &&
        sender.SetVideoSlotSendingActive(kScreenMainVideoSlot, true).ok(),
        "OUTAGE_REAL_SENDER_ENABLED")) return false;

    // Neither mode resets the estimator, reapplies RTP parameters, changes FPS,
    // requests a keyframe, nor sets a synthetic network target during the test.
    std::cout << "SHORT_OUTAGE_BEGIN,mode=" << mode << ",backend=" << backend << ",outage_ms=" << outageMilliseconds
        << ",profile=" << profile << ",capture_fps=" << fps << ",video_cap_bps=" << userPolicy.maxBitrateBps
        << ",normal_link_bps=" << normalCapacity
        << ",recovery_hints=" << enableRecoveryHints
        << ",quality_share=" << kDefaultScreenQualityDeficitShareHundredths / 100.0
        << ",limited_link_bps=" << limitedCapacity << ",queue_ms=250,one_way_delay_ms=10" << std::endl;
    using Clock = std::chrono::steady_clock;
    const auto start = Clock::now();
    const auto limitStart = start + 20s;
    const auto restore = limitStart + std::chrono::milliseconds(outageMilliseconds);
    const auto finish = restore + 15s;
    auto nextFrame = start;
    auto nextStats = start;
    auto nextReport = start + 1s;
    bool limited = false, restored = false;
    unsigned frameIndex = 0;
    struct FrameSample { Clock::time_point at; std::uint32_t encoded, sent; };
    std::deque<FrameSample> frameSamples;
    double baselineAvailable = 0, baselineBudget = 0, baselineFps = 0;
    unsigned baselineSamples = 0;
    std::uint64_t minimumAvailable = (std::numeric_limits<std::uint64_t>::max)();
    std::uint64_t minimumBudget = (std::numeric_limits<std::uint64_t>::max)();
    int bweRecoveryMs = -1, budgetRecoveryMs = -1, fpsRecoveryMs = -1;
    unsigned bweStable = 0, budgetStable = 0, fpsStable = 0;
    unsigned actualSamples = 0;
    std::uint64_t warmBytes = 0, warmDropped = 0, limitedBytes = 0, limitedDropped = 0;
    std::uint64_t previousReportBytes = 0;
    bool fixedSpecification = true;
    bool coefficientCorrect = true;
    bool qualityFormulaObserved = false;
    std::uint32_t maximumRecoveryAttempts = 0, maximumRecoveryEpisodes = 0;
    bool requestedBackendObserved = false;
    double tailFps = 0;
    unsigned tailSamples = 0;
    bool sourceTimingHealthy = true;
    while (Clock::now() < finish) {
        const auto now = Clock::now();
        if (!limited && now >= limitStart) {
            warmBytes = shaper.ForwardedBytes(); warmDropped = shaper.Dropped();
            shaper.SetCapacity(limitedCapacity); limited = true;
        }
        if (!restored && now >= restore) {
            limitedBytes = shaper.ForwardedBytes() - warmBytes;
            limitedDropped = shaper.Dropped() - warmDropped;
            shaper.SetCapacity(normalCapacity); restored = true;
        }
        if (now >= nextFrame) {
            source->Push(static_cast<int>(frameIndex++));
            nextFrame += std::chrono::nanoseconds(1'000'000'000 / fps);
            // If the encoder stalls, do not flood historical captures later.
            if (nextFrame < now - 100ms) nextFrame = now;
        }
        if (now >= nextStats) {
            sender.RequestStats();
            nextStats += 100ms;
            const auto stats = sender.StatsSnapshot();
            const auto screen = std::find_if(stats.rtpStreams.begin(), stats.rtpStreams.end(), [](const auto& stream) {
                return stream.direction == RtpStreamDirection::kOutbound && stream.kind == "video" &&
                    stream.slot == kScreenMainVideoSlot;
            });
            if (screen != stats.rtpStreams.end() && stats.transport.googCc.feedbackFresh) {
                ++actualSamples;
                frameSamples.push_back({now, screen->framesEncoded, screen->framesSent});
                while (frameSamples.size() > 1 && frameSamples[1].at <= now - 1s) frameSamples.pop_front();
                const auto& previous = frameSamples.front();
                const auto elapsed = std::chrono::duration<double>(now - previous.at).count();
                const double encodedFps = elapsed > 0 && screen->framesEncoded >= previous.encoded ?
                    (screen->framesEncoded - previous.encoded) / elapsed : 0;
                const double sentFps = elapsed > 0 && screen->framesSent >= previous.sent ?
                    (screen->framesSent - previous.sent) / elapsed : 0;
                if (now >= finish - 3s) { tailFps += encodedFps; ++tailSamples; }
                fixedSpecification = fixedSpecification && screen->configuredMaxFrameRate == fps &&
                    screen->configuredMaxBitrateBps == userPolicy.maxBitrateBps &&
                    screen->configuredOutputWidth == width && screen->configuredOutputHeight == height;
                if (protection && screen->screenQualityProtectionActive &&
                    screen->screenQualityNetworkBudgetBps &&
                    screen->screenQualityReferenceBps > screen->screenQualityNetworkBudgetBps) {
                    qualityFormulaObserved = true;
                    const auto expected = screen->screenQualityReferenceBps -
                        (static_cast<std::uint64_t>(screen->screenQualityReferenceBps -
                            screen->screenQualityNetworkBudgetBps) * kDefaultScreenQualityDeficitShareHundredths + 50) / 100;
                    coefficientCorrect = coefficientCorrect &&
                        screen->screenQualityDeficitShareHundredths == kDefaultScreenQualityDeficitShareHundredths &&
                        screen->screenQualityEncoderReferenceBps == expected;
                }
                maximumRecoveryAttempts = (std::max)(maximumRecoveryAttempts, stats.transport.googCc.recoveryProbeAttempts);
                maximumRecoveryEpisodes = (std::max)(maximumRecoveryEpisodes, stats.transport.googCc.recoveryProbeEpisodes);
                requestedBackendObserved = requestedBackendObserved ||
                    (backend == "x264" && screen->encoderImplementation.find("x264") != std::string::npos) ||
                    (backend == "nvenc" && screen->encoderImplementation.find("NVENC") != std::string::npos) ||
                    (backend == "openh264" && screen->encoderImplementation.find("OpenH264") != std::string::npos);
                const auto budget = protection ? screen->screenQualityNetworkBudgetBps : screen->targetBitrateBps;
                if (!limited && now > start + 16s) {
                    baselineAvailable += static_cast<double>(stats.transport.availableOutgoingBitrateBps);
                    baselineBudget += static_cast<double>(budget);
                    baselineFps += encodedFps;
                    ++baselineSamples;
                }
                if (limited && !restored) {
                    if (stats.transport.availableOutgoingBitrateBps)
                        minimumAvailable = (std::min)(minimumAvailable, stats.transport.availableOutgoingBitrateBps);
                    if (budget) minimumBudget = (std::min)(minimumBudget, static_cast<std::uint64_t>(budget));
                }
                if (restored && baselineSamples) {
                    const int recovery = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(now - restore).count());
                    bweStable = stats.transport.availableOutgoingBitrateBps >= baselineAvailable / baselineSamples * .90 ? bweStable + 1 : 0;
                    budgetStable = budget >= baselineBudget / baselineSamples * .90 ? budgetStable + 1 : 0;
                    fpsStable = encodedFps >= baselineFps / baselineSamples * .90 ? fpsStable + 1 : 0;
                    if (bweRecoveryMs < 0 && bweStable >= 3) bweRecoveryMs = recovery;
                    if (budgetRecoveryMs < 0 && budgetStable >= 3) budgetRecoveryMs = recovery;
                    if (fpsRecoveryMs < 0 && fpsStable >= 3) fpsRecoveryMs = recovery;
                }
                if (now >= nextReport) {
                    const auto& gcc = stats.transport.googCc;
                    std::cout << "SHORT_OUTAGE_SAMPLE,mode=" << mode << ",t_ms="
                        << std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count()
                        << ",phase=" << (restored ? "recover" : limited ? "limited" : "warm")
                        << ",available_bps=" << stats.transport.availableOutgoingBitrateBps
                        << ",gcc_target_bps=" << gcc.targetRateBps
                        << ",gcc_effective_bps=" << gcc.effectiveTargetRateBps
                        << ",video_target_bps=" << screen->targetBitrateBps
                        << ",network_budget_bps=" << screen->screenQualityNetworkBudgetBps
                        << ",encoder_adjusted_bps=" << screen->screenQualityEncoderAdjustedBudgetBps
                        << ",bandwidth_allocation_bps=" << screen->screenQualityBandwidthAllocationBps
                        << ",nominal_encoder_bps=" << screen->screenQualityEncoderReferenceBps
                        << ",encoder=" << screen->encoderImplementation
                        << ",encoded_fps=" << std::fixed << std::setprecision(1) << encodedFps
                        << ",sent_fps=" << sentFps << ",sample_qp=" << screen->windowQp
                        << ",delay=" << gcc.delayState << ",cwnd_reduction=" << gcc.congestionWindowReduction
                        << ",application_limited=" << gcc.applicationLimited
                        << ",recovery_probe_active=" << gcc.recoveryProbeActive
                        << ",recovery_probe_remaining=" << gcc.recoveryProbeRemainingAttempts
                        << ",recovery_probe_history_bps=" << gcc.recoveryProbeHistoricalBudgetBps
                        << ",recovery_probe_attempts=" << gcc.recoveryProbeAttempts
                        << ",recovery_probe_episodes=" << gcc.recoveryProbeEpisodes
                        << ",shaper_window_bytes=" << shaper.ForwardedBytes() - previousReportBytes
                        << ",shaper_peak_queue_bytes=" << shaper.PeakQueuedBytes()
                        << ",shaper_dropped=" << shaper.Dropped() << std::endl;
                    previousReportBytes = shaper.ForwardedBytes();
                    nextReport += 1s;
                }
            }
        }
        if (!sourceTick.Wait()) { sourceTimingHealthy = false; break; }
    }
    std::cout << "SHORT_OUTAGE_RESULT,mode=" << mode << ",backend=" << backend << ",profile=" << profile
        << ",baseline_available_bps="
        << (baselineSamples ? baselineAvailable / baselineSamples : 0)
        << ",baseline_budget_bps=" << (baselineSamples ? baselineBudget / baselineSamples : 0)
        << ",baseline_fps=" << (baselineSamples ? baselineFps / baselineSamples : 0)
        << ",minimum_available_bps=" << minimumAvailable << ",minimum_budget_bps=" << minimumBudget
        << ",bwe_recovery_ms=" << bweRecoveryMs << ",budget_recovery_ms=" << budgetRecoveryMs
        << ",fps_recovery_ms=" << fpsRecoveryMs << ",captured_frames=" << frameIndex
        << ",decoded_frames=" << sink.frames.load() << ",forwarded_bytes=" << shaper.ForwardedBytes()
        << ",dropped_packets=" << shaper.Dropped()
        << ",warm_forwarded_bytes=" << warmBytes << ",warm_dropped_packets=" << warmDropped
        << ",limited_forwarded_bytes=" << limitedBytes << ",limited_dropped_packets=" << limitedDropped
        << ",recover_forwarded_bytes=" << shaper.ForwardedBytes() - warmBytes - limitedBytes
        << ",recover_dropped_packets=" << shaper.Dropped() - warmDropped - limitedDropped
        << ",peak_queue_bytes=" << shaper.PeakQueuedBytes()
        << ",maximum_recovery_probe_attempts=" << maximumRecoveryAttempts
        << ",maximum_recovery_probe_episodes=" << maximumRecoveryEpisodes
        << ",tail_steady_fps=" << (tailSamples ? tailFps / tailSamples : 0) << std::endl;
    measurement.measured = baselineSamples > 10;
    measurement.bweMs = bweRecoveryMs; measurement.budgetMs = budgetRecoveryMs; measurement.fpsMs = fpsRecoveryMs;
    bool ok = Check(actualSamples > 100 && baselineSamples > 10 && sink.frames.load() > 100 &&
        shaper.ForwardedBytes() > 1'000'000 && shaper.PeakQueuedBytes() > 3000 &&
        limitedBytes >= outageMilliseconds * (limitedCapacity / 8.0) / 1000.0 * .50 &&
        // Packets already serialized on the healthy link can arrive just after
        // the limit starts (the relay models propagation separately). Account
        // for its bounded 5-ms burst; sustained traffic still obeys the shaper.
        limitedBytes <= outageMilliseconds * (limitedCapacity / 8.0) / 1000.0 * 1.05 +
            normalCapacity / 1600.0,
        "SHORT_OUTAGE_REAL_PACKET_QUEUE_AND_FEEDBACK_EXERCISED");
    ok &= Check(sourceTimingHealthy && shaper.TimingHealthy(),
        "SHORT_OUTAGE_HIGH_RESOLUTION_TIMING_RETAINED");
    ok &= Check(fixedSpecification && coefficientCorrect && (!protection || qualityFormulaObserved) &&
        static_cast<double>(frameIndex) / std::chrono::duration<double>(finish - start).count() >= fps * .98,
        "SHORT_OUTAGE_USER_FPS_CAPTURE_CADENCE_AND_COEFFICIENT_PRESERVED");
    ok &= Check(baselineSamples && baselineFps / baselineSamples >= fps * .90,
        "SHORT_OUTAGE_BASELINE_REACHES_USER_FPS");
    ok &= Check(baselineSamples && minimumAvailable < baselineAvailable / baselineSamples * .70,
        "SHORT_OUTAGE_REAL_BANDWIDTH_ESTIMATE_DECREASED");
    ok &= Check(bweRecoveryMs >= 0 && budgetRecoveryMs >= 0 && fpsRecoveryMs >= 0,
        "SHORT_OUTAGE_BWE_BUDGET_AND_FPS_RECOVER_WITHOUT_USER_REAPPLY");
    ok &= Check(enableRecoveryHints ? maximumRecoveryEpisodes >= 1 &&
        maximumRecoveryAttempts <= ScreenRecoveryProbePolicy::kMaximumProbes :
        maximumRecoveryEpisodes == 0 && maximumRecoveryAttempts == 0,
        enableRecoveryHints ? "SHORT_OUTAGE_NATIVE_RECOVERY_PROBES_ARE_OBSERVED_AND_BOUNDED" :
        "REFERENCE_GCC_HAS_NO_RLINK_RECOVERY_INVITATIONS");
    ok &= Check(requestedBackendObserved, "SHORT_OUTAGE_REQUESTED_REAL_BACKEND_OBSERVED");
    ok &= Check(bweRecoveryMs >= 0 && bweRecoveryMs <= 5000 && budgetRecoveryMs >= 0 &&
        budgetRecoveryMs <= 5000 && fpsRecoveryMs >= 0 && fpsRecoveryMs <= 5000,
        "SHORT_OUTAGE_BWE_BUDGET_AND_FPS_RECOVER_WITHIN_FIVE_SECONDS");
    ok &= Check(fpsRecoveryMs >= 0 && bweRecoveryMs >= 0 && budgetRecoveryMs >= 0 &&
        fpsRecoveryMs <= (std::max)(bweRecoveryMs, budgetRecoveryMs) + 1500 &&
        tailSamples > 10 && tailFps / tailSamples >= fps * .95,
        "SHORT_OUTAGE_FPS_RECOVERY_FOLLOWS_REAL_BUDGET_AND_RETAINS_FULL_STEADY_CADENCE");
    return ok;
}
} // namespace
} // namespace remote

int main(int argc, char** argv)
{
    bool shortOutage = false;
    bool nativeGccReference = false;
    std::string onlyMode;
    std::string outageBackend = "x264";
    std::string outageProfile = "360p120";
    unsigned outageMilliseconds = 1500;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--short-outage") shortOutage = true;
        else if (argument == "--raw-gcc") nativeGccReference = true;
        else if (argument == "--native-info") {
            webrtc::LogMessage::LogToDebug(webrtc::LS_INFO);
            webrtc::LogMessage::SetLogToStderr(true);
        }
        else if (argument == "--short-outage=native") { shortOutage = true; onlyMode = "native"; }
        else if (argument == "--short-outage=quality") { shortOutage = true; onlyMode = "quality"; }
        else if (argument == "--outage-seconds=1") outageMilliseconds = 1000;
        else if (argument == "--outage-seconds=2") outageMilliseconds = 2000;
        else if (argument == "--outage-seconds=15") outageMilliseconds = 15000;
        else if (argument == "--outage-seconds=30") outageMilliseconds = 30000;
        else if (argument == "--outage-seconds=60") outageMilliseconds = 60000;
        else if (argument == "--outage-backend=x264") outageBackend = "x264";
        else if (argument == "--outage-backend=openh264") outageBackend = "openh264";
        else if (argument == "--outage-backend=nvenc") outageBackend = "nvenc";
        else if (argument == "--outage-profile=1080p80") outageProfile = "1080p80";
        else if (argument == "--outage-profile=360p120") outageProfile = "360p120";
        else { std::cerr << "UNKNOWN_ARGUMENT=" << argument << std::endl; return 1; }
    }
    if (nativeGccReference && (!shortOutage || onlyMode != "native")) {
        std::cerr << "--raw-gcc requires --short-outage=native" << std::endl;
        return 1;
    }
    const auto encoderPreference = !shortOutage || outageBackend == "openh264" ? remote::VideoEncoderPreference::kSoftwareOnly :
        outageBackend == "nvenc" ? remote::VideoEncoderPreference::kFfmpegHardware : remote::VideoEncoderPreference::kFfmpegX264Only;
    remote::WebRtcRuntime runtime(encoderPreference,
        remote::VideoDecoderPreference::kSoftwareOnly, {}, {}, std::nullopt,
        remote::kSystemDefaultMediaDeviceId, remote::kSystemDefaultMediaDeviceId,
        outageBackend == "nvenc" ? remote::FfmpegX264Preset::kMedium : remote::FfmpegX264Preset::kUltraFast,
        remote::FfmpegHardwareBackend::kNvenc);
    if (!remote::Check(runtime.Initialize(!nativeGccReference), "REAL_WEBRTC_RUNTIME_INITIALIZED")) return 1;
    bool ok = true;
    if (shortOutage) {
        std::cout << "SHORT_OUTAGE_CRITERIA,absolute_recovery_ms=5000,quality_bwe_budget_relative_ms=1000,"
            "quality_fps_vs_native_network_relative_ms=1000,own_fps_vs_network_relative_ms=1500,"
            "steady_tail_target_fraction=0.95,threshold_stable_samples=3" << std::endl;
        remote::RecoveryMeasurement native, quality;
        if (onlyMode != "quality") ok &= remote::RunShortOutage(runtime.PeerConnectionFactory(), false, outageMilliseconds, outageBackend, outageProfile, native, !nativeGccReference);
        if (onlyMode != "native") ok &= remote::RunShortOutage(runtime.PeerConnectionFactory(), true, outageMilliseconds, outageBackend, outageProfile, quality, !nativeGccReference);
        if (onlyMode.empty()) ok &= remote::Check(native.measured && quality.measured &&
            native.bweMs >= 0 && native.budgetMs >= 0 && native.fpsMs >= 0 &&
            quality.bweMs >= 0 && quality.budgetMs >= 0 && quality.fpsMs >= 0 &&
            quality.bweMs <= native.bweMs + 1000 && quality.budgetMs <= native.budgetMs + 1000 &&
            quality.fpsMs <= (std::max)(native.budgetMs, native.fpsMs) + 1000,
            "SHORT_OUTAGE_QUALITY_RECOVERY_MATCHES_NATIVE_NETWORK_RECOVERY_WITHIN_ONE_SECOND");
    } else ok = remote::Run(runtime.PeerConnectionFactory());
    runtime.Shutdown();
    remote::Check(ok, "GOOGCC_LIVE_TRANSPORT_SELF_TEST");
    return ok ? 0 : 1;
}
