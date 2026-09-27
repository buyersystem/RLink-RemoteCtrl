// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "LibWebRtcSession.Internal.h"
#include "PeerNegotiator.h"

namespace remote {
using namespace webrtc_session_detail;

OperationId LibWebRtcSession::CreateOffer()
{
    const OperationId operationId = NextOperationId();
    peerNegotiator_->CreateLocalDescription(
        PeerConnection(), operationId, SessionDescriptionType::kOffer);
    return operationId;
}

OperationId LibWebRtcSession::CreateIceRestartOffer()
{
    const OperationId operationId = NextOperationId();
    peerNegotiator_->CreateLocalDescription(
        PeerConnection(), operationId, SessionDescriptionType::kOffer, true);
    return operationId;
}

OperationId LibWebRtcSession::CreateAnswer()
{
    const OperationId operationId = NextOperationId();
    peerNegotiator_->CreateLocalDescription(
        PeerConnection(), operationId, SessionDescriptionType::kAnswer);
    return operationId;
}

OperationId LibWebRtcSession::ApplyRemoteDescription(
    const SessionDescription& description)
{
    const OperationId operationId = NextOperationId();
    peerNegotiator_->ApplyRemoteDescription(
        PeerConnection(), operationId, description);
    return operationId;
}

OperationId LibWebRtcSession::AddRemoteIceCandidate(
    const IceCandidate& candidate)
{
    const OperationId operationId = NextOperationId();
    peerNegotiator_->AddRemoteIceCandidate(
        PeerConnection(), operationId, candidate);
    return operationId;
}

}  // namespace remote
