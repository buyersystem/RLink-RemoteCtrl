// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "src/apps/remote/adapters/VisionApiFrameAnalyzer.h"

#include <QTimer>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <stop_token>
#include <thread>
#include <utility>

#include "media_intelligence/backends/openai_compatible/OpenAiCompatibleVisionProtocol.h"
#include "media_intelligence/remote/IClock.h"
#include "media_intelligence/remote/VisionApiRuntime.h"
#include "src/apps/remote/adapters/QtVisionApiExecutor.h"
#include "src/apps/remote/adapters/QtVisionApiHttpTransport.h"
#include "src/apps/remote/adapters/WebRtcVisionFrameEncoder.h"
#include "src/platform/win/DpapiContentAnalyzerSecretStore.h"

namespace remote::app {
namespace {

class SteadyClock final : public media_intelligence::IClock {
public:
    [[nodiscard]] std::uint64_t NowMs() const noexcept override
    {
        return static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch())
                .count());
    }
};

std::int64_t SteadyNowMs() noexcept
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

}  // namespace

struct VisionApiFrameAnalyzer::Impl {
    struct SharedState {
        std::atomic<bool> running{true};
        mutable std::mutex mutex;
        RemoteVisionFrameAnalyzerSnapshot snapshot;
        media_intelligence::SemanticClassification candidate;
        std::uint32_t candidateCount = 0;
    };

    explicit Impl(
        media_intelligence::VisionApiRuntimeConfig runtimeConfig,
        HostOptions hostOptions)
        : config(std::move(runtimeConfig)),
          activationCheck(std::move(hostOptions.activationCheck)),
          encodingOptionsProvider(
              std::move(hostOptions.encodingOptionsProvider)),
          maximumImageDimension(std::clamp<std::uint32_t>(
              hostOptions.maximumImageDimension, 256, 1280)),
          jpegQuality(std::clamp(hostOptions.jpegQuality, 30, 90)),
          executor(nullptr),
          transport(nullptr),
          secrets(std::move(hostOptions.credentialDirectory)),
          runtime(
              config,
              std::make_shared<
                  media_intelligence::OpenAiCompatibleVisionProtocol>(),
              transport,
              secrets,
              clock,
              executor)
    {
        runtime.Reset(1);
        uploadAllowed.store(
            !activationCheck || activationCheck(),
            std::memory_order_release);
        if (activationCheck) {
            activationTimer.setInterval(250);
            activationTimer.setSingleShot(false);
            QObject::connect(
                &activationTimer,
                &QTimer::timeout,
                &activationTimer,
                [this] { RefreshActivation(); });
            activationTimer.start();
        }
        worker = std::jthread(
            [this](std::stop_token stopToken) { Run(stopToken); });
    }

    ~Impl()
    {
        state->running.store(false, std::memory_order_release);
        worker.request_stop();
        condition.notify_all();
        if (worker.joinable()) {
            worker.join();
        }
        runtime.Stop();
    }

    void ResetState(
        std::uint64_t sessionToken,
        std::uint64_t nextRuntimeGeneration,
        bool active)
    {
        nextSampleAtMs.store(0, std::memory_order_release);
        nextFrameId.store(0, std::memory_order_release);
        {
            std::lock_guard queueLock(queueMutex);
            pendingFrame.reset();
            std::lock_guard stateLock(state->mutex);
            state->snapshot = {};
            state->snapshot.running = active;
            state->snapshot.generation = sessionToken;
            state->candidate = {};
            state->candidateCount = 0;
        }
        runtime.Reset(nextRuntimeGeneration);
    }

    void RefreshActivation()
    {
        if (!activationCheck) {
            return;
        }
        const bool allowed = activationCheck();
        std::lock_guard lifecycleLock(lifecycleMutex);
        const bool previous = uploadAllowed.exchange(
            allowed, std::memory_order_acq_rel);
        if (allowed == previous) {
            return;
        }
        const auto sessionToken = activeSessionToken.load(
            std::memory_order_acquire);
        const auto nextRuntimeGeneration =
            runtimeGeneration.fetch_add(
                1, std::memory_order_acq_rel) + 1;
        ResetState(
            sessionToken,
            nextRuntimeGeneration,
            allowed && sessionToken != 0);
    }

    void Run(std::stop_token stopToken)
    {
        for (;;) {
            webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame;
            std::uint64_t frameId = 0;
            std::uint64_t frameSessionToken = 0;
            std::uint64_t frameGeneration = 0;
            {
                std::unique_lock lock(queueMutex);
                condition.wait(lock, stopToken, [this] {
                    return !state->running.load(std::memory_order_acquire) ||
                        pendingFrame.has_value();
                });
                if (stopToken.stop_requested() ||
                    !state->running.load(std::memory_order_acquire)) {
                    return;
                }
                frame = std::move(pendingFrame->frame);
                frameId = pendingFrame->frameId;
                frameSessionToken = pendingFrame->sessionToken;
                frameGeneration = pendingFrame->generation;
                pendingFrame.reset();
            }

            if (!uploadAllowed.load(std::memory_order_acquire) ||
                activeSessionToken.load(std::memory_order_acquire) !=
                    frameSessionToken ||
                runtimeGeneration.load(std::memory_order_acquire) !=
                    frameGeneration) {
                std::lock_guard lock(state->mutex);
                ++state->snapshot.discardedResults;
                continue;
            }

            const auto startedAt = std::chrono::steady_clock::now();
            auto encodingOptions = FrameEncodingOptions{
                maximumImageDimension, jpegQuality};
            if (encodingOptionsProvider) {
                encodingOptions = encodingOptionsProvider();
            }
            encodingOptions.maximumImageDimension =
                std::clamp<std::uint32_t>(
                    encodingOptions.maximumImageDimension, 256, 1280);
            encodingOptions.jpegQuality =
                std::clamp(encodingOptions.jpegQuality, 30, 90);
            VisionFrameEncodingMetrics encodingMetrics;
            auto image = EncodeWebRtcFrameAsJpeg(
                std::move(frame),
                encodingOptions.maximumImageDimension,
                encodingOptions.jpegQuality,
                &encodingMetrics);
            if (!image.IsValid()) {
                std::lock_guard lock(state->mutex);
                ++state->snapshot.rejectedSamples;
                continue;
            }
            {
                std::lock_guard lock(state->mutex);
                state->snapshot.latestScaleConvertTimeUs =
                    encodingMetrics.scaleConvertTimeUs;
                state->snapshot.latestJpegEncodeTimeUs =
                    encodingMetrics.jpegEncodeTimeUs;
                state->snapshot.latestJpegBytes =
                    encodingMetrics.jpegBytes;
            }
            std::unique_lock lifecycleLock(lifecycleMutex);
            if (!uploadAllowed.load(std::memory_order_acquire) ||
                activeSessionToken.load(std::memory_order_acquire) !=
                    frameSessionToken ||
                runtimeGeneration.load(std::memory_order_acquire) !=
                    frameGeneration) {
                std::lock_guard lock(state->mutex);
                ++state->snapshot.discardedResults;
                continue;
            }

            media_intelligence::RemoteSemanticRequest request;
            request.generation = frameGeneration;
            request.sourceFrameId = frameId;
            request.timestampMs = clock.NowMs();
            request.image = std::move(image);
            const std::shared_ptr<SharedState> completionState = state;
            const std::uint64_t requestSessionToken =
                frameSessionToken;
            const std::uint64_t requestGeneration = frameGeneration;
            const auto status = runtime.Submit(
                std::move(request),
                [completionState, requestSessionToken,
                 requestGeneration, startedAt](
                    media_intelligence::RemoteSemanticResult result) {
                    const auto elapsedUs = static_cast<std::uint32_t>(
                        (std::min<std::int64_t>)(
                            std::chrono::duration_cast<
                                std::chrono::microseconds>(
                                    std::chrono::steady_clock::now() -
                                    startedAt).count(),
                            UINT32_MAX));
                    std::lock_guard lock(completionState->mutex);
                    if (!completionState->running.load(
                            std::memory_order_acquire) ||
                        result.generation != requestGeneration) {
                        ++completionState->snapshot.discardedResults;
                        return;
                    }
                    if (!completionState->snapshot.running ||
                        completionState->snapshot.generation !=
                            requestSessionToken) {
                        ++completionState->snapshot.discardedResults;
                        return;
                    }
                    completionState->snapshot.latestAnalysisTimeUs =
                        elapsedUs;
                    if (result.status ==
                            media_intelligence::RemoteClassificationStatus::kSuccess &&
                        result.classification.IsValid()) {
                        ++completionState->snapshot.processedSamples;
                        completionState->snapshot.latestReturnedClassification =
                            result.classification;
                        completionState->snapshot.latestReturnedAtMs =
                            static_cast<std::uint64_t>(SteadyNowMs());
                        if (result.classification.confidence < 0.70f) {
                            completionState->candidate = {};
                            completionState->candidateCount = 0;
                            return;
                        }
                        const auto current =
                            completionState->snapshot.classification;
                        const bool sameAsCurrent = current.IsValid() &&
                            current.scene == result.classification.scene;
                        // A confident model result identifies the scene itself;
                        // do not force another request interval for confirmation.
                        bool accept = sameAsCurrent ||
                            result.classification.confidence >= 0.85f;
                        if (!accept) {
                            if (completionState->candidate.IsValid() &&
                                completionState->candidate.scene ==
                                    result.classification.scene) {
                                ++completionState->candidateCount;
                            } else {
                                completionState->candidate =
                                    result.classification;
                                completionState->candidateCount = 1;
                            }
                            accept =
                                completionState->candidateCount >= 2;
                        }
                        if (accept) {
                            completionState->snapshot.classification =
                                result.classification;
                            completionState->snapshot.sourceFrameId =
                                result.sourceFrameId;
                            completionState->snapshot.completedAtMs =
                                static_cast<std::uint64_t>(SteadyNowMs());
                            completionState->candidate = {};
                            completionState->candidateCount = 0;
                        }
                    } else {
                        ++completionState->snapshot.rejectedSamples;
                        completionState->candidate = {};
                        completionState->candidateCount = 0;
                    }
                });
            lifecycleLock.unlock();
            if (status == media_intelligence::RemoteSubmitStatus::kRejected) {
                std::lock_guard lock(state->mutex);
                ++state->snapshot.rejectedSamples;
            }
        }
    }

    struct PendingFrame {
        webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame;
        std::uint64_t frameId = 0;
        std::uint64_t sessionToken = 0;
        std::uint64_t generation = 0;
    };

    const media_intelligence::VisionApiRuntimeConfig config;
    const ActivationCheck activationCheck;
    const EncodingOptionsProvider encodingOptionsProvider;
    const std::uint32_t maximumImageDimension;
    const int jpegQuality;
    SteadyClock clock;
    QTimer activationTimer;
    QtVisionApiExecutor executor;
    QtVisionApiHttpTransport transport;
    DpapiContentAnalyzerSecretStore secrets;
    media_intelligence::VisionApiRuntime runtime;
    std::jthread worker;
    std::shared_ptr<SharedState> state =
        std::make_shared<SharedState>();
    std::atomic<std::int64_t> nextSampleAtMs{0};
    std::atomic<std::uint64_t> nextFrameId{0};
    std::atomic<bool> uploadAllowed{true};
    std::atomic<std::uint64_t> activeSessionToken{0};
    std::atomic<std::uint64_t> nextSessionToken{1};
    std::atomic<std::uint64_t> runtimeGeneration{1};
    std::mutex lifecycleMutex;
    std::mutex queueMutex;
    std::condition_variable_any condition;
    std::optional<PendingFrame> pendingFrame;
};

std::shared_ptr<VisionApiFrameAnalyzer> VisionApiFrameAnalyzer::Create(
    media_intelligence::VisionApiRuntimeConfig config,
    HostOptions hostOptions,
    std::string* error)
{
    if (!media_intelligence::ValidateVisionApiEndpoint(
            config.endpoint, error)) {
        return {};
    }
    DpapiContentAnalyzerSecretStore secretProbe(
        hostOptions.credentialDirectory);
    if (!secretProbe.HasSecret(config.endpoint.credentialId)) {
        if (error) {
            *error = "credential_not_configured";
        }
        return {};
    }
    config.minimumRequestIntervalMs = std::clamp(
        config.minimumRequestIntervalMs,
        media_intelligence::kMinimumVisionApiRequestIntervalMs,
        media_intelligence::kMaximumVisionApiRequestIntervalMs);
    if (error) {
        error->clear();
    }
    return std::shared_ptr<VisionApiFrameAnalyzer>(
        new VisionApiFrameAnalyzer(
            std::make_unique<Impl>(
                std::move(config), std::move(hostOptions))));
}

VisionApiFrameAnalyzer::VisionApiFrameAnalyzer(
    std::unique_ptr<Impl> impl)
    : impl_(std::move(impl))
{
}

VisionApiFrameAnalyzer::~VisionApiFrameAnalyzer() = default;

std::uint64_t VisionApiFrameAnalyzer::BeginSession() noexcept
{
    if (!impl_ ||
        !impl_->state->running.load(std::memory_order_acquire)) {
        return 0;
    }
    std::lock_guard lifecycleLock(impl_->lifecycleMutex);
    if (!impl_->state->running.load(std::memory_order_acquire)) {
        return 0;
    }
    const std::uint64_t sessionToken =
        impl_->nextSessionToken.fetch_add(
            1, std::memory_order_acq_rel);
    impl_->activeSessionToken.store(
        sessionToken, std::memory_order_release);
    const std::uint64_t nextRuntimeGeneration =
        impl_->runtimeGeneration.fetch_add(
            1, std::memory_order_acq_rel) + 1;
    impl_->ResetState(
        sessionToken,
        nextRuntimeGeneration,
        impl_->uploadAllowed.load(std::memory_order_acquire));
    return sessionToken;
}

void VisionApiFrameAnalyzer::EndSession(
    std::uint64_t sessionToken) noexcept
{
    if (!impl_ || sessionToken == 0) {
        return;
    }
    std::lock_guard lifecycleLock(impl_->lifecycleMutex);
    auto current = sessionToken;
    if (!impl_->activeSessionToken.compare_exchange_strong(
            current,
            0,
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
        return;
    }
    const auto nextRuntimeGeneration =
        impl_->runtimeGeneration.fetch_add(
            1, std::memory_order_acq_rel) + 1;
    impl_->ResetState(0, nextRuntimeGeneration, false);
}

bool VisionApiFrameAnalyzer::SubmitFrame(
    std::uint64_t sessionToken,
    webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame) noexcept
{
    if (!impl_ || sessionToken == 0 || !frame ||
        impl_->activeSessionToken.load(
            std::memory_order_acquire) != sessionToken ||
        !impl_->uploadAllowed.load(std::memory_order_acquire) ||
        !impl_->state->running.load(std::memory_order_acquire)) {
        return false;
    }
    const std::int64_t nowMs = SteadyNowMs();
    auto nextAt = impl_->nextSampleAtMs.load(std::memory_order_acquire);
    if (nowMs < nextAt ||
        !impl_->nextSampleAtMs.compare_exchange_strong(
            nextAt,
            nowMs + static_cast<std::int64_t>(
                impl_->config.minimumRequestIntervalMs),
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
        return false;
    }

    Impl::PendingFrame pending;
    pending.frame = std::move(frame);
    pending.frameId = impl_->nextFrameId.fetch_add(
        1, std::memory_order_relaxed) + 1;
    pending.sessionToken = sessionToken;
    pending.generation = impl_->runtimeGeneration.load(
        std::memory_order_acquire);
    {
        std::lock_guard queueLock(impl_->queueMutex);
        if (impl_->activeSessionToken.load(
                std::memory_order_acquire) != sessionToken ||
            !impl_->uploadAllowed.load(std::memory_order_acquire) ||
            impl_->runtimeGeneration.load(std::memory_order_acquire) !=
                pending.generation) {
            return false;
        }
        const bool replaced = impl_->pendingFrame.has_value();
        impl_->pendingFrame = std::move(pending);
        std::lock_guard stateLock(impl_->state->mutex);
        ++impl_->state->snapshot.submittedSamples;
        if (replaced) {
            ++impl_->state->snapshot.replacedSamples;
        }
    }
    impl_->condition.notify_one();
    return true;
}

void VisionApiFrameAnalyzer::InvalidateResult(
    std::uint64_t sessionToken) noexcept
{
    if (!impl_ || sessionToken == 0 ||
        impl_->activeSessionToken.load(
            std::memory_order_acquire) != sessionToken) {
        return;
    }
    std::lock_guard lock(impl_->state->mutex);
    if (impl_->state->snapshot.generation != sessionToken) {
        return;
    }
    impl_->state->snapshot.classification = {};
    impl_->state->snapshot.sourceFrameId = 0;
    impl_->state->snapshot.completedAtMs = 0;
    impl_->state->candidate = {};
    impl_->state->candidateCount = 0;
}

std::uint64_t VisionApiFrameAnalyzer::MaximumSceneAgeMs() const noexcept
{
    if (!impl_) {
        return 65000;
    }
    // Runtime configuration is immutable. Allow three sampling intervals and
    // a request timeout, with a brief-failure grace period for fast sampling.
    return (std::max)(std::uint64_t{15000},
        std::uint64_t{3} * impl_->config.minimumRequestIntervalMs
            + impl_->config.endpoint.timeoutMs);
}

RemoteVisionFrameAnalyzerSnapshot VisionApiFrameAnalyzer::Snapshot()
    const noexcept
{
    if (!impl_) {
        return {};
    }
    std::lock_guard lock(impl_->state->mutex);
    auto result = impl_->state->snapshot;
    result.running = result.running &&
        impl_->state->running.load(std::memory_order_acquire);
    // Keep the last confirmed scene until a new classifier result or an
    // explicit session/configuration reset. A timeout does not identify a
    // different scene; completedAtMs still exposes the actual result age.
    return result;
}

}  // namespace remote::app
