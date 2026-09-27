// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "FileTransferController.h"
#include "FileTransferControllerInternal.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <random>
#include <sstream>
#include <unordered_set>
#include <utility>

namespace remote::app {
void FileTransferController::PublishSnapshots()
{
    IFileTransferControllerObserver* observer = nullptr;
    std::vector<FileTransferSnapshot> snapshots;
    {
        std::lock_guard lock(mutex_);
        observer = observer_;
        if (!observer) {
            return;
        }
        snapshots.reserve(transfers_.size());
        for (const auto& [id, transfer] : transfers_) {
            snapshots.push_back(transfer->snapshot);
        }
    }
    std::sort(snapshots.begin(), snapshots.end(),
              [](const auto& left, const auto& right) {
                  return left.displayOrder > right.displayOrder;
              });
    observer->OnFileTransfersChanged(snapshots);
}

std::string FileTransferController::GenerateTransferId()
{
    std::random_device random;
    for (;;) {
        std::array<std::uint32_t, 4> words{};
        for (auto& word : words) {
            word = random();
        }
        std::ostringstream stream;
        stream << 'f' << std::hex << std::setfill('0');
        for (const auto word : words) {
            stream << std::setw(8) << word;
        }
        const std::string value = stream.str();
        std::lock_guard lock(mutex_);
        if (!transfers_.contains(value)) {
            return value;
        }
    }
}

}  // namespace remote::app
