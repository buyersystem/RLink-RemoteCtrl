// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)
#pragma once

#include <atomic>
#include <memory>
#include <utility>

#include "api/peer_connection_interface.h"
#include "api/transport/bandwidth_usage.h"
#include "api/transport/network_control.h"
#include "src/core/SessionDiagnostics.h"
#include "src/core/ScreenFrameQualityPolicy.h"

namespace webrtc { class Thread; }
namespace remote {

// Enable this SDK's injected-controller gate and a dormant native recovery
// probe schedule. Preserve the other environment utilities and field trials.
webrtc::Environment CreateGoogCcTelemetryEnvironment(const webrtc::Environment& base,
    bool enableRecoveryHints = true);

// Observes the original controller without rewriting its output. Main-screen
// recovery can supply a bounded historical hint to its native probe controller.
// All timestamps use host steady time, matching the session snapshots.
class GoogCcTelemetryState final {
public:
    static std::uint64_t NowMs();
    void ObserveDelay(webrtc::BandwidthUsage state, std::uint64_t nowMs);
    void ObserveUpdate(const webrtc::NetworkControlUpdate& update,
                       std::uint64_t nowMs);
    void ConfirmFeedback(std::uint64_t nowMs, bool confirmsDelay);
    void ConfirmController(std::uint64_t nowMs);
    void ResetRoute(std::uint64_t nowMs);
    GoogCcNetworkDiagnostics Snapshot(std::uint64_t nowMs) const;

    void SetScreenQualityProtection(bool enabled) noexcept
    { screenQualityProtection_.store(enabled, std::memory_order_release); }
    bool ScreenQualityProtectionEnabled() const noexcept
    { return screenQualityProtection_.load(std::memory_order_acquire); }
    void SetScreenQualityDeficitShare(std::uint32_t hundredths) noexcept
    { screenQualityDeficitShare_.store(NormalizeScreenQualityDeficitShareHundredths(hundredths),
                                      std::memory_order_release); }
    std::uint32_t ScreenQualityDeficitShare() const noexcept
    { return screenQualityDeficitShare_.load(std::memory_order_acquire); }
    void SetScreenQualityTarget(std::uint32_t fps, std::uint32_t videoCapBps) noexcept
    { screenQualityTarget_.store((static_cast<std::uint64_t>(fps) << 32) | videoCapBps,
                                 std::memory_order_release); }
    std::pair<std::uint32_t, std::uint32_t> ScreenQualityTarget() const noexcept
    {
        const auto target = screenQualityTarget_.load(std::memory_order_acquire);
        return {static_cast<std::uint32_t>(target >> 32), static_cast<std::uint32_t>(target)};
    }
    void SetScreenRecoveryProbeEnabled(bool enabled) noexcept
    { screenRecoveryProbeEnabled_.store(enabled, std::memory_order_release); }
    bool ScreenRecoveryProbeEnabled() const noexcept
    { return screenRecoveryProbeEnabled_.load(std::memory_order_acquire); }
    void RecordScreenRecoveryProbe(bool active, std::uint32_t remainingAttempts,
                                   std::uint64_t historicalBudgetBps,
                                   std::uint32_t actualProbes,
                                   std::uint32_t lifetimeEpisodes) noexcept
    {
        recoveryProbeRemainingAttempts_.store(remainingAttempts, std::memory_order_relaxed);
        recoveryProbeHistoricalBudgetBps_.store(historicalBudgetBps, std::memory_order_relaxed);
        recoveryProbeAttempts_.store(actualProbes, std::memory_order_relaxed);
        recoveryProbeEpisodes_.store(lifetimeEpisodes, std::memory_order_relaxed);
        recoveryProbeActive_.store(active, std::memory_order_release);
    }

private:
    std::atomic<bool> screenRecoveryProbeEnabled_{false};
    std::atomic<bool> recoveryProbeActive_{false};
    std::atomic<std::uint32_t> recoveryProbeRemainingAttempts_{0};
    std::atomic<std::uint64_t> recoveryProbeHistoricalBudgetBps_{0};
    std::atomic<std::uint32_t> recoveryProbeAttempts_{0};
    std::atomic<std::uint32_t> recoveryProbeEpisodes_{0};
    std::atomic<bool> screenQualityProtection_{false};
    std::atomic<std::uint32_t> screenQualityDeficitShare_{kDefaultScreenQualityDeficitShareHundredths};
    std::atomic<std::uint64_t> screenQualityTarget_{0};
    std::atomic<int> delayState_{-1};
    std::atomic<bool> controllerObserved_{false};
    std::atomic<bool> resetting_{false};
    // Single controller writer; odd values mark an output being published.
    std::atomic<std::uint64_t> targetSequence_{0};
    std::atomic<std::uint64_t> routeRevision_{0};
    std::atomic<std::uint64_t> targetUpdatedAtMs_{0};
    std::atomic<std::uint64_t> feedbackAtMs_{0};
    std::atomic<std::uint64_t> delayUpdatedAtMs_{0};
    std::atomic<std::uint64_t> lastDelayOveruseAtMs_{0};
    std::atomic<std::uint64_t> lastCwndPushbackAtMs_{0};
    std::atomic<std::uint64_t> targetRateBps_{0};
    std::atomic<std::uint64_t> effectiveTargetRateBps_{0};
    std::atomic<double> cwndReduction_{0};
    std::atomic<double> rttMs_{0};
    std::atomic<double> lossPercent_{0};
    std::atomic<bool> applicationLimited_{false};
};

// Resolve once when an encoder is created. The environment's event log is
// unique to its PeerConnection; unrelated connections never share this mode.
std::shared_ptr<GoogCcTelemetryState> GoogCcTelemetryForEnvironment(
    const webrtc::Environment& environment);

std::unique_ptr<webrtc::NetworkControllerInterface>
CreateObservingNetworkController(
    std::unique_ptr<webrtc::NetworkControllerInterface> delegate,
    std::shared_ptr<GoogCcTelemetryState> state, bool enableRecoveryHints = true);

// Pending binding is set on the factory's signaling thread. Its event-log
// factory reads the binding during the synchronous worker-thread Create call.
// Consequently each PC's environment identifies exactly its own telemetry.
class GoogCcTelemetryFactoryContext final {
public:
    explicit GoogCcTelemetryFactoryContext(webrtc::Thread* signalingThread);
    ~GoogCcTelemetryFactoryContext();
    struct Impl;
    std::unique_ptr<Impl> impl;
};

std::unique_ptr<webrtc::RtcEventLogFactoryInterface>
CreateGoogCcTelemetryEventLogFactory(
    std::shared_ptr<GoogCcTelemetryFactoryContext> context);
std::unique_ptr<webrtc::NetworkControllerFactoryInterface>
CreateGoogCcTelemetryControllerFactory(
    std::shared_ptr<GoogCcTelemetryFactoryContext> context, bool enableRecoveryHints = true);
void RegisterGoogCcTelemetryFactory(
    webrtc::PeerConnectionFactoryInterface* factory,
    const std::shared_ptr<GoogCcTelemetryFactoryContext>& context);
void UnregisterGoogCcTelemetryFactory(
    webrtc::PeerConnectionFactoryInterface* factory);
webrtc::RTCErrorOr<webrtc::scoped_refptr<webrtc::PeerConnectionInterface>>
CreatePeerConnectionWithGoogCcTelemetry(
    const webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>& factory,
    const webrtc::PeerConnectionInterface::RTCConfiguration& configuration,
    webrtc::PeerConnectionDependencies dependencies,
    std::shared_ptr<GoogCcTelemetryState> state);

}  // namespace remote
