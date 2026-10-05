// SPDX-License-Identifier: GPL-3.0-only
#include "InProcessSessionEngineInternal.h"
#include "src/protocol/DataChannelCatalog.h"
#include "src/protocol/ScreenReceiverFeedbackProtocol.h"
#include "src/core/ScreenStreamPolicy.h"
#include "src/core/ScreenFrameQualityPolicy.h"

namespace remote::app {
std::uint32_t InProcessSessionEngine::ScreenQualityDeficitShareFromProvider() const
{
    // Host callbacks run outside the engine mutex and never on the frame path.
    try {
        return options_.screenQualityDeficitShareProvider
            ? NormalizeScreenQualityDeficitShareHundredths(options_.screenQualityDeficitShareProvider())
            : kDefaultScreenQualityDeficitShareHundredths;
    } catch (...) {
        return kDefaultScreenQualityDeficitShareHundredths;
    }
}

SessionCommandResult InProcessSessionEngine::SetScreenQualityDeficitShare(std::uint32_t hundredths)
{
    const auto normalized = NormalizeScreenQualityDeficitShareHundredths(hundredths);
    std::lock_guard lock(mutex_);
    liveScreenQualityDeficitShare_ = normalized;
    if (webRtcSession_) webRtcSession_->SetScreenQualityDeficitShare(normalized);
    for (const auto& [id, pair] : roomPairs_) {
        if (pair && pair->session) pair->session->SetScreenQualityDeficitShare(normalized);
    }
    return {true, {}, {}};
}

SessionCommandResult InProcessSessionEngine::SetScreenVideoBitrateBpp(std::uint32_t hundredths)
{
    if (hundredths < kMinimumScreenVideoBitrateBppHundredths ||
        hundredths > kMaximumScreenVideoBitrateBppHundredths)
        return {false, "invalid_bpp", "The screen bandwidth coefficient must be between 0.03 and 0.50."};
    // Metadata only: RTC calls happen on stats completion outside the engine
    // lock. This also protects the uniquely owned direct session's lifetime.
    std::lock_guard lock(mutex_);
    if (webRtcSession_) webRtcSession_->SetScreenVideoBitrateBpp(hundredths);
    for (const auto& [id, pair] : roomPairs_) {
        if (pair && pair->session) pair->session->SetScreenVideoBitrateBpp(hundredths);
    }
    return {true, {}, {}};
}

bool InProcessSessionEngine::DispatchScreenReceiverFeedback(const std::string& pairId,
    const std::string& label, std::span<const std::uint8_t> payload)
{
    if (label != kTelemetryChannel || !IsScreenReceiverFeedbackMessage(payload)) return false;
    ScreenReceiverFeedback feedback;
    if (!DecodeScreenReceiverFeedback(payload, &feedback)) return true;
    std::lock_guard lock(mutex_);
    if (pairId.empty()) {
        if (!webRtcSession_ || snapshot_.state != SessionEngineState::kActive ||
            snapshot_.remoteControlRole != RemoteControlRole::kControlled ||
            feedback.roomId != snapshot_.sessionId || feedback.senderDeviceId != snapshot_.peerDeviceId ||
            !screenShare_.HasCaptureSource() || feedback.screenShareGeneration != snapshot_.screenShare.generation ||
            feedback.preferenceSequence != (directSession_.screenPreferenceApplied
                ? directSession_.screenPreference.sequence : 0)) return true;
        // Metadata setters never enter WebRTC; the engine protects the direct
        // session lifetime while the session applies its sequence/freshness guard.
        webRtcSession_->SetScreenSenderFeedbackContract(feedback.screenShareGeneration, feedback.preferenceSequence);
        (void)webRtcSession_->AcceptScreenReceiverFeedback(feedback);
    } else {
        const auto found = roomPairs_.find(pairId);
        if (found == roomPairs_.end() || !found->second || !found->second->session ||
            snapshot_.room.membership != RoomMembershipState::kActive ||
            snapshot_.room.screenShareState != RoomScreenShareState::kActive ||
            snapshot_.room.screenSharerDeviceId != snapshot_.localDeviceId ||
            feedback.roomId != snapshot_.room.roomId || feedback.roomId != found->second->roomId ||
            feedback.senderDeviceId != found->second->peerDeviceId ||
            feedback.screenShareGeneration != snapshot_.room.screenShareEpoch) return true;
        const auto preference = roomSession_.screenStreamPreferences_.find(pairId);
        const auto sequence = preference == roomSession_.screenStreamPreferences_.end() ? 0 : preference->second.sequence;
        if (feedback.preferenceSequence != sequence) return true;
        found->second->session->SetScreenSenderFeedbackContract(feedback.screenShareGeneration, sequence);
        (void)found->second->session->AcceptScreenReceiverFeedback(feedback);
    }
    return true;
}
}  // namespace remote::app
