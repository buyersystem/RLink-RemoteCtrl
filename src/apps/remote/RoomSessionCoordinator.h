// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/core/ISessionEngine.h"
#include "src/protocol/ScreenShareControlProtocol.h"

namespace remote::app {

class InProcessSessionEngine;

class RoomSessionCoordinator final {
public:
    [[nodiscard]] SessionCommandResult ValidateCapacity(
        std::uint32_t capacity) const;

    [[nodiscard]] SessionCommandResult ValidateCreate(
        const SessionEngineSnapshot& snapshot,
        std::uint32_t capacity) const;
    void ApplyCreateRequested(
        SessionEngineSnapshot* snapshot,
        std::uint32_t capacity) const;

    [[nodiscard]] SessionCommandResult ValidateJoin(
        const SessionEngineSnapshot& snapshot,
        const std::string& roomId) const;
    void ApplyJoinRequested(
        SessionEngineSnapshot* snapshot,
        const std::string& roomId) const;

    [[nodiscard]] SessionCommandResult ValidateAvailabilityQuery(
        const std::vector<std::string>& roomIds) const;
    void ApplyAvailabilityQuery(
        SessionEngineSnapshot* snapshot,
        const std::vector<std::string>& roomIds) const;

    [[nodiscard]] SessionCommandResult PrepareJoinResponse(
        const SessionEngineSnapshot& snapshot,
        const std::string& requestId,
        std::string* roomId) const;

    [[nodiscard]] SessionCommandResult PrepareCapacityUpdate(
        const SessionEngineSnapshot& snapshot,
        std::uint32_t capacity,
        std::string* roomId) const;

    [[nodiscard]] SessionCommandResult PrepareLeave(
        SessionEngineSnapshot* snapshot,
        std::string* roomId) const;
    void ApplyLeaveFailed(
        SessionEngineSnapshot* snapshot,
        const std::string& errorCode,
        const std::string& errorMessage) const;

    [[nodiscard]] std::optional<std::uint64_t>
    TakeNextScreenControlSequence();
    void ResetActiveState();

private:
    friend class InProcessSessionEngine;

    bool audioDevicesApplied_ = false;
    std::string recoveryToken_;
    std::string screenShareGrantId_;
    std::string controlGrantId_;
    std::string controlGrantScreenSharerDeviceId_;
    std::string controlGrantControllerDeviceId_;
    std::uint64_t nextInputSequence_ = 0;
    std::uint64_t nextScreenControlSequence_ = 0;
    std::uint32_t localScreenFrameRate_ = kDefaultScreenFrameRate;
    std::unordered_map<std::string, ScreenStreamPreferenceRequest>
        screenStreamPreferences_;
    bool recoveryPending_ = false;
    bool leaveRequested_ = false;
    std::string deferredLeaveId_;
};

}  // namespace remote::app
