// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "media_intelligence/core/GoogCcNetworkPressure.h"

#include <cmath>

namespace remote::media_intelligence {
namespace {

bool Fresh(std::uint64_t timestamp, std::uint64_t now,
    std::uint64_t maximumAge) noexcept
{
    return timestamp != 0 && timestamp <= now && now - timestamp <= maximumAge;
}

}  // namespace

GoogCcNetworkPressureDecision EvaluateGoogCcNetworkPressure(
    GoogCcNetworkPressureState& state, const GoogCcNetworkPressureInput& input,
    const GoogCcNetworkPressureConfig& config) noexcept
{
    if (!state.initialized || state.routeGeneration != input.routeGeneration ||
        input.nowMs < state.lastNowMs) {
        state = {};
        state.initialized = true;
        state.routeGeneration = input.routeGeneration;
    }
    if (input.nowMs - state.lastNowMs > config.maximumSampleAgeMs) {
        state.hasStableNormal = false;
    }
    state.lastNowMs = input.nowMs;

    GoogCcNetworkPressureDecision result;
    result.targetBitrateBps = input.targetBitrateBps;
    const bool ratioValid = std::isfinite(input.cwndReduceRatio) &&
        input.cwndReduceRatio >= 0.0 && input.cwndReduceRatio <= 1.0;
    result.cwndReduceRatio = ratioValid ? input.cwndReduceRatio : 0.0;
    result.controllerFresh = input.controllerOutputAvailable &&
        input.targetBitrateBps != 0 &&
        Fresh(input.targetTimestampMs, input.nowMs, config.maximumSampleAgeMs);
    result.feedbackFresh = Fresh(input.transportFeedbackTimestampMs,
        input.nowMs, config.maximumSampleAgeMs);
    result.delayFresh = input.delayState != GoogCcDelayState::kUnknown &&
        Fresh(input.delayTimestampMs, input.nowMs, config.maximumSampleAgeMs);
    result.sampleFresh = result.controllerFresh && result.feedbackFresh;
    const bool recentDelayEvent = input.lastDelayOveruseAtMs > state.consumedDelayOveruseAtMs &&
        Fresh(input.lastDelayOveruseAtMs, input.nowMs, config.maximumSampleAgeMs);
    const bool recentCwndEvent = input.lastCwndPushbackAtMs > state.consumedCwndPushbackAtMs &&
        Fresh(input.lastCwndPushbackAtMs, input.nowMs, config.maximumSampleAgeMs);
    const bool delayPressure = result.feedbackFresh && ((result.delayFresh &&
        input.delayState == GoogCcDelayState::kOverusing) || recentDelayEvent);
    const bool cwndPressure = result.sampleFresh && ((ratioValid &&
        input.cwndReduceRatio > 0.0) || recentCwndEvent);
    if (delayPressure && recentDelayEvent)
        state.consumedDelayOveruseAtMs = input.lastDelayOveruseAtMs;
    if (cwndPressure && recentCwndEvent)
        state.consumedCwndPushbackAtMs = input.lastCwndPushbackAtMs;
    result.pressureNow = delayPressure || cwndPressure;

    if (result.pressureNow) {
        state.episodeActive = true;
        state.hasStableNormal = false;
        result.reason = delayPressure && cwndPressure
            ? GoogCcNetworkPressureReason::kDelayAndCongestionWindow
            : delayPressure ? GoogCcNetworkPressureReason::kDelayOveruse
                            : GoogCcNetworkPressureReason::kCongestionWindowPushback;
        state.lastPressureReason = result.reason;
    } else {
        // A settled queue at a reduced rate is still the same pressure episode.
        // Restoration must be observed by the host before normal confirmations
        // can clear it. Underuse/unknown/stale evidence breaks the confirmation.
        const bool restoredNormal = result.sampleFresh && result.delayFresh &&
            input.delayState == GoogCcDelayState::kNormal && ratioValid &&
            input.cwndReduceRatio == 0.0 && input.userSpecificationRestored;
        if (state.episodeActive && restoredNormal) {
            if (!state.hasStableNormal) {
                state.hasStableNormal = true;
                state.stableNormalSinceMs = input.nowMs;
            }
            if (input.nowMs - state.stableNormalSinceMs >= config.stableNormalMinimumMs) {
                state.episodeActive = false;
                state.hasStableNormal = false;
                state.lastPressureReason = GoogCcNetworkPressureReason::kUnavailable;
            }
        } else {
            state.hasStableNormal = false;
        }
        result.reason = !input.controllerOutputAvailable
            ? GoogCcNetworkPressureReason::kUnavailable
            : !result.feedbackFresh ? GoogCcNetworkPressureReason::kStaleFeedback
            : !result.controllerFresh ? GoogCcNetworkPressureReason::kStaleController
            : state.episodeActive ? GoogCcNetworkPressureReason::kEpisodeRetained
                                 : GoogCcNetworkPressureReason::kNormal;
    }
    result.episodeActive = state.episodeActive;
    return result;
}

const char* GoogCcNetworkPressureReasonName(GoogCcNetworkPressureReason reason) noexcept
{
    switch (reason) {
    case GoogCcNetworkPressureReason::kUnavailable: return "unavailable";
    case GoogCcNetworkPressureReason::kStaleFeedback: return "stale_feedback";
    case GoogCcNetworkPressureReason::kStaleController: return "stale_controller";
    case GoogCcNetworkPressureReason::kDelayOveruse: return "delay_overuse";
    case GoogCcNetworkPressureReason::kCongestionWindowPushback: return "congestion_window_pushback";
    case GoogCcNetworkPressureReason::kDelayAndCongestionWindow: return "delay_and_congestion_window";
    case GoogCcNetworkPressureReason::kEpisodeRetained: return "episode_retained";
    case GoogCcNetworkPressureReason::kNormal: return "normal";
    }
    return "unavailable";
}

}  // namespace remote::media_intelligence
