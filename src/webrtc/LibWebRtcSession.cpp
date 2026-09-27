// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "LibWebRtcSession.Internal.h"

namespace remote {

void LibWebRtcSession::DetachRemoteVideoSink()
{
    SetRemoteVideoSink(nullptr);
}

}  // namespace remote
