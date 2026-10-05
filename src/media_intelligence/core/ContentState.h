// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>

#include "ScreenScene.h"

namespace remote::media_intelligence {

enum class ScreenSemanticType : std::uint8_t {
    kUnknown,
    kTextUi,
    kMixedUi,
    kVideo,
};

enum class ScreenMotionLevel : std::uint8_t {
    kUnknown,
    kIdle,
    kLow,
    kMedium,
    kHigh,
};

struct ContentState {
    ScreenSemanticType semantic = ScreenSemanticType::kUnknown;
    ScreenMotionLevel motion = ScreenMotionLevel::kUnknown;

    float semanticConfidence = 0.0f;
    float motionScore = 0.0f;
    float changedAreaRatio = 0.0f;
    float textScore = 0.0f;
    float mixedUiScore = 0.0f;
    float videoScore = 0.0f;

    std::uint64_t sourceFrameId = 0;
    std::uint64_t timestampMs = 0;
    std::uint32_t inferenceTimeUs = 0;
    bool modelResultAvailable = false;
    ScreenScene scene = ScreenScene::kUnknown;
};

// Model semantics expire independently from rule-based motion. This lets a
// fresh capture-activity sample remain useful when a model result is late.
ContentState ApplyContentStateStaleness(
    ContentState state,
    std::uint64_t nowMs,
    std::uint32_t semanticMaximumAgeMs = 2000) noexcept;

const char* ScreenSemanticTypeName(ScreenSemanticType type) noexcept;
const char* ScreenMotionLevelName(ScreenMotionLevel level) noexcept;

}  // namespace remote::media_intelligence
