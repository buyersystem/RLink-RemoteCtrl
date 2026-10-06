// SPDX-License-Identifier: GPL-3.0-only
#include "src/apps/controller/ControlledSessionIndicator.h"
#include <QApplication>
#include <QEventLoop>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QTimer>
#include <iostream>
#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

using namespace remote;
using namespace remote::controller;
int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("ControlledSessionUiSelfTest");
    int failures = 0;
    const auto check = [&](bool ok, const char* message) {
        std::cout << (ok ? "PASS " : "FAIL ") << message << '\n';
        if (!ok) ++failures;
    };
    QString ended;
    QWidget titleBar;
    titleBar.resize(1000, 40);
    titleBar.show();
    ControlledSessionIndicator window(&titleBar, [&](QString identity) { ended = identity; });
    SessionEngineSnapshot s;
    s.localDeviceId = "local";
    s.sessionId = "direct-1";
    s.purpose = SessionPurpose::kRemoteControl;
    s.remoteControlRole = RemoteControlRole::kControlled;
    s.state = SessionEngineState::kConnecting;
    window.UpdateSession(s);
    check(!window.isVisible(), "initial negotiation does not show controlled indicator");
    s.state = SessionEngineState::kActive;
    s.remoteControlRole = RemoteControlRole::kController;
    window.UpdateSession(s);
    check(!window.isVisible(), "controller does not show controlled indicator");
    s.remoteControlRole = RemoteControlRole::kControlled;
    s.purpose = SessionPurpose::kCameraOnly;
    window.UpdateSession(s);
    check(!window.isVisible(), "camera-only session does not show controlled indicator");
    s.purpose = SessionPurpose::kRemoteControl;
#ifdef Q_OS_WIN
    const HWND foregroundBefore = GetForegroundWindow();
#endif
    window.UpdateSession(s);
    check(window.isVisible() && window.SessionIdentity() == "direct:direct-1", "controlled direct session appears");
    check(window.parentWidget() == &titleBar && !window.isWindow() &&
          !window.windowFlags().testFlag(Qt::WindowStaysOnTopHint),
          "indicator is an embedded child, not an independent topmost window");
#ifdef Q_OS_WIN
    check(GetForegroundWindow() == foregroundBefore, "showing indicator preserves foreground window");
#endif
    const auto geometry = window.geometry();
    window.ApplyTheme(false);
    const auto lightSize = window.size();
    if (app.arguments().contains("--screenshots"))
        check(window.grab().save(app.applicationDirPath() + "/controlled-session-light.png"), "save light preview");
    window.ApplyTheme(true);
    if (app.arguments().contains("--screenshots"))
        check(window.grab().save(app.applicationDirPath() + "/controlled-session-dark.png"), "save dark preview");
    check(window.size() == lightSize, "theme round trip preserves geometry");
    check(qAbs(geometry.center().x() - titleBar.rect().center().x()) <= 1 &&
          titleBar.rect().contains(window.geometry()), "indicator centered within title bar without clipping");
    titleBar.resize(1200, 40);
    app.processEvents();
    check(qAbs(window.geometry().center().x() - titleBar.rect().center().x()) <= 1,
          "parent resize recenters embedded indicator");
    titleBar.hide();
    check(!window.isVisible(), "hiding main window also hides indicator");
    titleBar.show();
    check(window.isVisible(), "showing main window restores active indicator");
    check(!window.close() && window.isVisible(), "close cannot silently hide active safety indicator");
    QEventLoop wait;
    QTimer::singleShot(1200, &wait, &QEventLoop::quit);
    wait.exec();
    s.state = SessionEngineState::kConnecting;
    s.direct.sessionEverActive = true;
    window.UpdateSession(s);
    const auto* duration = window.findChild<QLabel*>("controlledDuration");
    check(window.isVisible() && window.SessionIdentity() == "direct:direct-1" &&
          duration->text() != "00:00:00", "recovery and repeated snapshots preserve elapsed time");
    window.findChild<QPushButton*>("endControl")->click();
    check(ended == "direct:direct-1", "end button supplies current identity");
    s.state = SessionEngineState::kReady;
    window.UpdateSession(s);
    check(!window.isVisible() && window.SessionIdentity().isEmpty() &&
          !window.findChild<QTimer*>()->isActive(), "end hides indicator and stops timer");
    s.state = SessionEngineState::kActive;
    s.sessionId = "direct-2";
    window.UpdateSession(s);
    check(duration->text() == "00:00:00", "new direct session resets elapsed time");
    s.state = SessionEngineState::kReady;
    s.room.roomId = "room";
    s.room.membership = RoomMembershipState::kActive;
    s.room.screenShareState = RoomScreenShareState::kActive;
    s.room.screenSharerDeviceId = "local";
    s.room.activeControllerDeviceId = "peer";
    window.UpdateSession(s);
    check(!window.isVisible(), "room watching without authenticated grant is not control");
    s.roomControlGrantActive = true;
    window.UpdateSession(s);
    check(window.isVisible(), "room sharer with active controller is controlled");
    const auto roomId = window.SessionIdentity();
    s.room.screenShareState = RoomScreenShareState::kRecovering;
    window.UpdateSession(s);
    check(window.isVisible() && window.SessionIdentity() == roomId, "room recovery preserves identity");
    s.room.activeControllerDeviceId = "local";
    window.UpdateSession(s);
    check(!window.isVisible(), "local room controller is not controlled");
    s.room.activeControllerDeviceId = "peer";
    s.room.screenSharerDeviceId = "other";
    window.UpdateSession(s);
    check(!window.isVisible(), "another room sharer does not show local indicator");
    window.Reset();
    std::cout << "RESULT controlled_session_ui " << failures << " failures\n";
    return failures ? 1 : 0;
}
