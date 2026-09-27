// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <string>
#include <unordered_map>

#include "api/media_stream_interface.h"
#include "api/scoped_refptr.h"
#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"

namespace remote::app {

struct InProcessSessionMediaState {
    std::unordered_map<
        std::string,
        webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface>>
        localRoomVideoTracks;
    std::unordered_map<
        std::string,
        webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface>>
        idleRoomVideoTracks;
    webrtc::VideoSinkInterface<webrtc::VideoFrame>*
        localCameraPreviewSink = nullptr;
    webrtc::scoped_refptr<webrtc::VideoTrackInterface>
        localCameraPreviewTrack;
    webrtc::scoped_refptr<webrtc::AudioSourceInterface>
        localMicrophoneAudioSource;
    webrtc::scoped_refptr<webrtc::AudioTrackInterface>
        localMicrophoneAudioTrack;
};

}  // namespace remote::app
