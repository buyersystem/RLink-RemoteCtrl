// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "QtWebSocketSignalingClient.Internal.h"

namespace remote {
using namespace signaling_client_detail;

void QtWebSocketSignalingClient::Impl::DispatchMessage(
    const QString& type, const std::string& sessionId,
    const QJsonObject& payload)
{
    if (DispatchConnectionMessage(type, sessionId, payload) ||
        DispatchDirectSessionMessage(type, sessionId, payload) ||
        DispatchRoomMessage(type, sessionId, payload) ||
        DispatchTransportMessage(type, sessionId, payload)) {
        return;
    }
    NotifyError("unknown_signaling_message_type",
                "An unknown signaling message type was received.");
}

}  // namespace remote
