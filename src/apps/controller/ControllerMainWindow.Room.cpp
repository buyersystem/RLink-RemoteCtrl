// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ControllerMainWindow.h"
#include "ControllerMainWindowSupport.h"
#include "pages/RoomPage.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QListWidgetItem>
#include <QMenu>
#include <QSettings>
#include <QTimer>
#include <algorithm>
#include "RoundedPopupMenu.h"
#include "RemoteCDialog.h"
#include "RemoteCToast.h"

namespace remote::controller {
using namespace detail;

void ControllerMainWindow::QueueRoomJoinApproval(
    const SessionEngineSnapshot& snapshot)
{
    const QString roomId = QString::fromStdString(snapshot.room.roomId);
    if (approvalRoomId_ != roomId) {
        approvalRoomId_ = roomId;
        promptedRoomJoinRequestIds_.clear();
        promptedRoomScreenShareSwitchRequestIds_.clear();
        promptedRoomControlRequestIds_.clear();
        promptedRoomScreenShareViewRequestIds_.clear();
        handledRoomMemberActionResultKeys_.clear();
        roomJoinApprovalPromptPending_ = false;
        roomScreenShareSwitchApprovalPromptPending_ = false;
        roomControlApprovalPromptPending_ = false;
        roomScreenShareViewApprovalPromptPending_ = false;
    }
    if (roomJoinApprovalPromptPending_ ||
        roomScreenShareSwitchApprovalPromptPending_ ||
        roomControlApprovalPromptPending_ ||
        roomScreenShareViewApprovalPromptPending_ ||
        snapshot.connectivity != SessionConnectivityState::kOnline ||
        snapshot.room.membership != RoomMembershipState::kActive ||
        snapshot.room.ownerDeviceId != snapshot.localDeviceId) {
        return;
    }

    const auto request = std::find_if(
        snapshot.roomActivity.incomingJoinRequests.begin(),
        snapshot.roomActivity.incomingJoinRequests.end(),
        [this](const RoomJoinRequest& candidate) {
            return !promptedRoomJoinRequestIds_.contains(
                QString::fromStdString(candidate.requestId));
        });
    if (request == snapshot.roomActivity.incomingJoinRequests.end()) {
        return;
    }

    const RoomJoinRequest pendingRequest = *request;
    const QString requestId =
        QString::fromStdString(pendingRequest.requestId);
    promptedRoomJoinRequestIds_.insert(requestId);
    roomJoinApprovalPromptPending_ = true;
    QTimer::singleShot(0, this, [this, pendingRequest, requestId] {
        const auto current = engine_->Snapshot();
        const auto stillPending = std::find_if(
            current.roomActivity.incomingJoinRequests.begin(),
            current.roomActivity.incomingJoinRequests.end(),
            [&pendingRequest](const RoomJoinRequest& candidate) {
                return candidate.requestId == pendingRequest.requestId;
            });
        if (current.connectivity != SessionConnectivityState::kOnline ||
            current.room.membership != RoomMembershipState::kActive ||
            current.room.ownerDeviceId != current.localDeviceId ||
            stillPending == current.roomActivity.incomingJoinRequests.end()) {
            roomJoinApprovalPromptPending_ = false;
            promptedRoomJoinRequestIds_.remove(requestId);
            QueueRoomJoinApproval(current);
            return;
        }

        const QString requesterName =
            pendingRequest.requesterDeviceName.empty()
                ? QString::fromStdString(pendingRequest.requesterDeviceId)
                : QString::fromStdString(pendingRequest.requesterDeviceName);
        const QString requesterId =
            QString::fromStdString(pendingRequest.requesterDeviceId);
        const QString prompt =
            QStringLiteral(
                "设备 %1（%2）申请加入房间 %3。\n\n当前人数：%4 / %5。是否允许加入？")
                .arg(requesterName)
                .arg(requesterId)
                .arg(QString::fromStdString(pendingRequest.roomId))
                .arg(static_cast<qulonglong>(current.room.members.size()))
                .arg(current.room.capacity);
        const bool accepted = RemoteCDialog::Confirm(
            this, QStringLiteral("收到入房申请"), prompt,
            QStringLiteral("允许加入"), QStringLiteral("拒绝"),
            RemoteCDialog::Tone::kQuestion, true);
        const auto result = engine_->RespondToRoomJoin(
            pendingRequest.requestId, accepted);
        roomJoinApprovalPromptPending_ = false;
        if (!result.accepted) {
            promptedRoomJoinRequestIds_.remove(requestId);
            SetRoomActionHint(
                QStringLiteral("处理入房申请失败：%1")
                    .arg(QString::fromStdString(result.errorMessage)),
                true);
        } else {
            SetRoomActionHint(
                accepted ? QStringLiteral("已同意 %1 加入房间。")
                               .arg(requesterName)
                         : QStringLiteral("已拒绝 %1 的入房申请。")
                               .arg(requesterName));
        }
        QueueRoomJoinApproval(engine_->Snapshot());
        QueueRoomScreenShareSwitchApproval(engine_->Snapshot());
        QueueRoomControlApproval(engine_->Snapshot());
        QueueRoomScreenShareViewApproval(engine_->Snapshot());
    });
}

void ControllerMainWindow::QueueRoomScreenShareSwitchApproval(
    const SessionEngineSnapshot& snapshot)
{
    const QString roomId = QString::fromStdString(snapshot.room.roomId);
    if (approvalRoomId_ != roomId) {
        approvalRoomId_ = roomId;
        promptedRoomJoinRequestIds_.clear();
        promptedRoomScreenShareSwitchRequestIds_.clear();
        promptedRoomControlRequestIds_.clear();
        promptedRoomScreenShareViewRequestIds_.clear();
        handledRoomMemberActionResultKeys_.clear();
        roomJoinApprovalPromptPending_ = false;
        roomScreenShareSwitchApprovalPromptPending_ = false;
        roomControlApprovalPromptPending_ = false;
        roomScreenShareViewApprovalPromptPending_ = false;
    }
    if (roomScreenShareSwitchApprovalPromptPending_ ||
        roomJoinApprovalPromptPending_ ||
        roomControlApprovalPromptPending_ ||
        roomScreenShareViewApprovalPromptPending_ ||
        snapshot.connectivity != SessionConnectivityState::kOnline ||
        snapshot.room.membership != RoomMembershipState::kActive ||
        snapshot.room.screenShareState !=
            RoomScreenShareState::kActive ||
        snapshot.room.screenSharerDeviceId != snapshot.localDeviceId) {
        return;
    }

    const auto request = std::find_if(
        snapshot.roomActivity.incomingScreenShareSwitchRequests.begin(),
        snapshot.roomActivity.incomingScreenShareSwitchRequests.end(),
        [this](const RoomScreenShareSwitchRequest& candidate) {
            return !promptedRoomScreenShareSwitchRequestIds_.contains(
                QString::fromStdString(candidate.requestId));
        });
    if (request ==
        snapshot.roomActivity.incomingScreenShareSwitchRequests.end()) {
        return;
    }

    const RoomScreenShareSwitchRequest pendingRequest = *request;
    const QString requestId =
        QString::fromStdString(pendingRequest.requestId);
    promptedRoomScreenShareSwitchRequestIds_.insert(requestId);
    roomScreenShareSwitchApprovalPromptPending_ = true;
    QTimer::singleShot(0, this, [this, pendingRequest, requestId] {
        const auto current = engine_->Snapshot();
        const auto stillPending = std::find_if(
            current.roomActivity.incomingScreenShareSwitchRequests.begin(),
            current.roomActivity.incomingScreenShareSwitchRequests.end(),
            [&pendingRequest](
                const RoomScreenShareSwitchRequest& candidate) {
                return candidate.requestId ==
                       pendingRequest.requestId;
            });
        if (current.connectivity !=
                SessionConnectivityState::kOnline ||
            current.room.membership !=
                RoomMembershipState::kActive ||
            current.room.screenShareState !=
                RoomScreenShareState::kActive ||
            current.room.screenSharerDeviceId !=
                current.localDeviceId ||
            stillPending ==
                current.roomActivity.incomingScreenShareSwitchRequests.end()) {
            roomScreenShareSwitchApprovalPromptPending_ = false;
            promptedRoomScreenShareSwitchRequestIds_.remove(requestId);
            QueueRoomScreenShareSwitchApproval(current);
            return;
        }

        const QString requesterName =
            pendingRequest.requesterDeviceName.empty()
                ? QString::fromStdString(
                      pendingRequest.requesterDeviceId)
                : QString::fromStdString(
                      pendingRequest.requesterDeviceName);
        const QString requesterId =
            QString::fromStdString(pendingRequest.requesterDeviceId);
        const QString prompt = QStringLiteral(
            "设备 %1（%2）申请接替你成为主机器。\n\n同意后，本机屏幕共享会停止，现有远程控制授权会立即撤销，并切换到对方画面；拒绝则保持当前状态。")
                                   .arg(requesterName, requesterId);
        const bool accepted = RemoteCDialog::Confirm(
            this, QStringLiteral("收到主机器接替申请"), prompt,
            QStringLiteral("同意接替"), QStringLiteral("保持当前共享"),
            RemoteCDialog::Tone::kWarning, true);
        const auto result =
            engine_->RespondToRoomScreenShareSwitch(
                pendingRequest.requestId, accepted);
        roomScreenShareSwitchApprovalPromptPending_ = false;
        if (!result.accepted) {
            promptedRoomScreenShareSwitchRequestIds_.remove(requestId);
            SetRoomActionHint(
                QStringLiteral("处理主机器接替申请失败：%1")
                    .arg(QString::fromStdString(result.errorMessage)),
                true);
        } else {
            SetRoomActionHint(
                accepted
                    ? QStringLiteral(
                          "已同意 %1 接替，正在安全切换主机器。")
                          .arg(requesterName)
                    : QStringLiteral(
                          "已拒绝 %1 的主机器接替申请。")
                          .arg(requesterName));
        }
        QueueRoomScreenShareSwitchApproval(engine_->Snapshot());
        QueueRoomControlApproval(engine_->Snapshot());
        QueueRoomJoinApproval(engine_->Snapshot());
        QueueRoomScreenShareViewApproval(engine_->Snapshot());
    });
}

void ControllerMainWindow::QueueRoomControlApproval(
    const SessionEngineSnapshot& snapshot)
{
    const QString roomId = QString::fromStdString(snapshot.room.roomId);
    if (approvalRoomId_ != roomId) {
        approvalRoomId_ = roomId;
        promptedRoomJoinRequestIds_.clear();
        promptedRoomScreenShareSwitchRequestIds_.clear();
        promptedRoomControlRequestIds_.clear();
        promptedRoomScreenShareViewRequestIds_.clear();
        handledRoomMemberActionResultKeys_.clear();
        roomJoinApprovalPromptPending_ = false;
        roomScreenShareSwitchApprovalPromptPending_ = false;
        roomControlApprovalPromptPending_ = false;
        roomScreenShareViewApprovalPromptPending_ = false;
    }
    if (roomControlApprovalPromptPending_ ||
        roomJoinApprovalPromptPending_ ||
        roomScreenShareSwitchApprovalPromptPending_ ||
        roomScreenShareViewApprovalPromptPending_ ||
        snapshot.connectivity != SessionConnectivityState::kOnline ||
        snapshot.room.membership != RoomMembershipState::kActive ||
        snapshot.room.screenSharerDeviceId != snapshot.localDeviceId) {
        return;
    }

    const auto request = std::find_if(
        snapshot.roomActivity.incomingControlRequests.begin(),
        snapshot.roomActivity.incomingControlRequests.end(),
        [this](const RoomControlRequest& candidate) {
            return !promptedRoomControlRequestIds_.contains(
                QString::fromStdString(candidate.requestId));
        });
    if (request == snapshot.roomActivity.incomingControlRequests.end()) {
        return;
    }

    const RoomControlRequest pendingRequest = *request;
    const QString requestId =
        QString::fromStdString(pendingRequest.requestId);
    promptedRoomControlRequestIds_.insert(requestId);
    roomControlApprovalPromptPending_ = true;
    QTimer::singleShot(0, this, [this, pendingRequest, requestId] {
        const auto current = engine_->Snapshot();
        const auto stillPending = std::find_if(
            current.roomActivity.incomingControlRequests.begin(),
            current.roomActivity.incomingControlRequests.end(),
            [&pendingRequest](const RoomControlRequest& candidate) {
                return candidate.requestId == pendingRequest.requestId;
            });
        if (current.connectivity != SessionConnectivityState::kOnline ||
            current.room.membership != RoomMembershipState::kActive ||
            current.room.screenSharerDeviceId != current.localDeviceId ||
            stillPending == current.roomActivity.incomingControlRequests.end()) {
            roomControlApprovalPromptPending_ = false;
            promptedRoomControlRequestIds_.remove(requestId);
            QueueRoomControlApproval(current);
            return;
        }

        const QString requesterName =
            pendingRequest.requesterDeviceName.empty()
                ? QString::fromStdString(pendingRequest.requesterDeviceId)
                : QString::fromStdString(pendingRequest.requesterDeviceName);
        const QString requesterId =
            QString::fromStdString(pendingRequest.requesterDeviceId);
        const QString prompt = QStringLiteral(
            "设备 %1（%2）申请控制你正在分享的屏幕。\n\n同意后，房间中其他成员仍可观看，但只有该设备能够发送输入。")
            .arg(requesterName, requesterId);
        bool allowClipboard = QSettings().value(
            QString::fromLatin1(kRemotePasteEnabledSetting), true).toBool();
        const bool accepted = RemoteCDialog::ConfirmWithOption(
            this, QStringLiteral("收到控制申请"), prompt,
            QStringLiteral("允许控制"), QStringLiteral("拒绝"),
            QStringLiteral("允许本次控制会话接收远程粘贴"),
            allowClipboard, &allowClipboard,
            RemoteCDialog::Tone::kQuestion, true);
        clipboardAllowedForCurrentControl_ = accepted && allowClipboard;
        const auto result = engine_->RespondToRoomControl(
            pendingRequest.requestId, accepted);
        roomControlApprovalPromptPending_ = false;
        if (!result.accepted) {
            promptedRoomControlRequestIds_.remove(requestId);
            SetRoomActionHint(
                QStringLiteral("处理控制申请失败：%1")
                    .arg(QString::fromStdString(result.errorMessage)),
                true);
        } else {
            SetRoomActionHint(
                accepted ? QStringLiteral("已允许 %1 控制本机屏幕。")
                               .arg(requesterName)
                         : QStringLiteral("已拒绝 %1 的控制申请。")
                               .arg(requesterName));
        }
        QueueRoomControlApproval(engine_->Snapshot());
        QueueRoomScreenShareSwitchApproval(engine_->Snapshot());
        QueueRoomJoinApproval(engine_->Snapshot());
        QueueRoomScreenShareViewApproval(engine_->Snapshot());
    });
}

void ControllerMainWindow::QueueRoomScreenShareViewApproval(
    const SessionEngineSnapshot& snapshot)
{
    const QString roomId = QString::fromStdString(snapshot.room.roomId);
    if (approvalRoomId_ != roomId) {
        approvalRoomId_ = roomId;
        promptedRoomJoinRequestIds_.clear();
        promptedRoomScreenShareSwitchRequestIds_.clear();
        promptedRoomControlRequestIds_.clear();
        promptedRoomScreenShareViewRequestIds_.clear();
        handledRoomMemberActionResultKeys_.clear();
        roomJoinApprovalPromptPending_ = false;
        roomScreenShareSwitchApprovalPromptPending_ = false;
        roomControlApprovalPromptPending_ = false;
        roomScreenShareViewApprovalPromptPending_ = false;
    }
    if (roomScreenShareViewApprovalPromptPending_ ||
        roomJoinApprovalPromptPending_ ||
        roomScreenShareSwitchApprovalPromptPending_ ||
        roomControlApprovalPromptPending_ ||
        snapshot.connectivity != SessionConnectivityState::kOnline ||
        snapshot.room.membership != RoomMembershipState::kActive) {
        return;
    }

    const auto request = std::find_if(
        snapshot.roomActivity.incomingScreenShareViewRequests.begin(),
        snapshot.roomActivity.incomingScreenShareViewRequests.end(),
        [this](const RoomScreenShareViewRequest& candidate) {
            const QString key = QStringLiteral("%1:%2")
                .arg(QString::fromStdString(
                         candidate.requesterDeviceId))
                .arg(static_cast<qulonglong>(candidate.sequence));
            return !promptedRoomScreenShareViewRequestIds_.contains(key);
        });
    if (request ==
        snapshot.roomActivity.incomingScreenShareViewRequests.end()) {
        return;
    }

    const RoomScreenShareViewRequest pendingRequest = *request;
    const QString requestKey = QStringLiteral("%1:%2")
        .arg(QString::fromStdString(
                 pendingRequest.requesterDeviceId))
        .arg(static_cast<qulonglong>(pendingRequest.sequence));
    promptedRoomScreenShareViewRequestIds_.insert(requestKey);
    roomScreenShareViewApprovalPromptPending_ = true;
    QTimer::singleShot(
        0, this, [this, pendingRequest, requestKey] {
            const auto current = engine_->Snapshot();
            const auto stillPending = std::find_if(
                current.roomActivity.incomingScreenShareViewRequests.begin(),
                current.roomActivity.incomingScreenShareViewRequests.end(),
                [&pendingRequest](
                    const RoomScreenShareViewRequest& candidate) {
                    return candidate.sequence ==
                               pendingRequest.sequence &&
                           candidate.requesterDeviceId ==
                               pendingRequest.requesterDeviceId;
                });
            if (current.connectivity !=
                    SessionConnectivityState::kOnline ||
                current.room.membership !=
                    RoomMembershipState::kActive ||
                stillPending ==
                    current.roomActivity.incomingScreenShareViewRequests.end()) {
                roomScreenShareViewApprovalPromptPending_ = false;
                promptedRoomScreenShareViewRequestIds_.remove(
                    requestKey);
                QueueRoomScreenShareViewApproval(current);
                return;
            }

            const QString requesterName =
                pendingRequest.requesterDeviceName.empty()
                    ? QString::fromStdString(
                          pendingRequest.requesterDeviceId)
                    : QString::fromStdString(
                          pendingRequest.requesterDeviceName);
            const QString prompt = QStringLiteral(
                "%1 请求观看你的屏幕。\n\n允许后，本机会立即开始屏幕共享；如果房间中已有主机器，将进入现有的主机器切换确认流程。")
                .arg(requesterName);
            const bool accepted = RemoteCDialog::Confirm(
                this, QStringLiteral("收到屏幕观看申请"), prompt,
                QStringLiteral("允许共享"), QStringLiteral("拒绝"),
                RemoteCDialog::Tone::kQuestion, true);
            const auto result =
                engine_->RespondToRoomMemberScreenShare(
                    pendingRequest.requesterDeviceId,
                    pendingRequest.sequence, accepted);
            roomScreenShareViewApprovalPromptPending_ = false;
            if (!result.accepted) {
                promptedRoomScreenShareViewRequestIds_.remove(
                    requestKey);
                SetRoomActionHint(
                    QStringLiteral("处理屏幕观看申请失败：%1")
                        .arg(QString::fromStdString(
                            result.errorMessage)),
                    true);
            } else {
                SetRoomActionHint(
                    accepted
                        ? QStringLiteral(
                              "已允许 %1 观看本机屏幕，正在建立共享。")
                              .arg(requesterName)
                        : QStringLiteral(
                              "已拒绝 %1 的屏幕观看申请。")
                              .arg(requesterName));
            }
            const auto latest = engine_->Snapshot();
            QueueRoomScreenShareViewApproval(latest);
            QueueRoomJoinApproval(latest);
            QueueRoomScreenShareSwitchApproval(latest);
            QueueRoomControlApproval(latest);
        });
}
void ControllerMainWindow::HandleRoomMemberActionResults(
    const SessionEngineSnapshot& snapshot)
{
    for (const auto& result : snapshot.roomActivity.memberActionResults) {
        const QString key = QStringLiteral("%1:%2:%3:%4")
            .arg(QString::fromStdString(result.roomId),
                 QString::fromStdString(result.peerDeviceId))
            .arg(static_cast<qulonglong>(result.sequence))
            .arg(static_cast<int>(result.action));
        if (handledRoomMemberActionResultKeys_.contains(key)) {
            continue;
        }
        handledRoomMemberActionResultKeys_.insert(key);
        const QString peerName = MemberDisplayName(
            snapshot.room, result.peerDeviceId,
            snapshot.localDeviceId);
        if (result.accepted) {
            QString message;
            switch (result.action) {
            case RoomMemberAction::kRequestScreenShare:
                message = QStringLiteral(
                    "%1 已允许观看，正在建立屏幕共享。")
                    .arg(peerName);
                break;
            case RoomMemberAction::kStopScreenShare:
                message = QStringLiteral(
                    "%1 已停止屏幕共享。").arg(peerName);
                break;
            case RoomMemberAction::kDisableMicrophone:
                message = QStringLiteral(
                    "%1 的麦克风已关闭。").arg(peerName);
                break;
            }
            RemoteCToast::Show(
                this, message, RemoteCToast::Tone::kSuccess);
            continue;
        }

        QString title;
        QString message;
        switch (result.action) {
        case RoomMemberAction::kRequestScreenShare:
            title = QStringLiteral("屏幕观看申请被拒绝");
            message = QStringLiteral(
                "%1 拒绝了你的屏幕观看申请。").arg(peerName);
            break;
        case RoomMemberAction::kStopScreenShare:
            title = QStringLiteral("未能停止对方共享");
            message = QStringLiteral(
                "%1 未能停止屏幕共享，请检查当前控制权和网络状态。")
                .arg(peerName);
            break;
        case RoomMemberAction::kDisableMicrophone:
            title = QStringLiteral("未能关闭对方麦克风");
            message = QStringLiteral(
                "无法关闭 %1 的麦克风，请检查房主权限和 P2P 状态。")
                .arg(peerName);
            break;
        }
        QTimer::singleShot(0, this, [this, title, message] {
            RemoteCDialog::Alert(
                this, title, message, QStringLiteral("知道了"),
                RemoteCDialog::Tone::kDanger, true);
        });
    }
}

void ControllerMainWindow::ShowRoomMemberContextMenu(
    const QPoint& position)
{
    if (!engine_ || !RoomControls().memberList) {
        return;
    }
    QListWidgetItem* item = RoomControls().memberList->itemAt(position);
    if (!item) {
        return;
    }
    const QString peerDeviceId =
        item->data(Qt::UserRole).toString();
    const QString peerName =
        item->data(Qt::UserRole + 1).toString();
    const auto snapshot = engine_->Snapshot();
    if (peerDeviceId.isEmpty() ||
        peerDeviceId.toStdString() == snapshot.localDeviceId ||
        snapshot.room.membership != RoomMembershipState::kActive) {
        return;
    }
    const auto member = std::find_if(
        snapshot.room.members.begin(), snapshot.room.members.end(),
        [&peerDeviceId](const RoomMemberSnapshot& current) {
            return current.deviceId == peerDeviceId.toStdString();
        });
    if (member == snapshot.room.members.end() || !member->online) {
        return;
    }
    const auto pair = std::find_if(
        snapshot.roomActivity.peerConnections.begin(),
        snapshot.roomActivity.peerConnections.end(),
        [&peerDeviceId](const RoomPeerConnectionSnapshot& current) {
            return current.peerDeviceId ==
                       peerDeviceId.toStdString() &&
                   current.state ==
                       RoomPeerConnectionState::kActive;
        });
    const bool pairActive =
        pair != snapshot.roomActivity.peerConnections.end();
    const bool peerSharing =
        snapshot.room.screenSharerDeviceId ==
        peerDeviceId.toStdString();
    const bool localOwner =
        snapshot.room.ownerDeviceId == snapshot.localDeviceId;
    const bool localController =
        snapshot.room.activeControllerDeviceId ==
        snapshot.localDeviceId;

    RoundedPopupMenu menu(this);
    menu.setObjectName(QStringLiteral("roomMemberActionMenu"));
    menu.setStyleSheet(QStringLiteral(R"(
QMenu#roomMemberActionMenu {
    background:#ffffff;
    border:1px solid #dce2eb;
    border-radius:12px;
    padding:7px;
    color:#263248;
}
QMenu#roomMemberActionMenu::item {
    min-width:220px;
    min-height:34px;
    padding:4px 14px;
    margin:1px 0;
    border-radius:8px;
}
QMenu#roomMemberActionMenu::item:selected {
    background:#e7f0ff;
    color:#0868e8;
}
QMenu#roomMemberActionMenu::item:disabled {
    color:#8995a8;
}
QMenu#roomMemberActionMenu::separator {
    height:1px;
    background:#e7ebf1;
    margin:6px 8px;
}
)"));
    if (darkInterfaceTheme_) {
        menu.setStyleSheet(menu.styleSheet() + QStringLiteral(R"(
QMenu#roomMemberActionMenu {
    background:#151F2E; border-color:#34445B; color:#E4EBF5;
}
QMenu#roomMemberActionMenu::item:selected {
    background:#263A61; color:#9BB4FF;
}
QMenu#roomMemberActionMenu::item:disabled { color:#69788E; }
QMenu#roomMemberActionMenu::separator { background:#2A394E; }
)"));
    }
    QAction* title = menu.addAction(
        peerName.isEmpty() ? peerDeviceId : peerName);
    title->setEnabled(false);
    menu.addSeparator();

    QAction* screenAction = menu.addAction(
        peerSharing ? QStringLiteral("打开共享画面")
                    : QStringLiteral("申请观看对方屏幕"));
    screenAction->setEnabled(pairActive);
    QAction* controlAction = nullptr;
    QAction* stopShareAction = nullptr;
    if (peerSharing && !localController &&
        snapshot.room.activeControllerDeviceId.empty()) {
        controlAction = menu.addAction(
            QStringLiteral("申请远程控制"));
        controlAction->setEnabled(pairActive);
    }
    if (peerSharing && (localOwner || localController)) {
        stopShareAction = menu.addAction(
            QStringLiteral("停止对方屏幕共享"));
        stopShareAction->setEnabled(pairActive);
    }
    QAction* muteAction = nullptr;
    if (localOwner && member->microphonePublishing) {
        muteAction = menu.addAction(
            QStringLiteral("关闭对方麦克风"));
        muteAction->setEnabled(pairActive);
    }
    menu.addSeparator();
    QAction* copyAction = menu.addAction(
        QStringLiteral("复制设备 ID"));

    QAction* selected = menu.exec(
        RoomControls().memberList->viewport()->mapToGlobal(position));
    if (!selected) {
        return;
    }
    SessionCommandResult result;
    QString successText;
    if (selected == screenAction) {
        if (peerSharing) {
            dismissedRemoteScreenSharerDeviceId_.clear();
            dismissedRemoteScreenShareEpoch_ = 0;
            OpenRemoteSession(RemoteSessionBinding::Room(
                peerDeviceId, peerName,
                QString::fromStdString(pair->pairId)));
            return;
        }
        result = engine_->RequestRoomMemberScreenShare(
            peerDeviceId.toStdString());
        successText = QStringLiteral("屏幕观看申请已发送");
    } else if (selected == controlAction) {
        result = engine_->RequestRoomControl();
        successText = QStringLiteral("远程控制申请已发送");
    } else if (selected == stopShareAction) {
        dismissedRemoteScreenSharerDeviceId_ = peerDeviceId;
        dismissedRemoteScreenShareEpoch_ =
            snapshot.room.screenShareEpoch;
        result = engine_->RequestRemoteRoomScreenShareStop(
            peerDeviceId.toStdString(),
            snapshot.room.screenShareEpoch);
        successText = QStringLiteral("已通知对方停止屏幕共享");
    } else if (selected == muteAction) {
        result = engine_->RequestRoomMemberMicrophoneMute(
            peerDeviceId.toStdString());
        successText = QStringLiteral("关闭对方麦克风的指令已发送");
    } else if (selected == copyAction) {
        QApplication::clipboard()->setText(peerDeviceId);
        RemoteCToast::ShowAbove(
            RoomControls().memberList, QStringLiteral("设备 ID 已复制"),
            RemoteCToast::Tone::kSuccess);
        return;
    } else {
        return;
    }
    if (!result.accepted) {
        RemoteCToast::ShowAbove(
            RoomControls().memberList,
            QString::fromStdString(result.errorMessage),
            RemoteCToast::Tone::kError);
        return;
    }
    RemoteCToast::ShowAbove(
        RoomControls().memberList, successText,
        RemoteCToast::Tone::kSuccess);
}
}  // namespace remote::controller
