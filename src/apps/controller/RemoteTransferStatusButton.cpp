// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RemoteTransferStatusButton.h"

#include <algorithm>
#include <cmath>

#include <QAbstractAnimation>
#include <QColor>
#include <QEasingCurve>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QVariantAnimation>

namespace remote::controller {

RemoteTransferStatusButton::RemoteTransferStatusButton(QWidget* parent)
    : QToolButton(parent)
{
    setCursor(Qt::PointingHandCursor);
    setToolTip(QStringLiteral("远程粘贴正在后台传输，点击查看"));
    setFixedSize(24, 28);
    waveTimer_.setInterval(34);
    connect(&waveTimer_, &QTimer::timeout, this, [this] {
        if (!isVisible()) {
            return;
        }
        wavePhase_ += 0.22;
        update();
    });
    waveTimer_.start();
}

void RemoteTransferStatusButton::SetProgress(double progress)
{
    progress = std::clamp(progress, 0.0, 1.0);
    if (std::abs(progress - targetProgress_) < 0.0005) {
        return;
    }
    targetProgress_ = progress;
    if (progressAnimation_) {
        progressAnimation_->stop();
        progressAnimation_->deleteLater();
    }
    auto* animation = new QVariantAnimation(this);
    progressAnimation_ = animation;
    animation->setDuration(260);
    animation->setStartValue(progress_);
    animation->setEndValue(targetProgress_);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    connect(animation, &QVariantAnimation::valueChanged, this,
        [this](const QVariant& value) {
            progress_ = value.toDouble();
            update();
        });
    connect(animation, &QVariantAnimation::finished, this,
        [this, animation] {
            if (progressAnimation_ == animation) {
                progressAnimation_ = nullptr;
            }
        });
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void RemoteTransferStatusButton::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    const QRectF bounds = QRectF(rect()).adjusted(0.75, 0.75, -0.75, -0.75);
    QPainterPath clip;
    clip.addRoundedRect(bounds, 7.0, 7.0);
    painter.fillPath(
        clip,
        underMouse() ? QColor(22, 38, 54, 242)
                     : QColor(15, 27, 41, 228));

    painter.save();
    painter.setClipPath(clip);
    const qreal liquidTop = bounds.bottom() - bounds.height() * progress_;
    QPainterPath liquid;
    liquid.moveTo(bounds.left(), bounds.bottom());
    liquid.lineTo(bounds.left(), liquidTop);
    for (qreal x = bounds.left(); x <= bounds.right() + 1.0; x += 1.5) {
        const qreal wave =
            std::sin((x - bounds.left()) * 0.34 + wavePhase_) * 1.35;
        liquid.lineTo(x, liquidTop + wave);
    }
    liquid.lineTo(bounds.right(), bounds.bottom());
    liquid.closeSubpath();
    QLinearGradient water(bounds.topLeft(), bounds.bottomLeft());
    water.setColorAt(0.0, QColor(92, 238, 174, 235));
    water.setColorAt(1.0, QColor(20, 155, 112, 245));
    painter.fillPath(liquid, water);
    painter.restore();

    painter.setPen(QPen(
        underMouse() ? QColor(112, 240, 180, 205)
                     : QColor(77, 201, 148, 120),
        1.1));
    painter.drawPath(clip);
    const QColor glyph =
        progress_ > 0.43 ? QColor(246, 255, 251) : QColor(105, 229, 174);
    painter.setPen(QPen(
        glyph, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    const qreal centerX = bounds.center().x();
    painter.drawLine(QPointF(centerX, 7.0), QPointF(centerX, 17.0));
    painter.drawLine(
        QPointF(centerX - 3.5, 13.5), QPointF(centerX, 17.0));
    painter.drawLine(
        QPointF(centerX + 3.5, 13.5), QPointF(centerX, 17.0));
    painter.drawLine(
        QPointF(centerX - 5.0, 20.5), QPointF(centerX + 5.0, 20.5));
}

}  // namespace remote::controller
