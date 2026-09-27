// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "LibWebRtcSession.Internal.h"

namespace remote {
using namespace webrtc_session_detail;

OperationId LibWebRtcSession::NextOperationId()
{
    return nextOperationId_.fetch_add(1, std::memory_order_relaxed);
}

webrtc::scoped_refptr<webrtc::PeerConnectionInterface>
LibWebRtcSession::PeerConnection() const
{
    std::lock_guard lock(mutex_);
    return peerConnection_;
}

}  // namespace remote
