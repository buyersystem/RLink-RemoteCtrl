// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ControllerMainWindow.h"
#include <QStackedWidget>

#include "pages/DirectConnectPage.h"
#include "pages/OwnedDevicesPage.h"
#include "pages/RecentConnectionsPage.h"

namespace remote::controller {
void ControllerMainWindow::BuildDeviceAndRecentPages()
{
    directConnectPage_ = new DirectConnectPage(pageStack_);
    localDevicePage_ = directConnectPage_;
    pageStack_->addWidget(directConnectPage_);

    ownedDevicesPage_ = new OwnedDevicesPage(pageStack_);
    pageStack_->addWidget(ownedDevicesPage_);

    recentConnectionsPage_ = new RecentConnectionsPage(pageStack_);
    pageStack_->addWidget(recentConnectionsPage_);
}

}  // namespace remote::controller
