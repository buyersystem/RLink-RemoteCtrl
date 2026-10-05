// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)
#include "GoogCcTelemetry.h"
#include "src/core/ScreenRecoveryProbePolicy.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <mutex>
#include <unordered_map>
#include <utility>

#include "api/rtc_event_log/rtc_event_log_factory.h"
#include "api/environment/environment_factory.h"
#include "api/transport/goog_cc_factory.h"
#include "logging/rtc_event_log/events/rtc_event_bwe_update_delay_based.h"
#include "rtc_base/thread.h"

namespace remote {
namespace {
constexpr std::uint64_t kMaximumFeedbackAgeMs = 3000;
class TelemetryFieldTrials final : public webrtc::FieldTrialsView {
public:
    explicit TelemetryFieldTrials(const webrtc::Environment& base, bool enableRecoveryHints)
        : base_(base), enableRecoveryHints_(enableRecoveryHints) {}
    std::string Lookup(absl::string_view key) const override
    {
        if (key == "WebRTC-Bwe-InjectedCongestionController") return "Enabled";
        // This schedule is dormant unless an actual main-screen congestion
        // episode installs a short-lived, previously measured capacity hint.
        // ProbeController retains its native guards, ceilings and 2x probe
        // scale. Static low flow alone cannot start a hint; actual screen
        // congestion can. Camera-only connections receive no recovery hint.
        if (enableRecoveryHints_ && key == "WebRTC-Bwe-ProbingConfiguration") {
            auto configured = base_.field_trials().Lookup(key);
            if (!configured.empty()) configured += ',';
            return configured + "est_lower_than_network_ratio:0.95,"
                "est_lower_than_network_interval:1s,network_state_interval:1s,"
                "network_state_probe_duration:15ms,network_state_min_probe_delta:2ms,"
                "network_state_scale:2";
        }
        return base_.field_trials().Lookup(key);
    }
    bool IsTest() const override { return base_.field_trials().IsTest(); }
    std::unique_ptr<webrtc::FieldTrialsView> CreateCopy() const override
    { return std::make_unique<TelemetryFieldTrials>(base_, enableRecoveryHints_); }
private:
    webrtc::Environment base_;
    bool enableRecoveryHints_;
};
std::mutex factoryRegistryMutex;
std::unordered_map<webrtc::PeerConnectionFactoryInterface*,
                   std::weak_ptr<GoogCcTelemetryFactoryContext>> factoryRegistry;

std::uint32_t AgeMs(std::uint64_t now, std::uint64_t timestamp)
{
    if (timestamp == 0 || now < timestamp) {
        return std::numeric_limits<std::uint32_t>::max();
    }
    return static_cast<std::uint32_t>(std::min<std::uint64_t>(
        now - timestamp, std::numeric_limits<std::uint32_t>::max()));
}

webrtc::TransportPacketsFeedback TransferFeedback(
    webrtc::TransportPacketsFeedback& message)
{
    // This WebRTC version explicitly declares a copy constructor but no move
    // constructor. Forwarding std::move(message) would therefore copy both
    // vectors. Transfer their ownership into the returned value instead; the
    // Release compiler elides the local return object (NRVO), and the returned
    // prvalue initializes the delegate's argument directly.
    webrtc::TransportPacketsFeedback transferred;
    transferred.feedback_time = message.feedback_time;
    transferred.data_in_flight = message.data_in_flight;
    transferred.transport_supports_ecn = message.transport_supports_ecn;
    transferred.packet_feedbacks.swap(message.packet_feedbacks);
    transferred.sendless_arrival_times.swap(message.sendless_arrival_times);
    return transferred;
}
}  // namespace

webrtc::Environment CreateGoogCcTelemetryEnvironment(const webrtc::Environment& base,
    bool enableRecoveryHints)
{
    webrtc::EnvironmentFactory factory(base);
    factory.Set(std::make_unique<TelemetryFieldTrials>(base, enableRecoveryHints));
    return factory.Create();
}

std::uint64_t GoogCcTelemetryState::NowMs()
{
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
}

void GoogCcTelemetryState::ObserveDelay(
    webrtc::BandwidthUsage state, std::uint64_t nowMs)
{
    if (state == webrtc::BandwidthUsage::kLast) {
        return;
    }
    delayState_.store(static_cast<int>(state), std::memory_order_relaxed);
    if (state == webrtc::BandwidthUsage::kBwOverusing)
        lastDelayOveruseAtMs_.store(nowMs, std::memory_order_release);
    delayUpdatedAtMs_.store(nowMs, std::memory_order_release);
}

void GoogCcTelemetryState::ObserveUpdate(
    const webrtc::NetworkControlUpdate& update, std::uint64_t nowMs)
{
    if (!update.target_rate) {
        return;
    }
    const auto& target = *update.target_rate;
    if (!target.target_rate.IsFinite()) {
        return;
    }
    const auto rate = static_cast<std::uint64_t>(
        std::max<std::int64_t>(0, target.target_rate.bps()));
    const double reduction = std::isfinite(target.cwnd_reduce_ratio)
        ? std::clamp(target.cwnd_reduce_ratio, 0.0, 1.0) : 0.0;
    targetSequence_.fetch_add(1, std::memory_order_acq_rel);
    targetRateBps_.store(rate, std::memory_order_relaxed);
    // Drop-frame-only mode reports the unreduced rate and a separate ratio.
    // This derived diagnostic budget observes that instruction without altering
    // the update returned to WebRTC or double-reducing ordinary pushback mode.
    effectiveTargetRateBps_.store(static_cast<std::uint64_t>(
        static_cast<double>(rate) * (1.0 - reduction)),
        std::memory_order_relaxed);
    cwndReduction_.store(reduction, std::memory_order_relaxed);
    if (reduction > 0.0) lastCwndPushbackAtMs_.store(nowMs, std::memory_order_relaxed);
    const auto rtt = target.network_estimate.round_trip_time;
    rttMs_.store(rtt.IsFinite() ? std::max(0.0, rtt.ms<double>()) : 0.0,
                 std::memory_order_relaxed);
    const double loss = target.network_estimate.loss_rate_ratio;
    lossPercent_.store(std::isfinite(loss)
        ? std::clamp(loss, 0.0, 1.0) * 100.0 : 0.0,
        std::memory_order_relaxed);
    applicationLimited_.store(!target.is_bandwidth_limited,
                              std::memory_order_relaxed);
    controllerObserved_.store(true, std::memory_order_relaxed);
    targetUpdatedAtMs_.store(nowMs, std::memory_order_release);
    targetSequence_.fetch_add(1, std::memory_order_release);
}

void GoogCcTelemetryState::ConfirmFeedback(
    std::uint64_t nowMs, bool confirmsDelay)
{
    ConfirmController(nowMs);
    feedbackAtMs_.store(nowMs, std::memory_order_release);
    // Delay events are change notifications, not a heartbeat. A completed
    // receive-time feedback calculation confirms an already observed state.
    // Never turn the default/unknown state into an inferred "normal" state.
    if (confirmsDelay && delayState_.load(std::memory_order_relaxed) >= 0) {
        delayUpdatedAtMs_.store(nowMs, std::memory_order_release);
    }
}

void GoogCcTelemetryState::ConfirmController(std::uint64_t nowMs)
{
    if (controllerObserved_.load(std::memory_order_relaxed)) {
        targetUpdatedAtMs_.store(nowMs, std::memory_order_release);
    }
}

void GoogCcTelemetryState::ResetRoute(std::uint64_t /*nowMs*/)
{
    resetting_.store(true, std::memory_order_release);
    controllerObserved_.store(false, std::memory_order_relaxed);
    delayState_.store(-1, std::memory_order_relaxed);
    targetUpdatedAtMs_.store(0, std::memory_order_relaxed);
    feedbackAtMs_.store(0, std::memory_order_relaxed);
    delayUpdatedAtMs_.store(0, std::memory_order_relaxed);
    lastDelayOveruseAtMs_.store(0, std::memory_order_relaxed);
    lastCwndPushbackAtMs_.store(0, std::memory_order_relaxed);
    targetRateBps_.store(0, std::memory_order_relaxed);
    effectiveTargetRateBps_.store(0, std::memory_order_relaxed);
    cwndReduction_.store(0, std::memory_order_relaxed);
    rttMs_.store(0, std::memory_order_relaxed);
    lossPercent_.store(0, std::memory_order_relaxed);
    applicationLimited_.store(false, std::memory_order_relaxed);
    RecordScreenRecoveryProbe(false, 0, 0, 0, 0);
    routeRevision_.fetch_add(1, std::memory_order_release);
    resetting_.store(false, std::memory_order_release);
}

GoogCcNetworkDiagnostics GoogCcTelemetryState::Snapshot(
    std::uint64_t nowMs) const
{
    GoogCcNetworkDiagnostics result;
    if (resetting_.load(std::memory_order_acquire)) {
        result.routeRevision = routeRevision_.load(std::memory_order_acquire);
        return result;
    }
    const auto targetSequence = targetSequence_.load(std::memory_order_acquire);
    if ((targetSequence & 1) != 0) {
        result.routeRevision = routeRevision_.load(std::memory_order_acquire);
        return result;
    }
    result.routeRevision = routeRevision_.load(std::memory_order_acquire);
    result.controllerObserved = controllerObserved_.load(std::memory_order_relaxed);
    result.targetUpdatedAtMs = targetUpdatedAtMs_.load(std::memory_order_acquire);
    result.feedbackAtMs = feedbackAtMs_.load(std::memory_order_acquire);
    result.delayUpdatedAtMs = delayUpdatedAtMs_.load(std::memory_order_acquire);
    result.lastDelayOveruseAtMs = lastDelayOveruseAtMs_.load(std::memory_order_acquire);
    result.lastCwndPushbackAtMs = lastCwndPushbackAtMs_.load(std::memory_order_relaxed);
    const int delay = delayState_.load(std::memory_order_relaxed);
    result.delayObserved = delay >= 0;
    switch (static_cast<webrtc::BandwidthUsage>(delay)) {
    case webrtc::BandwidthUsage::kBwNormal: result.delayState = "normal"; break;
    case webrtc::BandwidthUsage::kBwUnderusing: result.delayState = "underuse"; break;
    case webrtc::BandwidthUsage::kBwOverusing: result.delayState = "overuse"; break;
    default: result.delayState = "unknown"; break;
    }
    result.feedbackAgeMs = AgeMs(nowMs, result.feedbackAtMs);
    result.feedbackFresh = result.controllerObserved &&
        result.feedbackAgeMs <= kMaximumFeedbackAgeMs;
    result.targetRateBps = targetRateBps_.load(std::memory_order_relaxed);
    result.effectiveTargetRateBps = effectiveTargetRateBps_.load(std::memory_order_relaxed);
    result.congestionWindowReduction = cwndReduction_.load(std::memory_order_relaxed);
    result.roundTripTimeMs = rttMs_.load(std::memory_order_relaxed);
    result.lossPercent = lossPercent_.load(std::memory_order_relaxed);
    result.applicationLimited = applicationLimited_.load(std::memory_order_relaxed);
    result.recoveryProbeActive = recoveryProbeActive_.load(std::memory_order_acquire);
    result.recoveryProbeRemainingAttempts = recoveryProbeRemainingAttempts_.load(std::memory_order_relaxed);
    result.recoveryProbeHistoricalBudgetBps = recoveryProbeHistoricalBudgetBps_.load(std::memory_order_relaxed);
    result.recoveryProbeAttempts = recoveryProbeAttempts_.load(std::memory_order_relaxed);
    result.recoveryProbeEpisodes = recoveryProbeEpisodes_.load(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);
    // A reset may overlap this lock-free read. Return an unknown snapshot rather
    // than combine the old detector state with a new route's target budget.
    const auto finalRevision = routeRevision_.load(std::memory_order_acquire);
    if (resetting_.load(std::memory_order_acquire) ||
        finalRevision != result.routeRevision ||
        targetSequence_.load(std::memory_order_acquire) != targetSequence) {
        result = {};
        result.routeRevision = finalRevision;
    }
    return result;
}

namespace {
class ObservingNetworkController final : public webrtc::NetworkControllerInterface {
public:
    ObservingNetworkController(
        std::unique_ptr<webrtc::NetworkControllerInterface> delegate,
        std::shared_ptr<GoogCcTelemetryState> state, bool enableRecoveryHints)
        : delegate_(std::move(delegate)), state_(std::move(state)),
          enableRecoveryHints_(enableRecoveryHints)
    {
        state_->ResetRoute(GoogCcTelemetryState::NowMs());
    }
    ~ObservingNetworkController() override
    {
        state_->ResetRoute(GoogCcTelemetryState::NowMs());
    }

    webrtc::NetworkControlUpdate OnNetworkAvailability(
        webrtc::NetworkAvailability message) override
    {
        if (!message.network_available) {
            ResetRecoveryHint(message.at_time);
            state_->ResetRoute(GoogCcTelemetryState::NowMs());
        }
        return Observe(delegate_->OnNetworkAvailability(std::move(message)));
    }
    webrtc::NetworkControlUpdate OnNetworkRouteChange(
        webrtc::NetworkRouteChange message) override
    {
        ResetRecoveryHint(message.at_time);
        state_->ResetRoute(GoogCcTelemetryState::NowMs());
        return Observe(delegate_->OnNetworkRouteChange(std::move(message)));
    }

#define RLINK_OBSERVE_CALLBACK(Name, Type) \
    webrtc::NetworkControlUpdate Name(webrtc::Type message) override \
    { return Observe(delegate_->Name(std::move(message))); }
    RLINK_OBSERVE_CALLBACK(OnRemoteBitrateReport, RemoteBitrateReport)
    RLINK_OBSERVE_CALLBACK(OnRoundTripTimeUpdate, RoundTripTimeUpdate)
    RLINK_OBSERVE_CALLBACK(OnSentPacket, SentPacket)
    RLINK_OBSERVE_CALLBACK(OnReceivedPacket, ReceivedPacket)
    RLINK_OBSERVE_CALLBACK(OnStreamsConfig, StreamsConfig)
    RLINK_OBSERVE_CALLBACK(OnTargetRateConstraints, TargetRateConstraints)
    RLINK_OBSERVE_CALLBACK(OnNetworkStateEstimate, NetworkStateEstimate)
#undef RLINK_OBSERVE_CALLBACK

    webrtc::NetworkControlUpdate OnProcessInterval(
        webrtc::ProcessInterval message) override
    {
        PrepareRecoveryHint(message.at_time);
        auto update = Observe(delegate_->OnProcessInterval(std::move(message)));
        state_->ConfirmController(GoogCcTelemetryState::NowMs());
        // Processing is not incoming feedback and cannot keep delay evidence
        // alive after the remote endpoint stops acknowledging packets.
        PrepareRecoveryHint(lastNativeTime_);
        return update;
    }

    webrtc::NetworkControlUpdate OnTransportLossReport(
        webrtc::TransportLossReport message) override
    {
        const bool hasFeedback = message.packets_received_delta != 0 ||
            message.packets_lost_delta != 0;
        auto update = delegate_->OnTransportLossReport(std::move(message));
        if (hasFeedback) {
            state_->ConfirmFeedback(GoogCcTelemetryState::NowMs(), false);
        }
        return Observe(std::move(update));
    }
    webrtc::NetworkControlUpdate OnTransportPacketsFeedback(
        webrtc::TransportPacketsFeedback message) override
    {
        const auto feedbackTime = message.feedback_time;
        // The delegate logs detector changes synchronously. Confirm afterward
        // so an unchanged state remains fresh while real feedback continues.
        const bool hasFeedback = !message.packet_feedbacks.empty();
        const bool hasReceiveTimes = std::any_of(
            message.packet_feedbacks.begin(), message.packet_feedbacks.end(),
            [](const auto& packet) {
                return packet.IsReceived() && packet.sent_packet.send_time.IsFinite();
            });
        auto update = delegate_->OnTransportPacketsFeedback(TransferFeedback(message));
        if (hasFeedback) {
            state_->ConfirmFeedback(GoogCcTelemetryState::NowMs(), hasReceiveTimes);
        }
        if (hasReceiveTimes) lastReceiveFeedbackAtMs_ = GoogCcTelemetryState::NowMs();
        auto observed = Observe(std::move(update));
        PrepareRecoveryHint(feedbackTime);
        return observed;
    }
    bool SupportsEcnAdaptation() const override
    { return delegate_->SupportsEcnAdaptation(); }

private:
    webrtc::NetworkControlUpdate Observe(webrtc::NetworkControlUpdate update)
    {
        if (!update.probe_cluster_configs.empty()) {
            recoveryPolicy_.ObserveProbes(static_cast<std::uint32_t>(
                update.probe_cluster_configs.size()), GoogCcTelemetryState::NowMs());
        }
        if (update.target_rate) {
            state_->ObserveUpdate(update, GoogCcTelemetryState::NowMs());
        }
        PublishRecoveryDiagnostics();
        return update;
    }
    void PublishRecoveryDiagnostics()
    {
        state_->RecordScreenRecoveryProbe(recoveryPolicy_.Active(),
            recoveryPolicy_.Active() && recoveryPolicy_.EpisodeProbes() <
                recoveryPolicy_.ProbeLimit() ? recoveryPolicy_.ProbeLimit() -
                recoveryPolicy_.EpisodeProbes() : 0,
            recoveryPolicy_.HistoricalBudgetBps(), recoveryPolicy_.EpisodeProbes(),
            recoveryPolicy_.Episodes());
    }
    void SendRecoveryHint(webrtc::Timestamp atTime, bool install)
    {
        if (!atTime.IsFinite()) return;
        // Native OnNetworkStateEstimate records the hint for probing and a
        // conservative upper constraint. Its zero lower bound cannot install
        // a higher sending-rate prior; only real feedback can raise the target.
        webrtc::NetworkStateEstimate hint;
        hint.update_time = atTime;
        hint.link_capacity_lower = webrtc::DataRate::Zero();
        hint.link_capacity_upper = install
            ? webrtc::DataRate::BitsPerSec(recoveryPolicy_.HistoricalBudgetBps())
            : webrtc::DataRate::PlusInfinity();
        // Install and clear can occur at the same native timestamp (e.g. a
        // returned probe reaches the episode limit). Both must be consumed.
        if (lastHintTime_.IsFinite() && hint.update_time <= lastHintTime_)
            hint.update_time = lastHintTime_ + webrtc::TimeDelta::Micros(1);
        lastHintTime_ = hint.update_time;
        delegate_->OnNetworkStateEstimate(hint);
        nativeHintInstalled_ = install;
    }
    void ResetRecoveryHint(webrtc::Timestamp atTime)
    {
        if (nativeHintInstalled_) SendRecoveryHint(atTime, false);
        recoveryPolicy_.Reset();
        lastReceiveFeedbackAtMs_ = 0;
        PublishRecoveryDiagnostics();
    }
    void PrepareRecoveryHint(webrtc::Timestamp atTime)
    {
        if (!atTime.IsFinite()) return;
        lastNativeTime_ = atTime;
        const auto now = GoogCcTelemetryState::NowMs();
        const auto network = state_->Snapshot(now);
        const auto target = state_->ScreenQualityTarget();
        ScreenRecoveryProbeSample sample;
        sample.nowMs = now;
        sample.routeRevision = network.routeRevision;
        sample.feedbackAtMs = lastReceiveFeedbackAtMs_;
        sample.feedbackFresh = network.feedbackFresh;
        sample.budgetBps = network.effectiveTargetRateBps;
        sample.congestionEventAtMs = (std::max)(network.lastDelayOveruseAtMs,
                                               network.lastCwndPushbackAtMs);
        sample.targetFps = target.first;
        sample.videoCapBps = target.second;
        sample.enabled = enableRecoveryHints_ && state_->ScreenRecoveryProbeEnabled();
        sample.healthy = network.delayObserved &&
            (network.delayState == "normal" || network.delayState == "underuse") &&
            network.delayUpdatedAtMs != 0 && now >= network.delayUpdatedAtMs &&
            now - network.delayUpdatedAtMs <= 1000 &&
            network.congestionWindowReduction < 0.01 && network.lossPercent < 2.0;
        const auto change = recoveryPolicy_.Update(sample);
        if (change == ScreenRecoveryHintChange::kInstall) SendRecoveryHint(atTime, true);
        else if (change == ScreenRecoveryHintChange::kClear) SendRecoveryHint(atTime, false);
        PublishRecoveryDiagnostics();
    }
    std::unique_ptr<webrtc::NetworkControllerInterface> delegate_;
    std::shared_ptr<GoogCcTelemetryState> state_;
    ScreenRecoveryProbePolicy recoveryPolicy_;
    webrtc::Timestamp lastNativeTime_ = webrtc::Timestamp::MinusInfinity();
    webrtc::Timestamp lastHintTime_ = webrtc::Timestamp::MinusInfinity();
    bool nativeHintInstalled_ = false;
    const bool enableRecoveryHints_;
    std::uint64_t lastReceiveFeedbackAtMs_ = 0;
};
}  // namespace

std::unique_ptr<webrtc::NetworkControllerInterface>
CreateObservingNetworkController(
    std::unique_ptr<webrtc::NetworkControllerInterface> delegate,
    std::shared_ptr<GoogCcTelemetryState> state, bool enableRecoveryHints)
{
    if (!delegate || !state) {
        return delegate;
    }
    return std::make_unique<ObservingNetworkController>(
        std::move(delegate), std::move(state), enableRecoveryHints);
}

struct GoogCcTelemetryFactoryContext::Impl {
    explicit Impl(webrtc::Thread* thread) : signalingThread(thread) {}
    webrtc::Thread* signalingThread;
    std::mutex mutex;
    std::shared_ptr<GoogCcTelemetryState> pendingState;
    std::unordered_map<const webrtc::RtcEventLog*,
                       std::weak_ptr<GoogCcTelemetryState>> eventLogs;
};

GoogCcTelemetryFactoryContext::GoogCcTelemetryFactoryContext(
    webrtc::Thread* signalingThread) : impl(std::make_unique<Impl>(signalingThread)) {}
GoogCcTelemetryFactoryContext::~GoogCcTelemetryFactoryContext() = default;

std::shared_ptr<GoogCcTelemetryState> GoogCcTelemetryForEnvironment(
    const webrtc::Environment& environment)
{
    std::lock_guard registryLock(factoryRegistryMutex);
    for (const auto& [factory, weakContext] : factoryRegistry) {
        const auto context = weakContext.lock();
        if (!context) continue;
        std::lock_guard contextLock(context->impl->mutex);
        const auto found = context->impl->eventLogs.find(&environment.event_log());
        if (found != context->impl->eventLogs.end()) return found->second.lock();
    }
    return {};
}

namespace {
class ObservingEventLog final : public webrtc::RtcEventLog {
public:
    ObservingEventLog(std::unique_ptr<webrtc::RtcEventLog> delegate,
                     std::shared_ptr<GoogCcTelemetryState> state,
                     std::shared_ptr<GoogCcTelemetryFactoryContext> context)
        : delegate_(std::move(delegate)), state_(std::move(state)),
          context_(std::move(context)) {}
    ~ObservingEventLog() override
    {
        std::lock_guard lock(context_->impl->mutex);
        context_->impl->eventLogs.erase(this);
    }
    bool StartLogging(std::unique_ptr<webrtc::RtcEventLogOutput> output,
                      std::int64_t periodMs) override
    { return delegate_->StartLogging(std::move(output), periodMs); }
    void StopLogging() override { delegate_->StopLogging(); }
    void StopLogging(std::function<void()> callback) override
    { delegate_->StopLogging(std::move(callback)); }
    void Log(std::unique_ptr<webrtc::RtcEvent> event) override
    {
        if (state_ && event && event->GetType() ==
            webrtc::RtcEventBweUpdateDelayBased::kType) {
            state_->ObserveDelay(static_cast<const
                webrtc::RtcEventBweUpdateDelayBased&>(*event).detector_state(),
                GoogCcTelemetryState::NowMs());
        }
        delegate_->Log(std::move(event));
    }
private:
    std::unique_ptr<webrtc::RtcEventLog> delegate_;
    std::shared_ptr<GoogCcTelemetryState> state_;
    std::shared_ptr<GoogCcTelemetryFactoryContext> context_;
};

class ObservingEventLogFactory final : public webrtc::RtcEventLogFactoryInterface {
public:
    explicit ObservingEventLogFactory(
        std::shared_ptr<GoogCcTelemetryFactoryContext> context)
        : context_(std::move(context)) {}
    std::unique_ptr<webrtc::RtcEventLog> Create(
        const webrtc::Environment& env) const override
    {
        auto original = delegate_.Create(env);
        std::lock_guard lock(context_->impl->mutex);
        auto result = std::make_unique<ObservingEventLog>(
            std::move(original), context_->impl->pendingState, context_);
        if (context_->impl->pendingState) {
            context_->impl->eventLogs.emplace(result.get(), context_->impl->pendingState);
        }
        return result;
    }
private:
    webrtc::RtcEventLogFactory delegate_;
    std::shared_ptr<GoogCcTelemetryFactoryContext> context_;
};

class ObservingControllerFactory final : public webrtc::NetworkControllerFactoryInterface {
public:
    explicit ObservingControllerFactory(
        std::shared_ptr<GoogCcTelemetryFactoryContext> context, bool enableRecoveryHints)
        : context_(std::move(context)), enableRecoveryHints_(enableRecoveryHints) {}
    std::unique_ptr<webrtc::NetworkControllerInterface> Create(
        webrtc::NetworkControllerConfig config) override
    {
        std::shared_ptr<GoogCcTelemetryState> state;
        {
            std::lock_guard lock(context_->impl->mutex);
            auto found = context_->impl->eventLogs.find(&config.env.event_log());
            if (found != context_->impl->eventLogs.end()) {
                state = found->second.lock();
            }
        }
        return CreateObservingNetworkController(delegate_.Create(std::move(config)),
                                                std::move(state), enableRecoveryHints_);
    }
    webrtc::TimeDelta GetProcessInterval() const override
    { return delegate_.GetProcessInterval(); }
private:
    webrtc::GoogCcNetworkControllerFactory delegate_;
    std::shared_ptr<GoogCcTelemetryFactoryContext> context_;
    const bool enableRecoveryHints_;
};
}  // namespace

std::unique_ptr<webrtc::RtcEventLogFactoryInterface>
CreateGoogCcTelemetryEventLogFactory(
    std::shared_ptr<GoogCcTelemetryFactoryContext> context)
{ return std::make_unique<ObservingEventLogFactory>(std::move(context)); }

std::unique_ptr<webrtc::NetworkControllerFactoryInterface>
CreateGoogCcTelemetryControllerFactory(
    std::shared_ptr<GoogCcTelemetryFactoryContext> context, bool enableRecoveryHints)
{ return std::make_unique<ObservingControllerFactory>(std::move(context), enableRecoveryHints); }

void RegisterGoogCcTelemetryFactory(
    webrtc::PeerConnectionFactoryInterface* factory,
    const std::shared_ptr<GoogCcTelemetryFactoryContext>& context)
{
    std::lock_guard lock(factoryRegistryMutex);
    factoryRegistry.insert_or_assign(factory, context);
}
void UnregisterGoogCcTelemetryFactory(webrtc::PeerConnectionFactoryInterface* factory)
{
    std::lock_guard lock(factoryRegistryMutex);
    factoryRegistry.erase(factory);
}

webrtc::RTCErrorOr<webrtc::scoped_refptr<webrtc::PeerConnectionInterface>>
CreatePeerConnectionWithGoogCcTelemetry(
    const webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>& factory,
    const webrtc::PeerConnectionInterface::RTCConfiguration& configuration,
    webrtc::PeerConnectionDependencies dependencies,
    std::shared_ptr<GoogCcTelemetryState> state)
{
    std::shared_ptr<GoogCcTelemetryFactoryContext> context;
    {
        std::lock_guard lock(factoryRegistryMutex);
        const auto found = factoryRegistry.find(factory.get());
        if (found != factoryRegistry.end()) {
            context = found->second.lock();
        }
    }
    if (!context || !context->impl->signalingThread) {
        return factory->CreatePeerConnectionOrError(configuration, std::move(dependencies));
    }
    // Factory proxies already serialize creation on signaling. Establish the
    // binding inside that same serialization so concurrent sessions cannot
    // attach telemetry to one another's event logs.
    return context->impl->signalingThread->BlockingCall([&]() {
        {
            std::lock_guard lock(context->impl->mutex);
            context->impl->pendingState = std::move(state);
        }
        auto result = factory->CreatePeerConnectionOrError(
            configuration, std::move(dependencies));
        {
            std::lock_guard lock(context->impl->mutex);
            context->impl->pendingState.reset();
        }
        return result;
    });
}

}  // namespace remote
