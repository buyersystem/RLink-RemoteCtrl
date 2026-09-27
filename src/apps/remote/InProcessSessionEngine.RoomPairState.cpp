// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "InProcessSessionEngine.h"
#include "InProcessSessionMediaState.h"

#include <algorithm>

#include "ScreenShareCoordinator.h"
#include "InProcessSessionEngineInternal.h"
#include "RoomMediaSlots.h"
#include "src/platform/win/WindowsDesktopCaptureSource.h"
#include "src/webrtc/LibWebRtcSession.h"
#include "src/webrtc/WebRtcRuntime.h"

namespace remote::app {

void InProcessSessionEngine::OnRoomPairControllerSnapshot(
    const std::string& pairId,
    const SessionControllerSnapshot& controllerSnapshot)
{
    bool prepareOffererMedia = false;
    bool reapplyAudioDevices = false;
    bool synchronizeMicrophoneCapture = false;
    bool microphoneCaptureEnabled = false;
    std::shared_ptr<RoomPairRuntime> pair;
    IRemoteInputSink* releaseInputSink = nullptr;
    webrtc::scoped_refptr<WindowsDesktopCaptureSource>
        sourceToRefresh;
    {
        std::lock_guard lock(mutex_);
        const auto pairIt = roomPairs_.find(pairId);
        if (pairIt == roomPairs_.end()) {
            return;
        }
        pair = pairIt->second;
        auto snapshotIt = std::find_if(
            snapshot_.roomActivity.peerConnections.begin(),
            snapshot_.roomActivity.peerConnections.end(),
            [&pairId](const RoomPeerConnectionSnapshot& current) {
                return current.pairId == pairId;
            });
        if (snapshotIt == snapshot_.roomActivity.peerConnections.end()) {
            return;
        }
        snapshotIt->negotiationGeneration =
            controllerSnapshot.negotiationGeneration;
        snapshotIt->iceRestartAttempt =
            controllerSnapshot.iceRestartAttempt;
        switch (controllerSnapshot.state) {
        case SessionControllerState::kReady:
            snapshotIt->state = RoomPeerConnectionState::kStarting;
            if (!roomSession_.audioDevicesApplied_) {
                roomSession_.audioDevicesApplied_ = true;
                reapplyAudioDevices = true;
            }
            if (pairIt->second->localIsOfferer &&
                !pairIt->second->mediaSlotsPrepared &&
                !pairIt->second->mediaSlotsPreparing) {
                pairIt->second->mediaSlotsPreparing = true;
                prepareOffererMedia = true;
            }
            break;
        case SessionControllerState::kNegotiating:
            snapshotIt->state = RoomPeerConnectionState::kNegotiating;
            break;
        case SessionControllerState::kDisconnected:
            snapshotIt->state = RoomPeerConnectionState::kDisconnected;
            if (snapshot_.room.screenSharerDeviceId == snapshot_.localDeviceId &&
                snapshot_.room.activeControllerDeviceId ==
                    pairIt->second->peerDeviceId) {
                releaseInputSink = remoteInputSink_;
            }
            break;
        case SessionControllerState::kWaitingForSignaling:
        case SessionControllerState::kRestartingIce:
            snapshotIt->state = RoomPeerConnectionState::kRecovering;
            if (snapshot_.room.screenSharerDeviceId == snapshot_.localDeviceId &&
                snapshot_.room.activeControllerDeviceId ==
                    pairIt->second->peerDeviceId) {
                releaseInputSink = remoteInputSink_;
            }
            break;
        case SessionControllerState::kConnected:
            if (snapshotIt->state !=
                    RoomPeerConnectionState::kActive &&
                snapshot_.room.screenSharerDeviceId ==
                    snapshot_.localDeviceId) {
                sourceToRefresh = screenShare_.CaptureSource();
            }
            snapshotIt->state = RoomPeerConnectionState::kActive;
            snapshotIt->errorCode.clear();
            snapshotIt->errorMessage.clear();
            synchronizeMicrophoneCapture = true;
            microphoneCaptureEnabled =
                snapshot_.media.localMicrophone ==
                LocalMicrophoneState::kPublishing;
            break;
        case SessionControllerState::kFailed:
            snapshotIt->state = RoomPeerConnectionState::kFailed;
            snapshotIt->errorCode = controllerSnapshot.errorCode;
            snapshotIt->errorMessage = controllerSnapshot.errorMessage;
            break;
        case SessionControllerState::kClosed:
            snapshotIt->state = RoomPeerConnectionState::kClosed;
            break;
        default:
            break;
        }
    }

    if (releaseInputSink) {
        releaseInputSink->ReleaseAllRemoteInputs();
    }
    if (sourceToRefresh) {
        sourceToRefresh->RequestRefreshFrame();
    }
    if (reapplyAudioDevices) {
        (void)runtime_->ReapplyPreferredAudioDevices();
        const auto mediaState = Snapshot();
        const bool microphoneShouldRecord =
            mediaState.media.localMicrophone ==
                LocalMicrophoneState::kStarting ||
            mediaState.media.localMicrophone ==
                LocalMicrophoneState::kPublishing;
        // Reapplying a Windows endpoint preserves the ADM's previous running
        // state. Restore the product-level microphone state immediately so
        // selecting a device or preparing a new pair cannot implicitly open
        // the microphone.
        (void)runtime_->SetRecordingEnabled(
            microphoneShouldRecord);
        (void)RefreshLocalMediaDevices();
    }
    if (synchronizeMicrophoneCapture) {
        std::string detachError;
        if (!microphoneCaptureEnabled && pair && pair->session &&
            pair->session->AudioSlotPrepared(
                kMicrophoneMainAudioSlot)) {
            const auto result = pair->session->SetAudioSlotTrack(
                kMicrophoneMainAudioSlot, nullptr);
            if (!result.ok()) {
                detachError = std::string(result.message());
            }
        }
        // A negotiated sendrecv audio transceiver can start the ADM even
        // while no local microphone track is published. Enforce the product
        // state after negotiation, and independently clear the RTP sender
        // above, so UI state, transmitted audio and the physical device all
        // describe the same state.
        const auto result = runtime_->SetRecordingEnabled(
            microphoneCaptureEnabled);
        if (!detachError.empty() || !result.succeeded) {
            std::lock_guard lock(mutex_);
            if ((snapshot_.media.localMicrophone ==
                     LocalMicrophoneState::kPublishing) ==
                microphoneCaptureEnabled) {
                if (!result.succeeded) {
                    snapshot_.media.localMicrophone =
                        LocalMicrophoneState::kFailed;
                    snapshot_.error.code = result.errorCode;
                    snapshot_.error.message = result.errorMessage;
                } else {
                    snapshot_.error.code =
                        "microphone_sender_detach_failed";
                    snapshot_.error.message =
                        detachError;
                }
            }
        }
    }

    if (prepareOffererMedia) {
        const auto error = PrepareRoomPairMedia(
            pairId, false, true);
        if (error) {
            if (pair && pair->controller) {
                pair->controller->Close();
            }
            PublishSnapshot();
            return;
        }

        bool startOffer = false;
        {
            std::lock_guard lock(mutex_);
            const auto found = roomPairs_.find(pairId);
            if (found != roomPairs_.end() &&
                found->second == pair &&
                pair->mediaSlotsPrepared &&
                !pair->offerNegotiationStarted) {
                pair->offerNegotiationStarted = true;
                startOffer = true;
            }
        }
        if (startOffer && pair && pair->controller) {
            static_cast<ControllerSessionController*>(
                pair->controller.get())
                ->Connect(DefaultRemoteControlDataChannels());
        }
    }
    PublishSnapshot();
}

std::optional<OperationError>
InProcessSessionEngine::PrepareRoomPairMedia(
    const std::string& pairId,
    bool bindNegotiatedSlots,
    bool preparationAlreadyClaimed)
{
    std::shared_ptr<RoomPairRuntime> pair;
    std::vector<std::pair<
        std::string,
        webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface>>>
        videoTracks;
    webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> audioTrack;
    webrtc::scoped_refptr<WindowsDesktopCaptureSource> desktopSource;
    std::optional<ScreenStreamPreferenceRequest> screenPreference;
    std::uint32_t screenFrameRate = 60;
    bool remoteAudioEnabled = true;

    {
        std::lock_guard lock(mutex_);
        const auto found = roomPairs_.find(pairId);
        if (found == roomPairs_.end() || !found->second->session ||
            found->second->localIsOfferer == bindNegotiatedSlots) {
            return OperationError{
                "room_pair_media_prepare_invalid",
                "The room pair is unavailable or has the wrong negotiation role."};
        }
        pair = found->second;
        if (pair->mediaSlotsPrepared) {
            return std::nullopt;
        }
        if (preparationAlreadyClaimed) {
            if (!pair->mediaSlotsPreparing) {
                return OperationError{
                    "room_pair_media_prepare_not_claimed",
                    "The room media preparation claim is no longer current."};
            }
        }
        else {
            if (pair->mediaSlotsPreparing) {
                return OperationError{
                    "room_pair_media_prepare_in_progress",
                    "The room media slots are already being prepared."};
            }
            pair->mediaSlotsPreparing = true;
        }

        videoTracks.reserve(kRoomVideoSlots.size());
        for (const char* slot : kRoomVideoSlots) {
            webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track;
            const auto published = mediaState_->localRoomVideoTracks.find(slot);
            if (published != mediaState_->localRoomVideoTracks.end()) {
                track = published->second;
            }
            else if (const auto idle = mediaState_->idleRoomVideoTracks.find(slot);
                     idle != mediaState_->idleRoomVideoTracks.end()) {
                track = idle->second;
            }
            if (!track) {
                pair->mediaSlotsPreparing = false;
                return OperationError{
                    "idle_video_track_unavailable",
                    "The room video placeholder is unavailable."};
            }
            videoTracks.emplace_back(slot, std::move(track));
        }
        audioTrack = mediaState_->localMicrophoneAudioTrack;
        desktopSource = screenShare_.CaptureSource();
        screenFrameRate = roomSession_.localScreenFrameRate_;
        if (const auto preference =
                roomSession_.screenStreamPreferences_.find(pairId);
            preference != roomSession_.screenStreamPreferences_.end()) {
            screenPreference = preference->second;
        }
        remoteAudioEnabled = !snapshot_.media.roomAudioPlaybackMuted;
    }

    const auto fail = [this, &pair, &pairId](
                          std::string code,
                          std::string message)
        -> std::optional<OperationError> {
        std::lock_guard lock(mutex_);
        const auto found = roomPairs_.find(pairId);
        if (found != roomPairs_.end() && found->second == pair) {
            pair->mediaSlotsPreparing = false;
            const auto snapshotIt = std::find_if(
                snapshot_.roomActivity.peerConnections.begin(),
                snapshot_.roomActivity.peerConnections.end(),
                [&pairId](const RoomPeerConnectionSnapshot& current) {
                    return current.pairId == pairId;
                });
            if (snapshotIt != snapshot_.roomActivity.peerConnections.end()) {
                snapshotIt->state = RoomPeerConnectionState::kFailed;
                snapshotIt->errorCode = code;
                snapshotIt->errorMessage = message;
            }
        }
        return OperationError{std::move(code), std::move(message)};
    };

    // Everything below crosses into libwebrtc. It intentionally runs without
    // engine mutex ownership so WebRTC worker callbacks can update the pair
    // state concurrently without deadlocking the Qt UI thread.
    if (bindNegotiatedSlots) {
        const std::vector<std::string> slots = {
            kScreenMainVideoSlot, kCameraMainVideoSlot};
        auto result =
            pair->session->BindNegotiatedVideoTransceiverSlots(slots);
        if (!result.ok()) {
            return fail("room_media_slot_bind_failed",
                        std::string(result.message()));
        }
        result = pair->session->BindNegotiatedAudioTransceiverSlot(
            kMicrophoneMainAudioSlot);
        if (!result.ok()) {
            return fail("room_audio_slot_bind_failed",
                        std::string(result.message()));
        }
    }
    else {
        auto result = pair->session->PrepareVideoTransceiverSlot(
            kScreenMainVideoSlot);
        if (result.ok()) {
            result = pair->session->PrepareVideoTransceiverSlot(
                kCameraMainVideoSlot);
        }
        if (result.ok()) {
            result = pair->session->PrepareAudioTransceiverSlot(
                kMicrophoneMainAudioSlot);
        }
        if (!result.ok()) {
            return fail("room_media_slot_prepare_failed",
                        std::string(result.message()));
        }
    }

    if (desktopSource) {
        const auto width = desktopSource->CapturedWidth();
        const auto height = desktopSource->CapturedHeight();
        if (width > 0 && height > 0) {
            auto result = pair->session->SetVideoSlotSendingActive(
                kScreenMainVideoSlot, false);
            if (result.ok()) {
                if (screenPreference) {
                    const auto effective = ScreenShareCoordinator::ResolvePolicy(
                        width, height, *screenPreference);
                    result = pair->session->SetVideoSlotEncodingPolicy(
                        kScreenMainVideoSlot,
                        effective.framesPerSecond,
                        effective.width,
                        effective.height);
                } else {
                    result = pair->session->SetVideoSlotEncodingPolicy(
                        kScreenMainVideoSlot,
                        screenFrameRate,
                        width,
                        height);
                }
            }
            if (!result.ok()) {
                return fail("room_screen_encoding_policy_failed",
                            std::string(result.message()));
            }
        }
    }
    for (const auto& [slot, track] : videoTracks) {
        const auto result = pair->session->SetVideoSlotTrack(slot, track);
        if (!result.ok()) {
            return fail("room_video_track_attach_failed",
                        std::string(result.message()));
        }
    }
    if (desktopSource) {
        const auto result = pair->session->SetVideoSlotSendingActive(
            kScreenMainVideoSlot, true);
        if (!result.ok()) {
            return fail("room_screen_sender_activate_failed",
                        std::string(result.message()));
        }
    } else {
        const auto result = pair->session->SetVideoSlotSendingActive(
            kScreenMainVideoSlot, false);
        if (!result.ok()) {
            return fail("room_screen_sender_deactivate_failed",
                        std::string(result.message()));
        }
    }
    const auto audioResult = pair->session->SetAudioSlotTrack(
        kMicrophoneMainAudioSlot, audioTrack);
    if (!audioResult.ok()) {
        return fail(
            audioTrack ? "room_audio_track_attach_failed"
                       : "room_audio_track_clear_failed",
            std::string(audioResult.message()));
    }
    {
        const auto mediaState = Snapshot();
        const bool microphoneShouldRecord =
            mediaState.media.localMicrophone ==
                LocalMicrophoneState::kStarting ||
            mediaState.media.localMicrophone ==
                LocalMicrophoneState::kPublishing;
        // Audio transceiver preparation may initialize the shared ADM. Keep
        // physical recording synchronized before negotiation can complete,
        // not only after the pair reaches kConnected.
        (void)runtime_->SetRecordingEnabled(
            microphoneShouldRecord);
    }
    pair->session->SetRemoteAudioSlotEnabled(
        kMicrophoneMainAudioSlot, remoteAudioEnabled);
    const auto preparedVideoSlots = static_cast<std::uint32_t>(
        pair->session->PreparedVideoSlotCount());

    {
        std::lock_guard lock(mutex_);
        const auto found = roomPairs_.find(pairId);
        if (found == roomPairs_.end() || found->second != pair) {
            return OperationError{
                "room_pair_retired_during_media_prepare",
                "The room pair closed while its media slots were being prepared."};
        }
        pair->mediaSlotsPreparing = false;
        pair->mediaSlotsPrepared = true;
        const auto snapshotIt = std::find_if(
            snapshot_.roomActivity.peerConnections.begin(),
            snapshot_.roomActivity.peerConnections.end(),
            [&pairId](const RoomPeerConnectionSnapshot& current) {
                return current.pairId == pairId;
            });
        if (snapshotIt != snapshot_.roomActivity.peerConnections.end()) {
            snapshotIt->preparedVideoSlotCount = preparedVideoSlots;
        }
    }
    return std::nullopt;
}

std::optional<OperationError>
InProcessSessionEngine::PrepareRoomPairAnswer(
    const std::string& pairId)
{
    return PrepareRoomPairMedia(pairId, true);
}

}  // namespace remote::app
