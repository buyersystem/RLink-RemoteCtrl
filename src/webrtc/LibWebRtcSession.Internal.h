// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include "LibWebRtcSession.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <condition_variable>
#include <functional>
#include <limits>
#include <utility>
#include <vector>

#include "src/core/ScreenNetworkPolicy.h"
#include "src/webrtc/PeerConnectionStatsCollector.h"
#include "src/core/ScreenStreamPolicy.h"

namespace remote {
namespace webrtc_session_detail {

inline constexpr int kMaximumScreenBitrateBps = 100'000'000;
inline constexpr int kDesktopStartupProbeFloorBps = 2'000'000;
inline constexpr int kDefaultWebRtcMinimumBitrateBps = 30'000;
inline constexpr int kMaximumScreenConnectionBitrateBps = 105'000'000;

inline std::uint64_t SteadyNowMs()
{
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
}

inline const char* ProgressiveBitrateCeilingStatusName(
    ProgressiveBitrateCeilingStatus status)
{
    switch (status) {
    case ProgressiveBitrateCeilingStatus::kDisabled:
        return "disabled";
    case ProgressiveBitrateCeilingStatus::kWaitingForStats:
        return "waiting_for_stats";
    case ProgressiveBitrateCeilingStatus::kStabilizing:
        return "stabilizing";
    case ProgressiveBitrateCeilingStatus::kStable:
        return "stable";
    case ProgressiveBitrateCeilingStatus::kProbePending:
        return "probe_pending";
    case ProgressiveBitrateCeilingStatus::kCooldown:
        return "cooldown";
    }
    return "unknown";
}

inline const char* AdaptiveScreenFrameRateStatusName(
    AdaptiveScreenFrameRateStatus status)
{
    switch (status) {
    case AdaptiveScreenFrameRateStatus::kDisabled:
        return "disabled";
    case AdaptiveScreenFrameRateStatus::kWaitingForActivity:
        return "waiting_for_activity";
    case AdaptiveScreenFrameRateStatus::kIdleSuspended:
        return "idle_suspended";
    case AdaptiveScreenFrameRateStatus::kStartupGrace:
        return "startup_grace";
    case AdaptiveScreenFrameRateStatus::kWaitingForCapacity:
        return "waiting_for_capacity";
    case AdaptiveScreenFrameRateStatus::kStable:
        return "stable";
    case AdaptiveScreenFrameRateStatus::kReducing:
        return "reducing";
    case AdaptiveScreenFrameRateStatus::kRecovering:
        return "recovering";
    }
    return "unknown";
}

inline WebRtcIceGatheringState ToPublicIceGatheringState(
    webrtc::PeerConnectionInterface::IceGatheringState state)
{
    switch (state) {
    case webrtc::PeerConnectionInterface::kIceGatheringNew:
        return WebRtcIceGatheringState::kNew;
    case webrtc::PeerConnectionInterface::kIceGatheringGathering:
        return WebRtcIceGatheringState::kGathering;
    case webrtc::PeerConnectionInterface::kIceGatheringComplete:
        return WebRtcIceGatheringState::kComplete;
    }
    return WebRtcIceGatheringState::kNew;
}

inline bool EqualsIgnoreCase(const std::string& left, const char* right)
{
    const std::string rightText(right);
    if (left.size() != rightText.size()) {
        return false;
    }
    return std::equal(left.begin(), left.end(), rightText.begin(),
                      [](char a, char b) {
                          return std::tolower(
                                     static_cast<unsigned char>(a)) ==
                                 std::tolower(
                                     static_cast<unsigned char>(b));
                      });
}

inline std::vector<webrtc::RtpCodecCapability> H264CodecPreferences(
    webrtc::PeerConnectionFactoryInterface* factory)
{
    auto codecs = factory
                      ->GetRtpReceiverCapabilities(webrtc::MediaType::VIDEO)
                      .codecs;
    std::erase_if(codecs, [](const webrtc::RtpCodecCapability& codec) {
        return !codec.IsResiliencyCodec() &&
               !EqualsIgnoreCase(codec.name, "H264");
    });
    return codecs;
}

}  // namespace webrtc_session_detail

using namespace webrtc_session_detail;

class LibWebRtcSession::CallbackGate final {
public:
    class Lease final {
    public:
        Lease() = default;
        Lease(CallbackGate* gate, LibWebRtcSession* owner)
            : gate_(gate), owner_(owner)
        {}
        Lease(const Lease&) = delete;
        Lease& operator=(const Lease&) = delete;
        Lease(Lease&& other) noexcept
            : gate_(std::exchange(other.gate_, nullptr)),
              owner_(std::exchange(other.owner_, nullptr))
        {}
        ~Lease()
        {
            if (gate_) {
                gate_->Leave();
            }
        }

        LibWebRtcSession* Owner() const { return owner_; }

    private:
        CallbackGate* gate_ = nullptr;
        LibWebRtcSession* owner_ = nullptr;
    };

    explicit CallbackGate(LibWebRtcSession* owner) : owner_(owner) {}

    Lease Enter()
    {
        std::lock_guard lock(mutex_);
        if (!owner_) {
            return {};
        }
        ++activeCallbacks_;
        return {this, owner_};
    }

    void DetachAndWait()
    {
        std::unique_lock lock(mutex_);
        owner_ = nullptr;
        condition_.wait(lock, [this] { return activeCallbacks_ == 0; });
    }

private:
    void Leave()
    {
        std::lock_guard lock(mutex_);
        --activeCallbacks_;
        if (activeCallbacks_ == 0) {
            condition_.notify_all();
        }
    }

    std::mutex mutex_;
    std::condition_variable condition_;
    LibWebRtcSession* owner_ = nullptr;
    int activeCallbacks_ = 0;
};

}  // namespace remote
