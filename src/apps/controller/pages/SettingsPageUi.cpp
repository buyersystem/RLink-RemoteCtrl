// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "SettingsPageUi.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

namespace remote::controller {

void AddSettingsDetailHeader(QVBoxLayout* layout,
                             QWidget* parent,
                             const QString& title,
                             const QString& description)
{
    auto* titleLabel = new QLabel(title, parent);
    titleLabel->setObjectName(QStringLiteral("settingsDetailTitle"));
    layout->addWidget(titleLabel);
    auto* descriptionLabel = new QLabel(description, parent);
    descriptionLabel->setProperty("muted", true);
    descriptionLabel->setWordWrap(true);
    layout->addWidget(descriptionLabel);
    layout->addSpacing(8);
}

std::pair<QFrame*, QHBoxLayout*> CreateSettingsRow(
    QWidget* parent,
    const QString& title,
    const QString& description)
{
    auto* row = new QFrame(parent);
    row->setObjectName(QStringLiteral("settingRow"));
    row->setFixedHeight(108);
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(20, 17, 20, 17);
    rowLayout->setSpacing(18);
    auto* textHost = new QWidget(row);
    textHost->setFixedWidth(390);
    auto* textLayout = new QVBoxLayout(textHost);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(3);
    auto* titleLabel = new QLabel(title, row);
    titleLabel->setObjectName(QStringLiteral("settingTitle"));
    auto* hintLabel = new QLabel(description, row);
    hintLabel->setProperty("muted", true);
    hintLabel->setWordWrap(false);
    hintLabel->setToolTip(description);
    textLayout->addWidget(titleLabel);
    textLayout->addWidget(hintLabel);
    rowLayout->addWidget(textHost, 0, Qt::AlignVCenter);
    return {row, rowLayout};
}

std::pair<QFrame*, QHBoxLayout*> CreatePathSettingsRow(
    QWidget* parent,
    const QString& title,
    const QString& description)
{
    auto* row = new QFrame(parent);
    row->setObjectName(QStringLiteral("settingRow"));
    row->setFixedHeight(132);
    auto* rowLayout = new QVBoxLayout(row);
    rowLayout->setContentsMargins(20, 15, 20, 15);
    rowLayout->setSpacing(9);
    auto* textHost = new QWidget(row);
    auto* textLayout = new QVBoxLayout(textHost);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(3);
    auto* titleLabel = new QLabel(title, textHost);
    titleLabel->setObjectName(QStringLiteral("settingTitle"));
    auto* hintLabel = new QLabel(description, textHost);
    hintLabel->setProperty("muted", true);
    hintLabel->setWordWrap(true);
    textLayout->addWidget(titleLabel);
    textLayout->addWidget(hintLabel);
    rowLayout->addWidget(textHost);
    auto* pathLayout = new QHBoxLayout();
    pathLayout->setContentsMargins(0, 0, 0, 0);
    pathLayout->setSpacing(10);
    rowLayout->addLayout(pathLayout);
    return {row, pathLayout};
}

}  // namespace remote::controller
