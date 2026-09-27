// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QElapsedTimer>
#include <QString>
#include <QTimer>
#include <QToolButton>

#include "ui/morph/MorphIconCore.h"

class QPaintEvent;
class QWidget;

namespace remote::controller {

class MorphIconToolButton final : public QToolButton {
public:
    MorphIconToolButton(
        const QString& sourceResource,
        const QString& targetResource,
        QWidget* parent = nullptr);

    void SetTarget(bool target);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QString sourceResource_;
    QString targetResource_;
    remotec::ui::morph::MorphIconCore morphIcon_;
    remotec::ui::morph::Spring spring_;
    QTimer timer_;
    QElapsedTimer elapsed_;
    double progress_ = 0.0;
    double start_ = 0.0;
    double end_ = 0.0;
};

}  // namespace remote::controller
