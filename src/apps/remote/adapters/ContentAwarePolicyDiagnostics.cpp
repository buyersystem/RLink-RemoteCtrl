// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ContentAwarePolicyDiagnostics.h"

namespace remote::app {

SceneQualityObservation BuildSceneQualityObservation(
    bool enabled, std::uint64_t generation,
    const media_intelligence::ContentState& contentState,
    std::uint64_t nowMs, std::uint64_t maximumSceneAgeMs) noexcept
{
    SceneQualityObservation observation;
    observation.enabled = enabled;
    observation.generation = generation;
    observation.observedAtMs = nowMs;
    observation.maximumSceneAgeMs = maximumSceneAgeMs;
    if (enabled && contentState.modelResultAvailable && contentState.timestampMs != 0 &&
        contentState.timestampMs <= nowMs) {
        observation.scene = contentState.scene;
        // Preserve the actual model result age. Repeated statistics polling
        // must not make a failed/stopped classifier's old scene fresh again.
        observation.sceneObservedAtMs = contentState.timestampMs;
    }
    return observation;
}

}  // namespace remote::app
