// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "api/media_stream_interface.h"
#include "api/rtp_transceiver_interface.h"
#include "api/scoped_refptr.h"
#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"
#include "src/core/ScreenNetworkPolicy.h"
#include "src/core/SessionDiagnostics.h"
#include "media_intelligence/core/ContentAwareStreamPolicy.h"
#include "media_intelligence/core/GoogCcNetworkPressure.h"

namespace remote {

class LibWebRtcSession;

class MediaSlotManager final {
private:
    friend class LibWebRtcSession;
    friend class ContentAwareStreamExecutionTestAccess;

    struct VideoSlotBinding {
        webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver;
        webrtc::scoped_refptr<webrtc::VideoTrackInterface> remoteTrack;
        webrtc::VideoSinkInterface<webrtc::VideoFrame>* remoteSink = nullptr;
        std::uint32_t configuredMaxFrameRate = 0;
        std::uint32_t configuredOutputWidth = 0;
        std::uint32_t configuredOutputHeight = 0;
        std::uint64_t configuredStartBitrateBps = 0;
        std::uint64_t configuredMaxBitrateBps = 0;
        std::uint64_t configuredNetworkProbeMaxBitrateBps = 0;
        std::uint32_t configuredVideoBppHundredths = 15;
        std::uint32_t effectiveWidth = 0;
        std::uint32_t effectiveHeight = 0;
        std::uint32_t effectiveMaxFps = 0;
        std::uint64_t effectiveDesiredBitrateBps = 0;
        std::uint64_t effectiveMaxBitrateBps = 0;
        media_intelligence::ContentAwareStreamPolicyState contentPolicyState;
        media_intelligence::GoogCcNetworkPressureState contentNetworkPressure;
        media_intelligence::ScreenScene verifiedUserScene = media_intelligence::ScreenScene::kUnknown;
        std::uint64_t verifiedUserBitrateBps = 0;
        std::uint64_t verifiedUserLastSampleMs = 0;
        std::uint64_t verifiedUserRouteRevision = 0;
        std::uint64_t verifiedUserFirstSampleMs = 0;
        std::uint32_t verifiedUserSamples = 0;
        std::uint32_t verifiedUserWidth = 0, verifiedUserHeight = 0, verifiedUserFps = 0;
        std::uint64_t verifiedUserVideoCeilingBps = 0;
        ContentPolicyShadowSnapshot contentPolicyRecommendation;
        ContentPolicyExecutionSnapshot contentPolicyExecution;
        std::uint64_t contentPolicyRevision = 0;
        bool contentPolicyNeedsRestore = false;
        std::uint64_t contentPolicyEvidenceNotBeforeMs = 0;
        bool contentQualityMetricAvailable = false;
        bool contentQualityVerified = false;
        bool contentProcessingEvidenceAvailable = false;
        bool contentProcessingHealthy = false;
        AdaptiveScreenFrameRateState adaptiveFrameRate;
        std::uint64_t adaptiveFrameRateRevision = 0;
        std::string adaptiveFrameRateError;
        bool sendingActive = false;
        bool startBitrateBootstrapPending = true;
        std::uint32_t bitrateBootstrapAttempts = 0;
        std::uint32_t bitrateBootstrapSuccesses = 0;
        std::uint32_t mediaReadyBitrateRestarts = 0;
        std::uint32_t allocationProbePulses = 0;
        std::uint32_t bitrateProbeFloorReleases = 0;
        bool bitrateProbeFloorActive = false;
        std::string bitrateBootstrapError;
    };

    struct AudioSlotBinding {
        std::string name;
        webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver;
        webrtc::scoped_refptr<webrtc::AudioTrackInterface> remoteTrack;
        bool remotePlaybackEnabled = true;
    };

    std::unordered_map<std::string, VideoSlotBinding> videoSlots_;
    std::vector<std::string> videoSlotOrder_;
    AudioSlotBinding audioSlot_;
    webrtc::VideoSinkInterface<webrtc::VideoFrame>* remoteVideoSink_ = nullptr;
    webrtc::scoped_refptr<webrtc::VideoTrackInterface> remoteVideoTrack_;
};

}  // namespace remote
