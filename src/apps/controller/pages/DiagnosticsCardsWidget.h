// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QVector>
#include <QWidget>

class QFrame;
class QLabel;
class QToolButton;
class QVBoxLayout;

namespace remote::controller::detail {

struct DiagnosticsChip {
  QString key;
  QString label;
  QString value;
  QByteArray tone = "normal";
  bool wide = false;
};

struct DiagnosticsCard {
  QString key;
  QString title;
  QString subtitle;
  QVector<DiagnosticsChip> chips;
};

struct DiagnosticsSection {
  QString key;
  QString title;
  QString description;
  QVector<DiagnosticsCard> cards;
};

class DiagnosticsCardsWidget final : public QWidget {
public:
  explicit DiagnosticsCardsWidget(QWidget *parent = nullptr);

  void SetSections(const QVector<DiagnosticsSection> &sections,
                   const QString &emptyText);

private:
  void Rebuild(const QVector<DiagnosticsSection> &sections,
               const QString &emptyText);

  QVBoxLayout *layout_ = nullptr;
  QString structure_;
  QHash<QString, QLabel *> textLabels_;
  QHash<QString, QLabel *> chipNameLabels_;
  QHash<QString, QFrame *> chipFrames_;
  QHash<QString, QToolButton *> titleButtons_;
  QHash<QString, QString> cardCopyTexts_;
  QHash<QString, bool> sectionExpanded_;
  QHash<QString, bool> cardExpanded_;
};

} // namespace remote::controller::detail
