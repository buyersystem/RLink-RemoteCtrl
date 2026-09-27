// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "LibWebRtcSession.Internal.h"
#include "DataChannelManager.h"

namespace remote {
using namespace webrtc_session_detail;

OperationId LibWebRtcSession::CreateDataChannels(
    const std::vector<DataChannelSpec>& channels)
{
    const OperationId operationId = NextOperationId();
    auto peer = PeerConnection();
    if (!peer) {
        FailOperation(operationId, "session_not_started",
                      "Cannot create DataChannels before Start.");
        return operationId;
    }

    if (const auto error = dataChannelManager_->Create(peer, channels)) {
        FailOperation(operationId, error->code, error->message);
        return operationId;
    }

    CompleteOperation(operationId);
    return operationId;
}

SendResult LibWebRtcSession::SendData(const std::string& channelName,
                                      std::span<const std::uint8_t> data,
                                      bool binary)
{
    if (!PeerConnection()) {
        return SendResult::kSessionNotStarted;
    }

    return dataChannelManager_->Send(channelName, data, binary);
}

std::optional<std::uint64_t>
LibWebRtcSession::DataChannelBufferedAmount(
    const std::string& channelName) const
{
    if (!PeerConnection()) {
        return std::nullopt;
    }

    return dataChannelManager_->BufferedAmount(channelName);
}

}  // namespace remote
