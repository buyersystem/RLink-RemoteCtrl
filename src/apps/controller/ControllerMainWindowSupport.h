// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <string>
#include <utility>

#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QString>
#include <QStringList>

#include "src/core/DesktopCaptureTypes.h"
#include "src/core/MediaDevice.h"
#include "src/core/SessionDiagnostics.h"
#include "src/core/SessionEngineTypes.h"
#include "src/protocol/ScreenShareControlProtocol.h"
#include "src/webrtc/VideoDecoderRuntimeStatus.h"
#include "src/webrtc/VideoEncoderRuntimeStatus.h"

class QComboBox;
class QFrame;
class QLabel;
class QPushButton;
class QScrollArea;
class QVBoxLayout;
class QWidget;

namespace remote::controller::detail {

extern const char kDefaultFileSaveDirectorySetting[];
extern const char kVideoEncoderPreferenceSetting[];
extern const char kFfmpegX264PresetSetting[];
extern const char kFfmpegHardwareBackendSetting[];
extern const char kVideoDecoderPreferenceSetting[];
extern const char kVideoRendererPreferenceSetting[];
extern const char kDesktopCaptureBackendSetting[];
extern const char kScreenFrameRateLogEnabledSetting[];
extern const char kInputEventStatsEnabledSetting[];
extern const char kBestDecoderNameSetting[];
extern const char kBestDecoderAverageSetting[];
extern const char kBestDecoderP95Setting[];
extern const char kBestDecoderTestTimeSetting[];
extern const char kDecoderCandidatesSetting[];
extern const char kDecoderHardwareFingerprintSetting[];
extern const char kDecoderBenchmarkCompletedSetting[];
extern const char kDecoderBenchmarkPassedSetting[];
extern const char kDecoderBenchmarkPolicyVersionSetting[];
extern const int kDecoderBenchmarkPolicyVersion;
extern const char kBestEncoderIdSetting[];
extern const char kBestEncoderNameSetting[];
extern const char kEncoderCandidatesSetting[];
extern const char kEncoderHardwareFingerprintSetting[];
extern const char kEncoderCaptureBackendSetting[];
extern const char kEncoderX264PresetSetting[];
extern const char kEncoderBenchmarkCompletedSetting[];
extern const char kEncoderBenchmarkPassedSetting[];
extern const char kEncoderBenchmarkPolicyVersionSetting[];
extern const char kBestEncoderTestTimeSetting[];
extern const int kEncoderBenchmarkPolicyVersion;
extern const char kCameraDeviceSetting[];
extern const char kMicrophoneDeviceSetting[];
extern const char kSpeakerDeviceSetting[];
extern const char kDefaultRoomCapacitySetting[];
extern const char kCloseButtonBehaviorSetting[];
extern const char kStartupVisibilitySetting[];
extern const char kWindowsAutoStartValueName[];
extern const char kAutoOpenCameraGallerySetting[];
extern const char kInterfaceFontFamilySetting[];
extern const char kRemoteScreenQualitySetting[];
extern const char kDragPointerSampleRateSetting[];
extern const char kRemotePasteEnabledSetting[];
extern const char kClipboardFormatsSetting[];
extern const char kClipboardFileLimitSetting[];
extern const char kClipboardCacheBaseDirectorySetting[];
extern const char kClipboardCacheRetentionSetting[];
extern const char kClipboardCacheCapacitySetting[];
extern const std::uint64_t kRemotePastePopupThresholdBytes;
extern const int kSettingsControlWidth;
extern const char kMainStyle[];

bool IsNineDigitPublicId(const QString& value);
QString LocalizedDirectSessionError(
    const std::string& errorCode,
    const std::string& fallbackMessage,
    const SessionEngineSnapshot* snapshot = nullptr);
bool IsDirectRecoveryFailureCode(const std::string& errorCode);
QStringList InstalledChineseInterfaceFonts();
void ApplyInterfaceFontPreference(
    const QString& systemFamily, int pixelSize);
QString AdaptBenchmarkHtmlForTheme(QString html);
VideoEncoderPreference VideoEncoderPreferenceFromSetting(
    const QString& value);
DesktopCaptureImplementation DesktopCaptureImplementationFromSetting(
    const QString& value);
FfmpegHardwareBackend FfmpegHardwareBackendFromSetting(
    const QString& value);
FfmpegX264Preset EncoderQualityFromSetting(const QString& value);
VideoDecoderPreference VideoDecoderPreferenceFromSetting(
    const QString& value);
QString InitialClipboardCacheBaseDirectory();
QString ClipboardCacheRootForBase(const QString& baseDirectory);
qulonglong SafeClipboardCacheCapacityGiB(const QString& baseDirectory);
QString WindowsAutoStartCommand();
bool WindowsAutoStartEnabled();
bool SetWindowsAutoStartEnabled(bool enabled);
std::pair<std::uint32_t, std::uint32_t> SavedScreenQualityBounds(
    ScreenQualityTier quality);
QString FormatBitrate(std::uint64_t bitsPerSecond);
QString FormatByteCount(std::uint64_t bytes);
QString SampleWindowSuffix(std::uint32_t windowMs);
QString LatestFrameTimingText(
    const RtpStreamStatsSnapshot& stream, const QString& action);
QString RouteDisplayName(const std::string& route);
QString SlotDisplayName(
    const std::string& slot, const std::string& kind);
QString CandidateDisplayText(const IceCandidateStatsSnapshot& candidate);
QString InitialFileSaveDirectory();
void SetBusyStatusAnimation(QLabel* label, bool busy);
void AnimateSmallUiChange(QWidget* widget);

enum class NavigationIcon {
    kRoom,
    kDevice,
    kOwnedDevices,
    kRecent,
    kTransfer,
    kDebug,
    kSettings,
    kHelp,
    kAuthor,
};

QPixmap DrawNavigationIcon(NavigationIcon icon, const QColor& color);
QIcon MakeNavigationIcon(NavigationIcon icon, bool dark);
QScrollArea* MakePageSurface(
    const QString& title,
    const QString& subtitle,
    QWidget* parent,
    QVBoxLayout** pageLayout);
QPushButton* MakeNavigationButton(
    const QString& text,
    NavigationIcon icon,
    bool active,
    QWidget* parent);
QFrame* MakeDivider(QWidget* parent);
void AddRoomCapacityItems(QComboBox* comboBox);
QString MemberDisplayName(
    const RoomSnapshot& room,
    const std::string& deviceId,
    const std::string& localDeviceId);
QString ConnectivityDebugName(SessionConnectivityState state);
QString RoomMembershipDebugName(RoomMembershipState state);
QString PeerConnectionDebugName(RoomPeerConnectionState state);
QString MediaDeviceSelectionDebugName(MediaDeviceSelectionState state);
QString MediaDeviceDebugName(
    const MediaDeviceCategorySnapshot& category,
    const std::string& deviceId);
QString MediaDeviceCategoryDebugText(
    const MediaDeviceCategorySnapshot& category);

}  // namespace remote::controller::detail
