// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "InProcessSessionEngine.h"

#include <algorithm>
#include <utility>

#include "InProcessSessionEngineInternal.h"
#include "src/platform/win/WindowsDesktopCaptureSource.h"

namespace remote::app {

void InProcessSessionEngine::OnRoomReady(
    const SignalingRoomReady& ready)
{
    std::vector<std::shared_ptr<RoomPairRuntime>> pairs;
    bool initialRoomActivation = false;
    {
        std::lock_guard lock(mutex_);
        if (ready.room.roomId.empty() || ready.recoveryToken.empty()) {
            return;
        }
        if (!snapshot_.room.roomId.empty() &&
            snapshot_.room.roomId != ready.room.roomId &&
            snapshot_.room.membership != RoomMembershipState::kCreating) {
            return;
        }
        initialRoomActivation =
            snapshot_.room.membership !=
                RoomMembershipState::kActive;
        snapshot_.room = ready.room;
        snapshot_.room.membership = RoomMembershipState::kActive;
        snapshot_.room.errorCode.clear();
        snapshot_.room.errorMessage.clear();
        std::erase_if(snapshot_.roomActivity.incomingJoinRequests,
                      [&ready](const RoomJoinRequest& request) {
                          return request.roomId != ready.room.roomId;
                      });
        std::erase_if(
            snapshot_.roomActivity.incomingScreenShareSwitchRequests,
            [&ready](const RoomScreenShareSwitchRequest& request) {
                return request.roomId != ready.room.roomId;
            });
        std::erase_if(snapshot_.roomActivity.incomingControlRequests,
                      [&ready](const RoomControlRequest& request) {
                          return request.roomId != ready.room.roomId;
                      });
        std::erase_if(
            snapshot_.roomActivity.incomingScreenShareViewRequests,
            [&ready](const RoomScreenShareViewRequest& request) {
                return request.roomId != ready.room.roomId;
            });
        std::erase_if(
            snapshot_.roomActivity.memberActionResults,
            [&ready](const RoomMemberActionResult& result) {
                return result.roomId != ready.room.roomId;
            });
        roomSession_.recoveryToken_ = ready.recoveryToken;
        roomSession_.recoveryPending_ = false;
        roomSession_.leaveRequested_ = false;
        for (const auto& [pairId, pair] : roomPairs_) {
            (void)pairId;
            pairs.push_back(pair);
        }
        if (snapshot_.error.code == "room_signaling_recovering" ||
            snapshot_.error.code == "room_resume_rejected") {
            snapshot_.error.code.clear();
            snapshot_.error.message.clear();
        }
    }
    for (const auto& pair : pairs) {
        if (pair && pair->controller) {
            pair->controller->SetSignalingAvailable(true);
        }
    }
    if (initialRoomActivation) {
        // Entering a room is microphone-off by definition. Run the complete
        // idempotent teardown even when the snapshot already says kOff so a
        // recording session or sender left alive by WebRTC negotiation cannot
        // disagree with the UI.
        (void)SetLocalMicrophoneEnabled(false);
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnRoomState(const RoomSnapshot& room)
{
    std::vector<std::shared_ptr<RoomPairRuntime>> removedPairs;
    bool stopLocalDesktopCapture = false;
    IRemoteInputSink* releaseInputSink = nullptr;
    webrtc::scoped_refptr<WindowsDesktopCaptureSource> sourceToRetune;
    std::uint32_t captureFrameRate = kDefaultScreenFrameRate;
    bool publishSharedDisplayLayout = false;
    {
        std::lock_guard lock(mutex_);
        if (room.roomId.empty() || snapshot_.room.roomId != room.roomId ||
            (snapshot_.room.membership != RoomMembershipState::kActive &&
             snapshot_.room.membership != RoomMembershipState::kLeaving)) {
            return;
        }
        const RoomMembershipState membership = snapshot_.room.membership;
        const bool localWasControlled =
            snapshot_.room.screenSharerDeviceId == snapshot_.localDeviceId &&
            !snapshot_.room.activeControllerDeviceId.empty() &&
            !roomSession_.controlGrantId_.empty();
        const std::string previousScreenSharer =
            snapshot_.room.screenSharerDeviceId;
        const std::uint64_t previousScreenShareEpoch =
            snapshot_.room.screenShareEpoch;
        const std::string previousController =
            snapshot_.room.activeControllerDeviceId;
        snapshot_.room = room;
        snapshot_.room.membership = membership;
        snapshot_.room.errorCode.clear();
        snapshot_.room.errorMessage.clear();
        if (previousScreenSharer != room.screenSharerDeviceId ||
            previousScreenShareEpoch != room.screenShareEpoch) {
            for (auto& peer : snapshot_.roomActivity.peerConnections) {
                peer.screenMaximumFrameRate = kMaximumScreenFrameRate;
                peer.screenCaptureCapabilityReported = false;
                peer.screenCaptureConfiguredBackend.clear();
                peer.screenCaptureActiveBackend.clear();
                peer.screenCaptureFallbackReason.clear();
                peer.sharedDisplayLayoutVersion = 0;
                peer.sharedDisplayId = 0;
                peer.sharedDisplayName.clear();
                peer.sharedDisplayStableKey.clear();
                peer.sharedDisplayWidth = 0;
                peer.sharedDisplayHeight = 0;
                peer.remoteDisplayCatalogLayoutVersion = 0;
                peer.remoteDisplayCatalogReported = false;
                peer.remoteDisplays.clear();
                peer.remoteDisplaySwitchPending = false;
                peer.remoteDisplaySwitchSequence = 0;
                peer.remoteDisplaySwitchError.clear();
            }
        }
        if (previousScreenShareEpoch != room.screenShareEpoch) {
            for (auto& peer : snapshot_.roomActivity.peerConnections) {
                peer.screenPreferencePending = false;
                peer.screenPreferenceSequence = 0;
                peer.screenPreferenceGeneration = 0;
                peer.screenWidth = 0;
                peer.screenHeight = 0;
                peer.screenFramesPerSecond = kDefaultScreenFrameRate;
                peer.screenMaxBitrateBps = 0;
                peer.screenScaleBackend = 0;
                if (peer.errorCode ==
                    "screen_stream_preference_rejected") {
                    peer.errorCode.clear();
                    peer.errorMessage.clear();
                }
            }
        }
        std::erase_if(snapshot_.roomActivity.incomingJoinRequests,
                      [&room](const RoomJoinRequest& request) {
                          return request.roomId != room.roomId;
                      });
        std::erase_if(
            snapshot_.roomActivity.incomingControlRequests,
            [this, &room](const RoomControlRequest& request) {
                return request.roomId != room.roomId ||
                       room.screenSharerDeviceId != snapshot_.localDeviceId ||
                       room.pendingControllerDeviceId !=
                           request.requesterDeviceId;
            });
        std::erase_if(
            snapshot_.roomActivity.incomingScreenShareViewRequests,
            [&room](const RoomScreenShareViewRequest& request) {
                return request.roomId != room.roomId ||
                       std::none_of(
                           room.members.begin(), room.members.end(),
                           [&request](
                               const RoomMemberSnapshot& member) {
                               return member.online &&
                                      member.deviceId ==
                                          request.requesterDeviceId;
                           });
            });
        if (room.screenSharerDeviceId != snapshot_.localDeviceId &&
            room.pendingScreenSharerDeviceId != snapshot_.localDeviceId) {
            roomSession_.screenShareGrantId_.clear();
            stopLocalDesktopCapture =
                screenShare_.HasCaptureSource();
        }
        const bool controlGrantConflictsWithRoom =
            !roomSession_.controlGrantId_.empty() &&
            ((!room.screenSharerDeviceId.empty() &&
              room.screenSharerDeviceId !=
                  roomSession_.controlGrantScreenSharerDeviceId_) ||
             (!room.activeControllerDeviceId.empty() &&
              room.activeControllerDeviceId !=
                  roomSession_.controlGrantControllerDeviceId_));
        if (controlGrantConflictsWithRoom) {
            roomSession_.controlGrantId_.clear();
            roomSession_.controlGrantScreenSharerDeviceId_.clear();
            roomSession_.controlGrantControllerDeviceId_.clear();
        }
        // Empty controller fields can occur in an intermediate room snapshot
        // between approval and the final broadcast. The authenticated grant
        // is authoritative for both participants: keeping only the grant ID
        // leaves the UI looking approved while SendRoomInput rejects every
        // event because activeControllerDeviceId was overwritten with empty.
        // Restore both participants until a conflicting lease or an explicit
        // revoke arrives.
        if (!roomSession_.controlGrantId_.empty()) {
            snapshot_.room.screenSharerDeviceId =
                roomSession_.controlGrantScreenSharerDeviceId_;
            snapshot_.room.activeControllerDeviceId =
                roomSession_.controlGrantControllerDeviceId_;
            snapshot_.room.pendingControllerDeviceId.clear();
        }
        snapshot_.roomControlGrantActive =
            !roomSession_.controlGrantId_.empty();
        if (localWasControlled &&
            (snapshot_.room.screenSharerDeviceId !=
                 snapshot_.localDeviceId ||
             snapshot_.room.activeControllerDeviceId !=
                 previousController)) {
            releaseInputSink = remoteInputSink_;
        }
        for (auto pairIt = roomPairs_.begin();
             pairIt != roomPairs_.end();) {
            const bool peerStillPresent = std::any_of(
                room.members.begin(), room.members.end(),
                [&pairIt](const RoomMemberSnapshot& member) {
                    return member.deviceId ==
                           pairIt->second->peerDeviceId;
                });
            if (peerStillPresent) {
                ++pairIt;
                continue;
            }
            const std::string pairId = pairIt->first;
            removedPairs.push_back(std::move(pairIt->second));
            pairIt = roomPairs_.erase(pairIt);
            roomSession_.screenStreamPreferences_.erase(pairId);
            std::erase_if(
                snapshot_.roomActivity.peerConnections,
                [&pairId](const RoomPeerConnectionSnapshot& current) {
                    return current.pairId == pairId;
                });
        }
        if (roomPairs_.empty()) {
            roomSession_.audioDevicesApplied_ = false;
        }
        if (screenShare_.HasCaptureSource() &&
            snapshot_.room.screenSharerDeviceId == snapshot_.localDeviceId) {
            publishSharedDisplayLayout =
                snapshot_.screenShare.activeDisplay.sessionDisplayId != 0 &&
                snapshot_.screenShare.activeDisplayLayoutVersion != 0;
            const std::size_t onlineMemberCount =
                static_cast<std::size_t>(std::count_if(
                    room.members.begin(), room.members.end(),
                    [](const RoomMemberSnapshot& member) {
                        return member.online;
                    }));
            const std::uint32_t roomFrameRateLimit =
                onlineMemberCount > kHighOccupancyRoomMemberThreshold
                ? kMultiMemberMaximumScreenFrameRate
                : kMaximumScreenFrameRate;
            for (auto& [requestPairId, preference] :
                 roomSession_.screenStreamPreferences_) {
                (void)requestPairId;
                preference.framesPerSecond = (std::min)(
                    preference.framesPerSecond, roomFrameRateLimit);
            }
            if (!roomSession_.screenStreamPreferences_.empty()) {
                captureFrameRate = kMinimumScreenFrameRate;
            }
            for (const auto& [requestPairId, preference] :
                 roomSession_.screenStreamPreferences_) {
                (void)requestPairId;
                captureFrameRate = (std::max)(
                    captureFrameRate, preference.framesPerSecond);
            }
            roomSession_.localScreenFrameRate_ = captureFrameRate;
            sourceToRetune = screenShare_.CaptureSource();
        }
    }
    if (stopLocalDesktopCapture) {
        StopLocalDesktopCapture();
    }
    if (releaseInputSink) {
        releaseInputSink->ReleaseAllRemoteInputs();
    }
    if (sourceToRetune) {
        sourceToRetune->SetTargetFrameRate(captureFrameRate);
    }
    for (auto& pair : removedPairs) {
        RetireRoomPair(std::move(pair));
    }
    PublishSnapshot();
    if (publishSharedDisplayLayout) {
        BroadcastSharedDisplayLayout();
        BroadcastSharedDisplayCatalog();
    }
}

void InProcessSessionEngine::OnRoomJoinPending(
    const SignalingRoomJoinPending& pending)
{
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership != RoomMembershipState::kJoinPending ||
            snapshot_.room.roomId != pending.roomId) {
            return;
        }
        snapshot_.room.errorCode.clear();
        snapshot_.room.errorMessage.clear();
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnRoomJoinRequested(
    const RoomJoinRequest& request)
{
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership != RoomMembershipState::kActive ||
            snapshot_.room.roomId != request.roomId ||
            snapshot_.room.ownerDeviceId != snapshot_.localDeviceId) {
            return;
        }
        const auto existing = std::find_if(
            snapshot_.roomActivity.incomingJoinRequests.begin(),
            snapshot_.roomActivity.incomingJoinRequests.end(),
            [&request](const RoomJoinRequest& current) {
                return current.requestId == request.requestId;
            });
        if (existing == snapshot_.roomActivity.incomingJoinRequests.end()) {
            snapshot_.roomActivity.incomingJoinRequests.push_back(request);
        }
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnRoomJoinResult(
    const SignalingRoomJoinResult& result)
{
    bool publish = false;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.roomId != result.roomId) {
            return;
        }
        const auto previousSize =
            snapshot_.roomActivity.incomingJoinRequests.size();
        std::erase_if(snapshot_.roomActivity.incomingJoinRequests,
                      [&result](const RoomJoinRequest& request) {
                          return request.requestId == result.requestId;
                      });
        publish = previousSize !=
                  snapshot_.roomActivity.incomingJoinRequests.size();
        if (snapshot_.room.membership !=
            RoomMembershipState::kJoinPending) {
            // Owners also receive the result so a timed-out/offline request
            // can be removed from their approval queue.
        } else if (result.accepted) {
            // room_ready carries the recovery token and authoritative state.
        } else {
            snapshot_.room.membership = RoomMembershipState::kFailed;
            snapshot_.room.errorCode = result.reasonCode.empty()
                                           ? "room_join_rejected"
                                           : result.reasonCode;
            snapshot_.room.errorMessage = result.reasonMessage.empty()
                                              ? "The room join request was rejected."
                                              : result.reasonMessage;
            roomSession_.recoveryToken_.clear();
            roomSession_.recoveryPending_ = false;
            publish = true;
        }
    }
    if (publish) {
        PublishSnapshot();
    }
}

void InProcessSessionEngine::OnRoomAvailabilityResult(
    const SignalingRoomAvailabilityResult& result)
{
    {
        std::lock_guard lock(mutex_);
        snapshot_.roomActivity.availabilities.clear();
        snapshot_.roomActivity.availabilities.reserve(result.rooms.size());
        for (const auto& room : result.rooms) {
            const RoomAvailabilityState state = !room.exists
                ? RoomAvailabilityState::kClosed
                : (room.joinable
                       ? RoomAvailabilityState::kAvailable
                       : RoomAvailabilityState::kTemporarilyUnavailable);
            snapshot_.roomActivity.availabilities.push_back({room.roomId, state});
        }
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnRoomClosed(
    const SignalingRoomClosed& closed)
{
    std::vector<std::shared_ptr<RoomPairRuntime>> roomPairs;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.roomId != closed.roomId) {
            return;
        }
        const bool locallyRequested = roomSession_.leaveRequested_ ||
            closed.initiatorDeviceId == snapshot_.localDeviceId;
        roomPairs.reserve(roomPairs_.size());
        for (auto& [pairId, pair] : roomPairs_) {
            (void)pairId;
            roomPairs.push_back(std::move(pair));
        }
        roomPairs_.clear();
        ResetRoomStateLocked();
        if (!locallyRequested) {
            snapshot_.error.code = closed.reasonCode;
            snapshot_.error.message =
                "The room was closed by its owner or expired during recovery.";
        }
    }
    for (auto& pair : roomPairs) {
        RetireRoomPair(std::move(pair));
    }
    PublishSnapshot();
}

}  // namespace remote::app
