// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "InProcessSessionEngine.h"
#include "InProcessSessionMediaState.h"
#include "InProcessSessionMediaAdapter.h"

#include <thread>
#include <utility>

#include "InProcessSessionEngineInternal.h"
#include "src/platform/win/WindowsCameraCaptureSource.h"
#include "src/platform/win/WindowsDesktopCaptureSource.h"
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

SessionCommandResult InProcessSessionEngine::RequireReady(
    const char* operation) const
{
    const auto snapshot = Snapshot();
    if (snapshot.state == SessionEngineState::kReady) {
        return Success();
    }
    return Failure("engine_not_ready",
                   std::string("Cannot ") + operation +
                       " while the session engine is not ready.");
}

bool InProcessSessionEngine::SignalingIsOnline() const
{
    std::lock_guard lock(mutex_);
    return snapshot_.connectivity == SessionConnectivityState::kOnline;
}

void InProcessSessionEngine::DisposeClosedSession()
{
    std::unique_ptr<SessionControllerBase> controller;
    std::unique_ptr<LibWebRtcSession> session;
    {
        std::lock_guard lock(mutex_);
        if (!sessionController_ ||
            sessionController_->Snapshot().state !=
                SessionControllerState::kClosed) {
            return;
        }
        controller = std::move(sessionController_);
        session = std::move(webRtcSession_);
    }
    controller->SetObserver(nullptr);
    controller.reset();
    session.reset();
    {
        std::lock_guard lock(mutex_);
        directSession_.audioDevicesApplied = false;
    }
}

void InProcessSessionEngine::ResetSessionStateLocked()
{
    snapshot_.state = SessionEngineState::kReady;
    snapshot_.purpose = SessionPurpose::kNone;
    snapshot_.origin = SessionOrigin::kNone;
    snapshot_.remoteControlRole = RemoteControlRole::kNone;
    snapshot_.media.localCamera = LocalCameraState::kOff;
    snapshot_.media.remoteCameraPublishing = false;
    snapshot_.sessionId.clear();
    snapshot_.peerDeviceId.clear();
    snapshot_.direct.signalingSessionReady = false;
    snapshot_.direct.mediaSlotsPrepared = false;
    snapshot_.direct.controlReliableChannelOpen = false;
    snapshot_.direct.inputFastChannelOpen = false;
    snapshot_.direct.fileTransferChannelOpen = false;
    snapshot_.direct.clipboardReliableChannelOpen = false;
    snapshot_.direct.clipboardTransferChannelOpen = false;
    snapshot_.direct.sessionEverActive = false;
    snapshot_.direct.iceRestartAttempt = 0;
    snapshot_.direct.remoteDisplay = {};
    snapshot_.direct.remoteDisplayLayoutVersion = 0;
    snapshot_.direct.remoteScreenShareGeneration = 0;
    snapshot_.direct.screenPreferencePending = false;
    snapshot_.direct.screenPreferenceSequence = 0;
    snapshot_.direct.screenWidth = 0;
    snapshot_.direct.screenHeight = 0;
    snapshot_.direct.screenFramesPerSecond = kDefaultScreenFrameRate;
    snapshot_.direct.screenMaximumFrameRate = kMaximumScreenFrameRate;
    snapshot_.direct.screenMaxBitrateBps = 0;
    snapshot_.direct.remoteDisplayCatalogReported = false;
    snapshot_.direct.remoteDisplayCatalogLayoutVersion = 0;
    snapshot_.direct.remoteDisplays.clear();
    snapshot_.direct.remoteDisplaySwitchPending = false;
    snapshot_.direct.remoteDisplaySwitchSequence = 0;
    snapshot_.direct.remoteDisplaySwitchError.clear();
    snapshot_.error.code.clear();
    snapshot_.error.message.clear();
    directSession_.Reset();
}

void InProcessSessionEngine::ResetRoomStateLocked()
{
    snapshot_.room = {};
    snapshot_.roomActivity.incomingJoinRequests.clear();
    snapshot_.roomActivity.incomingScreenShareSwitchRequests.clear();
    snapshot_.roomActivity.incomingControlRequests.clear();
    snapshot_.roomActivity.incomingScreenShareViewRequests.clear();
    snapshot_.roomActivity.memberActionResults.clear();
    snapshot_.roomActivity.outgoingScreenShareSwitchRequestId.clear();
    snapshot_.roomActivity.peerConnections.clear();
    snapshot_.screenShare.activeDisplay = {};
    snapshot_.screenShare.activeDisplayLayoutVersion = 0;
    const auto shareGeneration = screenShare_.BeginShare();
    snapshot_.screenShare.generation = shareGeneration;
    if (runtime_) {
        runtime_->SetDesktopShareGeneration(shareGeneration);
        runtime_->SetDesktopCaptureAdapterLuid(0);
    }
    snapshot_.roomControlGrantActive = false;
    (void)localMedia_.BeginCameraOperation();
    roomSession_.ResetActiveState();
    mediaState_->localRoomVideoTracks.clear();
    if (auto capture = screenShare_.TakeCaptureSource()) {
        std::jthread stopThread(
            [capture = std::move(capture)](std::stop_token) {
                capture->StopCapture();
            });
        retiredDesktopStopThreads_.push_back(std::move(stopThread));
    }
    if (auto cameraCapture = localMedia_.TakeCameraCaptureSource()) {
        if (mediaState_->localCameraPreviewTrack && mediaState_->localCameraPreviewSink) {
            mediaState_->localCameraPreviewTrack->RemoveSink(mediaState_->localCameraPreviewSink);
        }
        cameraCapture->StopCapture();
    }
    mediaState_->localCameraPreviewTrack = nullptr;
    if (mediaState_->localMicrophoneAudioTrack) {
        mediaState_->localMicrophoneAudioTrack->set_enabled(false);
    }
    mediaState_->localMicrophoneAudioTrack = nullptr;
    mediaState_->localMicrophoneAudioSource = nullptr;
    snapshot_.media.localCamera = LocalCameraState::kOff;
    snapshot_.media.localMediaDevices.camera.activeDeviceId.clear();
    if (snapshot_.media.localMediaDevices.camera.state ==
        MediaDeviceSelectionState::kSwitching) {
        snapshot_.media.localMediaDevices.camera.state =
            MediaDeviceSelectionState::kReady;
        snapshot_.media.localMediaDevices.camera.errorCode.clear();
        snapshot_.media.localMediaDevices.camera.errorMessage.clear();
        ++snapshot_.media.localMediaDevices.revision;
    }
    snapshot_.media.localMicrophone = LocalMicrophoneState::kOff;
    snapshot_.media.roomAudioPlaybackMuted = false;
}

void InProcessSessionEngine::StopLocalDesktopCapture()
{
    StopRemoteCursorPublishing();
    webrtc::scoped_refptr<WindowsDesktopCaptureSource> source;
    {
        std::lock_guard lock(mutex_);
        const auto shareGeneration = screenShare_.BeginShare();
        snapshot_.screenShare.generation = shareGeneration;
        if (runtime_) {
            runtime_->SetDesktopShareGeneration(shareGeneration);
            runtime_->SetDesktopCaptureAdapterLuid(0);
        }
        source = screenShare_.TakeCaptureSource();
        snapshot_.screenShare.activeDisplay = {};
        snapshot_.screenShare.activeDisplayLayoutVersion = 0;
    }

    (void)mediaAccess_->ClearRoomVideoSource(kScreenMainVideoSlot);
    if (!source) {
        return;
    }
    // OnFrame feeds WebRTC synchronously. If an encoder or driver is busy,
    // joining that capture thread here freezes the Qt/signaling caller. The
    // track has already been replaced with the idle source, so physical
    // capture teardown can safely finish in the background.
    std::jthread stopThread(
        [capture = std::move(source)](std::stop_token) {
            capture->StopCapture();
        });
    std::lock_guard lock(mutex_);
    retiredDesktopStopThreads_.push_back(std::move(stopThread));
}

void InProcessSessionEngine::BroadcastSharedDisplayLayout()
{
    SharedDisplayLayout layout;
    std::vector<std::shared_ptr<RoomPairRuntime>> recipients;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership != RoomMembershipState::kActive ||
            snapshot_.room.screenSharerDeviceId !=
                snapshot_.localDeviceId ||
            snapshot_.screenShare.activeDisplay.sessionDisplayId == 0 ||
            snapshot_.screenShare.activeDisplayLayoutVersion == 0) {
            return;
        }
        layout.roomId = snapshot_.room.roomId;
        layout.senderDeviceId = snapshot_.localDeviceId;
        // Use the server-authoritative share epoch so every room member can
        // reject a late layout packet from an earlier lease.
        layout.screenShareGeneration =
            snapshot_.room.screenShareEpoch;
        layout.layoutVersion =
            snapshot_.screenShare.activeDisplayLayoutVersion;
        layout.selectedDisplay = snapshot_.screenShare.activeDisplay;
        for (const auto& [pairId, pair] : roomPairs_) {
            (void)pairId;
            const auto channel = pair->openDataChannels.find(
                std::string(kControlReliableChannel));
            if (pair->controller &&
                channel != pair->openDataChannels.end() &&
                channel->second) {
                recipients.push_back(pair);
            }
        }
    }

    std::vector<std::uint8_t> encoded;
    if (!EncodeSharedDisplayLayout(layout, &encoded)) {
        return;
    }
    for (const auto& pair : recipients) {
        (void)pair->controller->QueueData(
            std::string(kControlReliableChannel), encoded, true);
    }
}

void InProcessSessionEngine::BroadcastSharedDisplayCatalog()
{
    SharedDisplayCatalog catalog;
    std::vector<std::shared_ptr<RoomPairRuntime>> recipients;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership !=
                RoomMembershipState::kActive ||
            snapshot_.room.screenSharerDeviceId !=
                snapshot_.localDeviceId ||
            snapshot_.room.screenShareEpoch == 0 ||
            snapshot_.screenShare.topology.layoutVersion == 0 ||
            snapshot_.screenShare.topology.displays.empty()) {
            return;
        }
        catalog.roomId = snapshot_.room.roomId;
        catalog.senderDeviceId = snapshot_.localDeviceId;
        catalog.screenShareGeneration =
            snapshot_.room.screenShareEpoch;
        catalog.layoutVersion =
            snapshot_.screenShare.topology.layoutVersion;
        catalog.displays =
            snapshot_.screenShare.topology.displays;
        for (const auto& [pairId, pair] : roomPairs_) {
            (void)pairId;
            const auto channel = pair->openDataChannels.find(
                std::string(kControlReliableChannel));
            if (pair->controller &&
                channel != pair->openDataChannels.end() &&
                channel->second) {
                recipients.push_back(pair);
            }
        }
    }
    std::vector<std::uint8_t> encoded;
    if (!EncodeSharedDisplayCatalog(catalog, &encoded)) {
        return;
    }
    for (const auto& pair : recipients) {
        (void)pair->controller->QueueData(
            std::string(kControlReliableChannel), encoded, true);
    }
}

}  // namespace remote::app
