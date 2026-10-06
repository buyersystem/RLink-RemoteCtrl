// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "InProcessSessionEngine.h"

#include "src/platform/win/WindowsCursorMonitor.h"
#include "src/platform/win/WindowsDesktopCaptureSource.h"
#include "src/protocol/DataChannelCatalog.h"
#include "src/protocol/RemoteInputProtocol.h"
#include "src/protocol/ScreenShareControlProtocol.h"
#include "src/webrtc/WebRtcRuntime.h"

namespace remote::app {

void InProcessSessionEngine::OnRoomPairRemoteTrackAdded(
    const std::string& pairId,
    const RemoteTrackInfo& track)
{
    (void)pairId;
    (void)track;
}

bool InProcessSessionEngine::SendDescription(
    const SessionDescription& description)
{
    SignalingSessionDescription signalingDescription;
    ISignalingClient* signaling = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (!signaling_ || snapshot_.sessionId.empty() ||
            snapshot_.state == SessionEngineState::kStopping ||
            snapshot_.state == SessionEngineState::kStopped) {
            return false;
        }
        signaling = signaling_.get();
        signalingDescription.sessionId = snapshot_.sessionId;
    }
    signalingDescription.type =
        description.type == SessionDescriptionType::kOffer ? "offer"
                                                            : "answer";
    signalingDescription.sdp = description.sdp;
    signalingDescription.negotiationGeneration =
        description.negotiationGeneration;
    return signaling->SendDescription(signalingDescription).accepted;
}

bool InProcessSessionEngine::SendIceCandidate(
    const IceCandidate& candidate)
{
    SignalingIceCandidate signalingCandidate;
    ISignalingClient* signaling = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (!signaling_ || snapshot_.sessionId.empty() ||
            snapshot_.state == SessionEngineState::kStopping ||
            snapshot_.state == SessionEngineState::kStopped) {
            return false;
        }
        signaling = signaling_.get();
        signalingCandidate.sessionId = snapshot_.sessionId;
    }
    signalingCandidate.candidate = candidate.candidate;
    signalingCandidate.sdpMid = candidate.sdpMid;
    signalingCandidate.sdpMLineIndex = candidate.sdpMLineIndex;
    signalingCandidate.negotiationGeneration =
        candidate.negotiationGeneration;
    return signaling->SendIceCandidate(signalingCandidate).accepted;
}

bool InProcessSessionEngine::RequestIceRestart(
    std::uint64_t observedGeneration,
    std::uint64_t requestSequence)
{
    SignalingIceRestartRequest request;
    ISignalingClient* signaling = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (!signaling_ || snapshot_.sessionId.empty() ||
            snapshot_.connectivity != SessionConnectivityState::kOnline) {
            return false;
        }
        signaling = signaling_.get();
        request.sessionId = snapshot_.sessionId;
    }
    request.observedGeneration = observedGeneration;
    request.requestSequence = requestSequence;
    return signaling->SendIceRestartRequest(request).accepted;
}

bool InProcessSessionEngine::CancelIceRestart(
    std::uint64_t observedGeneration,
    std::uint64_t requestSequence)
{
    SignalingIceRestartCancel cancel;
    ISignalingClient* signaling = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (!signaling_ || snapshot_.sessionId.empty() ||
            snapshot_.connectivity != SessionConnectivityState::kOnline) {
            return false;
        }
        signaling = signaling_.get();
        cancel.sessionId = snapshot_.sessionId;
    }
    cancel.observedGeneration = observedGeneration;
    cancel.requestSequence = requestSequence;
    return signaling->SendIceRestartCancel(cancel).accepted;
}

void InProcessSessionEngine::OnControllerSnapshot(
    const SessionControllerSnapshot& controllerSnapshot)
{
    bool reapplyAudioDevices = false;
    bool startOffer = false;
    bool stopDirectCapture = false;
    std::string failedSessionIdToClose;
    std::string failedSessionReason;
    ISignalingClient* signaling = nullptr;
    ControllerSessionController* offerController = nullptr;
    std::vector<DataChannelSpec> offerChannels;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state == SessionEngineState::kStopping ||
            snapshot_.state == SessionEngineState::kStopped) {
            return;
        }
        snapshot_.direct.iceRestartAttempt =
            controllerSnapshot.iceRestartAttempt;
        switch (controllerSnapshot.state) {
        case SessionControllerState::kReady:
            if (!directSession_.audioDevicesApplied) {
                directSession_.audioDevicesApplied = true;
                reapplyAudioDevices = true;
            }
            if (directSession_.localIsOfferer_ &&
                !directSession_.offerNegotiationStarted_ &&
                sessionController_) {
                directSession_.offerNegotiationStarted_ = true;
                offerChannels =
                    snapshot_.purpose == SessionPurpose::kCameraOnly
                        ? DefaultCameraSessionDataChannels()
                        : DefaultRemoteControlDataChannels();
                offerController = static_cast<
                    ControllerSessionController*>(sessionController_.get());
                startOffer = true;
            }
            break;
        case SessionControllerState::kNegotiating:
        case SessionControllerState::kDisconnected:
        case SessionControllerState::kWaitingForSignaling:
        case SessionControllerState::kRestartingIce:
            snapshot_.state = SessionEngineState::kConnecting;
            break;
        case SessionControllerState::kConnected:
            snapshot_.state = SessionEngineState::kActive;
            snapshot_.direct.sessionEverActive = true;
            snapshot_.error.code.clear();
            snapshot_.error.message.clear();
            break;
        case SessionControllerState::kFailed:
            snapshot_.state = SessionEngineState::kFailed;
            snapshot_.error.code = controllerSnapshot.errorCode;
            snapshot_.error.message = controllerSnapshot.errorMessage;
            break;
        case SessionControllerState::kClosed: {
            stopDirectCapture =
                screenShare_.HasCaptureSource() ||
                directSession_.mediaSlotsPrepared;
            const bool preserveError =
                !directSession_.sessionCloseRequested_;
            const std::string errorCode =
                controllerSnapshot.errorCode.empty()
                    ? snapshot_.error.code
                    : controllerSnapshot.errorCode;
            const std::string errorMessage =
                controllerSnapshot.errorMessage.empty()
                    ? snapshot_.error.message
                    : controllerSnapshot.errorMessage;
            // A negotiation/recovery failure closes SessionController without
            // going through Disconnect().  The signaling session is already
            // active at this point, so explicitly close it before discarding
            // its identity.  Otherwise the server keeps both devices busy and
            // rejects the next assisted/owned-device request.
            if (preserveError && directSession_.serverSessionActive_ &&
                signaling_ && !snapshot_.sessionId.empty() &&
                !directSession_.sessionEndSignalSent_) {
                failedSessionIdToClose = snapshot_.sessionId;
                failedSessionReason = errorCode.empty()
                    ? "p2p_connection_failed"
                    : errorCode;
                signaling = signaling_.get();
                directSession_.sessionEndSignalSent_ = true;
            }
            ResetSessionStateLocked();
            if (preserveError) {
                snapshot_.error.code = errorCode;
                snapshot_.error.message = errorMessage;
            }
            break;
        }
        default:
            break;
        }
    }
    if (reapplyAudioDevices) {
        (void)runtime_->ReapplyPreferredAudioDevices();
        (void)RefreshLocalMediaDevices();
    }
    if (startOffer && offerController) {
        if (controllerSnapshot.state == SessionControllerState::kReady &&
            Snapshot().purpose == SessionPurpose::kRemoteControl) {
            if (const auto error = PrepareDirectMedia(false)) {
                {
                    std::lock_guard lock(mutex_);
                    snapshot_.state = SessionEngineState::kFailed;
                    snapshot_.error.code = error->code;
                    snapshot_.error.message = error->message;
                }
                PublishSnapshot();
                return;
            }
        }
        offerController->Connect(offerChannels);
    }
    if (stopDirectCapture) {
        StopDirectMicrophoneCapture();
        StopDirectDesktopCapture();
    }
    if (signaling && !failedSessionIdToClose.empty()) {
        (void)signaling->CloseSession(
            failedSessionIdToClose, failedSessionReason);
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnDataChannelStateChanged(
    const DataChannelInfo& channel)
{
    bool publishLayout = false;
    bool publishCatalog = false;
    bool requestLayout = false;
    bool republishCursor = false;
    {
        std::lock_guard lock(mutex_);
        directSession_.openDataChannels[channel.label] =
            channel.state == DataChannelState::kOpen;
        if (channel.label == kControlReliableChannel) {
            snapshot_.direct.controlReliableChannelOpen =
                channel.state == DataChannelState::kOpen;
            publishLayout =
                snapshot_.direct.controlReliableChannelOpen &&
                snapshot_.remoteControlRole ==
                    RemoteControlRole::kControlled &&
                snapshot_.screenShare.activeDisplay.sessionDisplayId != 0 &&
                snapshot_.screenShare.activeDisplayLayoutVersion != 0;
            requestLayout =
                snapshot_.direct.controlReliableChannelOpen &&
                snapshot_.remoteControlRole ==
                    RemoteControlRole::kController;
            publishCatalog =
                snapshot_.direct.controlReliableChannelOpen &&
                snapshot_.remoteControlRole ==
                    RemoteControlRole::kControlled;
        } else if (channel.label == kInputFastChannel) {
            snapshot_.direct.inputFastChannelOpen =
                channel.state == DataChannelState::kOpen;
        } else if (channel.label == kFileTransferChannel) {
            snapshot_.direct.fileTransferChannelOpen =
                channel.state == DataChannelState::kOpen;
        } else if (channel.label == kClipboardReliableChannel) {
            snapshot_.direct.clipboardReliableChannelOpen =
                channel.state == DataChannelState::kOpen;
        } else if (channel.label == kClipboardTransferChannel) {
            snapshot_.direct.clipboardTransferChannelOpen =
                channel.state == DataChannelState::kOpen;
        }
        republishCursor =
            channel.state == DataChannelState::kOpen &&
            (channel.label == kControlReliableChannel ||
             channel.label == kTelemetryChannel) &&
            snapshot_.remoteControlRole == RemoteControlRole::kControlled;
    }
    PublishSnapshot();
    if (publishLayout) {
        BroadcastDirectSharedDisplayLayout();
    }
    if (publishCatalog) {
        BroadcastDirectSharedDisplayCatalog();
    }
    if (requestLayout) {
        RequestDirectSharedDisplayLayout();
    }
    if (republishCursor) {
        RepublishRemoteCursor();
    }
}

void InProcessSessionEngine::OnDataMessage(
    const std::string& label,
    std::span<const std::uint8_t> payload,
    bool binary)
{
    if (!binary) {
        return;
    }
    if (DispatchScreenReceiverFeedback({}, label, payload)) return;
    if (DispatchRemoteCursorData({}, label, payload)) {
        return;
    }
    if (DispatchDirectAuxiliaryData(label, payload)) {
        return;
    }
    if (DispatchDirectScreenData(label, payload)) {
        return;
    }
    if (label == kControlReliableChannel) {
        ScreenRefreshRequest refreshRequest;
        if (DecodeScreenRefreshRequest(payload, &refreshRequest)) {
            webrtc::scoped_refptr<WindowsDesktopCaptureSource> source;
            {
                std::lock_guard lock(mutex_);
                if (snapshot_.remoteControlRole !=
                        RemoteControlRole::kControlled ||
                    refreshRequest.roomId != snapshot_.sessionId ||
                    refreshRequest.senderDeviceId !=
                        snapshot_.peerDeviceId) {
                    return;
                }
                source = screenShare_.CaptureSource();
            }
            if (source) {
                source->RequestStartupFrameBurst(4);
            }
            BroadcastDirectSharedDisplayLayout();
            return;
        }
        SharedDisplayLayout layout;
        if (DecodeSharedDisplayLayout(payload, &layout)) {
            {
                std::lock_guard lock(mutex_);
                if (snapshot_.remoteControlRole !=
                        RemoteControlRole::kController ||
                    layout.roomId != snapshot_.sessionId ||
                    layout.senderDeviceId != snapshot_.peerDeviceId ||
                    layout.screenShareGeneration == 0 ||
                    layout.layoutVersion == 0 ||
                    layout.selectedDisplay.sessionDisplayId == 0) {
                    return;
                }
                snapshot_.direct.remoteDisplay = layout.selectedDisplay;
                snapshot_.direct.remoteDisplayLayoutVersion =
                    layout.layoutVersion;
                snapshot_.direct.remoteScreenShareGeneration =
                    layout.screenShareGeneration;
            }
            PublishSnapshot();
            return;
        }
    }

    const bool fastChannel = label == kInputFastChannel;
    const bool reliableChannel = label == kControlReliableChannel;
    if (!fastChannel && !reliableChannel) {
        return;
    }
    RemoteInputEnvelope input;
    if (!DecodeRemoteInput(payload, &input)) {
        return;
    }
    const bool buttonTransition =
        input.event.type == RemoteInputMessageType::kMouseButton;
    if (!buttonTransition &&
        UsesFastInputChannel(input.event.type) != fastChannel) {
        return;
    }
    IRemoteInputSink* sink = nullptr;
    webrtc::scoped_refptr<WindowsDesktopCaptureSource> sourceToBoost;
    {
        std::lock_guard lock(mutex_);
        if (directSession_.sessionCloseRequested_ ||
            snapshot_.state != SessionEngineState::kActive ||
            snapshot_.remoteControlRole != RemoteControlRole::kControlled ||
            input.roomId != snapshot_.sessionId ||
            input.senderDeviceId != snapshot_.peerDeviceId ||
            input.controlGrantId != snapshot_.sessionId) {
            return;
        }
        if (IsPointerInput(input.event.type) &&
            (snapshot_.screenShare.activeDisplay.sessionDisplayId == 0 ||
             snapshot_.screenShare.activeDisplayLayoutVersion == 0 ||
             input.event.displayId !=
                 snapshot_.screenShare.activeDisplay.sessionDisplayId ||
             input.event.displayLayoutVersion !=
                 snapshot_.screenShare.activeDisplayLayoutVersion)) {
            return;
        }
        std::uint64_t& lastSequence = fastChannel
            ? directSession_.lastFastInputSequence
            : directSession_.lastReliableInputSequence;
        if (input.sequence <= lastSequence) {
            return;
        }
        lastSequence = input.sequence;
        if (buttonTransition ||
            input.event.type == RemoteInputMessageType::kReleaseAll) {
            if (input.sequence <= directSession_.lastPointerStateSequence) {
                return;
            }
            directSession_.lastPointerStateSequence = input.sequence;
        }
        input.event.deliverySequence = input.sequence;
        sink = remoteInputSink_;
        if (sink && ShouldBoostDesktopCaptureForInput(input.event)) {
            sourceToBoost = screenShare_.CaptureSource();
        }
    }
    if (sink) {
        {
            std::lock_guard lock(mutex_);
            // Serialize final authorization/injection with a local stop so an
            // in-flight press cannot arrive after ReleaseAllRemoteInputs().
            if (directSession_.sessionCloseRequested_ ||
                snapshot_.state != SessionEngineState::kActive ||
                snapshot_.remoteControlRole != RemoteControlRole::kControlled ||
                snapshot_.sessionId != input.roomId ||
                snapshot_.peerDeviceId != input.senderDeviceId || remoteInputSink_ != sink) return;
            sink->OnRemoteInput(input.event);
            cursorMonitor_->SetLastAppliedInputSequence(input.sequence);
        }
        if (sourceToBoost) {
            sourceToBoost->NotifyRemoteInputActivity();
        }
    }
}

void InProcessSessionEngine::OnRemoteTrackAdded(
    const RemoteTrackInfo& track)
{
    (void)track;
}

}  // namespace remote::app
