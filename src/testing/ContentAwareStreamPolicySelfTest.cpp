// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "media_intelligence/core/ContentAwareStreamPolicy.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>

namespace {

using namespace remote::media_intelligence;

bool Check(bool value, const char* name)
{
    std::cout << name << '=' << (value ? "PASS" : "FAIL") << '\n';
    return value;
}

// A deliberately simple, calibrated TEST model. Production calibration must
// come from measured codec/encoder/scene data, not from these test numbers.
StreamQualityEstimate TestModel(const StreamQualityRequest& request,
    const void*) noexcept
{
    const auto pixels = static_cast<std::uint64_t>(request.width) * request.height;
    return {(pixels * request.frameRate + 9u) / 10u, true, true, true};
}

StreamQualityEstimate UnavailableModel(const StreamQualityRequest&,
    const void*) noexcept
{
    return {};
}

StreamQualityEstimate ReferenceModel(const StreamQualityRequest& request,
    const void* context) noexcept
{
    auto estimate = TestModel(request, context);
    estimate.calibrated = false;
    estimate.reference = true;
    return estimate;
}

StreamQualityEstimate SevereBudgetReferenceModel(const StreamQualityRequest& request,
    const void*) noexcept
{
    const auto pixels = static_cast<std::uint64_t>(request.width) * request.height;
    return {(pixels * request.frameRate * 3144 + 9999) / 10000,
        true, false, true, true};
}

StreamQualityEstimate EmptyReferenceModel(const StreamQualityRequest&,
    const void*) noexcept
{
    return {0, true, false, true, true};
}

StreamQualityEstimate MaximumEstimate(const StreamQualityRequest&,
    const void*) noexcept
{
    return {std::numeric_limits<std::uint64_t>::max(), true, true, true};
}

StreamQualityEstimate ProcessingModel(const StreamQualityRequest& request,
    const void* context) noexcept
{
    auto estimate = TestModel(request, context);
    estimate.processingFeasible = request.frameRate <= 60;
    return estimate;
}

ContentAwareStreamInput Base()
{
    ContentAwareStreamInput input;
    input.sourceWidth = input.currentWidth = 1920;
    input.sourceHeight = input.currentHeight = 1080;
    input.currentFrameRate = 120;
    input.maximumFrameRate = 120;
    input.currentDesiredVideoBitrateBps = 25'000'000;
    input.currentSenderMaxBitrateBps = 25'000'000;
    input.scene = ScreenScene::kCodeTerminal;
    input.activityAvailable = input.active = true;
    input.networkBudgetAvailable = true;
    input.networkConstrained = true; // Explicit simulated weak-network episode.
    input.safeVideoBudgetBps = 100'000'000;
    input.nowMs = input.networkTimestampMs = 10'000;
    input.generation = input.networkGeneration = 3;
    input.qualityAvailable = input.qualityAcceptable = true;
    input.processingAvailable = input.processingHealthy = true;
    return input;
}

bool Decisions()
{
    bool ok = true;
    ContentAwareStreamConfig config;
    config.qualityEstimator = TestModel;
    auto input = Base();
    auto decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width == 1920 && decision.height == 1080 &&
        decision.senderMaxFps == 120 && decision.automaticControlEligible,
        "FULL_HD_120_SUPPORTED");
    ok &= Check(decision.desiredVideoBitrateBps == 25'000'000 &&
        decision.senderMaxBitrateBps == 25'000'000,
        "DO_NOT_FILL_NETWORK_BUDGET");

    input.safeVideoBudgetBps = 22'000'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width == 1920 && decision.height == 1080 &&
        decision.senderMaxFps == 100 && decision.estimatedFeasible,
        "TEXT_PRESERVES_RESOLUTION_BEFORE_FPS");
    input.scene = ScreenScene::kGame3d;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width == 1760 && decision.height == 990 &&
        decision.senderMaxFps == 120 &&
        decision.reason == ContentAwareStreamReason::kProtectFrameRate,
        "MOTION_SMALL_SPATIAL_REDUCTION_PRESERVES_120");
    ok &= Check(decision.desiredVideoBitrateBps == 22'000'000 &&
        decision.senderMaxBitrateBps == 22'000'000,
        "REAL_CAPACITY_DROP_OVERRIDES_PREVIOUS_BITRATE");
    input.safeVideoBudgetBps = 25'000'000;
    input.qualityAcceptable = false;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width < 1920 && decision.width >= 1728 &&
        decision.senderMaxFps == 120 && decision.desiredVideoBitrateBps == 25'000'000 &&
        decision.senderMaxBitrateBps == 25'000'000 &&
        decision.automaticControlEligible && decision.qualityRepairRequired &&
        !decision.qualityRequirementMet,
        "POOR_QUALITY_EXECUTABLE_REPAIR_RETAINS_BITRATE_WITHOUT_CLAIMING_PASS");

    input = Base();
    input.scene = ScreenScene::kVideo;
    input.currentFrameRate = 60;
    input.safeVideoBudgetBps = 16'000'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width == 1920 && decision.height == 1080 &&
        decision.senderMaxFps == 60,
        "NO_SPATIAL_SACRIFICE_TO_REACH_NEW_FPS_PEAK");
    input.safeVideoBudgetBps = 100'000'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.senderMaxFps == 120 && decision.width == 1920,
        "FPS_UPGRADE_ONLY_WITH_HEADROOM");
    input.maximumFrameRate = 75;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.senderMaxFps == 75,
        "EXACT_75_FPS_REQUEST_IS_CANDIDATE");

    input = Base();
    input.scene = ScreenScene::kGame3d;
    input.currentWidth = 1760;
    input.currentHeight = 990;
    input.stableAnchorWidth = 1920;
    input.stableAnchorHeight = 1080;
    input.safeVideoBudgetBps = 18'000'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width >= 1728 && decision.height >= 972 &&
        decision.senderMaxFps < 120 && decision.estimatedFeasible,
        "CUMULATIVE_MOTION_REDUCTION_STOPS_AT_STABLE_ANCHOR_LIMIT");
    input.currentFrameRate = 60;
    input.safeVideoBudgetBps = 22'000'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width == 1920 && decision.height == 1080 &&
        decision.senderMaxFps <= 90,
        "RESTORE_ANCHOR_BEFORE_NEW_FPS_PEAK");

    input = Base();
    input.scene = ScreenScene::kUnknown;
    input.currentFrameRate = 60;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.senderMaxFps == 60 &&
        decision.reason == ContentAwareStreamReason::kUnknownSceneHold,
        "UNKNOWN_SCENE_DOES_NOT_UPGRADE");
    input.safeVideoBudgetBps = 5'000'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.estimatedFeasible && decision.senderMaxFps < 60,
        "UNKNOWN_SCENE_STILL_OBEYS_REAL_RESOURCE_CONSTRAINT");

    input = Base();
    input.qualityAvailable = false;
    input.currentFrameRate = 60;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.senderMaxFps == 60 && !decision.automaticControlEligible,
        "MISSING_QUALITY_IS_NOT_UPGRADE_PERMISSION");
    input = Base();
    decision = RecommendContentAwareStream(input);
    ok &= Check(decision.estimatedFeasible && !decision.modelCalibrated &&
        !decision.qualityRequirementMet && !decision.automaticControlEligible,
        "SEED_MODEL_IS_SHADOW_ONLY");
    input.processingAvailable = false;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.estimatedFeasible && decision.qualityRequirementMet &&
        decision.automaticControlEligible && !decision.processingRepairRequired,
        "MISSING_PROCESSING_EVIDENCE_DOES_NOT_BLOCK_QUALIFIED_NETWORK_DECISION");

    input = Base();
    input.resolutionManual = input.frameRateManual = true;
    input.requestedWidth = 1280;
    input.requestedHeight = 720;
    input.requestedFrameRate = 75;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width == 1280 && decision.height == 720 &&
        decision.senderMaxFps == 75,
        "MANUAL_PREFERENCES_PRESERVED");
    input.safeVideoBudgetBps = 1'000'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width == 1280 && decision.height == 720 &&
        decision.senderMaxFps == 75 && !decision.estimatedFeasible &&
        decision.senderMaxBitrateBps <= 1'000'000 &&
        decision.reason == ContentAwareStreamReason::kManualQualityLimited,
        "MANUAL_INFEASIBILITY_IS_EXPLICIT_AND_NETWORK_BOUNDED");

    config.qualityEstimator = ProcessingModel;
    input = Base();
    input.safeVideoBudgetBps = 22'000'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.senderMaxFps == 100 && decision.estimatedFeasible &&
        decision.automaticControlEligible && !decision.processingRepairRequired,
        "MODEL_PROCESSING_INFEASIBILITY_DOES_NOT_FILTER_WEAK_NETWORK_CANDIDATE");
    input.processingAvailable = true;
    input.processingHealthy = false;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.senderMaxFps == 100 && decision.automaticControlEligible &&
        !decision.processingRepairRequired,
        "UNHEALTHY_PROCESSING_DOES_NOT_CHANGE_NETWORK_ONLY_SCENE_TRADEOFF");
    return ok;
}

bool NetworkPressureGate()
{
    ContentAwareStreamConfig config;
    config.qualityEstimator = ReferenceModel;
    config.allowReferenceModel = true;
    auto input = Base();
    input.currentFrameRate = input.maximumFrameRate = 60;
    input.safeVideoBudgetBps = 5'000'000; // Seed alone would suggest much less.
    input.networkConstrained = false;
    auto decision = RecommendContentAwareStream(input, config);
    bool ok = Check(decision.width == 1920 && decision.height == 1080 &&
        decision.senderMaxFps == 60 && decision.senderMaxBitrateBps == 25'000'000 &&
        decision.reason == ContentAwareStreamReason::kHealthyHold,
        "HEALTHY_CURRENT_1080P60_IS_NOT_DOWNGRADED_BY_REFERENCE_DEMAND");
    input.scene = ScreenScene::kGame3d;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width == 1920 && decision.senderMaxFps == 60,
        "SCENE_CHANGE_WITHOUT_NETWORK_PRESSURE_PRESERVES_R_F_B");
    input.processingAvailable = true;
    input.processingHealthy = false;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width == 1920 && decision.height == 1080 &&
        decision.senderMaxFps == 60 && decision.senderMaxBitrateBps == 25'000'000 &&
        !decision.processingRepairRequired && decision.reason == ContentAwareStreamReason::kHealthyHold,
        "PROCESSING_SLOWDOWN_WITHOUT_WEAK_NETWORK_PRESERVES_USER_EXPERIENCE");
    input.processingAvailable = false;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width == 1920 && decision.senderMaxFps == 60 &&
        decision.reason == ContentAwareStreamReason::kHealthyHold,
        "MISSING_PROCESSING_WITHOUT_WEAK_NETWORK_PRESERVES_USER_EXPERIENCE");
    input.networkConstrained = true;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check((decision.senderMaxFps < 60 || decision.width < input.currentWidth) &&
        decision.senderMaxFps >= 50 && decision.senderMaxBitrateBps <= 5'000'000,
        "CONFIRMED_NETWORK_PRESSURE_STILL_USES_SCENE_TRADEOFFS");
    input.networkConstrained = false;
    input.currentFrameRate = 30;
    input.safeVideoBudgetBps = 100'000'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width == 1920 && decision.senderMaxFps == 60,
        "NETWORK_RECOVERY_CAN_RESTORE_USER_SPECIFICATION");
    ok &= Check(decision.width <= input.sourceWidth && decision.height <= input.sourceHeight &&
        decision.senderMaxFps <= input.maximumFrameRate &&
        decision.senderMaxBitrateBps <= input.safeVideoBudgetBps,
        "RECOVERY_WITH_MISSING_PROCESSING_STAYS_INSIDE_USER_R_F_AND_NETWORK_BOUNDS");
    input.safeVideoBudgetBps = 1'000'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width == 1920 && decision.senderMaxFps == 30 &&
        !decision.automaticControlEligible,
        "UNCERTAIN_RECOVERY_BUDGET_DOES_NOT_FORCE_ANOTHER_DOWNGRADE");
    input = Base();
    input.networkConstrained = false;
    input.qualityAcceptable = false;
    input.safeVideoBudgetBps = 1'000'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width == 1920 && decision.senderMaxFps == 120 &&
        !decision.automaticControlEligible,
        "BAD_QP_WITHOUT_NETWORK_PRESSURE_DOES_NOT_AUTHORIZE_R_F_LOSS");
    return ok;
}

bool SizesAndInvalidData()
{
    bool ok = true;
    ContentAwareStreamConfig config;
    config.qualityEstimator = TestModel;
    auto input = Base();
    input.sourceWidth = input.currentWidth = 3440;
    input.sourceHeight = input.currentHeight = 1440;
    input.maximumWidth = 2560;
    input.maximumHeight = 1080;
    auto decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width == 2560 && decision.height == 1070 &&
        decision.width % 2 == 0 && decision.height % 2 == 0 &&
        std::abs(static_cast<double>(decision.width) / input.sourceWidth -
            static_cast<double>(decision.height) / input.sourceHeight) < 0.002,
        "ULTRAWIDE_CAP_IS_PROPORTIONAL_NOT_STRETCHED");
    input = Base();
    input.scene = ScreenScene::kVideo;
    input.sourceWidth = input.currentWidth = 1080;
    input.sourceHeight = input.currentHeight = 1920;
    input.safeVideoBudgetBps = 22'000'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width == 990 && decision.height == 1760 &&
        decision.senderMaxFps == 120,
        "PORTRAIT_DENSE_REFERENCE_SIZES");
    input = Base();
    input.sourceWidth = input.currentWidth = 641;
    input.sourceHeight = input.currentHeight = 479;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width <= 641 && decision.height <= 479 &&
        decision.width % 2 == 0 && decision.height % 2 == 0,
        "SMALL_ODD_SOURCE_IS_NOT_UPSAMPLED");

    input = Base();
    input.active = false;
    input.safeVideoBudgetBps = 1;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width == 1920 && decision.senderMaxFps == 120 &&
        decision.senderMaxBitrateBps <= 1 &&
        decision.reason == ContentAwareStreamReason::kIdleHold,
        "IDLE_DOES_NOT_CHANGE_SPEC_OR_CLAIM_FEASIBILITY");
    input = Base();
    input.activityAvailable = false;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.senderMaxFps == 120 &&
        decision.reason == ContentAwareStreamReason::kActivityUnavailable,
        "MISSING_ACTIVITY_HOLDS");
    input = Base();
    input.networkBudgetAvailable = false;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.senderMaxFps == 120 && !decision.estimatedFeasible &&
        decision.reason == ContentAwareStreamReason::kCapacityUnavailable,
        "MISSING_NETWORK_HOLDS");
    input.networkBudgetAvailable = true;
    input.safeVideoBudgetBps = std::numeric_limits<double>::quiet_NaN();
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(!decision.automaticControlEligible &&
        decision.reason == ContentAwareStreamReason::kCapacityUnavailable,
        "NAN_CAPACITY_REJECTED");
    input.safeVideoBudgetBps = std::numeric_limits<double>::infinity();
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(!decision.estimatedFeasible,
        "INFINITE_CAPACITY_REJECTED");
    input.safeVideoBudgetBps = 0;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(!decision.estimatedFeasible && decision.senderMaxBitrateBps == 0 &&
        decision.reason == ContentAwareStreamReason::kQualityLimited,
        "ZERO_BUDGET_IS_NOT_INFINITE_CAPACITY");
    input = Base();
    input.networkTimestampMs = 5'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.reason == ContentAwareStreamReason::kStaleNetwork,
        "STALE_NETWORK_HOLDS");
    input.networkTimestampMs = input.nowMs + 1;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.reason == ContentAwareStreamReason::kStaleNetwork,
        "FUTURE_TIMESTAMP_HOLDS_WITHOUT_UNDERFLOW");
    input = Base();
    input.networkGeneration = input.generation - 1;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.reason == ContentAwareStreamReason::kGenerationMismatch,
        "OLD_SESSION_CAPACITY_NOT_REUSED");
    input = Base();
    input.sourceWidth = std::numeric_limits<std::uint32_t>::max();
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(!decision.hasRecommendation,
        "OVERFLOW_DIMENSIONS_REJECTED");
    input.sourceWidth = 0;
    ok &= Check(!RecommendContentAwareStream(input, config).hasRecommendation,
        "ZERO_SOURCE_REJECTED");
    input = Base();
    config.profiles[0].seedBitsPerPixelPerFrame =
        std::numeric_limits<double>::quiet_NaN();
    ok &= Check(!RecommendContentAwareStream(input, config).hasRecommendation,
        "INVALID_PROFILE_REJECTED");
    config = {};
    config.qualityEstimator = UnavailableModel;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(!decision.estimatedFeasible && !decision.automaticControlEligible,
        "MISSING_MODEL_NEVER_APPROVES");
    config.qualityEstimator = MaximumEstimate;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(!decision.estimatedFeasible &&
        decision.senderMaxBitrateBps <= static_cast<std::uint64_t>(input.safeVideoBudgetBps),
        "EXTREME_MODEL_RESULT_IS_NETWORK_BOUNDED");
    return ok;
}

bool ReferenceQualification()
{
    bool ok = true;
    ContentAwareStreamConfig config;
    config.qualityEstimator = ReferenceModel;
    auto input = Base();
    auto decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.estimatedFeasible && decision.modelReference &&
        !decision.modelCalibrated && !decision.qualityRequirementMet &&
        !decision.automaticControlEligible,
        "REFERENCE_MODEL_REQUIRES_EXPLICIT_HOST_OPT_IN");
    config.allowReferenceModel = true;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.estimatedFeasible && decision.modelReference &&
        !decision.modelCalibrated && decision.qualityRequirementMet &&
        decision.automaticControlEligible,
        "ALLOWED_REFERENCE_WITH_LIVE_EVIDENCE_IS_EXECUTABLE_NOT_CALIBRATED");

    config.qualityEstimator = nullptr;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(!decision.modelReference && !decision.modelCalibrated &&
        !decision.automaticControlEligible,
        "ALLOW_REFERENCE_DOES_NOT_PROMOTE_UNCALIBRATED_SEED");
    config.qualityEstimator = EmptyReferenceModel;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(!decision.modelReference && !decision.estimatedFeasible &&
        !decision.automaticControlEligible,
        "EMPTY_REFERENCE_IS_UNAVAILABLE");
    config.qualityEstimator = ReferenceModel;

    input.qualityAvailable = false;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(!decision.qualityRequirementMet && !decision.automaticControlEligible,
        "REFERENCE_CANNOT_BYPASS_MISSING_QUALITY_EVIDENCE");
    input = Base();
    input.qualityAcceptable = false;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(!decision.qualityRequirementMet && decision.automaticControlEligible &&
        decision.qualityRepairRequired && decision.senderMaxFps < input.currentFrameRate,
        "REFERENCE_REPAIRS_BAD_QUALITY_WITHOUT_CLAIMING_IT_PASSED");
    input = Base();
    input.processingAvailable = false;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.qualityRequirementMet && decision.automaticControlEligible &&
        !decision.processingRepairRequired,
        "REFERENCE_NETWORK_DECISION_DOES_NOT_REQUIRE_PROCESSING_EVIDENCE");
    input = Base();
    input.processingHealthy = false;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.automaticControlEligible && !decision.processingRepairRequired &&
        decision.senderMaxFps == input.currentFrameRate,
        "REFERENCE_PROCESSING_SLOWDOWN_ALONE_DOES_NOT_LOWER_FPS");
    input = Base();
    input.networkBudgetAvailable = false;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(!decision.automaticControlEligible,
        "REFERENCE_CANNOT_BYPASS_MISSING_NETWORK");
    input = Base();
    input.networkTimestampMs = input.nowMs - config.maximumNetworkSampleAgeMs - 1;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(!decision.automaticControlEligible &&
        decision.reason == ContentAwareStreamReason::kStaleNetwork,
        "REFERENCE_CANNOT_BYPASS_STALE_NETWORK");
    input = Base();
    input.networkGeneration = input.generation - 1;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(!decision.automaticControlEligible &&
        decision.reason == ContentAwareStreamReason::kGenerationMismatch,
        "REFERENCE_CANNOT_BYPASS_NETWORK_GENERATION");
    input = Base();
    input.active = false;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(!decision.automaticControlEligible && decision.senderMaxFps == 120,
        "REFERENCE_IDLE_HOLDS_CAPTURE_OWNED_FRAME_RATE");

    bool caps = true;
    bool budgetLimits = true;
    // User FPS is an upper bound, not a manual lock. Scene recommendations
    // remain within that bound at every capacity, including emergency output.
    for (const auto fps : {30u, 60u, 120u}) {
        for (const auto budget : {0ULL, 1'000'000ULL, 5'000'000ULL,
            13'000'000ULL, 100'000'000ULL}) {
            for (const auto scene : {ScreenScene::kCodeTerminal, ScreenScene::kGame3d}) {
                input = Base();
                input.currentFrameRate = input.maximumFrameRate = fps;
                input.maximumWidth = 1600;
                input.maximumHeight = 900;
                input.scene = scene;
                input.safeVideoBudgetBps = static_cast<double>(budget);
                decision = RecommendContentAwareStream(input, config);
                caps &= decision.hasRecommendation && decision.senderMaxFps <= fps &&
                    decision.width <= 1600 && decision.height <= 900 &&
                    decision.modelReference && !decision.modelCalibrated;
                budgetLimits &= decision.desiredVideoBitrateBps <= budget &&
                    decision.senderMaxBitrateBps <= budget &&
                    (!decision.automaticControlEligible ||
                     (decision.estimatedFeasible &&
                      decision.requiredVideoBitrateBps <= budget) ||
                     (decision.networkRepairRequired && !decision.qualityRequirementMet &&
                      decision.reason == ContentAwareStreamReason::kEmergencyNetworkReduction &&
                      decision.senderMaxFps <= input.currentFrameRate));
            }
        }
    }
    ok &= Check(caps, "REFERENCE_30_60_120_FPS_AND_SPATIAL_USER_CAPS");
    ok &= Check(budgetLimits, "REFERENCE_EXECUTION_AND_BITRATES_NEVER_EXCEED_BUDGET");

    ContentAwareStreamPolicyState state;
    input = Base();
    input.safeVideoBudgetBps = 22'000'000;
    config.allowReferenceModel = false;
    EvaluateContentAwareStream(state, input, config);
    input.nowMs = input.networkTimestampMs = 12'000;
    auto evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(!evaluation.confirmed && state.pendingSamples == 0,
        "REFERENCE_DISABLED_CANNOT_ACCUMULATE_EXECUTION_CONFIRMATION");
    config.allowReferenceModel = true;
    state = {};
    EvaluateContentAwareStream(state, input, config);
    input.nowMs = input.networkTimestampMs = 14'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(evaluation.confirmed && evaluation.recommendation.senderMaxFps == 100 &&
        evaluation.recommendation.modelReference &&
        !evaluation.recommendation.modelCalibrated,
        "ALLOWED_REFERENCE_STILL_REQUIRES_FRESH_WINDOW_CONFIRMATION");
    return ok;
}

bool HysteresisAndAnchor()
{
    bool ok = true;
    ContentAwareStreamConfig config;
    config.qualityEstimator = TestModel;
    ContentAwareStreamPolicyState state;
    auto input = Base();
    input.currentFrameRate = 60;
    auto evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(!evaluation.confirmed && state.pendingSamples == 1,
        "UPGRADE_NOT_CONFIRMED_FROM_ONE_WINDOW");
    const auto before = state.pendingSamples;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(!evaluation.confirmed && state.pendingSamples == before,
        "DUPLICATE_STATS_NOT_COUNTED");
    --input.networkTimestampMs;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(!evaluation.confirmed && state.pendingSamples == before,
        "OUT_OF_ORDER_STATS_NOT_COUNTED");
    for (int i = 1; i <= 5; ++i) {
        input.nowMs = input.networkTimestampMs = 10'000 + i * 1000;
        evaluation = EvaluateContentAwareStream(state, input, config);
    }
    ok &= Check(evaluation.confirmed && evaluation.recommendation.senderMaxFps == 120,
        "UPGRADE_NEEDS_FRESH_WINDOWS_AND_DWELL");
    input.generation = input.networkGeneration = 4;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(!evaluation.confirmed && state.pendingSamples == 1,
        "GENERATION_RESETS_UPGRADE_CONFIRMATION");

    state = {};
    input = Base();
    input.scene = ScreenScene::kGame3d;
    input.safeVideoBudgetBps = 22'000'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    input.nowMs = input.networkTimestampMs = 12'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(evaluation.confirmed && evaluation.recommendation.width == 1760,
        "DOWNGRADE_CONFIRMATION");
    input.currentWidth = evaluation.recommendation.width;
    input.currentHeight = evaluation.recommendation.height;
    input.currentFrameRate = evaluation.recommendation.senderMaxFps;
    input.currentDesiredVideoBitrateBps = evaluation.recommendation.desiredVideoBitrateBps;
    input.currentSenderMaxBitrateBps = evaluation.recommendation.senderMaxBitrateBps;
    ConfirmContentAwareStreamApplied(state, input, true);
    ok &= Check(state.anchorWidth == 1920 && state.anchorHeight == 1080,
        "VERIFIED_DOWNGRADE_DOES_NOT_MOVE_ANCHOR_DOWNWARD");
    input.safeVideoBudgetBps = 18'000'000;
    input.nowMs = input.networkTimestampMs = 14'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(evaluation.recommendation.width >= 1728 &&
        evaluation.recommendation.senderMaxFps < 120,
        "STATEFUL_CUMULATIVE_SPATIAL_LOSS_IS_BOUNDED");
    input.active = false;
    input.nowMs = input.networkTimestampMs = 16'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(!evaluation.confirmed && state.pendingSamples == 0,
        "IDLE_WINDOWS_CANNOT_CONFIRM_ACTION");
    return ok;
}

bool ExecutionConfirmation()
{
    bool ok = true;
    ContentAwareStreamConfig config;
    config.qualityEstimator = TestModel;
    ContentAwareStreamPolicyState state;
    auto input = Base();
    input.scene = ScreenScene::kGame3d;
    input.safeVideoBudgetBps = 22'000'000;
    auto evaluation = EvaluateContentAwareStream(state, input, config);
    auto effective = input;
    effective.currentWidth = evaluation.recommendation.width;
    effective.currentHeight = evaluation.recommendation.height;
    effective.currentFrameRate = evaluation.recommendation.senderMaxFps;
    effective.currentDesiredVideoBitrateBps = evaluation.recommendation.desiredVideoBitrateBps;
    effective.currentSenderMaxBitrateBps = evaluation.recommendation.senderMaxBitrateBps;
    ConfirmContentAwareStreamApplied(state, effective, true);
    ok &= Check(state.appliedWidth == 1920 && state.pendingSamples == 1,
        "UNCONFIRMED_CANDIDATE_CANNOT_BE_MARKED_APPLIED");

    input.nowMs = input.networkTimestampMs = 12'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    effective.nowMs = 12'000;
    --effective.currentSenderMaxBitrateBps;
    ConfirmContentAwareStreamApplied(state, effective, true);
    ok &= Check(evaluation.confirmed && state.appliedWidth == 1920 &&
        state.lastAppliedMs == 10'000,
        "PARTIALLY_APPLIED_BITRATE_CANNOT_CONFIRM_SPEC");
    ++effective.currentSenderMaxBitrateBps;
    ConfirmContentAwareStreamApplied(state, effective, true);
    ok &= Check(state.appliedWidth == 1760 && state.lastAppliedMs == 12'000 &&
        state.lastResolutionChangeMs == 12'000 && state.resolutionChangeApplied,
        "MATCHING_R_F_B_EXECUTION_CONFIRMATION");

    input = effective;
    input.safeVideoBudgetBps = 20'200'000;
    input.nowMs = input.networkTimestampMs = 13'000;
    EvaluateContentAwareStream(state, input, config);
    input.nowMs = input.networkTimestampMs = 14'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(evaluation.confirmed && state.pendingSamples == 2 &&
        evaluation.recommendation.width < input.currentWidth,
        "SECOND_NETWORK_RESOLUTION_DOWNGRADE_BYPASSES_RECOVERY_RESIDENCE");
    input.nowMs = input.networkTimestampMs = 15'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(evaluation.confirmed,
        "CONFIRMED_NETWORK_DOWNGRADE_REMAINS_WITHIN_SAFE_BUDGET");

    state = {};
    input = Base();
    input.currentFrameRate = input.maximumFrameRate = 60;
    input.currentDesiredVideoBitrateBps = 10'000'000;
    input.currentSenderMaxBitrateBps = 20'000'000;
    input.safeVideoBudgetBps = 14'000'000;
    EvaluateContentAwareStream(state, input, config);
    input.nowMs = input.networkTimestampMs = 11'000;
    input.safeVideoBudgetBps = 15'000'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(!evaluation.confirmed && state.pendingSamples == 2 &&
        state.pendingSenderMaxBitrateBps == 14'000'000 &&
        evaluation.confirmRequiredSamples == 3 && evaluation.confirmationRemainingMs == 1000,
        "FEASIBLE_BUDGET_RAMP_KEEPS_COUNTER_AND_LOWEST_SAFE_LIMIT");

    state = {};
    input = Base();
    input.currentFrameRate = input.maximumFrameRate = 60;
    input.safeVideoBudgetBps = 14'000'000;
    EvaluateContentAwareStream(state, input, config);
    input.nowMs = input.networkTimestampMs = 12'000;
    input.safeVideoBudgetBps = 13'800'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(evaluation.confirmed && state.pendingSamples == 2 &&
        evaluation.recommendation.senderMaxBitrateBps == 13'800'000 &&
        evaluation.recommendation.desiredVideoBitrateBps == 13'800'000,
        "SMALL_BWE_JITTER_CONFIRMS_SAME_SPEC_WITH_LOWER_SAFE_BUDGET");

    state = {};
    input = Base();
    input.currentSenderMaxBitrateBps = 50'000'000;
    input.safeVideoBudgetBps = 40'000'000;
    EvaluateContentAwareStream(state, input, config);
    input.nowMs = input.networkTimestampMs = 12'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(evaluation.confirmed && evaluation.recommendation.width == 1920 &&
        evaluation.recommendation.senderMaxFps == 120 &&
        evaluation.recommendation.senderMaxBitrateBps == 40'000'000,
        "SENDER_CEILING_ONLY_CHANGE_IS_NOT_IGNORED");
    return ok;
}

bool BoundedConfirmationTiming()
{
    bool ok = true;
    ContentAwareStreamConfig config;
    config.qualityEstimator = TestModel;
    ContentAwareStreamPolicyState state;
    auto input = Base();
    input.currentFrameRate = 30;
    input.maximumFrameRate = 60;
    input.currentDesiredVideoBitrateBps = input.currentSenderMaxBitrateBps = 10'000'000;
    input.safeVideoBudgetBps = 16'000'000;
    auto evaluation = EvaluateContentAwareStream(state, input, config);
    input.nowMs = input.networkTimestampMs = 11'000;
    input.safeVideoBudgetBps = 18'000'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(!evaluation.confirmed && evaluation.confirmObservedSamples == 2 &&
        evaluation.confirmRequiredSamples == 3 && evaluation.confirmationRemainingMs == 1000,
        "BUDGET_RAMP_RECOVERY_RETAINS_TWO_FRESH_WINDOWS");
    input.nowMs = input.networkTimestampMs = 12'000;
    input.safeVideoBudgetBps = 20'000'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(evaluation.confirmed && evaluation.recommendation.senderMaxFps == 60 &&
        evaluation.recommendation.senderMaxBitrateBps == 12'441'600 &&
        evaluation.recommendation.senderMaxBitrateBps <= state.pendingSafeVideoBudgetBps &&
        state.pendingSafeVideoBudgetBps == 16'000'000,
        "LARGE_GCC_BUDGET_RAMP_CONFIRMS_AFTER_TWO_SECONDS_WITH_LOWEST_LIMIT");

    state = {};
    input = Base();
    input.currentFrameRate = input.maximumFrameRate = 60;
    input.safeVideoBudgetBps = 14'000'000;
    EvaluateContentAwareStream(state, input, config);
    input.nowMs = input.networkTimestampMs = 11'000;
    input.safeVideoBudgetBps = 12'600'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(evaluation.confirmed && evaluation.confirmObservedSamples == 2 &&
        evaluation.recommendation.senderMaxFps == 60 &&
        evaluation.recommendation.senderMaxBitrateBps == 12'600'000,
        "LARGE_FEASIBLE_BUDGET_DECLINE_DOES_NOT_RESTART_DOWNGRADE");

    for (const auto scene : {ScreenScene::kCodeTerminal, ScreenScene::kMixed}) {
        state = {};
        input = Base();
        input.scene = scene;
        input.currentWidth = 1280;
        input.currentHeight = 720;
        input.currentFrameRate = input.maximumFrameRate = 60;
        input.currentDesiredVideoBitrateBps = input.currentSenderMaxBitrateBps = 10'000'000;
        input.safeVideoBudgetBps = 16'000'000;
        EvaluateContentAwareStream(state, input, config);
        for (int second = 1; second <= 2; ++second) {
            input.nowMs = input.networkTimestampMs = 10'000 + second * 1000;
            input.safeVideoBudgetBps += 2'000'000;
            evaluation = EvaluateContentAwareStream(state, input, config);
        }
        ok &= Check(!evaluation.confirmed && evaluation.confirmObservedSamples == 3 &&
            evaluation.confirmationBlock == ContentAwareStreamConfirmationBlock::kAwaitingResidence &&
            evaluation.confirmationRemainingMs == (scene == ScreenScene::kMixed ? 2000 : 1000),
            "SPATIAL_RECOVERY_RETAINS_BOUNDED_RESIDENCE_DURING_GCC_RAMP");
        input.nowMs = input.networkTimestampMs = 13'000;
        evaluation = EvaluateContentAwareStream(state, input, config);
        if (scene == ScreenScene::kMixed) {
            ok &= Check(!evaluation.confirmed && evaluation.confirmationRemainingMs == 1000,
                "MIXED_SCENE_RECOVERY_REQUIRES_FOUR_SECOND_SPATIAL_RESIDENCE");
            input.nowMs = input.networkTimestampMs = 14'000;
            evaluation = EvaluateContentAwareStream(state, input, config);
        }
        ok &= Check(evaluation.confirmed && evaluation.recommendation.width == 1920 &&
            evaluation.recommendation.senderMaxFps == 60 &&
            evaluation.recommendation.senderMaxBitrateBps == 12'441'600 &&
            state.pendingSafeVideoBudgetBps == 16'000'000,
            "SPATIAL_RECOVERY_CONFIRMS_WITHIN_USER_CAP_AND_BOUNDED_TIMER");
    }

    state = {};
    input = Base();
    input.currentFrameRate = 30;
    input.maximumFrameRate = 60;
    input.currentDesiredVideoBitrateBps = input.currentSenderMaxBitrateBps = 10'000'000;
    input.safeVideoBudgetBps = 16'000'000;
    EvaluateContentAwareStream(state, input, config);
    input.nowMs = input.networkTimestampMs = 14'001;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(!evaluation.confirmed && evaluation.confirmObservedSamples == 1,
        "FEEDBACK_GAP_BREAKS_CONSECUTIVE_RECOVERY_CONFIRMATION");
    input.safeVideoBudgetBps = 8'500'000;
    input.nowMs = input.networkTimestampMs = 15'001;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(!evaluation.confirmed && evaluation.confirmObservedSamples == 1 &&
        evaluation.recommendation.senderMaxFps < 60,
        "NEW_LOWER_BUDGET_CANNOT_REUSE_OLD_HIGHER_TARGET_CONFIRMATION");
    return ok;
}

bool EmergencyNetworkReduction()
{
    bool ok = true;
    ContentAwareStreamConfig config;
    config.qualityEstimator = SevereBudgetReferenceModel;
    config.allowReferenceModel = true;
    for (const auto fps : {120u, 100u, 90u, 75u, 60u, 50u, 45u, 30u, 24u, 20u}) {
        auto staged = Base();
        staged.currentFrameRate = fps;
        staged.safeVideoBudgetBps = 560'100;
        staged.qualityAcceptable = false;
        std::uint32_t nextFps = 0;
        for (const auto candidate : config.frameRateCandidates) {
            if (candidate < fps) nextFps = std::max(nextFps, candidate);
        }
        const auto next = RecommendContentAwareStream(staged, config);
        ok &= Check(next.senderMaxFps == nextFps && next.width == staged.currentWidth &&
            next.senderMaxBitrateBps == 560'100 &&
            next.reason == ContentAwareStreamReason::kEmergencyNetworkReduction,
            "EVERY_EMERGENCY_TEXT_FPS_DOWNGRADE_USES_ONLY_THE_ADJACENT_CONFIGURED_TIER");
    }
    ContentAwareStreamPolicyState state;
    auto input = Base();
    input.scene = ScreenScene::kWebApp;
    input.currentFrameRate = input.maximumFrameRate = 60;
    input.safeVideoBudgetBps = 560'100;
    input.qualityAcceptable = false;
    auto evaluation = EvaluateContentAwareStream(state, input, config);
    auto& recommendation = evaluation.recommendation;
    ok &= Check(recommendation.width == 1920 && recommendation.height == 1080 &&
        recommendation.senderMaxFps == 50 && recommendation.requiredVideoBitrateBps > 3'500'000 &&
        !recommendation.estimatedFeasible && !recommendation.qualityRequirementMet &&
        recommendation.automaticControlEligible && recommendation.networkRepairRequired &&
        recommendation.reason == ContentAwareStreamReason::kEmergencyNetworkReduction &&
        recommendation.senderMaxBitrateBps == 560'100,
        "SEVERE_560KBPS_BUDGET_FIRST_PRESERVES_TEXT_RESOLUTION_AND_ONLY_LOSES_ONE_FPS_TIER");
    input.nowMs = input.networkTimestampMs = 11'000;
    input.safeVideoBudgetBps = 400'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(evaluation.confirmed && evaluation.confirmObservedSamples == 2 &&
        evaluation.recommendation.senderMaxBitrateBps == 400'000,
        "SEVERE_NETWORK_REPAIR_CONFIRMS_TWO_WINDOWS_DESPITE_THEORETICAL_QUALITY_SHORTFALL");
    input.currentWidth = evaluation.recommendation.width;
    input.currentHeight = evaluation.recommendation.height;
    input.currentFrameRate = evaluation.recommendation.senderMaxFps;
    input.currentDesiredVideoBitrateBps = evaluation.recommendation.desiredVideoBitrateBps;
    input.currentSenderMaxBitrateBps = evaluation.recommendation.senderMaxBitrateBps;
    ConfirmContentAwareStreamApplied(state, input, false);
    bool gradualSteps = true;
    for (unsigned step = 0; step != 32; ++step) {
        input.nowMs = input.networkTimestampMs += 500;
        evaluation = EvaluateContentAwareStream(state, input, config);
        if (!evaluation.recommendation.automaticControlEligible) break;
        auto nextFps = input.currentFrameRate;
        std::uint32_t lowerFps = 0;
        for (const auto fps : config.frameRateCandidates) {
            if (fps < input.currentFrameRate) lowerFps = std::max(lowerFps, fps);
        }
        if (lowerFps) nextFps = lowerFps;
        gradualSteps &= !evaluation.confirmed &&
            evaluation.recommendation.senderMaxFps >= nextFps &&
            evaluation.recommendation.senderMaxFps <= input.currentFrameRate;
        input.nowMs = input.networkTimestampMs += 501;
        evaluation = EvaluateContentAwareStream(state, input, config);
        gradualSteps &= evaluation.confirmed;
        if (!evaluation.confirmed) break;
        input.currentWidth = evaluation.recommendation.width;
        input.currentHeight = evaluation.recommendation.height;
        input.currentFrameRate = evaluation.recommendation.senderMaxFps;
        input.currentDesiredVideoBitrateBps = evaluation.recommendation.desiredVideoBitrateBps;
        input.currentSenderMaxBitrateBps = evaluation.recommendation.senderMaxBitrateBps;
        ConfirmContentAwareStreamApplied(state, input, false);
    }
    ok &= Check(gradualSteps && input.currentWidth == 1152 &&
        input.currentHeight == 648 && input.currentFrameRate == 15,
        "PERSISTENT_WEAK_NETWORK_REQUIRES_FRESH_WINDOWS_FOR_EACH_DOWNWARD_FPS_TIER");
    input.nowMs = input.networkTimestampMs += 1000;
    input.safeVideoBudgetBps = 600'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(!evaluation.confirmed && !evaluation.recommendation.automaticControlEligible &&
        evaluation.recommendation.width == 1152 && evaluation.recommendation.senderMaxFps == 15,
        "MINIMUM_SCENE_SPEC_DOES_NOT_REPEATEDLY_DOWNGRADE_OR_RECOVER_FROM_BUDGET_NOISE");
    input.safeVideoBudgetBps = 100'000'000;
    input.nowMs = input.networkTimestampMs += 1000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(!evaluation.confirmed && evaluation.recommendation.width <= input.currentWidth &&
        evaluation.recommendation.senderMaxFps <= input.currentFrameRate,
        "EMERGENCY_RECOVERY_STILL_REQUIRES_CURRENT_QUALITY_PASS");

    for (unsigned guard = 0; guard != 7; ++guard) {
        input = Base();
        input.scene = ScreenScene::kWebApp;
        input.safeVideoBudgetBps = 560'100;
        if (guard == 0) input.networkConstrained = false;
        if (guard == 1) input.scene = ScreenScene::kUnknown;
        if (guard == 2) input.networkTimestampMs = 6000;
        if (guard == 3) input.active = false;
        if (guard == 4) input.networkBudgetAvailable = false;
        if (guard == 5) input.networkGeneration = input.generation - 1;
        if (guard == 6) {
            input.resolutionManual = input.frameRateManual = true;
            input.requestedWidth = 1920;
            input.requestedHeight = 1080;
            input.requestedFrameRate = 120;
        }
        const auto decision = RecommendContentAwareStream(input, config);
        ok &= Check(!decision.networkRepairRequired &&
            (guard == 0 ? (decision.width == input.currentWidth &&
                decision.senderMaxFps == input.currentFrameRate &&
                decision.reason == ContentAwareStreamReason::kHealthyHold) :
                !decision.automaticControlEligible),
            "EMERGENCY_REDUCTION_PRESERVES_NETWORK_SCENE_FRESHNESS_IDLE_AND_MANUAL_GUARDS");
    }
    return ok;
}

bool UserSpecificationRestore()
{
    bool ok = true;
    ContentAwareStreamConfig config;
    config.qualityEstimator = SevereBudgetReferenceModel;
    config.allowReferenceModel = true;
    for (const std::uint64_t coefficient : {3ULL, 15ULL}) {
        auto input = Base();
        input.scene = ScreenScene::kWebApp;
        input.currentFrameRate = 20;
        input.currentDesiredVideoBitrateBps = input.currentSenderMaxBitrateBps = 560'100;
        input.userVideoBitrateCeilingBps = 1920ULL * 1080 * 120 * coefficient / 100;
        input.safeVideoBudgetBps = static_cast<double>(ContentAwareVideoBudgetBps(
            input.userVideoBitrateCeilingBps, input.userVideoBitrateCeilingBps));
        input.qualityAcceptable = false;
        input.userSpecificationRecoveryAllowed = true;
        ContentAwareStreamPolicyState state;
        auto evaluation = EvaluateContentAwareStream(state, input, config);
        ok &= Check(evaluation.recommendation.senderMaxFps == 120 &&
            evaluation.recommendation.userSpecificationRestoreRequired &&
            evaluation.recommendation.reason == ContentAwareStreamReason::kUserSpecificationRestore &&
            evaluation.recommendation.automaticControlEligible &&
            !evaluation.recommendation.estimatedFeasible &&
            !evaluation.recommendation.qualityRequirementMet &&
            evaluation.recommendation.requiredVideoBitrateBps > input.userVideoBitrateCeilingBps,
            "LOW_BPP_USER_120_FPS_RESTORATION_IS_NOT_PERMANENTLY_LOCKED_BY_REFERENCE_OR_BAD_QP");
        input.nowMs = input.networkTimestampMs = 11'000;
        evaluation = EvaluateContentAwareStream(state, input, config);
        ok &= Check(!evaluation.confirmed && evaluation.confirmObservedSamples == 2,
            "USER_SPEC_RESTORATION_STILL_REQUIRES_FRESH_STABLE_CONFIRMATION");
        input.nowMs = input.networkTimestampMs = 12'000;
        evaluation = EvaluateContentAwareStream(state, input, config);
        ok &= Check(evaluation.confirmed && evaluation.confirmObservedSamples == 3 &&
            evaluation.recommendation.senderMaxFps == input.maximumFrameRate &&
            evaluation.recommendation.senderMaxBitrateBps <= input.safeVideoBudgetBps &&
            evaluation.recommendation.senderMaxBitrateBps <= input.userVideoBitrateCeilingBps,
            "USER_SPEC_120_FPS_RESTORES_AFTER_THREE_WINDOWS_AND_TWO_SECONDS_WITHIN_BUDGET");
        for (unsigned guard = 0; guard != 6; ++guard) {
            auto guarded = input;
            if (guard == 0) guarded.userSpecificationRecoveryAllowed = false;
            if (guard == 1) guarded.safeVideoBudgetBps -= 1;
            if (guard == 2) guarded.networkTimestampMs -= 4000;
            if (guard == 3) guarded.scene = ScreenScene::kUnknown;
            if (guard == 4) guarded.active = false;
            if (guard == 5) guarded.networkGeneration = guarded.generation - 1;
            ok &= Check(!RecommendContentAwareStream(guarded, config).userSpecificationRestoreRequired,
                "RESTORATION_REQUIRES_HOST_RECOVERY_PERMISSION_NEAR_FULL_BUDGET_FRESHNESS_AND_KNOWN_SCENE");
        }
        input.currentFrameRate = input.maximumFrameRate;
        input.currentDesiredVideoBitrateBps = input.currentSenderMaxBitrateBps =
            evaluation.recommendation.senderMaxBitrateBps;
        const auto held = RecommendContentAwareStream(input, config);
        ok &= Check(held.reason == ContentAwareStreamReason::kHealthyHold &&
            held.width == 1920 && held.senderMaxFps == 120 &&
            !held.networkRepairRequired && !held.qualityRequirementMet &&
            held.senderMaxBitrateBps == input.currentSenderMaxBitrateBps,
            "RESTORED_USER_BASELINE_IS_NOT_REDOWNSHIFTED_BY_BAD_QP_DURING_STABLE_GCC_CONFIRMATION");
    }
    auto input = Base();
    input.currentWidth = 1280;
    input.currentHeight = 720;
    input.currentFrameRate = 20;
    input.userSpecificationRecoveryAllowed = true;
    input.userVideoBitrateCeilingBps = 37'324'800;
    input.safeVideoBudgetBps = 36'000'000;
    input.qualityAcceptable = false;
    config.qualityEstimator = TestModel;
    ContentAwareStreamPolicyState state;
    EvaluateContentAwareStream(state, input, config);
    input.nowMs = input.networkTimestampMs = 11'000;
    EvaluateContentAwareStream(state, input, config);
    input.nowMs = input.networkTimestampMs = 12'000;
    auto evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(!evaluation.confirmed && evaluation.recommendation.userSpecificationRestoreRequired &&
        evaluation.recommendation.modelCalibrated && !evaluation.recommendation.qualityRequirementMet &&
        evaluation.confirmationBlock == ContentAwareStreamConfirmationBlock::kAwaitingResidence,
        "CALIBRATED_MODEL_BAD_QP_CANNOT_LOCK_USER_BASELINE_BUT_SPATIAL_RESIDENCE_REMAINS");
    input.nowMs = input.networkTimestampMs = 13'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(evaluation.confirmed && evaluation.recommendation.width == 1920 &&
        evaluation.recommendation.senderMaxFps == 120,
        "USER_BASELINE_SPATIAL_AND_FPS_RESTORATION_STAYS_INSIDE_USER_CAPS");
    return ok;
}

bool CandidateBoundaries()
{
    bool ok = true;
    ContentAwareStreamConfig config;
    config.qualityEstimator = TestModel;
    config.referenceHeights = {777, 999, 1234};
    config.frameRateCandidates = {17, 37, 73, 119};
    auto input = Base();
    input.scene = ScreenScene::kGame3d;
    input.safeVideoBudgetBps = 22'000'000;
    auto decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.height == 998 && decision.senderMaxFps == 120,
        "CUSTOM_NON_STANDARD_RESOLUTION_TIERS");
    input = Base();
    // At the largest feasible custom size, 119 FPS fits but 120 does not.
    input.safeVideoBudgetBps = 12'800'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.senderMaxFps == 119 && decision.width < 1920,
        "CUSTOM_FRAME_RATE_TIERS_PRESERVE_ADJACENT_DOWNGRADE");

    // A bounded deterministic sweep checks invariants beyond the example
    // transitions: disparate aspect ratios, odd sources, exact low FPS and
    // hard budgets. No test assumes actual bitrate must fill capacity.
    config = {};
    config.qualityEstimator = TestModel;
    constexpr std::array<std::array<std::uint32_t, 2>, 7> sources{{
        {5120, 1440}, {1080, 1920}, {1919, 1079}, {640, 480},
        {4096, 2160}, {7680, 4320}, {2, 16384}}};
    constexpr std::array<std::uint32_t, 5> caps{1, 15, 75, 120, 240};
    constexpr std::array<std::uint64_t, 6> budgets{
        0, 1'000'000, 5'000'000, 20'000'000, 100'000'000, 1'000'000'000};
    bool invariants = true;
    for (const auto source : sources) {
        for (const auto fps : caps) {
            for (const auto budget : budgets) {
                for (const auto scene : {ScreenScene::kDocument, ScreenScene::kGame3d}) {
                    input = Base();
                    input.sourceWidth = input.currentWidth = source[0];
                    input.sourceHeight = input.currentHeight = source[1];
                    input.maximumFrameRate = input.currentFrameRate = fps;
                    input.safeVideoBudgetBps = static_cast<double>(budget);
                    input.scene = scene;
                    decision = RecommendContentAwareStream(input, config);
                    invariants &= decision.hasRecommendation && decision.width &&
                        decision.height && decision.senderMaxFps &&
                        decision.width <= source[0] && decision.height <= source[1] &&
                        decision.width % 2 == 0 && decision.height % 2 == 0 &&
                        decision.senderMaxFps <= std::min(fps, 120u) &&
                        decision.desiredVideoBitrateBps <= budget &&
                        decision.senderMaxBitrateBps <= budget &&
                        (!decision.estimatedFeasible ||
                         decision.requiredVideoBitrateBps <= budget);
                }
            }
        }
    }
    ok &= Check(invariants, "420_BOUNDARY_COMBINATIONS_PRESERVE_CAPS_AND_BUDGET");

    ContentAwareStreamPolicyState state;
    input = Base();
    input.currentFrameRate = 60;
    input.nowMs = input.networkTimestampMs = 0;
    EvaluateContentAwareStream(state, input, config);
    const auto count = state.pendingSamples;
    const auto evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(!evaluation.confirmed && state.pendingSamples == count,
        "ZERO_TIMESTAMP_DUPLICATE_CANNOT_CONFIRM");
    return ok;
}

}  // namespace

bool SceneContractsAndRepair()
{
    bool ok = true;
    ContentAwareStreamConfig config;
    config.qualityEstimator = ReferenceModel;
    config.allowReferenceModel = true;
    const auto profile = [&](ScreenScene scene) -> StreamSceneProfile& {
        return config.profiles[static_cast<unsigned>(scene)];
    };
    ok &= Check(profile(ScreenScene::kCodeTerminal).minimumSpatialScale == .65 &&
        profile(ScreenScene::kSpreadsheet).minimumSpatialScale == .75 &&
        profile(ScreenScene::kCadDiagram).minimumSpatialScale == .75,
        "SCENE_SPECIFIC_TEXT_AND_FINE_LINE_SPATIAL_FLOORS");
    ok &= Check(profile(ScreenScene::kPhotoGraphics).seedBitsPerPixelPerFrame >
        profile(ScreenScene::kCodeTerminal).seedBitsPerPixelPerFrame &&
        profile(ScreenScene::kVideo).seedBitsPerPixelPerFrame !=
        profile(ScreenScene::kGame3d).seedBitsPerPixelPerFrame,
        "BITRATE_SEEDS_ARE_INDEPENDENT_PER_SCENE");
    ok &= Check(profile(ScreenScene::kPhotoGraphics).maximumReferenceAverageQp <
        profile(ScreenScene::kVideo).maximumReferenceAverageQp &&
        profile(ScreenScene::kGame3d).maximumReceiverProcessingMs <
        profile(ScreenScene::kDocument).maximumReceiverProcessingMs &&
        profile(ScreenScene::kGame3d).maximumEncodeFrameBudgetRatio <
        profile(ScreenScene::kDocument).maximumEncodeFrameBudgetRatio,
        "SCENE_QUALITY_AND_PROCESSING_REFERENCE_BUDGETS_ARE_DISTINCT");

    auto input = Base();
    input.currentDesiredVideoBitrateBps = input.currentSenderMaxBitrateBps = 5'000'000;
    input.qualityAcceptable = false;
    auto decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width == input.currentWidth && decision.senderMaxFps == input.currentFrameRate &&
        decision.desiredVideoBitrateBps > 5'000'000 && decision.qualityRepairRequired &&
        decision.automaticControlEligible && !decision.qualityRequirementMet,
        "QUALITY_REPAIR_OPENS_REQUIRED_BITRATE_BEFORE_CHANGING_R_OR_F");

    ContentAwareStreamPolicyState state;
    auto evaluation = EvaluateContentAwareStream(state, input, config);
    input.nowMs = input.networkTimestampMs = 12'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(evaluation.confirmed && evaluation.recommendation.qualityRepairRequired,
        "BITRATE_QUALITY_REPAIR_USES_DOWNGRADE_CONFIRMATION_NOT_SLOW_UPGRADE");

    input = Base();
    input.qualityAcceptable = false;
    input.qualityRecoveryBitrateBps = 28'750'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.qualityRepairRequired && decision.automaticControlEligible &&
        !decision.qualityRequirementMet && decision.width == input.currentWidth &&
        decision.senderMaxFps == input.currentFrameRate &&
        decision.requiredVideoBitrateBps == 28'750'000 &&
        decision.desiredVideoBitrateBps == 28'750'000 && decision.senderMaxBitrateBps == 28'750'000,
        "BAD_QP_WITH_SPARE_CAPACITY_RAISES_B_FIRST_EVEN_ABOVE_REFERENCE_SEED");
    input.safeVideoBudgetBps = 25'000'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.qualityRepairRequired && decision.senderMaxFps < input.currentFrameRate &&
        decision.width == input.currentWidth && decision.desiredVideoBitrateBps == 25'000'000 &&
        decision.requiredVideoBitrateBps <= 25'000'000,
        "UNAFFORDABLE_B_RECOVERY_FALLS_BACK_TO_SCENE_R_F_PROTECTION");
    input.qualityAcceptable = true;
    input.safeVideoBudgetBps = 100'000'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(!decision.qualityRepairRequired && decision.desiredVideoBitrateBps == 25'000'000,
        "RECOVERY_PRESSURE_IS_IGNORED_ON_GOOD_QUALITY");

    bool repairs = true;
    bool missingQualityNetworkRepair = true;
    bool missingProcessingDoesNotBlock = true;
    for (unsigned scene = 1; scene <= 9; ++scene) {
        for (unsigned fps : {30u, 60u, 120u}) {
            input = Base();
            input.scene = static_cast<ScreenScene>(scene);
            input.maximumFrameRate = input.currentFrameRate = fps;
            input.qualityAcceptable = false;
            input.processingHealthy = false;
            input.qualityRecoveryBitrateBps = 28'750'000;
            input.safeVideoBudgetBps = static_cast<double>(input.currentDesiredVideoBitrateBps);
            decision = RecommendContentAwareStream(input, config);
            repairs &= decision.automaticControlEligible &&
                (decision.qualityRepairRequired || decision.networkRepairRequired) &&
                !decision.processingRepairRequired && decision.senderMaxFps <= fps &&
                decision.width <= input.currentWidth && decision.height <= input.currentHeight &&
                (decision.senderMaxFps < fps || decision.width < input.currentWidth) &&
                decision.desiredVideoBitrateBps == input.currentDesiredVideoBitrateBps &&
                !decision.qualityRequirementMet;
            input.qualityAvailable = false;
            input.safeVideoBudgetBps = static_cast<double>(input.currentWidth) *
                input.currentHeight * fps * .075;
            const auto missingQuality = RecommendContentAwareStream(input, config);
            missingQualityNetworkRepair &= missingQuality.automaticControlEligible &&
                missingQuality.networkRepairRequired && !missingQuality.qualityRequirementMet &&
                !missingQuality.qualityRepairRequired;
            input.qualityAvailable = true;
            input.safeVideoBudgetBps = static_cast<double>(input.currentDesiredVideoBitrateBps);
            input.processingAvailable = false;
            missingProcessingDoesNotBlock &= RecommendContentAwareStream(input, config).automaticControlEligible;
        }
    }
    ok &= Check(repairs, "ALL_SCENES_REPAIR_WEAK_NETWORK_QUALITY_WITHIN_USER_CAPS");
    ok &= Check(missingQualityNetworkRepair,
        "GCC_PRESSURE_REPAIR_PRECEDES_MISSING_QP_WITHOUT_CLAIMING_QUALITY_PASS");
    ok &= Check(missingProcessingDoesNotBlock, "MISSING_PROCESSING_DOES_NOT_BLOCK_ALL_SCENE_WEAK_NETWORK_REPAIRS");

    input = Base();
    input.scene = ScreenScene::kVideo;
    input.currentFrameRate = input.maximumFrameRate = 60;
    input.safeVideoBudgetBps = 3'500'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(!decision.estimatedFeasible && decision.automaticControlEligible &&
        decision.senderMaxFps >= 50 && !decision.qualityRequirementMet &&
        decision.reason == ContentAwareStreamReason::kEmergencyNetworkReduction,
        "REAL_BUDGET_SHORTAGE_KEEPS_SCENE_PRIORITY_WITHOUT_SKIPPING_FPS_TIERS");
    input.currentFrameRate = input.maximumFrameRate = 20;
    input.safeVideoBudgetBps = 100'000'000;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.senderMaxFps == 20 && decision.activeFrameRateRequirementMet,
        "ACTIVITY_FLOOR_NEVER_EXCEEDS_USER_FPS_LIMIT");

    input = Base();
    input.currentWidth = 1280;
    input.currentHeight = 720;
    input.currentFrameRate = 60;
    profile(ScreenScene::kCodeTerminal).desiredSpatialScale = .80;
    decision = RecommendContentAwareStream(input, config);
    ok &= Check(decision.width <= 1536 && decision.height <= 864 && decision.width > 1280,
        "CONFIGURABLE_DESIRED_SPATIAL_TARGET_LIMITS_RESOLUTION_UPGRADE");
    profile(ScreenScene::kCodeTerminal).maximumEncodeFrameBudgetRatio =
        std::numeric_limits<double>::quiet_NaN();
    ok &= Check(RecommendContentAwareStream(input, config).hasRecommendation,
        "DIAGNOSTIC_PROCESSING_CONFIG_DOES_NOT_INVALIDATE_NETWORK_DECISION");

    config = ContentAwareStreamConfig{};
    config.qualityEstimator = ReferenceModel;
    config.allowReferenceModel = true;
    state = {};
    input = Base();
    input.safeVideoBudgetBps = 22'000'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    input.scene = ScreenScene::kWebApp;
    input.nowMs = input.networkTimestampMs = 12'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(!evaluation.confirmed && state.pendingSamples == 1 &&
        state.anchorWidth == 1920 && state.anchorHeight == 1080,
        "SCENE_CHANGE_RESETS_CONFIRMATION_AND_PRESERVES_STABLE_ANCHOR");
    input.nowMs = input.networkTimestampMs = 14'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(evaluation.confirmed && evaluation.recommendation.senderMaxFps == 100,
        "NEW_SCENE_REQUIRES_ITS_OWN_FRESH_CONFIRMATION_WINDOWS");

    state = {};
    input = Base();
    input.safeVideoBudgetBps = 22'000'000;
    input.qualityAcceptable = false;
    evaluation = EvaluateContentAwareStream(state, input, config);
    input.qualityAcceptable = true;
    input.nowMs = input.networkTimestampMs = 12'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(!evaluation.confirmed && state.pendingSamples == 1 &&
        !evaluation.recommendation.qualityRepairRequired,
        "REPAIR_TO_HEALTHY_ACTION_CANNOT_REUSE_EQUAL_R_F_B_CONFIRMATION");
    input.nowMs = input.networkTimestampMs = 14'000;
    evaluation = EvaluateContentAwareStream(state, input, config);
    ok &= Check(evaluation.confirmed,
        "HEALTHY_ACTION_AFTER_REPAIR_REQUIRES_NEW_FULL_WINDOWS");
    return ok;
}

int main()
{
    const auto maximum = std::numeric_limits<std::uint64_t>::max();
    const bool budget = Check(ContentAwareVideoBudgetBps(13'000'000, 20'000'000) == 12'350'000 &&
        ContentAwareVideoBudgetBps(80'000'000, 18'662'400) == 18'662'400 &&
        ContentAwareVideoBudgetBps(0, maximum) == 0 &&
        ContentAwareVideoBudgetBps(maximum, maximum) == 17'524'406'870'024'074'034ULL,
        "DIRECT_95_PERCENT_BUDGET_RESPECTS_USER_CEILING_ZERO_AND_UINT64_BOUNDARY");
    const bool decisions = Decisions();
    const bool sizes = SizesAndInvalidData();
    const bool state = HysteresisAndAnchor();
    const bool boundaries = CandidateBoundaries();
    const bool confirmation = ExecutionConfirmation();
    const bool reference = ReferenceQualification();
    const bool sceneContracts = SceneContractsAndRepair();
    const bool pressure = NetworkPressureGate();
    const bool boundedTiming = BoundedConfirmationTiming();
    const bool emergency = EmergencyNetworkReduction();
    const bool restore = UserSpecificationRestore();
    return budget && decisions && sizes && state && boundaries && confirmation && reference && sceneContracts && pressure && boundedTiming && emergency && restore ? 0 : 1;
}
