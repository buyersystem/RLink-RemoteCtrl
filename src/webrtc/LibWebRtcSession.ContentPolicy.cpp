// SPDX-License-Identifier: GPL-3.0-only
#include "LibWebRtcSession.Internal.h"
#include "MediaSlotManager.h"
#include <cmath>

namespace remote {
using namespace webrtc_session_detail;
using namespace media_intelligence;

void LibWebRtcSession::SetScreenContentPolicyObservation(
    const ScreenContentPolicyObservation& observation)
{
    std::lock_guard lock(mutex_);
    const bool enabling = observation.enabled && !screenContentPolicyObservation_.enabled;
    const bool reset = observation.enabled != screenContentPolicyObservation_.enabled ||
        observation.generation != screenContentPolicyObservation_.generation ||
        observation.sourceWidth != screenContentPolicyObservation_.sourceWidth ||
        observation.sourceHeight != screenContentPolicyObservation_.sourceHeight;
    screenContentPolicyObservation_ = observation;
    if (reset) {
        ++screenContentPolicyEpoch_;
        const auto found = mediaSlots_->videoSlots_.find(kScreenMainVideoSlot);
        if (found != mediaSlots_->videoSlots_.end()) {
            auto& binding = found->second;
            binding.contentPolicyNeedsRestore = binding.contentPolicyNeedsRestore ||
                binding.contentPolicyExecution.applied || binding.contentPolicyExecution.pending ||
                (enabling && binding.effectiveMaxFps != binding.configuredMaxFrameRate);
            binding.adaptiveFrameRate.enabled = false;
            binding.contentPolicyExecution.pending = false;
            binding.contentPolicyExecution.ownsSender = false;
            binding.contentPolicyRecommendation = {};
            found->second.contentPolicyState = {};
            binding.contentNetworkPressure = {};
            binding.verifiedUserSamples = 0;
            binding.verifiedUserBitrateBps = 0;
            ++found->second.contentPolicyRevision;
            binding.contentPolicyEvidenceNotBeforeMs = SteadyNowMs();
            binding.contentQualityMetricAvailable = binding.contentQualityVerified = false;
            binding.contentProcessingEvidenceAvailable = binding.contentProcessingHealthy = false;
            ++binding.adaptiveFrameRateRevision;
        }
    }
    UpdateScreenQualityProtectionLocked();
}

void LibWebRtcSession::SetScreenContentPolicyModel(
    ContentAwareStreamConfig config, std::shared_ptr<const void> estimatorContext)
{
    std::lock_guard lock(mutex_);
    // Never retain an unowned, potentially temporary caller context.
    if (estimatorContext) config.qualityEstimatorContext = estimatorContext.get();
    else if (config.qualityEstimatorContext) {
        config.qualityEstimator = nullptr;
        config.qualityEstimatorContext = nullptr;
    }
    screenContentPolicyEstimatorContext_ = std::move(estimatorContext);
    screenCalibrationModel_.reset();
    screenCalibrationContext_.reset();
    screenReferenceContext_.reset();
    screenReferenceEnabled_ = false;
    screenContentPolicyConfig_ = config;
    ++screenContentPolicyEpoch_;
    const auto found = mediaSlots_->videoSlots_.find(kScreenMainVideoSlot);
    if (found != mediaSlots_->videoSlots_.end()) {
        auto& binding = found->second;
        binding.contentPolicyNeedsRestore = binding.contentPolicyNeedsRestore ||
            binding.contentPolicyExecution.applied || binding.contentPolicyExecution.pending;
        binding.contentPolicyExecution.pending = false;
        binding.contentPolicyExecution.ownsSender = false;
        binding.contentPolicyRecommendation = {};
        found->second.contentPolicyState = {};
        ++found->second.contentPolicyRevision;
        binding.contentPolicyEvidenceNotBeforeMs = SteadyNowMs();
        binding.contentQualityMetricAvailable = binding.contentQualityVerified = false;
        binding.contentProcessingEvidenceAvailable = binding.contentProcessingHealthy = false;
        ++binding.adaptiveFrameRateRevision;
    }
    UpdateScreenQualityProtectionLocked();
}

void LibWebRtcSession::SetScreenContentPolicyCalibration(
    std::shared_ptr<const CalibratedStreamQualityModel> model, std::string encoderProfile)
{
    // Publish through the same reset contract as a generic estimator. The
    // first real RTC sample binds codec/backend before creating the context.
    bool referenceEnabled;
    std::string referenceProfile;
    {
        std::lock_guard lock(mutex_);
        if (screenCalibrationModel_ == model && screenCalibrationProfile_ == encoderProfile) return;
        referenceEnabled = screenReferenceEnabled_;
        referenceProfile = screenReferenceProfile_;
    }
    SetScreenContentPolicyModel({});
    std::lock_guard lock(mutex_);
    screenCalibrationModel_ = std::move(model);
    screenCalibrationProfile_ = std::move(encoderProfile);
    screenCalibrationCodec_.clear();
    screenCalibrationEncoder_.clear();
    screenReferenceEnabled_ = referenceEnabled;
    screenReferenceProfile_ = std::move(referenceProfile);
}

void LibWebRtcSession::SetScreenContentPolicyReferenceEnabled(bool enabled, std::string encoderProfile)
{
    std::lock_guard lock(mutex_);
    if (screenReferenceEnabled_ == enabled && screenReferenceProfile_ == encoderProfile) return;
    screenReferenceEnabled_ = enabled;
    screenReferenceProfile_ = std::move(encoderProfile);
    screenReferenceContext_.reset();
    screenCalibrationContext_.reset();
    screenContentPolicyConfig_ = {};
    screenContentPolicyConfig_.allowReferenceModel = enabled;
    ++screenContentPolicyEpoch_;
    const auto found = mediaSlots_->videoSlots_.find(kScreenMainVideoSlot);
    if (found == mediaSlots_->videoSlots_.end()) return;
    auto& binding = found->second;
    binding.contentPolicyNeedsRestore = binding.contentPolicyNeedsRestore ||
        binding.contentPolicyExecution.applied || binding.contentPolicyExecution.pending;
    binding.contentPolicyExecution.pending = binding.contentPolicyExecution.ownsSender = false;
    binding.contentPolicyRecommendation = {};
    binding.contentPolicyState = {};
    binding.contentPolicyEvidenceNotBeforeMs = SteadyNowMs();
    binding.contentQualityMetricAvailable = binding.contentQualityVerified = false;
    binding.contentProcessingEvidenceAvailable = binding.contentProcessingHealthy = false;
    ++binding.contentPolicyRevision;
    ++binding.adaptiveFrameRateRevision;
}

bool LibWebRtcSession::HandleContentAwareStreamSample(
    const WebRtcSessionStatsSnapshot& snapshot,
    std::optional<std::uint64_t> observationEpoch)
{
    const auto stream = std::find_if(snapshot.rtpStreams.begin(), snapshot.rtpStreams.end(),
        [](const auto& value) { return value.direction == RtpStreamDirection::kOutbound &&
            value.kind == "video" && value.slot == kScreenMainVideoSlot; });
    if (stream == snapshot.rtpStreams.end()) return false;
    ContentAwareStreamInput input;
    ContentAwareStreamEvaluation evaluation;
    webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver;
    std::uint64_t revision = 0;
    bool restore = false;
    bool owns = false;
    {
        std::lock_guard lock(mutex_);
        if (observationEpoch && *observationEpoch != screenContentPolicyEpoch_)
            return false;
        const auto found = mediaSlots_->videoSlots_.find(kScreenMainVideoSlot);
        if (found == mediaSlots_->videoSlots_.end() || !found->second.sendingActive)
            return false;
        auto& binding = found->second;
        const auto& observation = screenContentPolicyObservation_;
        auto& execution = binding.contentPolicyExecution;
        execution.observed = observation.enabled;
        execution.generation = observation.generation;
        execution.confirmObservedSamples = execution.confirmRequiredSamples = 0;
        execution.confirmationRemainingMs = 0;
        execution.confirmationBlock.clear();
        input.nowMs = SteadyNowMs();
        input.generation = observation.generation;
        input.networkGeneration = observation.generation;
        input.networkTimestampMs = snapshot.transport.receivedAtSteadyMs;
        input.sourceWidth = observation.sourceWidth;
        input.sourceHeight = observation.sourceHeight;
        input.maximumWidth = binding.configuredOutputWidth;
        input.maximumHeight = binding.configuredOutputHeight;
        input.maximumFrameRate = binding.configuredMaxFrameRate;
        input.currentWidth = binding.effectiveWidth;
        input.currentHeight = binding.effectiveHeight;
        input.currentFrameRate = binding.effectiveMaxFps;
        input.currentDesiredVideoBitrateBps = binding.effectiveDesiredBitrateBps;
        input.currentSenderMaxBitrateBps = binding.effectiveMaxBitrateBps;

        if ((screenCalibrationModel_ || screenReferenceEnabled_) &&
            ((screenCalibrationModel_ && !screenCalibrationContext_) ||
             (screenReferenceEnabled_ && !screenReferenceContext_) ||
            screenCalibrationCodec_ != stream->codec || screenCalibrationEncoder_ != stream->encoderImplementation)) {
            screenCalibrationCodec_ = stream->codec;
            screenCalibrationEncoder_ = stream->encoderImplementation;
            if (screenCalibrationModel_) screenCalibrationContext_ = std::make_shared<CalibratedStreamQualityContext>(
                *screenCalibrationModel_, StreamEncoderProfile{stream->codec, stream->encoderImplementation,
                    screenCalibrationProfile_});
            if (screenReferenceEnabled_) screenReferenceContext_ = std::make_shared<H264ReferenceQualityContext>(
                StreamEncoderProfile{stream->codec, stream->encoderImplementation, screenReferenceProfile_});
            binding.contentPolicyState = {};
            binding.contentPolicyNeedsRestore = binding.contentPolicyNeedsRestore || execution.applied;
            binding.contentPolicyEvidenceNotBeforeMs = input.nowMs;
            binding.verifiedUserSamples = 0;
            binding.verifiedUserBitrateBps = 0;
        }

        const StreamQualityRequest currentRequest{input.currentWidth, input.currentHeight,
            input.currentFrameRate, observation.scene};
        const bool useCalibration = screenCalibrationContext_ &&
            CalibratedStreamQualityContext::Estimate(currentRequest, screenCalibrationContext_.get()).available;
        const bool useReference = !useCalibration && screenReferenceEnabled_ && screenReferenceContext_;
        if (screenCalibrationModel_ || screenReferenceEnabled_) {
            screenContentPolicyConfig_.allowReferenceModel = screenReferenceEnabled_;
            screenContentPolicyConfig_.qualityEstimator = useReference
                ? H264ReferenceQualityContext::Estimate : CalibratedStreamQualityContext::Estimate;
            screenContentPolicyConfig_.qualityEstimatorContext = useReference
                ? static_cast<const void*>(screenReferenceContext_.get()) : screenCalibrationContext_.get();
        }

        if (!observation.enabled || binding.contentPolicyNeedsRestore) {
            binding.contentPolicyRecommendation = {};
            execution.eligible = false;
            execution.ownsSender = false;
            execution.status = observation.enabled ? "context_changed_restoring" : "disabled";
            restore = execution.applied || binding.contentPolicyNeedsRestore;
            if (!restore) return false;
            auto& decision = evaluation.recommendation;
            decision.width = binding.configuredOutputWidth;
            decision.height = binding.configuredOutputHeight;
            decision.senderMaxFps = !observation.enabled || !execution.applied ? binding.configuredMaxFrameRate
                : (std::min)(binding.configuredMaxFrameRate, binding.effectiveMaxFps);
            decision.desiredVideoBitrateBps = binding.configuredMaxBitrateBps;
            decision.senderMaxBitrateBps = binding.configuredMaxBitrateBps;
            evaluation.confirmed = true;
            execution.pending = true;
        } else {
            input.scene = observation.scene;
            input.activityAvailable = observation.activity == ScreenContentActivity::kActive ||
                observation.activity == ScreenContentActivity::kIdle;
            input.active = observation.activity == ScreenContentActivity::kActive;
            input.qualityAvailable = observation.qualityAvailable;
            input.qualityAcceptable = observation.qualityAcceptable;
            input.processingAvailable = observation.processingAvailable;
            input.processingHealthy = observation.processingHealthy;
            const bool completeWindow = snapshot.transport.receivedAtSteadyMs >= stream->sampleWindowMs &&
                snapshot.transport.receivedAtSteadyMs - stream->sampleWindowMs >= binding.contentPolicyEvidenceNotBeforeMs;
            const bool matchingFrames = stream->frameWidth == binding.effectiveWidth &&
                stream->frameHeight == binding.effectiveHeight;
            binding.contentQualityMetricAvailable = stream->windowQpAvailable && completeWindow && matchingFrames;
            if (screenCalibrationModel_ || screenReferenceEnabled_) {
                StreamCurrentQualityVerification quality;
                if (binding.contentQualityMetricAvailable) {
                    if (useReference) quality = screenReferenceContext_->VerifyCurrentQuality(currentRequest, stream->windowQp);
                    else if (screenCalibrationContext_)
                        quality = screenCalibrationContext_->VerifyCurrentQuality(currentRequest, stream->windowQp);
                }
                input.qualityAvailable = quality.available;
                input.qualityAcceptable = quality.acceptable;
            }
            binding.contentQualityVerified = input.qualityAvailable && input.qualityAcceptable;
            const bool receiverFresh = receiverFeedbackReceivedAtMs_ && receiverFeedbackReceivedAtMs_ <= input.nowMs &&
                receiverFeedbackReceivedAtMs_ >= binding.contentPolicyEvidenceNotBeforeMs &&
                receiverFeedbackReceivedAtMs_ - binding.contentPolicyEvidenceNotBeforeMs >= receiverFeedback_.sampleWindowMs &&
                input.nowMs - receiverFeedbackReceivedAtMs_ <= 3000 &&
                receiverFeedback_.screenShareGeneration == input.generation &&
                receiverFeedback_.frameWidth == input.currentWidth && receiverFeedback_.frameHeight == input.currentHeight;
            if (receiverFresh && completeWindow && matchingFrames && stream->windowEncodeTimeAvailable &&
                receiverFeedback_.decodeTimeAvailable && receiverFeedback_.processingTimeAvailable) {
                const auto& profile = screenContentPolicyConfig_.profiles[
                    static_cast<std::size_t>(input.scene) < screenContentPolicyConfig_.profiles.size()
                        ? static_cast<std::size_t>(input.scene) : 0];
                const double periodMs = 1000.0 / (std::max)(1u, input.currentFrameRate);
                input.processingAvailable = true;
                input.processingHealthy = std::isfinite(stream->windowEncodeTimeMs) &&
                    stream->windowEncodeTimeMs >= 0 && stream->windowEncodeTimeMs <=
                        periodMs * profile.maximumEncodeFrameBudgetRatio &&
                    receiverFeedback_.windowDecodeTimeUs <=
                        periodMs * 1000 * profile.maximumDecodeFrameBudgetRatio &&
                    receiverFeedback_.processingTimeUs <= profile.maximumReceiverProcessingMs * 1000 &&
                    receiverFeedback_.decodedFrames >= 4 &&
                    receiverFeedback_.droppedFrames * 100ULL <= receiverFeedback_.decodedFrames * 2ULL;
            }
            binding.contentProcessingEvidenceAvailable = input.processingAvailable;
            binding.contentProcessingHealthy = input.processingAvailable && input.processingHealthy;
            // The observation itself must be fresh, independently of RTC stats.
            if (observation.observedAtMs > input.nowMs ||
                input.nowMs - observation.observedAtMs > 3000) {
                input.activityAvailable = false;
                input.qualityAvailable = input.processingAvailable = false;
            }
            binding.contentQualityVerified = input.qualityAvailable && input.qualityAcceptable;
            binding.contentProcessingEvidenceAvailable = input.processingAvailable;
            binding.contentProcessingHealthy = input.processingAvailable && input.processingHealthy;
            const bool deliveryWindow = completeWindow && matchingFrames &&
                stream->sampleWindowMs >= 500 &&
                snapshot.transport.receivedAtSteadyMs <= input.nowMs &&
                input.nowMs - snapshot.transport.receivedAtSteadyMs <=
                    screenContentPolicyConfig_.maximumNetworkSampleAgeMs;
            // Reuse GoogCC decisions verbatim. Video adaptation labels, QP,
            // delivery FPS, RTT/loss thresholds and low BWE are not congestion
            // detectors. A normal detector at a reduced budget retains recovery
            // context until the user's actual R/F has been restored stably.
            const auto& gcc = snapshot.transport.googCc;
            GoogCcNetworkPressureInput pressureInput;
            pressureInput.nowMs = input.nowMs;
            pressureInput.routeGeneration = gcc.routeRevision;
            pressureInput.controllerOutputAvailable = gcc.controllerObserved;
            pressureInput.targetTimestampMs = gcc.targetUpdatedAtMs;
            pressureInput.transportFeedbackTimestampMs = gcc.feedbackAtMs;
            pressureInput.delayTimestampMs = gcc.delayUpdatedAtMs;
            pressureInput.lastDelayOveruseAtMs = gcc.lastDelayOveruseAtMs;
            pressureInput.lastCwndPushbackAtMs = gcc.lastCwndPushbackAtMs;
            pressureInput.delayState = !gcc.delayObserved ? GoogCcDelayState::kUnknown
                : gcc.delayState == "overuse" ? GoogCcDelayState::kOverusing
                : gcc.delayState == "underuse" ? GoogCcDelayState::kUnderusing
                : gcc.delayState == "normal" ? GoogCcDelayState::kNormal
                : GoogCcDelayState::kUnknown;
            pressureInput.targetBitrateBps = gcc.targetRateBps;
            pressureInput.cwndReduceRatio = gcc.congestionWindowReduction;
            pressureInput.userSpecificationRestored = input.currentWidth == input.maximumWidth &&
                input.currentHeight == input.maximumHeight && input.currentFrameRate == input.maximumFrameRate;
            const auto pressure = EvaluateGoogCcNetworkPressure(binding.contentNetworkPressure,
                pressureInput, {.maximumSampleAgeMs = screenContentPolicyConfig_.maximumNetworkSampleAgeMs});
            execution.networkPressure = pressure.episodeActive;
            execution.networkStatus = GoogCcNetworkPressureReasonName(pressure.reason);
            execution.networkTrigger = pressure.episodeActive
                ? GoogCcNetworkPressureReasonName(binding.contentNetworkPressure.lastPressureReason) : "";
            const auto capacity = gcc.controllerObserved ? gcc.effectiveTargetRateBps
                : snapshot.transport.availableOutgoingBitrateBps;
            const auto budget = ContentAwareVideoBudgetBps(capacity, binding.configuredMaxBitrateBps);
            input.networkBudgetAvailable = snapshot.transport.collected && deliveryWindow &&
                pressure.sampleFresh && capacity != 0;
            input.safeVideoBudgetBps = static_cast<double>(budget);
            input.networkConstrained = pressure.episodeActive;
            input.userVideoBitrateCeilingBps = binding.configuredMaxBitrateBps;
            input.userSpecificationRecoveryAllowed = pressure.sampleFresh && pressure.delayFresh && pressure.episodeActive &&
                pressure.reason == GoogCcNetworkPressureReason::kEpisodeRetained &&
                (gcc.delayState == "normal" || gcc.delayState == "underuse") &&
                gcc.congestionWindowReduction <= 0;

            const bool sameBaselineContext = binding.verifiedUserScene == input.scene &&
                binding.verifiedUserRouteRevision == gcc.routeRevision &&
                binding.verifiedUserWidth == input.maximumWidth &&
                binding.verifiedUserHeight == input.maximumHeight &&
                binding.verifiedUserFps == input.maximumFrameRate &&
                binding.verifiedUserVideoCeilingBps == binding.configuredMaxBitrateBps;
            if (!sameBaselineContext) {
                binding.verifiedUserScene = input.scene;
                binding.verifiedUserRouteRevision = gcc.routeRevision;
                binding.verifiedUserWidth = input.maximumWidth;
                binding.verifiedUserHeight = input.maximumHeight;
                binding.verifiedUserFps = input.maximumFrameRate;
                binding.verifiedUserVideoCeilingBps = binding.configuredMaxBitrateBps;
                binding.verifiedUserSamples = 0;
                binding.verifiedUserBitrateBps = 0;
                binding.verifiedUserFirstSampleMs = binding.verifiedUserLastSampleMs = 0;
            }
            // A target is an allowance, not the actual cost of delivering this
            // specification. Learn the largest real RTP rate across qualifying
            // moving/quality-passing windows; a ceiling-valued target would
            // otherwise make recovery impossible at the 95% budget boundary.
            const auto measuredDemand = stream->bitrateBps;
            const bool verifiesOriginal = !pressure.episodeActive && pressure.sampleFresh && deliveryWindow &&
                input.activityAvailable && input.active && input.scene != ScreenScene::kUnknown &&
                input.qualityAvailable && input.qualityAcceptable && pressureInput.userSpecificationRestored &&
                std::isfinite(stream->encodedFramesPerSecond) && std::isfinite(stream->sentFramesPerSecond) &&
                stream->encodedFramesPerSecond >= input.maximumFrameRate * 0.90 &&
                stream->sentFramesPerSecond >= input.maximumFrameRate * 0.90 &&
                measuredDemand != 0 && measuredDemand <= binding.configuredMaxBitrateBps;
            if (verifiesOriginal && binding.verifiedUserSamples < 3 && binding.verifiedUserLastSampleMs &&
                snapshot.transport.receivedAtSteadyMs > binding.verifiedUserLastSampleMs &&
                snapshot.transport.receivedAtSteadyMs - binding.verifiedUserLastSampleMs >
                    screenContentPolicyConfig_.maximumNetworkSampleAgeMs) {
                binding.verifiedUserSamples = 0;
                binding.verifiedUserBitrateBps = 0;
            }
            if (verifiesOriginal && snapshot.transport.receivedAtSteadyMs > binding.verifiedUserLastSampleMs) {
                if (!binding.verifiedUserSamples)
                    binding.verifiedUserFirstSampleMs = snapshot.transport.receivedAtSteadyMs;
                binding.verifiedUserLastSampleMs = snapshot.transport.receivedAtSteadyMs;
                binding.verifiedUserSamples = (std::min)(binding.verifiedUserSamples + 1, 1000u);
                binding.verifiedUserBitrateBps = (std::max)(binding.verifiedUserBitrateBps, measuredDemand);
            } else if (!verifiesOriginal && !pressure.episodeActive && binding.verifiedUserSamples < 3) {
                binding.verifiedUserSamples = 0;
                binding.verifiedUserBitrateBps = 0;
            }
            input.userSpecificationVerifiedBitrateBps = binding.verifiedUserSamples >= 3 &&
                binding.verifiedUserLastSampleMs >= binding.verifiedUserFirstSampleMs &&
                binding.verifiedUserLastSampleMs - binding.verifiedUserFirstSampleMs >= 2000
                ? binding.verifiedUserBitrateBps : 0;
            execution.userSpecificationVerifiedBitrateBps = input.userSpecificationVerifiedBitrateBps;
            if (input.qualityAvailable && !input.qualityAcceptable) {
                // The reference is a starting estimate, not a permanent cap.
                // A bad real QP window first requests a bounded 15% budget
                // increase at the current specification. The pure policy
                // tests it against confirmed capacity before reducing R/F.
                const auto currentBudget = (std::max)(input.currentDesiredVideoBitrateBps,
                    input.currentSenderMaxBitrateBps);
                const auto base = (std::min<std::uint64_t>)(currentBudget, 100'000'000);
                input.qualityRecoveryBitrateBps = (std::min<std::uint64_t>)(100'000'000,
                    base + base / 100 * 15 + (base % 100 * 15 + 99) / 100);
            }
            evaluation = EvaluateContentAwareStream(binding.contentPolicyState, input,
                screenContentPolicyConfig_);
            const auto& decision = evaluation.recommendation;
            execution.confirmObservedSamples = evaluation.confirmObservedSamples;
            execution.confirmRequiredSamples = evaluation.confirmRequiredSamples;
            execution.confirmationRemainingMs = evaluation.confirmationRemainingMs;
            execution.confirmationBlock = ContentAwareStreamConfirmationBlockName(evaluation.confirmationBlock);
            binding.contentPolicyRecommendation = {
                .observed = true, .hasRecommendation = decision.hasRecommendation,
                .estimatedFeasible = decision.estimatedFeasible,
                .modelCalibrated = decision.modelCalibrated, .modelReference = decision.modelReference,
                .width = decision.width,
                .height = decision.height, .senderMaxFps = decision.senderMaxFps,
                .estimatedSafeVideoBudgetBps = budget,
                .requiredVideoBitrateBps = decision.requiredVideoBitrateBps,
                .desiredVideoBitrateBps = decision.desiredVideoBitrateBps,
                .senderMaxBitrateBps = decision.senderMaxBitrateBps,
                .reason = ContentAwareStreamReasonName(decision.reason)};
            execution.eligible = decision.automaticControlEligible;
            execution.pending = evaluation.confirmed;
            // Scene allocation has exclusive application-level ownership.
            // Unqualified windows leave native WebRTC congestion control active.
            owns = decision.automaticControlEligible ||
                (input.activityAvailable && input.scene != ScreenScene::kUnknown);
            execution.ownsSender = owns;
            execution.status = input.scene == ScreenScene::kUnknown ? "awaiting_scene"
                : !input.activityAvailable ? "awaiting_activity"
                : !input.active ? "idle_hold"
                : !(decision.modelCalibrated ||
                (decision.modelReference && screenContentPolicyConfig_.allowReferenceModel)) ? "awaiting_calibration"
                : !deliveryWindow ? "awaiting_stream_window"
                : !input.networkBudgetAvailable ? "awaiting_network_evidence"
                : decision.reason == ContentAwareStreamReason::kHealthyHold ? "healthy_hold"
                : !decision.estimatedFeasible && !decision.networkRepairRequired &&
                    !decision.userSpecificationRestoreRequired &&
                    decision.requiredVideoBitrateBps > budget
                    ? "awaiting_candidate_budget"
                : !input.qualityAvailable && !decision.networkRepairRequired &&
                    !decision.userSpecificationRestoreRequired ? "awaiting_quality_evidence"
                : !decision.automaticControlEligible ? "holding_for_evidence"
                : decision.userSpecificationRestoreRequired
                    ? (evaluation.confirmed ? "applying_user_restore" : "observing_user_restore")
                : decision.reason == ContentAwareStreamReason::kEmergencyNetworkReduction
                    ? (evaluation.confirmed ? "applying_emergency" : "observing_emergency")
                : decision.qualityRepairRequired
                    ? (evaluation.confirmed ? "applying_repair" : "observing_repair")
                : evaluation.confirmed ? "applying" : "observing";
            if (!evaluation.confirmed) return owns;
        }
        transceiver = binding.transceiver;
        revision = ++binding.contentPolicyRevision;
        ++binding.adaptiveFrameRateRevision;
        execution.revision = revision;
    }
    const auto result = ApplyContentAwareStreamDecision(input, evaluation, revision,
        transceiver, restore);
    // Native GoogCC rate control remains active even if this scene transaction fails.
    return result.ok() ? !restore : false;
}

webrtc::RTCError LibWebRtcSession::ApplyContentAwareStreamDecision(
    const ContentAwareStreamInput& input, const ContentAwareStreamEvaluation& evaluation,
    std::uint64_t revision, webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver,
    bool restoreUserRequest)
{
    const auto& decision = evaluation.recommendation;
    if (!evaluation.confirmed || (!restoreUserRequest && !decision.automaticControlEligible))
        return webrtc::RTCError::InvalidState("The scene decision is not qualified for execution.");
    const auto failed = [&](webrtc::RTCError error) {
        std::lock_guard lock(mutex_);
        const auto found = mediaSlots_->videoSlots_.find(kScreenMainVideoSlot);
        if (found != mediaSlots_->videoSlots_.end() &&
            found->second.contentPolicyRevision == revision) {
            auto& execution = found->second.contentPolicyExecution;
            execution.pending = false;
            execution.ownsSender = false;
            execution.status = "apply_failed";
            execution.error = std::string(error.message());
        }
        return error;
    };
    // Stats completion runs on the signaling thread. A user operation may
    // hold this mutex while its proxy waits for that same thread: never wait
    // for the mutex here. Keep the unconfirmed candidate for a later window.
    std::unique_lock senderLock(videoSenderParametersMutex_, std::try_to_lock);
    if (!senderLock.owns_lock()) {
        std::lock_guard lock(mutex_);
        const auto found = mediaSlots_->videoSlots_.find(kScreenMainVideoSlot);
        if (found != mediaSlots_->videoSlots_.end() && found->second.contentPolicyRevision == revision) {
            auto& execution = found->second.contentPolicyExecution;
            execution.pending = false;
            execution.ownsSender = false;
            execution.status = "sender_busy";
            execution.error.clear();
        }
        return webrtc::RTCError::InvalidState("Sender parameters are busy; retry on the next stats window.");
    }
    {
        std::lock_guard lock(mutex_);
        const auto found = mediaSlots_->videoSlots_.find(kScreenMainVideoSlot);
        if (found == mediaSlots_->videoSlots_.end() || !found->second.sendingActive ||
            found->second.transceiver != transceiver ||
            found->second.contentPolicyRevision != revision ||
            screenContentPolicyObservation_.generation != input.generation)
            return webrtc::RTCError::InvalidState("The scene decision has expired.");
        const auto& binding = found->second;
        if (!decision.width || !decision.height || !decision.senderMaxFps ||
            decision.width > binding.configuredOutputWidth ||
            decision.height > binding.configuredOutputHeight ||
            decision.senderMaxFps > binding.configuredMaxFrameRate ||
            !decision.senderMaxBitrateBps ||
            decision.senderMaxBitrateBps > binding.configuredMaxBitrateBps ||
            decision.senderMaxBitrateBps > 100'000'000) {
            auto& execution = found->second.contentPolicyExecution;
            execution.pending = false;
            execution.ownsSender = false;
            execution.status = "apply_failed";
            execution.error = "The scene decision exceeds user/session limits.";
            return webrtc::RTCError::InvalidParameter("The scene decision exceeds user/session limits.");
        }
    }
    auto sender = transceiver ? transceiver->sender() : nullptr;
    if (!sender) return failed(webrtc::RTCError::InvalidState("The screen sender is unavailable."));
    auto parameters = sender->GetParameters();
    if (parameters.encodings.empty())
        return failed(webrtc::RTCError::InvalidState("The screen sender has no RTP encoding."));
    for (auto& encoding : parameters.encodings) {
        encoding.max_framerate = static_cast<double>(decision.senderMaxFps);
        encoding.max_bitrate_bps = static_cast<int>(decision.senderMaxBitrateBps);
        encoding.scale_resolution_down_to = webrtc::Resolution{
            static_cast<int>(decision.width), static_cast<int>(decision.height)};
    }
    const auto result = sender->SetParameters(parameters);
    std::lock_guard lock(mutex_);
    const auto found = mediaSlots_->videoSlots_.find(kScreenMainVideoSlot);
    if (found == mediaSlots_->videoSlots_.end() || found->second.transceiver != transceiver ||
        found->second.contentPolicyRevision != revision) {
        // Metadata can change during the external RTP call. The reset marks
        // restoration pending, so the stale transaction is never confirmed.
        if (result.ok() && found != mediaSlots_->videoSlots_.end() && found->second.transceiver == transceiver) {
            auto& binding = found->second;
            // Record what the sender accepted without confirming an obsolete
            // scene. The next restoration can then return to the current user
            // request, including its original FPS when content awareness is off.
            binding.effectiveWidth = decision.width;
            binding.effectiveHeight = decision.height;
            binding.effectiveMaxFps = decision.senderMaxFps;
            binding.effectiveDesiredBitrateBps = decision.desiredVideoBitrateBps;
            binding.effectiveMaxBitrateBps = decision.senderMaxBitrateBps;
            binding.contentPolicyNeedsRestore = true;
            binding.contentPolicyExecution.applied = false;
            binding.contentPolicyExecution.pending = false;
            binding.contentPolicyExecution.ownsSender = false;
            binding.contentPolicyExecution.status = "context_changed_restoring";
            UpdateScreenQualityProtectionLocked();
        }
        return result.ok() ? webrtc::RTCError::InvalidState("The scene context changed during execution.") : result;
    }
    auto& binding = found->second;
    auto& execution = binding.contentPolicyExecution;
    execution.pending = false;
    if (!result.ok()) {
        execution.ownsSender = false;
        execution.status = "apply_failed";
        execution.error = std::string(result.message());
        return result;
    }
    binding.effectiveWidth = decision.width;
    binding.effectiveHeight = decision.height;
    binding.effectiveMaxFps = decision.senderMaxFps;
    binding.effectiveDesiredBitrateBps = decision.desiredVideoBitrateBps;
    binding.effectiveMaxBitrateBps = decision.senderMaxBitrateBps;
    binding.contentPolicyEvidenceNotBeforeMs = SteadyNowMs();
    binding.contentPolicyNeedsRestore = false;
    execution.applied = !restoreUserRequest;
    execution.ownsSender = !restoreUserRequest;
    execution.appliedWidth = decision.width;
    execution.appliedHeight = decision.height;
    execution.appliedMaxFps = decision.senderMaxFps;
    execution.appliedMaxBitrateBps = decision.senderMaxBitrateBps;
    execution.status = restoreUserRequest
        ? (screenContentPolicyObservation_.enabled ? "context_restored" : "disabled_restored")
        : decision.userSpecificationRestoreRequired ? "user_specification_restored"
        : decision.reason == ContentAwareStreamReason::kEmergencyNetworkReduction ? "emergency_applied"
        : decision.qualityRepairRequired ? "repair_applied" : "applied";
    execution.error.clear();
    ++execution.successfulChanges;
    if (restoreUserRequest) {
        binding.contentPolicyState = {};
        ResetAdaptiveScreenFrameRate(&binding.adaptiveFrameRate,
            false,
            binding.configuredMaxFrameRate,
            binding.configuredOutputWidth, binding.configuredOutputHeight, SteadyNowMs());
        binding.adaptiveFrameRate.effectiveFrameRate = decision.senderMaxFps;
    } else {
        auto effective = input;
        effective.currentWidth = decision.width;
        effective.currentHeight = decision.height;
        effective.currentFrameRate = decision.senderMaxFps;
        effective.currentDesiredVideoBitrateBps = decision.desiredVideoBitrateBps;
        effective.currentSenderMaxBitrateBps = decision.senderMaxBitrateBps;
        effective.nowMs = SteadyNowMs();
        ConfirmContentAwareStreamApplied(binding.contentPolicyState, effective,
            decision.modelCalibrated && decision.qualityRequirementMet);
    }
    UpdateScreenQualityProtectionLocked();
    return result;
}

}  // namespace remote
