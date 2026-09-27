// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include "FileTransferController.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <string>
#include <utility>

namespace remote::app {

inline constexpr int kControlBackpressureRetryCount = 200;
inline constexpr auto kBackpressureRetryDelay =
    std::chrono::milliseconds(10);
inline constexpr auto kFilePayloadBackpressureRetryDelay =
    std::chrono::milliseconds(20);
inline constexpr std::size_t kFileTransferChunkBytes = 60 * 1024;
inline constexpr std::uint64_t kFileTransferSendBurstBytes = 1024 * 1024;
inline constexpr std::uint64_t kFileTransferMaximumInFlightBytes =
    2 * 1024 * 1024;
inline constexpr std::uint64_t kFileTransferProgressAckBytes = 256 * 1024;
inline constexpr auto kFileTransferProgressAckInterval =
    std::chrono::milliseconds(100);
inline constexpr auto kFileTransferProgressPublishInterval =
    std::chrono::milliseconds(100);
inline constexpr auto kFileTransferStallTimeout = std::chrono::seconds(15);
inline constexpr auto kIncomingFileTransferStallTimeout =
    std::chrono::seconds(60);
inline constexpr std::uint32_t kMaximumFileResyncAttempts = 3;
inline constexpr auto kFileTransferStallCheckInterval =
    std::chrono::milliseconds(500);
inline constexpr auto kFileTransferRateWindow = std::chrono::seconds(3);
inline constexpr auto kFileTransferRateRefreshInterval =
    std::chrono::milliseconds(500);

inline bool IsTerminal(FileTransferState state)
{
    return state == FileTransferState::kCompleted ||
           state == FileTransferState::kRejected ||
           state == FileTransferState::kPaused ||
           state == FileTransferState::kCanceled ||
           state == FileTransferState::kFailed;
}

inline bool IsFileTransferBackpressure(const SessionCommandResult& result)
{
    return result.errorCode == "room_file_transfer_backpressure" ||
           result.errorCode == "direct_file_transfer_backpressure";
}

inline FileTransferCommandResult Accepted(std::string transferId)
{
    FileTransferCommandResult result;
    result.accepted = true;
    result.transferId = std::move(transferId);
    return result;
}

inline FileTransferCommandResult Rejected(std::string errorCode,
                                          std::string errorMessage)
{
    FileTransferCommandResult result;
    result.errorCode = std::move(errorCode);
    result.errorMessage = std::move(errorMessage);
    return result;
}

struct FileTransferController::TransferRecord {
    struct RateSample {
        std::chrono::steady_clock::time_point at;
        std::uint64_t bytes = 0;
    };

    FileTransferSnapshot snapshot;
    std::filesystem::path sourcePath;
    FileTransferDestination destination;
    std::array<std::uint8_t, 32> sha256{};
    std::uint64_t nextSendOffset = 0;
    std::uint64_t receiverCommittedOffset = 0;
    std::uint64_t lastAcknowledgedOffset = 0;
    std::chrono::steady_clock::time_point progressPublishedAt{};
    std::chrono::steady_clock::time_point lastAcknowledgedAt{};
    std::chrono::steady_clock::time_point lastProgressAt{};
    std::deque<RateSample> rateSamples;
    std::chrono::steady_clock::time_point lastRatePublishedAt{};
    bool replaceExisting = false;
    bool interruptedByNetwork = false;
    bool sendBackpressured = false;
    std::uint32_t resyncAttempts = 0;
};

}  // namespace remote::app
