// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "DirectSessionCoordinator.h"

#include <algorithm>

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

SessionCommandResult DirectSessionCoordinator::PrepareOutgoingStart(
    const DirectSessionConnectRequest& request,
    DirectSessionStartPlan* plan) const
{
    if (!plan) {
        return Failure("direct_start_plan_missing",
                       "A direct session start plan is required.");
    }
    if (request.targetDeviceId.empty()) {
        return Failure("device_id_empty", "A device ID is required.");
    }
    if (request.purpose == SessionPurpose::kNone) {
        return Failure("session_purpose_missing",
                       "A session purpose is required.");
    }

    const bool usesVerificationCode =
        request.authorization ==
        DirectAuthorizationMethod::kVerificationCode;
    if (usesVerificationCode &&
        (request.purpose != SessionPurpose::kRemoteControl ||
         request.verificationCode.size() != 6 ||
         !std::all_of(
             request.verificationCode.cbegin(),
             request.verificationCode.cend(),
             [](char value) { return value >= '0' && value <= '9'; }))) {
        return Failure(
            "invalid_assistance_credentials",
            "A six-digit verification code is required for remote assistance.");
    }
    if (!usesVerificationCode && !request.verificationCode.empty()) {
        return Failure(
            "unexpected_verification_code",
            "This direct authorization method does not use a verification code.");
    }

    plan->request = request;
    plan->permissions =
        request.purpose == SessionPurpose::kRemoteControl
            ? std::vector<std::string>{"viewScreen", "controlInput"}
            : std::vector<std::string>{"cameraMedia"};
    switch (request.authorization) {
    case DirectAuthorizationMethod::kOwnedAccount:
        plan->origin = SessionOrigin::kOwnedDevice;
        break;
    case DirectAuthorizationMethod::kVerificationCode:
        plan->origin = SessionOrigin::kRemoteAssistance;
        break;
    case DirectAuthorizationMethod::kManualApproval:
    default:
        plan->origin = SessionOrigin::kManualDeviceId;
        break;
    }
    return Success();
}

void DirectSessionCoordinator::ApplyOutgoingStart(
    SessionEngineSnapshot* snapshot,
    const DirectSessionStartPlan& plan) const
{
    if (!snapshot) {
        return;
    }
    snapshot->state = SessionEngineState::kConnecting;
    snapshot->purpose = plan.request.purpose;
    snapshot->origin = plan.origin;
    snapshot->remoteControlRole =
        plan.request.purpose == SessionPurpose::kRemoteControl
            ? RemoteControlRole::kController
            : RemoteControlRole::kNone;
    snapshot->peerDeviceId = plan.request.targetDeviceId;
    snapshot->sessionId.clear();
    snapshot->direct.signalingSessionReady = false;
    snapshot->direct.sessionEverActive = false;
    snapshot->error.code.clear();
    snapshot->error.message.clear();
}

SessionCommandResult DirectSessionCoordinator::ValidateIncomingDecision(
    const SessionEngineSnapshot& snapshot,
    const std::string& sessionId,
    bool rejecting) const
{
    if (sessionId.empty()) {
        return Failure("session_id_empty", "A session ID is required.");
    }
    if (snapshot.state != SessionEngineState::kAwaitingLocalApproval ||
        snapshot.sessionId != sessionId) {
        return Failure(
            "incoming_session_not_found",
            rejecting
                ? "There is no matching incoming session to reject."
                : "The incoming session is no longer pending.");
    }
    return Success();
}

void DirectSessionCoordinator::ApplyIncomingAccepted(
    SessionEngineSnapshot* snapshot) const
{
    if (!snapshot) {
        return;
    }
    snapshot->state = SessionEngineState::kConnecting;
    snapshot->error.code.clear();
    snapshot->error.message.clear();
}

void DirectSessionCoordinator::Reset()
{
    DirectSessionRuntimeState::Reset();
    localIsOfferer_ = false;
    offerNegotiationStarted_ = false;
    sessionCloseRequested_ = false;
    sessionEndSignalSent_ = false;
    cancelWhenSessionIdKnown_ = false;
    serverSessionActive_ = false;
    signalingRecoveryPending_ = false;
    peerSignalingSuspended_ = false;
    sessionRecoveryToken_.clear();
    pendingRemoteDescription_.reset();
    pendingRemoteCandidates_.clear();
}

}  // namespace remote::app
