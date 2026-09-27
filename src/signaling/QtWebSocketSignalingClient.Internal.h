// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include "QtWebSocketSignalingClient.h"

#include <algorithm>
#include <atomic>
#include <deque>
#include <limits>
#include <string_view>
#include <unordered_set>
#include <utility>

#include <QAbstractSocket>
#include <QDateTime>
#include <QElapsedTimer>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMetaObject>
#include <QNetworkProxy>
#include <QNetworkRequest>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslError>
#include <QStringList>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QUuid>
#include <QWebSocket>

#include "SignalingJsonCodec.h"

namespace remote {
namespace signaling_client_detail {

inline constexpr int kProtocolVersion = 5;
inline constexpr quint64 kMaximumMessageBytes = 1024 * 1024;
inline constexpr std::size_t kMaximumAccessTokenBytes = 16 * 1024;
inline constexpr std::size_t kMaximumRememberedMessageIds = 4096;

using signaling_json::ReadStringArray;
using signaling_json::StringArray;
using signaling_json::ToQString;
using signaling_json::ToString;

SignalingOperationResult Success();
SignalingOperationResult Failure(std::string code, std::string message);
bool IsNineDigitPublicId(std::string_view value);
bool ShouldBypassProxy(const QUrl& endpoint);
QString PurposeToString(SessionPurpose purpose);
SessionPurpose PurposeFromString(const QString& purpose);
RoomScreenShareState ScreenShareStateFromString(
    const QString& state, bool* valid);
bool ReadRoomSnapshot(const QJsonValue& value, RoomSnapshot* snapshot);

}  // namespace signaling_client_detail

class QtWebSocketSignalingClient::Impl final {
public:
    Impl();
    ~Impl();
    void SetObserver(ISignalingClientObserver* observer);
    SignalingOperationResult Connect(const SignalingClientConfig& config);
    SignalingOperationResult UpdateAccessToken(
        const std::string& accessToken);
    SignalingOperationResult RequestAccountDeletion();
    void Disconnect();
    SignalingConnectionState State() const;
    SignalingOperationResult RequestOwnedDevices();
    SignalingOperationResult RequestSession(
        const std::string& targetDeviceId,
        SessionPurpose purpose,
        const std::vector<std::string>& permissions);
    SignalingOperationResult RequestOwnedDeviceSession(
        const std::string& targetDeviceId,
        SessionPurpose purpose,
        const std::vector<std::string>& permissions);
    SignalingOperationResult RequestAssistedSession(
        const std::string& targetDeviceId,
        const std::string& verificationCode,
        const std::vector<std::string>& permissions);
    SignalingOperationResult RespondToSession(
        const std::string& sessionId,
        bool accepted,
        const std::string& reasonCode);
    SignalingOperationResult CancelSession(
        const std::string& sessionId,
        const std::string& reasonCode);
    SignalingOperationResult CloseSession(
        const std::string& sessionId,
        const std::string& reasonCode);
    SignalingOperationResult ResumeSession(
        const std::string& sessionId,
        const std::string& recoveryToken);
    SignalingOperationResult CreateRoom(std::uint32_t capacity);
    SignalingOperationResult RequestRoomJoin(const std::string& roomId);
    SignalingOperationResult QueryRoomAvailability(
        const std::vector<std::string>& roomIds);
    SignalingOperationResult RespondToRoomJoin(
        const std::string& roomId,
        const std::string& requestId,
        bool accepted,
        const std::string& reasonCode);
    SignalingOperationResult SetRoomCapacity(const std::string& roomId,
                                             std::uint32_t capacity);
    SignalingOperationResult LeaveRoom(const std::string& roomId,
                                       const std::string& reasonCode);
    SignalingOperationResult ResumeRoom(const std::string& roomId,
                                        const std::string& recoveryToken);
    SignalingOperationResult SetRoomMediaState(
        const std::string& roomId,
        bool cameraPublishing,
        bool microphonePublishing);
    SignalingOperationResult RequestRoomScreenShare(
        const std::string& roomId);
    SignalingOperationResult ConfirmRoomScreenShare(
        const std::string& roomId,
        const std::string& grantId);
    SignalingOperationResult StopRoomScreenShare(
        const std::string& roomId,
        const std::string& grantId,
        const std::string& reasonCode);
    SignalingOperationResult RespondToRoomScreenShareSwitch(
        const std::string& roomId,
        const std::string& requestId,
        bool accepted,
        const std::string& reasonCode);
    SignalingOperationResult CancelRoomScreenShareSwitch(
        const std::string& roomId,
        const std::string& requestId,
        const std::string& reasonCode);
    SignalingOperationResult RequestRoomControl(const std::string& roomId);
    SignalingOperationResult RespondToRoomControl(
        const std::string& roomId,
        const std::string& requestId,
        bool accepted,
        const std::string& reasonCode);
    SignalingOperationResult ReleaseRoomControl(
        const std::string& roomId,
        const std::string& grantId,
        const std::string& reasonCode);
    void AbortConnectionForTesting();
    SignalingOperationResult SendDescription(
        const SignalingSessionDescription& description);
    SignalingOperationResult SendIceCandidate(
        const SignalingIceCandidate& candidate);
    SignalingOperationResult SendIceRestartRequest(
        const SignalingIceRestartRequest& request);
    SignalingOperationResult SendIceRestartCancel(
        const SignalingIceRestartCancel& cancel);
private:
    SignalingOperationResult SendSessionEndAction(
        const QString& type,
        const std::string& sessionId,
        const std::string& reasonCode);
    void SendHeartbeat();
    void OpenConfiguredSocket();
    void ScheduleReconnect();
    SignalingOperationResult SendDescriptionOnOwnerThread(
        const SignalingSessionDescription& description);
    SignalingOperationResult SendIceCandidateOnOwnerThread(
        const SignalingIceCandidate& candidate);
    void OnConnected();
    void BeginDeviceRegistration();
    void OnDisconnected();
    void OnTextMessage(const QString& message);
    void DispatchMessage(const QString& type,
                         const std::string& sessionId,
                         const QJsonObject& payload);
    bool DispatchConnectionMessage(const QString& type,
                                   const std::string& sessionId,
                                   const QJsonObject& payload);
    bool DispatchDirectSessionMessage(const QString& type,
                                      const std::string& sessionId,
                                      const QJsonObject& payload);
    bool DispatchRoomMessage(const QString& type,
                             const std::string& sessionId,
                             const QJsonObject& payload);
    bool DispatchTransportMessage(const QString& type,
                                  const std::string& sessionId,
                                  const QJsonObject& payload);
    SignalingOperationResult SendEnvelope(const QString& type,
                                          const std::string& sessionId,
                                          const QJsonObject& payload)
    {
        if (socket_.state() != QAbstractSocket::ConnectedState) {
            return signaling_client_detail::Failure(
                "signaling_socket_not_connected",
                "The signaling WebSocket is not connected.");
        }
        QJsonObject envelope;
        envelope.insert(
            QStringLiteral("protocolVersion"),
            signaling_client_detail::kProtocolVersion);
        envelope.insert(
            QStringLiteral("messageId"),
            QUuid::createUuid().toString(QUuid::WithoutBraces));
        envelope.insert(QStringLiteral("type"), type);
        if (!sessionId.empty()) {
            envelope.insert(
                QStringLiteral("sessionId"),
                signaling_client_detail::ToQString(sessionId));
        }
        envelope.insert(QStringLiteral("payload"), payload);
        const QString message = QString::fromUtf8(
            QJsonDocument(envelope).toJson(QJsonDocument::Compact));
        if (socket_.sendTextMessage(message) < 0) {
            return signaling_client_detail::Failure(
                "signaling_send_failed",
                "The signaling message could not be queued.");
        }
        return signaling_client_detail::Success();
    }

    SignalingOperationResult RequireRegistered() const
    {
        if (State() == SignalingConnectionState::kRegistered) {
            return signaling_client_detail::Success();
        }
        return signaling_client_detail::Failure(
            "signaling_not_registered",
            "The device is not registered with signaling.");
    }

    bool RememberMessageId(const std::string& messageId)
    {
        if (!receivedMessageIds_.insert(messageId).second) {
            return false;
        }
        receivedMessageOrder_.push_back(messageId);
        if (receivedMessageOrder_.size() >
            signaling_client_detail::kMaximumRememberedMessageIds) {
            receivedMessageIds_.erase(receivedMessageOrder_.front());
            receivedMessageOrder_.pop_front();
        }
        return true;
    }

    void SetState(SignalingConnectionState state)
    {
        if (state_.exchange(state, std::memory_order_acq_rel) == state) {
            return;
        }
        if (observer_) {
            observer_->OnSignalingStateChanged(state);
        }
    }

    void NotifyError(const std::string& code, const std::string& message)
    {
        if (observer_) {
            observer_->OnSignalingError(code, message);
        }
    }

    void Fail(const std::string& code, const std::string& message)
    {
        if (State() == SignalingConnectionState::kFailed) {
            return;
        }
        SetState(SignalingConnectionState::kFailed);
        NotifyError(code, message);
    }

    QWebSocket socket_;
    QTimer heartbeatTimer_;
    QTimer heartbeatDeadline_;
    QTimer authenticationDeadline_;
    QTimer reconnectTimer_;
    QElapsedTimer heartbeatRoundTrip_;
    bool heartbeatOutstanding_ = false;
    bool manualDisconnect_ = true;
    bool fatalDisconnect_ = false;
    std::uint32_t reconnectAttempt_ = 0;
    ISignalingClientObserver* observer_ = nullptr;
    SignalingClientConfig config_;
    QUrl endpoint_;
    std::atomic<SignalingConnectionState> state_{
        SignalingConnectionState::kDisconnected};
    std::unordered_set<std::string> receivedMessageIds_;
    std::deque<std::string> receivedMessageOrder_;
};

}  // namespace remote
