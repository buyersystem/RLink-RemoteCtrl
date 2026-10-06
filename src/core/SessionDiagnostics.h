// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "RemoteInputTelemetry.h"

namespace remote {

enum class RtpStreamDirection {
    kOutbound,
    kInbound,
};

// Scene recognition owns only the quality/FPS tradeoff coefficient.
struct SceneQualitySmoothingSnapshot {
    bool enabled = false;
    bool observed = false;
    std::string scene;
    double minimumCoefficient = 0;
    double maximumCoefficient = 1;
    double targetCoefficient = 0.5;
    double currentCoefficient = 0.5;
    std::uint32_t manualCoefficientHundredths = 50;
    std::uint64_t remainingMs = 0;
    bool transitioning = false;
    std::string status;
};

// Read-only GoogCC telemetry. RTT/loss are measurements, not locally invented
// congestion thresholds. The timestamps use the same host steady clock as stats.
struct GoogCcNetworkDiagnostics {
    bool controllerObserved = false;
    bool delayObserved = false;
    bool feedbackFresh = false;
    std::string delayState = "unknown";
    std::uint64_t routeRevision = 0;
    std::uint64_t targetUpdatedAtMs = 0;
    std::uint64_t feedbackAtMs = 0;
    std::uint64_t delayUpdatedAtMs = 0;
    std::uint64_t lastDelayOveruseAtMs = 0;
    std::uint64_t lastCwndPushbackAtMs = 0;
    std::uint64_t targetRateBps = 0;
    std::uint64_t effectiveTargetRateBps = 0;
    std::uint32_t feedbackAgeMs = 0;
    double congestionWindowReduction = 0;
    double roundTripTimeMs = 0;
    double lossPercent = 0;
    bool applicationLimited = false;
    bool recoveryProbeActive = false;
    std::uint32_t recoveryProbeRemainingAttempts = 0;
    std::uint64_t recoveryProbeHistoricalBudgetBps = 0;
    std::uint32_t recoveryProbeAttempts = 0;
    std::uint32_t recoveryProbeEpisodes = 0;
};

struct RtpStreamStatsSnapshot {
    std::string statsId;
    RtpStreamDirection direction = RtpStreamDirection::kOutbound;
    std::string kind;
    std::string slot;
    std::string mid;
    std::string codec;
    std::string encoderImplementation;
    std::string decoderImplementation;
    std::string qualityLimitationReason;
    std::uint64_t bytes = 0;
    std::uint64_t bitrateBps = 0;
    std::uint32_t sampleWindowMs = 0;
    std::uint64_t packets = 0;
    std::int64_t packetsLost = 0;
    std::uint64_t retransmittedPackets = 0;
    std::uint64_t retransmittedBytes = 0;
    double lossPercent = 0.0;
    double jitterMs = 0.0;
    double roundTripTimeMs = 0.0;
    std::uint64_t targetBitrateBps = 0;
    bool screenQualityProtectionAvailable = false;
    bool screenQualityProtectionActive = false;
    bool screenQualityProtectionLimited = false;
    std::uint32_t screenQualityNetworkBudgetBps = 0;
    std::uint32_t screenQualityEncoderAdjustedBudgetBps = 0;
    std::uint32_t screenQualityBandwidthAllocationBps = 0;
    std::uint32_t screenQualityEncoderReferenceBps = 0;
    std::uint32_t screenQualityReferenceBps = 0;
    std::uint32_t screenQualityDeficitShareHundredths = 50;
    std::uint64_t configuredStartBitrateBps = 0;
    std::uint64_t configuredMaxBitrateBps = 0;
    // User request ceiling remains independent of an adaptive RTP allocation.
    // Zero means not supplied; never infer the user's bpp from actual traffic.
    std::uint32_t userVideoBitrateBppHundredths = 0;
    std::uint64_t userVideoBitrateLimitBps = 0;
    std::uint32_t bitrateBootstrapAttempts = 0;
    std::uint32_t bitrateBootstrapSuccesses = 0;
    std::uint32_t mediaReadyBitrateRestarts = 0;
    std::uint32_t allocationProbePulses = 0;
    std::uint32_t bitrateProbeFloorReleases = 0;
    bool bitrateProbeFloorActive = false;
    std::string bitrateBootstrapError;
    bool progressiveBitrateCeilingEnabled = false;
    std::uint64_t desiredPeerConnectionMaxBitrateBps = 0;
    std::uint64_t appliedPeerConnectionMaxBitrateBps = 0;
    std::uint64_t smoothedOutgoingCapacityBps = 0;
    std::uint32_t bitrateCeilingStableSamples = 0;
    std::uint32_t bitrateCeilingCooldownRemainingMs = 0;
    bool bitrateCeilingProbePending = false;
    bool bitrateCeilingLastProbeFailed = false;
    std::string bitrateCeilingStatus;
    std::string bitrateCeilingError;
    std::uint32_t configuredMaxFrameRate = 0;
    std::uint32_t effectiveNetworkFrameRate = 0;
    bool adaptiveNetworkFrameRateEnabled = false;
    std::uint32_t adaptiveNetworkFrameRateReductionSamples = 0;
    std::uint32_t adaptiveNetworkFrameRateRecoverySamples = 0;
    std::uint64_t adaptiveNetworkFrameRateCapacityBps = 0;
    std::string adaptiveNetworkFrameRateStatus;
    std::string adaptiveNetworkFrameRateError;
    std::uint32_t configuredOutputWidth = 0;
    std::uint32_t configuredOutputHeight = 0;
    std::uint32_t frameWidth = 0;
    std::uint32_t frameHeight = 0;
    double framesPerSecond = 0.0;
    double encodedFramesPerSecond = 0.0;
    double sentFramesPerSecond = 0.0;
    std::uint32_t framesEncoded = 0;
    std::uint32_t framesSent = 0;
    std::uint32_t framesDecoded = 0;
    bool receiverFrameCountersAvailable = false;
    std::uint32_t framesDropped = 0;
    std::uint32_t keyFrames = 0;
    std::uint32_t sourceWidth = 0;
    std::uint32_t sourceHeight = 0;
    double sourceFramesPerSecond = 0.0;
    std::uint32_t captureTargetFrameRate = 0;
    std::string captureConfiguredBackend;
    std::string captureActiveBackend;
    std::string captureFallbackReason;
    std::string captureActivityState;
    bool captureAdaptiveFrameDeliveryEnabled = false;
    double captureAttemptsPerSecond = 0.0;
    double captureDeliveredFramesPerSecond = 0.0;
    double captureChangedFramesPerSecond = 0.0;
    double captureChangedAreaRatio = 0.0;
    double captureIdleHeartbeatFramesPerSecond = 0.0;
    std::uint64_t captureAttempts = 0;
    std::uint64_t captureDeliveredFrames = 0;
    std::uint64_t captureChangedFrames = 0;
    std::uint64_t captureIdleHeartbeatFrames = 0;
    std::uint64_t captureSuppressedUnchangedFrames = 0;
    std::uint64_t captureActivityTransitions = 0;
    std::uint64_t captureFailures = 0;
    bool captureInputBoostActive = false;
    std::uint64_t captureInputBoosts = 0;
    std::uint64_t captureForcedRefreshFrames = 0;
    double latestCaptureCallMs = 0.0;
    bool contentAnalyzerEnabled = false;
    std::string contentAnalyzerBackend;
    std::string contentSemanticType;
    std::string contentScene;
    double contentSemanticConfidence = 0.0;
    std::string contentMotionLevel;
    double contentMotionScore = 0.0;
    std::uint64_t contentSourceFrameId = 0;
    std::uint32_t contentStateAgeMs = 0;
    std::uint32_t contentLatestAnalysisTimeUs = 0;
    std::uint32_t contentLatestScaleConvertTimeUs = 0;
    std::uint32_t contentLatestJpegEncodeTimeUs = 0;
    std::uint64_t contentLatestJpegBytes = 0;
    std::string contentLatestReturnedScene;
    double contentLatestReturnedSemanticConfidence = 0.0;
    std::uint32_t contentLatestReturnedAgeMs = 0;
    std::uint64_t contentSubmittedSamples = 0;
    std::uint64_t contentReplacedSamples = 0;
    std::uint64_t contentProcessedSamples = 0;
    std::uint64_t contentRejectedSamples = 0;
    std::uint64_t contentDiscardedResults = 0;
    SceneQualitySmoothingSnapshot sceneQualitySmoothing;
    GoogCcNetworkDiagnostics googCc;
    bool receiverFeedbackAvailable = false;
    std::uint32_t receiverFeedbackAgeMs = 0;
    std::uint32_t receiverFeedbackWidth = 0;
    std::uint32_t receiverFeedbackHeight = 0;
    std::uint32_t receiverFeedbackDecodedFrames = 0;
    std::uint32_t receiverFeedbackDroppedFrames = 0;
    double receiverFeedbackDecodeTimeMs = 0;
    double receiverFeedbackProcessingTimeMs = 0;
    bool receiverFeedbackDecodeTimeAvailable = false;
    bool receiverFeedbackProcessingTimeAvailable = false;
    double averageEncodeTimeMs = 0.0;
    double averageDecodeTimeMs = 0.0;
    double averageProcessingDelayMs = 0.0;
    double averageJitterBufferDelayMs = 0.0;
    double averageQp = 0.0;
    bool latestFrameTimingAvailable = false;
    double latestFrameTimeMs = 0.0;
    double averageFrameTimingMs = 0.0;
    double p95FrameTimingMs = 0.0;
    double maximumFrameTimingMs = 0.0;
    std::uint32_t latestFrameRtpTimestamp = 0;
    std::uint32_t latestFrameTimingAgeMs = 0;
    std::uint32_t latestFrameWidth = 0;
    std::uint32_t latestFrameHeight = 0;
    std::uint64_t latestEncodedBytes = 0;
    bool latestFrameQpAvailable = false;
    std::int32_t latestFrameQp = 0;
    std::uint64_t timingCompletedFrames = 0;
    std::uint64_t timingDroppedFrames = 0;
    bool decodePipelineTimingAvailable = false;
    bool decodePipelineAsynchronous = false;
    double decodeInputPreparationMs = 0.0;
    double decodeInputQueueWaitMs = 0.0;
    double decodeTransformWaitMs = 0.0;
    double averageDecodeTransformWaitMs = 0.0;
    double p95DecodeTransformWaitMs = 0.0;
    double maximumDecodeTransformWaitMs = 0.0;
    double decodeOutputDeliveryMs = 0.0;
    std::uint32_t decodeQueuedInputFrames = 0;
    std::uint32_t decodeInFlightFrames = 0;
    std::uint32_t decodePeakBacklogFrames = 0;
    std::uint64_t decodeBackpressureFallbacks = 0;
    bool windowEncodeTimeAvailable = false;
    double windowEncodeTimeMs = 0.0;
    bool windowDecodeTimeAvailable = false;
    double windowDecodeTimeMs = 0.0;
    bool windowProcessingDelayAvailable = false;
    double windowProcessingDelayMs = 0.0;
    bool windowJitterBufferDelayAvailable = false;
    double windowJitterBufferDelayMs = 0.0;
    bool windowQpAvailable = false;
    double windowQp = 0.0;
    double audioLevel = 0.0;
    std::uint64_t totalAudioSamples = 0;
    std::uint64_t concealedAudioSamples = 0;
    std::uint64_t concealmentEvents = 0;
    std::uint32_t freezeCount = 0;
    double totalFreezeDurationMs = 0.0;
    std::uint32_t pauseCount = 0;
    double totalPauseDurationMs = 0.0;
    std::uint32_t nackCount = 0;
    std::uint32_t pliCount = 0;
    std::uint32_t firCount = 0;
    bool presentationTimingAvailable = false;
    std::string presentationPath;
    double localRefreshRateHz = 0.0;
    double presentationArrivalFramesPerSecond = 0.0;
    double presentedFramesPerSecond = 0.0;
    std::uint64_t presentationArrivedFrames = 0;
    std::uint64_t presentedFrames = 0;
    std::uint64_t presentationSupersededFrames = 0;
    std::uint64_t presentationConvertedFrames = 0;
    std::uint64_t presentationFailures = 0;
    double latestPresentationConversionMs = 0.0;
    double averagePresentationConversionMs = 0.0;
    double latestPresentationRenderSubmitMs = 0.0;
    double averagePresentationRenderSubmitMs = 0.0;
    double latestPresentCallMs = 0.0;
    double averagePresentCallMs = 0.0;
    double averagePresentedIntervalMs = 0.0;
    double p95PresentedIntervalMs = 0.0;
    double maximumPresentedIntervalMs = 0.0;
    double latestReceiverPipelineMs = 0.0;
    double averageReceiverPipelineMs = 0.0;
    double p95ReceiverPipelineMs = 0.0;
    double maximumReceiverPipelineMs = 0.0;
    bool active = false;
    bool powerEfficient = false;
};

struct DataChannelStatsSnapshot {
    std::string label;
    std::string protocol;
    std::string state;
    std::uint64_t bytesSent = 0;
    std::uint64_t bytesReceived = 0;
    std::uint64_t sendBitrateBps = 0;
    std::uint64_t receiveBitrateBps = 0;
    std::uint32_t sampleWindowMs = 0;
    std::uint64_t bufferedAmountBytes = 0;
    std::uint32_t messagesSent = 0;
    std::uint32_t messagesReceived = 0;
};

struct IceCandidateStatsSnapshot {
    std::string candidateType;
    std::string protocol;
    std::string address;
    std::int32_t port = 0;
    std::string networkType;
    std::string adapterType;
    std::string relayProtocol;
};

struct WebRtcTransportStatsSnapshot {
    bool collected = false;
    GoogCcNetworkDiagnostics googCc;
    std::int64_t timestampMs = 0;
    // Host steady clock; RTC report timestamps may use another epoch.
    std::uint64_t receivedAtSteadyMs = 0;
    std::string iceState;
    std::string dtlsState;
    std::string iceRole;
    std::string candidatePairState;
    std::string routeType;
    std::string tlsVersion;
    std::string dtlsCipher;
    std::string srtpCipher;
    IceCandidateStatsSnapshot localCandidate;
    IceCandidateStatsSnapshot remoteCandidate;
    bool nominated = false;
    bool writable = false;
    std::uint32_t selectedCandidatePairChanges = 0;
    std::uint64_t packetsDiscardedOnSend = 0;
    std::uint64_t bytesDiscardedOnSend = 0;
    double currentRoundTripTimeMs = 0.0;
    std::uint64_t availableOutgoingBitrateBps = 0;
    std::uint64_t availableIncomingBitrateBps = 0;
    std::uint64_t sendBitrateBps = 0;
    std::uint64_t receiveBitrateBps = 0;
    std::uint32_t sampleWindowMs = 0;
    std::uint64_t bytesSent = 0;
    std::uint64_t bytesReceived = 0;
};

struct WebRtcSessionStatsSnapshot {
    WebRtcTransportStatsSnapshot transport;
    std::vector<RtpStreamStatsSnapshot> rtpStreams;
    std::vector<DataChannelStatsSnapshot> dataChannels;
};

struct PeerConnectionDiagnosticsSnapshot {
    std::string pairId;
    std::string peerDeviceId;
    WebRtcSessionStatsSnapshot stats;
};

struct SessionDiagnosticsSnapshot {
    struct RemoteCursorDiagnosticsSnapshot {
        bool publishing = false;
        bool shapeAvailable = false;
        std::uint32_t displayId = 0;
        std::uint64_t displayLayoutVersion = 0;
        std::uint64_t shapeId = 0;
        std::uint64_t positionMessagesPublished = 0;
        std::uint64_t shapeMessagesPublished = 0;
        std::uint64_t positionMessagesReceived = 0;
        std::uint64_t shapeMessagesReceived = 0;
        std::uint64_t lastAppliedInputSequence = 0;
    } remoteCursor;
    std::int64_t collectedAtMs = 0;
    // Most recent reason reported by a hardware video encoder before asking
    // libwebrtc to use its software fallback. Keep this in the session
    // diagnostics as well as the global capability snapshot so the reason is
    // visible beside the affected outbound stream.
    std::string videoEncoderLastFallbackReason;
    std::vector<PeerConnectionDiagnosticsSnapshot> peerConnections;
    RemoteInputTelemetrySnapshot remoteInput;
};

}  // namespace remote
