// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "InProcessSessionEngine.h"

#include <utility>

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

SessionCommandResult InProcessSessionEngine::ConnectDirectDevice(
    const DirectSessionConnectRequest& request)
{
    DisposeClosedSession();
    if (auto ready = RequireReady("connect device"); !ready.accepted) {
        return ready;
    }
    DirectSessionStartPlan plan;
    if (const auto prepared =
            directSession_.PrepareOutgoingStart(request, &plan);
        !prepared.accepted) {
        return prepared;
    }
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership != RoomMembershipState::kNone &&
            snapshot_.room.membership != RoomMembershipState::kFailed) {
            return Failure("room_active",
                           "Leave the current room before starting a legacy direct session.");
        }
    }
    if (!signaling_) {
        return Failure("signaling_not_configured",
                       "WSS signaling is not configured.");
    }
    if (!SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }

    SignalingOperationResult result;
    if (request.authorization == DirectAuthorizationMethod::kOwnedAccount) {
        result = signaling_->RequestOwnedDeviceSession(
            request.targetDeviceId, request.purpose, plan.permissions);
    } else if (request.authorization ==
               DirectAuthorizationMethod::kVerificationCode) {
        result = signaling_->RequestAssistedSession(
            request.targetDeviceId, request.verificationCode,
            plan.permissions);
    } else {
        result = signaling_->RequestSession(
            request.targetDeviceId, request.purpose, plan.permissions);
    }
    if (!result.accepted) {
        return {false, result.errorCode, result.errorMessage};
    }
    {
        std::lock_guard lock(mutex_);
        directSession_.ApplyOutgoingStart(&snapshot_, plan);
        directSession_.localIsOfferer_ = true;
        directSession_.offerNegotiationStarted_ = false;
        directSession_.sessionCloseRequested_ = false;
        directSession_.sessionEndSignalSent_ = false;
        directSession_.cancelWhenSessionIdKnown_ = false;
        directSession_.serverSessionActive_ = false;
        directSession_.signalingRecoveryPending_ = false;
        directSession_.peerSignalingSuspended_ = false;
        directSession_.sessionRecoveryToken_.clear();
        directSession_.pendingRemoteDescription_.reset();
        directSession_.pendingRemoteCandidates_.clear();
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult InProcessSessionEngine::RefreshOwnedDevices()
{
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    const auto result = signaling_->RequestOwnedDevices();
    return {result.accepted, result.errorCode, result.errorMessage};
}

SessionCommandResult InProcessSessionEngine::AcceptIncomingSession(
    const std::string& sessionId)
{
    if (sessionId.empty()) {
        return Failure("session_id_empty", "A session ID is required.");
    }
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    {
        std::lock_guard lock(mutex_);
        const auto validation =
            directSession_.ValidateIncomingDecision(
                snapshot_, sessionId, false);
        if (!validation.accepted) {
            return validation;
        }
    }
    const auto result = signaling_->RespondToSession(sessionId, true, {});
    if (!result.accepted) {
        return {false, result.errorCode, result.errorMessage};
    }
    {
        std::lock_guard lock(mutex_);
        directSession_.ApplyIncomingAccepted(&snapshot_);
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult InProcessSessionEngine::RejectIncomingSession(
    const std::string& sessionId)
{
    if (sessionId.empty()) {
        return Failure("session_id_empty", "A session ID is required.");
    }
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    {
        std::lock_guard lock(mutex_);
        const auto validation =
            directSession_.ValidateIncomingDecision(
                snapshot_, sessionId, true);
        if (!validation.accepted) {
            return validation;
        }
    }
    const auto result =
        signaling_->RespondToSession(sessionId, false, "rejected_by_user");
    if (!result.accepted) {
        return {false, result.errorCode, result.errorMessage};
    }
    {
        std::lock_guard lock(mutex_);
        ResetSessionStateLocked();
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult InProcessSessionEngine::Disconnect()
{
    enum class EndAction {
        kNone,
        kCancel,
        kClose,
    };
    EndAction action = EndAction::kNone;
    std::string sessionId;
    ISignalingClient* signaling = nullptr;
    bool resetImmediately = false;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state == SessionEngineState::kReady ||
            snapshot_.state == SessionEngineState::kStopped) {
            return Success();
        }
        if (directSession_.sessionEndSignalSent_) {
            return Success();
        }
        directSession_.sessionCloseRequested_ = true;
        signaling = signaling_.get();
        sessionId = snapshot_.sessionId;
        if (sessionController_) {
            sessionController_->Close();
        }
        if (sessionId.empty()) {
            if (snapshot_.state == SessionEngineState::kConnecting &&
                directSession_.localIsOfferer_) {
                directSession_.cancelWhenSessionIdKnown_ = true;
                return Success();
            }
        } else {
            action = directSession_.serverSessionActive_
                ? EndAction::kClose
                : EndAction::kCancel;
            directSession_.sessionEndSignalSent_ = true;
        }
        if (sessionController_) {
            // The controller's Closed callback returns the engine to Ready.
            // Keep the immutable session identity until then.
        } else if (snapshot_.state == SessionEngineState::kConnecting ||
                   snapshot_.state ==
                       SessionEngineState::kAwaitingLocalApproval) {
            resetImmediately = true;
        } else {
            return Failure("session_not_disconnectable",
                           "There is no live session to disconnect.");
        }
    }

    SignalingOperationResult result = {true, {}, {}};
    if (signaling && action == EndAction::kCancel) {
        result = signaling->CancelSession(sessionId,
                                          "cancelled_by_local_user");
    } else if (signaling && action == EndAction::kClose) {
        result = signaling->CloseSession(sessionId,
                                         "closed_by_local_user");
    }
    if (resetImmediately || !result.accepted) {
        std::lock_guard lock(mutex_);
        ResetSessionStateLocked();
        if (!result.accepted) {
            snapshot_.error.code = result.errorCode;
            snapshot_.error.message = result.errorMessage;
        }
    }
    PublishSnapshot();
    return result.accepted
               ? Success()
               : Failure(result.errorCode, result.errorMessage);
}

}  // namespace remote::app
