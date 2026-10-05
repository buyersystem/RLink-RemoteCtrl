// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "InProcessSessionEngine.h"

#include <random>
#include <utility>

#include "InProcessSessionEngineInternal.h"
#include "src/webrtc/LibWebRtcSession.h"
#include "src/webrtc/WebRtcRuntime.h"

namespace remote::app {
namespace {

std::string GenerateRotatedVerificationCode(
    const std::string& previous)
{
    std::string code;
    static thread_local std::mt19937 generator{
        std::random_device{}()};
    static thread_local std::uniform_int_distribution<std::uint32_t>
        distribution(0, 999'999);
    do {
        const auto value = distribution(generator);
        code = std::to_string(1'000'000u + value).substr(1);
    } while (code == previous);
    return code;
}

}  // namespace

void InProcessSessionEngine::OnSignalingStateChanged(
    SignalingConnectionState state)
{
    std::vector<std::shared_ptr<RoomPairRuntime>> pairs;
    SessionControllerBase* legacyController = nullptr;
    bool signalingAvailable = false;
    {
        std::lock_guard lock(mutex_);
        switch (state) {
        case SignalingConnectionState::kConnecting:
        case SignalingConnectionState::kAuthenticating:
        case SignalingConnectionState::kRegistering:
            snapshot_.connectivity = SessionConnectivityState::kConnecting;
            break;
        case SignalingConnectionState::kRegistered:
            snapshot_.connectivity = SessionConnectivityState::kOnline;
            signalingAvailable = !roomSession_.recoveryPending_;
            break;
        case SignalingConnectionState::kFailed:
            snapshot_.connectivity = SessionConnectivityState::kFailed;
            snapshot_.ownedDevices.loaded = false;
            for (auto& device : snapshot_.ownedDevices.devices) {
                device.online = false;
            }
            snapshot_.roomActivity.incomingScreenShareSwitchRequests.clear();
            snapshot_.roomActivity.outgoingScreenShareSwitchRequestId.clear();
            if (directSession_.serverSessionActive_ && sessionController_) {
                directSession_.signalingRecoveryPending_ = true;
            }
            if (snapshot_.room.membership == RoomMembershipState::kActive &&
                !roomSession_.recoveryToken_.empty()) {
                roomSession_.recoveryPending_ = true;
                snapshot_.room.errorCode = "room_signaling_recovering";
                snapshot_.room.errorMessage =
                    "Room signaling is temporarily unavailable; the membership slot is being preserved.";
            }
            break;
        case SignalingConnectionState::kDisconnected:
        case SignalingConnectionState::kClosing:
            snapshot_.connectivity = SessionConnectivityState::kOffline;
            snapshot_.ownedDevices.loaded = false;
            for (auto& device : snapshot_.ownedDevices.devices) {
                device.online = false;
            }
            snapshot_.roomActivity.incomingScreenShareSwitchRequests.clear();
            snapshot_.roomActivity.outgoingScreenShareSwitchRequestId.clear();
            if (state == SignalingConnectionState::kDisconnected &&
                directSession_.serverSessionActive_ && sessionController_ &&
                snapshot_.state != SessionEngineState::kStopping &&
                snapshot_.state != SessionEngineState::kStopped) {
                directSession_.signalingRecoveryPending_ = true;
            }
            if (state == SignalingConnectionState::kDisconnected &&
                snapshot_.room.membership == RoomMembershipState::kActive &&
                !roomSession_.recoveryToken_.empty() &&
                snapshot_.state != SessionEngineState::kStopping &&
                snapshot_.state != SessionEngineState::kStopped) {
                roomSession_.recoveryPending_ = true;
                snapshot_.room.errorCode = "room_signaling_recovering";
                snapshot_.room.errorMessage =
                    "Room signaling is temporarily unavailable; the membership slot is being preserved.";
            }
            break;
        }
        legacyController = sessionController_.get();
        for (const auto& [pairId, pair] : roomPairs_) {
            (void)pairId;
            pairs.push_back(pair);
        }
    }
    if (legacyController) {
        legacyController->SetSignalingAvailable(
            state == SignalingConnectionState::kRegistered);
    }
    for (const auto& pair : pairs) {
        if (pair && pair->controller) {
            pair->controller->SetSignalingAvailable(signalingAvailable);
        }
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnDeviceRegistered(const std::string& deviceId)
{
    bool resumeSession = false;
    bool resumeRoom = false;
    std::string sessionId;
    std::string recoveryToken;
    std::string roomId;
    std::string roomRecoveryToken;
    std::string deferredRoomLeaveId;
    {
        std::lock_guard lock(mutex_);
        snapshot_.connectivity = SessionConnectivityState::kOnline;
        snapshot_.localDeviceId = deviceId;
        deferredRoomLeaveId =
            std::exchange(roomSession_.deferredLeaveId_, {});
        resumeSession = directSession_.signalingRecoveryPending_ &&
                        directSession_.serverSessionActive_ &&
                        sessionController_ &&
                        !snapshot_.sessionId.empty() &&
                        !directSession_.sessionRecoveryToken_.empty();
        if (resumeSession) {
            sessionId = snapshot_.sessionId;
            recoveryToken = directSession_.sessionRecoveryToken_;
            snapshot_.error.code = "signaling_session_recovering";
            snapshot_.error.message =
                "The signaling connection was restored; the active session is being resumed.";
        }
        resumeRoom = roomSession_.recoveryPending_ &&
                     snapshot_.room.membership ==
                         RoomMembershipState::kActive &&
                     !snapshot_.room.roomId.empty() &&
                     !roomSession_.recoveryToken_.empty();
        if (resumeRoom) {
            roomId = snapshot_.room.roomId;
            roomRecoveryToken = roomSession_.recoveryToken_;
            snapshot_.room.errorCode = "room_signaling_recovering";
            snapshot_.room.errorMessage =
                "The signaling connection was restored; room membership is being resumed.";
        }
        if (!resumeSession && !resumeRoom &&
            !directSession_.peerSignalingSuspended_) {
            snapshot_.error.code.clear();
            snapshot_.error.message.clear();
        }
    }
    PublishSnapshot();
    if (!deferredRoomLeaveId.empty()) {
        const auto result = signaling_->LeaveRoom(
            deferredRoomLeaveId, "p2p_recovery_failed");
        if (!result.accepted) {
            std::lock_guard lock(mutex_);
            if (roomSession_.deferredLeaveId_.empty()) {
                roomSession_.deferredLeaveId_ = deferredRoomLeaveId;
            }
        }
    }
    if (resumeSession) {
        const auto result = signaling_->ResumeSession(sessionId,
                                                      recoveryToken);
        if (!result.accepted) {
            OnSignalingError(result.errorCode, result.errorMessage);
        }
    }
    if (resumeRoom) {
        const auto result = signaling_->ResumeRoom(roomId,
                                                   roomRecoveryToken);
        if (!result.accepted) {
            OnSignalingError(result.errorCode, result.errorMessage);
        }
    }
    if (signalingConfig_.authenticationMode ==
        SignalingAuthenticationMode::kMessageAccessToken) {
        const auto ownedDevicesResult = signaling_->RequestOwnedDevices();
        if (!ownedDevicesResult.accepted) {
            OnSignalingError(ownedDevicesResult.errorCode,
                             ownedDevicesResult.errorMessage);
        }
    }
}

void InProcessSessionEngine::OnOwnedDevicesChanged(
    const SignalingOwnedDevicesSnapshot& owned)
{
    {
        std::lock_guard lock(mutex_);
        if (owned.revision < snapshot_.ownedDevices.revision) {
            return;
        }
        snapshot_.ownedDevices.revision = owned.revision;
        snapshot_.ownedDevices.loaded = true;
        snapshot_.ownedDevices.devices.clear();
        snapshot_.ownedDevices.devices.reserve(owned.devices.size());
        for (const auto& source : owned.devices) {
            OwnedDeviceSnapshot device;
            device.deviceId = source.deviceId;
            device.deviceName = source.deviceName;
            device.online = source.online;
            device.current = source.current;
            device.createdAt = source.createdAt;
            device.lastSeenAt = source.lastSeenAt;
            snapshot_.ownedDevices.devices.push_back(std::move(device));
        }
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnIncomingSessionRequest(
    const IncomingSessionRequest& request)
{
    DisposeClosedSession();
    const bool ownedDeviceRequest =
        request.authorization ==
        DirectAuthorizationMethod::kOwnedAccount;
    const bool assistedRequest =
        request.authorization ==
        DirectAuthorizationMethod::kVerificationCode;
    bool busy = false;
    const bool ownedDeviceAutoAccept =
        ownedDeviceRequest && request.sameAccount && request.autoAccept;
    bool assistedCodeValid = false;
    {
        std::lock_guard lock(mutex_);
        assistedCodeValid = assistedRequest &&
            !signalingConfig_.deviceVerificationCode.empty() &&
            request.verificationCode ==
                signalingConfig_.deviceVerificationCode;
    }
    if (assistedRequest && !assistedCodeValid) {
        signaling_->RespondToSession(
            request.sessionId, false, "verification_code_invalid");
        return;
    }
    const bool autoAccept = ownedDeviceAutoAccept || assistedCodeValid;
    {
        std::lock_guard lock(mutex_);
        busy = snapshot_.state != SessionEngineState::kReady ||
               (snapshot_.room.membership != RoomMembershipState::kNone &&
                snapshot_.room.membership != RoomMembershipState::kFailed);
        if (!busy) {
            snapshot_.state = autoAccept
                ? SessionEngineState::kConnecting
                : SessionEngineState::kAwaitingLocalApproval;
            snapshot_.purpose = request.purpose;
            snapshot_.origin = ownedDeviceAutoAccept
                ? SessionOrigin::kOwnedDevice
                : (assistedRequest
                       ? SessionOrigin::kRemoteAssistance
                       : SessionOrigin::kManualDeviceId);
            snapshot_.remoteControlRole =
                request.purpose == SessionPurpose::kRemoteControl
                    ? RemoteControlRole::kControlled
                    : RemoteControlRole::kNone;
            snapshot_.sessionId = request.sessionId;
            snapshot_.peerDeviceId = request.requesterDeviceId;
            snapshot_.direct.sessionEverActive = false;
            snapshot_.error.code.clear();
            snapshot_.error.message.clear();
            directSession_.localIsOfferer_ = false;
            directSession_.offerNegotiationStarted_ = false;
            directSession_.sessionCloseRequested_ = false;
            directSession_.sessionEndSignalSent_ = false;
            directSession_.cancelWhenSessionIdKnown_ = false;
            directSession_.serverSessionActive_ = false;
            snapshot_.direct.signalingSessionReady = false;
            directSession_.signalingRecoveryPending_ = false;
            directSession_.peerSignalingSuspended_ = false;
            directSession_.sessionRecoveryToken_.clear();
            directSession_.pendingRemoteDescription_.reset();
            directSession_.pendingRemoteCandidates_.clear();
            if (assistedCodeValid) {
                signalingConfig_.deviceVerificationCode =
                    GenerateRotatedVerificationCode(
                        signalingConfig_.deviceVerificationCode);
                snapshot_.localVerificationCode =
                    signalingConfig_.deviceVerificationCode;
            }
        }
    }
    if (busy) {
        signaling_->RespondToSession(request.sessionId, false, "device_busy");
        return;
    }
    if (autoAccept) {
        const auto result = signaling_->RespondToSession(
            request.sessionId, true, {});
        if (!result.accepted) {
            std::lock_guard lock(mutex_);
            ResetSessionStateLocked();
            snapshot_.error.code = result.errorCode;
            snapshot_.error.message = result.errorMessage;
        }
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnSessionResponse(
    const SignalingSessionResponse& response)
{
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state != SessionEngineState::kConnecting ||
            (!snapshot_.sessionId.empty() &&
             snapshot_.sessionId != response.sessionId)) {
            return;
        }
        snapshot_.sessionId = response.sessionId;
        if (!response.accepted) {
            ResetSessionStateLocked();
            snapshot_.error.code = response.reasonCode.empty()
                                      ? "session_rejected"
                                      : response.reasonCode;
            snapshot_.error.message = response.reasonMessage.empty()
                                         ? "The peer rejected the session."
                                         : response.reasonMessage;
        }
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnSessionPending(
    const SignalingSessionPending& pending)
{
    bool cancel = false;
    ISignalingClient* signaling = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (!directSession_.localIsOfferer_ ||
            snapshot_.state != SessionEngineState::kConnecting ||
            (!snapshot_.sessionId.empty() &&
             snapshot_.sessionId != pending.sessionId)) {
            return;
        }
        snapshot_.sessionId = pending.sessionId;
        if (!pending.peerDeviceId.empty()) {
            snapshot_.peerDeviceId = pending.peerDeviceId;
        }
        if (directSession_.cancelWhenSessionIdKnown_) {
            cancel = true;
            directSession_.sessionEndSignalSent_ = true;
            signaling = signaling_.get();
        }
    }
    if (cancel && signaling) {
        const auto result = signaling->CancelSession(
            pending.sessionId, "cancelled_by_local_user");
        {
            std::lock_guard lock(mutex_);
            ResetSessionStateLocked();
            if (!result.accepted) {
                snapshot_.error.code = result.errorCode;
                snapshot_.error.message = result.errorMessage;
            }
        }
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnSessionReady(
    const SignalingSessionReady& ready)
{
    const auto initialQualityDeficitShare = ScreenQualityDeficitShareFromProvider();
    SessionControllerConfig controllerConfig;
    controllerConfig.negotiationTimeout = options_.negotiationTimeout;
    controllerConfig.reconnectTimeout = options_.reconnectTimeout;
    controllerConfig.webRtc.includeLoopbackAdapter =
        options_.includeLoopbackAdapter;
    controllerConfig.webRtc.fastDesktopBweStartup =
        options_.desktopCaptureImplementation ==
        DesktopCaptureImplementation::kLibWebRtc;
    controllerConfig.webRtc.adaptiveDesktopNetworkFrameRate = true;
    controllerConfig.webRtc.iceMinPort = options_.iceMinPort;
    controllerConfig.webRtc.iceMaxPort = options_.iceMaxPort;
    for (const auto& signalingServer : ready.iceServers) {
        IceServerConfig server;
        server.urls = signalingServer.urls;
        server.username = signalingServer.username;
        server.password = signalingServer.credential;
        controllerConfig.webRtc.iceServers.push_back(std::move(server));
    }

    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state != SessionEngineState::kConnecting ||
            (!snapshot_.sessionId.empty() &&
             snapshot_.sessionId != ready.sessionId)) {
            return;
        }
        snapshot_.sessionId = ready.sessionId;
        if (!ready.peerDeviceId.empty()) {
            snapshot_.peerDeviceId = ready.peerDeviceId;
        }
        directSession_.serverSessionActive_ = true;
        snapshot_.direct.signalingSessionReady = true;
        directSession_.signalingRecoveryPending_ = false;
        directSession_.peerSignalingSuspended_ = false;
        directSession_.sessionRecoveryToken_ = ready.recoveryToken;
        if (!sessionController_) {
            auto session = std::make_unique<LibWebRtcSession>(
                runtime_->PeerConnectionFactory());
            session->SetScreenVideoBitrateBppProvider(
                options_.screenVideoBitrateBppProvider);
            session->SetScreenQualityDeficitShare(
                liveScreenQualityDeficitShare_.value_or(initialQualityDeficitShare));
            std::unique_ptr<SessionControllerBase> controller;
            if (directSession_.localIsOfferer_) {
                controller =
                    std::make_unique<ControllerSessionController>(
                        *session,
                        static_cast<ISessionSignalingSender&>(*this));
            } else {
                auto agent = std::make_unique<AgentSessionController>(
                    *session,
                    static_cast<ISessionSignalingSender&>(*this));
                if (snapshot_.purpose ==
                    SessionPurpose::kRemoteControl) {
                    agent->SetAnswerPreparation([this] {
                        return PrepareDirectMedia(true);
                    });
                }
                controller = std::move(agent);
            }
            webRtcSession_ = std::move(session);
            sessionController_ = std::move(controller);
            ++directSessionGeneration_;
            sessionController_->SetObserver(this);
            sessionController_->Start(controllerConfig);
            if (directSession_.pendingRemoteDescription_) {
                sessionController_->HandleRemoteDescription(
                    *directSession_.pendingRemoteDescription_);
                directSession_.pendingRemoteDescription_.reset();
            }
            for (const auto& candidate :
                 directSession_.pendingRemoteCandidates_) {
                sessionController_->HandleRemoteIceCandidate(candidate);
            }
            directSession_.pendingRemoteCandidates_.clear();
        }
        // Remain kConnecting until the PeerConnection reaches its active
        // state. A signaling session alone is not a working remote session.
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnSessionSuspended(
    const SignalingSessionSuspended& suspended)
{
    {
        std::lock_guard lock(mutex_);
        if (suspended.sessionId.empty() ||
            snapshot_.sessionId != suspended.sessionId ||
            suspended.peerDeviceId != snapshot_.peerDeviceId ||
            !sessionController_) {
            return;
        }
        directSession_.peerSignalingSuspended_ = true;
        snapshot_.error.code = "peer_signaling_recovering";
        snapshot_.error.message =
            "The peer signaling connection is temporarily unavailable; the P2P session remains active.";
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnSessionResumed(
    const SignalingSessionResumed& resumed)
{
    {
        std::lock_guard lock(mutex_);
        if (resumed.sessionId.empty() ||
            snapshot_.sessionId != resumed.sessionId ||
            !sessionController_) {
            return;
        }
        if (resumed.resumedDeviceId == snapshot_.localDeviceId) {
            directSession_.signalingRecoveryPending_ = false;
            directSession_.serverSessionActive_ = true;
            snapshot_.direct.signalingSessionReady = true;
        } else if (resumed.resumedDeviceId == snapshot_.peerDeviceId) {
            directSession_.peerSignalingSuspended_ = false;
        } else {
            return;
        }
        if (!directSession_.signalingRecoveryPending_ &&
            !directSession_.peerSignalingSuspended_ &&
            (snapshot_.error.code == "signaling_session_recovering" ||
             snapshot_.error.code == "peer_signaling_recovering" ||
             snapshot_.error.code == "websocket_error" ||
             snapshot_.error.code == "heartbeat_timeout")) {
            snapshot_.error.code.clear();
            snapshot_.error.message.clear();
        }
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnSessionEnded(
    const SignalingSessionEnded& ended)
{
    bool publish = false;
    {
        std::lock_guard lock(mutex_);
        if (ended.sessionId.empty() ||
            snapshot_.sessionId != ended.sessionId) {
            return;
        }
        const bool initiatedLocally =
            ended.initiatorDeviceId == snapshot_.localDeviceId;
        if (!initiatedLocally) {
            directSession_.sessionCloseRequested_ = false;
            snapshot_.error.code =
                ended.reasonCode.empty()
                    ? (ended.kind == SignalingSessionEndKind::kCancelled
                           ? "session_cancelled_by_peer"
                           : "session_closed_by_peer")
                    : ended.reasonCode;
            snapshot_.error.message =
                ended.kind == SignalingSessionEndKind::kCancelled
                    ? "The peer cancelled the session."
                    : "The peer closed the session.";
        }
        directSession_.serverSessionActive_ = false;
        snapshot_.direct.signalingSessionReady = false;
        if (sessionController_) {
            sessionController_->Close();
        } else {
            const std::string errorCode = snapshot_.error.code;
            const std::string errorMessage = snapshot_.error.message;
            ResetSessionStateLocked();
            if (!initiatedLocally) {
                snapshot_.error.code = errorCode;
                snapshot_.error.message = errorMessage;
            }
        }
        publish = true;
    }
    if (publish) {
        PublishSnapshot();
    }
}

void InProcessSessionEngine::OnRemoteDescription(
    const SignalingSessionDescription& description)
{
    SessionDescription remoteDescription;
    remoteDescription.type = description.type == "answer"
                                 ? SessionDescriptionType::kAnswer
                                 : SessionDescriptionType::kOffer;
    remoteDescription.sdp = description.sdp;
    remoteDescription.negotiationGeneration =
        description.negotiationGeneration;
    std::shared_ptr<RoomPairRuntime> roomPair;
    {
        std::lock_guard lock(mutex_);
        const auto roomPairIt = roomPairs_.find(description.sessionId);
        if (roomPairIt != roomPairs_.end()) {
            roomPair = roomPairIt->second;
        }
        else {
            if (description.sessionId.empty() ||
                snapshot_.sessionId != description.sessionId ||
                snapshot_.state != SessionEngineState::kConnecting) {
                return;
            }
            if (sessionController_) {
                sessionController_->HandleRemoteDescription(
                    remoteDescription);
            }
            else {
                directSession_.pendingRemoteDescription_ =
                    std::move(remoteDescription);
            }
            return;
        }
    }
    // SDP application may synchronously invoke answer preparation and pair
    // observer callbacks. Keep the engine mutex out of that call chain.
    if (roomPair && roomPair->controller) {
        roomPair->controller->HandleRemoteDescription(remoteDescription);
    }
}

void InProcessSessionEngine::OnRemoteIceCandidate(
    const SignalingIceCandidate& candidate)
{
    IceCandidate remoteCandidate;
    remoteCandidate.candidate = candidate.candidate;
    remoteCandidate.sdpMid = candidate.sdpMid;
    remoteCandidate.sdpMLineIndex = candidate.sdpMLineIndex;
    remoteCandidate.negotiationGeneration =
        candidate.negotiationGeneration;
    std::shared_ptr<RoomPairRuntime> roomPair;
    {
        std::lock_guard lock(mutex_);
        const auto roomPairIt = roomPairs_.find(candidate.sessionId);
        if (roomPairIt != roomPairs_.end()) {
            roomPair = roomPairIt->second;
        }
        else {
            if (candidate.sessionId.empty() ||
                snapshot_.sessionId != candidate.sessionId ||
                (snapshot_.state != SessionEngineState::kConnecting &&
                 snapshot_.state != SessionEngineState::kActive)) {
                return;
            }
            if (sessionController_) {
                sessionController_->HandleRemoteIceCandidate(
                    remoteCandidate);
            }
            else {
                directSession_.pendingRemoteCandidates_.push_back(
                    std::move(remoteCandidate));
            }
            return;
        }
    }
    if (roomPair && roomPair->controller) {
        roomPair->controller->HandleRemoteIceCandidate(remoteCandidate);
    }
}

void InProcessSessionEngine::OnIceRestartRequested(
    const SignalingIceRestartRequest& request)
{
    std::shared_ptr<RoomPairRuntime> roomPair;
    SessionControllerBase* legacyController = nullptr;
    {
        std::lock_guard lock(mutex_);
        const auto found = roomPairs_.find(request.sessionId);
        if (found != roomPairs_.end()) {
            roomPair = found->second;
        } else if (snapshot_.sessionId == request.sessionId) {
            legacyController = sessionController_.get();
        }
    }
    if (roomPair && roomPair->controller) {
        roomPair->controller->HandleRemoteIceRestartRequest(
            request.observedGeneration, request.requestSequence);
    } else if (legacyController) {
        legacyController->HandleRemoteIceRestartRequest(
            request.observedGeneration, request.requestSequence);
    }
}

void InProcessSessionEngine::OnIceRestartCancelled(
    const SignalingIceRestartCancel& cancel)
{
    std::shared_ptr<RoomPairRuntime> roomPair;
    SessionControllerBase* legacyController = nullptr;
    {
        std::lock_guard lock(mutex_);
        const auto found = roomPairs_.find(cancel.sessionId);
        if (found != roomPairs_.end()) {
            roomPair = found->second;
        } else if (snapshot_.sessionId == cancel.sessionId) {
            legacyController = sessionController_.get();
        }
    }
    if (roomPair && roomPair->controller) {
        roomPair->controller->HandleRemoteIceRestartCancel(
            cancel.observedGeneration, cancel.requestSequence);
    } else if (legacyController) {
        legacyController->HandleRemoteIceRestartCancel(
            cancel.observedGeneration, cancel.requestSequence);
    }
}

void InProcessSessionEngine::OnHeartbeatAcknowledged(
    std::uint32_t roundTripMs)
{
    (void)roundTripMs;
}

void InProcessSessionEngine::OnSignalingError(
    const std::string& code,
    const std::string& message)
{
    std::vector<std::shared_ptr<RoomPairRuntime>> removedPairs;
    {
        std::lock_guard lock(mutex_);
        const bool pendingDirectRequestFailed =
            snapshot_.state == SessionEngineState::kConnecting &&
            directSession_.localIsOfferer_ && !sessionController_ &&
            !directSession_.serverSessionActive_ &&
            (code == "target_offline" || code == "device_busy" ||
             code == "rate_limited" ||
             code == "invalid_session_request" ||
             code == "invalid_session_permissions" ||
             code == "session_request_timeout");
        if (pendingDirectRequestFailed) {
            ResetSessionStateLocked();
        }
        snapshot_.error.code = code;
        snapshot_.error.message = message;
        const bool pendingRoomOperation =
            snapshot_.room.membership == RoomMembershipState::kCreating ||
            snapshot_.room.membership == RoomMembershipState::kJoinPending;
        if (pendingRoomOperation) {
            snapshot_.room.membership = RoomMembershipState::kFailed;
            snapshot_.room.errorCode = code;
            snapshot_.room.errorMessage = message;
        }
        if (code == "session_resume_rejected" && sessionController_) {
            directSession_.signalingRecoveryPending_ = false;
            directSession_.serverSessionActive_ = false;
            snapshot_.direct.signalingSessionReady = false;
            directSession_.sessionCloseRequested_ = false;
            sessionController_->Close();
        }
        if (code.rfind("room_", 0) == 0) {
            snapshot_.room.errorCode = code;
            snapshot_.room.errorMessage = message;
            if (code == "room_resume_rejected") {
                const std::string roomId = snapshot_.room.roomId;
                removedPairs.reserve(roomPairs_.size());
                for (auto& [pairId, pair] : roomPairs_) {
                    (void)pairId;
                    removedPairs.push_back(std::move(pair));
                }
                roomPairs_.clear();
                ResetRoomStateLocked();
                snapshot_.room.membership = RoomMembershipState::kFailed;
                snapshot_.room.roomId = roomId;
                snapshot_.room.errorCode = code;
                snapshot_.room.errorMessage = message;
            }
        }
    }
    for (auto& pair : removedPairs) {
        RetireRoomPair(std::move(pair));
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnAccountDeletionResult(
    const SignalingAccountDeletionResult& result)
{
    std::function<void(const SignalingAccountDeletionResult&)> callback;
    {
        std::lock_guard lock(mutex_);
        callback = accountDeletionResultCallback_;
    }
    if (callback) {
        callback(result);
    }
}

}  // namespace remote::app
