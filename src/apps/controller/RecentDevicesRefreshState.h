// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QString>
#include <algorithm>
#include <vector>

#include "src/core/SessionEngineTypes.h"

namespace remote::controller {

// Only gates display preparation. Every session snapshot still reaches the
// business-state handlers. Explicit history writes/navigation bypass this gate.
class RecentDevicesRefreshState {
public:
    bool Accept(const QString& accountKey, const OwnedDevicesSnapshot& owned,
                SessionConnectivityState connectivity, bool engineReady,
                bool dark, bool force = false)
    {
        if (!force && initialized_ && accountKey_ == accountKey &&
            ownedRevision_ == owned.revision && loaded_ == owned.loaded &&
            connectivity_ == connectivity && engineReady_ == engineReady &&
            dark_ == dark && availability_.size() == owned.devices.size() &&
            std::equal(availability_.begin(), availability_.end(),
                       owned.devices.begin(),
                       [](const Availability& saved, const OwnedDeviceSnapshot& device) {
                           return saved.deviceId == device.deviceId &&
                                  saved.online == device.online &&
                                  saved.current == device.current;
                       })) {
            return false;
        }
        initialized_ = true;
        accountKey_ = accountKey;
        ownedRevision_ = owned.revision;
        loaded_ = owned.loaded;
        connectivity_ = connectivity;
        engineReady_ = engineReady;
        dark_ = dark;
        // Disconnect/recovery may change availability without changing the
        // server-owned revision. Compare the exact fields used by the cards.
        availability_.clear();
        availability_.reserve(owned.devices.size());
        for (const auto& device : owned.devices) {
            availability_.push_back({device.deviceId, device.online, device.current});
        }
        return true;
    }

private:
    struct Availability {
        std::string deviceId;
        bool online = false;
        bool current = false;
    };
    QString accountKey_;
    std::uint64_t ownedRevision_ = 0;
    std::vector<Availability> availability_;
    SessionConnectivityState connectivity_ = SessionConnectivityState::kNotConfigured;
    bool loaded_ = false;
    bool initialized_ = false;
    bool engineReady_ = false;
    bool dark_ = false;
};

}  // namespace remote::controller
