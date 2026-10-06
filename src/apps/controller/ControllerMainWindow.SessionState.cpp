// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ControllerMainWindow.h"
#include "ControlledSessionIndicator.h"
#include "ControllerMainWindowSupport.h"

#include <QDateTime>
#include <QCoreApplication>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QStyle>
#include <QStringList>
#include <QTimer>
#include <QThread>
#include <QVariant>
#include <QMetaObject>
#include <utility>
#include <algorithm>
#include "CameraWindow.h"
#include "RemoteCDialog.h"
#include "RemoteSessionWindow.h"
#include "RemoteCToast.h"
#include "RoomCameraWindow.h"
#include "src/apps/remote/ClipboardController.h"
#include "src/platform/win/WindowsHardwareFingerprint.h"
#include "src/platform/win/WindowsInputExecutor.h"
#include "ScreenFrameRateLogger.h"
#include "pages/DirectConnectPage.h"
#include "pages/DiagnosticsCardsWidget.h"
#include "pages/DiagnosticsPage.h"

namespace remote::controller {
using namespace detail;

void ControllerMainWindow::OnSessionEngineSnapshot(
    const SessionEngineSnapshot& snapshot)
{
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(
            this, [this, snapshot] { OnSessionEngineSnapshot(snapshot); },
            Qt::QueuedConnection);
        return;
    }

    ApplyEngineInitializationState(snapshot);
    UpdateControlledSessionIndicator(snapshot);
    UpdateLocalMediaDevicesUi(snapshot);
    UpdateClipboardSession(snapshot);

    // During the signaling "switching" phase the future sharer's P2P control
    // channel already exists, while capture has not started yet. Send the
    // viewer's persisted final policy now so the sharer creates its first
    // track/encoder at the requested FPS and resolution instead of starting
    // at 30 FPS and being retuned after the monitor window opens.
    const bool canPreflightRemoteScreen =
        snapshot.room.membership == RoomMembershipState::kActive &&
        snapshot.room.screenShareState ==
            RoomScreenShareState::kSwitching &&
        snapshot.room.screenShareEpoch != 0 &&
        !snapshot.room.pendingScreenSharerDeviceId.empty() &&
        snapshot.room.pendingScreenSharerDeviceId !=
            snapshot.localDeviceId;
    if (canPreflightRemoteScreen &&
        preflightScreenPreferenceAttemptEpoch_ !=
            snapshot.room.screenShareEpoch) {
        preflightScreenPreferenceAttemptEpoch_ =
            snapshot.room.screenShareEpoch;
        preflightScreenPreferenceAttempts_ = 0;
        preflightScreenPreferenceScheduled_ = false;
    }
    if (canPreflightRemoteScreen &&
        preflightScreenPreferenceEpoch_ !=
            snapshot.room.screenShareEpoch &&
        preflightScreenPreferenceAttempts_ < 5 &&
        !preflightScreenPreferenceScheduled_) {
        const auto pair = std::find_if(
            snapshot.roomActivity.peerConnections.begin(),
            snapshot.roomActivity.peerConnections.end(),
            [&snapshot](const RoomPeerConnectionSnapshot& current) {
                return current.peerDeviceId ==
                    snapshot.room.pendingScreenSharerDeviceId;
            });
        if (pair != snapshot.roomActivity.peerConnections.end()) {
            preflightScreenPreferenceScheduled_ = true;
            ++preflightScreenPreferenceAttempts_;
            const std::string pairId = pair->pairId;
            const std::uint64_t expectedEpoch =
                snapshot.room.screenShareEpoch;
            QTimer::singleShot(0, this, [this, pairId, expectedEpoch] {
                if (preflightScreenPreferenceAttemptEpoch_ != expectedEpoch) return;
                if (!engine_) {
                    preflightScreenPreferenceScheduled_ = false;
                    return;
                }
                const auto current = engine_->Snapshot();
                if (current.room.screenShareState !=
                        RoomScreenShareState::kSwitching ||
                    current.room.screenShareEpoch != expectedEpoch ||
                    current.room.pendingScreenSharerDeviceId.empty() ||
                    current.room.pendingScreenSharerDeviceId ==
                        current.localDeviceId) {
                    preflightScreenPreferenceScheduled_ = false;
                    return;
                }
                ScreenStreamPreferenceRequest request;
                request.framesPerSecond = kDefaultScreenFrameRate;
                const QSettings settings;
                const int qualityValue = settings.value(
                    QString::fromLatin1(
                        kRemoteScreenQualitySetting),
                    static_cast<int>(
                        ScreenQualityTier::kOriginal)).toInt();
                request.quality =
                    qualityValue >= static_cast<int>(
                        ScreenQualityTier::kAutomatic) &&
                    qualityValue <= static_cast<int>(
                        ScreenQualityTier::kOriginal)
                    ? static_cast<ScreenQualityTier>(qualityValue)
                    : ScreenQualityTier::kOriginal;
                const auto [width, height] =
                    SavedScreenQualityBounds(request.quality);
                request.maxWidth = width;
                request.maxHeight = height;
                const auto finish =
                    [window = QPointer<ControllerMainWindow>(this), expectedEngine = engine_.get(),
                     expectedRoomId = current.room.roomId, expectedEpoch]
                    (const SessionCommandResult& result) {
                        if (!window || window->engine_.get() != expectedEngine ||
                            window->preflightScreenPreferenceAttemptEpoch_ != expectedEpoch) {
                            return;
                        }
                        const auto now = window->engine_->Snapshot();
                        if (now.room.roomId != expectedRoomId ||
                            now.room.screenShareEpoch != expectedEpoch) return;
                        if (result.accepted) {
                            window->preflightScreenPreferenceEpoch_ = expectedEpoch;
                        }
                        window->preflightScreenPreferenceScheduled_ = false;
                        if (!result.accepted &&
                            window->preflightScreenPreferenceAttempts_ < 5) {
                            QTimer::singleShot(50, window.data(), [window, expectedEpoch] {
                                if (!window || !window->engine_) return;
                                const auto retry = window->engine_->Snapshot();
                                if (retry.room.screenShareState == RoomScreenShareState::kSwitching &&
                                    retry.room.screenShareEpoch == expectedEpoch) {
                                    window->OnSessionEngineSnapshot(retry);
                                }
                            });
                        }
                    };
                const auto result = engine_->QueueRoomScreenStreamPreference(
                    pairId, request, [finish](SessionCommandResult sent) {
                        // The application outlives engine shutdown. Check the
                        // window only on its GUI thread, never on the sender.
                        QMetaObject::invokeMethod(QCoreApplication::instance(),
                            [finish, sent = std::move(sent)] { finish(sent); },
                            Qt::QueuedConnection);
                    });
                if (!result.accepted) finish(result);
            });
        }
    } else if (!canPreflightRemoteScreen &&
               snapshot.room.screenShareState !=
                   RoomScreenShareState::kActive) {
        preflightScreenPreferenceEpoch_ = 0;
        preflightScreenPreferenceAttemptEpoch_ = 0;
        preflightScreenPreferenceAttempts_ = 0;
        preflightScreenPreferenceScheduled_ = false;
    }

    if (inputExecutor_) {
        const bool localIsRoomSharedDisplayOwner =
            snapshot.room.screenSharerDeviceId ==
                snapshot.localDeviceId &&
            snapshot.screenShare.activeDisplay.sessionDisplayId != 0 &&
            snapshot.screenShare.activeDisplayLayoutVersion != 0;
        const bool localIsDirectControlledDisplayOwner =
            snapshot.state == SessionEngineState::kActive &&
            snapshot.purpose == SessionPurpose::kRemoteControl &&
            snapshot.remoteControlRole == RemoteControlRole::kControlled &&
            snapshot.screenShare.activeDisplay.sessionDisplayId != 0 &&
            snapshot.screenShare.activeDisplayLayoutVersion != 0;
        if (localIsRoomSharedDisplayOwner ||
            localIsDirectControlledDisplayOwner) {
            inputExecutor_->SetActiveDisplay(
                snapshot.screenShare.topology,
                snapshot.screenShare.activeDisplay.stableDisplayKey);
        } else {
            inputExecutor_->ClearActiveDisplay();
        }
    }

    const QString roomErrorCode =
        QString::fromStdString(snapshot.room.errorCode);
    const bool joinRejected =
        roomErrorCode == QStringLiteral("rejected_by_owner") ||
        roomErrorCode == QStringLiteral("room_join_rejected");
    const bool controlRejected =
        roomErrorCode == QStringLiteral("rejected_by_screen_sharer") ||
        roomErrorCode == QStringLiteral("room_control_rejected");
    const bool screenShareSwitchFailed =
        roomErrorCode ==
            QStringLiteral("screen_share_switch_rejected_by_sharer") ||
        roomErrorCode == QStringLiteral("screen_share_switch_timeout") ||
        roomErrorCode ==
            QStringLiteral("screen_share_switch_sharer_offline");
    const bool screenShareStartFailed =
        roomErrorCode == QStringLiteral("display_not_available") ||
        roomErrorCode ==
            QStringLiteral("desktop_capture_start_failed") ||
        roomErrorCode ==
            QStringLiteral("desktop_capture_frame_rate_rejected") ||
        roomErrorCode ==
            QStringLiteral("screen_encoding_policy_failed") ||
        roomErrorCode ==
            QStringLiteral("screen_sender_activate_failed");
    if (joinRejected || controlRejected || screenShareSwitchFailed ||
        screenShareStartFailed) {
        const QString decisionKey =
            QString::fromStdString(snapshot.room.roomId) +
            QLatin1Char('|') + roomErrorCode;
        if (lastRoomDecisionAlertKey_ != decisionKey) {
            lastRoomDecisionAlertKey_ = decisionKey;
            const QString title =
                joinRejected
                    ? QStringLiteral("加入房间被拒绝")
                    : (controlRejected
                           ? QStringLiteral("控制申请被拒绝")
                           : (screenShareStartFailed
                                  ? QStringLiteral("屏幕共享启动失败")
                                  : QStringLiteral("接替主机器未完成")));
            const QString message =
                joinRejected
                    ? QStringLiteral("对方已拒绝你的加入房间申请。")
                    : (controlRejected
                           ? QStringLiteral(
                                 "对方已拒绝你的远程控制申请，你仍可继续观看共享画面。")
                           : (screenShareStartFailed
                                  ? (snapshot.room.errorMessage.empty()
                                         ? QStringLiteral(
                                               "当前显示器无法启动采集，请检查调试信息中的采集错误。")
                                         : QString::fromStdString(
                                               snapshot.room.errorMessage))
                                  : (roomErrorCode ==
                                             QStringLiteral(
                                                 "screen_share_switch_rejected_by_sharer")
                                         ? QStringLiteral(
                                               "当前主机器拒绝了你的接替申请，原共享画面不受影响。")
                                         : QStringLiteral(
                                               "接替申请已超时或当前主机器暂时离线，原共享状态未被改变。"))));
            const QPointer<QWidget> alertParent =
                controlRejected && remoteSessionWindow_
                    ? static_cast<QWidget*>(remoteSessionWindow_.data())
                    : static_cast<QWidget*>(this);
            QTimer::singleShot(0, this,
                               [this, alertParent, title, message] {
                RemoteCDialog::Alert(
                    alertParent ? alertParent.data() : this,
                    title, message, QStringLiteral("知道了"),
                    RemoteCDialog::Tone::kDanger, true);
            });
        }
    } else {
        lastRoomDecisionAlertKey_.clear();
    }
    const QString previousConnectivityText = connectivityPill_->text();
    const QString previousServiceText = serviceStatus_->text();
    const auto setConnectivityPill = [this](const QString& text,
                                            const QString& foreground,
                                            const QString& background,
                                            const QString& border) {
        connectivityPill_->setText(text);
        const QString style = QStringLiteral(
                "background:%1;color:%2;border:1px solid %3;"
                "border-radius:14px;padding:5px 12px;font-weight:700;")
                .arg(background, foreground, border);
        if (connectivityPill_->styleSheet() != style) {
            connectivityPill_->setStyleSheet(style);
        }
    };
    switch (snapshot.connectivity) {
    case SessionConnectivityState::kNotConfigured:
        setConnectivityPill(QStringLiteral("●  信令未配置"),
                            QStringLiteral("#a66b12"),
                            QStringLiteral("#fff6df"),
                            QStringLiteral("#f1d99e"));
        serviceStatus_->setText(QStringLiteral("● 本地引擎就绪"));
        SetRoomActionHint(QStringLiteral(
            "尚未配置 WSS。请检查 RLink 启动配置或重新登录。"));
        break;
    case SessionConnectivityState::kConnecting:
        setConnectivityPill(QStringLiteral("●  正在连接信令"),
                            QStringLiteral("#a66b12"),
                            QStringLiteral("#fff6df"),
                            QStringLiteral("#f1d99e"));
        serviceStatus_->setText(QStringLiteral("● 正在注册设备"));
        SetRoomActionHint(QStringLiteral("正在通过 WSS 认证并注册本机设备。"));
        break;
    case SessionConnectivityState::kOnline:
        setConnectivityPill(QStringLiteral("●  信令在线"),
                            QStringLiteral("#0d8b50"),
                            QStringLiteral("#e9f8ef"),
                            QStringLiteral("#bde8cd"));
        serviceStatus_->setText(QStringLiteral("● 可创建或加入房间"));
        SetRoomActionHint(QStringLiteral(
            "设备已注册，房间控制面可用。默认 2 人房间天然兼容两人远控。"));
        break;
    case SessionConnectivityState::kOffline:
        setConnectivityPill(QStringLiteral("●  信令离线"),
                            QStringLiteral("#a66b12"),
                            QStringLiteral("#fff6df"),
                            QStringLiteral("#f1d99e"));
        serviceStatus_->setText(QStringLiteral("● 等待信令连接"));
        SetRoomActionHint(QStringLiteral(
            "WSS 当前离线；如果已经在房间中，席位会在恢复窗口内保留。"));
        break;
    case SessionConnectivityState::kFailed:
        setConnectivityPill(QStringLiteral("●  信令连接失败"),
                            QStringLiteral("#b4232f"),
                            QStringLiteral("#fff0f1"),
                            QStringLiteral("#f2bec3"));
        serviceStatus_->setText(QStringLiteral("● 本地引擎可用"));
        SetRoomActionHint(
            snapshot.error.message.empty()
                ? QStringLiteral("WSS 信令连接失败，请检查地址、令牌和证书。")
                : QString::fromStdString(snapshot.error.message),
            true);
        break;
    }
    const bool connectivityBusy =
        snapshot.connectivity == SessionConnectivityState::kConnecting ||
        snapshot.connectivity == SessionConnectivityState::kOffline;
    if (directConnectPage_) {
        directConnectPage_->SetSignalStatus(
            connectivityPill_->text(), connectivityPill_->styleSheet());
    }

    if (!snapshot.roomActivity.availabilities.empty()) {
        QSettings settings;
        QVariantList records = settings.value(
            RecentSettingsKey(QStringLiteral("recentRooms"))).toList();
        bool changed = false;
        for (const auto& availability : snapshot.roomActivity.availabilities) {
            const QString roomId = QString::fromStdString(
                availability.roomId);
            for (QVariant& value : records) {
                QVariantMap record = value.toMap();
                if (record.value(QStringLiteral("roomId")).toString() !=
                    roomId) {
                    continue;
                }
                const int state = static_cast<int>(availability.state);
                if (record.value(QStringLiteral("availability"), -1)
                        .toInt() != state) {
                    record.insert(QStringLiteral("availability"), state);
                    value = record;
                    changed = true;
                }
                break;
            }
        }
        if (changed) {
            settings.setValue(
                RecentSettingsKey(QStringLiteral("recentRooms")), records);
            RefreshRecentRooms();
        }
    }
    if (snapshot.room.membership == RoomMembershipState::kFailed &&
        (snapshot.room.errorCode == "room_unavailable" ||
         snapshot.room.errorCode == "room_not_found") &&
        !snapshot.room.roomId.empty()) {
        QSettings settings;
        QVariantList records = settings.value(
            RecentSettingsKey(QStringLiteral("recentRooms"))).toList();
        const QString failedRoomId = QString::fromStdString(
            snapshot.room.roomId);
        bool changed = false;
        for (QVariant& value : records) {
            QVariantMap record = value.toMap();
            if (record.value(QStringLiteral("roomId")).toString() ==
                failedRoomId) {
                record.insert(
                    QStringLiteral("availability"),
                    static_cast<int>(RoomAvailabilityState::kClosed));
                value = record;
                changed = true;
                break;
            }
        }
        if (changed) {
            settings.setValue(
                RecentSettingsKey(QStringLiteral("recentRooms")), records);
            RefreshRecentRooms();
        }
    }
    if (snapshot.connectivity == SessionConnectivityState::kOnline) {
        if (!recentRoomAvailabilityRequested_) {
            recentRoomAvailabilityRequested_ = true;
            QTimer::singleShot(
                0, this,
                [this] { RequestRecentRoomAvailability(); });
        }
    } else {
        recentRoomAvailabilityRequested_ = false;
    }
    RefreshOwnedDevicesUi(snapshot);
    directConnectPage_->SetLocalCredentials(
        QString::fromStdString(snapshot.localDeviceId),
        QString::fromStdString(snapshot.localVerificationCode));

    RefreshDiagnosticsSnapshotUi(snapshot);
    RememberRecentRoom(snapshot);
    RememberRecentDevice(snapshot);
    UpdateRoomUi(snapshot);
    UpdateDirectFileTransferSession(snapshot);
    UpdateVideoPipelineSettingsAvailability(snapshot);

    // UpdateRoomUi may replace the general signaling text with the final
    // room-specific state. Compare and animate only that final value; doing
    // this earlier made the sidebar status briefly animate an intermediate
    // string on every media-state snapshot.
    SetBusyStatusAnimation(connectivityPill_, connectivityBusy);
    SetBusyStatusAnimation(serviceStatus_, connectivityBusy);
    if (!connectivityBusy &&
        previousConnectivityText != connectivityPill_->text()) {
        AnimateSmallUiChange(connectivityPill_);
    }
    if (!connectivityBusy && previousServiceText != serviceStatus_->text()) {
        AnimateSmallUiChange(serviceStatus_);
    }

    if (snapshot.media.localCamera == LocalCameraState::kOff ||
        snapshot.media.localCamera == LocalCameraState::kFailed) {
        localCameraStopRequested_ = false;
    }
    const bool roomActiveForCamera =
        snapshot.room.membership == RoomMembershipState::kActive;
    const bool remoteCameraPublished = roomActiveForCamera && std::any_of(
        snapshot.room.members.begin(), snapshot.room.members.end(),
        [&snapshot](const RoomMemberSnapshot& member) {
            return member.deviceId != snapshot.localDeviceId &&
                member.online && member.cameraPublishing;
        });
    const bool localCameraPublished = roomActiveForCamera &&
        !localCameraStopRequested_ &&
        (snapshot.media.localCamera == LocalCameraState::kStarting ||
         snapshot.media.localCamera == LocalCameraState::kPublishing);
    const bool anyPublishedCamera =
        remoteCameraPublished || localCameraPublished;
    if (anyPublishedCamera && sessionMedia_) {
        if (!roomCameraWindow_) {
            roomCameraWindow_ = new RoomCameraWindow(sessionMedia_, nullptr);
            roomCameraWindow_->SetHiddenByUserCallback([this] {
                cameraGalleryManuallyHidden_ = true;
                if (engine_) UpdateRoomUi(engine_->Snapshot());
            });
        }
        roomCameraWindow_->SyncSnapshot(snapshot);
        const bool autoOpenCameraGallery = QSettings().value(
            QString::fromLatin1(kAutoOpenCameraGallerySetting),
            true).toBool();
        const bool mayOpenCameraGallery =
            localCameraPublished || autoOpenCameraGallery;
        if (mayOpenCameraGallery && !cameraGalleryManuallyHidden_ &&
            !roomCameraWindow_->isVisible()) {
            roomCameraWindow_->OpenBesideMainWindow(frameGeometry());
        }
    } else if (roomCameraWindow_) {
        roomCameraWindow_->SyncSnapshot(snapshot);
        roomCameraWindow_->hide();
        cameraGalleryManuallyHidden_ = false;
    }

    const bool roomMembershipActive =
        snapshot.room.membership == RoomMembershipState::kActive;
    const bool remoteRoomShareActive =
        roomMembershipActive &&
        snapshot.room.screenShareState == RoomScreenShareState::kActive &&
        !snapshot.room.screenSharerDeviceId.empty() &&
        snapshot.room.screenSharerDeviceId != snapshot.localDeviceId;
    if (remoteRoomShareActive) {
        const QString activeSharer = QString::fromStdString(
            snapshot.room.screenSharerDeviceId);
        const bool manuallyDismissed =
            dismissedRemoteScreenSharerDeviceId_ == activeSharer &&
            dismissedRemoteScreenShareEpoch_ ==
                snapshot.room.screenShareEpoch;
        if (!manuallyDismissed &&
            dismissedRemoteScreenShareEpoch_ != 0) {
            dismissedRemoteScreenSharerDeviceId_.clear();
            dismissedRemoteScreenShareEpoch_ = 0;
        }
        const auto pair = std::find_if(
            snapshot.roomActivity.peerConnections.begin(),
            snapshot.roomActivity.peerConnections.end(),
            [&snapshot](const RoomPeerConnectionSnapshot& candidate) {
                return candidate.peerDeviceId ==
                           snapshot.room.screenSharerDeviceId &&
                       candidate.state == RoomPeerConnectionState::kActive;
            });
        if (pair != snapshot.roomActivity.peerConnections.end() &&
            !manuallyDismissed) {
            const auto member = std::find_if(
                snapshot.room.members.begin(), snapshot.room.members.end(),
                [&snapshot](const RoomMemberSnapshot& candidate) {
                    return candidate.deviceId ==
                           snapshot.room.screenSharerDeviceId;
                });
            const QString deviceId = QString::fromStdString(
                snapshot.room.screenSharerDeviceId);
            const QString deviceName =
                member != snapshot.room.members.end() &&
                        !member->deviceName.empty()
                    ? QString::fromStdString(member->deviceName)
                    : deviceId;
            OpenRemoteSession(RemoteSessionBinding::Room(
                deviceId, deviceName,
                QString::fromStdString(pair->pairId)));
        }
    } else {
        const bool localRoomShareActive =
            roomMembershipActive &&
            snapshot.room.screenShareState ==
                RoomScreenShareState::kActive &&
            snapshot.room.screenSharerDeviceId == snapshot.localDeviceId;
        const bool roomShareDefinitivelyEnded =
            !roomMembershipActive ||
            snapshot.room.screenShareState ==
                RoomScreenShareState::kIdle ||
            localRoomShareActive;
        if (remoteSessionBinding_ && remoteSessionBinding_->IsRoom() &&
            roomShareDefinitivelyEnded) {
            if (remoteSessionWindow_) {
                remoteSessionWindow_->close();
            }
            remoteSessionBinding_.reset();
            dismissedRemoteScreenSharerDeviceId_.clear();
            dismissedRemoteScreenShareEpoch_ = 0;
        } else if (snapshot.room.screenShareState ==
                       RoomScreenShareState::kIdle) {
            dismissedRemoteScreenSharerDeviceId_.clear();
            dismissedRemoteScreenShareEpoch_ = 0;
        }
    }

    // The direct 1V1 path remains during migration so existing tests and the
    // independent media windows continue to compile and behave as before.
    if (snapshot.state == SessionEngineState::kConnecting) {
        if (assistedSessionPending_) {
            directConnectPage_->SetActionState(
                !assistedSessionCancellationPending_ &&
                    !assistedSessionTimedOut_,
                assistedSessionCancellationPending_
                    ? (assistedSessionTimedOut_
                           ? QStringLiteral("连接已超时")
                           : QStringLiteral("正在取消…"))
                    : QStringLiteral("正在连接…（点击取消）"));
            if (!assistedSessionCancellationPending_) {
                directConnectPage_->SetAssistHint(
                    QStringLiteral("正在校验验证码并建立远程桌面…"),
                    DirectConnectPage::AssistHintTone::kWarning);
            }
        } else {
            directConnectPage_->SetActionState(
                false, QStringLiteral("正在建立会话…"));
        }
    } else if (snapshot.state == SessionEngineState::kActive &&
               (assistedSessionPending_ || assistedSessionActive_)) {
        directConnectPage_->SetActionState(
            false, QStringLiteral("远程桌面已连接"));
    } else {
        directConnectPage_->SetActionState(
            authenticationAvailable_ &&
            snapshot.connectivity == SessionConnectivityState::kOnline &&
            snapshot.state == SessionEngineState::kReady &&
            IsNineDigitPublicId(directConnectPage_->DeviceId()) &&
            directConnectPage_->VerificationCode().size() == 6 &&
            directConnectPage_->DeviceId().toStdString() !=
                snapshot.localDeviceId,
            QStringLiteral("发起连接"));
    }

    const QString directSessionError =
        QString::fromStdString(snapshot.error.code);
    const bool directRequestWasPending =
        assistedSessionPending_ || ownedDeviceSessionPending_;
    const bool initialPeerConnectionFailed =
        directSessionError == QStringLiteral("negotiation_timeout") ||
        directSessionError == QStringLiteral("peer_connection_create_failed") ||
        directSessionError == QStringLiteral("p2p_connection_failed");
    if (snapshot.state == SessionEngineState::kConnecting &&
        directSessionError.isEmpty()) {
        // The controlled side does not go through StartAssistedSession() or
        // StartOwnedDeviceSession(), so reset the one-shot alert guard when a
        // fresh incoming direct session starts as well.
        lastDirectSessionToastError_.clear();
    }
    QString directSessionErrorText;
    if (directSessionError == QStringLiteral("verification_code_invalid")) {
        directSessionErrorText =
            QStringLiteral("验证码错误，请向对方核对当前显示的 6 位一次性验证码。");
    } else if (directSessionError == QStringLiteral("rate_limited")) {
        directSessionErrorText =
            QStringLiteral("尝试过于频繁，请稍后再试。");
    } else if (directSessionError == QStringLiteral("target_offline")) {
        directSessionErrorText =
            QStringLiteral("对方设备不在线，请确认 RLink 已运行且信令已连接。");
    } else if (directSessionError == QStringLiteral("device_busy")) {
        directSessionErrorText =
            QStringLiteral("对方设备已在协助房间或其他远程会话中，请结束后再试。");
    } else if (directSessionError ==
               QStringLiteral("session_request_timeout")) {
        directSessionErrorText =
            QStringLiteral("连接请求超时，请确认对方在线并使用最新验证码后重试。");
    } else if (directSessionError ==
                   QStringLiteral("invalid_session_request") ||
               directSessionError ==
                   QStringLiteral("invalid_session_permissions")) {
        directSessionErrorText =
            QStringLiteral("连接请求无效，请刷新验证码后重新发起连接。");
    } else if (directSessionError == QStringLiteral("session_rejected")) {
        directSessionErrorText =
            QStringLiteral("对方设备未接受本次连接，请重新获取验证码后再试。");
    } else if (initialPeerConnectionFailed) {
        directSessionErrorText =
            QStringLiteral("设备间的 P2P 连接未能建立，请更换网络后重试。");
    }
    if (assistedSessionPending_ &&
        snapshot.state == SessionEngineState::kReady) {
        assistedSessionTimeoutTimer_->stop();
        if (assistedSessionTimedOut_) {
            directSessionErrorText = QStringLiteral(
                "连接请求超时，请确认对方在线并使用最新验证码后重试。");
        } else if (assistedSessionCancellationPending_) {
            directConnectPage_->SetAssistHint(
                QStringLiteral("连接已取消，可以重新发起。"),
                DirectConnectPage::AssistHintTone::kDefault);
        } else if (directSessionErrorText.isEmpty() &&
                   !directSessionError.isEmpty()) {
            directSessionErrorText = QStringLiteral(
                "远程桌面未能建立（错误代码：%1），请稍后重试。")
                .arg(directSessionError);
        } else if (directSessionErrorText.isEmpty()) {
            directConnectPage_->SetAssistHint(
                QStringLiteral(
                    "本次连接未建立，可以检查设备 ID 和验证码后重试。"),
                DirectConnectPage::AssistHintTone::kDefault);
        }
        assistedSessionPending_ = false;
        assistedSessionCancellationPending_ = false;
        assistedSessionTimedOut_ = false;
    } else if (assistedSessionPending_ &&
               snapshot.state == SessionEngineState::kActive) {
        assistedSessionTimeoutTimer_->stop();
        assistedSessionPending_ = false;
        assistedSessionActive_ = true;
        assistedSessionCancellationPending_ = false;
        assistedSessionTimedOut_ = false;
        directConnectPage_->SetAssistHint(
            QStringLiteral("远程桌面已建立，断开后可以重新发起连接。"),
            DirectConnectPage::AssistHintTone::kSuccess);
    } else if (assistedSessionActive_ &&
               snapshot.state == SessionEngineState::kReady) {
        assistedSessionActive_ = false;
        directConnectPage_->SetAssistHint(
            QStringLiteral(
                "远程桌面已断开，可以使用对方当前验证码重新连接。"),
            DirectConnectPage::AssistHintTone::kDefault);
    }
    if (!directSessionErrorText.isEmpty() &&
        snapshot.state == SessionEngineState::kReady) {
        directConnectPage_->SetAssistHint(
            directSessionErrorText,
            DirectConnectPage::AssistHintTone::kError);

        if ((directRequestWasPending || initialPeerConnectionFailed) &&
            lastDirectSessionToastError_ != directSessionError) {
            lastDirectSessionToastError_ = directSessionError;
            if (initialPeerConnectionFailed) {
                // Let this snapshot finish closing the failed monitor window
                // before entering a modal dialog. Otherwise the old session
                // window can remain above the warning until it is dismissed.
                QTimer::singleShot(0, this, [this] {
                    RemoteCDialog::Alert(
                        this,
                        QStringLiteral("P2P 连接失败"),
                        QStringLiteral(
                            "未能建立设备之间的 P2P 连接，请更换本机或对方设备所使用的网络后再试。"
                            "<br><br><b>提示：使用手机热点连接成功率更高哦。</b>"),
                        QStringLiteral("知道了"),
                        RemoteCDialog::Tone::kWarning,
                        true);
                });
            } else {
                RemoteCToast::Show(
                    this, directSessionErrorText,
                    RemoteCToast::Tone::kError);
            }
        }
    }

    if (ownedDeviceSessionPending_) {
        if (snapshot.state == SessionEngineState::kReady) {
            if (directSessionErrorText.isEmpty()) {
                const QString message = QStringLiteral(
                    "未能连接到该设备，请确认对方在线且未处于其他会话中。");
                RemoteCToast::Show(
                    this, message, RemoteCToast::Tone::kError);
            }
            ownedDeviceSessionPending_ = false;
        } else if (snapshot.direct.signalingSessionReady ||
                   snapshot.state == SessionEngineState::kActive) {
            ownedDeviceSessionPending_ = false;
        }
    }

    if (snapshot.state == SessionEngineState::kAwaitingLocalApproval &&
        !snapshot.sessionId.empty()) {
        const QString sessionId = QString::fromStdString(snapshot.sessionId);
        if (promptedSessionId_ != sessionId) {
            promptedSessionId_ = sessionId;
            const QString peer = QString::fromStdString(snapshot.peerDeviceId);
            const QString purpose =
                snapshot.purpose == SessionPurpose::kRemoteControl
                    ? QStringLiteral("远程控制")
                    : QStringLiteral("摄像头画面");
            const bool accepted = RemoteCDialog::Confirm(
                this, QStringLiteral("收到会话请求"),
                QStringLiteral("设备 %1 请求建立%2会话，是否允许？")
                    .arg(peer, purpose),
                QStringLiteral("允许"), QStringLiteral("拒绝"),
                RemoteCDialog::Tone::kQuestion);
            if (accepted) {
                engine_->AcceptIncomingSession(snapshot.sessionId);
            } else {
                engine_->RejectIncomingSession(snapshot.sessionId);
            }
        }
    } else if (snapshot.state == SessionEngineState::kReady) {
        promptedSessionId_.clear();
        dismissedDirectSessionId_.clear();
    }

    if (!assistedSessionCancellationPending_ &&
        !assistedSessionTimedOut_ &&
        snapshot.state == SessionEngineState::kConnecting &&
        (snapshot.origin == SessionOrigin::kRemoteAssistance ||
         snapshot.origin == SessionOrigin::kOwnedDevice) &&
        snapshot.remoteControlRole == RemoteControlRole::kController &&
        snapshot.direct.signalingSessionReady) {
        const QString peer = QString::fromStdString(snapshot.peerDeviceId);
        dismissedDirectSessionId_.clear();
        OpenRemoteSession(RemoteSessionBinding::Direct(
            peer,
            pendingDeviceName_.isEmpty()
                ? QStringLiteral("远程协助设备")
                : pendingDeviceName_,
            SessionOrigin::kRemoteAssistance));
    }

    if (snapshot.state == SessionEngineState::kActive) {
        const QString peer = QString::fromStdString(snapshot.peerDeviceId);
        if (snapshot.purpose == SessionPurpose::kRemoteControl &&
            snapshot.remoteControlRole == RemoteControlRole::kController &&
            QString::fromStdString(snapshot.sessionId) !=
                dismissedDirectSessionId_) {
            OpenRemoteSession(RemoteSessionBinding::Direct(
                peer,
                pendingDeviceName_.isEmpty()
                    ? QStringLiteral("远程设备")
                    : pendingDeviceName_,
                snapshot.origin));
        } else if (snapshot.purpose == SessionPurpose::kCameraOnly) {
            OpenCameraWindow(peer, pendingDeviceName_.isEmpty()
                                       ? QStringLiteral("远程设备")
                                       : pendingDeviceName_);
        }
    }

    if (remoteSessionBinding_ && remoteSessionBinding_->IsDirect() &&
        snapshot.state != SessionEngineState::kActive &&
        snapshot.state != SessionEngineState::kConnecting &&
        !IsDirectRecoveryFailureCode(snapshot.error.code) &&
        remoteSessionWindow_) {
        remoteSessionWindow_->close();
    }

    if (remoteSessionWindow_) {
        const std::size_t onlineMemberCount =
            static_cast<std::size_t>(std::count_if(
                snapshot.room.members.begin(),
                snapshot.room.members.end(),
                [](const RoomMemberSnapshot& member) {
                    return member.online;
                }));
        remoteSessionWindow_->SetRoomOnlineMemberCount(
            onlineMemberCount);
        remoteSessionWindow_->RefreshControlState();
    }

    QueueRoomJoinApproval(snapshot);
    QueueRoomScreenShareSwitchApproval(snapshot);
    QueueRoomControlApproval(snapshot);
    QueueRoomScreenShareViewApproval(snapshot);
    HandleRoomMemberActionResults(snapshot);
}

}  // namespace remote::controller

