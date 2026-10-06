// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <functional>
#include <QElapsedTimer>
#include <QWidget>
#include "src/core/SessionEngineTypes.h"

class QLabel;
class QPushButton;
class QTimer;

namespace remote::controller {

// Identity deliberately excludes display/FPS revisions: changing a monitor or
// recovering the transport must not restart the control-session clock.
struct ControlledSessionStatus {
    QString identity;
    bool direct = false;
    bool room = false;
    bool recovering = false;
    static ControlledSessionStatus FromSnapshot(const SessionEngineSnapshot& s);
};

class ControlledSessionIndicator final : public QWidget {
public:
    ControlledSessionIndicator(QWidget* parent, std::function<void(QString)> endControl);
    void UpdateSession(const SessionEngineSnapshot& snapshot);
    void ApplyTheme(bool dark);
    void Reset();
    void SetEnding(bool ending);
    const QString& SessionIdentity() const { return identity_; }

protected:
    void closeEvent(QCloseEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void RefreshTime();
    void PositionAtTop();
    QLabel* statusLabel_ = nullptr;
    QLabel* durationLabel_ = nullptr;
    QPushButton* endButton_ = nullptr;
    QTimer* timer_ = nullptr;
    QElapsedTimer elapsed_;
    QString identity_;
};
} // namespace remote::controller
