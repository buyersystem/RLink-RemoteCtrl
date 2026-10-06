// SPDX-License-Identifier: GPL-3.0-only
#include "ControlledSessionIndicator.h"

#include <QCloseEvent>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include "ui/RemoteCTheme.h"

namespace remote::controller {
ControlledSessionStatus ControlledSessionStatus::FromSnapshot(const SessionEngineSnapshot& s)
{
    ControlledSessionStatus result;
    const bool recoveringDirect = s.state == SessionEngineState::kConnecting && s.direct.sessionEverActive;
    result.direct = s.purpose == SessionPurpose::kRemoteControl &&
        s.remoteControlRole == RemoteControlRole::kControlled &&
        !s.sessionId.empty() && (s.state == SessionEngineState::kActive || recoveringDirect);
    result.room = s.room.membership == RoomMembershipState::kActive &&
        !s.localDeviceId.empty() && s.room.screenSharerDeviceId == s.localDeviceId &&
        s.roomControlGrantActive && !s.room.activeControllerDeviceId.empty() &&
        s.room.activeControllerDeviceId != s.localDeviceId &&
        (s.room.screenShareState == RoomScreenShareState::kActive ||
         s.room.screenShareState == RoomScreenShareState::kRecovering);
    if (result.direct) result.identity = QStringLiteral("direct:%1").arg(QString::fromStdString(s.sessionId));
    if (result.room) result.identity += QStringLiteral("|room:%1:%2:%3")
        .arg(QString::fromStdString(s.room.roomId), QString::fromStdString(s.room.activeControllerDeviceId))
        .arg(s.room.screenShareEpoch);
    result.recovering = (result.direct && recoveringDirect) ||
        (result.room && s.room.screenShareState == RoomScreenShareState::kRecovering);
    if (result.room) {
        for (const auto& peer : s.roomActivity.peerConnections) {
            if (peer.peerDeviceId == s.room.activeControllerDeviceId &&
                peer.state != RoomPeerConnectionState::kActive) {
                result.recovering = true;
                break;
            }
        }
    }
    return result;
}

ControlledSessionIndicator::ControlledSessionIndicator(QWidget* parent, std::function<void(QString)> endControl)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("controlledSessionIndicator"));
    setAttribute(Qt::WA_StyledBackground);
    parent->installEventFilter(this);
    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(12, 3, 6, 3);
    row->setSpacing(12);
    statusLabel_ = new QLabel(QStringLiteral("正被控制"), this);
    statusLabel_->setObjectName(QStringLiteral("controlledStatus"));
    statusLabel_->setMinimumWidth(76); // Reserve recovery text without shifting the toolbar.
    durationLabel_ = new QLabel(QStringLiteral("00:00:00"), this);
    durationLabel_->setObjectName(QStringLiteral("controlledDuration"));
    durationLabel_->setMinimumWidth(66);
    endButton_ = new QPushButton(QStringLiteral("结束远控"), this);
    endButton_->setObjectName(QStringLiteral("endControl"));
    endButton_->setCursor(Qt::PointingHandCursor);
    endButton_->setFocusPolicy(Qt::NoFocus);
    row->addWidget(statusLabel_);
    row->addWidget(durationLabel_);
    row->addWidget(endButton_);
    connect(endButton_, &QPushButton::clicked, this, [this, endControl] {
        if (!identity_.isEmpty()) endControl(identity_);
    });
    timer_ = new QTimer(this);
    timer_->setInterval(1000);
    connect(timer_, &QTimer::timeout, this, [this] { RefreshTime(); });
    ApplyTheme(ui::RemoteCTheme::IsDark(ui::RemoteCTheme::LoadPreference()));
    hide();
}

void ControlledSessionIndicator::ApplyTheme(bool dark)
{
    setStyleSheet(QStringLiteral(
        "QWidget#controlledSessionIndicator { background:%1; border:1px solid %2; border-radius:8px; }"
        "QLabel { color:%3; border:0; background:transparent; font-size:12px; }"
        "QLabel#controlledStatus { font-weight:600; }"
        "QPushButton { color:%4; background:%5; border:1px solid %6; border-radius:5px; padding:3px 10px; font-size:12px; }"
        "QPushButton:hover { background:%7; } QPushButton:disabled { color:%3; background:%2; }")
        .arg(dark ? "#152131" : "#FFFEFB", dark ? "#35465E" : "#D7DBE2",
             dark ? "#E6EDF7" : "#243247", dark ? "#FF93A5" : "#B4233D",
             dark ? "#362331" : "#FFF0F1", dark ? "#744051" : "#EAC1C7",
             dark ? "#4B2C3A" : "#FFE0E4"));
    adjustSize();
    if (!isHidden()) PositionAtTop();
}

void ControlledSessionIndicator::UpdateSession(const SessionEngineSnapshot& snapshot)
{
    const auto status = ControlledSessionStatus::FromSnapshot(snapshot);
    if (status.identity.isEmpty()) { Reset(); return; }
    if (identity_ != status.identity) {
        identity_ = status.identity;
        elapsed_.start();
        SetEnding(false);
    }
    statusLabel_->setText(status.recovering ? QStringLiteral("远控重连中") : QStringLiteral("正被控制"));
    RefreshTime();
    if (isHidden()) { adjustSize(); PositionAtTop(); show(); raise(); }
    if (!timer_->isActive()) timer_->start();
}

void ControlledSessionIndicator::RefreshTime()
{
    const qint64 seconds = elapsed_.isValid() ? elapsed_.elapsed() / 1000 : 0;
    durationLabel_->setText(QStringLiteral("%1:%2:%3")
        .arg(seconds / 3600, 2, 10, QLatin1Char('0'))
        .arg(seconds / 60 % 60, 2, 10, QLatin1Char('0'))
        .arg(seconds % 60, 2, 10, QLatin1Char('0')));
}

void ControlledSessionIndicator::PositionAtTop()
{
    if (const auto* parent = parentWidget())
        move((parent->width() - width()) / 2, (parent->height() - height()) / 2);
}

void ControlledSessionIndicator::Reset()
{
    if (identity_.isEmpty() && !isVisible() && !timer_->isActive()) return;
    timer_->stop();
    identity_.clear();
    elapsed_.invalidate();
    hide();
    SetEnding(false);
}
void ControlledSessionIndicator::SetEnding(bool ending)
{
    endButton_->setEnabled(!ending);
    endButton_->setText(ending ? QStringLiteral("正在结束…") : QStringLiteral("结束远控"));
    adjustSize();
    if (!isHidden()) PositionAtTop();
}
void ControlledSessionIndicator::closeEvent(QCloseEvent* event)
{
    // Only session state, not an incidental child close, dismisses the indicator.
    if (!identity_.isEmpty()) event->ignore();
    else QWidget::closeEvent(event);
}
bool ControlledSessionIndicator::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == parentWidget() && !isHidden() &&
        (event->type() == QEvent::Resize || event->type() == QEvent::Show ||
         event->type() == QEvent::LayoutRequest)) PositionAtTop();
    return QWidget::eventFilter(watched, event);
}
} // namespace remote::controller
