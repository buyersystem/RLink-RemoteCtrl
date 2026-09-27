// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "InProcessSessionEngine.h"

#include <cstdint>
#include <span>
#include <string>

#include "InProcessSessionEngineInternal.h"
#include "SessionDataChannelPolicy.h"
#include "src/protocol/ClipboardProtocol.h"
#include "src/protocol/DataChannelCatalog.h"
#include "src/protocol/FileTransferProtocol.h"

namespace remote::app {

bool InProcessSessionEngine::DispatchRoomPairTransferData(
    const std::string& pairId,
    const std::string& label,
    std::span<const std::uint8_t> payload)
{
    if (label == kFileTransferChannel) {
        FileTransferEnvelope envelope;
        if (!DecodeFileTransferMessage(payload, &envelope)) {
            return true;
        }

        IFileTransferSink* sink = nullptr;
        {
            std::lock_guard lock(mutex_);
            const auto pairIt = roomPairs_.find(pairId);
            if (pairIt == roomPairs_.end() ||
                snapshot_.room.membership != RoomMembershipState::kActive ||
                envelope.roomId != snapshot_.room.roomId ||
                envelope.roomId != pairIt->second->roomId ||
                envelope.senderDeviceId != pairIt->second->peerDeviceId ||
                envelope.receiverDeviceId != snapshot_.localDeviceId ||
                envelope.sequence <=
                    pairIt->second->lastFileTransferSequence) {
                return true;
            }
            pairIt->second->lastFileTransferSequence = envelope.sequence;
            sink = remoteFileTransferSink_;
        }
        if (sink) {
            sink->OnFileTransferMessage(envelope);
        }
        return true;
    }

    if (label == kClipboardReliableChannel ||
        label == kClipboardTransferChannel) {
        if (label == kClipboardTransferChannel &&
            IsClipboardWarmupPayload(payload)) {
            return true;
        }
        ClipboardEnvelope envelope;
        if (!DecodeClipboardMessage(payload, &envelope)) {
            return true;
        }
        IClipboardSink* sink = nullptr;
        {
            std::lock_guard lock(mutex_);
            const auto pairIt = roomPairs_.find(pairId);
            if (pairIt == roomPairs_.end() || !pairIt->second ||
                snapshot_.room.membership != RoomMembershipState::kActive ||
                envelope.roomId != snapshot_.room.roomId ||
                envelope.roomId != pairIt->second->roomId ||
                envelope.senderDeviceId != pairIt->second->peerDeviceId ||
                envelope.receiverDeviceId != snapshot_.localDeviceId ||
                roomSession_.controlGrantId_.empty() ||
                envelope.controlGrantId != roomSession_.controlGrantId_) {
                return true;
            }
            const bool localIsSharer =
                snapshot_.room.screenSharerDeviceId ==
                    snapshot_.localDeviceId &&
                snapshot_.room.activeControllerDeviceId ==
                    envelope.senderDeviceId;
            const bool localIsController =
                snapshot_.room.activeControllerDeviceId ==
                    snapshot_.localDeviceId &&
                snapshot_.room.screenSharerDeviceId ==
                    envelope.senderDeviceId;
            if (!localIsSharer && !localIsController) {
                return true;
            }
            auto& lastSequence = label == kClipboardTransferChannel
                ? pairIt->second->lastClipboardTransferSequence
                : pairIt->second->lastClipboardReliableSequence;
            if (envelope.sequence <= lastSequence ||
                IsClipboardTransferMessage(envelope.message.type) !=
                    (label == kClipboardTransferChannel)) {
                return true;
            }
            lastSequence = envelope.sequence;
            sink = remoteClipboardSink_;
        }
        if (sink) {
            sink->OnClipboardMessage(envelope);
        }
        return true;
    }

    return false;
}

}  // namespace remote::app
