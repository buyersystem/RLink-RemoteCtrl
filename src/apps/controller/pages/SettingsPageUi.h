// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <utility>

class QFrame;
class QHBoxLayout;
class QString;
class QVBoxLayout;
class QWidget;

namespace remote::controller {

void AddSettingsDetailHeader(QVBoxLayout* layout,
                             QWidget* parent,
                             const QString& title,
                             const QString& description);

std::pair<QFrame*, QHBoxLayout*> CreateSettingsRow(
    QWidget* parent,
    const QString& title,
    const QString& description);

std::pair<QFrame*, QHBoxLayout*> CreatePathSettingsRow(
    QWidget* parent,
    const QString& title,
    const QString& description);

}  // namespace remote::controller
