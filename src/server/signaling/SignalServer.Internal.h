// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include "SignalServer.h"

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <utility>

#include <QAbstractSocket>
#include <QDateTime>
#include <QElapsedTimer>
#include <QFile>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPointer>
#include <QQueue>
#include <QSet>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslKey>
#include <QSslSocket>
#include <QTextStream>
#include <QTimer>
#include <QUuid>
#include <QWebSocket>
#include <QWebSocketProtocol>
#include <QWebSocketServer>

#include "AccessTokenService.h"
#include "DirectSessionRegistry.h"
#include "RoomRegistry.h"
#include "SignalServerSupport.h"
#include "SlidingWindowRateLimiter.h"
#include "src/server/auth/LogtoWebhookServer.h"
#include "src/server/auth/LogtoManagementClient.h"
#include "src/server/auth/LogtoUserInfoClient.h"
#include "src/server/persistence/IdentityStore.h"

namespace remote::signaling_server {
using namespace detail;

class SignalServer::Impl final {
public:
    explicit Impl(SignalServerConfig config);
    bool Start(QString* error);
    void Stop();
    bool IsListening() const;
    quint16 ServerPort() const;
    quint16 WebhookPort() const;
private:
    struct ClientState {
        QWebSocket* socket = nullptr;
        AccessTokenClaims claims;
        bool authenticated = false;
        bool authenticationPending = false;
        bool accountDeletionPending = false;
        bool messageAuthenticated = false;
        bool registered = false;
        QPointer<QNetworkReply> userInfoReply;
        qint64 trustedUserId = 0;
        QString logtoSubject;
        QString peerKey;
        QString deviceName;
        QSet<QByteArray> receivedMessageIds;
        QQueue<QByteArray> receivedMessageOrder;
        qint64 lastActivityMs = 0;
        qint64 authenticationDeadlineMs = 0;
    };

    using SessionState = DirectSessionState;

    void HandleMessageAuthentication(
        ClientState* client,
        const QJsonObject& payload);
    void SendAuthenticationError(
        QWebSocket* socket,
        const QString& reason,
        const QString& message,
        bool retryable);
    void SendAccountDeletionResult(
        QWebSocket* socket,
        bool deleted,
        const QString& code,
        const QString& message,
        bool retryable);
    void HandleAccountDelete(ClientState* client);
    void AcceptConnections();
    void RejectConnection(QWebSocket* socket, const QString& reason);
    void OnTextMessage(QWebSocket* socket, const QString& message);
    QJsonObject BuildOwnedDevicesPayloadFromRecords(
        ClientState* recipient,
        quint64 revision,
        const QList<remote::server_persistence::OwnedDeviceRecord>& owned,
        const QString& storeError);
    QJsonObject BuildOwnedDevicesPayload(ClientState* recipient,
                                         quint64 revision);
    void HandleMyDevicesRequest(ClientState* client);
    void BroadcastOwnedDevices(qint64 trustedUserId);
    void HandleRegister(ClientState* client, const QJsonObject& payload);
    void HandleRoomCreate(ClientState* creator,
                          const QJsonObject& payload);
    void HandleRoomAvailabilityQuery(ClientState* client,
                                     const QJsonObject& payload);
    void HandleRoomJoinRequest(ClientState* requester,
                               const QJsonObject& payload);
    void HandleRoomJoinResponse(ClientState* owner,
                                const QJsonObject& payload);
    void HandleRoomSetCapacity(ClientState* owner,
                               const QJsonObject& payload);
    void HandleRoomLeave(ClientState* memberClient,
                         const QJsonObject& payload);
    void HandleRoomMediaState(ClientState* memberClient,
                              const QJsonObject& payload);
    void HandleRoomResume(ClientState* client,
                          const QJsonObject& payload);
    bool IsOnlineRoomMember(const RoomState& room,
                            const QString& deviceId) const;
    void SendRoomScreenShareGranted(QWebSocket* socket,
                                    const RoomState& room);
    void SendRoomScreenShareSwitchPending(QWebSocket* socket,
                                          const RoomState& room);
    void SendRoomScreenShareSwitchRequested(QWebSocket* socket,
                                            const RoomState& room);
    void SendRoomScreenShareSwitchResult(
        QWebSocket* socket,
        const RoomState& room,
        const QString& requestId,
        bool accepted,
        const QString& reasonCode,
        const QString& reasonMessage);
    void CancelPendingRoomScreenShareSwitch(
        RoomState& room,
        const QString& reasonCode,
        const QString& reasonMessage);
    void SendRoomControlResult(QWebSocket* socket,
                               const RoomState& room,
                               const QString& requestId,
                               bool accepted,
                               const QString& reasonCode,
                               const QString& reasonMessage);
    void SendRoomControlGranted(QWebSocket* socket,
                                const RoomState& room);
    void SendRoomControlRevoked(QWebSocket* socket,
                                const RoomState& room,
                                const QString& initiatorDeviceId,
                                const QString& reasonCode);
    void CancelPendingRoomControl(RoomState& room,
                                  const QString& reasonCode,
                                  const QString& reasonMessage);
    void RevokeRoomControl(RoomState& room,
                           const QString& initiatorDeviceId,
                           const QString& reasonCode);
    void ResetRoomScreenShare(RoomState& room,
                              const QString& initiatorDeviceId,
                              const QString& reasonCode);
    void HandleRoomScreenShareRequest(ClientState* client,
                                      const QJsonObject& payload);
    void HandleRoomScreenShareReady(ClientState* client,
                                    const QJsonObject& payload);
    void HandleRoomScreenShareSwitchResponse(
        ClientState* client,
        const QJsonObject& payload);
    void HandleRoomScreenShareSwitchCancel(
        ClientState* client,
        const QJsonObject& payload);
    void HandleRoomScreenShareStop(ClientState* client,
                                   const QJsonObject& payload);
    void HandleRoomControlRequest(ClientState* client,
                                  const QJsonObject& payload);
    void HandleRoomControlResponse(ClientState* client,
                                   const QJsonObject& payload);
    void HandleRoomControlRelease(ClientState* client,
                                  const QJsonObject& payload);
    QJsonArray SerializeIceServers() const;
    void CreateRoomPair(RoomState& room,
                        const QString& firstDeviceId,
                        const QString& secondDeviceId);
    QString PairPeerDeviceId(const RoomPairState& pair,
                             const QString& deviceId) const;
    bool RoomPairMembersAreOnline(const RoomState& room,
                                  const RoomPairState& pair) const;
    void SendRoomPairReadyToMember(const RoomState& room,
                                   const RoomPairState& pair,
                                   const QString& deviceId);
    void SendRoomPairReady(const RoomState& room,
                           const RoomPairState& pair);
    void SendRoomPairsReadyForMember(const RoomState& room,
                                     const QString& deviceId);
    void SendRoomPairClosedToMember(const RoomState& room,
                                    const RoomPairState& pair,
                                    const QString& deviceId,
                                    const QString& initiatorDeviceId,
                                    const QString& reasonCode);
    void CloseRoomPair(RoomState& room,
                       const QString& pairId,
                       const QString& initiatorDeviceId,
                       const QString& reasonCode);
    void CloseRoomPairsForMember(RoomState& room,
                                 const QString& deviceId,
                                 const QString& initiatorDeviceId,
                                 const QString& reasonCode);
    void CloseAllRoomPairs(RoomState& room,
                           const QString& initiatorDeviceId,
                           const QString& reasonCode);
    QJsonObject SerializeRoom(const RoomState& room) const;
    void SendRoomReady(QWebSocket* socket,
                       const RoomState& room,
                       const QString& recoveryToken);
    void BroadcastRoomState(const RoomState& room);
    void SendRoomJoinPending(QWebSocket* socket,
                             const QString& roomId,
                             const QString& requestId);
    void SendRoomJoinResult(QWebSocket* socket,
                            const QString& roomId,
                            const QString& requestId,
                            bool accepted,
                            const QString& reasonCode,
                            const QString& reasonMessage);
    void SendRoomClosed(QWebSocket* socket,
                        const QString& roomId,
                        const QString& initiatorDeviceId,
                        const QString& reasonCode);
    void CloseRoom(const QString& roomId,
                   const QString& initiatorDeviceId,
                   const QString& reasonCode);
    void CancelPendingRoomJoinForDevice(const QString& deviceId);
    void HandleSessionRequest(ClientState* requester,
                              const QJsonObject& payload);
    void HandleSessionResponse(ClientState* target,
                               const QString& sessionId,
                               const QJsonObject& payload);
    void SendSessionReady(QWebSocket* socket,
                          const QString& sessionId,
                          const QString& peerDeviceId,
                          const QString& recoveryToken);
    void HandleSessionResume(ClientState* client,
                             const QString& sessionId,
                             const QJsonObject& payload);
    void SendSessionResumed(QWebSocket* socket,
                            const QString& sessionId,
                            const QString& peerDeviceId,
                            const QString& resumedDeviceId);
    bool RelayRoomPairMessage(ClientState* sender,
                              const QString& pairId,
                              const QString& type,
                              const QJsonObject& payload);
    void RelaySessionMessage(ClientState* sender,
                             const QString& sessionId,
                             const QString& type,
                             const QJsonObject& payload);
    void CloseSession(ClientState* sender,
                      const QString& sessionId,
                      const QString& type,
                      const QJsonObject& payload);
    void SendSessionEnded(QWebSocket* socket,
                          const QString& sessionId,
                          const QString& initiatorDeviceId,
                          const QString& reasonCode,
                          const QString& disposition);
    void OnDisconnected(QWebSocket* socket);
    void MarkDeviceDisconnected(const QString& deviceId);
    void SweepIdleClients();
    void SweepPendingSessions(qint64 now);
    void SweepRecoverySessions(qint64 now);
    void SweepRoomJoinRequests(qint64 now);
    void SweepRoomControlRequests(qint64 now);
    void SweepRoomScreenShareSwitchRequests(qint64 now);
    void SweepRoomRecoveries(qint64 now);
    ClientState* FindClient(QWebSocket* socket);
    bool RememberMessageId(ClientState* client, const QByteArray& messageId)
    {
        if (client->receivedMessageIds.contains(messageId)) {
            return false;
        }
        client->receivedMessageIds.insert(messageId);
        client->receivedMessageOrder.enqueue(messageId);
        ++totalRememberedMessageIds_;
        if (client->receivedMessageOrder.size() >
            kMaximumRememberedMessageIds) {
            if (client->receivedMessageIds.remove(
                    client->receivedMessageOrder.dequeue())) {
                --totalRememberedMessageIds_;
            }
        }
        return true;
    }

    void CompleteClientAuthentication(ClientState* client)
    {
        if (!client || client->authenticated) {
            return;
        }
        client->authenticated = true;
        client->authenticationDeadlineMs = 0;
        unauthenticatedConnectionCount_ =
            std::max(0, unauthenticatedConnectionCount_ - 1);
    }

    bool DeviceHasSession(const QString& deviceId) const
    {
        return directSessions_.HasDeviceSession(deviceId);
    }

    bool InsertSession(SessionState session)
    {
        return directSessions_.Insert(std::move(session));
    }

    void RemoveSession(const QString& sessionId)
    {
        directSessions_.Remove(sessionId);
    }

    void ClearSessions()
    {
        directSessions_.Clear();
    }

    void AddAccountConnection(ClientState* client)
    {
        if (!client || !client->socket || client->trustedUserId <= 0) {
            return;
        }
        accountConnections_[client->trustedUserId].insert(client->socket);
    }

    void RemoveAccountConnection(const ClientState* client)
    {
        if (!client || !client->socket || client->trustedUserId <= 0) {
            return;
        }
        auto accountIt = accountConnections_.find(client->trustedUserId);
        if (accountIt == accountConnections_.end()) {
            return;
        }
        accountIt->remove(client->socket);
        if (accountIt->isEmpty()) {
            accountConnections_.erase(accountIt);
        }
    }

    bool DeviceHasRoom(const QString& deviceId) const
    {
        return roomRegistry_.DeviceRooms().contains(deviceId);
    }

    bool ReleaseDisconnectedRoomForFreshStart(const QString& deviceId)
    {
        const QString roomId = roomRegistry_.DeviceRooms().value(deviceId);
        auto roomIt = roomRegistry_.Rooms().find(roomId);
        if (roomIt == roomRegistry_.Rooms().end()) {
            if (!roomId.isEmpty()) {
                roomRegistry_.DeviceRooms().remove(deviceId);
            }
            return false;
        }
        const auto memberIt = roomIt->members.constFind(deviceId);
        if (memberIt == roomIt->members.cend() ||
            memberIt->disconnectedAtMs == 0) {
            return false;
        }

        const QString reasonCode =
            QStringLiteral("stale_membership_replaced");
        if (roomIt->ownerDeviceId == deviceId) {
            CloseRoom(roomId, deviceId, reasonCode);
            return true;
        }
        if (roomIt->screenSharerDeviceId == deviceId ||
            roomIt->pendingScreenSharerDeviceId == deviceId) {
            ResetRoomScreenShare(*roomIt, deviceId, reasonCode);
        } else {
            if (roomIt->pendingControllerDeviceId == deviceId) {
                CancelPendingRoomControl(
                    *roomIt, reasonCode,
                    QStringLiteral("The stale room member was replaced."));
            }
            if (roomIt->activeControllerDeviceId == deviceId) {
                RevokeRoomControl(*roomIt, deviceId, reasonCode);
            }
        }
        CloseRoomPairsForMember(*roomIt, deviceId, deviceId, reasonCode);
        roomIt->members.remove(deviceId);
        roomRegistry_.DeviceRooms().remove(deviceId);
        BroadcastRoomState(*roomIt);
        return true;
    }

    bool DeleteLocalAccount(const QString& subject, QString* error)
    {
        bool existed = false;
        if (!identityStore_.DeleteUserByLogtoSubject(
                subject, &existed, error)) {
            return false;
        }

        QList<QPointer<QWebSocket>> accountSockets;
        QSet<QString> deviceIds;
        for (const auto& [socket, state] : clients_) {
            if (state->logtoSubject == subject) {
                accountSockets.append(socket);
                if (state->registered && !state->claims.deviceId.isEmpty()) {
                    deviceIds.insert(state->claims.deviceId);
                }
            }
        }

        for (const QString& deviceId : deviceIds) {
            CancelPendingRoomJoinForDevice(deviceId);
            const QString roomId = roomRegistry_.DeviceRooms().value(deviceId);
            auto roomIt = roomRegistry_.Rooms().find(roomId);
            if (roomIt != roomRegistry_.Rooms().end()) {
                const QString reason = QStringLiteral("account_deleted");
                if (roomIt->ownerDeviceId == deviceId) {
                    CloseRoom(roomId, deviceId, reason);
                } else {
                    if (roomIt->screenSharerDeviceId == deviceId ||
                        roomIt->pendingScreenSharerDeviceId == deviceId) {
                        ResetRoomScreenShare(*roomIt, deviceId, reason);
                    } else {
                        if (roomIt->pendingControllerDeviceId == deviceId) {
                            CancelPendingRoomControl(
                                *roomIt, reason,
                                QStringLiteral(
                                    "The account was deleted."));
                        }
                        if (roomIt->activeControllerDeviceId == deviceId) {
                            RevokeRoomControl(*roomIt, deviceId, reason);
                        }
                    }
                    CloseRoomPairsForMember(
                        *roomIt, deviceId, deviceId, reason);
                    roomIt->members.remove(deviceId);
                    roomRegistry_.DeviceRooms().remove(deviceId);
                    BroadcastRoomState(*roomIt);
                }
            }

            const QString sessionId =
                directSessions_.SessionIdForDevice(deviceId);
            const auto& sessions = directSessions_.Sessions();
            const auto sessionIt = sessions.constFind(sessionId);
            if (sessionIt != sessions.cend()) {
                const QString peerDeviceId =
                    sessionIt->requesterDeviceId == deviceId
                        ? sessionIt->targetDeviceId
                        : sessionIt->requesterDeviceId;
                if (QWebSocket* peer = devices_.value(peerDeviceId, nullptr)) {
                    SendSessionEnded(
                        peer, sessionIt.key(), deviceId,
                        QStringLiteral("account_deleted"),
                        QStringLiteral("closed"));
                }
                RemoveSession(sessionId);
            }
            devices_.remove(deviceId);
        }

        for (const QPointer<QWebSocket>& socket : accountSockets) {
            if (!socket) {
                continue;
            }
            SendEnvelope(socket, QStringLiteral("account_deleted"), {},
                         QJsonObject{{QStringLiteral("subject"), subject}});
            QTimer::singleShot(100, socket, [socket] {
                if (socket) {
                    socket->close(
                        QWebSocketProtocol::CloseCodeNormal,
                        QStringLiteral("account deleted"));
                }
            });
        }
        Q_UNUSED(existed);
        return true;
    }

    void SendError(QWebSocket* socket,
                   const QString& sessionId,
                   const QString& code,
                   const QString& message)
    {
        QJsonObject payload;
        payload.insert(QStringLiteral("code"), code);
        payload.insert(QStringLiteral("message"), message);
        SendEnvelope(socket, QStringLiteral("error"), sessionId, payload);
    }

    QString PeerRateLimitKey(const QWebSocket* socket) const
    {
        if (!socket) {
            return QStringLiteral("unknown");
        }
        const QHostAddress address = socket->peerAddress();
        bool isIpv4 = false;
        const quint32 ipv4 = address.toIPv4Address(&isIpv4);
        if (isIpv4) {
            return QHostAddress(ipv4).toString();
        }
        const QString text = address.toString().toLower();
        return text.isEmpty() ? QStringLiteral("unknown") : text;
    }

    bool AllowAuthenticationAttempt(QWebSocket* socket, qint64 now)
    {
        return authenticationIpRateLimiter_.TryAcquire(
            PeerRateLimitKey(socket), 1, now);
    }

    bool AllowRoomAvailabilityQuery(ClientState* client,
                                    int requestedRoomCount,
                                    qint64 now)
    {
        if (config_.disableBusinessRateLimitsForTest) {
            return true;
        }
        const QString ipKey = PeerRateLimitKey(client->socket);
        const QString userKey = client->trustedUserId > 0
            ? QString::number(client->trustedUserId)
            : QString();
        const QString deviceKey = client->claims.deviceId;
        const bool allowedByIp = availabilityIpRateLimiter_.CanAcquire(
            ipKey, requestedRoomCount, now);
        const bool allowedByUser = userKey.isEmpty() ||
            availabilityUserRateLimiter_.CanAcquire(
                userKey, requestedRoomCount, now);
        const bool allowedByDevice = deviceKey.isEmpty() ||
            availabilityDeviceRateLimiter_.CanAcquire(
                deviceKey, requestedRoomCount, now);
        if (!allowedByIp || !allowedByUser || !allowedByDevice) {
            return false;
        }
        availabilityIpRateLimiter_.Record(
            ipKey, requestedRoomCount, now);
        if (!userKey.isEmpty()) {
            availabilityUserRateLimiter_.Record(
                userKey, requestedRoomCount, now);
        }
        if (!deviceKey.isEmpty()) {
            availabilityDeviceRateLimiter_.Record(
                deviceKey, requestedRoomCount, now);
        }
        return true;
    }

    bool AllowRoomJoinAttempt(ClientState* client, qint64 now)
    {
        if (config_.disableBusinessRateLimitsForTest) {
            return true;
        }
        const QString ipKey = PeerRateLimitKey(client->socket);
        const QString userKey = client->trustedUserId > 0
            ? QString::number(client->trustedUserId)
            : QString();
        const QString deviceKey = client->claims.deviceId;
        const bool allowedByIp = roomJoinIpRateLimiter_.CanAcquire(
            ipKey, 1, now);
        const bool allowedByUser = userKey.isEmpty() ||
            roomJoinUserRateLimiter_.CanAcquire(userKey, 1, now);
        const bool allowedByDevice = deviceKey.isEmpty() ||
            roomJoinDeviceRateLimiter_.CanAcquire(deviceKey, 1, now);
        if (!allowedByIp || !allowedByUser || !allowedByDevice) {
            return false;
        }
        roomJoinIpRateLimiter_.Record(ipKey, 1, now);
        if (!userKey.isEmpty()) {
            roomJoinUserRateLimiter_.Record(userKey, 1, now);
        }
        if (!deviceKey.isEmpty()) {
            roomJoinDeviceRateLimiter_.Record(deviceKey, 1, now);
        }
        return true;
    }

    bool AllowAccountDeletion(ClientState* client, qint64 now)
    {
        return client->trustedUserId > 0 &&
               accountDeletionUserRateLimiter_.TryAcquire(
                   QString::number(client->trustedUserId), 1, now);
    }

    bool AllowAssistanceAttempt(ClientState* client,
                                const QString& targetDeviceId,
                                qint64 now)
    {
        if (config_.disableBusinessRateLimitsForTest) {
            return true;
        }
        const QString ipKey = PeerRateLimitKey(client->socket);
        const QString requesterKey = client->claims.deviceId;
        const bool allowedByIp = assistanceIpRateLimiter_.CanAcquire(
            ipKey, 1, now);
        const bool allowedByRequester =
            assistanceRequesterRateLimiter_.CanAcquire(
                requesterKey, 1, now);
        const bool allowedByTarget =
            assistanceTargetRateLimiter_.CanAcquire(
                targetDeviceId, 1, now);
        if (!allowedByIp || !allowedByRequester || !allowedByTarget) {
            return false;
        }
        assistanceIpRateLimiter_.Record(ipKey, 1, now);
        assistanceRequesterRateLimiter_.Record(requesterKey, 1, now);
        assistanceTargetRateLimiter_.Record(targetDeviceId, 1, now);
        return true;
    }

    void PruneRateLimiters(qint64 now)
    {
        authenticationIpRateLimiter_.Prune(now);
        availabilityIpRateLimiter_.Prune(now);
        availabilityUserRateLimiter_.Prune(now);
        availabilityDeviceRateLimiter_.Prune(now);
        roomJoinIpRateLimiter_.Prune(now);
        roomJoinUserRateLimiter_.Prune(now);
        roomJoinDeviceRateLimiter_.Prune(now);
        accountDeletionUserRateLimiter_.Prune(now);
        assistanceIpRateLimiter_.Prune(now);
        assistanceRequesterRateLimiter_.Prune(now);
        assistanceTargetRateLimiter_.Prune(now);
    }

    void ClearRateLimiters()
    {
        authenticationIpRateLimiter_.Clear();
        availabilityIpRateLimiter_.Clear();
        availabilityUserRateLimiter_.Clear();
        availabilityDeviceRateLimiter_.Clear();
        roomJoinIpRateLimiter_.Clear();
        roomJoinUserRateLimiter_.Clear();
        roomJoinDeviceRateLimiter_.Clear();
        accountDeletionUserRateLimiter_.Clear();
        assistanceIpRateLimiter_.Clear();
        assistanceRequesterRateLimiter_.Clear();
        assistanceTargetRateLimiter_.Clear();
    }

    qsizetype RateLimiterKeyCount() const
    {
        return authenticationIpRateLimiter_.keyCount() +
               availabilityIpRateLimiter_.keyCount() +
               availabilityUserRateLimiter_.keyCount() +
               availabilityDeviceRateLimiter_.keyCount() +
               roomJoinIpRateLimiter_.keyCount() +
               roomJoinUserRateLimiter_.keyCount() +
               roomJoinDeviceRateLimiter_.keyCount() +
               accountDeletionUserRateLimiter_.keyCount() +
               assistanceIpRateLimiter_.keyCount() +
               assistanceRequesterRateLimiter_.keyCount() +
               assistanceTargetRateLimiter_.keyCount();
    }

    void ProbeEventLoopLag()
    {
        if (!diagnosticsClock_.isValid()) {
            return;
        }
        const qint64 now = diagnosticsClock_.elapsed();
        if (diagnosticProbeExpectedMs_ > 0) {
            maximumEventLoopLagMs_ = std::max(
                maximumEventLoopLagMs_,
                std::max<qint64>(0, now - diagnosticProbeExpectedMs_));
        }
        diagnosticProbeExpectedMs_ = now + 1000;
    }

    void WriteDiagnostics()
    {
        const qint64 nowMs = diagnosticsClock_.isValid()
                                 ? diagnosticsClock_.elapsed()
                                 : 0;
        const qint64 intervalMs = lastDiagnosticsAtMs_ > 0
                                      ? nowMs - lastDiagnosticsAtMs_
                                      : 0;
        const qint64 receivedMessages =
            receivedMessagesTotal_ - lastReceivedMessagesTotal_;
        const qint64 receivedBytes =
            receivedBytesTotal_ - lastReceivedBytesTotal_;
        const qint64 sentMessages =
            sentMessagesTotal_ - lastSentMessagesTotal_;
        const qint64 sentBytes = sentBytesTotal_ - lastSentBytesTotal_;
        const double seconds = intervalMs > 0 ? intervalMs / 1000.0 : 0.0;
        const double receivedMessagesPerSecond =
            seconds > 0 ? receivedMessages / seconds : 0.0;
        const double sentMessagesPerSecond =
            seconds > 0 ? sentMessages / seconds : 0.0;
        const double receivedBitsPerSecond =
            seconds > 0 ? receivedBytes * 8.0 / seconds : 0.0;
        const double sentBitsPerSecond =
            seconds > 0 ? sentBytes * 8.0 / seconds : 0.0;
        qint64 queuedOutgoingBytes = 0;
        qint64 maximumClientQueuedOutgoingBytes = 0;
        for (const auto& [socket, state] : clients_) {
            Q_UNUSED(state);
            const qint64 queued = socket ? socket->bytesToWrite() : 0;
            queuedOutgoingBytes += queued;
            maximumClientQueuedOutgoingBytes =
                std::max(maximumClientQueuedOutgoingBytes, queued);
        }

        QJsonObject snapshot;
        snapshot.insert(QStringLiteral("timestampMs"),
                        QDateTime::currentMSecsSinceEpoch());
        snapshot.insert(
            QStringLiteral("timestampUtc"),
            QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
        snapshot.insert(QStringLiteral("connections"),
                        static_cast<qint64>(clients_.size()));
        snapshot.insert(
            QStringLiteral("authenticatedConnections"),
            static_cast<qint64>(clients_.size()) -
                unauthenticatedConnectionCount_);
        snapshot.insert(QStringLiteral("unauthenticatedConnections"),
                        unauthenticatedConnectionCount_);
        snapshot.insert(QStringLiteral("registeredDevices"), devices_.size());
        snapshot.insert(QStringLiteral("accounts"),
                        accountConnections_.size());
        snapshot.insert(QStringLiteral("onlineAccounts"),
                        accountConnections_.size());
        snapshot.insert(QStringLiteral("sessions"), directSessions_.Size());
        snapshot.insert(QStringLiteral("rooms"), roomRegistry_.Size());
        snapshot.insert(QStringLiteral("roomPairs"), roomRegistry_.PairSize());
        snapshot.insert(QStringLiteral("roomJoinRequests"),
                        roomRegistry_.JoinRequestSize());
        snapshot.insert(QStringLiteral("rememberedMessageIds"),
                        totalRememberedMessageIds_);
        snapshot.insert(QStringLiteral("rateLimiterKeys"),
                        RateLimiterKeyCount());
        snapshot.insert(QStringLiteral("maximumEventLoopLagMs"),
                        maximumEventLoopLagMs_);
        snapshot.insert(QStringLiteral("acceptedConnectionsTotal"),
                        acceptedConnectionsTotal_);
        snapshot.insert(QStringLiteral("rejectedConnectionsTotal"),
                        rejectedConnectionsTotal_);
        snapshot.insert(QStringLiteral("receivedMessagesTotal"),
                        receivedMessagesTotal_);
        snapshot.insert(QStringLiteral("receivedBytesTotal"),
                        receivedBytesTotal_);
        snapshot.insert(QStringLiteral("sentMessagesTotal"),
                        sentMessagesTotal_);
        snapshot.insert(QStringLiteral("sentBytesTotal"), sentBytesTotal_);
        snapshot.insert(QStringLiteral("queuedOutgoingBytes"),
                        queuedOutgoingBytes);
        snapshot.insert(QStringLiteral("maximumClientQueuedOutgoingBytes"),
                        maximumClientQueuedOutgoingBytes);
        snapshot.insert(QStringLiteral("receivedMessagesPerSecond"),
                        receivedMessagesPerSecond);
        snapshot.insert(QStringLiteral("sentMessagesPerSecond"),
                        sentMessagesPerSecond);
        snapshot.insert(QStringLiteral("receivedBitsPerSecond"),
                        receivedBitsPerSecond);
        snapshot.insert(QStringLiteral("sentBitsPerSecond"),
                        sentBitsPerSecond);
        const QByteArray line = QByteArrayLiteral("SIGNAL_SERVER_DIAGNOSTICS=") +
                                QJsonDocument(snapshot).toJson(
                                    QJsonDocument::Compact) +
                                '\n';
        if (StdoutIsTerminal()) {
            QTextStream output(stdout);
            output.flush();
            ClearStdoutTerminal();
            output << "RemoteC 信令服务器运行状态\n"
                   << "========================================\n"
                   << "更新时间："
                   << QDateTime::currentDateTime().toString(
                          QStringLiteral("yyyy-MM-dd HH:mm:ss"))
                   << '\n'
                   << "运行时间：" << FormatDuration(nowMs) << '\n'
                   << "服务地址：wss://" << config_.listenAddress.toString()
                   << ':' << server_.serverPort() << "/signaling\n\n"
                   << "【在线状态】\n"
                   << "在线账号：" << accountConnections_.size()
                   << "    在线设备：" << devices_.size() << '\n'
                   << "WSS 连接：" << clients_.size()
                   << "    已认证："
                   << static_cast<qint64>(clients_.size()) -
                          unauthenticatedConnectionCount_
                   << "    待认证：" << unauthenticatedConnectionCount_
                   << '\n'
                   << "累计接入：" << acceptedConnectionsTotal_
                   << "    累计拒绝：" << rejectedConnectionsTotal_ << "\n\n"
                   << "【业务状态】\n"
                   << "直接会话：" << directSessions_.Size()
                   << "    协助房间：" << roomRegistry_.Size()
                   << "    房间连接对：" << roomRegistry_.PairSize() << '\n'
                   << "等待入房：" << roomRegistry_.JoinRequestSize() << "\n\n"
                   << "【最近一个统计周期】\n"
                   << "接收：" << FormatBitRate(receivedBitsPerSecond)
                   << "    "
                   << QString::number(receivedMessagesPerSecond, 'f', 1)
                   << " 条/秒\n"
                   << "发送：" << FormatBitRate(sentBitsPerSecond)
                   << "    "
                   << QString::number(sentMessagesPerSecond, 'f', 1)
                   << " 条/秒\n\n"
                   << "【累计流量】\n"
                   << "接收：" << FormatBytes(receivedBytesTotal_)
                   << "    " << receivedMessagesTotal_ << " 条消息\n"
                   << "发送：" << FormatBytes(sentBytesTotal_)
                   << "    " << sentMessagesTotal_ << " 条消息\n\n"
                   << "【运行健康】\n"
                   << "事件循环最大延迟：" << maximumEventLoopLagMs_
                   << " ms\n"
                   << "发送积压：" << FormatBytes(queuedOutgoingBytes)
                   << "    单连接最大积压："
                   << FormatBytes(maximumClientQueuedOutgoingBytes) << '\n'
                   << "限流记录：" << RateLimiterKeyCount() << '\n'
                   << "========================================\n"
                   << "状态每 " << config_.diagnosticsIntervalMs / 1000
                   << " 秒自动刷新；详细记录保存在 logs 文件夹。"
                   << Qt::endl;
        } else {
            QTextStream(stdout) << line;
        }
        if (diagnosticsFile_.isOpen()) {
            diagnosticsFile_.write(line);
            diagnosticsFile_.flush();
        }
        lastDiagnosticsAtMs_ = nowMs;
        lastReceivedMessagesTotal_ = receivedMessagesTotal_;
        lastReceivedBytesTotal_ = receivedBytesTotal_;
        lastSentMessagesTotal_ = sentMessagesTotal_;
        lastSentBytesTotal_ = sentBytesTotal_;
        maximumEventLoopLagMs_ = 0;
    }

    void SendEnvelope(QWebSocket* socket,
                      const QString& type,
                      const QString& sessionId,
                      const QJsonObject& payload)
    {
        if (!socket ||
            socket->state() != QAbstractSocket::ConnectedState) {
            return;
        }
        QJsonObject envelope;
        envelope.insert(QStringLiteral("protocolVersion"), kProtocolVersion);
        envelope.insert(
            QStringLiteral("messageId"),
            QUuid::createUuid().toString(QUuid::WithoutBraces));
        envelope.insert(QStringLiteral("type"), type);
        if (!sessionId.isEmpty()) {
            envelope.insert(QStringLiteral("sessionId"), sessionId);
        }
        envelope.insert(QStringLiteral("payload"), payload);
        const QByteArray encoded =
            QJsonDocument(envelope).toJson(QJsonDocument::Compact);
        if (socket->sendTextMessage(QString::fromUtf8(encoded)) >= 0) {
            ++sentMessagesTotal_;
            sentBytesTotal_ += encoded.size();
        }
    }

    SignalServerConfig config_;
    AccessTokenService tokenService_;
    remote::server_auth::LogtoUserInfoClient userInfoClient_;
    remote::server_auth::LogtoManagementClient managementClient_;
    remote::server_auth::LogtoWebhookServer webhookServer_;
    remote::server_persistence::IdentityStore identityStore_;
    SlidingWindowRateLimiter authenticationIpRateLimiter_;
    SlidingWindowRateLimiter availabilityIpRateLimiter_;
    SlidingWindowRateLimiter availabilityUserRateLimiter_;
    SlidingWindowRateLimiter availabilityDeviceRateLimiter_;
    SlidingWindowRateLimiter roomJoinIpRateLimiter_;
    SlidingWindowRateLimiter roomJoinUserRateLimiter_;
    SlidingWindowRateLimiter roomJoinDeviceRateLimiter_;
    SlidingWindowRateLimiter accountDeletionUserRateLimiter_{
        kAccountDeletionUserRateLimit};
    SlidingWindowRateLimiter assistanceIpRateLimiter_{
        kAssistanceIpRateLimit};
    SlidingWindowRateLimiter assistanceRequesterRateLimiter_{
        kAssistanceRequesterRateLimit};
    SlidingWindowRateLimiter assistanceTargetRateLimiter_{
        kAssistanceTargetRateLimit};
    QWebSocketServer server_;
    QTimer maintenanceTimer_;
    QTimer diagnosticProbeTimer_;
    QTimer diagnosticsTimer_;
    QFile diagnosticsFile_;
    QElapsedTimer diagnosticsClock_;
    qint64 diagnosticProbeExpectedMs_ = 0;
    qint64 maximumEventLoopLagMs_ = 0;
    qint64 lastDiagnosticsAtMs_ = 0;
    qint64 lastReceivedMessagesTotal_ = 0;
    qint64 lastReceivedBytesTotal_ = 0;
    qint64 lastSentMessagesTotal_ = 0;
    qint64 lastSentBytesTotal_ = 0;
    qint64 acceptedConnectionsTotal_ = 0;
    qint64 rejectedConnectionsTotal_ = 0;
    qint64 receivedMessagesTotal_ = 0;
    qint64 receivedBytesTotal_ = 0;
    qint64 sentMessagesTotal_ = 0;
    qint64 sentBytesTotal_ = 0;
    std::unordered_map<QWebSocket*, std::unique_ptr<ClientState>> clients_;
    int unauthenticatedConnectionCount_ = 0;
    QHash<QString, int> connectionCountByIp_;
    qint64 totalRememberedMessageIds_ = 0;
    QHash<QString, QWebSocket*> devices_;
    QHash<qint64, QSet<QWebSocket*>> accountConnections_;
    quint64 ownedDevicesRevision_ = 1;
    DirectSessionRegistry directSessions_;
    RoomRegistry roomRegistry_;
};

}  // namespace remote::signaling_server
