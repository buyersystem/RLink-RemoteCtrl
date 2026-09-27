// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QScrollArea>

class QButtonGroup;
class QComboBox;
class QLabel;
class QPushButton;
class QStackedWidget;
class QVBoxLayout;

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

signals:
    void CategoryChanged(int index);
    void ClipboardCacheBaseDirectoryChanged();
    void SoftwareUpdateRequested();

private:
    void ApplyClipboardCacheBaseDirectory(const QString& selectedDirectory);
    void BuildAudioVideoSettingsPage();
    void BuildFileTransferSettingsPage();
    void BuildGeneralSettingsPage();
    void BuildRemoteDesktopSettingsPage();
    void BuildRemotePasteSettingsPage();
    void BuildShortcutSettingsPage();

    QVBoxLayout* contentLayout_ = nullptr;
    SettingsPageControls controls_;
    QButtonGroup* categoryGroup_ = nullptr;
    QStackedWidget* detailStack_ = nullptr;
};

}  // namespace remote::controller
