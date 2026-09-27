// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RoomStatusIndicator.h"

#include <QColor>
#include <QIcon>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QRect>

#include "ui/RemoteCTheme.h"

namespace remote::controller::detail {

QLabel* CreateRoomStatusIndicator(QWidget* parent, RoomStatusIcon type)
{
    constexpr int kLogicalSize = 24;
    constexpr qreal kBackingScale = 2.0;
    QPixmap pixmap(
        static_cast<int>(kLogicalSize * kBackingScale),
        static_cast<int>(kLogicalSize * kBackingScale));
    pixmap.setDevicePixelRatio(kBackingScale);
    pixmap.fill(Qt::transparent);
    QString iconName;
    switch (type) {
    case RoomStatusIcon::kPerson:
        iconName = QStringLiteral("circle-user-round");
        break;
    case RoomStatusIcon::kScreen:
        iconName = QStringLiteral("monitor");
        break;
    case RoomStatusIcon::kController:
        iconName = QStringLiteral("mouse-pointer-2");
        break;
    case RoomStatusIcon::kNetwork:
        iconName = QStringLiteral("wifi");
        break;
    case RoomStatusIcon::kSeats:
        iconName = QStringLiteral("users-round");
        break;
    }
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QIcon(QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(iconName))
        .paint(&painter, QRect(0, 0, kLogicalSize, kLogicalSize));
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    const bool dark = ui::RemoteCTheme::IsDark(
        ui::RemoteCTheme::LoadPreference());
    painter.fillRect(QRect(0, 0, kLogicalSize, kLogicalSize),
                     QColor(dark ? QStringLiteral("#91ADFF")
                                 : QStringLiteral("#5365F5")));
    painter.end();
    auto* label = new QLabel(parent);
    label->setFixedSize(24, 24);
    label->setPixmap(pixmap);
    label->setAlignment(Qt::AlignCenter);
    return label;
}

}  // namespace remote::controller::detail
