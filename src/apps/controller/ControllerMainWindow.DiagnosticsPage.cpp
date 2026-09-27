// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ControllerMainWindow.h"

#include <QStackedWidget>

#include "RemoteCToast.h"
#include "pages/DiagnosticsPage.h"

namespace remote::controller {

void ControllerMainWindow::BuildDiagnosticsPage()
{
    debugPage_ = new DiagnosticsPage(pageStack_);
    connect(debugPage_, &DiagnosticsPage::ScreenFrameRateLogToggled,
            this, [this](bool enabled) {
                RemoteCToast::Show(
                    this,
                    enabled
                        ? QStringLiteral(
                              "FPS记录日志已开启，将写入程序目录")
                        : QStringLiteral("FPS记录日志已关闭"),
                    RemoteCToast::Tone::kInformation);
            });
    connect(debugPage_, &DiagnosticsPage::InputEventStatsToggled,
            this, [this](bool enabled) {
                RefreshDiagnosticsUi();
                RemoteCToast::Show(
                    this,
                    enabled
                        ? QStringLiteral("鼠标与键盘统计已开启")
                        : QStringLiteral("鼠标与键盘统计已关闭"),
                    RemoteCToast::Tone::kInformation);
            });
    connect(debugPage_, &DiagnosticsPage::RefreshRequested,
            this, &ControllerMainWindow::RefreshDiagnosticsUi);
    pageStack_->addWidget(debugPage_);
}

}  // namespace remote::controller
