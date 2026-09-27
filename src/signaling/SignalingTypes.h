// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "src/core/DirectSessionRequest.h"
#include "src/core/RoomState.h"

namespace remote {

enum class SignalingConnectionState {
    kDisconnected,
    kConnecting,
    kAuthenticating,
    kRegistering,
    kRegistered,
    kClosing,
    kFailed,
};

enum class SignalingAuthenticationMode {
    // Transitional HMAC test-token path. The bearer token is supplied in the
    // WebSocket upgrade request and verified before the socket is accepted.
    kLegacyUpgradeBearer,
    // Production Logto path. The WebSocket opens without an Authorization
    // header, then sends an auth envelope and waits for auth_success before
    // registering the device.
    kMessageAccessToken,
};

struct SignalingCapabilities {
    bool screenCapture = true;
    bool cameraCapture = true;
    bool inputControl = true;
    bool h264Encode = false;
    bool h264Decode = false;
    bool d3d11NativeDecode = false;
    std::uint32_t protocolVersion = 5;
};

struct SignalingClientConfig {
    std::string endpoint;
    std::string accessToken;
    SignalingAuthenticationMode authenticationMode =
        SignalingAuthenticationMode::kLegacyUpgradeBearer;
    std::string deviceId;
    std::string deviceName;
    // Six-digit code generated locally for this process. It is never stored
    // by the signaling server and is only relayed over authenticated WSS.
    std::string deviceVerificationCode;
    std::string appVersion;
    // Optional PEM CA bundle for local/private deployments. Supplying it adds
    // a trust anchor; certificate and hostname verification remain enabled.
    std::string trustedCaPem;
    std::uint32_t heartbeatIntervalMs = 15000;
    std::uint32_t heartbeatTimeoutMs = 10000;
    std::uint32_t authenticationTimeoutMs = 10000;
    std::uint32_t reconnectInitialDelayMs = 1000;
    std::uint32_t reconnectMaximumDelayMs = 10000;
    // Zero disables automatic reconnect.
    std::uint32_t reconnectAttemptLimit = 8;
    SignalingCapabilities capabilities;
};

struct SignalingOperationResult {
    bool accepted = false;
    std::string errorCode;
    std::string errorMessage;
};

struct SignalingAccountDeletionResult {
    bool deleted = false;
    std::string errorCode;
    std::string errorMessage;
    bool retryable = false;
};

struct IncomingSessionRequest {
    std::string sessionId;
    std::string requesterDeviceId;
    SessionPurpose purpose = SessionPurpose::kNone;
    std::vector<std::string> requestedPermissions;
    bool sameAccount = false;
    bool autoAccept = false;
    DirectAuthorizationMethod authorization =
        DirectAuthorizationMethod::kManualApproval;
    std::string verificationCode;
};

struct SignalingOwnedDevice {
    std::string deviceId;
    std::string deviceName;
    bool online = false;
    bool current = false;
    std::int64_t createdAt = 0;
    std::int64_t lastSeenAt = 0;
};

struct SignalingOwnedDevicesSnapshot {
    std::uint64_t revision = 0;
    std::vector<SignalingOwnedDevice> devices;
};

struct SignalingSessionResponse {
    std::string sessionId;
    bool accepted = false;
    std::string reasonCode;
    std::string reasonMessage;
};

struct SignalingSessionPending {
    std::string sessionId;
    std::string peerDeviceId;
};

enum class SignalingSessionEndKind {
    kCancelled,
    kClosed,
};

struct SignalingSessionEnded {
    std::string sessionId;
    std::string initiatorDeviceId;
    std::string reasonCode;
    SignalingSessionEndKind kind = SignalingSessionEndKind::kClosed;
};

struct SignalingIceServer {
    std::vector<std::string> urls;
    std::string username;
    std::string credential;
};

struct SignalingSessionReady {
    std::string sessionId;
    std::string peerDeviceId;
    std::string recoveryToken;
    std::vector<SignalingIceServer> iceServers;
};

struct SignalingSessionSuspended {
    std::string sessionId;
    std::string peerDeviceId;
    std::uint32_t recoveryWindowMs = 0;
};

struct SignalingSessionResumed {
    std::string sessionId;
    std::string peerDeviceId;
    std::string resumedDeviceId;
};

struct SignalingSessionDescription {
    std::string sessionId;
    std::string type;
    std::string sdp;
    std::uint64_t negotiationGeneration = 1;
};

struct SignalingIceCandidate {
    std::string sessionId;
    std::string candidate;
    std::string sdpMid;
    std::int32_t sdpMLineIndex = -1;
    std::uint64_t negotiationGeneration = 1;
};

struct SignalingIceRestartRequest {
    std::string sessionId;
    std::uint64_t observedGeneration = 1;
    std::uint64_t requestSequence = 0;
};

struct SignalingIceRestartCancel {
    std::string sessionId;
    std::uint64_t observedGeneration = 1;
    std::uint64_t requestSequence = 0;
};

struct SignalingRoomReady {
    RoomSnapshot room;
    std::string recoveryToken;
};

struct SignalingRoomJoinPending {
    std::string roomId;
    std::string requestId;
};

struct SignalingRoomJoinResult {
    std::string roomId;
    std::string requestId;
    bool accepted = false;
    std::string reasonCode;
    std::string reasonMessage;
};

struct SignalingRoomAvailability {
    std::string roomId;
    bool exists = false;
    bool joinable = false;
};

struct SignalingRoomAvailabilityResult {
    std::vector<SignalingRoomAvailability> rooms;
};

struct SignalingRoomClosed {
    std::string roomId;
    std::string initiatorDeviceId;
    std::string reasonCode;
};

struct SignalingRoomPairReady {
    std::string pairId;
    std::string roomId;
    std::string peerDeviceId;
    bool localIsOfferer = false;
    std::vector<SignalingIceServer> iceServers;
};

struct SignalingRoomPairClosed {
    std::string pairId;
    std::string roomId;
    std::string peerDeviceId;
    std::string initiatorDeviceId;
    std::string reasonCode;
};

struct SignalingRoomScreenShareGranted {
    std::string roomId;
    std::string grantId;
    std::uint64_t epoch = 0;
};

struct SignalingRoomScreenShareSwitchPending {
    std::string roomId;
    std::string requestId;
    std::string screenSharerDeviceId;
};

struct SignalingRoomScreenShareSwitchResult {
    std::string roomId;
    std::string requestId;
    bool accepted = false;
    std::string reasonCode;
    std::string reasonMessage;
};

struct SignalingRoomControlResult {
    std::string roomId;
    std::string requestId;
    bool accepted = false;
    std::string reasonCode;
    std::string reasonMessage;
};

struct SignalingRoomControlGranted {
    std::string roomId;
    std::string grantId;
    std::string screenSharerDeviceId;
    std::string controllerDeviceId;
};

struct SignalingRoomControlRevoked {
    std::string roomId;
    std::string initiatorDeviceId;
    std::string reasonCode;
};

}  // namespace remote
