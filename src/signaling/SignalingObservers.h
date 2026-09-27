// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include "SignalingTypes.h"

namespace remote {

class ISignalingConnectionObserver {
public:
    virtual ~ISignalingConnectionObserver() = default;

    virtual void OnSignalingStateChanged(
        SignalingConnectionState state) = 0;
    virtual void OnDeviceRegistered(const std::string& deviceId) = 0;
    virtual void OnHeartbeatAcknowledged(std::uint32_t roundTripMs) = 0;
    virtual void OnSignalingError(const std::string& code,
                                  const std::string& message) = 0;
};

class IDirectSignalingObserver {
public:
    virtual ~IDirectSignalingObserver() = default;

    virtual void OnIncomingSessionRequest(
        const IncomingSessionRequest& request) = 0;
    virtual void OnSessionResponse(
        const SignalingSessionResponse& response) = 0;
    virtual void OnSessionPending(
        const SignalingSessionPending& pending) = 0;
    virtual void OnSessionReady(const SignalingSessionReady& ready) = 0;
    virtual void OnSessionSuspended(
        const SignalingSessionSuspended& suspended) = 0;
    virtual void OnSessionResumed(
        const SignalingSessionResumed& resumed) = 0;
    virtual void OnSessionEnded(const SignalingSessionEnded& ended) = 0;
    virtual void OnRemoteDescription(
        const SignalingSessionDescription& description) = 0;
    virtual void OnRemoteIceCandidate(
        const SignalingIceCandidate& candidate) = 0;
    virtual void OnIceRestartRequested(
        const SignalingIceRestartRequest& request)
    {
        (void)request;
    }
    virtual void OnIceRestartCancelled(
        const SignalingIceRestartCancel& cancel)
    {
        (void)cancel;
    }
};

class IAccountSignalingObserver {
public:
    virtual ~IAccountSignalingObserver() = default;

    virtual void OnAccountDeletionResult(
        const SignalingAccountDeletionResult& result)
    {
        (void)result;
    }
    virtual void OnOwnedDevicesChanged(
        const SignalingOwnedDevicesSnapshot& snapshot)
    {
        (void)snapshot;
    }
};

class IRoomSignalingObserver {
public:
    virtual ~IRoomSignalingObserver() = default;

    // Default no-op implementations keep direct-session-only observers small.
    virtual void OnRoomReady(const SignalingRoomReady& ready)
    {
        (void)ready;
    }
    virtual void OnRoomState(const RoomSnapshot& room)
    {
        (void)room;
    }
    virtual void OnRoomJoinPending(
        const SignalingRoomJoinPending& pending)
    {
        (void)pending;
    }
    virtual void OnRoomJoinRequested(const RoomJoinRequest& request)
    {
        (void)request;
    }
    virtual void OnRoomJoinResult(
        const SignalingRoomJoinResult& result)
    {
        (void)result;
    }
    virtual void OnRoomAvailabilityResult(
        const SignalingRoomAvailabilityResult& result)
    {
        (void)result;
    }
    virtual void OnRoomClosed(const SignalingRoomClosed& closed)
    {
        (void)closed;
    }
    virtual void OnRoomPairReady(const SignalingRoomPairReady& ready)
    {
        (void)ready;
    }
    virtual void OnRoomPairClosed(const SignalingRoomPairClosed& closed)
    {
        (void)closed;
    }
    virtual void OnRoomScreenShareGranted(
        const SignalingRoomScreenShareGranted& granted)
    {
        (void)granted;
    }
    virtual void OnRoomScreenShareSwitchPending(
        const SignalingRoomScreenShareSwitchPending& pending)
    {
        (void)pending;
    }
    virtual void OnRoomScreenShareSwitchRequested(
        const RoomScreenShareSwitchRequest& request)
    {
        (void)request;
    }
    virtual void OnRoomScreenShareSwitchResult(
        const SignalingRoomScreenShareSwitchResult& result)
    {
        (void)result;
    }
    virtual void OnRoomControlRequested(const RoomControlRequest& request)
    {
        (void)request;
    }
    virtual void OnRoomControlResult(
        const SignalingRoomControlResult& result)
    {
        (void)result;
    }
    virtual void OnRoomControlGranted(
        const SignalingRoomControlGranted& granted)
    {
        (void)granted;
    }
    virtual void OnRoomControlRevoked(
        const SignalingRoomControlRevoked& revoked)
    {
        (void)revoked;
    }
};

class ISignalingClientObserver : public ISignalingConnectionObserver,
                                 public IDirectSignalingObserver,
                                 public IAccountSignalingObserver,
                                 public IRoomSignalingObserver {
public:
    ~ISignalingClientObserver() override = default;
};

}  // namespace remote
