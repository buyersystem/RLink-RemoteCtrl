// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "SceneQualityCoefficientSmoother.h"

#include <algorithm>
#include <cmath>

namespace remote::media_intelligence {

SceneQualityCoefficientSmoother::SceneQualityCoefficientSmoother(
    std::uint64_t durationMs) noexcept
    : durationMs_(durationMs)
{
}

SceneQualityCoefficientRange SceneQualityCoefficientSmoother::RangeForScene(
    ScreenScene scene) noexcept
{
    switch (scene) {
    case ScreenScene::kCodeTerminal:
    case ScreenScene::kDocument:
    case ScreenScene::kSpreadsheet:
    case ScreenScene::kCadDiagram:
        return {0.15, 0.35, true};
    case ScreenScene::kWebApp:
    case ScreenScene::kMixed:
        return {0.35, 0.65, true};
    case ScreenScene::kPhotoGraphics:
        return {0.10, 0.30, true};
    case ScreenScene::kVideo:
        return {0.60, 0.80, true};
    case ScreenScene::kGame3d:
        return {0.70, 0.90, true};
    case ScreenScene::kUnknown:
    default:
        return {};
    }
}

void SceneQualityCoefficientSmoother::Reset(double initialK, std::uint64_t now) noexcept
{
    startK_ = std::isfinite(initialK) ? std::clamp(initialK, 0.0, 1.0) : 0.50;
    targetK_ = startK_;
    startTimeMs_ = now;
    lastMutationMs_ = now;
    scene_ = ScreenScene::kUnknown;
}

void SceneQualityCoefficientSmoother::BeginTransition(
    double targetK, std::uint64_t now) noexcept
{
    lastMutationMs_ = now;
    // Equal midpoints, including different scenes, must not extend the ramp.
    if (std::abs(targetK - targetK_) <= 1e-12) {
        return;
    }
    startK_ = Evaluate(now);
    targetK_ = targetK;
    startTimeMs_ = now;
}

void SceneQualityCoefficientSmoother::SetScene(
    ScreenScene scene, std::uint64_t now) noexcept
{
    const auto range = RangeForScene(scene);
    if (!range.known || now < lastMutationMs_) {
        return;
    }
    scene_ = scene;
    BeginTransition(range.Midpoint(), now);
}

void SceneQualityCoefficientSmoother::SetTarget(
    double targetK, std::uint64_t now) noexcept
{
    if (!std::isfinite(targetK) || now < lastMutationMs_) {
        return;
    }
    scene_ = ScreenScene::kUnknown;
    BeginTransition(std::clamp(targetK, 0.0, 1.0), now);
}

double SceneQualityCoefficientSmoother::Evaluate(std::uint64_t now) const noexcept
{
    if (durationMs_ == 0 || startK_ == targetK_) {
        return targetK_;
    }
    if (now <= startTimeMs_) {
        return startK_;
    }
    const auto elapsedMs = now - startTimeMs_;
    if (elapsedMs >= durationMs_) {
        return targetK_;
    }
    const auto progress = static_cast<double>(elapsedMs) / static_cast<double>(durationMs_);
    return std::clamp(startK_ + (targetK_ - startK_) * progress, 0.0, 1.0);
}

bool SceneQualityCoefficientSmoother::IsTransitioning(std::uint64_t now) const noexcept
{
    return durationMs_ != 0 && startK_ != targetK_
        && (now < startTimeMs_ || now - startTimeMs_ < durationMs_);
}

}  // namespace remote::media_intelligence
