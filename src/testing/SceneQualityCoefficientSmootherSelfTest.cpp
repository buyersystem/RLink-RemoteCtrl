// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "media_intelligence/core/SceneQualityCoefficientSmoother.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>

namespace {

using namespace remote::media_intelligence;

bool Near(double actual, double expected) { return std::abs(actual - expected) < 1e-10; }

bool Check(bool value, const char* name)
{
    std::cout << name << '=' << (value ? "PASS" : "FAIL") << '\n';
    return value;
}

}  // namespace

int main()
{
    bool passed = true;
    struct ExpectedScene {
        ScreenScene scene;
        double minimum;
        double maximum;
    };
    constexpr std::array<ExpectedScene, 9> scenes{{
        {ScreenScene::kCodeTerminal, 0.15, 0.35},
        {ScreenScene::kDocument, 0.15, 0.35},
        {ScreenScene::kSpreadsheet, 0.15, 0.35},
        {ScreenScene::kWebApp, 0.35, 0.65},
        {ScreenScene::kPhotoGraphics, 0.10, 0.30},
        {ScreenScene::kCadDiagram, 0.15, 0.35},
        {ScreenScene::kVideo, 0.60, 0.80},
        {ScreenScene::kGame3d, 0.70, 0.90},
        {ScreenScene::kMixed, 0.35, 0.65},
    }};
    bool allScenesMatch = true;
    SceneQualityCoefficientSmoother smoother;
    for (const auto& scene : scenes) {
        smoother.Reset(0.5, 0);
        smoother.SetScene(scene.scene, 100);
        const auto range = smoother.Range();
        allScenesMatch = allScenesMatch && range.known
            && Near(range.minimum, scene.minimum) && Near(range.maximum, scene.maximum)
            && Near(smoother.Target(), (scene.minimum + scene.maximum) * 0.5)
            && Near(smoother.Evaluate(1100), smoother.Target());
    }
    passed &= Check(allScenesMatch, "ALL_SCENE_RANGES_AND_MIDPOINTS");

    smoother.Reset(0.50, 1000);
    smoother.SetScene(ScreenScene::kCodeTerminal, 1000);
    passed &= Check(Near(smoother.Evaluate(1000), 0.50)
        && Near(smoother.Evaluate(1500), 0.375)
        && Near(smoother.Evaluate(2000), 0.25)
        && smoother.IsTransitioning(1999) && !smoother.IsTransitioning(2000),
        "LINEAR_ONE_SECOND_EXACT_COMPLETION");

    smoother.SetScene(ScreenScene::kCodeTerminal, 1600);
    passed &= Check(Near(smoother.Evaluate(2000), 0.25) && !smoother.IsTransitioning(2000),
        "REPEATED_SCENE_DOES_NOT_RESTART");

    smoother.Reset(0.50, 0);
    smoother.SetScene(ScreenScene::kCodeTerminal, 100);
    smoother.SetScene(ScreenScene::kDocument, 600);
    passed &= Check(smoother.Scene() == ScreenScene::kDocument
        && Near(smoother.Evaluate(600), 0.375)
        && Near(smoother.Evaluate(1100), 0.25) && !smoother.IsTransitioning(1100),
        "DIFFERENT_SCENE_SAME_MIDPOINT_KEEPS_DEADLINE");

    smoother.Reset(0.50, 0);
    smoother.SetScene(ScreenScene::kCodeTerminal, 100);
    const auto before = smoother.Evaluate(600);
    smoother.SetScene(ScreenScene::kGame3d, 600);
    passed &= Check(Near(smoother.Evaluate(600), before)
        && Near(smoother.Evaluate(1100), (before + 0.80) * 0.5)
        && Near(smoother.Evaluate(1600), 0.80),
        "INTERRUPTION_STARTS_AT_CURRENT_VALUE");

    smoother.SetScene(ScreenScene::kUnknown, 700);
    smoother.SetScene(static_cast<ScreenScene>(255), 800);
    passed &= Check(smoother.Scene() == ScreenScene::kGame3d
        && Near(smoother.Target(), 0.80) && Near(smoother.Evaluate(1600), 0.80)
        && !SceneQualityCoefficientSmoother::RangeForScene(ScreenScene::kUnknown).known,
        "UNKNOWN_SCENE_KEEPS_ACCEPTED_SCENE_AND_TARGET");

    smoother.SetTarget(0.50, 1100);
    passed &= Check(smoother.Scene() == ScreenScene::kUnknown && !smoother.Range().known
        && Near(smoother.Evaluate(1100), (before + 0.80) * 0.5)
        && Near(smoother.Evaluate(2100), 0.50),
        "HOST_FALLBACK_IS_SMOOTH");
    smoother.SetTarget(std::numeric_limits<double>::quiet_NaN(), 1500);
    smoother.SetTarget(std::numeric_limits<double>::infinity(), 1500);
    passed &= Check(Near(smoother.Target(), 0.50) && Near(smoother.Evaluate(2100), 0.50),
        "INVALID_TARGET_IGNORED");

    smoother.Reset(-1.0, 0);
    const auto lower = smoother.Evaluate(0);
    smoother.SetTarget(2.0, 0);
    bool bounded = Near(lower, 0.0) && Near(smoother.Target(), 1.0);
    for (std::uint64_t now = 0; now < 1500; now += 7) {
        bounded = bounded && smoother.Evaluate(now) >= 0.0 && smoother.Evaluate(now) <= 1.0;
    }
    smoother.Reset(std::numeric_limits<double>::quiet_NaN(), 1500);
    passed &= Check(bounded && Near(smoother.Evaluate(1500), 0.50),
        "FINITE_VALIDATION_AND_OUTPUT_BOUNDS");

    smoother.Reset(0.50, 1000);
    smoother.SetScene(ScreenScene::kGame3d, 1200);
    smoother.SetScene(ScreenScene::kCodeTerminal, 1100);
    smoother.SetTarget(0.0, 1100);
    passed &= Check(smoother.Scene() == ScreenScene::kGame3d
        && Near(smoother.Target(), 0.80) && Near(smoother.Evaluate(1100), 0.50)
        && smoother.IsTransitioning(1100) && Near(smoother.Evaluate(2200), 0.80),
        "CLOCK_ROLLBACK_CANNOT_WRAP_OR_RESTART");

    smoother.Reset(0.50, 0);
    smoother.SetScene(ScreenScene::kPhotoGraphics, 100);
    passed &= Check(Near(smoother.Evaluate(24u * 60u * 60u * 1000u), 0.20)
        && !smoother.IsTransitioning(24u * 60u * 60u * 1000u),
        "LONG_PAUSE_COMPLETES_WITHOUT_CATCHUP_TICKS");

    const auto almostMax = std::numeric_limits<std::uint64_t>::max() - 100;
    smoother.Reset(0.50, almostMax);
    smoother.SetScene(ScreenScene::kCodeTerminal, almostMax);
    passed &= Check(Near(smoother.Evaluate(std::numeric_limits<std::uint64_t>::max()), 0.475)
        && smoother.IsTransitioning(std::numeric_limits<std::uint64_t>::max()),
        "TIME_ARITHMETIC_DOES_NOT_ADD_OVERFLOWING_DEADLINE");

    SceneQualityCoefficientSmoother instant(0);
    instant.Reset(0.50, 0);
    instant.SetScene(ScreenScene::kVideo, 10);
    SceneQualityCoefficientSmoother custom(2000);
    custom.Reset(0.50, 0);
    custom.SetScene(ScreenScene::kCodeTerminal, 10);
    passed &= Check(Near(instant.Evaluate(10), 0.70) && !instant.IsTransitioning(10)
        && Near(custom.Evaluate(1010), 0.375) && Near(custom.Evaluate(2010), 0.25),
        "ZERO_AND_CUSTOM_TRANSITION_DURATION");

    smoother.Reset(0.50, 0);
    smoother.SetScene(ScreenScene::kVideo, 100);
    const auto evaluated = smoother.Evaluate(900);
    passed &= Check(Near(smoother.Evaluate(900), evaluated)
        && Near(smoother.Evaluate(200), 0.52),
        "EVALUATE_HAS_NO_OBSERVATION_SIDE_EFFECT");

    std::cout << "SCENE_QUALITY_COEFFICIENT_SMOOTHER=" << (passed ? "PASS" : "FAIL") << '\n';
    return passed ? 0 : 1;
}
