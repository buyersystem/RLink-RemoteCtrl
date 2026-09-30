// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ContentMotionAnalyzer.h"

#include <algorithm>
#include <cmath>

namespace remote::media_intelligence {

ContentState AnalyzeRuleMotion(
    const RuleMotionSample& sample,
    const MotionRuleConfig& config) noexcept
{
    ContentState result;
    result.sourceFrameId = sample.sourceFrameId;
    result.timestampMs = sample.timestampMs;
    result.changedAreaRatio = std::clamp(
        sample.changedAreaRatio, 0.0f, 1.0f);

    if (sample.activity == CaptureActivity::kIdle) {
        result.motion = ScreenMotionLevel::kIdle;
        return result;
    }

    const float referenceRate = (std::max)(
        config.referenceChangedFramesPerSecond, 1.0f);
    const float frequency = std::clamp(
        sample.changedFramesPerSecond / referenceRate, 0.0f, 1.0f);
    // Area gates the frequency contribution so a blinking caret cannot be
    // classified as high motion merely because it changes often.
    result.motionScore = std::clamp(
        0.65f * std::sqrt(result.changedAreaRatio) * frequency +
            0.35f * result.changedAreaRatio,
        0.0f,
        1.0f);

    if (result.motionScore >= config.highScore) {
        result.motion = ScreenMotionLevel::kHigh;
    } else if (result.motionScore >= config.mediumScore) {
        result.motion = ScreenMotionLevel::kMedium;
    } else if (result.changedAreaRatio > 0.0f ||
               sample.changedFramesPerSecond > 0.0f ||
               sample.activity == CaptureActivity::kActive) {
        result.motion = ScreenMotionLevel::kLow;
    } else if (sample.activity == CaptureActivity::kStarting) {
        result.motion = ScreenMotionLevel::kUnknown;
    } else {
        result.motion = ScreenMotionLevel::kUnknown;
    }
    return result;
}

}  // namespace remote::media_intelligence
