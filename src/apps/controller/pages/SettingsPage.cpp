// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "SettingsPage.h"
#include "SettingsPageUi.h"

#include <algorithm>
#include <utility>

#include <QApplication>
#include <QButtonGroup>
#include <QClipboard>
#include <QCoreApplication>
#include <QFrame>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "src/apps/controller/ControllerMainWindowSupport.h"
#include "src/apps/controller/CurrentPageStack.h"
#include "src/apps/controller/FramelessWindow.h"
#include "src/apps/controller/RemoteCComboBox.h"
#include "src/apps/controller/RemoteCToast.h"
#include "src/apps/controller/ui/RemoteCTheme.h"

namespace remote::controller {
using namespace detail;

SettingsPage::SettingsPage(QWidget* parent)
    : QScrollArea(parent)
{
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    EnableSmoothWheelScrolling(this);

    auto* content = new QWidget(this);
    contentLayout_ = new QVBoxLayout(content);
    contentLayout_->setContentsMargins(32, 28, 32, 30);
    contentLayout_->setSpacing(20);

    auto* header = new QVBoxLayout();
    header->setSpacing(3);
    auto* title = new QLabel(QStringLiteral("设置"), content);
    title->setObjectName(QStringLiteral("pageTitle"));
    auto* subtitle = new QLabel(
        QStringLiteral("按分类调整 RLink 在这台电脑上的行为。"),
        content);
    subtitle->setObjectName(QStringLiteral("pageSubtitle"));
    subtitle->setWordWrap(true);
    header->addWidget(title);
    header->addWidget(subtitle);
    contentLayout_->addLayout(header);

    auto* workspace = new QHBoxLayout();
    workspace->setSpacing(20);

    auto* categories = new QFrame(content);
    categories->setObjectName(QStringLiteral("settingsCategoryPanel"));
    categories->setFixedWidth(120);
    categories->setFixedHeight(549);
    auto* categoryLayout = new QVBoxLayout(categories);
    categoryLayout->setContentsMargins(10, 12, 10, 12);
    categoryLayout->setSpacing(5);
    auto* categoryCaption = new QLabel(
        QStringLiteral("设置分类"), categories);
    categoryCaption->setProperty("muted", true);
    categoryCaption->setContentsMargins(10, 3, 0, 7);
    categoryLayout->addWidget(categoryCaption);

    categoryGroup_ = new QButtonGroup(this);
    categoryGroup_->setExclusive(true);
    const auto makeCategoryButton =
        [categories, categoryLayout, this](
            const QString& text, const int index) {
            auto* button = new QPushButton(text, categories);
            button->setObjectName(QStringLiteral("settingsCategoryButton"));
            button->setCheckable(true);
            button->setCursor(Qt::PointingHandCursor);
            button->setFixedHeight(58);
            categoryGroup_->addButton(button, index);
            categoryLayout->addWidget(button);
            return button;
        };
    auto* generalCategoryButton = makeCategoryButton(
        QStringLiteral("常规"), 0);
    makeCategoryButton(QStringLiteral("远程桌面"), 1);
    makeCategoryButton(QStringLiteral("内容感知"), 2);
    makeCategoryButton(QStringLiteral("音视频设备"), 3);
    makeCategoryButton(QStringLiteral("文件传输"), 4);
    makeCategoryButton(QStringLiteral("远程粘贴"), 5);
    makeCategoryButton(QStringLiteral("快捷键"), 6);
    categoryLayout->addStretch(1);
    workspace->addWidget(categories, 0, Qt::AlignTop);

    // Hidden codec summaries and long-text categories must not participate in
    // sizing the visible category. Let the scroll area track its natural height.
    auto* currentDetailStack = new CurrentPageStack(content, false);
    detailStack_ = currentDetailStack;
    detailStack_->setObjectName(QStringLiteral("settingsDetailStack"));
    detailStack_->setMinimumWidth(0);
    detailStack_->setSizePolicy(
        QSizePolicy::Ignored, QSizePolicy::Expanding);
    workspace->addItem(new CurrentPageStackItem(currentDetailStack));
    workspace->setStretch(workspace->count() - 1, 1);
    contentLayout_->addLayout(workspace, 1);

    connect(categoryGroup_, &QButtonGroup::idClicked,
            this, [this](const int index) {
                detailStack_->setCurrentIndex(index);
                emit CategoryChanged(index);
            });
    BuildGeneralSettingsPage();
    BuildRemoteDesktopSettingsPage();
    BuildContentAwarenessSettingsPage();
    BuildAudioVideoSettingsPage();
    BuildFileTransferSettingsPage();
    BuildRemotePasteSettingsPage();
    BuildShortcutSettingsPage();
    generalCategoryButton->setChecked(true);
    detailStack_->setCurrentIndex(0);
    setWidget(content);
}

void SettingsPage::BuildGeneralSettingsPage()
{
    const QSettings currentSettings;
    auto* page = new QWidget(detailStack_);
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    AddSettingsDetailHeader(
        layout, page, QStringLiteral("常规"),
        QStringLiteral("调整界面显示、交互反馈和本机配置。"));

    const auto [themeRow, themeLayout] = CreateSettingsRow(
        page, QStringLiteral("界面主题"),
        QStringLiteral("选择浅色、深色，或随 Windows 外观自动切换。"));
    controls_.themeModeSelector = new RemoteCComboBox(themeRow);
    controls_.themeModeSelector->setObjectName(QStringLiteral("capacitySelector"));
    controls_.themeModeSelector->addItem(
        QStringLiteral("跟随系统（推荐）"), QStringLiteral("system"));
    controls_.themeModeSelector->addItem(
        QStringLiteral("浅色"), QStringLiteral("light"));
    controls_.themeModeSelector->addItem(
        QStringLiteral("深色"), QStringLiteral("dark"));
    const QString configuredTheme = ui::RemoteCTheme::PreferenceValue(
        ui::RemoteCTheme::LoadPreference());
    controls_.themeModeSelector->setCurrentIndex(std::max(
        0, controls_.themeModeSelector->findData(configuredTheme)));
    controls_.themeModeSelector->setFixedWidth(kSettingsControlWidth);
    themeLayout->addWidget(
        controls_.themeModeSelector, 0, Qt::AlignVCenter);
    layout->addWidget(themeRow);

    const auto [motionRow, motionLayout] = CreateSettingsRow(
        page, QStringLiteral("界面动画"),
        QStringLiteral("调整提示、弹窗和轻量面板的动画效果。"));
    controls_.animationLevelSelector = new RemoteCComboBox(motionRow);
    controls_.animationLevelSelector->setObjectName(
        QStringLiteral("capacitySelector"));
    controls_.animationLevelSelector->addItem(QStringLiteral("关闭"), 0);
    controls_.animationLevelSelector->addItem(QStringLiteral("简洁"), 1);
    controls_.animationLevelSelector->addItem(
        QStringLiteral("完整（推荐）"), 2);
    const int legacyAnimationLevel = currentSettings.value(
        QStringLiteral("ui/animationsEnabled"), true).toBool() ? 2 : 0;
    const int configuredAnimationLevel = std::clamp(
        currentSettings.value(QStringLiteral("ui/animationLevel"),
                              legacyAnimationLevel).toInt(),
        0, 2);
    controls_.animationLevelSelector->setCurrentIndex(std::max(
        0, controls_.animationLevelSelector->findData(
               configuredAnimationLevel)));
    controls_.animationLevelSelector->setFixedWidth(kSettingsControlWidth);
    motionLayout->addWidget(
        controls_.animationLevelSelector, 0, Qt::AlignVCenter);
    layout->addWidget(motionRow);

    const auto [fontFamilyRow, fontFamilyLayout] = CreateSettingsRow(
        page, QStringLiteral("界面字体"),
        QStringLiteral("使用这台电脑已安装且支持中文的字体。"));
    controls_.fontFamilySelector = new RemoteCComboBox(fontFamilyRow);
    controls_.fontFamilySelector->setObjectName(
        QStringLiteral("capacitySelector"));
    controls_.fontFamilySelector->addItem(
        QStringLiteral("默认（Microsoft YaHei UI）"), QString());
    for (const QString& family : InstalledChineseInterfaceFonts()) {
        controls_.fontFamilySelector->addItem(family, family);
        controls_.fontFamilySelector->setItemData(
            controls_.fontFamilySelector->count() - 1,
            QFont(family), Qt::FontRole);
    }
    const QString configuredFontFamily = currentSettings.value(
        QString::fromLatin1(kInterfaceFontFamilySetting)).toString();
    const int configuredFontFamilyIndex =
        controls_.fontFamilySelector->findData(configuredFontFamily);
    controls_.fontFamilySelector->setCurrentIndex(
        configuredFontFamilyIndex >= 0 ? configuredFontFamilyIndex : 0);
    controls_.fontFamilySelector->setFixedWidth(kSettingsControlWidth);
    controls_.fontFamilySelector->setToolTip(
        QStringLiteral("列表来自 Windows 系统字体，不会向安装包添加中文字体文件。"));
    fontFamilyLayout->addWidget(
        controls_.fontFamilySelector, 0, Qt::AlignVCenter);
    layout->addWidget(fontFamilyRow);

    const auto [fontRow, fontLayout] = CreateSettingsRow(
        page, QStringLiteral("字体大小"),
        QStringLiteral("调整主窗口和独立窗口的界面文字大小。"));
    controls_.fontSizeSelector = new RemoteCComboBox(fontRow);
    controls_.fontSizeSelector->setObjectName(QStringLiteral("capacitySelector"));
    controls_.fontSizeSelector->addItem(QStringLiteral("小"), 12);
    controls_.fontSizeSelector->addItem(QStringLiteral("标准"), 13);
    controls_.fontSizeSelector->addItem(QStringLiteral("大"), 15);
    controls_.fontSizeSelector->addItem(QStringLiteral("特大"), 17);
    const int configuredFontSize = currentSettings.value(
        QStringLiteral("ui/fontPixelSize"), 13).toInt();
    controls_.fontSizeSelector->setCurrentIndex(std::max(
        0, controls_.fontSizeSelector->findData(configuredFontSize)));
    controls_.fontSizeSelector->setFixedWidth(kSettingsControlWidth);
    fontLayout->addWidget(
        controls_.fontSizeSelector, 0, Qt::AlignVCenter);
    layout->addWidget(fontRow);

    const auto [autoStartRow, autoStartLayout] = CreateSettingsRow(
        page, QStringLiteral("开机启动"),
        QStringLiteral("登录 Windows 后自动启动 RLink。"));
    controls_.autoStartSelector = new RemoteCComboBox(autoStartRow);
    controls_.autoStartSelector->setObjectName(QStringLiteral("capacitySelector"));
    controls_.autoStartSelector->addItem(QStringLiteral("关闭"), false);
    controls_.autoStartSelector->addItem(QStringLiteral("开启"), true);
    controls_.autoStartSelector->setCurrentIndex(std::max(
        0, controls_.autoStartSelector->findData(WindowsAutoStartEnabled())));
    controls_.autoStartSelector->setFixedWidth(kSettingsControlWidth);
    autoStartLayout->addWidget(
        controls_.autoStartSelector, 0, Qt::AlignVCenter);
    layout->addWidget(autoStartRow);

    const auto [startupRow, startupLayout] = CreateSettingsRow(
        page, QStringLiteral("启动后"),
        QStringLiteral("选择显示主窗口，或仅在系统托盘中运行。"));
    controls_.startupVisibilitySelector = new RemoteCComboBox(startupRow);
    controls_.startupVisibilitySelector->setObjectName(
        QStringLiteral("capacitySelector"));
    controls_.startupVisibilitySelector->addItem(
        QStringLiteral("显示主窗口"), QStringLiteral("window"));
    controls_.startupVisibilitySelector->addItem(
        QStringLiteral("在托盘中运行"), QStringLiteral("tray"));
    controls_.startupVisibilitySelector->setCurrentIndex(std::max(
        0, controls_.startupVisibilitySelector->findData(
               currentSettings.value(
                   QString::fromLatin1(kStartupVisibilitySetting),
                   QStringLiteral("window")).toString())));
    controls_.startupVisibilitySelector->setFixedWidth(kSettingsControlWidth);
    startupLayout->addWidget(
        controls_.startupVisibilitySelector, 0, Qt::AlignVCenter);
    layout->addWidget(startupRow);

    const auto [closeBehaviorRow, closeBehaviorLayout] = CreateSettingsRow(
        page, QStringLiteral("关闭主窗口"),
        QStringLiteral("选择隐藏到托盘，或直接退出 RLink。"));
    controls_.closeButtonBehaviorSelector = new RemoteCComboBox(closeBehaviorRow);
    controls_.closeButtonBehaviorSelector->setObjectName(
        QStringLiteral("capacitySelector"));
    controls_.closeButtonBehaviorSelector->addItem(
        QStringLiteral("隐藏到托盘（推荐）"), QStringLiteral("tray"));
    controls_.closeButtonBehaviorSelector->addItem(
        QStringLiteral("退出程序"), QStringLiteral("exit"));
    controls_.closeButtonBehaviorSelector->setCurrentIndex(std::max(
        0, controls_.closeButtonBehaviorSelector->findData(
               currentSettings.value(
                   QString::fromLatin1(kCloseButtonBehaviorSetting),
                   QStringLiteral("tray")).toString())));
    controls_.closeButtonBehaviorSelector->setFixedWidth(kSettingsControlWidth);
    closeBehaviorLayout->addWidget(
        controls_.closeButtonBehaviorSelector, 0, Qt::AlignVCenter);
    layout->addWidget(closeBehaviorRow);

    const auto [defaultCapacityRow, defaultCapacityLayout] = CreateSettingsRow(
        page, QStringLiteral("默认房间人数"),
        QStringLiteral("设置创建新房间时默认选择的人数上限。"));
    controls_.defaultRoomCapacitySelector =
        new RemoteCComboBox(defaultCapacityRow);
    controls_.defaultRoomCapacitySelector->setObjectName(
        QStringLiteral("capacitySelector"));
    AddRoomCapacityItems(controls_.defaultRoomCapacitySelector);
    controls_.defaultRoomCapacitySelector->setFixedWidth(kSettingsControlWidth);
    defaultCapacityLayout->addWidget(
        controls_.defaultRoomCapacitySelector, 0, Qt::AlignVCenter);
    layout->addWidget(defaultCapacityRow);

    auto* configRow = new QFrame(page);
    configRow->setObjectName(QStringLiteral("settingRow"));
    auto* configLayout = new QVBoxLayout(configRow);
    configLayout->setContentsMargins(20, 17, 20, 17);
    configLayout->setSpacing(5);
    auto* configTitle = new QLabel(QStringLiteral("配置文件"), configRow);
    configTitle->setObjectName(QStringLiteral("settingTitle"));
    configLayout->addWidget(configTitle);
    auto* configPath = new QLabel(currentSettings.fileName(), configRow);
    configPath->setProperty("muted", true);
    configPath->setWordWrap(true);
    configPath->setTextInteractionFlags(Qt::TextSelectableByMouse);
    configLayout->addWidget(configPath);
    layout->addWidget(configRow);
    layout->addStretch(1);

    const QString installedVersion =
        QCoreApplication::applicationVersion().trimmed();
    if (!installedVersion.isEmpty()) {
        auto* aboutCard = new QFrame(page);
        aboutCard->setObjectName(QStringLiteral("aboutCard"));
        auto* aboutLayout = new QHBoxLayout(aboutCard);
        aboutLayout->setContentsMargins(20, 16, 20, 16);
        aboutLayout->setSpacing(14);

        auto* aboutLogo = new QLabel(aboutCard);
        aboutLogo->setObjectName(QStringLiteral("aboutLogo"));
        aboutLogo->setFixedSize(48, 48);
        aboutLogo->setAlignment(Qt::AlignCenter);
        aboutLogo->setPixmap(CreateRemoteCIcon().pixmap(44, 44));
        aboutLayout->addWidget(aboutLogo, 0, Qt::AlignVCenter);

        auto* aboutTextHost = new QWidget(aboutCard);
        auto* aboutTextLayout = new QVBoxLayout(aboutTextHost);
        aboutTextLayout->setContentsMargins(0, 0, 0, 0);
        aboutTextLayout->setSpacing(4);
        auto* aboutTitle = new QLabel(
            QStringLiteral("关于 RLink"), aboutTextHost);
        aboutTitle->setObjectName(QStringLiteral("aboutTitle"));
        aboutTextLayout->addWidget(aboutTitle);
        controls_.softwareUpdateStatusLabel = new QLabel(
            QStringLiteral("当前安装版本由安装程序统一管理"), aboutTextHost);
        controls_.softwareUpdateStatusLabel->setProperty("muted", true);
        aboutTextLayout->addWidget(controls_.softwareUpdateStatusLabel);
        aboutLayout->addWidget(aboutTextHost, 1, Qt::AlignVCenter);

        auto* versionLabel = new QLabel(
            QStringLiteral("v%1").arg(installedVersion), aboutCard);
        versionLabel->setObjectName(QStringLiteral("aboutVersion"));
        versionLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        aboutLayout->addWidget(versionLabel, 0, Qt::AlignVCenter);

        auto* copyVersionButton = new QPushButton(
            QStringLiteral("复制版本"), aboutCard);
        copyVersionButton->setObjectName(QStringLiteral("softButton"));
        copyVersionButton->setCursor(Qt::PointingHandCursor);
        aboutLayout->addWidget(copyVersionButton, 0, Qt::AlignVCenter);
        controls_.softwareUpdateCheckButton = new QPushButton(
            QStringLiteral("检查更新"), aboutCard);
        controls_.softwareUpdateCheckButton->setObjectName(
            QStringLiteral("softButton"));
        controls_.softwareUpdateCheckButton->setCursor(Qt::PointingHandCursor);
        aboutLayout->addWidget(
            controls_.softwareUpdateCheckButton, 0, Qt::AlignVCenter);
        connect(controls_.softwareUpdateCheckButton, &QPushButton::clicked,
                this, &SettingsPage::SoftwareUpdateRequested);
        connect(copyVersionButton, &QPushButton::clicked,
                this, [this, installedVersion] {
                    QApplication::clipboard()->setText(
                        QStringLiteral("RLink v%1").arg(installedVersion));
                    RemoteCToast::Show(
                        this, QStringLiteral("版本号已复制"),
                        RemoteCToast::Tone::kSuccess);
                });
        layout->addWidget(aboutCard);
    }
    detailStack_->addWidget(page);
}

QStackedWidget* SettingsPage::DetailStack() const
{
    return detailStack_;
}

SettingsPageControls& SettingsPage::Controls()
{
    return controls_;
}

const SettingsPageControls& SettingsPage::Controls() const
{
    return controls_;
}

int SettingsPage::CurrentCategory() const
{
    return detailStack_ ? detailStack_->currentIndex() : -1;
}

}  // namespace remote::controller
