// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>

#include "ContentState.h"

namespace remote::media_intelligence {

enum class CaptureActivity : std::uint8_t {
    kUnknown,
    kStarting,
    kActive,
    kIdle,
};

struct RuleMotionSample {
    CaptureActivity activity = CaptureActivity::kUnknown;
    float changedAreaRatio = 0.0f;
    float changedFramesPerSecond = 0.0f;
    bool inputBoostActive = false;
    std::uint64_t sourceFrameId = 0;
    std::uint64_t timestampMs = 0;
};

struct MotionRuleConfig {
    float referenceChangedFramesPerSecond = 30.0f;
    float mediumScore = 0.16f;
    float highScore = 0.45f;
};

// The rule analyzer deliberately classifies motion only. Semantic type stays
// unknown until a model backend produces a validated result.
ContentState AnalyzeRuleMotion(
    const RuleMotionSample& sample,
    const MotionRuleConfig& config = {}) noexcept;

}  // namespace remote::media_intelligence
