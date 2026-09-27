// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ControllerMainWindow.h"

#include <QComboBox>
#include <QStackedWidget>
#include "RemoteCComboBox.h"
#include "pages/SettingsPage.h"

namespace remote::controller {

void ControllerMainWindow::BuildSettingsPage()
{
    settingsPage_ = new SettingsPage(pageStack_);
    auto* settingsPage = settingsPage_;
    connect(settingsPage_, &SettingsPage::SoftwareUpdateRequested,
            this, [this] { OpenSoftwareUpdate(); });
    connect(settingsPage_, &SettingsPage::ClipboardCacheBaseDirectoryChanged,
            this, [this] {
                ApplyClipboardConfigurationFromUi(false);
                if (clipboardController_) {
                    (void)clipboardController_->RefreshCacheStatistics();
                }
            });

    RefreshEncoderBenchmarkSummary();
    RefreshDecoderBenchmarkSummary();
    RefreshDecoderHardwareSelectionAvailability();
    if (engine_) {
        UpdateVideoPipelineSettingsAvailability(engine_->Snapshot());
    }

    connect(settingsPage_, &SettingsPage::CategoryChanged, this,
            [this](const int index) {
                if (index == 2) {
                    RequestMediaDeviceRefresh(false);
                } else if (index == 4) {
                    settingsPage_->RefreshClipboardCacheCapacityOptions();
                    ApplyClipboardConfigurationFromUi(false);
                    if (clipboardController_) {
                        (void)clipboardController_
                            ->RefreshCacheStatistics();
                    }
                }
            });
    // Let wheel events continue to the settings scroll area unless a combo's
    // popup is open, preventing accidental value changes while scrolling.
    const auto settingsSelectors = settingsPage->findChildren<QComboBox*>();
    for (auto* selector : settingsSelectors) {
        if (auto* remoteSelector =
                dynamic_cast<RemoteCComboBox*>(selector)) {
            remoteSelector->SetWheelSelectionEnabled(false);
        }
    }
    pageStack_->addWidget(settingsPage);
}

}  // namespace remote::controller
