// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include <chrono>
#include <functional>
#include <iostream>
#include <thread>

#include "src/media_intelligence/core/ContentMotionAnalyzer.h"
#include "src/media_intelligence/runtime/ContentAnalysisWorker.h"

namespace {

using namespace remote::media_intelligence;

bool WaitUntil(const std::function<bool()>& predicate)
{
    const auto deadline = std::chrono::steady_clock::now() +
        std::chrono::seconds(2);
    while (std::chrono::steady_clock::now() < deadline) {
        if (predicate()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return predicate();
}

bool Check(bool value, const char* name)
{
    std::cout << name << '=' << (value ? "PASS" : "FAIL") << '\n';
    return value;
}

ContentAnalysisRequest Request(
    std::uint64_t generation,
    std::uint64_t frameId,
    float area,
    float changedFps,
    CaptureActivity activity = CaptureActivity::kActive)
{
    ContentAnalysisRequest request;
    request.generation = generation;
    request.ruleSample.activity = activity;
    request.ruleSample.changedAreaRatio = area;
    request.ruleSample.changedFramesPerSecond = changedFps;
    request.ruleSample.sourceFrameId = frameId;
    request.ruleSample.timestampMs = frameId * 10;
    return request;
}

}  // namespace

int main()
{
    bool passed = true;

    const auto idle = AnalyzeRuleMotion(
        Request(1, 1, 0.0f, 0.0f, CaptureActivity::kIdle).ruleSample);
    passed &= Check(
        idle.motion == ScreenMotionLevel::kIdle,
        "RULE_IDLE");

    const auto smallChange = AnalyzeRuleMotion(
        Request(1, 2, 0.01f, 5.0f).ruleSample);
    passed &= Check(
        smallChange.motion == ScreenMotionLevel::kLow,
        "RULE_SMALL_CHANGE");

    const auto fullMotion = AnalyzeRuleMotion(
        Request(1, 3, 1.0f, 30.0f).ruleSample);
    passed &= Check(
        fullMotion.motion == ScreenMotionLevel::kHigh,
        "RULE_FULL_MOTION");
    passed &= Check(
        fullMotion.semantic == ScreenSemanticType::kUnknown &&
            !fullMotion.modelResultAvailable,
        "RULE_DOES_NOT_GUESS_SEMANTICS");

    ContentState modelState;
    modelState.semantic = ScreenSemanticType::kVideo;
    modelState.semanticConfidence = 0.9f;
    modelState.videoScore = 0.9f;
    modelState.modelResultAvailable = true;
    modelState.motion = ScreenMotionLevel::kMedium;
    modelState.timestampMs = 1000;
    const auto stale = ApplyContentStateStaleness(modelState, 3001);
    passed &= Check(
        stale.semantic == ScreenSemanticType::kUnknown &&
            stale.motion == ScreenMotionLevel::kMedium &&
            !stale.modelResultAvailable,
        "STALE_SEMANTICS_ONLY");

    ContentAnalysisWorker worker({5, 1, 2000});
    passed &= Check(worker.Start(7), "WORKER_START");
    worker.Submit(Request(7, 10, 0.1f, 10.0f));
    passed &= Check(
        WaitUntil([&worker] {
            return worker.Snapshot(100).processedSamples >= 1;
        }),
        "WORKER_FIRST_RESULT");

    worker.Submit(Request(7, 11, 0.2f, 15.0f));
    worker.Submit(Request(7, 12, 1.0f, 30.0f));
    passed &= Check(
        WaitUntil([&worker] {
            const auto snapshot = worker.Snapshot(120);
            return snapshot.processedSamples >= 2 &&
                snapshot.state.sourceFrameId == 12 &&
                snapshot.replacedSamples >= 1;
        }),
        "WORKER_LATEST_SAMPLE_ONLY");

    passed &= Check(worker.Reset(8), "WORKER_GENERATION_RESET");
    passed &= Check(
        !worker.Submit(Request(7, 13, 1.0f, 30.0f)),
        "WORKER_REJECTS_OLD_GENERATION");
    passed &= Check(
        worker.Submit(Request(8, 14, 0.01f, 2.0f)),
        "WORKER_ACCEPTS_NEW_GENERATION");
    passed &= Check(
        WaitUntil([&worker] {
            const auto snapshot = worker.Snapshot(140);
            return snapshot.generation == 8 &&
                snapshot.state.sourceFrameId == 14;
        }),
        "WORKER_CLEARS_OLD_RESULT");

    worker.Stop();
    const auto stopped = worker.Snapshot(200);
    passed &= Check(
        !stopped.running && stopped.state.sourceFrameId == 0,
        "WORKER_STOP_CLEARS_STATE");

    std::cout << "MEDIA_INTELLIGENCE_SELF_TEST="
              << (passed ? "PASS" : "FAIL") << '\n';
    return passed ? 0 : 1;
}
