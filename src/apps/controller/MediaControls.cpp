// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "MediaControls.h"

#include <QColor>
#include <QEasingCurve>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QTimer>
#include <QToolButton>
#include <QVariantAnimation>

#include <utility>

#include "FramelessWindow.h"
#include "ui/RemoteCTheme.h"
#include "ui/morph/MorphIconButtonBinding.h"

namespace remote::controller::detail {

MediaDeviceButton::MediaDeviceButton(QWidget *parent)
    : QPushButton(parent), arrowAnimation_(new QVariantAnimation(this)) {
  arrowAnimation_->setEasingCurve(QEasingCurve::OutCubic);
  connect(arrowAnimation_, &QVariantAnimation::valueChanged, this,
          [this](const QVariant &value) {
            arrowRotation_ = value.toReal();
            update();
          });
}

void MediaDeviceButton::SetDeviceMenuHandler(std::function<void()> handler) {
  deviceMenuHandler_ = std::move(handler);
  setProperty("deviceMenu", static_cast<bool>(deviceMenuHandler_));
  update();
}

void MediaDeviceButton::mousePressEvent(QMouseEvent *event) {
  if (event && event->button() == Qt::LeftButton && deviceMenuHandler_ &&
      event->position().x() >= width() - kMenuAreaWidth) {
    setDown(false);
    SetDeviceMenuOpen(true);
    deviceMenuHandler_();
    SetDeviceMenuOpen(false);
    event->accept();
    return;
  }
  QPushButton::mousePressEvent(event);
}

void MediaDeviceButton::paintEvent(QPaintEvent *event) {
  QPushButton::paintEvent(event);
  if (!deviceMenuHandler_) {
    return;
  }
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setPen(QPen(isEnabled() ? QColor(QStringLiteral("#9aa6b7"))
                                  : QColor(QStringLiteral("#c5ccd5")),
                      1.2));
  const int dividerX = width() - kMenuAreaWidth;
  QPainterPath arrow;
  const qreal centerX = dividerX + kMenuAreaWidth / 2.0;
  const qreal centerY = height() / 2.0;
  painter.translate(centerX, centerY);
  painter.rotate(arrowRotation_);
  arrow.moveTo(-4, -2);
  arrow.lineTo(0, 2);
  arrow.lineTo(4, -2);
  painter.drawPath(arrow);
}

void MediaDeviceButton::SetDeviceMenuOpen(bool open) {
  const qreal target = open ? 180.0 : 0.0;
  const int level = CurrentUiAnimationLevel();
  arrowAnimation_->stop();
  if (level <= 0) {
    arrowRotation_ = target;
    update();
    return;
  }
  arrowAnimation_->setDuration(level == 1 ? 80 : 135);
  arrowAnimation_->setStartValue(arrowRotation_);
  arrowAnimation_->setEndValue(target);
  arrowAnimation_->start();
}

QIcon CreateMediaStateIcon(MediaStateIcon type, bool active) {
  QString iconName;
  switch (type) {
  case MediaStateIcon::kCamera:
    iconName = active ? QStringLiteral("camera") : QStringLiteral("camera-off");
    break;
  case MediaStateIcon::kMicrophone:
    iconName = active ? QStringLiteral("mic") : QStringLiteral("mic-off");
    break;
  case MediaStateIcon::kSpeaker:
    iconName = active ? QStringLiteral("volume-2") : QStringLiteral("volume-x");
    break;
  case MediaStateIcon::kScreen:
    iconName = active ? QStringLiteral("screen-share")
                      : QStringLiteral("screen-share-off");
    break;
  }

  constexpr int kLogicalSize = 24;
  constexpr qreal kBackingScale = 2.0;
  QPixmap pixmap(kLogicalSize * kBackingScale, kLogicalSize * kBackingScale);
  pixmap.setDevicePixelRatio(kBackingScale);
  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing, true);
  QIcon(QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(iconName))
      .paint(&painter, QRect(0, 0, kLogicalSize, kLogicalSize));
  painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  const bool dark =
      ui::RemoteCTheme::IsDark(ui::RemoteCTheme::LoadPreference());
  painter.fillRect(QRect(0, 0, kLogicalSize, kLogicalSize),
                   QColor(active ? (dark ? QStringLiteral("#4FF0B5")
                                         : QStringLiteral("#168A5B"))
                                 : (dark ? QStringLiteral("#7F8DA3")
                                         : QStringLiteral("#667085"))));
  painter.end();

  QIcon icon;
  icon.addPixmap(pixmap, QIcon::Normal, QIcon::Off);
  icon.addPixmap(pixmap, QIcon::Disabled, QIcon::Off);
  return icon;
}

void SetMediaStateButton(QPushButton *button, MediaStateIcon type, bool active,
                         const QString &toolTip) {
  if (!button) {
    return;
  }
  button->setText({});
  const QString effectiveToolTip =
      button->property("deviceMenu").toBool()
          ? toolTip + QStringLiteral("；点击右侧箭头选择设备")
          : toolTip;
  button->setToolTip(effectiveToolTip);
  button->setAccessibleName(effectiveToolTip);

  const bool dark =
      ui::RemoteCTheme::IsDark(ui::RemoteCTheme::LoadPreference());
  const int iconState = 1 + static_cast<int>(type) * 4 +
      (active ? 2 : 0) + (dark ? 1 : 0);
  // Room snapshots often change unrelated state. Keep the current morph
  // frame instead of repainting/parsing this unchanged icon several times.
  if (button->property("remoteCMediaIconState").toInt() == iconState &&
      button->iconSize() == QSize(25, 25)) {
    return;
  }
  button->setProperty("remoteCMediaIconState", iconState);
  button->setIcon(CreateMediaStateIcon(type, active));
  button->setIconSize(QSize(25, 25));

  QString source;
  QString target;
  switch (type) {
  case MediaStateIcon::kCamera:
    source = QStringLiteral("camera");
    target = QStringLiteral("camera-off");
    break;
  case MediaStateIcon::kMicrophone:
    source = QStringLiteral("mic");
    target = QStringLiteral("mic-off");
    break;
  case MediaStateIcon::kSpeaker:
    source = QStringLiteral("volume-2");
    target = QStringLiteral("volume-x");
    break;
  case MediaStateIcon::kScreen:
    source = QStringLiteral("screen-share");
    target = QStringLiteral("screen-share-off");
    break;
  }
  auto *morph = remotec::ui::morph::MorphIconButtonBinding::attach(
      button, QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(source),
      QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(target),
      remotec::ui::morph::MorphIconButtonBinding::Interaction::State,
      QSize(25, 25),
      QColor(dark ? QStringLiteral("#4FF0B5") : QStringLiteral("#168A5B")),
      QColor(dark ? QStringLiteral("#7F8DA3") : QStringLiteral("#667085")));
  if (morph)
    morph->setTarget(!active);
}

void SetCameraGalleryStateButton(QPushButton *button, bool camerasAvailable,
                                 bool galleryVisible, const QString &toolTip) {
  if (!button)
    return;
  button->setText({});
  button->setToolTip(toolTip);
  button->setAccessibleName(toolTip);
  const bool dark =
      ui::RemoteCTheme::IsDark(ui::RemoteCTheme::LoadPreference());
  const int iconState = 1 + (camerasAvailable ? 4 : 0) +
      (galleryVisible ? 2 : 0) + (dark ? 1 : 0);
  if (button->property("remoteCGalleryIconState").toInt() == iconState &&
      button->iconSize() == QSize(25, 25)) {
    return;
  }
  button->setProperty("remoteCGalleryIconState", iconState);
  button->setIcon(QIcon(QStringLiteral(":/ui/icons/lucide/base/eye-off.svg")));
  button->setIconSize(QSize(25, 25));
  const QColor stateColor(
      camerasAvailable
          ? (dark ? QStringLiteral("#4FF0B5") : QStringLiteral("#168A5B"))
          : (dark ? QStringLiteral("#7F8DA3") : QStringLiteral("#667085")));
  auto *morph = remotec::ui::morph::MorphIconButtonBinding::attach(
      button, QStringLiteral(":/ui/icons/lucide/base/eye-off.svg"),
      QStringLiteral(":/ui/icons/lucide/base/eye.svg"),
      remotec::ui::morph::MorphIconButtonBinding::Interaction::State,
      QSize(25, 25), stateColor, stateColor);
  if (morph)
    morph->setTarget(galleryVisible);
}

QToolButton *CreateMemberMediaIndicator(QWidget *parent, MediaStateIcon type,
                                        bool active, const QString &toolTip,
                                        bool hadPreviousState,
                                        bool previousActive) {
  auto *indicator = new QToolButton(parent);
  indicator->setFixedSize(28, 28);
  indicator->setAutoRaise(true);
  indicator->setFocusPolicy(Qt::NoFocus);
  indicator->setAttribute(Qt::WA_TransparentForMouseEvents);
  indicator->setStyleSheet(QStringLiteral(
      "QToolButton{background:transparent;border:none;padding:0;}"));
  indicator->setIcon(CreateMediaStateIcon(type, active));
  indicator->setIconSize(QSize(21, 21));
  indicator->setToolTip(toolTip);
  indicator->setAccessibleName(toolTip);

  QString source;
  QString target;
  switch (type) {
  case MediaStateIcon::kCamera:
    source = QStringLiteral("camera");
    target = QStringLiteral("camera-off");
    break;
  case MediaStateIcon::kMicrophone:
    source = QStringLiteral("mic");
    target = QStringLiteral("mic-off");
    break;
  case MediaStateIcon::kSpeaker:
    source = QStringLiteral("volume-2");
    target = QStringLiteral("volume-x");
    break;
  case MediaStateIcon::kScreen:
    source = QStringLiteral("screen-share");
    target = QStringLiteral("screen-share-off");
    break;
  }
  const bool dark =
      ui::RemoteCTheme::IsDark(ui::RemoteCTheme::LoadPreference());
  auto *morph = remotec::ui::morph::MorphIconButtonBinding::attach(
      indicator, QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(source),
      QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(target),
      remotec::ui::morph::MorphIconButtonBinding::Interaction::State,
      QSize(21, 21),
      QColor(dark ? QStringLiteral("#4FF0B5") : QStringLiteral("#168A5B")),
      QColor(dark ? QStringLiteral("#7F8DA3") : QStringLiteral("#667085")));
  if (morph) {
    morph->setTarget(!(hadPreviousState ? previousActive : active), false);
    if (hadPreviousState && previousActive != active) {
      QTimer::singleShot(0, indicator,
                         [morph, active] { morph->setTarget(!active, true); });
    }
  }
  return indicator;
}

} // namespace remote::controller::detail
