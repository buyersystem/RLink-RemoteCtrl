// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ControllerMainWindow.h"
#include "ControllerMainWindowSupport.h"

#include "RemoteCToast.h"
#include "pages/OwnedDevicesPage.h"

namespace remote::controller {
using namespace detail;

void ControllerMainWindow::RefreshOwnedDevicesUi(
    const SessionEngineSnapshot& snapshot)
{
    if (ownedDevicesPage_) {
        ownedDevicesPage_->UpdateSnapshot(
            snapshot.ownedDevices, snapshot.connectivity);
    }
    RefreshRecentDevices(snapshot);
}

void ControllerMainWindow::StartOwnedDeviceSession(
    const QString& deviceId,
    const QString& deviceName)
{
    pendingDeviceName_ = deviceName;
    lastDirectSessionToastError_.clear();
    ownedDeviceSessionPending_ = true;
    const auto beforeConnect = engine_->Snapshot();

    DirectSessionConnectRequest request;
    request.targetDeviceId = deviceId.toStdString();
    request.purpose = SessionPurpose::kRemoteControl;
    request.authorization = DirectAuthorizationMethod::kOwnedAccount;
    const auto result = engine_->ConnectDirectDevice(request);
    if (!result.accepted) {
        ownedDeviceSessionPending_ = false;
        RemoteCToast::Show(
            this,
            LocalizedDirectSessionError(
                result.errorCode, result.errorMessage, &beforeConnect),
            RemoteCToast::Tone::kError);
    }
}

}  // namespace remote::controller
