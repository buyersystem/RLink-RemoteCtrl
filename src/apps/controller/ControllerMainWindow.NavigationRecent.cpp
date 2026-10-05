// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ControllerMainWindow.h"
#include "ControllerMainWindowSupport.h"

#include <QColor>
#include <QDateTime>
#include <QEasingCurve>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QRegion>
#include <QSettings>
#include <QStackedWidget>
#include <QStyle>
#include <QTimer>
#include <QVariant>
#include <QVBoxLayout>
#include <algorithm>
#include <utility>
#include "RemoteCToast.h"
#include "src/apps/controller/ui/morph/MorphIconButtonBinding.h"
#include "src/apps/controller/ui/RemoteCTheme.h"
#include "CurrentPageStack.h"
#include "pages/DirectConnectPage.h"
#include "pages/RecentConnectionsPage.h"
#include "pages/RoomPage.h"

namespace remote::controller {
using namespace detail;

QString ControllerMainWindow::RecentSettingsKey(
    const QString& listName) const
{
    const QString accountNamespace = recentHistoryAccountKey_.isEmpty()
        ? QStringLiteral("unavailable")
        : recentHistoryAccountKey_;
    return QStringLiteral("ui/accounts/%1/%2")
        .arg(accountNamespace, listName);
}

void ControllerMainWindow::MigrateLegacyRecentHistory()
{
    if (recentHistoryAccountKey_.isEmpty()) {
        return;
    }
    QSettings settings;
    const auto migrate = [&settings, this](const QString& legacyKey,
                                            const QString& listName,
                                            int maximumRecords) {
        const QString scopedKey = RecentSettingsKey(listName);
        if (!settings.contains(scopedKey) && settings.contains(legacyKey)) {
            QVariantList records = settings.value(legacyKey).toList();
            while (records.size() > maximumRecords) {
                records.removeLast();
            }
            settings.setValue(scopedKey, records);
        }
        settings.remove(legacyKey);
    };
    migrate(QStringLiteral("ui/recentRooms"),
            QStringLiteral("recentRooms"), 3);
    migrate(QStringLiteral("ui/recentDevices"),
            QStringLiteral("recentDevices"), 5);
}

void ControllerMainWindow::RememberRecentRoom(
    const SessionEngineSnapshot& snapshot)
{
    if (recentHistoryAccountKey_.isEmpty()) {
        return;
    }
    if (snapshot.room.membership != RoomMembershipState::kActive ||
        snapshot.room.roomId.empty()) {
        if (snapshot.room.membership == RoomMembershipState::kNone ||
            snapshot.room.membership == RoomMembershipState::kFailed) {
            const bool hadRememberedRoom =
                !lastRememberedRoomId_.isEmpty();
            lastRememberedRoomId_.clear();
            if (hadRememberedRoom &&
                snapshot.connectivity == SessionConnectivityState::kOnline) {
                recentRoomAvailabilityRequested_ = true;
                QTimer::singleShot(
                    0, this,
                    [this] { RequestRecentRoomAvailability(); });
            }
        }
        return;
    }

    const QString roomId = QString::fromStdString(snapshot.room.roomId);
    if (lastRememberedRoomId_ == roomId) {
        return;
    }
    lastRememberedRoomId_ = roomId;

    QSettings settings;
    QVariantList records = settings.value(
        RecentSettingsKey(QStringLiteral("recentRooms"))).toList();
    for (int index = records.size() - 1; index >= 0; --index) {
        if (records[index].toMap().value(
                QStringLiteral("roomId")).toString() == roomId) {
            records.removeAt(index);
        }
    }
    QVariantMap record;
    record.insert(QStringLiteral("roomId"), roomId);
    record.insert(
        QStringLiteral("ownerId"),
        QString::fromStdString(snapshot.room.ownerDeviceId));
    record.insert(QStringLiteral("capacity"), snapshot.room.capacity);
    record.insert(
        QStringLiteral("availability"),
        static_cast<int>(RoomAvailabilityState::kAvailable));
    record.insert(
        QStringLiteral("lastUsed"),
        QDateTime::currentDateTime().toMSecsSinceEpoch());
    records.prepend(record);
    while (records.size() > 3) {
        records.removeLast();
    }
    settings.setValue(
        RecentSettingsKey(QStringLiteral("recentRooms")), records);
    RefreshRecentRooms();
}

void ControllerMainWindow::RefreshRecentRooms()
{
    if (!recentConnectionsPage_) {
        return;
    }

    QSettings settings;
    QVariantList records = settings.value(
        RecentSettingsKey(QStringLiteral("recentRooms"))).toList();
    bool recordsChanged = false;
    for (int index = records.size() - 1; index >= 0; --index) {
        const QString roomId = records[index].toMap().value(
            QStringLiteral("roomId")).toString();
        if (!IsNineDigitPublicId(roomId)) {
            records.removeAt(index);
            recordsChanged = true;
        }
    }
    while (records.size() > 3) {
        records.removeLast();
        recordsChanged = true;
    }
    if (recordsChanged) {
        settings.setValue(
            RecentSettingsKey(QStringLiteral("recentRooms")), records);
    }

    QVector<RecentRoomCardData> cards;
    cards.reserve(records.size());
    for (const QVariant& value : records) {
        const QVariantMap record = value.toMap();
        RecentRoomCardData card;
        card.roomId = record.value(QStringLiteral("roomId")).toString();
        if (!IsNineDigitPublicId(card.roomId)) {
            continue;
        }
        const QString ownerId = record.value(
            QStringLiteral("ownerId")).toString();
        const auto capacity = record.value(
            QStringLiteral("capacity"), 2).toUInt();
        const QDateTime lastUsed = QDateTime::fromMSecsSinceEpoch(
            record.value(QStringLiteral("lastUsed")).toLongLong());
        const auto availability = static_cast<RoomAvailabilityState>(
            record.value(
                QStringLiteral("availability"),
                static_cast<int>(RoomAvailabilityState::kChecking)).toInt());

        const QString ownerText = ownerId.isEmpty()
            ? QStringLiteral("房主未知")
            : QStringLiteral("房主 %1").arg(ownerId);
        card.detail = QStringLiteral("%1 · 上限 %2 人 · %3")
            .arg(ownerText)
            .arg(capacity)
            .arg(lastUsed.toString(QStringLiteral("MM-dd HH:mm")));
        switch (availability) {
        case RoomAvailabilityState::kAvailable:
            card.availabilityText = QStringLiteral("房间有效");
            card.availabilityTone = QStringLiteral("available");
            card.actionText = QStringLiteral("申请加入");
            card.canJoin = true;
            break;
        case RoomAvailabilityState::kTemporarilyUnavailable:
            card.availabilityText = QStringLiteral("房间暂不可加入");
            card.availabilityTone = QStringLiteral("warning");
            card.actionText = QStringLiteral("暂不可用");
            break;
        case RoomAvailabilityState::kClosed:
            card.availabilityText = QStringLiteral("房间已关闭");
            card.availabilityTone = QStringLiteral("closed");
            card.actionText = QStringLiteral("已关闭");
            break;
        case RoomAvailabilityState::kChecking:
            card.availabilityText = QStringLiteral("正在检查房间状态");
            card.availabilityTone = QStringLiteral("checking");
            card.actionText = QStringLiteral("检查中");
            break;
        }
        cards.push_back(std::move(card));
    }
    recentConnectionsPage_->SetRooms(cards);
}

void ControllerMainWindow::RememberRecentDevice(
    const SessionEngineSnapshot& snapshot)
{
    if (recentHistoryAccountKey_.isEmpty()) {
        return;
    }
    if (snapshot.state != SessionEngineState::kActive ||
        snapshot.remoteControlRole != RemoteControlRole::kController ||
        snapshot.sessionId.empty() || snapshot.peerDeviceId.empty() ||
        (snapshot.origin != SessionOrigin::kOwnedDevice &&
         snapshot.origin != SessionOrigin::kRemoteAssistance)) {
        if (snapshot.state == SessionEngineState::kReady) {
            lastRememberedDirectSessionId_.clear();
        }
        return;
    }

    const QString sessionId = QString::fromStdString(snapshot.sessionId);
    if (lastRememberedDirectSessionId_ == sessionId) {
        return;
    }
    lastRememberedDirectSessionId_ = sessionId;
    const QString deviceId = QString::fromStdString(snapshot.peerDeviceId);
    QString deviceName = pendingDeviceName_.trimmed();
    for (const auto& owned : snapshot.ownedDevices.devices) {
        if (owned.deviceId == snapshot.peerDeviceId &&
            !owned.deviceName.empty()) {
            deviceName = QString::fromStdString(owned.deviceName);
            break;
        }
    }
    if (deviceName.isEmpty()) {
        deviceName = snapshot.origin == SessionOrigin::kOwnedDevice
            ? QStringLiteral("我的设备")
            : QStringLiteral("远程协助设备");
    }

    QSettings settings;
    QVariantList records = settings.value(
        RecentSettingsKey(QStringLiteral("recentDevices"))).toList();
    for (int index = records.size() - 1; index >= 0; --index) {
        if (records[index].toMap().value(
                QStringLiteral("deviceId")).toString() == deviceId) {
            records.removeAt(index);
        }
    }
    QVariantMap record;
    record.insert(QStringLiteral("deviceId"), deviceId);
    record.insert(QStringLiteral("deviceName"), deviceName);
    record.insert(QStringLiteral("origin"),
                  static_cast<int>(snapshot.origin));
    record.insert(QStringLiteral("lastUsed"),
                  QDateTime::currentMSecsSinceEpoch());
    records.prepend(record);
    while (records.size() > 5) {
        records.removeLast();
    }
    settings.setValue(
        RecentSettingsKey(QStringLiteral("recentDevices")), records);
    RefreshRecentDevices(snapshot, true);
}

void ControllerMainWindow::RefreshRecentDevices()
{
    if (!recentConnectionsPage_ || !engine_) {
        return;
    }

    // Explicit navigation/account/history refreshes also pick up persisted
    // changes. Snapshot-driven refreshes below reuse their already copied state.
    RefreshRecentDevices(engine_->Snapshot(), true);
}

void ControllerMainWindow::RefreshRecentDevices(
    const SessionEngineSnapshot& snapshot, bool forceRefresh)
{
    if (!recentConnectionsPage_) {
        return;
    }
    const bool engineReady =
        snapshot.connectivity == SessionConnectivityState::kOnline &&
        snapshot.state == SessionEngineState::kReady;
    if (!recentDevicesRefreshState_.Accept(
            recentHistoryAccountKey_, snapshot.ownedDevices,
            snapshot.connectivity, engineReady, darkInterfaceTheme_, forceRefresh)) {
        return;
    }

    QSettings settings;
    QVariantList records = settings.value(
        RecentSettingsKey(QStringLiteral("recentDevices"))).toList();
    bool recordsChanged = false;
    for (int index = records.size() - 1; index >= 0; --index) {
        if (!IsNineDigitPublicId(records[index].toMap().value(
                QStringLiteral("deviceId")).toString())) {
            records.removeAt(index);
            recordsChanged = true;
        }
    }
    while (records.size() > 5) {
        records.removeLast();
        recordsChanged = true;
    }
    if (recordsChanged) {
        settings.setValue(
            RecentSettingsKey(QStringLiteral("recentDevices")), records);
    }

    QVector<RecentDeviceCardData> cards;
    cards.reserve(records.size());
    for (const QVariant& value : records) {
        const QVariantMap record = value.toMap();
        RecentDeviceCardData card;
        card.deviceId = record.value(
            QStringLiteral("deviceId")).toString();
        card.deviceName = record.value(
            QStringLiteral("deviceName")).toString().trimmed();
        if (card.deviceName.isEmpty()) {
            card.deviceName = QStringLiteral("远程设备");
        }
        const auto origin = static_cast<SessionOrigin>(record.value(
            QStringLiteral("origin"),
            static_cast<int>(SessionOrigin::kRemoteAssistance)).toInt());
        card.ownedDevice = origin == SessionOrigin::kOwnedDevice;
        const auto owned = std::find_if(
            snapshot.ownedDevices.devices.begin(),
            snapshot.ownedDevices.devices.end(),
            [&card](const OwnedDeviceSnapshot& candidate) {
                return candidate.deviceId == card.deviceId.toStdString();
            });
        card.ownedDeviceOnline =
            card.ownedDevice &&
            owned != snapshot.ownedDevices.devices.end() &&
            owned->online && !owned->current;
        const QDateTime lastUsed = QDateTime::fromMSecsSinceEpoch(
            record.value(QStringLiteral("lastUsed")).toLongLong());
        card.detail = QStringLiteral("设备 ID  %1 · %2 · %3")
            .arg(card.deviceId,
                 card.ownedDevice ? QStringLiteral("我的设备")
                                  : QStringLiteral("验证码协助"),
                 lastUsed.toString(QStringLiteral("MM-dd HH:mm")));
        card.actionText = card.ownedDevice
            ? (card.ownedDeviceOnline ? QStringLiteral("进入桌面")
                                      : QStringLiteral("设备离线"))
            : QStringLiteral("再次连接  →");
        card.actionEnabled =
            engineReady && (!card.ownedDevice || card.ownedDeviceOnline);
        cards.push_back(std::move(card));
    }
    recentConnectionsPage_->SetDevices(cards);
}

void ControllerMainWindow::RequestRecentRoomAvailability()
{
    if (recentHistoryAccountKey_.isEmpty()) {
        recentRoomAvailabilityRequested_ = true;
        return;
    }
    const QSettings settings;
    const QVariantList records = settings.value(
        RecentSettingsKey(QStringLiteral("recentRooms"))).toList();
    std::vector<std::string> roomIds;
    roomIds.reserve(static_cast<std::size_t>(records.size()));
    for (const QVariant& value : records) {
        const QString roomId = value.toMap().value(
            QStringLiteral("roomId")).toString();
        if (IsNineDigitPublicId(roomId)) {
            roomIds.push_back(roomId.toStdString());
        }
    }
    if (roomIds.empty()) {
        recentRoomAvailabilityRequested_ = true;
        return;
    }
    const auto result = engine_->QueryRoomAvailability(roomIds);
    recentRoomAvailabilityRequested_ = result.accepted;
}

void ControllerMainWindow::SelectMainPage(
    int pageIndex,
    QPushButton* navigationButton,
    const QString& title)
{
    if (!pageStack_ || pageIndex < 0 ||
        pageIndex >= pageStack_->count()) {
        return;
    }

    AnimateNavigationIndicator(navigationButton);
    for (auto* button : pageNavigationButtons_) {
        if (!button) {
            continue;
        }
        const bool active = button == navigationButton;
        const bool activeChanged =
            button->property("navActive").toBool() != active;
        if (activeChanged) {
            button->setProperty("navActive", active);
        }
        button->setChecked(active);
        if (activeChanged) {
            button->style()->unpolish(button);
            button->style()->polish(button);
            button->update();
        }
    }
    if (titleBar_) {
        titleBar_->SetTitle(title);
    }
    setWindowTitle(QStringLiteral("RLink - %1").arg(title));

    if (pageStack_->currentIndex() == pageIndex) {
        return;
    }
    pageStack_->setCurrentIndex(pageIndex);
}

void ControllerMainWindow::AnimateNavigationIndicator(
    QPushButton* navigationButton)
{
    if (!navigationIndicator_ || !navigationButton) {
        return;
    }
    const QPoint topLeft = navigationButton->mapTo(
        navigationIndicator_->parentWidget(), QPoint(0, 0));
    const QRect destination(
        5, topLeft.y() + 9, 3,
        std::max(18, navigationButton->height() - 18));
    if (CurrentUiAnimationLevel() <= 0 ||
        !navigationIndicator_->isVisible() ||
        navigationIndicator_->geometry().isEmpty()) {
        navigationIndicator_->setGeometry(destination);
        return;
    }
    navigationAnimation_->stop();
    navigationAnimation_->setDuration(
        CurrentUiAnimationLevel() == 1 ? 95 : 155);
    navigationAnimation_->setStartValue(navigationIndicator_->geometry());
    navigationAnimation_->setEndValue(destination);
    navigationAnimation_->start();
}

void ControllerMainWindow::QueueRoomWorkspaceActive(bool active)
{
    QWidget* desired = active
        ? static_cast<QWidget*>(RoomControls().roomPanel)
        : static_cast<QWidget*>(RoomControls().entryPanel);
    if (!RoomControls().workspaceStack || !desired) {
        return;
    }
    if ((RoomControls().workspaceAnimation || roomWorkspaceTransitionPending_) &&
        roomWorkspaceTargetActive_ == active) {
        return;
    }
    if (!RoomControls().workspaceAnimation && !roomWorkspaceTransitionPending_ &&
        RoomControls().workspaceStack->currentWidget() == desired) {
        return;
    }

    roomWorkspaceTargetActive_ = active;
    roomWorkspaceTransitionPending_ = true;
    const quint64 request = ++roomWorkspaceTransitionRequest_;
    // Let all labels, member rows and media states from the current snapshot
    // settle through one UI frame before the transition captures the page.
    QTimer::singleShot(16, this, [this, active, request] {
        if (request != roomWorkspaceTransitionRequest_) {
            return;
        }
        roomWorkspaceTransitionPending_ = false;
        SetRoomWorkspaceActive(active);
    });
}

void ControllerMainWindow::SetRoomWorkspaceActive(bool active)
{
    if (!RoomControls().workspaceStack || !RoomControls().entryPanel || !RoomControls().roomPanel) {
        return;
    }

    QWidget* incoming = active
        ? static_cast<QWidget*>(RoomControls().roomPanel)
        : static_cast<QWidget*>(RoomControls().entryPanel);

    if (RoomControls().workspaceAnimation) {
        // Session snapshots can arrive several times while the room remains
        // active. They must not cancel an already-running transition toward
        // the same page.
        if (roomWorkspaceTargetActive_ == active) {
            return;
        }
        RoomControls().workspaceAnimation->stop();
        RoomControls().workspaceAnimation->deleteLater();
        RoomControls().workspaceAnimation = nullptr;
        if (RoomControls().workspaceTransitionLayer) {
            RoomControls().workspaceTransitionLayer->deleteLater();
            RoomControls().workspaceTransitionLayer = nullptr;
        }
        RoomControls().workspaceStack->setCurrentWidget(
            active ? static_cast<QWidget*>(RoomControls().roomPanel)
                   : static_cast<QWidget*>(RoomControls().entryPanel));
        if (auto* stack =
                dynamic_cast<CurrentPageStack*>(RoomControls().workspaceStack)) {
            stack->RefreshCurrentHeight();
        }
    }
    roomWorkspaceTargetActive_ = active;

    QWidget* outgoing = RoomControls().workspaceStack->currentWidget();
    if (outgoing == incoming) {
        return;
    }

    const QRect viewport = RoomControls().workspaceStack->contentsRect();
    const int level = CurrentUiAnimationLevel();
    if (level <= 0 || !isVisible() || viewport.width() <= 0) {
        RoomControls().workspaceStack->setCurrentWidget(incoming);
        if (auto* stack =
                dynamic_cast<CurrentPageStack*>(RoomControls().workspaceStack)) {
            stack->RefreshCurrentHeight();
        }
        return;
    }

    // QStackedWidget owns its page geometry, so animating the real pages is
    // immediately overwritten by its layout. Render both pages into a short-
    // lived overlay and animate those independent layers instead.
    const int direction = active ? 1 : -1;
    auto* currentStack =
        dynamic_cast<CurrentPageStack*>(RoomControls().workspaceStack);
    const int outgoingHeight = viewport.height();
    const int incomingHeight = currentStack
        ? currentStack->PageHeightForWidth(incoming)
        : outgoingHeight;
    const int transitionHeight = std::max(outgoingHeight, incomingHeight);
    const int transitionWidth = viewport.width();
    const QSize outgoingSize(transitionWidth, outgoingHeight);
    const QSize incomingSize(transitionWidth, incomingHeight);
    const QColor transitionColor(
        darkInterfaceTheme_ ? QStringLiteral("#0E1522")
                            : QStringLiteral("#F5F5F7"));
    QPixmap outgoingPixmap(outgoingSize);
    outgoingPixmap.fill(transitionColor);
    outgoing->render(&outgoingPixmap, QPoint(),
                     QRegion(QRect(QPoint(), outgoingSize)));

    incoming->ensurePolished();
    incoming->resize(incomingSize);
    QPixmap incomingPixmap(incomingSize);
    incomingPixmap.fill(transitionColor);
    incoming->render(&incomingPixmap, QPoint(),
                     QRegion(QRect(QPoint(), incomingSize)));

    auto* transitionLayer = new QWidget(RoomControls().workspaceStack);
    RoomControls().workspaceTransitionLayer = transitionLayer;
    transitionLayer->setAttribute(Qt::WA_StyledBackground, true);
    transitionLayer->setStyleSheet(QStringLiteral("background:%1;")
        .arg(transitionColor.name()));
    transitionLayer->setGeometry(
        QRect(viewport.topLeft(), QSize(transitionWidth, transitionHeight)));

    auto* outgoingLayer = new QLabel(transitionLayer);
    outgoingLayer->setPixmap(outgoingPixmap);
    outgoingLayer->setGeometry(QRect(QPoint(), outgoingSize));

    auto* incomingLayer = new QLabel(transitionLayer);
    incomingLayer->setPixmap(incomingPixmap);
    incomingLayer->setGeometry(
        QRect(QPoint(direction * transitionWidth, 0), incomingSize));
    transitionLayer->show();
    transitionLayer->raise();

    auto* group = new QParallelAnimationGroup(this);
    RoomControls().workspaceAnimation = group;
    const int duration = level == 1 ? 180 : 270;

    auto* outgoingAnimation =
        new QPropertyAnimation(outgoingLayer, "pos", group);
    outgoingAnimation->setDuration(duration);
    outgoingAnimation->setStartValue(QPoint(0, 0));
    outgoingAnimation->setEndValue(
        QPoint(-direction * transitionWidth, 0));
    outgoingAnimation->setEasingCurve(QEasingCurve::InOutCubic);

    auto* incomingAnimation =
        new QPropertyAnimation(incomingLayer, "pos", group);
    incomingAnimation->setDuration(duration);
    incomingAnimation->setStartValue(
        QPoint(direction * transitionWidth, 0));
    incomingAnimation->setEndValue(QPoint(0, 0));
    incomingAnimation->setEasingCurve(QEasingCurve::OutCubic);

    auto* minimumHeightAnimation = new QPropertyAnimation(
        RoomControls().workspaceStack, "minimumHeight", group);
    minimumHeightAnimation->setDuration(duration);
    minimumHeightAnimation->setStartValue(outgoingHeight);
    minimumHeightAnimation->setEndValue(incomingHeight);
    minimumHeightAnimation->setEasingCurve(QEasingCurve::InOutCubic);

    auto* maximumHeightAnimation = new QPropertyAnimation(
        RoomControls().workspaceStack, "maximumHeight", group);
    maximumHeightAnimation->setDuration(duration);
    maximumHeightAnimation->setStartValue(outgoingHeight);
    maximumHeightAnimation->setEndValue(incomingHeight);
    maximumHeightAnimation->setEasingCurve(QEasingCurve::InOutCubic);

    connect(group, &QParallelAnimationGroup::finished, this,
            [this, group, incoming, transitionLayer] {
                if (RoomControls().workspaceAnimation != group) {
                    return;
                }
                RoomControls().workspaceStack->setCurrentWidget(incoming);
                if (auto* stack = dynamic_cast<CurrentPageStack*>(
                        RoomControls().workspaceStack)) {
                    stack->RefreshCurrentHeight();
                }
                transitionLayer->deleteLater();
                RoomControls().workspaceTransitionLayer = nullptr;
                RoomControls().workspaceAnimation = nullptr;
                group->deleteLater();
            });
    group->start();
}

void ControllerMainWindow::SetAnimationLevel(int level)
{
    level = std::clamp(level, 0, 2);
    SaveUiAnimationLevel(level);
    if (level == 0) {
        SetBusyStatusAnimation(connectivityPill_, false);
        SetBusyStatusAnimation(serviceStatus_, false);
    }
    const QString label = level == 0
        ? QStringLiteral("关闭")
        : (level == 1 ? QStringLiteral("简洁")
                      : QStringLiteral("完整"));
    RemoteCToast::Show(
        this, QStringLiteral("界面动画已设为“%1”").arg(label),
        RemoteCToast::Tone::kSuccess);
}

}  // namespace remote::controller
