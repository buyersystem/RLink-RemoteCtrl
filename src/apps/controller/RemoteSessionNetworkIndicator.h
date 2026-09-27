// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QWidget>

class QPaintEvent;

namespace remote::controller {

class NetworkSignalIndicator final : public QWidget {
public:
    explicit NetworkSignalIndicator(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};

}  // namespace remote::controller
