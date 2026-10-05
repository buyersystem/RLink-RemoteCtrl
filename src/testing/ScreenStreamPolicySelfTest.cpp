// SPDX-License-Identifier: GPL-3.0-only
#include "src/core/ScreenStreamPolicy.h"
#include "src/core/ScreenNetworkPolicy.h"
#include "media_intelligence/core/H264ReferenceQualityModel.h"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>

namespace {
bool TestOriginalNetworkFrameRateControl()
{
    using namespace remote;
    bool passed = true;
    const auto check = [&](bool value, const char* name) {
        passed &= value;
        std::cout << name << '=' << (value ? "PASS" : "FAIL") << '\n';
    };
    AdaptiveScreenFrameRateConfig config;
    check(config.capacityEmaAlpha == 0.25 && config.capacitySafetyRatio == 0.85 &&
        config.requiredReductionSamples == 2 && config.startupGraceMs == 8000 &&
        config.minimumReductionIntervalMs == 2000,
        "ORIGINAL_REDUCTION_DEFAULTS_PRESERVED_WITHOUT_RECOVERY_TIMER");
    AdaptiveScreenFrameRateState state;
    AdaptiveScreenFrameRateSample sample{.activity = ScreenContentActivity::kActive,
        .capacityBps = 1'600'000};
    ResetAdaptiveScreenFrameRate(&state, true, 120, 1920, 1080, 1000);
    sample.timestampMs = 8999;
    const auto grace = EvaluateAdaptiveScreenFrameRate(&state, sample);
    check(!grace.applyEffectiveFrameRate && state.effectiveFrameRate == 120 &&
        state.status == AdaptiveScreenFrameRateStatus::kStartupGrace,
        "ORIGINAL_STARTUP_WAITS_EIGHT_SECONDS");
    sample.timestampMs = 9000;
    const auto first = EvaluateAdaptiveScreenFrameRate(&state, sample);
    sample.timestampMs = 10000;
    const auto second = EvaluateAdaptiveScreenFrameRate(&state, sample);
    check(!first.applyEffectiveFrameRate && second.applyEffectiveFrameRate &&
        second.effectiveFrameRate == 15 && state.requestedFrameRate == 120,
        "ORIGINAL_WEAK_NETWORK_REDUCES_DIRECTLY_TO_SUPPORTED_TIER");
    sample.capacityBps = 40'000'000;
    sample.timestampMs = 10001;
    auto decision = EvaluateAdaptiveScreenFrameRate(&state, sample);
    check(decision.applyEffectiveFrameRate && decision.effectiveFrameRate == 120 &&
        state.smoothedCapacityBps == sample.capacityBps,
        "GCC_FULL_BUDGET_RECOVERS_USER_120_IMMEDIATELY_WITHOUT_EMA_OR_TIER_WAIT");
    bool recoveredHeld = true;
    for (const auto timestamp : {11000u, 12000u, 13000u}) {
        sample.timestampMs = timestamp;
        recoveredHeld &= !EvaluateAdaptiveScreenFrameRate(&state, sample).applyEffectiveFrameRate &&
            state.effectiveFrameRate == 120;
    }
    check(recoveredHeld, "OLD_LOW_BUDGET_EMA_CANNOT_UNDO_GCC_RECOVERY");
    bool requestBounded = true;
    for (const auto requested : {30u, 60u, 75u, 120u}) {
        ResetAdaptiveScreenFrameRate(&state, true, requested, 1920, 1080, 1000);
        sample.capacityBps = 1'600'000;
        sample.timestampMs = 9000;
        (void)EvaluateAdaptiveScreenFrameRate(&state, sample);
        sample.timestampMs = 10000;
        (void)EvaluateAdaptiveScreenFrameRate(&state, sample);
        sample.capacityBps = 40'000'000;
        sample.timestampMs = 10001;
        decision = EvaluateAdaptiveScreenFrameRate(&state, sample);
        requestBounded &= decision.applyEffectiveFrameRate && decision.effectiveFrameRate == requested;
    }
    check(requestBounded, "GCC_RECOVERY_IS_BOUNDED_BY_EXACT_USER_TARGET_INCLUDING_75_FPS");
    const auto duplicate = EvaluateAdaptiveScreenFrameRate(&state, sample);
    check(!duplicate.applyEffectiveFrameRate, "ORIGINAL_DUPLICATE_SAMPLE_IS_IGNORED");
    ResetAdaptiveScreenFrameRate(&state, true, 60, 1920, 1080, 1000);
    sample.capacityBps = 4'800'000;
    sample.timestampMs = 9000;
    (void)EvaluateAdaptiveScreenFrameRate(&state, sample);
    sample.timestampMs = 10000;
    decision = EvaluateAdaptiveScreenFrameRate(&state, sample);
    check(decision.applyEffectiveFrameRate && decision.effectiveFrameRate == 24,
        "ORIGINAL_600_KB_UPLOAD_REFERENCE_SELECTS_24_FPS");
    sample.capacityBps = 10'000'000;
    sample.timestampMs = 10001;
    decision = EvaluateAdaptiveScreenFrameRate(&state, sample);
    check(decision.applyEffectiveFrameRate && decision.effectiveFrameRate == 45,
        "PARTIAL_GCC_RECOVERY_LIFTS_FPS_ONLY_TO_SUPPORTED_TIER");
    sample.activity = ScreenContentActivity::kIdle;
    sample.timestampMs = 12000;
    decision = EvaluateAdaptiveScreenFrameRate(&state, sample);
    check(!decision.applyEffectiveFrameRate && state.effectiveFrameRate == 45 &&
        state.recoverySampleCount == 0 && state.status == AdaptiveScreenFrameRateStatus::kIdleSuspended,
        "ORIGINAL_IDLE_SUSPENDS_NETWORK_CONTROLLER");
    ResetAdaptiveScreenFrameRate(&state, true, 60, 640, 360, 1000);
    sample.activity = ScreenContentActivity::kActive;
    sample.capacityBps = 2'100'000;
    sample.timestampMs = 9000;
    (void)EvaluateAdaptiveScreenFrameRate(&state, sample);
    sample.timestampMs = 10000;
    decision = EvaluateAdaptiveScreenFrameRate(&state, sample);
    check(decision.applyEffectiveFrameRate && decision.effectiveFrameRate == 15,
        "ORIGINAL_REFERENCE_CURVE_RETAINS_TWO_MBPS_FLOOR_ONLY_FOR_FPS_DECISIONS");
    return passed;
}
} // namespace

int main()
{
    bool passed = true;
    passed &= TestOriginalNetworkFrameRateControl();
    for (const std::uint32_t fps : {15u, 30u, 60u, 120u}) {
        const auto result = remote::ResolveScreenStreamPolicy(1920, 1080, {1920, 1080, fps});
        passed &= result.framesPerSecond == fps &&
            result.networkProbeMaxBitrateBps == 1920ULL * 1080u * fps * 15u / 100u * 105u / 100u;
        passed &= result.maxBitrateBps == 1920u * 1080u * fps * 15u / 100u;
        passed &= result.startBitrateBps <= result.maxBitrateBps;
        passed &= result.maxBitrateBps <= result.networkProbeMaxBitrateBps;
    }
    const auto user30 = remote::ResolveScreenStreamPolicy(1920, 1080, {1920, 1080, 30});
    passed &= user30.maxBitrateBps == 9'331'200 && user30.startBitrateBps == 4'976'640;
    const auto user60 = remote::ResolveScreenStreamPolicy(1920, 1080, {1920, 1080, 60});
    passed &= user60.startBitrateBps == 9'953'280;
    const auto user120 = remote::ResolveScreenStreamPolicy(1920, 1080, {1920, 1080, 120});
    const bool followsTarget = user30.networkProbeMaxBitrateBps == 9'797'760 &&
        user60.networkProbeMaxBitrateBps == 19'595'520 &&
        user120.networkProbeMaxBitrateBps == 39'191'040;
    passed &= followsTarget;
    std::cout << "CONNECTION_CEILING_FOLLOWS_USER_30_60_120=" <<
        (followsTarget ? "PASS" : "FAIL") << '\n';
    const auto excessiveReference = remote::ResolveScreenStreamPolicy(1920, 1080, {1920, 1080, 30, 120});
    passed &= excessiveReference.networkProbeMaxBitrateBps == user30.networkProbeMaxBitrateBps;
    const auto customReference = remote::ResolveScreenStreamPolicy(1920, 1080, {1920, 1080, 60, 50});
    passed &= customReference.maxBitrateBps == 62'208'000 &&
        customReference.networkProbeMaxBitrateBps == 65'318'400;
    const auto uhd = remote::ResolveScreenStreamPolicy(3840, 2160, {3840, 2160, 60});
    passed &= uhd.maxBitrateBps == 74'649'600 && uhd.networkProbeMaxBitrateBps == 78'382'080;
    const auto huge = (std::numeric_limits<std::uint32_t>::max)();
    const auto bounded = remote::ResolveScreenStreamPolicy(huge, huge, {0, 0, huge});
    passed &= bounded.maxBitrateBps == 100'000'000 && bounded.startBitrateBps == 100'000'000 &&
        bounded.networkProbeMaxBitrateBps == 105'000'000;
    const auto minimum = remote::ResolveScreenStreamPolicy(0, 0, {0, 0, 0});
    passed &= minimum.width == 2 && minimum.height == 2 && minimum.maxBitrateBps == 1 &&
        minimum.startBitrateBps == 1 && minimum.networkProbeMaxBitrateBps == 1;
    const auto small = remote::ResolveScreenStreamPolicy(320, 180, {320, 180, 30, 3});
    const bool noFloor = small.maxBitrateBps == 51'840 && small.startBitrateBps == 51'840 &&
        small.networkProbeMaxBitrateBps == 54'432;
    passed &= noFloor;
    std::cout << "LOW_RESOLUTION_HAS_NO_ARTIFICIAL_MEDIA_OR_START_FLOOR=" <<
        (noFloor ? "PASS" : "FAIL") << '\n';
    const auto scaled = remote::ResolveScreenStreamPolicy(3840, 2160, {1920, 1080, 30});
    passed &= scaled.width == 1920 && scaled.height == 1080 &&
        scaled.maxBitrateBps == user30.maxBitrateBps &&
        scaled.networkProbeMaxBitrateBps == user30.networkProbeMaxBitrateBps;
    bool selectableBpp = true;
    for (std::uint32_t hundredths = 3; hundredths <= 50; ++hundredths) {
        for (const auto fps : {30u, 60u, 120u}) {
            const auto selected = remote::ResolveScreenStreamPolicy(
                1920, 1080, {1920, 1080, fps, hundredths});
            const auto expected = std::clamp<std::uint64_t>(
                1920ULL * 1080u * fps * hundredths / 100u, 1, 100'000'000);
            selectableBpp &= selected.networkProbeMaxBitrateBps == expected * 105 / 100 &&
                selected.maxBitrateBps == expected &&
                selected.startBitrateBps <= selected.maxBitrateBps &&
                selected.width == 1920 && selected.height == 1080 && selected.framesPerSecond == fps;
        }
    }
    passed &= selectableBpp;
    std::cout << "SELECTABLE_BPP_BOUNDS_MEDIA_AND_START_ALLOCATION=" <<
        (selectableBpp ? "PASS" : "FAIL") << '\n';
    bool invalidBppFallback = true;
    for (const auto hundredths : {0u, 2u, 51u, huge}) {
        const auto invalid = remote::ResolveScreenStreamPolicy(
            1920, 1080, {1920, 1080, 60, hundredths});
        invalidBppFallback &= invalid.networkProbeMaxBitrateBps == user60.networkProbeMaxBitrateBps;
    }
    const auto lowBpp = remote::ResolveScreenStreamPolicy(1920, 1080, {1920, 1080, 60, 10});
    passed &= lowBpp.networkProbeMaxBitrateBps == 13'063'680 &&
        lowBpp.maxBitrateBps == 12'441'600 && lowBpp.startBitrateBps == 9'953'280;
    const auto minimumBpp = remote::ResolveScreenStreamPolicy(1920, 1080, {1920, 1080, 60, 3});
    passed &= minimumBpp.networkProbeMaxBitrateBps == 3'919'104 &&
        minimumBpp.maxBitrateBps == 3'732'480 && minimumBpp.startBitrateBps == 3'732'480;
    const auto highestBpp = remote::ResolveScreenStreamPolicy(3840, 2160, {3840, 2160, 120, 50});
    passed &= highestBpp.networkProbeMaxBitrateBps == 105'000'000;
    passed &= invalidBppFallback;
    std::cout << "INVALID_BPP_FALLS_BACK_TO_DEFAULT=" <<
        (invalidBppFallback ? "PASS" : "FAIL") << '\n';
    using namespace remote::media_intelligence;
    H264ReferenceQualityContext reference({"video/H264","FFmpeg/QSV","medium"});
    ContentAwareStreamConfig config;
    config.qualityEstimator=H264ReferenceQualityContext::Estimate;
    config.qualityEstimatorContext=&reference;
    config.allowReferenceModel=true;
    bool healthyReferenceFits = true;
    bool healthyReferenceRecovers = true;
    for (const auto fps : {30u,60u,120u}) {
        const auto policy=remote::ResolveScreenStreamPolicy(1920,1080,{1920,1080,fps});
        for (unsigned scene=1;scene<=9;++scene) {
            ContentAwareStreamInput input;
            input.sourceWidth=input.currentWidth=input.maximumWidth=1920;
            input.sourceHeight=input.currentHeight=input.maximumHeight=1080;
            input.currentFrameRate=input.maximumFrameRate=fps;
            input.currentDesiredVideoBitrateBps=input.currentSenderMaxBitrateBps=policy.maxBitrateBps;
            input.scene=static_cast<ScreenScene>(scene);
            input.activityAvailable=input.active=input.networkBudgetAvailable=true;
            input.qualityAvailable=input.qualityAcceptable=input.processingAvailable=input.processingHealthy=true;
            input.safeVideoBudgetBps=policy.networkProbeMaxBitrateBps*0.85-128000;
            input.nowMs=input.networkTimestampMs=1000;
            input.generation=input.networkGeneration=1;
            const auto decision=RecommendContentAwareStream(input,config);
            healthyReferenceFits &= decision.automaticControlEligible && decision.modelReference &&
                decision.width==1920 && decision.height==1080 && decision.senderMaxFps==fps;
            // A deliberately high choice has sufficient reference-model
            // headroom for every scene; lower choices may cap this recovery.
            const auto recoveryPolicy = remote::ResolveScreenStreamPolicy(
                1920, 1080, {1920, 1080, fps, 30});
            input.safeVideoBudgetBps = recoveryPolicy.networkProbeMaxBitrateBps * 0.85 - 128000;
            input.currentFrameRate = fps / 2;
            const auto recovery = RecommendContentAwareStream(input, config);
            healthyReferenceRecovers &= recovery.automaticControlEligible &&
                recovery.width == 1920 && recovery.height == 1080 && recovery.senderMaxFps == fps;
        }
    }
    passed &= healthyReferenceFits;
    passed &= healthyReferenceRecovers;
    std::cout << "HEALTHY_REFERENCE_CEILING_PRESERVES_30_60_120=" <<
        (healthyReferenceFits?"PASS":"FAIL") << '\n';
    std::cout << "HIGHER_BPP_CEILING_ALLOWS_ALL_SCENE_FPS_RECOVERY=" <<
        (healthyReferenceRecovers?"PASS":"FAIL") << '\n';
    std::cout << "SCREEN_STREAM_POLICY_SELF_TEST=" << (passed ? "PASS" : "FAIL") << '\n';
    return passed ? 0 : 1;
}
