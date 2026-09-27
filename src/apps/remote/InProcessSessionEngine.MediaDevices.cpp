// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "InProcessSessionEngine.h"
#include "InProcessSessionMediaState.h"
#include "InProcessSessionMediaAdapter.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "api/make_ref_counted.h"
#include "InProcessSessionEngineInternal.h"
#include "RoomMediaSlots.h"
#include "src/platform/win/WindowsCameraCaptureSource.h"
#include "src/webrtc/LibWebRtcSession.h"
#include "src/webrtc/WebRtcRuntime.h"

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

SessionCommandResult InProcessSessionEngine::SetRemoteAudioPlaybackMuted(
    bool muted)
{
    std::vector<std::shared_ptr<RoomPairRuntime>> pairs;
    LibWebRtcSession* directSession = nullptr;
    {
        std::lock_guard lock(mutex_);
        snapshot_.media.roomAudioPlaybackMuted = muted;
        if (webRtcSession_ && directSession_.mediaSlotsPrepared &&
            snapshot_.purpose == SessionPurpose::kRemoteControl) {
            directSession = webRtcSession_.get();
        }
        pairs.reserve(roomPairs_.size());
        for (const auto& [pairId, pair] : roomPairs_) {
            (void)pairId;
            if (pair && pair->session && pair->mediaSlotsPrepared) {
                pairs.push_back(pair);
            }
        }
    }
    for (const auto& pair : pairs) {
        pair->session->SetRemoteAudioSlotEnabled(
            kMicrophoneMainAudioSlot, !muted);
    }
    if (directSession) {
        directSession->SetRemoteAudioSlotEnabled(
            kMicrophoneMainAudioSlot, !muted);
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult InProcessSessionEngine::SetRoomAudioPlaybackMuted(
    bool muted)
{
    return SetRemoteAudioPlaybackMuted(muted);
}

SessionCommandResult
InProcessSessionEngine::RefreshLocalMediaDevices()
{
    if (!runtime_) {
        return Failure(
            "webrtc_runtime_unavailable",
            "The WebRTC runtime is unavailable.");
    }

    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state == SessionEngineState::kStopping ||
            snapshot_.state == SessionEngineState::kStopped) {
            return Failure(
                "session_engine_stopping",
                "The session engine is stopping.");
        }
        if (snapshot_.media.localMediaDevices.refreshing) {
            return Success();
        }
        snapshot_.media.localMediaDevices.refreshing = true;
        ++snapshot_.media.localMediaDevices.revision;
        mediaDeviceOperationThreads_.emplace_back(
            [this](std::stop_token stopToken) {
                std::vector<MediaDeviceDescriptor> cameras;
                for (const auto& device :
                     WindowsCameraCaptureSource::EnumerateDevices()) {
                    if (stopToken.stop_requested()) {
                        return;
                    }
                    cameras.push_back(
                        {device.id, device.name, true});
                }
                const auto audioDevices =
                    runtime_->EnumerateAudioDevices();

                {
                    std::lock_guard lock(mutex_);
                    if (snapshot_.state ==
                            SessionEngineState::kStopping ||
                        snapshot_.state ==
                            SessionEngineState::kStopped ||
                        stopToken.stop_requested()) {
                        snapshot_.media.localMediaDevices.refreshing = false;
                        return;
                    }
                    auto& camera =
                        snapshot_.media.localMediaDevices.camera;
                    auto& microphone =
                        snapshot_.media.localMediaDevices.microphone;
                    auto& speaker =
                        snapshot_.media.localMediaDevices.speaker;
                    camera.devices = std::move(cameras);
                    microphone.devices =
                        audioDevices.microphones;
                    speaker.devices = audioDevices.speakers;

                    if (const auto cameraSource =
                            localMedia_.CameraCaptureSource()) {
                        camera.activeDeviceId =
                            cameraSource->ActiveDeviceId();
                        const auto activeCamera = std::find_if(
                            camera.devices.begin(),
                            camera.devices.end(),
                            [&camera](const MediaDeviceDescriptor& device) {
                                return device.id ==
                                    camera.activeDeviceId;
                            });
                        camera.activeDeviceName =
                            activeCamera == camera.devices.end()
                                ? std::string{}
                                : activeCamera->name;
                    } else {
                        camera.activeDeviceId.clear();
                        camera.activeDeviceName.clear();
                    }
                    microphone.activeDeviceId =
                        audioDevices.activeMicrophoneId;
                    microphone.activeDeviceName =
                        audioDevices.activeMicrophoneName;
                    speaker.activeDeviceId =
                        audioDevices.activeSpeakerId;
                    speaker.activeDeviceName =
                        audioDevices.activeSpeakerName;

                    localMedia_.UpdateAvailability(
                        camera, LocalMediaDeviceKind::kCamera);
                    localMedia_.UpdateAvailability(
                        microphone, LocalMediaDeviceKind::kMicrophone);
                    localMedia_.UpdateAvailability(
                        speaker, LocalMediaDeviceKind::kSpeaker);
                    if (!audioDevices.error.empty()) {
                        if (microphone.devices.empty()) {
                            microphone.errorMessage =
                                audioDevices.error;
                        }
                        if (speaker.devices.empty()) {
                            speaker.errorMessage =
                                audioDevices.error;
                        }
                    }
                    snapshot_.media.localMediaDevices.refreshing = false;
                    ++snapshot_.media.localMediaDevices.revision;
                }
                PublishSnapshot();
            });
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult
InProcessSessionEngine::SelectLocalCameraDevice(
    const std::string& deviceId)
{
    const std::string normalizedId =
        localMedia_.NormalizeDeviceId(deviceId);
    bool cameraPublishing = false;
    std::uint64_t trackGeneration = 0;
    std::string expectedRoomId;
    {
        std::lock_guard lock(mutex_);
        auto& category = snapshot_.media.localMediaDevices.camera;
        if (snapshot_.state == SessionEngineState::kStopping ||
            snapshot_.state == SessionEngineState::kStopped) {
            return Failure(
                "session_engine_stopping",
                "The session engine is stopping.");
        }
        const auto validation = localMedia_.ValidateSelection(
            category, normalizedId, LocalMediaDeviceKind::kCamera);
        if (!validation.accepted) {
            return validation;
        }
        if (category.preferredDeviceId == normalizedId &&
            category.state ==
                MediaDeviceSelectionState::kReady) {
            return Success();
        }
        cameraPublishing =
            snapshot_.media.localCamera ==
            LocalCameraState::kPublishing;
        if (!cameraPublishing) {
            category.preferredDeviceId = normalizedId;
            category.activeDeviceId.clear();
            category.state =
                MediaDeviceSelectionState::kReady;
            category.errorCode.clear();
            category.errorMessage.clear();
            options_.preferredCameraDeviceId = normalizedId;
            ++snapshot_.media.localMediaDevices.revision;
        } else {
            category.state =
                MediaDeviceSelectionState::kSwitching;
            category.errorCode.clear();
            category.errorMessage.clear();
            trackGeneration = localMedia_.BeginCameraOperation();
            expectedRoomId = snapshot_.room.roomId;
            ++snapshot_.media.localMediaDevices.revision;
        }
    }
    PublishSnapshot();
    if (!cameraPublishing) {
        return Success();
    }

    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state == SessionEngineState::kStopping ||
            snapshot_.state == SessionEngineState::kStopped) {
            return Failure(
                "session_engine_stopping",
                "The session engine is stopping.");
        }
        mediaDeviceOperationThreads_.emplace_back(
            [this, normalizedId, trackGeneration,
             expectedRoomId](std::stop_token) {
            auto source =
                webrtc::make_ref_counted<
                    WindowsCameraCaptureSource>();
            const std::string requestedId =
                normalizedId == kSystemDefaultMediaDeviceId
                    ? std::string{}
                    : normalizedId;
            if (!source->StartCapture(requestedId)) {
                const std::string error = source->LastError();
                {
                    std::lock_guard lock(mutex_);
                    auto& category =
                        snapshot_.media.localMediaDevices.camera;
                    if (localMedia_.IsCurrentCameraOperation(
                            trackGeneration) &&
                        category.state ==
                            MediaDeviceSelectionState::kSwitching) {
                        category.state =
                            MediaDeviceSelectionState::kFailed;
                        category.errorCode =
                            "camera_device_switch_failed";
                        category.errorMessage = error;
                        ++snapshot_.media.localMediaDevices.revision;
                    }
                }
                PublishSnapshot();
                return;
            }

            webrtc::scoped_refptr<WindowsCameraCaptureSource>
                previousSource;
            webrtc::scoped_refptr<webrtc::VideoTrackInterface>
                previousPreviewTrack;
            webrtc::VideoSinkInterface<webrtc::VideoFrame>*
                previewSink = nullptr;
            bool switchStillCurrent = false;
            {
                std::lock_guard lock(mutex_);
                previousSource = localMedia_.CameraCaptureSource();
                previousPreviewTrack = mediaState_->localCameraPreviewTrack;
                previewSink = mediaState_->localCameraPreviewSink;
                const auto& category =
                    snapshot_.media.localMediaDevices.camera;
                switchStillCurrent =
                    localMedia_.IsCurrentCameraOperation(
                        trackGeneration) &&
                    category.state ==
                        MediaDeviceSelectionState::kSwitching &&
                    snapshot_.media.localCamera ==
                        LocalCameraState::kPublishing &&
                    snapshot_.room.membership ==
                        RoomMembershipState::kActive &&
                    snapshot_.room.roomId == expectedRoomId;
            }
            if (!switchStillCurrent) {
                source->StopCapture();
                return;
            }

            const auto publishResult = mediaAccess_->SetRoomVideoSource(
                kCameraMainVideoSlot, source,
                "room-camera-switch-" +
                    std::to_string(trackGeneration),
                expectedRoomId, trackGeneration);
            if (!publishResult.accepted) {
                source->StopCapture();
                {
                    std::lock_guard lock(mutex_);
                    auto& category =
                        snapshot_.media.localMediaDevices.camera;
                    if (localMedia_.IsCurrentCameraOperation(
                            trackGeneration) &&
                        category.state ==
                            MediaDeviceSelectionState::kSwitching) {
                        category.state =
                            MediaDeviceSelectionState::kFailed;
                        category.errorCode =
                            publishResult.errorCode;
                        category.errorMessage =
                            publishResult.errorMessage;
                        ++snapshot_.media.localMediaDevices.revision;
                    }
                }
                PublishSnapshot();
                return;
            }

            webrtc::scoped_refptr<webrtc::VideoTrackInterface>
                newPreviewTrack;
            bool committed = false;
            {
                std::lock_guard lock(mutex_);
                auto& category =
                    snapshot_.media.localMediaDevices.camera;
                if (localMedia_.IsCurrentCameraOperation(
                        trackGeneration) &&
                    category.state ==
                        MediaDeviceSelectionState::kSwitching &&
                    snapshot_.media.localCamera ==
                        LocalCameraState::kPublishing &&
                    snapshot_.room.roomId == expectedRoomId) {
                    const auto track = mediaState_->localRoomVideoTracks.find(
                        kCameraMainVideoSlot);
                    if (track != mediaState_->localRoomVideoTracks.end()) {
                        newPreviewTrack =
                            static_cast<webrtc::VideoTrackInterface*>(
                                track->second.get());
                    }
                    localMedia_.SetCameraCaptureSource(source);
                    mediaState_->localCameraPreviewTrack = newPreviewTrack;
                    category.preferredDeviceId = normalizedId;
                    category.activeDeviceId =
                        source->ActiveDeviceId();
                    category.state =
                        MediaDeviceSelectionState::kReady;
                    category.errorCode.clear();
                    category.errorMessage.clear();
                    options_.preferredCameraDeviceId = normalizedId;
                    ++snapshot_.media.localMediaDevices.revision;
                    committed = true;
                }
            }
            if (!committed) {
                source->StopCapture();
                return;
            }
            if (newPreviewTrack && previewSink) {
                newPreviewTrack->AddOrUpdateSink(
                    previewSink, webrtc::VideoSinkWants());
            }
            if (previousPreviewTrack && previewSink &&
                previousPreviewTrack != newPreviewTrack) {
                previousPreviewTrack->RemoveSink(previewSink);
            }
            if (previousSource && previousSource != source) {
                previousSource->StopCapture();
            }
            PublishSnapshot();
        });
    }
    return Success();
}

SessionCommandResult
InProcessSessionEngine::SelectLocalMicrophoneDevice(
    const std::string& deviceId)
{
    const std::string normalizedId =
        localMedia_.NormalizeDeviceId(deviceId);
    {
        std::lock_guard lock(mutex_);
        auto& category =
            snapshot_.media.localMediaDevices.microphone;
        if (snapshot_.state == SessionEngineState::kStopping ||
            snapshot_.state == SessionEngineState::kStopped) {
            return Failure(
                "session_engine_stopping",
                "The session engine is stopping.");
        }
        const auto validation = localMedia_.ValidateSelection(
            category, normalizedId,
            LocalMediaDeviceKind::kMicrophone);
        if (!validation.accepted) {
            return validation;
        }
        if (category.preferredDeviceId == normalizedId &&
            category.state ==
                MediaDeviceSelectionState::kReady) {
            return Success();
        }
        category.state =
            MediaDeviceSelectionState::kSwitching;
        category.errorCode.clear();
        category.errorMessage.clear();
        ++snapshot_.media.localMediaDevices.revision;
    }
    PublishSnapshot();

    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state == SessionEngineState::kStopping ||
            snapshot_.state == SessionEngineState::kStopped) {
            return Failure(
                "session_engine_stopping",
                "The session engine is stopping.");
        }
        mediaDeviceOperationThreads_.emplace_back(
            [this, normalizedId](std::stop_token) {
            const auto result =
                runtime_->SelectRecordingDevice(normalizedId);
            {
                std::lock_guard lock(mutex_);
                auto& category =
                    snapshot_.media.localMediaDevices.microphone;
                category.activeDeviceId =
                    result.activeDeviceId;
                if (result.succeeded) {
                    category.preferredDeviceId =
                        normalizedId;
                    category.state =
                        MediaDeviceSelectionState::kReady;
                    category.errorCode.clear();
                    category.errorMessage.clear();
                    options_.preferredMicrophoneDeviceId =
                        normalizedId;
                } else {
                    category.state =
                        MediaDeviceSelectionState::kFailed;
                    category.errorCode = result.errorCode;
                    category.errorMessage =
                        result.errorMessage;
                }
                ++snapshot_.media.localMediaDevices.revision;
            }
            PublishSnapshot();
        });
    }
    return Success();
}

SessionCommandResult
InProcessSessionEngine::SelectLocalSpeakerDevice(
    const std::string& deviceId)
{
    const std::string normalizedId =
        localMedia_.NormalizeDeviceId(deviceId);
    {
        std::lock_guard lock(mutex_);
        auto& category =
            snapshot_.media.localMediaDevices.speaker;
        if (snapshot_.state == SessionEngineState::kStopping ||
            snapshot_.state == SessionEngineState::kStopped) {
            return Failure(
                "session_engine_stopping",
                "The session engine is stopping.");
        }
        const auto validation = localMedia_.ValidateSelection(
            category, normalizedId, LocalMediaDeviceKind::kSpeaker);
        if (!validation.accepted) {
            return validation;
        }
        if (category.preferredDeviceId == normalizedId &&
            category.state ==
                MediaDeviceSelectionState::kReady) {
            return Success();
        }
        category.state =
            MediaDeviceSelectionState::kSwitching;
        category.errorCode.clear();
        category.errorMessage.clear();
        ++snapshot_.media.localMediaDevices.revision;
    }
    PublishSnapshot();

    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state == SessionEngineState::kStopping ||
            snapshot_.state == SessionEngineState::kStopped) {
            return Failure(
                "session_engine_stopping",
                "The session engine is stopping.");
        }
        mediaDeviceOperationThreads_.emplace_back(
            [this, normalizedId](std::stop_token) {
            const auto result =
                runtime_->SelectPlayoutDevice(normalizedId);
            {
                std::lock_guard lock(mutex_);
                auto& category =
                    snapshot_.media.localMediaDevices.speaker;
                category.activeDeviceId =
                    result.activeDeviceId;
                if (result.succeeded) {
                    category.preferredDeviceId =
                        normalizedId;
                    category.state =
                        MediaDeviceSelectionState::kReady;
                    category.errorCode.clear();
                    category.errorMessage.clear();
                    options_.preferredSpeakerDeviceId =
                        normalizedId;
                } else {
                    category.state =
                        MediaDeviceSelectionState::kFailed;
                    category.errorCode = result.errorCode;
                    category.errorMessage =
                        result.errorMessage;
                }
                ++snapshot_.media.localMediaDevices.revision;
            }
            PublishSnapshot();
        });
    }
    return Success();
}

}  // namespace remote::app
