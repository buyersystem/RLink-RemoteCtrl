// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ContentAwarePolicyDiagnostics.h"

#include <algorithm>
#include "media_intelligence/core/ContentAwareStreamPolicy.h"
#include "src/webrtc/IWebRtcSession.h"

namespace remote::app {

ScreenContentPolicyObservation BuildScreenContentPolicyObservation(
    bool enabled, std::uint64_t generation,
    std::uint32_t sourceWidth, std::uint32_t sourceHeight,
    ScreenContentActivity activity,
    const media_intelligence::ContentState& contentState,
    std::uint64_t nowMs) noexcept
{
    ScreenContentPolicyObservation observation;
    observation.enabled = enabled;
    observation.generation = generation;
    observation.observedAtMs = nowMs;
    observation.sourceWidth = sourceWidth;
    observation.sourceHeight = sourceHeight;
    observation.activity = activity;
    if (enabled && contentState.modelResultAvailable && contentState.timestampMs != 0 &&
        contentState.timestampMs <= nowMs) {
        observation.scene = contentState.scene;
    }
    return observation;
}

void AnnotateContentAwarePolicyShadow(
    WebRtcSessionStatsSnapshot& stats, std::uint64_t nowMs)
{
    using namespace media_intelligence;
    for (auto& stream : stats.rtpStreams) {
        // Background execution owns its recommendation and evidence state.
        // Reading a diagnostics page must not reevaluate or refresh that state.
        if (stream.contentAnalyzerEnabled && stream.contentPolicyExecution.observed &&
            stream.contentPolicyShadow.observed) continue;
        stream.contentPolicyShadow = {};
        if (!stream.contentAnalyzerEnabled ||
            stream.direction != RtpStreamDirection::kOutbound ||
            stream.kind != "video" || stream.slot != kScreenMainVideoSlot) {
            continue;
        }
        const auto capacity = stats.transport.googCc.controllerObserved
            ? stats.transport.googCc.effectiveTargetRateBps : stats.transport.availableOutgoingBitrateBps;
        const auto userCeiling = stream.userVideoBitrateLimitBps
            ? stream.userVideoBitrateLimitBps : stream.configuredMaxBitrateBps;
        const auto budget = ContentAwareVideoBudgetBps(capacity, userCeiling);
        ContentAwareStreamInput input;
        input.sourceWidth = stream.sourceWidth;
        input.sourceHeight = stream.sourceHeight;
        input.maximumWidth = stream.configuredOutputWidth;
        input.maximumHeight = stream.configuredOutputHeight;
        input.maximumFrameRate = (std::min)(120u, stream.configuredMaxFrameRate);
        input.currentWidth = stream.frameWidth;
        input.currentHeight = stream.frameHeight;
        input.currentFrameRate = stream.effectiveNetworkFrameRate != 0
            ? stream.effectiveNetworkFrameRate : stream.configuredMaxFrameRate;
        input.currentSenderMaxBitrateBps = stream.configuredMaxBitrateBps;
        input.scene = ParseScreenScene(stream.contentScene);
        input.activityAvailable = stream.captureActivityState == "active" ||
            stream.captureActivityState == "idle";
        input.active = stream.captureActivityState == "active";
        input.networkBudgetAvailable = stats.transport.collected &&
            stats.transport.availableOutgoingBitrateBps != 0;
        input.safeVideoBudgetBps = static_cast<double>(budget);
        input.nowMs = nowMs;
        input.networkTimestampMs = stats.transport.receivedAtSteadyMs;
        // Fallback for snapshots without a background policy result. Do not
        // infer acceptable quality from a generic QP or claim manual/auto state.
        // Without the executor's actual pressure evidence, preserve current
        // R/F rather than publishing a theoretical weak-network downgrade.
        const auto decision = RecommendContentAwareStream(input);
        auto& shadow = stream.contentPolicyShadow;
        shadow.observed = true;
        shadow.hasRecommendation = decision.hasRecommendation;
        shadow.estimatedFeasible = decision.estimatedFeasible;
        shadow.modelCalibrated = decision.modelCalibrated;
        shadow.modelReference = decision.modelReference;
        shadow.width = decision.width;
        shadow.height = decision.height;
        shadow.senderMaxFps = decision.senderMaxFps;
        shadow.estimatedSafeVideoBudgetBps = budget;
        shadow.requiredVideoBitrateBps = decision.requiredVideoBitrateBps;
        shadow.desiredVideoBitrateBps = decision.desiredVideoBitrateBps;
        shadow.senderMaxBitrateBps = decision.senderMaxBitrateBps;
        shadow.reason = ContentAwareStreamReasonName(decision.reason);
    }
}

}  // namespace remote::app
