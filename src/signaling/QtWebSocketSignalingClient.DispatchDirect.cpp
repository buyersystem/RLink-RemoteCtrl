// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "QtWebSocketSignalingClient.Internal.h"

namespace remote {
using namespace signaling_client_detail;

bool QtWebSocketSignalingClient::Impl::DispatchDirectSessionMessage(
    const QString& type, const std::string& sessionId,
    const QJsonObject& payload)
{
    if (type == QStringLiteral("session_pending")) {
        SignalingSessionPending pending;
        pending.sessionId = sessionId;
        pending.peerDeviceId = ToString(
            payload.value(QStringLiteral("peerDeviceId")).toString());
        if (pending.sessionId.empty() ||
            pending.peerDeviceId.empty()) {
            NotifyError("invalid_session_pending",
                        "Pending-session identity fields are invalid.");
            return true;
        }
        if (observer_) {
            observer_->OnSessionPending(pending);
        }
        return true;
    }

    if (type == QStringLiteral("session_request")) {
        IncomingSessionRequest request;
        request.sessionId = sessionId;
        request.requesterDeviceId = ToString(
            payload.value(QStringLiteral("requesterDeviceId")).toString());
        request.purpose = PurposeFromString(
            payload.value(QStringLiteral("purpose")).toString());
        request.requestedPermissions = ReadStringArray(
            payload.value(QStringLiteral("permissions")));
        request.sameAccount = payload.value(
            QStringLiteral("sameAccount")).toBool(false);
        request.autoAccept = payload.value(
            QStringLiteral("autoAccept")).toBool(false);
        const bool assistedRequest = payload.value(
            QStringLiteral("assistedRequest")).toBool(false);
        request.authorization = assistedRequest
            ? DirectAuthorizationMethod::kVerificationCode
            : (request.sameAccount && request.autoAccept
                   ? DirectAuthorizationMethod::kOwnedAccount
                   : DirectAuthorizationMethod::kManualApproval);
        request.verificationCode = ToString(payload.value(
            QStringLiteral("verificationCode")).toString());
        if (request.sessionId.empty() ||
            request.requesterDeviceId.empty() ||
            request.purpose == SessionPurpose::kNone) {
            NotifyError("invalid_incoming_session_request",
                        "Incoming session request fields are invalid.");
            return true;
        }
        if (observer_) {
            observer_->OnIncomingSessionRequest(request);
        }
        return true;
    }

    if (type == QStringLiteral("session_response")) {
        if (sessionId.empty() ||
            !payload.value(QStringLiteral("accepted")).isBool()) {
            NotifyError("invalid_session_response",
                        "Session response fields are invalid.");
            return true;
        }
        SignalingSessionResponse response;
        response.sessionId = sessionId;
        response.accepted =
            payload.value(QStringLiteral("accepted")).toBool(false);
        response.reasonCode = ToString(
            payload.value(QStringLiteral("reasonCode")).toString());
        response.reasonMessage = ToString(
            payload.value(QStringLiteral("reasonMessage")).toString());
        if (observer_) {
            observer_->OnSessionResponse(response);
        }
        return true;
    }

    if (type == QStringLiteral("session_ready")) {
        SignalingSessionReady ready;
        ready.sessionId = sessionId;
        ready.peerDeviceId = ToString(
            payload.value(QStringLiteral("peerDeviceId")).toString());
        ready.recoveryToken = ToString(
            payload.value(QStringLiteral("recoveryToken")).toString());
        if (ready.sessionId.empty() || ready.peerDeviceId.empty() ||
            ready.recoveryToken.empty()) {
            NotifyError("invalid_session_ready",
                        "Session-ready identity fields are invalid.");
            return true;
        }
        const auto servers =
            payload.value(QStringLiteral("iceServers")).toArray();
        for (const auto& serverValue : servers) {
            const auto serverObject = serverValue.toObject();
            SignalingIceServer server;
            server.urls = ReadStringArray(
                serverObject.value(QStringLiteral("urls")));
            server.username = ToString(
                serverObject.value(QStringLiteral("username")).toString());
            server.credential = ToString(
                serverObject.value(QStringLiteral("credential")).toString());
            if (!server.urls.empty()) {
                ready.iceServers.push_back(std::move(server));
            }
        }
        if (observer_) {
            observer_->OnSessionReady(ready);
        }
        return true;
    }

    if (type == QStringLiteral("session_suspended")) {
        SignalingSessionSuspended suspended;
        suspended.sessionId = sessionId;
        suspended.peerDeviceId = ToString(
            payload.value(QStringLiteral("peerDeviceId")).toString());
        const int recoveryWindowMs =
            payload.value(QStringLiteral("recoveryWindowMs")).toInt(-1);
        if (suspended.sessionId.empty() ||
            suspended.peerDeviceId.empty() || recoveryWindowMs < 0) {
            NotifyError("invalid_session_suspended",
                        "Session-suspended fields are invalid.");
            return true;
        }
        suspended.recoveryWindowMs =
            static_cast<std::uint32_t>(recoveryWindowMs);
        if (observer_) {
            observer_->OnSessionSuspended(suspended);
        }
        return true;
    }

    if (type == QStringLiteral("session_resumed")) {
        SignalingSessionResumed resumed;
        resumed.sessionId = sessionId;
        resumed.peerDeviceId = ToString(
            payload.value(QStringLiteral("peerDeviceId")).toString());
        resumed.resumedDeviceId = ToString(
            payload.value(QStringLiteral("resumedDeviceId")).toString());
        if (resumed.sessionId.empty() || resumed.peerDeviceId.empty() ||
            resumed.resumedDeviceId.empty()) {
            NotifyError("invalid_session_resumed",
                        "Session-resumed fields are invalid.");
            return true;
        }
        if (observer_) {
            observer_->OnSessionResumed(resumed);
        }
        return true;
    }

    if (type == QStringLiteral("session_ended")) {
        SignalingSessionEnded ended;
        ended.sessionId = sessionId;
        ended.initiatorDeviceId = ToString(
            payload.value(QStringLiteral("initiatorDeviceId")).toString());
        ended.reasonCode = ToString(
            payload.value(QStringLiteral("reasonCode")).toString());
        const QString disposition =
            payload.value(QStringLiteral("disposition")).toString();
        ended.kind = disposition == QStringLiteral("cancelled")
                         ? SignalingSessionEndKind::kCancelled
                         : SignalingSessionEndKind::kClosed;
        if (ended.sessionId.empty() ||
            ended.initiatorDeviceId.empty() ||
            (disposition != QStringLiteral("cancelled") &&
             disposition != QStringLiteral("closed"))) {
            NotifyError("invalid_session_ended",
                        "Session-ended fields are invalid.");
            return true;
        }
        if (observer_) {
            observer_->OnSessionEnded(ended);
        }
        return true;
    }

    return false;
}

}  // namespace remote
