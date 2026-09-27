// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "OwnedDevicesPage.h"

#include <algorithm>

#include <QColor>
#include <QDateTime>
#include <QFile>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QWidget>

#include "src/apps/controller/ui/RemoteCTheme.h"
#include "src/apps/controller/ui/morph/MorphIconButtonBinding.h"

namespace remote::controller {

OwnedDevicesPage::OwnedDevicesPage(QWidget* parent)
    : QScrollArea(parent)
{
    setObjectName(QStringLiteral("ownedDevicesPage"));
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    content_ = new QWidget(this);
    auto* pageLayout = new QVBoxLayout(content_);
    pageLayout->setContentsMargins(32, 28, 32, 30);
    pageLayout->setSpacing(20);

    auto* pageHeader = new QVBoxLayout();
    pageHeader->setSpacing(3);
    auto* title = new QLabel(QStringLiteral("我的设备"), content_);
    title->setObjectName(QStringLiteral("pageTitle"));
    auto* subtitle = new QLabel(
        QStringLiteral("同一 RLink 账户下的电脑；在线设备可直接进入桌面。"),
        content_);
    subtitle->setObjectName(QStringLiteral("pageSubtitle"));
    subtitle->setWordWrap(true);
    pageHeader->addWidget(title);
    pageHeader->addWidget(subtitle);
    pageLayout->addLayout(pageHeader);

    auto* summaryCard = new QFrame(content_);
    summaryCard->setProperty("card", true);
    summaryCard->setObjectName(QStringLiteral("ownedDevicesSummaryCard"));
    auto* summaryLayout = new QHBoxLayout(summaryCard);
    summaryLayout->setContentsMargins(22, 18, 18, 18);
    summaryLayout->setSpacing(16);

    auto* headerIcon = new QLabel(summaryCard);
    headerIcon->setObjectName(QStringLiteral("ownedDevicesHeaderIcon"));
    headerIcon->setAlignment(Qt::AlignCenter);
    headerIcon->setFixedSize(48, 48);
    ui::RemoteCTheme::SetPixmap(
        headerIcon, QStringLiteral(":/ui/icons/light/devices.svg"),
        QSize(26, 26), ui::ThemeIconTone::kPrimary);
    summaryLayout->addWidget(headerIcon);

    auto* summaryText = new QVBoxLayout();
    summaryText->setSpacing(3);
    auto* summaryTitle = new QLabel(
        QStringLiteral("同账号设备"), summaryCard);
    summaryTitle->setObjectName(QStringLiteral("ownedDevicesHeaderTitle"));
    summaryLabel_ = new QLabel(
        QStringLiteral("正在读取设备列表…"), summaryCard);
    summaryLabel_->setObjectName(
        QStringLiteral("ownedDevicesSummaryText"));
    summaryText->addWidget(summaryTitle);
    summaryText->addWidget(summaryLabel_);
    summaryLayout->addLayout(summaryText, 1);

    auto* scopePill = new QLabel(
        QStringLiteral("安全设备列表"), summaryCard);
    scopePill->setObjectName(QStringLiteral("ownedDevicesScopePill"));
    scopePill->setAlignment(Qt::AlignCenter);
    summaryLayout->addWidget(scopePill, 0, Qt::AlignVCenter);

    refreshButton_ = new QPushButton(summaryCard);
    refreshButton_->setObjectName(
        QStringLiteral("ownedDevicesRefreshButton"));
    refreshButton_->setIconSize(QSize(18, 18));
    refreshButton_->setToolTip(QStringLiteral("刷新设备列表"));
    refreshButton_->setCursor(Qt::PointingHandCursor);
    refreshButton_->setFixedSize(42, 42);
    summaryLayout->addWidget(refreshButton_);
    pageLayout->addWidget(summaryCard);

    cardsLayout_ = new QVBoxLayout();
    cardsLayout_->setSpacing(10);
    pageLayout->addLayout(cardsLayout_);

    emptyState_ = new QFrame(content_);
    emptyState_->setObjectName(QStringLiteral("ownedDevicesEmptyState"));
    auto* emptyLayout = new QVBoxLayout(emptyState_);
    emptyLayout->setContentsMargins(32, 30, 32, 34);
    emptyLayout->setSpacing(11);
    emptyLayout->setAlignment(Qt::AlignCenter);

    emptyArtwork_ = new QLabel(emptyState_);
    emptyArtwork_->setAlignment(Qt::AlignCenter);
    emptyArtwork_->setFixedSize(220, 128);
    emptyArtwork_->setPixmap(
        QIcon(QStringLiteral(":/ui/illustrations/devices/no-devices.svg"))
            .pixmap(210, 122));
    emptyLayout->addWidget(emptyArtwork_, 0, Qt::AlignCenter);
    emptyLayout->addSpacing(2);

    auto* emptyTitle = new QLabel(
        QStringLiteral("还没有其他设备"), emptyState_);
    emptyTitle->setObjectName(QStringLiteral("ownedDevicesEmptyTitle"));
    emptyLayout->addWidget(emptyTitle, 0, Qt::AlignCenter);

    auto* emptyHint = new QLabel(
        QStringLiteral("在另一台电脑登录同一账户后，它会自动出现在这里。"),
        emptyState_);
    emptyHint->setObjectName(QStringLiteral("ownedDevicesEmptyHint"));
    emptyHint->setAlignment(Qt::AlignCenter);
    emptyHint->setWordWrap(false);
    emptyHint->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    emptyLayout->addWidget(emptyHint);

    pageLayout->addWidget(emptyState_);
    pageLayout->addStretch(1);
    setWidget(content_);

    connect(refreshButton_, &QPushButton::clicked, this, [this] {
        PulseRefreshIcon();
        emit refreshRequested();
    });
    RefreshThemeStyle();

    QFile pageStyle(QStringLiteral(":/ui/theme/owned-devices.qss"));
    if (pageStyle.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setStyleSheet(QString::fromUtf8(pageStyle.readAll()));
    }
}

void OwnedDevicesPage::UpdateSnapshot(
    const OwnedDevicesSnapshot& ownedDevices,
    SessionConnectivityState connectivity)
{
    if (renderedRevision_ == ownedDevices.revision &&
        renderedConnectivity_ == connectivity) {
        return;
    }
    renderedRevision_ = ownedDevices.revision;
    renderedConnectivity_ = connectivity;

    while (QLayoutItem* item = cardsLayout_->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }

    const bool online =
        connectivity == SessionConnectivityState::kOnline;
    const int onlineCount = static_cast<int>(std::count_if(
        ownedDevices.devices.begin(), ownedDevices.devices.end(),
        [](const OwnedDeviceSnapshot& device) { return device.online; }));
    summaryLabel_->setText(
        !online ? QStringLiteral("信令离线，在线状态暂不可用")
                : QStringLiteral("%1 台设备，%2 台在线")
                      .arg(static_cast<int>(ownedDevices.devices.size()))
                      .arg(onlineCount));
    refreshButton_->setEnabled(online);

    const bool hasOtherDevice = std::any_of(
        ownedDevices.devices.begin(), ownedDevices.devices.end(),
        [](const OwnedDeviceSnapshot& device) { return !device.current; });
    emptyState_->setVisible(!hasOtherDevice);

    for (const auto& device : ownedDevices.devices) {
        auto* card = new QFrame(content_);
        card->setProperty("card", true);
        auto* row = new QHBoxLayout(card);
        row->setContentsMargins(18, 14, 18, 14);
        row->setSpacing(14);

        auto* icon = new QLabel(card);
        icon->setObjectName(QStringLiteral("ownedDeviceIcon"));
        icon->setAlignment(Qt::AlignCenter);
        icon->setFixedSize(52, 52);
        icon->setProperty("online", device.online);
        ui::RemoteCTheme::SetPixmap(
            icon, QStringLiteral(":/ui/icons/light/monitor.svg"),
            QSize(27, 27), device.online ? ui::ThemeIconTone::kPrimary
                                         : ui::ThemeIconTone::kNeutral);
        row->addWidget(icon);

        auto* text = new QVBoxLayout();
        text->setSpacing(4);
        const QString deviceName = device.deviceName.empty()
            ? QStringLiteral("RLink 设备")
            : QString::fromStdString(device.deviceName);
        auto* name = new QLabel(deviceName, card);
        name->setObjectName(QStringLiteral("cardTitle"));
        auto* details = new QLabel(
            QStringLiteral("设备 ID  %1  ·  %2")
                .arg(QString::fromStdString(device.deviceId),
                     device.current
                         ? QStringLiteral("当前设备")
                         : (device.online ? QStringLiteral("在线")
                                          : QStringLiteral("离线"))),
            card);
        details->setProperty("muted", true);
        text->addWidget(name);
        text->addWidget(details);
        if (!device.online && device.lastSeenAt > 0) {
            auto* lastSeen = new QLabel(
                QStringLiteral("最近在线：%1")
                    .arg(QDateTime::fromSecsSinceEpoch(device.lastSeenAt)
                             .toLocalTime()
                             .toString(QStringLiteral("yyyy-MM-dd HH:mm"))),
                card);
            lastSeen->setProperty("muted", true);
            text->addWidget(lastSeen);
        }
        row->addLayout(text, 1);

        auto* status = new QLabel(
            device.online ? QStringLiteral("● 在线")
                          : QStringLiteral("● 离线"), card);
        status->setObjectName(QStringLiteral("ownedDeviceStatus"));
        status->setProperty("online", device.online);
        status->setAlignment(Qt::AlignCenter);
        row->addWidget(status);

        auto* action = new QPushButton(
            device.current ? QStringLiteral("本机")
                           : QStringLiteral("进入桌面"), card);
        action->setObjectName(
            device.current ? QStringLiteral("softButton")
                           : QStringLiteral("primaryButton"));
        action->setCursor(device.online && !device.current
                              ? Qt::PointingHandCursor
                              : Qt::ArrowCursor);
        action->setEnabled(device.online && !device.current);
        action->setIconSize(QSize(21, 21));
        ui::RemoteCTheme::SetIcon(
            action,
            device.current
                ? QStringLiteral(":/ui/icons/light/monitor.svg")
                : QStringLiteral(":/ui/icons/actions/monitor.svg"),
            device.current ? ui::ThemeIconTone::kNeutral
                           : ui::ThemeIconTone::kOnDark);
        if (!device.current) {
            remotec::ui::morph::MorphIconButtonBinding::attach(
                action,
                QStringLiteral(":/ui/icons/lucide/base/monitor.svg"),
                QStringLiteral(":/ui/icons/lucide/base/monitor-check.svg"),
                remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
                QSize(21, 21), QColor(QStringLiteral("#FFFFFF")),
                QColor(QStringLiteral("#FFFFFF")));
            const QString deviceId =
                QString::fromStdString(device.deviceId);
            connect(action, &QPushButton::clicked, this,
                    [this, deviceId, deviceName] {
                        emit connectRequested(deviceId, deviceName);
                    });
        }
        row->addWidget(action);
        cardsLayout_->addWidget(card);
    }
}

void OwnedDevicesPage::RefreshThemeStyle()
{
    const bool dark = ui::RemoteCTheme::IsDark(
        ui::RemoteCTheme::LoadPreference());
    const QString artwork = dark
        ? QStringLiteral(
              ":/ui/illustrations/devices/no-devices-dark.svg")
        : QStringLiteral(
              ":/ui/illustrations/devices/no-devices.svg");
    emptyArtwork_->setPixmap(QIcon(artwork).pixmap(210, 122));
    const QColor refreshColor(
        dark ? QStringLiteral("#F7F9FF") : QStringLiteral("#2563EB"));
    ui::RemoteCTheme::SetIcon(
        refreshButton_, QStringLiteral(":/ui/icons/actions/refresh.svg"),
        dark ? ui::ThemeIconTone::kOnDark
             : ui::ThemeIconTone::kPrimary);
    remotec::ui::morph::MorphIconButtonBinding::attach(
        refreshButton_,
        QStringLiteral(":/ui/icons/lucide/base/refresh-cw.svg"),
        QStringLiteral(":/ui/icons/lucide/base/refresh-ccw.svg"),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::Feedback,
        QSize(18, 18), refreshColor, refreshColor);
}

void OwnedDevicesPage::PulseRefreshIcon()
{
    const bool dark = ui::RemoteCTheme::IsDark(
        ui::RemoteCTheme::LoadPreference());
    const QColor refreshColor(
        dark ? QStringLiteral("#F7F9FF") : QStringLiteral("#2563EB"));
    if (auto* morph = remotec::ui::morph::MorphIconButtonBinding::attach(
            refreshButton_,
            QStringLiteral(":/ui/icons/lucide/base/refresh-cw.svg"),
            QStringLiteral(":/ui/icons/lucide/base/refresh-ccw.svg"),
            remotec::ui::morph::MorphIconButtonBinding::Interaction::Feedback,
            QSize(18, 18), refreshColor, refreshColor)) {
        morph->pulse(500);
    }
}

}  // namespace remote::controller
