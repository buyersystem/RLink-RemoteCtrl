// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <string>
#include <utility>

#include <QString>

#include "src/core/MediaDevice.h"
#include "src/protocol/ScreenShareControlProtocol.h"

class QToolButton;
class QWidget;

namespace remote::controller {

QToolButton* MakeToolButton(
    const QString& text,
    const QString& tooltip,
    QWidget* parent);
QString ScreenQualityText(ScreenQualityTier quality);
QString CaptureBackendText(const std::string& backend);
QString ActiveMediaDeviceText(
    const MediaDeviceCategorySnapshot& category);
std::pair<std::uint32_t, std::uint32_t> ScreenQualityBounds(
    ScreenQualityTier quality);

}  // namespace remote::controller
