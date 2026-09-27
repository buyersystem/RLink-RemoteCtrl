// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "SignalServer.Internal.h"

namespace remote::signaling_server {
using namespace detail;

void SignalServer::Impl::CreateRoomPair(RoomState& room,
                    const QString& firstDeviceId,
                    const QString& secondDeviceId)
{
    if (firstDeviceId.isEmpty() || secondDeviceId.isEmpty() ||
        firstDeviceId == secondDeviceId) {
        return;
    }
    QString first = firstDeviceId;
    QString second = secondDeviceId;
    if (QString::compare(first, second, Qt::CaseSensitive) > 0) {
        std::swap(first, second);
    }
    for (auto pairIt = room.pairs.cbegin();
         pairIt != room.pairs.cend(); ++pairIt) {
        if (pairIt->firstDeviceId == first &&
            pairIt->secondDeviceId == second) {
            return;
        }
    }

    RoomPairState pair;
    pair.pairId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    pair.firstDeviceId = first;
    pair.secondDeviceId = second;
    pair.offererDeviceId = first;
    roomRegistry_.PairRooms().insert(pair.pairId, room.roomId);
    room.pairs.insert(pair.pairId, std::move(pair));
}

QString SignalServer::Impl::PairPeerDeviceId(const RoomPairState& pair,
                         const QString& deviceId) const
{
    if (pair.firstDeviceId == deviceId) {
        return pair.secondDeviceId;
    }
    if (pair.secondDeviceId == deviceId) {
        return pair.firstDeviceId;
    }
    return {};
}

bool SignalServer::Impl::RoomPairMembersAreOnline(const RoomState& room,
                              const RoomPairState& pair) const
{
    const auto first = room.members.constFind(pair.firstDeviceId);
    const auto second = room.members.constFind(pair.secondDeviceId);
    return first != room.members.cend() &&
           second != room.members.cend() &&
           first->disconnectedAtMs == 0 &&
           second->disconnectedAtMs == 0 &&
           devices_.value(pair.firstDeviceId, nullptr) &&
           devices_.value(pair.secondDeviceId, nullptr);
}

void SignalServer::Impl::SendRoomPairReadyToMember(const RoomState& room,
                               const RoomPairState& pair,
                               const QString& deviceId)
{
    QWebSocket* socket = devices_.value(deviceId, nullptr);
    const QString peerDeviceId = PairPeerDeviceId(pair, deviceId);
    if (!socket || peerDeviceId.isEmpty()) {
        return;
    }
    QJsonObject payload;
    payload.insert(QStringLiteral("roomId"), room.roomId);
    payload.insert(QStringLiteral("peerDeviceId"), peerDeviceId);
    payload.insert(QStringLiteral("localIsOfferer"),
                   pair.offererDeviceId == deviceId);
    payload.insert(QStringLiteral("iceServers"),
                   SerializeIceServers());
    SendEnvelope(socket, QStringLiteral("room_pair_ready"),
                 pair.pairId, payload);
}

void SignalServer::Impl::SendRoomPairReady(const RoomState& room,
                       const RoomPairState& pair)
{
    if (!RoomPairMembersAreOnline(room, pair)) {
        return;
    }
    const QString answererDeviceId =
        pair.offererDeviceId == pair.firstDeviceId
            ? pair.secondDeviceId
            : pair.firstDeviceId;
    // The answerer must create its local pair runtime before the offerer
    // is allowed to generate SDP. QWebSocket preserves message order for
    // each connection, and the offerer callback runs asynchronously.
    SendRoomPairReadyToMember(room, pair, answererDeviceId);
    SendRoomPairReadyToMember(room, pair, pair.offererDeviceId);
}

void SignalServer::Impl::SendRoomPairsReadyForMember(const RoomState& room,
                                 const QString& deviceId)
{
    QStringList pairIds = room.pairs.keys();
    pairIds.sort(Qt::CaseSensitive);
    for (const auto& pairId : pairIds) {
        const RoomPairState& pair = room.pairs[pairId];
        if (pair.firstDeviceId == deviceId ||
            pair.secondDeviceId == deviceId) {
            SendRoomPairReady(room, pair);
        }
    }
}

void SignalServer::Impl::SendRoomPairClosedToMember(const RoomState& room,
                                const RoomPairState& pair,
                                const QString& deviceId,
                                const QString& initiatorDeviceId,
                                const QString& reasonCode)
{
    QWebSocket* socket = devices_.value(deviceId, nullptr);
    const QString peerDeviceId = PairPeerDeviceId(pair, deviceId);
    if (!socket || peerDeviceId.isEmpty()) {
        return;
    }
    QJsonObject payload;
    payload.insert(QStringLiteral("roomId"), room.roomId);
    payload.insert(QStringLiteral("peerDeviceId"), peerDeviceId);
    payload.insert(QStringLiteral("initiatorDeviceId"),
                   initiatorDeviceId);
    payload.insert(QStringLiteral("reasonCode"), reasonCode.left(64));
    SendEnvelope(socket, QStringLiteral("room_pair_closed"),
                 pair.pairId, payload);
}

void SignalServer::Impl::CloseRoomPair(RoomState& room,
                   const QString& pairId,
                   const QString& initiatorDeviceId,
                   const QString& reasonCode)
{
    auto pairIt = room.pairs.find(pairId);
    if (pairIt == room.pairs.end()) {
        return;
    }
    const RoomPairState pair = *pairIt;
    const QString finalReason = reasonCode.isEmpty()
        ? QStringLiteral("room_pair_closed")
        : reasonCode.left(64);
    SendRoomPairClosedToMember(room, pair, pair.firstDeviceId,
                               initiatorDeviceId, finalReason);
    SendRoomPairClosedToMember(room, pair, pair.secondDeviceId,
                               initiatorDeviceId, finalReason);
    roomRegistry_.PairRooms().remove(pair.pairId);
    room.pairs.erase(pairIt);
}

void SignalServer::Impl::CloseRoomPairsForMember(RoomState& room,
                             const QString& deviceId,
                             const QString& initiatorDeviceId,
                             const QString& reasonCode)
{
    QStringList pairsToClose;
    for (auto pairIt = room.pairs.cbegin();
         pairIt != room.pairs.cend(); ++pairIt) {
        if (pairIt->firstDeviceId == deviceId ||
            pairIt->secondDeviceId == deviceId) {
            pairsToClose.append(pairIt.key());
        }
    }
    for (const auto& pairId : pairsToClose) {
        CloseRoomPair(room, pairId, initiatorDeviceId, reasonCode);
    }
}

void SignalServer::Impl::CloseAllRoomPairs(RoomState& room,
                       const QString& initiatorDeviceId,
                       const QString& reasonCode)
{
    const QStringList pairIds = room.pairs.keys();
    for (const auto& pairId : pairIds) {
        CloseRoomPair(room, pairId, initiatorDeviceId, reasonCode);
    }
}

}  // namespace remote::signaling_server
