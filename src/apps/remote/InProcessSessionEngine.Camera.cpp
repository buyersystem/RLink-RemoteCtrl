// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "InProcessSessionEngine.h"
#include "InProcessSessionMediaState.h"
#include "InProcessSessionMediaAdapter.h"

#include <thread>
#include <utility>

#include "src/platform/win/WindowsCameraCaptureSource.h"

namespace remote::app {
namespace {

SessionCommandResult Success()
{
    return {true, {}, {}};
}

SessionCommandResult Failure(std::string code, std::string message)
{
    return {false, std::move(code), std::move(message)};
}

}  // namespace

SessionCommandResult InProcessSessionEngine::SetLocalCameraEnabled(bool enabled)
{
    const auto snapshot = Snapshot();
    if (snapshot.room.membership != RoomMembershipState::kActive) {
        return Failure("room_not_active",
                       "Join a room before publishing the local camera.");
    }
    const bool alreadyEnabled =
        snapshot.media.localCamera == LocalCameraState::kPublishing;
    if (alreadyEnabled == enabled) {
        return Success();
    }

    if (enabled) {
        if (!options_.enableRealCameraCapture) {
            return Failure("camera_capture_disabled",
                           "Real camera capture is disabled for this engine.");
        }
        const std::string preferredCameraId =
            snapshot.media.localMediaDevices.camera.preferredDeviceId;
        if ((preferredCameraId == kSystemDefaultMediaDeviceId &&
             snapshot.media.localMediaDevices.camera.devices.empty()) ||
            (preferredCameraId != kSystemDefaultMediaDeviceId &&
             !LocalMediaCoordinator::ContainsDevice(
                 snapshot.media.localMediaDevices.camera.devices,
                 preferredCameraId))) {
            return Failure(
                "camera_device_unavailable",
                "The selected camera is no longer available.");
        }
        {
            std::lock_guard lock(mutex_);
            snapshot_.media.localCamera = LocalCameraState::kStarting;
        }
        PublishSnapshot();
        auto source =
            webrtc::make_ref_counted<WindowsCameraCaptureSource>();
        if (!source->StartCapture(
                preferredCameraId == kSystemDefaultMediaDeviceId
                    ? std::string{}
                    : preferredCameraId)) {
            const std::string error = source->LastError();
            {
                std::lock_guard lock(mutex_);
                snapshot_.media.localCamera = LocalCameraState::kFailed;
                auto& category =
                    snapshot_.media.localMediaDevices.camera;
                category.state =
                    MediaDeviceSelectionState::kFailed;
                category.errorCode =
                    "camera_capture_start_failed";
                category.errorMessage = error;
                ++snapshot_.media.localMediaDevices.revision;
                snapshot_.error.code = "camera_capture_start_failed";
                snapshot_.error.message = error;
            }
            PublishSnapshot();
            return Failure("camera_capture_start_failed", error);
        }
        const auto publishResult = mediaAccess_->SetRoomVideoSource(
            kCameraMainVideoSlot, source,
            "room-camera-" + snapshot.localDeviceId);
        if (!publishResult.accepted) {
            source->StopCapture();
            {
                std::lock_guard lock(mutex_);
                snapshot_.media.localCamera = LocalCameraState::kFailed;
            }
            PublishSnapshot();
            return publishResult;
        }

        webrtc::VideoSinkInterface<webrtc::VideoFrame>* previewSink = nullptr;
        webrtc::scoped_refptr<webrtc::VideoTrackInterface> previewTrack;
        {
            std::lock_guard lock(mutex_);
            localMedia_.SetCameraCaptureSource(source);
            previewSink = mediaState_->localCameraPreviewSink;
            const auto trackIt = mediaState_->localRoomVideoTracks.find(
                kCameraMainVideoSlot);
            if (trackIt != mediaState_->localRoomVideoTracks.end()) {
                previewTrack = static_cast<webrtc::VideoTrackInterface*>(
                    trackIt->second.get());
            }
            mediaState_->localCameraPreviewTrack = previewTrack;
            auto& category =
                snapshot_.media.localMediaDevices.camera;
            category.activeDeviceId =
                source->ActiveDeviceId();
            category.state =
                MediaDeviceSelectionState::kReady;
            category.errorCode.clear();
            category.errorMessage.clear();
            ++snapshot_.media.localMediaDevices.revision;
        }
        if (previewTrack && previewSink) {
            previewTrack->AddOrUpdateSink(
                previewSink, webrtc::VideoSinkWants());
        }

        const bool microphonePublishing =
            snapshot.media.localMicrophone == LocalMicrophoneState::kPublishing;
        const auto signalResult = signaling_->SetRoomMediaState(
            snapshot.room.roomId, true, microphonePublishing);
        if (!signalResult.accepted) {
            if (previewTrack && previewSink) {
                previewTrack->RemoveSink(previewSink);
            }
            (void)mediaAccess_->ClearRoomVideoSource(kCameraMainVideoSlot);
            source->StopCapture();
            {
                std::lock_guard lock(mutex_);
                localMedia_.SetCameraCaptureSource(nullptr);
                mediaState_->localCameraPreviewTrack = nullptr;
                snapshot_.media.localCamera = LocalCameraState::kFailed;
                snapshot_.media.localMediaDevices.camera.activeDeviceId.clear();
                ++snapshot_.media.localMediaDevices.revision;
            }
            PublishSnapshot();
            return Failure(signalResult.errorCode, signalResult.errorMessage);
        }

        {
            std::lock_guard lock(mutex_);
            snapshot_.media.localCamera = LocalCameraState::kPublishing;
            snapshot_.error.code.clear();
            snapshot_.error.message.clear();
        }
        PublishSnapshot();
        return Success();
    }

    webrtc::scoped_refptr<WindowsCameraCaptureSource> source;
    webrtc::scoped_refptr<webrtc::VideoTrackInterface> previewTrack;
    webrtc::VideoSinkInterface<webrtc::VideoFrame>* previewSink = nullptr;
    {
        std::lock_guard lock(mutex_);
        (void)localMedia_.BeginCameraOperation();
        snapshot_.media.localCamera = LocalCameraState::kStopping;
        source = localMedia_.TakeCameraCaptureSource();
        previewTrack = std::exchange(mediaState_->localCameraPreviewTrack, nullptr);
        previewSink = mediaState_->localCameraPreviewSink;
    }
    PublishSnapshot();
    const auto clearResult =
        mediaAccess_->ClearRoomVideoSource(kCameraMainVideoSlot);
    if (!clearResult.accepted) {
        {
            std::lock_guard lock(mutex_);
            localMedia_.SetCameraCaptureSource(source);
            mediaState_->localCameraPreviewTrack = previewTrack;
            snapshot_.media.localCamera = LocalCameraState::kFailed;
        }
        PublishSnapshot();
        return clearResult;
    }
    if (previewTrack && previewSink) {
        previewTrack->RemoveSink(previewSink);
    }
    if (source) {
        // StopCapture can block in a camera driver's StopCapture call. The
        // UI has already detached the WebRTC track and published kOff, so
        // complete only the physical-device teardown on a retained worker.
        std::jthread stopThread(
            [capture = std::move(source)](std::stop_token) {
                capture->StopCapture();
            });
        std::lock_guard lock(mutex_);
        retiredCameraStopThreads_.push_back(std::move(stopThread));
    }
    const bool microphonePublishing =
        snapshot.media.localMicrophone == LocalMicrophoneState::kPublishing;
    const auto signalResult = signaling_->SetRoomMediaState(
        snapshot.room.roomId, false, microphonePublishing);
    {
        std::lock_guard lock(mutex_);
        snapshot_.media.localCamera = LocalCameraState::kOff;
        auto& category = snapshot_.media.localMediaDevices.camera;
        category.activeDeviceId.clear();
        category.state = MediaDeviceSelectionState::kReady;
        category.errorCode.clear();
        category.errorMessage.clear();
        ++snapshot_.media.localMediaDevices.revision;
    }
    PublishSnapshot();
    return signalResult.accepted
               ? Success()
               : Failure(signalResult.errorCode, signalResult.errorMessage);
}

}  // namespace remote::app
