// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QTimer>
#include <QToolButton>

class QPaintEvent;
class QVariantAnimation;
class QWidget;

namespace remote::controller {

class RemoteTransferStatusButton final : public QToolButton {
public:
    explicit RemoteTransferStatusButton(QWidget* parent = nullptr);

    void SetProgress(double progress);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QTimer waveTimer_{this};
    QVariantAnimation* progressAnimation_ = nullptr;
    double progress_ = 0.0;
    double targetProgress_ = 0.0;
    double wavePhase_ = 0.0;
};

}  // namespace remote::controller
