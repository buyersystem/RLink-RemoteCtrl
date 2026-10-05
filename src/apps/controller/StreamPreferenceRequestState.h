// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>

#include "src/core/SessionEngineTypes.h"

namespace remote::controller {

// UI-only sequencing. Transport completions remain distinct from remote ACKs.
// Requests are sent in order; stale failures must never undo a newer choice.
class StreamPreferenceRequestState {
public:
    enum class Completion { kStale, kSucceeded, kFailed };

    void Reset(const ScreenStreamPreferenceRequest& preference)
    {
        ++generation_;
        latestSequence_ = 0;
        completedSequence_ = 0;
        successfulSequence_ = 0;
        awaitingAckSequence_ = 0;
        lastSuccessful_ = preference;
    }

    std::uint64_t Begin() { return ++latestSequence_; }
    std::uint64_t Generation() const { return generation_; }
    bool HasPending() const { return completedSequence_ != latestSequence_; }
    const ScreenStreamPreferenceRequest& LastSuccessful() const
    {
        return lastSuccessful_;
    }

    void UpdateAcknowledged(const ScreenStreamPreferenceRequest& preference)
    {
        if (!HasPending()) lastSuccessful_ = preference;
    }

    void MarkSentTransportSequence(std::uint64_t sequence)
    {
        if (sequence > awaitingAckSequence_) awaitingAckSequence_ = sequence;
    }

    bool CanApplyAcknowledged(std::uint64_t acceptedSequence,
                              std::uint64_t requestSequence, bool rejected)
    {
        if (HasPending()) return false;
        if (acceptedSequence >= awaitingAckSequence_ ||
            (rejected && requestSequence >= awaitingAckSequence_)) {
            awaitingAckSequence_ = 0;
        }
        return awaitingAckSequence_ == 0;
    }

    Completion Complete(std::uint64_t generation, std::uint64_t sequence,
                        const ScreenStreamPreferenceRequest& preference,
                        bool succeeded)
    {
        if (generation != generation_ || sequence == 0 ||
            sequence > latestSequence_) {
            return Completion::kStale;
        }
        if (succeeded && sequence > successfulSequence_) {
            successfulSequence_ = sequence;
            lastSuccessful_ = preference;
        }
        if (sequence != latestSequence_ || sequence <= completedSequence_) {
            return Completion::kStale;
        }
        completedSequence_ = sequence;
        return succeeded ? Completion::kSucceeded : Completion::kFailed;
    }

private:
    std::uint64_t generation_ = 0;
    std::uint64_t latestSequence_ = 0;
    std::uint64_t completedSequence_ = 0;
    std::uint64_t successfulSequence_ = 0;
    std::uint64_t awaitingAckSequence_ = 0;
    ScreenStreamPreferenceRequest lastSuccessful_;
};

}  // namespace remote::controller
