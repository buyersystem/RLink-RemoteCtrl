// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

class QLabel;
class QWidget;

namespace remote::controller::detail {

enum class RoomStatusIcon {
    kPerson,
    kScreen,
    kController,
    kNetwork,
    kSeats
};

QLabel* CreateRoomStatusIndicator(QWidget* parent, RoomStatusIcon type);

}  // namespace remote::controller::detail
