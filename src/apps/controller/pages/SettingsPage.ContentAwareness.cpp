// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "SettingsPage.h"
#include "SettingsPageUi.h"

#include <algorithm>
#include <cstdint>

#include <QByteArray>
#include <QApplication>
#include <QBuffer>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QImage>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QSettings>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QUuid>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

#include "media_intelligence/remote/VisionApiTypes.h"
#include "src/apps/controller/ControllerMainWindowSupport.h"
#include "src/apps/controller/RemoteCComboBox.h"
#include "src/apps/remote/adapters/VisionApiSettingsController.h"

namespace remote::controller {
namespace {

constexpr char kContentAnalyzerEnabledSetting[] =
    "media/contentAnalyzerEnabled";
constexpr char kContentAnalyzerModeSetting[] =
    "media/contentAnalyzerMode";
constexpr char kVisionApiProviderSetting[] =
    "media/visionApiProvider";
constexpr char kVisionApiBaseUrlSetting[] =
    "media/visionApiBaseUrl";
constexpr char kVisionApiModelSetting[] =
    "media/visionApiModel";
constexpr char kVisionApiCredentialIdSetting[] =
    "media/visionApiCredentialId";
constexpr char kVisionApiRequestIntervalSetting[] =
    "media/visionApiRequestIntervalSeconds";
constexpr char kVisionApiMaximumDimensionSetting[] =
    "media/visionApiMaximumImageDimension";
constexpr char kVisionApiJpegQualitySetting[] =
    "media/visionApiJpegQuality";
constexpr char kVisionApiConsentRevisionSetting[] =
    "media/visionApiConsentRevision";
constexpr char kVisionApiConfigRevisionSetting[] =
    "media/visionApiConfigRevision";
constexpr char kVisionApiTestedRevisionSetting[] =
    "media/visionApiTestedRevision";
constexpr std::uint64_t kVisionApiConsentRevision = 1;
constexpr int kVisionFieldMaximumWidth = 560;

std::string ToUtf8(const QString& value)
{
    const QByteArray utf8 = value.toUtf8();
    return std::string(
        utf8.constData(),
        static_cast<std::size_t>(utf8.size()));
}

QString VisionApiFailureText(
    const media_intelligence::RemoteSemanticResult& result)
{
    using media_intelligence::RemoteClassificationStatus;
    switch (result.status) {
    case RemoteClassificationStatus::kCredentialUnavailable:
        return QStringLiteral("API 密钥不可用");
    case RemoteClassificationStatus::kTimeout:
        return QStringLiteral("连接超时");
    case RemoteClassificationStatus::kTransportError:
        return QStringLiteral("网络连接失败");
    case RemoteClassificationStatus::kHttpError:
        if (result.httpStatus == 401 || result.httpStatus == 403) {
            return QStringLiteral("API 密钥无效或无权访问该模型");
        }
        if (result.httpStatus == 429) {
            return QStringLiteral("服务请求过于频繁或额度不足（HTTP 429）");
        }
        return QStringLiteral("服务返回 HTTP %1").arg(result.httpStatus);
    case RemoteClassificationStatus::kResponseTooLarge:
        return QStringLiteral("服务返回的数据过大");
    case RemoteClassificationStatus::kInvalidResponse:
        if (result.error == "response_empty") {
            return QStringLiteral("模型返回了空内容，请重试");
        }
        if (result.error == "response_content_missing") {
            return QStringLiteral("服务响应中缺少 message.content");
        }
        if (result.error == "classification_scene_invalid") {
            return QStringLiteral("模型返回的场景类型不受支持");
        }
        if (result.error == "classification_schema_invalid") {
            return QStringLiteral(
                "返回数据必须只包含场景类型（scene）和置信度（confidence）");
        }
        if (result.error.starts_with("json_")) {
            return QStringLiteral("模型返回的数据不是有效 JSON");
        }
        return QStringLiteral("模型返回格式不符合要求");
    case RemoteClassificationStatus::kCanceled:
        return QStringLiteral("测试已取消");
    case RemoteClassificationStatus::kCircuitOpen:
        return QStringLiteral("多次识别失败，已暂停请求");
    case RemoteClassificationStatus::kInvalidConfiguration:
        return QStringLiteral("API 配置无效");
    case RemoteClassificationStatus::kInvalidRequest:
        return QStringLiteral("测试请求无效");
    case RemoteClassificationStatus::kBusy:
        return QStringLiteral("已有测试正在运行");
    case RemoteClassificationStatus::kSuccess:
        return {};
    }
    return QStringLiteral("连接测试失败");
}

void ConfigureVisionField(QWidget* field)
{
    field->setMinimumWidth(240);
    field->setMaximumWidth(kVisionFieldMaximumWidth);
    field->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

media_intelligence::EncodedImage EncodeClipboardScreenshot(
    int maximumDimension,
    int jpegQuality)
{
    const QImage clipboardImage = QApplication::clipboard()->image();
    if (clipboardImage.isNull()) {
        return {};
    }
    const QSize boundedSize = clipboardImage.size().scaled(
        std::clamp(maximumDimension, 256, 1280),
        std::clamp(maximumDimension, 256, 1280),
        Qt::KeepAspectRatio);
    const QImage uploadImage =
        boundedSize == clipboardImage.size()
            ? clipboardImage
            : clipboardImage.scaled(
                  boundedSize,
                  Qt::KeepAspectRatio,
                  Qt::SmoothTransformation);
    QByteArray encoded;
    QBuffer buffer(&encoded);
    if (!buffer.open(QIODevice::WriteOnly) ||
        !uploadImage.save(
            &buffer, "JPEG", std::clamp(jpegQuality, 30, 90)) ||
        encoded.isEmpty()) {
        return {};
    }
    auto storage =
        std::make_shared<const media_intelligence::EncodedImage::Storage>(
            reinterpret_cast<const std::uint8_t*>(encoded.constData()),
            reinterpret_cast<const std::uint8_t*>(encoded.constData()) +
                encoded.size());
    return media_intelligence::EncodedImage(
        std::move(storage),
        media_intelligence::EncodedImageFormat::kJpeg,
        static_cast<std::uint32_t>(uploadImage.width()),
        static_cast<std::uint32_t>(uploadImage.height()));
}

QString ClassificationText(
    const media_intelligence::SemanticClassification& classification)
{
    const QString semantic = detail::ContentSceneDisplayText(
        media_intelligence::ScreenSceneName(classification.scene));
    return QStringLiteral("%1，置信度 %2%")
        .arg(semantic.isEmpty() ? QStringLiteral("未知") : semantic)
        .arg(qRound(classification.confidence * 100.0f));
}

}  // namespace

using namespace detail;

void SettingsPage::BuildContentAwarenessSettingsPage()
{
    const QSettings currentSettings;
    auto* page = new QWidget(detailStack_);
    page->setObjectName(QStringLiteral("contentAwarenessSettingsPage"));
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    AddSettingsDetailHeader(
        layout, page, QStringLiteral("AI 场景优化"),
        QStringLiteral("根据画面场景，自动调整网络波动时画质与帧率的取舍。"));

    const auto [enabledRow, enabledLayout] = CreateSettingsRow(
        page, QStringLiteral("AI 场景优化"),
        QStringLiteral("开启后根据场景自动调整取舍；关闭后使用手动设置，并停止场景识别。"));
    controls_.contentAwareStreamingSelector =
        new RemoteCComboBox(enabledRow);
    controls_.contentAwareStreamingSelector->setObjectName(
        QStringLiteral("capacitySelector"));
    controls_.contentAwareStreamingSelector->addItem(
        QStringLiteral("关闭"), false);
    controls_.contentAwareStreamingSelector->addItem(
        QStringLiteral("开启"), true);
    controls_.contentAwareStreamingSelector->setCurrentIndex(std::max(
        0,
        controls_.contentAwareStreamingSelector->findData(
            currentSettings.value(
                QString::fromLatin1(kContentAnalyzerEnabledSetting),
                false).toBool())));
    controls_.contentAwareStreamingSelector->setFixedWidth(
        kSettingsControlWidth);
    enabledLayout->addWidget(
        controls_.contentAwareStreamingSelector, 0, Qt::AlignVCenter);
    layout->addWidget(enabledRow);

    const auto [modeRow, modeLayout] = CreateSettingsRow(
        page, QStringLiteral("场景识别方式"),
        QStringLiteral("本地模型正在开发中，暂不支持场景识别。当前请使用 AI 模型 API。"));
    controls_.contentAnalyzerModeSelector = new RemoteCComboBox(modeRow);
    controls_.contentAnalyzerModeSelector->setObjectName(
        QStringLiteral("capacitySelector"));
    controls_.contentAnalyzerModeSelector->addItem(
        QStringLiteral("本地模型（开发中）"), QStringLiteral("local"));
    controls_.contentAnalyzerModeSelector->addItem(
        QStringLiteral("AI 模型 API"), QStringLiteral("vision_api"));
    controls_.contentAnalyzerModeSelector->setCurrentIndex(std::max(
        0,
        controls_.contentAnalyzerModeSelector->findData(
            currentSettings.value(
                QString::fromLatin1(kContentAnalyzerModeSetting),
                QStringLiteral("local")).toString())));
    controls_.contentAnalyzerModeSelector->setFixedWidth(
        kSettingsControlWidth);
    modeLayout->addWidget(
        controls_.contentAnalyzerModeSelector, 0, Qt::AlignVCenter);
    layout->addWidget(modeRow);

    controls_.visionApiConfigurationPanel = new QFrame(page);
    controls_.visionApiConfigurationPanel->setObjectName(
        QStringLiteral("settingRow"));
    auto* visionLayout = new QVBoxLayout(
        controls_.visionApiConfigurationPanel);
    visionLayout->setContentsMargins(20, 17, 20, 17);
    visionLayout->setSpacing(12);

    auto* apiTitle = new QLabel(
        QStringLiteral("AI 模型 API"),
        controls_.visionApiConfigurationPanel);
    apiTitle->setObjectName(QStringLiteral("settingTitle"));
    visionLayout->addWidget(apiTitle);
    auto* apiHint = new QLabel(
        QStringLiteral("支持 OpenAI 兼容接口。密钥通过 Windows DPAPI 保存，不写入配置文件或日志。"),
        controls_.visionApiConfigurationPanel);
    apiHint->setProperty("muted", true);
    apiHint->setWordWrap(true);
    visionLayout->addWidget(apiHint);

    auto* fields = new QGridLayout();
    fields->setContentsMargins(0, 4, 0, 0);
    fields->setHorizontalSpacing(18);
    fields->setVerticalSpacing(10);
    fields->setColumnMinimumWidth(0, 96);
    fields->setColumnStretch(1, 1);
    fields->setColumnStretch(2, 1);
    int fieldRow = 0;
    const auto addVisionField = [&fields, &fieldRow](
                                    const QString& labelText,
                                    QWidget* field) {
        auto* label = new QLabel(labelText, field->parentWidget());
        label->setObjectName(QStringLiteral("visionApiFieldLabel"));
        label->setProperty("muted", true);
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        fields->addWidget(label, fieldRow, 0, Qt::AlignVCenter);
        fields->addWidget(field, fieldRow, 1);
        ++fieldRow;
    };

    controls_.visionApiProviderSelector = new RemoteCComboBox(
        controls_.visionApiConfigurationPanel);
    controls_.visionApiProviderSelector->setObjectName(
        QStringLiteral("capacitySelector"));
    controls_.visionApiProviderSelector->addItem(
        QStringLiteral("OpenAI 兼容接口"),
        QStringLiteral("openai_compatible"));
    controls_.visionApiProviderSelector->addItem(
        QStringLiteral("DeepSeek"), QStringLiteral("deepseek"));
    ConfigureVisionField(controls_.visionApiProviderSelector);
    const QString configuredProvider = currentSettings.value(
        QString::fromLatin1(kVisionApiProviderSetting),
        QStringLiteral("openai_compatible")).toString();
    controls_.visionApiProviderSelector->setCurrentIndex(std::max(
        0,
        controls_.visionApiProviderSelector->findData(configuredProvider)));
    addVisionField(
        QStringLiteral("API 服务"), controls_.visionApiProviderSelector);

    media_intelligence::VisionApiEndpointConfig deepSeekPreset =
        media_intelligence::DeepSeekVisionApiPreset();
    const QString defaultBaseUrl = configuredProvider == QStringLiteral("deepseek")
        ? QString::fromStdString(deepSeekPreset.baseUrl)
        : QString();
    const QString defaultModel = configuredProvider == QStringLiteral("deepseek")
        ? QString::fromStdString(deepSeekPreset.model)
        : QString();

    controls_.visionApiBaseUrlEdit = new QLineEdit(
        controls_.visionApiConfigurationPanel);
    controls_.visionApiBaseUrlEdit->setObjectName(
        QStringLiteral("visionApiInput"));
    controls_.visionApiBaseUrlEdit->setMaxLength(2048);
    controls_.visionApiBaseUrlEdit->setPlaceholderText(
        QStringLiteral("https://api.example.com/v1"));
    controls_.visionApiBaseUrlEdit->setText(currentSettings.value(
        QString::fromLatin1(kVisionApiBaseUrlSetting),
        defaultBaseUrl).toString());
    ConfigureVisionField(controls_.visionApiBaseUrlEdit);
    addVisionField(QStringLiteral("API 地址"), controls_.visionApiBaseUrlEdit);

    controls_.visionApiModelEdit = new QLineEdit(
        controls_.visionApiConfigurationPanel);
    controls_.visionApiModelEdit->setObjectName(
        QStringLiteral("visionApiInput"));
    controls_.visionApiModelEdit->setMaxLength(256);
    controls_.visionApiModelEdit->setPlaceholderText(
        QStringLiteral("输入支持图片识别的模型名称"));
    controls_.visionApiModelEdit->setText(currentSettings.value(
        QString::fromLatin1(kVisionApiModelSetting),
        defaultModel).toString());
    ConfigureVisionField(controls_.visionApiModelEdit);
    addVisionField(QStringLiteral("模型名称"), controls_.visionApiModelEdit);

    controls_.visionApiKeyEdit = new QLineEdit(
        controls_.visionApiConfigurationPanel);
    controls_.visionApiKeyEdit->setObjectName(
        QStringLiteral("visionApiInput"));
    controls_.visionApiKeyEdit->setEchoMode(QLineEdit::Password);
    controls_.visionApiKeyEdit->setMaxLength(4096);
    ConfigureVisionField(controls_.visionApiKeyEdit);
    addVisionField(QStringLiteral("API 密钥"), controls_.visionApiKeyEdit);

    controls_.visionApiRequestIntervalSelector = new RemoteCComboBox(
        controls_.visionApiConfigurationPanel);
    controls_.visionApiRequestIntervalSelector->setObjectName(
        QStringLiteral("capacitySelector"));
    for (const int intervalMs : {500, 1000, 2000, 5000, 10000, 20000, 30000, 60000}) {
        controls_.visionApiRequestIntervalSelector->addItem(
            QStringLiteral("%1 秒").arg(intervalMs / 1000.0, 0, 'g', 3),
            intervalMs);
    }
    const int savedIntervalMs = static_cast<int>(
        media_intelligence::NormalizeVisionApiRequestIntervalMs(
            currentSettings.value(
                QString::fromLatin1(kVisionApiRequestIntervalSetting),
                media_intelligence::kDefaultVisionApiRequestIntervalMs / 1000.0).toDouble()));
    const int savedIntervalIndex =
        controls_.visionApiRequestIntervalSelector->findData(savedIntervalMs);
    controls_.visionApiRequestIntervalSelector->setCurrentIndex(
        savedIntervalIndex >= 0 ? savedIntervalIndex
            : controls_.visionApiRequestIntervalSelector->findData(
                media_intelligence::kDefaultVisionApiRequestIntervalMs));
    ConfigureVisionField(controls_.visionApiRequestIntervalSelector);
    addVisionField(
        QStringLiteral("识别间隔"),
        controls_.visionApiRequestIntervalSelector);

    controls_.visionApiMaximumDimensionSelector = new RemoteCComboBox(
        controls_.visionApiConfigurationPanel);
    controls_.visionApiMaximumDimensionSelector->setObjectName(
        QStringLiteral("capacitySelector"));
    for (const int pixels : {256, 384, 512, 768, 1024, 1280}) {
        controls_.visionApiMaximumDimensionSelector->addItem(
            QStringLiteral("%1 像素").arg(pixels), pixels);
    }
    controls_.visionApiMaximumDimensionSelector->setCurrentIndex(std::max(
        0,
        controls_.visionApiMaximumDimensionSelector->findData(
            currentSettings.value(
                QString::fromLatin1(kVisionApiMaximumDimensionSetting),
                media_intelligence::kDefaultVisionApiMaximumImageDimension).toInt())));
    ConfigureVisionField(controls_.visionApiMaximumDimensionSelector);
    controls_.visionApiMaximumDimensionSelector->setToolTip(
        QStringLiteral("按上传图片的最长边计算，保持原有宽高比例。"));
    addVisionField(
        QStringLiteral("上传图片尺寸"),
        controls_.visionApiMaximumDimensionSelector);

    controls_.visionApiJpegQualitySelector = new RemoteCComboBox(
        controls_.visionApiConfigurationPanel);
    controls_.visionApiJpegQualitySelector->setObjectName(
        QStringLiteral("capacitySelector"));
    for (const int quality : {40, 50, 60, 70, 80, 90}) {
        controls_.visionApiJpegQualitySelector->addItem(
            QStringLiteral("%1").arg(quality), quality);
    }
    controls_.visionApiJpegQualitySelector->setCurrentIndex(std::max(
        0,
        controls_.visionApiJpegQualitySelector->findData(
            currentSettings.value(
                QString::fromLatin1(kVisionApiJpegQualitySetting),
                media_intelligence::kDefaultVisionApiJpegQuality).toInt())));
    ConfigureVisionField(controls_.visionApiJpegQualitySelector);
    addVisionField(
        QStringLiteral("图片质量"),
        controls_.visionApiJpegQualitySelector);
    visionLayout->addLayout(fields);

    controls_.visionApiConsentCheckBox = new QCheckBox(
        QStringLiteral("我同意上传屏幕缩略图用于场景识别"),
        controls_.visionApiConfigurationPanel);
    controls_.visionApiConsentCheckBox->setObjectName(
        QStringLiteral("visionApiConsentCheckBox"));
    controls_.visionApiConsentCheckBox->setChecked(
        currentSettings.value(
            QString::fromLatin1(kVisionApiConsentRevisionSetting),
            0).toULongLong() == kVisionApiConsentRevision);
    visionLayout->addWidget(controls_.visionApiConsentCheckBox);
    auto* consentHint = new QLabel(
        QStringLiteral("缩略图会发送到所选第三方服务，并可能产生服务费用。"),
        controls_.visionApiConfigurationPanel);
    consentHint->setProperty("muted", true);
    consentHint->setWordWrap(true);
    visionLayout->addWidget(consentHint);

    auto* visionActions = new QHBoxLayout();
    controls_.visionApiTestButton = new QPushButton(
        QStringLiteral("测试连接"), controls_.visionApiConfigurationPanel);
    controls_.visionApiTestButton->setObjectName(
        QStringLiteral("softButton"));
    controls_.visionApiScreenshotTestButton = new QPushButton(
        QStringLiteral("测试截图识别"), controls_.visionApiConfigurationPanel);
    controls_.visionApiScreenshotTestButton->setObjectName(
        QStringLiteral("softButton"));
    controls_.visionApiScreenshotTestButton->setToolTip(
        QStringLiteral("先按 Win+Shift+S 截图，再用剪贴板中的图片测试识别"));
    controls_.visionApiClearKeyButton = new QPushButton(
        QStringLiteral("清除密钥"), controls_.visionApiConfigurationPanel);
    controls_.visionApiClearKeyButton->setObjectName(
        QStringLiteral("softButton"));
    visionActions->addWidget(controls_.visionApiTestButton);
    visionActions->addWidget(controls_.visionApiScreenshotTestButton);
    visionActions->addWidget(controls_.visionApiClearKeyButton);
    visionActions->addStretch(1);
    visionLayout->addLayout(visionActions);
    auto* screenshotTestHint = new QLabel(
        QStringLiteral(
            "测试使用剪贴板中的截图，不会将图片保存到本地。"),
        controls_.visionApiConfigurationPanel);
    screenshotTestHint->setProperty("muted", true);
    screenshotTestHint->setWordWrap(true);
    visionLayout->addWidget(screenshotTestHint);

    controls_.visionApiStatusLabel = new QLabel(
        controls_.visionApiConfigurationPanel);
    controls_.visionApiStatusLabel->setProperty("muted", true);
    controls_.visionApiStatusLabel->setWordWrap(true);
    controls_.visionApiStatusLabel->setTextInteractionFlags(
        Qt::TextSelectableByMouse);
    visionLayout->addWidget(controls_.visionApiStatusLabel);
    layout->addWidget(controls_.visionApiConfigurationPanel);
    layout->addStretch(1);
    detailStack_->addWidget(page);

    visionApiSettingsController_ =
        new remote::app::VisionApiSettingsController(this);

    connect(
        controls_.contentAwareStreamingSelector,
        &QComboBox::currentIndexChanged,
        this,
        [this](int) {
            QSettings().setValue(
                QString::fromLatin1(kContentAnalyzerEnabledSetting),
                controls_.contentAwareStreamingSelector->currentData().toBool());
            RefreshVisionApiSettingsUi();
        });
    connect(
        controls_.contentAnalyzerModeSelector,
        &QComboBox::currentIndexChanged,
        this,
        [this](int) {
            QSettings().setValue(
                QString::fromLatin1(kContentAnalyzerModeSetting),
                controls_.contentAnalyzerModeSelector->currentData().toString());
            RefreshVisionApiSettingsUi();
        });
    connect(
        controls_.visionApiProviderSelector,
        &QComboBox::currentIndexChanged,
        this,
        [this](int) {
            if (controls_.visionApiProviderSelector->currentData().toString() ==
                QStringLiteral("deepseek")) {
                const auto preset =
                    media_intelligence::DeepSeekVisionApiPreset();
                const QSignalBlocker baseBlocker(
                    controls_.visionApiBaseUrlEdit);
                const QSignalBlocker modelBlocker(
                    controls_.visionApiModelEdit);
                controls_.visionApiBaseUrlEdit->setText(
                    QString::fromStdString(preset.baseUrl));
                controls_.visionApiModelEdit->setText(
                    QString::fromStdString(preset.model));
            }
            PersistVisionApiConfiguration(true);
            RefreshVisionApiSettingsUi();
        });
    connect(
        controls_.visionApiBaseUrlEdit,
        &QLineEdit::editingFinished,
        this,
        [this] {
            PersistVisionApiConfiguration(true);
            RefreshVisionApiSettingsUi();
        });
    connect(
        controls_.visionApiModelEdit,
        &QLineEdit::editingFinished,
        this,
        [this] {
            PersistVisionApiConfiguration(true);
            RefreshVisionApiSettingsUi();
        });
    connect(
        controls_.visionApiRequestIntervalSelector,
        &QComboBox::currentIndexChanged,
        this,
        [this](int) {
            PersistVisionApiConfiguration(true);
            RefreshVisionApiSettingsUi();
        });
    connect(
        controls_.visionApiMaximumDimensionSelector,
        &QComboBox::currentIndexChanged,
        this,
        [this](int) {
            PersistVisionApiConfiguration(true);
            RefreshVisionApiSettingsUi();
        });
    connect(
        controls_.visionApiJpegQualitySelector,
        &QComboBox::currentIndexChanged,
        this,
        [this](int) {
            PersistVisionApiConfiguration(true);
            RefreshVisionApiSettingsUi();
        });
    connect(
        controls_.visionApiConsentCheckBox,
        &QCheckBox::toggled,
        this,
        [this](bool checked) {
            QSettings().setValue(
                QString::fromLatin1(kVisionApiConsentRevisionSetting),
                checked ? QVariant::fromValue<qulonglong>(
                              kVisionApiConsentRevision)
                        : QVariant::fromValue<qulonglong>(0));
            RefreshVisionApiSettingsUi();
        });
    connect(
        controls_.visionApiKeyEdit,
        &QLineEdit::textChanged,
        this,
        [this] { RefreshVisionApiSettingsUi(); });
    connect(
        controls_.visionApiTestButton,
        &QPushButton::clicked,
        this,
        [this] { StartVisionApiConnectionTest(); });
    connect(
        controls_.visionApiScreenshotTestButton,
        &QPushButton::clicked,
        this,
        [this] { StartVisionApiScreenshotTest(); });
    connect(
        controls_.visionApiClearKeyButton,
        &QPushButton::clicked,
        this,
        [this] { ClearVisionApiCredential(); });
    RefreshVisionApiSettingsUi();
}

std::uint64_t SettingsPage::PersistVisionApiConfiguration(
    bool invalidateTest)
{
    QSettings settings;
    const QString provider =
        controls_.visionApiProviderSelector->currentData().toString();
    QString baseUrl = controls_.visionApiBaseUrlEdit->text().trimmed();
    while (baseUrl.endsWith(QLatin1Char('/')) && baseUrl.size() > 8) {
        baseUrl.chop(1);
    }
    const QString model = controls_.visionApiModelEdit->text().trimmed();
    const double interval =
        controls_.visionApiRequestIntervalSelector->currentData().toInt() / 1000.0;
    const int maximumDimension =
        controls_.visionApiMaximumDimensionSelector->currentData().toInt();
    const int jpegQuality =
        controls_.visionApiJpegQualitySelector->currentData().toInt();

    const bool changed =
        settings.value(
            QString::fromLatin1(kVisionApiProviderSetting),
            QStringLiteral("openai_compatible")).toString() != provider ||
        settings.value(
            QString::fromLatin1(kVisionApiBaseUrlSetting)).toString() != baseUrl ||
        settings.value(
            QString::fromLatin1(kVisionApiModelSetting)).toString() != model ||
        settings.value(
            QString::fromLatin1(kVisionApiRequestIntervalSetting),
            media_intelligence::kDefaultVisionApiRequestIntervalMs / 1000.0).toDouble() != interval;
    settings.setValue(
        QString::fromLatin1(kVisionApiProviderSetting), provider);
    settings.setValue(
        QString::fromLatin1(kVisionApiBaseUrlSetting), baseUrl);
    settings.setValue(
        QString::fromLatin1(kVisionApiModelSetting), model);
    settings.setValue(
        QString::fromLatin1(kVisionApiRequestIntervalSetting), interval);
    settings.setValue(
        QString::fromLatin1(kVisionApiMaximumDimensionSetting),
        maximumDimension);
    settings.setValue(
        QString::fromLatin1(kVisionApiJpegQualitySetting), jpegQuality);

    std::uint64_t revision = settings.value(
        QString::fromLatin1(kVisionApiConfigRevisionSetting),
        0).toULongLong();
    if (revision == 0 || (changed && invalidateTest)) {
        ++revision;
        settings.setValue(
            QString::fromLatin1(kVisionApiConfigRevisionSetting),
            QVariant::fromValue<qulonglong>(revision));
        settings.setValue(
            QString::fromLatin1(kVisionApiTestedRevisionSetting),
            QVariant::fromValue<qulonglong>(0));
        if (visionApiTestRunning_ && visionApiSettingsController_) {
            visionApiSettingsController_->CancelTest();
            visionApiTestRunning_ = false;
        }
    }
    return revision;
}

void SettingsPage::RefreshVisionApiSettingsUi()
{
    if (!controls_.visionApiConfigurationPanel ||
        !visionApiSettingsController_) {
        return;
    }
    const bool remoteMode =
        controls_.contentAnalyzerModeSelector->currentData().toString() ==
        QStringLiteral("vision_api");
    controls_.visionApiConfigurationPanel->setVisible(remoteMode);

    const QSettings settings;
    const std::string credentialId = ToUtf8(settings.value(
        QString::fromLatin1(kVisionApiCredentialIdSetting)).toString());
    const bool hasStoredCredential =
        !credentialId.empty() &&
        visionApiSettingsController_->HasCredential(credentialId);
    const bool hasNewCredential =
        !controls_.visionApiKeyEdit->text().isEmpty();
    controls_.visionApiKeyEdit->setPlaceholderText(
        hasStoredCredential
            ? QStringLiteral("已使用 Windows DPAPI 安全保存")
            : QStringLiteral("输入你自己的 API 密钥"));
    controls_.visionApiClearKeyButton->setEnabled(
        hasStoredCredential && !visionApiTestRunning_);
    controls_.visionApiTestButton->setEnabled(
        !visionApiTestRunning_ &&
        (hasStoredCredential || hasNewCredential) &&
        !controls_.visionApiBaseUrlEdit->text().trimmed().isEmpty() &&
        !controls_.visionApiModelEdit->text().trimmed().isEmpty());
    controls_.visionApiScreenshotTestButton->setEnabled(
        controls_.visionApiTestButton->isEnabled());
    controls_.visionApiTestButton->setText(
        visionApiTestRunning_ ? QStringLiteral("正在测试…")
                              : QStringLiteral("测试连接"));
    controls_.visionApiScreenshotTestButton->setText(
        visionApiTestRunning_ ? QStringLiteral("正在测试…")
                              : QStringLiteral("测试截图识别"));
    if (visionApiTestRunning_) {
        controls_.visionApiStatusLabel->setText(
            QStringLiteral("正在测试模型连接和场景识别…"));
        return;
    }

    const std::uint64_t revision = settings.value(
        QString::fromLatin1(kVisionApiConfigRevisionSetting),
        0).toULongLong();
    const std::uint64_t testedRevision = settings.value(
        QString::fromLatin1(kVisionApiTestedRevisionSetting),
        0).toULongLong();
    if (!hasStoredCredential) {
        controls_.visionApiStatusLabel->setText(
            QStringLiteral("尚未保存 API 密钥。密钥不会写入配置文件或日志。"));
    } else if (revision != 0 && testedRevision == revision) {
        controls_.visionApiStatusLabel->setText(
            controls_.visionApiConsentCheckBox->isChecked()
                ? QStringLiteral("测试通过，可以使用场景识别。")
                : QStringLiteral("测试通过；同意上传屏幕缩略图后即可使用。"));
    } else {
        controls_.visionApiStatusLabel->setText(
            QStringLiteral("密钥已保存，请测试连接。"));
    }
}

void SettingsPage::StartVisionApiConnectionTest()
{
    StartVisionApiConnectionTestWithImage(
        nullptr, QStringLiteral("内置测试图片"));
}

void SettingsPage::StartVisionApiScreenshotTest()
{
    auto testImage = std::make_unique<media_intelligence::EncodedImage>(
        EncodeClipboardScreenshot(
            controls_.visionApiMaximumDimensionSelector->currentData().toInt(),
            controls_.visionApiJpegQualitySelector->currentData().toInt()));
    if (!testImage->IsValid()) {
        controls_.visionApiStatusLabel->setText(
            QStringLiteral(
                "剪贴板中没有可用图片。请先按 Win+Shift+S 截图，再点击“测试截图识别”。"));
        return;
    }
    StartVisionApiConnectionTestWithImage(
        std::move(testImage), QStringLiteral("剪贴板截图"));
}

void SettingsPage::StartVisionApiConnectionTestWithImage(
    std::unique_ptr<media_intelligence::EncodedImage> testImage,
    const QString& testImageName)
{
    if (!visionApiSettingsController_ || visionApiTestRunning_) {
        return;
    }
    std::uint64_t revision = PersistVisionApiConfiguration(true);
    QSettings settings;
    QString credentialId = settings.value(
        QString::fromLatin1(kVisionApiCredentialIdSetting)).toString();
    QByteArray enteredSecret =
        controls_.visionApiKeyEdit->text().trimmed().toUtf8();
    if (!enteredSecret.isEmpty()) {
        if (credentialId.isEmpty()) {
            credentialId = QStringLiteral("vision-api-%1").arg(
                QUuid::createUuid().toString(QUuid::WithoutBraces));
        }
        std::string secret(
            enteredSecret.constData(),
            static_cast<std::size_t>(enteredSecret.size()));
        std::string saveError;
        const bool saved = visionApiSettingsController_->SaveCredential(
            ToUtf8(credentialId), secret, &saveError);
        volatile char* secretData = secret.empty() ? nullptr : secret.data();
        for (std::size_t index = 0;
             secretData && index < secret.size();
             ++index) {
            secretData[index] = '\0';
        }
        secret.clear();
        std::fill(enteredSecret.begin(), enteredSecret.end(), '\0');
        enteredSecret.clear();
        controls_.visionApiKeyEdit->clear();
        if (!saved) {
            RefreshVisionApiSettingsUi();
            controls_.visionApiStatusLabel->setText(
                QStringLiteral("保存 API 密钥失败：%1")
                    .arg(QString::fromStdString(saveError)));
            return;
        }
        settings.setValue(
            QString::fromLatin1(kVisionApiCredentialIdSetting),
            credentialId);
        ++revision;
        settings.setValue(
            QString::fromLatin1(kVisionApiConfigRevisionSetting),
            QVariant::fromValue<qulonglong>(revision));
        settings.setValue(
            QString::fromLatin1(kVisionApiTestedRevisionSetting),
            QVariant::fromValue<qulonglong>(0));
    }
    if (credentialId.isEmpty() ||
        !visionApiSettingsController_->HasCredential(ToUtf8(credentialId))) {
        RefreshVisionApiSettingsUi();
        controls_.visionApiStatusLabel->setText(
            QStringLiteral("请先输入 API 密钥。"));
        return;
    }

    media_intelligence::VisionApiEndpointConfig endpoint;
    if (controls_.visionApiProviderSelector->currentData().toString() ==
        QStringLiteral("deepseek")) {
        endpoint = media_intelligence::DeepSeekVisionApiPreset();
    }
    endpoint.providerId = ToUtf8(
        controls_.visionApiProviderSelector->currentData().toString());
    endpoint.baseUrl = ToUtf8(
        controls_.visionApiBaseUrlEdit->text().trimmed());
    endpoint.model = ToUtf8(
        controls_.visionApiModelEdit->text().trimmed());
    endpoint.credentialId = ToUtf8(credentialId);
    endpoint.timeoutMs = 8000;
    endpoint.maximumResponseBytes = 16 * 1024;

    visionApiTestRunning_ = true;
    RefreshVisionApiSettingsUi();
    QPointer<SettingsPage> owner(this);
    std::string startError;
    remote::app::VisionApiSettingsController::TestCompletion completion =
        [owner, revision, testImageName](
            media_intelligence::RemoteSemanticResult result) {
                if (!owner) {
                    return;
                }
                owner->visionApiTestRunning_ = false;
                QSettings settings;
                const std::uint64_t currentRevision = settings.value(
                    QString::fromLatin1(kVisionApiConfigRevisionSetting),
                    0).toULongLong();
                if (result.status ==
                        media_intelligence::RemoteClassificationStatus::kSuccess &&
                    currentRevision == revision) {
                    settings.setValue(
                        QString::fromLatin1(kVisionApiTestedRevisionSetting),
                        QVariant::fromValue<qulonglong>(revision));
                } else if (currentRevision == revision) {
                    settings.setValue(
                        QString::fromLatin1(kVisionApiTestedRevisionSetting),
                        QVariant::fromValue<qulonglong>(0));
                }
                owner->RefreshVisionApiSettingsUi();
                if (result.status ==
                        media_intelligence::RemoteClassificationStatus::kSuccess &&
                    currentRevision == revision) {
                    owner->controls_.visionApiStatusLabel->setText(
                        QStringLiteral("测试通过，%1的识别结果：%2。")
                            .arg(testImageName,
                                 ClassificationText(result.classification)));
                } else if (currentRevision != revision) {
                    owner->controls_.visionApiStatusLabel->setText(
                        QStringLiteral("设置已更改，请重新测试。"));
                } else {
                    owner->controls_.visionApiStatusLabel->setText(
                        QStringLiteral("连接测试失败：%1")
                            .arg(VisionApiFailureText(result)));
                }
            };
    const bool started = testImage
        ? visionApiSettingsController_->TestConnectionWithImage(
              std::move(endpoint), std::move(*testImage),
              std::move(completion), &startError)
        : visionApiSettingsController_->TestConnection(
              std::move(endpoint),
              controls_.visionApiMaximumDimensionSelector->currentData().toInt(),
              controls_.visionApiJpegQualitySelector->currentData().toInt(),
              std::move(completion), &startError);
    if (!started) {
        visionApiTestRunning_ = false;
        RefreshVisionApiSettingsUi();
        controls_.visionApiStatusLabel->setText(
            QStringLiteral("无法开始连接测试：%1")
                .arg(QString::fromStdString(startError)));
    }
}

void SettingsPage::ClearVisionApiCredential()
{
    if (!visionApiSettingsController_) {
        return;
    }
    QSettings settings;
    const QString credentialId = settings.value(
        QString::fromLatin1(kVisionApiCredentialIdSetting)).toString();
    std::string error;
    if (!credentialId.isEmpty() &&
        !visionApiSettingsController_->DeleteCredential(
            ToUtf8(credentialId), &error)) {
        RefreshVisionApiSettingsUi();
        controls_.visionApiStatusLabel->setText(
            QStringLiteral("清除 API 密钥失败：%1")
                .arg(QString::fromStdString(error)));
        return;
    }
    settings.remove(QString::fromLatin1(kVisionApiCredentialIdSetting));
    const std::uint64_t revision = settings.value(
        QString::fromLatin1(kVisionApiConfigRevisionSetting),
        0).toULongLong() + 1;
    settings.setValue(
        QString::fromLatin1(kVisionApiConfigRevisionSetting),
        QVariant::fromValue<qulonglong>(revision));
    settings.setValue(
        QString::fromLatin1(kVisionApiTestedRevisionSetting),
        QVariant::fromValue<qulonglong>(0));
    controls_.visionApiKeyEdit->clear();
    RefreshVisionApiSettingsUi();
    controls_.visionApiStatusLabel->setText(
        QStringLiteral("API 密钥已清除。"));
}

}  // namespace remote::controller
