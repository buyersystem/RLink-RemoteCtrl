// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "QtWebSocketSignalingClient.Internal.h"

namespace remote {
using namespace signaling_client_detail;

SignalingOperationResult QtWebSocketSignalingClient::Impl::RequestOwnedDevices()
{
    if (auto ready = RequireRegistered(); !ready.accepted) {
        return ready;
    }
    return SendEnvelope(QStringLiteral("my_devices_request"), {}, {});
}

}  // namespace remote
