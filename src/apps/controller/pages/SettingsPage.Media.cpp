// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "SettingsPage.h"
#include "SettingsPageUi.h"

#include <algorithm>

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "src/apps/controller/ControllerMainWindowSupport.h"
#include "src/apps/controller/RemoteCComboBox.h"

namespace remote::controller {
using namespace detail;

void SettingsPage::BuildAudioVideoSettingsPage()
{
    const QSettings currentSettings;
    auto* page = new QWidget(detailStack_);
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    AddSettingsDetailHeader(
        layout, page, QStringLiteral("音视频设备"),
        QStringLiteral("调整摄像头画廊与本机音视频窗口的显示方式。"));

    const auto [cameraGalleryRow, cameraGalleryLayout] = CreateSettingsRow(
        page, QStringLiteral("远端摄像头画廊"),
        QStringLiteral("选择成员开启摄像头时是否自动显示画廊。"));
    controls_.cameraGalleryBehaviorSelector =
        new RemoteCComboBox(cameraGalleryRow);
    controls_.cameraGalleryBehaviorSelector->setObjectName(
        QStringLiteral("capacitySelector"));
    controls_.cameraGalleryBehaviorSelector->addItem(
        QStringLiteral("自动显示（推荐）"), true);
    controls_.cameraGalleryBehaviorSelector->addItem(
        QStringLiteral("仅手动打开"), false);
    controls_.cameraGalleryBehaviorSelector->setCurrentIndex(std::max(
        0, controls_.cameraGalleryBehaviorSelector->findData(
               currentSettings.value(
                   QString::fromLatin1(kAutoOpenCameraGallerySetting),
                   true).toBool())));
    controls_.cameraGalleryBehaviorSelector->setFixedWidth(
        kSettingsControlWidth);
    cameraGalleryLayout->addWidget(
        controls_.cameraGalleryBehaviorSelector, 0, Qt::AlignVCenter);
    layout->addWidget(cameraGalleryRow);

    const auto [cameraDeviceRow, cameraDeviceLayout] = CreateSettingsRow(
        page, QStringLiteral("本机摄像头"),
        QStringLiteral("选择开启本机摄像头时使用的设备，房间中可直接切换。"));
    controls_.cameraDeviceSelector = new RemoteCComboBox(cameraDeviceRow);
    controls_.cameraDeviceSelector->addItem(
        QStringLiteral("正在读取设备…"), QStringLiteral("default"));
    controls_.cameraDeviceSelector->setFixedWidth(kSettingsControlWidth);
    controls_.cameraDeviceSelector->setEnabled(false);
    cameraDeviceLayout->addWidget(
        controls_.cameraDeviceSelector, 0, Qt::AlignVCenter);
    layout->addWidget(cameraDeviceRow);

    const auto [microphoneDeviceRow, microphoneDeviceLayout] =
        CreateSettingsRow(
            page, QStringLiteral("本机麦克风"),
            QStringLiteral("选择房间麦克风使用的录音设备，切换时保持现有音频 Track。"));
    controls_.microphoneDeviceSelector =
        new RemoteCComboBox(microphoneDeviceRow);
    controls_.microphoneDeviceSelector->addItem(
        QStringLiteral("正在读取设备…"), QStringLiteral("default"));
    controls_.microphoneDeviceSelector->setFixedWidth(kSettingsControlWidth);
    controls_.microphoneDeviceSelector->setEnabled(false);
    microphoneDeviceLayout->addWidget(
        controls_.microphoneDeviceSelector, 0, Qt::AlignVCenter);
    layout->addWidget(microphoneDeviceRow);

    const auto [speakerDeviceRow, speakerDeviceLayout] = CreateSettingsRow(
        page, QStringLiteral("本机扬声器"),
        QStringLiteral("选择接收房间音频时使用的播放设备，可在会话中切换。"));
    controls_.speakerDeviceSelector = new RemoteCComboBox(speakerDeviceRow);
    controls_.speakerDeviceSelector->addItem(
        QStringLiteral("正在读取设备…"), QStringLiteral("default"));
    controls_.speakerDeviceSelector->setFixedWidth(kSettingsControlWidth);
    controls_.speakerDeviceSelector->setEnabled(false);
    speakerDeviceLayout->addWidget(
        controls_.speakerDeviceSelector, 0, Qt::AlignVCenter);
    layout->addWidget(speakerDeviceRow);

    auto* footer = new QFrame(page);
    footer->setObjectName(QStringLiteral("settingRow"));
    auto* footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(20, 14, 20, 14);
    footerLayout->setSpacing(12);
    controls_.mediaDeviceStatusLabel = new QLabel(
        QStringLiteral("正在读取本机音视频设备…"), footer);
    controls_.mediaDeviceStatusLabel->setProperty("muted", true);
    controls_.mediaDeviceStatusLabel->setWordWrap(true);
    footerLayout->addWidget(controls_.mediaDeviceStatusLabel, 1);
    controls_.refreshMediaDevicesButton = new QPushButton(
        QStringLiteral("刷新设备"), footer);
    controls_.refreshMediaDevicesButton->setObjectName(
        QStringLiteral("softButton"));
    controls_.refreshMediaDevicesButton->setCursor(Qt::PointingHandCursor);
    footerLayout->addWidget(
        controls_.refreshMediaDevicesButton, 0, Qt::AlignRight);
    layout->addWidget(footer);
    layout->addStretch(1);
    detailStack_->addWidget(page);
}

}  // namespace remote::controller
