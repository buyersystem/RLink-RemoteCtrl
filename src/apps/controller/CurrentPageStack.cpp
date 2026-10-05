// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "CurrentPageStack.h"

#include <QLayout>
#include <QResizeEvent>
#include <QTimer>
#include <QWidget>

#include <algorithm>

namespace remote::controller::detail {

CurrentPageStack::CurrentPageStack(QWidget* parent, bool trackFixedHeight)
    : QStackedWidget(parent), trackFixedHeight_(trackFixedHeight)
{
    if (!trackFixedHeight_) {
        layout()->setSizeConstraint(QLayout::SetNoConstraint);
    }
    connect(this, &QStackedWidget::currentChanged, this, [this] {
        updateGeometry();
    });
}

int CurrentPageStackItem::heightForWidth(int width) const
{
    const int height = wid->heightForWidth(width);
    return height < 0 ? height : std::clamp(height, wid->minimumHeight(), wid->maximumHeight());
}

int CurrentPageStackItem::minimumHeightForWidth(int width) const
{
    return heightForWidth(width);
}

int CurrentPageStack::PageHeightForWidth(QWidget* page) const
{
    if (!page) {
        return 0;
    }
    page->ensurePolished();
    if (page->layout()) {
        page->layout()->activate();
    }
    int height = page->sizeHint().height();
    if (page->layout() && page->layout()->hasHeightForWidth() &&
        width() > 0) {
        const int heightForWidth = page->layout()->heightForWidth(width());
        if (heightForWidth > 0) {
            height = heightForWidth;
        }
    }
    return std::max(height, page->minimumSizeHint().height());
}

void CurrentPageStack::RefreshCurrentHeight()
{
    QWidget* page = currentWidget();
    if (!page) {
        return;
    }
    const int height = PageHeightForWidth(page);
    if (height > 0 && this->height() != height) {
        setFixedHeight(height);
        updateGeometry();
    }
}

QSize CurrentPageStack::sizeHint() const
{
    return currentWidget() ? currentWidget()->sizeHint()
                           : QStackedWidget::sizeHint();
}

QSize CurrentPageStack::minimumSizeHint() const
{
    return currentWidget() ? currentWidget()->minimumSizeHint()
                           : QStackedWidget::minimumSizeHint();
}

bool CurrentPageStack::hasHeightForWidth() const
{
    return currentWidget() && currentWidget()->hasHeightForWidth();
}

int CurrentPageStack::heightForWidth(int width) const
{
    // QStackedLayout measures every page, including hidden long-text tabs.
    // Only the visible page determines the scroll viewport's height.
    return currentWidget() ? currentWidget()->heightForWidth(width) : -1;
}

void CurrentPageStack::resizeEvent(QResizeEvent* event)
{
    const bool widthChanged =
        event->oldSize().width() != event->size().width();
    QStackedWidget::resizeEvent(event);
    if (widthChanged && trackFixedHeight_ && !heightRefreshPending_) {
        heightRefreshPending_ = true;
        QTimer::singleShot(0, this, [this] {
            heightRefreshPending_ = false;
            RefreshCurrentHeight();
        });
    }
}

}  // namespace remote::controller::detail
