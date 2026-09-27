// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RoomPage.h"

#include <QAbstractItemView>
#include <QColor>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSizePolicy>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

#include "src/apps/controller/ControllerMainWindowSupport.h"
#include "src/apps/controller/CurrentPageStack.h"
#include "src/apps/controller/FramelessWindow.h"
#include "src/apps/controller/MediaControls.h"
#include "src/apps/controller/RemoteCComboBox.h"
#include "src/apps/controller/RoomStatusIndicator.h"
#include "src/apps/controller/ui/RemoteCTheme.h"
#include "src/apps/controller/ui/morph/MorphIconButtonBinding.h"

namespace remote::controller {

using namespace detail;

void RoomPage::BuildActiveRoom()
{
    controls_.roomPanel = new QFrame(content_);
    controls_.roomPanel->setObjectName(QStringLiteral("activeRoomSurface"));
    auto* activeRoomShell = new QHBoxLayout(controls_.roomPanel);
    activeRoomShell->setContentsMargins(0, 0, 0, 0);
    activeRoomShell->setSpacing(16);

    auto* activeRoomMainCard = new QFrame(controls_.roomPanel);
    activeRoomMainCard->setProperty("card", true);
    auto* roomLayout = new QVBoxLayout(activeRoomMainCard);
    roomLayout->setContentsMargins(22, 20, 22, 22);
    roomLayout->setSpacing(12);

    auto* roomHeader = new QHBoxLayout();
    auto* roomTitle =
        new QLabel(QStringLiteral("活动房间"), controls_.roomPanel);
    roomTitle->setObjectName(QStringLiteral("sectionTitle"));
    roomHeader->addWidget(roomTitle);
    roomHeader->addStretch(1);
    controls_.occupancyLabel = new QLabel(
        QStringLiteral("0 在线 · 0/2 席位"), controls_.roomPanel);
    controls_.occupancyLabel->setObjectName(QStringLiteral("readyPill"));
    controls_.occupancyLabel->setAlignment(Qt::AlignCenter);
    controls_.occupancyLabel->setSizePolicy(
        QSizePolicy::Maximum, QSizePolicy::Fixed);
    controls_.occupancyLabel->setFixedHeight(32);
    controls_.occupancyLabel->setMaximumWidth(176);
    roomHeader->addWidget(
        controls_.occupancyLabel, 0, Qt::AlignTop | Qt::AlignRight);
    roomLayout->addLayout(roomHeader);

    auto* roomIdCaption =
        new QLabel(QStringLiteral("房间 ID"), controls_.roomPanel);
    roomIdCaption->setProperty("muted", true);
    controls_.roomIdLabel =
        new QLabel(QStringLiteral("—"), controls_.roomPanel);
    controls_.roomIdLabel->setObjectName(QStringLiteral("roomIdValue"));
    controls_.roomIdLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    auto* roomIdRow = new QHBoxLayout();
    roomIdRow->setSpacing(10);
    roomIdRow->addWidget(roomIdCaption);
    roomIdRow->addWidget(controls_.roomIdLabel, 1);
    controls_.copyRoomIdButton = new QPushButton(
        QStringLiteral("复制房间 ID"), controls_.roomPanel);
    controls_.copyRoomIdButton->setObjectName(QStringLiteral("softButton"));
    controls_.copyRoomIdButton->setIconSize(QSize(16, 16));
    ui::RemoteCTheme::SetIcon(
        controls_.copyRoomIdButton,
        QStringLiteral(":/ui/icons/actions/copy.svg"),
        ui::ThemeIconTone::kPrimary);
    remotec::ui::morph::MorphIconButtonBinding::attach(
        controls_.copyRoomIdButton,
        QStringLiteral(":/ui/icons/lucide/base/copy.svg"),
        QStringLiteral(":/ui/icons/lucide/base/circle-check-big.svg"),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::Feedback,
        QSize(16, 16), QColor(QStringLiteral("#2563EB")),
        QColor(QStringLiteral("#12B76A")));
    controls_.copyRoomIdButton->setCursor(Qt::PointingHandCursor);
    controls_.copyRoomIdButton->setEnabled(false);
    controls_.copyRoomIdButton->ensurePolished();
    const int copyRoomIdWidth = controls_.copyRoomIdButton->sizeHint().width();
    controls_.copyRoomIdButton->setText(QStringLiteral("已复制"));
    controls_.copyRoomIdButton->setFixedWidth(std::max(
        copyRoomIdWidth, controls_.copyRoomIdButton->sizeHint().width()));
    controls_.copyRoomIdButton->setText(QStringLiteral("复制房间 ID"));
    roomIdRow->addWidget(controls_.copyRoomIdButton);
    roomLayout->addLayout(roomIdRow);

    auto* memberPanel = new QFrame(controls_.roomPanel);
    memberPanel->setProperty("card", true);
    memberPanel->setObjectName(QStringLiteral("roomMembersPanel"));
    auto* memberLayout = new QVBoxLayout(memberPanel);
    memberLayout->setContentsMargins(16, 16, 16, 14);
    memberLayout->setSpacing(10);
    controls_.memberSummaryLabel =
        new QLabel(QStringLiteral("房间成员  (0/2)"), memberPanel);
    controls_.memberSummaryLabel->setObjectName(QStringLiteral("sectionTitle"));
    memberLayout->addWidget(controls_.memberSummaryLabel);
    memberLayout->addWidget(detail::MakeDivider(memberPanel));
    controls_.memberList = new QListWidget(memberPanel);
    controls_.memberList->setObjectName(QStringLiteral("roomMemberList"));
    controls_.memberList->setSelectionMode(QAbstractItemView::NoSelection);
    controls_.memberList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(controls_.memberList, &QListWidget::customContextMenuRequested,
            this, &RoomPage::MemberContextMenuRequested);
    controls_.memberList->setMinimumHeight(360);
    EnableSmoothWheelScrolling(controls_.memberList);
    memberLayout->addWidget(controls_.memberList, 1);
    memberLayout->addWidget(detail::MakeDivider(memberPanel));
    controls_.memberFooterLabel = new QLabel(
        QStringLiteral("人数上限：2 人        0 在线"), memberPanel);
    controls_.memberFooterLabel->setProperty("muted", true);
    memberLayout->addWidget(controls_.memberFooterLabel);

    auto* statusPanel = new QFrame(controls_.roomPanel);
    statusPanel->setObjectName(QStringLiteral("roomSectionPanel"));
    auto* roomStatusLayout = new QVBoxLayout(statusPanel);
    roomStatusLayout->setContentsMargins(14, 12, 14, 12);
    roomStatusLayout->setSpacing(8);
    auto* roomStatusTitle = new QLabel(QStringLiteral("房间状态"), statusPanel);
    roomStatusTitle->setObjectName(QStringLiteral("roomMediaTitle"));
    roomStatusLayout->addWidget(roomStatusTitle);
    auto* roomInfo = new QGridLayout();
    roomInfo->setHorizontalSpacing(12);
    roomInfo->setVerticalSpacing(6);
    auto addRoomInfoRow = [statusPanel, roomInfo](
                              int row, int column, RoomStatusIcon icon,
                              const QString& caption, QLabel** valueLabel) {
        const int baseColumn = column * 4;
        roomInfo->addWidget(CreateRoomStatusIndicator(statusPanel, icon),
                            row, baseColumn, Qt::AlignVCenter);
        auto* label = new QLabel(caption, statusPanel);
        label->setProperty("muted", true);
        roomInfo->addWidget(label, row, baseColumn + 1);
        *valueLabel = new QLabel(QStringLiteral("—"), statusPanel);
        (*valueLabel)->setProperty("roomValue", true);
        (*valueLabel)->setTextInteractionFlags(Qt::TextSelectableByMouse);
        (*valueLabel)->setWordWrap(false);
        (*valueLabel)->setSizePolicy(QSizePolicy::Ignored,
                                     QSizePolicy::Preferred);
        roomInfo->addWidget(*valueLabel, row, baseColumn + 2);
    };
    addRoomInfoRow(0, 0, RoomStatusIcon::kPerson, QStringLiteral("房主"),
                   &controls_.ownerLabel);
    addRoomInfoRow(1, 0, RoomStatusIcon::kScreen,
                   QStringLiteral("当前主机器"), &controls_.screenSharerLabel);
    addRoomInfoRow(2, 0, RoomStatusIcon::kController,
                   QStringLiteral("当前控制者"), &controls_.controllerLabel);
    addRoomInfoRow(0, 1, RoomStatusIcon::kNetwork,
                   QStringLiteral("本机 P2P"),
                   &controls_.peerConnectivityLabel);
    roomInfo->setColumnStretch(2, 1);
    roomInfo->setColumnMinimumWidth(3, 24);
    roomInfo->setColumnStretch(6, 1);

    roomInfo->addWidget(
        CreateRoomStatusIndicator(statusPanel, RoomStatusIcon::kSeats),
        1, 4, Qt::AlignVCenter);
    auto* activeCapacityCaption =
        new QLabel(QStringLiteral("人数上限"), statusPanel);
    activeCapacityCaption->setProperty("muted", true);
    roomInfo->addWidget(activeCapacityCaption, 1, 5);
    auto* capacityControls = new QHBoxLayout();
    capacityControls->setContentsMargins(0, 0, 0, 0);
    capacityControls->setSpacing(6);
    capacityControls->addStretch(1);
    controls_.activeRoomCapacity = new RemoteCComboBox(statusPanel);
    controls_.activeRoomCapacity->setObjectName(
        QStringLiteral("capacitySelector"));
    detail::AddRoomCapacityItems(controls_.activeRoomCapacity);
    controls_.activeRoomCapacity->setEnabled(false);
    controls_.activeRoomCapacity->setFixedHeight(36);
    controls_.activeRoomCapacity->setMinimumWidth(92);
    capacityControls->addWidget(controls_.activeRoomCapacity);
    controls_.applyRoomCapacityButton =
        new QPushButton(QStringLiteral("应用"), statusPanel);
    controls_.applyRoomCapacityButton->setObjectName(
        QStringLiteral("softButton"));
    controls_.applyRoomCapacityButton->setCursor(Qt::PointingHandCursor);
    controls_.applyRoomCapacityButton->setEnabled(false);
    controls_.applyRoomCapacityButton->setMinimumHeight(36);
    capacityControls->addWidget(controls_.applyRoomCapacityButton);
    roomInfo->addLayout(capacityControls, 1, 6);
    addRoomInfoRow(2, 1, RoomStatusIcon::kSeats,
                   QStringLiteral("当前席位"), &controls_.seatUsageLabel);
    roomStatusLayout->addLayout(roomInfo);
    roomLayout->addWidget(statusPanel);

    auto* mediaBar = new QFrame(controls_.roomPanel);
    mediaBar->setObjectName(QStringLiteral("roomMediaBar"));
    auto* mediaLayout = new QVBoxLayout(mediaBar);
    mediaLayout->setContentsMargins(11, 10, 11, 11);
    mediaLayout->setSpacing(8);
    auto* mediaTitle = new QLabel(QStringLiteral("快速控制"), mediaBar);
    mediaTitle->setObjectName(QStringLiteral("roomMediaTitle"));
    mediaLayout->addWidget(mediaTitle);
    auto* mediaGrid = new QGridLayout();
    mediaGrid->setHorizontalSpacing(8);
    mediaGrid->setVerticalSpacing(8);

    controls_.screenShareButton = new QPushButton(mediaBar);
    controls_.screenShareButton->setObjectName(
        QStringLiteral("mediaIconButton"));
    controls_.screenShareButton->setCursor(Qt::PointingHandCursor);
    controls_.screenShareButton->setFocusPolicy(Qt::NoFocus);
    controls_.screenShareButton->setEnabled(false);
    controls_.screenShareButton->setFixedHeight(54);
    SetMediaStateButton(controls_.screenShareButton, MediaStateIcon::kScreen,
                        false, QStringLiteral("共享本机屏幕"));
    mediaGrid->addWidget(controls_.screenShareButton, 0, 0);

    auto* cameraDeviceButton = new MediaDeviceButton(mediaBar);
    controls_.cameraButton = cameraDeviceButton;
    controls_.cameraButton->setObjectName(QStringLiteral("mediaIconButton"));
    controls_.cameraButton->setCursor(Qt::PointingHandCursor);
    controls_.cameraButton->setFocusPolicy(Qt::NoFocus);
    controls_.cameraButton->setEnabled(false);
    controls_.cameraButton->setFixedHeight(54);
    SetMediaStateButton(controls_.cameraButton, MediaStateIcon::kCamera, false,
                        QStringLiteral("开启摄像头；右侧箭头选择设备"));
    cameraDeviceButton->SetDeviceMenuHandler([this] {
        emit MediaDeviceMenuRequested(MediaDeviceKind::kCamera,
                                      controls_.cameraButton);
    });
    mediaGrid->addWidget(controls_.cameraButton, 0, 1);

    auto* microphoneDeviceButton = new MediaDeviceButton(mediaBar);
    controls_.microphoneButton = microphoneDeviceButton;
    controls_.microphoneButton->setObjectName(
        QStringLiteral("mediaIconButton"));
    controls_.microphoneButton->setCursor(Qt::PointingHandCursor);
    controls_.microphoneButton->setFocusPolicy(Qt::NoFocus);
    controls_.microphoneButton->setEnabled(false);
    controls_.microphoneButton->setFixedHeight(54);
    SetMediaStateButton(controls_.microphoneButton,
                        MediaStateIcon::kMicrophone, false,
                        QStringLiteral("开启麦克风；右侧箭头选择设备"));
    microphoneDeviceButton->SetDeviceMenuHandler([this] {
        emit MediaDeviceMenuRequested(MediaDeviceKind::kMicrophone,
                                      controls_.microphoneButton);
    });
    mediaGrid->addWidget(controls_.microphoneButton, 0, 2);

    auto* speakerDeviceButton = new MediaDeviceButton(mediaBar);
    controls_.speakerButton = speakerDeviceButton;
    controls_.speakerButton->setObjectName(QStringLiteral("mediaIconButton"));
    controls_.speakerButton->setCursor(Qt::PointingHandCursor);
    controls_.speakerButton->setFocusPolicy(Qt::NoFocus);
    controls_.speakerButton->setEnabled(false);
    controls_.speakerButton->setFixedHeight(54);
    SetMediaStateButton(controls_.speakerButton, MediaStateIcon::kSpeaker, true,
                        QStringLiteral("关闭远端声音；右侧箭头选择设备"));
    speakerDeviceButton->SetDeviceMenuHandler([this] {
        emit MediaDeviceMenuRequested(MediaDeviceKind::kSpeaker,
                                      controls_.speakerButton);
    });
    mediaGrid->addWidget(controls_.speakerButton, 0, 3);

    controls_.cameraGalleryButton = new QPushButton(mediaBar);
    controls_.cameraGalleryButton->setObjectName(
        QStringLiteral("mediaIconButton"));
    controls_.cameraGalleryButton->setCursor(Qt::PointingHandCursor);
    controls_.cameraGalleryButton->setFocusPolicy(Qt::NoFocus);
    controls_.cameraGalleryButton->setEnabled(false);
    controls_.cameraGalleryButton->setFixedHeight(54);
    SetCameraGalleryStateButton(
        controls_.cameraGalleryButton, false, false,
        QStringLiteral("摄像头画廊 · 0 人开启"));
    mediaGrid->addWidget(controls_.cameraGalleryButton, 0, 4);
    for (int column = 0; column < 5; ++column) {
        mediaGrid->setColumnStretch(column, 1);
    }
    mediaLayout->addLayout(mediaGrid);
    roomLayout->addWidget(mediaBar);

    controls_.stageLabel = new QLabel(
        QStringLiteral("房间控制面已连接；成员加入后将自动建立 P2P 连接。"),
        controls_.roomPanel);
    controls_.stageLabel->setObjectName(QStringLiteral("roomStageHint"));
    controls_.stageLabel->setWordWrap(true);
    auto* roomFooter = new QHBoxLayout();
    roomFooter->setSpacing(12);
    roomFooter->addWidget(controls_.stageLabel, 1);
    controls_.fileTransferButton =
        new QPushButton(QStringLiteral("文件传输"), controls_.roomPanel);
    controls_.fileTransferButton->setObjectName(QStringLiteral("softButton"));
    controls_.fileTransferButton->setCursor(Qt::PointingHandCursor);
    controls_.fileTransferButton->setEnabled(false);
    controls_.fileTransferButton->setMinimumHeight(40);
    controls_.fileTransferButton->hide();
    controls_.leaveButton =
        new QPushButton(QStringLiteral("离开房间"), controls_.roomPanel);
    controls_.leaveButton->setObjectName(QStringLiteral("dangerButton"));
    controls_.leaveButton->setCursor(Qt::PointingHandCursor);
    controls_.leaveButton->setEnabled(false);
    roomFooter->addWidget(controls_.leaveButton);
    roomLayout->addLayout(roomFooter);

    activeRoomShell->addWidget(activeRoomMainCard, 2);
    activeRoomShell->addWidget(memberPanel, 1);

    controls_.workspaceStack = new CurrentPageStack(content_);
    controls_.workspaceStack->setObjectName(
        QStringLiteral("roomWorkspaceStack"));
    controls_.workspaceStack->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Fixed);
    controls_.workspaceStack->addWidget(controls_.entryPanel);
    controls_.workspaceStack->addWidget(controls_.roomPanel);
    controls_.workspaceStack->setCurrentWidget(controls_.entryPanel);

    auto* columns = new QHBoxLayout();
    columns->setSpacing(18);
    auto* primaryColumn = new QVBoxLayout();
    primaryColumn->setSpacing(18);
    primaryColumn->addWidget(controls_.workspaceStack);
    primaryColumn->addStretch(1);
    columns->addLayout(primaryColumn, 1);
    contentLayout_->addLayout(columns, 1);

    QTimer::singleShot(0, controls_.workspaceStack, [this] {
        if (auto* stack =
                dynamic_cast<CurrentPageStack*>(controls_.workspaceStack)) {
            stack->RefreshCurrentHeight();
        }
    });
}

}  // namespace remote::controller
