// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QIcon>
#include <QPushButton>
#include <QString>

#include <functional>

class QMouseEvent;
class QPaintEvent;
class QToolButton;
class QVariantAnimation;

namespace remote::controller::detail {

enum class MediaStateIcon { kCamera, kMicrophone, kSpeaker, kScreen };

class MediaDeviceButton final : public QPushButton {
public:
  explicit MediaDeviceButton(QWidget *parent = nullptr);

  void SetDeviceMenuHandler(std::function<void()> handler);

protected:
  void mousePressEvent(QMouseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;

private:
  void SetDeviceMenuOpen(bool open);

  static constexpr int kMenuAreaWidth = 30;
  std::function<void()> deviceMenuHandler_;
  QVariantAnimation *arrowAnimation_ = nullptr;
  qreal arrowRotation_ = 0.0;
};

QIcon CreateMediaStateIcon(MediaStateIcon type, bool active);
void SetMediaStateButton(QPushButton *button, MediaStateIcon type, bool active,
                         const QString &toolTip);
void SetCameraGalleryStateButton(QPushButton *button, bool camerasAvailable,
                                 bool galleryVisible, const QString &toolTip);
QToolButton *CreateMemberMediaIndicator(QWidget *parent, MediaStateIcon type,
                                        bool active, const QString &toolTip,
                                        bool hadPreviousState = false,
                                        bool previousActive = false);

} // namespace remote::controller::detail
