// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "SettingsPage.h"
#include "SettingsPageUi.h"

#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

namespace remote::controller {

void SettingsPage::BuildShortcutSettingsPage()
{
    auto* page = new QWidget(detailStack_);
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    AddSettingsDetailHeader(
        layout, page, QStringLiteral("快捷键"),
        QStringLiteral("查看 RLink 当前可用的本地快捷操作。"));
    const auto addShortcutRow =
        [page, layout](const QString& title,
                       const QString& description,
                       const QString& keys) {
            const auto [row, rowLayout] =
                CreateSettingsRow(page, title, description);
            auto* badge = new QLabel(keys, row);
            badge->setObjectName(QStringLiteral("aboutVersion"));
            badge->setAlignment(Qt::AlignCenter);
            badge->setFixedWidth(132);
            badge->setTextInteractionFlags(Qt::TextSelectableByMouse);
            rowLayout->addWidget(
                badge, 0, Qt::AlignRight | Qt::AlignVCenter);
            layout->addWidget(row);
        };
    addShortcutRow(
        QStringLiteral("切换全屏"),
        QStringLiteral("在监控窗口中进入或退出全屏显示。"),
        QStringLiteral("F11"));
    addShortcutRow(
        QStringLiteral("平移原始像素画面"),
        QStringLiteral("仅在 100% 原始像素模式下生效。"),
        QStringLiteral("Alt + 左键拖动"));
    addShortcutRow(
        QStringLiteral("远程粘贴"),
        QStringLiteral("控制远端时粘贴本机剪贴板中的内容。"),
        QStringLiteral("Ctrl + V"));
    addShortcutRow(
        QStringLiteral("远程复制"),
        QStringLiteral("控制远端时将复制命令发送给被控端。"),
        QStringLiteral("Ctrl + C"));
    layout->addStretch(1);
    detailStack_->addWidget(page);
}

}  // namespace remote::controller
