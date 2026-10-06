// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "LibWebRtcSession.Internal.h"
#include "MediaSlotManager.h"
#include "GoogCcTelemetry.h"
#include "src/core/ScreenFrameQualityPolicy.h"

namespace remote {
using namespace webrtc_session_detail;

webrtc::RTCError LibWebRtcSession::PrepareVideoTransceiverSlot(
    const std::string& slot)
{
    if (slot.empty()) {
        return webrtc::RTCError::InvalidParameter(
            "The video slot name is empty.");
    }
    auto peer = PeerConnection();
    if (!peer || !factory_) {
        return webrtc::RTCError::InvalidState(
            "PeerConnection is not ready.");
    }
    {
        std::lock_guard lock(mutex_);
        if (mediaSlots_->videoSlots_.contains(slot)) {
            return webrtc::RTCError::OK();
        }
    }

    webrtc::RtpTransceiverInit init;
    init.direction = webrtc::RtpTransceiverDirection::kSendRecv;
    auto transceiverOrError =
        peer->AddTransceiver(webrtc::MediaType::VIDEO, init);
    if (!transceiverOrError.ok()) {
        return transceiverOrError.MoveError();
    }

    auto codecs = H264CodecPreferences(factory_.get());
    const bool hasH264 = std::any_of(
        codecs.begin(), codecs.end(), [](const auto& codec) {
            return EqualsIgnoreCase(codec.name, "H264");
        });
    if (!hasH264) {
        return webrtc::RTCError::UnsupportedOperation(
            "H264 is unavailable for the video slot.");
    }
    auto transceiver = transceiverOrError.MoveValue();
    auto codecResult = transceiver->SetCodecPreferences(codecs);
    if (!codecResult.ok()) {
        return codecResult;
    }

    {
        std::lock_guard lock(mutex_);
        if (mediaSlots_->videoSlots_.contains(slot)) {
            return webrtc::RTCError::InvalidModification(
                "The video slot was concurrently prepared.");
        }
        MediaSlotManager::VideoSlotBinding binding;
        binding.transceiver = std::move(transceiver);
        mediaSlots_->videoSlots_.emplace(slot, std::move(binding));
        mediaSlots_->videoSlotOrder_.push_back(slot);
    }
    return webrtc::RTCError::OK();
}

webrtc::RTCError LibWebRtcSession::BindNegotiatedVideoTransceiverSlots(
    const std::vector<std::string>& slots)
{
    if (slots.empty() ||
        std::any_of(slots.begin(), slots.end(),
                    [](const std::string& slot) { return slot.empty(); })) {
        return webrtc::RTCError::InvalidParameter(
            "At least one named video slot is required.");
    }
    auto peer = PeerConnection();
    if (!peer || !factory_) {
        return webrtc::RTCError::InvalidState(
            "PeerConnection is not ready.");
    }
    {
        std::lock_guard lock(mutex_);
        if (!mediaSlots_->videoSlots_.empty()) {
            return webrtc::RTCError::InvalidState(
                "Video slots are already bound.");
        }
    }

    std::vector<webrtc::scoped_refptr<
        webrtc::RtpTransceiverInterface>> negotiatedVideo;
    for (const auto& transceiver : peer->GetTransceivers()) {
        if (transceiver &&
            transceiver->media_type() == webrtc::MediaType::VIDEO &&
            transceiver->mid().has_value() && !transceiver->stopped()) {
            negotiatedVideo.push_back(transceiver);
        }
    }
    if (negotiatedVideo.size() < slots.size()) {
        return webrtc::RTCError::InvalidState(
            "The remote offer does not contain all required video slots.");
    }

    auto codecs = H264CodecPreferences(factory_.get());
    const bool hasH264 = std::any_of(
        codecs.begin(), codecs.end(), [](const auto& codec) {
            return EqualsIgnoreCase(codec.name, "H264");
        });
    if (!hasH264) {
        return webrtc::RTCError::UnsupportedOperation(
            "H264 is unavailable for the negotiated video slots.");
    }

    std::unordered_map<std::string, MediaSlotManager::VideoSlotBinding> bindings;
    for (std::size_t index = 0; index < slots.size(); ++index) {
        auto& transceiver = negotiatedVideo[index];
        auto directionResult = transceiver->SetDirectionWithError(
            webrtc::RtpTransceiverDirection::kSendRecv);
        if (!directionResult.ok()) {
            return directionResult;
        }
        auto codecResult = transceiver->SetCodecPreferences(codecs);
        if (!codecResult.ok()) {
            return codecResult;
        }

        MediaSlotManager::VideoSlotBinding binding;
        binding.transceiver = transceiver;
        if (transceiver->receiver()) {
            auto remoteTrack = transceiver->receiver()->track();
            if (remoteTrack &&
                remoteTrack->kind() ==
                    webrtc::MediaStreamTrackInterface::kVideoKind) {
                binding.remoteTrack =
                    static_cast<webrtc::VideoTrackInterface*>(
                        remoteTrack.get());
            }
        }
        bindings.emplace(slots[index], std::move(binding));
    }

    {
        std::lock_guard lock(mutex_);
        if (!mediaSlots_->videoSlots_.empty()) {
            return webrtc::RTCError::InvalidModification(
                "Video slots were concurrently bound.");
        }
        mediaSlots_->videoSlots_ = std::move(bindings);
        mediaSlots_->videoSlotOrder_ = slots;
    }
    return webrtc::RTCError::OK();
}

webrtc::RTCError LibWebRtcSession::SetVideoSlotTrack(
    const std::string& slot,
    webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track)
{
    if (track &&
        track->kind() != webrtc::MediaStreamTrackInterface::kVideoKind) {
        return webrtc::RTCError::InvalidParameter(
            "Only a video track may be attached to a video slot.");
    }
    webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver;
    {
        std::lock_guard lock(mutex_);
        const auto found = mediaSlots_->videoSlots_.find(slot);
        if (found == mediaSlots_->videoSlots_.end() || !found->second.transceiver) {
            return webrtc::RTCError::InvalidState(
                "The requested video slot is not prepared.");
        }
        transceiver = found->second.transceiver;
    }
    auto sender = transceiver->sender();
    if (!sender || !sender->SetTrack(track.get())) {
        return webrtc::RTCError::InternalError(
            "The video sender rejected the track.");
    }
    return webrtc::RTCError::OK();
}

webrtc::RTCError LibWebRtcSession::SetVideoSlotSendingActive(
    const std::string& slot,
    bool active)
{
    webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver;
    {
        std::lock_guard lock(mutex_);
        const auto found = mediaSlots_->videoSlots_.find(slot);
        if (found == mediaSlots_->videoSlots_.end() || !found->second.transceiver) {
            return webrtc::RTCError::InvalidState(
                "The requested video slot is not prepared.");
        }
        transceiver = found->second.transceiver;
    }
    auto sender = transceiver->sender();
    if (!sender) {
        return webrtc::RTCError::InvalidState(
            "The requested video slot has no RTP sender.");
    }
    std::unique_lock senderParametersLock(videoSenderParametersMutex_);
    {
        std::lock_guard lock(mutex_);
        const auto found = mediaSlots_->videoSlots_.find(slot);
        if (found == mediaSlots_->videoSlots_.end() || found->second.transceiver != transceiver)
            return webrtc::RTCError::InvalidState("The video slot changed before activation.");
    }
    auto parameters = sender->GetParameters();
    if (parameters.encodings.empty()) {
        return webrtc::RTCError::InvalidState(
            "The video sender has no encoding parameters.");
    }
    for (auto& encoding : parameters.encodings) {
        encoding.active = active;
    }
    auto result = sender->SetParameters(parameters);
    if (!result.ok()) {
        return result;
    }

    // A bitrate policy may be negotiated before a real screen track exists.
    // Do not consume its one-shot BWE bootstrap at that preflight boundary.
    // The bootstrap belongs to the real inactive -> active transition, after
    // SetTrack has attached the generation's capture source. If transport is
    // not connected yet, the connection callback applies it later.
    {
        std::lock_guard lock(mutex_);
        const auto found = mediaSlots_->videoSlots_.find(slot);
        if (found != mediaSlots_->videoSlots_.end() &&
            found->second.transceiver == transceiver) {
            found->second.sendingActive = active;
            if (!active) {
                found->second.startBitrateBootstrapPending = true;
                if (slot == kScreenMainVideoSlot) {
                    progressiveBitrateCeiling_ = {};
                    ++progressiveBitrateCeilingRevision_;
                    progressiveBitrateCeilingError_.clear();
                    found->second.adaptiveFrameRate = {};
                    ++found->second.adaptiveFrameRateRevision;
                    found->second.adaptiveFrameRateError.clear();
                }
            } else if (slot == kScreenMainVideoSlot &&
                       found->second.configuredMaxFrameRate > 0) {
                ResetAdaptiveScreenFrameRate(
                    &found->second.adaptiveFrameRate,
                    false,
                    found->second.configuredMaxFrameRate,
                    found->second.configuredOutputWidth,
                    found->second.configuredOutputHeight,
                    SteadyNowMs());
                ++found->second.adaptiveFrameRateRevision;
                found->second.adaptiveFrameRateError.clear();
            }
        }
        UpdateScreenQualityProtectionLocked();
    }
    senderParametersLock.unlock();
    if (active) {
        ApplyPendingVideoStartBitrateBootstrap();
    } else {
        // A share may stop before the bounded startup burst completes. Never
        // leave its temporary probe floor installed on this long-lived room
        // PeerConnection.
        FinishVideoSlotBandwidthBootstrap(slot);
    }
    return result;
}

void LibWebRtcSession::SetFastDesktopBweStartupEnabled(bool enabled)
{
    std::lock_guard lock(mutex_);
    fastDesktopBweStartup_ = enabled;
    if (!enabled) {
        progressiveBitrateCeiling_ = {};
        ++progressiveBitrateCeilingRevision_;
        progressiveBitrateCeilingError_.clear();
    }
}

void LibWebRtcSession::SetAdaptiveDesktopNetworkFrameRateEnabled(
    bool enabled)
{
    std::lock_guard lock(mutex_);
    // The public switch enables encoder quality protection, not the legacy
    // FPS-cap controller. Content awareness only supplies a smooth coefficient.
    adaptiveDesktopNetworkFrameRate_ = enabled;
    const auto found = mediaSlots_->videoSlots_.find(kScreenMainVideoSlot);
    if (found != mediaSlots_->videoSlots_.end()) {
        ResetAdaptiveScreenFrameRate(
            &found->second.adaptiveFrameRate,
            false,
            found->second.configuredMaxFrameRate,
            found->second.configuredOutputWidth,
            found->second.configuredOutputHeight,
            SteadyNowMs());
        ++found->second.adaptiveFrameRateRevision;
        found->second.adaptiveFrameRateError.clear();
    }
    UpdateScreenQualityProtectionLocked();
}

void LibWebRtcSession::SetScreenContentActivity(
    ScreenContentActivity activity)
{
    std::lock_guard lock(mutex_);
    screenContentActivity_ = activity;
}

void LibWebRtcSession::RestartVideoSlotBandwidthEstimation(
    const std::string& slot)
{
    webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver;
    std::uint64_t startBitrate = 0;
    std::uint64_t maxBitrate = 0;
    {
        std::lock_guard lock(mutex_);
        const auto found = mediaSlots_->videoSlots_.find(slot);
        if (!fastDesktopBweStartup_ || found == mediaSlots_->videoSlots_.end() ||
            !found->second.sendingActive ||
            found->second.configuredStartBitrateBps == 0) {
            return;
        }
        // The inactive -> active transition can precede delivery of the
        // capture source's first real frame. Re-arm the same one-shot prior
        // after that frame enters WebRTC so an application-limited desktop
        // source cannot consume its startup probe before media is ready.
        found->second.startBitrateBootstrapPending = true;
        ++found->second.mediaReadyBitrateRestarts;
        transceiver = found->second.transceiver;
        startBitrate = found->second.configuredStartBitrateBps;
        maxBitrate = found->second.configuredNetworkProbeMaxBitrateBps;
        if (progressiveBitrateCeiling_.enabled &&
            progressiveBitrateCeiling_.appliedMaxBitrateBps > 0) {
            maxBitrate = progressiveBitrateCeiling_.appliedMaxBitrateBps;
            startBitrate = (std::min)(startBitrate, maxBitrate);
        }
    }
    ApplyPendingVideoStartBitrateBootstrap();

    auto peer = PeerConnection();
    webrtc::RTCError floorResult = webrtc::RTCError::InvalidState(
        "PeerConnection is not ready for the startup probe floor.");
    if (peer) {
        const int boundedStartBitrate = static_cast<int>((std::min)(
            startBitrate,
            static_cast<std::uint64_t>(
                (std::numeric_limits<int>::max)())));
        const int boundedMaxBitrate = static_cast<int>((std::min)(
            maxBitrate,
            static_cast<std::uint64_t>(
                (std::numeric_limits<int>::max)())));
        webrtc::BitrateSettings floorSettings;
        floorSettings.min_bitrate_bps = (std::min)(
            kDesktopStartupProbeFloorBps, boundedStartBitrate);
        floorSettings.start_bitrate_bps = boundedStartBitrate;
        floorSettings.max_bitrate_bps = boundedMaxBitrate;
        floorResult = peer->SetBitrate(floorSettings);
        if (floorResult.ok()) {
            peer->ReconfigureBandwidthEstimation(
                webrtc::BandwidthEstimationSettings{
                    .allow_probe_without_media = true});
        }
    }
    {
        std::lock_guard lock(mutex_);
        const auto found = mediaSlots_->videoSlots_.find(slot);
        if (found != mediaSlots_->videoSlots_.end() &&
            found->second.transceiver == transceiver) {
            found->second.bitrateProbeFloorActive = floorResult.ok();
            if (!floorResult.ok()) {
                found->second.bitrateBootstrapError =
                    std::string(floorResult.message());
            }
        }
    }

    std::string pulseError;
    const bool pulseSucceeded = PulseVideoSlotAllocationProbe(
        transceiver, startBitrate, &pulseError);
    {
        std::lock_guard lock(mutex_);
        const auto found = mediaSlots_->videoSlots_.find(slot);
        if (found != mediaSlots_->videoSlots_.end() &&
            found->second.transceiver == transceiver) {
            if (pulseSucceeded) {
                ++found->second.allocationProbePulses;
                if (floorResult.ok()) {
                    found->second.bitrateBootstrapError.clear();
                }
            } else if (!pulseError.empty()) {
                found->second.bitrateBootstrapError =
                    std::move(pulseError);
            }
        }
    }
}

void LibWebRtcSession::FinishVideoSlotBandwidthBootstrap(
    const std::string& slot)
{
    webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver;
    std::uint64_t startBitrate = 0;
    std::uint64_t maxBitrate = 0;
    {
        std::lock_guard lock(mutex_);
        const auto found = mediaSlots_->videoSlots_.find(slot);
        if (found == mediaSlots_->videoSlots_.end() ||
            !found->second.bitrateProbeFloorActive) {
            return;
        }
        transceiver = found->second.transceiver;
        startBitrate = found->second.configuredStartBitrateBps;
        maxBitrate = found->second.configuredNetworkProbeMaxBitrateBps;
        if (progressiveBitrateCeiling_.enabled &&
            progressiveBitrateCeiling_.appliedMaxBitrateBps > 0) {
            maxBitrate = progressiveBitrateCeiling_.appliedMaxBitrateBps;
            startBitrate = (std::min)(startBitrate, maxBitrate);
        }
    }

    auto peer = PeerConnection();
    webrtc::RTCError result = webrtc::RTCError::InvalidState(
        "PeerConnection is not ready to release the startup probe floor.");
    if (peer) {
        webrtc::BitrateSettings settings;
        settings.min_bitrate_bps = static_cast<int>((std::min)(maxBitrate,
            static_cast<std::uint64_t>(kDefaultWebRtcMinimumBitrateBps)));
        settings.max_bitrate_bps = static_cast<int>((std::min)(
            maxBitrate,
            static_cast<std::uint64_t>(
                (std::numeric_limits<int>::max)())));
        result = peer->SetBitrate(settings);
    }
    std::string pulseError;
    const bool pulseSucceeded = result.ok() &&
        PulseVideoSlotAllocationProbe(
            transceiver, startBitrate, &pulseError);
    {
        std::lock_guard lock(mutex_);
        const auto found = mediaSlots_->videoSlots_.find(slot);
        if (found == mediaSlots_->videoSlots_.end() ||
            found->second.transceiver != transceiver) {
            return;
        }
        if (result.ok()) {
            found->second.bitrateProbeFloorActive = false;
            ++found->second.bitrateProbeFloorReleases;
            if (pulseSucceeded) {
                ++found->second.allocationProbePulses;
                found->second.bitrateBootstrapError.clear();
            } else if (!pulseError.empty()) {
                found->second.bitrateBootstrapError =
                    std::move(pulseError);
            }
        } else {
            found->second.bitrateBootstrapError =
                std::string(result.message());
        }
    }
}

bool LibWebRtcSession::PulseVideoSlotAllocationProbe(
    webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver,
    std::uint64_t startBitrate,
    std::string* error)
{
    std::lock_guard senderParametersLock(videoSenderParametersMutex_);
    auto sender = transceiver ? transceiver->sender() : nullptr;
    if (sender && startBitrate > 0) {
        auto parameters = sender->GetParameters();
        if (!parameters.encodings.empty()) {
            const int pulseBitrate = static_cast<int>((std::min)(
                startBitrate,
                static_cast<std::uint64_t>(
                    (std::numeric_limits<int>::max)())));
            const auto originalEncodings = parameters.encodings;
            bool changed = false;
            for (auto& encoding : parameters.encodings) {
                const int boundedPulse = encoding.max_bitrate_bps
                    ? (std::min)(pulseBitrate, *encoding.max_bitrate_bps)
                    : pulseBitrate;
                changed = changed || encoding.max_bitrate_bps != boundedPulse;
                encoding.max_bitrate_bps = boundedPulse;
            }
            // A pulse must never raise a media cap. No smaller temporary
            // allocation is possible when the sender is already below start.
            if (!changed) return true;
            auto pulseResult = sender->SetParameters(parameters);
            if (pulseResult.ok()) {
                const auto restore = [&]() {
                    // Get a fresh transaction ID; restore each original optional
                    // cap, never the (possibly much higher) connection ceiling.
                    auto restored = sender->GetParameters();
                    if (restored.encodings.size() != originalEncodings.size()) {
                        return webrtc::RTCError::InvalidState(
                            "The screen RTP encodings changed during the allocation probe.");
                    }
                    for (std::size_t i = 0; i < originalEncodings.size(); ++i) {
                        if (restored.encodings[i].rid != originalEncodings[i].rid) {
                            return webrtc::RTCError::InvalidState(
                                "The screen RTP encoding identity changed during the allocation probe.");
                        }
                        restored.encodings[i].max_bitrate_bps =
                            originalEncodings[i].max_bitrate_bps;
                    }
                    return sender->SetParameters(restored);
                };
                pulseResult = restore();
                if (!pulseResult.ok()) {
                    const auto firstError = std::string(pulseResult.message());
                    const auto retry = restore();
                    if (error) {
                        *error = "Failed to restore the screen media cap: " + firstError;
                        if (!retry.ok()) *error += "; retry: " + std::string(retry.message());
                    }
                    return false;
                }
            }
            if (!pulseResult.ok() && error) {
                *error = std::string(pulseResult.message());
            }
            return pulseResult.ok();
        } else {
            if (error) {
                *error = "The active screen sender has no RTP encoding.";
            }
        }
    } else {
        if (error) {
            *error = "The active screen allocation is not ready.";
        }
    }
    return false;
}

void LibWebRtcSession::SetScreenVideoBitrateBppProvider(
    std::function<std::uint32_t()> provider)
{
    std::lock_guard lock(mutex_);
    screenVideoBitrateBppProvider_ = std::move(provider);
}

void LibWebRtcSession::SetScreenVideoBitrateBpp(std::uint32_t hundredths)
{
    if (hundredths < kMinimumScreenVideoBitrateBppHundredths ||
        hundredths > kMaximumScreenVideoBitrateBppHundredths) return;
    std::lock_guard lock(mutex_);
    liveScreenVideoBitrateBpp_ = hundredths;
}

void LibWebRtcSession::SetScreenQualityDeficitShare(std::uint32_t hundredths)
{
    std::lock_guard lock(mutex_);
    screenQualityDeficitShareHundredths_ = NormalizeScreenQualityDeficitShareHundredths(hundredths);
    const auto now = (std::max)(SteadyNowMs(), sceneQualityLastTickMs_);
    if (!sceneQualityObservation_.enabled)
        sceneQualitySmoother_.Reset(screenQualityDeficitShareHundredths_ / 100.0, now);
    else if (sceneQualitySmoother_.Scene() == media_intelligence::ScreenScene::kUnknown)
        sceneQualitySmoother_.SetTarget(screenQualityDeficitShareHundredths_ / 100.0, now);
    UpdateSceneQualityCoefficientLocked(now);
}

webrtc::RTCError LibWebRtcSession::SetVideoSlotEncodingPolicy(
    const std::string& slot,
    std::uint32_t framesPerSecond,
    std::uint32_t width,
    std::uint32_t height,
    ScreenStreamPolicyResult* appliedPolicy)
{
    if (framesPerSecond < 5 || framesPerSecond > 120) {
        return webrtc::RTCError::InvalidParameter(
            "The video frame rate must be between 5 and 120 FPS.");
    }
    if (width == 0 || height == 0 || width > 7680 || height > 4320) {
        return webrtc::RTCError::InvalidParameter(
            "The target video resolution is invalid.");
    }

    webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver;
    std::function<std::uint32_t()> referenceBppProvider;
    std::optional<std::uint32_t> liveReferenceBpp;
    {
        std::lock_guard lock(mutex_);
        const auto found = mediaSlots_->videoSlots_.find(slot);
        if (found == mediaSlots_->videoSlots_.end() || !found->second.transceiver) {
            return webrtc::RTCError::InvalidState(
                "The requested video slot is not prepared.");
        }
        transceiver = found->second.transceiver;
        if (slot == kScreenMainVideoSlot) {
            referenceBppProvider = screenVideoBitrateBppProvider_;
            liveReferenceBpp = liveScreenVideoBitrateBpp_;
        }
    }
    auto sender = transceiver->sender();
    if (!sender) {
        return webrtc::RTCError::InvalidState(
            "The requested video slot has no RTP sender.");
    }

    // Host configuration reads stay outside session/parameters locks.
    ScreenStreamPolicyRequest policyRequest{width, height, framesPerSecond};
    if (referenceBppProvider) {
        policyRequest.videoBitrateBppHundredths = referenceBppProvider();
    }
    if (liveReferenceBpp) policyRequest.videoBitrateBppHundredths = *liveReferenceBpp;
    std::unique_lock senderParametersLock(videoSenderParametersMutex_);
    auto parameters = sender->GetParameters();
    if (parameters.encodings.empty()) {
        return webrtc::RTCError::InvalidState(
            "The screen RTP sender has no encoding parameters.");
    }

    // Screen resolution and frame rate are explicit viewer preferences. Do
    // not let libwebrtc's generic video adaptation silently lower the frame
    // rate while preserving the requested resolution; doing so makes an
    // 80-FPS request arrive at the encoder as roughly 57 FPS and prevents the
    // hardware MFT from ever switching its media type above 60 FPS.
    //
    // Congestion control still adjusts the encoder bitrate and may drop a
    // frame when queues would otherwise grow. This only prevents automatic
    // resolution/FPS degradation behind the user's quality controls.
    parameters.degradation_preference =
        webrtc::DegradationPreference::MAINTAIN_FRAMERATE_AND_RESOLUTION;

    // libwebrtc otherwise caps every singlecast stream above 960x540 at its
    // generic 2.5 Mbps default. Bound media by the requested workload,
    // separately from the user-configured connection reference.
    // These are user ceilings, not a substitute for GoogCC's safe allocation.
    auto screenPolicy = ResolveScreenStreamPolicy(
        width, height, policyRequest);
    if (slot != kScreenMainVideoSlot) {
        // Preserve the previous camera limits; this preference is screen-only.
        screenPolicy.maxBitrateBps = (std::max)(4'000'000u, screenPolicy.maxBitrateBps);
        screenPolicy.startBitrateBps = (std::min)(screenPolicy.maxBitrateBps,
            (std::max)(2'000'000u, screenPolicy.startBitrateBps));
        auto cameraReferenceRequest = policyRequest;
        cameraReferenceRequest.videoBitrateBppHundredths = 30;
        screenPolicy.networkProbeMaxBitrateBps = (std::max)(4'000'000u,
            ResolveScreenStreamPolicy(width, height, cameraReferenceRequest).maxBitrateBps);
    }
    const std::uint64_t bitrate = screenPolicy.maxBitrateBps;
    const std::uint64_t startBitrate = screenPolicy.startBitrateBps;
    for (auto& encoding : parameters.encodings) {
        encoding.max_framerate =
            static_cast<double>(framesPerSecond);
        encoding.max_bitrate_bps = static_cast<int>(bitrate);
        encoding.scale_resolution_down_to = webrtc::Resolution{
            static_cast<int>(width), static_cast<int>(height)};
    }
    auto result = sender->SetParameters(parameters);
    if (!result.ok()) {
        return result;
    }
    // Store the generation's BWE prior, but do not apply it here. Viewer
    // preference preflight also reaches this method while the screen sender
    // is inactive. Consuming the prior at that point makes the later identical
    // SetBitrate call a no-op inside libwebrtc's BitrateConfigurator, leaving
    // the first real share near its generic low initial estimate. The actual
    // inactive -> active transition owns the one-shot bootstrap.
    bool updateGlobalBitrateLimit = false;
    bool keepStartupProbeFloor = false;
    std::uint64_t globalMaxBitrate = 0;
    bool progressiveCeilingEnabled = false;
    ProgressiveBitrateCeilingDecision progressiveDecision;
    ProgressiveBitrateCeilingState previousProgressiveState;
    std::uint64_t progressiveDecisionRevision = 0;
    {
        std::lock_guard lock(mutex_);
        const auto found = mediaSlots_->videoSlots_.find(slot);
        if (found != mediaSlots_->videoSlots_.end() &&
            found->second.transceiver == transceiver) {
            found->second.configuredMaxFrameRate = framesPerSecond;
            found->second.configuredOutputWidth = width;
            found->second.configuredOutputHeight = height;
            found->second.configuredStartBitrateBps = startBitrate;
            found->second.configuredMaxBitrateBps = bitrate;
            found->second.configuredNetworkProbeMaxBitrateBps =
                screenPolicy.networkProbeMaxBitrateBps;
            found->second.configuredVideoBppHundredths =
                NormalizeScreenVideoBitrateBppHundredths(policyRequest.videoBitrateBppHundredths);
            found->second.effectiveWidth = width;
            found->second.effectiveHeight = height;
            found->second.effectiveMaxFps = framesPerSecond;
            found->second.effectiveDesiredBitrateBps = bitrate;
            found->second.effectiveMaxBitrateBps = bitrate;
            if (slot == kScreenMainVideoSlot) {
                ResetAdaptiveScreenFrameRate(
                    &found->second.adaptiveFrameRate,
                    false,
                    framesPerSecond,
                    width,
                    height,
                    SteadyNowMs());
                ++found->second.adaptiveFrameRateRevision;
                found->second.adaptiveFrameRateError.clear();
            }
            updateGlobalBitrateLimit = found->second.sendingActive;
        }
        if (updateGlobalBitrateLimit) {
            for (const auto& [name, binding] : mediaSlots_->videoSlots_) {
                if (!binding.sendingActive) {
                    continue;
                }
                globalMaxBitrate = (std::max)(
                    globalMaxBitrate,
                    binding.configuredNetworkProbeMaxBitrateBps);
                keepStartupProbeFloor = keepStartupProbeFloor ||
                    binding.bitrateProbeFloorActive;
            }
            progressiveCeilingEnabled = fastDesktopBweStartup_;
            if (progressiveCeilingEnabled && globalMaxBitrate > 0) {
                previousProgressiveState = progressiveBitrateCeiling_;
                if (!progressiveBitrateCeiling_.enabled ||
                    progressiveBitrateCeiling_.appliedMaxBitrateBps == 0) {
                    ResetProgressiveBitrateCeiling(
                        &progressiveBitrateCeiling_,
                        true,
                        globalMaxBitrate,
                        globalMaxBitrate,
                        SteadyNowMs());
                    progressiveDecision.applyPeerConnectionMax = true;
                    progressiveDecision.peerConnectionMaxBitrateBps =
                        globalMaxBitrate;
                } else {
                    progressiveDecision =
                        RetargetProgressiveBitrateCeiling(
                            &progressiveBitrateCeiling_,
                            globalMaxBitrate,
                            SteadyNowMs());
                }
                if (progressiveDecision.applyPeerConnectionMax) {
                    progressiveDecisionRevision =
                        ++progressiveBitrateCeilingRevision_;
                }
            }
        }
        UpdateScreenQualityProtectionLocked();
    }
    senderParametersLock.unlock();

    // RtpSender::SetParameters controls this stream, while SetBitrate controls
    // the PeerConnection-wide BWE constraint. Updating only the sender leaves
    // the old global ceiling behind when the viewer changes resolution.
    // Preserve independent media/connection ceilings without installing a
    // new start-bitrate prior or restarting BWE during ordinary FPS changes.
    // libwebrtc still bounds probing by 2x total media allocation; a higher
    // connection ceiling alone does not guarantee independent capacity discovery.
    if (updateGlobalBitrateLimit && globalMaxBitrate > 0 &&
        progressiveCeilingEnabled) {
        result = ApplyProgressiveBitrateCeilingDecision(
            progressiveDecision,
            progressiveDecisionRevision,
            previousProgressiveState);
        if (!result.ok()) {
            std::lock_guard lock(mutex_);
            const auto found = mediaSlots_->videoSlots_.find(slot);
            if (found != mediaSlots_->videoSlots_.end() &&
                found->second.transceiver == transceiver) {
                found->second.bitrateBootstrapError =
                    std::string(result.message());
            }
            return result;
        }
    } else if (updateGlobalBitrateLimit && globalMaxBitrate > 0) {
        auto peer = PeerConnection();
        if (!peer) {
            return webrtc::RTCError::InvalidState(
                "PeerConnection is not ready for the bitrate policy update.");
        }
        webrtc::BitrateSettings settings;
        settings.min_bitrate_bps = static_cast<int>((std::min)(globalMaxBitrate,
            static_cast<std::uint64_t>(keepStartupProbeFloor ? kDesktopStartupProbeFloorBps : kDefaultWebRtcMinimumBitrateBps)));
        settings.max_bitrate_bps = static_cast<int>((std::min)(
            globalMaxBitrate,
            static_cast<std::uint64_t>(kMaximumScreenConnectionBitrateBps)));
        result = peer->SetBitrate(settings);
        if (!result.ok()) {
            std::lock_guard lock(mutex_);
            const auto found = mediaSlots_->videoSlots_.find(slot);
            if (found != mediaSlots_->videoSlots_.end() &&
                found->second.transceiver == transceiver) {
                found->second.bitrateBootstrapError =
                    std::string(result.message());
            }
            return result;
        }
    }
    if (appliedPolicy) *appliedPolicy = screenPolicy;
    return webrtc::RTCError::OK();
}

void LibWebRtcSession::ApplyPendingVideoStartBitrateBootstrap()
{
    // Every desktop sender needs its start/max bitrate prior installed at the
    // real inactive -> active boundary. fastDesktopBweStartup_ controls only
    // the libwebrtc-specific second, media-ready probe and progressive
    // PeerConnection ceiling; using it to gate this base bootstrap leaves the
    // native DXGI first share on libwebrtc's old low estimate until the user
    // reapplies the same FPS setting.
    std::uint64_t startBitrate = 0;
    std::uint64_t maxBitrate = 0;
    std::uint64_t desiredMaxBitrate = 0;
    std::vector<webrtc::scoped_refptr<
        webrtc::RtpTransceiverInterface>> claimedTransceivers;
    {
        std::lock_guard lock(mutex_);
        if (CombinedConnectionStateLocked() !=
            WebRtcSessionState::kConnected) {
            return;
        }
        for (auto& [slot, binding] : mediaSlots_->videoSlots_) {
            if (!binding.startBitrateBootstrapPending ||
                !binding.sendingActive ||
                binding.configuredStartBitrateBps == 0 ||
                !binding.transceiver) {
                continue;
            }
            binding.startBitrateBootstrapPending = false;
            ++binding.bitrateBootstrapAttempts;
            startBitrate = (std::max)(
                startBitrate, binding.configuredStartBitrateBps);
            maxBitrate = (std::max)(
                maxBitrate, binding.configuredNetworkProbeMaxBitrateBps);
            desiredMaxBitrate = (std::max)(
                desiredMaxBitrate, binding.configuredNetworkProbeMaxBitrateBps);
            claimedTransceivers.push_back(binding.transceiver);
        }
        if (progressiveBitrateCeiling_.enabled &&
            progressiveBitrateCeiling_.appliedMaxBitrateBps > 0) {
            maxBitrate = progressiveBitrateCeiling_.appliedMaxBitrateBps;
            startBitrate = (std::min)(startBitrate, maxBitrate);
        }
    }
    if (startBitrate == 0 || claimedTransceivers.empty()) {
        return;
    }

    auto peer = PeerConnection();
    webrtc::RTCError result = webrtc::RTCError::InvalidState(
        "PeerConnection is not ready for the deferred bitrate bootstrap.");
    if (peer) {
        const int boundedStartBitrate = static_cast<int>((std::min)(
            startBitrate,
            static_cast<std::uint64_t>(kMaximumScreenBitrateBps)));
        const int boundedMaxBitrate = static_cast<int>((std::min)(
            (std::max)(maxBitrate, startBitrate),
            static_cast<std::uint64_t>(kMaximumScreenConnectionBitrateBps)));

        // Install the screen-share start prior before recreating the bandwidth
        // controller. ReconfigureBandwidthEstimation immediately constructs a
        // fresh ProbeController from RtpBitrateConfigurator::GetConfig(). If
        // it runs first, that controller starts probing at libwebrtc's old low
        // estimate and enters kWaitingForProbingResult; a later SetBitrate()
        // updates the constraint but deliberately does not launch another
        // initial probe in that state. This ordering is therefore essential
        // for the first share on a new PeerConnection.
        webrtc::BitrateSettings bootstrapSettings;
        bootstrapSettings.start_bitrate_bps = boundedStartBitrate;
        bootstrapSettings.max_bitrate_bps = boundedMaxBitrate;
        result = peer->SetBitrate(bootstrapSettings);
        if (result.ok()) {
            // Recreate BWE only after the prior is stored. At this point the
            // video sender is active and RTX/transport feedback have already
            // been negotiated, so padding may fill the initial probe even if
            // the desktop encoder is application-limited. This is not a
            // minimum bitrate; the measured path may still back off normally.
            peer->ReconfigureBandwidthEstimation(
                webrtc::BandwidthEstimationSettings{
                    .allow_probe_without_media = true});
        }
    }
    if (!result.ok()) {
        std::lock_guard lock(mutex_);
        for (const auto& transceiver : claimedTransceivers) {
            for (auto& [slot, binding] : mediaSlots_->videoSlots_) {
                if (binding.transceiver == transceiver) {
                    binding.startBitrateBootstrapPending = true;
                    binding.bitrateBootstrapError =
                        std::string(result.message());
                }
            }
        }
        return;
    }
    {
        std::lock_guard lock(mutex_);
        ResetProgressiveBitrateCeiling(
            &progressiveBitrateCeiling_,
            fastDesktopBweStartup_,
            desiredMaxBitrate,
            maxBitrate,
            SteadyNowMs());
        ++progressiveBitrateCeilingRevision_;
        progressiveBitrateCeilingError_.clear();
        for (const auto& transceiver : claimedTransceivers) {
            for (auto& [slot, binding] : mediaSlots_->videoSlots_) {
                if (binding.transceiver == transceiver) {
                    ++binding.bitrateBootstrapSuccesses;
                    binding.bitrateBootstrapError.clear();
                }
            }
        }
    }
    for (const auto& transceiver : claimedTransceivers) {
        if (transceiver && transceiver->sender()) {
            (void)transceiver->sender()->GenerateKeyFrame({});
        }
    }
}

}  // namespace remote
