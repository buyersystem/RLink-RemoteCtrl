// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RecentConnectionsPage.h"

#include <QColor>
#include <QFile>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QWidget>

#include "src/apps/controller/FramelessWindow.h"
#include "src/apps/controller/ui/RemoteCTheme.h"
#include "src/apps/controller/ui/morph/MorphIconButtonBinding.h"

namespace remote::controller {
namespace {

QFrame* MakeEmptyState(
    QWidget* parent,
    const QString& iconPath,
    const QString& title,
    const QString& hint)
{
    auto* emptyState = new QFrame(parent);
    emptyState->setObjectName(QStringLiteral("recentEmptyState"));
    auto* layout = new QVBoxLayout(emptyState);
    layout->setContentsMargins(26, 26, 26, 28);
    layout->setSpacing(11);
    layout->setAlignment(Qt::AlignCenter);

    auto* icon = new QLabel(emptyState);
    icon->setObjectName(QStringLiteral("recentEmptyIcon"));
    icon->setAlignment(Qt::AlignCenter);
    icon->setFixedSize(52, 52);
    ui::RemoteCTheme::SetPixmap(
        icon, iconPath, QSize(26, 26), ui::ThemeIconTone::kPrimary);
    layout->addWidget(icon, 0, Qt::AlignCenter);

    auto* titleLabel = new QLabel(title, emptyState);
    titleLabel->setObjectName(QStringLiteral("recentEmptyTitle"));
    layout->addWidget(titleLabel, 0, Qt::AlignCenter);

    auto* hintLabel = new QLabel(hint, emptyState);
    hintLabel->setObjectName(QStringLiteral("recentEmptyHint"));
    hintLabel->setAlignment(Qt::AlignCenter);
    hintLabel->setWordWrap(false);
    hintLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    layout->addWidget(hintLabel);
    return emptyState;
}

void ClearCards(QVBoxLayout* layout, QFrame* emptyState)
{
    while (auto* item = layout->takeAt(0)) {
        if (auto* widget = item->widget();
            widget && widget != emptyState) {
            widget->deleteLater();
        }
        delete item;
    }
}

}  // namespace

RecentConnectionsPage::RecentConnectionsPage(QWidget* parent)
    : QScrollArea(parent)
{
    setObjectName(QStringLiteral("recentConnectionsPage"));
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    EnableSmoothWheelScrolling(this);

    content_ = new QWidget(this);
    auto* pageLayout = new QVBoxLayout(content_);
    pageLayout->setContentsMargins(32, 28, 32, 30);
    pageLayout->setSpacing(20);

    auto* header = new QVBoxLayout();
    header->setSpacing(3);
    auto* title = new QLabel(QStringLiteral("最近连接"), content_);
    title->setObjectName(QStringLiteral("pageTitle"));
    auto* subtitle = new QLabel(
        QStringLiteral("快速回到近期使用过的设备和协作房间。"),
        content_);
    subtitle->setObjectName(QStringLiteral("pageSubtitle"));
    subtitle->setWordWrap(true);
    header->addWidget(title);
    header->addWidget(subtitle);
    pageLayout->addLayout(header);

    auto* devicesHeader = new QHBoxLayout();
    auto* devicesTitle = new QLabel(QStringLiteral("最近设备"), content_);
    devicesTitle->setObjectName(QStringLiteral("recentSectionTitle"));
    devicesHeader->addWidget(devicesTitle);
    auto* devicesLimit = new QLabel(QStringLiteral("最多 5 个"), content_);
    devicesLimit->setObjectName(QStringLiteral("recentLimitPill"));
    devicesHeader->addWidget(devicesLimit);
    devicesHeader->addStretch(1);
    pageLayout->addLayout(devicesHeader);

    devicesLayout_ = new QVBoxLayout();
    devicesLayout_->setSpacing(10);
    devicesEmptyState_ = MakeEmptyState(
        content_, QStringLiteral(":/ui/icons/light/devices.svg"),
        QStringLiteral("暂无最近设备"),
        QStringLiteral("成功进入远程桌面后，这里会保留最近使用的 5 台设备。"));
    devicesLayout_->addWidget(devicesEmptyState_);
    pageLayout->addLayout(devicesLayout_);
    pageLayout->addSpacing(18);

    auto* roomsHeader = new QHBoxLayout();
    auto* roomsTitle = new QLabel(QStringLiteral("最近房间"), content_);
    roomsTitle->setObjectName(QStringLiteral("recentSectionTitle"));
    roomsHeader->addWidget(roomsTitle);
    auto* roomsLimit = new QLabel(QStringLiteral("最多 3 个"), content_);
    roomsLimit->setObjectName(QStringLiteral("recentLimitPill"));
    roomsHeader->addWidget(roomsLimit);
    roomsHeader->addStretch(1);
    pageLayout->addLayout(roomsHeader);

    roomsLayout_ = new QVBoxLayout();
    roomsLayout_->setSpacing(10);
    roomsEmptyState_ = MakeEmptyState(
        content_, QStringLiteral(":/ui/icons/light/room.svg"),
        QStringLiteral("暂无最近房间"),
        QStringLiteral("创建或加入房间后，这里会保留最近使用的 3 个房间。"));
    roomsLayout_->addWidget(roomsEmptyState_);
    pageLayout->addLayout(roomsLayout_);
    pageLayout->addStretch(1);
    setWidget(content_);

    QFile pageStyle(QStringLiteral(":/ui/theme/recent-connections.qss"));
    if (pageStyle.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setStyleSheet(QString::fromUtf8(pageStyle.readAll()));
    }
}

void RecentConnectionsPage::SetRooms(
    const QVector<RecentRoomCardData>& rooms)
{
    ClearCards(roomsLayout_, roomsEmptyState_);
    roomsEmptyState_->setVisible(rooms.isEmpty());
    if (rooms.isEmpty()) {
        roomsLayout_->addWidget(roomsEmptyState_);
        return;
    }

    for (const auto& room : rooms) {
        auto* card = new QFrame(content_);
        card->setProperty("card", true);
        auto* row = new QHBoxLayout(card);
        row->setContentsMargins(16, 10, 16, 10);
        row->setSpacing(13);

        auto* icon = new QLabel(card);
        icon->setObjectName(QStringLiteral("recentRoomIcon"));
        icon->setAlignment(Qt::AlignCenter);
        icon->setFixedSize(44, 44);
        ui::RemoteCTheme::SetPixmap(
            icon, QStringLiteral(":/ui/icons/light/room.svg"),
            QSize(23, 23), ui::ThemeIconTone::kPrimary);
        row->addWidget(icon);

        auto* text = new QVBoxLayout();
        text->setSpacing(4);
        auto* id = new QLabel(room.roomId, card);
        id->setObjectName(QStringLiteral("cardTitle"));
        id->setTextInteractionFlags(Qt::TextSelectableByMouse);
        text->addWidget(id);
        auto* detail = new QLabel(room.detail, card);
        detail->setProperty("muted", true);
        text->addWidget(detail);
        row->addLayout(text, 1);

        auto* availability = new QLabel(room.availabilityText, card);
        availability->setProperty(
            "availabilityTone", room.availabilityTone);
        availability->setObjectName(
            QStringLiteral("recentAvailabilityPill"));
        availability->setAlignment(Qt::AlignCenter);
        row->addWidget(availability);

        auto* action = new QPushButton(room.actionText, card);
        action->setObjectName(QStringLiteral("softButton"));
        action->setCursor(room.canJoin ? Qt::PointingHandCursor
                                       : Qt::ArrowCursor);
        action->setEnabled(room.canJoin);
        action->setIconSize(QSize(16, 16));
        ui::RemoteCTheme::SetIcon(
            action, QStringLiteral(":/ui/icons/lucide/base/users-round.svg"),
            ui::ThemeIconTone::kPrimary);
        const bool dark = ui::RemoteCTheme::IsDark(
            ui::RemoteCTheme::LoadPreference());
        remotec::ui::morph::MorphIconButtonBinding::attach(
            action,
            QStringLiteral(":/ui/icons/lucide/base/users-round.svg"),
            QStringLiteral(":/ui/icons/lucide/base/handshake.svg"),
            remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
            QSize(16, 16),
            QColor(dark ? QStringLiteral("#8EA5FF")
                        : QStringLiteral("#315EFB")),
            QColor(dark ? QStringLiteral("#4FF0B5")
                        : QStringLiteral("#168A5B")));
        connect(action, &QPushButton::clicked, this,
                [this, roomId = room.roomId] {
                    emit joinRoomRequested(roomId);
                });
        row->addWidget(action);
        roomsLayout_->addWidget(card);
    }
}

void RecentConnectionsPage::SetDevices(
    const QVector<RecentDeviceCardData>& devices)
{
    ClearCards(devicesLayout_, devicesEmptyState_);
    devicesEmptyState_->setVisible(devices.isEmpty());
    if (devices.isEmpty()) {
        devicesLayout_->addWidget(devicesEmptyState_);
        return;
    }

    for (const auto& device : devices) {
        auto* card = new QFrame(content_);
        card->setProperty("card", true);
        auto* row = new QHBoxLayout(card);
        row->setContentsMargins(16, 10, 16, 10);
        row->setSpacing(13);

        auto* icon = new QLabel(card);
        icon->setObjectName(QStringLiteral("recentDeviceIcon"));
        icon->setAlignment(Qt::AlignCenter);
        icon->setFixedSize(44, 44);
        ui::RemoteCTheme::SetPixmap(
            icon, QStringLiteral(":/ui/icons/light/monitor.svg"),
            QSize(23, 23), device.ownedDeviceOnline
                ? ui::ThemeIconTone::kPrimary
                : ui::ThemeIconTone::kNeutral);
        row->addWidget(icon);

        auto* text = new QVBoxLayout();
        text->setSpacing(4);
        auto* name = new QLabel(device.deviceName, card);
        name->setObjectName(QStringLiteral("cardTitle"));
        text->addWidget(name);
        auto* detail = new QLabel(device.detail, card);
        detail->setProperty("muted", true);
        text->addWidget(detail);
        row->addLayout(text, 1);

        auto* action = new QPushButton(device.actionText, card);
        const bool primary = device.ownedDeviceOnline || !device.ownedDevice;
        action->setObjectName(primary ? QStringLiteral("primaryButton")
                                      : QStringLiteral("softButton"));
        action->setEnabled(device.actionEnabled);
        action->setCursor(device.actionEnabled ? Qt::PointingHandCursor
                                                : Qt::ArrowCursor);
        action->setIconSize(QSize(21, 21));
        ui::RemoteCTheme::SetIcon(
            action, QStringLiteral(":/ui/icons/lucide/base/monitor.svg"),
            primary ? ui::ThemeIconTone::kOnDark
                    : ui::ThemeIconTone::kNeutral);
        remotec::ui::morph::MorphIconButtonBinding::attach(
            action,
            QStringLiteral(":/ui/icons/lucide/base/monitor.svg"),
            QStringLiteral(":/ui/icons/lucide/base/monitor-check.svg"),
            remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
            QSize(21, 21), QColor(QStringLiteral("#FFFFFF")),
            QColor(QStringLiteral("#FFFFFF")));
        if (device.ownedDevice) {
            connect(action, &QPushButton::clicked, this,
                    [this, id = device.deviceId,
                     name = device.deviceName] {
                        emit connectOwnedDeviceRequested(id, name);
                    });
        } else {
            connect(action, &QPushButton::clicked, this,
                    [this, id = device.deviceId] {
                        emit reconnectAssistedDeviceRequested(id);
                    });
        }
        row->addWidget(action);
        devicesLayout_->addWidget(card);
    }
}

}  // namespace remote::controller
