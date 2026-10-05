// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <functional>
#include <string>

#include "src/core/DesktopCaptureTypes.h"
#include "src/core/ISessionEngine.h"
#include "src/protocol/ClipboardProtocol.h"
#include "src/protocol/FileTransferProtocol.h"
#include "src/protocol/RemoteCursorProtocol.h"
#include "src/protocol/RemoteInputProtocol.h"
#include "src/webrtc/VideoDecoderRuntimeStatus.h"
#include "src/webrtc/VideoEncoderRuntimeStatus.h"

namespace webrtc {
class VideoFrame;
template <typename VideoFrameT>
class VideoSinkInterface;
}

namespace remote::app {

// Narrow UI port for media bindings that are intentionally outside the
// command/snapshot session facade.
class ISessionMediaAccess {
public:
    using RemoteCursorCallback = std::function<void(
        const std::string& pairId,
        const RemoteCursorEnvelope& envelope)>;

    virtual ~ISessionMediaAccess() = default;

    virtual SessionCommandResult SetRoomRemoteVideoSink(
        const std::string& pairId,
        const std::string& slot,
        webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink) = 0;
    virtual SessionCommandResult SetDirectRemoteVideoSink(
        webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink) = 0;
    virtual SessionCommandResult NotifyRoomScreenFirstFramePresented(
        const std::string& pairId,
        std::uint64_t screenShareGeneration,
        std::uint32_t startupElapsedMs) = 0;
    virtual void SetLocalCameraPreviewSink(
        webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink) = 0;
    virtual void SetRemoteInputSink(IRemoteInputSink* sink) = 0;
    virtual void SetRemoteFileTransferSink(IFileTransferSink* sink) = 0;
    virtual void SetRemoteClipboardSink(IClipboardSink* sink) = 0;
    virtual void SetRemoteCursorCallback(RemoteCursorCallback callback) = 0;

    virtual SessionCommandResult SendRemoteInput(
        const RemoteInputEvent& event) = 0;
    virtual SessionCommandResult SendRemoteFileMessage(
        const std::string& peerDeviceId,
        const FileTransferMessage& message) = 0;
    virtual SessionCommandResult SendRemoteClipboardMessage(
        const std::string& peerDeviceId,
        const std::string& clipboardSessionId,
        const ClipboardMessage& message) = 0;
    virtual SessionCommandResult SetRemoteAudioPlaybackMuted(bool muted) = 0;
    virtual SessionCommandResult SetDirectScreenStreamPreference(
        const ScreenStreamPreferenceRequest& preference) = 0;
    // Queue acceptance is separate from the transport result. Completion may
    // run on a controller/cleanup thread and does not replace the remote ACK.
    // Rejected requests do not invoke completion.
    virtual SessionCommandResult QueueDirectScreenStreamPreference(
        const ScreenStreamPreferenceRequest& preference,
        std::function<void(SessionCommandResult)> completion)
    {
        (void)preference;
        (void)completion;
        return {false, "screen_stream_queue_unsupported",
                "Queued direct screen preferences are not supported."};
    }
    virtual SessionCommandResult RequestDirectSharedDisplaySwitch(
        const std::string& stableDisplayKey) = 0;
    virtual void SetPreferredHardwareDecoderName(std::string name) = 0;
    virtual SessionCommandResult ApplyVideoPipelinePreferences(
        DesktopCaptureImplementation desktopCaptureImplementation,
        VideoEncoderPreference videoEncoderPreference,
        FfmpegX264Preset quality,
        FfmpegHardwareBackend ffmpegHardwareBackend,
        std::string preferredAutomaticEncoderId,
        VideoDecoderPreference videoDecoderPreference) = 0;
};

}  // namespace remote::app
