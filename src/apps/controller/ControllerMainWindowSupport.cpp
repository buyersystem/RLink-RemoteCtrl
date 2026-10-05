// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

// Out-of-line support used by ControllerMainWindow translation units.

#include "ControllerMainWindowSupport.h"
#include "ControllerMainWindow.h"
#include "src/core/ScreenFrameQualityPolicy.h"

#include <QAction>
#include <QAbstractItemView>
#include <QApplication>
#include <QButtonGroup>
#include <QClipboard>
#include <QCloseEvent>
#include <QColor>
#include <QComboBox>
#include <QCryptographicHash>
#include <QCursor>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QEasingCurve>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QGraphicsOpacityEffect>
#include <QGridLayout>
#include <QHash>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QProcess>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPushButton>
#include <QResizeEvent>
#include <QRegion>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QScrollArea>
#include <QScrollBar>
#include <QScreen>
#include <QSettings>
#include <QSizePolicy>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QStorageInfo>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QStyle>
#include <QStyleHints>
#include <QString>
#include <QStringConverter>
#include <QStringList>
#include <QTextStream>
#include <QTemporaryFile>
#include <QTimer>
#include <QWheelEvent>
#include <QThread>
#include <QSystemTrayIcon>
#include <QToolButton>
#include <QUrl>
#include <QVariant>
#include <QVariantAnimation>
#include <QVBoxLayout>
#include <QWidget>
#include <QWidgetAction>
#include <QWindow>

#include "RoundedPopupMenu.h"

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <dbt.h>
#endif

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <deque>
#include <functional>
#include <filesystem>
#include <mutex>
#include <thread>
#include <tuple>
#include <utility>

#include "CameraWindow.h"
#include "FileTransferWindow.h"
#include "LoginWindow.h"
#include "RemoteSessionWindow.h"
#include "RemoteCComboBox.h"
#include "RemoteCDialog.h"
#include "RemoteCToast.h"
#include "RoomCameraWindow.h"
#include "src/apps/remote/FileTransferController.h"
#include "src/apps/remote/ClipboardController.h"
#include "src/apps/remote/EncoderBenchmarkProfileCache.h"
#include "src/apps/remote/ISessionMediaAccess.h"
#include "src/apps/controller/ui/morph/MorphIconButtonBinding.h"
#include "src/apps/controller/ui/RemoteCTheme.h"
#include "src/core/ISessionEngine.h"
#include "src/webrtc/IWebRtcSession.h"
#include "src/platform/win/FfmpegHardwareH264Encoder.h"
#include "src/platform/win/H264EncoderBenchmark.h"
#include "src/platform/win/WindowsHardwareFingerprint.h"
#include "src/platform/win/VideoDecoderProbePolicy.h"
#include "src/platform/win/VideoEncoderProbePolicy.h"
#include "src/platform/win/WindowsInputExecutor.h"


namespace remote::controller {
namespace detail {


const char kDefaultFileSaveDirectorySetting[] =
    "files/defaultSaveDirectory";

std::uint32_t ConfiguredScreenQualityDeficitShareHundredths()
{
    return NormalizeScreenQualityDeficitShareHundredths(
        QSettings().value(
            QStringLiteral("media/screenQualityDeficitShareHundredths"),
            kDefaultScreenQualityDeficitShareHundredths).toUInt());
}

bool IsNineDigitPublicId(const QString& value)
{
    static const QRegularExpression pattern(
        QStringLiteral("^[1-9][0-9]{8}$"));
    return pattern.match(value).hasMatch();
}

QString LocalizedDirectSessionError(
    const std::string& errorCode,
    const std::string& /*fallbackMessage*/,
    const SessionEngineSnapshot* snapshot)
{
    const QString code = QString::fromStdString(errorCode);
    if (code == QStringLiteral("engine_not_ready")) {
        if (snapshot) {
            if (snapshot->state == SessionEngineState::kStarting) {
                return QStringLiteral("远程服务正在初始化，请稍后重试。");
            }
            if (snapshot->room.membership != RoomMembershipState::kNone &&
                snapshot->room.membership != RoomMembershipState::kFailed) {
                return QStringLiteral(
                    "当前设备已在协助房间中，请先退出房间后再连接其他设备。");
            }
            if (snapshot->state == SessionEngineState::kConnecting ||
                snapshot->state == SessionEngineState::kAwaitingLocalApproval ||
                snapshot->state == SessionEngineState::kActive ||
                snapshot->state == SessionEngineState::kStopping) {
                return QStringLiteral(
                    "当前已有远程会话正在进行，请先结束当前会话后再连接其他设备。");
            }
        }
        return QStringLiteral("远程会话服务暂未就绪，请稍后重试。");
    }
    if (code == QStringLiteral("room_active")) {
        return QStringLiteral(
            "当前设备已在协助房间中，请先退出房间后再发起远程连接。");
    }
    if (code == QStringLiteral("device_busy")) {
        return QStringLiteral(
            "对方设备已在协助房间或其他远程会话中，请结束后再试。");
    }
    if (code == QStringLiteral("verification_code_invalid")) {
        return QStringLiteral(
            "验证码错误，请向对方核对当前显示的 6 位一次性验证码。");
    }
    if (code == QStringLiteral("target_offline")) {
        return QStringLiteral("对方设备当前不在线，请确认 RLink 已运行。");
    }
    if (code == QStringLiteral("rate_limited")) {
        return QStringLiteral("连接尝试过于频繁，请稍后再试。");
    }
    if (code == QStringLiteral("session_request_timeout")) {
        return QStringLiteral("连接请求超时，请确认对方在线后重试。");
    }
    if (code == QStringLiteral("session_rejected")) {
        return QStringLiteral("对方设备未接受本次远程连接。");
    }
    if (code == QStringLiteral("invalid_assistance_credentials")) {
        return QStringLiteral("请输入正确的设备 ID 和 6 位验证码。");
    }
    if (code == QStringLiteral("signaling_not_online")) {
        return QStringLiteral("信令服务尚未连接，请稍后重试。");
    }
    return code.isEmpty()
        ? QStringLiteral("远程连接未能建立，请稍后重试。")
        : QStringLiteral("远程连接未能建立（错误代码：%1），请稍后重试。")
              .arg(code);
}

bool IsDirectRecoveryFailureCode(const std::string& errorCode)
{
    return errorCode == "peer_reconnect_timeout" ||
           errorCode == "ice_restart_exhausted";
}
const char kVideoEncoderPreferenceSetting[] =
    "media/videoEncoderPreference";
const char kFfmpegX264PresetSetting[] =
    "media/ffmpegX264Preset";
const char kFfmpegHardwareBackendSetting[] =
    "media/ffmpegHardwareBackend";
const char kVideoDecoderPreferenceSetting[] =
    "media/videoDecoderPreference";
const char kVideoRendererPreferenceSetting[] =
    "media/videoRendererPreference";
const char kDesktopCaptureBackendSetting[] =
    "media/desktopCaptureBackend";
const char kScreenFrameRateLogEnabledSetting[] =
    "diagnostics/screenFrameRateCsvEnabled";
const char kInputEventStatsEnabledSetting[] =
    "diagnostics/inputEventStatsEnabled";
const char kBestDecoderNameSetting[] =
    "media/decoderProbe/bestDecoderName";
const char kBestDecoderAverageSetting[] =
    "media/decoderProbe/averageLatencyMs";
const char kBestDecoderP95Setting[] =
    "media/decoderProbe/p95LatencyMs";
const char kBestDecoderTestTimeSetting[] =
    "media/decoderProbe/testedAtUtc";
const char kDecoderCandidatesSetting[] =
    "media/decoderProbe/candidates";
const char kDecoderHardwareFingerprintSetting[] =
    "media/decoderProbe/hardwareFingerprint";
const char kDecoderBenchmarkCompletedSetting[] =
    "media/decoderProbe/completed";
const char kDecoderBenchmarkPassedSetting[] =
    "media/decoderProbe/passed";
const char kDecoderBenchmarkPolicyVersionSetting[] =
    "media/decoderProbe/policyVersion";
const int kDecoderBenchmarkPolicyVersion =
    kVideoDecoderProbePolicyVersion;
const char kBestEncoderIdSetting[] =
    "media/encoderProbe/bestEncoderId";
const char kBestEncoderNameSetting[] =
    "media/encoderProbe/bestEncoderName";
const char kEncoderCandidatesSetting[] =
    "media/encoderProbe/candidates";
const char kEncoderHardwareFingerprintSetting[] =
    "media/encoderProbe/hardwareFingerprint";
const char kEncoderCaptureBackendSetting[] =
    "media/encoderProbe/captureBackend";
const char kEncoderX264PresetSetting[] =
    "media/encoderProbe/ffmpegX264Preset";
const char kEncoderBenchmarkCompletedSetting[] =
    "media/encoderProbe/completed";
const char kEncoderBenchmarkPassedSetting[] =
    "media/encoderProbe/passed";
const char kEncoderBenchmarkPolicyVersionSetting[] =
    "media/encoderProbe/policyVersion";
const char kBestEncoderTestTimeSetting[] =
    "media/encoderProbe/testedAtUtc";
const int kEncoderBenchmarkPolicyVersion =
    kVideoEncoderProbePolicyVersion;
const char kCameraDeviceSetting[] =
    "media/cameraDeviceId";
const char kMicrophoneDeviceSetting[] =
    "media/microphoneDeviceId";
const char kSpeakerDeviceSetting[] =
    "media/speakerDeviceId";
const char kDefaultRoomCapacitySetting[] =
    "rooms/defaultCapacity";
const char kCloseButtonBehaviorSetting[] =
    "app/closeButtonBehavior";
const char kStartupVisibilitySetting[] =
    "app/startupVisibility";
const char kWindowsAutoStartValueName[] = "RemoteC";
const char kAutoOpenCameraGallerySetting[] =
    "media/autoOpenCameraGallery";
const char kInterfaceFontFamilySetting[] = "ui/systemFontFamily";

QStringList InstalledChineseInterfaceFonts()
{
    QStringList families;
    for (const QString& family : QFontDatabase::families()) {
        if (family.startsWith(QLatin1Char('@'))) {
            continue;
        }
        const auto writingSystems = QFontDatabase::writingSystems(family);
        if (writingSystems.contains(QFontDatabase::SimplifiedChinese) ||
            writingSystems.contains(QFontDatabase::TraditionalChinese)) {
            families.push_back(family);
        }
    }
    families.removeDuplicates();
    families.sort(Qt::CaseInsensitive);
    return families;
}

void ApplyInterfaceFontPreference(const QString& systemFamily,
                                  int pixelSize)
{
    QFont font;
    if (systemFamily.trimmed().isEmpty()) {
        font.setFamilies({
            QStringLiteral("Microsoft YaHei UI"),
            QStringLiteral("Segoe UI")});
    } else {
        font.setFamilies({systemFamily,
                          QStringLiteral("Microsoft YaHei UI"),
                          QStringLiteral("Segoe UI")});
    }
    font.setHintingPreference(QFont::PreferDefaultHinting);
    font.setStyleStrategy(QFont::PreferAntialias);
    font.setPixelSize(std::clamp(pixelSize, 12, 17));
    QApplication::setFont(font);
}

QString AdaptBenchmarkHtmlForTheme(QString html)
{
    if (!ui::RemoteCTheme::IsDark(ui::RemoteCTheme::LoadPreference())) {
        return html;
    }
    const std::pair<const char*, const char*> colors[] = {
        {"#172033", "#F1F5FB"},
        {"#24324b", "#E6EDF7"},
        {"#667085", "#AEBBD0"},
        {"#7a8496", "#9EACC0"},
        {"#98a2b3", "#7F8DA3"},
        {"#1769e8", "#8EA5FF"},
        {"#138b57", "#4FF0B5"},
        {"#c12b37", "#FF98A5"},
        {"#a76508", "#F2C572"},
        {"#e3e8f1", "#354861"},
        {"#e8ecf3", "#354861"},
    };
    for (const auto& [light, dark] : colors) {
        html.replace(QString::fromLatin1(light),
                     QString::fromLatin1(dark),
                     Qt::CaseInsensitive);
    }
    return html;
}

VideoEncoderPreference VideoEncoderPreferenceFromSetting(
    const QString& value)
{
    if (value == QStringLiteral("hardware")) {
        return VideoEncoderPreference::kHardwareOnly;
    }
    if (value == QStringLiteral("ffmpeg_hardware")) {
        return VideoEncoderPreference::kFfmpegHardware;
    }
    if (value == QStringLiteral("software")) {
        return VideoEncoderPreference::kSoftwareOnly;
    }
    if (value == QStringLiteral("ffmpeg")) {
        return VideoEncoderPreference::kFfmpegX264Only;
    }
    return VideoEncoderPreference::kAutomatic;
}

DesktopCaptureImplementation DesktopCaptureImplementationFromSetting(
    const QString& value)
{
    return value == QStringLiteral("libwebrtc")
        ? DesktopCaptureImplementation::kLibWebRtc
        : DesktopCaptureImplementation::kNativeDxgi;
}

FfmpegHardwareBackend FfmpegHardwareBackendFromSetting(
    const QString& value)
{
    if (value == QStringLiteral("qsv")) {
        return FfmpegHardwareBackend::kQsv;
    }
    if (value == QStringLiteral("nvenc")) {
        return FfmpegHardwareBackend::kNvenc;
    }
    if (value == QStringLiteral("amf")) {
        return FfmpegHardwareBackend::kAmf;
    }
    return FfmpegHardwareBackend::kAutomatic;
}

FfmpegX264Preset EncoderQualityFromSetting(const QString& value)
{
    if (value == QStringLiteral("veryslow")) {
        return FfmpegX264Preset::kVerySlow;
    }
    if (value == QStringLiteral("slow")) {
        return FfmpegX264Preset::kSlow;
    }
    if (value == QStringLiteral("veryfast")) {
        return FfmpegX264Preset::kVeryFast;
    }
    if (value == QStringLiteral("ultrafast")) {
        return FfmpegX264Preset::kUltraFast;
    }
    return FfmpegX264Preset::kMedium;
}

VideoDecoderPreference VideoDecoderPreferenceFromSetting(
    const QString& value)
{
    if (value == QStringLiteral("hardware")) {
        return VideoDecoderPreference::kHardwareOnly;
    }
    if (value == QStringLiteral("software")) {
        return VideoDecoderPreference::kSoftwareOnly;
    }
    return VideoDecoderPreference::kAutomatic;
}
const char kRemoteScreenQualitySetting[] =
    "remoteSession/qualityTier";
const char kDragPointerSampleRateSetting[] =
    "remoteSession/dragPointerSampleRateHz";
const char kRemotePasteEnabledSetting[] =
    "remoteSession/remotePasteEnabled";
const char kClipboardFormatsSetting[] =
    "remoteSession/clipboardFormats";
const char kClipboardFileLimitSetting[] =
    "remoteSession/clipboardFileLimitMiB";
const char kClipboardCacheBaseDirectorySetting[] =
    "transfer/remotePaste/cacheBaseDirectory";
const char kClipboardCacheRetentionSetting[] =
    "transfer/remotePaste/cacheRetentionMinutes";
const char kClipboardCacheCapacitySetting[] =
    "transfer/remotePaste/cacheLimitGiB";
const std::uint64_t kRemotePastePopupThresholdBytes = 64ull * 1024ull;
const int kSettingsControlWidth = 270;

QString InitialClipboardCacheBaseDirectory()
{
    const QString localAppData = qEnvironmentVariable("LOCALAPPDATA");
    return QDir::cleanPath(localAppData.isEmpty()
        ? QDir::tempPath() : localAppData);
}

QString ClipboardCacheRootForBase(const QString& baseDirectory)
{
    return QDir::cleanPath(QDir(baseDirectory).filePath(
        QStringLiteral("RemoteC/ClipboardCache")));
}

qulonglong SafeClipboardCacheCapacityGiB(const QString& baseDirectory)
{
    QStorageInfo storage(baseDirectory);
    if (!storage.isValid() || !storage.isReady()) return 0;
    constexpr qulonglong kGiB = 1024ull * 1024 * 1024;
    return static_cast<qulonglong>(storage.bytesAvailable()) /
        2ull / kGiB;
}

QString WindowsAutoStartCommand()
{
    return QStringLiteral("\"%1\"").arg(
        QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
}

bool WindowsAutoStartEnabled()
{
#ifdef Q_OS_WIN
    QSettings runKey(
        QStringLiteral(
            "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
        QSettings::NativeFormat);
    return runKey.value(QString::fromLatin1(kWindowsAutoStartValueName))
               .toString() == WindowsAutoStartCommand();
#else
    return false;
#endif
}

bool SetWindowsAutoStartEnabled(bool enabled)
{
#ifdef Q_OS_WIN
    QSettings runKey(
        QStringLiteral(
            "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
        QSettings::NativeFormat);
    const QString valueName =
        QString::fromLatin1(kWindowsAutoStartValueName);
    if (enabled) {
        runKey.setValue(valueName, WindowsAutoStartCommand());
    } else {
        runKey.remove(valueName);
    }
    runKey.sync();
    return runKey.status() == QSettings::NoError &&
           WindowsAutoStartEnabled() == enabled;
#else
    Q_UNUSED(enabled);
    return false;
#endif
}

std::pair<std::uint32_t, std::uint32_t> SavedScreenQualityBounds(
    ScreenQualityTier quality)
{
    switch (quality) {
    case ScreenQualityTier::k720p:
        return {1280, 720};
    case ScreenQualityTier::k1080p:
    case ScreenQualityTier::kAutomatic:
        return {1920, 1080};
    case ScreenQualityTier::k1440p:
        return {2560, 1440};
    case ScreenQualityTier::kOriginal:
        return {0, 0};
    }
    return {0, 0};
}

QString ContentSceneDisplayText(const std::string& scene)
{
    if (scene == "code_terminal") return QStringLiteral("代码 / 终端");
    if (scene == "document") return QStringLiteral("文档 / 阅读");
    if (scene == "spreadsheet") return QStringLiteral("表格 / 报表");
    if (scene == "web_app") return QStringLiteral("网页 / 普通软件");
    if (scene == "photo_graphics") return QStringLiteral("图片 / 图形编辑");
    if (scene == "cad_diagram") return QStringLiteral("工程图 / 图示");
    if (scene == "video") return QStringLiteral("视频播放");
    if (scene == "game_3d") return QStringLiteral("游戏 / 实时三维");
    if (scene == "mixed") return QStringLiteral("混合内容");
    return scene.empty() || scene == "unknown" ? QStringLiteral("尚未识别")
        : QString::fromStdString(scene);
}

QString ContentPolicyExecutionDisplayText(const std::string& status)
{
    if (status == "awaiting_scene") return QStringLiteral("等待有效场景识别，尚未执行场景调整");
    if (status == "awaiting_activity") return QStringLiteral("等待当前采集活动窗口，尚未执行新调整");
    if (status == "idle_hold") return QStringLiteral("画面静止，保持规格；活动恢复后重新评估");
    if (status == "healthy_hold") return QStringLiteral("未确认网络压力，保持当前规格；不按参考曲线降级");
    if (status == "awaiting_calibration") return QStringLiteral("等待需求模型标定，未执行场景调整");
    if (status == "awaiting_quality_evidence") return QStringLiteral("等待画质证据，未执行新调整");
    if (status == "awaiting_network_evidence") return QStringLiteral("等待新鲜的 GoogCC 状态与传输反馈，保持当前规格");
    if (status == "awaiting_stream_window") return QStringLiteral("等待当前规格的完整统计窗口，保持已应用规格");
    if (status == "awaiting_processing_evidence") return QStringLiteral("等待编码与接收端负载证据，未执行新调整");
    if (status == "holding_for_evidence") return QStringLiteral("尚无满足预算和质量要求的候选，保持并由网络策略保护");
    if (status == "awaiting_candidate_budget") return QStringLiteral("当前 GoogCC 预算不足以满足候选需求，尚未进入调整确认");
    if (status == "observing") return QStringLiteral("正在观察连续窗口，尚未确认新调整");
    if (status == "observing_emergency") return QStringLiteral("预算不足，正在确认按场景逐步降级；参考画质尚未达标");
    if (status == "applying_emergency") return QStringLiteral("正在应用按场景逐步降级；参考画质尚未达标");
    if (status == "emergency_applied") return QStringLiteral("逐步降级已应用，重新评估网络预算；参考画质尚未达标");
    if (status == "observing_user_restore") return QStringLiteral("网络预算已回升，正在确认恢复用户原规格");
    if (status == "applying_user_restore") return QStringLiteral("正在恢复用户原分辨率与目标帧率");
    if (status == "user_specification_restored") return QStringLiteral("已恢复用户原规格，继续观察画质");
    if (status == "observing_repair") return QStringLiteral("画质需改善，正在确认码率或规格修复");
    if (status == "applying_repair") return QStringLiteral("正在应用码率或规格修复，随后重新验证");
    if (status == "repair_applied") return QStringLiteral("修复参数已应用到 RTP 发送器，等待新窗口验证");
    if (status == "applying") return QStringLiteral("正在应用已确认的场景策略");
    if (status == "applied") return QStringLiteral("场景参数已应用到 RTP 发送器");
    if (status == "apply_failed") return QStringLiteral("参数应用失败，未确认新规格");
    if (status == "sender_busy") return QStringLiteral("发送参数正在更新，等待下一统计窗口");
    if (status == "context_changed_restoring") return QStringLiteral("场景上下文已变化，正在恢复用户范围");
    if (status == "context_restored") return QStringLiteral("已恢复用户范围，重新评估场景");
    if (status == "disabled_restored") return QStringLiteral("内容感知已关闭，已恢复用户上限，使用 WebRTC 原生丢帧与拥塞控制");
    if (status == "disabled") return QStringLiteral("内容感知未启用，使用 WebRTC 原生丢帧与拥塞控制");
    return QStringLiteral("仅观察，未执行场景调整");
}

QString ContentPolicyReasonDisplayText(const std::string& reason)
{
    if (reason == "invalid_input") return QStringLiteral("等待有效规格数据");
    if (reason == "idle_hold") return QStringLiteral("画面空闲，保留规格上限");
    if (reason == "activity_unavailable") return QStringLiteral("等待采集活动信息");
    if (reason == "capacity_unavailable") return QStringLiteral("等待有效网络容量");
    if (reason == "stale_network") return QStringLiteral("网络统计已过期");
    if (reason == "generation_mismatch") return QStringLiteral("网络统计不属于当前会话");
    if (reason == "unknown_scene_hold") return QStringLiteral("场景未知，暂不升级");
    if (reason == "hold") return QStringLiteral("保持当前规格");
    if (reason == "healthy_hold") return QStringLiteral("未确认网络受限，保留当前规格");
    if (reason == "quality_limited") return QStringLiteral("预算不足以满足估算画质需求");
    if (reason == "emergency_network_reduction") return QStringLiteral("网络预算不足，按场景优先级逐步减少负载");
    if (reason == "user_specification_restore") return QStringLiteral("网络预算已接近用户视频上限，恢复原规格并验证画质");
    if (reason == "manual_quality_limited") return QStringLiteral("手动规格超出估算预算");
    if (reason == "protect_resolution") return QStringLiteral("优先保留分辨率，减少发送帧率");
    if (reason == "protect_frame_rate") return QStringLiteral("小幅减少像素，保留当前帧率");
    if (reason == "resolution_upgrade") return QStringLiteral("优先恢复分辨率");
    if (reason == "frame_rate_upgrade") return QStringLiteral("预算允许提高帧率");
    if (reason == "bitrate_recovery") return QStringLiteral("恢复所需编码预算");
    if (reason == "processing_limited") return QStringLiteral("编解码处理能力受限");
    if (reason == "activity_frame_rate_limited") return QStringLiteral("预算受限，暂低于本场景的活动流畅度目标");
    if (reason == "awaiting_quality_evidence") return QStringLiteral("等待画质验证数据");
    return QString::fromStdString(reason);
}

QString FormatBitrate(std::uint64_t bitsPerSecond)
{
    if (bitsPerSecond >= 1'000'000) {
        return QStringLiteral("%1 Mbps")
            .arg(static_cast<double>(bitsPerSecond) / 1'000'000.0,
                 0, 'f', 2);
    }
    if (bitsPerSecond >= 1'000) {
        return QStringLiteral("%1 Kbps")
            .arg(static_cast<double>(bitsPerSecond) / 1'000.0,
                 0, 'f', 1);
    }
    return QStringLiteral("%1 bps").arg(bitsPerSecond);
}

QString FormatByteCount(std::uint64_t bytes)
{
    if (bytes >= 1024ULL * 1024ULL * 1024ULL) {
        return QStringLiteral("%1 GB")
            .arg(static_cast<double>(bytes) /
                     (1024.0 * 1024.0 * 1024.0),
                 0, 'f', 2);
    }
    if (bytes >= 1024ULL * 1024ULL) {
        return QStringLiteral("%1 MB")
            .arg(static_cast<double>(bytes) / (1024.0 * 1024.0),
                 0, 'f', 2);
    }
    if (bytes >= 1024ULL) {
        return QStringLiteral("%1 KB")
            .arg(static_cast<double>(bytes) / 1024.0,
                 0, 'f', 1);
    }
    return QStringLiteral("%1 B").arg(bytes);
}

QString SampleWindowSuffix(std::uint32_t windowMs)
{
    return windowMs > 0
               ? QStringLiteral(" · %1 ms采样窗").arg(windowMs)
               : QStringLiteral(" · 等待第二次采样");
}

QString LatestFrameTimingText(
    const RtpStreamStatsSnapshot& stream,
    const QString& action)
{
    if (!stream.latestFrameTimingAvailable) {
        return QStringLiteral("等待逐帧%1计时").arg(action);
    }
    QString text =
        QStringLiteral("%1 ms · RTP %2 · %3 ms前")
            .arg(stream.latestFrameTimeMs, 0, 'f', 3)
            .arg(stream.latestFrameRtpTimestamp)
            .arg(stream.latestFrameTimingAgeMs);
    if (stream.latestFrameWidth > 0 &&
        stream.latestFrameHeight > 0) {
        text += QStringLiteral(" · %1×%2")
                    .arg(stream.latestFrameWidth)
                    .arg(stream.latestFrameHeight);
    }
    if (stream.latestEncodedBytes > 0) {
        text += QStringLiteral(" · %1")
                    .arg(FormatByteCount(
                        stream.latestEncodedBytes));
    }
    return text;
}

QString RouteDisplayName(const std::string& route)
{
    if (route == "host") {
        return QStringLiteral("局域网/主机直连");
    }
    if (route == "stun") {
        return QStringLiteral("STUN 公网直连");
    }
    if (route == "turn") {
        return QStringLiteral("TURN 中继");
    }
    return QStringLiteral("尚未确定");
}

QString SlotDisplayName(const std::string& slot,
                        const std::string& kind)
{
    if (slot == kScreenMainVideoSlot) {
        return QStringLiteral("屏幕");
    }
    if (slot == kCameraMainVideoSlot) {
        return QStringLiteral("摄像头");
    }
    if (slot == kMicrophoneMainAudioSlot || kind == "audio") {
        return QStringLiteral("麦克风");
    }
    return kind == "video" ? QStringLiteral("视频")
                           : QStringLiteral("媒体");
}

QString CandidateDisplayText(
    const IceCandidateStatsSnapshot& candidate)
{
    if (candidate.address.empty()) {
        return QStringLiteral("未报告");
    }
    QString result = QStringLiteral("%1:%2 · %3/%4")
        .arg(QString::fromStdString(candidate.address))
        .arg(candidate.port)
        .arg(QString::fromStdString(candidate.candidateType))
        .arg(QString::fromStdString(candidate.protocol));
    if (!candidate.networkType.empty()) {
        result += QStringLiteral(" · %1")
                      .arg(QString::fromStdString(
                          candidate.networkType));
    }
    if (!candidate.adapterType.empty()) {
        result += QStringLiteral(" · %1")
                      .arg(QString::fromStdString(
                          candidate.adapterType));
    }
    return result;
}

QString InitialFileSaveDirectory()
{
    QString directory = QStandardPaths::writableLocation(
        QStandardPaths::DownloadLocation);
    if (directory.isEmpty()) {
        directory = QDir::homePath();
    }
    return QDir::cleanPath(directory);
}

void SetBusyStatusAnimation(QLabel* label, bool busy)
{
    if (!label) {
        return;
    }
    auto* previous = label->findChild<QPropertyAnimation*>(
        QStringLiteral("remoteCBusyPulse"));
    if (busy && previous && CurrentUiAnimationLevel() > 0) {
        return;
    }
    if (previous) {
        previous->stop();
        previous->deleteLater();
        label->setGraphicsEffect(nullptr);
    }
    if (!busy || CurrentUiAnimationLevel() <= 0) {
        return;
    }
    auto* effect = new QGraphicsOpacityEffect(label);
    label->setGraphicsEffect(effect);
    auto* animation = new QPropertyAnimation(effect, "opacity", label);
    animation->setObjectName(QStringLiteral("remoteCBusyPulse"));
    animation->setDuration(
        CurrentUiAnimationLevel() == 1 ? 760 : 1050);
    animation->setStartValue(1.0);
    animation->setKeyValueAt(0.5, 0.48);
    animation->setEndValue(1.0);
    animation->setEasingCurve(QEasingCurve::InOutSine);
    animation->setLoopCount(-1);
    animation->start();
}

void AnimateSmallUiChange(QWidget* widget)
{
    const int level = CurrentUiAnimationLevel();
    if (!widget || level <= 0) {
        return;
    }
    auto* effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    auto* animation = new QPropertyAnimation(effect, "opacity", effect);
    animation->setDuration(level == 1 ? 85 : 135);
    animation->setStartValue(0.45);
    animation->setEndValue(1.0);
    QObject::connect(animation, &QPropertyAnimation::finished, widget,
                     [widget, effect] {
                if (widget->graphicsEffect() == effect) {
                    widget->setGraphicsEffect(nullptr);
                }
            });
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

const char kMainStyle[] = R"(
QMainWindow {
    background: #F5F4F0;
}
QWidget#customTitleBar {
    background: #FAFAF7;
    border-bottom: 1px solid #DDE0E4;
}
QLabel#titleBarAppName {
    color: #172033;
    font-size: 13px;
    font-weight: 700;
}
QLabel#titleBarDivider, QLabel#titleBarTitle {
    color: #718096;
    font-size: 12px;
}
QToolButton#titleBarButton, QToolButton#titleBarCloseButton {
    background: transparent;
    border: none;
    color: #637087;
    font-family: "Segoe UI Symbol";
    font-size: 15px;
}
QToolButton#titleBarButton:hover {
    background: #e9eef6;
    color: #172033;
}
QToolButton#titleBarCloseButton:hover {
    background: #d84a55;
    color: white;
}
QWidget {
    color: #172033;
    font-size: 13px;
}
QFrame#sidebar {
    background: #F8F7F3;
    border-right: 1px solid #DDE0E4;
}
QFrame#navigationIndicator {
    background: #315efb;
    border: none;
    border-radius: 2px;
}
QLabel#brandMark {
    background: #5b6cf9;
    border-radius: 10px;
    color: white;
    font-size: 18px;
    font-weight: 800;
}
QLabel#brandName {
    color: #172033;
    font-size: 18px;
    font-weight: 700;
}
QLabel#brandCaption, QLabel#sidebarCaption {
    color: #8491a6;
    font-size: 11px;
}
QPushButton[nav="true"] {
    background: transparent;
    border: none;
    border-radius: 9px;
    color: #526079;
    font-size: 14px;
    outline: none;
    padding: 11px 14px;
    text-align: left;
}
QPushButton[nav="true"]:hover {
    background: #ECEFEE;
    color: #315efb;
}
QPushButton[navActive="true"] {
    background: #E5E9F7;
    color: #315efb;
    font-weight: 600;
}
QFrame#profileCard {
    background: #F1F2F0;
    border: 1px solid #D9DDE2;
    border-radius: 11px;
}
QFrame#profileCard:hover {
    background: #E9ECEE;
    border-color: #C9CED6;
}
QToolButton#profileUpdateButton {
    background: #E8EDFF;
    border: 1px solid #C8D2FF;
    border-radius: 10px;
    padding: 6px;
}
QToolButton#profileUpdateButton:hover {
    background: #DCE4FF;
    border-color: #AAB9FF;
}
QToolButton#profileUpdateButton:pressed {
    background: #CDD8FF;
    border-color: #8FA3F7;
}
QLabel#profileAvatar {
    background: #315EFB;
    border-radius: 17px;
    color: white;
    font-weight: 700;
}
QLabel#profileName {
    color: #20304a;
    font-weight: 600;
}
QLabel#profileState {
    color: #168a5b;
    font-size: 11px;
}
QFrame#accountMenu {
    background: #FFFEFB;
    border: 1px solid #D9DDE2;
    border-radius: 16px;
}
QWidget#accountMenuHeader { background: transparent; }
QLabel#accountMenuAvatar {
    background: #315EFB;
    border: none;
    border-radius: 19px;
    color: #FFFFFF;
    font-size: 13px;
    font-weight: 750;
}
QLabel#accountMenuName {
    color: #172033;
    font-size: 14px;
    font-weight: 750;
}
QLabel#accountMenuDetail {
    color: #667085;
    font-size: 10px;
}
QFrame#accountMenuSeparator {
    background: #E1E4E8;
    border: none;
}
QPushButton#accountMenuAction {
    background: transparent;
    border: none;
    border-radius: 10px;
    color: #344054;
    font-size: 13px;
    font-weight: 600;
    padding: 0 12px;
    text-align: left;
}
QPushButton#accountMenuAction:hover {
    background: #ECEFEE;
    color: #172033;
}
QPushButton#accountMenuAction[accountHover="true"] {
    background: #ECEFEE;
    color: #172033;
}
QPushButton#accountMenuAction[tone="danger"] { color: #B4232F; }
QPushButton#accountMenuAction[tone="danger"]:hover {
    background: #FFF0F1;
    color: #B4232F;
}
QPushButton#accountMenuAction[tone="danger"][accountHover="true"] {
    background: #FFF0F1;
    color: #B4232F;
}
QLabel#deviceLoginMark {
    background: #5668f6;
    border-radius: 19px;
    color: white;
    font-size: 30px;
    font-weight: 750;
}
QFrame#deviceLoginHero {
    background: #FFFEFB;
    border: 1px solid #DDE0E4;
    border-radius: 22px;
}
QLabel#deviceLoginBrand {
    color: #14203a;
    font-size: 18px;
    font-weight: 750;
}
QLabel#deviceLoginTitle {
    color: #14203a;
    font-size: 27px;
    font-weight: 780;
}
QLabel#deviceLoginStatus {
    color: #68758c;
    font-size: 14px;
}
QLabel#deviceLoginArtwork {
    background: #f1f4ff;
    border: 1px solid #e4e9ff;
    border-radius: 75px;
    color: #6c7df4;
    font-family: "Segoe UI Symbol";
    font-size: 31px;
    font-weight: 700;
}
QPushButton#deviceLoginButton {
    background: #5264f5;
    border: none;
    border-radius: 12px;
    color: white;
    font-size: 16px;
    font-weight: 750;
    padding: 0 30px;
}
QPushButton#deviceLoginButton:hover { background: #4356ec; }
QPushButton#deviceLoginButton:pressed { background: #3749d7; }
QPushButton#deviceLoginButton:disabled {
    background: #aab3df;
    color: #eef0fa;
}
QLabel#deviceLoginSecureNote {
    color: #63718a;
    font-size: 12px;
}
QFrame#deviceLoginFeatures {
    background: #F8F7F3;
    border: 1px solid #E1E3E6;
    border-radius: 15px;
}
QLabel#deviceFeatureIcon {
    background: #e9edff;
    border-radius: 12px;
    color: #5264f5;
    font-family: "Segoe UI Symbol";
    font-size: 20px;
    font-weight: 700;
}
QLabel#deviceFeatureTitle {
    color: #26324a;
    font-size: 13px;
    font-weight: 700;
}
QLabel#deviceFeatureDetail {
    color: #8290a6;
    font-size: 11px;
}
QLabel#pageTitle {
    color: #1d1d1f;
    font-size: 25px;
    font-weight: 750;
}
QLabel#pageSubtitle, QLabel[muted="true"] {
    color: #536176;
}
QCheckBox#visionApiConsentCheckBox {
    color: #344054;
}
QFrame[card="true"] {
    background: #FFFEFB;
    border: 1px solid #DDE0E4;
    border-radius: 14px;
}
QMainWindow[themeRoot="light"] QFrame[card="true"]:hover {
    background: #F4F7FF;
    border-color: #AEBFDD;
}
QLabel#cardTitle {
    color: #172033;
    font-size: 16px;
    font-weight: 700;
}
QLabel#sectionTitle {
    color: #172033;
    font-size: 17px;
    font-weight: 700;
}
QLabel#eyebrow {
    color: #007aff;
    font-size: 11px;
    font-weight: 700;
}
QLineEdit#deviceIdInput, QLineEdit#roomIdInput {
    background: #F8F7F3;
    border: 1px solid #DDE0E4;
    border-radius: 10px;
    color: #111827;
    font-size: 16px;
    padding: 0 15px;
    selection-background-color: #007aff;
}
QLineEdit#deviceIdInput:focus, QLineEdit#roomIdInput:focus {
    background: #FFFEFB;
    border: 1px solid #007aff;
}
QLineEdit#assistDeviceIdInput, QLineEdit#assistVerificationInput {
    background: #F8F7F3;
    border: 1px solid #DDE0E4;
    border-radius: 12px;
    color: #172033;
    font-size: 15px;
    padding: 0 16px;
    selection-background-color: #5668f7;
}
QLineEdit#assistVerificationInput {
    background: #F3F2F7;
    border-color: #d9def8;
    color: #3442a4;
    font-size: 16px;
    font-weight: 700;
    letter-spacing: 1px;
}
QLineEdit#assistDeviceIdInput:hover,
QLineEdit#assistVerificationInput:hover {
    background: #FFFEFB;
    border-color: #b9c4dc;
}
QLineEdit#assistDeviceIdInput:focus,
QLineEdit#assistVerificationInput:focus {
    background: #FFFEFB;
    border: 2px solid #5668f7;
    padding: 0 15px;
}
QComboBox#capacitySelector {
    background: #F8F7F3;
    border: 1px solid #DDE0E4;
    border-radius: 9px;
    color: #172033;
    min-width: 88px;
    padding: 8px 34px 8px 11px;
}
QComboBox#capacitySelector::drop-down {
    border: none;
    width: 32px;
}
)" R"(
QDoubleSpinBox#screenVideoBitrateBppInput,
QDoubleSpinBox#screenQualityDeficitShareInput {
    background: #F8F7F3;
    border: 1px solid #DDE0E4;
    border-radius: 9px;
    color: #172033;
    min-height: 28px;
    padding: 8px 11px;
}
QDoubleSpinBox#screenVideoBitrateBppInput:focus,
QDoubleSpinBox#screenQualityDeficitShareInput:focus {
    background: #FFFEFB;
    border-color: #007aff;
}
QDoubleSpinBox#screenVideoBitrateBppInput QLineEdit,
QDoubleSpinBox#screenQualityDeficitShareInput QLineEdit {
    background: transparent;
    border: none;
    padding: 0;
    min-height: 0;
}
QComboBox#capacitySelector::down-arrow { image: none; }
QComboBox#capacitySelector:focus {
    background: #FFFEFB;
    border: 1px solid #007aff;
}
QComboBox#capacitySelector:disabled {
    background: #EEEFEF;
    color: #667085;
}
QComboBox#capacitySelector QAbstractItemView {
    background: #FFFEFB;
    border: 1px solid #DDE0E4;
    border-radius: 8px;
    padding: 4px;
    color: #172033;
    selection-background-color: #007aff;
    selection-color: white;
}
QPushButton#primaryButton {
    background: #007aff;
    border: none;
    border-radius: 10px;
    color: white;
    font-size: 14px;
    font-weight: 700;
    padding: 0 22px;
}
QPushButton#primaryButton:hover {
    background: #006ee6;
}
QPushButton#primaryButton:pressed {
    background: #005dcc;
}
QPushButton#primaryButton:disabled {
    background: #D8D5D0;
    color: #667085;
}
QPushButton#softButton {
    background: #eef5ff;
    border: 1px solid #d6e3f7;
    border-radius: 8px;
    color: #006ee6;
    font-weight: 600;
    padding: 8px 13px;
}
QPushButton#softButton:hover {
    background: #e1efff;
}
QPushButton#softButton:disabled {
    background: #EEEFEF;
    color: #667085;
}
QPushButton#screenFrameRateLogButton {
    background: #eef5ff;
    border: none;
    border-radius: 8px;
    color: #006ee6;
    font-weight: 650;
    padding: 8px 13px;
}
QPushButton#screenFrameRateLogButton:hover {
    background: #e1efff;
}
QPushButton#screenFrameRateLogButton:checked {
    background: #1769e8;
    color: #ffffff;
}
QPushButton#mediaIconButton {
    background: #F1F2F0;
    border: 1px solid #D9DDE2;
    border-radius: 9px;
    outline: none;
    padding: 5px;
}
QPushButton#mediaIconButton:hover {
    background: #e9edf4;
    border-color: #d4dae4;
}
QPushButton#mediaIconButton:pressed {
    background: #e0e6ef;
}
QPushButton#mediaIconButton:disabled {
    background: #f6f7f9;
    border-color: #eceef2;
}
QPushButton#dangerButton {
    background: #fff0f1;
    border: 1px solid #f0c5c9;
    border-radius: 8px;
    color: #b4232f;
    font-weight: 650;
    padding: 8px 14px;
}
QPushButton#dangerButton:hover {
    background: #ffe3e5;
}
QPushButton#dangerButton:disabled {
    background: #EEEFEF;
    border-color: #DDE0E4;
    color: #667085;
}
QFrame#deviceTile {
    background: #FAFAF7;
    border: 1px solid #DDE0E4;
    border-radius: 11px;
}
QMainWindow[themeRoot="light"] QFrame#deviceTile:hover {
    background: #F4F7FF;
    border: 1px solid #AEBFDD;
}
QLabel#deviceIcon {
    background: #edf0ff;
    border-radius: 10px;
    color: #5364e8;
    font-size: 18px;
    font-weight: 700;
}
QLabel#deviceName {
    color: #172033;
    font-size: 14px;
    font-weight: 650;
}
QLabel#onlineState {
    color: #169b62;
    font-size: 11px;
}
QLabel#offlineState {
    color: #667085;
    font-size: 11px;
}
QLabel#localId {
    color: #111827;
    font-size: 25px;
    font-weight: 700;
    letter-spacing: 2px;
}
QLabel#passwordValue {
    color: #26324a;
    font-family: "Consolas";
    font-size: 17px;
    font-weight: 700;
    letter-spacing: 3px;
}
QFrame#capabilityBox {
    background: #F8F7F3;
    border: 1px solid #E1E3E6;
    border-radius: 10px;
}
QFrame#settingRow, QFrame#emptyState {
    background: #FFFEFB;
    border: 1px solid #DDE0E4;
    border-radius: 14px;
}
QFrame#aboutCard {
    background: #FAFAF7;
    border: 1px solid #DDE0E4;
    border-radius: 14px;
}
QLabel#aboutLogo {
    background: transparent;
    border: none;
}
QLabel#aboutTitle {
    color: #172033;
    font-size: 17px;
    font-weight: 750;
}
QLabel#aboutVersion {
    background: #e9efff;
    border: 1px solid #d7e1ff;
    border-radius: 9px;
    color: #1769e8;
    font-size: 12px;
    font-weight: 700;
    padding: 3px 9px;
}
QFrame#settingsCategoryPanel {
    background: #F8F7F3;
    border: 1px solid #DDE0E4;
    border-radius: 14px;
}
QPushButton#settingsCategoryButton {
    background: transparent;
    border: none;
    border-radius: 10px;
    color: #5f6b80;
    font-size: 14px;
    font-weight: 600;
    padding: 0 14px;
    text-align: left;
    outline: none;
}
QPushButton#settingsCategoryButton:hover {
    background: #ECEFEE;
    color: #24324b;
}
QPushButton#settingsCategoryButton:checked {
    background: #e9efff;
    color: #1769e8;
    font-weight: 700;
}
QLabel#settingsDetailTitle {
    color: #172033;
    font-size: 21px;
    font-weight: 750;
}
QLabel#settingTitle {
    color: #1d1d1f;
    font-size: 14px;
    font-weight: 650;
}
QLabel#decoderBenchmarkSummary,
QLabel#encoderBenchmarkSummary {
    background: #F8F7F3;
    border: 1px solid #DDE0E4;
    border-radius: 11px;
    color: #344054;
    padding: 13px 15px;
}
QLabel#debugValue {
    color: #25324a;
    font-size: 14px;
    font-weight: 650;
}
QLabel#debugValue[tone="good"] {
    color: #138b57;
}
QLabel#debugValue[tone="warning"] {
    color: #a76508;
}
QLabel#debugValue[tone="error"] {
    color: #c12b37;
}
QLabel#debugValue[tone="muted"] {
    color: #536176;
    font-weight: 500;
}
QLabel#statsSectionTitle {
    color: #172033;
    font-size: 16px;
    font-weight: 750;
}
QLabel#statsSectionDescription {
    color: #53627a;
    font-size: 13px;
}
QLabel#statsCountPill, QLabel#statsPeerPill {
    background: #edf4ff;
    border: 1px solid #dbe8fb;
    border-radius: 9px;
    color: #1769e8;
    font-size: 11px;
    font-weight: 650;
    padding: 3px 8px;
}
QFrame#statsMetricCard {
    background: #FFFEFB;
    border: 1px solid #DDE0E4;
    border-radius: 13px;
}
QLabel#statsCardTitle {
    color: #172033;
    font-size: 15px;
    font-weight: 750;
}
QToolButton#statsSectionToggle, QToolButton#statsCardToggle {
    background: transparent;
    border: none;
    color: #172033;
    font-weight: 750;
    outline: none;
    padding: 3px 2px;
    text-align: left;
}
QToolButton#statsSectionToggle {
    font-size: 16px;
}
QToolButton#statsCardToggle {
    font-size: 15px;
}
QToolButton#statsSectionToggle:hover,
QToolButton#statsCardToggle:hover {
    color: #1769e8;
}
QFrame#statsMetricChip {
    background: #f6f8fb;
    border: 1px solid #e7ebf1;
    border-radius: 10px;
}
QFrame#statsMetricChip[tone="good"] {
    background: #eef9f3;
    border-color: #d5efe1;
}
QFrame#statsMetricChip[tone="warning"] {
    background: #fff8e9;
    border-color: #f4e4b8;
}
QFrame#statsMetricChip[tone="error"] {
    background: #fff1f2;
    border-color: #f4d4d8;
}
QLabel#statsChipLabel {
    color: #34435d;
    font-size: 12px;
    font-weight: 700;
}
QLabel#statsChipValue {
    color: #172033;
    font-size: 13px;
    font-weight: 650;
}
QFrame#statsMetricChip[tone="good"] QLabel#statsChipValue {
    color: #128553;
}
QFrame#statsMetricChip[stackedMetrics="true"] QLabel#statsChipLabel {
    font-size: 12px;
    font-weight: 400;
}
QFrame#statsMetricChip[stackedMetrics="true"] QLabel#statsChipValue {
    font-size: 15px;
    font-weight: 600;
}
QFrame#statsMetricChip[tone="warning"] QLabel#statsChipValue {
    color: #9b6208;
}
QFrame#statsMetricChip[tone="error"] QLabel#statsChipValue {
    color: #bd2c38;
}
QLabel#statsEmptyText {
    color: #8a95a8;
    font-size: 13px;
}
QLabel#emptyStateIcon {
    background: #eef5ff;
    border-radius: 24px;
    color: #007aff;
    font-size: 21px;
    font-weight: 700;
}
QLabel#emptyStateTitle {
    color: #1d1d1f;
    font-size: 17px;
    font-weight: 700;
}
QFrame#roomMediaBar {
    background: #f7f8fc;
    border: 1px solid #e5e8f1;
    border-radius: 10px;
}
QFrame#roomSectionPanel {
    background: #f7f8fc;
    border: 1px solid #e5e8f1;
    border-radius: 10px;
}
QLabel#roomMediaTitle {
    color: #27334a;
    font-weight: 700;
}
QLabel#readyPill {
    background: #e8f8f0;
    border-radius: 9px;
    color: #138b57;
    font-size: 11px;
    font-weight: 700;
    padding: 4px 9px;
}
QLabel#previewHint {
    background: #fff7e6;
    border: 1px solid #f4ddb0;
    border-radius: 9px;
    color: #9a6513;
    padding: 8px 11px;
}
QLabel#roomIdValue {
    color: #111827;
    font-size: 21px;
    font-weight: 700;
    letter-spacing: 1px;
}
QLabel[roomValue="true"] {
    color: #27334a;
    font-weight: 650;
}
QLabel#roomStageHint {
    background: #eef2ff;
    border: 1px solid #d9defe;
    border-radius: 9px;
    color: #5361c9;
    padding: 9px 11px;
}
QListWidget#roomMemberList {
    background: #f8f9fc;
    border: 1px solid #e3e7ee;
    border-radius: 10px;
    color: #27334a;
    outline: none;
    padding: 5px;
}
QListWidget#roomMemberList::item {
    border-bottom: 1px solid #e8ebf0;
    padding: 0px;
}
QListWidget#roomMemberList::item:last {
    border-bottom: none;
}
QListWidget#roomMemberList::item:selected {
    background: #edf0ff;
    color: #27334a;
}
QScrollArea {
    background: transparent;
    border: none;
}
QFrame#divider {
    color: #e9edf3;
}
QScrollBar:vertical {
    background: transparent;
    width: 8px;
    margin: 2px;
}
QScrollBar::handle:vertical {
    background: #cbd2dd;
    border-radius: 4px;
    min-height: 30px;
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0;
}
)";

QPixmap DrawNavigationIcon(NavigationIcon icon, const QColor& color)
{
    QPixmap pixmap(36, 36);
    pixmap.setDevicePixelRatio(2.0);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QString iconName;
    switch (icon) {
    case NavigationIcon::kRoom:
        iconName = QStringLiteral("users-round");
        break;
    case NavigationIcon::kDevice:
        iconName = QStringLiteral("monitor");
        break;
    case NavigationIcon::kOwnedDevices:
        iconName = QStringLiteral("monitor-smartphone");
        break;
    case NavigationIcon::kRecent:
        iconName = QStringLiteral("rotate-ccw-clock");
        break;
    case NavigationIcon::kTransfer:
        iconName = QStringLiteral("folder-sync");
        break;
    case NavigationIcon::kDebug:
        iconName = QStringLiteral("activity");
        break;
    case NavigationIcon::kSettings:
        iconName = QStringLiteral("settings");
        break;
    case NavigationIcon::kHelp:
        iconName = QStringLiteral("message-circle-question-mark");
        break;
    case NavigationIcon::kAuthor:
        iconName = QStringLiteral("circle-user-round");
        break;
    }
    QIcon(QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(iconName))
        .paint(&painter, QRect(0, 0, 18, 18));
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(QRect(0, 0, 18, 18), color);
    return pixmap;
}

QIcon MakeNavigationIcon(NavigationIcon icon, bool dark)
{
    QIcon result;
    const QColor normal(dark ? QStringLiteral("#AEBBD0")
                             : QStringLiteral("#64748B"));
    const QColor active(dark ? QStringLiteral("#8EA5FF")
                             : QStringLiteral("#315EFB"));
    result.addPixmap(DrawNavigationIcon(icon, normal),
                     QIcon::Normal, QIcon::Off);
    result.addPixmap(DrawNavigationIcon(icon, active),
                     QIcon::Active, QIcon::Off);
    result.addPixmap(DrawNavigationIcon(icon, active),
                     QIcon::Normal, QIcon::On);
    result.addPixmap(DrawNavigationIcon(icon, active),
                     QIcon::Active, QIcon::On);
    return result;
}

QScrollArea* MakePageSurface(const QString& title,
                             const QString& subtitle,
                             QWidget* parent,
                             QVBoxLayout** pageLayout)
{
    auto* scrollArea = new QScrollArea(parent);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    EnableSmoothWheelScrolling(scrollArea);
    auto* content = new QWidget(scrollArea);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(32, 28, 32, 30);
    layout->setSpacing(20);

    auto* header = new QVBoxLayout();
    header->setSpacing(3);
    auto* titleLabel = new QLabel(title, content);
    titleLabel->setObjectName(QStringLiteral("pageTitle"));
    auto* subtitleLabel = new QLabel(subtitle, content);
    subtitleLabel->setObjectName(QStringLiteral("pageSubtitle"));
    subtitleLabel->setWordWrap(true);
    header->addWidget(titleLabel);
    header->addWidget(subtitleLabel);
    layout->addLayout(header);
    scrollArea->setWidget(content);
    *pageLayout = layout;
    return scrollArea;
}

QPushButton* MakeNavigationButton(const QString& text,
                                  NavigationIcon icon,
                                  bool active,
                                  QWidget* parent)
{
    auto* button = new QPushButton(text, parent);
    const bool dark = ui::RemoteCTheme::IsDark(
        ui::RemoteCTheme::LoadPreference());
    button->setIcon(MakeNavigationIcon(icon, dark));
    button->setIconSize(QSize(18, 18));
    button->setCheckable(true);
    button->setChecked(active);
    button->setProperty("nav", true);
    button->setProperty("navActive", active);
    button->setProperty("navigationIcon", static_cast<int>(icon));
    button->setCursor(Qt::PointingHandCursor);
    button->setFixedHeight(43);
    QString source;
    QString target;
    bool enableMorph = true;
    switch (icon) {
    case NavigationIcon::kRoom:
        source = QStringLiteral("user-round");
        target = QStringLiteral("users-round");
        break;
    case NavigationIcon::kDevice:
        source = QStringLiteral("monitor");
        target = QStringLiteral("handshake");
        break;
    case NavigationIcon::kOwnedDevices:
        source = QStringLiteral("monitor-smartphone");
        target = QStringLiteral("monitor-check");
        break;
    case NavigationIcon::kRecent:
        enableMorph = false;
        break;
    case NavigationIcon::kTransfer:
        enableMorph = false;
        break;
    case NavigationIcon::kDebug:
        enableMorph = false;
        break;
    case NavigationIcon::kSettings:
        enableMorph = false;
        break;
    case NavigationIcon::kHelp:
    case NavigationIcon::kAuthor:
        enableMorph = false;
        break;
    }
    if (enableMorph) {
        button->setProperty("navigationMorphSource", source);
        button->setProperty("navigationMorphTarget", target);
        button->setProperty("remoteCMorphCheckedColor",
                            dark ? QStringLiteral("#8EA5FF")
                                 : QStringLiteral("#315EFB"));
        remotec::ui::morph::MorphIconButtonBinding::attach(
            button,
            QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(source),
            QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(target),
            remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
            QSize(18, 18),
            QColor(dark ? QStringLiteral("#AEBBD0")
                        : QStringLiteral("#64748B")),
            QColor(dark ? QStringLiteral("#8EA5FF")
                        : QStringLiteral("#315EFB")));
    }
    return button;
}

QFrame* MakeDivider(QWidget* parent)
{
    auto* line = new QFrame(parent);
    line->setObjectName(QStringLiteral("divider"));
    line->setFrameShape(QFrame::HLine);
    return line;
}

void AddRoomCapacityItems(QComboBox* comboBox)
{
    for (std::uint32_t capacity = kMinimumRoomMembers;
         capacity <= kProtocolMaximumRoomMembers; ++capacity) {
        comboBox->addItem(QStringLiteral("%1 人").arg(capacity),
                          QVariant::fromValue(capacity));
    }
    const std::uint32_t configuredCapacity = std::clamp(
        QSettings().value(
            QString::fromLatin1(kDefaultRoomCapacitySetting),
            QVariant::fromValue(kDefaultRoomCapacity)).toUInt(),
        kMinimumRoomMembers, kProtocolMaximumRoomMembers);
    comboBox->setCurrentIndex(comboBox->findData(
        QVariant::fromValue(configuredCapacity)));
}

QString MemberDisplayName(const RoomSnapshot& room,
                          const std::string& deviceId,
                          const std::string& localDeviceId)
{
    if (deviceId.empty()) {
        return {};
    }
    const auto member = std::find_if(
        room.members.begin(), room.members.end(),
        [&deviceId](const RoomMemberSnapshot& candidate) {
            return candidate.deviceId == deviceId;
        });
    QString displayName;
    if (member != room.members.end() && !member->deviceName.empty()) {
        displayName = QString::fromStdString(member->deviceName);
    } else {
        displayName = QString::fromStdString(deviceId);
    }
    if (deviceId == localDeviceId) {
        displayName += QStringLiteral("（本机）");
    }
    return displayName;
}

QString ConnectivityDebugName(SessionConnectivityState state)
{
    switch (state) {
    case SessionConnectivityState::kNotConfigured:
        return QStringLiteral("未配置");
    case SessionConnectivityState::kConnecting:
        return QStringLiteral("正在连接");
    case SessionConnectivityState::kOnline:
        return QStringLiteral("在线");
    case SessionConnectivityState::kOffline:
        return QStringLiteral("离线");
    case SessionConnectivityState::kFailed:
        return QStringLiteral("连接失败");
    }
    return QStringLiteral("未知");
}

QString RoomMembershipDebugName(RoomMembershipState state)
{
    switch (state) {
    case RoomMembershipState::kNone:
        return QStringLiteral("未加入房间");
    case RoomMembershipState::kCreating:
        return QStringLiteral("正在创建");
    case RoomMembershipState::kJoinPending:
        return QStringLiteral("等待加入");
    case RoomMembershipState::kActive:
        return QStringLiteral("房间活动中");
    case RoomMembershipState::kLeaving:
        return QStringLiteral("正在离开");
    case RoomMembershipState::kFailed:
        return QStringLiteral("房间操作失败");
    }
    return QStringLiteral("未知");
}

QString PeerConnectionDebugName(RoomPeerConnectionState state)
{
    switch (state) {
    case RoomPeerConnectionState::kStarting:
        return QStringLiteral("正在启动");
    case RoomPeerConnectionState::kNegotiating:
        return QStringLiteral("正在协商");
    case RoomPeerConnectionState::kConnecting:
        return QStringLiteral("正在连接");
    case RoomPeerConnectionState::kActive:
        return QStringLiteral("已连接");
    case RoomPeerConnectionState::kDisconnected:
        return QStringLiteral("网络暂时断开");
    case RoomPeerConnectionState::kRecovering:
        return QStringLiteral("正在恢复连接");
    case RoomPeerConnectionState::kFailed:
        return QStringLiteral("连接失败");
    case RoomPeerConnectionState::kClosed:
        return QStringLiteral("已关闭");
    }
    return QStringLiteral("未知");
}

QString MediaDeviceSelectionDebugName(
    MediaDeviceSelectionState state)
{
    switch (state) {
    case MediaDeviceSelectionState::kReady:
        return QStringLiteral("可用");
    case MediaDeviceSelectionState::kSwitching:
        return QStringLiteral("正在切换");
    case MediaDeviceSelectionState::kUnavailable:
        return QStringLiteral("设备不可用");
    case MediaDeviceSelectionState::kFailed:
        return QStringLiteral("切换失败");
    }
    return QStringLiteral("未知");
}

QString MediaDeviceDebugName(
    const MediaDeviceCategorySnapshot& category,
    const std::string& deviceId)
{
    if (deviceId.empty()) {
        return QStringLiteral("未激活");
    }
    if (deviceId == kSystemDefaultMediaDeviceId) {
        return QStringLiteral("跟随系统默认 [default]");
    }
    const auto device = std::find_if(
        category.devices.begin(), category.devices.end(),
        [&deviceId](const MediaDeviceDescriptor& candidate) {
            return candidate.id == deviceId;
        });
    const QString id = QString::fromStdString(deviceId);
    if (device == category.devices.end() ||
        device->name.empty()) {
        return id;
    }
    return QStringLiteral("%1 [%2]")
        .arg(QString::fromStdString(device->name), id);
}

QString MediaDeviceCategoryDebugText(
    const MediaDeviceCategorySnapshot& category)
{
    QString activeDevice = MediaDeviceDebugName(
        category, category.activeDeviceId);
    if (!category.activeDeviceName.empty()) {
        activeDevice = QString::fromStdString(
            category.activeDeviceName);
        if (category.activeDeviceId ==
            kSystemDefaultMediaDeviceId) {
            activeDevice += QStringLiteral("（系统默认）");
        }
    }
    QString result = QStringLiteral(
        "首选：%1\n活动：%2\n状态：%3")
        .arg(
            MediaDeviceDebugName(
                category, category.preferredDeviceId),
            activeDevice,
            MediaDeviceSelectionDebugName(category.state));
    if (!category.errorMessage.empty()) {
        result += QStringLiteral("\n错误：%1")
                      .arg(QString::fromStdString(
                          category.errorMessage));
    }
    return result;
}


}  // namespace detail
}  // namespace remote::controller
