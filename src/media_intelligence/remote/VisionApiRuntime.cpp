// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "media_intelligence/remote/VisionApiRuntime.h"

#include <algorithm>
#include <limits>
#include <mutex>
#include <optional>
#include <utility>

namespace remote::media_intelligence {
namespace {

struct PendingCall {
    RemoteSemanticRequest request;
    IRemoteSemanticClassifier::Completion completion;
};

struct ActiveCall {
    std::uint64_t serial = 0;
    RemoteSemanticRequest request;
    IRemoteSemanticClassifier::Completion completion;
    IHttpTransport::RequestId transportRequestId = 0;
};

RemoteSemanticResult MakeResult(
    const RemoteSemanticRequest& request,
    RemoteClassificationStatus status,
    std::string error = {})
{
    RemoteSemanticResult result;
    result.status = status;
    result.generation = request.generation;
    result.sourceFrameId = request.sourceFrameId;
    result.error = std::move(error);
    return result;
}

void Dispatch(
    IExecutor& executor,
    IRemoteSemanticClassifier::Completion completion,
    RemoteSemanticResult result)
{
    if (!completion) {
        return;
    }
    executor.Post(
        [completion = std::move(completion),
         result = std::move(result)]() mutable {
            completion(std::move(result));
        });
}

std::uint32_t FailureBackoffMs(std::uint32_t consecutiveFailures)
{
    const std::uint32_t shift =
        std::min<std::uint32_t>(consecutiveFailures > 0
                                   ? consecutiveFailures - 1
                                   : 0,
                               6);
    return std::min<std::uint32_t>(1000u << shift, 60000u);
}

}  // namespace

struct VisionApiRuntimeState {
    VisionApiRuntimeState(
        VisionApiRuntimeConfig runtimeConfig,
        std::shared_ptr<const IVisionApiProtocol> apiProtocol,
        IHttpTransport& httpTransport,
        ISecretProvider& secretProvider,
        IClock& runtimeClock,
        IExecutor& executor,
        ILogger* runtimeLogger)
        : config(std::move(runtimeConfig)),
          protocol(std::move(apiProtocol)),
          transport(httpTransport),
          secrets(secretProvider),
          clock(runtimeClock),
          completionExecutor(executor),
          logger(runtimeLogger)
    {
        config.minimumRequestIntervalMs =
            std::max<std::uint32_t>(config.minimumRequestIntervalMs, 1);
        config.maximumConsecutiveFailures =
            std::max<std::uint32_t>(config.maximumConsecutiveFailures, 1);
        config.circuitBreakDurationMs =
            std::max<std::uint32_t>(config.circuitBreakDurationMs, 1000);
    }

    mutable std::mutex mutex;
    VisionApiRuntimeConfig config;
    std::shared_ptr<const IVisionApiProtocol> protocol;
    IHttpTransport& transport;
    ISecretProvider& secrets;
    IClock& clock;
    IExecutor& completionExecutor;
    ILogger* logger = nullptr;

    bool running = true;
    std::uint64_t generation = 0;
    std::uint64_t nextSerial = 1;
    std::uint64_t scheduleRevision = 0;
    bool wakeScheduled = false;
    std::optional<PendingCall> pending;
    std::optional<ActiveCall> active;
    VisionApiRuntimeSnapshot counters;
};

namespace {

void AdvancePending(const std::shared_ptr<VisionApiRuntimeState>& state);

void LogEvent(
    const std::shared_ptr<VisionApiRuntimeState>& state,
    VisionLogLevel level,
    std::string_view event)
{
    if (state->logger) {
        state->logger->Log(level, event);
    }
}

void FinishLocalFailure(
    const std::shared_ptr<VisionApiRuntimeState>& state,
    std::uint64_t serial,
    RemoteClassificationStatus status,
    std::string error)
{
    IRemoteSemanticClassifier::Completion completion;
    RemoteSemanticResult result;
    const std::uint64_t now = state->clock.NowMs();
    {
        std::lock_guard lock(state->mutex);
        if (!state->active || state->active->serial != serial) {
            return;
        }
        result = MakeResult(state->active->request, status, std::move(error));
        completion = std::move(state->active->completion);
        state->active.reset();
        ++state->counters.completedRequests;
        ++state->counters.failedRequests;
        ++state->counters.consecutiveFailures;
        state->counters.nextEligibleAtMs = now +
            std::max(state->config.minimumRequestIntervalMs,
                     FailureBackoffMs(state->counters.consecutiveFailures));
        if (state->counters.consecutiveFailures >=
            state->config.maximumConsecutiveFailures) {
            state->counters.circuitOpenUntilMs =
                now + state->config.circuitBreakDurationMs;
        }
    }
    LogEvent(state, VisionLogLevel::kWarning, "vision_api_local_failure");
    Dispatch(state->completionExecutor, std::move(completion), std::move(result));
    AdvancePending(state);
}

void HandleHttpCompletion(
    const std::shared_ptr<VisionApiRuntimeState>& state,
    std::uint64_t serial,
    HttpResponse response)
{
    IRemoteSemanticClassifier::Completion completion;
    RemoteSemanticResult result;
    bool success = false;
    RemoteClassificationStatus status =
        RemoteClassificationStatus::kTransportError;
    SemanticClassification classification;
    std::string error;

    switch (response.transportStatus) {
    case HttpTransportStatus::kNetworkError:
        status = RemoteClassificationStatus::kTransportError;
        error = response.error.empty() ? "transport_error"
                                       : std::move(response.error);
        break;
    case HttpTransportStatus::kTimeout:
        status = RemoteClassificationStatus::kTimeout;
        error = "request_timeout";
        break;
    case HttpTransportStatus::kCanceled:
        status = RemoteClassificationStatus::kCanceled;
        error = "request_canceled";
        break;
    case HttpTransportStatus::kResponseTooLarge:
        status = RemoteClassificationStatus::kResponseTooLarge;
        error = "response_too_large";
        break;
    case HttpTransportStatus::kSuccess:
        if (response.statusCode < 200 || response.statusCode >= 300) {
            status = RemoteClassificationStatus::kHttpError;
            error = "http_status_error";
        } else if (response.body.size() >
                   state->config.endpoint.maximumResponseBytes) {
            status = RemoteClassificationStatus::kResponseTooLarge;
            error = "response_too_large";
        } else {
            const VisionProtocolParseResult parsed =
                state->protocol->ParseResponse(
                    state->config.endpoint,
                    response.body);
            if (parsed.success) {
                status = RemoteClassificationStatus::kSuccess;
                classification = parsed.classification;
                success = true;
            } else {
                status = RemoteClassificationStatus::kInvalidResponse;
                error = parsed.error;
            }
        }
        break;
    }

    const std::uint64_t now = state->clock.NowMs();
    {
        std::lock_guard lock(state->mutex);
        if (!state->active || state->active->serial != serial) {
            return;
        }

        result.generation = state->active->request.generation;
        result.sourceFrameId = state->active->request.sourceFrameId;
        result.httpStatus = response.statusCode;
        result.retryAfterMs = response.retryAfterMs;
        result.status = status;
        result.classification = classification;
        result.error = std::move(error);

        completion = std::move(state->active->completion);
        state->active.reset();
        ++state->counters.completedRequests;
        if (success) {
            ++state->counters.successfulRequests;
            state->counters.consecutiveFailures = 0;
            state->counters.circuitOpenUntilMs = 0;
            state->counters.nextEligibleAtMs =
                now + state->config.minimumRequestIntervalMs;
        } else {
            ++state->counters.failedRequests;
            ++state->counters.consecutiveFailures;
            const std::uint32_t backoff = std::max(
                {state->config.minimumRequestIntervalMs,
                 FailureBackoffMs(state->counters.consecutiveFailures),
                 response.retryAfterMs});
            state->counters.nextEligibleAtMs = now + backoff;
            if (state->counters.consecutiveFailures >=
                state->config.maximumConsecutiveFailures) {
                state->counters.circuitOpenUntilMs =
                    now + state->config.circuitBreakDurationMs;
            }
        }
    }

    LogEvent(
        state,
        success ? VisionLogLevel::kDebug : VisionLogLevel::kWarning,
        success ? "vision_api_request_succeeded"
                : "vision_api_request_failed");
    Dispatch(state->completionExecutor, std::move(completion), std::move(result));
    AdvancePending(state);
}

void StartActive(
    const std::shared_ptr<VisionApiRuntimeState>& state,
    std::uint64_t serial,
    EncodedImage image)
{
    VisionProtocolBuildResult built =
        state->protocol->BuildRequest(state->config.endpoint, image.View());
    if (!built.success) {
        FinishLocalFailure(
            state,
            serial,
            RemoteClassificationStatus::kInvalidConfiguration,
            built.error);
        return;
    }

    SecretValue credential;
    std::string secretError;
    if (!state->secrets.LoadSecret(
            state->config.endpoint.credentialId,
            &credential,
            &secretError) ||
        credential.Empty()) {
        FinishLocalFailure(
            state,
            serial,
            RemoteClassificationStatus::kCredentialUnavailable,
            secretError.empty() ? "credential_unavailable" : secretError);
        return;
    }

    HttpAuthorization authorization;
    authorization.credential = std::move(credential);
    const std::weak_ptr<VisionApiRuntimeState> weakState = state;
    const IHttpTransport::RequestId requestId = state->transport.Start(
        std::move(built.request),
        std::move(authorization),
        [weakState, serial](HttpResponse response) mutable {
            if (const auto locked = weakState.lock()) {
                HandleHttpCompletion(locked, serial, std::move(response));
            }
        });

    if (requestId == 0) {
        FinishLocalFailure(
            state,
            serial,
            RemoteClassificationStatus::kTransportError,
            "transport_rejected_request");
        return;
    }

    bool cancel = false;
    {
        std::lock_guard lock(state->mutex);
        if (!state->active || state->active->serial != serial) {
            cancel = true;
        } else {
            state->active->transportRequestId = requestId;
            ++state->counters.startedRequests;
        }
    }
    if (cancel) {
        state->transport.Cancel(requestId);
    } else {
        LogEvent(state, VisionLogLevel::kDebug, "vision_api_request_started");
    }
}

void AdvancePending(const std::shared_ptr<VisionApiRuntimeState>& state)
{
    std::optional<PendingCall> rejected;
    std::uint64_t serialToStart = 0;
    EncodedImage imageToStart;
    std::uint64_t scheduleRevision = 0;
    std::uint32_t scheduleDelayMs = 0;
    const std::uint64_t now = state->clock.NowMs();
    {
        std::lock_guard lock(state->mutex);
        if (!state->running || state->active || !state->pending) {
            return;
        }
        if (state->counters.circuitOpenUntilMs > now) {
            rejected = std::move(state->pending);
            state->pending.reset();
            state->wakeScheduled = false;
            ++state->scheduleRevision;
        } else if (state->counters.nextEligibleAtMs > now) {
            if (!state->wakeScheduled) {
                state->wakeScheduled = true;
                scheduleRevision = ++state->scheduleRevision;
                const std::uint64_t delay =
                    state->counters.nextEligibleAtMs - now;
                scheduleDelayMs = static_cast<std::uint32_t>(
                    std::min<std::uint64_t>(
                        delay,
                        std::numeric_limits<std::uint32_t>::max()));
            }
        } else {
            PendingCall pending = std::move(*state->pending);
            state->pending.reset();
            state->wakeScheduled = false;
            ++state->scheduleRevision;
            serialToStart = state->nextSerial++;
            imageToStart = pending.request.image;
            state->active = ActiveCall{
                serialToStart,
                std::move(pending.request),
                std::move(pending.completion),
                0};
        }
    }

    if (rejected) {
        Dispatch(
            state->completionExecutor,
            std::move(rejected->completion),
            MakeResult(
                rejected->request,
                RemoteClassificationStatus::kCircuitOpen,
                "circuit_open"));
    }
    if (scheduleRevision != 0) {
        const std::weak_ptr<VisionApiRuntimeState> weakState = state;
        state->completionExecutor.PostAfter(
            scheduleDelayMs,
            [weakState, scheduleRevision] {
                const auto locked = weakState.lock();
                if (!locked) {
                    return;
                }
                {
                    std::lock_guard lock(locked->mutex);
                    if (!locked->wakeScheduled ||
                        locked->scheduleRevision != scheduleRevision) {
                        return;
                    }
                    locked->wakeScheduled = false;
                }
                AdvancePending(locked);
            });
    }
    if (serialToStart != 0) {
        StartActive(state, serialToStart, std::move(imageToStart));
    }
}

}  // namespace

VisionApiRuntime::VisionApiRuntime(
    VisionApiRuntimeConfig config,
    std::shared_ptr<const IVisionApiProtocol> protocol,
    IHttpTransport& transport,
    ISecretProvider& secrets,
    IClock& clock,
    IExecutor& completionExecutor,
    ILogger* logger)
    : state_(std::make_shared<VisionApiRuntimeState>(
          std::move(config),
          std::move(protocol),
          transport,
          secrets,
          clock,
          completionExecutor,
          logger))
{
}

VisionApiRuntime::~VisionApiRuntime()
{
    Stop();
    state_.reset();
}

RemoteSubmitStatus VisionApiRuntime::Submit(
    RemoteSemanticRequest request,
    Completion completion)
{
    const auto state = state_;
    if (!state) {
        return RemoteSubmitStatus::kRejected;
    }

    if (!request.image.IsValid() || !completion || !state->protocol) {
        Dispatch(
            state->completionExecutor,
            std::move(completion),
            MakeResult(
                request,
                RemoteClassificationStatus::kInvalidRequest,
                "request_invalid"));
        return RemoteSubmitStatus::kRejected;
    }

    std::optional<PendingCall> displaced;
    std::uint64_t serialToStart = 0;
    EncodedImage imageToStart;
    RemoteSubmitStatus submitStatus = RemoteSubmitStatus::kStarted;
    std::optional<RemoteSemanticResult> immediateResult;
    const std::uint64_t now = state->clock.NowMs();
    {
        std::lock_guard lock(state->mutex);
        ++state->counters.submittedRequests;
        if (!state->running || request.generation != state->generation) {
            immediateResult = MakeResult(
                request,
                RemoteClassificationStatus::kCanceled,
                "generation_inactive");
            submitStatus = RemoteSubmitStatus::kRejected;
        } else if (state->counters.circuitOpenUntilMs > now) {
            immediateResult = MakeResult(
                request,
                RemoteClassificationStatus::kCircuitOpen,
                "circuit_open");
            submitStatus = RemoteSubmitStatus::kRejected;
        } else if (state->active || state->counters.nextEligibleAtMs > now) {
            if (state->pending) {
                displaced = std::move(state->pending);
                ++state->counters.replacedSamples;
                submitStatus = RemoteSubmitStatus::kReplacedPending;
            } else {
                submitStatus = RemoteSubmitStatus::kQueued;
            }
            state->pending = PendingCall{
                std::move(request),
                std::move(completion)};
        } else {
            if (state->pending) {
                displaced = std::move(state->pending);
                state->pending.reset();
                ++state->counters.replacedSamples;
            }
            serialToStart = state->nextSerial++;
            imageToStart = request.image;
            state->active = ActiveCall{
                serialToStart,
                std::move(request),
                std::move(completion),
                0};
        }
    }

    if (displaced) {
        Dispatch(
            state->completionExecutor,
            std::move(displaced->completion),
            MakeResult(
                displaced->request,
                RemoteClassificationStatus::kCanceled,
                "sample_replaced"));
    }
    if (immediateResult) {
        Dispatch(
            state->completionExecutor,
            std::move(completion),
            std::move(*immediateResult));
    }
    if (serialToStart != 0) {
        StartActive(state, serialToStart, std::move(imageToStart));
    } else if (submitStatus != RemoteSubmitStatus::kRejected) {
        AdvancePending(state);
    }
    return submitStatus;
}

void VisionApiRuntime::Reset(std::uint64_t generation)
{
    const auto state = state_;
    if (!state) {
        return;
    }

    std::optional<ActiveCall> active;
    std::optional<PendingCall> pending;
    {
        std::lock_guard lock(state->mutex);
        active = std::move(state->active);
        pending = std::move(state->pending);
        state->active.reset();
        state->pending.reset();
        state->running = true;
        state->generation = generation;
        state->wakeScheduled = false;
        ++state->scheduleRevision;
        state->counters.consecutiveFailures = 0;
        state->counters.nextEligibleAtMs = 0;
        state->counters.circuitOpenUntilMs = 0;
    }
    if (active && active->transportRequestId != 0) {
        state->transport.Cancel(active->transportRequestId);
    }
    if (active) {
        Dispatch(
            state->completionExecutor,
            std::move(active->completion),
            MakeResult(
                active->request,
                RemoteClassificationStatus::kCanceled,
                "generation_reset"));
    }
    if (pending) {
        Dispatch(
            state->completionExecutor,
            std::move(pending->completion),
            MakeResult(
                pending->request,
                RemoteClassificationStatus::kCanceled,
                "generation_reset"));
    }
}

void VisionApiRuntime::Stop()
{
    const auto state = state_;
    if (!state) {
        return;
    }

    std::optional<ActiveCall> active;
    std::optional<PendingCall> pending;
    {
        std::lock_guard lock(state->mutex);
        if (!state->running && !state->active && !state->pending) {
            return;
        }
        state->running = false;
        state->wakeScheduled = false;
        ++state->scheduleRevision;
        active = std::move(state->active);
        pending = std::move(state->pending);
        state->active.reset();
        state->pending.reset();
    }
    if (active && active->transportRequestId != 0) {
        state->transport.Cancel(active->transportRequestId);
    }
    if (active) {
        Dispatch(
            state->completionExecutor,
            std::move(active->completion),
            MakeResult(
                active->request,
                RemoteClassificationStatus::kCanceled,
                "runtime_stopped"));
    }
    if (pending) {
        Dispatch(
            state->completionExecutor,
            std::move(pending->completion),
            MakeResult(
                pending->request,
                RemoteClassificationStatus::kCanceled,
                "runtime_stopped"));
    }
}

VisionApiRuntimeSnapshot VisionApiRuntime::Snapshot() const
{
    const auto state = state_;
    if (!state) {
        return {};
    }
    const std::uint64_t now = state->clock.NowMs();
    std::lock_guard lock(state->mutex);
    VisionApiRuntimeSnapshot snapshot = state->counters;
    snapshot.running = state->running;
    snapshot.requestInFlight = state->active.has_value();
    snapshot.pendingSample = state->pending.has_value();
    snapshot.circuitOpen =
        state->counters.circuitOpenUntilMs > now;
    snapshot.generation = state->generation;
    return snapshot;
}

}  // namespace remote::media_intelligence
