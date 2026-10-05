// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>

namespace remote::media_intelligence {

enum class GoogCcDelayState : std::uint8_t {
    kUnknown,
    kNormal,
    kUnderusing,
    kOverusing,
};

enum class GoogCcNetworkPressureReason : std::uint8_t {
    kUnavailable,
    kStaleFeedback,
    kStaleController,
    kDelayOveruse,
    kCongestionWindowPushback,
    kDelayAndCongestionWindow,
    kEpisodeRetained,
    kNormal,
};

struct GoogCcNetworkPressureConfig {
    std::uint64_t maximumSampleAgeMs = 3000;
    std::uint64_t stableNormalMinimumMs = 5000;
};

struct GoogCcNetworkPressureInput {
    std::uint64_t nowMs = 0;
    std::uint64_t routeGeneration = 0;
    bool controllerOutputAvailable = false;
    // Last computation/confirmation of the snapshot, not last value change.
    // A periodic target confirmation must never renew packet-feedback age.
    std::uint64_t targetTimestampMs = 0;
    std::uint64_t transportFeedbackTimestampMs = 0;
    std::uint64_t delayTimestampMs = 0;
    // Actual positive detector/output events can precede a normal snapshot.
    std::uint64_t lastDelayOveruseAtMs = 0;
    std::uint64_t lastCwndPushbackAtMs = 0;
    GoogCcDelayState delayState = GoogCcDelayState::kUnknown;
    std::uint64_t targetBitrateBps = 0;
    double cwndReduceRatio = 0.0;
    // The host confirms actual R/F restoration to the user's specification,
    // or that no R/F downgrade has occurred in this episode. A recommendation
    // alone is not confirmation. Lower budget with normal delay is not recovery.
    bool userSpecificationRestored = false;
};

struct GoogCcNetworkPressureState {
    std::uint64_t routeGeneration = 0;
    std::uint64_t lastNowMs = 0;
    std::uint64_t stableNormalSinceMs = 0;
    std::uint64_t consumedDelayOveruseAtMs = 0;
    std::uint64_t consumedCwndPushbackAtMs = 0;
    bool initialized = false;
    bool episodeActive = false;
    bool hasStableNormal = false;
    GoogCcNetworkPressureReason lastPressureReason = GoogCcNetworkPressureReason::kUnavailable;
};

struct GoogCcNetworkPressureDecision {
    bool episodeActive = false;
    bool pressureNow = false;
    bool sampleFresh = false;
    bool controllerFresh = false;
    bool feedbackFresh = false;
    bool delayFresh = false;
    std::uint64_t targetBitrateBps = 0;
    double cwndReduceRatio = 0.0;
    GoogCcNetworkPressureReason reason = GoogCcNetworkPressureReason::kUnavailable;
};

// Portable, bounded, allocation-free observation of GoogCC decisions. It does
// not estimate bandwidth or impose RTT/loss/QP thresholds. Only fresh delay
// overuse or controller congestion-window pushback opens a pressure episode.
// Actions require episodeActive AND sampleFresh; stale observations keep the
// recovery context but never authorize a fresh scene adaptation.
GoogCcNetworkPressureDecision EvaluateGoogCcNetworkPressure(
    GoogCcNetworkPressureState& state,
    const GoogCcNetworkPressureInput& input,
    const GoogCcNetworkPressureConfig& config = {}) noexcept;

const char* GoogCcNetworkPressureReasonName(GoogCcNetworkPressureReason reason) noexcept;

}  // namespace remote::media_intelligence
