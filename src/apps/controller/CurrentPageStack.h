// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QStackedWidget>
#include <QLayoutItem>

namespace remote::controller::detail {

class CurrentPageStack final : public QStackedWidget {
public:
    explicit CurrentPageStack(QWidget* parent = nullptr,
                              bool trackFixedHeight = true);

    int PageHeightForWidth(QWidget* page) const;
    void RefreshCurrentHeight();
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    bool trackFixedHeight_ = true;
    bool heightRefreshPending_ = false;
};

// QWidgetItem normally bypasses QWidget::heightForWidth when it owns a layout,
// calling QStackedLayout directly and measuring hidden pages again.
class CurrentPageStackItem final : public QWidgetItem {
public:
    explicit CurrentPageStackItem(CurrentPageStack* stack) : QWidgetItem(stack) {}
    int heightForWidth(int width) const override;
    int minimumHeightForWidth(int width) const override;
};

}  // namespace remote::controller::detail
