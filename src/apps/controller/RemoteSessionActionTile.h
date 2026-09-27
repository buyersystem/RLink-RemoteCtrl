// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QElapsedTimer>
#include <QPushButton>
#include <QString>

#include "src/apps/controller/ui/morph/MorphIconCore.h"

class QEnterEvent;
class QEvent;
class QPaintEvent;
class QTimer;
class QVariantAnimation;
class QWidget;

namespace remote::controller {

class ActionTile final : public QPushButton {
public:
    ActionTile(const QString& icon,
               const QString& text,
               int fixedWidth,
               QWidget* parent = nullptr);

    void SetIcon(const QString& icon);
    void SetText(const QString& text);
    void SetTone(const QString& tone);
    void SetInteractive(bool interactive);
    void SetSlashVisible(bool visible);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    static constexpr int kTileHeight = 56;

    void ConfigureMorphPair();
    void StartMorphTransition(bool target);
    void StartIconTransition();
    QString ResolveIconResource() const;
    static void Repolish(QWidget* widget);

    QString icon_;
    QString text_;
    bool slashVisible_ = false;
    QVariantAnimation* transitionAnimation_ = nullptr;
    qreal iconScale_ = 1.0;
    remotec::ui::morph::MorphIconCore morphIcon_;
    remotec::ui::morph::Spring morphSpring_;
    QTimer* morphTimer_ = nullptr;
    QElapsedTimer morphElapsed_;
    double morphProgress_ = 0.0;
    double morphStart_ = 0.0;
    double morphEnd_ = 0.0;
    bool hoverMorph_ = false;
};

}  // namespace remote::controller
