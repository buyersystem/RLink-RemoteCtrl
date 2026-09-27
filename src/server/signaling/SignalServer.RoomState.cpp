// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "SignalServer.Internal.h"

namespace remote::signaling_server {
using namespace detail;

QJsonObject SignalServer::Impl::SerializeRoom(const RoomState& room) const
{
    QJsonObject result;
    result.insert(QStringLiteral("roomId"), room.roomId);
    result.insert(QStringLiteral("ownerDeviceId"), room.ownerDeviceId);
    result.insert(QStringLiteral("capacity"), room.capacity);
    result.insert(QStringLiteral("screenShareState"),
                  room.screenShareState);
    result.insert(QStringLiteral("screenSharerDeviceId"),
                  room.screenSharerDeviceId);
    result.insert(QStringLiteral("pendingScreenSharerDeviceId"),
                  room.pendingScreenSharerDeviceId);
    result.insert(QStringLiteral("screenShareEpoch"),
                  static_cast<qint64>(room.screenShareEpoch));
    result.insert(QStringLiteral("pendingControllerDeviceId"),
                  room.pendingControllerDeviceId);
    result.insert(QStringLiteral("activeControllerDeviceId"),
                  room.activeControllerDeviceId);

    QJsonArray members;
    QStringList memberIds = room.members.keys();
    memberIds.sort(Qt::CaseSensitive);
    for (const auto& deviceId : memberIds) {
        const RoomMemberState& member = room.members[deviceId];
        QJsonObject memberObject;
        memberObject.insert(QStringLiteral("deviceId"), member.deviceId);
        memberObject.insert(QStringLiteral("deviceName"),
                            member.deviceName);
        memberObject.insert(QStringLiteral("online"),
                            member.disconnectedAtMs == 0);
        memberObject.insert(QStringLiteral("cameraPublishing"),
                            member.cameraPublishing);
        memberObject.insert(QStringLiteral("microphonePublishing"),
                            member.microphonePublishing);
        members.append(memberObject);
    }
    result.insert(QStringLiteral("members"), members);
    return result;
}

void SignalServer::Impl::SendRoomReady(QWebSocket* socket,
                   const RoomState& room,
                   const QString& recoveryToken)
{
    QJsonObject payload;
    payload.insert(QStringLiteral("room"), SerializeRoom(room));
    payload.insert(QStringLiteral("recoveryToken"), recoveryToken);
    SendEnvelope(socket, QStringLiteral("room_ready"), {}, payload);
}

void SignalServer::Impl::BroadcastRoomState(const RoomState& room)
{
    QJsonObject payload;
    payload.insert(QStringLiteral("room"), SerializeRoom(room));
    for (auto memberIt = room.members.cbegin();
         memberIt != room.members.cend(); ++memberIt) {
        if (memberIt->disconnectedAtMs != 0) {
            continue;
        }
        if (QWebSocket* socket =
                devices_.value(memberIt.key(), nullptr)) {
            SendEnvelope(socket, QStringLiteral("room_state"), {},
                         payload);
        }
    }
}

void SignalServer::Impl::SendRoomJoinPending(QWebSocket* socket,
                         const QString& roomId,
                         const QString& requestId)
{
    QJsonObject payload;
    payload.insert(QStringLiteral("roomId"), roomId);
    payload.insert(QStringLiteral("requestId"), requestId);
    SendEnvelope(socket, QStringLiteral("room_join_pending"), {},
                 payload);
}

void SignalServer::Impl::SendRoomJoinResult(QWebSocket* socket,
                        const QString& roomId,
                        const QString& requestId,
                        bool accepted,
                        const QString& reasonCode,
                        const QString& reasonMessage)
{
    QJsonObject payload;
    payload.insert(QStringLiteral("roomId"), roomId);
    payload.insert(QStringLiteral("requestId"), requestId);
    payload.insert(QStringLiteral("accepted"), accepted);
    payload.insert(QStringLiteral("reasonCode"), reasonCode);
    payload.insert(QStringLiteral("reasonMessage"), reasonMessage);
    SendEnvelope(socket, QStringLiteral("room_join_result"), {},
                 payload);
}

void SignalServer::Impl::SendRoomClosed(QWebSocket* socket,
                    const QString& roomId,
                    const QString& initiatorDeviceId,
                    const QString& reasonCode)
{
    QJsonObject payload;
    payload.insert(QStringLiteral("roomId"), roomId);
    payload.insert(QStringLiteral("initiatorDeviceId"),
                   initiatorDeviceId);
    payload.insert(QStringLiteral("reasonCode"), reasonCode);
    SendEnvelope(socket, QStringLiteral("room_closed"), {}, payload);
}

void SignalServer::Impl::CloseRoom(const QString& roomId,
               const QString& initiatorDeviceId,
               const QString& reasonCode)
{
    auto roomIt = roomRegistry_.Rooms().find(roomId);
    if (roomIt == roomRegistry_.Rooms().end()) {
        return;
    }
    CloseAllRoomPairs(*roomIt, initiatorDeviceId, reasonCode);
    const RoomState closedRoom = *roomIt;
    roomRegistry_.Rooms().erase(roomIt);

    for (auto memberIt = closedRoom.members.cbegin();
         memberIt != closedRoom.members.cend(); ++memberIt) {
        if (roomRegistry_.DeviceRooms().value(memberIt.key()) == roomId) {
            roomRegistry_.DeviceRooms().remove(memberIt.key());
        }
    }

    QStringList joinRequestsToRemove;
    for (auto requestIt = roomRegistry_.JoinRequests().cbegin();
         requestIt != roomRegistry_.JoinRequests().cend(); ++requestIt) {
        if (requestIt->roomId == roomId) {
            joinRequestsToRemove.append(requestIt.key());
        }
    }
    for (const auto& requestId : joinRequestsToRemove) {
        const RoomJoinRequestState request =
            roomRegistry_.JoinRequests().take(requestId);
        if (roomRegistry_.PendingJoinRequestsByDevice().value(
                request.requesterDeviceId) == requestId) {
            roomRegistry_.PendingJoinRequestsByDevice().remove(
                request.requesterDeviceId);
        }
        if (QWebSocket* requesterSocket =
                devices_.value(request.requesterDeviceId, nullptr)) {
            SendRoomJoinResult(
                requesterSocket, roomId, requestId, false,
                QStringLiteral("room_closed"),
                QStringLiteral("The room closed before the join request was decided."));
        }
    }

    QString finalReasonCode = reasonCode.left(64);
    if (finalReasonCode.isEmpty()) {
        finalReasonCode = QStringLiteral("room_closed");
    }
    for (auto memberIt = closedRoom.members.cbegin();
         memberIt != closedRoom.members.cend(); ++memberIt) {
        if (QWebSocket* socket =
                devices_.value(memberIt.key(), nullptr)) {
            SendRoomClosed(socket, roomId, initiatorDeviceId,
                           finalReasonCode);
        }
    }
}

void SignalServer::Impl::CancelPendingRoomJoinForDevice(const QString& deviceId)
{
    const QString requestId =
        roomRegistry_.PendingJoinRequestsByDevice().take(deviceId);
    if (requestId.isEmpty()) {
        return;
    }
    const RoomJoinRequestState request =
        roomRegistry_.JoinRequests().take(requestId);
    if (request.requestId.isEmpty()) {
        return;
    }
    const auto roomIt = roomRegistry_.Rooms().constFind(request.roomId);
    if (roomIt == roomRegistry_.Rooms().cend()) {
        return;
    }
    const auto ownerMemberIt =
        roomIt->members.constFind(roomIt->ownerDeviceId);
    if (ownerMemberIt == roomIt->members.cend() ||
        ownerMemberIt->disconnectedAtMs != 0) {
        return;
    }
    if (QWebSocket* ownerSocket =
            devices_.value(roomIt->ownerDeviceId, nullptr)) {
        SendRoomJoinResult(
            ownerSocket, request.roomId, requestId, false,
            QStringLiteral("requester_offline"),
            QStringLiteral("The requesting device disconnected."));
    }
}

}  // namespace remote::signaling_server
