// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ControllerMainWindow.h"
#include "ControllerMainWindowSupport.h"

#include <QApplication>
#include <QComboBox>
#include <QDateTime>
#include <QDir>
#include <QFontMetrics>
#include <QProcess>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSignalBlocker>
#include <QStandardItemModel>
#include <QStringList>
#include <QTimer>
#include <QThread>
#include <QMetaObject>
#include <chrono>
#include <filesystem>
#include <utility>
#include <algorithm>
#include "RemoteCDialog.h"
#include "FileTransferWindow.h"
#include "RemoteSessionWindow.h"
#include "RemoteCToast.h"
#include "src/apps/remote/ClipboardController.h"
#include "src/apps/remote/EncoderBenchmarkProfileCache.h"
#include "src/apps/remote/FileTransferController.h"
#include "src/apps/remote/ISessionMediaAccess.h"
#include "src/platform/win/FfmpegHardwareH264Encoder.h"
#include "src/platform/win/H264EncoderBenchmark.h"
#include "src/platform/win/WindowsHardwareFingerprint.h"
#include "pages/DirectConnectPage.h"
#include "pages/SettingsPage.h"

namespace remote::controller {
using namespace detail;

bool ControllerMainWindow::InitializeEngine()
{
    if (!engine_) {
        return false;
    }
    engine_->SetObserver(this);
    const auto startResult = engine_->Start();
    if (!startResult.accepted) {
        PersistHardwareCapabilityCache();
        directConnectPage_->SetActionEnabled(false);
        SetRuntimeStatus(QStringLiteral("● WebRTC 初始化失败"),
                         QStringLiteral("#d14343"));
        directConnectPage_->SetDecoderStatus(
            QStringLiteral("● 视频能力不可用"),
            QStringLiteral("#d14343"));
        return false;
    }
    return true;
}

void ControllerMainWindow::ApplyEngineInitializationState(
    const SessionEngineSnapshot& snapshot)
{
    if (snapshot.state != SessionEngineState::kStarting &&
        snapshot.state != SessionEngineState::kReady &&
        snapshot.state != SessionEngineState::kFailed) {
        return;
    }
    if (engineInitializationUiState_ == snapshot.state) {
        return;
    }
    engineInitializationUiState_ = snapshot.state;

    if (snapshot.state == SessionEngineState::kStarting) {
        SetRuntimeStatus(QStringLiteral("● WebRTC 正在初始化"),
                         QStringLiteral("#a66b12"));
        directConnectPage_->SetDecoderStatus(
            QStringLiteral("● 正在检测视频能力"),
            QStringLiteral("#a66b12"));
        directConnectPage_->SetActionEnabled(false);
        return;
    }

    if (snapshot.state == SessionEngineState::kFailed) {
        // A valid negative hardware-encoder enumeration is still useful.
        // Persist it even when an explicit hardware-only preference makes
        // the runtime reject startup.
        PersistHardwareCapabilityCache();
        directConnectPage_->SetActionEnabled(false);
        SetRuntimeStatus(QStringLiteral("● WebRTC 初始化失败"),
                         QStringLiteral("#d14343"));
        directConnectPage_->SetDecoderStatus(
            QStringLiteral("● 视频能力不可用"),
            QStringLiteral("#d14343"));
        return;
    }

    const auto report = engine_->Capabilities();
    PersistHardwareCapabilityCache();
    SetRuntimeStatus(QStringLiteral("● WebRTC 运行时就绪"),
                     QStringLiteral("#138b57"));
    if (report.mfD3D11DecoderHardware) {
        directConnectPage_->SetDecoderStatus(
            QStringLiteral("● MF/D3D11 H264 硬件解码可用"),
            QStringLiteral("#138b57"));
    } else if (report.mfD3D11DecoderSoftware) {
        directConnectPage_->SetDecoderStatus(
            QStringLiteral("● MF H264 软件解码（D3D11 输出）"),
            QStringLiteral("#a66b12"));
    } else if (report.ffmpegSoftwareH264Decoder) {
        directConnectPage_->SetDecoderStatus(
            QStringLiteral("● FFmpeg H264 软件解码可用"),
            QStringLiteral("#a66b12"));
    } else if (report.hasH264Decoder) {
        directConnectPage_->SetDecoderStatus(
            QStringLiteral("● H264 解码可用"),
            QStringLiteral("#a66b12"));
    } else {
        directConnectPage_->SetDecoderStatus(
            QStringLiteral("● H264 解码不可用"),
            QStringLiteral("#d14343"));
    }
}

void ControllerMainWindow::PersistHardwareCapabilityCache()
{
    if (!engine_) {
        return;
    }
    const auto report = engine_->Capabilities();
    if (report.hardwareFingerprint.empty() ||
        !report.h264HardwareEncoderProbeSucceeded ||
        ((report.videoEncoderPreference == "software" ||
          report.videoEncoderPreference == "ffmpeg") &&
         !report.h264HardwareEncoderProbeFromCache)) {
        return;
    }

    QStringList descriptions;
    for (const auto& value :
         report.h264HardwareEncoderDescriptions) {
        descriptions.push_back(QString::fromStdString(value));
    }
    QStringList warnings;
    for (const auto& value :
         report.h264HardwareEncoderWarnings) {
        warnings.push_back(QString::fromStdString(value));
    }

    QSettings settings;
    settings.setValue(
        QStringLiteral("media/hardwareProbe/fingerprint"),
        QString::fromStdString(report.hardwareFingerprint));
    settings.setValue(
        QStringLiteral("media/hardwareProbe/valid"), true);
    settings.setValue(
        QStringLiteral(
            "media/hardwareProbe/encoderAvailable"),
        report.h264HardwareEncoderAvailable);
    settings.setValue(
        QStringLiteral(
            "media/hardwareProbe/cpuNv12InputSupported"),
        report.h264HardwareEncoderCpuNv12InputSupported);
    settings.setValue(
        QStringLiteral(
            "media/hardwareProbe/d3d11InputCandidate"),
        report.h264HardwareEncoderD3D11InputCandidate);
    settings.setValue(
        QStringLiteral("media/hardwareProbe/encoderCount"),
        report.h264HardwareEncoderCount);
    settings.setValue(
        QStringLiteral("media/hardwareProbe/descriptions"),
        descriptions);
    settings.setValue(
        QStringLiteral("media/hardwareProbe/warnings"),
        warnings);
}

void ControllerMainWindow::ApplyVideoPipelineSettingsFromUi(
    bool showFeedback,
    const QString& changedSettingName,
    bool refreshHardwareEnvironment)
{
    if (videoPipelineSettingsBusy_) {
        if (!showFeedback) {
            videoPipelineSettingsApplyPending_ = true;
        }
        return;
    }
    if (!SettingsControls().desktopCaptureSelector || !SettingsControls().videoEncoderSelector ||
        !SettingsControls().ffmpegHardwareBackendSelector ||
        !SettingsControls().ffmpegX264PresetSelector || !SettingsControls().videoDecoderSelector) {
        return;
    }
    if (!sessionMedia_) {
        return;
    }

    const QString captureValue =
        SettingsControls().desktopCaptureSelector->currentData().toString();
    const QString encoderValue =
        SettingsControls().videoEncoderSelector->currentData().toString();
    const QString backendValue =
        SettingsControls().ffmpegHardwareBackendSelector->currentData().toString();
    const QString qualityValue =
        SettingsControls().ffmpegX264PresetSelector->currentData().toString();
    const QString decoderValue =
        SettingsControls().videoDecoderSelector->currentData().toString();
    const QSettings currentSettings;
    QString automaticEncoderId;
    const QString captureBackend = captureValue;
    const QJsonObject encoderProfile = app::LoadEncoderBenchmarkProfile(
        currentSettings,
        HardwareFingerprintForUi(refreshHardwareEnvironment),
        captureBackend, qualityValue, kEncoderBenchmarkPolicyVersion);
    if (encoderProfile.value(QStringLiteral("passed")).toBool()) {
        automaticEncoderId = encoderProfile.value(
            QStringLiteral("bestEncoderId")).toString();
    } else if (encoderValue == QStringLiteral("auto")) {
        automaticEncoderId = QString::fromLatin1(
            kAutomaticEncoderFfmpegHardwareDefault);
    }
    const auto result = sessionMedia_->ApplyVideoPipelinePreferences(
        DesktopCaptureImplementationFromSetting(captureValue),
        VideoEncoderPreferenceFromSetting(encoderValue),
        EncoderQualityFromSetting(qualityValue),
        FfmpegHardwareBackendFromSetting(backendValue),
        automaticEncoderId.toStdString(),
        VideoDecoderPreferenceFromSetting(decoderValue));
    if (!result.accepted) {
        videoPipelineSettingsApplyPending_ = false;
        QSettings saved;
        const QSignalBlocker captureBlocker(SettingsControls().desktopCaptureSelector);
        const QSignalBlocker encoderBlocker(SettingsControls().videoEncoderSelector);
        const QSignalBlocker backendBlocker(
            SettingsControls().ffmpegHardwareBackendSelector);
        const QSignalBlocker qualityBlocker(
            SettingsControls().ffmpegX264PresetSelector);
        const QSignalBlocker decoderBlocker(SettingsControls().videoDecoderSelector);
        const auto restore = [&saved](QComboBox* selector,
                                     const char* key,
                                     const QString& fallback) {
            const int index = selector->findData(saved.value(
                QString::fromLatin1(key), fallback).toString());
            if (index >= 0) {
                selector->setCurrentIndex(index);
            }
        };
        restore(SettingsControls().desktopCaptureSelector, kDesktopCaptureBackendSetting,
                QStringLiteral("native_dxgi"));
        restore(SettingsControls().videoEncoderSelector, kVideoEncoderPreferenceSetting,
                QStringLiteral("auto"));
        restore(SettingsControls().ffmpegHardwareBackendSelector,
                kFfmpegHardwareBackendSetting,
                QStringLiteral("auto"));
        restore(SettingsControls().ffmpegX264PresetSelector,
                kFfmpegX264PresetSetting,
                QStringLiteral("medium"));
        restore(SettingsControls().videoDecoderSelector, kVideoDecoderPreferenceSetting,
                QStringLiteral("auto"));
        UpdateVideoPipelineSettingsAvailability(engine_->Snapshot());
        if (showFeedback) {
            RemoteCToast::Show(
                this,
                QStringLiteral("无法应用媒体设置：%1")
                    .arg(QString::fromStdString(result.errorMessage)),
                RemoteCToast::Tone::kError);
        }
        return;
    }

    QSettings settings;
    settings.setValue(QString::fromLatin1(
                          kDesktopCaptureBackendSetting),
                      captureValue);
    settings.setValue(QString::fromLatin1(
                          kVideoEncoderPreferenceSetting),
                      encoderValue);
    settings.setValue(QString::fromLatin1(
                          kFfmpegHardwareBackendSetting),
                      backendValue);
    settings.setValue(QString::fromLatin1(kFfmpegX264PresetSetting),
                      qualityValue);
    settings.setValue(QString::fromLatin1(
                          kVideoDecoderPreferenceSetting),
                      decoderValue);
    videoPipelineSettingsApplyPending_ = false;
    if (showFeedback) {
        RemoteCToast::Show(
            this,
            changedSettingName.isEmpty()
                ? QStringLiteral(
                      "远程桌面媒体设置已更新，将从下一次共享开始生效")
                : QStringLiteral(
                      "%1已更新，将从下一次共享开始生效")
                      .arg(changedSettingName),
            RemoteCToast::Tone::kSuccess);
    }
}

void ControllerMainWindow::UpdateVideoPipelineSettingsAvailability(
    const SessionEngineSnapshot& snapshot)
{
    if (!SettingsControls().desktopCaptureSelector || !SettingsControls().videoEncoderSelector ||
        !SettingsControls().ffmpegHardwareBackendSelector ||
        !SettingsControls().ffmpegX264PresetSelector || !SettingsControls().videoDecoderSelector) {
        return;
    }
    const bool cameraVideoActive =
        snapshot.media.localCamera != LocalCameraState::kOff ||
        std::any_of(
            snapshot.room.members.begin(),
            snapshot.room.members.end(),
            [](const RoomMemberSnapshot& member) {
                return member.cameraPublishing;
            });
    const bool directVideoActive =
        snapshot.state == SessionEngineState::kStarting ||
        snapshot.state == SessionEngineState::kConnecting ||
        snapshot.state == SessionEngineState::kAwaitingLocalApproval ||
        snapshot.state == SessionEngineState::kActive ||
        snapshot.state == SessionEngineState::kStopping;
    const bool wasBusy = videoPipelineSettingsBusy_;
    videoPipelineSettingsBusy_ =
        snapshot.room.screenShareState != RoomScreenShareState::kIdle ||
        cameraVideoActive || directVideoActive;
    const bool enabled = !videoPipelineSettingsBusy_;
    SettingsControls().desktopCaptureSelector->setEnabled(enabled);
    SettingsControls().videoEncoderSelector->setEnabled(enabled);
    SettingsControls().ffmpegX264PresetSelector->setEnabled(enabled);
    SettingsControls().videoDecoderSelector->setEnabled(enabled);
    SettingsControls().ffmpegHardwareBackendSelector->setEnabled(
        enabled &&
        SettingsControls().ffmpegHardwareBackendSelector->count() > 1 &&
        SettingsControls().videoEncoderSelector->currentData().toString() ==
            QStringLiteral("ffmpeg_hardware"));
    if (wasBusy && !videoPipelineSettingsBusy_ &&
        videoPipelineSettingsApplyPending_) {
        QTimer::singleShot(0, this, [this] {
            ApplyVideoPipelineSettingsFromUi(false);
        });
    }
}

void ControllerMainWindow::UpdateLocalMediaDevicesUi(
    const SessionEngineSnapshot& snapshot)
{
    if (!SettingsControls().cameraDeviceSelector ||
        !SettingsControls().microphoneDeviceSelector ||
        !SettingsControls().speakerDeviceSelector) {
        return;
    }
    const auto& media = snapshot.media.localMediaDevices;
    const bool deviceListChanged =
        !mediaDeviceSnapshotSeen_ || media.revision != mediaDeviceRevision_;
    const bool activityChanged =
        !mediaActivitySnapshotSeen_ ||
        displayedMicrophoneState_ != snapshot.media.localMicrophone ||
        displayedRoomAudioPlaybackMuted_ !=
            snapshot.media.roomAudioPlaybackMuted;
    if (!deviceListChanged && !activityChanged) {
        return;
    }
    mediaActivitySnapshotSeen_ = true;
    displayedMicrophoneState_ = snapshot.media.localMicrophone;
    displayedRoomAudioPlaybackMuted_ =
        snapshot.media.roomAudioPlaybackMuted;
    const bool hadPreviousMediaSnapshot =
        mediaDeviceSnapshotSeen_;
    mediaDeviceSnapshotSeen_ = true;
    mediaDeviceRevision_ = media.revision;

    const auto deviceName =
        [](const MediaDeviceCategorySnapshot& category,
           const std::string& deviceId,
           const QString& emptyName) {
            if (deviceId.empty()) {
                return emptyName;
            }
            if (deviceId == category.activeDeviceId &&
                !category.activeDeviceName.empty()) {
                const QString resolved = QString::fromStdString(
                    category.activeDeviceName);
                return deviceId == kSystemDefaultMediaDeviceId
                    ? QStringLiteral("%1（系统默认）").arg(resolved)
                    : resolved;
            }
            if (deviceId == kSystemDefaultMediaDeviceId) {
                return category.activeDeviceName.empty()
                    ? QStringLiteral("跟随系统默认")
                    : QStringLiteral("跟随系统默认（当前：%1）")
                          .arg(QString::fromStdString(
                              category.activeDeviceName));
            }
            const auto found = std::find_if(
                category.devices.begin(),
                category.devices.end(),
                [&deviceId](const MediaDeviceDescriptor& device) {
                    return device.id == deviceId;
                });
            return found == category.devices.end()
                ? QStringLiteral("已断开的设备")
                : QString::fromStdString(found->name);
        };
    const auto rebuild =
        [&deviceName, &media](
            QComboBox* selector,
            const MediaDeviceCategorySnapshot& category) {
            const QSignalBlocker blocker(selector);
            selector->clear();
            const bool defaultActive =
                category.activeDeviceId ==
                kSystemDefaultMediaDeviceId;
            selector->addItem(
                defaultActive
                    ? (category.activeDeviceName.empty()
                           ? QStringLiteral(
                                 "跟随系统默认（当前）")
                           : QStringLiteral(
                                 "跟随系统默认（当前：%1）")
                                 .arg(QString::fromStdString(
                                     category.activeDeviceName)))
                    : QStringLiteral("跟随系统默认"),
                QStringLiteral("default"));
            for (const auto& device : category.devices) {
                QString label =
                    QString::fromStdString(device.name);
                if (device.id == category.activeDeviceId) {
                    label += QStringLiteral("（当前）");
                }
                selector->addItem(
                    label,
                    QString::fromStdString(device.id));
            }
            const QString preferredId =
                QString::fromStdString(
                    category.preferredDeviceId.empty()
                        ? std::string(
                              kSystemDefaultMediaDeviceId)
                        : category.preferredDeviceId);
            int preferredIndex =
                selector->findData(preferredId);
            if (preferredIndex < 0) {
                selector->addItem(
                    QStringLiteral("首选设备已断开"),
                    preferredId);
                preferredIndex = selector->count() - 1;
            }
            selector->setCurrentIndex(preferredIndex);
            selector->setEnabled(
                !media.refreshing &&
                category.state !=
                MediaDeviceSelectionState::kSwitching);
            selector->setToolTip(deviceName(
                category, category.activeDeviceId,
                QStringLiteral("当前未启用")));
        };

    if (deviceListChanged) {
        rebuild(SettingsControls().cameraDeviceSelector, media.camera);
        rebuild(SettingsControls().microphoneDeviceSelector, media.microphone);
        rebuild(SettingsControls().speakerDeviceSelector, media.speaker);
    }

    const auto completePending =
        [this](
            const MediaDeviceCategorySnapshot& category,
            QString& pendingId,
            const char* settingKey,
            const QString& deviceLabel) {
            if (pendingId.isEmpty()) {
                return;
            }
            const QString preferred =
                QString::fromStdString(
                    category.preferredDeviceId);
            if (category.state ==
                    MediaDeviceSelectionState::kReady &&
                preferred == pendingId) {
                QSettings settings;
                settings.setValue(
                    QString::fromLatin1(settingKey),
                    pendingId);
                RemoteCToast::Show(
                    this,
                    QStringLiteral("%1已切换")
                        .arg(deviceLabel),
                    RemoteCToast::Tone::kSuccess);
                pendingId.clear();
                return;
            }
            if (category.state ==
                    MediaDeviceSelectionState::kFailed ||
                category.state ==
                    MediaDeviceSelectionState::kUnavailable) {
                const QString error =
                    category.errorMessage.empty()
                        ? QStringLiteral("设备不可用")
                        : QString::fromStdString(
                              category.errorMessage);
                RemoteCToast::Show(
                    this,
                    QStringLiteral("%1切换失败：%2")
                        .arg(deviceLabel, error),
                    RemoteCToast::Tone::kError);
                pendingId.clear();
                return;
            }
            if (category.state !=
                    MediaDeviceSelectionState::kSwitching &&
                preferred != pendingId) {
                RemoteCToast::Show(
                    this,
                    QStringLiteral("%1切换已取消")
                        .arg(deviceLabel),
                    RemoteCToast::Tone::kInformation);
                pendingId.clear();
            }
        };
    const bool cameraHadPending =
        !pendingCameraDeviceId_.isEmpty();
    const bool microphoneHadPending =
        !pendingMicrophoneDeviceId_.isEmpty();
    const bool speakerHadPending =
        !pendingSpeakerDeviceId_.isEmpty();
    completePending(
        media.camera, pendingCameraDeviceId_,
        kCameraDeviceSetting, QStringLiteral("摄像头"));
    completePending(
        media.microphone, pendingMicrophoneDeviceId_,
        kMicrophoneDeviceSetting, QStringLiteral("麦克风"));
    completePending(
        media.speaker, pendingSpeakerDeviceId_,
        kSpeakerDeviceSetting, QStringLiteral("扬声器"));

    if (SettingsControls().refreshMediaDevicesButton) {
        const bool switching =
            media.camera.state ==
                MediaDeviceSelectionState::kSwitching ||
            media.microphone.state ==
                MediaDeviceSelectionState::kSwitching ||
            media.speaker.state ==
                MediaDeviceSelectionState::kSwitching;
        SettingsControls().refreshMediaDevicesButton->setEnabled(
            !media.refreshing && !switching);
    }
    if (SettingsControls().mediaDeviceStatusLabel) {
        QStringList operations;
        if (media.refreshing) {
            operations << QStringLiteral("正在刷新设备");
        }
        if (media.camera.state ==
            MediaDeviceSelectionState::kSwitching) {
            operations << QStringLiteral("正在切换摄像头");
        }
        if (media.microphone.state ==
            MediaDeviceSelectionState::kSwitching) {
            operations << QStringLiteral("正在切换麦克风");
        }
        if (media.speaker.state ==
            MediaDeviceSelectionState::kSwitching) {
            operations << QStringLiteral("正在切换扬声器");
        }
        if (!operations.isEmpty()) {
            SettingsControls().mediaDeviceStatusLabel->setText(
                operations.join(QStringLiteral("   ·   ")) +
                QStringLiteral("，请稍候…"));
        } else {
            QStringList lines;
            lines << QStringLiteral("摄像头：%1")
                         .arg(deviceName(
                             media.camera,
                             media.camera.activeDeviceId,
                             QStringLiteral("尚未开启")));
            const QString microphoneName = deviceName(
                media.microphone,
                media.microphone.activeDeviceId,
                QStringLiteral("未找到可用设备"));
            switch (snapshot.media.localMicrophone) {
            case LocalMicrophoneState::kStarting:
                lines << QStringLiteral("正在开启麦克风：%1")
                             .arg(microphoneName);
                break;
            case LocalMicrophoneState::kPublishing:
                lines << QStringLiteral("麦克风正在使用：%1")
                             .arg(microphoneName);
                break;
            case LocalMicrophoneState::kStopping:
                lines << QStringLiteral("正在关闭麦克风：%1")
                             .arg(microphoneName);
                break;
            case LocalMicrophoneState::kFailed:
                lines << QStringLiteral(
                             "麦克风不可用（选择：%1）")
                             .arg(microphoneName);
                break;
            default:
                lines << QStringLiteral(
                             "麦克风已关闭（开启后使用：%1）")
                             .arg(microphoneName);
                break;
            }
            const QString speakerName = deviceName(
                media.speaker,
                media.speaker.activeDeviceId,
                QStringLiteral("未找到可用设备"));
            lines << (snapshot.media.roomAudioPlaybackMuted
                ? QStringLiteral("扬声器已静音（恢复后输出到：%1）")
                      .arg(speakerName)
                : QStringLiteral("声音输出到：%1")
                      .arg(speakerName));
            SettingsControls().mediaDeviceStatusLabel->setText(
                lines.join(QStringLiteral("   ·   ")));
        }
    }

    const auto unavailableText =
        [](const MediaDeviceCategorySnapshot& category,
           const QString& label) {
            if (category.state !=
                    MediaDeviceSelectionState::kUnavailable &&
                category.state !=
                    MediaDeviceSelectionState::kFailed) {
                return QString{};
            }
            return QStringLiteral("%1不可用，请重新选择设备")
                .arg(label);
        };
    QStringList deviceErrors;
    if (!cameraHadPending) {
        const QString error =
            unavailableText(media.camera, QStringLiteral("摄像头"));
        if (!error.isEmpty()) {
            deviceErrors << error;
        }
    }
    if (!microphoneHadPending) {
        const QString error = unavailableText(
            media.microphone, QStringLiteral("麦克风"));
        if (!error.isEmpty()) {
            deviceErrors << error;
        }
    }
    if (!speakerHadPending) {
        const QString error =
            unavailableText(media.speaker, QStringLiteral("扬声器"));
        if (!error.isEmpty()) {
            deviceErrors << error;
        }
    }
    const QString errorKey = deviceErrors.join(QLatin1Char('|'));
    if (errorKey.isEmpty()) {
        lastMediaDeviceErrorKey_.clear();
    } else if (!hadPreviousMediaSnapshot) {
        lastMediaDeviceErrorKey_ = errorKey;
    } else if (hadPreviousMediaSnapshot &&
               errorKey != lastMediaDeviceErrorKey_) {
        lastMediaDeviceErrorKey_ = errorKey;
        RemoteCToast::Show(
            this, deviceErrors.join(QStringLiteral("；")),
            RemoteCToast::Tone::kError);
    }
}
QString ControllerMainWindow::HardwareFingerprintForUi(bool refresh)
{
    // Presentation-only refreshes share the last UI environment sample.
    // Real capability checks and both benchmark boundaries request a fresh
    // query so GPU, driver and remote-session changes remain detectable.
    if (refresh || !uiHardwareFingerprintInitialized_) {
        uiHardwareFingerprint_ = QString::fromStdString(
            BuildWindowsHardwareFingerprint());
        uiHardwareFingerprintInitialized_ = true;
    }
    return uiHardwareFingerprint_;
}

void ControllerMainWindow::RefreshEncoderBenchmarkSummary(
    bool refreshHardwareEnvironment)
{
    if (!SettingsControls().encoderBenchmarkSummary) {
        return;
    }
    const QSettings settings;
    const QString fingerprint =
        HardwareFingerprintForUi(refreshHardwareEnvironment);
    const QString captureBackend = settings.value(
        QString::fromLatin1(kDesktopCaptureBackendSetting),
        QStringLiteral("native_dxgi")).toString();
    const QString ffmpegX264Preset = settings.value(
        QString::fromLatin1(kFfmpegX264PresetSetting),
        QStringLiteral("medium")).toString();
    const QJsonObject profile = app::LoadEncoderBenchmarkProfile(
        settings, fingerprint, captureBackend, ffmpegX264Preset,
        kEncoderBenchmarkPolicyVersion);
    if (profile.isEmpty()) {
        SettingsControls().encoderBenchmarkSummary->setText(AdaptBenchmarkHtmlForTheme(QStringLiteral(
            "<b style=\"color:#1769e8\">等待首次性能检测</b><br/>"
            "<span style=\"color:#667085\">检测结果按硬件环境、采集方式和编码质量分别缓存；自动模式在下次共享时应用。</span>")));
        return;
    }
    const bool passed = profile.value(QStringLiteral("passed")).toBool();
    const QString bestId =
        profile.value(QStringLiteral("bestEncoderId")).toString();
    const QString bestName =
        profile.value(QStringLiteral("bestEncoderName")).toString();
    const QString configuredMode = settings.value(
        QString::fromLatin1(kVideoEncoderPreferenceSetting),
        QStringLiteral("auto")).toString();
    const QString inputDescription =
        captureBackend == QStringLiteral("native_dxgi")
            ? QStringLiteral("自研 DXGI 输入")
            : QStringLiteral("libwebrtc CPU 输入");
    const QString testedAt = QDateTime::fromString(
        profile.value(QStringLiteral("testedAtUtc")).toString(),
        Qt::ISODateWithMs).toLocalTime().toString(
            QStringLiteral("yyyy-MM-dd HH:mm"));
    QString html;
    if (passed) {
        html = QStringLiteral(
            "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">"
            "<tr><td><span style=\"color:#667085\">实测最优</span>&nbsp;&nbsp;"
            "<b style=\"color:#172033\">%1</b></td>"
            "<td align=\"right\"><span style=\"color:#138b57\"><b>%2</b></span></td></tr>"
            "<tr><td colspan=\"2\" style=\"padding-top:5px;color:#667085\">"
            "%3 · %4；仅自动模式采用排名，手动选择不受影响。</td></tr></table>")
            .arg(bestName.toHtmlEscaped(),
                 configuredMode == QStringLiteral("auto")
                     ? QStringLiteral("自动采用")
                     : QStringLiteral("检测通过"),
                 inputDescription, testedAt);
    } else {
        // A failed overall result is still useful diagnostic evidence. Keep
        // every candidate and its partial measurements visible instead of
        // replacing the page with a single generic failure sentence.
        html = QStringLiteral(
            "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">"
            "<tr><td><b style=\"color:#a76508\">本次未产生自动选择结果</b></td>"
            "<td align=\"right\"><span style=\"color:#667085\"><b>已保留全部数据</b></span></td></tr>"
            "<tr><td colspan=\"2\" style=\"padding-top:5px;color:#667085\">"
            "%1 · %2；下方仍显示所有候选的实测数据和失败原因。</td></tr></table>")
            .arg(inputDescription, testedAt);
    }
    const QJsonArray candidates =
        profile.value(QStringLiteral("candidates")).toArray();
    QList<QJsonObject> ordered;
    for (const auto& item : candidates) {
        ordered.push_back(item.toObject());
    }
    std::stable_sort(ordered.begin(), ordered.end(),
        [&bestId](const auto& left, const auto& right) {
            const auto rank = [&bestId](const auto& item) {
                if (item.value(QStringLiteral("id")).toString() == bestId) {
                    return 0;
                }
                return item.value(QStringLiteral("passed")).toBool()
                    ? 1 : 2;
            };
            if (rank(left) != rank(right)) {
                return rank(left) < rank(right);
            }
            const double leftFps = left.value(
                QStringLiteral("inputFramesPerSecond")).toDouble();
            const double rightFps = right.value(
                QStringLiteral("inputFramesPerSecond")).toDouble();
            if (std::abs(leftFps - rightFps) > 0.25) {
                return leftFps > rightFps;
            }
            return left.value(QStringLiteral("score")).toDouble(1e9) <
                   right.value(QStringLiteral("score")).toDouble(1e9);
        });
    for (const auto& candidate : ordered) {
        const bool selected =
            candidate.value(QStringLiteral("id")).toString() == bestId;
        const bool candidatePassed =
            candidate.value(QStringLiteral("passed")).toBool();
        html += QStringLiteral(
            "<hr style=\"color:#e8ecf3\"/>"
            "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">"
            "<tr><td><b style=\"color:#24324b\">%1</b></td>"
            "<td align=\"right\"><span style=\"color:%2\"><b>%3</b></span></td></tr>"
            "<tr><td colspan=\"2\" style=\"padding-top:5px;color:#667085\">"
            "平均 %4 ms · P95 %5 ms · CPU %6 ms/帧 · PSNR %7 dB · 输出 %8/%9 帧 · 实测 %10 FPS%11</td></tr></table>")
            .arg(candidate.value(QStringLiteral("name")).toString()
                     .toHtmlEscaped(),
                 selected || candidatePassed
                     ? QStringLiteral("#138b57")
                     : QStringLiteral("#c12b37"),
                 selected ? QStringLiteral("最优")
                          : candidatePassed ? QStringLiteral("可用")
                                            : QStringLiteral("不可用"),
                 QString::number(candidate.value(
                     QStringLiteral("averageLatencyMs")).toDouble(), 'f', 2),
                 QString::number(candidate.value(
                     QStringLiteral("p95LatencyMs")).toDouble(), 'f', 2),
                 QString::number(candidate.value(
                     QStringLiteral("cpuTimePerFrameMs")).toDouble(), 'f', 2),
                 QString::number(candidate.value(
                     QStringLiteral("averageLumaPsnrDb")).toDouble(), 'f', 1),
                 QString::number(candidate.value(
                     QStringLiteral("encodedFrames")).toInt()),
                  QString::number(candidate.value(
                      QStringLiteral("submittedFrames")).toInt()),
                  QString::number(candidate.value(
                      QStringLiteral("inputFramesPerSecond")).toDouble(),
                      'f', 1),
                  candidate.value(QStringLiteral("error")).toString().isEmpty()
                     ? QString{}
                     : QStringLiteral("<br/><span style=\"color:#a76508\">%1</span>")
                           .arg(candidate.value(QStringLiteral("error"))
                               .toString().toHtmlEscaped()));
        const QString rateWarning =
            candidate.value(QStringLiteral("warning")).toString();
        if (!rateWarning.isEmpty()) {
            html.chop(QStringLiteral("</td></tr></table>").size());
            html += QStringLiteral(
                "<br/><span style=\"color:#a76508\">"
                "动态码率兼容性提示：%1</span></td></tr></table>")
                .arg(rateWarning.toHtmlEscaped());
        }
    }
    SettingsControls().encoderBenchmarkSummary->setText(AdaptBenchmarkHtmlForTheme(html));
}

void ControllerMainWindow::StartEncoderBenchmark(bool manualRequest)
{
    if (encoderBenchmarkProcess_) {
        if (manualRequest) {
            RemoteCToast::Show(
                this, QStringLiteral("编码器性能检测正在进行"),
                RemoteCToast::Tone::kInformation);
        }
        return;
    }
    if (decoderBenchmarkProcess_) {
        if (manualRequest) {
            RemoteCToast::Show(
                this, QStringLiteral("请等待解码器检测完成"),
                RemoteCToast::Tone::kInformation);
        } else {
            QTimer::singleShot(std::chrono::seconds(10), this,
                [this] { StartEncoderBenchmark(false); });
        }
        return;
    }
    if (engine_ && engine_->Snapshot().room.screenShareState !=
                       RoomScreenShareState::kIdle) {
        if (manualRequest) {
            RemoteCToast::Show(
                this, QStringLiteral("请结束当前屏幕共享后再检测编码器"),
                RemoteCToast::Tone::kInformation);
        } else {
            QTimer::singleShot(std::chrono::seconds(30), this,
                [this] { StartEncoderBenchmark(false); });
        }
        return;
    }

    const QSettings settings;
    encoderBenchmarkManualRequest_ = manualRequest;
    encoderBenchmarkHardwareFingerprint_ = HardwareFingerprintForUi(true);
    encoderBenchmarkCaptureBackend_ = settings.value(
        QString::fromLatin1(kDesktopCaptureBackendSetting),
        QStringLiteral("native_dxgi")).toString();
    encoderBenchmarkX264Preset_ = settings.value(
        QString::fromLatin1(kFfmpegX264PresetSetting),
        QStringLiteral("medium")).toString();
    encoderBenchmarkProcess_ = new QProcess(this);
    encoderBenchmarkProcess_->setProcessChannelMode(
        QProcess::SeparateChannels);
    SettingsControls().encoderBenchmarkButton->setEnabled(false);
    SettingsControls().encoderBenchmarkButton->setText(QStringLiteral("正在检测…"));
    SettingsControls().encoderBenchmarkSummary->setText(QStringLiteral(
        "正在独立进程中测试完整输入、转换和编码链路，请稍候…"));
    connect(encoderBenchmarkProcess_,
        qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
        this, [this](int exitCode, QProcess::ExitStatus) {
            FinishEncoderBenchmark(exitCode);
        });
    connect(encoderBenchmarkProcess_, &QProcess::errorOccurred,
        this, [this](QProcess::ProcessError error) {
            if (error == QProcess::FailedToStart) {
                FinishEncoderBenchmark(-1);
            }
        });
    encoderBenchmarkProcess_->start(
        QCoreApplication::applicationFilePath(),
        {QStringLiteral("--encoder-optimal-probe=%1")
             .arg(encoderBenchmarkCaptureBackend_)});
    QPointer<QProcess> guarded(encoderBenchmarkProcess_);
    QTimer::singleShot(std::chrono::seconds(120), this, [guarded] {
        if (guarded && guarded->state() != QProcess::NotRunning) {
            guarded->kill();
        }
    });
}

void ControllerMainWindow::FinishEncoderBenchmark(int exitCode)
{
    Q_UNUSED(exitCode);
    auto* process = encoderBenchmarkProcess_;
    if (!process) {
        return;
    }
    encoderBenchmarkProcess_ = nullptr;
    const QByteArray output = process->readAllStandardOutput().trimmed();
    const QString processError = process->errorString();
    process->deleteLater();
    QJsonDocument document;
    const QList<QByteArray> lines = output.split('\n');
    for (auto it = lines.crbegin(); it != lines.crend(); ++it) {
        QJsonParseError error;
        const auto candidate = QJsonDocument::fromJson(it->trimmed(), &error);
        if (error.error == QJsonParseError::NoError &&
            candidate.isObject()) {
            document = candidate;
            break;
        }
    }
    const QJsonObject result = document.object();
    const QString currentFingerprint = HardwareFingerprintForUi(true);
    const QSettings currentSettings;
    const QString currentCaptureBackend = currentSettings.value(
        QString::fromLatin1(kDesktopCaptureBackendSetting),
        QStringLiteral("native_dxgi")).toString();
    const QString testedFingerprint = std::exchange(
        encoderBenchmarkHardwareFingerprint_, QString{});
    const QString testedCaptureBackend = std::exchange(
        encoderBenchmarkCaptureBackend_, QString{});
    const QString testedX264Preset = std::exchange(
        encoderBenchmarkX264Preset_, QString{});
    const bool hardwareEnvironmentChanged =
        testedFingerprint != currentFingerprint;
    const bool completed = !result.isEmpty() &&
        result.contains(QStringLiteral("passed")) &&
        !hardwareEnvironmentChanged;
    const bool passed = completed &&
        result.value(QStringLiteral("passed")).toBool();
    if (completed) {
        QSettings settings;
        QJsonObject profile;
        profile.insert(QStringLiteral("hardwareFingerprint"),
                       testedFingerprint);
        profile.insert(QStringLiteral("captureBackend"),
                       testedCaptureBackend);
        profile.insert(QStringLiteral("quality"), testedX264Preset);
        profile.insert(QStringLiteral("policyVersion"),
                       kEncoderBenchmarkPolicyVersion);
        profile.insert(QStringLiteral("completed"), true);
        profile.insert(QStringLiteral("passed"), passed);
        profile.insert(QStringLiteral("testedAtUtc"),
                       QDateTime::currentDateTimeUtc().toString(
                           Qt::ISODateWithMs));
        profile.insert(QStringLiteral("candidates"),
                       result.value(QStringLiteral("candidates")).toArray());
        if (passed) {
            profile.insert(
                QStringLiteral("bestEncoderId"),
                result.value(QStringLiteral("bestEncoderId")).toString());
            profile.insert(
                QStringLiteral("bestEncoderName"),
                result.value(QStringLiteral("bestEncoderName")).toString());
        }
        app::SaveEncoderBenchmarkProfile(settings, std::move(profile));
        // Keep the last profile in the former keys so an older executable can
        // still read its most recent benchmark after a temporary downgrade.
        settings.setValue(
            QString::fromLatin1(kEncoderHardwareFingerprintSetting),
            testedFingerprint);
        settings.setValue(
            QString::fromLatin1(kEncoderCaptureBackendSetting),
            testedCaptureBackend);
        settings.setValue(
            QString::fromLatin1(kEncoderX264PresetSetting),
            testedX264Preset);
        settings.setValue(
            QString::fromLatin1(kEncoderBenchmarkCompletedSetting), true);
        settings.setValue(
            QString::fromLatin1(kEncoderBenchmarkPassedSetting), passed);
        settings.setValue(
            QString::fromLatin1(kEncoderBenchmarkPolicyVersionSetting),
            kEncoderBenchmarkPolicyVersion);
        settings.setValue(
            QString::fromLatin1(kBestEncoderTestTimeSetting),
            QDateTime::currentDateTimeUtc());
        settings.setValue(
            QString::fromLatin1(kEncoderCandidatesSetting),
            QJsonDocument(result.value(QStringLiteral("candidates"))
                              .toArray())
                .toJson(QJsonDocument::Compact));
        if (passed) {
            settings.setValue(
                QString::fromLatin1(kBestEncoderIdSetting),
                result.value(QStringLiteral("bestEncoderId")).toString());
            settings.setValue(
                QString::fromLatin1(kBestEncoderNameSetting),
                result.value(QStringLiteral("bestEncoderName")).toString());
        } else {
            settings.remove(QString::fromLatin1(kBestEncoderIdSetting));
            settings.remove(QString::fromLatin1(kBestEncoderNameSetting));
        }
        settings.sync();
    }
    RefreshEncoderBenchmarkSummary(false);
    const bool testedProfileIsCurrent =
        testedCaptureBackend == currentCaptureBackend &&
        testedX264Preset == currentSettings.value(
            QString::fromLatin1(kFfmpegX264PresetSetting),
            QStringLiteral("medium")).toString();
    if (passed && testedProfileIsCurrent && SettingsControls().videoEncoderSelector &&
        SettingsControls().videoEncoderSelector->currentData().toString() ==
            QStringLiteral("auto")) {
        ApplyVideoPipelineSettingsFromUi(false, {}, false);
    }
    if (encoderBenchmarkManualRequest_) {
        RemoteCToast::Show(
            this,
            hardwareEnvironmentChanged
                ? QStringLiteral("环境已变化，本次编码检测结果未保存")
                : passed
                ? QStringLiteral("编码器检测完成，自动模式将在下次共享采用最优结果")
                : completed
                ? QStringLiteral("编码器检测完成，已保留所有候选结果")
                : QStringLiteral("编码器性能检测失败：%1")
                      .arg(processError),
            passed ? RemoteCToast::Tone::kSuccess
                   : RemoteCToast::Tone::kInformation);
    }
    encoderBenchmarkManualRequest_ = false;
    SettingsControls().encoderBenchmarkButton->setEnabled(true);
    SettingsControls().encoderBenchmarkButton->setText(QStringLiteral("重新检测"));
    if (hardwareEnvironmentChanged) {
        QTimer::singleShot(std::chrono::seconds(5), this,
            [this] { StartEncoderBenchmark(false); });
    } else if (!completed && SettingsControls().encoderBenchmarkSummary) {
        SettingsControls().encoderBenchmarkSummary->setText(QStringLiteral("检测失败：%1")
            .arg(processError.toHtmlEscaped()));
    }
}
void ControllerMainWindow::RefreshDecoderBenchmarkSummary(
    bool refreshHardwareEnvironment)
{
    if (!SettingsControls().decoderBenchmarkSummary) {
        return;
    }
    const QSettings settings;
    const QString currentFingerprint =
        HardwareFingerprintForUi(refreshHardwareEnvironment);
    const bool resultMatchesCurrentHardware =
        settings.value(
            QString::fromLatin1(
                kDecoderBenchmarkCompletedSetting),
            false).toBool() &&
        settings.value(
            QString::fromLatin1(
                kDecoderBenchmarkPolicyVersionSetting),
            0).toInt() == kDecoderBenchmarkPolicyVersion &&
        settings.value(
            QString::fromLatin1(
                kDecoderHardwareFingerprintSetting))
            .toString() == currentFingerprint;
    const QString name = resultMatchesCurrentHardware &&
            settings.value(
                QString::fromLatin1(
                    kDecoderBenchmarkPassedSetting),
                false).toBool()
        ? settings.value(
              QString::fromLatin1(kBestDecoderNameSetting))
              .toString()
        : QString{};
    if (name.isEmpty()) {
        const bool currentNegativeResult =
            resultMatchesCurrentHardware &&
            !settings.value(
                QString::fromLatin1(
                    kDecoderBenchmarkPassedSetting),
                false).toBool();
        SettingsControls().decoderBenchmarkSummary->setText(AdaptBenchmarkHtmlForTheme(QStringLiteral(
            "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">"
            "<tr><td><b style=\"color:#24324b\">%1</b></td>"
            "<td align=\"right\"><span style=\"color:%2\"><b>%3</b></span></td></tr>"
            "<tr><td colspan=\"2\" style=\"padding-top:5px;color:#667085\">%4</td></tr>"
            "</table>")
            .arg(currentNegativeResult
                     ? QStringLiteral("当前使用软件解码")
                     : QStringLiteral("等待首次性能检测"),
                 currentNegativeResult
                     ? QStringLiteral("#a76508")
                     : QStringLiteral("#1769e8"),
                 currentNegativeResult
                     ? QStringLiteral("无合格硬解")
                     : QStringLiteral("待检测"),
                 currentNegativeResult
                     ? QStringLiteral(
                           "本机硬件解码器未达到低延迟要求，自动模式已安全使用 WebRTC FFmpeg。")
                     : QStringLiteral(
                           "程序会在当前硬件环境空闲时自动检测，不影响正常使用。"))));
        return;
    }
    const QDateTime testedAt = QDateTime::fromString(
        settings.value(
            QString::fromLatin1(kBestDecoderTestTimeSetting)).toString(),
        Qt::ISODateWithMs).toLocalTime();
    const QString configuredMode = settings.value(
        QStringLiteral("media/videoDecoderPreference"),
        QStringLiteral("auto")).toString();
    const QString compactBestName = name.startsWith(
            QStringLiteral("FFmpeg D3D11VA"))
        ? QStringLiteral("FFmpeg D3D11VA")
        : name.contains(QStringLiteral("Microsoft H264"))
        ? QStringLiteral("Microsoft H264 MFT")
        : name;
    const QString strategyText =
        configuredMode == QStringLiteral("software")
            ? QStringLiteral("仅软件 · WebRTC FFmpeg")
            : configuredMode == QStringLiteral("hardware")
            ? QStringLiteral("仅硬件 · %1").arg(compactBestName)
            : QStringLiteral("自动 · 优先 %1").arg(compactBestName);
    QString html = QStringLiteral(
        "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">"
        "<tr><td><span style=\"color:#667085\">当前策略</span>"
        "&nbsp;&nbsp;<b style=\"color:#172033\">%1</b></td>"
        "<td align=\"right\"><span style=\"color:#138b57\"><b>检测通过</b></span></td></tr>"
        "<tr><td colspan=\"2\" style=\"padding-top:5px;color:#667085\">"
        "硬解异常时自动回退软件解码；已建立的画面连接不会中途切换。</td></tr>"
        "</table>"
        "<hr style=\"color:#e3e8f1\"/>")
        .arg(strategyText.toHtmlEscaped());
    const QJsonArray candidates = QJsonDocument::fromJson(
        settings.value(QString::fromLatin1(kDecoderCandidatesSetting))
            .toByteArray()).array();
    QList<QJsonObject> orderedCandidates;
    orderedCandidates.reserve(candidates.size());
    for (const auto& value : candidates) {
        orderedCandidates.push_back(value.toObject());
    }
    std::stable_sort(
        orderedCandidates.begin(), orderedCandidates.end(),
        [&name](const QJsonObject& left, const QJsonObject& right) {
            const auto rank = [&name](const QJsonObject& candidate) {
                const QString candidateName =
                    candidate.value(QStringLiteral("name")).toString();
                if (candidateName == name) {
                    return 0;
                }
                if (!candidate.value(QStringLiteral("hardware")).toBool(true)) {
                    return 1;
                }
                return candidate.value(QStringLiteral("passed")).toBool()
                    ? 2 : 3;
            };
            return rank(left) < rank(right);
        });
    bool firstCandidate = true;
    for (const auto& candidate : orderedCandidates) {
        const QString candidateName =
            candidate.value(QStringLiteral("name")).toString();
        const bool hardware =
            candidate.value(QStringLiteral("hardware")).toBool(true);
        const bool passed =
            candidate.value(QStringLiteral("passed")).toBool();
        QString displayName = candidateName;
        if (candidateName.startsWith(QStringLiteral("FFmpeg D3D11VA"))) {
            displayName = QStringLiteral("FFmpeg D3D11VA");
        } else if (candidateName == QStringLiteral("FFmpeg (WebRTC Builtin)")) {
            displayName = QStringLiteral("WebRTC FFmpeg");
        } else if (candidateName.contains(QStringLiteral("Microsoft H264"))) {
            displayName = QStringLiteral("Microsoft H264 MFT");
        }
        const QString kind = hardware
            ? (candidateName.startsWith(QStringLiteral("FFmpeg D3D11VA"))
                   ? QStringLiteral("硬件 · D3D11VA")
                   : candidate.value(QStringLiteral("asynchronous")).toBool()
                   ? QStringLiteral("硬件 · 异步 MFT")
                   : QStringLiteral("硬件 · 同步 MFT"))
            : QStringLiteral("软件 · CPU");
        const bool selected = hardware && candidateName == name;
        const QString status = selected
            ? QStringLiteral("最优硬解")
            : !hardware
            ? QStringLiteral("软件备用")
            : passed ? QStringLiteral("可用") : QStringLiteral("未通过");
        const QString statusColor = selected || passed
            ? QStringLiteral("#138b57")
            : hardware ? QStringLiteral("#c12b37")
                       : QStringLiteral("#1769e8");
        const double realtimeAverage = candidate.value(
            QStringLiteral("realtimeAverageLatencyMs")).toDouble();
        const double realtimeP95 = candidate.value(
            QStringLiteral("realtimeP95LatencyMs")).toDouble();
        const double sparseAverage = candidate.value(
            QStringLiteral("sparseAverageLatencyMs")).toDouble();
        const double sparseP95 = candidate.value(
            QStringLiteral("sparseP95LatencyMs")).toDouble();
        if (!firstCandidate) {
            html += QStringLiteral("<hr style=\"color:#e8ecf3\"/>");
        }
        firstCandidate = false;
        html += QStringLiteral(
            "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">"
            "<tr><td><b style=\"color:#24324b\">%1</b>"
            "&nbsp;&nbsp;<span style=\"color:#7a8496\">%2</span></td>"
            "<td align=\"right\"><span style=\"color:%3\"><b>%4</b></span></td></tr>"
            "<tr><td colspan=\"2\" style=\"padding-top:5px;color:#667085\">"
            "连续 60 FPS&nbsp;&nbsp;平均 %5 ms&nbsp;·&nbsp;P95 %6 ms"
            "&nbsp;&nbsp;&nbsp;&nbsp;稀疏 5 FPS&nbsp;&nbsp;平均 %7 ms&nbsp;·&nbsp;P95 %8 ms"
            "</td></tr>")
            .arg(displayName.toHtmlEscaped(), kind, statusColor, status)
            .arg(realtimeAverage, 0, 'f', 2)
            .arg(realtimeP95, 0, 'f', 2)
            .arg(sparseAverage, 0, 'f', 2)
            .arg(sparseP95, 0, 'f', 2);
        if (!passed && hardware) {
            QString reason = QStringLiteral("未达到远程控制低延迟要求");
            if (sparseP95 > 180.0) {
                reason = QStringLiteral("稀疏画面响应过慢（P95 %1 ms）")
                             .arg(sparseP95, 0, 'f', 2);
            } else if (realtimeP95 > 80.0) {
                reason = QStringLiteral("连续画面响应过慢（P95 %1 ms）")
                             .arg(realtimeP95, 0, 'f', 2);
            }
            html += QStringLiteral(
                "<tr><td colspan=\"2\" style=\"padding-top:4px;color:#a76508\">"
                "原因：%1</td></tr>")
                .arg(reason);
        }
        html += QStringLiteral("</table>");
    }
    if (candidates.isEmpty()) {
        const double average = settings.value(
            QString::fromLatin1(kBestDecoderAverageSetting)).toDouble();
        const double p95 = settings.value(
            QString::fromLatin1(kBestDecoderP95Setting)).toDouble();
        html += QStringLiteral(
            "<b style=\"color:#24324b\">%1</b>"
            "&nbsp;&nbsp;<span style=\"color:#138b57\"><b>最优硬解</b></span><br/>"
            "<span style=\"color:#667085\">平均 %2 ms · P95 %3 ms</span>")
            .arg(compactBestName.toHtmlEscaped())
            .arg(average, 0, 'f', 2)
            .arg(p95, 0, 'f', 2);
    }
    if (testedAt.isValid()) {
        html += QStringLiteral(
            "<div style=\"margin-top:10px;color:#98a2b3\">最近检测：%1</div>")
            .arg(testedAt.toString(
                QStringLiteral("yyyy-MM-dd HH:mm")));
    }
    SettingsControls().decoderBenchmarkSummary->setText(AdaptBenchmarkHtmlForTheme(html));
    SettingsControls().decoderBenchmarkSummary->setToolTip(QStringLiteral(
        "自动模式优先使用通过连续与稀疏帧低延迟检测的硬件解码器；运行失败后永久切换到软件解码，直到下次会话。"));
}

void ControllerMainWindow::RefreshDecoderHardwareSelectionAvailability(
    bool refreshHardwareEnvironment)
{
    if (!SettingsControls().videoDecoderSelector) {
        return;
    }
    const QSettings settings;
    const QString currentFingerprint =
        HardwareFingerprintForUi(refreshHardwareEnvironment);
    const bool available =
        settings.value(
            QString::fromLatin1(kDecoderBenchmarkCompletedSetting),
            false).toBool() &&
        settings.value(
            QString::fromLatin1(kDecoderBenchmarkPassedSetting),
            false).toBool() &&
        settings.value(
            QString::fromLatin1(
                kDecoderBenchmarkPolicyVersionSetting),
            0).toInt() == kDecoderBenchmarkPolicyVersion &&
        settings.value(
            QString::fromLatin1(kDecoderHardwareFingerprintSetting))
                .toString() == currentFingerprint;
    const int hardwareIndex = SettingsControls().videoDecoderSelector->findData(
        QStringLiteral("hardware"));
    if (hardwareIndex < 0) {
        return;
    }
    if (auto* model = qobject_cast<QStandardItemModel*>(
            SettingsControls().videoDecoderSelector->model())) {
        if (auto* item = model->item(hardwareIndex)) {
            item->setEnabled(available);
        }
    }
    SettingsControls().videoDecoderSelector->setItemData(
        hardwareIndex,
        available
            ? QStringLiteral(
                  "当前硬件已通过原生 D3D11 输出和低延迟检测。")
            : QStringLiteral(
                  "当前机器尚未通过硬件解码低延迟检测，不能选择仅硬件。"),
        Qt::ToolTipRole);
    if (!available &&
        SettingsControls().videoDecoderSelector->currentData().toString() ==
            QStringLiteral("hardware")) {
        const QSignalBlocker blocker(SettingsControls().videoDecoderSelector);
        SettingsControls().videoDecoderSelector->setCurrentIndex(std::max(
            0, SettingsControls().videoDecoderSelector->findData(
                   QStringLiteral("auto"))));
        QSettings writableSettings;
        writableSettings.setValue(
            QString::fromLatin1(kVideoDecoderPreferenceSetting),
            QStringLiteral("auto"));
        writableSettings.sync();
    }
}

void ControllerMainWindow::StartDecoderBenchmark(bool manualRequest)
{
    if (decoderBenchmarkProcess_) {
        if (manualRequest) {
            RemoteCToast::Show(
                this, QStringLiteral("最优解码器检测正在进行"),
                RemoteCToast::Tone::kInformation);
        }
        return;
    }
    if (encoderBenchmarkProcess_) {
        if (manualRequest) {
            RemoteCToast::Show(
                this, QStringLiteral("请等待编码器检测完成"),
                RemoteCToast::Tone::kInformation);
        } else {
            QTimer::singleShot(
                std::chrono::seconds(10), this,
                [this] { StartDecoderBenchmark(false); });
        }
        return;
    }
    if (engine_) {
        const auto snapshot = engine_->Snapshot();
        if (snapshot.room.screenShareState !=
            RoomScreenShareState::kIdle) {
            if (manualRequest) {
                RemoteCToast::Show(
                    this,
                    QStringLiteral("请结束当前屏幕共享后再检测解码器"),
                    RemoteCToast::Tone::kInformation);
            } else {
                QTimer::singleShot(
                    std::chrono::seconds(30), this,
                    [this] { StartDecoderBenchmark(false); });
            }
            return;
        }
    }

    decoderBenchmarkManualRequest_ = manualRequest;
    decoderBenchmarkHardwareFingerprint_ = HardwareFingerprintForUi(true);
    decoderBenchmarkProcess_ = new QProcess(this);
    decoderBenchmarkProcess_->setProcessChannelMode(
        QProcess::SeparateChannels);
    if (SettingsControls().decoderBenchmarkButton) {
        SettingsControls().decoderBenchmarkButton->setEnabled(false);
        SettingsControls().decoderBenchmarkButton->setText(QStringLiteral("正在检测…"));
    }
    if (SettingsControls().decoderBenchmarkSummary) {
        SettingsControls().decoderBenchmarkSummary->setText(
            QStringLiteral("正在后台对比软件与硬件解码器，请稍候…"));
    }
    connect(
        decoderBenchmarkProcess_,
        qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
        this,
        [this](int exitCode, QProcess::ExitStatus) {
            FinishDecoderBenchmark(exitCode);
        });
    connect(decoderBenchmarkProcess_, &QProcess::errorOccurred,
            this, [this](QProcess::ProcessError error) {
                if (error == QProcess::FailedToStart) {
                    FinishDecoderBenchmark(-1);
                }
            });
    decoderBenchmarkProcess_->start(
        QCoreApplication::applicationFilePath(),
        {QStringLiteral("--decoder-optimal-probe")});

    QPointer<QProcess> guardedProcess(decoderBenchmarkProcess_);
    QTimer::singleShot(
        std::chrono::seconds(60), this,
        [guardedProcess] {
            if (guardedProcess &&
                guardedProcess->state() != QProcess::NotRunning) {
                guardedProcess->kill();
            }
        });
}

void ControllerMainWindow::FinishDecoderBenchmark(int exitCode)
{
    auto* process = decoderBenchmarkProcess_;
    if (!process) {
        return;
    }
    decoderBenchmarkProcess_ = nullptr;
    const QByteArray output = process->readAllStandardOutput().trimmed();
    const QString processError = process->errorString();
    process->deleteLater();

    QJsonDocument document;
    const QList<QByteArray> lines = output.split('\n');
    for (auto it = lines.crbegin(); it != lines.crend(); ++it) {
        QJsonParseError parseError;
        const auto candidate = QJsonDocument::fromJson(
            it->trimmed(), &parseError);
        if (parseError.error == QJsonParseError::NoError &&
            candidate.isObject()) {
            document = candidate;
            break;
        }
    }
    const QJsonObject result = document.object();
    const bool resultDocumentComplete =
        !result.isEmpty() &&
        result.contains(QStringLiteral("passed"));
    const QString currentHardwareFingerprint = HardwareFingerprintForUi(true);
    const QString benchmarkHardwareFingerprint =
        std::exchange(
            decoderBenchmarkHardwareFingerprint_, QString{});
    const bool hardwareEnvironmentChanged =
        resultDocumentComplete &&
        (!benchmarkHardwareFingerprint.isEmpty() &&
         benchmarkHardwareFingerprint !=
             currentHardwareFingerprint);
    const bool completedNormally =
        resultDocumentComplete &&
        !hardwareEnvironmentChanged;
    const bool passed = completedNormally &&
        result.value(QStringLiteral("passed")).toBool();
    const QString bestName =
        result.value(QStringLiteral("bestDecoderName")).toString();
    if (completedNormally) {
        QSettings settings;
        settings.setValue(
            QString::fromLatin1(
                kDecoderHardwareFingerprintSetting),
            benchmarkHardwareFingerprint);
        settings.setValue(
            QString::fromLatin1(
                kDecoderBenchmarkCompletedSetting),
            true);
        settings.setValue(
            QString::fromLatin1(
                kDecoderBenchmarkPolicyVersionSetting),
            kDecoderBenchmarkPolicyVersion);
        settings.setValue(
            QString::fromLatin1(
                kDecoderBenchmarkPassedSetting),
            passed);
        settings.setValue(
            QString::fromLatin1(kBestDecoderTestTimeSetting),
            QDateTime::currentDateTimeUtc().toString(
                Qt::ISODateWithMs));
        settings.setValue(
            QString::fromLatin1(kDecoderCandidatesSetting),
            QJsonDocument(
                result.value(QStringLiteral("candidates"))
                    .toArray())
                .toJson(QJsonDocument::Compact));
        if (!passed) {
            settings.remove(
                QString::fromLatin1(kBestDecoderNameSetting));
            settings.remove(
                QString::fromLatin1(
                    kBestDecoderAverageSetting));
            settings.remove(
                QString::fromLatin1(kBestDecoderP95Setting));
        }
        settings.sync();
    }
    if (passed && !bestName.isEmpty()) {
        QSettings settings;
        settings.setValue(
            QString::fromLatin1(kBestDecoderNameSetting), bestName);
        settings.setValue(
            QString::fromLatin1(kBestDecoderAverageSetting),
            result.value(
                QStringLiteral("bestAverageLatencyMs")).toDouble());
        settings.setValue(
            QString::fromLatin1(kBestDecoderP95Setting),
            result.value(QStringLiteral("bestP95LatencyMs")).toDouble());
        settings.sync();
        if (sessionMedia_) {
            sessionMedia_->SetPreferredHardwareDecoderName(
                bestName.toStdString());
        }
        RefreshDecoderBenchmarkSummary(false);
        if (decoderBenchmarkManualRequest_) {
            RemoteCToast::Show(
                this, QStringLiteral("解码器性能检测完成"),
                RemoteCToast::Tone::kSuccess);
        }
    } else {
        const QString reportedError =
            result.value(QStringLiteral("error")).toString();
        if (SettingsControls().decoderBenchmarkSummary) {
            SettingsControls().decoderBenchmarkSummary->setText(
                hardwareEnvironmentChanged
                    ? QStringLiteral(
                          "检测期间硬件、驱动或远程桌面环境发生变化，结果已丢弃。")
                    : completedNormally
                    ? QStringLiteral(
                          "当前硬件环境没有通过检测的硬件解码器，自动模式继续使用 FFmpeg。")
                    : QStringLiteral("检测失败：%1")
                          .arg(!reportedError.isEmpty()
                                   ? reportedError
                                   : processError));
        }
        if (decoderBenchmarkManualRequest_) {
            RemoteCToast::Show(
                this,
                hardwareEnvironmentChanged
                    ? QStringLiteral(
                          "硬件环境已变化，本次检测结果未保存")
                    : completedNormally
                    ? QStringLiteral(
                          "未找到可用硬件解码器，继续使用 FFmpeg")
                    : QStringLiteral("最优解码器检测失败"),
                completedNormally
                    ? RemoteCToast::Tone::kInformation
                    : RemoteCToast::Tone::kError);
        }
        if (completedNormally) {
            RefreshDecoderBenchmarkSummary(false);
        }
    }
    decoderBenchmarkManualRequest_ = false;
    RefreshDecoderHardwareSelectionAvailability(false);
    if (SettingsControls().decoderBenchmarkButton) {
        SettingsControls().decoderBenchmarkButton->setEnabled(true);
        SettingsControls().decoderBenchmarkButton->setText(
            QStringLiteral("重新检测"));
    }
    if (hardwareEnvironmentChanged) {
        QTimer::singleShot(
            std::chrono::seconds(5), this,
            [this] { StartDecoderBenchmark(false); });
    }
}
void ControllerMainWindow::ApplyClipboardConfigurationFromUi(
    bool showFeedback)
{
    if (!SettingsControls().remotePasteEnabledSelector || !SettingsControls().clipboardFormatsSelector ||
        !SettingsControls().clipboardLargeFileLimitSelector ||
        !SettingsControls().clipboardCacheRetentionSelector ||
        !SettingsControls().clipboardCacheCapacitySelector) {
        return;
    }
    const bool enabled =
        SettingsControls().remotePasteEnabledSelector->currentData().toBool();
    const QString formats =
        SettingsControls().clipboardFormatsSelector->currentData().toString();
    const int fileLimitMiB = (std::min)(
        SettingsControls().clipboardLargeFileLimitSelector->currentData().toInt(), 512);
    const int retentionMinutes =
        SettingsControls().clipboardCacheRetentionSelector->currentData().toInt();
    const qulonglong cacheLimitGiB =
        SettingsControls().clipboardCacheCapacitySelector->currentData().toULongLong();

    QSettings settings;
    settings.setValue(QString::fromLatin1(kRemotePasteEnabledSetting),
                      enabled);
    settings.setValue(QString::fromLatin1(kClipboardFormatsSetting),
                      formats);
    settings.setValue(QString::fromLatin1(kClipboardFileLimitSetting),
                      fileLimitMiB);
    settings.setValue(
        QString::fromLatin1(kClipboardCacheRetentionSetting),
        retentionMinutes);
    settings.setValue(
        QString::fromLatin1(kClipboardCacheCapacitySetting),
        cacheLimitGiB);
    if (clipboardController_) {
        app::RemotePasteConfiguration configuration;
        configuration.enabled = enabled;
        configuration.unicodeText = true;
        configuration.html = formats != QStringLiteral("text");
        configuration.rtf = formats != QStringLiteral("text");
        configuration.png =
            formats == QStringLiteral("all") ||
            formats == QStringLiteral("no_files");
        configuration.files = formats == QStringLiteral("all");
        configuration.automaticFileLimitBytes =
            static_cast<std::uint64_t>((std::max)(1, fileLimitMiB)) *
            1024ull * 1024ull;
        configuration.cacheBaseDirectory = std::filesystem::path(
            settings.value(
                QString::fromLatin1(
                    kClipboardCacheBaseDirectorySetting),
                InitialClipboardCacheBaseDirectory())
                .toString().toStdWString());
        configuration.cacheRetention = std::chrono::minutes(
            (std::max)(10, retentionMinutes));
        configuration.cacheLimitBytes =
            static_cast<std::uint64_t>(cacheLimitGiB) *
            1024ull * 1024ull * 1024ull;
        clipboardController_->SetConfiguration(configuration);
    }
    if (showFeedback) {
        RemoteCToast::Show(
            this,
            enabled
                ? QStringLiteral("远程粘贴已开启")
                : QStringLiteral("远程粘贴已关闭"),
            RemoteCToast::Tone::kSuccess);
    }
}

void ControllerMainWindow::UpdateClipboardSession(
    const SessionEngineSnapshot& snapshot)
{
    if (!clipboardController_) {
        return;
    }
    app::ClipboardSessionContext context;
    context.localDeviceId = snapshot.localDeviceId;
    const bool directActive =
        snapshot.state == SessionEngineState::kActive &&
        snapshot.purpose == SessionPurpose::kRemoteControl &&
        snapshot.remoteControlRole != RemoteControlRole::kNone &&
        !snapshot.sessionId.empty() && !snapshot.peerDeviceId.empty();
    if (directActive) {
        context.roomId = snapshot.sessionId;
        context.active = true;
        context.localIsController =
            snapshot.remoteControlRole == RemoteControlRole::kController;
        context.peerDeviceId = snapshot.peerDeviceId;
        context.transportReady =
            snapshot.direct.clipboardReliableChannelOpen &&
            snapshot.direct.clipboardTransferChannelOpen;
        clipboardController_->UpdateSession(std::move(context));
        return;
    }

    context.roomId = snapshot.room.roomId;
    const bool roomAndControlActive =
        snapshot.room.membership == RoomMembershipState::kActive &&
        snapshot.roomControlGrantActive &&
        (snapshot.room.screenShareState == RoomScreenShareState::kActive ||
         snapshot.room.screenShareState ==
             RoomScreenShareState::kRecovering) &&
        !snapshot.room.screenSharerDeviceId.empty() &&
        !snapshot.room.activeControllerDeviceId.empty();
    if (roomAndControlActive &&
        snapshot.room.screenSharerDeviceId == snapshot.localDeviceId) {
        context.active = clipboardAllowedForCurrentControl_;
        context.localIsController = false;
        context.peerDeviceId = snapshot.room.activeControllerDeviceId;
    } else if (roomAndControlActive &&
               snapshot.room.activeControllerDeviceId ==
                   snapshot.localDeviceId) {
        context.active = true;
        context.localIsController = true;
        context.peerDeviceId = snapshot.room.screenSharerDeviceId;
    }
    if (context.active) {
        const auto pair = std::find_if(
            snapshot.roomActivity.peerConnections.begin(),
            snapshot.roomActivity.peerConnections.end(),
            [&context](const RoomPeerConnectionSnapshot& candidate) {
                return candidate.peerDeviceId == context.peerDeviceId;
            });
        context.transportReady =
            pair != snapshot.roomActivity.peerConnections.end() &&
            pair->state == RoomPeerConnectionState::kActive &&
            pair->clipboardReliableChannelOpen &&
            pair->clipboardTransferChannelOpen;
    }
    clipboardController_->UpdateSession(std::move(context));
}

void ControllerMainWindow::UpdateDirectFileTransferSession(
    const SessionEngineSnapshot& snapshot)
{
    const bool directSession =
        snapshot.state == SessionEngineState::kActive &&
        snapshot.purpose == SessionPurpose::kRemoteControl &&
        !snapshot.peerDeviceId.empty();
    if (!directSession) {
        return;
    }

    std::vector<FileTransferPeer> peers;
    std::vector<std::string> availablePeerIds;
    std::vector<std::string> recoveringPeerIds;
    if (snapshot.direct.fileTransferChannelOpen) {
        FileTransferPeer peer;
        peer.deviceId = snapshot.peerDeviceId;
        peer.displayName = remoteSessionBinding_ &&
                remoteSessionBinding_->peerDeviceId ==
                    QString::fromStdString(snapshot.peerDeviceId) &&
                !remoteSessionBinding_->peerDeviceName.isEmpty()
            ? remoteSessionBinding_->peerDeviceName
            : QString::fromStdString(snapshot.peerDeviceId);
        peers.push_back(std::move(peer));
        availablePeerIds.push_back(snapshot.peerDeviceId);
    } else {
        recoveringPeerIds.push_back(snapshot.peerDeviceId);
    }
    if (fileTransferController_) {
        fileTransferController_->UpdatePeerConnectivity(
            availablePeerIds, recoveringPeerIds);
    }
    if (fileTransferWindow_) {
        fileTransferWindow_->SyncPeers(peers, frameGeometry());
    }
}

void ControllerMainWindow::OnClipboardStateChanged(
    const app::ClipboardControllerSnapshot& snapshot)
{
    if (QThread::currentThread() != thread()) {
        bool expected = false;
        if (clipboardUiUpdatePending_.compare_exchange_strong(
                expected, true, std::memory_order_acq_rel)) {
            QMetaObject::invokeMethod(this, [this] {
                clipboardUiUpdatePending_.store(
                    false, std::memory_order_release);
                if (clipboardController_) {
                    OnClipboardStateChanged(
                        clipboardController_->Snapshot());
                }
            }, Qt::QueuedConnection);
        }
        return;
    }
    if (SettingsControls().clipboardCacheUsageLabel) {
        const auto formatCacheBytes = [](std::uint64_t bytes) {
            constexpr double kMiB = 1024.0 * 1024.0;
            constexpr double kGiB = 1024.0 * 1024.0 * 1024.0;
            return bytes >= static_cast<std::uint64_t>(kGiB)
                ? QStringLiteral("%1 GiB").arg(bytes / kGiB, 0, 'f', 2)
                : QStringLiteral("%1 MiB").arg(bytes / kMiB, 0, 'f', 1);
        };
        SettingsControls().clipboardCacheUsageLabel->setText(QStringLiteral(
            "%1 个项目 · %2 / 上限 %3")
            .arg(static_cast<qulonglong>(snapshot.cacheEntryCount))
            .arg(formatCacheBytes(snapshot.cacheBytes))
            .arg(formatCacheBytes(snapshot.cacheEffectiveLimitBytes)));
        SettingsControls().clipboardCacheUsageLabel->setToolTip(
            QStringLiteral("缓存路径：%1\n磁盘可用：%2\n安全容量：%3")
                .arg(QString::fromUtf8(snapshot.cacheRootPath))
                .arg(formatCacheBytes(snapshot.cacheAvailableBytes))
                .arg(formatCacheBytes(snapshot.cacheSafeCapacityBytes)));
    }
    if (SettingsControls().clearClipboardCacheButton) {
        SettingsControls().clearClipboardCacheButton->setEnabled(
            snapshot.cacheEntryCount > 0);
    }
    const bool receivedAdvanced =
        snapshot.receivedItems > displayedClipboardReceivedItems_;
    const bool rejectedAdvanced =
        snapshot.rejectedItems > displayedClipboardRejectedItems_;
    const QString transferId =
        QString::fromStdString(snapshot.transferId);
    if (snapshot.explorerConflictPending && !transferId.isEmpty()) {
        if (remoteSessionWindow_) {
            // Metadata preflight has not started a transfer. Keep exactly one
            // window visible: the decision prompt. This also closes a legacy
            // requesting popup if the peer did not provide V8 metadata.
            remoteSessionWindow_->CloseRemotePasteProgress(transferId);
        }
        if (promptedClipboardConflictId_ != transferId) {
            promptedClipboardConflictId_ = transferId;
            QStringList names;
            for (const auto& name : snapshot.explorerConflictNames) {
                if (names.size() >= 5) break;
                names.push_back(QString::fromUtf8(name));
            }
            QString conflictList = names.join(QStringLiteral("\n"));
            if (snapshot.explorerConflictNames.size() >
                static_cast<std::size_t>(names.size())) {
                conflictList += QStringLiteral("\n等 %1 个同名项目")
                    .arg(snapshot.explorerConflictNames.size());
            }
            const QString destination = QString::fromUtf8(
                snapshot.localPasteDestinationPath);
            QTimer::singleShot(0, this,
                [this, transferId, destination, conflictList] {
                    if (!clipboardController_) return;
                    const auto latest = clipboardController_->Snapshot();
                    if (!latest.explorerConflictPending ||
                        QString::fromStdString(latest.transferId) !=
                            transferId) {
                        return;
                    }
                    const bool replace = RemoteCDialog::Confirm(
                        nullptr,
                        QStringLiteral("目标位置已有同名项目"),
                        QStringLiteral(
                            "粘贴位置：%1\n\n"
                            "以下项目已经存在：\n%2\n\n"
                            "继续后将合并同名文件夹，并替换其中的同名文件。")
                            .arg(QDir::toNativeSeparators(destination),
                                 conflictList),
                        QStringLiteral("替换并粘贴"),
                        QStringLiteral("取消粘贴"),
                        RemoteCDialog::Tone::kDanger, true, true,
                        latest.localPasteTargetWindow, true);
                    (void)clipboardController_->ResolveExplorerConflict(
                        replace);
                });
        }
        return;
    }
    if (!snapshot.explorerConflictPending &&
        promptedClipboardConflictId_ == transferId) {
        promptedClipboardConflictId_.clear();
    }
    const auto formatTransferBytes = [](std::uint64_t bytes) {
        constexpr double kKiB = 1024.0;
        constexpr double kMiB = 1024.0 * 1024.0;
        constexpr double kGiB = 1024.0 * 1024.0 * 1024.0;
        if (bytes >= static_cast<std::uint64_t>(kGiB)) {
            return QStringLiteral("%1 GB").arg(bytes / kGiB, 0, 'f', 2);
        }
        if (bytes >= static_cast<std::uint64_t>(kMiB)) {
            return QStringLiteral("%1 MB").arg(bytes / kMiB, 0, 'f', 1);
        }
        if (bytes >= static_cast<std::uint64_t>(kKiB)) {
            return QStringLiteral("%1 KB").arg(bytes / kKiB, 0, 'f', 1);
        }
        return QStringLiteral("%1 B").arg(
            static_cast<qulonglong>(bytes));
    };
    const bool needsProgressDialog =
        snapshot.transferActive &&
        snapshot.transferContainsFiles &&
        (snapshot.state == "requesting_remote" ||
         snapshot.transferTotalBytes > kRemotePastePopupThresholdBytes);

    if (needsProgressDialog && remoteSessionWindow_) {
        QString progressText;
        const QString progressTitle = snapshot.transferOutgoing
            ? QStringLiteral("正在粘贴到远端")
            : QStringLiteral("正在从远端粘贴到本机");
        const QString totalText =
            formatTransferBytes(snapshot.transferTotalBytes);
        const QString completedText =
            formatTransferBytes(snapshot.transferCompletedBytes);
        const QString speedText = snapshot.transferBytesPerSecond > 1.0
            ? QStringLiteral("%1/s").arg(formatTransferBytes(
                  static_cast<std::uint64_t>(
                      snapshot.transferBytesPerSecond)))
            : QStringLiteral("正在估算速度");
        const QString remainingText =
            snapshot.transferEstimatedRemainingSeconds > 0
                ? QStringLiteral("预计剩余 %1 秒").arg(
                      static_cast<qulonglong>(
                          snapshot.transferEstimatedRemainingSeconds))
                : QStringLiteral("正在估算剩余时间");
        if (snapshot.state == "requesting_remote") {
            progressText = snapshot.transferTotalBytes > 0
                ? QStringLiteral(
                    "正在与远端建立文件传输 · %1 个项目 · %2\n"
                    "连接确认后将立即开始传输")
                    .arg(snapshot.transferItemCount)
                    .arg(totalText)
                : QStringLiteral(
                    "正在与远端建立文件传输…\n"
                    "连接确认后将立即开始传输");
        } else if (snapshot.state == "offering") {
            progressText = QStringLiteral(
                "正在与远端确认文件传输…\n"
                "%1 个文件项目 · 总大小 %2")
                .arg(snapshot.transferItemCount)
                .arg(totalText);
        } else if (snapshot.state == "applying") {
            progressText = (snapshot.transferOutgoing
                ? QStringLiteral(
                    "文件已经传输完成，正在粘贴到远端目标位置…\n"
                    "%1 / %2 · %3")
                : QStringLiteral(
                    "文件已经传输完成，正在粘贴到本机目标位置…\n"
                    "%1 / %2 · %3"))
                .arg(completedText, totalText, speedText);
        } else {
            const int percent = snapshot.transferTotalBytes == 0
                ? 0
                : static_cast<int>((std::min)(
                      100ull,
                      snapshot.transferCompletedBytes * 100ull /
                          snapshot.transferTotalBytes));
            progressText = (snapshot.transferOutgoing
                ? QStringLiteral(
                    "正在向远端传输 %1 个文件项目 · %2%\n"
                    "%3 / %4 · %5 · %6")
                : QStringLiteral(
                    "正在从远端接收 %1 个文件项目 · %2%\n"
                    "%3 / %4 · %5 · %6"))
                .arg(snapshot.transferItemCount)
                .arg(percent)
                .arg(completedText, totalText, speedText, remainingText);
        }
        if (!snapshot.transferOutgoing &&
            !snapshot.localPasteDestinationPath.empty()) {
            const QString nativeDestination = QDir::toNativeSeparators(
                QString::fromUtf8(snapshot.localPasteDestinationPath));
            const QString visibleDestination = QFontMetrics(
                QApplication::font()).elidedText(
                    nativeDestination, Qt::ElideMiddle, 315);
            progressText.prepend(
                QStringLiteral("粘贴位置：%1\n")
                    .arg(visibleDestination));
        }

        if (pendingRemotePasteDialogId_ != transferId) {
            pendingRemotePasteDialogId_ = transferId;
            // Show the prompt on the initial RequestCurrent snapshot instead
            // of waiting for hashing and Offer construction, so the very
            // first Ctrl+V gives immediate feedback.
            visibleRemotePasteDialogId_ = transferId;
        }
        if (visibleRemotePasteDialogId_ == transferId) {
            const double progress = snapshot.transferTotalBytes == 0
                ? 0.0
                : static_cast<double>(snapshot.transferCompletedBytes) /
                      static_cast<double>(snapshot.transferTotalBytes);
            remoteSessionWindow_->ShowRemotePasteProgress(
                transferId, progressTitle, progressText, progress,
                snapshot.localPasteTargetWindow);
        }
    } else if (!snapshot.transferActive) {
        const QString completedTransferId =
            !visibleRemotePasteDialogId_.isEmpty()
                ? visibleRemotePasteDialogId_
                : pendingRemotePasteDialogId_;
        if (rejectedAdvanced && !snapshot.lastErrorMessage.empty() &&
            remoteSessionWindow_) {
            QString message =
                QString::fromStdString(snapshot.lastErrorMessage);
            if (snapshot.lastErrorCode == "clipboard_transfer_timeout") {
                message = QStringLiteral(
                    "远端没有及时确认本次传输。当前粘贴状态已经自动释放，"
                    "请确认 P2P 连接正常后重新粘贴。");
            } else if (snapshot.lastErrorCode ==
                       "clipboard_capture_timeout") {
                message = QStringLiteral(
                    "本机剪贴板没有及时返回内容。当前粘贴状态已经自动释放，"
                    "请重新复制文件后再按 Ctrl+V。");
            } else if (snapshot.lastErrorCode ==
                       "clipboard_capture_rejected") {
                message = QStringLiteral(
                    "文件无法读取、超过设置中的大小上限，或包含当前不支持的内容。");
            } else if (snapshot.lastErrorCode ==
                       "remote_paste_clipboard_unsupported") {
                message = QStringLiteral(
                    "本机剪贴板中没有可读取的文件或文本。请重新复制文件后再试。");
            } else if (snapshot.lastErrorCode ==
                       "clipboard_apply_failed") {
                message = QStringLiteral(
                    "文件已经传到远端，但远端系统剪贴板未能接收该文件。请重试。");
            } else if (snapshot.lastErrorCode ==
                       "remote_paste_input_unavailable") {
                message = QStringLiteral(
                    "远端文件已准备好，但 Ctrl+V 输入没有成功送达。请确认控制权仍然有效。");
            } else if (snapshot.lastErrorCode ==
                       "clipboard_remote_request_timeout") {
                message = QStringLiteral(
                    "被控端没有及时响应剪贴板请求，请确认连接正常后重新粘贴。");
            } else if (snapshot.lastErrorCode ==
                       "clipboard_local_paste_failed") {
                message = QStringLiteral(
                    "文件已经接收到本机剪贴板，但未能自动送入刚才的目标窗口。"
                    "请回到目标应用再按一次 Ctrl+V。");
            } else if (snapshot.lastErrorCode ==
                       "clipboard_current_unavailable") {
                message = QStringLiteral(
                    "被控端的剪贴板内容已经变化，请重新复制后再粘贴。");
            } else if (snapshot.lastErrorCode ==
                       "clipboard_source_changed") {
                message = QStringLiteral(
                    "源文件在传输过程中发生了变化，本次粘贴已安全取消。"
                    "请等待文件写入完成后重新复制并粘贴。");
            } else if (snapshot.lastErrorCode ==
                       "clipboard_hash_update_failed" ||
                       snapshot.lastErrorCode ==
                           "clipboard_hash_reset_failed") {
                message = QStringLiteral(
                    "文件完整性校验未能继续，本次粘贴已取消。请重新粘贴；"
                    "若问题持续出现，请检查系统加密服务状态。");
            }
            remoteSessionWindow_->ShowRemotePasteFailure(
                completedTransferId, message);
        } else if (!completedTransferId.isEmpty() &&
                   remoteSessionWindow_) {
            remoteSessionWindow_->CompleteRemotePasteProgress(
                completedTransferId);
        }
        pendingRemotePasteDialogId_.clear();
        visibleRemotePasteDialogId_.clear();
    }
    if (receivedAdvanced) {
        RemoteCToast::Show(
            this,
            snapshot.lastFormat == "file" ||
                    snapshot.lastFormat == "directory"
                ? QStringLiteral("已接收远程粘贴文件")
                : QStringLiteral("已接收远程粘贴内容"),
            RemoteCToast::Tone::kSuccess);
    } else if (rejectedAdvanced && !snapshot.lastErrorMessage.empty() &&
               !remoteSessionWindow_) {
        RemoteCToast::Show(
            this,
            QStringLiteral("远程粘贴未完成：%1")
                .arg(QString::fromStdString(snapshot.lastErrorMessage)),
            RemoteCToast::Tone::kError);
    }
    displayedClipboardSentItems_ = snapshot.sentItems;
    displayedClipboardReceivedItems_ = snapshot.receivedItems;
    displayedClipboardRejectedItems_ = snapshot.rejectedItems;
    displayedClipboardErrorCode_ =
        QString::fromStdString(snapshot.lastErrorCode);
}
}  // namespace remote::controller
