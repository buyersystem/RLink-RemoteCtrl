// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "InProcessSessionEngine.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "InProcessSessionEngineInternal.h"
#include "SessionDataChannelPolicy.h"
#include "SessionDiagnosticsFormatting.h"
#include "VideoPipelinePreferenceNames.h"
#include "src/core/RemoteInputTelemetry.h"
#include "src/protocol/ClipboardProtocol.h"
#include "src/protocol/DataChannelCatalog.h"
#include "src/protocol/FileTransferProtocol.h"
#include "src/protocol/RemoteInputProtocol.h"
#include "src/protocol/RoomMemberControlProtocol.h"
#include "src/protocol/ScreenShareControlProtocol.h"
#include "src/webrtc/LibWebRtcSession.h"

namespace remote::app {
namespace {

SessionCommandResult Failure(std::string code, std::string message)
{
    return {false, std::move(code), std::move(message)};
}

}  // namespace

void InProcessSessionEngine::OnRoomPairDataMessage(
    const std::string& pairId,
    const std::string& label,
    std::span<const std::uint8_t> payload,
    bool binary)
{
    if (!binary) {
        return;
    }
    if (DispatchScreenReceiverFeedback(pairId, label, payload)) return;
    if (DispatchRemoteCursorData(pairId, label, payload) ||
        DispatchRoomPairTransferData(pairId, label, payload)) {
        return;
    }
    const bool fastChannel = label == kInputFastChannel;
    const bool reliableChannel = label == kControlReliableChannel;
    if (!fastChannel && !reliableChannel) {
        return;
    }
    if (reliableChannel) {
        DispatchRoomPairReliableData(pairId, payload);
        return;
    }
    DispatchRoomPairInputData(pairId, payload, fastChannel);
}

}  // namespace remote::app
