// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <optional>
#include <stop_token>
#include <thread>

#include "media_intelligence/core/ContentMotionAnalyzer.h"

namespace remote::media_intelligence {

struct ContentAnalysisRequest {
    std::uint64_t generation = 0;
    RuleMotionSample ruleSample;
};

struct ContentAnalysisSnapshot {
    bool running = false;
    std::uint64_t generation = 0;
    ContentState state;
    std::uint32_t stateAgeMs = 0;
    std::uint32_t latestAnalysisTimeUs = 0;
    std::uint64_t submittedSamples = 0;
    std::uint64_t replacedSamples = 0;
    std::uint64_t processedSamples = 0;
    std::uint64_t rejectedSamples = 0;
    std::uint64_t discardedResults = 0;
};

class ContentAnalysisWorker final {
public:
    struct Config {
        std::uint32_t activeRateHz = 3;
        std::uint32_t idleRateHz = 1;
        std::uint32_t semanticMaximumAgeMs = 2000;
    };

    explicit ContentAnalysisWorker(Config config = {});
    ~ContentAnalysisWorker();

    ContentAnalysisWorker(const ContentAnalysisWorker&) = delete;
    ContentAnalysisWorker& operator=(const ContentAnalysisWorker&) = delete;

    bool Start(std::uint64_t generation);
    void Stop();
    bool Reset(std::uint64_t generation);
    bool Submit(ContentAnalysisRequest request);
    ContentAnalysisSnapshot Snapshot(std::uint64_t nowMs) const;

private:
    void Run(std::stop_token stopToken);
    std::chrono::milliseconds IntervalFor(
        CaptureActivity activity) const noexcept;
    void ResetLocked(std::uint64_t generation);

    const Config config_;
    mutable std::mutex mutex_;
    std::condition_variable_any condition_;
    std::jthread worker_;
    bool running_ = false;
    std::uint64_t generation_ = 0;
    std::optional<ContentAnalysisRequest> pending_;
    ContentState latestState_;
    std::chrono::steady_clock::time_point nextEligibleAt_{};
    std::uint32_t latestAnalysisTimeUs_ = 0;
    std::uint64_t submittedSamples_ = 0;
    std::uint64_t replacedSamples_ = 0;
    std::uint64_t processedSamples_ = 0;
    std::uint64_t rejectedSamples_ = 0;
    std::uint64_t discardedResults_ = 0;
};

}  // namespace remote::media_intelligence
