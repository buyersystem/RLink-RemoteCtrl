// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <cstdint>
#include "media_intelligence/core/ScreenScene.h"

namespace remote {
// Only a confirmed semantic result may change the coefficient. All timestamps
// use the host's steady clock; motion is deliberately absent from this contract.
struct SceneQualityObservation {
    bool enabled = false;
    std::uint64_t generation = 0;
    std::uint64_t observedAtMs = 0;
    std::uint64_t sceneObservedAtMs = 0;
    std::uint64_t maximumSceneAgeMs = 65000;
    media_intelligence::ScreenScene scene = media_intelligence::ScreenScene::kUnknown;
};
} // namespace remote
