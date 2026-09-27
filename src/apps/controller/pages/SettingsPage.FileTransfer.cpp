// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "SettingsPage.h"
#include "SettingsPageUi.h"

#include <QDir>
#include <QFileDialog>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "src/apps/controller/ControllerMainWindowSupport.h"
#include "src/apps/controller/RemoteCToast.h"

namespace remote::controller {
using namespace detail;

void SettingsPage::BuildFileTransferSettingsPage()
{
    const QSettings currentSettings;
    auto* page = new QWidget(detailStack_);
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    AddSettingsDetailHeader(
        layout, page, QStringLiteral("文件传输"),
        QStringLiteral("管理接收文件时使用的默认存储位置。"));
    const auto [saveDirectoryRow, saveDirectoryPathLayout] =
        CreatePathSettingsRow(
            page, QStringLiteral("文件默认存储路径"),
            QStringLiteral("点击“接受”时直接保存到此目录。"));
    auto* saveDirectoryPath = new QLabel(saveDirectoryRow);
    saveDirectoryPath->setProperty("muted", true);
    saveDirectoryPath->setWordWrap(true);
    saveDirectoryPath->setTextInteractionFlags(Qt::TextSelectableByMouse);
    const QString configuredSaveDirectory = QDir::cleanPath(
        currentSettings.value(
            QString::fromLatin1(kDefaultFileSaveDirectorySetting),
            InitialFileSaveDirectory()).toString());
    saveDirectoryPath->setText(
        QDir::toNativeSeparators(configuredSaveDirectory));
    saveDirectoryPath->setToolTip(saveDirectoryPath->text());
    saveDirectoryPath->setMinimumWidth(0);
    saveDirectoryPath->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto* chooseSaveDirectory = new QPushButton(
        QStringLiteral("选择路径"), saveDirectoryRow);
    chooseSaveDirectory->setObjectName(QStringLiteral("softButton"));
    chooseSaveDirectory->setCursor(Qt::PointingHandCursor);
    saveDirectoryPathLayout->addWidget(saveDirectoryPath, 1);
    saveDirectoryPathLayout->addWidget(chooseSaveDirectory);
    layout->addWidget(saveDirectoryRow);
    layout->addStretch(1);

    auto* scroll = new QScrollArea(detailStack_);
    scroll->setObjectName(QStringLiteral("settingsDetailScroll"));
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setWidget(page);
    detailStack_->addWidget(scroll);

    connect(chooseSaveDirectory, &QPushButton::clicked,
            this, [this, saveDirectoryPath] {
                QSettings settings;
                const QString currentDirectory = settings.value(
                    QString::fromLatin1(kDefaultFileSaveDirectorySetting),
                    InitialFileSaveDirectory()).toString();
                const QString selectedDirectory =
                    QFileDialog::getExistingDirectory(
                        this, QStringLiteral("选择文件默认存储路径"),
                        currentDirectory);
                if (selectedDirectory.isEmpty()) {
                    return;
                }
                const QString normalizedDirectory =
                    QDir::cleanPath(selectedDirectory);
                settings.setValue(
                    QString::fromLatin1(kDefaultFileSaveDirectorySetting),
                    normalizedDirectory);
                saveDirectoryPath->setText(
                    QDir::toNativeSeparators(normalizedDirectory));
                saveDirectoryPath->setToolTip(saveDirectoryPath->text());
                RemoteCToast::Show(
                    this, QStringLiteral("默认存储路径已更新"),
                    RemoteCToast::Tone::kSuccess);
            });
}

}  // namespace remote::controller
