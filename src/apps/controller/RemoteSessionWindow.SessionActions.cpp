// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RemoteSessionWindow.h"

#include <algorithm>
#include <array>
#include <cstdint>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QDialog>
#include <QLabel>
#include <QMenu>
#include <QTimer>
#include <QToolButton>

#include "RemoteCDialog.h"
#include "RemoteCToast.h"
#include "RemoteDesktopCanvas.h"
#include "RemoteSessionActionTile.h"
#include "RemoteSessionWindowHelpers.h"
#include "src/apps/remote/ISessionMediaAccess.h"

namespace remote::controller {

    void RemoteSessionWindow::HandleDisconnectAction()
    {
        ReleaseRemoteInputs();
        closeWithoutConfirmation_ = true;
        if (disconnectHandler_) {
            disconnectHandler_();
        }
        close();
    }

    void RemoteSessionWindow::closeEvent(QCloseEvent* event)
    {
        if (!event) return;

        bool liveRemoteControl = false;
        if (!closeWithoutConfirmation_ && sessionControl_) {
            const auto snapshot = sessionControl_->Snapshot();
            if (binding_.IsDirect()) {
                liveRemoteControl =
                    snapshot.state == SessionEngineState::kConnecting ||
                    snapshot.state == SessionEngineState::kActive;
            } else if (binding_.IsRoom()) {
                liveRemoteControl =
                    snapshot.room.membership == RoomMembershipState::kActive &&
                    snapshot.room.screenShareState ==
                        RoomScreenShareState::kActive &&
                    !snapshot.room.screenSharerDeviceId.empty() &&
                    snapshot.room.screenSharerDeviceId !=
                        snapshot.localDeviceId;
            }
        }

        if (!liveRemoteControl) {
            FramelessMainWindow::closeEvent(event);
            return;
        }
        if (closeConfirmationVisible_) {
            event->ignore();
            return;
        }

        event->ignore();
        closeConfirmationVisible_ = true;
        const bool confirmed = RemoteCDialog::Confirm(
            this,
            QStringLiteral("结束本次远程控制？"),
            QStringLiteral(
                "关闭监控窗口将断开本次远程控制连接。\n"
                "如果还要继续操作，请选择“继续控制”。"),
            QStringLiteral("结束远控"),
            QStringLiteral("继续控制"),
            RemoteCDialog::Tone::kWarning,
            true);
        closeConfirmationVisible_ = false;
        if (!confirmed) return;

        closeWithoutConfirmation_ = true;
        ReleaseRemoteInputs();
        if (disconnectHandler_) disconnectHandler_();
        event->accept();
        FramelessMainWindow::closeEvent(event);
    }

    void RemoteSessionWindow::UpdateSessionDuration()
    {
        RefreshControlState();
        const qint64 totalSeconds = sessionElapsed_.elapsed() / 1000;
        const qint64 hours = totalSeconds / 3600;
        const qint64 minutes = (totalSeconds % 3600) / 60;
        const qint64 seconds = totalSeconds % 60;
        durationLabel_->setText(
            QStringLiteral("%1:%2:%3")
            .arg(hours, 2, 10, QLatin1Char('0'))
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0')));
    }

    void RemoteSessionWindow::UpdateNetworkRecoveryPrompt(
        bool recovering,
        std::uint32_t attempt,
        bool waitingForSignaling,
        bool failed)
    {
        if (!networkRecoveryDialog_) {
            networkRecoveryDialog_ = RemoteCDialog::CreateStatus(
                this,
                QStringLiteral("正在恢复网络连接"),
                QStringLiteral(
                    "正在检测可用网络并等待 P2P 自恢复…\n当前显示的是断开前最后一帧，远程输入已暂停。"),
                QStringLiteral("隐藏提示"),
                RemoteCDialog::Tone::kWarning);
            connect(networkRecoveryDialog_, &QDialog::finished,
                    this, [this](int result) {
                        if (networkRecoveryShowingFailure_) {
                            networkRecoveryFailureDismissed_ = true;
                            if (result == QDialog::Accepted) {
                                auto* sessionControl = sessionControl_;
                                BindSessionVideo(
                                    nullptr, nullptr, binding_);
                                close();
                                if (sessionControl) {
                                    if (binding_.IsDirect()) {
                                        (void)sessionControl->Disconnect();
                                    } else {
                                        sessionControl->
                                            ExitRoomAfterRecoveryFailure();
                                    }
                                }
                            }
                        } else {
                            networkRecoveryPromptDismissed_ = true;
                        }
                    });
        }

        if (!recovering && !failed) {
            networkRecoveryPromptDismissed_ = false;
            networkRecoveryFailureDismissed_ = false;
            networkRecoveryShowingFailure_ = false;
            const QPoint recoveryDialogCenter =
                networkRecoveryDialog_->frameGeometry().center();
            if (networkRecoveryDialog_->isVisible()) {
                networkRecoveryDialog_->hide();
            }
            if (recoveryPromptWasVisible_) {
                recoveryPromptWasVisible_ = false;
                RemoteCToast::ShowAtGlobalCenter(
                    this,
                    recoveryDialogCenter,
                    QStringLiteral("网络连接已经恢复"),
                    RemoteCToast::Tone::kSuccess,
                    true);
            }
            return;
        }

        if (failed && networkRecoveryFailureDismissed_) {
            return;
        }

        if (failed) {
            if (!networkRecoveryShowingFailure_) {
                networkRecoveryFailureDismissed_ = false;
            }
            networkRecoveryShowingFailure_ = true;
            networkRecoveryDialog_->SetContent(
                QStringLiteral("连接恢复失败"),
                binding_.IsDirect()
                    ? QStringLiteral(
                          "连接已彻底中断。点击确定后将关闭监控窗口并结束当前远程会话。")
                    : QStringLiteral(
                          "连接已彻底中断。点击确定后将关闭监控窗口并退出当前房间。"),
                binding_.IsDirect()
                    ? QStringLiteral("确定并结束")
                    : QStringLiteral("确定并退出"),
                RemoteCDialog::Tone::kDanger);
        } else {
            networkRecoveryShowingFailure_ = false;
            networkRecoveryFailureDismissed_ = false;
            networkRecoveryDialog_->SetContent(
                waitingForSignaling
                    ? QStringLiteral("正在恢复信令连接")
                    : (attempt == 0
                        ? QStringLiteral("正在恢复网络连接")
                        : QStringLiteral("正在重新建立 P2P 连接")),
                waitingForSignaling
                    ? QStringLiteral(
                          "正在重新连接信令服务器…\n连接恢复后将自动重建 P2P，无需关闭远程窗口。")
                    : (attempt == 0
                        ? QStringLiteral(
                              "正在检测可用网络并等待 P2P 自恢复…\n当前显示的是断开前最后一帧，远程输入已暂停。")
                        : QStringLiteral(
                              "正在执行第 %1/3 次 ICE Restart…\n连接稳定后，画面和远程控制将自动继续。")
                              .arg(attempt)),
                QStringLiteral("隐藏提示"),
                RemoteCDialog::Tone::kWarning);
            if (networkRecoveryPromptDismissed_) {
                return;
            }
        }
        if (!networkRecoveryDialog_->isVisible()) {
            QApplication::beep();
            networkRecoveryDialog_->show();
        }
        networkRecoveryDialog_->raise();
        recoveryPromptWasVisible_ = recovering;
    }

    void RemoteSessionWindow::HandleControlAction()
    {
        if (!sessionControl_ || !binding_.IsRoom() || !controlButton_) {
            return;
        }

        const auto snapshot = sessionControl_->Snapshot();
        const bool localControls =
            snapshot.room.activeControllerDeviceId == snapshot.localDeviceId;
        if (localControls) {
            ReleaseRemoteInputs();
        }

        controlButton_->setEnabled(false);
        controlButton_->SetIcon(QString());
        controlButton_->SetText(localControls
            ? QStringLiteral("正在释放…")
            : QStringLiteral("申请已发送…"));
        const auto result = localControls
            ? sessionControl_->ReleaseRoomControl()
            : sessionControl_->RequestRoomControl();
        if (!result.accepted) {
            RefreshControlState();
            controlButton_->setToolTip(
                QStringLiteral("控制操作失败：%1")
                .arg(QString::fromStdString(result.errorMessage)));
        }
    }

    void RemoteSessionWindow::RebuildFrameRateMenu()
    {
        if (!frameRateMenu_ || !frameRateGroup_) {
            return;
        }
        reportedRemoteMaximumFrameRate_ = std::clamp(
            reportedRemoteMaximumFrameRate_,
            kMinimumScreenFrameRate,
            kMaximumScreenFrameRate);
        remoteMaximumFrameRate_ = (std::min)(
            reportedRemoteMaximumFrameRate_, roomMaximumFrameRate_);
        selectedFrameRate_ = (std::min)(
            selectedFrameRate_, remoteMaximumFrameRate_);
        if (desktopCanvas_) {
            static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
                ->SetTargetFrameRate(selectedFrameRate_);
        }
        frameRateMenu_->clear();
        constexpr std::array<std::uint32_t, 8> kFrameRates = {
            15u, 24u, 30u, 45u, 60u, 80u, 100u, 120u};
        for (const std::uint32_t framesPerSecond : kFrameRates) {
            if (framesPerSecond > remoteMaximumFrameRate_) {
                continue;
            }
            QAction* action = frameRateMenu_->addAction(
                QStringLiteral("%1 FPS").arg(framesPerSecond));
            action->setCheckable(true);
            action->setData(static_cast<qulonglong>(framesPerSecond));
            action->setChecked(framesPerSecond == selectedFrameRate_);
            frameRateGroup_->addAction(action);
        }
        if (frameRateButton_) {
            frameRateButton_->setText(
                QStringLiteral("目标帧率\n%1 FPS")
                    .arg(selectedFrameRate_));
            frameRateButton_->setToolTip(
                QStringLiteral(
                    "当前共享端最高支持 %1 FPS；点击选择目标帧率")
                    .arg(remoteMaximumFrameRate_));
        }
    }

    void RemoteSessionWindow::RebuildQualityMenu()
    {
        if (!qualityMenu_ || !qualityGroup_) {
            return;
        }
        qualityMenu_->clear();
        constexpr std::array<ScreenQualityTier, 4> kQualities = {
            ScreenQualityTier::k720p,
            ScreenQualityTier::k1080p,
            ScreenQualityTier::k1440p,
            ScreenQualityTier::kOriginal};
        const bool knownSource =
            remoteSourceWidth_ > 0 && remoteSourceHeight_ > 0;
        for (const ScreenQualityTier quality : kQualities) {
            const auto [width, height] = ScreenQualityBounds(quality);
            const bool presetExceedsSource =
                quality != ScreenQualityTier::kOriginal && knownSource &&
                width >= remoteSourceWidth_ &&
                height >= remoteSourceHeight_ &&
                (width > remoteSourceWidth_ ||
                 height > remoteSourceHeight_);
            if (presetExceedsSource) {
                continue;
            }
            const QString actionText =
                quality == ScreenQualityTier::kOriginal && knownSource
                ? QStringLiteral("原始画质（%1 × %2）")
                      .arg(remoteSourceWidth_)
                      .arg(remoteSourceHeight_)
                : ScreenQualityText(quality);
            QAction* action = qualityMenu_->addAction(actionText);
            action->setCheckable(true);
            action->setData(static_cast<int>(quality));
            action->setChecked(quality == selectedQuality_);
            qualityGroup_->addAction(action);
        }
        if (knownSource) {
            qualityButton_->setToolTip(
                QStringLiteral(
                    "共享端当前显示器为 %1 × %2；不会提供超过源分辨率的档位")
                    .arg(remoteSourceWidth_)
                    .arg(remoteSourceHeight_));
        }
    }

    void RemoteSessionWindow::HandleFrameRateSelection(
        std::uint32_t framesPerSecond)
    {
        if (!sessionControl_ || !binding_.IsValid() || !frameRateButton_) {
            return;
        }
        if (framesPerSecond > remoteMaximumFrameRate_) {
            RemoteCToast::ShowAbove(
                frameRateButton_,
                QStringLiteral("当前共享端的目标帧率上限为 %1 FPS")
                    .arg(remoteMaximumFrameRate_),
                RemoteCToast::Tone::kInformation);
            return;
        }
        const std::uint32_t previous = selectedFrameRate_;
        selectedFrameRate_ = framesPerSecond;
        if (desktopCanvas_) {
            static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
                ->SetTargetFrameRate(selectedFrameRate_);
        }
        if (!RequestStreamPreference()) {
            selectedFrameRate_ = previous;
            if (desktopCanvas_) {
                static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
                    ->SetTargetFrameRate(selectedFrameRate_);
            }
            if (frameRateMenu_) {
                for (QAction* action : frameRateMenu_->actions()) {
                    action->setChecked(
                        action->data().toUInt() == selectedFrameRate_);
                }
            }
            return;
        }
        frameRateButton_->setText(
            QStringLiteral("目标帧率\n%1 FPS").arg(framesPerSecond));
        RemoteCToast::ShowAbove(
            frameRateButton_,
            QStringLiteral("已请求远端切换到 %1 FPS")
            .arg(framesPerSecond),
            RemoteCToast::Tone::kSuccess);
    }

    void RemoteSessionWindow::HandleQualitySelection(ScreenQualityTier quality)
    {
        if (!sessionControl_ || !binding_.IsValid() || !qualityButton_) {
            return;
        }
        const ScreenQualityTier previous = selectedQuality_;
        selectedQuality_ = quality;
        if (!RequestStreamPreference()) {
            selectedQuality_ = previous;
            if (qualityMenu_) {
                for (QAction* action : qualityMenu_->actions()) {
                    action->setChecked(
                        action->data().toInt() ==
                        static_cast<int>(selectedQuality_));
                }
            }
            return;
        }
        const auto [width, height] = ScreenQualityBounds(quality);
        qualityButton_->setText(
            width > 0 && height > 0
            ? QStringLiteral("分辨率\n%1 × %2")
            .arg(width)
            .arg(height)
            : QStringLiteral("分辨率\n原始画质"));
        RemoteCToast::ShowAbove(
            qualityButton_,
            QStringLiteral("已请求远端切换到%1")
            .arg(ScreenQualityText(quality)),
            RemoteCToast::Tone::kSuccess);
    }

    bool RemoteSessionWindow::RequestStreamPreference(bool showError)
    {
        if (!sessionControl_ || !sessionMedia_ || !binding_.IsValid()) {
            return false;
        }
        const auto [width, height] = ScreenQualityBounds(selectedQuality_);
        ScreenStreamPreferenceRequest request;
        request.maxWidth = width;
        request.maxHeight = height;
        request.framesPerSecond = selectedFrameRate_;
        request.quality = selectedQuality_;
        const auto result = binding_.IsDirect()
            ? sessionMedia_->SetDirectScreenStreamPreference(request)
            : sessionControl_->SetRoomScreenStreamPreference(
                  binding_.roomPairId.toStdString(), request);
        if (result.accepted) {
            return true;
        }
        if (showError) {
            QWidget* anchor = qualityButton_
                ? static_cast<QWidget*>(qualityButton_)
                : static_cast<QWidget*>(frameRateButton_);
            RemoteCToast::ShowAbove(
                anchor, QString::fromStdString(result.errorMessage),
                RemoteCToast::Tone::kError);
        }
        return false;
    }

    void RemoteSessionWindow::ToggleRemoteSound()
    {
        if (!sessionControl_ || !sessionMedia_ || !speakerButton_) {
            return;
        }
        const auto snapshot = sessionControl_->Snapshot();
        const auto result = sessionMedia_->SetRemoteAudioPlaybackMuted(
            !snapshot.media.roomAudioPlaybackMuted);
        if (!result.accepted) {
            RemoteCToast::ShowAbove(
                speakerButton_, QString::fromStdString(result.errorMessage),
                RemoteCToast::Tone::kError);
            return;
        }
        RemoteCToast::ShowAbove(
            speakerButton_,
            snapshot.media.roomAudioPlaybackMuted
            ? QStringLiteral("远端声音已开启")
            : QStringLiteral("远端声音已关闭"),
            RemoteCToast::Tone::kSuccess);
        RefreshControlState();
    }

    void RemoteSessionWindow::ToggleLocalMicrophone()
    {
        if (!sessionControl_ || !microphoneButton_) {
            return;
        }
        const auto snapshot = sessionControl_->Snapshot();
        const bool enabled =
            snapshot.media.localMicrophone == LocalMicrophoneState::kPublishing ||
            snapshot.media.localMicrophone == LocalMicrophoneState::kStarting;
        const auto result = sessionControl_->SetLocalMicrophoneEnabled(!enabled);
        if (!result.accepted) {
            RemoteCToast::ShowAbove(
                microphoneButton_, QString::fromStdString(result.errorMessage),
                RemoteCToast::Tone::kError);
            return;
        }
        RemoteCToast::ShowAbove(
            microphoneButton_,
            enabled ? QStringLiteral("正在关闭本机麦克风")
            : QStringLiteral("正在开启本机麦克风"),
            RemoteCToast::Tone::kSuccess);
        RefreshControlState();
    }

}  // namespace remote::controller
