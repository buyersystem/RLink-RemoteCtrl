// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)
#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>

namespace remote {

struct ScreenRecoveryProbeSample {
    std::uint64_t nowMs = 0, routeRevision = 0, feedbackAtMs = 0;
    std::uint64_t budgetBps = 0, congestionEventAtMs = 0;
    std::uint32_t targetFps = 0, videoCapBps = 0;
    bool enabled = false, feedbackFresh = false, healthy = false;
};

enum class ScreenRecoveryHintChange { kNone, kInstall, kClear };

// A bounded invitation to the ORIGINAL GoogCC probe controller, not a bandwidth
// estimator. High-history invitations retain acknowledged controller output;
// sustained invitations test 2x CURRENT acknowledged budget, not known capacity.
// No packet budget, RTP setting, frame scheduling or network target is changed.
class ScreenRecoveryProbePolicy final {
public:
    static constexpr std::uint64_t kHealthyWindowMs = 2500;
    static constexpr std::uint64_t kRecoveryWindowMs = 8000;
    static constexpr std::uint64_t kHistoryLifetimeMs = 30'000;
    // Count all native clusters, including successful continuation clusters.
    // Four can exhaust the invitation before a 24-Mbps stream recovers from
    // 1.6 Mbps. The eight-second deadline still bounds the whole short window.
    static constexpr std::uint32_t kMaximumProbes = 8;
    // A finite upper also limits native successful continuation to 70% of
    // that upper. Consume each invitation after its first cluster, then leave
    // the native chain unconstrained until it has been quiet for one second.
    static constexpr std::uint64_t kShortProbeQuietMs = 1000;
    static constexpr std::uint64_t kSustainedProbeIntervalMs = 3000;
    static constexpr std::uint64_t kSustainedHintWindowMs = 2000;

    ScreenRecoveryHintChange Update(const ScreenRecoveryProbeSample& sample) noexcept
    {
        lastSampleAtMs_ = sample.nowMs;
        const bool contextChanged = routeRevision_ != sample.routeRevision ||
            targetFps_ != sample.targetFps || videoCapBps_ != sample.videoCapBps;
        const bool contextValid = sample.enabled && sample.targetFps != 0 && sample.videoCapBps != 0;
        const bool feedbackValid = sample.feedbackFresh && sample.feedbackAtMs != 0 &&
            sample.nowMs >= sample.feedbackAtMs && sample.nowMs - sample.feedbackAtMs <= 1000;
        if (contextChanged || !contextValid) {
            const bool clear = active_;
            Reset();
            routeRevision_ = sample.routeRevision;
            targetFps_ = sample.targetFps;
            videoCapBps_ = sample.videoCapBps;
            return clear ? ScreenRecoveryHintChange::kClear : ScreenRecoveryHintChange::kNone;
        }
        if (historyAtMs_ != 0 && sample.nowMs - historyAtMs_ > kHistoryLifetimeMs) {
            const bool clear = active_ && !sustainedHint_;
            const auto pendingGoal = recoveryGoalBps_;
            const auto lastProbe = lastNativeProbeAtMs_;
            // An expired HIGH hint cannot be reused as capacity. The local
            // recovery goal may remain; sustained probes use ONLY current ACKs.
            if (sustainedHint_) {
                historicalBudgetBps_ = historyAtMs_ = 0;
                armed_ = false;
            } else {
                ResetHistory();
                recoveryGoalBps_ = pendingGoal;
                lastNativeProbeAtMs_ = lastProbe;
            }
            return clear ? ScreenRecoveryHintChange::kClear : ScreenRecoveryHintChange::kNone;
        }
        if (feedbackValid && recoveryGoalBps_ != 0 &&
            sample.budgetBps >= recoveryGoalBps_ * 0.90) {
            recoveryGoalBps_ = 0;
            sustainedHealthySinceMs_ = 0;
        }
        if (episodeInProgress_) {
            const bool recovered = feedbackValid &&
                (sustainedHint_ ? recoveryGoalBps_ == 0
                                : sample.budgetBps >= historicalBudgetBps_ * 0.90);
            const auto window = sustainedHint_ ? kSustainedHintWindowMs : kRecoveryWindowMs;
            if (recovered || sample.nowMs - episodeStartedAtMs_ >= window ||
                episodeProbes_ >= ProbeLimit()) {
                const bool clear = active_;
                episodeInProgress_ = false;
                active_ = false;
                sustainedHint_ = false;
                nextSustainedProbeAtMs_ = sample.nowMs + kSustainedProbeIntervalMs;
                sustainedHealthySinceMs_ = sustainedBudgetBps_ = 0;
                healthySinceMs_ = 0;
                candidateBudgetBps_ = 0;
                return clear ? ScreenRecoveryHintChange::kClear : ScreenRecoveryHintChange::kNone;
            }
        }
        if (!feedbackValid) {
            // An ACK gap pauses the native hint, not the measured history or
            // the episode clock. It consumes no fresh window or attempt budget.
            const bool clear = active_;
            active_ = false;
            healthySinceMs_ = 0;
            candidateBudgetBps_ = 0;
            sustainedHealthySinceMs_ = sustainedBudgetBps_ = 0;
            return clear ? ScreenRecoveryHintChange::kClear : ScreenRecoveryHintChange::kNone;
        }
        if (episodeInProgress_) {
            if (!sustainedHint_ && active_ && hintConsumed_) {
                active_ = false;
                return ScreenRecoveryHintChange::kClear;
            }
            if (!active_) {
                if (!sustainedHint_ && episodeProbes_ != 0 &&
                    sample.nowMs - lastNativeProbeAtMs_ < kShortProbeQuietMs)
                    return ScreenRecoveryHintChange::kNone;
                active_ = true;
                hintConsumed_ = false;
                return ScreenRecoveryHintChange::kInstall;
            }
            return ScreenRecoveryHintChange::kNone;
        }

        // Following a failed recovery episode, a low-but-stable limited link
        // cannot re-arm the old high-bandwidth hint. A real healthy restoration
        // must be acknowledged for another whole window first.
        const bool healthyEnoughToArm = sample.healthy && sample.budgetBps != 0 &&
            recoveryGoalBps_ == 0 &&
            (armed_ || historicalBudgetBps_ == 0 ||
             sample.budgetBps >= historicalBudgetBps_ * 0.90);
        if (healthyEnoughToArm && sample.feedbackAtMs != lastHealthyFeedbackAtMs_) {
            if (healthySinceMs_ == 0 || candidateBudgetBps_ == 0 ||
                sample.budgetBps < candidateBudgetBps_ * 0.80 ||
                sample.budgetBps > candidateBudgetBps_ * 1.25) {
                healthySinceMs_ = sample.nowMs;
                candidateBudgetBps_ = sample.budgetBps;
            } else candidateBudgetBps_ = (std::min)(candidateBudgetBps_, sample.budgetBps);
            lastHealthyFeedbackAtMs_ = sample.feedbackAtMs;
            if (sample.nowMs - healthySinceMs_ >= kHealthyWindowMs) {
                historicalBudgetBps_ = (std::min<std::uint64_t>)(
                    candidateBudgetBps_, sample.videoCapBps);
                historyAtMs_ = sample.nowMs;
                armed_ = true;
                // Start another measured window. Keeping the old minimum
                // forever loses a gradual healthy increase (<25% at a time)
                // and can declare recovery before the actual pre-outage rate.
                healthySinceMs_ = sample.nowMs;
                candidateBudgetBps_ = sample.budgetBps;
            }
        } else if (!healthyEnoughToArm) {
            healthySinceMs_ = 0;
            candidateBudgetBps_ = 0;
        }
        const bool freshCongestion = sample.congestionEventAtMs != 0 &&
            sample.nowMs >= sample.congestionEventAtMs &&
            sample.nowMs - sample.congestionEventAtMs <= 1500;
        // A modest real congestion drop can already hold the quality-protected
        // cadence below its user target. Use the measured network history,
        // never the image-quality reference or a theoretical user cap. This
        // subtraction is overflow-safe even for the full unsigned input range.
        const auto recoveryThreshold = historicalBudgetBps_ - historicalBudgetBps_ / 10;
        if (armed_ && historicalBudgetBps_ != 0 && sample.budgetBps != 0 &&
            sample.budgetBps < recoveryThreshold && freshCongestion) {
            armed_ = false;
            episodeInProgress_ = true;
            active_ = true;
            episodeStartedAtMs_ = sample.nowMs;
            episodeProbes_ = 0;
            hintConsumed_ = false;
            recoveryGoalBps_ = historicalBudgetBps_;
            sustainedHint_ = false;
            ++episodes_;
            return ScreenRecoveryHintChange::kInstall;
        }
        // After the finite high-history window, only an unresolved REAL
        // congestion episode permits small maintenance probes. Their upper
        // hint is 2x the CURRENT stable acknowledged budget, never the old high
        // capacity or theoretical bpp. Clear after the first native cluster so
        // successful native exponential probing is not capped by a stale hint.
        if (recoveryGoalBps_ != 0 && sample.healthy && sample.budgetBps != 0) {
            if (sample.feedbackAtMs != lastSustainedFeedbackAtMs_) {
                if (sustainedHealthySinceMs_ == 0 || sustainedBudgetBps_ == 0 ||
                    sample.budgetBps < sustainedBudgetBps_ * 0.80 ||
                    sample.budgetBps > sustainedBudgetBps_ * 1.25) {
                    sustainedHealthySinceMs_ = sample.nowMs;
                    sustainedBudgetBps_ = sample.budgetBps;
                } else sustainedBudgetBps_ = (std::min)(sustainedBudgetBps_, sample.budgetBps);
                lastSustainedFeedbackAtMs_ = sample.feedbackAtMs;
            }
            const bool nativeQuiet = lastNativeProbeAtMs_ == 0 ||
                sample.nowMs - lastNativeProbeAtMs_ >= kSustainedProbeIntervalMs;
            if (sample.nowMs >= nextSustainedProbeAtMs_ && nativeQuiet &&
                sample.nowMs - sustainedHealthySinceMs_ >= kHealthyWindowMs) {
                sustainedHintBudgetBps_ = (std::min<std::uint64_t>)(sample.videoCapBps,
                    (std::min<std::uint64_t>)(sample.videoCapBps,
                        (std::min)(sustainedBudgetBps_, sample.budgetBps)) * 2);
                if (sustainedHintBudgetBps_ > sample.budgetBps) {
                    sustainedHint_ = episodeInProgress_ = active_ = true;
                    episodeStartedAtMs_ = sample.nowMs;
                    episodeProbes_ = 0;
                    ++episodes_;
                    return ScreenRecoveryHintChange::kInstall;
                }
            }
        } else sustainedHealthySinceMs_ = sustainedBudgetBps_ = 0;
        return ScreenRecoveryHintChange::kNone;
    }

    void ObserveProbes(std::uint32_t count, std::uint64_t nowMs = 0) noexcept
    {
        if (count != 0) lastNativeProbeAtMs_ = nowMs != 0 ? nowMs : lastSampleAtMs_;
        if (count != 0 && episodeInProgress_) hintConsumed_ = true;
        if (episodeInProgress_) episodeProbes_ = static_cast<std::uint32_t>(
            (std::min<std::uint64_t>)(std::numeric_limits<std::uint32_t>::max(),
                static_cast<std::uint64_t>(episodeProbes_) + count));
    }
    bool Active() const noexcept { return active_; }
    std::uint64_t HistoricalBudgetBps() const noexcept
    { return sustainedHint_ ? sustainedHintBudgetBps_ : historicalBudgetBps_; }
    std::uint32_t ProbeLimit() const noexcept { return sustainedHint_ ? 1 : kMaximumProbes; }
    std::uint32_t EpisodeProbes() const noexcept { return episodeProbes_; }
    std::uint32_t Episodes() const noexcept { return episodes_; }

    void Reset() noexcept
    {
        ResetHistory();
        routeRevision_ = 0;
        targetFps_ = videoCapBps_ = 0;
    }

private:
    void ResetHistory() noexcept
    {
        historicalBudgetBps_ = historyAtMs_ = healthySinceMs_ = candidateBudgetBps_ = 0;
        lastHealthyFeedbackAtMs_ = episodeStartedAtMs_ = 0;
        recoveryGoalBps_ = lastNativeProbeAtMs_ = nextSustainedProbeAtMs_ = 0;
        sustainedHealthySinceMs_ = sustainedBudgetBps_ = lastSustainedFeedbackAtMs_ = 0;
        sustainedHintBudgetBps_ = 0;
        sustainedHint_ = false;
        active_ = armed_ = episodeInProgress_ = false;
        hintConsumed_ = false;
        episodeProbes_ = 0;
    }
    std::uint64_t routeRevision_ = 0, historicalBudgetBps_ = 0, historyAtMs_ = 0;
    std::uint64_t healthySinceMs_ = 0, candidateBudgetBps_ = 0;
    std::uint64_t lastHealthyFeedbackAtMs_ = 0, episodeStartedAtMs_ = 0;
    std::uint64_t recoveryGoalBps_ = 0, lastNativeProbeAtMs_ = 0, lastSampleAtMs_ = 0;
    std::uint64_t nextSustainedProbeAtMs_ = 0, sustainedHealthySinceMs_ = 0;
    std::uint64_t sustainedBudgetBps_ = 0, lastSustainedFeedbackAtMs_ = 0;
    std::uint64_t sustainedHintBudgetBps_ = 0;
    std::uint32_t targetFps_ = 0, videoCapBps_ = 0, episodeProbes_ = 0, episodes_ = 0;
    bool active_ = false, armed_ = false, episodeInProgress_ = false;
    bool sustainedHint_ = false;
    bool hintConsumed_ = false;
};

} // namespace remote
