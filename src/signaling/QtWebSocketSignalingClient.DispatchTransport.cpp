// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "QtWebSocketSignalingClient.Internal.h"

namespace remote {
using namespace signaling_client_detail;

bool QtWebSocketSignalingClient::Impl::DispatchTransportMessage(
    const QString& type, const std::string& sessionId,
    const QJsonObject& payload)
{
    if (type == QStringLiteral("sdp")) {
        SignalingSessionDescription description;
        description.sessionId = sessionId;
        description.type = ToString(
            payload.value(QStringLiteral("type")).toString());
        description.sdp = ToString(
            payload.value(QStringLiteral("sdp")).toString());
        description.negotiationGeneration = static_cast<std::uint64_t>(
            payload.value(QStringLiteral("negotiationGeneration"))
                .toVariant().toULongLong());
        if (description.sessionId.empty() || description.sdp.empty() ||
            description.negotiationGeneration == 0 ||
            (description.type != "offer" &&
             description.type != "answer")) {
            NotifyError("invalid_remote_description",
                        "Remote SDP fields are invalid.");
            return true;
        }
        if (observer_) {
            observer_->OnRemoteDescription(description);
        }
        return true;
    }

    if (type == QStringLiteral("ice_candidate")) {
        SignalingIceCandidate candidate;
        candidate.sessionId = sessionId;
        candidate.candidate = ToString(
            payload.value(QStringLiteral("candidate")).toString());
        candidate.sdpMid = ToString(
            payload.value(QStringLiteral("sdpMid")).toString());
        candidate.sdpMLineIndex = payload
            .value(QStringLiteral("sdpMLineIndex"))
            .toInt(-1);
        candidate.negotiationGeneration = static_cast<std::uint64_t>(
            payload.value(QStringLiteral("negotiationGeneration"))
                .toVariant().toULongLong());
        if (candidate.sessionId.empty() ||
            candidate.candidate.empty() ||
            candidate.sdpMLineIndex < 0 ||
            candidate.negotiationGeneration == 0) {
            NotifyError("invalid_remote_ice_candidate",
                        "Remote ICE candidate fields are invalid.");
            return true;
        }
        if (observer_) {
            observer_->OnRemoteIceCandidate(candidate);
        }
        return true;
    }

    if (type == QStringLiteral("ice_restart_request")) {
        SignalingIceRestartRequest request;
        request.sessionId = sessionId;
        request.observedGeneration = static_cast<std::uint64_t>(
            payload.value(QStringLiteral("observedGeneration"))
                .toVariant().toULongLong());
        request.requestSequence = static_cast<std::uint64_t>(
            payload.value(QStringLiteral("requestSequence"))
                .toVariant().toULongLong());
        if (request.sessionId.empty() || request.observedGeneration == 0 ||
            request.requestSequence == 0) {
            NotifyError("invalid_ice_restart_request",
                        "ICE restart request fields are invalid.");
            return true;
        }
        if (observer_) {
            observer_->OnIceRestartRequested(request);
        }
        return true;
    }

    if (type == QStringLiteral("ice_restart_cancel")) {
        SignalingIceRestartCancel cancel;
        cancel.sessionId = sessionId;
        cancel.observedGeneration = static_cast<std::uint64_t>(
            payload.value(QStringLiteral("observedGeneration"))
                .toVariant().toULongLong());
        cancel.requestSequence = static_cast<std::uint64_t>(
            payload.value(QStringLiteral("requestSequence"))
                .toVariant().toULongLong());
        if (cancel.sessionId.empty() || cancel.observedGeneration == 0 ||
            cancel.requestSequence == 0) {
            NotifyError("invalid_ice_restart_cancel",
                        "ICE restart cancellation fields are invalid.");
            return true;
        }
        if (observer_) {
            observer_->OnIceRestartCancelled(cancel);
        }
        return true;
    }

    if (type == QStringLiteral("error")) {
        NotifyError(
            ToString(payload.value(QStringLiteral("code")).toString()),
            ToString(payload.value(QStringLiteral("message")).toString()));
        return true;
    }

    if (type == QStringLiteral("heartbeat_ack")) {
        if (heartbeatOutstanding_) {
            heartbeatOutstanding_ = false;
            heartbeatDeadline_.stop();
            const auto elapsed = heartbeatRoundTrip_.elapsed();
            if (observer_) {
                observer_->OnHeartbeatAcknowledged(
                    static_cast<std::uint32_t>(
                        elapsed < 0 ? 0 : elapsed));
            }
        }
        return true;
    }

    return false;
}

}  // namespace remote
