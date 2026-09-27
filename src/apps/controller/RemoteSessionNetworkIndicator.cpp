// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RemoteSessionNetworkIndicator.h"

#include <QColor>
#include <QPainter>
#include <QPaintEvent>
#include <QRect>
#include <QVariant>

namespace remote::controller {

NetworkSignalIndicator::NetworkSignalIndicator(QWidget* parent)
    : QWidget(parent)
{
    setFixedSize(24, 22);
    setProperty("activeBars", 0);
}

void NetworkSignalIndicator::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    const int activeBars = property("activeBars").toInt();
    constexpr int barWidth = 4;
    constexpr int gap = 3;
    constexpr int heights[] = {7, 12, 18};
    for (int index = 0; index < 3; ++index) {
        painter.setBrush(index < activeBars ? QColor("#54dc9a")
                                            : QColor("#344255"));
        painter.setPen(Qt::NoPen);
        const int x = 2 + index * (barWidth + gap);
        const int y = height() - heights[index] - 2;
        painter.drawRoundedRect(
            QRect(x, y, barWidth, heights[index]), 1.5, 1.5);
    }
}

}  // namespace remote::controller
