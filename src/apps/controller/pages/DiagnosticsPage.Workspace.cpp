// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "DiagnosticsPage.h"

#include <QButtonGroup>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "DiagnosticsCardsWidget.h"
#include "src/apps/controller/CurrentPageStack.h"
#include "src/apps/controller/ControllerMainWindowSupport.h"
#include "src/core/RemoteInputTelemetry.h"

namespace remote::controller {

using namespace detail;

void DiagnosticsPage::BuildWorkspace()
{
    auto* debugToolbar = new QHBoxLayout();
    auto* debugNotice = new QLabel(
        QStringLiteral("状态会随信令、房间和成员连接自动更新"), this);
    debugNotice->setProperty("muted", true);
    debugToolbar->addWidget(debugNotice);
    debugToolbar->addStretch(1);

    screenFrameRateLogEnabled_ = QSettings().value(
        QString::fromLatin1(kScreenFrameRateLogEnabledSetting), false).toBool();
    auto* screenFrameRateLogButton = new QPushButton(this);
    screenFrameRateLogButton->setObjectName(
        QStringLiteral("screenFrameRateLogButton"));
    screenFrameRateLogButton->setCheckable(true);
    screenFrameRateLogButton->setChecked(screenFrameRateLogEnabled_);
    screenFrameRateLogButton->setCursor(Qt::PointingHandCursor);
    const auto updateFrameRateLogButton =
        [this, screenFrameRateLogButton] {
            screenFrameRateLogButton->setText(
                screenFrameRateLogEnabled_
                    ? QStringLiteral("FPS 日志：已开启")
                    : QStringLiteral("FPS 日志：已关闭"));
            screenFrameRateLogButton->setToolTip(
                screenFrameRateLogEnabled_
                    ? QStringLiteral(
                          "正在向本机应用数据目录写入 screen-frame-rate-进程ID.csv，点击关闭")
                    : QStringLiteral(
                          "默认关闭；点击后开始记录采集、编码、发送和显示帧率"));
        };
    updateFrameRateLogButton();
    connect(screenFrameRateLogButton, &QPushButton::toggled, this,
            [this, updateFrameRateLogButton](bool enabled) {
                screenFrameRateLogEnabled_ = enabled;
                QSettings().setValue(
                    QString::fromLatin1(kScreenFrameRateLogEnabledSetting),
                    enabled);
                updateFrameRateLogButton();
                emit ScreenFrameRateLogToggled(enabled);
            });
    debugToolbar->addWidget(screenFrameRateLogButton);

    copyAllButton_ = new QPushButton(QStringLiteral("复制调试信息"), this);
    copyAllButton_->setObjectName(QStringLiteral("softButton"));
    copyAllButton_->setCursor(Qt::PointingHandCursor);
    debugToolbar->addWidget(copyAllButton_);
    contentLayout_->addLayout(debugToolbar);

    auto* debugWorkspace = new QHBoxLayout();
    debugWorkspace->setSpacing(20);
    auto* debugCategories = new QFrame(this);
    debugCategories->setObjectName(QStringLiteral("settingsCategoryPanel"));
    debugCategories->setFixedWidth(190);
    auto* debugCategoryLayout = new QVBoxLayout(debugCategories);
    debugCategoryLayout->setContentsMargins(10, 12, 10, 12);
    debugCategoryLayout->setSpacing(5);
    auto* debugCategoryCaption =
        new QLabel(QStringLiteral("信息分类"), debugCategories);
    debugCategoryCaption->setProperty("muted", true);
    debugCategoryCaption->setContentsMargins(10, 3, 0, 7);
    debugCategoryLayout->addWidget(debugCategoryCaption);

    auto* debugCategoryGroup = new QButtonGroup(this);
    debugCategoryGroup->setExclusive(true);
    const auto makeCategory =
        [debugCategories, debugCategoryLayout, debugCategoryGroup](
            const QString& text, int index) {
            auto* button = new QPushButton(text, debugCategories);
            button->setObjectName(QStringLiteral("settingsCategoryButton"));
            button->setCheckable(true);
            button->setCursor(Qt::PointingHandCursor);
            button->setMinimumHeight(46);
            debugCategoryGroup->addButton(button, index);
            debugCategoryLayout->addWidget(button);
            return button;
        };
    auto* firstCategory = makeCategory(QStringLiteral("设备与信令"), 0);
    makeCategory(QStringLiteral("媒体能力"), 1);
    makeCategory(QStringLiteral("连接质量"), 2);
    makeCategory(QStringLiteral("房间状态"), 3);
    makeCategory(QStringLiteral("成员连接"), 4);
    makeCategory(QStringLiteral("鼠标与键盘"), 5);
    makeCategory(QStringLiteral("远程粘贴"), 6);
    makeCategory(QStringLiteral("最近错误"), 7);
    makeCategory(QStringLiteral("场景识别性能"), 8);
    makeCategory(QStringLiteral("场景优化状态"), 9);
    debugCategoryLayout->addStretch(1);
    debugWorkspace->addWidget(debugCategories, 0, Qt::AlignTop);

    // Let the outer scroll area's layout size the selected detail page.
    // Fixed-height tracking is only for the animated room workspace.
    auto* detailStack = new CurrentPageStack(this, false);
    detailStack_ = detailStack;
    detailStack->setObjectName(QStringLiteral("settingsDetailStack"));
    debugWorkspace->addItem(new detail::CurrentPageStackItem(detailStack));
    debugWorkspace->setStretch(debugWorkspace->count() - 1, 1);
    const auto makeDetail =
        [this, detailStack](const QString& title,
                            const QString& description,
                            QPushButton** copyButton = nullptr) {
            return AddDetailPage(
                detailStack, title, description, copyButton);
        };
    const auto addValue =
        [this](QVBoxLayout* layout, const QString& key,
               const QString& title, bool expanded = false) {
            AddValue(layout, key, title, expanded);
        };

    auto* deviceLayout = makeDetail(
        QStringLiteral("设备与信令"),
        QStringLiteral("查看本机身份、信令连接和 WebRTC 运行状态。"));
    addValue(deviceLayout, QStringLiteral("deviceId"),
             QStringLiteral("本机设备 ID"));
    addValue(deviceLayout, QStringLiteral("signaling"),
             QStringLiteral("信令连接"));
    addValue(deviceLayout, QStringLiteral("webrtc"),
             QStringLiteral("WebRTC 运行时"));
    addValue(deviceLayout, QStringLiteral("systemEnvironment"),
             QStringLiteral("Windows 环境"));
    addValue(deviceLayout, QStringLiteral("sessionEnvironment"),
             QStringLiteral("会话类型"));
    addValue(deviceLayout, QStringLiteral("graphicsAdapters"),
             QStringLiteral("图形适配器"), true);
    addValue(deviceLayout, QStringLiteral("hardwareFingerprint"),
             QStringLiteral("硬件能力指纹"), true);
    addValue(deviceLayout, QStringLiteral("encoderProbeSource"),
             QStringLiteral("编码能力探测"));
    addValue(deviceLayout, QStringLiteral("audioDeviceModule"),
             QStringLiteral("WebRTC 音频模块"), true);
    addValue(deviceLayout, QStringLiteral("cameraDevice"),
             QStringLiteral("摄像头设备"), true);
    addValue(deviceLayout, QStringLiteral("microphoneDevice"),
             QStringLiteral("麦克风设备"), true);
    addValue(deviceLayout, QStringLiteral("speakerDevice"),
             QStringLiteral("扬声器设备"), true);
    deviceLayout->addStretch(1);

    auto* mediaLayout = makeDetail(
        QStringLiteral("媒体能力"),
        QStringLiteral("显示当前机器的 H264 编解码和 D3D11 能力。"),
        &copyMediaButton_);
    addValue(mediaLayout, QStringLiteral("builtinEncoder"),
             QStringLiteral("H264 编码链路"));
    addValue(mediaLayout, QStringLiteral("desktopCapture"),
             QStringLiteral("屏幕采集模式"));
    addValue(mediaLayout, QStringLiteral("encoderPreference"),
             QStringLiteral("视频编码模式"));
    addValue(mediaLayout, QStringLiteral("encoderQuality"),
             QStringLiteral("编码质量"), true);
    addValue(mediaLayout, QStringLiteral("hardwareEncoderCount"),
             QStringLiteral("硬件编码器"));
    addValue(mediaLayout, QStringLiteral("cpuNv12"),
             QStringLiteral("CPU NV12 输入"));
    addValue(mediaLayout, QStringLiteral("d3d11Encoder"),
             QStringLiteral("D3D11 硬件编码"));
    addValue(mediaLayout, QStringLiteral("encoderFallback"),
             QStringLiteral("软件编码链路"));
    addValue(mediaLayout, QStringLiteral("encoderRuntime"),
             QStringLiteral("当前编码实例"), true);
    addValue(mediaLayout, QStringLiteral("encoderFallbackReason"),
             QStringLiteral("最近回退原因"), true);
    addValue(mediaLayout, QStringLiteral("decoder"),
             QStringLiteral("H264 解码"));
    addValue(mediaLayout, QStringLiteral("decoderPreference"),
             QStringLiteral("视频解码模式"));
    addValue(mediaLayout, QStringLiteral("mfDecoderType"),
             QStringLiteral("MF D3D11 解码类型"));
    addValue(mediaLayout, QStringLiteral("mfDecoderName"),
             QStringLiteral("MF 解码器实例"), true);
    addValue(mediaLayout, QStringLiteral("d3d11Output"),
             QStringLiteral("D3D11 原生输出"));
    addValue(mediaLayout, QStringLiteral("mfDecoderAsync"),
             QStringLiteral("硬件 MFT 驱动"));
    addValue(mediaLayout, QStringLiteral("softwareFallback"),
             QStringLiteral("FFmpeg 软件解码"));
    addValue(mediaLayout, QStringLiteral("mfDecoderError"),
             QStringLiteral("MF 解码检测说明"), true);
    addValue(mediaLayout, QStringLiteral("encoderDetails"),
             QStringLiteral("检测到的编码器"), true);
    mediaLayout->addStretch(1);

    auto* connectionLayout = makeDetail(
        QStringLiteral("连接质量"),
        QStringLiteral(
            "每秒读取 WebRTC 标准 Stats，显示真实链路、媒体流和 DataChannel 状态。"));
    statsCardsWidget_ = new DiagnosticsCardsWidget(
        connectionLayout->parentWidget());
    connectionLayout->addWidget(statsCardsWidget_);
    connectionLayout->addStretch(1);

    auto* roomLayout = makeDetail(
        QStringLiteral("房间状态"),
        QStringLiteral("查看当前房间、席位和控制租约状态。"));
    addValue(roomLayout, QStringLiteral("membership"),
             QStringLiteral("房间状态"));
    addValue(roomLayout, QStringLiteral("roomId"), QStringLiteral("房间 ID"));
    addValue(roomLayout, QStringLiteral("members"),
             QStringLiteral("成员与席位"));
    addValue(roomLayout, QStringLiteral("screenSharer"),
             QStringLiteral("当前主机器"));
    addValue(roomLayout, QStringLiteral("screenShareGeneration"),
             QStringLiteral("屏幕共享代次"));
    addValue(roomLayout, QStringLiteral("controller"),
             QStringLiteral("当前控制者"));
    addValue(roomLayout, QStringLiteral("controlGrant"),
             QStringLiteral("控制授权令牌"));
    roomLayout->addStretch(1);

    auto* peerLayout = makeDetail(
        QStringLiteral("成员连接"),
        QStringLiteral("查看每个成员对的 P2P、DataChannel 和媒体槽状态。"));
    addValue(peerLayout, QStringLiteral("peerSummary"),
             QStringLiteral("连接概况"));
    addValue(peerLayout, QStringLiteral("peerDetails"),
             QStringLiteral("成员对明细"), true);
    peerLayout->addStretch(1);

    auto* inputLayout = makeDetail(
        QStringLiteral("鼠标与键盘"),
        QStringLiteral(
            "按秒查看远控输入从本机生成、网络发送到远端 Windows 注入的完整链路。"));
    inputEventStatsEnabled_ = QSettings().value(
        QString::fromLatin1(kInputEventStatsEnabledSetting), false).toBool();
    RemoteInputTelemetry::Instance().SetEnabled(inputEventStatsEnabled_);
    auto* inputStatsRow = new QFrame(inputLayout->parentWidget());
    inputStatsRow->setObjectName(QStringLiteral("settingRow"));
    auto* inputStatsRowLayout = new QHBoxLayout(inputStatsRow);
    inputStatsRowLayout->setContentsMargins(20, 14, 20, 14);
    inputStatsRowLayout->setSpacing(18);
    auto* inputStatsTitle =
        new QLabel(QStringLiteral("输入事件统计"), inputStatsRow);
    inputStatsTitle->setObjectName(QStringLiteral("settingTitle"));
    inputStatsTitle->setFixedWidth(170);
    auto* inputStatsDescription = new QLabel(
        QStringLiteral("默认关闭；开启后每秒刷新计数，不写入日志文件。"),
        inputStatsRow);
    inputStatsDescription->setProperty("muted", true);
    inputStatsDescription->setWordWrap(true);
    auto* inputEventStatsButton = new QPushButton(inputStatsRow);
    inputEventStatsButton->setObjectName(QStringLiteral("softButton"));
    inputEventStatsButton->setCheckable(true);
    inputEventStatsButton->setChecked(inputEventStatsEnabled_);
    inputEventStatsButton->setCursor(Qt::PointingHandCursor);
    inputEventStatsButton->setMinimumWidth(130);
    const auto updateInputStatsButton =
        [this, inputEventStatsButton] {
            inputEventStatsButton->setText(
                inputEventStatsEnabled_
                    ? QStringLiteral("统计已开启")
                    : QStringLiteral("开启统计"));
            inputEventStatsButton->setToolTip(
                inputEventStatsEnabled_
                    ? QStringLiteral("点击停止鼠标与键盘事件计数")
                    : QStringLiteral("点击开始统计远控输入链路"));
        };
    updateInputStatsButton();
    connect(inputEventStatsButton, &QPushButton::toggled, this,
            [this, updateInputStatsButton](bool enabled) {
                inputEventStatsEnabled_ = enabled;
                RemoteInputTelemetry::Instance().SetEnabled(enabled);
                QSettings().setValue(
                    QString::fromLatin1(kInputEventStatsEnabledSetting),
                    enabled);
                updateInputStatsButton();
                emit InputEventStatsToggled(enabled);
            });
    inputStatsRowLayout->addWidget(inputStatsTitle);
    inputStatsRowLayout->addWidget(inputStatsDescription, 1);
    inputStatsRowLayout->addWidget(inputEventStatsButton);
    inputLayout->addWidget(inputStatsRow);
    addValue(inputLayout, QStringLiteral("inputMovePolicy"),
             QStringLiteral("移动调度策略"), true);
    addValue(inputLayout, QStringLiteral("inputMoveGenerated"),
             QStringLiteral("Qt 移动事件"));
    addValue(inputLayout, QStringLiteral("inputMoveDispatched"),
             QStringLiteral("实际调度移动"));
    addValue(inputLayout, QStringLiteral("inputImmediateEvents"),
             QStringLiteral("按键与滚轮"));
    addValue(inputLayout, QStringLiteral("inputPacketsSent"),
             QStringLiteral("网络发送包"));
    addValue(inputLayout, QStringLiteral("inputPacketsReceived"),
             QStringLiteral("网络接收包"));
    addValue(inputLayout, QStringLiteral("inputInjected"),
             QStringLiteral("Windows 注入"), true);
    inputLayout->addStretch(1);

    auto* clipboardLayout = makeDetail(
        QStringLiteral("远程粘贴"),
        QStringLiteral(
            "只显示格式、大小和状态；正文、文件名和完整哈希永不进入调试信息。"));
    addValue(clipboardLayout, QStringLiteral("clipboardState"),
             QStringLiteral("粘贴状态"));
    addValue(clipboardLayout, QStringLiteral("clipboardPeer"),
             QStringLiteral("当前成员对"));
    addValue(clipboardLayout, QStringLiteral("clipboardTransfer"),
             QStringLiteral("当前传输"));
    addValue(clipboardLayout, QStringLiteral("clipboardLast"),
             QStringLiteral("最近项目"));
    addValue(clipboardLayout, QStringLiteral("clipboardCounters"),
             QStringLiteral("累计统计"), true);
    addValue(clipboardLayout, QStringLiteral("clipboardError"),
             QStringLiteral("最近错误"), true);
    clipboardLayout->addStretch(1);

    auto* errorLayout = makeDetail(
        QStringLiteral("最近错误"),
        QStringLiteral("显示会话和房间最近一次报告的错误。"));
    addValue(errorLayout, QStringLiteral("sessionError"),
             QStringLiteral("会话错误"), true);
    addValue(errorLayout, QStringLiteral("roomError"),
             QStringLiteral("房间错误"), true);
    errorLayout->addStretch(1);

    auto* visionPerformanceLayout = makeDetail(
        QStringLiteral("场景识别性能"),
        QStringLiteral(
            "显示远控过程中最近一次场景识别的图片处理耗时，不包含设置页的测试数据。"));
    addValue(
        visionPerformanceLayout,
        QStringLiteral("visionScaleConvertTime"),
        QStringLiteral("图片缩放与转换耗时"));
    addValue(
        visionPerformanceLayout,
        QStringLiteral("visionJpegEncodeTime"),
        QStringLiteral("JPEG 编码耗时"));
    addValue(
        visionPerformanceLayout,
        QStringLiteral("visionJpegSize"),
        QStringLiteral("JPEG 大小"));
    addValue(
        visionPerformanceLayout,
        QStringLiteral("visionReturnedScene"),
        QStringLiteral("最近识别结果"));
    addValue(
        visionPerformanceLayout,
        QStringLiteral("visionReturnedConfidence"),
        QStringLiteral("识别置信度"));
    addValue(visionPerformanceLayout, QStringLiteral("visionAcceptedScene"),
             QStringLiteral("当前场景"));
    addValue(visionPerformanceLayout, QStringLiteral("visionResultAge"),
             QStringLiteral("距上次识别"));
    auto* visionResultHint = new QLabel(QStringLiteral(
        "显示最近一次识别结果，该结果不一定已被采用。"));
    visionResultHint->setWordWrap(true);
    visionPerformanceLayout->addWidget(visionResultHint);
    visionPerformanceLayout->addStretch(1);

    auto* policyLayout = makeDetail(
        QStringLiteral("场景优化状态"),
        QStringLiteral("根据识别到的场景调整画质与帧率取舍，不改变采集帧率和用户设置。"));
    policyCardsWidget_ = new DiagnosticsCardsWidget(policyLayout->parentWidget());
    policyCardsWidget_->setObjectName(QStringLiteral("contentPolicyCards"));
    static_cast<DiagnosticsCardsWidget*>(policyCardsWidget_)->SetSections({},
        QStringLiteral("暂无场景优化数据\n开启 AI 场景优化并共享屏幕后，可查看每个连接的场景和取舍设置。"));
    policyLayout->addWidget(policyCardsWidget_);
    auto* policyHint = new QLabel(QStringLiteral(
        "场景优化只调整网络波动时的画质与帧率取舍。关闭后使用手动设置，原有网络控制保持不变。"));
    policyHint->setWordWrap(true);
    policyHint->setProperty("muted", true);
    policyLayout->addWidget(policyHint);
    policyLayout->addStretch(1);

    connect(debugCategoryGroup, &QButtonGroup::idClicked, this,
            [this, detailStack](int index) {
                detailStack->setCurrentIndex(index);
                for (auto it = values_.cbegin(); it != values_.cend(); ++it) {
                    ApplyValue(it.key());
                }
                emit RefreshRequested();
            });
    firstCategory->setChecked(true);
    detailStack->setCurrentIndex(0);
    contentLayout_->addLayout(debugWorkspace, 1);
}

}  // namespace remote::controller
