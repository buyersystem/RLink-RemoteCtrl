// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>

#include "ScreenScene.h"

namespace remote::media_intelligence {

struct SceneQualityCoefficientRange {
    double minimum = 0.0;
    double maximum = 1.0;
    bool known = false;

    constexpr double Midpoint() const noexcept { return (minimum + maximum) * 0.5; }
};

// A pure, host-clock-driven coefficient transition. The host confirms semantic
// scenes upstream and publishes Evaluate() to its existing encoder parameters.
// This class has no network, motion, capture, thread, or timer dependencies.
// All mutation calls must be serialized; now is a monotonic millisecond clock.
class SceneQualityCoefficientSmoother {
public:
    explicit SceneQualityCoefficientSmoother(
        std::uint64_t durationMs = 1000) noexcept;

    // Finite values are clamped to [0, 1]; an invalid initial value uses 0.50.
    void Reset(double initialK, std::uint64_t now) noexcept;
    // Unknown and invalid scenes keep the last accepted scene and target.
    void SetScene(ScreenScene scene, std::uint64_t now) noexcept;
    // Host-directed fallback/manual transition; non-finite values are ignored.
    // This clears the scene identity, without making a transport decision.
    void SetTarget(double targetK, std::uint64_t now) noexcept;

    // Side-effect-free: late calls skip directly to their time's value. A time
    // before the transition start returns its start value rather than wrapping.
    double Evaluate(std::uint64_t now) const noexcept;
    bool IsTransitioning(std::uint64_t now) const noexcept;
    std::uint64_t RemainingMs(std::uint64_t now) const noexcept {
        if (!IsTransitioning(now)) return 0;
        return now <= startTimeMs_ ? durationMs_ : durationMs_ - (now - startTimeMs_);
    }

    ScreenScene Scene() const noexcept { return scene_; }
    double Target() const noexcept { return targetK_; }
    SceneQualityCoefficientRange Range() const noexcept { return RangeForScene(scene_); }
    std::uint64_t DurationMs() const noexcept { return durationMs_; }
    static SceneQualityCoefficientRange RangeForScene(ScreenScene scene) noexcept;

private:
    void BeginTransition(double targetK, std::uint64_t now) noexcept;

    const std::uint64_t durationMs_;
    ScreenScene scene_ = ScreenScene::kUnknown;
    double startK_ = 0.50;
    double targetK_ = 0.50;
    std::uint64_t startTimeMs_ = 0;
    std::uint64_t lastMutationMs_ = 0;
};

}  // namespace remote::media_intelligence
