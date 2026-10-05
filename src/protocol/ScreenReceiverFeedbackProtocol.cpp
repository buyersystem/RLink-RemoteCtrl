// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ScreenReceiverFeedbackProtocol.h"

#include <algorithm>
#include <array>
#include <utility>

#include "BinaryProtocol.h"

namespace remote {
namespace {

constexpr std::array<std::uint8_t, 4> kMagic = {'R', 'C', 'R', 'F'};
constexpr std::uint8_t kReportType = 1;
constexpr std::uint16_t kDecodeAvailable = 1;
constexpr std::uint16_t kProcessingAvailable = 2;
constexpr std::uint16_t kJitterAvailable = 4;
constexpr std::uint16_t kKnownFlags = 7;

bool Fail(std::string* error, const char* message)
{
    if (error) {
        *error = message;
    }
    return false;
}

bool ValidIdentity(const std::string& value, std::size_t maximum)
{
    return !value.empty() && value.size() <= maximum &&
        std::none_of(value.begin(), value.end(), [](unsigned char character) {
            return character < 0x20 || character == 0x7f;
        });
}

bool ValidTime(bool available, std::uint64_t value,
               std::uint32_t decodedFrames)
{
    return available ? (decodedFrames != 0 &&
                         value <= kMaximumScreenReceiverFeedbackTimeUs)
                     : value == 0;
}

}  // namespace

bool IsScreenReceiverFeedbackMessage(std::span<const std::uint8_t> encoded)
{
    return encoded.size() >= kMagic.size() &&
        std::equal(kMagic.begin(), kMagic.end(), encoded.begin());
}

bool ValidateScreenReceiverFeedback(const ScreenReceiverFeedback& feedback,
                                    std::string* error)
{
    if (!ValidIdentity(feedback.roomId,
                       kMaximumScreenReceiverFeedbackRoomIdBytes) ||
        !ValidIdentity(feedback.senderDeviceId,
                       kMaximumScreenReceiverFeedbackDeviceIdBytes) ||
        feedback.sequence == 0 || feedback.screenShareGeneration == 0 ||
        feedback.sampleWindowMs < kMinimumScreenReceiverFeedbackWindowMs ||
        feedback.sampleWindowMs > kMaximumScreenReceiverFeedbackWindowMs ||
        feedback.decodedFrames > kMaximumScreenReceiverFeedbackWindowFrames ||
        feedback.droppedFrames > kMaximumScreenReceiverFeedbackWindowFrames ||
        feedback.frameWidth < 2 || feedback.frameHeight < 2 ||
        feedback.frameWidth > kMaximumScreenReceiverFeedbackFrameDimension ||
        feedback.frameHeight > kMaximumScreenReceiverFeedbackFrameDimension ||
        static_cast<std::uint64_t>(feedback.frameWidth) * feedback.frameHeight >
            kMaximumScreenReceiverFeedbackFramePixels ||
        !ValidTime(feedback.decodeTimeAvailable, feedback.windowDecodeTimeUs,
                   feedback.decodedFrames) ||
        !ValidTime(feedback.processingTimeAvailable, feedback.processingTimeUs,
                   feedback.decodedFrames) ||
        !ValidTime(feedback.jitterBufferAvailable, feedback.jitterBufferUs,
                   feedback.decodedFrames)) {
        return Fail(error, "Screen receiver feedback fields are invalid.");
    }
    if (error) {
        error->clear();
    }
    return true;
}

bool EncodeScreenReceiverFeedback(const ScreenReceiverFeedback& feedback,
                                  std::vector<std::uint8_t>* encoded,
                                  std::string* error)
{
    if (!encoded) {
        return Fail(error, "Screen receiver feedback output is required.");
    }
    if (!ValidateScreenReceiverFeedback(feedback, error)) {
        return false;
    }
    std::vector<std::uint8_t> output;
    output.reserve(kScreenReceiverFeedbackHeaderBytes + feedback.roomId.size() +
                   feedback.senderDeviceId.size());
    BinaryProtocolWriter writer(&output);
    writer.WriteBytes(kMagic);
    writer.WriteU8(kScreenReceiverFeedbackProtocolVersion);
    writer.WriteU8(kReportType);
    writer.WriteU16(static_cast<std::uint16_t>(
        (feedback.decodeTimeAvailable ? kDecodeAvailable : 0) |
        (feedback.processingTimeAvailable ? kProcessingAvailable : 0) |
        (feedback.jitterBufferAvailable ? kJitterAvailable : 0)));
    writer.WriteU64(feedback.sequence);
    writer.WriteU64(feedback.screenShareGeneration);
    writer.WriteU64(feedback.preferenceSequence);
    writer.WriteU32(feedback.sampleWindowMs);
    writer.WriteU32(feedback.decodedFrames);
    writer.WriteU32(feedback.droppedFrames);
    writer.WriteU32(feedback.frameWidth);
    writer.WriteU32(feedback.frameHeight);
    writer.WriteU64(feedback.windowDecodeTimeUs);
    writer.WriteU64(feedback.processingTimeUs);
    writer.WriteU64(feedback.jitterBufferUs);
    writer.WriteU16(static_cast<std::uint16_t>(feedback.roomId.size()));
    writer.WriteU16(static_cast<std::uint16_t>(feedback.senderDeviceId.size()));
    writer.WriteString(feedback.roomId);
    writer.WriteString(feedback.senderDeviceId);
    *encoded = std::move(output);
    return true;
}

bool DecodeScreenReceiverFeedback(std::span<const std::uint8_t> encoded,
                                  ScreenReceiverFeedback* feedback,
                                  std::string* error)
{
    if (!feedback || encoded.size() < kScreenReceiverFeedbackHeaderBytes ||
        encoded.size() > kMaximumScreenReceiverFeedbackMessageBytes ||
        !IsScreenReceiverFeedbackMessage(encoded)) {
        return Fail(error, "Screen receiver feedback header is invalid.");
    }
    BinaryProtocolReader reader(encoded.subspan(kMagic.size()));
    ScreenReceiverFeedback decoded;
    std::uint8_t version = 0;
    std::uint8_t type = 0;
    std::uint16_t flags = 0;
    std::uint16_t roomLength = 0;
    std::uint16_t deviceLength = 0;
    if (!reader.ReadU8(&version) ||
        version != kScreenReceiverFeedbackProtocolVersion ||
        !reader.ReadU8(&type) || type != kReportType ||
        !reader.ReadU16(&flags) || (flags & ~kKnownFlags) != 0 ||
        !reader.ReadU64(&decoded.sequence) ||
        !reader.ReadU64(&decoded.screenShareGeneration) ||
        !reader.ReadU64(&decoded.preferenceSequence) ||
        !reader.ReadU32(&decoded.sampleWindowMs) ||
        !reader.ReadU32(&decoded.decodedFrames) ||
        !reader.ReadU32(&decoded.droppedFrames) ||
        !reader.ReadU32(&decoded.frameWidth) ||
        !reader.ReadU32(&decoded.frameHeight) ||
        !reader.ReadU64(&decoded.windowDecodeTimeUs) ||
        !reader.ReadU64(&decoded.processingTimeUs) ||
        !reader.ReadU64(&decoded.jitterBufferUs) ||
        !reader.ReadU16(&roomLength) ||
        !reader.ReadU16(&deviceLength) || roomLength == 0 || deviceLength == 0 ||
        roomLength > kMaximumScreenReceiverFeedbackRoomIdBytes ||
        deviceLength > kMaximumScreenReceiverFeedbackDeviceIdBytes ||
        reader.remaining() != static_cast<std::size_t>(roomLength) + deviceLength ||
        !reader.ReadString(roomLength, &decoded.roomId) ||
        !reader.ReadString(deviceLength, &decoded.senderDeviceId) ||
        reader.remaining() != 0) {
        return Fail(error, "Screen receiver feedback payload is invalid.");
    }
    decoded.decodeTimeAvailable = (flags & kDecodeAvailable) != 0;
    decoded.processingTimeAvailable = (flags & kProcessingAvailable) != 0;
    decoded.jitterBufferAvailable = (flags & kJitterAvailable) != 0;
    if (!ValidateScreenReceiverFeedback(decoded, error)) {
        return false;
    }
    *feedback = std::move(decoded);
    return true;
}

bool AcceptScreenReceiverFeedbackSequence(
    ScreenReceiverFeedbackSequenceState* state,
    const ScreenReceiverFeedback& feedback,
    std::uint64_t currentGeneration,
    std::uint64_t currentPreferenceSequence)
{
    if (!state || currentGeneration == 0 ||
        feedback.screenShareGeneration != currentGeneration ||
        feedback.preferenceSequence != currentPreferenceSequence ||
        !ValidateScreenReceiverFeedback(feedback)) {
        return false;
    }
    const bool sameContext = state->screenShareGeneration == currentGeneration &&
        state->preferenceSequence == currentPreferenceSequence;
    if (sameContext && feedback.sequence <= state->lastSequence) {
        return false;
    }
    *state = {currentGeneration, currentPreferenceSequence, feedback.sequence};
    return true;
}

}  // namespace remote
