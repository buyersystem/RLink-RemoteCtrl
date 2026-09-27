// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ControllerMainWindow.h"
#include "ControllerMainWindowSupport.h"

#include <QDateTime>
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
                const auto result =
                    engine_->SetRoomScreenStreamPreference(
                        pairId, request);
                if (result.accepted) {
                    preflightScreenPreferenceEpoch_ = expectedEpoch;
                }
                preflightScreenPreferenceScheduled_ = false;
                if (!result.accepted &&
                    preflightScreenPreferenceAttempts_ < 5) {
                    QTimer::singleShot(
                        50, this, [this, expectedEpoch] {
                            if (!engine_) {
                                return;
                            }
                            const auto retry = engine_->Snapshot();
                            if (retry.room.screenShareState ==
                                    RoomScreenShareState::kSwitching &&
                                retry.room.screenShareEpoch ==
                                    expectedEpoch) {
                                OnSessionEngineSnapshot(retry);
                            }
                        });
                }
            });
        }
    } else if (!canPreflightRemoteScreen &&
               snapshot.room.screenShareState !=
                   RoomScreenShareState::kActive) {
        preflightScreenPreferenceEpoch_ = 0;
        preflightScreenPreferenceAttemptEpoch_ = 0;
        preflightScreenPreferenceAttempts_ = 0;
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
        connectivityPill_->setStyleSheet(
            QStringLiteral(
                "background:%1;color:%2;border:1px solid %3;"
                "border-radius:14px;padding:5px 12px;font-weight:700;")
                .arg(background, foreground, border));
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

    const bool debugPageVisible =
        debugPage_ && debugPage_->isVisibleTo(this);
    if (debugPage_ && debugPage_->HasValues() &&
        (debugPageVisible || diagnosticsCopyTextRequested_)) {
        const auto capabilities = engine_->Capabilities();
        const auto onlineMembers = std::count_if(
            snapshot.room.members.begin(), snapshot.room.members.end(),
            [](const RoomMemberSnapshot& member) { return member.online; });
        const auto activePeers = std::count_if(
            snapshot.roomActivity.peerConnections.begin(),
            snapshot.roomActivity.peerConnections.end(),
            [](const RoomPeerConnectionSnapshot& peer) {
                return peer.state == RoomPeerConnectionState::kActive;
            });
        const auto setDebugValue =
            [this](const QString& key, const QString& value,
                   const char* tone = "normal") {
                auto* label = debugPage_->ValueLabel(key);
                if (!label) {
                    return;
                }
                label->setText(value);
                label->setToolTip(value);
                if (label->property("tone").toByteArray() != tone) {
                    label->setProperty("tone", tone);
                    label->style()->unpolish(label);
                    label->style()->polish(label);
                }
            };
        const QString deviceId = snapshot.localDeviceId.empty()
                                     ? QStringLiteral("未注册")
                                     : QString::fromStdString(
                                           snapshot.localDeviceId);
        const QString signaling =
            ConnectivityDebugName(snapshot.connectivity);
        const bool signalingOnline =
            snapshot.connectivity == SessionConnectivityState::kOnline;
        const QString webRtc =
            capabilities.webRtcReady ? QStringLiteral("运行正常")
                                     : QStringLiteral("不可用");
        const QString systemEnvironment =
            QStringLiteral("%1 · %2")
                .arg(
                    capabilities.operatingSystemDescription.empty()
                        ? QStringLiteral("Windows 版本未报告")
                        : QString::fromStdString(
                              capabilities.operatingSystemDescription),
                    capabilities.nativeArchitecture.empty()
                        ? QStringLiteral("架构未知")
                        : QString::fromStdString(
                              capabilities.nativeArchitecture));
        const QString sessionEnvironment =
            capabilities.remoteSession
                ? QStringLiteral("Windows RDP 远程会话")
                : QStringLiteral("本地控制台会话");
        QStringList graphicsAdapters;
        for (const auto& adapter :
             capabilities.graphicsAdapterDescriptions) {
            graphicsAdapters
                << QString::fromStdString(adapter);
        }
        if (!capabilities.graphicsEnumerationError.empty()) {
            graphicsAdapters
                << QStringLiteral("错误：%1")
                       .arg(QString::fromStdString(
                           capabilities.graphicsEnumerationError));
        }
        if (graphicsAdapters.isEmpty()) {
            graphicsAdapters
                << QStringLiteral("没有枚举到 DXGI 图形适配器。");
        }
        const QString hardwareFingerprint =
            capabilities.hardwareFingerprint.empty()
                ? QStringLiteral("未生成")
                : QString::fromStdString(
                      capabilities.hardwareFingerprint);
        QString encoderProbeSource;
        if (capabilities.h264HardwareEncoderProbeFromCache) {
            encoderProbeSource =
                QStringLiteral("已复用当前硬件指纹的缓存结果");
        } else if (
            capabilities.h264HardwareEncoderProbeSucceeded) {
            encoderProbeSource =
                QStringLiteral("本次启动重新探测成功");
        } else if (
            capabilities.videoEncoderPreference == "software" ||
            capabilities.videoEncoderPreference == "ffmpeg") {
            encoderProbeSource =
                QStringLiteral("仅软件模式，未执行硬件探测");
        } else {
            encoderProbeSource =
                QStringLiteral("探测失败或尚未完成");
        }
        QString audioDeviceModule =
            capabilities.audioDeviceModuleCreated
                ? QStringLiteral("ADM 已创建并接入")
                : QStringLiteral("ADM 不可用");
        if (!capabilities.audioDeviceError.empty()) {
            audioDeviceModule += QStringLiteral("\n说明：%1")
                .arg(QString::fromStdString(
                    capabilities.audioDeviceError));
        }
        const QString cameraDevice =
            MediaDeviceCategoryDebugText(
                snapshot.media.localMediaDevices.camera);
        const QString microphoneDevice =
            MediaDeviceCategoryDebugText(
                snapshot.media.localMediaDevices.microphone);
        const QString speakerDevice =
            MediaDeviceCategoryDebugText(
                snapshot.media.localMediaDevices.speaker);
        const QString builtinEncoder =
            capabilities.hasH264Encoder
                ? QStringLiteral("可用")
                : QStringLiteral("不可用");
        const QString desktopCapture =
            capabilities.desktopCapturePreference == "libwebrtc"
                ? QStringLiteral("libwebrtc（像素差分）")
                : QStringLiteral("自研 DXGI（GPU 原生纹理）");
        QString encoderPreference = QStringLiteral("自动（硬件优先）");
        if (capabilities.videoEncoderPreference == "hardware") {
            encoderPreference =
                QStringLiteral("硬件编码（失败回退 FFmpeg/libx264）");
        } else if (capabilities.videoEncoderPreference ==
                   "ffmpeg_hardware") {
            encoderPreference =
                QStringLiteral(
                    "FFmpeg 硬件编码（失败回退 FFmpeg/libx264）");
        } else if (capabilities.videoEncoderPreference == "software") {
            encoderPreference = QStringLiteral("软件编码（OpenH264）");
        } else if (capabilities.videoEncoderPreference == "ffmpeg") {
            encoderPreference = QStringLiteral("软件编码（FFmpeg/libx264）");
        }
        const QString configuredEncoderQuality = QSettings().value(
            QString::fromLatin1(kFfmpegX264PresetSetting),
            QStringLiteral("medium")).toString().toLower();
        QString encoderQuality = QStringLiteral(
            "中 · libx264 medium · NVENC p4 · QSV medium · AMF balanced · MFT 50 · OpenH264 MEDIUM");
        if (configuredEncoderQuality == QStringLiteral("veryslow")) {
            encoderQuality = QStringLiteral(
                "极高 · libx264 veryslow · NVENC p7 · QSV veryslow · AMF quality · MFT 100 · OpenH264 HIGH");
        } else if (configuredEncoderQuality == QStringLiteral("slow")) {
            encoderQuality = QStringLiteral(
                "高 · libx264 slow · NVENC p6 · QSV slow · AMF quality · MFT 75 · OpenH264 HIGH");
        } else if (configuredEncoderQuality == QStringLiteral("veryfast")) {
            encoderQuality = QStringLiteral(
                "低 · libx264 veryfast · NVENC p2 · QSV faster · AMF speed · MFT 25 · OpenH264 LOW");
        } else if (configuredEncoderQuality == QStringLiteral("ultrafast")) {
            encoderQuality = QStringLiteral(
                "极低 · libx264 ultrafast · NVENC p1 · QSV veryfast · AMF speed · MFT 0 · OpenH264 LOW");
        }
        const QString hardwareEncoderCount =
            capabilities.h264HardwareEncoderCount == 0
                ? QStringLiteral("未检测到")
                : QStringLiteral("检测到 %1 个")
                      .arg(capabilities.h264HardwareEncoderCount);
        const QString cpuNv12 =
            capabilities.h264HardwareEncoderCpuNv12InputSupported
                ? QStringLiteral("支持")
                : QStringLiteral("暂未确认");
        const QString d3d11Encoder =
            capabilities.h264HardwareEncoderWired
                ? QStringLiteral("已接入（硬件优先）")
                : (capabilities.videoEncoderPreference == "software" ||
                   capabilities.videoEncoderPreference == "ffmpeg")
                      ? QStringLiteral("当前模式已禁用")
                      : QStringLiteral("未接入");
        const QString ffmpegHardwareEncoder =
            capabilities.ffmpegHardwareEncoderWired
                ? QStringLiteral("已接入并作为当前硬件路径")
                : capabilities.ffmpegHardwareEncoderAvailable
                    ? QStringLiteral("运行时可用，当前模式未选择")
                    : capabilities.ffmpegHardwareEncoderError.empty()
                        ? QStringLiteral("未接入")
                        : QStringLiteral("未接入：%1").arg(
                              QString::fromStdString(
                                  capabilities.ffmpegHardwareEncoderError));
        QString encoderFallback;
        if (capabilities.videoEncoderPreference == "hardware") {
            encoderFallback = QStringLiteral("当前模式已禁用");
        } else if (!capabilities.h264SoftwareEncoderWired) {
            encoderFallback =
                capabilities.videoEncoderPreference == "ffmpeg" &&
                        !capabilities.ffmpegX264EncoderError.empty()
                    ? QStringLiteral("未接入：%1")
                          .arg(QString::fromStdString(
                              capabilities.ffmpegX264EncoderError))
                    : QStringLiteral("未接入");
        } else if (capabilities.videoEncoderPreference == "software") {
            encoderFallback = QStringLiteral("OpenH264 已接入（显式选择）");
        } else if (capabilities.videoEncoderPreference == "ffmpeg") {
            encoderFallback = QStringLiteral("FFmpeg/libx264 已接入（显式选择）");
        } else if (capabilities.h264HardwareEncoderWired) {
            encoderFallback =
                QStringLiteral("已接入（硬件失败时回退）");
        } else {
            encoderFallback =
                QStringLiteral("已接入（当前主编码路径）");
        }
        const QString decoder =
            capabilities.hasH264Decoder ? QStringLiteral("可用")
                                        : QStringLiteral("不可用");
        QString decoderPreference =
            QStringLiteral("自动（FFmpeg 硬件优先，软件回退）");
        if (capabilities.videoDecoderPreference == "hardware") {
            decoderPreference = QStringLiteral("仅硬件");
        } else if (capabilities.videoDecoderPreference == "software") {
            decoderPreference = QStringLiteral("仅软件");
        }
        QString mfDecoderType;
        if (capabilities.mfD3D11DecoderHardware) {
            mfDecoderType = QStringLiteral("MF 硬件解码");
        } else if (capabilities.mfD3D11DecoderSoftware) {
            mfDecoderType =
                QStringLiteral("MF 软件解码（支持 D3D11 输出）");
        } else if (capabilities.mfD3D11DecoderConfigured) {
            mfDecoderType = QStringLiteral("MF 解码（类型未识别）");
        } else {
            mfDecoderType = QStringLiteral("未配置");
        }
        const QString mfDecoderName =
            capabilities.mfD3D11DecoderName.empty()
                ? QStringLiteral("当前没有可用的 MF D3D11 H264 解码器。")
                : QString::fromStdString(
                      capabilities.mfD3D11DecoderName);
        const QString d3d11Output =
            capabilities.d3d11NativeDecoderOutput
                ? QStringLiteral("支持")
                : QStringLiteral("不支持或未启用");
        const QString mfDecoderAsync =
            capabilities.mfD3D11DecoderHardware
                ? capabilities.mfD3D11DecoderAsynchronous
                      ? QStringLiteral("异步事件驱动")
                      : QStringLiteral("同步调用")
                : QStringLiteral("当前未使用硬件 MFT");
        const QString softwareFallback =
            capabilities.ffmpegSoftwareH264Decoder
                ? QStringLiteral("已接入")
                : QStringLiteral("未接入");
        const QString mfDecoderError =
            capabilities.mfD3D11DecoderError.empty()
                ? QStringLiteral("当前没有 MF 解码检测错误。")
                : QString::fromStdString(
                      capabilities.mfD3D11DecoderError);

        QStringList encoderDetails;
        for (const auto& encoder :
             capabilities.h264HardwareEncoderDescriptions) {
            encoderDetails
                << QString::fromStdString(encoder);
        }
        for (const auto& warning :
             capabilities.h264HardwareEncoderWarnings) {
            encoderDetails
                << QStringLiteral("警告：%1")
                       .arg(QString::fromStdString(warning));
        }
        for (const auto& encoder :
             capabilities.ffmpegHardwareEncoderDescriptions) {
            encoderDetails
                << QStringLiteral("FFmpeg 硬编：%1")
                       .arg(QString::fromStdString(encoder));
        }
        if (encoderDetails.isEmpty()) {
            encoderDetails
                << QStringLiteral("当前机器没有报告可用的硬件 H264 编码器。");
        }
        QStringList encoderRuntimeDetails;
        for (const auto& detail :
             capabilities.videoEncoderRuntimeDetails) {
            encoderRuntimeDetails
                << QString::fromStdString(detail);
        }
        if (encoderRuntimeDetails.isEmpty()) {
            encoderRuntimeDetails
                << QStringLiteral("当前没有活动的视频编码实例。");
        }
        const QString encoderFallbackReason =
            capabilities.videoEncoderLastFallbackReason.empty()
                ? QStringLiteral("当前没有发生硬件编码回退。")
                : QString::fromStdString(
                      capabilities.videoEncoderLastFallbackReason);

        const QString membership =
            RoomMembershipDebugName(snapshot.room.membership);
        const QString roomId = snapshot.room.roomId.empty()
                                   ? QStringLiteral("当前未加入房间")
                                   : QString::fromStdString(
                                         snapshot.room.roomId);
        const QString members =
            QStringLiteral("%1 人在线 · %2 个席位已占用 · 上限 %3 人")
                .arg(static_cast<qulonglong>(onlineMembers))
                .arg(static_cast<qulonglong>(
                    snapshot.room.members.size()))
                .arg(snapshot.room.capacity);
        const QString screenSharer =
            snapshot.room.screenSharerDeviceId.empty()
                ? QStringLiteral("无人共享")
                : MemberDisplayName(snapshot.room,
                                    snapshot.room.screenSharerDeviceId,
                                    snapshot.localDeviceId);
        const QString controller =
            snapshot.room.activeControllerDeviceId.empty()
                ? QStringLiteral("无人控制")
                : MemberDisplayName(snapshot.room,
                                    snapshot.room.activeControllerDeviceId,
                                    snapshot.localDeviceId);
        const QString controlGrant =
            snapshot.room.activeControllerDeviceId.empty()
                ? QStringLiteral("当前没有控制租约")
                : snapshot.roomControlGrantActive
                      ? QStringLiteral("授权令牌已同步")
                      : QStringLiteral("正在等待信令授权令牌");
        const QString peerSummary =
            QStringLiteral("%1 / %2 个成员对已连接")
                .arg(static_cast<qulonglong>(activePeers))
                .arg(static_cast<qulonglong>(
                    snapshot.roomActivity.peerConnections.size()));

        QStringList peerDetails;
        for (const auto& peer : snapshot.roomActivity.peerConnections) {
            const QString screenStartup =
                peer.screenFirstFramePresentedGeneration != 0
                    ? QStringLiteral("首屏已显示 · %1 ms · 刷新请求 %2 次")
                          .arg(peer.screenFirstFrameStartupMs)
                          .arg(peer.screenStartupRefreshRequests)
                    : peer.screenStartupRefreshRequests > 0
                        ? QStringLiteral("等待首屏 · 刷新请求 %1 次")
                              .arg(peer.screenStartupRefreshRequests)
                        : QStringLiteral("尚未启动");
            peerDetails
                << QStringLiteral(
                       "%1\n状态：%2 · DataChannel：%3 条 · 媒体槽：%4 个\n输入通道：键盘/可靠 %5 · 鼠标/快速 %6 · 显示映射 %7\n协商代次：%8 · ICE Restart：%9/3\n视频启动：%10\n成员对 ID：%11")
                       .arg(MemberDisplayName(
                                snapshot.room,
                                peer.peerDeviceId,
                                snapshot.localDeviceId),
                            PeerConnectionDebugName(peer.state))
                       .arg(peer.openDataChannelCount)
                       .arg(peer.preparedVideoSlotCount)
                       .arg(peer.controlReliableChannelOpen
                                ? QStringLiteral("已就绪")
                                : QStringLiteral("未就绪"))
                       .arg(peer.inputFastChannelOpen
                                ? QStringLiteral("已就绪")
                                : QStringLiteral("未就绪"))
                       .arg(peer.sharedDisplayId != 0 &&
                                    peer.sharedDisplayLayoutVersion != 0
                                ? QStringLiteral("已同步")
                                : QStringLiteral("未同步"))
                       .arg(peer.negotiationGeneration)
                       .arg(peer.iceRestartAttempt)
                       .arg(screenStartup)
                       .arg(QString::fromStdString(peer.pairId));
            if (!peer.errorMessage.empty()) {
                peerDetails
                    << QStringLiteral("错误：%1")
                           .arg(QString::fromStdString(
                               peer.errorMessage));
            }
        }
        if (peerDetails.isEmpty()) {
            peerDetails
                << QStringLiteral("当前没有需要显示的成员对连接。");
        }

        const QString sessionError =
            snapshot.error.message.empty()
                ? QStringLiteral("暂无错误")
                : QString::fromStdString(snapshot.error.message);
        const QString roomError =
            snapshot.room.errorMessage.empty()
                ? QStringLiteral("暂无错误")
                : QString::fromStdString(
                      snapshot.room.errorMessage);
        const app::ClipboardControllerSnapshot clipboard =
            clipboardController_
                ? clipboardController_->Snapshot()
                : app::ClipboardControllerSnapshot{};
        const QString clipboardState =
            !clipboard.enabled
                ? QStringLiteral("已关闭")
                : !clipboard.sessionActive
                      ? QStringLiteral("等待远程控制权限")
                      : QString::fromStdString(clipboard.state);
        const QString clipboardPeer = clipboard.peerDeviceId.empty()
            ? QStringLiteral("当前没有活动成员对")
            : QString::fromStdString(clipboard.peerDeviceId);
        const QString clipboardTransfer = clipboard.transferActive
            ? QStringLiteral(
                  "%1 · %2 · %3 / %4 字节 · %5 个项目 · "
                  "%6 KB/s · 预计剩余 %7 秒")
                  .arg(clipboard.transferOutgoing
                           ? QStringLiteral("发送")
                           : QStringLiteral("接收"))
                  .arg(QString::fromStdString(clipboard.state))
                  .arg(static_cast<qulonglong>(
                      clipboard.transferCompletedBytes))
                  .arg(static_cast<qulonglong>(
                      clipboard.transferTotalBytes))
                  .arg(clipboard.transferItemCount)
                  .arg(clipboard.transferBytesPerSecond / 1024.0,
                       0, 'f', 1)
                  .arg(static_cast<qulonglong>(
                      clipboard.transferEstimatedRemainingSeconds))
            : QStringLiteral("当前没有活动传输");
        const QString clipboardLast = clipboard.lastFormat.empty()
            ? QStringLiteral("尚未执行远程粘贴")
            : QStringLiteral("%1 · %2 字节")
                  .arg(QString::fromStdString(clipboard.lastFormat))
                  .arg(static_cast<qulonglong>(clipboard.lastBytes));
        const QString clipboardCounters = QStringLiteral(
            "发起粘贴 %1 项 · 接收粘贴 %2 项 · 失败/拒绝 %3 项")
            .arg(static_cast<qulonglong>(clipboard.sentItems))
            .arg(static_cast<qulonglong>(clipboard.receivedItems))
            .arg(static_cast<qulonglong>(clipboard.rejectedItems));
        const QString clipboardError =
            clipboard.lastErrorMessage.empty()
                ? QStringLiteral("暂无错误")
                : QString::fromStdString(clipboard.lastErrorMessage);

        setDebugValue(QStringLiteral("deviceId"), deviceId,
                      snapshot.localDeviceId.empty() ? "warning"
                                                     : "normal");
        setDebugValue(QStringLiteral("signaling"), signaling,
                      signalingOnline ? "good" : "warning");
        setDebugValue(QStringLiteral("webrtc"), webRtc,
                      capabilities.webRtcReady ? "good" : "error");
        setDebugValue(QStringLiteral("systemEnvironment"),
                      systemEnvironment);
        setDebugValue(
            QStringLiteral("sessionEnvironment"),
            sessionEnvironment,
            capabilities.remoteSession ? "warning" : "good");
        setDebugValue(
            QStringLiteral("graphicsAdapters"),
            graphicsAdapters.join(QStringLiteral("\n")),
            capabilities.graphicsEnumerationError.empty()
                ? "normal"
                : "warning");
        setDebugValue(QStringLiteral("hardwareFingerprint"),
                      hardwareFingerprint,
                      capabilities.hardwareFingerprint.empty()
                          ? "warning"
                          : "muted");
        setDebugValue(
            QStringLiteral("encoderProbeSource"),
            encoderProbeSource,
            capabilities.h264HardwareEncoderProbeSucceeded
                ? "good"
                : (capabilities.videoEncoderPreference == "software" ||
                   capabilities.videoEncoderPreference == "ffmpeg")
                      ? "normal"
                      : "warning");
        setDebugValue(
            QStringLiteral("audioDeviceModule"),
            audioDeviceModule,
            capabilities.audioDeviceModuleCreated ? "good"
                                                  : "warning");
        setDebugValue(QStringLiteral("cameraDevice"),
                      cameraDevice);
        setDebugValue(QStringLiteral("microphoneDevice"),
                      microphoneDevice);
        setDebugValue(QStringLiteral("speakerDevice"),
                      speakerDevice);
        setDebugValue(QStringLiteral("builtinEncoder"), builtinEncoder,
                      capabilities.hasH264Encoder ? "good" : "error");
        setDebugValue(QStringLiteral("desktopCapture"),
                      desktopCapture, "good");
        setDebugValue(QStringLiteral("encoderPreference"),
                      encoderPreference, "good");
        setDebugValue(QStringLiteral("encoderQuality"),
                      encoderQuality, "good");
        setDebugValue(QStringLiteral("hardwareEncoderCount"),
                      hardwareEncoderCount,
                      capabilities.h264HardwareEncoderAvailable
                          ? "good"
                          : "warning");
        setDebugValue(QStringLiteral("cpuNv12"), cpuNv12,
                      capabilities
                              .h264HardwareEncoderCpuNv12InputSupported
                          ? "good"
                          : "warning");
        setDebugValue(QStringLiteral("d3d11Encoder"), d3d11Encoder,
                      capabilities.h264HardwareEncoderWired
                          ? "good"
                          : "warning");
        setDebugValue(QStringLiteral("encoderFallback"),
                      encoderFallback,
                      capabilities.h264SoftwareEncoderWired
                          ? "good"
                          : capabilities.videoEncoderPreference ==
                                    "hardware"
                                ? "normal"
                                : "error");
        setDebugValue(QStringLiteral("encoderRuntime"),
                      encoderRuntimeDetails.join(
                          QStringLiteral("\n\n")));
        setDebugValue(
            QStringLiteral("encoderFallbackReason"),
            encoderFallbackReason,
            capabilities.videoEncoderLastFallbackReason.empty()
                ? "muted"
                : "warning");
        setDebugValue(QStringLiteral("decoder"), decoder,
                      capabilities.hasH264Decoder ? "good" : "error");
        setDebugValue(QStringLiteral("decoderPreference"),
                      decoderPreference, "good");
        setDebugValue(
            QStringLiteral("mfDecoderType"), mfDecoderType,
            capabilities.mfD3D11DecoderHardware
                ? "good"
                : capabilities.mfD3D11DecoderSoftware
                      ? "warning"
                      : "normal");
        setDebugValue(QStringLiteral("mfDecoderName"),
                      mfDecoderName,
                      capabilities.mfD3D11DecoderConfigured
                          ? "good"
                          : "muted");
        setDebugValue(QStringLiteral("d3d11Output"), d3d11Output,
                      capabilities.d3d11NativeDecoderOutput
                          ? "good"
                          : "warning");
        setDebugValue(QStringLiteral("mfDecoderAsync"),
                      mfDecoderAsync,
                      capabilities.mfD3D11DecoderHardware
                          ? "good"
                          : "muted");
        setDebugValue(QStringLiteral("softwareFallback"),
                      softwareFallback,
                      capabilities.ffmpegSoftwareH264Decoder
                          ? "good"
                          : "error");
        setDebugValue(
            QStringLiteral("mfDecoderError"), mfDecoderError,
            capabilities.mfD3D11DecoderError.empty()
                ? "muted"
                : "warning");
        setDebugValue(QStringLiteral("encoderDetails"),
                      encoderDetails.join(QStringLiteral("\n\n")));
        setDebugValue(QStringLiteral("membership"), membership,
                      snapshot.room.membership ==
                              RoomMembershipState::kActive
                          ? "good"
                          : "normal");
        setDebugValue(QStringLiteral("roomId"), roomId);
        setDebugValue(QStringLiteral("members"), members);
        setDebugValue(QStringLiteral("screenSharer"), screenSharer);
        setDebugValue(
            QStringLiteral("screenShareGeneration"),
            QString::number(snapshot.screenShare.generation));
        setDebugValue(QStringLiteral("controller"), controller);
        setDebugValue(QStringLiteral("controlGrant"), controlGrant,
                      snapshot.room.activeControllerDeviceId.empty()
                          ? "muted"
                          : snapshot.roomControlGrantActive
                                ? "good" : "warning");
        setDebugValue(QStringLiteral("peerSummary"), peerSummary,
                      activePeers > 0 ? "good" : "normal");
        setDebugValue(QStringLiteral("peerDetails"),
                      peerDetails.join(QStringLiteral("\n\n")));
        setDebugValue(QStringLiteral("clipboardState"), clipboardState,
                      clipboard.enabled && clipboard.sessionActive
                          ? "good" : "normal");
        setDebugValue(QStringLiteral("clipboardPeer"), clipboardPeer);
        setDebugValue(QStringLiteral("clipboardTransfer"),
                      clipboardTransfer,
                      clipboard.transferActive ? "warning" : "muted");
        setDebugValue(QStringLiteral("clipboardLast"), clipboardLast);
        setDebugValue(QStringLiteral("clipboardCounters"),
                      clipboardCounters);
        setDebugValue(QStringLiteral("clipboardError"), clipboardError,
                      clipboard.lastErrorMessage.empty() ? "muted"
                                                         : "warning");
        setDebugValue(QStringLiteral("sessionError"), sessionError,
                      snapshot.error.message.empty() ? "muted"
                                                    : "error");
        setDebugValue(QStringLiteral("roomError"), roomError,
                      snapshot.room.errorMessage.empty() ? "muted"
                                                         : "error");
        if (diagnosticsCopyTextRequested_) {
        const QSettings encoderSelectionSettings;
        const QString encoderSelectionCaptureBackend =
            encoderSelectionSettings.value(
                QString::fromLatin1(kDesktopCaptureBackendSetting),
                QStringLiteral("native_dxgi")).toString();
        const QString encoderSelectionX264Preset =
            encoderSelectionSettings.value(
                QString::fromLatin1(kFfmpegX264PresetSetting),
                QStringLiteral("medium")).toString();
        const bool encoderSelectionCurrent =
            encoderSelectionSettings.value(
                QString::fromLatin1(kEncoderBenchmarkCompletedSetting),
                false).toBool() &&
            encoderSelectionSettings.value(
                QString::fromLatin1(kEncoderBenchmarkPassedSetting),
                false).toBool() &&
            encoderSelectionSettings.value(
                QString::fromLatin1(kEncoderBenchmarkPolicyVersionSetting),
                0).toInt() == kEncoderBenchmarkPolicyVersion &&
            encoderSelectionSettings.value(
                QString::fromLatin1(kEncoderHardwareFingerprintSetting))
                .toString() == hardwareFingerprint &&
            encoderSelectionSettings.value(
                QString::fromLatin1(kEncoderCaptureBackendSetting))
                .toString() == encoderSelectionCaptureBackend &&
            encoderSelectionSettings.value(
                QString::fromLatin1(kEncoderX264PresetSetting))
                .toString() == encoderSelectionX264Preset;
        const QString automaticEncoderSelection = encoderSelectionCurrent
            ? QStringLiteral("%1 [%2]")
                  .arg(encoderSelectionSettings.value(
                           QString::fromLatin1(kBestEncoderNameSetting))
                           .toString(),
                       encoderSelectionSettings.value(
                           QString::fromLatin1(kBestEncoderIdSetting))
                           .toString())
            : QStringLiteral("等待当前采集方式的性能检测");
        QStringList copyLines;
        copyLines
            << QStringLiteral("RLink 调试信息")
            << QStringLiteral("================")
            << QString()
            << QStringLiteral("【设备与信令】")
            << QStringLiteral("本机设备 ID：%1").arg(deviceId)
            << QStringLiteral("信令连接：%1").arg(signaling)
            << QStringLiteral("WebRTC 运行时：%1").arg(webRtc)
            << QString()
            << QStringLiteral("【兼容性环境】")
            << QStringLiteral("系统与架构：%1")
                   .arg(systemEnvironment)
            << QStringLiteral("会话类型：%1")
                   .arg(sessionEnvironment)
            << QStringLiteral("图形适配器：%1")
                   .arg(graphicsAdapters.join(
                       QStringLiteral("；")))
            << QStringLiteral("硬件能力指纹：%1")
                   .arg(hardwareFingerprint)
            << QStringLiteral("编码能力探测：%1")
                   .arg(encoderProbeSource)
            << QStringLiteral("WebRTC 音频模块：%1")
                   .arg(audioDeviceModule.replace(
                       QLatin1Char('\n'), QStringLiteral("；")))
            << QStringLiteral("摄像头：%1")
                   .arg(QString(cameraDevice).replace(
                       QLatin1Char('\n'), QStringLiteral("；")))
            << QStringLiteral("麦克风：%1")
                   .arg(QString(microphoneDevice).replace(
                       QLatin1Char('\n'), QStringLiteral("；")))
            << QStringLiteral("扬声器：%1")
                   .arg(QString(speakerDevice).replace(
                       QLatin1Char('\n'), QStringLiteral("；")))
            << QString()
            << QStringLiteral("【媒体能力】")
            << QStringLiteral("H264 编码链路：%1")
                   .arg(builtinEncoder)
            << QStringLiteral("屏幕采集模式：%1")
                   .arg(desktopCapture)
            << QStringLiteral("视频编码模式：%1")
                   .arg(encoderPreference)
            << QStringLiteral("编码质量：%1")
                   .arg(encoderQuality)
            << QStringLiteral("自动编码实测选择：%1")
                   .arg(automaticEncoderSelection)
            << QStringLiteral("硬件编码器：%1")
                   .arg(hardwareEncoderCount)
            << QStringLiteral("CPU NV12 输入：%1").arg(cpuNv12)
            << QStringLiteral("D3D11 硬件编码：%1")
                   .arg(d3d11Encoder)
            << QStringLiteral("FFmpeg 硬件编码：%1")
                   .arg(ffmpegHardwareEncoder)
            << QStringLiteral("软件编码链路：%1")
                   .arg(encoderFallback)
            << QStringLiteral("当前编码实例：%1")
                   .arg(encoderRuntimeDetails.join(
                       QStringLiteral("；")))
            << QStringLiteral("最近回退原因：%1")
                   .arg(encoderFallbackReason)
            << QStringLiteral("H264 解码：%1").arg(decoder)
            << QStringLiteral("视频解码模式：%1")
                   .arg(decoderPreference)
            << QStringLiteral("MF D3D11 解码类型：%1")
                   .arg(mfDecoderType)
            << QStringLiteral("MF 解码器实例：%1")
                   .arg(mfDecoderName)
            << QStringLiteral("D3D11 原生输出：%1")
                   .arg(d3d11Output)
            << QStringLiteral("硬件 MFT 驱动：%1")
                   .arg(mfDecoderAsync)
            << QStringLiteral("FFmpeg 软件解码：%1")
                   .arg(softwareFallback)
            << QStringLiteral("MF 解码检测说明：%1")
                   .arg(mfDecoderError)
            << encoderDetails
            << QString()
            << QStringLiteral("【房间状态】")
            << QStringLiteral("状态：%1").arg(membership)
            << QStringLiteral("房间 ID：%1").arg(roomId)
            << QStringLiteral("成员与席位：%1").arg(members)
            << QStringLiteral("当前主机器：%1").arg(screenSharer)
            << QStringLiteral("当前控制者：%1").arg(controller)
            << QStringLiteral("控制授权令牌：%1").arg(controlGrant)
            << QString()
            << QStringLiteral("【成员连接】")
            << peerSummary
            << peerDetails
            << QString()
            << QStringLiteral("【远程粘贴】")
            << QStringLiteral("状态：%1").arg(clipboardState)
            << QStringLiteral("成员对：%1").arg(clipboardPeer)
            << QStringLiteral("当前传输：%1").arg(clipboardTransfer)
            << QStringLiteral("最近项目：%1").arg(clipboardLast)
            << QStringLiteral("累计统计：%1").arg(clipboardCounters)
            << QStringLiteral("最近错误：%1").arg(clipboardError)
            << QString()
            << QStringLiteral("【最近错误】")
            << QStringLiteral("会话错误：%1").arg(sessionError)
            << QStringLiteral("房间错误：%1").arg(roomError);
        debugCopyText_ = copyLines.join(QLatin1Char('\n'));

        QStringList mediaCopyLines;
        mediaCopyLines
            << QStringLiteral("RLink 媒体能力")
            << QStringLiteral("================")
            << QStringLiteral("H264 编码链路：%1")
                   .arg(builtinEncoder)
            << QStringLiteral("屏幕采集模式：%1")
                   .arg(desktopCapture)
            << QStringLiteral("视频编码模式：%1")
                   .arg(encoderPreference)
            << QStringLiteral("编码质量：%1")
                   .arg(encoderQuality)
            << QStringLiteral("自动编码实测选择：%1")
                   .arg(automaticEncoderSelection)
            << QStringLiteral("硬件编码器：%1")
                   .arg(hardwareEncoderCount)
            << QStringLiteral("CPU NV12 输入：%1").arg(cpuNv12)
            << QStringLiteral("D3D11 硬件编码：%1")
                   .arg(d3d11Encoder)
            << QStringLiteral("FFmpeg 硬件编码：%1")
                   .arg(ffmpegHardwareEncoder)
            << QStringLiteral("软件编码链路：%1")
                   .arg(encoderFallback)
            << QStringLiteral("当前编码实例：%1")
                   .arg(encoderRuntimeDetails.join(
                       QStringLiteral("；")))
            << QStringLiteral("最近回退原因：%1")
                   .arg(encoderFallbackReason)
            << QStringLiteral("H264 解码：%1").arg(decoder)
            << QStringLiteral("视频解码模式：%1")
                   .arg(decoderPreference)
            << QStringLiteral("MF D3D11 解码类型：%1")
                   .arg(mfDecoderType)
            << QStringLiteral("MF 解码器实例：%1")
                   .arg(mfDecoderName)
            << QStringLiteral("D3D11 原生输出：%1")
                   .arg(d3d11Output)
            << QStringLiteral("硬件 MFT 驱动：%1")
                   .arg(mfDecoderAsync)
            << QStringLiteral("FFmpeg 软件解码：%1")
                   .arg(softwareFallback)
            << QStringLiteral("MF 解码检测说明：%1")
                   .arg(mfDecoderError)
            << encoderDetails;
        mediaDebugCopyText_ =
            mediaCopyLines.join(QLatin1Char('\n'));
        }
    }
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

