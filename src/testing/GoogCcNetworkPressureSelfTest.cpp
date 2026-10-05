// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "media_intelligence/core/GoogCcNetworkPressure.h"
#include "media_intelligence/core/ContentAwareStreamPolicy.h"

#include <iostream>
#include <limits>

namespace {
using namespace remote::media_intelligence;

bool Check(bool value, const char* name)
{
    std::cout << name << '=' << (value ? "PASS" : "FAIL") << '\n';
    return value;
}

GoogCcNetworkPressureInput Input(std::uint64_t now = 10'000)
{
    GoogCcNetworkPressureInput input;
    input.nowMs = input.targetTimestampMs = input.transportFeedbackTimestampMs =
        input.delayTimestampMs = now;
    input.routeGeneration = 1;
    input.controllerOutputAvailable = true;
    input.delayState = GoogCcDelayState::kNormal;
    input.targetBitrateBps = 3'000'000;
    return input;
}

StreamQualityEstimate Reference(const StreamQualityRequest& request, const void*) noexcept
{
    return {static_cast<std::uint64_t>(request.width) * request.height *
        request.frameRate / 10, true, false, true, true};
}

StreamQualityEstimate Calibrated(const StreamQualityRequest& request, const void*) noexcept
{
    auto estimate = Reference(request, nullptr);
    estimate.calibrated = true;
    return estimate;
}

bool PressureStates()
{
    bool ok = true;
    for (const bool delayEvent : {true, false}) {
        GoogCcNetworkPressureState transientState;
        auto transient = Input();
        transient.userSpecificationRestored = true;
        if (delayEvent) transient.lastDelayOveruseAtMs = 9500;
        else transient.lastCwndPushbackAtMs = 9500;
        auto event = EvaluateGoogCcNetworkPressure(transientState, transient);
        ok &= Check(event.episodeActive && event.pressureNow,
            delayEvent ? "OVERUSE_THEN_NORMAL_BETWEEN_STATS_IS_NOT_LOST"
                       : "PUSHBACK_THEN_ZERO_BETWEEN_STATS_IS_NOT_LOST");
        for (std::uint64_t now = 11'000; now <= 16'000; now += 1000) {
            transient.nowMs = transient.targetTimestampMs = transient.transportFeedbackTimestampMs =
                transient.delayTimestampMs = now;
            event = EvaluateGoogCcNetworkPressure(transientState, transient);
        }
        ok &= Check(!event.episodeActive,
            delayEvent ? "CONSUMED_OVERUSE_EVENT_DOES_NOT_RESTART_NORMAL_RECOVERY"
                       : "CONSUMED_PUSHBACK_EVENT_DOES_NOT_RESTART_NORMAL_RECOVERY");
        transientState = {};
        transient = Input();
        transient.lastDelayOveruseAtMs = 6999;
        transient.lastCwndPushbackAtMs = 6999;
        event = EvaluateGoogCcNetworkPressure(transientState, transient);
        ok &= Check(!event.episodeActive, "EXPIRED_TRANSIENT_CONGESTION_IS_NOT_AUTHORIZATION");
        transient.lastDelayOveruseAtMs = 9500;
        transient.transportFeedbackTimestampMs = 0;
        event = EvaluateGoogCcNetworkPressure(transientState, transient);
        ok &= Check(!event.episodeActive, "TRANSIENT_CONGESTION_STILL_REQUIRES_REAL_FRESH_FEEDBACK");
    }
    GoogCcNetworkPressureState state;
    auto input = Input();
    auto decision = EvaluateGoogCcNetworkPressure(state, input);
    ok &= Check(decision.sampleFresh && !decision.episodeActive && !decision.pressureNow,
        "HEALTHY_LOW_BUDGET_IS_NOT_PRESSURE");
    input.delayState = GoogCcDelayState::kUnderusing;
    decision = EvaluateGoogCcNetworkPressure(state, input);
    ok &= Check(!decision.episodeActive, "UNDERUSE_DOES_NOT_OPEN_EPISODE");
    input.delayState = GoogCcDelayState::kOverusing;
    decision = EvaluateGoogCcNetworkPressure(state, input);
    ok &= Check(decision.episodeActive && decision.pressureNow &&
        decision.reason == GoogCcNetworkPressureReason::kDelayOveruse,
        "FRESH_GCC_OVERUSE_OPENS_EPISODE");
    for (std::uint64_t now = 11'000; now <= 20'000; now += 1000) {
        input = Input(now);
        decision = EvaluateGoogCcNetworkPressure(state, input);
    }
    ok &= Check(decision.episodeActive && !decision.pressureNow,
        "NORMAL_AT_REDUCED_BUDGET_RETAINS_EPISODE");
    ok &= Check(state.lastPressureReason == GoogCcNetworkPressureReason::kDelayOveruse,
        "EPISODE_RETAINS_ORIGINAL_GCC_TRIGGER_FOR_DIAGNOSTICS");
    input = Input(21'000);
    input.userSpecificationRestored = true;
    decision = EvaluateGoogCcNetworkPressure(state, input);
    for (std::uint64_t now = 22'000; now <= 25'000; now += 1000) {
        input = Input(now);
        input.userSpecificationRestored = true;
        decision = EvaluateGoogCcNetworkPressure(state, input);
    }
    ok &= Check(decision.episodeActive, "RESTORED_NORMAL_NEEDS_FULL_CONFIRMATION");
    input = Input(26'000);
    input.userSpecificationRestored = true;
    decision = EvaluateGoogCcNetworkPressure(state, input);
    ok &= Check(!decision.episodeActive, "RESTORED_STABLE_NORMAL_CLEARS_EPISODE");
    ok &= Check(state.lastPressureReason == GoogCcNetworkPressureReason::kUnavailable,
        "EPISODE_CLEAR_RESETS_TRIGGER");

    state = {};
    input = Input();
    input.delayState = GoogCcDelayState::kUnknown;
    input.cwndReduceRatio = .20;
    decision = EvaluateGoogCcNetworkPressure(state, input);
    ok &= Check(decision.episodeActive && decision.pressureNow && !decision.delayFresh &&
        decision.reason == GoogCcNetworkPressureReason::kCongestionWindowPushback,
        "CWND_PUSHBACK_DOES_NOT_REQUIRE_DELAY_EVENT");
    input = Input(11'000);
    input.routeGeneration = 2;
    decision = EvaluateGoogCcNetworkPressure(state, input);
    ok &= Check(!decision.episodeActive, "ROUTE_CHANGE_DROPS_OLD_EPISODE");
    input.delayState = GoogCcDelayState::kOverusing;
    decision = EvaluateGoogCcNetworkPressure(state, input);
    input = Input(12'000);
    input.nowMs = 9000;
    decision = EvaluateGoogCcNetworkPressure(state, input);
    ok &= Check(!decision.episodeActive && !decision.sampleFresh,
        "CLOCK_REGRESSION_DROPS_OLD_EPISODE");
    return ok;
}

bool VerifiedSpecificationRecovery()
{
    ContentAwareStreamConfig config;
    config.qualityEstimator = Reference;
    config.allowReferenceModel = true;
    ContentAwareStreamInput input;
    input.sourceWidth = input.currentWidth = 1920;
    input.sourceHeight = input.currentHeight = 1080;
    input.maximumFrameRate = 60;
    input.currentFrameRate = 30;
    input.scene = ScreenScene::kCodeTerminal;
    input.activityAvailable = input.active = input.networkBudgetAvailable = true;
    input.networkConstrained = true;
    input.safeVideoBudgetBps = 10'000'000;
    input.nowMs = input.networkTimestampMs = 10'000;
    input.generation = input.networkGeneration = 1;
    input.currentDesiredVideoBitrateBps = input.currentSenderMaxBitrateBps = 10'000'000;
    input.qualityAvailable = input.qualityAcceptable = true;
    auto decision = RecommendContentAwareStream(input, config);
    bool ok = Check(decision.senderMaxFps < 60,
        "UNVERIFIED_REFERENCE_STILL_GATES_RECOVERY");
    input.userSpecificationVerifiedBitrateBps = 10'000'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.senderMaxFps == 60 && decision.requiredVideoBitrateBps == 10'000'000 &&
        decision.modelReference && !decision.modelCalibrated && decision.automaticControlEligible,
        "VERIFIED_USER_SPEC_CAN_RECOVER_WITHOUT_IMPOSSIBLE_REFERENCE_HEADROOM");
    config.qualityEstimator = Calibrated;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.senderMaxFps < 60 && decision.requiredVideoBitrateBps != 10'000'000,
        "VERIFIED_USER_REFERENCE_OVERRIDE_NEVER_REPLACES_CALIBRATED_MODEL");
    config.qualityEstimator = Reference;
    input.safeVideoBudgetBps = 9'999'999;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.senderMaxFps < 60,
        "VERIFIED_SPEC_CANNOT_EXCEED_REAL_BUDGET");
    input.safeVideoBudgetBps = 10'000'000;
    input.qualityAcceptable = false;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.senderMaxFps <= input.currentFrameRate,
        "VERIFIED_SPEC_STILL_REQUIRES_CURRENT_GOOD_QUALITY");
    input.qualityAcceptable = true;
    ContentAwareStreamPolicyState state;
    auto evaluation = EvaluateContentAwareStream(state, input, config);
    input.nowMs = input.networkTimestampMs = 11'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(!evaluation.confirmed, "VERIFIED_RECOVERY_RETAINS_TIME_HYSTERESIS");
    input.nowMs = input.networkTimestampMs = 12'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(evaluation.confirmed && evaluation.recommendation.senderMaxFps == 60,
        "VERIFIED_RECOVERY_CONFIRMS_AFTER_THREE_WINDOWS_AND_TWO_SECONDS");
    return ok;
}

bool Freshness()
{
    bool ok = true;
    for (unsigned mode = 0; mode != 6; ++mode) {
        GoogCcNetworkPressureState state;
        auto input = Input();
        input.delayState = GoogCcDelayState::kOverusing;
        if (mode == 0) input.transportFeedbackTimestampMs = 6999;
        if (mode == 1) input.transportFeedbackTimestampMs = 10'001;
        if (mode == 2) input.delayTimestampMs = 6999;
        if (mode == 3) input.delayState = GoogCcDelayState::kUnknown;
        if (mode == 4) input.transportFeedbackTimestampMs = 0;
        if (mode == 5) input.delayTimestampMs = 0;
        const auto decision = EvaluateGoogCcNetworkPressure(state, input);
        ok &= Check(!decision.episodeActive && !decision.pressureNow,
            "STALE_FUTURE_UNKNOWN_OR_ABSENT_DELAY_FEEDBACK_NEVER_TRIGGERS");
    }
    for (double ratio : {-1.0, 1.01, std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::infinity()}) {
        GoogCcNetworkPressureState state;
        auto input = Input();
        input.cwndReduceRatio = ratio;
        ok &= Check(!EvaluateGoogCcNetworkPressure(state, input).episodeActive,
            "INVALID_CWND_RATIO_NEVER_TRIGGERS");
    }
    for (unsigned mode = 0; mode != 3; ++mode) {
        GoogCcNetworkPressureState state;
        auto input = Input();
        input.delayState = GoogCcDelayState::kUnknown;
        input.cwndReduceRatio = .20;
        if (mode == 0) input.targetTimestampMs = 6999;
        if (mode == 1) input.transportFeedbackTimestampMs = 6999;
        if (mode == 2) input.controllerOutputAvailable = false;
        ok &= Check(!EvaluateGoogCcNetworkPressure(state, input).episodeActive,
            "CWND_PUSHBACK_REQUIRES_FRESH_CONTROLLER_AND_FEEDBACK");
    }
    GoogCcNetworkPressureState state;
    auto input = Input();
    input.delayState = GoogCcDelayState::kOverusing;
    auto decision = EvaluateGoogCcNetworkPressure(state, input);
    input = Input(14'000);
    input.transportFeedbackTimestampMs = 10'000;
    input.delayState = GoogCcDelayState::kOverusing;
    decision = EvaluateGoogCcNetworkPressure(state, input);
    ok &= Check(decision.episodeActive && !decision.sampleFresh && !decision.pressureNow,
        "PERIODIC_CONTROLLER_OUTPUT_CANNOT_RENEW_STALE_FEEDBACK");
    input = Input(15'000);
    input.userSpecificationRestored = true;
    decision = EvaluateGoogCcNetworkPressure(state, input);
    input = Input(21'000);
    input.userSpecificationRestored = true;
    decision = EvaluateGoogCcNetworkPressure(state, input);
    ok &= Check(decision.episodeActive,
        "UNOBSERVED_NORMAL_GAP_RESTARTS_CONFIRMATION");
    return ok;
}

bool PolicyIntegration()
{
    bool ok = true;
    ContentAwareStreamConfig config;
    config.qualityEstimator = Reference;
    config.allowReferenceModel = true;
    ContentAwareStreamInput input;
    input.sourceWidth = input.currentWidth = 1920;
    input.sourceHeight = input.currentHeight = 1080;
    input.maximumFrameRate = input.currentFrameRate = 60;
    input.scene = ScreenScene::kCodeTerminal;
    input.activityAvailable = input.active = input.networkBudgetAvailable = true;
    input.safeVideoBudgetBps = 5'000'000;
    input.nowMs = input.networkTimestampMs = 10'000;
    input.generation = input.networkGeneration = 1;
    input.currentDesiredVideoBitrateBps = input.currentSenderMaxBitrateBps = 12'000'000;
    input.qualityAvailable = input.qualityAcceptable = true;
    GoogCcNetworkPressureState pressure;
    auto gcc = Input();
    auto observation = EvaluateGoogCcNetworkPressure(pressure, gcc);
    input.networkConstrained = observation.episodeActive && observation.sampleFresh;
    auto decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width == 1920 && decision.senderMaxFps == 60 &&
        !decision.networkRepairRequired, "LOW_BWE_WITHOUT_GCC_PRESSURE_PRESERVES_USER_SPEC");
    gcc.delayState = GoogCcDelayState::kOverusing;
    observation = EvaluateGoogCcNetworkPressure(pressure, gcc);
    input.networkConstrained = observation.episodeActive && observation.sampleFresh;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.senderMaxFps == 50 && decision.width <= 1920 &&
        decision.estimatedFeasible && decision.requiredVideoBitrateBps <= 5'000'000 &&
        decision.automaticControlEligible && decision.networkRepairRequired,
        "GCC_OVERUSE_CAN_REPAIR_BEFORE_QP_DETERIORATES");
    input.qualityAvailable = false;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.senderMaxFps < 60 && decision.automaticControlEligible &&
        decision.networkRepairRequired && !decision.qualityRequirementMet,
        "GCC_OVERUSE_CAN_REPAIR_BEFORE_QP_ARRIVES");
    input.currentFrameRate = decision.senderMaxFps;
    gcc = Input(11'000);
    observation = EvaluateGoogCcNetworkPressure(pressure, gcc);
    input.networkConstrained = observation.episodeActive && observation.sampleFresh;
    input.nowMs = input.networkTimestampMs = 11'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.senderMaxFps == input.currentFrameRate,
        "NORMAL_LOW_BUDGET_NEVER_IMMEDIATELY_RESTORES_FPS");
    input.safeVideoBudgetBps = 100'000'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.senderMaxFps == input.currentFrameRate,
        "RECOVERY_UPGRADE_STILL_REQUIRES_QUALITY_EVIDENCE");
    input.qualityAvailable = input.qualityAcceptable = true;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.senderMaxFps == 60 && !decision.networkRepairRequired,
        "FRESH_QUALITY_AND_HEADROOM_ALLOW_USER_SPEC_RESTORATION");
    return ok;
}

}  // namespace

int main()
{
    const bool states = PressureStates();
    const bool freshness = Freshness();
    const bool policy = PolicyIntegration();
    const bool recovery = VerifiedSpecificationRecovery();
    return states && freshness && policy && recovery ? 0 : 1;
}
