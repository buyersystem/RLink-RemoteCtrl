// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include "IRemoteSessionControl.h"
#include "src/protocol/ClipboardProtocol.h"
#include "src/protocol/FileTransferProtocol.h"
#include "src/protocol/RemoteInputProtocol.h"

namespace remote {

class ISessionEngineObserver {
public:
    virtual ~ISessionEngineObserver() = default;

    // V1 callbacks are synchronous on the thread that changes engine state.
    // That may be the Qt signaling thread or the session controller executor;
    // UI observers must marshal the immutable snapshot to the UI thread. The
    // future IPC implementation keeps this interface and the same payload.
    virtual void OnSessionEngineSnapshot(
        const SessionEngineSnapshot& snapshot) = 0;
};

class ISessionEngine : public IRemoteSessionControl {
public:
    ~ISessionEngine() override = default;

    virtual void SetObserver(ISessionEngineObserver* observer) = 0;
    // Starts initialization without blocking the caller. A successful result
    // means startup was accepted; observers receive kStarting followed by
    // kReady or kFailed when initialization finishes.
    virtual SessionCommandResult Start() = 0;
    virtual void Stop() = 0;

    virtual SessionEngineCapabilities Capabilities() const = 0;
    virtual SessionDiagnosticsSnapshot Diagnostics() const { return {}; }

    virtual SessionCommandResult ConnectDirectDevice(
        const DirectSessionConnectRequest& request) = 0;

    virtual SessionCommandResult RefreshOwnedDevices() = 0;
    virtual SessionCommandResult AcceptIncomingSession(
        const std::string& sessionId) = 0;
    virtual SessionCommandResult RejectIncomingSession(
        const std::string& sessionId) = 0;

    // Room control-plane operations are independent from direct sessions.
    // Room membership and member-pair WebRTC connections use the same engine
    // snapshot without overwriting the verified direct-session fields above.
    virtual SessionCommandResult CreateRoom(std::uint32_t capacity) = 0;
    virtual SessionCommandResult JoinRoom(const std::string& roomId) = 0;
    virtual SessionCommandResult QueryRoomAvailability(
        const std::vector<std::string>& roomIds)
    {
        (void)roomIds;
        return {false, "room_availability_unsupported",
                "Room availability queries are not supported."};
    }
    virtual SessionCommandResult RespondToRoomJoin(
        const std::string& requestId,
        bool accepted) = 0;
    virtual SessionCommandResult SetRoomCapacity(
        std::uint32_t capacity) = 0;
    virtual SessionCommandResult LeaveRoom() = 0;
    virtual SessionCommandResult RefreshLocalDisplays() = 0;
    virtual SessionCommandResult SelectRoomScreenShareDisplay(
        const std::string& stableDisplayKey) = 0;
    virtual SessionCommandResult StartRoomScreenShare() = 0;
    virtual SessionCommandResult StopRoomScreenShare() = 0;
    virtual SessionCommandResult RespondToRoomScreenShareSwitch(
        const std::string& requestId,
        bool accepted) = 0;
    virtual SessionCommandResult CancelRoomScreenShareSwitch() = 0;
    virtual SessionCommandResult RespondToRoomControl(
        const std::string& requestId,
        bool accepted) = 0;
    virtual SessionCommandResult RequestRoomMemberScreenShare(
        const std::string& peerDeviceId) = 0;
    virtual SessionCommandResult RespondToRoomMemberScreenShare(
        const std::string& requesterDeviceId,
        std::uint64_t sequence,
        bool accepted) = 0;
    virtual SessionCommandResult RequestRoomMemberMicrophoneMute(
        const std::string& peerDeviceId) = 0;
    virtual SessionCommandResult RequestRemoteRoomScreenShareStop(
        const std::string& peerDeviceId,
        std::uint64_t screenShareEpoch) = 0;
    virtual SessionCommandResult SendRoomInput(
        const RemoteInputEvent& event) = 0;
    virtual SessionCommandResult SetRoomScreenFrameRate(
        const std::string& pairId,
        std::uint32_t framesPerSecond) = 0;
    virtual SessionCommandResult SendRoomFileMessage(
        const std::string& peerDeviceId,
        const FileTransferMessage& message) = 0;
    virtual SessionCommandResult SendRoomClipboardMessage(
        const std::string& peerDeviceId,
        const std::string& clipboardSessionId,
        const ClipboardMessage& message) = 0;

    // A peer may only enable its own camera. Enabling starts local preview and
    // publishes the local Camera Track; disabling stops capture and sending.
    virtual SessionCommandResult SetLocalCameraEnabled(bool enabled) = 0;
    virtual SessionCommandResult SetRoomAudioPlaybackMuted(bool muted) = 0;
};

}  // namespace remote
