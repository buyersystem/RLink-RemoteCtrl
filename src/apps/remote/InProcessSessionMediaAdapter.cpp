// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "InProcessSessionMediaAdapter.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "InProcessSessionEngine.h"
#include "InProcessSessionEngineInternal.h"
#include "InProcessSessionMediaState.h"
#include "RoomMediaSlots.h"
#include "VideoPipelinePreferenceNames.h"
#include "src/protocol/DataChannelCatalog.h"
#include "src/protocol/ScreenShareControlProtocol.h"
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

InProcessSessionMediaAdapter::InProcessSessionMediaAdapter(
    InProcessSessionEngine& engine)
    : engine_(&engine)
{}

SessionCommandResult InProcessSessionMediaAdapter::SetRoomVideoSource(
    const std::string& slot,
    webrtc::scoped_refptr<webrtc::VideoTrackSourceInterface> source,
    const std::string& trackId,
    std::optional<std::string> expectedRoomId,
    std::optional<std::uint64_t> expectedCameraGeneration)
{
    if (!IsRoomVideoSlot(slot)) {
        return Failure("room_video_slot_invalid",
                       "The requested room video slot is invalid.");
    }
    if (!source || trackId.empty()) {
        return Failure("room_video_source_invalid",
                       "A video source and track ID are required.");
    }
    const auto factory = engine_->runtime_
        ? engine_->runtime_->PeerConnectionFactory()
        : nullptr;
    if (!factory) {
        return Failure("webrtc_factory_unavailable",
                       "The WebRTC factory is not ready.");
    }
    auto track = factory->CreateVideoTrack(std::move(source), trackId);
    if (!track) {
        return Failure("room_video_track_create_failed",
                       "The room video track could not be created.");
    }

    std::vector<std::shared_ptr<InProcessSessionEngine::RoomPairRuntime>>
        candidatePairs;
    webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> fallback;
    {
        std::lock_guard lock(engine_->mutex_);
        if (engine_->snapshot_.state == SessionEngineState::kStopping ||
            engine_->snapshot_.state == SessionEngineState::kStopped) {
            return Failure("session_engine_stopping",
                           "The session engine is stopping.");
        }
        if (engine_->snapshot_.room.membership !=
            RoomMembershipState::kActive) {
            return Failure("room_not_active",
                           "Join a room before publishing room video.");
        }
        if ((expectedRoomId &&
             engine_->snapshot_.room.roomId != *expectedRoomId) ||
            (expectedCameraGeneration &&
             !engine_->localMedia_.IsCurrentCameraOperation(
                 *expectedCameraGeneration))) {
            return Failure(
                "camera_switch_superseded",
                "The camera switch no longer belongs to the active room.");
        }
        const auto previous =
            engine_->mediaState_->localRoomVideoTracks.find(slot);
        if (previous !=
            engine_->mediaState_->localRoomVideoTracks.end()) {
            fallback = previous->second;
        } else if (const auto idle =
                       engine_->mediaState_->idleRoomVideoTracks.find(slot);
                   idle !=
                       engine_->mediaState_->idleRoomVideoTracks.end()) {
            fallback = idle->second;
        }
        candidatePairs.reserve(engine_->roomPairs_.size());
        for (const auto& [pairId, pair] : engine_->roomPairs_) {
            (void)pairId;
            if (pair && pair->session && pair->mediaSlotsPrepared) {
                candidatePairs.push_back(pair);
            }
        }
    }

    // RtpSender::SetTrack may synchronously marshal work to WebRTC's
    // signaling thread. Do not hold the engine mutex while waiting for it.
    std::vector<std::shared_ptr<InProcessSessionEngine::RoomPairRuntime>>
        attachedPairs;
    for (const auto& pair : candidatePairs) {
        if (slot == kScreenMainVideoSlot) {
            const auto inactive = pair->session->SetVideoSlotSendingActive(
                slot, false);
            if (!inactive.ok()) {
                return Failure("room_video_sender_deactivate_failed",
                               std::string(inactive.message()));
            }
        }
        auto result = pair->session->SetVideoSlotTrack(slot, track);
        if (!result.ok()) {
            for (const auto& attached : attachedPairs) {
                (void)attached->session->SetVideoSlotTrack(slot, fallback);
            }
            return Failure("room_video_track_attach_failed",
                           std::string(result.message()));
        }
        attachedPairs.push_back(pair);
    }
    bool roomStillActive = false;
    {
        std::lock_guard lock(engine_->mutex_);
        roomStillActive =
            engine_->snapshot_.state != SessionEngineState::kStopping &&
            engine_->snapshot_.state != SessionEngineState::kStopped &&
            engine_->snapshot_.room.membership ==
                RoomMembershipState::kActive;
        if (roomStillActive && expectedRoomId) {
            roomStillActive =
                engine_->snapshot_.room.roomId == *expectedRoomId;
        }
        if (roomStillActive && expectedCameraGeneration) {
            roomStillActive =
                engine_->localMedia_.IsCurrentCameraOperation(
                    *expectedCameraGeneration);
        }
        if (roomStillActive) {
            engine_->mediaState_->localRoomVideoTracks[slot] =
                std::move(track);
        }
    }
    if (!roomStillActive) {
        for (const auto& attached : attachedPairs) {
            (void)attached->session->SetVideoSlotTrack(slot, fallback);
        }
        return Failure("room_not_active",
                       "The room closed while attaching room video.");
    }
    return Success();
}

SessionCommandResult InProcessSessionMediaAdapter::ClearRoomVideoSource(
    const std::string& slot)
{
    if (!IsRoomVideoSlot(slot)) {
        return Failure("room_video_slot_invalid",
                       "The requested room video slot is invalid.");
    }
    const bool detachScreenTrack = slot == kScreenMainVideoSlot;
    webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> idleTrack;
    webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> previousTrack;
    std::vector<std::shared_ptr<InProcessSessionEngine::RoomPairRuntime>>
        candidatePairs;
    {
        std::lock_guard lock(engine_->mutex_);
        if (!detachScreenTrack) {
            const auto idle =
                engine_->mediaState_->idleRoomVideoTracks.find(slot);
            if (idle ==
                    engine_->mediaState_->idleRoomVideoTracks.end() ||
                !idle->second) {
                return Failure(
                    "idle_video_track_unavailable",
                    "The room video placeholder is unavailable.");
            }
            idleTrack = idle->second;
        }
        if (const auto previous =
                engine_->mediaState_->localRoomVideoTracks.find(slot);
            previous !=
                engine_->mediaState_->localRoomVideoTracks.end()) {
            previousTrack = previous->second;
        }
        candidatePairs.reserve(engine_->roomPairs_.size());
        for (const auto& [pairId, pair] : engine_->roomPairs_) {
            (void)pairId;
            if (pair && pair->session && pair->mediaSlotsPrepared) {
                candidatePairs.push_back(pair);
            }
        }
    }
    for (const auto& pair : candidatePairs) {
        if (detachScreenTrack) {
            const auto inactive = pair->session->SetVideoSlotSendingActive(
                slot, false);
            if (!inactive.ok()) {
                return Failure("room_video_sender_deactivate_failed",
                               std::string(inactive.message()));
            }
        }
        auto result = pair->session->SetVideoSlotTrack(slot, idleTrack);
        if (!result.ok()) {
            return Failure("room_video_track_detach_failed",
                           std::string(result.message()));
        }
    }
    {
        std::lock_guard lock(engine_->mutex_);
        const auto current =
            engine_->mediaState_->localRoomVideoTracks.find(slot);
        if (current !=
                engine_->mediaState_->localRoomVideoTracks.end() &&
            current->second == previousTrack) {
            engine_->mediaState_->localRoomVideoTracks.erase(current);
        }
    }
    return Success();
}

SessionCommandResult
InProcessSessionMediaAdapter::SetRoomVideoSlotSendingActive(
    const std::string& slot,
    bool active)
{
    if (!IsRoomVideoSlot(slot)) {
        return Failure("room_video_slot_invalid",
                       "The requested room video slot is invalid.");
    }
    std::vector<std::shared_ptr<InProcessSessionEngine::RoomPairRuntime>>
        candidatePairs;
    {
        std::lock_guard lock(engine_->mutex_);
        candidatePairs.reserve(engine_->roomPairs_.size());
        for (const auto& [pairId, pair] : engine_->roomPairs_) {
            (void)pairId;
            if (pair && pair->session && pair->mediaSlotsPrepared) {
                candidatePairs.push_back(pair);
            }
        }
    }
    for (const auto& pair : candidatePairs) {
        const auto result = pair->session->SetVideoSlotSendingActive(
            slot, active);
        if (!result.ok()) {
            return Failure(active
                               ? "room_video_sender_activate_failed"
                               : "room_video_sender_deactivate_failed",
                           std::string(result.message()));
        }
    }
    return Success();
}

SessionCommandResult InProcessSessionMediaAdapter::SetRoomRemoteVideoSink(
    const std::string& pairId,
    const std::string& slot,
    webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink)
{
    if (!IsRoomVideoSlot(slot)) {
        return Failure("room_video_slot_invalid",
                       "The requested room video slot is invalid.");
    }
    std::shared_ptr<InProcessSessionEngine::RoomPairRuntime> pair;
    ScreenRefreshRequest refreshRequest;
    bool requestRemoteRefresh = false;
    std::uint64_t refreshSequence = 0;
    {
        std::lock_guard lock(engine_->mutex_);
        const auto found = engine_->roomPairs_.find(pairId);
        if (found == engine_->roomPairs_.end() ||
            !found->second->mediaSlotsPrepared ||
            !found->second->session) {
            return Failure("room_pair_not_ready",
                           "The requested room pair is not ready.");
        }
        pair = found->second;
        const auto channel = pair->openDataChannels.find(
            std::string(kControlReliableChannel));
        requestRemoteRefresh =
            sink && slot == kScreenMainVideoSlot &&
            engine_->snapshot_.room.membership ==
                RoomMembershipState::kActive &&
            engine_->snapshot_.room.screenShareState ==
                RoomScreenShareState::kActive &&
            engine_->snapshot_.room.screenShareEpoch != 0 &&
            engine_->snapshot_.room.screenSharerDeviceId ==
                pair->peerDeviceId &&
            channel != pair->openDataChannels.end() &&
            channel->second;
        if (requestRemoteRefresh) {
            const auto sequence =
                engine_->roomSession_.TakeNextScreenControlSequence();
            if (!sequence) {
                requestRemoteRefresh = false;
            } else {
                refreshSequence = *sequence;
            }
        }
        if (requestRemoteRefresh) {
            if (pair->screenStartupRefreshGeneration !=
                engine_->snapshot_.room.screenShareEpoch) {
                pair->screenStartupRefreshGeneration =
                    engine_->snapshot_.room.screenShareEpoch;
                pair->screenStartupRefreshRequests = 0;
                pair->screenFirstFramePresentedGeneration = 0;
                pair->screenFirstFrameStartupMs = 0;
            }
            ++pair->screenStartupRefreshRequests;
            refreshRequest.roomId = engine_->snapshot_.room.roomId;
            refreshRequest.senderDeviceId =
                engine_->snapshot_.localDeviceId;
            refreshRequest.sequence = refreshSequence;
            refreshRequest.screenShareGeneration =
                engine_->snapshot_.room.screenShareEpoch;
            const auto snapshotIt = std::find_if(
                engine_->snapshot_.roomActivity.peerConnections.begin(),
                engine_->snapshot_.roomActivity.peerConnections.end(),
                [&pairId](const RoomPeerConnectionSnapshot& current) {
                    return current.pairId == pairId;
                });
            if (snapshotIt !=
                engine_->snapshot_.roomActivity.peerConnections.end()) {
                snapshotIt->screenStartupRefreshRequests =
                    pair->screenStartupRefreshRequests;
                snapshotIt->screenFirstFramePresentedGeneration = 0;
                snapshotIt->screenFirstFrameStartupMs = 0;
            }
        }
    }

    // AddSink/RemoveSink enters libwebrtc and may wait for its worker thread.
    // Never hold the engine mutex across that boundary: the worker can publish
    // a pair snapshot at the same time, and the Qt UI is allowed to bind a
    // sink while negotiation is converging.
    pair->session->SetRemoteVideoSlotSink(slot, sink);
    if (requestRemoteRefresh) {
        std::vector<std::uint8_t> encoded;
        if (EncodeScreenRefreshRequest(refreshRequest, &encoded)) {
            (void)pair->controller->SendData(
                std::string(kControlReliableChannel), encoded, true);
        }
    }
    return Success();
}

SessionCommandResult InProcessSessionMediaAdapter::SetDirectRemoteVideoSink(
    webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink)
{
    LibWebRtcSession* session = nullptr;
    {
        std::lock_guard lock(engine_->mutex_);
        if (!engine_->webRtcSession_ ||
            !engine_->directSession_.mediaSlotsPrepared ||
            engine_->snapshot_.purpose != SessionPurpose::kRemoteControl) {
            return Failure("direct_media_not_ready",
                           "The direct desktop video is not ready.");
        }
        session = engine_->webRtcSession_.get();
    }
    session->SetRemoteVideoSlotSink(kScreenMainVideoSlot, sink);
    if (sink) {
        engine_->RequestDirectSharedDisplayLayout();
    }
    return Success();
}

SessionCommandResult
InProcessSessionMediaAdapter::NotifyRoomScreenFirstFramePresented(
    const std::string& pairId,
    std::uint64_t screenShareGeneration,
    std::uint32_t startupElapsedMs)
{
    std::shared_ptr<InProcessSessionEngine::RoomPairRuntime> pair;
    std::shared_ptr<InProcessSessionEngine::RoomPairRuntime>
        clipboardWarmupPair;
    ScreenFirstFramePresented presented;
    {
        std::lock_guard lock(engine_->mutex_);
        const auto pairIt = engine_->roomPairs_.find(pairId);
        if (pairIt == engine_->roomPairs_.end() || !pairIt->second ||
            !pairIt->second->controller ||
            engine_->snapshot_.room.membership !=
                RoomMembershipState::kActive ||
            engine_->snapshot_.room.screenShareState !=
                RoomScreenShareState::kActive ||
            engine_->snapshot_.room.screenShareEpoch !=
                screenShareGeneration ||
            engine_->snapshot_.room.screenSharerDeviceId !=
                pairIt->second->peerDeviceId ||
            !pairIt->second->openDataChannels[
                std::string(kControlReliableChannel)]) {
            return Failure(
                "screen_first_present_pair_unavailable",
                "The active screen-share pair is unavailable.");
        }
        const auto sequence =
            engine_->roomSession_.TakeNextScreenControlSequence();
        if (!sequence) {
            return Failure("screen_first_present_sequence_exhausted",
                           "The screen control sequence is exhausted.");
        }
        pair = pairIt->second;
        pair->screenFirstFramePresentedGeneration = screenShareGeneration;
        pair->screenFirstFrameStartupMs = startupElapsedMs;
        const auto snapshotIt = std::find_if(
            engine_->snapshot_.roomActivity.peerConnections.begin(),
            engine_->snapshot_.roomActivity.peerConnections.end(),
            [&pairId](const RoomPeerConnectionSnapshot& current) {
                return current.pairId == pairId;
            });
        if (snapshotIt !=
            engine_->snapshot_.roomActivity.peerConnections.end()) {
            snapshotIt->screenFirstFramePresentedGeneration =
                screenShareGeneration;
            snapshotIt->screenFirstFrameStartupMs = startupElapsedMs;
            snapshotIt->screenStartupRefreshRequests =
                pair->screenStartupRefreshRequests;
        }
        if (pair->clipboardWarmupPending &&
            !pair->clipboardTransferPrimed &&
            !pair->clipboardWarmupRunning) {
            pair->clipboardWarmupPending = false;
            pair->clipboardWarmupRunning = true;
            clipboardWarmupPair = pair;
        }
        presented.roomId = engine_->snapshot_.room.roomId;
        presented.senderDeviceId = engine_->snapshot_.localDeviceId;
        presented.sequence = *sequence;
        presented.screenShareGeneration = screenShareGeneration;
        presented.startupElapsedMs = startupElapsedMs;
    }

    std::vector<std::uint8_t> encoded;
    std::string encodeError;
    if (!EncodeScreenFirstFramePresented(
            presented, &encoded, &encodeError)) {
        return Failure("screen_first_present_encode_failed", encodeError);
    }
    const SendResult sendResult = pair->controller->SendData(
        std::string(kControlReliableChannel), encoded, true);
    if (clipboardWarmupPair) {
        engine_->StartClipboardWarmup(clipboardWarmupPair);
    }
    engine_->PublishSnapshot();
    return sendResult == SendResult::kSent
        ? Success()
        : Failure(
              "screen_first_present_send_failed",
              "The first-frame presentation acknowledgement could not be sent.");
}

void InProcessSessionMediaAdapter::SetLocalCameraPreviewSink(
    webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink)
{
    webrtc::scoped_refptr<webrtc::VideoTrackInterface> track;
    webrtc::VideoSinkInterface<webrtc::VideoFrame>* previous = nullptr;
    {
        std::lock_guard lock(engine_->mutex_);
        track = engine_->mediaState_->localCameraPreviewTrack;
        previous = engine_->mediaState_->localCameraPreviewSink;
        engine_->mediaState_->localCameraPreviewSink = sink;
    }
    if (track && previous && previous != sink) {
        track->RemoveSink(previous);
    }
    if (track && sink && previous != sink) {
        track->AddOrUpdateSink(sink, webrtc::VideoSinkWants());
    }
}

void InProcessSessionMediaAdapter::SetRemoteInputSink(IRemoteInputSink* sink)
{
    std::lock_guard lock(engine_->mutex_);
    engine_->remoteInputSink_ = sink;
}

void InProcessSessionMediaAdapter::SetRemoteFileTransferSink(
    IFileTransferSink* sink)
{
    std::lock_guard lock(engine_->mutex_);
    engine_->remoteFileTransferSink_ = sink;
}

void InProcessSessionMediaAdapter::SetRemoteClipboardSink(IClipboardSink* sink)
{
    std::lock_guard lock(engine_->mutex_);
    engine_->remoteClipboardSink_ = sink;
}

void InProcessSessionMediaAdapter::SetRemoteCursorCallback(
    RemoteCursorCallback callback)
{
    std::lock_guard lock(engine_->mutex_);
    engine_->remoteCursorCallback_ = std::move(callback);
}

SessionCommandResult InProcessSessionMediaAdapter::SendRemoteInput(
    const RemoteInputEvent& event)
{
    return engine_->SendRemoteInput(event);
}

SessionCommandResult InProcessSessionMediaAdapter::SendRemoteFileMessage(
    const std::string& peerDeviceId,
    const FileTransferMessage& message)
{
    return engine_->SendRemoteFileMessage(peerDeviceId, message);
}

SessionCommandResult InProcessSessionMediaAdapter::SendRemoteClipboardMessage(
    const std::string& peerDeviceId,
    const std::string& clipboardSessionId,
    const ClipboardMessage& message)
{
    return engine_->SendRemoteClipboardMessage(
        peerDeviceId, clipboardSessionId, message);
}

SessionCommandResult
InProcessSessionMediaAdapter::SetRemoteAudioPlaybackMuted(bool muted)
{
    return engine_->SetRemoteAudioPlaybackMuted(muted);
}

SessionCommandResult
InProcessSessionMediaAdapter::SetDirectScreenStreamPreference(
    const ScreenStreamPreferenceRequest& preference)
{
    return engine_->SetDirectScreenStreamPreference(preference);
}

SessionCommandResult
InProcessSessionMediaAdapter::QueueDirectScreenStreamPreference(
    const ScreenStreamPreferenceRequest& preference,
    std::function<void(SessionCommandResult)> completion)
{
    return engine_->QueueDirectScreenStreamPreference(
        preference, std::move(completion));
}

SessionCommandResult
InProcessSessionMediaAdapter::RequestDirectSharedDisplaySwitch(
    const std::string& stableDisplayKey)
{
    return engine_->RequestDirectSharedDisplaySwitch(stableDisplayKey);
}

void InProcessSessionMediaAdapter::SetPreferredHardwareDecoderName(
    std::string name)
{
    if (engine_->runtime_) {
        engine_->runtime_->SetPreferredHardwareDecoderName(std::move(name));
    }
}

SessionCommandResult
InProcessSessionMediaAdapter::ApplyVideoPipelinePreferences(
    DesktopCaptureImplementation desktopCaptureImplementation,
    VideoEncoderPreference videoEncoderPreference,
    FfmpegX264Preset quality,
    FfmpegHardwareBackend ffmpegHardwareBackend,
    std::string preferredAutomaticEncoderId,
    VideoDecoderPreference videoDecoderPreference)
{
    std::string runtimeError;
    std::vector<std::shared_ptr<InProcessSessionEngine::RoomPairRuntime>>
        roomPairs;
    {
        std::lock_guard lock(engine_->mutex_);
        const bool roomCameraActive =
            engine_->snapshot_.media.localCamera !=
                LocalCameraState::kOff ||
            std::any_of(
                engine_->snapshot_.room.members.begin(),
                engine_->snapshot_.room.members.end(),
                [](const RoomMemberSnapshot& member) {
                    return member.cameraPublishing;
                });
        const bool directMediaActive =
            engine_->snapshot_.state == SessionEngineState::kStarting ||
            engine_->snapshot_.state == SessionEngineState::kConnecting ||
            engine_->snapshot_.state ==
                SessionEngineState::kAwaitingLocalApproval ||
            engine_->snapshot_.state == SessionEngineState::kActive ||
            engine_->snapshot_.state == SessionEngineState::kStopping;
        if (engine_->snapshot_.room.screenShareState !=
                RoomScreenShareState::kIdle ||
            roomCameraActive || directMediaActive) {
            return Failure(
                "video_pipeline_settings_busy",
                "Stop the active video share before changing capture or codec settings.");
        }
        if (!engine_->runtime_ ||
            !engine_->runtime_->ApplyVideoCodecPreferences(
                videoEncoderPreference, quality, ffmpegHardwareBackend,
                preferredAutomaticEncoderId, videoDecoderPreference,
                &runtimeError)) {
            return Failure(
                "video_pipeline_settings_apply_failed",
                runtimeError.empty()
                    ? "The selected video codec settings could not be applied."
                    : std::move(runtimeError));
        }

        engine_->options_.desktopCaptureImplementation =
            desktopCaptureImplementation;
        engine_->options_.videoEncoderPreference = videoEncoderPreference;
        engine_->options_.ffmpegX264Preset = quality;
        engine_->options_.ffmpegHardwareBackend = ffmpegHardwareBackend;
        engine_->options_.preferredAutomaticEncoderId =
            std::move(preferredAutomaticEncoderId);
        engine_->options_.videoDecoderPreference = videoDecoderPreference;
        const auto& report = engine_->runtime_->CapabilityReport();
        engine_->capabilities_.hasH264Encoder = report.hasH264Encoder;
        engine_->capabilities_.hasH264Decoder = report.hasH264Decoder;
        engine_->capabilities_.h264HardwareEncoderWired =
            report.h264HardwareEncoderWired;
        engine_->capabilities_.h264SoftwareEncoderWired =
            report.h264SoftwareEncoderWired;
        engine_->capabilities_.ffmpegX264EncoderWired =
            report.h264FfmpegX264EncoderWired;
        engine_->capabilities_.ffmpegX264EncoderError =
            report.h264FfmpegX264EncoderError;
        engine_->capabilities_.ffmpegHardwareEncoderWired =
            report.h264FfmpegHardwareEncoderWired;
        engine_->capabilities_.h264SoftwareEncoderFallback =
            report.h264SoftwareEncoderFallbackWired;
        engine_->capabilities_.videoEncoderPreference =
            EncoderPreferenceName(report.videoEncoderPreference);
        engine_->capabilities_.videoDecoderPreference =
            DecoderPreferenceName(report.videoDecoderPreference);
        engine_->capabilities_.desktopCapturePreference =
            DesktopCaptureImplementationName(
                engine_->options_.desktopCaptureImplementation);
        roomPairs.reserve(engine_->roomPairs_.size());
        for (const auto& [pairId, pair] : engine_->roomPairs_) {
            (void)pairId;
            if (pair && pair->session) {
                roomPairs.push_back(pair);
            }
        }
    }
    const bool fastDesktopBweStartup =
        desktopCaptureImplementation ==
        DesktopCaptureImplementation::kLibWebRtc;
    for (const auto& pair : roomPairs) {
        pair->session->SetFastDesktopBweStartupEnabled(
            fastDesktopBweStartup);
        pair->session->SetAdaptiveDesktopNetworkFrameRateEnabled(true);
    }
    engine_->PublishSnapshot();
    return Success();
}

}  // namespace remote::app
