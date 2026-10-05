// SPDX-License-Identifier: GPL-3.0-only
#include "src/webrtc/GoogCcTelemetry.h"
#include "src/core/ScreenRecoveryProbePolicy.h"
#include "api/environment/environment_factory.h"
#include "modules/congestion_controller/goog_cc/probe_controller.h"
#include <cmath>
#include <iostream>
#include <thread>
#include <utility>

namespace {
using namespace webrtc;
using namespace remote;
class FixtureTrials final : public FieldTrialsView {
public:
    std::string Lookup(absl::string_view key) const override {
        if (key == "WebRTC-Bwe-InjectedCongestionController") return "Disabled";
        if (key == "WebRTC-PcFactoryDefaultBitrates") return "Enabled-min:100kbps";
        return {};
    }
};
bool Check(bool ok, const char* name) {
    std::cout << name << '=' << (ok ? "PASS" : "FAIL") << '\n';
    return ok;
}
NetworkControlUpdate Fixture(int marker) {
    NetworkControlUpdate update;
    TargetTransferRate target;
    target.at_time = Timestamp::Millis(marker);
    target.target_rate = DataRate::BitsPerSec(10'000'000 + marker);
    target.cwnd_reduce_ratio = 0.25;
    target.network_estimate.round_trip_time = TimeDelta::Millis(75);
    target.network_estimate.loss_rate_ratio = 0.06f;
    target.is_bandwidth_limited = false;
    update.target_rate = target;
    update.congestion_window = DataSize::Bytes(200'000 + marker);
    update.pacer_config = PacerConfig::Create(Timestamp::Millis(marker),
        DataRate::BitsPerSec(25'000'000), DataRate::BitsPerSec(100'000));
    ProbeClusterConfig probe;
    probe.id = marker;
    probe.target_data_rate = DataRate::BitsPerSec(30'000'000);
    probe.target_duration = TimeDelta::Millis(20);
    probe.target_probe_count = 5;
    update.probe_cluster_configs.push_back(probe);
    return update;
}
bool Unchanged(const NetworkControlUpdate& update, int marker) {
    return update.target_rate && update.target_rate->at_time == Timestamp::Millis(marker) &&
        update.target_rate->target_rate.bps() == 10'000'000 + marker &&
        update.target_rate->cwnd_reduce_ratio == 0.25 &&
        update.target_rate->network_estimate.round_trip_time == TimeDelta::Millis(75) &&
        update.target_rate->network_estimate.loss_rate_ratio == 0.06f &&
        !update.target_rate->is_bandwidth_limited && update.congestion_window &&
        update.congestion_window->bytes() == 200'000 + marker && update.pacer_config &&
        update.pacer_config->at_time == Timestamp::Millis(marker) &&
        update.pacer_config->data_rate().bps() == 25'000'000 &&
        update.pacer_config->pad_rate().bps() == 100'000 &&
        update.probe_cluster_configs.size() == 1 && update.probe_cluster_configs[0].id == marker &&
        update.probe_cluster_configs[0].target_data_rate.bps() == 30'000'000 &&
        update.probe_cluster_configs[0].target_duration == TimeDelta::Millis(20) &&
        update.probe_cluster_configs[0].target_probe_count == 5;
}
class Controller final : public NetworkControllerInterface {
public:
    bool emptyProcess = false;
    const PacketResult* packetStorage = nullptr;
    const Timestamp* arrivalStorage = nullptr;
    bool feedbackFieldsPreserved = false;
    std::shared_ptr<GoogCcTelemetryState> state;
    NetworkControlUpdate OnProcessInterval(ProcessInterval) override {
        return emptyProcess ? NetworkControlUpdate{} : Fixture(3);
    }
    NetworkControlUpdate OnTransportPacketsFeedback(TransportPacketsFeedback feedback) override {
        packetStorage = feedback.packet_feedbacks.data();
        arrivalStorage = feedback.sendless_arrival_times.data();
        feedbackFieldsPreserved = feedback.feedback_time == Timestamp::Millis(120) &&
            feedback.data_in_flight == DataSize::Bytes(9876) &&
            feedback.packet_feedbacks.size() == 1 &&
            feedback.packet_feedbacks[0].sent_packet.sequence_number == 456 &&
            feedback.sendless_arrival_times.size() == 1 &&
            feedback.sendless_arrival_times[0] == Timestamp::Millis(111);
        if (state) state->ObserveDelay(BandwidthUsage::kBwOverusing, GoogCcTelemetryState::NowMs());
        return Fixture(11);
    }
#define RLINK_TEST_CONTROLLER_CALLBACK(Name, Type, Marker) \
    NetworkControlUpdate Name(Type) override { return Fixture(Marker); }
    RLINK_TEST_CONTROLLER_CALLBACK(OnNetworkAvailability, NetworkAvailability, 1)
    RLINK_TEST_CONTROLLER_CALLBACK(OnNetworkRouteChange, NetworkRouteChange, 2)
    RLINK_TEST_CONTROLLER_CALLBACK(OnRemoteBitrateReport, RemoteBitrateReport, 4)
    RLINK_TEST_CONTROLLER_CALLBACK(OnRoundTripTimeUpdate, RoundTripTimeUpdate, 5)
    RLINK_TEST_CONTROLLER_CALLBACK(OnSentPacket, SentPacket, 6)
    RLINK_TEST_CONTROLLER_CALLBACK(OnReceivedPacket, ReceivedPacket, 7)
    RLINK_TEST_CONTROLLER_CALLBACK(OnStreamsConfig, StreamsConfig, 8)
    RLINK_TEST_CONTROLLER_CALLBACK(OnTargetRateConstraints, TargetRateConstraints, 9)
    RLINK_TEST_CONTROLLER_CALLBACK(OnTransportLossReport, TransportLossReport, 10)
    RLINK_TEST_CONTROLLER_CALLBACK(OnNetworkStateEstimate, NetworkStateEstimate, 12)
#undef RLINK_TEST_CONTROLLER_CALLBACK
    bool SupportsEcnAdaptation() const override { return true; }
};

ScreenRecoveryProbeSample RecoverySample(std::uint64_t now, std::uint64_t budget = 8'000'000)
{
    ScreenRecoveryProbeSample sample;
    sample.nowMs = sample.feedbackAtMs = now;
    sample.routeRevision = 1;
    sample.budgetBps = budget;
    sample.targetFps = 80;
    sample.videoCapBps = 10'000'000;
    sample.enabled = sample.feedbackFresh = sample.healthy = true;
    return sample;
}

bool RecoveryPolicyTests()
{
    bool ok = true;
    ScreenRecoveryProbePolicy policy;
    for (std::uint64_t now = 1000; now <= 4000; now += 100)
        policy.Update(RecoverySample(now));
    ok &= Check(policy.HistoricalBudgetBps() == 8'000'000 && !policy.Active(),
        "RECOVERY_HISTORY_REQUIRES_STABLE_ACKNOWLEDGED_HEALTHY_GCC_BUDGET");
    ScreenRecoveryProbePolicy growingHistory;
    for (std::uint64_t now = 1000; now <= 12'000; now += 100) {
        const auto budget = now <= 4000 ? 8'000'000u : now <= 6000 ? 8'800'000u : 9'600'000u;
        growingHistory.Update(RecoverySample(now, budget));
    }
    ok &= Check(growingHistory.HistoricalBudgetBps() == 9'600'000 &&
        growingHistory.Episodes() == 0,
        "HEALTHY_WINDOWS_REFRESH_GRADUAL_CAPACITY_GROWTH_WITHOUT_PROBING");
    ScreenRecoveryProbePolicy modestDropPolicy;
    for (std::uint64_t now = 1000; now <= 4000; now += 100)
        modestDropPolicy.Update(RecoverySample(now));
    auto modestDrop = RecoverySample(4100, 7'600'000);
    modestDrop.healthy = false;
    modestDrop.congestionEventAtMs = 4100;
    ok &= Check(modestDropPolicy.Update(modestDrop) == ScreenRecoveryHintChange::kNone &&
        !modestDropPolicy.Active(), "FIVE_PERCENT_NETWORK_VARIATION_DOES_NOT_ENABLE_RECOVERY_HINT");
    modestDrop.nowMs = modestDrop.feedbackAtMs = modestDrop.congestionEventAtMs = 4200;
    modestDrop.budgetBps = 7'200'000;
    ok &= Check(modestDropPolicy.Update(modestDrop) == ScreenRecoveryHintChange::kNone,
        "RECOVERY_HINT_BOUNDARY_DOES_NOT_CONFLICT_WITH_NINETY_PERCENT_CLEAR_THRESHOLD");
    modestDrop.nowMs = modestDrop.feedbackAtMs = modestDrop.congestionEventAtMs = 4300;
    modestDrop.budgetBps = 6'560'000;
    ok &= Check(modestDropPolicy.Update(modestDrop) == ScreenRecoveryHintChange::kInstall &&
        modestDropPolicy.HistoricalBudgetBps() == 8'000'000,
        "EIGHTEEN_PERCENT_REAL_GCC_DROP_PROBES_PREVIOUS_HEALTHY_CAPACITY_WITHOUT_IMAGE_REFERENCE");
    auto limited = RecoverySample(4100, 2'000'000);
    limited.healthy = false;
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kNone && !policy.Active(),
        "LOW_FLOW_OR_THEORETICAL_DEMAND_ALONE_CANNOT_ENABLE_RECOVERY_PROBING");
    limited.nowMs = limited.feedbackAtMs = limited.congestionEventAtMs = 4200;
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kInstall && policy.Active() &&
        policy.HistoricalBudgetBps() == 8'000'000 && policy.Episodes() == 1,
        "FRESH_GCC_CONGESTION_INSTALLS_ONLY_REAL_PREVIOUS_CAPACITY_HINT");
    for (unsigned index = 0; index < ScreenRecoveryProbePolicy::kMaximumProbes - 1; ++index) {
        policy.ObserveProbes(1);
        limited.nowMs = limited.feedbackAtMs = 4300 + index * 400;
        policy.Update(limited);
    }
    policy.ObserveProbes(1);
    limited.nowMs = limited.feedbackAtMs = 7400;
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kNone && !policy.Active() &&
        policy.EpisodeProbes() == ScreenRecoveryProbePolicy::kMaximumProbes,
        "RECOVERY_HINT_STOPS_AT_CONFIGURED_NATIVE_CLUSTER_LIMIT");
    limited.congestionEventAtMs = limited.nowMs = limited.feedbackAtMs = 7500;
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kNone,
        "CONTINUED_CONGESTION_CANNOT_REARM_EXHAUSTED_RECOVERY_EPISODE");
    for (std::uint64_t now = 7600; now <= 11'000; now += 100)
        policy.Update(RecoverySample(now, 2'000'000));
    limited.congestionEventAtMs = limited.nowMs = limited.feedbackAtMs = 11'100;
    policy.Update(limited);
    ok &= Check(policy.Active() && policy.HistoricalBudgetBps() == 4'000'000 &&
        policy.ProbeLimit() == 1,
        "LOW_STABLE_WEAK_LINK_USES_CURRENT_BUDGET_SINGLE_PROBE_NOT_OLD_HIGH_HINT");
    policy.ObserveProbes(1);
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kClear && !policy.Active(),
        "SUSTAINED_HINT_CLEARS_AFTER_FIRST_NATIVE_CLUSTER_TO_ALLOW_SUCCESS_CHAIN");
    for (std::uint64_t now = 11'200; now <= 14'000; now += 100)
        policy.Update(RecoverySample(now));
    limited.congestionEventAtMs = limited.nowMs = limited.feedbackAtMs = 14'100;
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kInstall && policy.Episodes() == 3,
        "ONLY_CONFIRMED_HEALTHY_RESTORATION_REARMS_ANOTHER_EPISODE");
    auto recovered = RecoverySample(14'200, 7'200'000);
    ok &= Check(policy.Update(recovered) == ScreenRecoveryHintChange::kClear && !policy.Active(),
        "NINETY_PERCENT_MEASURED_RECOVERY_CLEARS_HINT_IMMEDIATELY");

    const auto prepare = [&policy] {
        policy.Reset();
        for (std::uint64_t now = 1000; now <= 4000; now += 100)
            policy.Update(RecoverySample(now));
        auto sample = RecoverySample(4100, 2'000'000);
        sample.healthy = false;
        sample.congestionEventAtMs = 4100;
        policy.Update(sample);
        return sample;
    };
    limited = prepare();
    const auto shortEpisode = policy.Episodes();
    policy.ObserveProbes(1, 4200);
    limited.nowMs = limited.feedbackAtMs = 4200;
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kClear && !policy.Active(),
        "SHORT_HINT_CLEARS_FIRST_CLUSTER_WITHOUT_ENDING_RECOVERY_EPISODE");
    limited.nowMs = limited.feedbackAtMs = 4250;
    limited.budgetBps = 6'000'000;
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kNone && !policy.Active(),
        "SEVENTY_TO_NINETY_PERCENT_RECOVERY_DOES_NOT_REINSTALL_FINITE_UPPER_DURING_CHAIN");
    policy.ObserveProbes(1, 4250);
    limited.nowMs = limited.feedbackAtMs = 5249;
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kNone && !policy.Active(),
        "EVERY_NATIVE_CONTINUATION_RESTARTS_ONLY_ONE_SECOND_QUIET_TIMER");
    limited.nowMs = limited.feedbackAtMs = 5250;
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kInstall && policy.Active() &&
        policy.Episodes() == shortEpisode && policy.EpisodeProbes() == 2,
        "QUIET_NATIVE_CHAIN_MAY_RETRY_WITHIN_ORIGINAL_WINDOW_AND_ATTEMPT_BUDGET");
    policy.ObserveProbes(1, 5260);
    limited.nowMs = limited.feedbackAtMs = 5260;
    policy.Update(limited);
    limited.nowMs = limited.feedbackAtMs = 12'100;
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kNone && !policy.Active() &&
        policy.EpisodeProbes() == 3,
        "CONSUMED_SHORT_INVITATIONS_DO_NOT_EXTEND_OR_RESTART_EIGHT_SECOND_DEADLINE");
    limited.nowMs = limited.feedbackAtMs = 12'200;
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kNone,
        "EXPIRED_SHORT_EPISODE_CANNOT_RETRY_HISTORICAL_UPPER_AFTER_QUIET_TIME");
    limited = prepare();
    limited.nowMs = limited.feedbackAtMs = 12'100;
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kClear,
        "FAILED_PROBE_ATTEMPTS_CANNOT_EXTEND_EIGHT_SECOND_RECOVERY_WINDOW");
    limited = prepare();
    policy.ObserveProbes(2);
    policy.ObserveProbes(ScreenRecoveryProbePolicy::kMaximumProbes - 1);
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kClear &&
        policy.EpisodeProbes() == ScreenRecoveryProbePolicy::kMaximumProbes + 1,
        "MULTIPLE_NATIVE_CLUSTERS_ARE_COUNTED_TRUTHFULLY_BEFORE_HINT_IS_CLEARED");
    limited = prepare();
    limited.enabled = false;
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kClear &&
        policy.HistoricalBudgetBps() == 0, "STOPPED_SCREEN_CLEARS_HINT_AND_HISTORY");
    limited = prepare();
    limited.routeRevision = 2;
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kClear &&
        policy.HistoricalBudgetBps() == 0, "ROUTE_CHANGE_CANNOT_REUSE_PREVIOUS_LINK_CAPACITY");
    limited = prepare();
    limited.targetFps = 120;
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kClear &&
        policy.HistoricalBudgetBps() == 0, "USER_TARGET_CHANGE_DISCARDS_RECOVERY_HINT");
    limited = prepare();
    policy.ObserveProbes(1);
    limited.nowMs = 5200;
    const auto episode = policy.Episodes();
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kClear && !policy.Active() &&
        policy.HistoricalBudgetBps() == 8'000'000 && policy.EpisodeProbes() == 1,
        "ACK_GAP_PAUSES_HINT_AND_PRESERVES_MEASURED_HISTORY_AND_ATTEMPTS");
    limited.nowMs = limited.feedbackAtMs = 5650;
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kInstall && policy.Active() &&
        policy.Episodes() == episode && policy.EpisodeProbes() == 1,
        "FRESH_ACK_RESUMES_SAME_RECOVERY_EPISODE_AFTER_ONE_POINT_FIVE_SECOND_GAP");
    limited.nowMs = 6800;
    policy.Update(limited);
    limited.nowMs = limited.feedbackAtMs = 12'150;
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kNone && !policy.Active() &&
        policy.Episodes() == episode && policy.EpisodeProbes() == 1,
        "ACK_PAUSE_CANNOT_EXTEND_OR_RESTART_ORIGINAL_EIGHT_SECOND_WINDOW");
    policy.Reset();
    for (std::uint64_t now = 1000; now <= 4000; now += 100)
        policy.Update(RecoverySample(now));
    auto gap = RecoverySample(5500);
    gap.feedbackAtMs = 4000;
    policy.Update(gap);
    limited = RecoverySample(5600, 2'000'000);
    limited.healthy = false;
    limited.congestionEventAtMs = 5600;
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kInstall &&
        policy.HistoricalBudgetBps() == 8'000'000,
        "ACK_GAP_BEFORE_CONGESTION_RETAINS_ARMED_REAL_CAPACITY_HISTORY");
    policy.Reset();
    for (std::uint64_t now = 1000; now <= 4000; now += 100)
        policy.Update(RecoverySample(now));
    limited = RecoverySample(34'100, 2'000'000);
    limited.healthy = false;
    limited.congestionEventAtMs = limited.nowMs;
    ok &= Check(policy.Update(limited) == ScreenRecoveryHintChange::kNone &&
        policy.HistoricalBudgetBps() == 0, "EXPIRED_HEALTHY_HISTORY_CANNOT_REQUEST_PROBING");
    ScreenRecoveryProbePolicy independent;
    ok &= Check(independent.HistoricalBudgetBps() == 0 && !independent.Active(),
        "RECOVERY_POLICY_HISTORY_AND_ATTEMPTS_ARE_PER_CONNECTION");
    return ok;
}

bool SustainedRecoveryTests()
{
    bool ok = true;
    ScreenRecoveryProbePolicy policy;
    for (std::uint64_t now = 1000; now <= 4000; now += 100)
        policy.Update(RecoverySample(now));
    auto weak = RecoverySample(4100, 1'600'000);
    weak.healthy = false;
    weak.congestionEventAtMs = 4100;
    policy.Update(weak);
    policy.ObserveProbes(ScreenRecoveryProbePolicy::kMaximumProbes, 4200);
    weak.nowMs = weak.feedbackAtMs = 4200;
    policy.Update(weak);
    unsigned maintenanceClusters = 0;
    bool bounded = true, expiredHighNotUsed = true;
    std::uint64_t lastMaintenanceMs = 0;
    for (std::uint64_t now = 4300; now <= 65'000; now += 100) {
        auto stable = RecoverySample(now, 1'600'000);
        const auto change = policy.Update(stable);
        if (change == ScreenRecoveryHintChange::kInstall) {
            bounded = bounded && policy.HistoricalBudgetBps() == 3'200'000 &&
                policy.ProbeLimit() == 1 &&
                (lastMaintenanceMs == 0 || now - lastMaintenanceMs >= 3000);
            if (now > 34'000) expiredHighNotUsed = expiredHighNotUsed &&
                policy.HistoricalBudgetBps() == 3'200'000;
            ++maintenanceClusters;
            lastMaintenanceMs = now;
            policy.ObserveProbes(1, now);
            bounded = bounded && policy.Update(stable) == ScreenRecoveryHintChange::kClear;
        }
    }
    ok &= Check(bounded && maintenanceClusters >= 10 && maintenanceClusters <= 21,
        "LONG_WEAK_LINK_ONLY_INVITES_ONE_CURRENT_BUDGET_PROBE_PER_THREE_SECONDS");
    ok &= Check(expiredHighNotUsed,
        "SIXTY_SECOND_LIMIT_RETAINS_RECOVERY_INTENT_WITHOUT_REUSING_EXPIRED_HIGH_CAPACITY");
    auto recovered = RecoverySample(65'100);
    policy.Update(recovered);
    const auto episodes = policy.Episodes();
    for (std::uint64_t now = 65'200; now <= 80'000; now += 100)
        policy.Update(RecoverySample(now));
    ok &= Check(!policy.Active() && policy.Episodes() == episodes,
        "HEALTHY_RESTORATION_STOPS_SUSTAINED_MAINTENANCE_PROBES");
    ScreenRecoveryProbePolicy normal;
    for (std::uint64_t now = 1000; now <= 65'000; now += 100)
        normal.Update(RecoverySample(now, 1'600'000));
    ok &= Check(normal.Episodes() == 0 && !normal.Active(),
        "LOW_NORMAL_FLOW_WITHOUT_REAL_CONGESTION_NEVER_STARTS_MAINTENANCE");
    auto limited = RecoverySample(80'100, 1'600'000);
    limited.healthy = false;
    limited.congestionEventAtMs = 80'100;
    policy.Update(limited);
    policy.ObserveProbes(ScreenRecoveryProbePolicy::kMaximumProbes, 80'200);
    limited.nowMs = limited.feedbackAtMs = 80'200;
    policy.Update(limited);
    for (std::uint64_t now = 80'300; now <= 85'000; now += 100) {
        limited.nowMs = limited.feedbackAtMs = now;
        policy.Update(limited);
    }
    ok &= Check(!policy.Active(), "UNHEALTHY_WEAK_LINK_CANNOT_START_MAINTENANCE_PROBE");
    for (std::uint64_t now = 85'100; now <= 87'000; now += 100)
        policy.Update(RecoverySample(now, 1'600'000));
    policy.ObserveProbes(1, 87'000); // Native ALR/continuation takes precedence.
    for (std::uint64_t now = 87'100; now <= 89'900; now += 100)
        policy.Update(RecoverySample(now, 1'600'000));
    ok &= Check(!policy.Active(), "ACTIVE_NATIVE_PROBE_CHAIN_DEFERS_MAINTENANCE_HINT");
    auto stable = RecoverySample(90'000, 1'600'000);
    ok &= Check(policy.Update(stable) == ScreenRecoveryHintChange::kInstall,
        "MAINTENANCE_RESUMES_ONLY_AFTER_NATIVE_PROBE_COOLDOWN");
    stable.nowMs = 91'100;
    ok &= Check(policy.Update(stable) == ScreenRecoveryHintChange::kClear,
        "MAINTENANCE_HINT_PAUSES_WITHOUT_REAL_RECEIVE_ACK");
    stable.nowMs = stable.feedbackAtMs = 91'200;
    stable.enabled = false;
    policy.Update(stable);
    stable.enabled = true;
    for (std::uint64_t now = 91'300; now <= 95'000; now += 100) {
        stable.nowMs = stable.feedbackAtMs = now;
        policy.Update(stable);
    }
    ok &= Check(!policy.Active(), "STOPPED_SCREEN_DISCARDS_LONG_WEAK_NETWORK_RECOVERY_INTENT");
    return ok;
}

class RecoveryController final : public NetworkControllerInterface {
public:
    std::shared_ptr<GoogCcTelemetryState> state;
    std::uint64_t budget = 8'000'000;
    bool congested = false;
    std::optional<NetworkStateEstimate> hint;
    unsigned hintCalls = 0;
    unsigned nativeContinuationProbes = 0;
    NetworkControlUpdate OnProcessInterval(ProcessInterval) override {
        NetworkControlUpdate output;
        const unsigned count = nativeContinuationProbes != 0
            ? std::exchange(nativeContinuationProbes, 0u)
            : hint && hint->link_capacity_upper.IsFinite() ? 1u : 0u;
        for (unsigned index = 0; index < count; ++index) {
            ProbeClusterConfig probe;
            probe.id = 99;
            probe.target_data_rate = DataRate::BitsPerSec(1'000'000);
            probe.target_duration = TimeDelta::Millis(15);
            probe.target_probe_count = 5;
            output.probe_cluster_configs.push_back(probe);
        }
        return output;
    }
    NetworkControlUpdate OnTransportPacketsFeedback(TransportPacketsFeedback feedback) override {
        state->ObserveDelay(congested ? BandwidthUsage::kBwOverusing : BandwidthUsage::kBwNormal,
                            GoogCcTelemetryState::NowMs());
        TargetTransferRate target;
        target.at_time = feedback.feedback_time;
        target.target_rate = DataRate::BitsPerSec(budget);
        target.cwnd_reduce_ratio = congested ? 0.2 : 0;
        target.network_estimate.loss_rate_ratio = 0;
        target.network_estimate.round_trip_time = TimeDelta::Millis(8);
        NetworkControlUpdate output;
        output.target_rate = target;
        return output;
    }
    NetworkControlUpdate OnNetworkStateEstimate(NetworkStateEstimate input) override {
        hint = input;
        ++hintCalls;
        return {};
    }
#define RLINK_EMPTY_RECOVERY_CALLBACK(Name, Type) \
    NetworkControlUpdate Name(Type) override { return {}; }
    RLINK_EMPTY_RECOVERY_CALLBACK(OnNetworkAvailability, NetworkAvailability)
    RLINK_EMPTY_RECOVERY_CALLBACK(OnNetworkRouteChange, NetworkRouteChange)
    RLINK_EMPTY_RECOVERY_CALLBACK(OnRemoteBitrateReport, RemoteBitrateReport)
    RLINK_EMPTY_RECOVERY_CALLBACK(OnRoundTripTimeUpdate, RoundTripTimeUpdate)
    RLINK_EMPTY_RECOVERY_CALLBACK(OnSentPacket, SentPacket)
    RLINK_EMPTY_RECOVERY_CALLBACK(OnReceivedPacket, ReceivedPacket)
    RLINK_EMPTY_RECOVERY_CALLBACK(OnStreamsConfig, StreamsConfig)
    RLINK_EMPTY_RECOVERY_CALLBACK(OnTargetRateConstraints, TargetRateConstraints)
    RLINK_EMPTY_RECOVERY_CALLBACK(OnTransportLossReport, TransportLossReport)
#undef RLINK_EMPTY_RECOVERY_CALLBACK
};

bool RecoveryWrapperTests()
{
    bool ok = true;
    auto state = std::make_shared<GoogCcTelemetryState>();
    state->SetScreenQualityTarget(80, 10'000'000);
    state->SetScreenRecoveryProbeEnabled(true);
    auto delegate = std::make_unique<RecoveryController>();
    auto* raw = delegate.get();
    raw->state = state;
    auto wrapper = CreateObservingNetworkController(std::move(delegate), state);
    const auto receivedFeedback = [] {
        TransportPacketsFeedback feedback;
        feedback.feedback_time = Timestamp::Millis(GoogCcTelemetryState::NowMs());
        PacketResult packet;
        packet.sent_packet.send_time = feedback.feedback_time - TimeDelta::Millis(8);
        packet.receive_time = feedback.feedback_time;
        feedback.packet_feedbacks.push_back(packet);
        return feedback;
    };
    auto referenceState = std::make_shared<GoogCcTelemetryState>();
    referenceState->SetScreenQualityTarget(80, 10'000'000);
    referenceState->SetScreenRecoveryProbeEnabled(true);
    auto referenceDelegate = std::make_unique<RecoveryController>();
    auto* reference = referenceDelegate.get();
    reference->state = referenceState;
    auto referenceWrapper = CreateObservingNetworkController(
        std::move(referenceDelegate), referenceState, false);
    const auto until = GoogCcTelemetryState::NowMs() + 2750;
    do {
        wrapper->OnTransportPacketsFeedback(receivedFeedback());
        referenceWrapper->OnTransportPacketsFeedback(receivedFeedback());
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    } while (GoogCcTelemetryState::NowMs() < until);
    ok &= Check(raw->hintCalls == 0,
        "HEALTHY_SCREEN_FLOW_NEVER_INJECTS_NETWORK_STATE_OR_EXTRA_PROBES");
    reference->budget = 2'000'000;
    reference->congested = true;
    const auto referenceTarget = referenceWrapper->OnTransportPacketsFeedback(receivedFeedback());
    reference->nativeContinuationProbes = 1;
    ProcessInterval referenceProcess;
    referenceProcess.at_time = Timestamp::Millis(GoogCcTelemetryState::NowMs());
    const auto referenceProbe = referenceWrapper->OnProcessInterval(referenceProcess);
    ok &= Check(reference->hintCalls == 0 && referenceTarget.target_rate &&
        referenceTarget.target_rate->target_rate.bps() == 2'000'000 &&
        referenceProbe.probe_cluster_configs.size() == 1 &&
        referenceProbe.probe_cluster_configs[0].id == 99 &&
        referenceState->Snapshot(GoogCcTelemetryState::NowMs()).feedbackFresh &&
        referenceState->Snapshot(GoogCcTelemetryState::NowMs()).recoveryProbeEpisodes == 0,
        "REFERENCE_CONTROLLER_ONLY_OBSERVES_WITHOUT_HINTS_OR_MODIFIED_NATIVE_OUTPUT");
    raw->budget = 2'000'000;
    raw->congested = true;
    const auto congested = wrapper->OnTransportPacketsFeedback(receivedFeedback());
    ok &= Check(congested.target_rate && congested.target_rate->target_rate.bps() == 2'000'000 &&
        congested.target_rate->cwnd_reduce_ratio == 0.2 && raw->hintCalls == 1 && raw->hint &&
        raw->hint->link_capacity_lower.IsZero() && raw->hint->link_capacity_upper.bps() == 8'000'000,
        "RECOVERY_HINT_IS_HISTORICAL_UPPER_WITH_ZERO_LOWER_AND_UNCHANGED_REAL_TARGET");
    ProcessInterval firstProcess;
    firstProcess.at_time = Timestamp::Millis(GoogCcTelemetryState::NowMs());
    const auto firstProbe = wrapper->OnProcessInterval(firstProcess);
    ok &= Check(firstProbe.probe_cluster_configs.size() == 1 && !firstProbe.target_rate &&
        raw->hintCalls == 2 && raw->hint->link_capacity_upper.IsInfinite(),
        "NATIVE_RECOVERY_PROBE_ADDS_NO_FABRICATED_TARGET");
    std::this_thread::sleep_for(std::chrono::milliseconds(1550));
    TransportLossReport lossOnly;
    lossOnly.packets_received_delta = 1;
    wrapper->OnTransportLossReport(lossOnly);
    ProcessInterval pauseProcess;
    pauseProcess.at_time = Timestamp::Millis(GoogCcTelemetryState::NowMs());
    const auto paused = wrapper->OnProcessInterval(pauseProcess);
    const auto pausedState = state->Snapshot(GoogCcTelemetryState::NowMs());
    ok &= Check(!paused.has_updates() && raw->hintCalls == 2 &&
        raw->hint->link_capacity_upper.IsInfinite() && !pausedState.recoveryProbeActive &&
        pausedState.recoveryProbeHistoricalBudgetBps == 8'000'000 &&
        pausedState.recoveryProbeAttempts == 1 && pausedState.recoveryProbeEpisodes == 1,
        "LOSS_ONLY_FEEDBACK_CANNOT_CONTINUE_HINT_OR_DISCARD_HISTORY_DURING_ACK_GAP");
    wrapper->OnTransportPacketsFeedback(receivedFeedback());
    const auto resumedState = state->Snapshot(GoogCcTelemetryState::NowMs());
    ok &= Check(raw->hintCalls == 3 && resumedState.recoveryProbeActive &&
        resumedState.recoveryProbeAttempts == 1 && resumedState.recoveryProbeEpisodes == 1,
        "REAL_RECEIVE_ACK_RESUMES_ORIGINAL_EPISODE_WITHOUT_NEW_PRIOR_OR_WINDOW");
    for (unsigned index = 0; index < ScreenRecoveryProbePolicy::kMaximumProbes - 1; ++index) {
        if (index != 0) raw->nativeContinuationProbes = 1;
        ProcessInterval process;
        process.at_time = Timestamp::Millis(GoogCcTelemetryState::NowMs());
        const auto update = wrapper->OnProcessInterval(process);
        ok &= !update.target_rate && !update.pacer_config && !update.congestion_window &&
            update.probe_cluster_configs.size() == 1 && update.probe_cluster_configs[0].id == 99 &&
            update.probe_cluster_configs[0].target_data_rate.bps() == 1'000'000;
    }
    ProcessInterval process;
    process.at_time = Timestamp::Millis(GoogCcTelemetryState::NowMs());
    const auto exhausted = wrapper->OnProcessInterval(process);
    ok &= Check(raw->hintCalls == 4 && raw->hint->link_capacity_upper.IsInfinite() &&
        raw->hint->link_capacity_lower.IsZero() && !exhausted.has_updates(),
        "WRAPPER_FORWARDS_NATIVE_PROBES_UNCHANGED_AND_CLEARS_AT_LIMIT_WITHOUT_TARGET_OR_PACING_RESET");
    wrapper->OnTransportPacketsFeedback(receivedFeedback());
    ok &= Check(raw->hintCalls == 4, "FAILED_RECOVERY_HINT_DOES_NOT_REARM_IN_CONTINUED_WEAK_NETWORK");
    auto cameraState = std::make_shared<GoogCcTelemetryState>();
    auto cameraDelegate = std::make_unique<RecoveryController>();
    auto* camera = cameraDelegate.get();
    camera->state = cameraState;
    auto cameraWrapper = CreateObservingNetworkController(std::move(cameraDelegate), cameraState);
    cameraWrapper->OnTransportPacketsFeedback(receivedFeedback());
    camera->budget = 2'000'000;
    camera->congested = true;
    cameraWrapper->OnTransportPacketsFeedback(receivedFeedback());
    ok &= Check(camera->hintCalls == 0 && !cameraWrapper->OnProcessInterval(process).has_updates(),
        "CAMERA_AND_UNRELATED_CONNECTION_HAVE_NO_SCREEN_RECOVERY_HINT");
    return ok;
}

// Exercise the linked, unmodified native controller as well as the policy and
// wrapper spies above. Advancing timestamps requires no sleeps or live traffic.
class NativeProbeFixture final {
public:
    explicit NativeProbeFixture(const FieldTrialsView& trials,
                                DataRate cap = DataRate::BitsPerSec(8'000'000))
        : controller(&trials, &eventLog)
    {
        NetworkAvailability available;
        available.at_time = Timestamp::Millis(1000);
        available.network_available = true;
        initialized = controller.OnNetworkAvailability(available).empty();
        const auto initial = controller.SetBitrates(DataRate::KilobitsPerSec(100),
            DataRate::BitsPerSec(1'000'000), cap, available.at_time);
        initialized &= !initial.empty();
        // Let the original connection-start probes expire before testing the
        // recovery schedule, so they cannot masquerade as recovery invitations.
        initialized &= controller.SetEstimatedBitrate(DataRate::BitsPerSec(1'000'000),
            BandwidthLimitedCause::kDelayBasedLimited, available.at_time).empty();
        initialized &= controller.Process(Timestamp::Millis(2101)).empty();
    }

    void Hint(DataRate upper)
    {
        NetworkStateEstimate estimate;
        estimate.update_time = Timestamp::Millis(2101);
        estimate.link_capacity_lower = DataRate::Zero();
        estimate.link_capacity_upper = upper;
        controller.SetNetworkStateEstimate(estimate);
    }

    RtcEventLogNull eventLog;
    ProbeController controller;
    bool initialized = false;
};

bool NativeRecoveryProbeTests(const Environment& environment)
{
    bool ok = true;
    const auto& trials = environment.field_trials();
    NativeProbeFixture dormant(trials);
    bool noExtraProbes = dormant.initialized;
    for (std::int64_t now = 2200; now <= 12'200; now += 100)
        noExtraProbes &= dormant.controller.Process(Timestamp::Millis(now)).empty();
    dormant.Hint(DataRate::PlusInfinity());
    for (std::int64_t now = 12'300; now <= 32'300; now += 100)
        noExtraProbes &= dormant.controller.Process(Timestamp::Millis(now)).empty();
    ok &= Check(noExtraProbes,
        "NATIVE_NO_HINT_AND_INFINITE_CLEAR_HAVE_NO_ONE_SECOND_RESIDENT_PROBES");

    NativeProbeFixture retry(trials);
    retry.Hint(DataRate::BitsPerSec(4'000'000));
    auto probe = retry.controller.Process(Timestamp::Millis(2101));
    bool shortRetry = retry.initialized && probe.size() == 1 &&
        probe[0].target_data_rate == DataRate::BitsPerSec(2'000'000) &&
        probe[0].target_duration == TimeDelta::Millis(15) &&
        probe[0].min_probe_delta == TimeDelta::Millis(2) &&
        probe[0].target_probe_count == 5;
    shortRetry &= retry.controller.Process(Timestamp::Millis(3100)).empty();
    probe = retry.controller.Process(Timestamp::Millis(3102));
    shortRetry &= probe.size() == 1 &&
        probe[0].target_data_rate == DataRate::BitsPerSec(2'000'000);
    ok &= Check(shortRetry,
        "NATIVE_HINT_USES_SHORT_TWO_TIMES_PROBE_AND_RETRIES_AFTER_RESULT_TIMEOUT");

    for (const auto cause : {BandwidthLimitedCause::kLossLimitedBwe,
                            BandwidthLimitedCause::kRttBasedBackOffHighRtt,
                            BandwidthLimitedCause::kDelayBasedLimitedDelayIncreased}) {
        NativeProbeFixture guarded(trials);
        guarded.Hint(DataRate::BitsPerSec(4'000'000));
        bool blocked = guarded.initialized && guarded.controller.SetEstimatedBitrate(
            DataRate::BitsPerSec(1'000'000), cause, Timestamp::Millis(2101)).empty();
        for (std::int64_t now = 2101; now <= 6101; now += 100)
            blocked &= guarded.controller.Process(Timestamp::Millis(now)).empty();
        const char* label = cause == BandwidthLimitedCause::kLossLimitedBwe
            ? "NATIVE_RECOVERY_HINT_PRESERVES_LOSS_LIMITED_PROBE_GUARD"
            : cause == BandwidthLimitedCause::kRttBasedBackOffHighRtt
                ? "NATIVE_RECOVERY_HINT_PRESERVES_HIGH_RTT_PROBE_GUARD"
                : "NATIVE_RECOVERY_HINT_PRESERVES_INCREASING_DELAY_PROBE_GUARD";
        ok &= Check(blocked, label);
    }

    NativeProbeFixture increasingLoss(trials);
    increasingLoss.Hint(DataRate::BitsPerSec(4'000'000));
    bool limitedLossScale = increasingLoss.initialized &&
        increasingLoss.controller.SetEstimatedBitrate(DataRate::BitsPerSec(1'000'000),
            BandwidthLimitedCause::kLossLimitedBweIncreasing,
            Timestamp::Millis(2101)).empty();
    probe = increasingLoss.controller.Process(Timestamp::Millis(2101));
    limitedLossScale &= probe.size() == 1 &&
        probe[0].target_data_rate == DataRate::KilobitsPerSec(1500);
    ok &= Check(limitedLossScale,
        "NATIVE_RECOVERY_HINT_PRESERVES_LOSS_INCREASING_ONE_POINT_FIVE_SCALE");

    NativeProbeFixture capped(trials, DataRate::BitsPerSec(3'000'000));
    capped.Hint(DataRate::BitsPerSec(4'000'000));
    bool respectsCap = capped.initialized && capped.controller.SetEstimatedBitrate(
        DataRate::BitsPerSec(2'000'000), BandwidthLimitedCause::kDelayBasedLimited,
        Timestamp::Millis(2101)).empty();
    probe = capped.controller.Process(Timestamp::Millis(2101));
    respectsCap &= probe.size() == 1 &&
        probe[0].target_data_rate == DataRate::BitsPerSec(3'000'000);
    ok &= Check(respectsCap, "NATIVE_RECOVERY_PROBE_NEVER_EXCEEDS_USER_CONNECTION_CAP");

    NativeProbeFixture continuation(trials);
    continuation.Hint(DataRate::BitsPerSec(2'000'000));
    probe = continuation.controller.Process(Timestamp::Millis(2101));
    bool nativeContinuation = continuation.initialized && probe.size() == 1 &&
        probe[0].target_data_rate == DataRate::BitsPerSec(2'000'000);
    continuation.Hint(DataRate::PlusInfinity());
    // Clearing an invitation does not cancel an already successful native
    // exponential chain or falsify its feedback. Its original PC cap ends it.
    probe = continuation.controller.SetEstimatedBitrate(DataRate::KilobitsPerSec(1800),
        BandwidthLimitedCause::kDelayBasedLimited, Timestamp::Millis(2150));
    nativeContinuation &= probe.size() == 1 &&
        probe[0].target_data_rate == DataRate::KilobitsPerSec(3600);
    probe = continuation.controller.SetEstimatedBitrate(DataRate::BitsPerSec(3'000'000),
        BandwidthLimitedCause::kDelayBasedLimited, Timestamp::Millis(2200));
    nativeContinuation &= probe.size() == 1 &&
        probe[0].target_data_rate == DataRate::BitsPerSec(6'000'000);
    probe = continuation.controller.SetEstimatedBitrate(DataRate::BitsPerSec(5'000'000),
        BandwidthLimitedCause::kDelayBasedLimited, Timestamp::Millis(2250));
    nativeContinuation &= probe.size() == 1 &&
        probe[0].target_data_rate == DataRate::BitsPerSec(8'000'000);
    nativeContinuation &= continuation.controller.SetEstimatedBitrate(
        DataRate::BitsPerSec(8'000'000), BandwidthLimitedCause::kDelayBasedLimited,
        Timestamp::Millis(2300)).empty();
    for (std::int64_t now = 3301; now <= 23'301; now += 100)
        nativeContinuation &= continuation.controller.Process(Timestamp::Millis(now)).empty();
    ok &= Check(nativeContinuation,
        "CLEARED_CURRENT_BUDGET_HINT_PRESERVES_SUCCESSFUL_NATIVE_CHAIN_WHICH_STOPS_AT_USER_CAP");

    // The short historical upper has the same native side effect as a small
    // sustained upper: after a successful result >70% of the finite upper,
    // ProbeController refuses immediate continuation. Use the linked native
    // implementation to verify both the old blockage and the policy fix.
    NativeProbeFixture retainedHistory(trials, DataRate::BitsPerSec(10'000'000));
    retainedHistory.Hint(DataRate::BitsPerSec(8'000'000));
    bool historyBlocked = retainedHistory.initialized &&
        retainedHistory.controller.SetEstimatedBitrate(DataRate::BitsPerSec(3'000'000),
            BandwidthLimitedCause::kDelayBasedLimited, Timestamp::Millis(2101)).empty();
    probe = retainedHistory.controller.Process(Timestamp::Millis(2101));
    historyBlocked &= probe.size() == 1 &&
        probe[0].target_data_rate == DataRate::BitsPerSec(6'000'000);
    historyBlocked &= retainedHistory.controller.SetEstimatedBitrate(
        DataRate::BitsPerSec(6'000'000), BandwidthLimitedCause::kDelayBasedLimited,
        Timestamp::Millis(2150)).empty();
    ok &= Check(historyBlocked,
        "FINITE_HISTORICAL_UPPER_REPRODUCES_NATIVE_SEVENTY_PERCENT_CONTINUATION_BLOCK");

    ScreenRecoveryProbePolicy historicalPolicy;
    for (std::uint64_t now = 1000; now <= 4000; now += 100)
        historicalPolicy.Update(RecoverySample(now));
    auto historySample = RecoverySample(4100, 3'000'000);
    historySample.healthy = false;
    historySample.congestionEventAtMs = 4100;
    NativeProbeFixture clearedHistory(trials, DataRate::BitsPerSec(10'000'000));
    bool historyContinuation = clearedHistory.initialized &&
        historicalPolicy.Update(historySample) == ScreenRecoveryHintChange::kInstall;
    clearedHistory.Hint(DataRate::BitsPerSec(historicalPolicy.HistoricalBudgetBps()));
    clearedHistory.controller.SetEstimatedBitrate(DataRate::BitsPerSec(3'000'000),
        BandwidthLimitedCause::kDelayBasedLimited, Timestamp::Millis(2101));
    probe = clearedHistory.controller.Process(Timestamp::Millis(2101));
    historyContinuation &= probe.size() == 1;
    historicalPolicy.ObserveProbes(static_cast<std::uint32_t>(probe.size()), 4200);
    historySample.nowMs = historySample.feedbackAtMs = 4200;
    const auto consumed = historicalPolicy.Update(historySample);
    historyContinuation &= consumed == ScreenRecoveryHintChange::kClear;
    if (consumed == ScreenRecoveryHintChange::kClear)
        clearedHistory.Hint(DataRate::PlusInfinity());
    probe = clearedHistory.controller.SetEstimatedBitrate(DataRate::BitsPerSec(6'000'000),
        BandwidthLimitedCause::kDelayBasedLimited, Timestamp::Millis(2150));
    historyContinuation &= probe.size() == 1 &&
        probe[0].target_data_rate == DataRate::BitsPerSec(10'000'000);
    historicalPolicy.ObserveProbes(static_cast<std::uint32_t>(probe.size()), 4250);
    historySample.nowMs = historySample.feedbackAtMs = 4250;
    historySample.budgetBps = 6'000'000; // 75%, still below the 90% recovery goal.
    historyContinuation &= historicalPolicy.Update(historySample) == ScreenRecoveryHintChange::kNone &&
        !historicalPolicy.Active() && historicalPolicy.Episodes() == 1 &&
        historicalPolicy.EpisodeProbes() == 2;
    historyContinuation &= clearedHistory.controller.SetEstimatedBitrate(
        DataRate::BitsPerSec(10'000'000), BandwidthLimitedCause::kDelayBasedLimited,
        Timestamp::Millis(2200)).empty();
    ok &= Check(historyContinuation,
        "CONSUMED_SHORT_HINT_RESTORES_IMMEDIATE_NATIVE_CHAIN_ABOVE_SEVENTY_PERCENT_TO_USER_CAP");

    NativeProbeFixture failed(trials);
    failed.Hint(DataRate::BitsPerSec(4'000'000));
    probe = failed.controller.Process(Timestamp::Millis(2101));
    bool noInfiniteFailures = failed.initialized && probe.size() == 1;
    failed.Hint(DataRate::PlusInfinity());
    // Without successful probe feedback, clearing the bounded hint and the
    // native result timeout stop retries even when fresh weak-link ACKs return.
    noInfiniteFailures &= failed.controller.Process(Timestamp::Millis(3202)).empty();
    noInfiniteFailures &= failed.controller.SetEstimatedBitrate(
        DataRate::KilobitsPerSec(1800), BandwidthLimitedCause::kDelayBasedLimited,
        Timestamp::Millis(3300)).empty();
    for (std::int64_t now = 3400; now <= 123'400; now += 100)
        noInfiniteFailures &= failed.controller.Process(Timestamp::Millis(now)).empty();
    ok &= Check(noInfiniteFailures,
        "CLEARED_FAILED_HINT_AND_NATIVE_TIMEOUT_CANNOT_START_INFINITE_RETRIES");
    return ok;
}
}
int main() {
    auto base = CreateEnvironment(std::make_unique<FixtureTrials>());
    const auto enabled = CreateGoogCcTelemetryEnvironment(base);
    const auto copy = enabled.field_trials().CreateCopy();
    bool ok = Check(enabled.field_trials().IsEnabled("WebRTC-Bwe-InjectedCongestionController") &&
        base.field_trials().IsDisabled("WebRTC-Bwe-InjectedCongestionController") &&
        enabled.field_trials().Lookup("WebRTC-PcFactoryDefaultBitrates") ==
            base.field_trials().Lookup("WebRTC-PcFactoryDefaultBitrates") &&
        &enabled.clock() == &base.clock() &&
        &enabled.task_queue_factory() == &base.task_queue_factory() &&
        &enabled.event_log() == &base.event_log() &&
        copy->IsEnabled("WebRTC-Bwe-InjectedCongestionController") &&
        copy->Lookup("WebRTC-PcFactoryDefaultBitrates") ==
            base.field_trials().Lookup("WebRTC-PcFactoryDefaultBitrates"),
        "INJECTED_CONTROLLER_GATE_ENABLED_WITHOUT_REPLACING_OTHER_TRIALS_OR_UTILITIES");
    const auto referenceEnv = CreateGoogCcTelemetryEnvironment(base, false);
    const auto referenceCopy = referenceEnv.field_trials().CreateCopy();
    ok &= Check(referenceEnv.field_trials().IsEnabled("WebRTC-Bwe-InjectedCongestionController") &&
        referenceEnv.field_trials().Lookup("WebRTC-Bwe-ProbingConfiguration") ==
            base.field_trials().Lookup("WebRTC-Bwe-ProbingConfiguration") &&
        referenceCopy->Lookup("WebRTC-Bwe-ProbingConfiguration") ==
            base.field_trials().Lookup("WebRTC-Bwe-ProbingConfiguration"),
        "REFERENCE_ENVIRONMENT_PRESERVES_ORIGINAL_PROBING_CONFIGURATION_INCLUDING_COPY");
    const ProbeControllerConfig probing(&enabled.field_trials());
    ok &= Check(probing.probe_if_estimate_lower_than_network_state_estimate_ratio.Get() == 0.95 &&
        probing.estimate_lower_than_network_state_estimate_probing_interval.Get() == TimeDelta::Seconds(1) &&
        probing.network_state_estimate_probing_interval.Get() == TimeDelta::Seconds(1) &&
        probing.network_state_probe_duration.Get() == TimeDelta::Millis(15) &&
        probing.network_state_min_probe_delta.Get() == TimeDelta::Millis(2) &&
        probing.network_state_probe_scale.Get() == 2 &&
        !enabled.field_trials().IsEnabled("WebRTC-BweRapidRecoveryExperiment"),
        "BOUNDED_HINT_CONFIGURES_SHORT_NATIVE_PROBES_WITHOUT_GLOBAL_ALR_OR_EARLY_RECOVERY_PROBE");
    ok &= RecoveryPolicyTests();
    ok &= SustainedRecoveryTests();
    ok &= RecoveryWrapperTests();
    ok &= NativeRecoveryProbeTests(enabled);
    auto state = std::make_shared<GoogCcTelemetryState>();
    ok &= Check(!state->Snapshot(1000).controllerObserved &&
        state->Snapshot(1000).delayState == "unknown", "NO_GCC_EVENT_IS_UNKNOWN");
    state->ObserveDelay(BandwidthUsage::kBwOverusing, 1000);
    state->ObserveUpdate(Fixture(0), 1000);
    state->ConfirmFeedback(1000, true);
    const auto initial = state->Snapshot(1001);
    ok &= Check(initial.controllerObserved && initial.feedbackFresh && initial.delayState == "overuse" &&
        initial.targetRateBps == 10'000'000 && initial.effectiveTargetRateBps == 7'500'000 &&
        initial.roundTripTimeMs == 75 && std::abs(initial.lossPercent - 6) < 0.0001 &&
        initial.applicationLimited, "RAW_GCC_OUTPUT_AND_PUSHBACK_BUDGET");
    ok &= Check(!state->Snapshot(4001).feedbackFresh, "OLD_ACKS_CANNOT_STAY_FRESH");
    state->ObserveDelay(BandwidthUsage::kBwNormal, 4002);
    auto settled = Fixture(1);
    settled.target_rate->cwnd_reduce_ratio = 0;
    state->ObserveUpdate(settled, 4002);
    const auto transient = state->Snapshot(4003);
    ok &= Check(transient.delayState == "normal" && transient.congestionWindowReduction == 0 &&
        transient.lastDelayOveruseAtMs == 1000 && transient.lastCwndPushbackAtMs == 1000,
        "REAL_CONGESTION_EVENTS_SURVIVE_LATER_NORMAL_OUTPUT");
    state->ConfirmFeedback(5000, false);
    ok &= Check(state->Snapshot(5000).delayUpdatedAtMs == 4002,
        "LOSS_REPORT_CANNOT_RENEW_DELAY_DETECTOR");
    state->ResetRoute(5001);
    auto reset = state->Snapshot(5001);
    ok &= Check(reset.routeRevision == 1 && !reset.controllerObserved && !reset.delayObserved &&
        !reset.feedbackFresh && reset.targetRateBps == 0 && reset.lastDelayOveruseAtMs == 0 &&
        reset.lastCwndPushbackAtMs == 0, "ROUTE_RESET_DISCARDS_OLD_EVIDENCE");

    auto delegate = std::make_unique<Controller>();
    auto* raw = delegate.get();
    raw->state = state;
    auto wrapper = CreateObservingNetworkController(std::move(delegate), state);
    ok &= Check(Unchanged(wrapper->OnNetworkAvailability({}), 1) &&
        Unchanged(wrapper->OnNetworkRouteChange({}), 2) &&
        Unchanged(wrapper->OnProcessInterval({}), 3) &&
        Unchanged(wrapper->OnRemoteBitrateReport({}), 4) &&
        Unchanged(wrapper->OnRoundTripTimeUpdate({}), 5) &&
        Unchanged(wrapper->OnSentPacket({}), 6) &&
        Unchanged(wrapper->OnReceivedPacket({}), 7) &&
        Unchanged(wrapper->OnStreamsConfig({}), 8) &&
        Unchanged(wrapper->OnTargetRateConstraints({}), 9) &&
        Unchanged(wrapper->OnTransportLossReport({}), 10) &&
        Unchanged(wrapper->OnNetworkStateEstimate({}), 12) && wrapper->SupportsEcnAdaptation(),
        "ALL_CONTROLLER_OUTPUTS_PACING_PROBES_ECN_UNCHANGED");
    TransportPacketsFeedback feedback;
    PacketResult packet;
    packet.sent_packet.send_time = Timestamp::Millis(100);
    packet.receive_time = Timestamp::Millis(110);
    feedback.packet_feedbacks.push_back(packet);
    ok &= Check(Unchanged(wrapper->OnTransportPacketsFeedback(feedback), 11) &&
        state->Snapshot(GoogCcTelemetryState::NowMs()).feedbackFresh &&
        state->Snapshot(GoogCcTelemetryState::NowMs()).delayState == "overuse",
        "REAL_RECEIVE_TIME_FEEDBACK_CONFIRMS_INLINE_DELAY_EVENT");
    const PacketResult* expectedPacketStorage = nullptr;
    const Timestamp* expectedArrivalStorage = nullptr;
    wrapper->OnTransportPacketsFeedback([&] {
        TransportPacketsFeedback transferred;
        transferred.feedback_time = Timestamp::Millis(120);
        transferred.data_in_flight = DataSize::Bytes(9876);
        packet.sent_packet.sequence_number = 456;
        transferred.packet_feedbacks.push_back(packet);
        transferred.sendless_arrival_times.push_back(Timestamp::Millis(111));
        expectedPacketStorage = transferred.packet_feedbacks.data();
        expectedArrivalStorage = transferred.sendless_arrival_times.data();
        return transferred;
    }());
    ok &= Check(raw->packetStorage == expectedPacketStorage &&
        raw->arrivalStorage == expectedArrivalStorage && raw->feedbackFieldsPreserved,
        "FEEDBACK_VECTORS_RETAIN_STORAGE_AND_FIELDS_WITHOUT_EXTRA_COPY");
    if (raw->packetStorage != expectedPacketStorage || raw->arrivalStorage != expectedArrivalStorage ||
        !raw->feedbackFieldsPreserved) {
        std::cout << "FEEDBACK_STORAGE_DETAILS packet=" << (raw->packetStorage == expectedPacketStorage)
            << " arrival=" << (raw->arrivalStorage == expectedArrivalStorage)
            << " fields=" << raw->feedbackFieldsPreserved << '\n';
    }
    state->ObserveUpdate(Fixture(0), 1);
    state->ConfirmFeedback(1, true);
    raw->emptyProcess = true;
    ok &= Check(!wrapper->OnProcessInterval({}).has_updates(), "EMPTY_GCC_UPDATE_REMAINS_EMPTY");
    auto confirmed = state->Snapshot(GoogCcTelemetryState::NowMs());
    ok &= Check(confirmed.targetUpdatedAtMs > 1 && confirmed.feedbackAtMs == 1 &&
        !confirmed.feedbackFresh, "PROCESS_CONFIRMS_TARGET_WITHOUT_INVENTING_ACKS");
    auto independent = std::make_shared<GoogCcTelemetryState>();
    independent->ObserveDelay(BandwidthUsage::kBwNormal, 9999);
    ok &= Check(state->Snapshot(9999).delayState == "overuse" &&
        independent->Snapshot(9999).delayState == "normal", "CONNECTION_STATE_IS_ISOLATED");
    wrapper.reset();
    ok &= Check(!state->Snapshot(9999).controllerObserved &&
        state->Snapshot(9999).delayState == "unknown", "CONTROLLER_DESTRUCTION_EXPIRES_EVIDENCE");
    std::atomic<bool> finished{false};
    std::thread writer([&] {
        for (int index = 0; index < 20'000; ++index)
            state->ObserveUpdate(Fixture(index), 1000);
        finished.store(true, std::memory_order_release);
    });
    bool consistent = true;
    unsigned reads = 0;
    while (!finished.load(std::memory_order_acquire) || reads < 1000) {
        const auto view = state->Snapshot(1000);
        consistent &= !view.controllerObserved ||
            view.effectiveTargetRateBps == static_cast<std::uint64_t>(view.targetRateBps * 0.75);
        ++reads;
    }
    writer.join();
    ok &= Check(consistent, "CONCURRENT_DIAGNOSTICS_NEVER_MIX_CONTROLLER_OUTPUTS");
    std::cout << "GOOG_CC_TELEMETRY=" << (ok ? "PASS" : "FAIL") << '\n';
    return ok ? 0 : 1;
}
