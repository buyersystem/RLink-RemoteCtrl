// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include <iostream>
#include <limits>
#include "src/apps/remote/adapters/ContentAwarePolicyDiagnostics.h"
#include "src/webrtc/IWebRtcSession.h"

namespace {
bool Check(bool valid, const char* name)
{
    std::cout << name << '=' << (valid ? "PASS" : "FAIL") << '\n';
    return valid;
}

remote::WebRtcSessionStatsSnapshot Sample()
{
    remote::WebRtcSessionStatsSnapshot stats;
    stats.transport.collected = true;
    stats.transport.receivedAtSteadyMs = 10000;
    stats.transport.availableOutgoingBitrateBps = 40000000;
    remote::RtpStreamStatsSnapshot screen;
    screen.kind = "video";
    screen.slot = remote::kScreenMainVideoSlot;
    screen.sourceWidth = screen.frameWidth = 1920;
    screen.sourceHeight = screen.frameHeight = 1080;
    screen.configuredMaxFrameRate = screen.effectiveNetworkFrameRate = 120;
    screen.configuredMaxBitrateBps = 40000000;
    screen.contentAnalyzerEnabled = true;
    screen.contentScene = "code_terminal";
    screen.captureActivityState = "active";
    stats.rtpStreams.push_back(screen);
    return stats;
}
}  // namespace

int main()
{
    bool passed = true;
    using namespace remote::media_intelligence;
    ContentState state;
    state.timestampMs = 10000;
    state.motion = ScreenMotionLevel::kHigh;
    auto observation = remote::app::BuildScreenContentPolicyObservation(
        true, 7, 1920, 1080, remote::ScreenContentActivity::kActive,
        state, 10001);
    passed &= Check(observation.enabled && observation.generation == 7 &&
        observation.sourceWidth == 1920 && observation.sourceHeight == 1080 &&
        observation.observedAtMs == 10001 &&
        observation.activity == remote::ScreenContentActivity::kActive &&
        observation.scene == ScreenScene::kUnknown,
        "HOST_MOTION_WITHOUT_CLASSIFICATION_REMAINS_UNKNOWN_AND_ACTIVE");
    state.scene = ScreenScene::kDocument;
    state.modelResultAvailable = true;
    observation = remote::app::BuildScreenContentPolicyObservation(
        true, 8, 2560, 1440, remote::ScreenContentActivity::kIdle, state, 40000);
    passed &= Check(observation.scene == ScreenScene::kDocument &&
        observation.activity == remote::ScreenContentActivity::kIdle &&
        observation.generation == 8 && observation.sourceWidth == 2560 &&
        !observation.qualityAvailable && !observation.processingAvailable,
        "HOST_FRESH_API_RESULT_PRESERVES_CAPTURE_ACTIVITY_WITHOUT_INVENTING_EVIDENCE");
    observation = remote::app::BuildScreenContentPolicyObservation(
        true, 8, 2560, 1440, remote::ScreenContentActivity::kActive, state, 40001);
    passed &= Check(observation.scene == ScreenScene::kDocument &&
        observation.activity == remote::ScreenContentActivity::kActive,
        "HOST_MOTION_AND_RESULT_AGE_DO_NOT_CLEAR_CONFIRMED_SCENE");
    for (const auto timestamp : {std::uint64_t{0}, std::uint64_t{10002}}) {
        state.timestampMs = timestamp;
        observation = remote::app::BuildScreenContentPolicyObservation(
            true, 7, 1920, 1080, remote::ScreenContentActivity::kActive, state, 10001);
        passed &= Check(observation.scene == ScreenScene::kUnknown,
            timestamp == 0 ? "HOST_MISSING_CLASSIFICATION_TIMESTAMP_REJECTED"
                : "HOST_FUTURE_CLASSIFICATION_TIMESTAMP_REJECTED");
    }
    state.timestampMs = 10000;
    observation = remote::app::BuildScreenContentPolicyObservation(
        false, 7, 1920, 1080, remote::ScreenContentActivity::kActive, state, 10001);
    passed &= Check(!observation.enabled && observation.scene == ScreenScene::kUnknown,
        "HOST_DISABLED_ANALYZER_DOES_NOT_REUSE_CLASSIFICATION");
    auto stats = Sample();
    remote::app::AnnotateContentAwarePolicyShadow(stats, 10000);
    const auto& result = stats.rtpStreams[0].contentPolicyShadow;
    passed &= Check(result.observed && result.hasRecommendation &&
        result.estimatedSafeVideoBudgetBps == 38000000 &&
        result.width == 1920 && result.senderMaxFps == 120 &&
        !result.modelCalibrated && !result.modelReference,
        "SHADOW_WITHOUT_PRESSURE_EVIDENCE_PRESERVES_CURRENT_SPECIFICATION");
    passed &= Check(stats.rtpStreams[0].configuredMaxFrameRate == 120 &&
        stats.rtpStreams[0].configuredMaxBitrateBps == 40000000 &&
        stats.rtpStreams[0].frameWidth == 1920,
        "SHADOW_NEVER_WRITES_EFFECTIVE_PARAMETERS");
    auto audio = remote::RtpStreamStatsSnapshot{};
    audio.kind = "audio";
    audio.bitrateBps = 64000;
    stats.rtpStreams.push_back(audio);
    auto camera = remote::RtpStreamStatsSnapshot{};
    camera.kind = "video";
    camera.slot = remote::kCameraMainVideoSlot;
    camera.targetBitrateBps = 1000000;
    camera.contentAnalyzerEnabled = true;
    stats.rtpStreams.push_back(camera);
    auto inbound = audio;
    inbound.direction = remote::RtpStreamDirection::kInbound;
    inbound.bitrateBps = std::numeric_limits<std::uint64_t>::max();
    stats.rtpStreams.push_back(inbound);
    auto channel = remote::DataChannelStatsSnapshot{};
    channel.sendBitrateBps = 200000;
    stats.dataChannels.push_back(channel);
    remote::app::AnnotateContentAwarePolicyShadow(stats, 10000);
    passed &= Check(stats.rtpStreams[0].contentPolicyShadow.
        estimatedSafeVideoBudgetBps == 38000000,
        "DIRECT_95_PERCENT_BUDGET_WITHOUT_SECOND_MEDIA_RESERVATION");
    passed &= Check(!stats.rtpStreams[2].contentPolicyShadow.observed,
        "CAMERA_NOT_SCREEN_POLICY_TARGET");
    stats.rtpStreams[0].appliedPeerConnectionMaxBitrateBps = 20000000;
    remote::app::AnnotateContentAwarePolicyShadow(stats, 10000);
    passed &= Check(stats.rtpStreams[0].contentPolicyShadow.
        estimatedSafeVideoBudgetBps == 38000000,
        "OBSERVATION_USES_REPORTED_CAPACITY_WITHOUT_SECOND_CONNECTION_CLAMP");
    remote::app::AnnotateContentAwarePolicyShadow(stats, 14001);
    passed &= Check(stats.rtpStreams[0].contentPolicyShadow.reason == "stale_network",
        "SNAPSHOT_READ_DOES_NOT_REFRESH_NETWORK_SAMPLE");
    stats = Sample();
    stats.transport.availableOutgoingBitrateBps = 0;
    stats.rtpStreams[0].bitrateBps = 40000000;
    remote::app::AnnotateContentAwarePolicyShadow(stats, 10000);
    passed &= Check(stats.rtpStreams[0].contentPolicyShadow.reason ==
        "capacity_unavailable", "SENT_RATE_IS_NOT_CAPACITY");
    stats = Sample();
    stats.rtpStreams[0].captureActivityState = "idle";
    remote::app::AnnotateContentAwarePolicyShadow(stats, 10000);
    passed &= Check(stats.rtpStreams[0].contentPolicyShadow.senderMaxFps == 120 &&
        stats.rtpStreams[0].contentPolicyShadow.reason == "idle_hold",
        "STATIC_HEARTBEAT_DOES_NOT_REDUCE_FPS_CAP");
    stats.rtpStreams[0].contentAnalyzerEnabled = false;
    remote::app::AnnotateContentAwarePolicyShadow(stats, 10000);
    passed &= Check(!stats.rtpStreams[0].contentPolicyShadow.observed,
        "DISABLED_ANALYZER_CLEARS_SHADOW");
    stats = Sample();
    audio.bitrateBps = std::numeric_limits<std::uint64_t>::max();
    stats.rtpStreams.push_back(audio);
    remote::app::AnnotateContentAwarePolicyShadow(stats, 10000);
    passed &= Check(stats.rtpStreams[0].contentPolicyShadow.
        estimatedSafeVideoBudgetBps == 38000000,
        "OTHER_MEDIA_TRAFFIC_DOES_NOT_REDUCE_DIRECT_BUDGET");
    stats.transport.availableOutgoingBitrateBps = std::numeric_limits<std::uint64_t>::max();
    remote::app::AnnotateContentAwarePolicyShadow(stats, 10000);
    passed &= Check(stats.rtpStreams[0].contentPolicyShadow.estimatedSafeVideoBudgetBps == 40000000,
        "DIRECT_BUDGET_STILL_CANNOT_EXCEED_VIDEO_CEILING_OR_OVERFLOW");
    stats = Sample();
    stats.rtpStreams[0].configuredMaxBitrateBps = 1'000'000;
    stats.rtpStreams[0].userVideoBitrateBppHundredths = 15;
    stats.rtpStreams[0].userVideoBitrateLimitBps = 18'662'400;
    stats.transport.availableOutgoingBitrateBps = 20'000'000;
    remote::app::AnnotateContentAwarePolicyShadow(stats, 10000);
    passed &= Check(stats.rtpStreams[0].contentPolicyShadow.estimatedSafeVideoBudgetBps == 18'662'400,
        "ADAPTIVE_BUDGET_USES_USER_BPP_TOTAL_CEILING_NOT_REDUCED_RTP_ALLOCATION");
    stats.transport.availableOutgoingBitrateBps = 1'600'000;
    remote::app::AnnotateContentAwarePolicyShadow(stats, 10000);
    passed &= Check(stats.rtpStreams[0].contentPolicyShadow.estimatedSafeVideoBudgetBps == 1'520'000,
        "USER_BPP_CEILING_NEVER_OVERRIDES_LOWER_GCC_BUDGET");
    stats = Sample();
    auto& background = stats.rtpStreams[0];
    background.contentPolicyExecution.observed = true;
    background.contentPolicyExecution.status = "observing";
    background.contentPolicyShadow.observed = true;
    background.contentPolicyShadow.senderMaxFps = 75;
    background.contentPolicyShadow.modelReference = true;
    background.contentPolicyShadow.reason = "background_candidate";
    remote::app::AnnotateContentAwarePolicyShadow(stats, 99999);
    passed &= Check(background.contentPolicyShadow.senderMaxFps == 75 &&
        background.contentPolicyShadow.reason == "background_candidate" &&
        background.contentPolicyShadow.modelReference &&
        !background.contentPolicyShadow.modelCalibrated &&
        background.contentPolicyExecution.status == "observing",
        "DIAGNOSTICS_READ_PRESERVES_BACKGROUND_CANDIDATE_AND_EXECUTION");
    return passed ? 0 : 1;
}
