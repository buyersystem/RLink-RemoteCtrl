// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QScrollArea>
#include <QString>
#include <QVector>

class QFrame;
class QVBoxLayout;

namespace remote::controller {

struct RecentRoomCardData {
    QString roomId;
    QString detail;
    QString availabilityText;
    QString availabilityTone;
    QString actionText;
    bool canJoin = false;
};

struct RecentDeviceCardData {
    QString deviceId;
    QString deviceName;
    QString detail;
    QString actionText;
    bool ownedDevice = false;
    bool ownedDeviceOnline = false;
    bool actionEnabled = false;
};

class RecentConnectionsPage final : public QScrollArea {
    Q_OBJECT

public:
    explicit RecentConnectionsPage(QWidget* parent = nullptr);

    void SetRooms(const QVector<RecentRoomCardData>& rooms);
    void SetDevices(const QVector<RecentDeviceCardData>& devices);

signals:
    void joinRoomRequested(const QString& roomId);
    void connectOwnedDeviceRequested(const QString& deviceId,
                                     const QString& deviceName);
    void reconnectAssistedDeviceRequested(const QString& deviceId);

private:
    QWidget* content_ = nullptr;
    QVBoxLayout* roomsLayout_ = nullptr;
    QVBoxLayout* devicesLayout_ = nullptr;
    QFrame* roomsEmptyState_ = nullptr;
    QFrame* devicesEmptyState_ = nullptr;
};

}  // namespace remote::controller
