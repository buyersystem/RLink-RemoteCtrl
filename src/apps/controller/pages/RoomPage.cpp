// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RoomPage.h"

#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QVBoxLayout>
#include <QWidget>

#include "src/apps/controller/ControllerMainWindowSupport.h"
#include "src/apps/controller/FramelessWindow.h"
#include "src/apps/controller/RemoteCComboBox.h"

namespace remote::controller {

RoomPage::RoomPage(QWidget* parent)
    : QScrollArea(parent)
{
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // A stable scrollbar gutter prevents room-status text changes from
    // shifting the complete workspace horizontally.
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    EnableSmoothWheelScrolling(this);

    content_ = new QWidget(this);
    content_->setObjectName(QStringLiteral("content"));
    contentLayout_ = new QVBoxLayout(content_);
    contentLayout_->setContentsMargins(32, 28, 32, 30);
    contentLayout_->setSpacing(20);
    setWidget(content_);

    BuildHeaderAndEntry();
    BuildActiveRoom();
}

void RoomPage::BuildHeaderAndEntry()
{
    auto* headerRow = new QHBoxLayout();
    auto* roomHeaderIcon = new QLabel(content_);
    roomHeaderIcon->setObjectName(QStringLiteral("roomHeaderIcon"));
    roomHeaderIcon->setFixedSize(44, 44);
    roomHeaderIcon->setAlignment(Qt::AlignCenter);
    roomHeaderIcon->setStyleSheet(QStringLiteral(
        "background:#0b7cff;border-radius:11px;"));
    QPixmap roomHeaderPixmap(30, 30);
    roomHeaderPixmap.fill(Qt::transparent);
    {
        QPainter iconPainter(&roomHeaderPixmap);
        iconPainter.setRenderHint(QPainter::Antialiasing, true);
        iconPainter.setPen(QPen(Qt::white, 2.4, Qt::SolidLine,
                                Qt::RoundCap, Qt::RoundJoin));
        iconPainter.drawRoundedRect(QRectF(3, 4, 24, 17), 2, 2);
        iconPainter.drawLine(QPointF(15, 21), QPointF(15, 26));
        iconPainter.drawLine(QPointF(10, 27), QPointF(20, 27));
    }
    roomHeaderIcon->setPixmap(roomHeaderPixmap);
    headerRow->addWidget(roomHeaderIcon, 0, Qt::AlignTop);

    auto* headerLabels = new QVBoxLayout();
    headerLabels->setSpacing(3);
    auto* title = new QLabel(QStringLiteral("协作房间"), content_);
    title->setObjectName(QStringLiteral("pageTitle"));
    auto* subtitle = new QLabel(
        QStringLiteral("创建或加入房间；任意成员之后都可以成为主屏幕分享者"),
        content_);
    subtitle->setObjectName(QStringLiteral("pageSubtitle"));
    headerLabels->addWidget(title);
    headerLabels->addWidget(subtitle);
    headerRow->addLayout(headerLabels, 1);

    controls_.connectivityLabel =
        new QLabel(QStringLiteral("●  信令未连接"), content_);
    controls_.connectivityLabel->setObjectName(QStringLiteral("readyPill"));
    headerRow->addWidget(controls_.connectivityLabel, 0, Qt::AlignTop);
    contentLayout_->addLayout(headerRow);

    controls_.entryPanel = new QFrame(content_);
    controls_.entryPanel->setProperty("card", true);
    auto* connectLayout = new QVBoxLayout(controls_.entryPanel);
    connectLayout->setContentsMargins(22, 21, 22, 22);
    connectLayout->setSpacing(13);
    auto* eyebrow =
        new QLabel(QStringLiteral("房间入口"), controls_.entryPanel);
    eyebrow->setObjectName(QStringLiteral("eyebrow"));
    connectLayout->addWidget(eyebrow);
    auto* connectTitle =
        new QLabel(QStringLiteral("创建或加入协作房间"), controls_.entryPanel);
    connectTitle->setObjectName(QStringLiteral("cardTitle"));
    connectLayout->addWidget(connectTitle);
    auto* connectHint = new QLabel(
        QStringLiteral(
            "房间人数可以动态增加到设置的上限。默认 2 人时，同一套逻辑就是普通的两人远控。"),
        controls_.entryPanel);
    connectHint->setProperty("muted", true);
    connectHint->setWordWrap(true);
    connectLayout->addWidget(connectHint);

    auto* createCaption =
        new QLabel(QStringLiteral("创建房间"), controls_.entryPanel);
    createCaption->setObjectName(QStringLiteral("deviceName"));
    connectLayout->addWidget(createCaption);
    auto* createRow = new QHBoxLayout();
    createRow->setSpacing(10);
    auto* capacityCaption =
        new QLabel(QStringLiteral("人数上限"), controls_.entryPanel);
    capacityCaption->setProperty("muted", true);
    createRow->addWidget(capacityCaption);
    auto* createRoomCapacity = new RemoteCComboBox(controls_.entryPanel);
    createRoomCapacity->setObjectName(QStringLiteral("capacitySelector"));
    detail::AddRoomCapacityItems(createRoomCapacity);
    createRoomCapacity->setFixedHeight(43);
    createRoomCapacity->setEnabled(false);
    createRow->addWidget(createRoomCapacity);
    createRow->addStretch(1);
    auto* createRoomButton =
        new QPushButton(QStringLiteral("创建房间"), controls_.entryPanel);
    createRoomButton->setObjectName(QStringLiteral("primaryButton"));
    createRoomButton->setCursor(Qt::PointingHandCursor);
    createRoomButton->setFixedHeight(43);
    createRoomButton->setEnabled(false);
    createRow->addWidget(createRoomButton);
    connectLayout->addLayout(createRow);
    connectLayout->addWidget(detail::MakeDivider(controls_.entryPanel));

    auto* joinCaption =
        new QLabel(QStringLiteral("加入已有房间"), controls_.entryPanel);
    joinCaption->setObjectName(QStringLiteral("deviceName"));
    connectLayout->addWidget(joinCaption);
    auto* joinRow = new QHBoxLayout();
    joinRow->setSpacing(10);
    auto* roomIdEdit = new QLineEdit(controls_.entryPanel);
    roomIdEdit->setObjectName(QStringLiteral("roomIdInput"));
    roomIdEdit->setPlaceholderText(QStringLiteral("输入 9 位房间号"));
    roomIdEdit->setMaxLength(9);
    roomIdEdit->setValidator(new QRegularExpressionValidator(
        QRegularExpression(QStringLiteral("[0-9]{0,9}")), roomIdEdit));
    roomIdEdit->setClearButtonEnabled(true);
    roomIdEdit->setContextMenuPolicy(Qt::NoContextMenu);
    roomIdEdit->setFixedHeight(43);
    roomIdEdit->setEnabled(false);
    joinRow->addWidget(roomIdEdit, 1);
    auto* joinRoomButton =
        new QPushButton(QStringLiteral("申请加入  →"), controls_.entryPanel);
    joinRoomButton->setObjectName(QStringLiteral("primaryButton"));
    joinRoomButton->setCursor(Qt::PointingHandCursor);
    joinRoomButton->setFixedHeight(43);
    joinRoomButton->setEnabled(false);
    joinRow->addWidget(joinRoomButton);
    connectLayout->addLayout(joinRow);

    controls_.actionHint = new QLabel(
        QStringLiteral("正在启动本地引擎并检查 WSS 房间控制面。"),
        controls_.entryPanel);
    controls_.actionHint->setObjectName(QStringLiteral("previewHint"));
    controls_.actionHint->setWordWrap(true);
    connectLayout->addWidget(controls_.actionHint);

    SetEntryControls(createRoomCapacity, createRoomButton, roomIdEdit,
                     joinRoomButton);
}

RoomPageControls& RoomPage::Controls()
{
    return controls_;
}

const RoomPageControls& RoomPage::Controls() const
{
    return controls_;
}

void RoomPage::SetEntryControls(QComboBox* capacity,
                                QPushButton* createButton,
                                QLineEdit* roomId,
                                QPushButton* joinButton)
{
    capacity_ = capacity;
    createButton_ = createButton;
    roomId_ = roomId;
    joinButton_ = joinButton;

    connect(createButton_, &QPushButton::clicked, this, [this] {
        emit CreateRequested(capacity_->currentData().toUInt());
    });
    connect(joinButton_, &QPushButton::clicked, this, [this] {
        emit JoinRequested(roomId_->text().trimmed());
    });
    connect(roomId_, &QLineEdit::returnPressed,
            joinButton_, &QPushButton::click);
}

void RoomPage::FocusRoomId()
{
    if (roomId_) {
        roomId_->setFocus(Qt::OtherFocusReason);
    }
}

void RoomPage::SetEntryEnabled(bool enabled)
{
    if (capacity_) capacity_->setEnabled(enabled);
    if (createButton_) createButton_->setEnabled(enabled);
    if (roomId_) roomId_->setEnabled(enabled);
    if (joinButton_) joinButton_->setEnabled(enabled);
}

void RoomPage::SetEntryActionsEnabled(bool enabled)
{
    if (createButton_) createButton_->setEnabled(enabled);
    if (joinButton_) joinButton_->setEnabled(enabled);
}

void RoomPage::SetEntryActionTexts(const QString& createText,
                                   const QString& joinText)
{
    SetCreateActionText(createText);
    SetJoinActionText(joinText);
}

void RoomPage::SetCreateActionText(const QString& text)
{
    if (createButton_) createButton_->setText(text);
}

void RoomPage::SetJoinActionText(const QString& text)
{
    if (joinButton_) joinButton_->setText(text);
}

void RoomPage::SetCapacity(uint capacity)
{
    if (!capacity_) {
        return;
    }
    capacity_->setCurrentIndex(
        capacity_->findData(QVariant::fromValue(capacity)));
}

void RoomPage::SetRoomId(const QString& roomId)
{
    if (roomId_) roomId_->setText(roomId);
}

}  // namespace remote::controller
