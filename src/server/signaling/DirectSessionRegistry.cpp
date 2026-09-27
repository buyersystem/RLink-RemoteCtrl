// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "DirectSessionRegistry.h"

#include <utility>

namespace remote::signaling_server {

void DirectSessionRegistry::Reserve(qsizetype capacity)
{
    sessions_.reserve(capacity);
    deviceSessions_.reserve(capacity);
}

bool DirectSessionRegistry::HasDeviceSession(const QString& deviceId) const
{
    return deviceSessions_.contains(deviceId);
}

QString DirectSessionRegistry::SessionIdForDevice(
    const QString& deviceId) const
{
    return deviceSessions_.value(deviceId);
}

bool DirectSessionRegistry::Insert(DirectSessionState session)
{
    if (session.sessionId.isEmpty() ||
        session.requesterDeviceId.isEmpty() ||
        session.targetDeviceId.isEmpty() ||
        sessions_.contains(session.sessionId) ||
        HasDeviceSession(session.requesterDeviceId) ||
        HasDeviceSession(session.targetDeviceId)) {
        return false;
    }
    const QString sessionId = session.sessionId;
    const QString requesterDeviceId = session.requesterDeviceId;
    const QString targetDeviceId = session.targetDeviceId;
    sessions_.insert(sessionId, std::move(session));
    deviceSessions_.insert(requesterDeviceId, sessionId);
    deviceSessions_.insert(targetDeviceId, sessionId);
    return true;
}

void DirectSessionRegistry::Remove(const QString& sessionId)
{
    const auto sessionIt = sessions_.find(sessionId);
    if (sessionIt == sessions_.end()) {
        return;
    }
    const QString requesterDeviceId = sessionIt->requesterDeviceId;
    const QString targetDeviceId = sessionIt->targetDeviceId;
    sessions_.erase(sessionIt);
    if (deviceSessions_.value(requesterDeviceId) == sessionId) {
        deviceSessions_.remove(requesterDeviceId);
    }
    if (deviceSessions_.value(targetDeviceId) == sessionId) {
        deviceSessions_.remove(targetDeviceId);
    }
}

void DirectSessionRegistry::Clear()
{
    sessions_.clear();
    deviceSessions_.clear();
}

qsizetype DirectSessionRegistry::Size() const
{
    return sessions_.size();
}

DirectSessionRegistry::SessionMap& DirectSessionRegistry::Sessions()
{
    return sessions_;
}

const DirectSessionRegistry::SessionMap&
DirectSessionRegistry::Sessions() const
{
    return sessions_;
}

bool RunDirectSessionRegistrySelfTest(QString* errorMessage)
{
    DirectSessionRegistry registry;
    registry.Reserve(4);

    DirectSessionState first;
    first.sessionId = QStringLiteral("session-a");
    first.requesterDeviceId = QStringLiteral("device-a");
    first.targetDeviceId = QStringLiteral("device-b");
    if (!registry.Insert(first) || registry.Size() != 1 ||
        registry.SessionIdForDevice(QStringLiteral("device-a")) !=
            QStringLiteral("session-a") ||
        registry.SessionIdForDevice(QStringLiteral("device-b")) !=
            QStringLiteral("session-a")) {
        if (errorMessage) {
            *errorMessage = QStringLiteral(
                "Direct session insertion/indexing failed.");
        }
        return false;
    }

    DirectSessionState duplicateDevice;
    duplicateDevice.sessionId = QStringLiteral("session-b");
    duplicateDevice.requesterDeviceId = QStringLiteral("device-b");
    duplicateDevice.targetDeviceId = QStringLiteral("device-c");
    if (registry.Insert(std::move(duplicateDevice))) {
        if (errorMessage) {
            *errorMessage = QStringLiteral(
                "A busy device was accepted into a second session.");
        }
        return false;
    }

    registry.Remove(QStringLiteral("session-a"));
    if (registry.Size() != 0 ||
        registry.HasDeviceSession(QStringLiteral("device-a")) ||
        registry.HasDeviceSession(QStringLiteral("device-b"))) {
        if (errorMessage) {
            *errorMessage = QStringLiteral(
                "Removing a session did not release device indexes.");
        }
        return false;
    }

    DirectSessionState second;
    second.sessionId = QStringLiteral("session-c");
    second.requesterDeviceId = QStringLiteral("device-b");
    second.targetDeviceId = QStringLiteral("device-c");
    if (!registry.Insert(std::move(second))) {
        if (errorMessage) {
            *errorMessage = QStringLiteral(
                "Released devices could not start a new session.");
        }
        return false;
    }
    registry.Clear();
    if (registry.Size() != 0 ||
        registry.HasDeviceSession(QStringLiteral("device-b")) ||
        registry.HasDeviceSession(QStringLiteral("device-c"))) {
        if (errorMessage) {
            *errorMessage = QStringLiteral(
                "Clearing the registry left stale device indexes.");
        }
        return false;
    }
    return true;
}

}  // namespace remote::signaling_server
