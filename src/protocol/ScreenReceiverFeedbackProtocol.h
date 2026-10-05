// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace remote {

inline constexpr std::uint8_t kScreenReceiverFeedbackProtocolVersion = 1;
inline constexpr std::size_t kScreenReceiverFeedbackHeaderBytes = 80;
inline constexpr std::size_t kMaximumScreenReceiverFeedbackRoomIdBytes = 128;
inline constexpr std::size_t kMaximumScreenReceiverFeedbackDeviceIdBytes = 64;
inline constexpr std::size_t kMaximumScreenReceiverFeedbackMessageBytes = 272;
inline constexpr std::uint32_t kMinimumScreenReceiverFeedbackWindowMs = 100;
inline constexpr std::uint32_t kMaximumScreenReceiverFeedbackWindowMs = 10000;
inline constexpr std::uint64_t kMaximumScreenReceiverFeedbackTimeUs = 10000000;
inline constexpr std::uint32_t kMaximumScreenReceiverFeedbackFrameDimension = 16384;
inline constexpr std::uint64_t kMaximumScreenReceiverFeedbackFramePixels =
    64ULL * 1024 * 1024;
inline constexpr std::uint32_t kMaximumScreenReceiverFeedbackWindowFrames = 65536;

// Low-frequency metadata only. No peer timestamps are transmitted: freshness
// is determined from the sender's own monotonic receive clock after dispatch.
struct ScreenReceiverFeedback {
    std::string roomId;
    // Device emitting this receiver report, not the video source device.
    std::string senderDeviceId;
    std::uint64_t sequence = 0;
    std::uint64_t screenShareGeneration = 0;
    // Zero identifies the initial default preference before any user request.
    std::uint64_t preferenceSequence = 0;
    std::uint32_t sampleWindowMs = 0;
    // Counter increments in this window, never cumulative session counters.
    std::uint32_t decodedFrames = 0;
    std::uint32_t droppedFrames = 0;
    std::uint32_t frameWidth = 0;
    std::uint32_t frameHeight = 0;
    bool decodeTimeAvailable = false;
    bool processingTimeAvailable = false;
    bool jitterBufferAvailable = false;
    // Average-per-frame microseconds in this window, not cumulative times.
    // Jitter-buffer averages use WebRTC's emitted-count denominator, which
    // need not equal decodedFrames. Unavailable fields must contain zero.
    std::uint64_t windowDecodeTimeUs = 0;
    std::uint64_t processingTimeUs = 0;
    std::uint64_t jitterBufferUs = 0;

    bool operator==(const ScreenReceiverFeedback&) const = default;
};

// Checks the family magic only, including malformed/unsupported versions so
// dispatchers can consume and reject them rather than try another protocol.
bool IsScreenReceiverFeedbackMessage(std::span<const std::uint8_t> encoded);

bool ValidateScreenReceiverFeedback(const ScreenReceiverFeedback& feedback,
                                    std::string* error = nullptr);
bool EncodeScreenReceiverFeedback(const ScreenReceiverFeedback& feedback,
                                  std::vector<std::uint8_t>* encoded,
                                  std::string* error = nullptr);
bool DecodeScreenReceiverFeedback(std::span<const std::uint8_t> encoded,
                                  ScreenReceiverFeedback* feedback,
                                  std::string* error = nullptr);

// One state per authenticated pair. Identity/room authorization is the host's
// responsibility. Current generation and current preference come from local
// authoritative state; an incoming message cannot advance either of them.
struct ScreenReceiverFeedbackSequenceState {
    std::uint64_t screenShareGeneration = 0;
    std::uint64_t preferenceSequence = 0;
    std::uint64_t lastSequence = 0;
};

bool AcceptScreenReceiverFeedbackSequence(
    ScreenReceiverFeedbackSequenceState* state,
    const ScreenReceiverFeedback& feedback,
    std::uint64_t currentGeneration,
    std::uint64_t currentPreferenceSequence);

}  // namespace remote
