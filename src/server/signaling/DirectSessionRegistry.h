// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QHash>
#include <QJsonArray>
#include <QString>

namespace remote::signaling_server {

struct DirectSessionState {
    enum class Phase {
        kPending,
        kActive,
    };

    QString sessionId;
    QString requesterDeviceId;
    QString targetDeviceId;
    QString purpose;
    QJsonArray permissions;
    Phase phase = Phase::kPending;
    QString requesterRecoveryToken;
    QString targetRecoveryToken;
    qint64 requesterDisconnectedAtMs = 0;
    qint64 targetDisconnectedAtMs = 0;
    qint64 pendingExpiresAtMs = 0;
};

class DirectSessionRegistry final {
public:
    using SessionMap = QHash<QString, DirectSessionState>;

    void Reserve(qsizetype capacity);
    bool HasDeviceSession(const QString& deviceId) const;
    QString SessionIdForDevice(const QString& deviceId) const;
    bool Insert(DirectSessionState session);
    void Remove(const QString& sessionId);
    void Clear();
    qsizetype Size() const;

    SessionMap& Sessions();
    const SessionMap& Sessions() const;

private:
    SessionMap sessions_;
    QHash<QString, QString> deviceSessions_;
};

bool RunDirectSessionRegistrySelfTest(QString* errorMessage);

}  // namespace remote::signaling_server
