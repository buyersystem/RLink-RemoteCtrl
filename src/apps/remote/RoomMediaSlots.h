// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <array>
#include <string_view>

#include "src/protocol/DataChannelCatalog.h"

namespace remote::app {

inline constexpr std::array<const char*, 2> kRoomVideoSlots = {
    kScreenMainVideoSlot, kCameraMainVideoSlot};

inline bool IsRoomVideoSlot(std::string_view slot)
{
    return slot == kScreenMainVideoSlot || slot == kCameraMainVideoSlot;
}

}  // namespace remote::app
