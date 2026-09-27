// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include "SignalingObservers.h"

namespace remote {

class ISignalingClient {
public:
    virtual ~ISignalingClient() = default;

    virtual void SetObserver(ISignalingClientObserver* observer) = 0;
    virtual SignalingOperationResult Connect(
        const SignalingClientConfig& config) = 0;
    // Replaces the in-memory token used by the next message authentication or
    // reconnect. It deliberately does not tear down an active WebRTC session.
    virtual SignalingOperationResult UpdateAccessToken(
        const std::string& accessToken) = 0;
    virtual SignalingOperationResult RequestAccountDeletion()
    {
        return {false, "account_deletion_unsupported",
                "Account deletion is not supported."};
    }
    virtual SignalingOperationResult RequestOwnedDevices()
    {
        return {false, "my_devices_unsupported",
                "Owned-device discovery is not supported."};
    }
    virtual void Disconnect() = 0;
    virtual SignalingConnectionState State() const = 0;

    virtual SignalingOperationResult RequestSession(
        const std::string& targetDeviceId,
        SessionPurpose purpose,
        const std::vector<std::string>& permissions) = 0;
    virtual SignalingOperationResult RequestOwnedDeviceSession(
        const std::string& targetDeviceId,
        SessionPurpose purpose,
        const std::vector<std::string>& permissions)
    {
        return RequestSession(targetDeviceId, purpose, permissions);
    }
    virtual SignalingOperationResult RequestAssistedSession(
        const std::string& targetDeviceId,
        const std::string& verificationCode,
        const std::vector<std::string>& permissions)
    {
        (void)verificationCode;
        return RequestSession(
            targetDeviceId, SessionPurpose::kRemoteControl, permissions);
    }
    virtual SignalingOperationResult RespondToSession(
        const std::string& sessionId,
        bool accepted,
        const std::string& reasonCode) = 0;
    virtual SignalingOperationResult CancelSession(
        const std::string& sessionId,
        const std::string& reasonCode) = 0;
    virtual SignalingOperationResult CloseSession(
        const std::string& sessionId,
        const std::string& reasonCode) = 0;
    virtual SignalingOperationResult ResumeSession(
        const std::string& sessionId,
        const std::string& recoveryToken) = 0;

    virtual SignalingOperationResult CreateRoom(
        std::uint32_t capacity) = 0;
    virtual SignalingOperationResult RequestRoomJoin(
        const std::string& roomId) = 0;
    virtual SignalingOperationResult QueryRoomAvailability(
        const std::vector<std::string>& roomIds)
    {
        (void)roomIds;
        return {false, "room_availability_unsupported",
                "Room availability queries are not supported."};
    }
    virtual SignalingOperationResult RespondToRoomJoin(
        const std::string& roomId,
        const std::string& requestId,
        bool accepted,
        const std::string& reasonCode) = 0;
    virtual SignalingOperationResult SetRoomCapacity(
        const std::string& roomId,
        std::uint32_t capacity) = 0;
    virtual SignalingOperationResult LeaveRoom(
        const std::string& roomId,
        const std::string& reasonCode) = 0;
    virtual SignalingOperationResult ResumeRoom(
        const std::string& roomId,
        const std::string& recoveryToken) = 0;
    virtual SignalingOperationResult SetRoomMediaState(
        const std::string& roomId,
        bool cameraPublishing,
        bool microphonePublishing) = 0;
    virtual SignalingOperationResult RequestRoomScreenShare(
        const std::string& roomId) = 0;
    virtual SignalingOperationResult ConfirmRoomScreenShare(
        const std::string& roomId,
        const std::string& grantId) = 0;
    virtual SignalingOperationResult StopRoomScreenShare(
        const std::string& roomId,
        const std::string& grantId,
        const std::string& reasonCode) = 0;
    virtual SignalingOperationResult RespondToRoomScreenShareSwitch(
        const std::string& roomId,
        const std::string& requestId,
        bool accepted,
        const std::string& reasonCode)
    {
        (void)roomId;
        (void)requestId;
        (void)accepted;
        (void)reasonCode;
        return {false, "room_screen_share_switch_unsupported",
                "Screen-share takeover approval is not supported."};
    }
    virtual SignalingOperationResult CancelRoomScreenShareSwitch(
        const std::string& roomId,
        const std::string& requestId,
        const std::string& reasonCode)
    {
        (void)roomId;
        (void)requestId;
        (void)reasonCode;
        return {false, "room_screen_share_switch_unsupported",
                "Screen-share takeover cancellation is not supported."};
    }
    virtual SignalingOperationResult RequestRoomControl(
        const std::string& roomId) = 0;
    virtual SignalingOperationResult RespondToRoomControl(
        const std::string& roomId,
        const std::string& requestId,
        bool accepted,
        const std::string& reasonCode) = 0;
    virtual SignalingOperationResult ReleaseRoomControl(
        const std::string& roomId,
        const std::string& grantId,
        const std::string& reasonCode) = 0;

    virtual SignalingOperationResult SendDescription(
        const SignalingSessionDescription& description) = 0;
    virtual SignalingOperationResult SendIceCandidate(
        const SignalingIceCandidate& candidate) = 0;
    virtual SignalingOperationResult SendIceRestartRequest(
        const SignalingIceRestartRequest& request)
    {
        (void)request;
        return {false, "ice_restart_unsupported",
                "ICE restart signaling is not supported."};
    }
    virtual SignalingOperationResult SendIceRestartCancel(
        const SignalingIceRestartCancel& cancel)
    {
        (void)cancel;
        return {false, "ice_restart_unsupported",
                "ICE restart signaling is not supported."};
    }
};

}  // namespace remote
