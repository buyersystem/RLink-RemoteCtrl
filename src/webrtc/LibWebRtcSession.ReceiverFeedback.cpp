// SPDX-License-Identifier: GPL-3.0-only
#include "LibWebRtcSession.Internal.h"
#include "src/protocol/DataChannelCatalog.h"

#include <cmath>

namespace remote {
using namespace webrtc_session_detail;

void LibWebRtcSession::SetScreenReceiverFeedbackContext(const ScreenReceiverFeedback& context)
{
    std::lock_guard lock(mutex_);
    if (receiverFeedbackContext_.roomId == context.roomId &&
        receiverFeedbackContext_.senderDeviceId == context.senderDeviceId &&
        receiverFeedbackContext_.screenShareGeneration == context.screenShareGeneration &&
        receiverFeedbackContext_.preferenceSequence == context.preferenceSequence) return;
    receiverFeedbackContext_ = context;
    ++receiverFeedbackEpoch_;
    // A temporary pause may return to the same generation/preference contract.
    // Its sender retains the last sequence, so keep ordering monotonic for
    // this session even when the sampling baseline is replaced.
    receiverFeedbackLastSentMs_ = receiverFeedbackLastSampleMs_ = 0;
    receiverFeedbackPreviousStatsId_.clear();
}

void LibWebRtcSession::SetScreenSenderFeedbackContract(std::uint64_t generation,
    std::uint64_t preferenceSequence)
{
    std::lock_guard lock(mutex_);
    if (receiverFeedbackExpectedGeneration_ == generation &&
        receiverFeedbackExpectedPreference_ == preferenceSequence) return;
    receiverFeedbackExpectedGeneration_ = generation;
    receiverFeedbackExpectedPreference_ = preferenceSequence;
    receiverFeedback_ = {};
    receiverFeedbackReceivedAtMs_ = 0;
}

bool LibWebRtcSession::AcceptScreenReceiverFeedback(const ScreenReceiverFeedback& feedback)
{
    if (!ValidateScreenReceiverFeedback(feedback)) return false;
    std::lock_guard lock(mutex_);
    if (!receiverFeedbackExpectedGeneration_ ||
        feedback.screenShareGeneration != receiverFeedbackExpectedGeneration_ ||
        feedback.preferenceSequence != receiverFeedbackExpectedPreference_ ||
        feedback.sequence <= receiverFeedback_.sequence) return false;
    receiverFeedback_ = feedback;
    receiverFeedbackReceivedAtMs_ = SteadyNowMs();
    return true;
}

void LibWebRtcSession::SendScreenReceiverFeedback(const WebRtcSessionStatsSnapshot& snapshot,
    std::uint64_t feedbackEpoch)
{
    const auto found = std::find_if(snapshot.rtpStreams.begin(), snapshot.rtpStreams.end(),
        [](const auto& stream) { return stream.direction == RtpStreamDirection::kInbound &&
            stream.kind == "video" && stream.slot == kScreenMainVideoSlot; });
    if (found == snapshot.rtpStreams.end()) return;
    ScreenReceiverFeedback feedback;
    {
        std::lock_guard lock(mutex_);
        if (receiverFeedbackEpoch_ != feedbackEpoch || receiverFeedbackContext_.roomId.empty() ||
            !receiverFeedbackContext_.screenShareGeneration) return;
        const auto timestamp = snapshot.transport.receivedAtSteadyMs;
        if (timestamp <= receiverFeedbackLastSampleMs_) return;
        const auto window = timestamp - receiverFeedbackLastSampleMs_;
        const bool newBaseline = !receiverFeedbackLastSampleMs_ ||
            receiverFeedbackPreviousStatsId_ != found->statsId ||
            found->framesDecoded < receiverFeedbackPreviousDecoded_ ||
            found->framesDropped < receiverFeedbackPreviousDropped_ ||
            !found->receiverFrameCountersAvailable;
        const auto decoded = found->framesDecoded - receiverFeedbackPreviousDecoded_;
        const auto dropped = found->framesDropped - receiverFeedbackPreviousDropped_;
        receiverFeedbackLastSampleMs_ = found->receiverFrameCountersAvailable ? timestamp : 0;
        receiverFeedbackPreviousStatsId_ = found->statsId;
        receiverFeedbackPreviousDecoded_ = found->framesDecoded;
        receiverFeedbackPreviousDropped_ = found->framesDropped;
        if (newBaseline || window < kMinimumScreenReceiverFeedbackWindowMs ||
            window > kMaximumScreenReceiverFeedbackWindowMs || !decoded ||
            (receiverFeedbackLastSentMs_ && timestamp - receiverFeedbackLastSentMs_ < 900) ||
            receiverFeedbackNextSequence_ == (std::numeric_limits<std::uint64_t>::max)()) return;
        feedback = receiverFeedbackContext_;
        feedback.sequence = ++receiverFeedbackNextSequence_;
        feedback.sampleWindowMs = static_cast<std::uint32_t>(window);
        feedback.decodedFrames = decoded;
        feedback.droppedFrames = dropped;
        feedback.frameWidth = found->frameWidth;
        feedback.frameHeight = found->frameHeight;
        const auto convert = [](bool available, double ms, bool& flag, std::uint64_t& us) {
            flag = available && std::isfinite(ms) && ms >= 0 && ms <= 10000;
            us = flag ? static_cast<std::uint64_t>(std::ceil(ms * 1000)) : 0;
        };
        convert(found->windowDecodeTimeAvailable, found->windowDecodeTimeMs,
            feedback.decodeTimeAvailable, feedback.windowDecodeTimeUs);
        convert(found->windowProcessingDelayAvailable, found->windowProcessingDelayMs,
            feedback.processingTimeAvailable, feedback.processingTimeUs);
        convert(found->windowJitterBufferDelayAvailable, found->windowJitterBufferDelayMs,
            feedback.jitterBufferAvailable, feedback.jitterBufferUs);
        receiverFeedbackLastSentMs_ = timestamp;
    }
    std::vector<std::uint8_t> encoded;
    if (!EncodeScreenReceiverFeedback(feedback, &encoded)) return;
    const auto buffered = DataChannelBufferedAmount(std::string(kTelemetryChannel));
    if (!buffered || *buffered > 16384) return;
    {
        std::lock_guard lock(mutex_);
        if (receiverFeedbackEpoch_ != feedbackEpoch) return;
    }
    // Unordered, no retransmission, no reliable fallback and no retry queue.
    (void)SendData(std::string(kTelemetryChannel), encoded, true);
}
}  // namespace remote
