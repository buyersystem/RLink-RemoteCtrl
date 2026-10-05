// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QHash>
#include <QByteArray>
#include <QScrollArea>
#include <QString>

class QLabel;
class QPushButton;
class QStackedWidget;
class QVBoxLayout;

namespace remote::controller {

class DiagnosticsPage final : public QScrollArea {
    Q_OBJECT

public:
    explicit DiagnosticsPage(QWidget* parent = nullptr);

    QVBoxLayout* AddDetailPage(QStackedWidget* stack,
                               const QString& title,
                               const QString& description,
                               QPushButton** copyButton = nullptr);
    void AddValue(QVBoxLayout* layout,
                  const QString& key,
                  const QString& title,
                  bool expanded = false);
    void RegisterValue(const QString& key, QLabel* label);
    QLabel* ValueLabel(const QString& key) const;
    void SetValue(const QString& key, const QString& value,
                  const QByteArray& tone = "normal");
    int CurrentCategory() const;
    bool NeedsRealtimeDiagnostics() const;
    bool HasValues() const;
    bool ScreenFrameRateLogEnabled() const;
    bool InputEventStatsEnabled() const;
    QWidget* StatsCardsWidget() const;
    QWidget* PolicyCardsWidget() const;
    QPushButton* CopyAllButton() const;
    QPushButton* CopyMediaButton() const;

signals:
    void ScreenFrameRateLogToggled(bool enabled);
    void InputEventStatsToggled(bool enabled);
    void RefreshRequested();

private:
    void BuildWorkspace();
    void ApplyValue(const QString& key);

    QVBoxLayout* contentLayout_ = nullptr;
    QHash<QString, QLabel*> valueLabels_;
    struct Value {
        QString text;
        QByteArray tone;
    };
    QHash<QString, Value> values_;
    QStackedWidget* detailStack_ = nullptr;
    QWidget* statsCardsWidget_ = nullptr;
    QWidget* policyCardsWidget_ = nullptr;
    QPushButton* copyAllButton_ = nullptr;
    QPushButton* copyMediaButton_ = nullptr;
    bool screenFrameRateLogEnabled_ = false;
    bool inputEventStatsEnabled_ = false;
};

}  // namespace remote::controller
