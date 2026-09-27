// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QScrollArea>
#include <QString>

#include "src/core/SessionEngineTypes.h"

class QLabel;
class QFrame;
class QPushButton;
class QVBoxLayout;

namespace remote::controller {

class OwnedDevicesPage final : public QScrollArea {
    Q_OBJECT

public:
    explicit OwnedDevicesPage(QWidget* parent = nullptr);

    void UpdateSnapshot(const OwnedDevicesSnapshot& ownedDevices,
                        SessionConnectivityState connectivity);
    void RefreshThemeStyle();

signals:
    void refreshRequested();
    void connectRequested(const QString& deviceId,
                          const QString& deviceName);

private:
    void PulseRefreshIcon();

    QWidget* content_ = nullptr;
    QVBoxLayout* cardsLayout_ = nullptr;
    QLabel* summaryLabel_ = nullptr;
    QFrame* emptyState_ = nullptr;
    QLabel* emptyArtwork_ = nullptr;
    QPushButton* refreshButton_ = nullptr;
    quint64 renderedRevision_ = 0;
    SessionConnectivityState renderedConnectivity_ =
        SessionConnectivityState::kNotConfigured;
};

}  // namespace remote::controller
