// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QHash>
#include <QString>

namespace remote::signaling_server {

struct RoomMemberState {
    QString deviceId;
    QString deviceName;
    QString recoveryToken;
    qint64 disconnectedAtMs = 0;
    bool cameraPublishing = false;
    bool microphonePublishing = false;
};

struct RoomPairState {
    QString pairId;
    QString firstDeviceId;
    QString secondDeviceId;
    QString offererDeviceId;
};

struct RoomState {
    QString roomId;
    QString ownerDeviceId;
    int capacity = 2;
    QString screenShareState = QStringLiteral("idle");
    QString screenSharerDeviceId;
    QString pendingScreenSharerDeviceId;
    quint64 screenShareEpoch = 0;
    QString screenShareGrantId;
    QString screenShareSwitchRequestId;
    QString screenShareSwitchRequesterDeviceId;
    qint64 screenShareSwitchRequestExpiresAtMs = 0;
    QString pendingControllerDeviceId;
    QString controlRequestId;
    qint64 controlRequestExpiresAtMs = 0;
    QString activeControllerDeviceId;
    QString controlGrantId;
    QHash<QString, RoomMemberState> members;
    QHash<QString, RoomPairState> pairs;
};

struct RoomJoinRequestState {
    QString requestId;
    QString roomId;
    QString requesterDeviceId;
    QString requesterDeviceName;
    qint64 expiresAtMs = 0;
};

class RoomRegistry final {
public:
    using RoomMap = QHash<QString, RoomState>;
    using DeviceRoomMap = QHash<QString, QString>;
    using PairRoomMap = QHash<QString, QString>;
    using JoinRequestMap = QHash<QString, RoomJoinRequestState>;
    using PendingJoinRequestMap = QHash<QString, QString>;

    void Reserve(qsizetype capacity);
    void Clear();
    qsizetype Size() const;
    qsizetype PairSize() const;
    qsizetype JoinRequestSize() const;

    RoomMap& Rooms();
    const RoomMap& Rooms() const;
    DeviceRoomMap& DeviceRooms();
    const DeviceRoomMap& DeviceRooms() const;
    PairRoomMap& PairRooms();
    const PairRoomMap& PairRooms() const;
    JoinRequestMap& JoinRequests();
    const JoinRequestMap& JoinRequests() const;
    PendingJoinRequestMap& PendingJoinRequestsByDevice();
    const PendingJoinRequestMap& PendingJoinRequestsByDevice() const;

private:
    RoomMap rooms_;
    DeviceRoomMap deviceRooms_;
    PairRoomMap pairRooms_;
    JoinRequestMap joinRequests_;
    PendingJoinRequestMap pendingJoinRequestsByDevice_;
};

bool RunRoomRegistrySelfTest(QString* errorMessage);

}  // namespace remote::signaling_server
