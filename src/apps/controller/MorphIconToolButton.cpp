// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "MorphIconToolButton.h"

#include <algorithm>

#include <QColor>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QPen>

#include "FramelessWindow.h"

namespace remote::controller {

MorphIconToolButton::MorphIconToolButton(
    const QString& sourceResource,
    const QString& targetResource,
    QWidget* parent)
    : QToolButton(parent),
      sourceResource_(sourceResource),
      targetResource_(targetResource)
{
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    QString error;
    if (!morphIcon_.configure(
            sourceResource, targetResource, 64, &error)) {
        setIcon(QIcon(sourceResource));
        setIconSize(QSize(18, 18));
    }
    timer_.setTimerType(Qt::PreciseTimer);
    timer_.setInterval(16);
    connect(&timer_, &QTimer::timeout, this, [this] {
        const double elapsedSeconds = std::clamp(
            elapsed_.restart() / 1000.0, 1.0 / 1000.0, 0.05);
        const bool settled = spring_.step(elapsedSeconds);
        progress_ = start_ + (end_ - start_) * spring_.value();
        update();
        if (settled) {
            progress_ = end_;
            timer_.stop();
            update();
        }
    });
}

void MorphIconToolButton::SetTarget(bool target)
{
    if (!morphIcon_.isValid()) {
        setIcon(QIcon(target ? targetResource_ : sourceResource_));
        return;
    }
    start_ = progress_;
    end_ = target ? 1.0 : 0.0;
    if (CurrentUiAnimationLevel() <= 0) {
        progress_ = end_;
        timer_.stop();
        update();
        return;
    }
    spring_.configure(
        CurrentUiAnimationLevel() == 1 ? 170.0 : 420.0,
        CurrentUiAnimationLevel() == 1 ? 26.0 : 30.0);
    spring_.start();
    elapsed_.restart();
    timer_.start();
}

void MorphIconToolButton::paintEvent(QPaintEvent* event)
{
    QToolButton::paintEvent(event);
    if (!morphIcon_.isValid()) {
        return;
    }
    QPainter painter(this);
    QColor color(QStringLiteral("#D8D1C3"));
    if (!isEnabled()) {
        color = QColor(QStringLiteral("#738096"));
    } else if (progress_ > 0.5) {
        color = QColor(QStringLiteral("#FFC75A"));
    }
    const QRectF iconRect(
        (width() - 18.0) / 2.0,
        (height() - 18.0) / 2.0,
        18.0,
        18.0);
    morphIcon_.paint(painter, iconRect, color, progress_, 2.0);

    const qreal lockedOverlay =
        std::clamp((progress_ - 0.55) / 0.45, 0.0, 1.0);
    if (lockedOverlay <= 0.0) {
        return;
    }
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setOpacity(lockedOverlay);
    painter.translate(iconRect.left(), iconRect.top());
    painter.scale(iconRect.width() / 24.0, iconRect.height() / 24.0);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(QStringLiteral("#FFC857")));
    painter.drawRoundedRect(QRectF(3.0, 10.0, 18.0, 12.0), 2.0, 2.0);
    QPainterPath shackle;
    shackle.moveTo(7.0, 10.0);
    shackle.lineTo(7.0, 7.0);
    shackle.cubicTo(7.0, 4.24, 9.24, 2.0, 12.0, 2.0);
    shackle.cubicTo(14.76, 2.0, 17.0, 4.24, 17.0, 7.0);
    shackle.lineTo(17.0, 10.0);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(
        QColor(QStringLiteral("#F4F7FB")),
        2.0,
        Qt::SolidLine,
        Qt::RoundCap,
        Qt::RoundJoin));
    painter.drawPath(shackle);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(QStringLiteral("#5B4310")));
    painter.drawEllipse(QPointF(12.0, 16.0), 1.25, 1.25);
    painter.restore();
}

}  // namespace remote::controller
