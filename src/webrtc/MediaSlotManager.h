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

namespace remote {

class LibWebRtcSession;

class MediaSlotManager final {
private:
    friend class LibWebRtcSession;

    struct VideoSlotBinding {
        webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver;
        webrtc::scoped_refptr<webrtc::VideoTrackInterface> remoteTrack;
        webrtc::VideoSinkInterface<webrtc::VideoFrame>* remoteSink = nullptr;
        std::uint32_t configuredMaxFrameRate = 0;
        std::uint32_t configuredOutputWidth = 0;
        std::uint32_t configuredOutputHeight = 0;
        std::uint64_t configuredStartBitrateBps = 0;
        std::uint64_t configuredMaxBitrateBps = 0;
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
