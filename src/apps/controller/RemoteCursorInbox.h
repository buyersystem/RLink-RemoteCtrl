// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <deque>
#include <mutex>
#include <string>
#include <utility>

#include "src/protocol/RemoteCursorProtocol.h"

namespace remote::controller {

inline bool RemoteCursorMatchesContext(
    const std::string& pairId, const RemoteCursorEnvelope& envelope,
    const std::string& expectedPairId, const std::string& expectedSender,
    const std::string& expectedContext, std::uint64_t expectedGeneration,
    bool allowUnknownGeneration = false)
{
    return pairId == expectedPairId &&
        envelope.senderDeviceId == expectedSender &&
        envelope.contextId == expectedContext &&
        envelope.screenShareGeneration != 0 &&
        ((allowUnknownGeneration && expectedGeneration == 0) ||
            envelope.screenShareGeneration == expectedGeneration);
}

// Refreshed from the UI's existing session-state snapshot. Cursor delivery
// consults this small GUI-owned cache rather than copying/locking the engine
// snapshot at pointer frequency.
struct RemoteCursorContext {
    std::string pairId;
    std::string sender;
    std::string contextId;
    std::uint64_t generation = 0;
    bool allowUnknownGeneration = false;

    bool Matches(const std::string& incomingPairId,
                 const RemoteCursorEnvelope& envelope) const
    {
        return !contextId.empty() && RemoteCursorMatchesContext(
            incomingPairId, envelope, pairId, sender, contextId,
            generation, allowUnknownGeneration);
    }
};

// One inbox belongs to one callback binding. Position runs are replaceable;
// shapes and resets are ordering barriers. Producers post a GUI wake only
// when the inbox changes from unclaimed to claimed, avoiding stale GUI tasks.
class RemoteCursorInbox final {
public:
    struct Message {
        std::string pairId;
        RemoteCursorEnvelope envelope;
    };

    bool Push(std::string pairId, const RemoteCursorEnvelope& envelope)
    {
        std::lock_guard lock(mutex_);
        if (!active_) return false;
        if (!messages_.empty() && SamePositionRun(
                messages_.back(), pairId, envelope)) {
            // Sequence validation normally happens in the engine; retain it
            // here so an old arrival never replaces the latest queued point.
            if (envelope.sequence > messages_.back().envelope.sequence) {
                messages_.back().envelope = envelope;
            }
        } else {
            messages_.push_back({std::move(pairId), envelope});
        }
        return !std::exchange(wakeClaimed_, true);
    }

    std::deque<Message> Take()
    {
        std::lock_guard lock(mutex_);
        std::deque<Message> batch;
        batch.swap(messages_);
        // A producer arriving after this atomic handoff claims a fresh wake.
        // A producer arriving before it is included in this batch.
        wakeClaimed_ = false;
        return batch;
    }

    void Deactivate()
    {
        std::lock_guard lock(mutex_);
        active_ = false;
        messages_.clear();
        wakeClaimed_ = false;
    }

private:
    static bool SamePositionRun(const Message& previous,
                                const std::string& pairId,
                                const RemoteCursorEnvelope& next)
    {
        const auto& old = previous.envelope;
        return old.type == RemoteCursorMessageType::kPosition &&
            next.type == RemoteCursorMessageType::kPosition &&
            previous.pairId == pairId &&
            old.contextId == next.contextId &&
            old.senderDeviceId == next.senderDeviceId &&
            old.screenShareGeneration == next.screenShareGeneration &&
            old.position.displayId == next.position.displayId &&
            old.position.displayLayoutVersion ==
                next.position.displayLayoutVersion &&
            old.position.shapeId == next.position.shapeId;
    }

    std::mutex mutex_;
    std::deque<Message> messages_;
    bool active_ = true;
    bool wakeClaimed_ = false;
};

}  // namespace remote::controller
