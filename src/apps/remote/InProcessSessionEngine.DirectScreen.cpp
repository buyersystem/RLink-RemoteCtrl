// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "InProcessSessionEngine.h"

#include <algorithm>
#include <limits>
#include <thread>
#include <utility>

#include "ScreenShareCoordinator.h"
#include "src/platform/win/WindowsDesktopCaptureSource.h"
#include "src/platform/win/WindowsDisplayTopology.h"
#include "src/protocol/DataChannelCatalog.h"
#include "src/protocol/ScreenShareControlProtocol.h"
#include "src/webrtc/LibWebRtcSession.h"
#include "src/webrtc/WebRtcRuntime.h"

namespace remote::app {
namespace {

SessionCommandResult Success()
{
    return {true, {}, {}};
}

SessionCommandResult Failure(std::string code, std::string message)
{
    return {false, std::move(code), std::move(message)};
}

}  // namespace

SessionCommandResult InProcessSessionEngine::SetDirectScreenStreamPreference(
    const ScreenStreamPreferenceRequest& preference)
{
    // The legacy direct API already returned queue acceptance. Keep that
    // behavior while sharing validation and state with the completion API.
    return QueueDirectScreenStreamPreference(preference, {});
}

SessionCommandResult InProcessSessionEngine::QueueDirectScreenStreamPreference(
    const ScreenStreamPreferenceRequest& preference,
    std::function<void(SessionCommandResult)> completion)
{
    const bool originalSize = preference.maxWidth == 0 &&
        preference.maxHeight == 0;
    const bool boundedSize = preference.maxWidth > 0 &&
        preference.maxHeight > 0 &&
        preference.maxWidth <= kMaximumScreenStreamWidth &&
        preference.maxHeight <= kMaximumScreenStreamHeight;
    if ((!originalSize && !boundedSize) ||
        preference.framesPerSecond < kMinimumScreenFrameRate ||
        preference.framesPerSecond > kMaximumScreenFrameRate) {
        return Failure("screen_stream_preference_invalid",
                       "The requested screen stream preference is invalid.");
    }

    SessionControllerBase* controller = nullptr;
    std::uint64_t sessionGeneration = 0;
    std::uint64_t shareGeneration = 0;
    ScreenStreamPreferenceRequest request = preference;
    {
        std::lock_guard lock(mutex_);
        if (!sessionController_ ||
            snapshot_.state != SessionEngineState::kActive ||
            snapshot_.purpose != SessionPurpose::kRemoteControl ||
            snapshot_.remoteControlRole != RemoteControlRole::kController ||
            snapshot_.sessionId.empty() ||
            !directSession_.IsChannelOpen(kControlReliableChannel)) {
            return Failure("direct_screen_stream_unavailable",
                           "The direct screen control channel is unavailable.");
        }
        if (request.framesPerSecond >
            snapshot_.direct.screenMaximumFrameRate) {
            return Failure("screen_stream_frame_rate_unsupported",
                           "The remote capture backend does not support the requested frame rate.");
        }
        if (directSession_.nextScreenControlSequence ==
            std::numeric_limits<std::uint64_t>::max()) {
            return Failure("screen_stream_sequence_exhausted",
                           "The screen control sequence is exhausted.");
        }
        request.roomId = snapshot_.sessionId;
        request.senderDeviceId = snapshot_.localDeviceId;
        request.sequence = ++directSession_.nextScreenControlSequence;
        snapshot_.direct.screenPreferencePending = true;
        snapshot_.direct.screenPreferenceSequence = request.sequence;
        // A previous ACK rejection must not be mistaken for this request's
        // rejection while its asynchronous send/ACK is still in flight.
        if (snapshot_.error.code == "direct_screen_preference_rejected") {
            snapshot_.error = {};
        }
        controller = sessionController_.get();
        sessionGeneration = directSessionGeneration_;
        shareGeneration = snapshot_.direct.remoteScreenShareGeneration;
    }
    const auto isCurrent =
        [this, controller, sessionId = request.roomId,
         localDeviceId = request.senderDeviceId, sessionGeneration, shareGeneration,
         sequence = request.sequence] {
            return sessionController_.get() == controller &&
                directSessionGeneration_ == sessionGeneration &&
                snapshot_.state == SessionEngineState::kActive &&
                snapshot_.purpose == SessionPurpose::kRemoteControl &&
                snapshot_.remoteControlRole == RemoteControlRole::kController &&
                !directSession_.sessionCloseRequested_ &&
                snapshot_.sessionId == sessionId &&
                snapshot_.localDeviceId == localDeviceId &&
                snapshot_.direct.remoteScreenShareGeneration == shareGeneration &&
                snapshot_.direct.screenPreferenceSequence == sequence;
        };
    const auto finish = [this, isCurrent](SendResult sendResult) {
        {
            std::lock_guard lock(mutex_);
            if (!isCurrent()) {
                return Failure("screen_stream_request_stale",
                               "The direct screen stream request has been superseded.");
            }
            if (sendResult != SendResult::kSent) {
                snapshot_.direct.screenPreferencePending = false;
            }
        }
        if (sendResult != SendResult::kSent) {
            PublishSnapshot();
        }
        switch (sendResult) {
        case SendResult::kSent:
            PublishSnapshot();
            return Success();
        case SendResult::kChannelNotFound:
            return Failure("direct_screen_stream_channel_not_found",
                           "The reliable control channel was not negotiated.");
        case SendResult::kChannelNotOpen:
            return Failure("direct_screen_stream_channel_not_open",
                           "The reliable control channel is not open.");
        case SendResult::kSessionNotStarted:
            return Failure("direct_screen_stream_session_not_started",
                           "The direct P2P session is not active.");
        case SendResult::kSendFailed:
            return Failure("direct_screen_stream_send_failed",
                           "WebRTC rejected the screen stream request.");
        }
        return Failure("direct_screen_stream_send_failed",
                       "The direct screen stream request could not be sent.");
    };
    std::vector<std::uint8_t> encoded;
    std::string error;
    if (!EncodeScreenStreamPreferenceRequest(request, &encoded, &error)) {
        (void)finish(SendResult::kSendFailed);
        return Failure("direct_screen_stream_encode_failed", error);
    }
    bool accepted = false;
    {
        // Holding the engine mutex across this nonblocking post protects the
        // uniquely owned direct controller from concurrent session disposal.
        std::lock_guard lock(mutex_);
        if (!isCurrent()) {
            return Failure("screen_stream_request_stale",
                           "The direct screen stream request has been superseded.");
        }
        accepted = controller->QueueData(
            std::string(kControlReliableChannel), encoded, true,
            [finish, completion = std::move(completion)](SendResult result) {
                const auto commandResult = finish(result);
                if (completion) {
                    completion(commandResult);
                }
            });
    }
    if (!accepted) {
        (void)finish(SendResult::kSessionNotStarted);
        return Failure("direct_screen_stream_send_failed",
                       "The screen stream request could not be queued.");
    }
    return Success();
}

SessionCommandResult InProcessSessionEngine::RequestDirectSharedDisplaySwitch(
    const std::string& stableDisplayKey)
{
    if (stableDisplayKey.empty()) {
        return Failure("shared_display_key_empty",
                       "A remote display must be selected.");
    }
    SessionControllerBase* controller = nullptr;
    SharedDisplaySwitchRequest request;
    {
        std::lock_guard lock(mutex_);
        if (!sessionController_ ||
            snapshot_.state != SessionEngineState::kActive ||
            snapshot_.remoteControlRole != RemoteControlRole::kController ||
            !snapshot_.direct.remoteDisplayCatalogReported ||
            !directSession_.IsChannelOpen(kControlReliableChannel)) {
            return Failure("shared_display_switch_not_authorized",
                           "The direct remote display list is unavailable.");
        }
        const auto selected = std::find_if(
            snapshot_.direct.remoteDisplays.begin(),
            snapshot_.direct.remoteDisplays.end(),
            [&stableDisplayKey](const DisplayDescriptor& display) {
                return display.stableDisplayKey == stableDisplayKey;
            });
        if (selected == snapshot_.direct.remoteDisplays.end()) {
            return Failure("shared_display_not_available",
                           "The selected remote display is no longer available.");
        }
        if (snapshot_.direct.remoteDisplay.stableDisplayKey ==
            stableDisplayKey) {
            return Success();
        }
        if (directSession_.nextScreenControlSequence ==
            std::numeric_limits<std::uint64_t>::max()) {
            return Failure("shared_display_sequence_exhausted",
                           "The screen control sequence is exhausted.");
        }
        request.roomId = snapshot_.sessionId;
        request.senderDeviceId = snapshot_.localDeviceId;
        request.sequence = ++directSession_.nextScreenControlSequence;
        request.screenShareGeneration =
            snapshot_.direct.remoteScreenShareGeneration;
        request.stableDisplayKey = stableDisplayKey;
        snapshot_.direct.remoteDisplaySwitchPending = true;
        snapshot_.direct.remoteDisplaySwitchSequence = request.sequence;
        snapshot_.direct.remoteDisplaySwitchError.clear();
        controller = sessionController_.get();
    }
    std::vector<std::uint8_t> encoded;
    std::string error;
    if (!EncodeSharedDisplaySwitchRequest(request, &encoded, &error) ||
        !controller->QueueData(
            std::string(kControlReliableChannel), encoded, true)) {
        std::lock_guard lock(mutex_);
        if (snapshot_.direct.remoteDisplaySwitchSequence ==
            request.sequence) {
            snapshot_.direct.remoteDisplaySwitchPending = false;
        }
        return Failure("shared_display_switch_send_failed",
                       error.empty()
                           ? "The remote display switch request could not be queued."
                           : error);
    }
    PublishSnapshot();
    return Success();
}

void InProcessSessionEngine::BroadcastDirectSharedDisplayCatalog()
{
    SharedDisplayCatalog catalog;
    SessionControllerBase* controller = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (!sessionController_ ||
            snapshot_.state != SessionEngineState::kActive ||
            snapshot_.remoteControlRole != RemoteControlRole::kControlled ||
            snapshot_.sessionId.empty() ||
            snapshot_.screenShare.generation == 0 ||
            snapshot_.screenShare.topology.layoutVersion == 0 ||
            snapshot_.screenShare.topology.displays.empty() ||
            !directSession_.IsChannelOpen(kControlReliableChannel)) {
            return;
        }
        catalog.roomId = snapshot_.sessionId;
        catalog.senderDeviceId = snapshot_.localDeviceId;
        catalog.screenShareGeneration = snapshot_.screenShare.generation;
        catalog.layoutVersion = snapshot_.screenShare.topology.layoutVersion;
        catalog.displays = snapshot_.screenShare.topology.displays;
        controller = sessionController_.get();
    }
    std::vector<std::uint8_t> encoded;
    if (EncodeSharedDisplayCatalog(catalog, &encoded)) {
        (void)controller->QueueData(
            std::string(kControlReliableChannel), encoded, true);
    }
}

SessionCommandResult InProcessSessionEngine::SwitchLocalDirectDisplay(
    const std::string& stableDisplayKey)
{
    const DisplayTopologySnapshot topology =
        EnumerateWindowsDisplayTopology();
    const DisplayDescriptor* selected =
        FindDisplayByStableKey(topology, stableDisplayKey);
    if (!selected) {
        return Failure("shared_display_not_available",
                       "The selected display is no longer attached.");
    }

    std::string sessionId;
    std::uint64_t nextGeneration = 0;
    std::uint64_t previousAdapterLuid = 0;
    std::uint32_t targetFrameRate = kDefaultScreenFrameRate;
    ScreenStreamPreferenceRequest preference;
    webrtc::scoped_refptr<WindowsDesktopCaptureSource> previousSource;
    webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> previousTrack;
    LibWebRtcSession* session = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (!webRtcSession_ || !directSession_.mediaSlotsPrepared ||
            snapshot_.state != SessionEngineState::kActive ||
            snapshot_.remoteControlRole != RemoteControlRole::kControlled ||
            !screenShare_.HasCaptureSource()) {
            return Failure("local_screen_share_not_active",
                           "The direct desktop capture is not active.");
        }
        if (snapshot_.screenShare.activeDisplay.stableDisplayKey ==
            stableDisplayKey) {
            return Success();
        }
        sessionId = snapshot_.sessionId;
        nextGeneration = screenShare_.NextGeneration();
        previousSource = screenShare_.CaptureSource();
        previousTrack = directSession_.screenTrack;
        previousAdapterLuid = previousSource->CaptureTarget().adapterLuid;
        session = webRtcSession_.get();
        if (directSession_.screenPreferenceApplied) {
            preference = directSession_.screenPreference;
            targetFrameRate = preference.framesPerSecond;
        }
    }

    auto replacement =
        webrtc::make_ref_counted<WindowsDesktopCaptureSource>(
            options_.desktopCaptureImplementation,
            *selected,
            options_.contentAnalyzerEnabled,
            options_.contentAnalyzerRateHz,
            options_.remoteVisionAnalyzer);
    if (!replacement->SetTargetFrameRate(targetFrameRate) ||
        !replacement->StartCapture()) {
        const std::string error = replacement->LastError();
        replacement->StopCapture();
        return Failure("shared_display_capture_start_failed",
                       error.empty()
                           ? "The selected display could not be captured."
                           : error);
    }
    const auto factory = runtime_ ? runtime_->PeerConnectionFactory()
                                  : nullptr;
    auto replacementTrack = factory ? factory->CreateVideoTrack(
        replacement,
        "direct-screen-switch-" + Snapshot().localDeviceId + "-" +
            std::to_string(nextGeneration)) : nullptr;
    if (!replacementTrack) {
        replacement->StopCapture();
        return Failure("direct_video_track_create_failed",
                       "The replacement desktop track could not be created.");
    }

    if (runtime_) {
        runtime_->SetDesktopCaptureAdapterLuid(selected->adapterLuid);
    }
    auto result = session->SetVideoSlotSendingActive(
        kScreenMainVideoSlot, false);
    if (result.ok()) {
        result = session->SetVideoSlotTrack(
            kScreenMainVideoSlot, replacementTrack);
    }
    const auto effective = ScreenShareCoordinator::ResolvePolicy(
        replacement->CapturedWidth(), replacement->CapturedHeight(),
        preference);
    if (result.ok()) {
        result = session->SetVideoSlotEncodingPolicy(
            kScreenMainVideoSlot, effective.framesPerSecond,
            effective.width, effective.height);
    }
    if (result.ok()) {
        result = session->SetVideoSlotSendingActive(
            kScreenMainVideoSlot, true);
    }
    if (!result.ok()) {
        if (runtime_) {
            runtime_->SetDesktopCaptureAdapterLuid(previousAdapterLuid);
        }
        (void)session->SetVideoSlotTrack(
            kScreenMainVideoSlot, previousTrack);
        (void)session->SetVideoSlotSendingActive(
            kScreenMainVideoSlot, true);
        replacement->StopCapture();
        return Failure("shared_display_track_replace_failed",
                       std::string(result.message()));
    }

    bool stillCurrent = false;
    {
        std::lock_guard lock(mutex_);
        stillCurrent = webRtcSession_.get() == session &&
            snapshot_.sessionId == sessionId &&
            snapshot_.state == SessionEngineState::kActive &&
            snapshot_.remoteControlRole == RemoteControlRole::kControlled &&
            screenShare_.CaptureSourceIs(previousSource.get());
        if (stillCurrent) {
            screenShare_.SetCaptureSource(replacement);
            directSession_.screenTrack = replacementTrack;
            screenShare_.CommitGeneration(nextGeneration);
            snapshot_.screenShare.generation = nextGeneration;
            snapshot_.screenShare.topology = topology;
            snapshot_.screenShare.selectedDisplayKey = selected->stableDisplayKey;
            snapshot_.screenShare.activeDisplay = *selected;
            snapshot_.screenShare.activeDisplayLayoutVersion =
                topology.layoutVersion;
            if (runtime_) {
                runtime_->SetDesktopShareGeneration(nextGeneration);
            }
        }
    }
    if (!stillCurrent) {
        (void)session->SetVideoSlotTrack(
            kScreenMainVideoSlot, previousTrack);
        (void)session->SetVideoSlotSendingActive(
            kScreenMainVideoSlot, true);
        replacement->StopCapture();
        return Failure("shared_display_switch_superseded",
                       "The direct session changed while switching displays.");
    }
    StartRemoteCursorPublishing(*selected, topology.layoutVersion);
    replacement->RequestStartupFrameBurst();
    std::jthread stopThread(
        [capture = std::move(previousSource)](std::stop_token) {
            capture->StopCapture();
        });
    {
        std::lock_guard lock(mutex_);
        retiredDesktopStopThreads_.push_back(std::move(stopThread));
    }
    PublishSnapshot();
    BroadcastDirectSharedDisplayLayout();
    BroadcastDirectSharedDisplayCatalog();
    return Success();
}

bool InProcessSessionEngine::DispatchDirectScreenData(
    const std::string& label,
    std::span<const std::uint8_t> payload)
{
    if (label != kControlReliableChannel) {
        return false;
    }

    SharedDisplayCatalog catalog;
    if (DecodeSharedDisplayCatalog(payload, &catalog)) {
        {
            std::lock_guard lock(mutex_);
            if (snapshot_.state != SessionEngineState::kActive ||
                snapshot_.remoteControlRole != RemoteControlRole::kController ||
                catalog.roomId != snapshot_.sessionId ||
                catalog.senderDeviceId != snapshot_.peerDeviceId ||
                catalog.screenShareGeneration == 0) {
                return true;
            }
            snapshot_.direct.remoteDisplayCatalogReported = true;
            snapshot_.direct.remoteDisplayCatalogLayoutVersion =
                catalog.layoutVersion;
            if (snapshot_.direct.remoteScreenShareGeneration !=
                    catalog.screenShareGeneration) {
                // The old send completion and ACK belong to another capture
                // generation. Keep the applied policy, but release its wait.
                snapshot_.direct.screenPreferencePending = false;
            }
            snapshot_.direct.remoteScreenShareGeneration =
                catalog.screenShareGeneration;
            snapshot_.direct.remoteDisplays = std::move(catalog.displays);
        }
        PublishSnapshot();
        return true;
    }

    SharedDisplaySwitchApplied switchApplied;
    if (DecodeSharedDisplaySwitchApplied(payload, &switchApplied)) {
        {
            std::lock_guard lock(mutex_);
            if (switchApplied.roomId != snapshot_.sessionId ||
                switchApplied.senderDeviceId != snapshot_.peerDeviceId ||
                !snapshot_.direct.remoteDisplaySwitchPending ||
                switchApplied.requestSequence !=
                    snapshot_.direct.remoteDisplaySwitchSequence) {
                return true;
            }
            snapshot_.direct.remoteDisplaySwitchPending = false;
            snapshot_.direct.remoteDisplaySwitchError =
                switchApplied.accepted ? std::string{} : switchApplied.error;
        }
        PublishSnapshot();
        return true;
    }

    SharedDisplaySwitchRequest switchRequest;
    if (DecodeSharedDisplaySwitchRequest(payload, &switchRequest)) {
        SessionControllerBase* controller = nullptr;
        bool authorized = false;
        {
            std::lock_guard lock(mutex_);
            if (switchRequest.roomId != snapshot_.sessionId ||
                switchRequest.senderDeviceId != snapshot_.peerDeviceId ||
                switchRequest.sequence <=
                    directSession_.lastScreenControlSequence) {
                return true;
            }
            directSession_.lastScreenControlSequence =
                switchRequest.sequence;
            authorized = sessionController_ &&
                snapshot_.state == SessionEngineState::kActive &&
                snapshot_.remoteControlRole == RemoteControlRole::kControlled &&
                switchRequest.screenShareGeneration ==
                    snapshot_.screenShare.generation;
            controller = sessionController_.get();
        }
        const auto result = authorized
            ? SwitchLocalDirectDisplay(switchRequest.stableDisplayKey)
            : Failure("shared_display_switch_not_authorized",
                      "The direct display switch request is stale or unauthorized.");
        SharedDisplaySwitchApplied response;
        response.roomId = switchRequest.roomId;
        response.senderDeviceId = Snapshot().localDeviceId;
        response.requestSequence = switchRequest.sequence;
        response.screenShareGeneration =
            switchRequest.screenShareGeneration;
        response.stableDisplayKey = switchRequest.stableDisplayKey;
        response.accepted = result.accepted;
        response.error = result.accepted ? std::string{} : result.errorMessage;
        std::vector<std::uint8_t> encoded;
        if (controller && EncodeSharedDisplaySwitchApplied(response, &encoded)) {
            (void)controller->QueueData(
                std::string(kControlReliableChannel), encoded, true);
        }
        return true;
    }

    ScreenCaptureCapability capability;
    if (DecodeScreenCaptureCapability(payload, &capability)) {
        {
            std::lock_guard lock(mutex_);
            if (capability.roomId != snapshot_.sessionId ||
                capability.senderDeviceId != snapshot_.peerDeviceId) {
                return true;
            }
            snapshot_.direct.screenMaximumFrameRate =
                capability.maximumFrameRate;
        }
        PublishSnapshot();
        return true;
    }

    ScreenStreamPreferenceApplied applied;
    if (DecodeScreenStreamPreferenceAppliedV2(payload, &applied) ||
        DecodeScreenStreamPreferenceApplied(payload, &applied)) {
        {
            std::lock_guard lock(mutex_);
            if (applied.roomId != snapshot_.sessionId ||
                applied.senderDeviceId != snapshot_.peerDeviceId ||
                !snapshot_.direct.screenPreferencePending ||
                applied.requestSequence !=
                    snapshot_.direct.screenPreferenceSequence ||
                (applied.screenShareGeneration != 0 &&
                 snapshot_.direct.remoteScreenShareGeneration != 0 &&
                 applied.screenShareGeneration !=
                    snapshot_.direct.remoteScreenShareGeneration)) {
                return true;
            }
            snapshot_.direct.screenPreferencePending = false;
            if (applied.accepted) {
                snapshot_.direct.screenPreferenceAcceptedSequence =
                    applied.requestSequence;
                snapshot_.direct.screenWidth = applied.width;
                snapshot_.direct.screenHeight = applied.height;
                snapshot_.direct.screenFramesPerSecond =
                    applied.framesPerSecond;
                snapshot_.direct.screenMaxBitrateBps =
                    applied.maxBitrateBps;
            } else {
                snapshot_.error.code = "direct_screen_preference_rejected";
                snapshot_.error.message = applied.error;
            }
        }
        PublishSnapshot();
        return true;
    }

    ScreenStreamPreferenceRequest request;
    if (DecodeScreenStreamPreferenceRequest(payload, &request)) {
        webrtc::scoped_refptr<WindowsDesktopCaptureSource> source;
        LibWebRtcSession* session = nullptr;
        SessionControllerBase* controller = nullptr;
        std::uint64_t generation = 0;
        std::uint32_t maximumFrameRate = kMaximumScreenFrameRate;
        bool authorized = false;
        {
            std::lock_guard lock(mutex_);
            if (request.roomId != snapshot_.sessionId ||
                request.senderDeviceId != snapshot_.peerDeviceId ||
                request.sequence <= directSession_.lastScreenControlSequence) {
                return true;
            }
            directSession_.lastScreenControlSequence = request.sequence;
            authorized = sessionController_ && webRtcSession_ &&
                snapshot_.state == SessionEngineState::kActive &&
                snapshot_.remoteControlRole == RemoteControlRole::kControlled &&
                directSession_.mediaSlotsPrepared &&
                screenShare_.HasCaptureSource();
            source = screenShare_.CaptureSource();
            session = webRtcSession_.get();
            controller = sessionController_.get();
            generation = snapshot_.screenShare.generation;
            maximumFrameRate = ScreenShareCoordinator::MaximumCaptureFrameRate(
                options_.desktopCaptureImplementation, source.get());
        }
        ScreenStreamPreferenceApplied response;
        response.roomId = request.roomId;
        response.senderDeviceId = Snapshot().localDeviceId;
        response.requestSequence = request.sequence;
        response.screenShareGeneration = generation;
        if (!authorized || request.framesPerSecond > maximumFrameRate) {
            response.error = "The direct screen preference is unavailable or unsupported.";
        } else {
            const auto effective = ScreenShareCoordinator::ResolvePolicy(
                source->CapturedWidth(), source->CapturedHeight(), request);
            const auto previousCaptureFrameRate = source->TargetFrameRate();
            const bool captureAccepted =
                source->SetTargetFrameRate(effective.framesPerSecond);
            ScreenStreamPolicyResult appliedPolicy;
            const auto policy = captureAccepted
                ? session->SetVideoSlotEncodingPolicy(
                      kScreenMainVideoSlot, effective.framesPerSecond,
                      effective.width, effective.height, &appliedPolicy)
                : webrtc::RTCError(webrtc::RTCErrorType::INVALID_STATE,
                                   "The desktop capturer rejected the frame rate.");
            response.accepted = policy.ok();
            response.width = effective.width;
            response.height = effective.height;
            response.framesPerSecond = effective.framesPerSecond;
            response.maxBitrateBps = policy.ok() ? appliedPolicy.maxBitrateBps : 0;
            response.scaleBackend = ScreenScaleBackend::kWebRtc;
            response.error = policy.ok() ? std::string{}
                                         : std::string(policy.message());
            if (policy.ok()) {
                std::lock_guard lock(mutex_);
                directSession_.screenPreferenceApplied = true;
                directSession_.screenPreference = request;
            } else if (captureAccepted) {
                bool restoreCapture = false;
                {
                    std::lock_guard lock(mutex_);
                    restoreCapture = screenShare_.CaptureSource() == source &&
                        snapshot_.screenShare.generation == generation &&
                        source->TargetFrameRate() == effective.framesPerSecond;
                }
                if (restoreCapture) {
                    (void)source->SetTargetFrameRate(previousCaptureFrameRate);
                }
            }
        }
        ScreenCaptureCapability captureCapability;
        captureCapability.roomId = request.roomId;
        captureCapability.senderDeviceId = response.senderDeviceId;
        captureCapability.maximumFrameRate = maximumFrameRate;
        std::vector<std::uint8_t> encoded;
        if (controller && EncodeScreenCaptureCapability(
                captureCapability, &encoded)) {
            (void)controller->QueueData(
                std::string(kControlReliableChannel), encoded, true);
        }
        if (controller && EncodeScreenStreamPreferenceAppliedV2(
                response, &encoded)) {
            (void)controller->QueueData(
                std::string(kControlReliableChannel), encoded, true);
        }
        if (controller && EncodeScreenStreamPreferenceApplied(
                response, &encoded)) {
            (void)controller->QueueData(
                std::string(kControlReliableChannel), encoded, true);
        }
        return true;
    }
    return false;
}

}  // namespace remote::app
