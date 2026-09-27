// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <optional>

#include "ISessionMediaAccess.h"

namespace webrtc {
template <class T>
class scoped_refptr;
class VideoTrackSourceInterface;
}

namespace remote::app {

class InProcessSessionEngine;

// Keeps UI media bindings on a narrow port instead of handing the concrete
// session engine to controller windows.
class InProcessSessionMediaAdapter final : public ISessionMediaAccess {
public:
    explicit InProcessSessionMediaAdapter(InProcessSessionEngine& engine);

    SessionCommandResult SetRoomRemoteVideoSink(
        const std::string& pairId,
        const std::string& slot,
        webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink) override;
    SessionCommandResult SetDirectRemoteVideoSink(
        webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink) override;
    SessionCommandResult NotifyRoomScreenFirstFramePresented(
        const std::string& pairId,
        std::uint64_t screenShareGeneration,
        std::uint32_t startupElapsedMs) override;
    void SetLocalCameraPreviewSink(
        webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink) override;
    void SetRemoteInputSink(IRemoteInputSink* sink) override;
    void SetRemoteFileTransferSink(IFileTransferSink* sink) override;
    void SetRemoteClipboardSink(IClipboardSink* sink) override;
    void SetRemoteCursorCallback(RemoteCursorCallback callback) override;
    SessionCommandResult SendRemoteInput(
        const RemoteInputEvent& event) override;
    SessionCommandResult SendRemoteFileMessage(
        const std::string& peerDeviceId,
        const FileTransferMessage& message) override;
    SessionCommandResult SendRemoteClipboardMessage(
        const std::string& peerDeviceId,
        const std::string& clipboardSessionId,
        const ClipboardMessage& message) override;
    SessionCommandResult SetRemoteAudioPlaybackMuted(bool muted) override;
    SessionCommandResult SetDirectScreenStreamPreference(
        const ScreenStreamPreferenceRequest& preference) override;
    SessionCommandResult RequestDirectSharedDisplaySwitch(
        const std::string& stableDisplayKey) override;
    void SetPreferredHardwareDecoderName(std::string name) override;
    SessionCommandResult ApplyVideoPipelinePreferences(
        DesktopCaptureImplementation desktopCaptureImplementation,
        VideoEncoderPreference videoEncoderPreference,
        FfmpegX264Preset quality,
        FfmpegHardwareBackend ffmpegHardwareBackend,
        std::string preferredAutomaticEncoderId,
        VideoDecoderPreference videoDecoderPreference) override;

private:
    friend class InProcessSessionEngine;

    SessionCommandResult SetRoomVideoSource(
        const std::string& slot,
        webrtc::scoped_refptr<webrtc::VideoTrackSourceInterface> source,
        const std::string& trackId,
        std::optional<std::string> expectedRoomId = std::nullopt,
        std::optional<std::uint64_t> expectedCameraGeneration =
            std::nullopt);
    SessionCommandResult ClearRoomVideoSource(const std::string& slot);
    SessionCommandResult SetRoomVideoSlotSendingActive(
        const std::string& slot,
        bool active);

    InProcessSessionEngine* engine_;
};

}  // namespace remote::app
