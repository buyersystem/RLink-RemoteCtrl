// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RoomSessionCoordinator.h"

#include <algorithm>
#include <limits>
#include <utility>

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

bool RoomOperationActive(const SessionEngineSnapshot& snapshot)
{
    return snapshot.room.membership != RoomMembershipState::kNone &&
        snapshot.room.membership != RoomMembershipState::kFailed;
}

}  // namespace

SessionCommandResult RoomSessionCoordinator::ValidateCapacity(
    std::uint32_t capacity) const
{
    if (capacity < kMinimumRoomMembers ||
        capacity > kProtocolMaximumRoomMembers) {
        return Failure("invalid_room_capacity",
                       "Room capacity must be between 2 and 5 members.");
    }
    return Success();
}

SessionCommandResult RoomSessionCoordinator::ValidateCreate(
    const SessionEngineSnapshot& snapshot,
    std::uint32_t capacity) const
{
    if (const auto result = ValidateCapacity(capacity); !result.accepted) {
        return result;
    }
    if (RoomOperationActive(snapshot)) {
        return Failure("room_already_active",
                       "The device already has a room operation in progress.");
    }
    return Success();
}

void RoomSessionCoordinator::ApplyCreateRequested(
    SessionEngineSnapshot* snapshot,
    std::uint32_t capacity) const
{
    if (!snapshot) {
        return;
    }
    snapshot->room.membership = RoomMembershipState::kCreating;
    snapshot->room.capacity = capacity;
    snapshot->room.errorCode.clear();
    snapshot->room.errorMessage.clear();
}

SessionCommandResult RoomSessionCoordinator::ValidateJoin(
    const SessionEngineSnapshot& snapshot,
    const std::string& roomId) const
{
    if (roomId.empty()) {
        return Failure("room_id_empty", "A room ID is required.");
    }
    if (RoomOperationActive(snapshot)) {
        return Failure("room_already_active",
                       "The device already has a room operation in progress.");
    }
    return Success();
}

void RoomSessionCoordinator::ApplyJoinRequested(
    SessionEngineSnapshot* snapshot,
    const std::string& roomId) const
{
    if (!snapshot) {
        return;
    }
    snapshot->room.membership = RoomMembershipState::kJoinPending;
    snapshot->room.roomId = roomId;
    snapshot->room.errorCode.clear();
    snapshot->room.errorMessage.clear();
}

SessionCommandResult RoomSessionCoordinator::ValidateAvailabilityQuery(
    const std::vector<std::string>& roomIds) const
{
    if (roomIds.empty() || roomIds.size() > 8) {
        return Failure("invalid_room_availability_query",
                       "Between 1 and 8 room IDs are required.");
    }
    return Success();
}

void RoomSessionCoordinator::ApplyAvailabilityQuery(
    SessionEngineSnapshot* snapshot,
    const std::vector<std::string>& roomIds) const
{
    if (!snapshot) {
        return;
    }
    snapshot->roomActivity.availabilities.clear();
    snapshot->roomActivity.availabilities.reserve(roomIds.size());
    for (const auto& roomId : roomIds) {
        snapshot->roomActivity.availabilities.push_back(
            {roomId, RoomAvailabilityState::kChecking});
    }
}

SessionCommandResult RoomSessionCoordinator::PrepareJoinResponse(
    const SessionEngineSnapshot& snapshot,
    const std::string& requestId,
    std::string* roomId) const
{
    if (requestId.empty()) {
        return Failure("room_join_request_id_empty",
                       "A room join request ID is required.");
    }
    if (!roomId) {
        return Failure("room_join_response_missing",
                       "A room join response target is required.");
    }
    const auto requestIt = std::find_if(
        snapshot.roomActivity.incomingJoinRequests.begin(),
        snapshot.roomActivity.incomingJoinRequests.end(),
        [&requestId](const RoomJoinRequest& request) {
            return request.requestId == requestId;
        });
    if (requestIt == snapshot.roomActivity.incomingJoinRequests.end() ||
        snapshot.room.membership != RoomMembershipState::kActive ||
        snapshot.room.ownerDeviceId != snapshot.localDeviceId) {
        return Failure("room_join_request_not_found",
                       "The room join request is no longer pending.");
    }
    *roomId = requestIt->roomId;
    return Success();
}

SessionCommandResult RoomSessionCoordinator::PrepareCapacityUpdate(
    const SessionEngineSnapshot& snapshot,
    std::uint32_t capacity,
    std::string* roomId) const
{
    if (const auto result = ValidateCapacity(capacity); !result.accepted) {
        return result;
    }
    if (!roomId) {
        return Failure("room_capacity_target_missing",
                       "A room capacity target is required.");
    }
    if (snapshot.room.membership != RoomMembershipState::kActive ||
        snapshot.room.ownerDeviceId != snapshot.localDeviceId) {
        return Failure("room_owner_required",
                       "Only the active room owner can change capacity.");
    }
    if (static_cast<std::size_t>(capacity) <
        snapshot.room.members.size()) {
        return Failure("room_capacity_below_occupancy",
                       "Room capacity cannot be smaller than current occupancy.");
    }
    *roomId = snapshot.room.roomId;
    return Success();
}

SessionCommandResult RoomSessionCoordinator::PrepareLeave(
    SessionEngineSnapshot* snapshot,
    std::string* roomId) const
{
    if (!snapshot || !roomId) {
        return Failure("room_leave_target_missing",
                       "A room leave target is required.");
    }
    if (snapshot->room.membership != RoomMembershipState::kActive ||
        snapshot->room.roomId.empty()) {
        return Failure("room_not_active",
                       "The device is not an active room member.");
    }
    *roomId = snapshot->room.roomId;
    snapshot->room.membership = RoomMembershipState::kLeaving;
    return Success();
}

void RoomSessionCoordinator::ApplyLeaveFailed(
    SessionEngineSnapshot* snapshot,
    const std::string& errorCode,
    const std::string& errorMessage) const
{
    if (!snapshot) {
        return;
    }
    snapshot->room.membership = RoomMembershipState::kActive;
    snapshot->room.errorCode = errorCode;
    snapshot->room.errorMessage = errorMessage;
}

std::optional<std::uint64_t>
RoomSessionCoordinator::TakeNextScreenControlSequence()
{
    if (nextScreenControlSequence_ ==
        std::numeric_limits<std::uint64_t>::max()) {
        return std::nullopt;
    }
    return ++nextScreenControlSequence_;
}

void RoomSessionCoordinator::ResetActiveState()
{
    audioDevicesApplied_ = false;
    recoveryToken_.clear();
    screenShareGrantId_.clear();
    controlGrantId_.clear();
    controlGrantScreenSharerDeviceId_.clear();
    controlGrantControllerDeviceId_.clear();
    locallyRevokedControlGrantIds_.clear();
    nextInputSequence_ = 0;
    nextScreenControlSequence_ = 0;
    localScreenFrameRate_ = kDefaultScreenFrameRate;
    screenStreamPreferences_.clear();
    recoveryPending_ = false;
    leaveRequested_ = false;
}

}  // namespace remote::app
