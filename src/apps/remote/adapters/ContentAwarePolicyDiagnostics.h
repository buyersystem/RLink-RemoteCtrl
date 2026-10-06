// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>

#include "src/webrtc/SceneQualityObservation.h"
#include "media_intelligence/core/ContentState.h"

namespace remote::app {

// Only the analyzer's confirmed semantic result is forwarded. Capture motion,
// theoretical bitrate demand, and diagnostics reads never select a scene.
SceneQualityObservation BuildSceneQualityObservation(
    bool enabled, std::uint64_t generation,
    const media_intelligence::ContentState& contentState,
    std::uint64_t nowMs, std::uint64_t maximumSceneAgeMs = 65000) noexcept;

}  // namespace remote::app
