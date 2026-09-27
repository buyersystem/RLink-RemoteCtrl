// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "DiagnosticsPage.h"

#include <QLabel>
#include <QFrame>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "src/apps/controller/FramelessWindow.h"

namespace remote::controller {

DiagnosticsPage::DiagnosticsPage(QWidget* parent)
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
    auto* title = new QLabel(QStringLiteral("调试信息"), content);
    title->setObjectName(QStringLiteral("pageTitle"));
    auto* subtitle = new QLabel(
        QStringLiteral("按分类查看运行状态和媒体能力；反馈问题时可以一键复制。"),
        content);
    subtitle->setObjectName(QStringLiteral("pageSubtitle"));
    subtitle->setWordWrap(true);
    header->addWidget(title);
    header->addWidget(subtitle);
    contentLayout_->addLayout(header);
    setWidget(content);

    BuildWorkspace();
}

QVBoxLayout* DiagnosticsPage::AddDetailPage(
    QStackedWidget* stack,
    const QString& title,
    const QString& description,
    QPushButton** copyButton)
{
    auto* page = new QWidget(stack);
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    auto* titleRow = new QHBoxLayout();
    titleRow->setContentsMargins(0, 0, 0, 0);
    auto* titleLabel = new QLabel(title, page);
    titleLabel->setObjectName(QStringLiteral("settingsDetailTitle"));
    titleRow->addWidget(titleLabel);
    titleRow->addStretch(1);
    if (copyButton) {
        *copyButton = new QPushButton(QStringLiteral("复制"), page);
        (*copyButton)->setObjectName(QStringLiteral("softButton"));
        (*copyButton)->setCursor(Qt::PointingHandCursor);
        (*copyButton)->setToolTip(QStringLiteral("复制当前媒体能力页面信息"));
        titleRow->addWidget(*copyButton);
    }
    layout->addLayout(titleRow);
    auto* descriptionLabel = new QLabel(description, page);
    descriptionLabel->setProperty("muted", true);
    descriptionLabel->setWordWrap(true);
    layout->addWidget(descriptionLabel);
    layout->addSpacing(8);
    stack->addWidget(page);
    return layout;
}

void DiagnosticsPage::AddValue(QVBoxLayout* layout,
                               const QString& key,
                               const QString& title,
                               bool expanded)
{
    auto* page = layout->parentWidget();
    auto* row = new QFrame(page);
    row->setObjectName(QStringLiteral("settingRow"));
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(20, 17, 20, 17);
    rowLayout->setSpacing(18);
    auto* titleLabel = new QLabel(title, row);
    titleLabel->setObjectName(QStringLiteral("settingTitle"));
    titleLabel->setFixedWidth(expanded ? 140 : 170);
    titleLabel->setAlignment(expanded ? Qt::AlignTop : Qt::AlignVCenter);
    auto* valueLabel = new QLabel(QStringLiteral("正在获取…"), row);
    valueLabel->setObjectName(QStringLiteral("debugValue"));
    valueLabel->setProperty("tone", "muted");
    valueLabel->setWordWrap(expanded);
    valueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    valueLabel->setAlignment(expanded ? Qt::AlignTop : Qt::AlignVCenter);
    rowLayout->addWidget(titleLabel);
    rowLayout->addWidget(valueLabel, 1);
    layout->addWidget(row);
    RegisterValue(key, valueLabel);
}

void DiagnosticsPage::RegisterValue(const QString& key, QLabel* label)
{
    valueLabels_.insert(key, label);
}

QLabel* DiagnosticsPage::ValueLabel(const QString& key) const
{
    return valueLabels_.value(key, nullptr);
}

bool DiagnosticsPage::HasValues() const
{
    return !valueLabels_.isEmpty();
}

bool DiagnosticsPage::ScreenFrameRateLogEnabled() const
{
    return screenFrameRateLogEnabled_;
}

bool DiagnosticsPage::InputEventStatsEnabled() const
{
    return inputEventStatsEnabled_;
}

QWidget* DiagnosticsPage::StatsCardsWidget() const
{
    return statsCardsWidget_;
}

QPushButton* DiagnosticsPage::CopyAllButton() const
{
    return copyAllButton_;
}

QPushButton* DiagnosticsPage::CopyMediaButton() const
{
    return copyMediaButton_;
}

}  // namespace remote::controller
