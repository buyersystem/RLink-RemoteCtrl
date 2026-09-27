// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "InProcessSessionEngine.h"

namespace remote::app {

bool InProcessSessionEngine::ShouldBoostDesktopCaptureForInput(
    const RemoteInputEvent& event)
{
    switch (event.type) {
    case RemoteInputMessageType::kMouseMove:
        // Plain hover movement should not keep the desktop capturer boosted.
        // During a drag, every move extends the short 50 ms capture window.
        return event.pressedMouseButtons != 0;
    case RemoteInputMessageType::kMouseButton:
    case RemoteInputMessageType::kMouseWheel:
    case RemoteInputMessageType::kKey:
    case RemoteInputMessageType::kReleaseAll:
        return true;
    default:
        return false;
    }
}

void InProcessSessionEngine::PublishSnapshot()
{
    ISessionEngineObserver* observer = nullptr;
    SessionEngineSnapshot snapshot;
    {
        std::lock_guard lock(mutex_);
        observer = observer_;
        snapshot = snapshot_;
    }
    if (observer) {
        observer->OnSessionEngineSnapshot(snapshot);
    }
}

}  // namespace remote::app
