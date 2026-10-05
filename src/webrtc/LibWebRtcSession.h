// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"
#include "api/rtc_error.h"
#include "api/rtp_transceiver_interface.h"
#include "api/scoped_refptr.h"
#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"
#include "src/core/ScreenNetworkPolicy.h"
#include "src/core/ScreenFrameQualityPolicy.h"
#include "src/webrtc/IWebRtcSession.h"
#include "src/webrtc/ScreenContentPolicy.h"
#include "src/protocol/ScreenReceiverFeedbackProtocol.h"
#include "media_intelligence/core/CalibratedStreamQualityModel.h"
#include "media_intelligence/core/H264ReferenceQualityModel.h"

namespace remote {

class PeerConnectionStatsCollector;
class GoogCcTelemetryState;
class DataChannelManager;
class MediaSlotManager;
class PeerNegotiator;
struct ScreenStreamPolicyResult;

class LibWebRtcSession final : public IWebRtcSession,
                               public webrtc::PeerConnectionObserver {
public:
    explicit LibWebRtcSession(
        webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory);
    ~LibWebRtcSession() override;

    LibWebRtcSession(const LibWebRtcSession&) = delete;
    LibWebRtcSession& operator=(const LibWebRtcSession&) = delete;

    void SetObserver(IWebRtcSessionObserver* observer) override;
    OperationId Start(const WebRtcSessionConfig& config) override;
    OperationId CreateOffer() override;
    OperationId CreateIceRestartOffer() override;
    OperationId CreateAnswer() override;
    OperationId ApplyRemoteDescription(
        const SessionDescription& description) override;
    OperationId AddRemoteIceCandidate(
        const IceCandidate& candidate) override;
    OperationId CreateDataChannels(
        const std::vector<DataChannelSpec>& channels) override;
    SendResult SendData(const std::string& channelName,
                        std::span<const std::uint8_t> data,
                        bool binary) override;
    std::optional<std::uint64_t> DataChannelBufferedAmount(
        const std::string& channelName) const override;
    void RequestStats() override;
    WebRtcSessionStatsSnapshot StatsSnapshot() const override;
    void Close() override;

    // Media-track adapters will own these responsibilities in the full
    // transport layer. They are exposed here now so the existing end-to-end
    // test can validate the formal session without bypassing it.
    webrtc::RTCErrorOr<
        webrtc::scoped_refptr<webrtc::RtpTransceiverInterface>>
    AddVideoTrack(
        webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track);
    webrtc::RTCErrorOr<
        webrtc::scoped_refptr<webrtc::RtpTransceiverInterface>>
    AddVideoReceiveTransceiver();
    webrtc::RTCError PrepareVideoTransceiverSlot(
        const std::string& slot);
    webrtc::RTCError BindNegotiatedVideoTransceiverSlots(
        const std::vector<std::string>& slots);
    webrtc::RTCError SetVideoSlotTrack(
        const std::string& slot,
        webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track);
    webrtc::RTCError SetVideoSlotSendingActive(
        const std::string& slot,
        bool active);
    void SetFastDesktopBweStartupEnabled(bool enabled);
    // Read only when applying a new user screen specification, never per frame.
    // Set before starting the session; the provider must remain thread safe.
    void SetScreenVideoBitrateBppProvider(
        std::function<std::uint32_t()> provider);
    void SetScreenVideoBitrateBpp(std::uint32_t hundredths);
    void SetScreenQualityDeficitShare(std::uint32_t hundredths);
    void SetAdaptiveDesktopNetworkFrameRateEnabled(bool enabled);
    void SetScreenContentActivity(ScreenContentActivity activity);
    void SetScreenContentPolicyObservation(const ScreenContentPolicyObservation& observation);
    void SetScreenReceiverFeedbackContext(const ScreenReceiverFeedback& context);
    void SetScreenSenderFeedbackContract(std::uint64_t generation, std::uint64_t preferenceSequence);
    bool AcceptScreenReceiverFeedback(const ScreenReceiverFeedback& feedback);
    void SetScreenContentPolicyCalibration(
        std::shared_ptr<const media_intelligence::CalibratedStreamQualityModel> model,
        std::string encoderProfile);
    void SetScreenContentPolicyReferenceEnabled(bool enabled, std::string encoderProfile);
    // The model's context is owned for all evaluations, including replacement
    // races. A null estimator keeps production in guarded observation mode.
    void SetScreenContentPolicyModel(
        media_intelligence::ContentAwareStreamConfig config,
        std::shared_ptr<const void> estimatorContext = {});
    void RestartVideoSlotBandwidthEstimation(const std::string& slot);
    void FinishVideoSlotBandwidthBootstrap(const std::string& slot);
    webrtc::RTCError SetVideoSlotEncodingPolicy(
        const std::string& slot,
        std::uint32_t framesPerSecond,
        std::uint32_t width,
        std::uint32_t height,
        ScreenStreamPolicyResult* appliedPolicy = nullptr);
    webrtc::RTCError PrepareAudioTransceiverSlot(
        const std::string& slot);
    webrtc::RTCError BindNegotiatedAudioTransceiverSlot(
        const std::string& slot);
    webrtc::RTCError SetAudioSlotTrack(
        const std::string& slot,
        webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track);
    void SetRemoteAudioSlotEnabled(const std::string& slot, bool enabled);
    bool AudioSlotPrepared(const std::string& slot) const;
    std::size_t PreparedVideoSlotCount() const;
    void SetRemoteVideoSink(
        webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink);
    void SetRemoteVideoSlotSink(
        const std::string& slot,
        webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink);

    void OnSignalingChange(
        webrtc::PeerConnectionInterface::SignalingState state) override;
    void OnDataChannel(
        webrtc::scoped_refptr<webrtc::DataChannelInterface> channel) override;
    void OnIceGatheringChange(
        webrtc::PeerConnectionInterface::IceGatheringState state) override;
    void OnIceCandidate(const webrtc::IceCandidate* candidate) override;
    void OnConnectionChange(
        webrtc::PeerConnectionInterface::PeerConnectionState state) override;
    void OnIceConnectionChange(
        webrtc::PeerConnectionInterface::IceConnectionState state) override;
    void OnTrack(
        webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver)
        override;

private:
    friend class ContentAwareStreamExecutionTestAccess;
    class CallbackGate;
    OperationId NextOperationId();
    webrtc::scoped_refptr<webrtc::PeerConnectionInterface>
        PeerConnection() const;
    void UpdatePeerConnectionState(
        webrtc::PeerConnectionInterface::PeerConnectionState state);
    void UpdateIceConnectionState(
        webrtc::PeerConnectionInterface::IceConnectionState state);
    void ApplyPendingVideoStartBitrateBootstrap();
    void HandleCompletedStatsSample(std::uint64_t observationEpoch, std::uint64_t feedbackEpoch);
    void UpdateScreenQualityProtectionLocked();
    void SendScreenReceiverFeedback(const WebRtcSessionStatsSnapshot& snapshot, std::uint64_t feedbackEpoch);
    bool HandleContentAwareStreamSample(const WebRtcSessionStatsSnapshot& snapshot,
        std::optional<std::uint64_t> observationEpoch = std::nullopt);
    webrtc::RTCError ApplyContentAwareStreamDecision(
        const media_intelligence::ContentAwareStreamInput& input,
        const media_intelligence::ContentAwareStreamEvaluation& evaluation,
        std::uint64_t revision,
        webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver,
        bool restoreUserRequest = false);
    webrtc::RTCError ApplyProgressiveBitrateCeilingDecision(
        const ProgressiveBitrateCeilingDecision& decision,
        std::uint64_t decisionRevision,
        const ProgressiveBitrateCeilingState& previousState);
    bool PulseVideoSlotAllocationProbe(
        webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver,
        std::uint64_t startBitrate,
        std::string* error);
    WebRtcSessionState CombinedConnectionStateLocked() const;
    void ChangeState(WebRtcSessionState state);
    void CompleteOperation(OperationId operationId);
    void FailOperation(OperationId operationId,
                       std::string code,
                       std::string message);
    IWebRtcSessionObserver* Observer() const;
    void DetachRemoteVideoSink();

    mutable std::mutex mutex_;
    // Serializes read-modify-write updates to RtpSender parameters. User
    // preference changes, share activation and stats-driven per-viewer FPS
    // adaptation may arrive on different threads.
    std::mutex videoSenderParametersMutex_;
    std::atomic<OperationId> nextOperationId_{1};
    std::shared_ptr<CallbackGate> callbackGate_;
    webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory_;
    std::unique_ptr<PeerConnectionStatsCollector> statsCollector_;
    std::shared_ptr<GoogCcTelemetryState> googCcTelemetry_;
    std::unique_ptr<DataChannelManager> dataChannelManager_;
    std::unique_ptr<PeerNegotiator> peerNegotiator_;
    std::unique_ptr<MediaSlotManager> mediaSlots_;
    webrtc::scoped_refptr<webrtc::PeerConnectionInterface> peerConnection_;
    IWebRtcSessionObserver* observer_ = nullptr;
    bool fastDesktopBweStartup_ = false;
    bool adaptiveDesktopNetworkFrameRate_ = false;
    std::function<std::uint32_t()> screenVideoBitrateBppProvider_;
    std::optional<std::uint32_t> liveScreenVideoBitrateBpp_;
    std::uint32_t screenQualityDeficitShareHundredths_ = kDefaultScreenQualityDeficitShareHundredths;
    bool ApplyPendingScreenVideoBitrateBpp();
    ProgressiveBitrateCeilingState progressiveBitrateCeiling_;
    std::uint64_t progressiveBitrateCeilingRevision_ = 0;
    std::string progressiveBitrateCeilingError_;
    ScreenContentActivity screenContentActivity_ =
        ScreenContentActivity::kUnknown;
    ScreenContentPolicyObservation screenContentPolicyObservation_;
    std::uint64_t screenContentPolicyEpoch_ = 0;
    ScreenReceiverFeedback receiverFeedbackContext_;
    std::uint64_t receiverFeedbackEpoch_ = 0;
    std::uint64_t receiverFeedbackNextSequence_ = 0;
    std::uint64_t receiverFeedbackLastSentMs_ = 0;
    std::uint64_t receiverFeedbackLastSampleMs_ = 0;
    std::uint32_t receiverFeedbackPreviousDecoded_ = 0;
    std::uint32_t receiverFeedbackPreviousDropped_ = 0;
    std::string receiverFeedbackPreviousStatsId_;
    std::uint64_t receiverFeedbackExpectedGeneration_ = 0;
    std::uint64_t receiverFeedbackExpectedPreference_ = 0;
    ScreenReceiverFeedback receiverFeedback_;
    std::uint64_t receiverFeedbackReceivedAtMs_ = 0;
    media_intelligence::ContentAwareStreamConfig screenContentPolicyConfig_;
    std::shared_ptr<const void> screenContentPolicyEstimatorContext_;
    std::shared_ptr<const media_intelligence::CalibratedStreamQualityModel> screenCalibrationModel_;
    std::shared_ptr<const media_intelligence::CalibratedStreamQualityContext> screenCalibrationContext_;
    std::shared_ptr<const media_intelligence::H264ReferenceQualityContext> screenReferenceContext_;
    bool screenReferenceEnabled_ = false;
    std::string screenReferenceProfile_;
    std::string screenCalibrationProfile_, screenCalibrationCodec_, screenCalibrationEncoder_;
    WebRtcSessionState state_ = WebRtcSessionState::kNew;
    webrtc::PeerConnectionInterface::PeerConnectionState
        peerConnectionState_ =
            webrtc::PeerConnectionInterface::PeerConnectionState::kNew;
    webrtc::PeerConnectionInterface::IceConnectionState iceConnectionState_ =
        webrtc::PeerConnectionInterface::kIceConnectionNew;
};

}  // namespace remote
