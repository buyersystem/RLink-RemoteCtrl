// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ContentState.h"

namespace remote::media_intelligence {

ContentState ApplyContentStateStaleness(
    ContentState state,
    std::uint64_t nowMs,
    std::uint32_t semanticMaximumAgeMs) noexcept
{
    const bool expired = state.timestampMs == 0 ||
        nowMs < state.timestampMs ||
        nowMs - state.timestampMs > semanticMaximumAgeMs;
    if (expired) {
        state.semantic = ScreenSemanticType::kUnknown;
        state.scene = ScreenScene::kUnknown;
        state.semanticConfidence = 0.0f;
        state.textScore = 0.0f;
        state.mixedUiScore = 0.0f;
        state.videoScore = 0.0f;
        state.modelResultAvailable = false;
    }
    return state;
}

const char* ScreenSemanticTypeName(ScreenSemanticType type) noexcept
{
    switch (type) {
    case ScreenSemanticType::kTextUi:
        return "text_ui";
    case ScreenSemanticType::kMixedUi:
        return "mixed_ui";
    case ScreenSemanticType::kVideo:
        return "video";
    case ScreenSemanticType::kUnknown:
    default:
        return "unknown";
    }
}

const char* ScreenMotionLevelName(ScreenMotionLevel level) noexcept
{
    switch (level) {
    case ScreenMotionLevel::kIdle:
        return "idle";
    case ScreenMotionLevel::kLow:
        return "low";
    case ScreenMotionLevel::kMedium:
        return "medium";
    case ScreenMotionLevel::kHigh:
        return "high";
    case ScreenMotionLevel::kUnknown:
    default:
        return "unknown";
    }
}

}  // namespace remote::media_intelligence
