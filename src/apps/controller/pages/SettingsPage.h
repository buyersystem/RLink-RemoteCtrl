// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <array>
#include <memory>
#include <vector>

#include <QScrollArea>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QStackedWidget;
class QVBoxLayout;
class QWidget;

namespace remote::app {
class VisionApiSettingsController;
}

namespace remote {
struct SessionDiagnosticsSnapshot;
}

namespace remote::media_intelligence {
class EncodedImage;
}

namespace remote::controller {

struct SettingsPageControls {
    QComboBox* animationLevelSelector = nullptr;
    QComboBox* themeModeSelector = nullptr;
    QComboBox* fontFamilySelector = nullptr;
    QComboBox* fontSizeSelector = nullptr;
    QComboBox* autoStartSelector = nullptr;
    QComboBox* startupVisibilitySelector = nullptr;
    QComboBox* closeButtonBehaviorSelector = nullptr;
    QComboBox* defaultRoomCapacitySelector = nullptr;
    QComboBox* desktopCaptureSelector = nullptr;
    QDoubleSpinBox* screenVideoBitrateBppInput = nullptr;
    QDoubleSpinBox* screenQualityDeficitShareInput = nullptr;
    QLabel* screenVideoTrafficEstimate = nullptr;
    QComboBox* contentAwareStreamingSelector = nullptr;
    QComboBox* contentAnalyzerModeSelector = nullptr;
    QComboBox* visionApiProviderSelector = nullptr;
    QLineEdit* visionApiBaseUrlEdit = nullptr;
    QLineEdit* visionApiModelEdit = nullptr;
    QLineEdit* visionApiKeyEdit = nullptr;
    QComboBox* visionApiRequestIntervalSelector = nullptr;
    QComboBox* visionApiMaximumDimensionSelector = nullptr;
    QComboBox* visionApiJpegQualitySelector = nullptr;
    QCheckBox* visionApiConsentCheckBox = nullptr;
    QPushButton* visionApiTestButton = nullptr;
    QPushButton* visionApiScreenshotTestButton = nullptr;
    QPushButton* visionApiClearKeyButton = nullptr;
    QLabel* visionApiStatusLabel = nullptr;
    QWidget* visionApiConfigurationPanel = nullptr;
    QComboBox* videoEncoderSelector = nullptr;
    QComboBox* ffmpegHardwareBackendSelector = nullptr;
    QComboBox* ffmpegX264PresetSelector = nullptr;
    QComboBox* videoDecoderSelector = nullptr;
    QComboBox* videoRendererSelector = nullptr;
    QComboBox* dragPointerSampleRateSelector = nullptr;
    QComboBox* remotePasteEnabledSelector = nullptr;
    QComboBox* clipboardFormatsSelector = nullptr;
    QComboBox* clipboardLargeFileLimitSelector = nullptr;
    QComboBox* clipboardCacheRetentionSelector = nullptr;
    QComboBox* clipboardCacheCapacitySelector = nullptr;
    QLabel* clipboardCachePathLabel = nullptr;
    QLabel* clipboardCacheUsageLabel = nullptr;
    QPushButton* clearClipboardCacheButton = nullptr;
    QLabel* decoderBenchmarkSummary = nullptr;
    QPushButton* decoderBenchmarkButton = nullptr;
    QLabel* encoderBenchmarkSummary = nullptr;
    QPushButton* encoderBenchmarkButton = nullptr;
    QComboBox* cameraGalleryBehaviorSelector = nullptr;
    QComboBox* cameraDeviceSelector = nullptr;
    QComboBox* microphoneDeviceSelector = nullptr;
    QComboBox* speakerDeviceSelector = nullptr;
    QLabel* mediaDeviceStatusLabel = nullptr;
    QPushButton* refreshMediaDevicesButton = nullptr;
    QLabel* softwareUpdateStatusLabel = nullptr;
    QPushButton* softwareUpdateCheckButton = nullptr;
};

class SettingsPage final : public QScrollArea {
    Q_OBJECT

public:
    explicit SettingsPage(QWidget* parent = nullptr);

    QStackedWidget* DetailStack() const;
    SettingsPageControls& Controls();
    const SettingsPageControls& Controls() const;
    void RefreshClipboardCacheCapacityOptions();
    int CurrentCategory() const;
    void UpdateScreenVideoTrafficEstimate(const SessionDiagnosticsSnapshot& diagnostics);

signals:
    void CategoryChanged(int index);
    void ClipboardCacheBaseDirectoryChanged();
    void SoftwareUpdateRequested();
    void ScreenVideoBitrateBppChanged(std::uint32_t hundredths);
    void ScreenQualityDeficitShareChanged(std::uint32_t hundredths);

private:
    void ApplyClipboardCacheBaseDirectory(const QString& selectedDirectory);
    void BuildAudioVideoSettingsPage();
    void BuildContentAwarenessSettingsPage();
    void BuildFileTransferSettingsPage();
    void BuildGeneralSettingsPage();
    void BuildRemoteDesktopSettingsPage();
    void RefreshScreenVideoTrafficEstimate(double previewBpp = -1.0);
    void BuildRemotePasteSettingsPage();
    void BuildShortcutSettingsPage();
    void ClearVisionApiCredential();
    std::uint64_t PersistVisionApiConfiguration(bool invalidateTest);
    void RefreshVisionApiSettingsUi();
    void StartVisionApiConnectionTest();
    void StartVisionApiConnectionTestWithImage(
        std::unique_ptr<media_intelligence::EncodedImage> testImage,
        const QString& testImageName);
    void StartVisionApiScreenshotTest();

    QVBoxLayout* contentLayout_ = nullptr;
    SettingsPageControls controls_;
    QButtonGroup* categoryGroup_ = nullptr;
    QStackedWidget* detailStack_ = nullptr;
    remote::app::VisionApiSettingsController*
        visionApiSettingsController_ = nullptr;
    bool visionApiTestRunning_ = false;
    std::vector<std::array<std::uint32_t, 3>> screenTrafficSpecifications_;
    std::uint64_t screenMeasuredTrafficBps_ = 0;
    bool screenMeasuredTrafficAvailable_ = false;
};

}  // namespace remote::controller
