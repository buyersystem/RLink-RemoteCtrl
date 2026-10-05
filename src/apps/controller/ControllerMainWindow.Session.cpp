// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ControllerMainWindow.h"
#include "ControllerMainWindowSupport.h"

#include <QApplication>
#include <QLabel>
#include <QScreen>
#include <QTimer>

#include <algorithm>
#include <filesystem>
#include <utility>
#include <vector>

#include "CameraWindow.h"
#include "FileTransferWindow.h"
#include "RemoteCToast.h"
#include "RemoteSessionWindow.h"
#include "pages/DirectConnectPage.h"
#include "pages/RoomPage.h"
#include "src/apps/remote/ClipboardController.h"

namespace remote::controller {
using namespace detail;

void ControllerMainWindow::StartSession(const QString& deviceId,
                                        const QString& deviceName,
                                        SessionPurpose purpose)
{
    pendingDeviceName_ = deviceName;
    DirectSessionConnectRequest request;
    request.targetDeviceId = deviceId.toStdString();
    request.purpose = purpose;
    request.authorization = DirectAuthorizationMethod::kManualApproval;
    const auto result = engine_->ConnectDirectDevice(request);
    if (!result.accepted) {
        directConnectPage_->SetAssistHint(
            QStringLiteral("无法发起会话：%1")
                .arg(QString::fromStdString(result.errorMessage)),
            DirectConnectPage::AssistHintTone::kError);
    } else {
        directConnectPage_->SetAssistHint(
            QStringLiteral("请求已发送，正在等待对方确认…"),
            DirectConnectPage::AssistHintTone::kWarning);
    }
}

void ControllerMainWindow::StartAssistedSession(
    const QString& deviceId,
    const QString& verificationCode)
{
    pendingDeviceName_ = QStringLiteral("远程协助设备");
    assistedSessionPending_ = true;
    assistedSessionActive_ = false;
    assistedSessionCancellationPending_ = false;
    assistedSessionTimedOut_ = false;
    lastDirectSessionToastError_.clear();
    const auto beforeConnect = engine_->Snapshot();
    DirectSessionConnectRequest request;
    request.targetDeviceId = deviceId.toStdString();
    request.purpose = SessionPurpose::kRemoteControl;
    request.authorization = DirectAuthorizationMethod::kVerificationCode;
    request.verificationCode = verificationCode.toStdString();
    const auto result = engine_->ConnectDirectDevice(request);
    if (!result.accepted) {
        assistedSessionPending_ = false;
        const QString errorText = LocalizedDirectSessionError(
            result.errorCode, result.errorMessage, &beforeConnect);
        directConnectPage_->SetAssistHint(
            errorText, DirectConnectPage::AssistHintTone::kError);
        RemoteCToast::Show(this, errorText, RemoteCToast::Tone::kError);
        return;
    }
    directConnectPage_->SetAssistHint(
        QStringLiteral("正在校验验证码并建立远程桌面…"),
        DirectConnectPage::AssistHintTone::kWarning);
    directConnectPage_->SetActionState(
        true, QStringLiteral("正在连接…（点击取消）"));
    assistedSessionTimeoutTimer_->start();
}
void ControllerMainWindow::HandleRemoteSessionDisconnect()
{
    if (!engine_) {
        return;
    }
    const auto snapshot = engine_->Snapshot();
    const bool remoteRoomShare =
        snapshot.room.membership == RoomMembershipState::kActive &&
        snapshot.room.screenShareState ==
            RoomScreenShareState::kActive &&
        !snapshot.room.screenSharerDeviceId.empty() &&
        snapshot.room.screenSharerDeviceId != snapshot.localDeviceId;
    if (!remoteRoomShare) {
        if (remoteSessionBinding_ && remoteSessionBinding_->IsDirect() &&
            !snapshot.sessionId.empty()) {
            dismissedDirectSessionId_ =
                QString::fromStdString(snapshot.sessionId);
        }
        if (snapshot.state == SessionEngineState::kActive ||
            snapshot.state == SessionEngineState::kConnecting) {
            (void)engine_->Disconnect();
        }
        return;
    }

    dismissedRemoteScreenSharerDeviceId_ =
        QString::fromStdString(
            snapshot.room.screenSharerDeviceId);
    dismissedRemoteScreenShareEpoch_ =
        snapshot.room.screenShareEpoch;
    const auto result =
        engine_->RequestRemoteRoomScreenShareStop(
            snapshot.room.screenSharerDeviceId,
            snapshot.room.screenShareEpoch);
    if (!result.accepted) {
        RemoteCToast::Show(
            this,
            QStringLiteral(
                "监控窗口已关闭，但未能停止对方共享：%1")
                .arg(QString::fromStdString(result.errorMessage)),
            RemoteCToast::Tone::kError);
        return;
    }
    RemoteCToast::Show(
        this, QStringLiteral("已断开监控并通知对方停止共享"),
        RemoteCToast::Tone::kSuccess);
}

void ControllerMainWindow::SetRoomActionHint(const QString& text, bool error)
{
    const bool changed = RoomControls().actionHint->text() != text;
    RoomControls().actionHint->setText(text);
    const QString style = error ? QStringLiteral("color:#b4232f;")
                                : QStringLiteral("color:#9a6513;");
    if (RoomControls().actionHint->styleSheet() != style) {
        RoomControls().actionHint->setStyleSheet(style);
    }
    if (changed) {
        AnimateSmallUiChange(RoomControls().actionHint);
    }
}

void ControllerMainWindow::OpenCameraWindow(const QString& deviceId,
                                            const QString& deviceName)
{
    if (cameraWindow_) {
        cameraWindow_->showNormal();
        cameraWindow_->raise();
        cameraWindow_->activateWindow();
        return;
    }

    cameraWindow_ = new CameraWindow(deviceId, deviceName, {}, nullptr);
    cameraWindow_->setAttribute(Qt::WA_DeleteOnClose);
    cameraWindow_->show();
    cameraWindow_->AnimateWindowEntrance(QPoint(8, 0));
    cameraWindow_->raise();
    cameraWindow_->activateWindow();
}

void ControllerMainWindow::OpenRemoteSession(
    RemoteSessionBinding binding)
{
    if (!binding.IsValid()) {
        return;
    }
    const auto centerRemoteSessionWindow = [this] {
        if (!remoteSessionWindow_) {
            return;
        }

        QScreen* targetScreen = QApplication::screenAt(
            frameGeometry().center());
        if (!targetScreen) {
            targetScreen = QApplication::primaryScreen();
        }
        if (!targetScreen) {
            return;
        }

        const QRect available = targetScreen->availableGeometry();
        const QSize windowSize = remoteSessionWindow_->size();
        const int x = std::clamp(
            available.center().x() - windowSize.width() / 2,
            available.left(),
            std::max(available.left(),
                     available.right() - windowSize.width() + 1));
        const int y = std::clamp(
            available.center().y() - windowSize.height() / 2,
            available.top(),
            std::max(available.top(),
                     available.bottom() - windowSize.height() + 1));
        remoteSessionWindow_->move(x, y);
    };

    if (remoteSessionWindow_) {
        remoteSessionWindow_->BindSessionVideo(
            engine_.get(), sessionMedia_, binding);
        remoteSessionBinding_ = binding;
        if (!remoteSessionWindow_->isVisible()) {
            centerRemoteSessionWindow();
            if (CurrentUiAnimationLevel() > 0) {
                remoteSessionWindow_->setWindowOpacity(0.0);
            }
            remoteSessionWindow_->show();
            remoteSessionWindow_->AnimateWindowEntrance(QPoint(8, 0));
        }
        return;
    }

    directConnectPage_->SetActionState(
        false, QStringLiteral("正在打开…"));
    remoteSessionWindow_ = new RemoteSessionWindow(
        binding, engine_.get(), sessionMedia_, nullptr);
    if (clipboardController_) {
        clipboardController_->SetRemotePastePassthroughWindow(
            static_cast<std::uintptr_t>(remoteSessionWindow_->winId()));
    }
    remoteSessionWindow_->SetDisconnectHandler(
        [this] { HandleRemoteSessionDisconnect(); });
    remoteSessionWindow_->SetRemotePasteHandler(
        [this](const QStringList& localFiles, bool keyboardPaste) {
            if (!clipboardController_) return false;
            if (keyboardPaste &&
                clipboardController_->ShouldPassThroughRemotePaste()) {
                // The local clipboard currently contains the item that was
                // copied on the controlled machine. Let Ctrl+V reach that
                // machine unchanged so it can paste its own clipboard item.
                return false;
            }
            const auto before = clipboardController_->Snapshot();
            const bool transferBusy = before.transferActive ||
                before.state == "capturing" ||
                before.state == "offering" ||
                before.state == "sending" ||
                before.state == "applying" ||
                before.state == "receiving";
            if (transferBusy) {
                // A paste is already in flight. Do not replace it, queue a
                // newer item, or distract the user with an error dialog.
                return true;
            }
            bool accepted = false;
            if (localFiles.isEmpty()) {
                accepted = clipboardController_
                    ->RequestPasteFromClipboard();
            } else {
                std::vector<std::filesystem::path> paths;
                paths.reserve(localFiles.size());
                for (const QString& file : localFiles) {
                    paths.emplace_back(file.toStdWString());
                }
                accepted = clipboardController_
                    ->RequestPasteFiles(std::move(paths));
            }
            if (!accepted) {
                // If all published preconditions were ready, rejection here
                // means another Ctrl+V/drop won the atomic preparation slot.
                if (before.state == "ready") return true;
                const auto clipboard = clipboardController_->Snapshot();
                QString unavailableMessage;
                if (!clipboard.enabled) {
                    unavailableMessage = QStringLiteral(
                        "远程粘贴已关闭，请先在设置中开启");
                } else if (!clipboard.sessionActive) {
                    unavailableMessage = QStringLiteral(
                        "控制权限正在同步，请稍后再试");
                } else if (!clipboard.peerCapabilitiesSeen) {
                    unavailableMessage = QStringLiteral(
                        "正在建立远程粘贴通道，请稍后再试");
                } else if (!clipboard.peerEnabled) {
                    unavailableMessage = QStringLiteral(
                        "对方未允许本次控制会话使用远程粘贴");
                } else {
                    unavailableMessage = QStringLiteral(
                        "远程粘贴正在准备，请稍后再试");
                }
                remoteSessionWindow_->ShowRemotePasteFailure(
                    {}, unavailableMessage);
            }
            // A failed RemoteC preparation is still consumed after showing
            // the reason. Only the explicit remote-origin case above is
            // allowed to fall through as a native Ctrl+V.
            return true;
        });
    remoteSessionWindow_->SetRemotePasteCancelHandler([this] {
        if (clipboardController_) {
            (void)clipboardController_->CancelActiveTransfer();
        }
    });
    remoteSessionWindow_->SetFileTransferHandlers(
        [this] {
            if (!fileTransferWindow_ || !remoteSessionWindow_) return;
            fileTransferWindow_->AttachAsDrawer(remoteSessionWindow_);
            fileTransferWindow_->OpenBesideMainWindow(
                remoteSessionWindow_->frameGeometry());
        },
        [this] {
            if (!fileTransferWindow_) return;
            fileTransferWindow_->HideImmediately();
            fileTransferWindow_->AttachAsDrawer(this);
        });
    remoteSessionBinding_ = binding;
    remoteSessionWindow_->setAttribute(Qt::WA_DeleteOnClose);
    connect(remoteSessionWindow_, &QObject::destroyed, this, [this] {
        if (clipboardController_) {
            clipboardController_->SetRemotePastePassthroughWindow(0);
        }
        remoteSessionBinding_.reset();
    });
    centerRemoteSessionWindow();
    if (CurrentUiAnimationLevel() > 0) {
        // Prepare the native window as transparent before show(). Calling the
        // entrance animation only after an opaque show paints one full frame,
        // then drops to zero opacity and looks like a close/reopen cycle.
        remoteSessionWindow_->setWindowOpacity(0.0);
    }
    remoteSessionWindow_->show();
    remoteSessionWindow_->AnimateWindowEntrance(QPoint(8, 0));
    remoteSessionWindow_->raise();
    remoteSessionWindow_->activateWindow();
    directConnectPage_->SetActionState(
        true, QStringLiteral("开始连接  →"));
}

void ControllerMainWindow::SetRuntimeStatus(const QString& status,
                                            const QString& color)
{
    directConnectPage_->SetRuntimeStatus(status, color);
}

}  // namespace remote::controller
