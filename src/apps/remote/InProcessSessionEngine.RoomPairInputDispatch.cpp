// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "InProcessSessionEngine.h"

#include <cstdint>
#include <span>
#include <string>

#include "InProcessSessionEngineInternal.h"
#include "src/core/RemoteInputTelemetry.h"
#include "src/platform/win/WindowsCursorMonitor.h"
#include "src/platform/win/WindowsDesktopCaptureSource.h"
#include "src/protocol/RemoteInputProtocol.h"

namespace remote::app {

void InProcessSessionEngine::DispatchRoomPairInputData(
    const std::string& pairId,
    std::span<const std::uint8_t> payload,
    bool fastChannel)
{
    RemoteInputEnvelope input;
    std::string inputDecodeError;
    if (!DecodeRemoteInput(payload, &input, &inputDecodeError)) {
        return;
    }
    auto& inputTelemetry = RemoteInputTelemetry::Instance();
    inputTelemetry.RecordPacketReceived();
    const auto recordInputDrop = [&inputTelemetry] {
        inputTelemetry.RecordPacketDropped();
    };
    const bool buttonTransition =
        input.event.type == RemoteInputMessageType::kMouseButton;
    if (!buttonTransition &&
        UsesFastInputChannel(input.event.type) != fastChannel) {
        recordInputDrop();
        return;
    }

    IRemoteInputSink* sink = nullptr;
    webrtc::scoped_refptr<WindowsDesktopCaptureSource>
        sourceToBoost;
    {
        std::lock_guard lock(mutex_);
        const auto pairIt = roomPairs_.find(pairId);
        if (pairIt == roomPairs_.end() ||
            snapshot_.room.membership != RoomMembershipState::kActive ||
            snapshot_.room.screenShareState !=
                RoomScreenShareState::kActive ||
            snapshot_.room.screenSharerDeviceId != snapshot_.localDeviceId ||
            snapshot_.room.activeControllerDeviceId !=
                pairIt->second->peerDeviceId ||
            input.roomId != snapshot_.room.roomId ||
            input.roomId != pairIt->second->roomId ||
            input.senderDeviceId != pairIt->second->peerDeviceId ||
            roomSession_.controlGrantId_.empty() ||
            input.controlGrantId != roomSession_.controlGrantId_) {
            recordInputDrop();
            return;
        }
        if (IsPointerInput(input.event.type) &&
            (snapshot_.screenShare.activeDisplay.sessionDisplayId == 0 ||
             snapshot_.screenShare.activeDisplayLayoutVersion == 0 ||
             input.event.displayId !=
                 snapshot_.screenShare.activeDisplay.sessionDisplayId ||
             input.event.displayLayoutVersion !=
                 snapshot_.screenShare.activeDisplayLayoutVersion)) {
            recordInputDrop();
            return;
        }

        std::uint64_t& lastSequence = fastChannel
            ? pairIt->second->lastFastInputSequence
            : pairIt->second->lastReliableInputSequence;
        if (input.sequence <= lastSequence) {
            recordInputDrop();
            return;
        }
        lastSequence = input.sequence;
        input.event.deliverySequence = input.sequence;
        if (buttonTransition ||
            input.event.type == RemoteInputMessageType::kReleaseAll) {
            if (input.sequence <=
                pairIt->second->lastPointerStateSequence) {
                recordInputDrop();
                return;
            }
            pairIt->second->lastPointerStateSequence = input.sequence;
        }
        sink = remoteInputSink_;
        if (sink && ShouldBoostDesktopCaptureForInput(input.event)) {
            sourceToBoost = screenShare_.CaptureSource();
        }
    }
    if (sink) {
        // Recheck after dispatch preparation: a local safety stop may have
        // revoked this lease while the packet was waiting for injection.
        // Serialize injection with ReleaseRoomControl's revoke + ReleaseAll.
        {
            std::lock_guard lock(mutex_);
            if (snapshot_.room.membership != RoomMembershipState::kActive ||
                snapshot_.room.screenShareState != RoomScreenShareState::kActive ||
                roomSession_.controlGrantId_ != input.controlGrantId ||
                snapshot_.room.activeControllerDeviceId != input.senderDeviceId ||
                snapshot_.room.screenSharerDeviceId != snapshot_.localDeviceId ||
                remoteInputSink_ != sink) {
                recordInputDrop();
                return;
            }
            sink->OnRemoteInput(input.event);
            cursorMonitor_->SetLastAppliedInputSequence(input.sequence);
        }
        // Wake capture only after SendInput has run, matching Chrome Remote
        // Desktop's input-to-frame ordering instead of racing the injection.
        if (sourceToBoost) {
            sourceToBoost->NotifyRemoteInputActivity();
        }
    } else {
        recordInputDrop();
    }
}

}  // namespace remote::app
