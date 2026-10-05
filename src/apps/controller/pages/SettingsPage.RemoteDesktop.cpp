// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "SettingsPage.h"
#include "SettingsPageUi.h"

#include <algorithm>
#include <QByteArray>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLocale>
#include <cmath>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>
#include <QWheelEvent>

#include "src/apps/controller/ControllerMainWindowSupport.h"
#include "src/apps/controller/RemoteCComboBox.h"
#include "src/platform/win/FfmpegHardwareH264Encoder.h"
#include "src/core/ScreenStreamPolicy.h"
#include "src/core/ScreenFrameQualityPolicy.h"
#include "src/core/SessionDiagnostics.h"
#include "src/webrtc/IWebRtcSession.h"

namespace remote::controller {
namespace {

class ScreenReferenceBppInput final : public QDoubleSpinBox {
public:
    using QDoubleSpinBox::QDoubleSpinBox;
protected:
    void wheelEvent(QWheelEvent* event) override { event->ignore(); }
};

void ConfigurePreferenceSelector(QComboBox* selector,
                                 const QSettings& settings,
                                 const char* settingKey,
                                 const bool renderer)
{
    selector->setObjectName(QStringLiteral("capacitySelector"));
    if (renderer) {
        selector->addItem(
            QStringLiteral("自动（D3D11 优先）"), QStringLiteral("auto"));
        selector->addItem(QStringLiteral("D3D11"), QStringLiteral("d3d11"));
        selector->addItem(
            QStringLiteral("CPU 兼容"), QStringLiteral("cpu"));
    } else {
        const bool decoder =
            QByteArray(settingKey) == QByteArray(detail::kVideoDecoderPreferenceSetting);
        selector->addItem(
            decoder
                ? QStringLiteral("自动（FFmpeg 硬件优先，软件回退）")
                : QStringLiteral("自动（硬件优先）"),
            QStringLiteral("auto"));
        selector->addItem(
            QStringLiteral("仅硬件"), QStringLiteral("hardware"));
        selector->addItem(
            QStringLiteral("仅软件"), QStringLiteral("software"));
    }
    const QString configured = settings.value(
        QString::fromLatin1(settingKey), QStringLiteral("auto")).toString();
    selector->setCurrentIndex(std::max(0, selector->findData(configured)));
    selector->setFixedWidth(detail::kSettingsControlWidth);
}

void AddBenchmarkRow(QWidget* page,
                     QVBoxLayout* layout,
                     const QString& title,
                     const QString& hint,
                     QLabel*& summary,
                     QPushButton*& button,
                     const QString& summaryObjectName,
                     const QSizePolicy::Policy horizontalPolicy)
{
    auto* row = new QFrame(page);
    row->setObjectName(QStringLiteral("settingRow"));
    auto* controls = new QVBoxLayout(row);
    controls->setContentsMargins(20, 17, 20, 17);
    controls->setSpacing(12);
    auto* header = new QHBoxLayout();
    header->setContentsMargins(0, 0, 0, 0);
    header->setSpacing(16);
    auto* heading = new QWidget(row);
    auto* headingLayout = new QVBoxLayout(heading);
    headingLayout->setContentsMargins(0, 0, 0, 0);
    headingLayout->setSpacing(3);
    auto* titleLabel = new QLabel(title, heading);
    titleLabel->setObjectName(QStringLiteral("settingTitle"));
    auto* hintLabel = new QLabel(hint, heading);
    hintLabel->setProperty("muted", true);
    headingLayout->addWidget(titleLabel);
    headingLayout->addWidget(hintLabel);
    header->addWidget(heading, 1);
    button = new QPushButton(QStringLiteral("重新检测"), row);
    button->setObjectName(QStringLiteral("softButton"));
    button->setCursor(Qt::PointingHandCursor);
    header->addWidget(button, 0, Qt::AlignTop);
    controls->addLayout(header);
    summary = new QLabel(row);
    summary->setObjectName(summaryObjectName);
    summary->setTextFormat(Qt::RichText);
    summary->setWordWrap(true);
    summary->setSizePolicy(horizontalPolicy, QSizePolicy::Minimum);
    controls->addWidget(summary);
    layout->addWidget(row);
}

}  // namespace

using namespace detail;

void SettingsPage::BuildRemoteDesktopSettingsPage()
{
    const QSettings currentSettings;
    auto* page = new QWidget(detailStack_);
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    AddSettingsDetailHeader(
        layout, page, QStringLiteral("远程桌面"),
        QStringLiteral("选择屏幕采集、视频编解码和远端画面的渲染策略。"));

    const auto [captureRow, captureLayout] = CreateSettingsRow(
        page, QStringLiteral("屏幕采集器"),
        QStringLiteral("选择屏幕采集方式。"));
    controls_.desktopCaptureSelector = new RemoteCComboBox(captureRow);
    controls_.desktopCaptureSelector->setObjectName(
        QStringLiteral("capacitySelector"));
    controls_.desktopCaptureSelector->addItem(
        QStringLiteral("libwebrtc（静态省带宽）"),
        QStringLiteral("libwebrtc"));
    controls_.desktopCaptureSelector->addItem(
        QStringLiteral("自研 DXGI（动态低延迟）"),
        QStringLiteral("native_dxgi"));
    controls_.desktopCaptureSelector->setItemData(
        0, QStringLiteral("带宽要求低，支持最高 120 FPS，静止画面流量消耗小。"),
        Qt::ToolTipRole);
    controls_.desktopCaptureSelector->setItemData(
        1, QStringLiteral("带宽要求高，FPS 上限 120 FPS，流量消耗大。"),
        Qt::ToolTipRole);
    controls_.desktopCaptureSelector->setCurrentIndex(std::max(
        0, controls_.desktopCaptureSelector->findData(
               currentSettings.value(
                   QString::fromLatin1(kDesktopCaptureBackendSetting),
                   QStringLiteral("native_dxgi")).toString())));
    controls_.desktopCaptureSelector->setFixedWidth(kSettingsControlWidth);
    const auto updateCaptureToolTip = [this](int) {
        controls_.desktopCaptureSelector->setToolTip(
            controls_.desktopCaptureSelector->currentData(
                Qt::ToolTipRole).toString());
    };
    connect(controls_.desktopCaptureSelector, &QComboBox::currentIndexChanged,
            this, updateCaptureToolTip);
    updateCaptureToolTip(controls_.desktopCaptureSelector->currentIndex());
    captureLayout->addWidget(
        controls_.desktopCaptureSelector, 0, Qt::AlignVCenter);
    layout->addWidget(captureRow);

    const auto [bppRow, bppLayout] = CreateSettingsRow(
        page, QStringLiteral("视频码率上限系数"),
        QStringLiteral("数值越低，视频码率上限越低、更省流量，但画面可能稍模糊。\n不担心流量时，可适当提高。"));
    // Keep the same text/control columns as the surrounding settings rows.
    auto* bppTextHost = bppLayout->itemAt(0)->widget();
    for (auto* label : bppTextHost->findChildren<QLabel*>()) {
        if (label->property("muted").toBool()) label->setWordWrap(true);
    }
    auto* bppControls = new QWidget(bppRow);
    bppControls->setFixedWidth(kSettingsControlWidth);
    auto* bppControlsLayout = new QHBoxLayout(bppControls);
    bppControlsLayout->setContentsMargins(0, 0, 0, 0);
    bppControlsLayout->setSpacing(10);
    controls_.screenVideoTrafficEstimate = new QLabel(bppControls);
    controls_.screenVideoTrafficEstimate->setObjectName(QStringLiteral("screenVideoTrafficEstimate"));
    controls_.screenVideoTrafficEstimate->setWordWrap(true);
    controls_.screenVideoTrafficEstimate->setProperty("muted", true);
    bppControlsLayout->addWidget(controls_.screenVideoTrafficEstimate, 1);
    controls_.screenVideoBitrateBppInput = new ScreenReferenceBppInput(bppRow);
    auto* bppInput = controls_.screenVideoBitrateBppInput;
    bppInput->setObjectName(QStringLiteral("screenVideoBitrateBppInput"));
    bppInput->setLocale(QLocale::c());
    bppInput->setDecimals(2);
    bppInput->setRange(kMinimumScreenVideoBitrateBppHundredths / 100.0,
                       kMaximumScreenVideoBitrateBppHundredths / 100.0);
    bppInput->setSingleStep(0.01);
    bppInput->setKeyboardTracking(false);
    bppInput->setButtonSymbols(QAbstractSpinBox::NoButtons);
    bppInput->setToolTip(QStringLiteral("输入 0.03～0.50，最多两位小数；默认 0.15。发送端按回车或移开焦点后动态应用，通常在下一次统计更新时生效（约 1 秒），无需重新连接。控制视频码率上限，不保证固定流量。"));
    const auto configuredBpp = NormalizeScreenVideoBitrateBppHundredths(
        currentSettings.value(
            QStringLiteral("media/screenVideoBitrateBppHundredths"),
            kDefaultScreenVideoBitrateBppHundredths).toUInt());
    bppInput->setValue(configuredBpp / 100.0);
    bppInput->setFixedWidth(100);
    connect(bppInput, &QDoubleSpinBox::valueChanged, this, [this](double value) {
        QSettings settings;
        settings.setValue(QStringLiteral("media/screenVideoBitrateBppHundredths"),
                          static_cast<std::uint32_t>(std::lround(value * 100.0)));
        RefreshScreenVideoTrafficEstimate();
        emit ScreenVideoBitrateBppChanged(static_cast<std::uint32_t>(std::lround(value * 100.0)));
    });
    connect(bppInput, &QDoubleSpinBox::textChanged, this, [this](const QString& text) {
        bool valid = false;
        const auto value = text.toDouble(&valid);
        RefreshScreenVideoTrafficEstimate(valid ? value : -1.0);
    });
    bppControlsLayout->addWidget(bppInput, 0, Qt::AlignVCenter);
    bppLayout->addWidget(bppControls, 0, Qt::AlignVCenter);
    RefreshScreenVideoTrafficEstimate();
    layout->addWidget(bppRow);

    const auto [qualityRow, qualityLayout] = CreateSettingsRow(
        page, QStringLiteral("网络波动画质取舍系数"),
        QStringLiteral("此值影响网络波动下的行为。越大，越优先保持 FPS；\n越小，越优先保持码率（画面清晰）。"));
    for (auto* label : qualityLayout->itemAt(0)->widget()->findChildren<QLabel*>()) {
        if (label->property("muted").toBool()) label->setWordWrap(true);
    }
    auto* qualityControls = new QWidget(qualityRow);
    qualityControls->setFixedWidth(kSettingsControlWidth);
    auto* qualityControlsLayout = new QHBoxLayout(qualityControls);
    qualityControlsLayout->setContentsMargins(0, 0, 0, 0);
    qualityControlsLayout->addStretch(1);
    controls_.screenQualityDeficitShareInput = new ScreenReferenceBppInput(qualityControls);
    auto* qualityInput = controls_.screenQualityDeficitShareInput;
    qualityInput->setObjectName(QStringLiteral("screenQualityDeficitShareInput"));
    qualityInput->setLocale(QLocale::c());
    qualityInput->setDecimals(2);
    qualityInput->setRange(0.00, 1.00);
    qualityInput->setSingleStep(0.01);
    qualityInput->setKeyboardTracking(false);
    qualityInput->setButtonSymbols(QAbstractSpinBox::NoButtons);
    qualityInput->setToolTip(QStringLiteral("输入 0.00～1.00，最多两位小数；默认 0.50。发送端按回车或移开焦点后动态应用，无需重新连接。"));
    qualityInput->setValue(ConfiguredScreenQualityDeficitShareHundredths() / 100.0);
    qualityInput->setFixedWidth(100);
    connect(qualityInput, &QDoubleSpinBox::valueChanged, this, [this](double value) {
        const auto hundredths = NormalizeScreenQualityDeficitShareHundredths(
            static_cast<std::uint32_t>(std::lround(value * 100.0)));
        QSettings().setValue(QStringLiteral("media/screenQualityDeficitShareHundredths"), hundredths);
        emit ScreenQualityDeficitShareChanged(hundredths);
    });
    qualityControlsLayout->addWidget(qualityInput, 0, Qt::AlignVCenter);
    qualityLayout->addWidget(qualityControls, 0, Qt::AlignVCenter);
    layout->addWidget(qualityRow);

    const auto [encoderRow, encoderLayout] = CreateSettingsRow(
        page, QStringLiteral("视频编码器"),
        QStringLiteral("选择发送屏幕和摄像头时的编码方式。"));
    controls_.videoEncoderSelector = new RemoteCComboBox(encoderRow);
    controls_.videoEncoderSelector->addItem(
        QStringLiteral("自动（首次 FFmpeg 硬件，检测后采用最优）"),
        QStringLiteral("auto"));
    controls_.videoEncoderSelector->addItem(
        QStringLiteral("硬件编码（失败回退 FFmpeg/libx264）"),
        QStringLiteral("hardware"));
    controls_.videoEncoderSelector->addItem(
        QStringLiteral("FFmpeg 硬件编码（失败回退 FFmpeg/libx264）"),
        QStringLiteral("ffmpeg_hardware"));
    controls_.videoEncoderSelector->addItem(
        QStringLiteral("软件编码（OpenH264）"),
        QStringLiteral("software"));
    controls_.videoEncoderSelector->addItem(
        QStringLiteral("软件编码（FFmpeg/libx264）"),
        QStringLiteral("ffmpeg"));
    controls_.videoEncoderSelector->setCurrentIndex(std::max(
        0, controls_.videoEncoderSelector->findData(
               currentSettings.value(
                   QString::fromLatin1(kVideoEncoderPreferenceSetting),
                   QStringLiteral("auto")).toString())));
    controls_.videoEncoderSelector->setFixedWidth(kSettingsControlWidth);
    encoderLayout->addWidget(
        controls_.videoEncoderSelector, 0, Qt::AlignVCenter);
    layout->addWidget(encoderRow);

    const auto [hardwareRow, hardwareLayout] = CreateSettingsRow(
        page, QStringLiteral("FFmpeg 硬件后端"),
        QStringLiteral(
            "自动模式按当前显示适配器和真实运行时能力选择；也可固定 QSV、NVENC 或 AMF 用于兼容性测试。"));
    controls_.ffmpegHardwareBackendSelector = new RemoteCComboBox(hardwareRow);
    controls_.ffmpegHardwareBackendSelector->addItem(
        QStringLiteral("自动选择（推荐）"), QStringLiteral("auto"));
    for (const auto& candidate : FfmpegHardwareH264Encoder::EnumerateAvailability()) {
        if (!candidate.compiled || !candidate.driverRuntimePresent ||
            !RunFfmpegHardwareEncoderSelfTest(candidate.backend).succeeded) {
            continue;
        }
        switch (candidate.backend) {
        case FfmpegHardwareBackend::kQsv:
            controls_.ffmpegHardwareBackendSelector->addItem(
                QStringLiteral("Intel Quick Sync（QSV）"), QStringLiteral("qsv"));
            break;
        case FfmpegHardwareBackend::kNvenc:
            controls_.ffmpegHardwareBackendSelector->addItem(
                QStringLiteral("NVIDIA NVENC"), QStringLiteral("nvenc"));
            break;
        case FfmpegHardwareBackend::kAmf:
            controls_.ffmpegHardwareBackendSelector->addItem(
                QStringLiteral("AMD AMF"), QStringLiteral("amf"));
            break;
        case FfmpegHardwareBackend::kAutomatic:
            break;
        }
    }
    const QString savedBackend = currentSettings.value(
        QString::fromLatin1(kFfmpegHardwareBackendSetting),
        QStringLiteral("auto")).toString();
    const int savedBackendIndex =
        controls_.ffmpegHardwareBackendSelector->findData(savedBackend);
    controls_.ffmpegHardwareBackendSelector->setCurrentIndex(
        std::max(0, savedBackendIndex));
    if (savedBackendIndex < 0) {
        QSettings settings;
        settings.setValue(
            QString::fromLatin1(kFfmpegHardwareBackendSetting),
            QStringLiteral("auto"));
    }
    controls_.ffmpegHardwareBackendSelector->setFixedWidth(kSettingsControlWidth);
    hardwareLayout->addWidget(
        controls_.ffmpegHardwareBackendSelector, 0, Qt::AlignVCenter);
    layout->addWidget(hardwareRow);

    const auto [presetRow, presetLayout] = CreateSettingsRow(
        page, QStringLiteral("编码质量"),
        QStringLiteral("编码质量越高，FPS 可能稍低；性能较差的电脑可适当降低编码质量。"));
    controls_.ffmpegX264PresetSelector = new RemoteCComboBox(presetRow);
    controls_.ffmpegX264PresetSelector->addItem(
        QStringLiteral("极高"), QStringLiteral("veryslow"));
    controls_.ffmpegX264PresetSelector->addItem(
        QStringLiteral("高"), QStringLiteral("slow"));
    controls_.ffmpegX264PresetSelector->addItem(
        QStringLiteral("中（推荐）"), QStringLiteral("medium"));
    controls_.ffmpegX264PresetSelector->addItem(
        QStringLiteral("低"), QStringLiteral("veryfast"));
    controls_.ffmpegX264PresetSelector->addItem(
        QStringLiteral("极低"), QStringLiteral("ultrafast"));
    QString quality = currentSettings.value(
        QString::fromLatin1(kFfmpegX264PresetSetting),
        QStringLiteral("medium")).toString().toLower();
    if (quality == QStringLiteral("superfast")) {
        quality = QStringLiteral("ultrafast");
    } else if (quality == QStringLiteral("faster") ||
               quality == QStringLiteral("fast")) {
        quality = QStringLiteral("veryfast");
    } else if (quality == QStringLiteral("slower")) {
        quality = QStringLiteral("slow");
    }
    controls_.ffmpegX264PresetSelector->setCurrentIndex(std::max(
        0, controls_.ffmpegX264PresetSelector->findData(quality)));
    controls_.ffmpegX264PresetSelector->setFixedWidth(kSettingsControlWidth);
    controls_.ffmpegX264PresetSelector->setToolTip(QStringLiteral(
        "极高：libx264 veryslow / NVENC p7 / QSV veryslow / AMF quality / MFT 100 / OpenH264 HIGH\n"
        "高：libx264 slow / NVENC p6 / QSV slow / AMF quality / MFT 75 / OpenH264 HIGH\n"
        "中：libx264 medium / NVENC p4 / QSV medium / AMF balanced / MFT 50 / OpenH264 MEDIUM\n"
        "低：libx264 veryfast / NVENC p2 / QSV faster / AMF speed / MFT 25 / OpenH264 LOW\n"
        "极低：libx264 ultrafast / NVENC p1 / QSV veryfast / AMF speed / MFT 0 / OpenH264 LOW"));
    presetLayout->addWidget(
        controls_.ffmpegX264PresetSelector, 0, Qt::AlignVCenter);
    layout->addWidget(presetRow);

    AddBenchmarkRow(
        page, layout, QStringLiteral("编码器性能检测"),
        QStringLiteral("按当前采集方式实测 MFT、FFmpeg 硬件、OpenH264 与 libx264"),
        controls_.encoderBenchmarkSummary,
        controls_.encoderBenchmarkButton,
        QStringLiteral("encoderBenchmarkSummary"), QSizePolicy::Preferred);

    const auto [decoderRow, decoderLayout] = CreateSettingsRow(
        page, QStringLiteral("视频解码器"),
        QStringLiteral("选择接收远端画面时的解码方式。"));
    controls_.videoDecoderSelector = new RemoteCComboBox(decoderRow);
    ConfigurePreferenceSelector(
        controls_.videoDecoderSelector, currentSettings,
        kVideoDecoderPreferenceSetting, false);
    decoderLayout->addWidget(
        controls_.videoDecoderSelector, 0, Qt::AlignVCenter);
    layout->addWidget(decoderRow);

    AddBenchmarkRow(
        page, layout, QStringLiteral("解码器性能检测"),
        QStringLiteral("按 1920 × 1080、连续 60 FPS 与稀疏画面检测本机解码器；结果按当前硬件环境保存。"),
        controls_.decoderBenchmarkSummary,
        controls_.decoderBenchmarkButton,
        QStringLiteral("decoderBenchmarkSummary"), QSizePolicy::Expanding);

    const auto [rendererRow, rendererLayout] = CreateSettingsRow(
        page, QStringLiteral("画面渲染器"),
        QStringLiteral("选择远端画面的显示渲染方式。"));
    controls_.videoRendererSelector = new RemoteCComboBox(rendererRow);
    ConfigurePreferenceSelector(
        controls_.videoRendererSelector, currentSettings,
        kVideoRendererPreferenceSetting, true);
    rendererLayout->addWidget(
        controls_.videoRendererSelector, 0, Qt::AlignVCenter);
    layout->addWidget(rendererRow);

    const auto [dragRow, dragLayout] = CreateSettingsRow(
        page, QStringLiteral("拖动采样率"),
        QStringLiteral("按住鼠标拖动时读取真实光标位置的频率；越高越丝滑，输入包也越多。"));
    controls_.dragPointerSampleRateSelector = new RemoteCComboBox(dragRow);
    controls_.dragPointerSampleRateSelector->setObjectName(
        QStringLiteral("capacitySelector"));
    for (const std::uint32_t hertz : {240u, 170u, 120u, 80u, 60u}) {
        controls_.dragPointerSampleRateSelector->addItem(
            hertz == 240
                ? QStringLiteral("%1 Hz（最丝滑）").arg(hertz)
                : QStringLiteral("%1 Hz").arg(hertz),
            QVariant::fromValue(hertz));
    }
    const std::uint32_t configuredDragSampleRate = currentSettings.value(
        QString::fromLatin1(kDragPointerSampleRateSetting),
        QVariant::fromValue(240u)).toUInt();
    controls_.dragPointerSampleRateSelector->setCurrentIndex(std::max(
        0, controls_.dragPointerSampleRateSelector->findData(
               QVariant::fromValue(configuredDragSampleRate))));
    controls_.dragPointerSampleRateSelector->setFixedWidth(kSettingsControlWidth);
    dragLayout->addWidget(
        controls_.dragPointerSampleRateSelector, 0, Qt::AlignVCenter);
    layout->addWidget(dragRow);

    auto* restartHint = new QLabel(
        QStringLiteral("屏幕采集与视频编码模式在运行时创建时固定；修改后重启 RLink 生效。"),
        page);
    restartHint->setProperty("muted", true);
    layout->addWidget(restartHint);
    layout->addStretch(1);
    detailStack_->addWidget(page);
}

void SettingsPage::UpdateScreenVideoTrafficEstimate(const SessionDiagnosticsSnapshot& diagnostics)
{
    std::vector<std::array<std::uint32_t, 3>> specifications;
    std::uint64_t measuredBps = 0;
    bool measuredAvailable = false;
    for (const auto& peer : diagnostics.peerConnections) {
        for (const auto& stream : peer.stats.rtpStreams) {
            if (stream.direction != RtpStreamDirection::kOutbound || stream.kind != "video" ||
                stream.slot != kScreenMainVideoSlot || stream.sourceWidth == 0 ||
                stream.configuredOutputWidth == 0 || stream.configuredOutputHeight == 0 ||
                stream.configuredMaxFrameRate == 0) continue;
            // One screen connection per peer; never count RTX/SSRC records twice.
            specifications.push_back({stream.configuredOutputWidth,
                stream.configuredOutputHeight, stream.configuredMaxFrameRate});
            if (stream.sampleWindowMs > 0 && peer.stats.transport.collected) {
                measuredBps += stream.bitrateBps;
                measuredAvailable = true;
            }
            break;
        }
    }
    if (screenTrafficSpecifications_ == specifications && screenMeasuredTrafficBps_ == measuredBps &&
        screenMeasuredTrafficAvailable_ == measuredAvailable) return;
    screenTrafficSpecifications_ = std::move(specifications);
    screenMeasuredTrafficBps_ = measuredBps;
    screenMeasuredTrafficAvailable_ = measuredAvailable;
    RefreshScreenVideoTrafficEstimate();
}

void SettingsPage::RefreshScreenVideoTrafficEstimate(double previewBpp)
{
    auto* estimate = controls_.screenVideoTrafficEstimate;
    auto* input = controls_.screenVideoBitrateBppInput;
    if (!estimate || !input) return;
    const auto bpp = std::isfinite(previewBpp) && previewBpp >= input->minimum() &&
        previewBpp <= input->maximum() ? previewBpp : input->value();
    const auto hundredths = static_cast<std::uint32_t>(std::lround(bpp * 100.0));
    const auto capacityFor = [hundredths](const std::array<std::uint32_t, 3>& specification) {
        return ResolveScreenStreamPolicy(specification[0], specification[1],
            {specification[0], specification[1], specification[2], hundredths}).maxBitrateBps;
    };
    std::uint64_t bitsPerSecond = 0;
    QString basis;
    if (screenTrafficSpecifications_.empty()) {
        bitsPerSecond = capacityFor({1920, 1080, 60});
        basis = QStringLiteral("1080p60 示例");
    } else {
        for (const auto& specification : screenTrafficSpecifications_) bitsPerSecond += capacityFor(specification);
        const auto& first = screenTrafficSpecifications_.front();
        basis = screenTrafficSpecifications_.size() == 1
            ? QStringLiteral("%1×%2 · %3 FPS").arg(first[0]).arg(first[1]).arg(first[2])
            : QStringLiteral("%1 路屏幕合计").arg(screenTrafficSpecifications_.size());
    }
    const auto measurement = screenMeasuredTrafficAvailable_
        ? QStringLiteral("实测 %1 MB/s").arg(screenMeasuredTrafficBps_ / 8'000'000.0, 0, 'f', 2)
        : basis;
    const auto text = QStringLiteral("上限 %1 MB/s\n%2")
        .arg(bitsPerSecond / 8'000'000.0, 0, 'f', 2).arg(measurement);
    if (estimate->text() != text) estimate->setText(text);
    estimate->setToolTip(QStringLiteral("%1\n%2\n上限为屏幕视频码率上限；连接参考上限内部按它的 1.05 倍计算。实际发送会随网络、画面和场景策略变化。实测来自屏幕 RTP 采样，不等于任务管理器的全部网卡流量。").arg(text, basis));
}

}  // namespace remote::controller
