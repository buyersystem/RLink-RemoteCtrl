// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "InProcessSessionEngine.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "ScreenShareCoordinator.h"
#include "InProcessSessionEngineInternal.h"
#include "SessionDataChannelPolicy.h"
#include "SessionDiagnosticsFormatting.h"
#include "VideoPipelinePreferenceNames.h"
#include "src/platform/win/WindowsDesktopCaptureSource.h"
#include "src/protocol/DataChannelCatalog.h"
#include "src/protocol/ScreenShareControlProtocol.h"
#include "src/webrtc/LibWebRtcSession.h"

namespace remote::app {

bool InProcessSessionEngine::DispatchRoomPairScreenData(
    const std::string& pairId,
    std::span<const std::uint8_t> payload)
{
        SharedDisplayLayout sharedDisplay;
        if (DecodeSharedDisplayLayout(payload, &sharedDisplay)) {
            {
                std::lock_guard lock(mutex_);
                const auto pairIt = roomPairs_.find(pairId);
                auto snapshotIt = std::find_if(
                    snapshot_.roomActivity.peerConnections.begin(),
                    snapshot_.roomActivity.peerConnections.end(),
                    [&pairId](const auto& current) {
                        return current.pairId == pairId;
                    });
                if (pairIt == roomPairs_.end() ||
                    snapshotIt == snapshot_.roomActivity.peerConnections.end() ||
                    sharedDisplay.roomId != snapshot_.room.roomId ||
                    sharedDisplay.senderDeviceId !=
                        pairIt->second->peerDeviceId ||
                    snapshot_.room.screenSharerDeviceId !=
                        sharedDisplay.senderDeviceId ||
                    sharedDisplay.screenShareGeneration !=
                        snapshot_.room.screenShareEpoch) {
                    return true;
                }
                snapshotIt->sharedDisplayLayoutVersion =
                    sharedDisplay.layoutVersion;
                snapshotIt->sharedDisplayId =
                    sharedDisplay.selectedDisplay.sessionDisplayId;
                snapshotIt->sharedDisplayName =
                    sharedDisplay.selectedDisplay.friendlyName;
                snapshotIt->sharedDisplayStableKey =
                    sharedDisplay.selectedDisplay.stableDisplayKey;
                snapshotIt->sharedDisplayWidth =
                    sharedDisplay.selectedDisplay.width;
                snapshotIt->sharedDisplayHeight =
                    sharedDisplay.selectedDisplay.height;
            }
            PublishSnapshot();
            return true;
        }

        ScreenCaptureRuntimeCapability runtimeCapability;
        if (DecodeScreenCaptureRuntimeCapability(
                payload, &runtimeCapability)) {
            {
                std::lock_guard lock(mutex_);
                const auto pairIt = roomPairs_.find(pairId);
                auto snapshotIt = std::find_if(
                    snapshot_.roomActivity.peerConnections.begin(),
                    snapshot_.roomActivity.peerConnections.end(),
                    [&pairId](const auto& current) {
                        return current.pairId == pairId;
                    });
                if (pairIt == roomPairs_.end() ||
                    snapshotIt == snapshot_.roomActivity.peerConnections.end() ||
                    runtimeCapability.roomId != snapshot_.room.roomId ||
                    runtimeCapability.senderDeviceId !=
                        pairIt->second->peerDeviceId ||
                    snapshot_.room.screenSharerDeviceId !=
                        runtimeCapability.senderDeviceId) {
                    return true;
                }
                snapshotIt->screenMaximumFrameRate =
                    runtimeCapability.maximumFrameRate;
                snapshotIt->screenCaptureCapabilityReported = true;
                snapshotIt->screenCaptureConfiguredBackend =
                    runtimeCapability.configuredBackend;
                snapshotIt->screenCaptureActiveBackend =
                    runtimeCapability.activeBackend;
                snapshotIt->screenCaptureFallbackReason =
                    runtimeCapability.fallbackReason;
            }
            PublishSnapshot();
            return true;
        }

        ScreenCaptureCapability captureCapability;
        if (DecodeScreenCaptureCapability(payload, &captureCapability)) {
            {
                std::lock_guard lock(mutex_);
                const auto pairIt = roomPairs_.find(pairId);
                auto snapshotIt = std::find_if(
                    snapshot_.roomActivity.peerConnections.begin(),
                    snapshot_.roomActivity.peerConnections.end(),
                    [&pairId](const auto& current) {
                        return current.pairId == pairId;
                    });
                if (pairIt == roomPairs_.end() ||
                    snapshotIt == snapshot_.roomActivity.peerConnections.end() ||
                    captureCapability.roomId != snapshot_.room.roomId ||
                    captureCapability.senderDeviceId !=
                        pairIt->second->peerDeviceId ||
                    snapshot_.room.screenSharerDeviceId !=
                        captureCapability.senderDeviceId) {
                    return true;
                }
                snapshotIt->screenMaximumFrameRate =
                    captureCapability.maximumFrameRate;
            }
            PublishSnapshot();
            return true;
        }

        ScreenStreamPreferenceApplied applied;
        const bool generationBoundPreferenceResult =
            DecodeScreenStreamPreferenceAppliedV2(payload, &applied);
        if (generationBoundPreferenceResult ||
            DecodeScreenStreamPreferenceApplied(payload, &applied)) {
            {
                std::lock_guard lock(mutex_);
                const auto pairIt = roomPairs_.find(pairId);
                auto snapshotIt = std::find_if(
                    snapshot_.roomActivity.peerConnections.begin(),
                    snapshot_.roomActivity.peerConnections.end(),
                    [&pairId](const auto& current) {
                        return current.pairId == pairId;
                    });
                if (pairIt == roomPairs_.end() ||
                    snapshotIt == snapshot_.roomActivity.peerConnections.end() ||
                    applied.roomId != snapshot_.room.roomId ||
                    applied.senderDeviceId != pairIt->second->peerDeviceId ||
                    !snapshotIt->screenPreferencePending ||
                    applied.requestSequence !=
                        snapshotIt->screenPreferenceSequence ||
                    snapshotIt->screenPreferenceGeneration == 0 ||
                    snapshotIt->screenPreferenceGeneration !=
                        snapshot_.room.screenShareEpoch ||
                    (generationBoundPreferenceResult &&
                     applied.screenShareGeneration !=
                         snapshotIt->screenPreferenceGeneration)) {
                    return true;
                }
                snapshotIt->screenPreferencePending = false;
                if (applied.accepted) {
                    snapshotIt->screenPreferenceAcceptedSequence =
                        applied.requestSequence;
                    snapshotIt->screenPreferenceAcceptedGeneration =
                        snapshotIt->screenPreferenceGeneration;
                    snapshotIt->screenWidth = applied.width;
                    snapshotIt->screenHeight = applied.height;
                    snapshotIt->screenFramesPerSecond =
                        applied.framesPerSecond;
                    snapshotIt->screenMaxBitrateBps =
                        applied.maxBitrateBps;
                    snapshotIt->screenScaleBackend =
                        static_cast<std::uint8_t>(applied.scaleBackend);
                    snapshotIt->errorCode.clear();
                    snapshotIt->errorMessage.clear();
                } else {
                    snapshotIt->errorCode =
                        "screen_stream_preference_rejected";
                    snapshotIt->errorMessage = applied.error;
                }
            }
            PublishSnapshot();
            return true;
        }

        ScreenStreamPreferenceRequest streamRequest;
        bool decodedStreamRequest = DecodeScreenStreamPreferenceRequest(
            payload, &streamRequest);
        if (!decodedStreamRequest) {
            ScreenFrameRateRequest legacyRequest;
            if (DecodeScreenFrameRateRequest(payload, &legacyRequest)) {
                streamRequest.roomId = std::move(legacyRequest.roomId);
                streamRequest.senderDeviceId =
                    std::move(legacyRequest.senderDeviceId);
                streamRequest.sequence = legacyRequest.sequence;
                streamRequest.framesPerSecond =
                    legacyRequest.framesPerSecond;
                streamRequest.quality = ScreenQualityTier::kOriginal;
                decodedStreamRequest = true;
            }
        }
        if (decodedStreamRequest) {
            std::uint32_t captureFrameRate = 30;
            std::uint32_t maximumFrameRate = 60;
            std::uint32_t sourceWidth = 0;
            std::uint32_t sourceHeight = 0;
            std::uint64_t screenShareGeneration = 0;
            std::uint32_t previousCaptureFrameRate = 0;
            bool preflightPreference = false;
            std::shared_ptr<RoomPairRuntime> pair;
            webrtc::scoped_refptr<WindowsDesktopCaptureSource> source;
            {
                std::lock_guard lock(mutex_);
                const auto pairIt = roomPairs_.find(pairId);
                const bool activeLocalShare =
                    snapshot_.room.screenShareState ==
                        RoomScreenShareState::kActive &&
                    snapshot_.room.screenSharerDeviceId ==
                        snapshot_.localDeviceId;
                const bool startingLocalShare =
                    snapshot_.room.screenShareState ==
                        RoomScreenShareState::kSwitching &&
                    snapshot_.room.pendingScreenSharerDeviceId ==
                        snapshot_.localDeviceId;
                if (pairIt == roomPairs_.end() ||
                    snapshot_.room.membership !=
                        RoomMembershipState::kActive ||
                    (!activeLocalShare && !startingLocalShare) ||
                    streamRequest.roomId != snapshot_.room.roomId ||
                    streamRequest.roomId != pairIt->second->roomId ||
                    streamRequest.senderDeviceId !=
                        pairIt->second->peerDeviceId ||
                    streamRequest.sequence <=
                        pairIt->second->lastScreenControlSequence) {
                    return true;
                }
                pairIt->second->lastScreenControlSequence =
                    streamRequest.sequence;
                source = screenShare_.CaptureSource();
                previousCaptureFrameRate = source ? source->TargetFrameRate() : 0;
                maximumFrameRate = ScreenShareCoordinator::MaximumCaptureFrameRate(
                    options_.desktopCaptureImplementation,
                    source.get());
                const std::size_t onlineMemberCount =
                    static_cast<std::size_t>(std::count_if(
                        snapshot_.room.members.begin(),
                        snapshot_.room.members.end(),
                        [](const RoomMemberSnapshot& member) {
                            return member.online;
                        }));
                std::uint32_t effectiveFrameRateLimit =
                    maximumFrameRate;
                if (onlineMemberCount >
                    kHighOccupancyRoomMemberThreshold) {
                    effectiveFrameRateLimit = (std::min)(
                        effectiveFrameRateLimit,
                        kMultiMemberMaximumScreenFrameRate);
                }
                streamRequest.framesPerSecond = (std::min)(
                    streamRequest.framesPerSecond,
                    effectiveFrameRateLimit);
                // Calculate capture arbitration with the proposed request,
                // but only publish it after the RTP sender accepts it.
                captureFrameRate = (std::max)(5u, streamRequest.framesPerSecond);
                for (const auto& [requestPairId, preference] :
                     roomSession_.screenStreamPreferences_) {
                    if (requestPairId == pairId) continue;
                    captureFrameRate = (std::max)(
                        captureFrameRate, preference.framesPerSecond);
                }
                screenShareGeneration =
                    snapshot_.room.screenShareEpoch;
                pair = pairIt->second;
                sourceWidth = source ? source->CapturedWidth() : 0;
                sourceHeight = source ? source->CapturedHeight() : 0;
                preflightPreference =
                    startingLocalShare && !source;
                if (preflightPreference) {
                    const auto* display = FindDisplayByStableKey(
                        snapshot_.screenShare.topology,
                        snapshot_.screenShare.selectedDisplayKey);
                    if (!display) {
                        display = FindPrimaryDisplay(
                            snapshot_.screenShare.topology);
                    }
                    if (display) {
                        sourceWidth = display->width;
                        sourceHeight = display->height;
                    }
                }
            }

            ScreenStreamPreferenceApplied response;
            response.roomId = streamRequest.roomId;
            response.senderDeviceId = Snapshot().localDeviceId;
            response.requestSequence = streamRequest.sequence;
            response.screenShareGeneration =
                screenShareGeneration;
            if (source &&
                !source->SetTargetFrameRate(captureFrameRate)) {
                response.error = "The desktop capture source rejected the requested frame rate.";
            } else if (sourceWidth < 2 || sourceHeight < 2) {
                response.error = preflightPreference
                    ? "The selected display is not available for screen-share preflight."
                    : "The first desktop frame is not ready yet.";
            } else {
                const auto effective = ScreenShareCoordinator::ResolvePolicy(
                    sourceWidth, sourceHeight, streamRequest);
                ScreenStreamPolicyResult appliedPolicy;
                const auto result = pair->session->SetVideoSlotEncodingPolicy(
                    kScreenMainVideoSlot,
                    effective.framesPerSecond,
                    effective.width,
                    effective.height, &appliedPolicy);
                response.accepted = result.ok();
                response.width = effective.width;
                response.height = effective.height;
                response.framesPerSecond = effective.framesPerSecond;
                response.maxBitrateBps = result.ok() ? appliedPolicy.maxBitrateBps : 0;
                response.scaleBackend = ScreenScaleBackend::kWebRtc;
                if (!result.ok()) {
                    response.error = std::string(result.message());
                }
            }

            bool restoreCaptureFrameRate = false;
            {
                std::lock_guard lock(mutex_);
                const auto currentPair = roomPairs_.find(pairId);
                const bool sameContext = currentPair != roomPairs_.end() &&
                    currentPair->second == pair &&
                    snapshot_.room.screenShareEpoch == screenShareGeneration;
                if (response.accepted && sameContext) {
                    roomSession_.screenStreamPreferences_[pairId] = streamRequest;
                    roomSession_.localScreenFrameRate_ = captureFrameRate;
                } else if (!response.accepted && sameContext && source &&
                    screenShare_.CaptureSource() == source &&
                    source->TargetFrameRate() == captureFrameRate) {
                    restoreCaptureFrameRate = previousCaptureFrameRate != 0;
                }
            }
            if (restoreCaptureFrameRate) {
                (void)source->SetTargetFrameRate(previousCaptureFrameRate);
            }

            ScreenCaptureCapability capability;
            capability.roomId = response.roomId;
            capability.senderDeviceId = response.senderDeviceId;
            capability.maximumFrameRate = maximumFrameRate;
            std::vector<std::uint8_t> capabilityBytes;
            if (EncodeScreenCaptureCapability(
                    capability, &capabilityBytes)) {
                pair->controller->SendData(
                    std::string(kControlReliableChannel),
                    capabilityBytes, true);
            }

            ScreenCaptureRuntimeCapability runtimeCapability;
            runtimeCapability.roomId = response.roomId;
            runtimeCapability.senderDeviceId = response.senderDeviceId;
            runtimeCapability.maximumFrameRate = maximumFrameRate;
            runtimeCapability.configuredBackend =
                DesktopCaptureImplementationName(
                    options_.desktopCaptureImplementation);
            runtimeCapability.activeBackend = source
                ? DesktopCaptureBackendName(source->Backend())
                : "not_initialized";
            runtimeCapability.fallbackReason = source
                ? source->FallbackReason()
                : "desktop_capture_source_not_initialized";
            std::vector<std::uint8_t> runtimeCapabilityBytes;
            if (EncodeScreenCaptureRuntimeCapability(
                    runtimeCapability, &runtimeCapabilityBytes)) {
                pair->controller->SendData(
                    std::string(kControlReliableChannel),
                    runtimeCapabilityBytes, true);
            }

            // Send the generation-bound result first. Current peers consume
            // it and then ignore the following legacy duplicate because the
            // request is no longer pending; older peers ignore type-11 and
            // continue to consume type-3.
            std::vector<std::uint8_t> responseBytes;
            if (EncodeScreenStreamPreferenceAppliedV2(
                    response, &responseBytes)) {
                pair->controller->SendData(
                    std::string(kControlReliableChannel),
                    responseBytes, true);
            }
            if (EncodeScreenStreamPreferenceApplied(
                    response, &responseBytes)) {
                pair->controller->SendData(
                    std::string(kControlReliableChannel),
                    responseBytes, true);
            }
            if (!response.accepted) {
                {
                    std::lock_guard lock(mutex_);
                    snapshot_.room.errorCode =
                        "screen_stream_preference_failed";
                    snapshot_.room.errorMessage = response.error;
                }
                PublishSnapshot();
            }
            BroadcastSharedDisplayLayout();
            BroadcastSharedDisplayCatalog();
            return true;
        }
    return false;
}

}  // namespace remote::app
