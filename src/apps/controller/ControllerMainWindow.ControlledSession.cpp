// SPDX-License-Identifier: GPL-3.0-only
#include "ControllerMainWindow.h"
#include "ControlledSessionIndicator.h"
#include "RemoteCDialog.h"
#include "src/platform/win/WindowsInputExecutor.h"

#include <QTimer>

namespace remote::controller {
void ControllerMainWindow::UpdateControlledSessionIndicator(const SessionEngineSnapshot& snapshot)
{
    if (applicationExitPrepared_ || quitting_) {
        if (controlledSessionWindow_) controlledSessionWindow_->Reset();
        return;
    }
    if (!controlledSessionWindow_ && !ControlledSessionStatus::FromSnapshot(snapshot).identity.isEmpty()) {
        controlledSessionWindow_ = new ControlledSessionIndicator(titleBar_,
            [this](const QString& identity) { EndIndicatedRemoteControl(identity); });
        controlledSessionWindow_->setFont(font());
        controlledSessionWindow_->ApplyTheme(darkInterfaceTheme_);
    }
    if (controlledSessionWindow_) controlledSessionWindow_->UpdateSession(snapshot);
}

void ControllerMainWindow::EndIndicatedRemoteControl(const QString& identity)
{
    if (!engine_ || !controlledSessionWindow_ || applicationExitPrepared_) return;
    const auto status = ControlledSessionStatus::FromSnapshot(engine_->Snapshot());
    // An old click must never end a replacement session that started meanwhile.
    if (status.identity != identity) { UpdateControlledSessionIndicator(engine_->Snapshot()); return; }
    controlledSessionWindow_->SetEnding(true);
    if (inputExecutor_) inputExecutor_->ReleaseAllRemoteInputs();
    SessionCommandResult direct{true, {}, {}}, room{true, {}, {}};
    if (status.direct) direct = engine_->Disconnect();
    if (status.room) room = engine_->ReleaseRoomControl();
    UpdateControlledSessionIndicator(engine_->Snapshot());
    if (!direct.accepted || !room.accepted || !room.errorCode.empty()) {
        if (controlledSessionWindow_) controlledSessionWindow_->SetEnding(false);
        const QString detail = room.errorCode == "room_control_ended_locally"
            ? QStringLiteral("本机已停止接受远程控制输入。当前无法通知服务器，但不影响本机结束控制；房间共享和音视频继续保留。")
            : QString::fromStdString(!direct.accepted ? direct.errorMessage : room.errorMessage);
        QTimer::singleShot(0, this, [this, detail, failed = !direct.accepted || !room.accepted] {
            RemoteCDialog::Alert(this, failed ? QStringLiteral("结束远控未完成") : QStringLiteral("远控已在本机结束"),
                detail.isEmpty() ? QStringLiteral("请稍后重试，当前控制状态仍显示在顶部。") : detail);
        });
    }
}
} // namespace remote::controller
