// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ContentAnalysisWorker.h"

#include <algorithm>
#include <utility>

namespace remote::media_intelligence {

ContentAnalysisWorker::ContentAnalysisWorker(Config config)
    : config_(config)
{
}

ContentAnalysisWorker::~ContentAnalysisWorker()
{
    Stop();
}

bool ContentAnalysisWorker::Start(std::uint64_t generation)
{
    if (generation == 0) {
        return false;
    }
    Stop();
    {
        std::lock_guard lock(mutex_);
        ResetLocked(generation);
        running_ = true;
    }
    worker_ = std::jthread(
        [this](std::stop_token stopToken) { Run(stopToken); });
    return true;
}

void ContentAnalysisWorker::Stop()
{
    std::jthread worker;
    {
        std::lock_guard lock(mutex_);
        running_ = false;
        pending_.reset();
        latestState_ = {};
        if (worker_.joinable()) {
            worker_.request_stop();
            worker = std::move(worker_);
        }
    }
    condition_.notify_all();
    if (worker.joinable()) {
        worker.join();
    }
}

bool ContentAnalysisWorker::Reset(std::uint64_t generation)
{
    if (generation == 0) {
        return false;
    }
    {
        std::lock_guard lock(mutex_);
        if (!running_) {
            return false;
        }
        ResetLocked(generation);
    }
    condition_.notify_all();
    return true;
}

bool ContentAnalysisWorker::Submit(ContentAnalysisRequest request)
{
    {
        std::lock_guard lock(mutex_);
        if (!running_ || request.generation != generation_) {
            ++rejectedSamples_;
            return false;
        }
        if (pending_) {
            ++replacedSamples_;
        }
        pending_ = std::move(request);
        ++submittedSamples_;
    }
    condition_.notify_one();
    return true;
}

ContentAnalysisSnapshot ContentAnalysisWorker::Snapshot(
    std::uint64_t nowMs) const
{
    std::lock_guard lock(mutex_);
    ContentAnalysisSnapshot result;
    result.running = running_;
    result.generation = generation_;
    result.state = ApplyContentStateStaleness(
        latestState_, nowMs, config_.semanticMaximumAgeMs);
    if (result.state.timestampMs != 0 &&
        nowMs >= result.state.timestampMs) {
        result.stateAgeMs = static_cast<std::uint32_t>((std::min)(
            nowMs - result.state.timestampMs,
            static_cast<std::uint64_t>(UINT32_MAX)));
    }
    result.latestAnalysisTimeUs = latestAnalysisTimeUs_;
    result.submittedSamples = submittedSamples_;
    result.replacedSamples = replacedSamples_;
    result.processedSamples = processedSamples_;
    result.rejectedSamples = rejectedSamples_;
    result.discardedResults = discardedResults_;
    return result;
}

void ContentAnalysisWorker::Run(std::stop_token stopToken)
{
    for (;;) {
        ContentAnalysisRequest request;
        {
            std::unique_lock lock(mutex_);
            condition_.wait(lock, stopToken, [this] {
                return !running_ || pending_.has_value();
            });
            if (stopToken.stop_requested() || !running_) {
                return;
            }

            const auto now = std::chrono::steady_clock::now();
            if (now < nextEligibleAt_) {
                const auto deadline = nextEligibleAt_;
                condition_.wait_until(
                    lock,
                    stopToken,
                    deadline,
                    [this, deadline] {
                        return !running_ ||
                            nextEligibleAt_ != deadline;
                    });
                if (stopToken.stop_requested() || !running_) {
                    return;
                }
                continue;
            }

            request = std::move(*pending_);
            pending_.reset();
            nextEligibleAt_ = now +
                IntervalFor(request.ruleSample.activity);
        }

        const auto startedAt = std::chrono::steady_clock::now();
        ContentState state = AnalyzeRuleMotion(request.ruleSample);
        const auto completedAt = std::chrono::steady_clock::now();
        const auto analysisUs = static_cast<std::uint32_t>((std::min)(
            std::chrono::duration_cast<std::chrono::microseconds>(
                completedAt - startedAt).count(),
            static_cast<std::int64_t>(UINT32_MAX)));

        std::lock_guard lock(mutex_);
        if (!running_ || request.generation != generation_) {
            ++discardedResults_;
            continue;
        }
        latestState_ = state;
        latestAnalysisTimeUs_ = analysisUs;
        ++processedSamples_;
    }
}

std::chrono::milliseconds ContentAnalysisWorker::IntervalFor(
    CaptureActivity activity) const noexcept
{
    const std::uint32_t requestedRate =
        activity == CaptureActivity::kIdle
        ? config_.idleRateHz
        : config_.activeRateHz;
    const std::uint32_t rate = std::clamp(requestedRate, 1u, 60u);
    return std::chrono::milliseconds(1000 / rate);
}

void ContentAnalysisWorker::ResetLocked(std::uint64_t generation)
{
    generation_ = generation;
    pending_.reset();
    latestState_ = {};
    nextEligibleAt_ = std::chrono::steady_clock::now();
    latestAnalysisTimeUs_ = 0;
    submittedSamples_ = 0;
    replacedSamples_ = 0;
    processedSamples_ = 0;
    rejectedSamples_ = 0;
    discardedResults_ = 0;
}

}  // namespace remote::media_intelligence
