// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "DirectSessionRuntimeState.h"
#include "src/core/ISessionEngine.h"
#include "src/webrtc/IWebRtcSession.h"

namespace remote::app {

class InProcessSessionEngine;

struct DirectSessionStartPlan {
    DirectSessionConnectRequest request;
    SessionOrigin origin = SessionOrigin::kManualDeviceId;
    std::vector<std::string> permissions;
};

// Owns direct-session command policy and lifecycle transitions. Transport and
// media callbacks are migrated here in later batches while the engine remains
// the public ISessionEngine facade.
class DirectSessionCoordinator final : public DirectSessionRuntimeState {
public:
    [[nodiscard]] SessionCommandResult PrepareOutgoingStart(
        const DirectSessionConnectRequest& request,
        DirectSessionStartPlan* plan) const;
    void ApplyOutgoingStart(
        SessionEngineSnapshot* snapshot,
        const DirectSessionStartPlan& plan) const;

    [[nodiscard]] SessionCommandResult ValidateIncomingDecision(
        const SessionEngineSnapshot& snapshot,
        const std::string& sessionId,
        bool rejecting) const;
    void ApplyIncomingAccepted(SessionEngineSnapshot* snapshot) const;

    void Reset();

private:
    friend class InProcessSessionEngine;

    bool localIsOfferer_ = false;
    bool offerNegotiationStarted_ = false;
    bool sessionCloseRequested_ = false;
    bool sessionEndSignalSent_ = false;
    bool cancelWhenSessionIdKnown_ = false;
    bool serverSessionActive_ = false;
    bool signalingRecoveryPending_ = false;
    bool peerSignalingSuspended_ = false;
    std::string sessionRecoveryToken_;
    std::optional<SessionDescription> pendingRemoteDescription_;
    std::vector<IceCandidate> pendingRemoteCandidates_;
};

}  // namespace remote::app
