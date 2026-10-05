// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "DisplayTopology.h"
#include "DirectSessionRequest.h"
#include "MediaDevice.h"
#include "RoomState.h"
#include "SessionDiagnostics.h"
#include "src/protocol/ScreenShareControlProtocol.h"

namespace remote {

enum class RemoteControlRole {
    kNone,
    kController,
    kControlled,
};

enum class SessionOrigin {
    kNone,
    kManualDeviceId,
    kRemoteAssistance,
    kOwnedDevice,
    kCollaborationRoom,
};

struct OwnedDeviceSnapshot {
    std::string deviceId;
    std::string deviceName;
    bool online = false;
    bool current = false;
    std::int64_t createdAt = 0;
    std::int64_t lastSeenAt = 0;
};

struct OwnedDevicesSnapshot {
    std::uint64_t revision = 0;
    bool loaded = false;
    std::vector<OwnedDeviceSnapshot> devices;
};

struct RoomActivitySnapshot {
    std::vector<RoomJoinRequest> incomingJoinRequests;
    std::vector<RoomScreenShareSwitchRequest>
        incomingScreenShareSwitchRequests;
    std::vector<RoomControlRequest> incomingControlRequests;
    std::vector<RoomScreenShareViewRequest> incomingScreenShareViewRequests;
    std::vector<RoomMemberActionResult> memberActionResults;
    std::string outgoingScreenShareSwitchRequestId;
    std::vector<RoomPeerConnectionSnapshot> peerConnections;
    std::vector<RoomAvailabilitySnapshot> availabilities;
};

enum class SessionEngineState {
    kStopped,
    kStarting,
    kReady,
    kConnecting,
    kAwaitingLocalApproval,
    kActive,
    kStopping,
    kFailed,
};

enum class SessionConnectivityState {
    kNotConfigured,
    kConnecting,
    kOnline,
    kOffline,
    kFailed,
};

enum class LocalCameraState {
    kOff,
    kStarting,
    kPublishing,
    kStopping,
    kFailed,
};

enum class LocalMicrophoneState {
    kOff,
    kStarting,
    kPublishing,
    kStopping,
    kFailed,
};

struct SessionEngineCapabilities {
    bool webRtcReady = false;
    bool hasH264Encoder = false;
    bool hasH264Decoder = false;
    bool h264HardwareEncoderAvailable = false;
    bool h264HardwareEncoderCpuNv12InputSupported = false;
    bool h264HardwareEncoderD3D11InputCandidate = false;
    uint32_t h264HardwareEncoderCount = 0;
    bool h264HardwareEncoderWired = false;
    bool h264SoftwareEncoderWired = false;
    bool ffmpegX264EncoderWired = false;
    std::string ffmpegX264EncoderError;
    bool ffmpegHardwareEncoderAvailable = false;
    bool ffmpegHardwareEncoderWired = false;
    std::string ffmpegHardwareEncoderError;
    std::vector<std::string> ffmpegHardwareEncoderDescriptions;
    bool h264SoftwareEncoderFallback = false;
    std::string desktopCapturePreference;
    std::string videoEncoderPreference;
    std::string videoDecoderPreference;
    std::vector<std::string> videoEncoderRuntimeDetails;
    std::string videoEncoderLastFallbackReason;
    std::vector<std::string> h264HardwareEncoderDescriptions;
    std::vector<std::string> h264HardwareEncoderWarnings;
    bool mfD3D11DecoderConfigured = false;
    bool mfD3D11DecoderHardware = false;
    bool mfD3D11DecoderSoftware = false;
    bool d3d11NativeDecoderOutput = false;
    bool mfD3D11DecoderAsynchronous = false;
    bool ffmpegSoftwareH264Decoder = false;
    std::string mfD3D11DecoderName;
    std::string mfD3D11DecoderError;
    std::string hardwareFingerprint;
    std::string operatingSystemDescription;
    std::string nativeArchitecture;
    bool remoteSession = false;
    std::vector<std::string> graphicsAdapterDescriptions;
    std::string graphicsEnumerationError;
    bool h264HardwareEncoderProbeSucceeded = false;
    bool h264HardwareEncoderProbeFromCache = false;
    bool audioDeviceModuleCreated = false;
    std::string audioDeviceError;
    std::string error;
};

struct DirectSessionSnapshot {
    // True after signaling has accepted direct-session authorization and
    // issued session-ready data. WebRTC may still be negotiating.
    bool signalingSessionReady = false;
    bool mediaSlotsPrepared = false;
    bool controlReliableChannelOpen = false;
    bool inputFastChannelOpen = false;
    bool fileTransferChannelOpen = false;
    bool clipboardReliableChannelOpen = false;
    bool clipboardTransferChannelOpen = false;
    // Remains true across temporary signaling/P2P interruptions after this
    // direct session has reached Connected at least once.
    bool sessionEverActive = false;
    // Initial negotiation also uses kConnecting. Consumers combine this
    // attempt count with their own "was active" latch before showing recovery.
    std::uint32_t iceRestartAttempt = 0;
    DisplayDescriptor remoteDisplay;
    std::uint64_t remoteDisplayLayoutVersion = 0;
    std::uint64_t remoteScreenShareGeneration = 0;
    bool screenPreferencePending = false;
    std::uint64_t screenPreferenceSequence = 0;
    // Last successfully applied request; a failed or pending request must
    // never replace the receiver-feedback contract still used by the sender.
    std::uint64_t screenPreferenceAcceptedSequence = 0;
    std::uint32_t screenWidth = 0;
    std::uint32_t screenHeight = 0;
    std::uint32_t screenFramesPerSecond = kDefaultScreenFrameRate;
    std::uint32_t screenMaximumFrameRate = kMaximumScreenFrameRate;
    std::uint32_t screenMaxBitrateBps = 0;
    bool remoteDisplayCatalogReported = false;
    std::uint64_t remoteDisplayCatalogLayoutVersion = 0;
    std::vector<DisplayDescriptor> remoteDisplays;
    bool remoteDisplaySwitchPending = false;
    std::uint64_t remoteDisplaySwitchSequence = 0;
    std::string remoteDisplaySwitchError;
};

struct LocalScreenShareSnapshot {
    std::uint64_t generation = 0;
    DisplayTopologySnapshot topology;
    std::string selectedDisplayKey;
    DisplayDescriptor activeDisplay;
    std::uint64_t activeDisplayLayoutVersion = 0;
};

struct SessionMediaSnapshot {
    LocalCameraState localCamera = LocalCameraState::kOff;
    LocalMicrophoneState localMicrophone = LocalMicrophoneState::kOff;
    bool roomAudioPlaybackMuted = false;
    bool remoteCameraPublishing = false;
    MediaDeviceSnapshot localMediaDevices;
};

struct SessionErrorSnapshot {
    std::string code;
    std::string message;
};

struct SessionEngineSnapshot {
    SessionEngineState state = SessionEngineState::kStopped;
    SessionConnectivityState connectivity =
        SessionConnectivityState::kNotConfigured;
    SessionPurpose purpose = SessionPurpose::kNone;
    SessionOrigin origin = SessionOrigin::kNone;
    RemoteControlRole remoteControlRole = RemoteControlRole::kNone;
    // True only after the signaling server's control-grant event has supplied
    // the lease token used to authenticate input and clipboard packets.
    bool roomControlGrantActive = false;
    std::string localDeviceId;
    std::string localVerificationCode;
    std::string sessionId;
    std::string peerDeviceId;
    DirectSessionSnapshot direct;
    SessionErrorSnapshot error;
    LocalScreenShareSnapshot screenShare;
    SessionMediaSnapshot media;
    RoomSnapshot room;
    RoomActivitySnapshot roomActivity;
    OwnedDevicesSnapshot ownedDevices;
};

struct SessionCommandResult {
    bool accepted = false;
    std::string errorCode;
    std::string errorMessage;
};

}  // namespace remote
