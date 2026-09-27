// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "InProcessSessionEngine.h"

#include <algorithm>
#include <utility>

#include "InProcessSessionEngineInternal.h"

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

SessionCommandResult InProcessSessionEngine::CreateRoom(
    std::uint32_t capacity)
{
    if (auto ready = RequireReady("create a room"); !ready.accepted) {
        return ready;
    }
    if (const auto validation = roomSession_.ValidateCapacity(capacity);
        !validation.accepted) {
        return validation;
    }
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    {
        std::lock_guard lock(mutex_);
        if (const auto validation =
                roomSession_.ValidateCreate(snapshot_, capacity);
            !validation.accepted) {
            return validation;
        }
    }
    const auto result = signaling_->CreateRoom(capacity);
    if (!result.accepted) {
        return {false, result.errorCode, result.errorMessage};
    }
    {
        std::lock_guard lock(mutex_);
        ResetRoomStateLocked();
        roomSession_.ApplyCreateRequested(&snapshot_, capacity);
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult InProcessSessionEngine::JoinRoom(
    const std::string& roomId)
{
    if (auto ready = RequireReady("join a room"); !ready.accepted) {
        return ready;
    }
    if (roomId.empty()) {
        return Failure("room_id_empty", "A room ID is required.");
    }
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    {
        std::lock_guard lock(mutex_);
        if (const auto validation =
                roomSession_.ValidateJoin(snapshot_, roomId);
            !validation.accepted) {
            return validation;
        }
    }
    const auto result = signaling_->RequestRoomJoin(roomId);
    if (!result.accepted) {
        return {false, result.errorCode, result.errorMessage};
    }
    {
        std::lock_guard lock(mutex_);
        ResetRoomStateLocked();
        roomSession_.ApplyJoinRequested(&snapshot_, roomId);
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult InProcessSessionEngine::QueryRoomAvailability(
    const std::vector<std::string>& roomIds)
{
    if (const auto validation =
            roomSession_.ValidateAvailabilityQuery(roomIds);
        !validation.accepted) {
        return validation;
    }
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    const auto result = signaling_->QueryRoomAvailability(roomIds);
    if (!result.accepted) {
        return {false, result.errorCode, result.errorMessage};
    }
    {
        std::lock_guard lock(mutex_);
        roomSession_.ApplyAvailabilityQuery(&snapshot_, roomIds);
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult InProcessSessionEngine::RespondToRoomJoin(
    const std::string& requestId,
    bool accepted)
{
    if (requestId.empty()) {
        return Failure("room_join_request_id_empty",
                       "A room join request ID is required.");
    }
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    std::string roomId;
    {
        std::lock_guard lock(mutex_);
        if (const auto prepared = roomSession_.PrepareJoinResponse(
                snapshot_, requestId, &roomId);
            !prepared.accepted) {
            return prepared;
        }
    }
    const auto result = signaling_->RespondToRoomJoin(
        roomId, requestId, accepted,
        accepted ? std::string{} : std::string{"rejected_by_owner"});
    if (!result.accepted) {
        return {false, result.errorCode, result.errorMessage};
    }
    {
        std::lock_guard lock(mutex_);
        std::erase_if(snapshot_.roomActivity.incomingJoinRequests,
                      [&requestId](const RoomJoinRequest& request) {
                          return request.requestId == requestId;
                      });
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult InProcessSessionEngine::SetRoomCapacity(
    std::uint32_t capacity)
{
    if (const auto validation = roomSession_.ValidateCapacity(capacity);
        !validation.accepted) {
        return validation;
    }
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    std::string roomId;
    {
        std::lock_guard lock(mutex_);
        if (const auto prepared = roomSession_.PrepareCapacityUpdate(
                snapshot_, capacity, &roomId);
            !prepared.accepted) {
            return prepared;
        }
    }
    const auto result = signaling_->SetRoomCapacity(roomId, capacity);
    return result.accepted
               ? Success()
               : Failure(result.errorCode, result.errorMessage);
}

SessionCommandResult InProcessSessionEngine::LeaveRoom()
{
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    std::string roomId;
    {
        std::lock_guard lock(mutex_);
        if (const auto prepared =
                roomSession_.PrepareLeave(&snapshot_, &roomId);
            !prepared.accepted) {
            return prepared;
        }
        roomSession_.leaveRequested_ = true;
    }
    PublishSnapshot();
    const auto result = signaling_->LeaveRoom(roomId,
                                               "left_by_local_user");
    if (!result.accepted) {
        {
            std::lock_guard lock(mutex_);
            roomSession_.leaveRequested_ = false;
            roomSession_.ApplyLeaveFailed(
                &snapshot_, result.errorCode, result.errorMessage);
        }
        PublishSnapshot();
        return Failure(result.errorCode, result.errorMessage);
    }
    return Success();
}

SessionCommandResult
InProcessSessionEngine::ExitRoomAfterRecoveryFailure()
{
    std::vector<std::shared_ptr<RoomPairRuntime>> roomPairs;
    ISignalingClient* signaling = nullptr;
    std::string roomId;
    bool signalingOnline = false;
    {
        std::lock_guard lock(mutex_);
        roomId = snapshot_.room.roomId;
        if (roomId.empty()) {
            return Success();
        }
        signaling = signaling_.get();
        signalingOnline = signaling &&
            snapshot_.connectivity == SessionConnectivityState::kOnline;
        roomPairs.reserve(roomPairs_.size());
        for (auto& [pairId, pair] : roomPairs_) {
            (void)pairId;
            roomPairs.push_back(std::move(pair));
        }
        roomPairs_.clear();
        ResetRoomStateLocked();
        if (!signalingOnline) {
            roomSession_.deferredLeaveId_ = roomId;
        }
    }

    for (auto& pair : roomPairs) {
        RetireRoomPair(std::move(pair));
    }

    if (signalingOnline) {
        const auto leave = signaling->LeaveRoom(
            roomId, "p2p_recovery_failed");
        if (!leave.accepted) {
            std::lock_guard lock(mutex_);
            roomSession_.deferredLeaveId_ = roomId;
        }
    }
    PublishSnapshot();
    return Success();
}

}  // namespace remote::app
