// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "SettingsPage.h"
#include "SettingsPageUi.h"

#include <algorithm>
#include <utility>

#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QTemporaryFile>
#include <QUrl>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

#include "src/apps/controller/ControllerMainWindowSupport.h"
#include "src/apps/controller/RemoteCComboBox.h"
#include "src/apps/controller/RemoteCDialog.h"
#include "src/apps/controller/RemoteCToast.h"

namespace remote::controller {
using namespace detail;

void SettingsPage::BuildRemotePasteSettingsPage()
{
    const QSettings currentSettings;
    auto* page = new QWidget(detailStack_);
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    AddSettingsDetailHeader(
        layout, page, QStringLiteral("远程粘贴"),
        QStringLiteral("管理远程粘贴内容、传输限制和临时缓存。"));

    const auto [enabledRow, enabledLayout] = CreateSettingsRow(
        page, QStringLiteral("远程粘贴"),
        QStringLiteral("仅在指定粘贴动作发生时传输，不会持续同步剪贴板正文。"));
    controls_.remotePasteEnabledSelector = new RemoteCComboBox(enabledRow);
    controls_.remotePasteEnabledSelector->setObjectName(
        QStringLiteral("capacitySelector"));
    controls_.remotePasteEnabledSelector->addItem(QStringLiteral("开启"), true);
    controls_.remotePasteEnabledSelector->addItem(QStringLiteral("关闭"), false);
    controls_.remotePasteEnabledSelector->setCurrentIndex(std::max(
        0, controls_.remotePasteEnabledSelector->findData(
               currentSettings.value(
                   QString::fromLatin1(kRemotePasteEnabledSetting),
                   true).toBool())));
    controls_.remotePasteEnabledSelector->setFixedWidth(kSettingsControlWidth);
    enabledLayout->addWidget(
        controls_.remotePasteEnabledSelector, 0, Qt::AlignVCenter);
    layout->addWidget(enabledRow);

    const auto [formatsRow, formatsLayout] = CreateSettingsRow(
        page, QStringLiteral("远程粘贴内容"),
        QStringLiteral("选择远程粘贴允许传输的内容格式。"));
    controls_.clipboardFormatsSelector = new RemoteCComboBox(formatsRow);
    controls_.clipboardFormatsSelector->setObjectName(
        QStringLiteral("capacitySelector"));
    controls_.clipboardFormatsSelector->addItem(
        QStringLiteral("文本、富文本、图片和文件"), QStringLiteral("all"));
    controls_.clipboardFormatsSelector->addItem(
        QStringLiteral("文本、富文本和图片"), QStringLiteral("no_files"));
    controls_.clipboardFormatsSelector->addItem(
        QStringLiteral("文本和富文本"), QStringLiteral("rich_text"));
    controls_.clipboardFormatsSelector->addItem(
        QStringLiteral("仅纯文本"), QStringLiteral("text"));
    controls_.clipboardFormatsSelector->setCurrentIndex(std::max(
        0, controls_.clipboardFormatsSelector->findData(
               currentSettings.value(
                   QString::fromLatin1(kClipboardFormatsSetting),
                   QStringLiteral("all")).toString())));
    controls_.clipboardFormatsSelector->setFixedWidth(kSettingsControlWidth);
    formatsLayout->addWidget(
        controls_.clipboardFormatsSelector, 0, Qt::AlignVCenter);
    layout->addWidget(formatsRow);

    const auto [fileLimitRow, fileLimitLayout] = CreateSettingsRow(
        page, QStringLiteral("单次文件粘贴上限"),
        QStringLiteral("超过上限的文件或目录会被拒绝，避免误传大目录。"));
    controls_.clipboardLargeFileLimitSelector =
        new RemoteCComboBox(fileLimitRow);
    controls_.clipboardLargeFileLimitSelector->setObjectName(
        QStringLiteral("capacitySelector"));
    for (const int mebibytes : {25, 100, 512}) {
        controls_.clipboardLargeFileLimitSelector->addItem(
            QStringLiteral("%1 MiB").arg(mebibytes), mebibytes);
    }
    const int configuredFileLimitMiB = std::min(
        currentSettings.value(
            QString::fromLatin1(kClipboardFileLimitSetting), 100).toInt(),
        512);
    controls_.clipboardLargeFileLimitSelector->setCurrentIndex(std::max(
        0, controls_.clipboardLargeFileLimitSelector->findData(
               configuredFileLimitMiB)));
    controls_.clipboardLargeFileLimitSelector->setFixedWidth(
        kSettingsControlWidth);
    fileLimitLayout->addWidget(
        controls_.clipboardLargeFileLimitSelector, 0, Qt::AlignVCenter);
    layout->addWidget(fileLimitRow);

    auto* cacheSection = new QLabel(QStringLiteral("远程粘贴缓存"), page);
    cacheSection->setObjectName(QStringLiteral("settingsDetailTitle"));
    layout->addSpacing(6);
    layout->addWidget(cacheSection);

    const QString configuredCacheBase = QDir::cleanPath(
        currentSettings.value(
            QString::fromLatin1(kClipboardCacheBaseDirectorySetting),
            InitialClipboardCacheBaseDirectory()).toString());
    const auto [cachePathRow, cachePathLayout] = CreatePathSettingsRow(
        page, QStringLiteral("缓存存储位置"),
        QStringLiteral("选择存储磁盘；RLink 会在其中创建专用缓存目录。"));
    controls_.clipboardCachePathLabel = new QLabel(cachePathRow);
    controls_.clipboardCachePathLabel->setProperty("muted", true);
    controls_.clipboardCachePathLabel->setWordWrap(true);
    controls_.clipboardCachePathLabel->setAlignment(
        Qt::AlignLeft | Qt::AlignVCenter);
    controls_.clipboardCachePathLabel->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Preferred);
    controls_.clipboardCachePathLabel->setTextInteractionFlags(
        Qt::TextSelectableByMouse);
    controls_.clipboardCachePathLabel->setText(QDir::toNativeSeparators(
        ClipboardCacheRootForBase(configuredCacheBase)));
    controls_.clipboardCachePathLabel->setToolTip(
        controls_.clipboardCachePathLabel->text());
    auto* pathButtons = new QHBoxLayout();
    pathButtons->setSpacing(8);
    auto* resetDirectory = new QPushButton(
        QStringLiteral("恢复默认"), cachePathRow);
    resetDirectory->setObjectName(QStringLiteral("softButton"));
    resetDirectory->setCursor(Qt::PointingHandCursor);
    auto* chooseDirectory = new QPushButton(
        QStringLiteral("选择路径"), cachePathRow);
    chooseDirectory->setObjectName(QStringLiteral("softButton"));
    chooseDirectory->setCursor(Qt::PointingHandCursor);
    auto* openDirectory = new QPushButton(
        QStringLiteral("打开缓存路径"), cachePathRow);
    openDirectory->setObjectName(QStringLiteral("softButton"));
    openDirectory->setCursor(Qt::PointingHandCursor);
    pathButtons->addWidget(resetDirectory);
    pathButtons->addWidget(chooseDirectory);
    pathButtons->addWidget(openDirectory);
    cachePathLayout->addWidget(controls_.clipboardCachePathLabel, 1);
    cachePathLayout->addLayout(pathButtons);
    layout->addWidget(cachePathRow);

    const auto [retentionRow, retentionLayout] = CreateSettingsRow(
        page, QStringLiteral("缓存保留时间"),
        QStringLiteral("当前剪贴板引用和正在传输的文件不受保留时间影响。"));
    controls_.clipboardCacheRetentionSelector =
        new RemoteCComboBox(retentionRow);
    controls_.clipboardCacheRetentionSelector->setObjectName(
        QStringLiteral("capacitySelector"));
    for (const auto [minutes, label] : {
            std::pair{30, QStringLiteral("30 分钟")},
            std::pair{60, QStringLiteral("1 小时（推荐）")},
            std::pair{360, QStringLiteral("6 小时")},
            std::pair{1440, QStringLiteral("24 小时")} }) {
        controls_.clipboardCacheRetentionSelector->addItem(label, minutes);
    }
    controls_.clipboardCacheRetentionSelector->setCurrentIndex(std::max(
        0, controls_.clipboardCacheRetentionSelector->findData(
               currentSettings.value(
                   QString::fromLatin1(kClipboardCacheRetentionSetting),
                   60).toInt())));
    controls_.clipboardCacheRetentionSelector->setFixedWidth(
        kSettingsControlWidth);
    retentionLayout->addWidget(
        controls_.clipboardCacheRetentionSelector, 0, Qt::AlignVCenter);
    layout->addWidget(retentionRow);

    const auto [capacityRow, capacityLayout] = CreateSettingsRow(
        page, QStringLiteral("缓存空间上限"),
        QStringLiteral("最大不超过所选磁盘可用空间的 50%，每次接收前都会复查。"));
    controls_.clipboardCacheCapacitySelector =
        new RemoteCComboBox(capacityRow);
    controls_.clipboardCacheCapacitySelector->setObjectName(
        QStringLiteral("capacitySelector"));
    controls_.clipboardCacheCapacitySelector->setFixedWidth(
        kSettingsControlWidth);
    RefreshClipboardCacheCapacityOptions();
    capacityLayout->addWidget(
        controls_.clipboardCacheCapacitySelector, 0, Qt::AlignVCenter);
    layout->addWidget(capacityRow);

    const auto [usageRow, usageLayout] = CreateSettingsRow(
        page, QStringLiteral("当前缓存"),
        QStringLiteral("清理全部非活动项目；当前剪贴板和正在传输的文件会继续保留。"));
    controls_.clipboardCacheUsageLabel = new QLabel(
        QStringLiteral("正在统计…"), usageRow);
    controls_.clipboardCacheUsageLabel->setProperty("muted", true);
    controls_.clearClipboardCacheButton = new QPushButton(
        QStringLiteral("立即清理"), usageRow);
    controls_.clearClipboardCacheButton->setObjectName(
        QStringLiteral("softButton"));
    controls_.clearClipboardCacheButton->setCursor(Qt::PointingHandCursor);
    controls_.clearClipboardCacheButton->setEnabled(false);
    auto* usageHost = new QWidget(usageRow);
    usageHost->setFixedWidth(kSettingsControlWidth);
    auto* usageControls = new QVBoxLayout(usageHost);
    usageControls->setContentsMargins(0, 0, 0, 0);
    usageControls->setSpacing(8);
    usageControls->addWidget(
        controls_.clipboardCacheUsageLabel, 0, Qt::AlignLeft);
    usageControls->addWidget(
        controls_.clearClipboardCacheButton, 0, Qt::AlignRight);
    usageLayout->addWidget(usageHost, 0, Qt::AlignVCenter);
    layout->addWidget(usageRow);
    layout->addStretch(1);

    auto* scroll = new QScrollArea(detailStack_);
    scroll->setObjectName(QStringLiteral("settingsDetailScroll"));
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setWidget(page);
    detailStack_->addWidget(scroll);

    connect(chooseDirectory, &QPushButton::clicked, this, [this] {
        QSettings settings;
        const QString currentDirectory = settings.value(
            QString::fromLatin1(kClipboardCacheBaseDirectorySetting),
            InitialClipboardCacheBaseDirectory()).toString();
        const QString selected = QFileDialog::getExistingDirectory(
            this, QStringLiteral("选择远程粘贴缓存位置"), currentDirectory);
        if (!selected.isEmpty()) {
            ApplyClipboardCacheBaseDirectory(selected);
        }
    });
    connect(resetDirectory, &QPushButton::clicked, this, [this] {
        ApplyClipboardCacheBaseDirectory(
            InitialClipboardCacheBaseDirectory());
    });
    connect(openDirectory, &QPushButton::clicked, this, [this] {
        QSettings settings;
        const QString base = settings.value(
            QString::fromLatin1(kClipboardCacheBaseDirectorySetting),
            InitialClipboardCacheBaseDirectory()).toString();
        const QString root = ClipboardCacheRootForBase(base);
        if (!QDir().mkpath(root) ||
            !QDesktopServices::openUrl(QUrl::fromLocalFile(root))) {
            RemoteCDialog::Alert(
                this, QStringLiteral("无法打开缓存路径"),
                QStringLiteral("RLink 无法创建或打开当前缓存目录。"));
        }
    });
}

void SettingsPage::RefreshClipboardCacheCapacityOptions()
{
    if (!controls_.clipboardCacheCapacitySelector) {
        return;
    }
    QSettings settings;
    const QString baseDirectory = QDir::cleanPath(settings.value(
        QString::fromLatin1(kClipboardCacheBaseDirectorySetting),
        InitialClipboardCacheBaseDirectory()).toString());
    const qulonglong preferredGiB =
        controls_.clipboardCacheCapacitySelector->currentData().isValid()
        ? controls_.clipboardCacheCapacitySelector->currentData().toULongLong()
        : settings.value(
              QString::fromLatin1(kClipboardCacheCapacitySetting),
              2).toULongLong();
    QSignalBlocker blocker(controls_.clipboardCacheCapacitySelector);
    controls_.clipboardCacheCapacitySelector->clear();
    const qulonglong safeGiB = SafeClipboardCacheCapacityGiB(baseDirectory);
    for (const qulonglong capacity : {1ull, 2ull, 5ull, 10ull}) {
        if (capacity > safeGiB) {
            continue;
        }
        const bool recommended = capacity == std::min(2ull, safeGiB);
        controls_.clipboardCacheCapacitySelector->addItem(
            recommended
                ? QStringLiteral("%1 GiB（推荐）").arg(capacity)
                : QStringLiteral("%1 GiB").arg(capacity),
            QVariant::fromValue(capacity));
    }
    if (safeGiB > 10) {
        controls_.clipboardCacheCapacitySelector->addItem(
            QStringLiteral("不限制（安全上限 %1 GiB）").arg(safeGiB),
            QVariant::fromValue(safeGiB));
    }
    if (controls_.clipboardCacheCapacitySelector->count() == 0) {
        controls_.clipboardCacheCapacitySelector->addItem(
            QStringLiteral("空间不足（至少需 2 GiB 可用）"),
            QVariant::fromValue(0ull));
        controls_.clipboardCacheCapacitySelector->setEnabled(false);
        return;
    }
    controls_.clipboardCacheCapacitySelector->setEnabled(true);
    int index = controls_.clipboardCacheCapacitySelector->findData(
        QVariant::fromValue(preferredGiB));
    if (index < 0) {
        index = controls_.clipboardCacheCapacitySelector->count() - 1;
    }
    controls_.clipboardCacheCapacitySelector->setCurrentIndex(index);
}

void SettingsPage::ApplyClipboardCacheBaseDirectory(
    const QString& selectedDirectory)
{
    const QString normalized = QDir::cleanPath(selectedDirectory);
    QTemporaryFile writeProbe(QDir(normalized).filePath(
        QStringLiteral(".remotec-write-test-XXXXXX")));
    if (!writeProbe.open()) {
        RemoteCDialog::Alert(
            this, QStringLiteral("缓存路径不可写"),
            QStringLiteral("RLink 无法在所选位置创建缓存文件，请选择当前用户具有写入权限的目录。"));
        return;
    }
    writeProbe.close();
    if (SafeClipboardCacheCapacityGiB(normalized) < 1) {
        RemoteCDialog::Alert(
            this, QStringLiteral("缓存路径空间不足"),
            QStringLiteral("所选磁盘的可用空间不足 2 GiB，无法设置至少 1 GiB 的安全缓存上限。"));
        return;
    }
    QSettings settings;
    settings.setValue(
        QString::fromLatin1(kClipboardCacheBaseDirectorySetting), normalized);
    controls_.clipboardCachePathLabel->setText(QDir::toNativeSeparators(
        ClipboardCacheRootForBase(normalized)));
    controls_.clipboardCachePathLabel->setToolTip(
        controls_.clipboardCachePathLabel->text());
    RefreshClipboardCacheCapacityOptions();
    emit ClipboardCacheBaseDirectoryChanged();
    RemoteCToast::Show(
        this, QStringLiteral("远程粘贴缓存路径已更新"),
        RemoteCToast::Tone::kSuccess);
}

}  // namespace remote::controller
