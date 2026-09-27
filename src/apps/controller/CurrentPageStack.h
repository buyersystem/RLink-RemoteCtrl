// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QStackedWidget>

namespace remote::controller::detail {

class CurrentPageStack final : public QStackedWidget {
public:
    using QStackedWidget::QStackedWidget;

    int PageHeightForWidth(QWidget* page) const;
    void RefreshCurrentHeight();
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void resizeEvent(QResizeEvent* event) override;
};

}  // namespace remote::controller::detail
