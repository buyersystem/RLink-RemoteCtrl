// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include <algorithm>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>

#include "api/make_ref_counted.h"
#include "api/video/adapted_video_track_source.h"
#include "src/webrtc/GoogCcTelemetry.h"
#include "src/webrtc/LibWebRtcSession.h"
#include "src/webrtc/MediaSlotManager.h"
#include "src/webrtc/WebRtcRuntime.h"

namespace remote {
class SceneQualityCoefficientIntegrationTestAccess {
public:
    struct Contract {
        webrtc::RtpParameters parameters;
        std::uint32_t fps = 0, width = 0, height = 0;
        std::uint32_t effectiveFps = 0, effectiveWidth = 0, effectiveHeight = 0;
        std::uint64_t maxBps = 0, startBps = 0, probeBps = 0, effectiveBps = 0;
        std::uint32_t restarts = 0, allocationPulses = 0;
        std::pair<std::uint32_t, std::uint32_t> nativeQualityTarget;
        bool recoveryEnabled = false;
    };
    static Contract ReadContract(LibWebRtcSession& session)
    {
        Contract result;
        webrtc::scoped_refptr<webrtc::RtpSenderInterface> sender;
        {
            std::lock_guard lock(session.mutex_);
            const auto& binding = session.mediaSlots_->videoSlots_.at(kScreenMainVideoSlot);
            sender = binding.transceiver->sender();
            result.fps = binding.configuredMaxFrameRate;
            result.width = binding.configuredOutputWidth;
            result.height = binding.configuredOutputHeight;
            result.maxBps = binding.configuredMaxBitrateBps;
            result.startBps = binding.configuredStartBitrateBps;
            result.probeBps = binding.configuredNetworkProbeMaxBitrateBps;
            result.effectiveFps = binding.effectiveMaxFps;
            result.effectiveWidth = binding.effectiveWidth;
            result.effectiveHeight = binding.effectiveHeight;
            result.effectiveBps = binding.effectiveMaxBitrateBps;
            result.restarts = binding.mediaReadyBitrateRestarts;
            result.allocationPulses = binding.allocationProbePulses;
            result.nativeQualityTarget = session.googCcTelemetry_->ScreenQualityTarget();
            result.recoveryEnabled = session.googCcTelemetry_->ScreenRecoveryProbeEnabled();
        }
        result.parameters = sender->GetParameters();
        return result;
    }
    static SceneQualitySmoothingSnapshot Read(LibWebRtcSession& session, std::uint64_t now)
    {
        std::lock_guard lock(session.mutex_);
        return session.SceneQualitySnapshotLocked(now);
    }
    static std::uint64_t Tick(LibWebRtcSession& session)
    {
        std::lock_guard lock(session.mutex_);
        return session.sceneQualityLastTickMs_;
    }
    static bool SameContract(const Contract& left, const Contract& right)
    {
        if (left.fps != right.fps || left.width != right.width || left.height != right.height ||
            left.maxBps != right.maxBps || left.startBps != right.startBps || left.probeBps != right.probeBps ||
            left.effectiveFps != right.effectiveFps || left.effectiveWidth != right.effectiveWidth ||
            left.effectiveHeight != right.effectiveHeight || left.effectiveBps != right.effectiveBps ||
            left.restarts != right.restarts || left.allocationPulses != right.allocationPulses ||
            left.nativeQualityTarget != right.nativeQualityTarget || left.recoveryEnabled != right.recoveryEnabled ||
            left.parameters.encodings.size() != right.parameters.encodings.size()) return false;
        for (std::size_t i = 0; i < left.parameters.encodings.size(); ++i) {
            const auto& a = left.parameters.encodings[i];
            const auto& b = right.parameters.encodings[i];
            if (a.max_framerate != b.max_framerate || a.max_bitrate_bps != b.max_bitrate_bps ||
                a.min_bitrate_bps != b.min_bitrate_bps || a.scale_resolution_down_by != b.scale_resolution_down_by ||
                a.scale_resolution_down_to != b.scale_resolution_down_to ||
                a.active != b.active) return false;
        }
        return true;
    }
};

namespace {
using Access = SceneQualityCoefficientIntegrationTestAccess;
using Scene = media_intelligence::ScreenScene;
using namespace std::chrono_literals;

void Check(bool passed, const char* name)
{
    std::cout << name << '=' << (passed ? "PASS" : "FAIL") << std::endl;
    if (!passed) throw std::runtime_error(name);
}
bool Near(double actual, double expected) { return std::abs(actual - expected) <= 0.0051; }

class Source : public webrtc::AdaptedVideoTrackSource {
public:
    SourceState state() const override { return kLive; }
    bool remote() const override { return false; }
    bool is_screencast() const override { return true; }
    std::optional<bool> needs_denoising() const override { return false; }
};

class Observer final : public IWebRtcSessionObserver {
public:
    void OnSessionStateChanged(WebRtcSessionState state) override
    {
        std::lock_guard lock(mutex_);
        state_ = state;
        condition_.notify_all();
    }
    bool WaitReady()
    {
        std::unique_lock lock(mutex_);
        return condition_.wait_for(lock, 8s, [&] {
            return state_ == WebRtcSessionState::kReady || state_ == WebRtcSessionState::kFailed;
        }) && state_ == WebRtcSessionState::kReady;
    }
    void OnIceGatheringStateChanged(WebRtcIceGatheringState) override {}
    void OnLocalDescription(const SessionDescription&) override {}
    void OnLocalIceCandidate(const IceCandidate&) override {}
    void OnDataChannelStateChanged(const DataChannelInfo&) override {}
    void OnDataMessage(const std::string&, std::span<const std::uint8_t>, bool) override {}
    void OnRemoteTrackAdded(const RemoteTrackInfo&) override {}
    void OnOperationCompleted(OperationId) override {}
    void OnWebRtcError(OperationId, const OperationError& error) override
    { std::cerr << "SESSION_ERROR=" << error.code << ':' << error.message << '\n'; }
private:
    std::mutex mutex_;
    std::condition_variable condition_;
    WebRtcSessionState state_ = WebRtcSessionState::kNew;
};

void Run(webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory)
{
    Observer firstObserver, secondObserver;
    LibWebRtcSession first(factory), second(factory);
    first.SetObserver(&firstObserver);
    second.SetObserver(&secondObserver);
    first.Start({.adaptiveDesktopNetworkFrameRate = true});
    second.Start({.adaptiveDesktopNetworkFrameRate = true});
    Check(firstObserver.WaitReady() && secondObserver.WaitReady(), "REAL_PEER_CONNECTIONS_READY");
    auto source = webrtc::make_ref_counted<Source>();
    for (auto* session : {&first, &second}) {
        Check(session->PrepareVideoTransceiverSlot(kScreenMainVideoSlot).ok(), "REAL_SCREEN_SLOT_PREPARED");
        auto track = factory->CreateVideoTrack(source, session == &first ? "TEST/scene-first" : "TEST/scene-second");
        Check(track && session->SetVideoSlotTrack(kScreenMainVideoSlot, track).ok(), "REAL_SCREEN_TRACK_ATTACHED");
        Check(session->SetVideoSlotEncodingPolicy(kScreenMainVideoSlot, 60, 1920, 1080).ok(), "USER_1080P60_POLICY_APPLIED");
        Check(session->SetVideoSlotSendingActive(kScreenMainVideoSlot, true).ok(), "REAL_RTP_SENDER_ACTIVE");
    }
    const auto firstContract = Access::ReadContract(first);
    const auto secondContract = Access::ReadContract(second);
    Check(!firstContract.parameters.encodings.empty() && firstContract.fps == 60 &&
        firstContract.width == 1920 && firstContract.height == 1080 && firstContract.probeBps > 0,
        "REAL_RTP_AND_NATIVE_PROBE_CONTRACT_AVAILABLE");

    // Only the scene clock is virtual: actual PeerConnection/RtpSender and its
    // configured transport limits remain production objects. No packets leave
    // this test because no SDP negotiation, ICE gathering, or frame push runs.
    auto now = (std::max)(Access::Tick(first), GoogCcTelemetryState::NowMs()) + 60'000;
    first.UpdateSceneQualityCoefficient(now);
    const auto observe = [&](Scene scene, std::uint64_t stamp, bool enabled = true,
                             std::uint64_t generation = 1) {
        first.SetSceneQualityObservation({.enabled = enabled, .generation = generation,
            .observedAtMs = now, .sceneObservedAtMs = stamp,
            .maximumSceneAgeMs = 15'000, .scene = scene});
        first.UpdateSceneQualityCoefficient(now);
    };
    const auto invariant = [&] {
        Check(Access::SameContract(firstContract, Access::ReadContract(first)),
            "SCENE_CHANGES_PRESERVE_RTP_SPEC_BITRATE_PROBE_AND_RECOVERY_CONTRACT");
        Check(Access::SameContract(secondContract, Access::ReadContract(second)) &&
            Near(Access::Read(second, now).currentCoefficient, 0.50), "UNRELATED_SESSION_ISOLATED");
    };
    observe(Scene::kCodeTerminal, now);
    Check(Near(Access::Read(first, now).currentCoefficient, 0.50), "ENABLE_STARTS_AT_MANUAL_WITHOUT_JUMP");
    for (unsigned tick = 1; tick <= 10; ++tick) {
        now += 50;
        first.UpdateSceneQualityCoefficient(now);
    }
    Check(Near(Access::Read(first, now).currentCoefficient, 0.375), "FIFTY_MS_TICKS_FOLLOW_ONE_SECOND_RAMP");
    const auto beforeSwitch = Access::Read(first, now).currentCoefficient;
    observe(Scene::kGame3d, now);
    Check(Near(Access::Read(first, now).currentCoefficient, beforeSwitch), "MID_TRANSITION_SCENE_SWITCH_HAS_NO_JUMP");
    now += 400;
    first.UpdateSceneQualityCoefficient(now);
    Check(Near(Access::Read(first, now).currentCoefficient, 0.545), "NEW_SCENE_RAMP_STARTS_FROM_IN_FLIGHT_VALUE");
    observe(Scene::kGame3d, now);
    now += 600;
    first.UpdateSceneQualityCoefficient(now);
    Check(Near(Access::Read(first, now).currentCoefficient, 0.80) && !Access::Read(first, now).transitioning,
        "REPEATED_SCENE_DOES_NOT_RESTART_TRANSITION");
    invariant();

    observe(Scene::kUnknown, 0);
    now += 500;
    first.UpdateSceneQualityCoefficient(now);
    Check(Near(Access::Read(first, now).currentCoefficient, 0.80) && Access::Read(first, now).scene == "game_3d",
        "UNKNOWN_SHORT_RESULT_RETAINS_ACCEPTED_SCENE");
    first.SetScreenQualityDeficitShare(65);
    Check(Access::Read(first, now).manualCoefficientHundredths == 65 &&
        Near(Access::Read(first, now).currentCoefficient, 0.80), "MANUAL_CHANGE_STORED_WITHOUT_OVERRIDING_AUTO");
    now += 16'000;
    first.UpdateSceneQualityCoefficient(now);
    Check(Near(Access::Read(first, now).currentCoefficient, 0.80) &&
        Near(Access::Read(first, now).targetCoefficient, 0.65) && !Access::Read(first, now).observed,
        "EXPIRED_RESULT_STARTS_SMOOTH_FALLBACK_TO_MANUAL");
    now += 1000;
    first.UpdateSceneQualityCoefficient(now);
    Check(Near(Access::Read(first, now).currentCoefficient, 0.65), "EXPIRED_RESULT_REACHES_STORED_MANUAL");
    observe(Scene::kCodeTerminal, now);
    now += 1000;
    first.UpdateSceneQualityCoefficient(now);
    Check(Near(Access::Read(first, now).currentCoefficient, 0.25), "FRESH_SCENE_RESUMES_AUTOMATIC_COEFFICIENT");
    invariant();

    now += 1;
    observe(Scene::kUnknown, 0, true, 2);
    Check(!Access::Read(first, now).observed && Near(Access::Read(first, now).targetCoefficient, 0.65),
        "NEW_CAPTURE_GENERATION_CLEARS_PREVIOUS_SCENE");
    now += 1000;
    first.UpdateSceneQualityCoefficient(now);
    Check(Near(Access::Read(first, now).currentCoefficient, 0.65), "NEW_GENERATION_SMOOTHLY_USES_MANUAL_UNTIL_CLASSIFIED");
    const auto modelResultAt = now;
    observe(Scene::kGame3d, modelResultAt, true, 2);
    now += 10'000;
    observe(Scene::kGame3d, modelResultAt, true, 2);
    Check(Near(Access::Read(first, now).currentCoefficient, 0.80), "REPEATED_MODEL_RESULT_HELD_WHILE_STILL_FRESH");
    now += 5001;
    observe(Scene::kGame3d, modelResultAt, true, 2);
    Check(!Access::Read(first, now).observed && Near(Access::Read(first, now).targetCoefficient, 0.65),
        "REPEATED_STATS_READS_CANNOT_RENEW_OLD_MODEL_RESULT");
    now += 1000;
    observe(Scene::kCodeTerminal, now, true, 2);
    now += 1000;
    first.UpdateSceneQualityCoefficient(now);
    invariant();

    const auto acceptedNow = now;
    first.SetSceneQualityObservation({.enabled = true, .generation = 1,
        .observedAtMs = acceptedNow - 5000, .sceneObservedAtMs = acceptedNow - 5000,
        .maximumSceneAgeMs = 15'000, .scene = Scene::kGame3d});
    first.UpdateSceneQualityCoefficient(now);
    Check(Near(Access::Read(first, now).targetCoefficient, 0.25), "OLDER_CAPTURE_OBSERVATION_CANNOT_RESURRECT_SCENE");
    observe(Scene::kUnknown, 0, false);
    Check(!Access::Read(first, now).enabled && Near(Access::Read(first, now).currentCoefficient, 0.65),
        "DISABLING_AUTO_RESTORES_MANUAL_IMMEDIATELY");
    first.SetScreenQualityDeficitShare(40);
    Check(Near(Access::Read(first, now).currentCoefficient, 0.40), "DISABLED_AUTO_USES_LIVE_MANUAL_VALUE");
    invariant();
    first.Close();
    second.Close();
}
} // namespace
} // namespace remote

int main()
{
    try {
        remote::WebRtcRuntime runtime(remote::VideoEncoderPreference::kSoftwareOnly,
            remote::VideoDecoderPreference::kSoftwareOnly);
        remote::Check(runtime.Initialize(), "WEBRTC_RUNTIME_INITIALIZED");
        remote::Run(runtime.PeerConnectionFactory());
        std::cout << "SCENE_QUALITY_COEFFICIENT_INTEGRATION=PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "SCENE_QUALITY_COEFFICIENT_INTEGRATION=FAIL: " << error.what() << '\n';
        return 1;
    }
}
