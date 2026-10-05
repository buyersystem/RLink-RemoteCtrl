// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <cstdint>
#include "media_intelligence/core/ContentAwareStreamPolicy.h"
#include "src/core/ScreenNetworkPolicy.h"

namespace remote {

// Metadata only. Capture and inference never call an RTP sender directly.
struct ScreenContentPolicyObservation {
    bool enabled = false;
    std::uint64_t generation = 0;
    std::uint64_t observedAtMs = 0;
    std::uint32_t sourceWidth = 0;
    std::uint32_t sourceHeight = 0;
    media_intelligence::ScreenScene scene = media_intelligence::ScreenScene::kUnknown;
    ScreenContentActivity activity = ScreenContentActivity::kUnknown;
    // Explicit host evidence; missing metrics must not be treated as healthy.
    bool qualityAvailable = false;
    bool qualityAcceptable = false;
    bool processingAvailable = false;
    bool processingHealthy = false;
};

}  // namespace remote
