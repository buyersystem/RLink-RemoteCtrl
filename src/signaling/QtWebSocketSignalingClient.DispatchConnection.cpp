// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "QtWebSocketSignalingClient.Internal.h"

namespace remote {
using namespace signaling_client_detail;

bool QtWebSocketSignalingClient::Impl::DispatchConnectionMessage(
    const QString& type, const std::string& sessionId,
    const QJsonObject& payload)
{
    if (type == QStringLiteral("auth_success")) {
        if (State() != SignalingConnectionState::kAuthenticating) {
            fatalDisconnect_ = true;
            Fail("unexpected_auth_success",
                 "Authentication succeeded outside the authentication state.");
            socket_.close(
                QWebSocketProtocol::CloseCodeProtocolError,
                QStringLiteral("unexpected auth_success"));
            return true;
        }
        BeginDeviceRegistration();
        return true;
    }

    if (type == QStringLiteral("auth_error")) {
        const SignalingConnectionState state = State();
        if (state != SignalingConnectionState::kAuthenticating &&
            state != SignalingConnectionState::kRegistering &&
            state != SignalingConnectionState::kRegistered) {
            fatalDisconnect_ = true;
            Fail("unexpected_auth_error",
                 "Authentication failed outside the authentication state.");
            socket_.close(
                QWebSocketProtocol::CloseCodeProtocolError,
                QStringLiteral("unexpected auth_error"));
            return true;
        }
        authenticationDeadline_.stop();
        QString reason = payload.value(
            QStringLiteral("reason")).toString().trimmed();
        if (reason.isEmpty()) {
            reason = QStringLiteral("authentication_rejected");
        }
        QString message = payload.value(
            QStringLiteral("message")).toString().trimmed();
        if (message.isEmpty()) {
            message = QStringLiteral(
                "The signaling server rejected authentication.");
        }
        const bool retryable = payload.value(
            QStringLiteral("retryable")).toBool(false);
        fatalDisconnect_ = !retryable;
        Fail(ToString(reason), ToString(message));
        socket_.close(
            retryable
                ? QWebSocketProtocol::CloseCodeNormal
                : QWebSocketProtocol::CloseCodePolicyViolated,
            retryable
                ? QStringLiteral("authentication retry")
                : QStringLiteral("authentication rejected"));
        return true;
    }

    if (State() == SignalingConnectionState::kAuthenticating) {
        fatalDisconnect_ = true;
        Fail("authentication_protocol_violation",
             "Only auth_success or auth_error is allowed while authentication is pending.");
        socket_.close(
            QWebSocketProtocol::CloseCodeProtocolError,
            QStringLiteral("authentication protocol violation"));
        return true;
    }

    if (type == QStringLiteral("device_registered")) {
        if (State() != SignalingConnectionState::kRegistering) {
            fatalDisconnect_ = true;
            Fail("unexpected_device_registered",
                 "Device registration completed in an invalid state.");
            socket_.close(
                QWebSocketProtocol::CloseCodeProtocolError,
                QStringLiteral("unexpected device_registered"));
            return true;
        }
        const std::string deviceId = ToString(
            payload.value(QStringLiteral("deviceId")).toString());
        if (deviceId.empty()) {
            NotifyError("invalid_device_registered",
                        "Registered device ID is missing.");
            return true;
        }
        SetState(SignalingConnectionState::kRegistered);
        reconnectAttempt_ = 0;
        if (observer_) {
            observer_->OnDeviceRegistered(deviceId);
        }
        heartbeatTimer_.start(
            static_cast<int>(config_.heartbeatIntervalMs));
        return true;
    }

    if (type == QStringLiteral("account_delete_result")) {
        const QJsonValue deletedValue =
            payload.value(QStringLiteral("deleted"));
        if (!deletedValue.isBool()) {
            NotifyError("invalid_account_delete_result",
                        "Account deletion result is invalid.");
            return true;
        }
        SignalingAccountDeletionResult result;
        result.deleted = deletedValue.toBool();
        result.errorCode = ToString(
            payload.value(QStringLiteral("code")).toString());
        result.errorMessage = ToString(
            payload.value(QStringLiteral("message")).toString());
        result.retryable =
            payload.value(QStringLiteral("retryable")).toBool(false);
        if (observer_) {
            observer_->OnAccountDeletionResult(result);
        }
        return true;
    }

    if (type == QStringLiteral("account_deleted")) {
        SignalingAccountDeletionResult result;
        result.deleted = true;
        result.errorCode = "account_deleted";
        result.errorMessage =
            "The RemoteC account was deleted.";
        if (observer_) {
            observer_->OnAccountDeletionResult(result);
        }
        return true;
    }

    if (type == QStringLiteral("my_devices_result") ||
        type == QStringLiteral("my_devices_changed")) {
        const QString errorCode = payload.value(
            QStringLiteral("errorCode")).toString();
        if (!errorCode.isEmpty()) {
            NotifyError(ToString(errorCode), ToString(payload.value(
                QStringLiteral("errorMessage")).toString()));
            return true;
        }
        const double revisionValue = payload.value(
            QStringLiteral("revision")).toDouble(-1.0);
        const QJsonValue devicesValue = payload.value(
            QStringLiteral("devices"));
        if (revisionValue < 1.0 ||
            revisionValue > 9007199254740991.0 ||
            revisionValue != static_cast<double>(
                static_cast<std::uint64_t>(revisionValue)) ||
            !devicesValue.isArray() ||
            devicesValue.toArray().size() > 100) {
            NotifyError("invalid_my_devices_snapshot",
                        "The owned-device snapshot is invalid.");
            return true;
        }
        SignalingOwnedDevicesSnapshot snapshot;
        snapshot.revision = static_cast<std::uint64_t>(revisionValue);
        const QJsonArray devices = devicesValue.toArray();
        snapshot.devices.reserve(static_cast<std::size_t>(devices.size()));
        std::unordered_set<std::string> deviceIds;
        for (const QJsonValue& value : devices) {
            if (!value.isObject()) {
                NotifyError("invalid_my_devices_snapshot",
                            "An owned-device entry is invalid.");
                return true;
            }
            const QJsonObject object = value.toObject();
            SignalingOwnedDevice device;
            device.deviceId = ToString(object.value(
                QStringLiteral("deviceId")).toString());
            device.deviceName = ToString(object.value(
                QStringLiteral("deviceName")).toString());
            const double createdAt = object.value(
                QStringLiteral("createdAt")).toDouble(-1.0);
            const double lastSeenAt = object.value(
                QStringLiteral("lastSeenAt")).toDouble(-1.0);
            if (!IsNineDigitPublicId(device.deviceId) ||
                device.deviceName.size() > 128 ||
                !object.value(QStringLiteral("online")).isBool() ||
                !object.value(QStringLiteral("current")).isBool() ||
                createdAt < 0.0 || lastSeenAt < 0.0 ||
                !deviceIds.insert(device.deviceId).second) {
                NotifyError("invalid_my_devices_snapshot",
                            "An owned-device entry is invalid.");
                return true;
            }
            device.online = object.value(
                QStringLiteral("online")).toBool();
            device.current = object.value(
                QStringLiteral("current")).toBool();
            device.createdAt = static_cast<std::int64_t>(createdAt);
            device.lastSeenAt = static_cast<std::int64_t>(lastSeenAt);
            snapshot.devices.push_back(std::move(device));
        }
        if (observer_) {
            observer_->OnOwnedDevicesChanged(snapshot);
        }
        return true;
    }

    return false;
}

}  // namespace remote
