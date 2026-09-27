// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <functional>
#include <string>

#include "api/peer_connection_interface.h"
#include "api/scoped_refptr.h"
#include "src/webrtc/IWebRtcSession.h"

namespace remote {

struct PeerNegotiatorCallbacks {
    std::function<
        webrtc::scoped_refptr<webrtc::PeerConnectionInterface>()>
        peerConnection;
    std::function<void(WebRtcSessionState)> changeState;
    std::function<void(OperationId, const SessionDescription&)>
        localDescriptionCompleted;
    std::function<void(OperationId)> completeOperation;
    std::function<void(OperationId, std::string, std::string)> failOperation;
};

class PeerNegotiator final {
public:
    explicit PeerNegotiator(PeerNegotiatorCallbacks callbacks);

    void CreateLocalDescription(
        webrtc::scoped_refptr<webrtc::PeerConnectionInterface> peer,
        OperationId operationId,
        SessionDescriptionType type,
        bool iceRestart = false) const;
    void ApplyRemoteDescription(
        webrtc::scoped_refptr<webrtc::PeerConnectionInterface> peer,
        OperationId operationId,
        const SessionDescription& description) const;
    void AddRemoteIceCandidate(
        webrtc::scoped_refptr<webrtc::PeerConnectionInterface> peer,
        OperationId operationId,
        const IceCandidate& candidate) const;

private:
    PeerNegotiatorCallbacks callbacks_;
};

}  // namespace remote
