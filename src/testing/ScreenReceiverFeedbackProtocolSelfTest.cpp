// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "src/protocol/ScreenReceiverFeedbackProtocol.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

using namespace remote;
int checks = 0;
int failures = 0;

void Check(bool result, const char* name)
{
    ++checks;
    if (!result) {
        ++failures;
    }
    std::cout << name << '=' << (result ? "PASS" : "FAIL") << '\n';
}

ScreenReceiverFeedback Report()
{
    ScreenReceiverFeedback value;
    value.roomId = "room";
    value.senderDeviceId = "viewer";
    value.sequence = 5;
    value.screenShareGeneration = 3;
    value.preferenceSequence = 2;
    value.sampleWindowMs = 1000;
    value.decodedFrames = 60;
    value.droppedFrames = 1;
    value.frameWidth = 1920;
    value.frameHeight = 1080;
    value.decodeTimeAvailable = true;
    value.processingTimeAvailable = true;
    value.jitterBufferAvailable = true;
    value.windowDecodeTimeUs = 1500;
    value.processingTimeUs = 3200;
    value.jitterBufferUs = 12000;
    return value;
}

bool RejectsWithoutChanging(const std::vector<std::uint8_t>& bytes)
{
    auto destination = Report();
    destination.sequence = 999;
    destination.roomId = "untouched";
    const auto before = destination;
    return !DecodeScreenReceiverFeedback(bytes, &destination) &&
        destination == before;
}

void PutU32(std::vector<std::uint8_t>& bytes, std::size_t offset,
            std::uint32_t value)
{
    for (int index = 0; index < 4; ++index) {
        bytes[offset + index] = static_cast<std::uint8_t>(
            value >> (24 - index * 8));
    }
}

void RoundTrips()
{
    const auto original = Report();
    std::vector<std::uint8_t> encoded;
    std::string error = "old error";
    Check(EncodeScreenReceiverFeedback(original, &encoded, &error) &&
              error.empty() && encoded.size() == 90,
          "BOUNDED_HEADER_AND_IDENTITIES");
    ScreenReceiverFeedback decoded;
    Check(DecodeScreenReceiverFeedback(encoded, &decoded, &error) &&
              decoded == original && error.empty(), "ALL_FIELDS_ROUND_TRIP");
    Check(encoded[0] == 'R' && encoded[1] == 'C' && encoded[2] == 'R' &&
              encoded[3] == 'F' && encoded[4] == 1 && encoded[5] == 1 &&
              encoded[6] == 0 && encoded[7] == 7 && encoded[15] == 5 &&
              encoded[23] == 3 && encoded[31] == 2,
          "DISTINCT_MAGIC_VERSION_AND_NETWORK_BYTE_ORDER");

    bool allFlags = true;
    for (int flags = 0; flags < 8; ++flags) {
        auto report = original;
        report.decodeTimeAvailable = (flags & 1) != 0;
        report.processingTimeAvailable = (flags & 2) != 0;
        report.jitterBufferAvailable = (flags & 4) != 0;
        if (!report.decodeTimeAvailable) report.windowDecodeTimeUs = 0;
        if (!report.processingTimeAvailable) report.processingTimeUs = 0;
        if (!report.jitterBufferAvailable) report.jitterBufferUs = 0;
        allFlags &= EncodeScreenReceiverFeedback(report, &encoded) &&
            DecodeScreenReceiverFeedback(encoded, &decoded) && report == decoded;
    }
    Check(allFlags, "EVERY_AVAILABILITY_COMBINATION");

    auto largest = original;
    largest.roomId.assign(kMaximumScreenReceiverFeedbackRoomIdBytes, 'r');
    largest.senderDeviceId.assign(kMaximumScreenReceiverFeedbackDeviceIdBytes, 'd');
    largest.sequence = std::numeric_limits<std::uint64_t>::max();
    largest.preferenceSequence = 0;
    largest.sampleWindowMs = kMaximumScreenReceiverFeedbackWindowMs;
    largest.decodedFrames = largest.droppedFrames =
        kMaximumScreenReceiverFeedbackWindowFrames;
    largest.frameWidth = 8192;
    largest.frameHeight = 8192;
    largest.windowDecodeTimeUs = largest.processingTimeUs = largest.jitterBufferUs =
        kMaximumScreenReceiverFeedbackTimeUs;
    Check(EncodeScreenReceiverFeedback(largest, &encoded) &&
              encoded.size() == kMaximumScreenReceiverFeedbackMessageBytes &&
              DecodeScreenReceiverFeedback(encoded, &decoded) && decoded == largest,
          "MAXIMUM_BOUNDARIES_AND_INITIAL_PREFERENCE");

    auto idle = original;
    idle.decodedFrames = 0;
    idle.droppedFrames = 0;
    idle.decodeTimeAvailable = idle.processingTimeAvailable =
        idle.jitterBufferAvailable = false;
    idle.windowDecodeTimeUs = idle.processingTimeUs = idle.jitterBufferUs = 0;
    idle.sampleWindowMs = kMinimumScreenReceiverFeedbackWindowMs;
    Check(EncodeScreenReceiverFeedback(idle, &encoded) &&
              DecodeScreenReceiverFeedback(encoded, &decoded) && decoded == idle,
          "IDLE_IS_METADATA_WITHOUT_FABRICATED_TIMINGS");
    auto zeroTime = original;
    zeroTime.windowDecodeTimeUs = zeroTime.processingTimeUs = zeroTime.jitterBufferUs = 0;
    Check(ValidateScreenReceiverFeedback(zeroTime), "MEASURED_ZERO_TIMING_VALID");
}

void InvalidFields()
{
    const auto valid = Report();
    const auto rejects = [](const ScreenReceiverFeedback& report) {
        std::vector<std::uint8_t> output{9, 8, 7};
        const auto before = output;
        std::string error;
        return !ValidateScreenReceiverFeedback(report) &&
            !EncodeScreenReceiverFeedback(report, &output, &error) &&
            output == before && !error.empty();
    };
    auto bad = valid;
    bad.roomId.clear(); Check(rejects(bad), "EMPTY_ROOM_REJECTED");
    bad = valid; bad.senderDeviceId.clear(); Check(rejects(bad), "EMPTY_DEVICE_REJECTED");
    bad = valid; bad.roomId.assign(129, 'r'); Check(rejects(bad), "OVERSIZE_ROOM_REJECTED");
    bad = valid; bad.senderDeviceId.assign(65, 'd'); Check(rejects(bad), "OVERSIZE_DEVICE_REJECTED");
    bad = valid; bad.roomId = std::string("a\0b", 3); Check(rejects(bad), "EMBEDDED_NULL_REJECTED");
    bad = valid; bad.senderDeviceId = "a\nb"; Check(rejects(bad), "CONTROL_CHARACTER_REJECTED");
    bad = valid; bad.sequence = 0; Check(rejects(bad), "ZERO_SEQUENCE_REJECTED");
    bad = valid; bad.screenShareGeneration = 0; Check(rejects(bad), "ZERO_GENERATION_REJECTED");
    bad = valid; bad.sampleWindowMs = 99; Check(rejects(bad), "SHORT_WINDOW_REJECTED");
    bad = valid; bad.sampleWindowMs = 10001; Check(rejects(bad), "LONG_WINDOW_REJECTED");
    bad = valid; bad.decodedFrames = 65537; Check(rejects(bad), "OVERSIZE_DECODE_COUNT_REJECTED");
    bad = valid; bad.droppedFrames = 65537; Check(rejects(bad), "OVERSIZE_DROP_COUNT_REJECTED");
    bad = valid; bad.frameWidth = 0; Check(rejects(bad), "ZERO_WIDTH_REJECTED");
    bad = valid; bad.frameHeight = 1; Check(rejects(bad), "INVALID_HEIGHT_REJECTED");
    bad = valid; bad.frameWidth = 16385; Check(rejects(bad), "OVERSIZE_DIMENSION_REJECTED");
    bad = valid; bad.frameWidth = bad.frameHeight = 16384; Check(rejects(bad), "OVERSIZE_PIXEL_AREA_REJECTED");
    bad = valid; bad.windowDecodeTimeUs = 10000001; Check(rejects(bad), "OVERSIZE_DECODE_TIME_REJECTED");
    bad = valid; bad.processingTimeUs = std::numeric_limits<std::uint64_t>::max(); Check(rejects(bad), "OVERFLOW_PROCESSING_TIME_REJECTED");
    bad = valid; bad.jitterBufferUs = 10000001; Check(rejects(bad), "OVERSIZE_JITTER_TIME_REJECTED");
    bad = valid; bad.decodeTimeAvailable = false; Check(rejects(bad), "MISSING_DECODE_FLAG_REQUIRES_ZERO");
    bad = valid; bad.processingTimeAvailable = false; Check(rejects(bad), "MISSING_PROCESSING_FLAG_REQUIRES_ZERO");
    bad = valid; bad.jitterBufferAvailable = false; Check(rejects(bad), "MISSING_JITTER_FLAG_REQUIRES_ZERO");
    bad = valid; bad.decodedFrames = 0; Check(rejects(bad), "NO_FRAMES_CANNOT_PROVIDE_TIMING_EVIDENCE");
    Check(!EncodeScreenReceiverFeedback(valid, nullptr), "NULL_ENCODER_OUTPUT_REJECTED");
}

void InvalidWire()
{
    std::vector<std::uint8_t> encoded;
    EncodeScreenReceiverFeedback(Report(), &encoded);
    bool truncations = true;
    for (std::size_t size = 0; size < encoded.size(); ++size) {
        truncations &= RejectsWithoutChanging({encoded.begin(), encoded.begin() + size});
    }
    Check(truncations, "ALL_TRUNCATIONS_REJECT_WITHOUT_MUTATION");
    auto bad = encoded; bad[0] = 'X'; Check(RejectsWithoutChanging(bad), "WRONG_MAGIC_REJECTED");
    bad = encoded; bad[4] = 2; Check(RejectsWithoutChanging(bad), "FUTURE_VERSION_REJECTED");
    Check(IsScreenReceiverFeedbackMessage(bad), "BAD_VERSION_STILL_DISPATCHES_TO_FAMILY");
    bad = encoded; bad[5] = 2; Check(RejectsWithoutChanging(bad), "UNKNOWN_TYPE_REJECTED");
    bad = encoded; bad[7] |= 8; Check(RejectsWithoutChanging(bad), "UNKNOWN_LOW_FLAG_REJECTED");
    bad = encoded; bad[6] = 128; Check(RejectsWithoutChanging(bad), "UNKNOWN_HIGH_FLAG_REJECTED");
    bad = encoded; bad.push_back(0); Check(RejectsWithoutChanging(bad), "TRAILING_DATA_REJECTED");
    bad = encoded; bad.resize(273, 0); Check(RejectsWithoutChanging(bad), "OVERSIZE_PACKET_REJECTED");
    bad = encoded; bad[76] = bad[77] = 0; Check(RejectsWithoutChanging(bad), "WIRE_EMPTY_ID_REJECTED");
    bad = encoded; bad[77] = 129; Check(RejectsWithoutChanging(bad), "WIRE_OVERSIZE_ID_REJECTED");
    bad = encoded; PutU32(bad, 32, 0); Check(RejectsWithoutChanging(bad), "WIRE_INVALID_WINDOW_REJECTED");
    bad = encoded; PutU32(bad, 44, 0); Check(RejectsWithoutChanging(bad), "WIRE_INVALID_DIMENSION_REJECTED");
    bad = encoded; bad[7] = 0; Check(RejectsWithoutChanging(bad), "WIRE_UNKNOWN_TIMINGS_REJECTED");
    Check(!DecodeScreenReceiverFeedback(encoded, nullptr), "NULL_DECODER_OUTPUT_REJECTED");
    Check(!IsScreenReceiverFeedbackMessage(std::span<const std::uint8_t>(encoded).first(3)),
          "SHORT_MAGIC_CANNOT_DISPATCH");
}

void SequenceChecks()
{
    auto report = Report();
    ScreenReceiverFeedbackSequenceState state;
    Check(AcceptScreenReceiverFeedbackSequence(&state, report, 3, 2) &&
              state.lastSequence == 5, "AUTHENTICATED_CURRENT_CONTEXT_ACCEPTED");
    Check(!AcceptScreenReceiverFeedbackSequence(&state, report, 3, 2) &&
              state.lastSequence == 5, "DUPLICATE_SEQUENCE_REJECTED");
    report.sequence = 4;
    Check(!AcceptScreenReceiverFeedbackSequence(&state, report, 3, 2) &&
              state.lastSequence == 5, "REORDERED_SEQUENCE_REJECTED");
    report.sequence = 6;
    Check(AcceptScreenReceiverFeedbackSequence(&state, report, 3, 2) &&
              state.lastSequence == 6, "NEWER_SEQUENCE_ACCEPTED");
    report.screenShareGeneration = 4;
    Check(!AcceptScreenReceiverFeedbackSequence(&state, report, 3, 2) &&
              state.screenShareGeneration == 3, "PEER_CANNOT_ADVANCE_GENERATION");
    Check(AcceptScreenReceiverFeedbackSequence(&state, report, 4, 2),
          "LOCAL_GENERATION_CHANGE_RESETS_ORDERING");
    report.preferenceSequence = 3;
    Check(!AcceptScreenReceiverFeedbackSequence(&state, report, 4, 2) &&
              state.preferenceSequence == 2, "PEER_CANNOT_ADVANCE_PREFERENCE");
    report.sequence = 1;
    Check(AcceptScreenReceiverFeedbackSequence(&state, report, 4, 3) &&
              state.lastSequence == 1, "LOCAL_PREFERENCE_CHANGE_RESETS_ORDERING");
    report.screenShareGeneration = 3;
    Check(!AcceptScreenReceiverFeedbackSequence(&state, report, 4, 3) &&
              state.screenShareGeneration == 4, "OLD_GENERATION_REJECTED");
    report.screenShareGeneration = 4;
    report.preferenceSequence = 2;
    Check(!AcceptScreenReceiverFeedbackSequence(&state, report, 4, 3),
          "OLD_PREFERENCE_REJECTED");
    report.preferenceSequence = 0;
    Check(AcceptScreenReceiverFeedbackSequence(&state, report, 4, 0),
          "INITIAL_DEFAULT_PREFERENCE_ACCEPTED");
    report.sequence = 0;
    Check(!AcceptScreenReceiverFeedbackSequence(&state, report, 4, 0) &&
              state.lastSequence == 1, "INVALID_REPORT_DOES_NOT_POISON_STATE");
    Check(!AcceptScreenReceiverFeedbackSequence(nullptr, Report(), 3, 2),
          "NULL_SEQUENCE_STATE_REJECTED");
    Check(!AcceptScreenReceiverFeedbackSequence(&state, Report(), 0, 2),
          "INVALID_LOCAL_GENERATION_REJECTED");
}

}  // namespace

int main()
{
    RoundTrips();
    InvalidFields();
    InvalidWire();
    SequenceChecks();
    std::cout << "SCREEN_RECEIVER_FEEDBACK_CHECKS=" << checks
              << " FAILURES=" << failures << '\n';
    return failures == 0 ? 0 : 1;
}
