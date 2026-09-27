// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include "SessionEngineTypes.h"

namespace remote {

// Commands and state required by the focused remote-session window. Keeping
// this surface separate prevents that view from depending on engine startup,
// room membership, owned-device, file-transfer, and clipboard operations.
class IRemoteSessionControl {
public:
    virtual ~IRemoteSessionControl() = default;

    virtual SessionEngineSnapshot Snapshot() const = 0;
    virtual SessionCommandResult Disconnect() = 0;
    virtual SessionCommandResult ExitRoomAfterRecoveryFailure() = 0;
    virtual SessionCommandResult RequestRoomControl() = 0;
    virtual SessionCommandResult ReleaseRoomControl() = 0;
    virtual SessionCommandResult SetRoomScreenStreamPreference(
        const std::string& pairId,
        const ScreenStreamPreferenceRequest& preference) = 0;
    virtual SessionCommandResult RequestRemoteSharedDisplaySwitch(
        const std::string& pairId,
        const std::string& stableDisplayKey) = 0;
    virtual SessionCommandResult SetLocalMicrophoneEnabled(bool enabled) = 0;
    virtual SessionCommandResult RefreshLocalMediaDevices() = 0;
    virtual SessionCommandResult SelectLocalCameraDevice(
        const std::string& deviceId) = 0;
    virtual SessionCommandResult SelectLocalMicrophoneDevice(
        const std::string& deviceId) = 0;
    virtual SessionCommandResult SelectLocalSpeakerDevice(
        const std::string& deviceId) = 0;
};

}  // namespace remote
