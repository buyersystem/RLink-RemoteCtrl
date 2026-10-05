// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include <chrono>
#include <array>
#include <cstdint>
#include <future>
#include <iostream>
#include <thread>

#include "src/core/ScreenStreamPolicy.h"
#include "src/webrtc/LibWebRtcSession.h"
#include "src/webrtc/GoogCcTelemetry.h"
#include "src/webrtc/MediaSlotManager.h"
#include "src/webrtc/WebRtcRuntime.h"
#include "src/platform/win/WindowsDesktopCaptureSource.h"
#include "api/make_ref_counted.h"
#include "api/video/i420_buffer.h"
#include "api/video/video_sink_interface.h"
#include "media_intelligence/core/CalibratedStreamQualityModel.h"

namespace remote {

class DesktopCaptureDeliveryTestAccess {
public:
    static void Deliver(WindowsDesktopCaptureSource& source,
        webrtc::scoped_refptr<webrtc::VideoFrameBuffer> buffer,
        std::int64_t timestampUs, bool libWebRtc)
    {
        if (libWebRtc) {
            source.DeliverLibWebRtcFrame(std::move(buffer),
                WindowsDesktopCaptureSource::FrameDeliveryReason::kStartupPrime,
                {5, 7, 32, 48}, true, timestampUs);
        } else {
            source.DeliverFrame(std::move(buffer),
                WindowsDesktopCaptureSource::FrameDeliveryReason::kDesktopChanged,
                timestampUs);
        }
    }

    static bool CaptureReady(WindowsDesktopCaptureSource& source)
    {
        std::lock_guard lock(source.mutex_);
        return source.firstFrameReady_;
    }
};

// Access only the test seam. Parameter reads still go through the real
// libwebrtc sender, so successful shadow decisions cannot masquerade as writes.
class ContentAwareStreamExecutionTestAccess {
public:
    struct BindingSnapshot {
        std::uint32_t userWidth, userHeight, userFps;
        std::uint64_t userBitrate, revision;
        std::uint32_t effectiveWidth, effectiveHeight, effectiveFps;
        std::uint64_t effectiveBitrate;
        ContentPolicyExecutionSnapshot execution;
    };

    static BindingSnapshot Binding(LibWebRtcSession& session)
    {
        std::lock_guard lock(session.mutex_);
        const auto& slot = session.mediaSlots_->videoSlots_.at(kScreenMainVideoSlot);
        return {slot.configuredOutputWidth, slot.configuredOutputHeight,
            slot.configuredMaxFrameRate, slot.configuredMaxBitrateBps,
            slot.contentPolicyRevision, slot.effectiveWidth, slot.effectiveHeight,
            slot.effectiveMaxFps, slot.effectiveMaxBitrateBps, slot.contentPolicyExecution};
    }

    static webrtc::RtpParameters Parameters(LibWebRtcSession& session,
        const std::string& slot = kScreenMainVideoSlot)
    {
        webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver;
        {
            std::lock_guard lock(session.mutex_);
            transceiver = session.mediaSlots_->videoSlots_.at(slot).transceiver;
        }
        return transceiver->sender()->GetParameters();
    }

    static std::uint64_t ConnectionCeiling(LibWebRtcSession& session,
        const std::string& slot = kScreenMainVideoSlot)
    {
        std::lock_guard lock(session.mutex_);
        return session.mediaSlots_->videoSlots_.at(slot).configuredNetworkProbeMaxBitrateBps;
    }

    static std::uint64_t StartBitrate(LibWebRtcSession& session)
    {
        std::lock_guard lock(session.mutex_);
        return session.mediaSlots_->videoSlots_.at(kScreenMainVideoSlot).configuredStartBitrateBps;
    }

    static bool ApplyPendingBpp(LibWebRtcSession& session)
    {
        return session.ApplyPendingScreenVideoBitrateBpp();
    }

    static ScreenContentPolicyObservation CurrentObservation(LibWebRtcSession& session)
    {
        std::lock_guard lock(session.mutex_);
        return session.screenContentPolicyObservation_;
    }

    static std::array<std::uint64_t, 4> Bootstrap(LibWebRtcSession& session)
    {
        std::lock_guard lock(session.mutex_);
        const auto& slot = session.mediaSlots_->videoSlots_.at(kScreenMainVideoSlot);
        return {slot.bitrateBootstrapAttempts,
            slot.bitrateBootstrapSuccesses, slot.mediaReadyBitrateRestarts,
            slot.startBitrateBootstrapPending};
    }

    static ContentPolicyShadowSnapshot Shadow(LibWebRtcSession& session)
    {
        std::lock_guard lock(session.mutex_);
        return session.mediaSlots_->videoSlots_.at(kScreenMainVideoSlot).contentPolicyRecommendation;
    }

    static bool QualityVerified(LibWebRtcSession& session)
    {
        std::lock_guard lock(session.mutex_);
        return session.mediaSlots_->videoSlots_.at(kScreenMainVideoSlot).contentQualityVerified;
    }

    static std::uint64_t PendingSafeBudget(LibWebRtcSession& session)
    {
        std::lock_guard lock(session.mutex_);
        return session.mediaSlots_->videoSlots_.at(kScreenMainVideoSlot)
            .contentPolicyState.pendingSafeVideoBudgetBps;
    }

    static bool LegacyFrameRateControllerEnabled(LibWebRtcSession& session)
    {
        std::lock_guard lock(session.mutex_);
        return session.mediaSlots_->videoSlots_.at(kScreenMainVideoSlot).adaptiveFrameRate.enabled;
    }

    static bool Sample(LibWebRtcSession& session,
                       const WebRtcSessionStatsSnapshot& sample,
                       std::optional<std::uint64_t> epoch = std::nullopt)
    {
        auto current = sample;
        // These fixtures model a new controller/feedback observation for each
        // call. Keep the RTC window timestamp intact: repeated or incomplete
        // windows must still fail the production evidence-window checks.
        auto& gcc = current.transport.googCc;
        if (gcc.controllerObserved) {
            const auto now = static_cast<std::uint64_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count());
            gcc.targetUpdatedAtMs = gcc.feedbackAtMs = now;
            if (gcc.delayObserved) gcc.delayUpdatedAtMs = now;
            gcc.targetRateBps = current.transport.availableOutgoingBitrateBps;
            gcc.effectiveTargetRateBps = gcc.targetRateBps;
            gcc.feedbackFresh = true;
        }
        return session.HandleContentAwareStreamSample(current, epoch);
    }

    static std::uint64_t Epoch(LibWebRtcSession& session)
    {
        std::lock_guard lock(session.mutex_);
        return session.screenContentPolicyEpoch_;
    }

    static bool NativeQualityProtectionEnabled(LibWebRtcSession& session)
    {
        std::lock_guard lock(session.mutex_);
        return session.googCcTelemetry_->ScreenQualityProtectionEnabled();
    }

    static std::pair<std::uint32_t, std::uint32_t> NativeQualityTarget(LibWebRtcSession& session)
    {
        std::lock_guard lock(session.mutex_);
        return session.googCcTelemetry_->ScreenQualityTarget();
    }

    static std::uint32_t NativeQualityDeficitShare(LibWebRtcSession& session)
    {
        std::lock_guard lock(session.mutex_);
        return session.googCcTelemetry_->ScreenQualityDeficitShare();
    }

    static void AdvanceEvidenceWindowForTest(LibWebRtcSession& session, std::uint64_t floor)
    {
        std::lock_guard lock(session.mutex_);
        session.mediaSlots_->videoSlots_.at(kScreenMainVideoSlot).contentPolicyEvidenceNotBeforeMs = floor;
    }

    static std::uint64_t ReceiverEpoch(LibWebRtcSession& session)
    {
        std::lock_guard lock(session.mutex_);
        return session.receiverFeedbackEpoch_;
    }

    static bool RejectOldReceiverEpoch(LibWebRtcSession& session,
        const WebRtcSessionStatsSnapshot& sample, std::uint64_t oldEpoch)
    {
        session.SendScreenReceiverFeedback(sample, oldEpoch);
        std::lock_guard lock(session.mutex_);
        return session.receiverFeedbackNextSequence_ == 0 && session.receiverFeedbackLastSampleMs_ == 0;
    }

    static webrtc::RTCError SetMinimumBitrate(LibWebRtcSession& session,
                                             std::optional<int> minimum)
    {
        webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver;
        {
            std::lock_guard lock(session.mutex_);
            transceiver = session.mediaSlots_->videoSlots_.at(kScreenMainVideoSlot).transceiver;
        }
        auto parameters = transceiver->sender()->GetParameters();
        for (auto& encoding : parameters.encodings) encoding.min_bitrate_bps = minimum;
        return transceiver->sender()->SetParameters(parameters);
    }

    static webrtc::RTCError Apply(LibWebRtcSession& session,
        const media_intelligence::ContentAwareStreamInput& input,
        const media_intelligence::ContentAwareStreamEvaluation& evaluation,
        std::uint64_t revision)
    {
        webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver;
        {
            std::lock_guard lock(session.mutex_);
            transceiver = session.mediaSlots_->videoSlots_.at(kScreenMainVideoSlot).transceiver;
        }
        return session.ApplyContentAwareStreamDecision(input, evaluation, revision, transceiver);
    }

    static void HoldSenderLock(LibWebRtcSession& session, std::promise<void>& acquired,
                               std::future<void> release)
    {
        std::lock_guard lock(session.videoSenderParametersMutex_);
        acquired.set_value();
        // A regression to blocking locks must fail the timing assertion, not
        // hang the entire test forever. This thread never calls a WebRTC proxy.
        (void)release.wait_for(std::chrono::seconds(3));
    }


};

}  // namespace remote

namespace {
using namespace remote;
using namespace remote::media_intelligence;
using Access = ContentAwareStreamExecutionTestAccess;
using namespace std::chrono;

std::uint64_t Now()
{
    return static_cast<std::uint64_t>(duration_cast<milliseconds>(
        steady_clock::now().time_since_epoch()).count());
}

bool Check(bool passed, const char* name)
{
    std::cout << name << '=' << (passed ? "PASS" : "FAIL") << '\n';
    return passed;
}

bool RunDesktopSinkFrameRateDelivery(bool libWebRtc)
{
    class NativeFrame : public webrtc::VideoFrameBuffer {
    public:
        Type type() const override { return Type::kNative; }
        int width() const override { return 1920; }
        int height() const override { return 1080; }
        webrtc::scoped_refptr<webrtc::I420BufferInterface> ToI420() override
        {
            ++cpuMappings;
            return nullptr;
        }
        unsigned cpuMappings = 0;
    };
    class Sink final : public webrtc::VideoSinkInterface<webrtc::VideoFrame> {
    public:
        void OnFrame(const webrtc::VideoFrame& frame) override
        {
            ++frames;
            nativeUnchanged &= frame.video_frame_buffer()->type() ==
                webrtc::VideoFrameBuffer::Type::kNative && frame.width() == 1920 &&
                frame.height() == 1080;
            if (discarded != discardedAtLastDelivery) {
                const auto region = frame.update_rect();
                fullUpdateAfterDrop &= region.offset_x == 0 && region.offset_y == 0 &&
                    region.width == 1920 && region.height == 1080 && !frame.is_repeat_frame();
            }
            discardedAtLastDelivery = discarded;
        }
        void OnDiscardedFrame() override { ++discarded; }
        void OnConstraintsChanged(const webrtc::VideoTrackSourceConstraints& constraints) override
        {
            ++constraintsUpdates;
            maximumConstraintFps = constraints.max_fps;
        }
        unsigned frames = 0, discarded = 0, discardedAtLastDelivery = 0;
        unsigned constraintsUpdates = 0;
        std::optional<double> maximumConstraintFps;
        bool nativeUnchanged = true, fullUpdateAfterDrop = true;
    };
    auto source = webrtc::make_ref_counted<WindowsDesktopCaptureSource>(libWebRtc ?
        DesktopCaptureImplementation::kLibWebRtc : DesktopCaptureImplementation::kNativeDxgi);
    auto buffer = webrtc::make_ref_counted<NativeFrame>();
    auto* sourceInterface = static_cast<webrtc::VideoSourceInterface<webrtc::VideoFrame>*>(source.get());
    webrtc::VideoTrackSourceInterface::Stats sourceStats;
    bool statsBeforeFirstFrame = source->GetStats(&sourceStats);
    std::int64_t timestampUs = 1'000'000'000;
    const auto push = [&](unsigned frames) {
        for (unsigned i = 0; i < frames; ++i) {
            DesktopCaptureDeliveryTestAccess::Deliver(*source, buffer, timestampUs, libWebRtc);
            timestampUs += 16'667; // Deterministic 60-FPS input; no desktop capture.
        }
    };
    // No sink must not block capture initialization or consume startup callbacks.
    unsigned firstCallback = 0, completeCallback = 0;
    if (libWebRtc) source->RequestStartupFrameBurst(6,
        [&] { ++firstCallback; }, [&] { ++completeCallback; });
    push(1);
    bool ok = Check(DesktopCaptureDeliveryTestAccess::CaptureReady(*source) &&
        firstCallback == 0 && completeCallback == 0,
        "FPS_DELIVERY_WITHOUT_SINK_PRESERVES_CAPTURE_READINESS_AND_STARTUP_CALLBACKS");
    ok &= Check(!statsBeforeFirstFrame && source->GetStats(&sourceStats) &&
        sourceStats.input_width == 1920 && sourceStats.input_height == 1080,
        "PER_SINK_SOURCE_PRESERVES_INPUT_SIZE_STATS_WITHOUT_NATIVE_MAPPING");
    Sink slow, fast;
    source->ProcessConstraints({.min_fps = 1.0, .max_fps = 37.0});
    webrtc::VideoSinkWants wants;
    wants.max_framerate_fps = 20;
    wants.requested_resolution = webrtc::VideoSinkWants::FrameSize{1760, 990};
    sourceInterface->AddOrUpdateSink(&slow, wants);
    ok &= Check(slow.constraintsUpdates == 1 && slow.maximumConstraintFps == 37.0,
        "SOURCE_STORED_CONSTRAINTS_REACH_NEW_PROXY_SINK");
    source->ProcessConstraints({.max_fps = 40.0});
    ok &= Check(slow.constraintsUpdates == 2 && slow.maximumConstraintFps == 40.0,
        "SOURCE_UPDATED_CONSTRAINTS_REACH_EXISTING_PROXY_SINK");
    push(120);
    ok &= Check(slow.frames >= 39 && slow.frames <= 41 && slow.discarded >= 79 &&
        source->TargetFrameRate() == 60 && buffer->cpuMappings == 0 && slow.nativeUnchanged,
        libWebRtc ? "LIBWEBRTC_CAPTURE_SINK_20_FPS_LIMIT_REDUCES_REAL_DELIVERY_WITHOUT_CPU_MAPPING"
                  : "DXGI_CAPTURE_SINK_20_FPS_LIMIT_REDUCES_REAL_DELIVERY_WITHOUT_CPU_MAPPING");
    if (libWebRtc) ok &= Check(firstCallback == 1 && completeCallback == 1,
        "FPS_LIMITED_STARTUP_BURST_FINISHES_ON_ACCEPTED_FRAMES_ONLY");
    if (libWebRtc) ok &= Check(slow.fullUpdateAfterDrop,
        "DROPPED_PARTIAL_DESKTOP_UPDATE_REACHES_NEXT_DELIVERED_FRAME_AS_FULL_NONREPEAT_UPDATE");
    const auto beforeRecovery = slow.frames;
    wants.max_framerate_fps = 60;
    sourceInterface->AddOrUpdateSink(&slow, wants);
    push(120);
    ok &= Check(slow.frames - beforeRecovery >= 119 && slow.frames - beforeRecovery <= 121,
        "DYNAMIC_SINK_FPS_RECOVERY_RESTORES_REAL_SOURCE_DELIVERY");
    wants.max_framerate_fps = 20;
    sourceInterface->AddOrUpdateSink(&slow, wants);
    auto fastWants = wants;
    fastWants.max_framerate_fps = 60;
    sourceInterface->AddOrUpdateSink(&fast, fastWants);
    const auto slowBeforeShared = slow.frames;
    push(120);
    ok &= Check(fast.frames >= 119 && fast.frames <= 121 &&
        slow.frames - slowBeforeShared >= 39 && slow.frames - slowBeforeShared <= 41,
        "SHARED_CAPTURE_PRESERVES_HIGHEST_ACTIVE_SINK_FPS");
    ok &= Check(fast.constraintsUpdates == 1 && fast.maximumConstraintFps == 40.0,
        "SECOND_SINK_RECEIVES_LAST_SOURCE_CONSTRAINTS");
    const auto slowBeforeThirty = slow.frames, fastBeforeThirty = fast.frames;
    wants.max_framerate_fps = 30;
    sourceInterface->AddOrUpdateSink(&slow, wants);
    push(120);
    ok &= Check(slow.frames - slowBeforeThirty >= 59 && slow.frames - slowBeforeThirty <= 61 &&
        fast.frames - fastBeforeThirty >= 119 && fast.frames - fastBeforeThirty <= 121 &&
        buffer->cpuMappings == 0,
        "SLOW_PEER_DYNAMIC_20_TO_30_FPS_PRESERVES_OTHER_PEER_60_FPS");
    const auto slowBeforeZero = slow.frames, fastBeforeZero = fast.frames;
    wants.max_framerate_fps = 0;
    sourceInterface->AddOrUpdateSink(&slow, wants);
    push(120);
    ok &= Check(slow.frames == slowBeforeZero && fast.frames - fastBeforeZero >= 119 &&
        fast.frames - fastBeforeZero <= 121,
        "ZERO_FPS_SINK_NEVER_RECEIVES_FRAMES_OR_THROTTLES_FAST_PEER");
    wants.max_framerate_fps = 20;
    sourceInterface->AddOrUpdateSink(&slow, wants);
    sourceInterface->RemoveSink(&fast);
    const auto beforeRemoval = slow.frames;
    push(120);
    ok &= Check(slow.frames - beforeRemoval >= 39 && slow.frames - beforeRemoval <= 41 &&
        buffer->cpuMappings == 0 && slow.nativeUnchanged && source->TargetFrameRate() == 60,
        "REMOVING_FAST_SINK_REAPPLIES_LOW_FPS_WITHOUT_CHANGING_USER_CAPTURE_RATE");
    sourceInterface->RemoveSink(&slow);
    const auto afterRemoval = slow.frames + fast.frames;
    push(10);
    ok &= Check(slow.frames + fast.frames == afterRemoval &&
        buffer->cpuMappings == 0 && slow.fullUpdateAfterDrop,
        "REMOVED_SINKS_STOP_RECEIVING_AND_PER_PEER_DROPS_PRESERVE_FULL_UPDATE");
    sourceInterface->AddOrUpdateSink(&slow, wants);
    source = nullptr;
    ok &= Check(slow.frames + fast.frames == afterRemoval,
        "SOURCE_DESTRUCTION_CLEANS_REMAINING_PROXY_WITHOUT_FRAME_CALLBACKS");
    return ok;
}

bool RunDesktopJitterDelivery(bool libWebRtc, bool requestedResolution = true)
{
    class Sink final : public webrtc::VideoSinkInterface<webrtc::VideoFrame> {
    public:
        void OnFrame(const webrtc::VideoFrame&) override { ++frames; }
        unsigned frames = 0;
    };
    bool ok = true;
    for (const unsigned fps : {30u, 60u, 80u, 100u, 120u}) {
        auto source = webrtc::make_ref_counted<WindowsDesktopCaptureSource>(libWebRtc ?
            DesktopCaptureImplementation::kLibWebRtc : DesktopCaptureImplementation::kNativeDxgi);
        source->SetTargetFrameRate(fps);
        auto buffer = webrtc::I420Buffer::Create(16, 16);
        Sink fast, slow;
        webrtc::VideoSinkWants fastWants, slowWants;
        fastWants.max_framerate_fps = fps;
        slowWants.max_framerate_fps = 20;
        if (requestedResolution) {
            fastWants.requested_resolution = webrtc::VideoSinkWants::FrameSize{16, 16};
            slowWants.requested_resolution = fastWants.requested_resolution;
        }
        source->AddOrUpdateSink(&fast, fastWants);
        source->AddOrUpdateSink(&slow, slowWants);
        constexpr std::int64_t base = 1'000'000'000;
        const auto push = [&](unsigned start, unsigned count) {
            for (unsigned i = start; i < start + count; ++i) {
                // Windows scheduling can land just before or after a frame
                // slot. Mean capture cadence remains exactly the user FPS.
                const std::int64_t jitter = i % 3 == 1 ? -200 : i % 3 == 2 ? 200 : 0;
                const auto time = base + static_cast<std::int64_t>(i) * 1'000'000 / fps + jitter;
                DesktopCaptureDeliveryTestAccess::Deliver(*source, buffer, time, libWebRtc);
            }
        };
        push(0, fps * 10);
        std::cout << "JITTER_DELIVERY_BACKEND=" << (libWebRtc ? "LIBWEBRTC" : "DXGI")
            << ",resolution_wants=" << requestedResolution
            << ",target=" << fps << ",delivered=" << fast.frames / 10.0
            << ",slow=" << slow.frames / 10.0 << '\n';
        ok &= Check(fast.frames >= fps * 10 - 2 && fast.frames <= fps * 10,
            "MATCHED_USER_FPS_WITH_SCHEDULING_JITTER_HAS_NO_SPURIOUS_SOURCE_DROPS");
        ok &= Check(slow.frames >= 199 && slow.frames <= 201,
            "JITTER_TOLERANCE_PRESERVES_SHARED_SLOW_PEER_CAP");
        fastWants.max_framerate_fps = 20;
        source->AddOrUpdateSink(&fast, fastWants);
        const auto lowBefore = fast.frames;
        push(fps * 10, fps * 5);
        ok &= Check(fast.frames - lowBefore >= 98 && fast.frames - lowBefore <= 102,
            "JITTERED_DYNAMIC_LOW_FPS_LIMIT_REMAINS_ENFORCED");
        fastWants.max_framerate_fps = fps;
        source->AddOrUpdateSink(&fast, fastWants);
        const auto restoredBefore = fast.frames;
        push(fps * 15, fps * 5);
        ok &= Check(fast.frames - restoredBefore >= fps * 5 - 3 && fast.frames - restoredBefore <= fps * 5,
            "JITTERED_SOURCE_RECOVERS_TO_EXACT_USER_FPS_AFTER_CAP_RESTORE");
        source->RemoveSink(&fast);
        source->RemoveSink(&slow);
    }
    return ok;
}

// Deliberately simple TEST-only calibration; these numbers are not production
// encoder quality measurements and must never be registered by the application.
StreamQualityEstimate TestModel(const StreamQualityRequest& request,
                               const void*) noexcept
{
    return {static_cast<std::uint64_t>(request.width) * request.height *
        request.frameRate / 10, true, true, true};
}

ScreenContentPolicyObservation Observation(std::uint64_t generation = 1,
    ScreenScene scene = ScreenScene::kCodeTerminal)
{
    return {.enabled = true, .generation = generation, .observedAtMs = Now(),
        .sourceWidth = 1920, .sourceHeight = 1080, .scene = scene,
        .activity = ScreenContentActivity::kActive,
        .qualityAvailable = true, .qualityAcceptable = true,
        .processingAvailable = true, .processingHealthy = true};
}

WebRtcSessionStatsSnapshot Sample(std::uint64_t capacity = 13'000'000)
{
    WebRtcSessionStatsSnapshot sample;
    sample.transport.collected = true;
    sample.transport.receivedAtSteadyMs = Now();
    sample.transport.availableOutgoingBitrateBps = capacity;
    sample.transport.googCc.controllerObserved = true;
    sample.transport.googCc.delayObserved = true;
    sample.transport.googCc.feedbackFresh = true;
    sample.transport.googCc.delayState = "overuse";
    sample.transport.googCc.routeRevision = 1;
    sample.transport.googCc.targetUpdatedAtMs = sample.transport.googCc.feedbackAtMs =
        sample.transport.googCc.delayUpdatedAtMs = Now();
    sample.transport.googCc.targetRateBps = sample.transport.googCc.effectiveTargetRateBps = capacity;
    RtpStreamStatsSnapshot video;
    video.kind = "video";
    video.slot = kScreenMainVideoSlot;
    video.sampleWindowMs = 1000;
    video.frameWidth = 1920;
    video.frameHeight = 1080;
    video.encodedFramesPerSecond = 60;
    video.sentFramesPerSecond = 30;
    video.qualityLimitationReason = "none"; // GoogCC evidence, not this label, authorizes repair.
    sample.rtpStreams.push_back(video);
    return sample;
}

bool Matches(const webrtc::RtpParameters& parameters, std::uint32_t width,
             std::uint32_t height, std::uint32_t fps, std::uint64_t bitrate)
{
    if (parameters.encodings.empty()) return false;
    for (const auto& encoding : parameters.encodings) {
        if (encoding.max_framerate != static_cast<double>(fps) ||
            encoding.max_bitrate_bps != static_cast<int>(bitrate) ||
            !encoding.scale_resolution_down_to ||
            encoding.scale_resolution_down_to->width != static_cast<int>(width) ||
            encoding.scale_resolution_down_to->height != static_cast<int>(height)) return false;
    }
    return true;
}

bool RunConnectionReferenceBpp(LibWebRtcSession& session)
{
    bool ok = true;
    std::uint32_t selectedBpp = 10;
    std::uint32_t providerReads = 0;
    session.SetScreenVideoBitrateBppProvider([&] {
        ++providerReads;
        // Verify the host callback can inspect session state without being
        // invoked under its mutex. It runs only when applying a user policy.
        (void)Access::ConnectionCeiling(session);
        return selectedBpp;
    });
    session.Start({});
    if (!Check(session.PrepareVideoTransceiverSlot(kScreenMainVideoSlot).ok() &&
        session.PrepareVideoTransceiverSlot(kCameraMainVideoSlot).ok(),
        "BPP_REAL_SCREEN_AND_CAMERA_TRANSCEIVERS")) return false;
    ScreenStreamPolicyResult appliedPolicy;
    ok &= Check(session.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot,60,1920,1080,
        &appliedPolicy).ok() &&
        Access::ConnectionCeiling(session) == 13'063'680 &&
        Access::StartBitrate(session) == 9'953'280 &&
        appliedPolicy.maxBitrateBps == 12'441'600 &&
        appliedPolicy.networkProbeMaxBitrateBps == 13'063'680 &&
        appliedPolicy.startBitrateBps == 9'953'280 &&
        Matches(Access::Parameters(session),1920,1080,60,12'441'600) && providerReads == 1,
        "LOW_BPP_CAPS_REAL_RTP_AND_START_PRIOR_BELOW_ORIGINAL_MEDIA_LIMIT");
    ok &= Check(session.SetVideoSlotEncodingPolicy(kCameraMainVideoSlot,60,1920,1080).ok() &&
        Access::ConnectionCeiling(session,kCameraMainVideoSlot) == 37'324'800 &&
        Matches(Access::Parameters(session,kCameraMainVideoSlot),1920,1080,60,18'662'400) &&
        providerReads == 1,
        "SCREEN_BPP_PROVIDER_DOES_NOT_CHANGE_CAMERA_POLICY");
    selectedBpp = 17;
    ok &= Check(Access::ConnectionCeiling(session) == 13'063'680 && providerReads == 1,
        "BPP_PREFERENCE_CHANGE_WAITS_FOR_NEXT_USER_POLICY_UPDATE");
    ok &= Check(session.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot,60,1920,1080).ok() &&
        Access::ConnectionCeiling(session) == 22'208'256 &&
        Matches(Access::Parameters(session),1920,1080,60,21'150'720) && providerReads == 2,
        "NEXT_USER_POLICY_UPDATE_READS_NEW_BPP_AND_WRITES_REAL_RTP");
    selectedBpp = 3;
    ok &= Check(session.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot,60,1920,1080).ok() &&
        Access::ConnectionCeiling(session) == 3'919'104 &&
        Access::StartBitrate(session) == 3'732'480 &&
        Matches(Access::Parameters(session),1920,1080,60,3'732'480),
        "MINIMUM_CUSTOM_BPP_HAS_NO_ARTIFICIAL_4_MBPS_FLOOR");
    selectedBpp = 0;
    ok &= Check(session.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot,60,1920,1080).ok() &&
        Access::ConnectionCeiling(session) == 19'595'520 &&
        Matches(Access::Parameters(session),1920,1080,60,18'662'400),
        "INVALID_HOST_BPP_FALLS_BACK_TO_DEFAULT_REFERENCE");
    session.SetScreenVideoBitrateBppProvider({});
    return ok;
}

bool RunDynamicBpp(LibWebRtcSession& session)
{
    session.Start({});
    if (!Check(session.PrepareVideoTransceiverSlot(kScreenMainVideoSlot).ok() &&
        session.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot, 60, 1920, 1080).ok() &&
        session.SetVideoSlotSendingActive(kScreenMainVideoSlot, true).ok(),
        "DYNAMIC_BPP_ACTIVE_REAL_RTP_FIXTURE")) return false;
    bool ok = true;
    session.SetScreenContentPolicyObservation(Observation());
    const auto semantic = Access::CurrentObservation(session);
    const auto bootstrap = Access::Bootstrap(session);
    const auto epoch = Access::Epoch(session);
    session.SetScreenVideoBitrateBpp(3);
    ok &= Check(Access::ApplyPendingBpp(session) &&
        Access::ConnectionCeiling(session) == 3'919'104 &&
        Access::Binding(session).userBitrate == 3'732'480 &&
        Access::StartBitrate(session) <= Access::Binding(session).userBitrate &&
        Matches(Access::Parameters(session), 1920, 1080, 60, 3'732'480) &&
        Access::NativeQualityTarget(session) == std::pair{60u, 3'732'480u},
        "DYNAMIC_BPP_LOWER_UPDATES_ACTIVE_RTP_WITHOUT_NEW_USER_SPEC");
    session.SetScreenVideoBitrateBpp(50);
    ok &= Check(Access::ApplyPendingBpp(session) &&
        Access::ConnectionCeiling(session) == 65'318'400 &&
        Access::Binding(session).userBitrate == 62'208'000 &&
        Matches(Access::Parameters(session), 1920, 1080, 60, 62'208'000) &&
        Access::NativeQualityTarget(session) == std::pair{60u, 62'208'000u},
        "DYNAMIC_BPP_HIGHER_RESTORES_MEDIA_LIMIT_AND_RAISES_CONNECTION_LIMIT");
    const auto retainedSemantic = Access::CurrentObservation(session);
    ok &= Check(Access::Bootstrap(session) == bootstrap && Access::Epoch(session) > epoch &&
        retainedSemantic.enabled == semantic.enabled && retainedSemantic.generation == semantic.generation &&
        retainedSemantic.scene == semantic.scene && retainedSemantic.observedAtMs == semantic.observedAtMs,
        "DYNAMIC_BPP_NEVER_REAPPLIES_STARTUP_AND_PRESERVES_SCENE_WHILE_EXPIRING_OLD_STATS");

    session.SetScreenContentPolicyObservation(Observation());
    ContentAwareStreamInput input;
    input.generation = 1;
    ContentAwareStreamEvaluation downgraded;
    downgraded.confirmed = true;
    downgraded.recommendation = {.width = 1280, .height = 720, .senderMaxFps = 30,
        .desiredVideoBitrateBps = 6'000'000, .senderMaxBitrateBps = 6'000'000,
        .hasRecommendation = true, .estimatedFeasible = true, .modelCalibrated = true,
        .qualityRequirementMet = true, .automaticControlEligible = true};
    ok &= Check(Access::Apply(session, input, downgraded, Access::Binding(session).revision).ok() &&
        Matches(Access::Parameters(session), 1280, 720, 30, 6'000'000),
        "DYNAMIC_BPP_SCENE_DOWNGRADE_FIXTURE");
    const auto before = Access::Binding(session);
    session.SetScreenVideoBitrateBpp(3);
    ok &= Check(Access::ApplyPendingBpp(session) &&
        Matches(Access::Parameters(session), 1280, 720, 30, 3'732'480),
        "DYNAMIC_BPP_LOWER_CLAMPS_BITRATE_AND_PRESERVES_SCENE_R_F");
    session.SetScreenVideoBitrateBpp(50);
    const bool raised = Access::ApplyPendingBpp(session);
    const auto after = Access::Binding(session);
    ok &= Check(raised && after.userWidth == 1920 && after.userHeight == 1080 &&
        after.userFps == 60 && after.userBitrate == 62'208'000 &&
        after.effectiveWidth == before.effectiveWidth &&
        after.effectiveHeight == before.effectiveHeight && after.effectiveFps == before.effectiveFps &&
        Matches(Access::Parameters(session), 1280, 720, 30, after.effectiveBitrate) &&
        after.effectiveBitrate <= 6'000'000 && Access::Bootstrap(session) == bootstrap,
        "DYNAMIC_BPP_HIGHER_PRESERVES_WEAK_NETWORK_ALLOCATION_UNTIL_POLICY_RECOVERY");
    return ok;
}

bool Run(LibWebRtcSession& session)
{
    bool ok = true;
    session.Start({});
    if (!Check(session.PrepareVideoTransceiverSlot(kScreenMainVideoSlot).ok(),
        "REAL_SCREEN_TRANSCEIVER")) return false;
    if (!Check(session.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot, 60, 1920, 1080).ok() &&
        session.SetVideoSlotSendingActive(kScreenMainVideoSlot, true).ok(),
        "REAL_RTP_USER_POLICY")) return false;
    const auto user = Access::Binding(session);
    session.SetScreenContentPolicyObservation(Observation());
    Access::Sample(session, Sample());
    ok &= Check(!Access::Binding(session).execution.applied &&
        Matches(Access::Parameters(session), 1920, 1080, 60, user.userBitrate),
        "UNCALIBRATED_MODEL_NEVER_WRITES_RTP");

    ContentAwareStreamConfig calibrated;
    calibrated.qualityEstimator = TestModel;
    session.SetScreenContentPolicyModel(calibrated);
    // This fixture represents a complete post-configuration window. The typed
    // evidence tests below separately verify that crossing windows are rejected.
    Access::AdvanceEvidenceWindowForTest(session, Now() - 1000);
    auto missingQuality = Observation();
    missingQuality.qualityAvailable = false;
    session.SetScreenContentPolicyObservation(missingQuality);
    Access::Sample(session, Sample());
    ok &= Check(!Access::Binding(session).execution.applied &&
        Access::Binding(session).execution.eligible && !Access::QualityVerified(session),
        "MISSING_QUALITY_EVIDENCE_DOES_NOT_BLOCK_CONFIRMED_GCC_NETWORK_REPAIR");
    auto missingProcessing = Observation();
    missingProcessing.processingAvailable = false;
    session.SetScreenContentPolicyObservation(missingProcessing);
    Access::Sample(session, Sample());
    ok &= Check(!Access::Binding(session).execution.applied &&
        Access::Binding(session).execution.eligible,
        "MISSING_PROCESSING_EVIDENCE_DOES_NOT_BLOCK_CONFIRMED_WEAK_NETWORK_DECISION");

    session.SetScreenContentPolicyObservation(Observation());
    std::this_thread::sleep_for(milliseconds(2));
    const auto first = Sample();
    Access::Sample(session, first);
    ok &= Check(!Access::Binding(session).execution.applied,
        "EARLY_FRESH_WINDOW_WAITING_ONE_SECOND_DOES_NOT_EXECUTE");
    Access::Sample(session, first);
    ok &= Check(!Access::Binding(session).execution.applied,
        "DUPLICATE_WINDOW_DOES_NOT_CONFIRM");
    std::this_thread::sleep_for(milliseconds(1050));
    session.SetScreenContentPolicyObservation(Observation());
    Access::Sample(session, Sample());
    const auto applied = Access::Binding(session);
    ok &= Check(applied.execution.applied &&
        Matches(Access::Parameters(session), 1920, 1080, 50, 12'350'000),
        "TEXT_DOWNGRADE_UPDATES_REAL_R_F_B_PARAMETERS");
    ok &= Check(applied.userWidth == 1920 && applied.userHeight == 1080 &&
        applied.userFps == 60 && applied.userBitrate == user.userBitrate,
        "SCENE_EXECUTION_PRESERVES_USER_LIMITS");

    ContentAwareStreamInput input;
    input.generation = 1;
    ContentAwareStreamEvaluation oldDecision;
    oldDecision.confirmed = true;
    oldDecision.recommendation = {.width = 1760, .height = 990, .senderMaxFps = 60,
        .desiredVideoBitrateBps = 10'000'000, .senderMaxBitrateBps = 10'000'000,
        .hasRecommendation = true, .estimatedFeasible = true, .modelCalibrated = true,
        .qualityRequirementMet = true, .automaticControlEligible = true};
    auto revision = applied.revision;
    std::promise<void> lockAcquired, releaseLock;
    auto acquired = lockAcquired.get_future();
    std::thread holdingThread(Access::HoldSenderLock, std::ref(session),
        std::ref(lockAcquired), releaseLock.get_future());
    acquired.wait();
    const auto sceneBusyStart = steady_clock::now();
    const auto busyResult = Access::Apply(session, input, oldDecision, revision);
    const auto sceneBusyMs = duration_cast<milliseconds>(
        steady_clock::now() - sceneBusyStart).count();
    const auto busyBinding = Access::Binding(session);
    releaseLock.set_value();
    holdingThread.join();
    ok &= Check(!busyResult.ok() && sceneBusyMs < 1000 &&
        busyBinding.execution.status == "sender_busy" &&
        busyBinding.execution.successfulChanges == applied.execution.successfulChanges &&
        Matches(Access::Parameters(session), 1920, 1080, 50, 12'350'000),
        "BUSY_SCENE_SENDER_NEVER_BLOCKS_OR_WRITES_RTP");
    std::cout << "BUSY_SCENE_RETURN_MS=" << sceneBusyMs << '\n';
    ok &= Check(!Access::Apply(session, input, oldDecision, revision - 1).ok() &&
        Matches(Access::Parameters(session), 1920, 1080, 50, 12'350'000),
        "EXPIRED_REVISION_CANNOT_WRITE_RTP");
    input.generation = 999;
    ok &= Check(!Access::Apply(session, input, oldDecision, revision).ok() &&
        Matches(Access::Parameters(session), 1920, 1080, 50, 12'350'000),
        "EXPIRED_GENERATION_CANNOT_WRITE_RTP");
    input.generation = 1;
    auto oversize = oldDecision;
    oversize.recommendation.width = 3840;
    ok &= Check(!Access::Apply(session, input, oversize, revision).ok(),
        "RESOLUTION_ABOVE_USER_LIMIT_REJECTED");
    auto tooFast = oldDecision;
    tooFast.recommendation.senderMaxFps = 120;
    ok &= Check(!Access::Apply(session, input, tooFast, revision).ok(),
        "FPS_ABOVE_USER_LIMIT_REJECTED");
    auto exceedsVideoCeiling = oldDecision;
    exceedsVideoCeiling.recommendation.desiredVideoBitrateBps = user.userBitrate + 1;
    exceedsVideoCeiling.recommendation.senderMaxBitrateBps = user.userBitrate + 1;
    ok &= Check(exceedsVideoCeiling.recommendation.senderMaxBitrateBps <=
        Access::ConnectionCeiling(session) &&
        !Access::Apply(session, input, exceedsVideoCeiling, revision).ok() &&
        Matches(Access::Parameters(session), 1920, 1080, 50, 12'350'000),
        "SCENE_CANNOT_SPEND_CONNECTION_HEADROOM_BEYOND_USER_VIDEO_CEILING");

    // A real sender rejects max < min. This exercises the actuator failure,
    // rather than merely an application's pre-validation rejection.
    const auto savedMinimum = Access::Parameters(session).encodings.front().min_bitrate_bps;
    ok &= Check(Access::SetMinimumBitrate(session, 10'500'000).ok(),
        "REAL_SENDER_FAILURE_FIXTURE_INSTALLED");
    const auto beforeFailure = Access::Binding(session);
    const auto failed = Access::Apply(session, input, oldDecision, revision);
    const auto afterFailure = Access::Binding(session);
    ok &= Check(!failed.ok() &&
        afterFailure.effectiveWidth == beforeFailure.effectiveWidth &&
        afterFailure.effectiveHeight == beforeFailure.effectiveHeight &&
        afterFailure.effectiveFps == beforeFailure.effectiveFps &&
        afterFailure.effectiveBitrate == beforeFailure.effectiveBitrate &&
        afterFailure.execution.successfulChanges == beforeFailure.execution.successfulChanges &&
        afterFailure.execution.status == "apply_failed" && !afterFailure.execution.error.empty() &&
        Matches(Access::Parameters(session), 1920, 1080, 50, 12'350'000),
        "REAL_RTP_FAILURE_PRESERVES_EFFECTIVE_STATE_AND_REPORTS_ERROR");
    ok &= Check(Access::SetMinimumBitrate(session, savedMinimum).ok(),
        "REAL_SENDER_FAILURE_FIXTURE_REMOVED");

    auto disabled = Observation();
    disabled.enabled = false;
    session.SetScreenContentPolicyObservation(disabled);
    Access::Sample(session, Sample());
    ok &= Check(!Access::Binding(session).execution.applied &&
        Matches(Access::Parameters(session), 1920, 1080, 60, user.userBitrate),
        "DISABLE_IMMEDIATELY_RESTORES_USER_R_F_B");

    // A fresh user request replaces every prior candidate and restore action.
    ok &= Check(session.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot, 60, 1920, 1080).ok(),
        "NEW_USER_REQUEST_RESETS_OLD_POLICY");
    session.SetScreenContentPolicyObservation(Observation(2, ScreenScene::kGame3d));
    // The new request excludes any window that crossed its reset boundary.
    std::this_thread::sleep_for(milliseconds(1050));
    Access::Sample(session, Sample());
    std::this_thread::sleep_for(milliseconds(1050));
    session.SetScreenContentPolicyObservation(Observation(2, ScreenScene::kGame3d));
    Access::Sample(session, Sample());
    const auto motion = Access::Binding(session);
    ok &= Check(motion.execution.applied &&
        Matches(Access::Parameters(session), 1760, 990, 60, 12'350'000),
        "MOTION_REDUCES_SIZE_WHILE_PRESERVING_REQUESTED_FPS");

    ok &= Check(session.SetVideoSlotSendingActive(kScreenMainVideoSlot, false).ok() &&
        Matches(Access::Parameters(session), 1920, 1080, 60, user.userBitrate) &&
        !Access::Parameters(session).encodings.front().active,
        "STOP_RESTORES_USER_POLICY_IN_SAME_RTP_TRANSACTION");
    ok &= Check(session.SetVideoSlotSendingActive(kScreenMainVideoSlot, true).ok() &&
        Matches(Access::Parameters(session), 1920, 1080, 60, user.userBitrate) &&
        Access::Parameters(session).encodings.front().active,
        "RESTART_WITHOUT_POLICY_SET_DOES_NOT_REUSE_SCENE_PARAMETERS");
    std::this_thread::sleep_for(milliseconds(1050));
    Access::Sample(session, Sample());
    std::this_thread::sleep_for(milliseconds(1050));
    session.SetScreenContentPolicyObservation(Observation(2, ScreenScene::kGame3d));
    Access::Sample(session, Sample());
    ok &= Check(Access::Binding(session).execution.applied &&
        Matches(Access::Parameters(session), 1760, 990, 60, 12'350'000),
        "RESTART_REQUIRES_FRESH_WINDOWS_BEFORE_SCENE_EXECUTION");

    const auto previousEpoch = Access::Epoch(session);
    session.SetScreenContentPolicyObservation(Observation(3, ScreenScene::kGame3d));
    ok &= Check(!Access::Sample(session, Sample(), previousEpoch) &&
        Matches(Access::Parameters(session), 1760, 990, 60, 12'350'000),
        "OLD_STATS_CALLBACK_EPOCH_CANNOT_DRIVE_NEW_SHARE");
    Access::Sample(session, Sample());
    ok &= Check(!Access::Binding(session).execution.applied &&
        Matches(Access::Parameters(session), 1920, 1080, 60, user.userBitrate),
        "SHARE_GENERATION_CHANGE_RESTORES_BEFORE_NEW_DECISION");
    auto stale = Observation(3);
    stale.observedAtMs -= 5000;
    session.SetScreenContentPolicyObservation(stale);
    Access::Sample(session, Sample());
    ok &= Check(!Access::Binding(session).execution.eligible,
        "STALE_SCENE_OBSERVATION_BLOCKS_EXECUTION");
    int unownedContext = 0;
    calibrated.qualityEstimatorContext = &unownedContext;
    session.SetScreenContentPolicyModel(calibrated);
    session.SetScreenContentPolicyObservation(Observation(3));
    Access::Sample(session, Sample());
    ok &= Check(!Access::Binding(session).execution.eligible &&
        Access::Binding(session).execution.status == "awaiting_calibration",
        "UNOWNED_ESTIMATOR_CONTEXT_CANNOT_ENABLE_EXECUTION");
    ok &= Check(session.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot, 30, 1280, 720).ok(),
        "LOWER_USER_LIMITS_APPLY");
    input.generation = 3;
    revision = Access::Binding(session).revision;
    ok &= Check(!Access::Apply(session, input, oldDecision, revision).ok(),
        "OLD_CANDIDATE_CANNOT_EXCEED_NEW_USER_LIMITS");
    ok &= Check(session.SetVideoSlotSendingActive(kScreenMainVideoSlot, false).ok(),
        "SHARE_STOP_APPLIES");
    revision = Access::Binding(session).revision;
    ok &= Check(!Access::Apply(session, input, oldDecision, revision).ok() &&
        !Access::Sample(session, Sample()), "STOPPED_SHARE_CANNOT_EXECUTE");
    return ok;
}

ScreenContentPolicyObservation EvidenceObservation()
{
    auto observation = Observation(10);
    // The production-metric tests must never enable either shortcut. Both
    // gates have to come from typed encoder stats and authenticated feedback.
    observation.qualityAvailable = observation.qualityAcceptable = false;
    observation.processingAvailable = observation.processingHealthy = false;
    return observation;
}

WebRtcSessionStatsSnapshot EncoderEvidence()
{
    auto sample = Sample();
    auto& stream = sample.rtpStreams.front();
    stream.codec = "video/H264";
    stream.encoderImplementation = "TEST/Software";
    stream.sampleWindowMs = 1000;
    stream.frameWidth = 1920;
    stream.frameHeight = 1080;
    stream.framesEncoded = 60;
    stream.framesSent = 60;
    stream.windowQpAvailable = true;
    stream.windowQp = 24;
    stream.windowEncodeTimeAvailable = true;
    stream.windowEncodeTimeMs = 1;
    return sample;
}

ScreenReceiverFeedback ReceiverEvidence(std::uint64_t sequence = 10)
{
    return {.roomId = "TEST/room", .senderDeviceId = "TEST/receiver",
        .sequence = sequence, .screenShareGeneration = 10, .preferenceSequence = 7,
        .sampleWindowMs = 1000, .decodedFrames = 60, .droppedFrames = 0,
        .frameWidth = 1920, .frameHeight = 1080,
        .decodeTimeAvailable = true, .processingTimeAvailable = true,
        .windowDecodeTimeUs = 2000, .processingTimeUs = 5000};
}

std::shared_ptr<const CalibratedStreamQualityModel> EvidenceModel(
    ScreenScene scene = ScreenScene::kCodeTerminal)
{
    auto model = std::make_shared<CalibratedStreamQualityModel>();
    // Synthetic TEST measurements only. Their reported criterion and evidence
    // ID explicitly do not claim a production encoder/perceptual benchmark.
    const std::array samples{
        StreamQualityCalibrationSample{scene, 1920, 1080, 30, 6'220'800, 30, true, 28},
        StreamQualityCalibrationSample{scene, 1920, 1080, 45, 9'331'200, 30, true, 28},
        StreamQualityCalibrationSample{scene, 1920, 1080, 60, 12'441'600, 30, true, 28}};
    if (model->Build(samples, {"video/H264", "TEST/Software", "TEST/preset"},
        "TEST/synthetic-quality-fixture-only") != StreamCalibrationError::kNone) return {};
    return model;
}

void CompleteEvidenceWindowForTest(LibWebRtcSession& session)
{
    // Skip only the wall-clock wait for the window floor. This does not change
    // stats, thresholds, dimensions, feedback contracts or production code.
    // Call AFTER the first sample has bound codec/encoder and reset the floor.
    Access::AdvanceEvidenceWindowForTest(session, Now() - 1000);
}

bool RunProcessingTelemetry(LibWebRtcSession& session, bool slowProcessing)
{
    session.Start({});
    if (!Check(session.PrepareVideoTransceiverSlot(kScreenMainVideoSlot).ok() &&
        session.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot, 60, 1920, 1080).ok() &&
        session.SetVideoSlotSendingActive(kScreenMainVideoSlot, true).ok(),
        "PROCESSING_DIAGNOSTIC_REAL_RTP_FIXTURE")) return false;
    const auto user = Access::Binding(session);
    auto model = EvidenceModel();
    if (!model || !model->IsValid()) return false;
    session.SetScreenContentPolicyCalibration(model, "TEST/preset");
    session.SetScreenSenderFeedbackContract(10, 7);
    auto evidence = EncoderEvidence();
    evidence.transport.googCc.delayState = "normal";
    evidence.rtpStreams.front().windowEncodeTimeAvailable = slowProcessing;
    // 40 ms is well above 80% of the 60-FPS frame period (13.33 ms).
    evidence.rtpStreams.front().windowEncodeTimeMs = 40;
    auto feedback = ReceiverEvidence();
    feedback.windowDecodeTimeUs = 40'000;
    feedback.processingTimeUs = 50'000;
    const auto sample = [&] {
        // Each call represents a fresh stats window. Millisecond timestamps
        // shared with the preceding healthy window must not confirm a change.
        std::this_thread::sleep_for(milliseconds(2));
        evidence.transport.receivedAtSteadyMs = Now();
        session.SetScreenContentPolicyObservation(EvidenceObservation());
        Access::Sample(session, evidence);
    };
    sample();
    CompleteEvidenceWindowForTest(session);
    if (slowProcessing) session.AcceptScreenReceiverFeedback(feedback);
    evidence.rtpStreams.front().sentFramesPerSecond = 60;
    evidence.rtpStreams.front().qualityLimitationReason = "none";
    sample();
    bool ok = Check(!Access::Binding(session).execution.applied &&
        Access::Shadow(session).reason == "healthy_hold" &&
        Matches(Access::Parameters(session), 1920, 1080, 60, user.userBitrate),
        slowProcessing ? "SLOWER_THAN_FRAME_BUDGET_WITHOUT_WEAK_NETWORK_NEVER_WRITES_R_F_B"
                       : "MISSING_TIMINGS_WITHOUT_WEAK_NETWORK_NEVER_WRITES_R_F_B");
    evidence.transport.googCc.delayState = "overuse";
    // Keep delivery healthy and the adaptation label clear: GoogCC overuse
    // must authorize allocation before FPS drops or quality becomes visibly bad.
    evidence.rtpStreams.front().sentFramesPerSecond = 60;
    evidence.rtpStreams.front().qualityLimitationReason = "none";
    sample();
    ok &= Check(Access::Binding(session).execution.eligible &&
        !Access::Binding(session).execution.applied && Access::Shadow(session).senderMaxFps == 50,
        slowProcessing ? "OVER_80_PERCENT_FRAME_BUDGET_DOES_NOT_BLOCK_WEAK_NETWORK_CANDIDATE"
                       : "MISSING_TIMINGS_DO_NOT_BLOCK_WEAK_NETWORK_CANDIDATE");
    std::this_thread::sleep_for(milliseconds(1050));
    if (slowProcessing) {
        ++feedback.sequence;
        session.AcceptScreenReceiverFeedback(feedback);
    }
    sample();
    ok &= Check(Access::Binding(session).execution.applied &&
        Matches(Access::Parameters(session), 1920, 1080, 50, 12'350'000),
        slowProcessing ? "OVER_80_PERCENT_FRAME_BUDGET_STILL_WRITES_CONFIRMED_SCENE_DOWNGRADE"
                       : "MISSING_TIMINGS_STILL_WRITE_CONFIRMED_SCENE_DOWNGRADE");
    CompleteEvidenceWindowForTest(session);
    evidence.transport.availableOutgoingBitrateBps = 80'000'000;
    evidence.transport.googCc.delayState = "normal";
    evidence.rtpStreams.front().encodedFramesPerSecond = 50;
    evidence.rtpStreams.front().sentFramesPerSecond = 50;
    evidence.rtpStreams.front().qualityLimitationReason = "none";
    if (slowProcessing) {
        ++feedback.sequence;
        session.AcceptScreenReceiverFeedback(feedback);
    }
    sample();
    const auto recovery = Access::Shadow(session);
    ok &= Check(recovery.width <= user.userWidth && recovery.height <= user.userHeight &&
        recovery.senderMaxFps <= user.userFps && recovery.senderMaxBitrateBps <= user.userBitrate,
        "RECOVERY_WITH_DIAGNOSTIC_ONLY_TIMINGS_CANNOT_EXCEED_USER_R_F_B_CEILINGS");
    return ok;
}

bool RunEvidence(LibWebRtcSession& session)
{
    bool ok = true;
    session.Start({});
    if (!Check(session.PrepareVideoTransceiverSlot(kScreenMainVideoSlot).ok() &&
        session.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot, 60, 1920, 1080).ok() &&
        session.SetVideoSlotSendingActive(kScreenMainVideoSlot, true).ok(),
        "TYPED_EVIDENCE_REAL_RTP_SESSION_READY")) return false;
    const auto user = Access::Binding(session);
    auto model = EvidenceModel();
    if (!Check(model && model->IsValid(), "TYPED_EVIDENCE_TEST_CALIBRATION_BUILDS")) return false;
    session.SetScreenContentPolicyCalibration(model, "TEST/preset");
    session.SetScreenSenderFeedbackContract(10, 7);
    session.SetScreenContentPolicyObservation(EvidenceObservation());
    Access::Sample(session, EncoderEvidence());
    ok &= Check(!Access::Binding(session).execution.eligible,
        "WINDOW_CROSSING_NEW_CALIBRATION_FLOOR_CANNOT_EXECUTE");
    CompleteEvidenceWindowForTest(session);
    Access::Sample(session, EncoderEvidence());
    ok &= Check(Access::Binding(session).execution.eligible &&
        !Access::Binding(session).execution.applied,
        "VALID_QUALITY_AND_WEAK_NETWORK_DO_NOT_REQUIRE_RECEIVER_TIMING_FEEDBACK");
    auto wrongGeneration = ReceiverEvidence();
    wrongGeneration.screenShareGeneration = 9;
    auto wrongPreference = ReceiverEvidence();
    wrongPreference.preferenceSequence = 6;
    ok &= Check(!session.AcceptScreenReceiverFeedback(wrongGeneration) &&
        !session.AcceptScreenReceiverFeedback(wrongPreference),
        "RECEIVER_OLD_GENERATION_AND_PREFERENCE_REJECTED");
    ok &= Check(session.AcceptScreenReceiverFeedback(ReceiverEvidence()),
        "CURRENT_RECEIVER_FEEDBACK_ACCEPTED");
    ok &= Check(!session.AcceptScreenReceiverFeedback(ReceiverEvidence()) &&
        !session.AcceptScreenReceiverFeedback(ReceiverEvidence(9)),
        "RECEIVER_DUPLICATE_AND_REORDERED_SEQUENCE_REJECTED");

    auto noQp = EncoderEvidence();
    noQp.rtpStreams.front().windowQpAvailable = false;
    Access::Sample(session, noQp);
    ok &= Check(Access::Binding(session).execution.eligible &&
        !Access::QualityVerified(session) &&
        Access::Shadow(session).senderMaxFps < Access::Binding(session).effectiveFps,
        "MISSING_QP_WITH_EXPLICIT_GCC_OVERUSE_PERMITS_NETWORK_REPAIR_WITHOUT_QUALITY_CLAIM");
    auto badQp = EncoderEvidence();
    badQp.rtpStreams.front().windowQp = 35;
    Access::Sample(session, badQp);
    ok &= Check(Access::Binding(session).execution.eligible &&
        !Access::Binding(session).execution.applied &&
        Access::Shadow(session).senderMaxFps < Access::Binding(session).effectiveFps &&
        !Access::Shadow(session).estimatedFeasible &&
        Access::Shadow(session).senderMaxFps == 50 &&
        Access::Binding(session).execution.status == "observing_emergency",
        "BAD_QP_WITH_INSUFFICIENT_ADJACENT_TIER_BUDGET_REQUIRES_CONFIRMED_NETWORK_REDUCTION");
    auto wrongDimensions = EncoderEvidence();
    wrongDimensions.rtpStreams.front().frameWidth = 1280;
    wrongDimensions.rtpStreams.front().frameHeight = 720;
    Access::Sample(session, wrongDimensions);
    ok &= Check(!Access::Binding(session).execution.eligible,
        "ENCODER_METRICS_FOR_DIFFERENT_SIZE_CANNOT_EXECUTE");

    auto wrongEncoder = EncoderEvidence();
    wrongEncoder.rtpStreams.front().encoderImplementation = "TEST/other-encoder";
    Access::Sample(session, wrongEncoder);
    CompleteEvidenceWindowForTest(session);
    Access::Sample(session, wrongEncoder);
    ok &= Check(!Access::Binding(session).execution.eligible,
        "ENCODER_MISMATCH_HAS_NO_CALIBRATED_PERMISSION");
    session.SetScreenContentPolicyCalibration(model, "TEST/wrong-preset");
    Access::Sample(session, EncoderEvidence());
    CompleteEvidenceWindowForTest(session);
    Access::Sample(session, EncoderEvidence());
    ok &= Check(!Access::Binding(session).execution.eligible,
        "ENCODER_PROFILE_MISMATCH_CANNOT_EXECUTE");
    session.SetScreenContentPolicyCalibration(EvidenceModel(ScreenScene::kVideo), "TEST/preset");
    Access::Sample(session, EncoderEvidence());
    CompleteEvidenceWindowForTest(session);
    Access::Sample(session, EncoderEvidence());
    ok &= Check(!Access::Binding(session).execution.eligible,
        "UNMEASURED_SCENE_HAS_NO_CALIBRATED_PERMISSION");

    session.SetScreenContentPolicyCalibration(model, "TEST/preset");
    Access::Sample(session, EncoderEvidence());
    CompleteEvidenceWindowForTest(session);
    ok &= Check(session.AcceptScreenReceiverFeedback(ReceiverEvidence(11)),
        "FRESH_RECEIVER_FEEDBACK_AFTER_CALIBRATION_ACCEPTED");
    std::this_thread::sleep_for(milliseconds(2));
    session.SetScreenContentPolicyObservation(EvidenceObservation());
    Access::Sample(session, EncoderEvidence());
    ok &= Check(Access::Binding(session).execution.eligible &&
        !Access::Binding(session).execution.applied,
        "COMPLETE_TYPED_EVIDENCE_FIRST_WINDOW_WAITS");
    std::this_thread::sleep_for(milliseconds(1050));
    session.SetScreenContentPolicyObservation(EvidenceObservation());
    ok &= Check(session.AcceptScreenReceiverFeedback(ReceiverEvidence(12)),
        "NEXT_WINDOW_RECEIVER_FEEDBACK_ACCEPTED");
    Access::Sample(session, EncoderEvidence());
    ok &= Check(Access::Binding(session).execution.applied &&
        Matches(Access::Parameters(session), 1920, 1080, 50, 12'350'000) &&
        Access::Binding(session).userFps == 60 && Access::Binding(session).userBitrate == user.userBitrate,
        "TYPED_QP_ENCODER_RECEIVER_AND_CALIBRATION_EXECUTE_REAL_RTP");

    auto receiverContext = ReceiverEvidence();
    session.SetScreenReceiverFeedbackContext(receiverContext);
    const auto oldReceiverEpoch = Access::ReceiverEpoch(session);
    receiverContext.screenShareGeneration = 11;
    session.SetScreenReceiverFeedbackContext(receiverContext);
    auto received = Sample();
    auto& inbound = received.rtpStreams.front();
    inbound.direction = RtpStreamDirection::kInbound;
    inbound.statsId = "TEST/inbound-generation-10";
    inbound.receiverFrameCountersAvailable = true;
    inbound.framesDecoded = 60;
    inbound.frameWidth = 1920;
    inbound.frameHeight = 1080;
    ok &= Check(Access::RejectOldReceiverEpoch(session, received, oldReceiverEpoch),
        "OLD_RECEIVER_CALLBACK_CANNOT_CREATE_NEW_GENERATION_REPORT");
    return ok;
}

bool RunVerifiedUserRecovery(LibWebRtcSession& session)
{
    session.Start({});
    bool ok = Check(session.PrepareVideoTransceiverSlot(kScreenMainVideoSlot).ok() &&
        session.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot,60,1920,1080).ok() &&
        session.SetVideoSlotSendingActive(kScreenMainVideoSlot,true).ok(), "VERIFIED_BASELINE_REAL_RTP_READY");
    if (!ok) return false;
    session.SetScreenContentPolicyReferenceEnabled(true, "medium");
    auto evidence = EncoderEvidence();
    evidence.rtpStreams.front().encoderImplementation = "FFmpeg/NVENC";
    evidence.rtpStreams.front().qualityLimitationReason = "none";
    evidence.rtpStreams.front().sentFramesPerSecond = 60;
    evidence.rtpStreams.front().bitrateBps = 12'000'000;
    evidence.rtpStreams.front().targetBitrateBps = Access::Binding(session).userBitrate;
    evidence.transport.googCc.delayState = "normal";
    evidence.transport.availableOutgoingBitrateBps = 20'000'000;
    const auto sample = [&] {
        evidence.transport.receivedAtSteadyMs = Now();
        session.SetScreenContentPolicyObservation(EvidenceObservation());
        Access::Sample(session, evidence);
    };
    sample();
    CompleteEvidenceWindowForTest(session);
    sample();
    std::this_thread::sleep_for(milliseconds(1010)); sample();
    std::this_thread::sleep_for(milliseconds(1010)); sample();
    ok &= Check(Access::Binding(session).execution.userSpecificationVerifiedBitrateBps == 12'000'000 &&
        Access::Binding(session).effectiveFps == 60 && !Access::Binding(session).execution.networkPressure,
        "THREE_MOVING_QP_PASS_WINDOWS_LEARN_ACTUAL_COST_NOT_TARGET_CEILING");
    evidence.transport.googCc.delayState = "overuse";
    evidence.transport.availableOutgoingBitrateBps = 10'000'000;
    std::this_thread::sleep_for(milliseconds(2)); sample();
    std::this_thread::sleep_for(milliseconds(1050)); sample();
    const auto lower = Access::Binding(session);
    ok &= Check(lower.execution.applied && lower.effectiveFps < 60 &&
        lower.execution.networkPressure, "GCC_BUDGET_BELOW_VERIFIED_COST_APPLIES_REAL_RTP_DOWNGRADE");
    CompleteEvidenceWindowForTest(session);
    evidence.transport.googCc.delayState = "normal";
    // This budget can restore the verified 12-Mbps cost, but stays below the
    // separate near-full-user-ceiling recovery permission. The adjacent FPS
    // floor now also reduced size, so consume windows for that actual size.
    evidence.transport.availableOutgoingBitrateBps = 16'000'000;
    evidence.rtpStreams.front().frameWidth = lower.effectiveWidth;
    evidence.rtpStreams.front().frameHeight = lower.effectiveHeight;
    evidence.rtpStreams.front().encodedFramesPerSecond = lower.effectiveFps;
    evidence.rtpStreams.front().sentFramesPerSecond = lower.effectiveFps;
    // Spatial recovery still requires three seconds since the size changed.
    std::this_thread::sleep_for(milliseconds(1010));
    std::this_thread::sleep_for(milliseconds(2)); sample();
    ok &= Check(Access::Binding(session).effectiveFps == lower.effectiveFps &&
        Access::Binding(session).execution.networkPressure,
        "NORMAL_AFTER_CONGESTION_REQUIRES_STABLE_RECOVERY_BEFORE_RESTORING_RTP");
    std::this_thread::sleep_for(milliseconds(1050)); sample();
    ok &= Check(Access::Binding(session).effectiveFps == lower.effectiveFps &&
        Access::Binding(session).execution.confirmObservedSamples == 2 &&
        Access::Binding(session).execution.confirmRequiredSamples == 3,
        "SECOND_RECOVERY_WINDOW_WAITS_FOR_THIRD_WINDOW_AND_TWO_SECONDS");
    std::this_thread::sleep_for(milliseconds(1050)); sample();
    const auto restored = Access::Binding(session);
    ok &= Check(restored.effectiveWidth == 1920 && restored.effectiveHeight == 1080 &&
        restored.effectiveFps == 60 && restored.effectiveBitrate <= restored.userBitrate &&
        Matches(Access::Parameters(session), 1920, 1080, 60, restored.effectiveBitrate),
        "THIRD_WINDOW_AFTER_TWO_SECONDS_RESTORES_REAL_RTP_USING_VERIFIED_ACTUAL_COST");
    return ok;
}

bool RunRecoveryBudgetRamp(LibWebRtcSession& session, bool spatialRecovery)
{
    // Network/encoder metrics are synthetic. The resulting writes and reads
    // exercise the real libwebrtc sender; this is not a two-machine link test.
    session.Start({});
    if (!Check(session.PrepareVideoTransceiverSlot(kScreenMainVideoSlot).ok() &&
        session.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot, 60, 1920, 1080).ok() &&
        session.SetVideoSlotSendingActive(kScreenMainVideoSlot, true).ok(),
        spatialRecovery ? "SPATIAL_RAMP_REAL_RTP_READY" : "FPS_RAMP_REAL_RTP_READY")) return false;
    ContentAwareStreamConfig calibrated;
    calibrated.qualityEstimator = TestModel;
    session.SetScreenContentPolicyModel(calibrated);
    const auto scene = spatialRecovery ? ScreenScene::kGame3d : ScreenScene::kCodeTerminal;
    auto evidence = Sample(13'000'000);
    const auto sample = [&] {
        evidence.transport.receivedAtSteadyMs = Now();
        session.SetScreenContentPolicyObservation(Observation(20, scene));
        Access::Sample(session, evidence);
    };
    sample();
    CompleteEvidenceWindowForTest(session);
    std::this_thread::sleep_for(milliseconds(2)); sample();
    bool ok = Check(!Access::Binding(session).execution.applied,
        "RAMP_FIRST_DOWNGRADE_WINDOW_WAITING_ONE_SECOND");
    std::this_thread::sleep_for(milliseconds(1050)); sample();
    const auto lowered = Access::Binding(session);
    ok &= Check(lowered.execution.applied && lowered.execution.successfulChanges == 1 &&
        Matches(Access::Parameters(session), lowered.effectiveWidth, lowered.effectiveHeight,
            lowered.effectiveFps, lowered.effectiveBitrate) &&
        (spatialRecovery ? lowered.effectiveWidth == 1760 && lowered.effectiveFps == 60
                         : lowered.effectiveWidth == 1920 && lowered.effectiveFps == 50),
        "RAMP_CONFIRMED_DOWNGRADE_WRITES_REAL_RTP_AFTER_ONE_SECOND");
    CompleteEvidenceWindowForTest(session);
    evidence.rtpStreams.front().frameWidth = lowered.effectiveWidth;
    evidence.rtpStreams.front().frameHeight = lowered.effectiveHeight;
    evidence.rtpStreams.front().encodedFramesPerSecond = lowered.effectiveFps;
    evidence.rtpStreams.front().sentFramesPerSecond = lowered.effectiveFps;
    evidence.transport.googCc.delayState = "normal";
    // Every step rises by > 5%. All support the SAME user R/F with 1.15
    // headroom. Confirmation must retain the first (lowest) safe budget;
    // the RTP ceiling opens only to the model's 12,441,600-bps requirement.
    evidence.transport.availableOutgoingBitrateBps = 16'000'000;
    std::this_thread::sleep_for(milliseconds(2)); sample();
    ok &= Check(Access::Binding(session).execution.confirmObservedSamples == 1 &&
        Access::Shadow(session).width == 1920 && Access::Shadow(session).senderMaxFps == 60 &&
        Access::Shadow(session).senderMaxBitrateBps == 12'441'600 &&
        Access::PendingSafeBudget(session) == 15'200'000,
        "RAMP_FIRST_RECOVERY_WINDOW_SUPPORTS_USER_R_F_WITH_HEADROOM");
    evidence.transport.availableOutgoingBitrateBps = 17'200'000;
    std::this_thread::sleep_for(milliseconds(1050)); sample();
    ok &= Check(Access::Binding(session).execution.confirmObservedSamples == 2 &&
        Access::Binding(session).execution.successfulChanges == 1 &&
        Access::Shadow(session).senderMaxBitrateBps == 12'441'600 &&
        Access::PendingSafeBudget(session) == 15'200'000,
        "RAMP_ABOVE_FIVE_PERCENT_RETAINS_CONFIRMATION_AND_LOWEST_SAFE_BUDGET");
    evidence.transport.availableOutgoingBitrateBps = 18'500'000;
    std::this_thread::sleep_for(milliseconds(1050)); sample();
    if (spatialRecovery) {
        ok &= Check(Access::Binding(session).execution.successfulChanges == 1 &&
            Access::Binding(session).execution.confirmObservedSamples == 3 &&
            Access::Binding(session).execution.confirmationBlock == "awaiting_residence",
            "THREE_RECOVERY_WINDOWS_STILL_WAIT_FOR_THREE_SECOND_SPATIAL_RESIDENCE");
        std::this_thread::sleep_for(milliseconds(1050)); sample();
    }
    const auto restored = Access::Binding(session);
    ok &= Check(restored.execution.successfulChanges == 2 &&
        restored.effectiveWidth == 1920 && restored.effectiveHeight == 1080 &&
        restored.effectiveFps == 60 && restored.effectiveBitrate == 12'441'600 &&
        restored.effectiveBitrate <= 15'200'000 &&
        restored.effectiveBitrate <= restored.userBitrate &&
        Matches(Access::Parameters(session), 1920, 1080, 60, 12'441'600),
        spatialRecovery ? "SPATIAL_RAMP_RECOVERS_REAL_RTP_AFTER_THREE_SECOND_RESIDENCE"
                        : "FPS_RAMP_RECOVERS_REAL_RTP_AFTER_TWO_SECONDS_WITHOUT_SPATIAL_RESIDENCE");
    return ok;
}

bool RunEmergencyNetworkReduction(LibWebRtcSession& session)
{
    session.Start({});
    if (!Check(session.PrepareVideoTransceiverSlot(kScreenMainVideoSlot).ok() &&
        session.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot, 60, 1920, 1080).ok() &&
        session.SetVideoSlotSendingActive(kScreenMainVideoSlot, true).ok(),
        "EMERGENCY_560_KBPS_REAL_RTP_READY")) return false;
    session.SetScreenContentPolicyReferenceEnabled(true, "medium");
    auto evidence = EncoderEvidence();
    evidence.rtpStreams.front().encoderImplementation = "FFmpeg/NVENC";
    evidence.rtpStreams.front().windowQp = 50;
    evidence.transport.availableOutgoingBitrateBps = 560'000;
    const auto sample = [&] {
        evidence.transport.receivedAtSteadyMs = Now();
        session.SetScreenContentPolicyObservation(EvidenceObservation());
        Access::Sample(session, evidence);
    };
    sample();
    CompleteEvidenceWindowForTest(session);
    std::this_thread::sleep_for(milliseconds(2)); sample();
    const auto emergency = Access::Shadow(session);
    bool ok = Check(Access::Binding(session).execution.eligible &&
        Access::Binding(session).execution.status == "observing_emergency" &&
        !emergency.estimatedFeasible && emergency.modelReference &&
        emergency.reason == "emergency_network_reduction" &&
        emergency.requiredVideoBitrateBps > emergency.estimatedSafeVideoBudgetBps &&
        emergency.senderMaxBitrateBps <= 532'000 && !Access::QualityVerified(session),
        "INFEASIBLE_QUALITY_ESTIMATE_PERMITS_EXPLICIT_GCC_EMERGENCY_WITHOUT_FALSE_QUALITY_PASS");
    std::this_thread::sleep_for(milliseconds(1050)); sample();
    const auto applied = Access::Binding(session);
    const auto appliedShadow = Access::Shadow(session);
    ok &= Check(applied.execution.status == "emergency_applied" &&
        applied.execution.successfulChanges == 1 && applied.execution.applied &&
        (applied.effectiveWidth < 1920 || applied.effectiveFps < 60) &&
        applied.effectiveWidth <= applied.userWidth && applied.effectiveHeight <= applied.userHeight &&
        applied.effectiveFps <= applied.userFps && applied.effectiveBitrate <= 532'000 &&
        !appliedShadow.estimatedFeasible && !Access::QualityVerified(session) &&
        Matches(Access::Parameters(session), applied.effectiveWidth, applied.effectiveHeight,
            applied.effectiveFps, applied.effectiveBitrate),
        "EMERGENCY_560_KBPS_WRITES_REAL_LOWER_R_F_B_WITH_FAILED_QUALITY_STILL_EXPLICIT");
    return ok;
}

bool RunDisableRestoresOriginal120(LibWebRtcSession& session)
{
    session.Start({});
    if (!Check(session.PrepareVideoTransceiverSlot(kScreenMainVideoSlot).ok() &&
        session.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot, 120, 1920, 1080).ok() &&
        session.SetVideoSlotSendingActive(kScreenMainVideoSlot, true).ok(),
        "DISABLE_AFTER_120_FPS_NETWORK_REPAIR_REAL_RTP_READY")) return false;
    // Off enables quality protection through native temporary frame dropping.
    // RTP/capture upper limits remain the original user request.
    session.SetAdaptiveDesktopNetworkFrameRateEnabled(true);
    ContentAwareStreamConfig calibrated;
    calibrated.qualityEstimator = TestModel;
    session.SetScreenContentPolicyModel(calibrated);
    const auto user = Access::Binding(session);
    const auto bootstrap = Access::Bootstrap(session);
    const auto connectionCeiling = Access::ConnectionCeiling(session);
    auto evidence = Sample(560'000);
    const auto sample = [&] {
        evidence.transport.receivedAtSteadyMs = Now();
        session.SetScreenContentPolicyObservation(Observation(30));
        Access::Sample(session, evidence);
    };
    sample();
    CompleteEvidenceWindowForTest(session);
    std::this_thread::sleep_for(milliseconds(2)); sample();
    std::this_thread::sleep_for(milliseconds(1050)); sample();
    const auto lower = Access::Binding(session);
    bool ok = Check(lower.execution.applied && lower.effectiveFps == 100 &&
        lower.execution.successfulChanges == 1 && lower.userFps == 120 &&
        lower.userBitrate == user.userBitrate &&
        Matches(Access::Parameters(session), lower.effectiveWidth, lower.effectiveHeight,
            100, lower.effectiveBitrate),
        "FIRST_120_FPS_CONGESTION_STEP_APPLIES_ONLY_ADJACENT_100_FPS");
    auto disabled = Observation(30);
    disabled.enabled = false;
    session.SetScreenContentPolicyObservation(disabled);
    ok &= Check(!Access::NativeQualityProtectionEnabled(session),
        "OFF_PENDING_SCENE_RESTORE_KEEPS_NATIVE_PROTECTION_SUSPENDED");
    // Reuse this old/incomplete low-capacity window deliberately. Off restores
    // user parameters immediately, without waiting for recovery evidence.
    const auto before = steady_clock::now();
    Access::Sample(session, evidence);
    const auto elapsed = duration_cast<milliseconds>(steady_clock::now() - before).count();
    const auto restored = Access::Binding(session);
    ok &= Check(elapsed < 1000 && !restored.execution.applied &&
        restored.effectiveFps == 120 && restored.effectiveWidth == 1920 &&
        restored.effectiveHeight == 1080 && restored.effectiveBitrate == user.userBitrate &&
        !Access::LegacyFrameRateControllerEnabled(session) &&
        Access::NativeQualityProtectionEnabled(session) &&
        Matches(Access::Parameters(session), 1920, 1080, 120, user.userBitrate),
        "OFF_RESTORES_USER_120_FPS_AND_ENABLES_NATIVE_QUALITY_PROTECTION_WITHOUT_DELAY");
    Access::Sample(session, Sample(560'000));
    ok &= Check(!Access::LegacyFrameRateControllerEnabled(session) &&
        Access::NativeQualityProtectionEnabled(session) &&
        Matches(Access::Parameters(session), 1920, 1080, 120, user.userBitrate) &&
        Access::Bootstrap(session) == bootstrap &&
        Access::ConnectionCeiling(session) == connectionCeiling,
        "OFF_CONTENT_POLICY_NO_LONGER_ALLOCATES_FPS_OR_RESTARTS_BANDWIDTH_BOOTSTRAP");
    // Even catastrophic/partial recovery budgets must never install a hard
    // FPS limit. Native frame-dropper behaviour is tested at the encoder layer.
    const std::array budgets{84'400ull, 560'000ull, 4'800'000ull, 10'000'000ull, 40'000'000ull};
    for (const auto budget : budgets) {
        session.SetScreenContentActivity(ScreenContentActivity::kActive);
        Access::Sample(session, Sample(budget));
        ok &= Check(Access::Binding(session).effectiveFps == 120 &&
            !Access::Binding(session).execution.applied &&
            !Access::LegacyFrameRateControllerEnabled(session) &&
            Access::NativeQualityProtectionEnabled(session) &&
            Matches(Access::Parameters(session), 1920, 1080, 120, user.userBitrate) &&
            Access::Bootstrap(session) == bootstrap &&
            Access::ConnectionCeiling(session) == connectionCeiling,
            "OFF_WEAK_AND_RECOVERY_BUDGETS_KEEP_USER_FPS_MEDIA_AND_PROBE_UPPER_LIMITS");
    }
    auto staleRecovery = Sample(40'000'000);
    staleRecovery.transport.receivedAtSteadyMs -= 4000;
    Access::Sample(session, staleRecovery);
    Access::Sample(session, Sample(40'000'000), Access::Epoch(session) - 1);
    auto missingBwe = Sample(0);
    missingBwe.rtpStreams.front().targetBitrateBps = 84'400;
    Access::Sample(session, missingBwe);
    ok &= Check(Matches(Access::Parameters(session), 1920, 1080, 120, user.userBitrate),
        "OFF_STALE_EXPIRED_AND_MISSING_BWE_WINDOWS_CANNOT_WRITE_FPS_CAP");
    session.SetAdaptiveDesktopNetworkFrameRateEnabled(false);
    ok &= Check(!Access::NativeQualityProtectionEnabled(session) &&
        Matches(Access::Parameters(session), 1920, 1080, 120, user.userBitrate),
        "QUALITY_PROTECTION_SWITCH_OFF_PRESERVES_USER_RTP_PARAMETERS");
    session.SetAdaptiveDesktopNetworkFrameRateEnabled(true);
    ok &= Check(Access::NativeQualityProtectionEnabled(session),
        "QUALITY_PROTECTION_SWITCH_ON_SYNCHRONIZES_WITHOUT_WAITING_FOR_STATS");
    session.SetScreenContentPolicyObservation(Observation(30, ScreenScene::kUnknown));
    ok &= Check(!Access::NativeQualityProtectionEnabled(session) &&
        !Access::LegacyFrameRateControllerEnabled(session),
        "ON_UNKNOWN_SCENE_IMMEDIATELY_SUSPENDS_NATIVE_QUALITY_PROTECTION");
    Access::Sample(session, Sample(84'400));
    ok &= Check(Matches(Access::Parameters(session), 1920, 1080, 120, user.userBitrate),
        "ON_UNKNOWN_SCENE_DOES_NOT_INHERIT_NETWORK_FPS_RESTRICTION");
    session.SetScreenContentPolicyObservation(disabled);
    ok &= Check(Access::NativeQualityProtectionEnabled(session),
        "OFF_UNMODIFIED_SCENE_SYNCHRONIZES_NATIVE_PROTECTION_IMMEDIATELY");
    ok &= Check(session.SetVideoSlotSendingActive(kScreenMainVideoSlot, false).ok() &&
        !Access::NativeQualityProtectionEnabled(session),
        "STOPPED_SCREEN_IMMEDIATELY_DISARMS_NATIVE_QUALITY_PROTECTION");
    ok &= Check(session.SetVideoSlotSendingActive(kScreenMainVideoSlot, true).ok() &&
        Access::NativeQualityProtectionEnabled(session) &&
        Matches(Access::Parameters(session), 1920, 1080, 120, user.userBitrate),
        "RESTART_ARMS_NATIVE_PROTECTION_WITH_USER_FPS_INTACT");
    return ok;
}

bool RunNativeQualityProfileIsolation(WebRtcRuntime& runtime)
{
    LibWebRtcSession first(runtime.PeerConnectionFactory());
    LibWebRtcSession second(runtime.PeerConnectionFactory());
    bool ok = Check(Access::NativeQualityDeficitShare(first) == 50 &&
        Access::NativeQualityDeficitShare(second) == 50,
        "NATIVE_QUALITY_DEFICIT_SHARE_DEFAULT_IS_HALF_PER_CONNECTION");
    first.SetScreenQualityDeficitShare(35);
    first.SetScreenVideoBitrateBppProvider([] { return 15u; });
    second.SetScreenVideoBitrateBppProvider([] { return 3u; });
    first.Start({});
    second.Start({});
    first.SetAdaptiveDesktopNetworkFrameRateEnabled(true);
    second.SetAdaptiveDesktopNetworkFrameRateEnabled(true);
    ok &= Check(first.PrepareVideoTransceiverSlot(kScreenMainVideoSlot).ok() &&
        second.PrepareVideoTransceiverSlot(kScreenMainVideoSlot).ok() &&
        first.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot, 120, 1920, 1080).ok() &&
        second.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot, 30, 1280, 720).ok() &&
        first.SetVideoSlotSendingActive(kScreenMainVideoSlot, true).ok() &&
        second.SetVideoSlotSendingActive(kScreenMainVideoSlot, true).ok(),
        "NATIVE_QUALITY_TWO_CONNECTIONS_REAL_RTP_READY");
    if (!ok) return false;
    const auto firstCeiling = Access::ConnectionCeiling(first);
    const auto secondCeiling = Access::ConnectionCeiling(second);
    const auto firstBootstrap = Access::Bootstrap(first);
    const auto secondBootstrap = Access::Bootstrap(second);
    const auto firstEpoch = Access::Epoch(first);
    const auto secondEpoch = Access::Epoch(second);
    ok &= Check(Access::NativeQualityDeficitShare(first) == 35 &&
        Access::NativeQualityDeficitShare(second) == 50,
        "NATIVE_QUALITY_DEFICIT_SHARE_PRESTART_CONFIGURATION_SURVIVES_START");
    first.SetScreenQualityDeficitShare(0);
    second.SetScreenQualityDeficitShare(99);
    ok &= Check(Access::NativeQualityDeficitShare(first) == 0 &&
        Access::NativeQualityDeficitShare(second) == 99 &&
        Access::ConnectionCeiling(first) == firstCeiling && Access::ConnectionCeiling(second) == secondCeiling &&
        Access::Bootstrap(first) == firstBootstrap && Access::Bootstrap(second) == secondBootstrap &&
        Access::Epoch(first) == firstEpoch && Access::Epoch(second) == secondEpoch &&
        Matches(Access::Parameters(first), 1920, 1080, 120, 37'324'800) &&
        Matches(Access::Parameters(second), 1280, 720, 30, 829'440),
        "NATIVE_QUALITY_DEFICIT_SHARE_LIVE_EDITS_CLAMP_INDEPENDENTLY_WITHOUT_BWE_RTP_OR_POLICY_RESET");
    first.SetScreenQualityDeficitShare(27);
    ok &= Check(first.SetVideoSlotSendingActive(kScreenMainVideoSlot, false).ok() &&
        first.SetVideoSlotSendingActive(kScreenMainVideoSlot, true).ok() &&
        Access::NativeQualityDeficitShare(first) == 27 &&
        Access::NativeQualityDeficitShare(second) == 99,
        "NATIVE_QUALITY_DEFICIT_SHARE_SURVIVES_SCREEN_STOP_RESTART_WITHOUT_CROSS_CONNECTION_CHANGE");
    ok &= Check(Access::NativeQualityTarget(first) == std::pair{120u, 37'324'800u} &&
        Access::NativeQualityTarget(second) == std::pair{30u, 829'440u} &&
        Access::NativeQualityProtectionEnabled(first) && Access::NativeQualityProtectionEnabled(second),
        "NATIVE_QUALITY_PROFILES_ARE_PER_CONNECTION_WITH_DISTINCT_USER_FPS_AND_BPP");
    first.SetScreenVideoBitrateBpp(3);
    ok &= Check(Access::ApplyPendingBpp(first) &&
        Access::NativeQualityTarget(first) == std::pair{120u, 7'464'960u} &&
        Access::NativeQualityTarget(second) == std::pair{30u, 829'440u} &&
        Matches(Access::Parameters(first), 1920, 1080, 120, 7'464'960) &&
        Matches(Access::Parameters(second), 1280, 720, 30, 829'440),
        "NATIVE_QUALITY_LIVE_BPP_PROFILE_CHANGE_NEVER_CONTAMINATES_ANOTHER_CONNECTION");
    ok &= Check(first.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot, 30, 1920, 1080).ok() &&
        Access::NativeQualityTarget(first) == std::pair{30u, 1'866'240u} &&
        Access::NativeQualityTarget(second) == std::pair{30u, 829'440u} &&
        Matches(Access::Parameters(first), 1920, 1080, 30, 1'866'240) &&
        Access::NativeQualityDeficitShare(first) == 27 && Access::NativeQualityDeficitShare(second) == 99,
        "NATIVE_QUALITY_USER_FPS_EDIT_PUBLISHES_CURRENT_TARGET_WITHOUT_ENCODER_RESET_ASSUMPTION");
    first.SetScreenContentPolicyObservation(Observation(70, ScreenScene::kUnknown));
    ok &= Check(!Access::NativeQualityProtectionEnabled(first) &&
        Access::NativeQualityProtectionEnabled(second) &&
        Access::NativeQualityDeficitShare(first) == 27 && Access::NativeQualityDeficitShare(second) == 99,
        "NATIVE_QUALITY_CONTENT_MODE_SWITCH_IS_PER_CONNECTION");
    first.Close();
    ok &= Check(!Access::NativeQualityProtectionEnabled(first) &&
        Access::NativeQualityTarget(first) == std::pair{0u, 0u} &&
        Access::NativeQualityProtectionEnabled(second) &&
        Access::NativeQualityDeficitShare(first) == 27 && Access::NativeQualityDeficitShare(second) == 99,
        "NATIVE_QUALITY_CLOSE_RETIRES_ONLY_ITS_OWN_MODE_AND_PROFILE");
    second.Close();
    return ok;
}

bool RunLowBppOriginal120Recovery(LibWebRtcSession& session)
{
    session.SetScreenVideoBitrateBppProvider([] { return 3u; });
    session.Start({});
    if (!Check(session.PrepareVideoTransceiverSlot(kScreenMainVideoSlot).ok() &&
        session.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot, 120, 1920, 1080).ok() &&
        session.SetVideoSlotSendingActive(kScreenMainVideoSlot, true).ok(),
        "LOW_BPP_ORIGINAL_120_FPS_RECOVERY_REAL_RTP_READY")) return false;
    const auto user = Access::Binding(session);
    const auto bootstrap = Access::Bootstrap(session);
    const auto connectionCeiling = Access::ConnectionCeiling(session);
    session.SetScreenContentPolicyReferenceEnabled(true, "medium");
    auto evidence = EncoderEvidence();
    evidence.rtpStreams.front().encoderImplementation = "FFmpeg/NVENC";
    evidence.rtpStreams.front().windowQp = 50;
    evidence.rtpStreams.front().encodedFramesPerSecond = 120;
    evidence.rtpStreams.front().sentFramesPerSecond = 120;
    evidence.transport.availableOutgoingBitrateBps = 560'000;
    const auto sample = [&] {
        evidence.transport.receivedAtSteadyMs = Now();
        session.SetScreenContentPolicyObservation(EvidenceObservation());
        Access::Sample(session, evidence);
    };
    const auto matchCurrentEvidence = [&] {
        const auto current = Access::Binding(session);
        evidence.rtpStreams.front().frameWidth = current.effectiveWidth;
        evidence.rtpStreams.front().frameHeight = current.effectiveHeight;
        evidence.rtpStreams.front().encodedFramesPerSecond = current.effectiveFps;
        evidence.rtpStreams.front().sentFramesPerSecond = current.effectiveFps;
        CompleteEvidenceWindowForTest(session);
    };
    sample();
    CompleteEvidenceWindowForTest(session);
    std::this_thread::sleep_for(milliseconds(2)); sample();
    std::this_thread::sleep_for(milliseconds(1050)); sample();
    bool ok = Check(Access::Binding(session).effectiveFps == 100 &&
        Access::Binding(session).execution.successfulChanges == 1 &&
        Access::Binding(session).userBitrate == 7'464'960 &&
        Access::Binding(session).userFps == 120,
        "LOW_BPP_120_FPS_EMERGENCY_REDUCES_ONE_ADJACENT_STEP_WITH_USER_CEILING_UNCHANGED");
    matchCurrentEvidence();
    std::this_thread::sleep_for(milliseconds(2)); sample();
    std::this_thread::sleep_for(milliseconds(1050)); sample();
    ok &= Check(Access::Binding(session).effectiveFps == 90 &&
        Access::Binding(session).execution.successfulChanges == 2 &&
        Access::Binding(session).userBitrate == user.userBitrate &&
        Access::Binding(session).userFps == 120,
        "SUSTAINED_CONGESTION_REQUIRES_NEW_CONFIRMATION_FOR_NEXT_100_TO_90_FPS_STEP");
    matchCurrentEvidence();
    evidence.transport.availableOutgoingBitrateBps = user.userBitrate;
    std::this_thread::sleep_for(milliseconds(2)); sample();
    ok &= Check(Access::Shadow(session).reason != "user_specification_restore" &&
        Access::Binding(session).effectiveFps < 120,
        "FULL_USER_VIDEO_BUDGET_WITH_ACTIVE_GCC_PRESSURE_DOES_NOT_AUTHORIZE_ORIGINAL_FPS_RECOVERY");
    // The connection can now sustain the exact user video ceiling; the queue
    // signal is normal, with no pushback. Bad QP and a >40-Mbps reference curve
    // remain explicit diagnostics, not a permanent lock below user 120 FPS.
    evidence.transport.googCc.delayState = "normal";
    std::this_thread::sleep_for(milliseconds(2)); sample();
    const auto recovering = Access::Shadow(session);
    ok &= Check(recovering.reason == "user_specification_restore" &&
        recovering.senderMaxFps == 120 && recovering.width == 1920 &&
        recovering.requiredVideoBitrateBps > 40'000'000 &&
        !recovering.estimatedFeasible && !Access::QualityVerified(session) &&
        Access::Binding(session).execution.eligible &&
        Access::Binding(session).execution.status == "observing_user_restore" &&
        recovering.senderMaxBitrateBps == 7'091'712,
        "FRESH_NORMAL_GCC_AT_USER_CEILING_PERMITS_120_FPS_RESTORE_WITH_BAD_QP_RECORDED_HONESTLY");
    std::this_thread::sleep_for(milliseconds(1050)); sample();
    ok &= Check(Access::Binding(session).effectiveFps < 120 &&
        Access::Binding(session).execution.confirmObservedSamples == 2,
        "LOW_BPP_USER_RESTORE_STILL_REQUIRES_THREE_FRESH_WINDOWS");
    std::this_thread::sleep_for(milliseconds(1050)); sample();
    const auto restored = Access::Binding(session);
    ok &= Check(restored.effectiveWidth == 1920 && restored.effectiveHeight == 1080 &&
        restored.effectiveFps == 120 && restored.effectiveBitrate == 7'091'712 &&
        restored.effectiveBitrate <= user.userBitrate &&
        restored.execution.status == "user_specification_restored" &&
        !Access::Shadow(session).estimatedFeasible && !Access::QualityVerified(session) &&
        Access::Bootstrap(session) == bootstrap &&
        Access::ConnectionCeiling(session) == connectionCeiling &&
        Matches(Access::Parameters(session), 1920, 1080, 120, 7'091'712),
        "LOW_BPP_ORIGINAL_120_FPS_RESTORES_REAL_RTP_WITHOUT_QUALITY_CLAIM_OR_EXTRA_PROBE");
    matchCurrentEvidence();
    std::this_thread::sleep_for(milliseconds(2)); sample();
    std::this_thread::sleep_for(milliseconds(2)); sample();
    ok &= Check(Access::Binding(session).effectiveFps == 120 &&
        Access::Binding(session).execution.successfulChanges == restored.execution.successfulChanges &&
        Access::Binding(session).execution.status == "healthy_hold" &&
        !Access::QualityVerified(session) && !Access::Shadow(session).estimatedFeasible &&
        Matches(Access::Parameters(session), 1920, 1080, 120, restored.effectiveBitrate),
        "RESTORED_USER_120_FPS_STAYS_STABLE_WITH_BAD_QP_UNTIL_FRESH_GCC_PRESSURE_OR_BUDGET_CHANGE");
    return ok;
}

bool RunReference(LibWebRtcSession& session)
{
    // Exercise reference repair with an explicitly roomy user choice. The
    // lower/default choices are checked separately by RunConnectionReferenceBpp.
    session.SetScreenVideoBitrateBppProvider([] { return 30u; });
    session.Start({});
    bool ok = Check(session.PrepareVideoTransceiverSlot(kScreenMainVideoSlot).ok() &&
        session.SetVideoSlotEncodingPolicy(kScreenMainVideoSlot,60,1920,1080).ok() &&
        session.SetVideoSlotSendingActive(kScreenMainVideoSlot,true).ok(), "REFERENCE_REAL_RTP_READY");
    if (!ok) return false;
    auto evidence = EncoderEvidence();
    evidence.rtpStreams.front().encoderImplementation = "FFmpeg/QSV";
    evidence.transport.googCc.delayState = "normal";
    evidence.rtpStreams.front().sentFramesPerSecond = 60;
    // The reference code/60 requirement is 21.46M; this capacity's 95%
    // budget covers code/45 (16.10M) but cannot cover code/60.
    evidence.transport.availableOutgoingBitrateBps = 20'000'000;
    const auto sample = [&]() {
        auto current = evidence;
        current.transport.receivedAtSteadyMs = Now();
        session.SetScreenContentPolicyObservation(EvidenceObservation());
        Access::Sample(session,current);
    };
    session.SetScreenSenderFeedbackContract(10,7);
    sample();
    ok &= Check(!Access::Binding(session).execution.eligible, "REFERENCE_OFF_REMAINS_OBSERVATION");
    session.SetScreenContentPolicyReferenceEnabled(true,"medium");
    sample();
    CompleteEvidenceWindowForTest(session);
    {
        auto unknown = EvidenceObservation();
        unknown.scene = ScreenScene::kUnknown;
        session.SetScreenContentPolicyObservation(unknown);
        auto current = evidence;
        current.transport.receivedAtSteadyMs = Now();
        Access::Sample(session, current);
        ok &= Check(Access::Binding(session).execution.status == "awaiting_scene" &&
            !Access::Binding(session).execution.eligible,
            "UNKNOWN_SCENE_DOES_NOT_MASQUERADE_AS_UNCALIBRATED_MODEL");
        auto starting = EvidenceObservation();
        starting.activity = ScreenContentActivity::kStarting;
        session.SetScreenContentPolicyObservation(starting);
        Access::Sample(session, current);
        ok &= Check(Access::Binding(session).execution.status == "awaiting_activity" &&
            Access::Shadow(session).modelReference,
            "STARTING_CAPTURE_REPORTS_ACTIVITY_WAIT_WITH_REFERENCE_MODEL");
    }
    sample();
    const auto healthy = Access::Shadow(session);
    ok &= Check(healthy.width == 1920 && healthy.height == 1080 &&
        healthy.senderMaxFps == 60 && healthy.reason == "healthy_hold" &&
        !Access::Binding(session).execution.applied &&
        Matches(Access::Parameters(session),1920,1080,60,Access::Binding(session).userBitrate),
        "HEALTHY_REAL_RTP_PRESERVED_DESPITE_REFERENCE_BUDGET_SHORTAGE");
    evidence.rtpStreams.front().qualityLimitationReason = "bandwidth";
    sample();
    ok &= Check(Access::Shadow(session).senderMaxFps == 60 &&
        Access::Shadow(session).reason == "healthy_hold" &&
        !Access::Binding(session).execution.applied,
        "BANDWIDTH_LABEL_WITHOUT_GCC_CONGESTION_DOES_NOT_DOWNGRADE");
    // Start a real GoogCC congestion episode only after the independent healthy
    // checks. Do not erase an earlier episode just to manufacture recovery.
    evidence.transport.googCc.delayState = "overuse";
    evidence.rtpStreams.front().qualityLimitationReason = "none";
    sample();
    ok &= Check(Access::Binding(session).execution.eligible && !Access::Binding(session).execution.applied &&
        Access::Binding(session).execution.networkPressure &&
        Access::Binding(session).execution.networkTrigger == "delay_overuse",
        "REFERENCE_GCC_OVERUSE_AT_FULL_FPS_WITHOUT_BANDWIDTH_LABEL_PERMITS_REPAIR");
    ok &= Check(session.AcceptScreenReceiverFeedback(ReceiverEvidence(20)), "REFERENCE_RECEIVER_ACCEPTED");
    evidence.rtpStreams.front().windowQpAvailable=false;
    sample();
    ok &= Check(Access::Binding(session).execution.eligible && !Access::QualityVerified(session) &&
        Access::Shadow(session).senderMaxFps < Access::Binding(session).effectiveFps,
        "REFERENCE_GCC_OVERUSE_WITHOUT_QP_PERMITS_NETWORK_REPAIR_WITHOUT_QUALITY_CLAIM");
    evidence.rtpStreams.front().windowQpAvailable=true;
    evidence.rtpStreams.front().windowQp=35;
    sample();
    ok &= Check(Access::Binding(session).execution.eligible &&
        !Access::Binding(session).execution.applied &&
        Access::Binding(session).execution.status == "observing_repair",
        "REFERENCE_BAD_QP_PERMITS_REPAIR_WITHOUT_CLAIMING_QUALITY_PASS");
    evidence.rtpStreams.front().windowQp=24;
    std::this_thread::sleep_for(milliseconds(2));
    sample();
    ok &= Check(Access::Binding(session).execution.eligible && !Access::Binding(session).execution.applied &&
        Access::Shadow(session).modelReference && !Access::Shadow(session).modelCalibrated,
        "REFERENCE_TYPED_EVIDENCE_CONFIRMS_APPROXIMATE_PROVENANCE");
    std::this_thread::sleep_for(milliseconds(1050));
    session.AcceptScreenReceiverFeedback(ReceiverEvidence(21));
    sample();
    const auto applied=Access::Shadow(session);
    ok &= Check(Access::Binding(session).execution.applied &&
        Matches(Access::Parameters(session),applied.width,applied.height,applied.senderMaxFps,applied.senderMaxBitrateBps) &&
        applied.width==1920 && applied.height==1080 && applied.senderMaxFps==50 &&
        Access::Binding(session).userFps==60, "REFERENCE_QSV_APPLIES_REAL_RTP_WITH_USER_LIMIT");
    session.SetScreenContentPolicyReferenceEnabled(false,"medium");
    sample();
    ok &= Check(!Access::Binding(session).execution.applied, "REFERENCE_DISABLE_RESTORES");
    evidence.rtpStreams.front().encoderImplementation="unknown-backend";
    session.SetScreenContentPolicyReferenceEnabled(true,"medium");
    sample(); CompleteEvidenceWindowForTest(session);
    session.AcceptScreenReceiverFeedback(ReceiverEvidence(22)); sample();
    ok &= Check(!Access::Binding(session).execution.eligible && !Access::Shadow(session).modelReference,
        "REFERENCE_UNKNOWN_BACKEND_BLOCKED");
    auto measured = std::make_shared<CalibratedStreamQualityModel>();
    const std::array measuredSamples{StreamQualityCalibrationSample{
        ScreenScene::kCodeTerminal,1920,1080,60,12'000'000,30,true,20}};
    ok &= Check(measured->Build(measuredSamples,{"video/H264","FFmpeg/QSV","medium"},
        "TEST/priority-fixture-only")==StreamCalibrationError::kNone,"REFERENCE_PRIORITY_FIXTURE_READY");
    session.SetScreenContentPolicyCalibration(measured,"medium");
    evidence.rtpStreams.front().encoderImplementation="FFmpeg/QSV";
    sample(); CompleteEvidenceWindowForTest(session);
    session.AcceptScreenReceiverFeedback(ReceiverEvidence(23)); sample();
    ok &= Check(Access::Shadow(session).modelCalibrated && !Access::Shadow(session).modelReference &&
        Access::Binding(session).execution.status == "observing_repair",
        "MATCHED_MEASUREMENT_REPAIRS_WITHOUT_USING_LOOSER_REFERENCE");
    evidence.rtpStreams.front().encoderImplementation="FFmpeg/AMF";
    sample(); CompleteEvidenceWindowForTest(session);
    session.AcceptScreenReceiverFeedback(ReceiverEvidence(24)); sample();
    ok &= Check(Access::Shadow(session).modelReference && !Access::Shadow(session).modelCalibrated,
        "MISMATCHED_MEASUREMENT_FALLS_BACK_TO_AMF_REFERENCE");
    // Model a retained allocation from the preceding weak-network period.
    // A stream already at its user video ceiling cannot legitimately grow
    // beyond that ceiling merely because QP is poor.
    const auto retained = Access::Binding(session);
    ContentAwareStreamInput repairInput;
    repairInput.generation = 10;
    ContentAwareStreamEvaluation retainedAllocation;
    retainedAllocation.confirmed = true;
    retainedAllocation.recommendation = {.width = retained.effectiveWidth,
        .height = retained.effectiveHeight, .senderMaxFps = retained.effectiveFps,
        .desiredVideoBitrateBps = 18'000'000, .senderMaxBitrateBps = 18'000'000,
        .hasRecommendation = true, .estimatedFeasible = true, .modelCalibrated = true,
        .qualityRequirementMet = true, .automaticControlEligible = true};
    ok &= Check(Access::Apply(session, repairInput, retainedAllocation, retained.revision).ok() &&
        Matches(Access::Parameters(session), retained.effectiveWidth, retained.effectiveHeight,
            retained.effectiveFps, 18'000'000),
        "BITRATE_REPAIR_FIXTURE_RETAINS_WEAK_NETWORK_ALLOCATION_BELOW_USER_CEILING");
    CompleteEvidenceWindowForTest(session);
    const auto beforeRepair = Access::Binding(session);
    evidence.transport.availableOutgoingBitrateBps = 80'000'000;
    evidence.rtpStreams.front().windowQp = 35;
    session.AcceptScreenReceiverFeedback(ReceiverEvidence(25));
    std::this_thread::sleep_for(milliseconds(2));
    sample();
    const auto bitrateRepair = Access::Shadow(session);
    ok &= Check(bitrateRepair.width == beforeRepair.effectiveWidth &&
        bitrateRepair.height == beforeRepair.effectiveHeight &&
        bitrateRepair.senderMaxFps == beforeRepair.effectiveFps &&
        bitrateRepair.desiredVideoBitrateBps > beforeRepair.effectiveBitrate &&
        bitrateRepair.desiredVideoBitrateBps <= bitrateRepair.estimatedSafeVideoBudgetBps &&
        !Access::QualityVerified(session) &&
        Access::Binding(session).execution.status == "observing_repair",
        "GOOD_NETWORK_BAD_QP_RECOVERS_BITRATE_BEFORE_SACRIFICING_R_OR_F");
    std::this_thread::sleep_for(milliseconds(1050));
    session.AcceptScreenReceiverFeedback(ReceiverEvidence(26));
    sample();
    ok &= Check(Access::Binding(session).execution.status == "repair_applied" &&
        Matches(Access::Parameters(session), bitrateRepair.width, bitrateRepair.height,
            bitrateRepair.senderMaxFps, bitrateRepair.senderMaxBitrateBps) &&
        Access::Binding(session).userFps == 60 && !Access::QualityVerified(session),
        "BITRATE_REPAIR_WRITES_REAL_RTP_WITHOUT_CLAIMING_VISUAL_SUCCESS");
    CompleteEvidenceWindowForTest(session);
    evidence.rtpStreams.front().windowQp = 24;
    session.AcceptScreenReceiverFeedback(ReceiverEvidence(27));
    sample();
    ok &= Check(Access::QualityVerified(session),
        "REPAIR_IS_REVALIDATED_BY_FRESH_QP_AND_RECEIVER_WINDOW");
    return ok;
}
}  // namespace

int main()
{
    WebRtcRuntime runtime(VideoEncoderPreference::kSoftwareOnly);
    if (!Check(runtime.Initialize(), "WEBRTC_RUNTIME_INITIALIZED")) return 1;
    bool ok = RunDesktopSinkFrameRateDelivery(false) && RunDesktopSinkFrameRateDelivery(true);
    ok = RunDesktopJitterDelivery(false) && ok;
    ok = RunDesktopJitterDelivery(true) && ok;
    ok = RunDesktopJitterDelivery(false, false) && ok;
    ok = RunDesktopJitterDelivery(true, false) && ok;
    ok = RunNativeQualityProfileIsolation(runtime) && ok;
    {
        LibWebRtcSession session(runtime.PeerConnectionFactory());
        ok = RunConnectionReferenceBpp(session) && ok;
        session.Close();
    }
    {
        LibWebRtcSession session(runtime.PeerConnectionFactory());
        ok = RunDynamicBpp(session) && ok;
        session.Close();
    }
    {
        LibWebRtcSession session(runtime.PeerConnectionFactory());
        ok = Run(session) && ok;
        session.Close();
    }
    {
        LibWebRtcSession session(runtime.PeerConnectionFactory());
        ok = RunEvidence(session) && ok;
        session.Close();
    }
    for (const bool slowProcessing : {false, true}) {
        LibWebRtcSession session(runtime.PeerConnectionFactory());
        ok = RunProcessingTelemetry(session, slowProcessing) && ok;
        session.Close();
    }
    {
        LibWebRtcSession session(runtime.PeerConnectionFactory());
        ok = RunVerifiedUserRecovery(session) && ok;
        session.Close();
    }
    for (const bool spatialRecovery : {false, true}) {
        LibWebRtcSession session(runtime.PeerConnectionFactory());
        ok = RunRecoveryBudgetRamp(session, spatialRecovery) && ok;
        session.Close();
    }
    {
        LibWebRtcSession session(runtime.PeerConnectionFactory());
        ok = RunEmergencyNetworkReduction(session) && ok;
        session.Close();
    }
    {
        LibWebRtcSession session(runtime.PeerConnectionFactory());
        ok = RunDisableRestoresOriginal120(session) && ok;
        session.Close();
    }
    {
        LibWebRtcSession session(runtime.PeerConnectionFactory());
        ok = RunLowBppOriginal120Recovery(session) && ok;
        session.Close();
    }
    {
        LibWebRtcSession session(runtime.PeerConnectionFactory());
        ok = RunReference(session) && ok;
        session.Close();
    }
    runtime.Shutdown();
    std::cout << "CONTENT_AWARE_STREAM_EXECUTION=" << (ok ? "PASS" : "FAIL") << '\n';
    return ok ? 0 : 1;
}
