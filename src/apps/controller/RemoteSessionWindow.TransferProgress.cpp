// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RemoteSessionWindow.h"

#include <Windows.h>

#include <algorithm>
#include <cmath>
#include <cstdint>

#include <QAbstractAnimation>
#include <QApplication>
#include <QDialog>
#include <QEasingCurve>
#include <QGuiApplication>
#include <QLabel>
#include <QLayout>
#include <QScreen>
#include <QTimer>
#include <QVariantAnimation>
#include <QWindow>
#include <QWidget>

#include "RemoteCDialog.h"
#include "RemoteTransferStatusButton.h"

namespace remote::controller {

    void RemoteSessionWindow::ShowRemotePasteProgress(
        const QString& transferId, const QString& title,
        const QString& message, double progress,
        std::uintptr_t localTargetWindow)
    {
        if (transferId.isEmpty()) return;
        const HWND localTarget = reinterpret_cast<HWND>(localTargetWindow);
        const bool localTargetValid = localTarget && IsWindow(localTarget);
        if (remotePasteTransferId_ != transferId) {
            if (remotePasteAnimation_) {
                remotePasteAnimation_->stop();
                remotePasteAnimation_->deleteLater();
                remotePasteAnimation_ = nullptr;
            }
            if (remotePasteAnimationOverlay_) {
                remotePasteAnimationOverlay_->deleteLater();
                remotePasteAnimationOverlay_ = nullptr;
            }
            if (remotePasteDialog_) {
                // close() runs the native dialog-finished/owner activation
                // path. Hide and retire the non-activating popup directly so
                // Explorer remains the foreground window.
                auto* oldDialog = remotePasteDialog_;
                remotePasteDialog_ = nullptr;
                oldDialog->hide();
                oldDialog->deleteLater();
            }
            remotePasteTransferId_ = transferId;
            remotePastePromptDismissed_ = false;
            remotePasteMinimized_ = false;
            remotePasteRestoreAnimating_ = false;
            remotePasteRestoreGeometry_ = {};
            remotePasteDialogSnapshot_ = {};
            SetRemotePasteStatusButtonVisible(false);
        }
        if (remotePastePromptDismissed_) return;
        if (!remotePasteDialog_) {
            auto* dialog = RemoteCDialog::CreateStatus(
                this,
                title,
                message,
                QStringLiteral("后台传输"),
                RemoteCDialog::Tone::kInformation);
            remotePasteDialog_ = dialog;
            if (localTargetValid) {
                dialog->SetNonActivatingWindow(true);
            }
            dialog->SetStatusActionHandler(
                [this] { MinimizeRemotePasteProgress(); });
            connect(dialog, &QDialog::finished, this,
                    [this, dialog](int) {
                        if (remotePasteDialog_ != dialog) return;
                        if (!remotePasteDialogClosing_) {
                            remotePastePromptDismissed_ = true;
                            if (remotePasteCancelHandler_) {
                                remotePasteCancelHandler_();
                            }
                        }
                        remotePasteDialog_ = nullptr;
                        remotePasteMinimized_ = false;
                        remotePasteRestoreAnimating_ = false;
                        remotePasteRestoreGeometry_ = {};
                        remotePasteDialogSnapshot_ = {};
                        SetRemotePasteStatusButtonVisible(false);
                        dialog->deleteLater();
                    });
        }
        remotePasteDialog_->SetProgress(progress);
        remotePasteDialog_->SetContent(
            title, message,
            QStringLiteral("后台传输"),
            RemoteCDialog::Tone::kInformation);
        if (remotePasteStatusButton_) {
            static_cast<RemoteTransferStatusButton*>(remotePasteStatusButton_)
                ->SetProgress(progress);
            remotePasteStatusButton_->setToolTip(
                QStringLiteral("%1\n%2").arg(title, message));
        }
        if (remotePasteMinimized_ || remotePasteRestoreAnimating_) return;
        const bool firstShow = !remotePasteDialog_->isVisible();
        if (firstShow) {
            remotePasteDialog_->show();
            if (localTargetValid) {
                RECT nativeRect{};
                if (GetWindowRect(localTarget, &nativeRect)) {
                    const QSize dialogSize = remotePasteDialog_->size();
                    int x = nativeRect.left +
                        ((nativeRect.right - nativeRect.left) -
                         dialogSize.width()) / 2;
                    int y = nativeRect.top + 28;
                    if (QScreen* screen = QGuiApplication::screenAt(
                            QPoint((nativeRect.left + nativeRect.right) / 2,
                                   (nativeRect.top + nativeRect.bottom) / 2))) {
                        const QRect available = screen->availableGeometry();
                        x = std::clamp(x, available.left(),
                            (std::max)(available.left(),
                                available.right() - dialogSize.width() + 1));
                        y = std::clamp(y, available.top(),
                            (std::max)(available.top(),
                                available.bottom() - dialogSize.height() + 1));
                    }
                    remotePasteDialog_->move(x, y);
                    SetWindowPos(
                        reinterpret_cast<HWND>(remotePasteDialog_->winId()),
                        HWND_TOPMOST, x, y, 0, 0,
                        SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
                }
            } else {
                remotePasteDialog_->raise();
            }
        }
    }

    void RemoteSessionWindow::CompleteRemotePasteProgress(
        const QString& transferId)
    {
        if (transferId.isEmpty() ||
            transferId != remotePasteTransferId_) return;
        if (!remotePasteDialog_ || remotePasteMinimized_ ||
            !remotePasteDialog_->isVisible()) {
            CloseRemotePasteProgress(transferId);
            return;
        }

        // The worker publishes the final active snapshot and the following
        // ready snapshot back-to-back. The UI intentionally coalesces worker
        // notifications, so explicitly paint the completed value for one
        // frame before retiring the popup.
        remotePasteDialog_->SetProgress(1.0);
        if (remotePasteStatusButton_) {
            static_cast<RemoteTransferStatusButton*>(remotePasteStatusButton_)
                ->SetProgress(1.0);
        }
        QTimer::singleShot(120, this, [this, transferId] {
            if (remotePasteTransferId_ == transferId) {
                CloseRemotePasteProgress(transferId);
            }
        });
    }

    void RemoteSessionWindow::CloseRemotePasteProgress(
        const QString& transferId)
    {
        if (!transferId.isEmpty() &&
            transferId != remotePasteTransferId_) return;
        if (remotePasteAnimation_) {
            remotePasteAnimation_->stop();
            remotePasteAnimation_->deleteLater();
            remotePasteAnimation_ = nullptr;
        }
        if (remotePasteAnimationOverlay_) {
            remotePasteAnimationOverlay_->deleteLater();
            remotePasteAnimationOverlay_ = nullptr;
        }
        if (remotePasteDialog_) {
            auto* finishedDialog = remotePasteDialog_;
            remotePasteDialog_ = nullptr;
            finishedDialog->hide();
            finishedDialog->deleteLater();
        }
        remotePasteTransferId_.clear();
        remotePastePromptDismissed_ = false;
        remotePasteMinimized_ = false;
        remotePasteRestoreAnimating_ = false;
        remotePasteRestoreGeometry_ = {};
        remotePasteDialogSnapshot_ = {};
        SetRemotePasteStatusButtonVisible(false);
    }

    void RemoteSessionWindow::ShowRemotePasteFailure(
        const QString& transferId, const QString& message)
    {
        if (!transferId.isEmpty()) {
            remotePasteTransferId_ = transferId;
        }
        if (remotePasteAnimation_) {
            remotePasteAnimation_->stop();
            remotePasteAnimation_->deleteLater();
            remotePasteAnimation_ = nullptr;
        }
        if (remotePasteAnimationOverlay_) {
            remotePasteAnimationOverlay_->deleteLater();
            remotePasteAnimationOverlay_ = nullptr;
        }
        remotePastePromptDismissed_ = false;
        remotePasteMinimized_ = false;
        remotePasteRestoreAnimating_ = false;
        SetRemotePasteStatusButtonVisible(false);
        if (!remotePasteDialog_) {
            auto* dialog = RemoteCDialog::CreateStatus(
                this,
                QStringLiteral("远程粘贴未完成"),
                message,
                QStringLiteral("知道了"),
                RemoteCDialog::Tone::kDanger);
            remotePasteDialog_ = dialog;
            connect(dialog, &QDialog::finished, this,
                    [this, dialog](int) {
                        if (remotePasteDialog_ != dialog) return;
                        remotePasteDialog_ = nullptr;
                        remotePasteTransferId_.clear();
                        remotePastePromptDismissed_ = false;
                        remotePasteRestoreGeometry_ = {};
                        remotePasteDialogSnapshot_ = {};
                        SetRemotePasteStatusButtonVisible(false);
                        dialog->deleteLater();
                    });
        }
        remotePasteDialog_->SetStatusActionHandler({});
        remotePasteDialog_->SetProgress(-1.0);
        remotePasteDialog_->SetContent(
            QStringLiteral("远程粘贴未完成"), message,
            QStringLiteral("知道了"),
            RemoteCDialog::Tone::kDanger);
        if (!remotePasteDialog_->isVisible()) {
            QApplication::beep();
            remotePasteDialog_->show();
        }
        remotePasteDialog_->raise();
    }

    void RemoteSessionWindow::SetRemotePasteStatusButtonVisible(bool visible)
    {
        if (!remotePasteStatusButton_ || !remotePasteStatusHost_ ||
            !sessionHud_) return;
        remotePasteStatusHost_->setVisible(visible);
        sessionHud_->setFixedHeight(visible ? 166 : 128);
        sessionHud_->updateGeometry();
        if (isMinimized()) {
            // mapToGlobal() is not meaningful while the native owner is
            // iconic; changeEvent() performs the layout after restoration.
            return;
        }
        LayoutSessionOverlays();
    }

    QRect RemoteSessionWindow::RemotePasteDialogRestoreGeometry() const
    {
        if (remotePasteRestoreGeometry_.isValid()) {
            return remotePasteRestoreGeometry_;
        }
        if (!remotePasteDialog_) return {};
        const QRect frame = frameGeometry();
        const QSize size = remotePasteDialog_->size();
        return QRect(
            frame.center().x() - size.width() / 2,
            frame.center().y() - size.height() / 2,
            size.width(), size.height());
    }

    QLabel* RemoteSessionWindow::CreateRemotePasteAnimationOverlay(
        const QPixmap& snapshot, const QRect& geometry, qreal opacity)
    {
        // The progress dialog may be displayed above Explorer or another local
        // application.  Keeping the animation owned by RemoteSessionWindow
        // causes Windows to raise the monitoring window before the animation
        // starts.  Use an ownerless, non-activating top-level overlay instead.
        auto* overlay = new QLabel(
            nullptr, Qt::Tool | Qt::FramelessWindowHint |
                Qt::WindowStaysOnTopHint |
                Qt::NoDropShadowWindowHint |
                Qt::WindowTransparentForInput |
                Qt::WindowDoesNotAcceptFocus);
        overlay->setAttribute(Qt::WA_TranslucentBackground, true);
        overlay->setAttribute(Qt::WA_ShowWithoutActivating, true);
        overlay->setScaledContents(true);
        overlay->setPixmap(snapshot);
        overlay->setGeometry(geometry);
        overlay->setWindowOpacity(opacity);

        const HWND overlayHwnd = reinterpret_cast<HWND>(overlay->winId());
        if (overlay->windowHandle()) {
            overlay->windowHandle()->setTransientParent(nullptr);
        }
        if (overlayHwnd) {
            LONG_PTR extendedStyle =
                ::GetWindowLongPtrW(overlayHwnd, GWL_EXSTYLE);
            extendedStyle |= WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW |
                WS_EX_TRANSPARENT;
            ::SetWindowLongPtrW(
                overlayHwnd, GWL_EXSTYLE, extendedStyle);
            ::SetWindowLongPtrW(overlayHwnd, GWLP_HWNDPARENT, 0);
            // winId() creates the native tool window. Position that hidden
            // HWND before Qt makes it visible; otherwise DWM can compose one
            // frame at the default top-left coordinate.
            ::SetWindowPos(
                overlayHwnd, HWND_TOPMOST,
                geometry.x(), geometry.y(),
                geometry.width(), geometry.height(),
                SWP_NOACTIVATE | SWP_NOOWNERZORDER);
        }

        overlay->show();
        if (overlayHwnd) {
            ::SetWindowPos(
                overlayHwnd, HWND_TOPMOST,
                geometry.x(), geometry.y(),
                geometry.width(), geometry.height(),
                SWP_NOACTIVATE | SWP_SHOWWINDOW);
        }
        return overlay;
    }

    void RemoteSessionWindow::MinimizeRemotePasteProgress()
    {
        if (!remotePasteDialog_ || remotePasteMinimized_) return;
        if (remotePasteAnimation_) {
            remotePasteAnimation_->stop();
            remotePasteAnimation_->deleteLater();
            remotePasteAnimation_ = nullptr;
        }
        if (remotePasteAnimationOverlay_) {
            remotePasteAnimationOverlay_->deleteLater();
            remotePasteAnimationOverlay_ = nullptr;
        }
        // This is a top-level, frameless dialog. Keep its global frame
        // coordinate for the ownerless native animation overlay.
        remotePasteRestoreGeometry_ = remotePasteDialog_->frameGeometry();
        remotePasteDialogSnapshot_ = remotePasteDialog_->grab();
        remotePasteMinimized_ = true;
        remotePasteRestoreAnimating_ = false;
        SetRemotePasteStatusButtonVisible(true);

        const HWND sessionWindow = reinterpret_cast<HWND>(winId());
        const bool sessionWindowMinimized = isMinimized() ||
            (sessionWindow && ::IsIconic(sessionWindow));
        if (sessionWindowMinimized) {
            // There is no visible destination button while the monitoring
            // window is minimized. Hide immediately instead of flashing an
            // animation toward an off-screen native window coordinate.
            remotePasteDialog_->hide();
            remotePasteDialog_->setWindowOpacity(1.0);
            remotePasteDialog_->setGeometry(remotePasteRestoreGeometry_);
            return;
        }

        const QRect buttonRect(
            remotePasteStatusButton_->mapToGlobal(QPoint(0, 0)),
            remotePasteStatusButton_->size());
        const QRect target = buttonRect;
        if (CurrentUiAnimationLevel() <= 0) {
            remotePasteDialog_->hide();
            remotePasteDialog_->setWindowOpacity(1.0);
            remotePasteDialog_->setGeometry(remotePasteRestoreGeometry_);
            return;
        }

        auto* overlay = CreateRemotePasteAnimationOverlay(
            remotePasteDialogSnapshot_, remotePasteRestoreGeometry_, 0.0);
        remotePasteAnimationOverlay_ = overlay;
        remotePasteDialog_->hide();
        remotePasteDialog_->setWindowOpacity(1.0);

        auto* animation = new QVariantAnimation(this);
        remotePasteAnimation_ = animation;
        animation->setDuration(560);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        QEasingCurve curve(QEasingCurve::BezierSpline);
        curve.addCubicBezierSegment(
            QPointF(0.20, 0.58), QPointF(0.18, 1.0), QPointF(1.0, 1.0));
        animation->setEasingCurve(curve);
        const QRect start = remotePasteRestoreGeometry_;
        const QPointF startCenter = start.center();
        const QPointF endCenter = target.center();
        const QPointF control(
            endCenter.x(), startCenter.y() +
                (endCenter.y() - startCenter.y()) * 0.22);
        connect(animation, &QVariantAnimation::valueChanged, overlay,
                [overlay, start, target, startCenter, endCenter, control]
                (const QVariant& value) {
                    const qreal t = value.toReal();
                    const qreal inverse = 1.0 - t;
                    const QPointF center = inverse * inverse * startCenter +
                        2.0 * inverse * t * control + t * t * endCenter;
                    const qreal sizeT = std::pow(t, 1.18);
                    qreal width = start.width() +
                        (target.width() - start.width()) * sizeT;
                    qreal height = start.height() +
                        (target.height() - start.height()) *
                            std::pow(t, 0.96);
                    if (t < 0.16) {
                        const qreal settle = std::sin(
                            t / 0.16 * 3.14159265358979323846) * 0.018;
                        width *= 1.0 - settle;
                        height *= 1.0 - settle;
                    }
                    overlay->setGeometry(QRect(
                        qRound(center.x() - width / 2.0),
                        qRound(center.y() - height / 2.0),
                        (std::max)(1, qRound(width)),
                        (std::max)(1, qRound(height))));
                    const qreal opacity = t <= 0.84
                        ? 1.0 : (1.0 - (t - 0.84) / 0.16);
                    overlay->setWindowOpacity(std::clamp(opacity, 0.0, 1.0));
                });
        QPointer<RemoteCDialog> dialog = remotePasteDialog_;
        connect(animation, &QVariantAnimation::finished, this,
                [this, animation, overlay, dialog] {
                    if (remotePasteAnimation_ == animation) {
                        remotePasteAnimation_ = nullptr;
                    }
                    if (remotePasteAnimationOverlay_ == overlay) {
                        remotePasteAnimationOverlay_ = nullptr;
                    }
                    overlay->deleteLater();
                    if (dialog && remotePasteMinimized_) {
                        dialog->hide();
                        dialog->setWindowOpacity(1.0);
                        dialog->setGeometry(remotePasteRestoreGeometry_);
                    }
                });
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void RemoteSessionWindow::RestoreRemotePasteProgress()
    {
        if (!remotePasteDialog_ || !remotePasteMinimized_) return;
        if (remotePasteAnimation_) {
            remotePasteAnimation_->stop();
            remotePasteAnimation_->deleteLater();
            remotePasteAnimation_ = nullptr;
        }
        if (remotePasteAnimationOverlay_) {
            remotePasteAnimationOverlay_->deleteLater();
            remotePasteAnimationOverlay_ = nullptr;
        }
        const QRect buttonRect(
            remotePasteStatusButton_->mapToGlobal(QPoint(0, 0)),
            remotePasteStatusButton_->size());
        const QRect start = buttonRect;
        const QRect target = RemotePasteDialogRestoreGeometry();
        remotePasteMinimized_ = false;
        remotePasteRestoreAnimating_ =
            CurrentUiAnimationLevel() > 0;
        SetRemotePasteStatusButtonVisible(false);

        if (CurrentUiAnimationLevel() <= 0) {
            remotePasteRestoreAnimating_ = false;
            remotePasteDialog_->setGeometry(target);
            remotePasteDialog_->setWindowOpacity(1.0);
            remotePasteDialog_->show();
            remotePasteDialog_->raise();
            return;
        }

        if (remotePasteDialog_->layout()) {
            remotePasteDialog_->layout()->activate();
        }
        remotePasteDialogSnapshot_ = remotePasteDialog_->grab();
        const QPixmap snapshot = remotePasteDialogSnapshot_.isNull()
            ? remotePasteDialog_->grab() : remotePasteDialogSnapshot_;
        auto* overlay = CreateRemotePasteAnimationOverlay(
            snapshot, start, 0.0);
        remotePasteAnimationOverlay_ = overlay;

        auto* animation = new QVariantAnimation(this);
        remotePasteAnimation_ = animation;
        animation->setDuration(470);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        QEasingCurve curve(QEasingCurve::BezierSpline);
        curve.addCubicBezierSegment(
            QPointF(0.18, 0.72), QPointF(0.20, 1.0), QPointF(1.0, 1.0));
        animation->setEasingCurve(curve);
        const QPointF startCenter = start.center();
        const QPointF endCenter = target.center();
        const QPointF control(
            startCenter.x(), endCenter.y() +
                (startCenter.y() - endCenter.y()) * 0.22);
        connect(animation, &QVariantAnimation::valueChanged, overlay,
                [overlay, start, target, startCenter, endCenter, control]
                (const QVariant& value) {
                    const qreal t = value.toReal();
                    const qreal inverse = 1.0 - t;
                    const QPointF center = inverse * inverse * startCenter +
                        2.0 * inverse * t * control + t * t * endCenter;
                    const qreal sizeT = std::pow(t, 0.78);
                    const qreal width = start.width() +
                        (target.width() - start.width()) * sizeT;
                    const qreal height = start.height() +
                        (target.height() - start.height()) *
                            std::pow(t, 0.86);
                    overlay->setGeometry(QRect(
                        qRound(center.x() - width / 2.0),
                        qRound(center.y() - height / 2.0),
                        (std::max)(1, qRound(width)),
                        (std::max)(1, qRound(height))));
                    overlay->setWindowOpacity(
                        std::clamp(0.15 + t * 0.85, 0.0, 1.0));
                });
        QPointer<RemoteCDialog> dialog = remotePasteDialog_;
        connect(animation, &QVariantAnimation::finished, this,
                [this, animation, overlay, dialog, target] {
                    if (remotePasteAnimation_ == animation) {
                        remotePasteAnimation_ = nullptr;
                    }
                    if (dialog) {
                        dialog->setGeometry(target);
                        dialog->setWindowOpacity(1.0);
                        dialog->show();
                        dialog->raise();
                    }
                    QPointer<QWidget> overlayGuard = overlay;
                    QTimer::singleShot(0, this,
                        [this, overlayGuard, dialog] {
                            remotePasteRestoreAnimating_ = false;
                            if (overlayGuard) {
                                if (remotePasteAnimationOverlay_ ==
                                    overlayGuard.data()) {
                                    remotePasteAnimationOverlay_ = nullptr;
                                }
                                overlayGuard->hide();
                                overlayGuard->deleteLater();
                            }
                            if (dialog) dialog->raise();
                        });
                });
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }

}  // namespace remote::controller
