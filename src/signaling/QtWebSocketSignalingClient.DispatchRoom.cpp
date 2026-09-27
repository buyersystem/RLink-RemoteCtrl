// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "QtWebSocketSignalingClient.Internal.h"

namespace remote {
using namespace signaling_client_detail;

bool QtWebSocketSignalingClient::Impl::DispatchRoomMessage(
    const QString& type, const std::string& sessionId,
    const QJsonObject& payload)
{
    if (type == QStringLiteral("room_ready")) {
        SignalingRoomReady ready;
        ready.recoveryToken = ToString(
            payload.value(QStringLiteral("recoveryToken")).toString());
        if (ready.recoveryToken.empty() ||
            !ReadRoomSnapshot(payload.value(QStringLiteral("room")),
                              &ready.room)) {
            NotifyError("invalid_room_ready",
                        "Room-ready fields are invalid.");
            return true;
        }
        if (observer_) {
            observer_->OnRoomReady(ready);
        }
        return true;
    }

    if (type == QStringLiteral("room_state")) {
        RoomSnapshot room;
        if (!ReadRoomSnapshot(payload.value(QStringLiteral("room")),
                              &room)) {
            NotifyError("invalid_room_state",
                        "Room-state fields are invalid.");
            return true;
        }
        if (observer_) {
            observer_->OnRoomState(room);
        }
        return true;
    }

    if (type == QStringLiteral("room_availability_result")) {
        const QJsonValue roomsValue =
            payload.value(QStringLiteral("rooms"));
        if (!roomsValue.isArray()) {
            NotifyError("invalid_room_availability_result",
                        "Room availability entries are missing.");
            return true;
        }
        SignalingRoomAvailabilityResult result;
        const QJsonArray rooms = roomsValue.toArray();
        result.rooms.reserve(static_cast<std::size_t>(rooms.size()));
        for (const QJsonValue& value : rooms) {
            if (!value.isObject()) {
                NotifyError("invalid_room_availability_result",
                            "A room availability entry is invalid.");
                return true;
            }
            const QJsonObject entry = value.toObject();
            SignalingRoomAvailability availability;
            availability.roomId = ToString(
                entry.value(QStringLiteral("roomId")).toString());
            if (availability.roomId.empty() ||
                !entry.value(QStringLiteral("exists")).isBool() ||
                !entry.value(QStringLiteral("joinable")).isBool()) {
                NotifyError("invalid_room_availability_result",
                            "Room availability fields are invalid.");
                return true;
            }
            availability.exists = entry.value(
                QStringLiteral("exists")).toBool();
            availability.joinable = entry.value(
                QStringLiteral("joinable")).toBool();
            result.rooms.push_back(std::move(availability));
        }
        if (observer_) {
            observer_->OnRoomAvailabilityResult(result);
        }
        return true;
    }

    if (type == QStringLiteral("room_join_pending")) {
        SignalingRoomJoinPending pending;
        pending.roomId = ToString(
            payload.value(QStringLiteral("roomId")).toString());
        pending.requestId = ToString(
            payload.value(QStringLiteral("requestId")).toString());
        if (pending.roomId.empty() || pending.requestId.empty()) {
            NotifyError("invalid_room_join_pending",
                        "Room join-pending fields are invalid.");
            return true;
        }
        if (observer_) {
            observer_->OnRoomJoinPending(pending);
        }
        return true;
    }

    if (type == QStringLiteral("room_join_requested")) {
        RoomJoinRequest request;
        request.roomId = ToString(
            payload.value(QStringLiteral("roomId")).toString());
        request.requestId = ToString(
            payload.value(QStringLiteral("requestId")).toString());
        request.requesterDeviceId = ToString(
            payload.value(QStringLiteral("requesterDeviceId")).toString());
        request.requesterDeviceName = ToString(
            payload.value(QStringLiteral("requesterDeviceName")).toString());
        if (request.roomId.empty() || request.requestId.empty() ||
            request.requesterDeviceId.empty()) {
            NotifyError("invalid_room_join_request",
                        "Incoming room join-request fields are invalid.");
            return true;
        }
        if (observer_) {
            observer_->OnRoomJoinRequested(request);
        }
        return true;
    }

    if (type == QStringLiteral("room_join_result")) {
        SignalingRoomJoinResult result;
        result.roomId = ToString(
            payload.value(QStringLiteral("roomId")).toString());
        result.requestId = ToString(
            payload.value(QStringLiteral("requestId")).toString());
        if (!payload.value(QStringLiteral("accepted")).isBool()) {
            NotifyError("invalid_room_join_result",
                        "Room join-result acceptance is missing.");
            return true;
        }
        result.accepted =
            payload.value(QStringLiteral("accepted")).toBool();
        result.reasonCode = ToString(
            payload.value(QStringLiteral("reasonCode")).toString());
        result.reasonMessage = ToString(
            payload.value(QStringLiteral("reasonMessage")).toString());
        if (result.roomId.empty() ||
            (result.accepted && result.requestId.empty())) {
            NotifyError("invalid_room_join_result",
                        "Room join-result identity fields are invalid.");
            return true;
        }
        if (observer_) {
            observer_->OnRoomJoinResult(result);
        }
        return true;
    }

    if (type == QStringLiteral("room_closed")) {
        SignalingRoomClosed closed;
        closed.roomId = ToString(
            payload.value(QStringLiteral("roomId")).toString());
        closed.initiatorDeviceId = ToString(
            payload.value(QStringLiteral("initiatorDeviceId")).toString());
        closed.reasonCode = ToString(
            payload.value(QStringLiteral("reasonCode")).toString());
        if (closed.roomId.empty() || closed.initiatorDeviceId.empty() ||
            closed.reasonCode.empty()) {
            NotifyError("invalid_room_closed",
                        "Room-closed fields are invalid.");
            return true;
        }
        if (observer_) {
            observer_->OnRoomClosed(closed);
        }
        return true;
    }

    if (type == QStringLiteral("room_screen_share_granted")) {
        SignalingRoomScreenShareGranted granted;
        granted.roomId = ToString(
            payload.value(QStringLiteral("roomId")).toString());
        granted.grantId = ToString(
            payload.value(QStringLiteral("grantId")).toString());
        const double epoch =
            payload.value(QStringLiteral("epoch")).toDouble(-1.0);
        if (granted.roomId.empty() || granted.grantId.empty() ||
            epoch < 0.0 || epoch > 9007199254740991.0) {
            NotifyError("invalid_room_screen_share_granted",
                        "Room screen-share grant fields are invalid.");
            return true;
        }
        granted.epoch = static_cast<std::uint64_t>(epoch);
        if (observer_) {
            observer_->OnRoomScreenShareGranted(granted);
        }
        return true;
    }

    if (type == QStringLiteral("room_screen_share_switch_pending")) {
        SignalingRoomScreenShareSwitchPending pending;
        pending.roomId = ToString(
            payload.value(QStringLiteral("roomId")).toString());
        pending.requestId = ToString(
            payload.value(QStringLiteral("requestId")).toString());
        pending.screenSharerDeviceId = ToString(
            payload.value(QStringLiteral("screenSharerDeviceId"))
                .toString());
        if (pending.roomId.empty() || pending.requestId.empty() ||
            pending.screenSharerDeviceId.empty()) {
            NotifyError("invalid_room_screen_share_switch_pending",
                        "Screen-share takeover pending fields are invalid.");
            return true;
        }
        if (observer_) {
            observer_->OnRoomScreenShareSwitchPending(pending);
        }
        return true;
    }

    if (type == QStringLiteral("room_screen_share_switch_requested")) {
        RoomScreenShareSwitchRequest request;
        request.roomId = ToString(
            payload.value(QStringLiteral("roomId")).toString());
        request.requestId = ToString(
            payload.value(QStringLiteral("requestId")).toString());
        request.requesterDeviceId = ToString(
            payload.value(QStringLiteral("requesterDeviceId")).toString());
        request.requesterDeviceName = ToString(
            payload.value(QStringLiteral("requesterDeviceName")).toString());
        if (request.roomId.empty() || request.requestId.empty() ||
            request.requesterDeviceId.empty()) {
            NotifyError("invalid_room_screen_share_switch_requested",
                        "Screen-share takeover request fields are invalid.");
            return true;
        }
        if (observer_) {
            observer_->OnRoomScreenShareSwitchRequested(request);
        }
        return true;
    }

    if (type == QStringLiteral("room_screen_share_switch_result")) {
        SignalingRoomScreenShareSwitchResult result;
        result.roomId = ToString(
            payload.value(QStringLiteral("roomId")).toString());
        result.requestId = ToString(
            payload.value(QStringLiteral("requestId")).toString());
        const QJsonValue accepted =
            payload.value(QStringLiteral("accepted"));
        result.reasonCode = ToString(
            payload.value(QStringLiteral("reasonCode")).toString());
        result.reasonMessage = ToString(
            payload.value(QStringLiteral("reasonMessage")).toString());
        if (result.roomId.empty() || result.requestId.empty() ||
            !accepted.isBool()) {
            NotifyError("invalid_room_screen_share_switch_result",
                        "Screen-share takeover result fields are invalid.");
            return true;
        }
        result.accepted = accepted.toBool();
        if (observer_) {
            observer_->OnRoomScreenShareSwitchResult(result);
        }
        return true;
    }

    if (type == QStringLiteral("room_control_requested")) {
        RoomControlRequest request;
        request.roomId = ToString(
            payload.value(QStringLiteral("roomId")).toString());
        request.requestId = ToString(
            payload.value(QStringLiteral("requestId")).toString());
        request.requesterDeviceId = ToString(
            payload.value(QStringLiteral("requesterDeviceId")).toString());
        request.requesterDeviceName = ToString(
            payload.value(QStringLiteral("requesterDeviceName")).toString());
        if (request.roomId.empty() || request.requestId.empty() ||
            request.requesterDeviceId.empty()) {
            NotifyError("invalid_room_control_requested",
                        "Room control-request fields are invalid.");
            return true;
        }
        if (observer_) {
            observer_->OnRoomControlRequested(request);
        }
        return true;
    }

    if (type == QStringLiteral("room_control_result")) {
        SignalingRoomControlResult result;
        result.roomId = ToString(
            payload.value(QStringLiteral("roomId")).toString());
        result.requestId = ToString(
            payload.value(QStringLiteral("requestId")).toString());
        const QJsonValue accepted =
            payload.value(QStringLiteral("accepted"));
        result.reasonCode = ToString(
            payload.value(QStringLiteral("reasonCode")).toString());
        result.reasonMessage = ToString(
            payload.value(QStringLiteral("reasonMessage")).toString());
        if (result.roomId.empty() || result.requestId.empty() ||
            !accepted.isBool()) {
            NotifyError("invalid_room_control_result",
                        "Room control-result fields are invalid.");
            return true;
        }
        result.accepted = accepted.toBool();
        if (observer_) {
            observer_->OnRoomControlResult(result);
        }
        return true;
    }

    if (type == QStringLiteral("room_control_granted")) {
        SignalingRoomControlGranted granted;
        granted.roomId = ToString(
            payload.value(QStringLiteral("roomId")).toString());
        granted.grantId = ToString(
            payload.value(QStringLiteral("grantId")).toString());
        granted.screenSharerDeviceId = ToString(
            payload.value(QStringLiteral("screenSharerDeviceId")).toString());
        granted.controllerDeviceId = ToString(
            payload.value(QStringLiteral("controllerDeviceId")).toString());
        if (granted.roomId.empty() || granted.grantId.empty() ||
            granted.screenSharerDeviceId.empty() ||
            granted.controllerDeviceId.empty()) {
            NotifyError("invalid_room_control_granted",
                        "Room control-grant fields are invalid.");
            return true;
        }
        if (observer_) {
            observer_->OnRoomControlGranted(granted);
        }
        return true;
    }

    if (type == QStringLiteral("room_control_revoked")) {
        SignalingRoomControlRevoked revoked;
        revoked.roomId = ToString(
            payload.value(QStringLiteral("roomId")).toString());
        revoked.initiatorDeviceId = ToString(
            payload.value(QStringLiteral("initiatorDeviceId")).toString());
        revoked.reasonCode = ToString(
            payload.value(QStringLiteral("reasonCode")).toString());
        if (revoked.roomId.empty() ||
            revoked.initiatorDeviceId.empty() ||
            revoked.reasonCode.empty()) {
            NotifyError("invalid_room_control_revoked",
                        "Room control-revoked fields are invalid.");
            return true;
        }
        if (observer_) {
            observer_->OnRoomControlRevoked(revoked);
        }
        return true;
    }

    if (type == QStringLiteral("room_pair_ready")) {
        SignalingRoomPairReady ready;
        ready.pairId = sessionId;
        ready.roomId = ToString(
            payload.value(QStringLiteral("roomId")).toString());
        ready.peerDeviceId = ToString(
            payload.value(QStringLiteral("peerDeviceId")).toString());
        const QJsonValue offererValue =
            payload.value(QStringLiteral("localIsOfferer"));
        if (ready.pairId.empty() || ready.roomId.empty() ||
            ready.peerDeviceId.empty() || !offererValue.isBool()) {
            NotifyError("invalid_room_pair_ready",
                        "Room pair-ready fields are invalid.");
            return true;
        }
        ready.localIsOfferer = offererValue.toBool();
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
            observer_->OnRoomPairReady(ready);
        }
        return true;
    }

    if (type == QStringLiteral("room_pair_closed")) {
        SignalingRoomPairClosed closed;
        closed.pairId = sessionId;
        closed.roomId = ToString(
            payload.value(QStringLiteral("roomId")).toString());
        closed.peerDeviceId = ToString(
            payload.value(QStringLiteral("peerDeviceId")).toString());
        closed.initiatorDeviceId = ToString(
            payload.value(QStringLiteral("initiatorDeviceId")).toString());
        closed.reasonCode = ToString(
            payload.value(QStringLiteral("reasonCode")).toString());
        if (closed.pairId.empty() || closed.roomId.empty() ||
            closed.peerDeviceId.empty() ||
            closed.initiatorDeviceId.empty() ||
            closed.reasonCode.empty()) {
            NotifyError("invalid_room_pair_closed",
                        "Room pair-closed fields are invalid.");
            return true;
        }
        if (observer_) {
            observer_->OnRoomPairClosed(closed);
        }
        return true;
    }

    return false;
}

}  // namespace remote
