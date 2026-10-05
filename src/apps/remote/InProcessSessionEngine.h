// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <thread>
#include <unordered_map>
#include <vector>

#include "DirectSessionCoordinator.h"
#include "ISessionMediaAccess.h"
#include "LocalMediaCoordinator.h"
#include "RoomSessionCoordinator.h"
#include "ScreenShareCoordinator.h"
#include "src/core/ISessionEngine.h"
#include "src/core/SessionController.h"
#include "src/signaling/ISignalingClient.h"
#include "src/platform/win/MfH264EncoderCapabilityProbe.h"
#include "src/webrtc/VideoDecoderRuntimeStatus.h"
#include "src/webrtc/VideoEncoderRuntimeStatus.h"
#include "media_intelligence/core/CalibratedStreamQualityModel.h"

namespace remote {

class WebRtcRuntime;
class LibWebRtcSession;
class WindowsCursorMonitor;
class IRemoteVisionFrameAnalyzer;
struct WindowsCursorObservation;
struct RoomMemberActionEnvelope;

namespace testing {
class InProcessSessionEngineTestAccess;
}

namespace app {

struct InProcessSessionMediaState;
class SessionStatsPoller;
class InProcessSessionMediaAdapter;

struct InProcessSessionEngineOptions {
    // Runs startup completion work on the signaling transport's owner thread.
    // Required when a thread-affine signaling client is configured. Returning
    // false reports that the work could not be queued.
    std::function<bool(std::function<void()>)>
        ownerThreadDispatcher;
    bool includeLoopbackAdapter = false;
    bool enableRealDesktopCapture = true;
    bool enableRealCameraCapture = true;
    // Host-owned initial preference; live edits are published through the
    // engine setter and applied on stats completion without capture work.
    std::function<std::uint32_t()> screenVideoBitrateBppProvider;
    std::function<std::uint32_t()> screenQualityDeficitShareProvider;
    DesktopCaptureImplementation desktopCaptureImplementation =
        DesktopCaptureImplementation::kNativeDxgi;
    bool contentAnalyzerEnabled = false;
    std::uint32_t contentAnalyzerRateHz = 3;
    std::shared_ptr<IRemoteVisionFrameAnalyzer> remoteVisionAnalyzer;
    // Optional immutable calibration measured for this codec/backend/profile.
    // No built-in sample table or generic QP threshold is promoted to verified.
    std::shared_ptr<const media_intelligence::CalibratedStreamQualityModel> screenQualityCalibration;
    // Allow a shared H.264 reference curve when no matching measurement is
    // available. Runtime QP/processing and user/network limits still gate it.
    bool allowScreenReferenceQualityModel = true;
    VideoEncoderPreference videoEncoderPreference =
        VideoEncoderPreference::kAutomatic;
    FfmpegX264Preset ffmpegX264Preset = FfmpegX264Preset::kMedium;
    FfmpegHardwareBackend ffmpegHardwareBackend =
        FfmpegHardwareBackend::kAutomatic;
    std::string preferredAutomaticEncoderId;
    VideoDecoderPreference videoDecoderPreference =
        VideoDecoderPreference::kAutomatic;
    std::string preferredHardwareDecoderName;
    std::string hardwareFingerprint;
    std::string operatingSystemDescription;
    std::string nativeArchitecture;
    bool remoteSession = false;
    std::vector<std::string> graphicsAdapterDescriptions;
    std::string graphicsEnumerationError;
    std::optional<MfH264EncoderCapabilityCache>
        encoderCapabilityCache;
    std::string preferredCameraDeviceId =
        kSystemDefaultMediaDeviceId;
    std::string preferredMicrophoneDeviceId =
        kSystemDefaultMediaDeviceId;
    std::string preferredSpeakerDeviceId =
        kSystemDefaultMediaDeviceId;
    int iceMinPort = kDefaultIceMinPort;
    int iceMaxPort = kDefaultIceMaxPort;
    std::chrono::milliseconds negotiationTimeout{15000};
    std::chrono::milliseconds reconnectTimeout{60000};
};

class InProcessSessionEngine final : public ISessionEngine,
                                     private ISignalingClientObserver,
                                     private ISessionSignalingSender,
                                     private ISessionControllerObserver {
public:
    InProcessSessionEngine();
    InProcessSessionEngine(std::unique_ptr<ISignalingClient> signaling,
                           SignalingClientConfig signalingConfig,
                           InProcessSessionEngineOptions options = {});
    ~InProcessSessionEngine() override;

    ISessionMediaAccess* MediaAccess() noexcept;

    InProcessSessionEngine(const InProcessSessionEngine&) = delete;
    InProcessSessionEngine& operator=(const InProcessSessionEngine&) = delete;

    void SetObserver(ISessionEngineObserver* observer) override;
    SessionCommandResult Start() override;
    void Stop() override;
    // Updates only the signaling credential retained for future
    // authentication/reconnect. Active PeerConnections and DataChannels are
    // intentionally left untouched.
    SessionCommandResult UpdateSignalingAccessToken(
        std::string accessToken);
    SessionCommandResult RequestAccountDeletion();
    void SetAccountDeletionResultCallback(
        std::function<void(const SignalingAccountDeletionResult&)> callback);

    SessionEngineSnapshot Snapshot() const override;
    SessionEngineCapabilities Capabilities() const override;
    SessionDiagnosticsSnapshot Diagnostics() const override;
    SessionCommandResult SetScreenVideoBitrateBpp(std::uint32_t hundredths) override;
    SessionCommandResult SetScreenQualityDeficitShare(std::uint32_t hundredths) override;

    SessionCommandResult ConnectDirectDevice(
        const DirectSessionConnectRequest& request) override;
    SessionCommandResult RefreshOwnedDevices() override;
    SessionCommandResult AcceptIncomingSession(
        const std::string& sessionId) override;
    SessionCommandResult RejectIncomingSession(
        const std::string& sessionId) override;
    SessionCommandResult Disconnect() override;
    SessionCommandResult CreateRoom(std::uint32_t capacity) override;
    SessionCommandResult JoinRoom(const std::string& roomId) override;
    SessionCommandResult QueryRoomAvailability(
        const std::vector<std::string>& roomIds) override;
    SessionCommandResult RespondToRoomJoin(
        const std::string& requestId,
        bool accepted) override;
    SessionCommandResult SetRoomCapacity(
        std::uint32_t capacity) override;
    SessionCommandResult LeaveRoom() override;
    // Used after an unrecoverable P2P failure. Local media/session state is
    // torn down immediately; a server leave is sent now or after WSS returns.
    SessionCommandResult ExitRoomAfterRecoveryFailure() override;
    SessionCommandResult RefreshLocalDisplays() override;
    SessionCommandResult SelectRoomScreenShareDisplay(
        const std::string& stableDisplayKey) override;
    SessionCommandResult StartRoomScreenShare() override;
    SessionCommandResult StopRoomScreenShare() override;
    SessionCommandResult RespondToRoomScreenShareSwitch(
        const std::string& requestId,
        bool accepted) override;
    SessionCommandResult CancelRoomScreenShareSwitch() override;
    SessionCommandResult RequestRoomControl() override;
    SessionCommandResult RespondToRoomControl(
        const std::string& requestId,
        bool accepted) override;
    SessionCommandResult ReleaseRoomControl() override;
    SessionCommandResult RequestRoomMemberScreenShare(
        const std::string& peerDeviceId) override;
    SessionCommandResult RespondToRoomMemberScreenShare(
        const std::string& requesterDeviceId,
        std::uint64_t sequence,
        bool accepted) override;
    SessionCommandResult RequestRoomMemberMicrophoneMute(
        const std::string& peerDeviceId) override;
    SessionCommandResult RequestRemoteRoomScreenShareStop(
        const std::string& peerDeviceId,
        std::uint64_t screenShareEpoch) override;
    SessionCommandResult SendRoomInput(
        const RemoteInputEvent& event) override;
    SessionCommandResult SetRoomScreenFrameRate(
        const std::string& pairId,
        std::uint32_t framesPerSecond) override;
    SessionCommandResult SetRoomScreenStreamPreference(
        const std::string& pairId,
        const ScreenStreamPreferenceRequest& preference) override;
    SessionCommandResult QueueRoomScreenStreamPreference(
        const std::string& pairId,
        const ScreenStreamPreferenceRequest& preference,
        std::function<void(SessionCommandResult)> completion) override;
    SessionCommandResult RequestRemoteSharedDisplaySwitch(
        const std::string& pairId,
        const std::string& stableDisplayKey) override;
    SessionCommandResult SendRoomFileMessage(
        const std::string& peerDeviceId,
        const FileTransferMessage& message) override;
    SessionCommandResult SendRoomClipboardMessage(
        const std::string& peerDeviceId,
        const std::string& clipboardSessionId,
        const ClipboardMessage& message) override;
    SessionCommandResult SetLocalCameraEnabled(bool enabled) override;
    SessionCommandResult SetLocalMicrophoneEnabled(bool enabled) override;
    SessionCommandResult SetRoomAudioPlaybackMuted(bool muted) override;
    SessionCommandResult RefreshLocalMediaDevices() override;
    SessionCommandResult SelectLocalCameraDevice(
        const std::string& deviceId) override;
    SessionCommandResult SelectLocalMicrophoneDevice(
        const std::string& deviceId) override;
    SessionCommandResult SelectLocalSpeakerDevice(
        const std::string& deviceId) override;

private:
    friend class InProcessSessionMediaAdapter;

    using RemoteCursorCallback = ISessionMediaAccess::RemoteCursorCallback;
    SessionCommandResult SendDirectInput(const RemoteInputEvent& event);
    SessionCommandResult SendRemoteInput(const RemoteInputEvent& event);
    SessionCommandResult SendRemoteFileMessage(
        const std::string& peerDeviceId,
        const FileTransferMessage& message);
    SessionCommandResult SendRemoteClipboardMessage(
        const std::string& peerDeviceId,
        const std::string& clipboardSessionId,
        const ClipboardMessage& message);
    SessionCommandResult SetDirectScreenStreamPreference(
        const ScreenStreamPreferenceRequest& preference);
    SessionCommandResult QueueDirectScreenStreamPreference(
        const ScreenStreamPreferenceRequest& preference,
        std::function<void(SessionCommandResult)> completion);
    SessionCommandResult SendRoomScreenStreamPreference(
        const std::string& pairId,
        const ScreenStreamPreferenceRequest& preference,
        bool queued,
        std::function<void(SessionCommandResult)> completion);
    SessionCommandResult RequestDirectSharedDisplaySwitch(
        const std::string& stableDisplayKey);
    SessionCommandResult SetRemoteAudioPlaybackMuted(bool muted);

private:
    friend class remote::testing::InProcessSessionEngineTestAccess;

    SessionCommandResult BeginStart();
    SessionCommandResult InitializeRuntimeForStart();
    SessionCommandResult CompleteStart(
        const SessionCommandResult& runtimeResult);
    void CompleteStartOnOwnerThread(
        std::uint64_t startupGeneration,
        const SessionCommandResult& runtimeResult);
    void MarkStartupDispatchFailed(std::uint64_t startupGeneration);

    void OnSignalingStateChanged(SignalingConnectionState state) override;
    void OnDeviceRegistered(const std::string& deviceId) override;
    void OnIncomingSessionRequest(
        const IncomingSessionRequest& request) override;
    void OnSessionResponse(
        const SignalingSessionResponse& response) override;
    void OnSessionPending(
        const SignalingSessionPending& pending) override;
    void OnSessionReady(const SignalingSessionReady& ready) override;
    void OnSessionSuspended(
        const SignalingSessionSuspended& suspended) override;
    void OnSessionResumed(
        const SignalingSessionResumed& resumed) override;
    void OnSessionEnded(const SignalingSessionEnded& ended) override;
    void OnRemoteDescription(
        const SignalingSessionDescription& description) override;
    void OnRemoteIceCandidate(
        const SignalingIceCandidate& candidate) override;
    void OnIceRestartRequested(
        const SignalingIceRestartRequest& request) override;
    void OnIceRestartCancelled(
        const SignalingIceRestartCancel& cancel) override;
    void OnHeartbeatAcknowledged(std::uint32_t roundTripMs) override;
    void OnSignalingError(const std::string& code,
                          const std::string& message) override;
    void OnAccountDeletionResult(
        const SignalingAccountDeletionResult& result) override;
    void OnOwnedDevicesChanged(
        const SignalingOwnedDevicesSnapshot& snapshot) override;
    void OnRoomReady(const SignalingRoomReady& ready) override;
    void OnRoomState(const RoomSnapshot& room) override;
    void OnRoomJoinPending(
        const SignalingRoomJoinPending& pending) override;
    void OnRoomJoinRequested(const RoomJoinRequest& request) override;
    void OnRoomJoinResult(
        const SignalingRoomJoinResult& result) override;
    void OnRoomAvailabilityResult(
        const SignalingRoomAvailabilityResult& result) override;
    void OnRoomClosed(const SignalingRoomClosed& closed) override;
    void OnRoomPairReady(const SignalingRoomPairReady& ready) override;
    void OnRoomPairClosed(const SignalingRoomPairClosed& closed) override;
    void OnRoomScreenShareGranted(
        const SignalingRoomScreenShareGranted& granted) override;
    void OnRoomScreenShareSwitchPending(
        const SignalingRoomScreenShareSwitchPending& pending) override;
    void OnRoomScreenShareSwitchRequested(
        const RoomScreenShareSwitchRequest& request) override;
    void OnRoomScreenShareSwitchResult(
        const SignalingRoomScreenShareSwitchResult& result) override;
    void OnRoomControlRequested(
        const RoomControlRequest& request) override;
    void OnRoomControlResult(
        const SignalingRoomControlResult& result) override;
    void OnRoomControlGranted(
        const SignalingRoomControlGranted& granted) override;
    void OnRoomControlRevoked(
        const SignalingRoomControlRevoked& revoked) override;

    bool SendDescription(
        const SessionDescription& description) override;
    bool SendIceCandidate(const IceCandidate& candidate) override;
    bool RequestIceRestart(std::uint64_t observedGeneration,
                           std::uint64_t requestSequence) override;
    bool CancelIceRestart(std::uint64_t observedGeneration,
                          std::uint64_t requestSequence) override;
    void OnControllerSnapshot(
        const SessionControllerSnapshot& snapshot) override;
    void OnDataChannelStateChanged(
        const DataChannelInfo& channel) override;
    void OnDataMessage(const std::string& label,
                       std::span<const std::uint8_t> payload,
                       bool binary) override;
    void OnRemoteTrackAdded(const RemoteTrackInfo& track) override;

    SessionCommandResult RequireReady(const char* operation) const;
    bool SignalingIsOnline() const;
    void DisposeClosedSession();
    void ResetSessionStateLocked();
    void ResetRoomStateLocked();
    static bool ShouldBoostDesktopCaptureForInput(
        const RemoteInputEvent& event);
    void StopLocalDesktopCapture();
    void StopDirectDesktopCapture();
    void StartRemoteCursorPublishing(
        const DisplayDescriptor& display,
        std::uint64_t layoutVersion);
    void StopRemoteCursorPublishing();
    void OnLocalCursorObservation(WindowsCursorObservation observation);
    void RepublishRemoteCursor();
    bool DispatchRemoteCursorData(
        const std::string& pairId,
        const std::string& label,
        std::span<const std::uint8_t> payload);
    void BroadcastDirectSharedDisplayLayout();
    void RequestDirectSharedDisplayLayout();
    SessionCommandResult SendDirectFileMessage(
        const std::string& peerDeviceId,
        const FileTransferMessage& message);
    SessionCommandResult SendDirectClipboardMessage(
        const std::string& peerDeviceId,
        const std::string& clipboardSessionId,
        const ClipboardMessage& message);
    bool DispatchDirectAuxiliaryData(
        const std::string& label,
        std::span<const std::uint8_t> payload);
    bool DispatchDirectScreenData(
        const std::string& label,
        std::span<const std::uint8_t> payload);
    SessionCommandResult SwitchLocalDirectDisplay(
        const std::string& stableDisplayKey);
    void BroadcastDirectSharedDisplayCatalog();
    void BroadcastSharedDisplayLayout();
    bool SendRoomPairDescription(
        const std::string& pairId,
        const SessionDescription& description);
    bool SendRoomPairIceCandidate(
        const std::string& pairId,
        const IceCandidate& candidate);
    bool SendRoomPairIceRestartRequest(
        const std::string& pairId,
        std::uint64_t observedGeneration,
        std::uint64_t requestSequence);
    bool SendRoomPairIceRestartCancel(
        const std::string& pairId,
        std::uint64_t observedGeneration,
        std::uint64_t requestSequence);
    void OnRoomPairControllerSnapshot(
        const std::string& pairId,
        const SessionControllerSnapshot& snapshot);
    void OnRoomPairDataChannelStateChanged(
        const std::string& pairId,
        const DataChannelInfo& channel);
    void OnRoomPairDataMessage(
        const std::string& pairId,
        const std::string& label,
        std::span<const std::uint8_t> payload,
        bool binary);
    bool DispatchRoomPairTransferData(
        const std::string& pairId,
        const std::string& label,
        std::span<const std::uint8_t> payload);
    void DispatchRoomPairReliableData(
        const std::string& pairId,
        std::span<const std::uint8_t> payload);
    bool DispatchRoomPairScreenData(
        const std::string& pairId,
        std::span<const std::uint8_t> payload);
    void DispatchRoomPairInputData(
        const std::string& pairId,
        std::span<const std::uint8_t> payload,
        bool fastChannel);
    void OnRoomPairRemoteTrackAdded(
        const std::string& pairId,
        const RemoteTrackInfo& track);
    std::optional<OperationError> PrepareRoomPairAnswer(
        const std::string& pairId);
    std::optional<OperationError> PrepareRoomPairMedia(
        const std::string& pairId,
        bool bindNegotiatedSlots,
        bool preparationAlreadyClaimed = false);
    std::optional<OperationError> PrepareDirectMedia(
        bool bindNegotiatedSlots);
    void StopDirectMicrophoneCapture();
    void PublishSnapshot();
    void StartStatsPolling();
    void StopStatsPolling();
    void PollStatsOnce();
    bool DispatchScreenReceiverFeedback(const std::string& pairId, const std::string& label,
        std::span<const std::uint8_t> payload);

    class RoomPairBridge;
    struct RoomPairRuntime;
    void RetireRoomPair(std::shared_ptr<RoomPairRuntime> pair);
    SessionCommandResult ApplyLocalScreenFrameRate(
        std::uint32_t framesPerSecond);
    SessionCommandResult SwitchLocalSharedDisplay(
        const std::string& stableDisplayKey);
    void BroadcastSharedDisplayCatalog();
    void StartClipboardWarmup(
        const std::shared_ptr<RoomPairRuntime>& pair);
    SessionCommandResult SendRoomMemberAction(
        const std::string& peerDeviceId,
        RoomMemberAction action,
        std::uint64_t screenShareEpoch = 0);
    SendResult SendRoomMemberActionResponse(
        const std::shared_ptr<RoomPairRuntime>& pair,
        const RoomMemberActionEnvelope& request,
        bool accepted,
        const std::string& error);

    std::unique_ptr<WebRtcRuntime> runtime_;
    std::unique_ptr<ISignalingClient> signaling_;
    SignalingClientConfig signalingConfig_;
    InProcessSessionEngineOptions options_;
    std::optional<std::uint32_t> liveScreenQualityDeficitShare_;
    std::uint32_t ScreenQualityDeficitShareFromProvider() const;
    std::unique_ptr<LibWebRtcSession> webRtcSession_;
    std::unique_ptr<SessionControllerBase> sessionController_;
    std::uint64_t directSessionGeneration_ = 0;
    std::unordered_map<std::string, std::shared_ptr<RoomPairRuntime>>
        roomPairs_;
    std::vector<std::jthread> retiredRoomPairThreads_;
    // Device drivers may block while stopping a capture module. Keep those
    // joins outside the Qt UI path, but retain ownership until engine stop.
    std::vector<std::jthread> retiredDesktopStopThreads_;
    std::vector<std::jthread> retiredCameraStopThreads_;
    std::vector<std::jthread> mediaDeviceOperationThreads_;
    std::vector<std::jthread> clipboardWarmupThreads_;
    std::mutex startupThreadMutex_;
    std::jthread startupThread_;
    std::shared_ptr<std::atomic_uint64_t> startupDispatchGeneration_ =
        std::make_shared<std::atomic_uint64_t>(0);
    std::unique_ptr<SessionStatsPoller> statsPoller_;
    std::unique_ptr<InProcessSessionMediaState> mediaState_;
    std::unique_ptr<InProcessSessionMediaAdapter> mediaAccess_;
    std::unique_ptr<WindowsCursorMonitor> cursorMonitor_;
    mutable std::mutex mutex_;
    ISessionEngineObserver* observer_ = nullptr;
    std::function<void(const SignalingAccountDeletionResult&)>
        accountDeletionResultCallback_;
    SessionEngineSnapshot snapshot_;
    SessionEngineCapabilities capabilities_;
    DirectSessionCoordinator directSession_;
    RoomSessionCoordinator roomSession_;
    LocalMediaCoordinator localMedia_;
    ScreenShareCoordinator screenShare_;
    IRemoteInputSink* remoteInputSink_ = nullptr;
    IFileTransferSink* remoteFileTransferSink_ = nullptr;
    IClipboardSink* remoteClipboardSink_ = nullptr;
    RemoteCursorCallback remoteCursorCallback_;
    std::optional<RemoteCursorPosition> latestLocalCursorPosition_;
    std::optional<RemoteCursorShape> latestLocalCursorShape_;
    std::uint64_t nextCursorSequence_ = 0;
    std::uint64_t cursorPositionsPublished_ = 0;
    std::uint64_t cursorShapesPublished_ = 0;
    std::uint64_t cursorPositionsReceived_ = 0;
    std::uint64_t cursorShapesReceived_ = 0;
    // One sequence spans both input DataChannels so late reliable button
    // transitions cannot overwrite newer fast pointer state.
};

}  // namespace app
}  // namespace remote
