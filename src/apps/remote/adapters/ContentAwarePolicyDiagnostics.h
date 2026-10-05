// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include "src/core/SessionDiagnostics.h"
#include "src/webrtc/ScreenContentPolicy.h"
#include "media_intelligence/core/ContentState.h"

namespace remote::app {

// Capture activity is independent from the last confirmed semantic identity.
// A missing classifier result stays unknown; it is never promoted to mixed.
ScreenContentPolicyObservation BuildScreenContentPolicyObservation(
    bool enabled, std::uint64_t generation,
    std::uint32_t sourceWidth, std::uint32_t sourceHeight,
    ScreenContentActivity activity,
    const media_intelligence::ContentState& contentState,
    std::uint64_t nowMs) noexcept;

// Runs only during diagnostics collection, on outbound screen streams whose
// content analyzer is enabled. Never schedules capture or changes a sender.
void AnnotateContentAwarePolicyShadow(
    WebRtcSessionStatsSnapshot& stats, std::uint64_t nowMs);

}  // namespace remote::app
