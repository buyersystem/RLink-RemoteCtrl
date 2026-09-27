// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "CurrentPageStack.h"

#include <QLayout>
#include <QResizeEvent>
#include <QTimer>
#include <QWidget>

#include <algorithm>

namespace remote::controller::detail {

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

void CurrentPageStack::resizeEvent(QResizeEvent* event)
{
    const bool widthChanged =
        event->oldSize().width() != event->size().width();
    QStackedWidget::resizeEvent(event);
    if (widthChanged) {
        QTimer::singleShot(0, this, [this] { RefreshCurrentHeight(); });
    }
}

}  // namespace remote::controller::detail
