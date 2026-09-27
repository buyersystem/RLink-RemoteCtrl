// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RoomRegistry.h"

namespace remote::signaling_server {

void RoomRegistry::Reserve(qsizetype capacity)
{
    rooms_.reserve(capacity);
    deviceRooms_.reserve(capacity);
    pairRooms_.reserve(capacity);
    joinRequests_.reserve(capacity);
    pendingJoinRequestsByDevice_.reserve(capacity);
}

void RoomRegistry::Clear()
{
    rooms_.clear();
    deviceRooms_.clear();
    pairRooms_.clear();
    joinRequests_.clear();
    pendingJoinRequestsByDevice_.clear();
}

qsizetype RoomRegistry::Size() const
{
    return rooms_.size();
}

qsizetype RoomRegistry::PairSize() const
{
    return pairRooms_.size();
}

qsizetype RoomRegistry::JoinRequestSize() const
{
    return joinRequests_.size();
}

RoomRegistry::RoomMap& RoomRegistry::Rooms()
{
    return rooms_;
}

const RoomRegistry::RoomMap& RoomRegistry::Rooms() const
{
    return rooms_;
}

RoomRegistry::DeviceRoomMap& RoomRegistry::DeviceRooms()
{
    return deviceRooms_;
}

const RoomRegistry::DeviceRoomMap& RoomRegistry::DeviceRooms() const
{
    return deviceRooms_;
}

RoomRegistry::PairRoomMap& RoomRegistry::PairRooms()
{
    return pairRooms_;
}

const RoomRegistry::PairRoomMap& RoomRegistry::PairRooms() const
{
    return pairRooms_;
}

RoomRegistry::JoinRequestMap& RoomRegistry::JoinRequests()
{
    return joinRequests_;
}

const RoomRegistry::JoinRequestMap& RoomRegistry::JoinRequests() const
{
    return joinRequests_;
}

RoomRegistry::PendingJoinRequestMap&
RoomRegistry::PendingJoinRequestsByDevice()
{
    return pendingJoinRequestsByDevice_;
}

const RoomRegistry::PendingJoinRequestMap&
RoomRegistry::PendingJoinRequestsByDevice() const
{
    return pendingJoinRequestsByDevice_;
}

bool RunRoomRegistrySelfTest(QString* errorMessage)
{
    RoomRegistry registry;
    registry.Reserve(4);

    RoomState room;
    room.roomId = QStringLiteral("room-a");
    room.ownerDeviceId = QStringLiteral("device-a");
    RoomMemberState owner;
    owner.deviceId = room.ownerDeviceId;
    room.members.insert(owner.deviceId, owner);
    registry.Rooms().insert(room.roomId, room);
    registry.DeviceRooms().insert(owner.deviceId, room.roomId);

    RoomPairState pair;
    pair.pairId = QStringLiteral("pair-a");
    pair.firstDeviceId = QStringLiteral("device-a");
    pair.secondDeviceId = QStringLiteral("device-b");
    registry.Rooms()[room.roomId].pairs.insert(pair.pairId, pair);
    registry.PairRooms().insert(pair.pairId, room.roomId);

    RoomJoinRequestState request;
    request.requestId = QStringLiteral("request-a");
    request.roomId = room.roomId;
    request.requesterDeviceId = QStringLiteral("device-c");
    registry.JoinRequests().insert(request.requestId, request);
    registry.PendingJoinRequestsByDevice().insert(
        request.requesterDeviceId, request.requestId);

    if (registry.Size() != 1 || registry.PairSize() != 1 ||
        registry.JoinRequestSize() != 1 ||
        registry.DeviceRooms().value(owner.deviceId) != room.roomId ||
        registry.PairRooms().value(pair.pairId) != room.roomId ||
        registry.PendingJoinRequestsByDevice().value(
            request.requesterDeviceId) != request.requestId) {
        if (errorMessage) {
            *errorMessage = QStringLiteral(
                "Room registry state or indexes were not retained.");
        }
        return false;
    }

    registry.Clear();
    if (registry.Size() != 0 || registry.PairSize() != 0 ||
        registry.JoinRequestSize() != 0 ||
        !registry.DeviceRooms().isEmpty() ||
        !registry.PendingJoinRequestsByDevice().isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral(
                "Clearing the room registry left stale indexes.");
        }
        return false;
    }
    return true;
}

}  // namespace remote::signaling_server
