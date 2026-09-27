// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ControllerMainWindow.h"
#include "ControllerMainWindowSupport.h"
#include "pages/OwnedDevicesPage.h"

namespace remote::controller {
using namespace detail;

void ControllerMainWindow::BuildUi()
{
    BuildShellAndRoomPage();
    BuildDeviceAndRecentPages();
    BuildDiagnosticsPage();
    BuildSettingsPage();
    BuildHelpAndAuthorPages();
    ConnectUiSignals();
}

}  // namespace remote::controller
