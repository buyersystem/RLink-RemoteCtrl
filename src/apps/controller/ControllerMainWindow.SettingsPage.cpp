// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ControllerMainWindow.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QStackedWidget>
#include "RemoteCComboBox.h"
#include "pages/SettingsPage.h"

namespace remote::controller {

void ControllerMainWindow::BuildSettingsPage()
{
    settingsPage_ = new SettingsPage(pageStack_);
    auto* settingsPage = settingsPage_;
    connect(settingsPage_, &SettingsPage::ScreenVideoBitrateBppChanged,
            this, [this](std::uint32_t hundredths) {
                if (engine_) {
                    const auto result = engine_->SetScreenVideoBitrateBpp(hundredths);
                    if (!result.accepted) {
                        settingsPage_->Controls().screenVideoBitrateBppInput->setToolTip(
                            QStringLiteral("设置已保存，动态应用失败：%1").arg(QString::fromStdString(result.errorMessage)));
                    }
                }
            });
    connect(settingsPage_, &SettingsPage::ScreenQualityDeficitShareChanged,
            this, [this](std::uint32_t hundredths) {
                if (engine_) {
                    const auto result = engine_->SetScreenQualityDeficitShare(hundredths);
                    if (!result.accepted) {
                        settingsPage_->Controls().screenQualityDeficitShareInput->setToolTip(
                            QStringLiteral("设置已保存，动态应用失败：%1").arg(QString::fromStdString(result.errorMessage)));
                    }
                }
            });
    connect(settingsPage_, &SettingsPage::SoftwareUpdateRequested,
            this, [this] { OpenSoftwareUpdate(); });
    connect(settingsPage_, &SettingsPage::ClipboardCacheBaseDirectoryChanged,
            this, [this] {
                ApplyClipboardConfigurationFromUi(false);
                if (clipboardController_) {
                    (void)clipboardController_->RefreshCacheStatistics();
                }
            });

    RefreshEncoderBenchmarkSummary(false);
    RefreshDecoderBenchmarkSummary(false);
    RefreshDecoderHardwareSelectionAvailability(false);
    if (engine_) {
        UpdateVideoPipelineSettingsAvailability(engine_->Snapshot());
    }

    connect(settingsPage_, &SettingsPage::CategoryChanged, this,
            [this](const int index) {
                if (index == 1 && engine_) {
                    settingsPage_->UpdateScreenVideoTrafficEstimate(engine_->Diagnostics());
                } else if (index == 3) {
                    RequestMediaDeviceRefresh(false);
                } else if (index == 5) {
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
