// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QPoint>
#include <QScrollArea>
#include <QString>

#include "src/core/MediaDevice.h"

class QComboBox;
class QFrame;
class QLabel;
class QLineEdit;
class QListWidget;
class QParallelAnimationGroup;
class QPushButton;
class QStackedWidget;
class QVBoxLayout;

namespace remote::controller {

struct RoomPageControls {
    QLabel* connectivityLabel = nullptr;
    QLabel* actionHint = nullptr;
    QFrame* entryPanel = nullptr;
    QFrame* roomPanel = nullptr;
    QStackedWidget* workspaceStack = nullptr;
    QParallelAnimationGroup* workspaceAnimation = nullptr;
    QWidget* workspaceTransitionLayer = nullptr;
    QLabel* roomIdLabel = nullptr;
    QPushButton* copyRoomIdButton = nullptr;
    QLabel* occupancyLabel = nullptr;
    QLabel* ownerLabel = nullptr;
    QLabel* screenSharerLabel = nullptr;
    QLabel* controllerLabel = nullptr;
    QLabel* peerConnectivityLabel = nullptr;
    QLabel* seatUsageLabel = nullptr;
    QLabel* memberSummaryLabel = nullptr;
    QLabel* memberFooterLabel = nullptr;
    QListWidget* memberList = nullptr;
    QComboBox* activeRoomCapacity = nullptr;
    QPushButton* applyRoomCapacityButton = nullptr;
    QPushButton* screenShareButton = nullptr;
    QPushButton* cameraButton = nullptr;
    QPushButton* microphoneButton = nullptr;
    QPushButton* speakerButton = nullptr;
    QPushButton* cameraGalleryButton = nullptr;
    QPushButton* fileTransferButton = nullptr;
    QPushButton* leaveButton = nullptr;
    QLabel* stageLabel = nullptr;
};

class RoomPage final : public QScrollArea {
    Q_OBJECT

public:
    explicit RoomPage(QWidget* parent = nullptr);

    RoomPageControls& Controls();
    const RoomPageControls& Controls() const;
    void SetEntryEnabled(bool enabled);
    void SetEntryActionsEnabled(bool enabled);
    void SetEntryActionTexts(const QString& createText,
                             const QString& joinText);
    void SetCreateActionText(const QString& text);
    void SetJoinActionText(const QString& text);
    void SetCapacity(uint capacity);
    void SetRoomId(const QString& roomId);
    void FocusRoomId();

signals:
    void CreateRequested(uint capacity);
    void JoinRequested(const QString& roomId);
    void MemberContextMenuRequested(const QPoint& position);
    void MediaDeviceMenuRequested(MediaDeviceKind kind, QWidget* anchor);

private:
    void BuildHeaderAndEntry();
    void BuildActiveRoom();
    void SetEntryControls(QComboBox* capacity,
                          QPushButton* createButton,
                          QLineEdit* roomId,
                          QPushButton* joinButton);

    QWidget* content_ = nullptr;
    QVBoxLayout* contentLayout_ = nullptr;
    RoomPageControls controls_;
    QComboBox* capacity_ = nullptr;
    QPushButton* createButton_ = nullptr;
    QLineEdit* roomId_ = nullptr;
    QPushButton* joinButton_ = nullptr;
};

}  // namespace remote::controller
