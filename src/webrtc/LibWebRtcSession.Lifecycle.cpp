// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "LibWebRtcSession.Internal.h"
#include "DataChannelManager.h"
#include "MediaSlotManager.h"
#include "PeerNegotiator.h"

namespace remote {
using namespace webrtc_session_detail;

LibWebRtcSession::LibWebRtcSession(
    webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory)
    : callbackGate_(std::make_shared<CallbackGate>(this)),
      factory_(std::move(factory)),
      statsCollector_(
          std::make_unique<PeerConnectionStatsCollector>()),
      dataChannelManager_(std::make_unique<DataChannelManager>(
          [this](const DataChannelInfo& info) {
              if (auto* observer = Observer()) {
                  observer->OnDataChannelStateChanged(info);
              }
          },
          [this](const std::string& label,
                 std::span<const std::uint8_t> payload,
                 bool binary) {
              if (auto* observer = Observer()) {
                  observer->OnDataMessage(label, payload, binary);
              }
          })),
      peerNegotiator_(std::make_unique<PeerNegotiator>(
          PeerNegotiatorCallbacks{
              [callbackGate = callbackGate_]() {
                  auto lease = callbackGate->Enter();
                  if (auto* owner = lease.Owner()) {
                      return owner->PeerConnection();
                  }
                  return webrtc::scoped_refptr<
                      webrtc::PeerConnectionInterface>{};
              },
              [callbackGate = callbackGate_](WebRtcSessionState state) {
                  auto lease = callbackGate->Enter();
                  if (auto* owner = lease.Owner()) {
                      owner->ChangeState(state);
                  }
              },
              [callbackGate = callbackGate_](
                  OperationId operationId,
                  const SessionDescription& description) {
                  auto lease = callbackGate->Enter();
                  if (auto* owner = lease.Owner()) {
                      if (auto* observer = owner->Observer()) {
                          observer->OnLocalDescription(description);
                      }
                      owner->CompleteOperation(operationId);
                  }
              },
              [callbackGate = callbackGate_](OperationId operationId) {
                  auto lease = callbackGate->Enter();
                  if (auto* owner = lease.Owner()) {
                      owner->CompleteOperation(operationId);
                  }
              },
              [callbackGate = callbackGate_](
                  OperationId operationId, std::string code,
                  std::string message) {
                  auto lease = callbackGate->Enter();
                  if (auto* owner = lease.Owner()) {
                      owner->FailOperation(operationId, std::move(code),
                                           std::move(message));
                  }
              }})),
      mediaSlots_(std::make_unique<MediaSlotManager>())
{}

LibWebRtcSession::~LibWebRtcSession()
{
    callbackGate_->DetachAndWait();
    SetObserver(nullptr);
    Close();
}

void LibWebRtcSession::SetObserver(IWebRtcSessionObserver* observer)
{
    std::lock_guard lock(mutex_);
    observer_ = observer;
}

OperationId LibWebRtcSession::Start(const WebRtcSessionConfig& config)
{
    const OperationId operationId = NextOperationId();
    bool canStart = false;
    {
        std::lock_guard lock(mutex_);
        canStart = state_ == WebRtcSessionState::kNew && !peerConnection_;
        if (canStart) {
            fastDesktopBweStartup_ = config.fastDesktopBweStartup;
            adaptiveDesktopNetworkFrameRate_ =
                config.adaptiveDesktopNetworkFrameRate;
        }
    }
    if (!canStart) {
        FailOperation(operationId, "already_started",
                      "A WebRTC session can only be started once.");
        return operationId;
    }
    ChangeState(WebRtcSessionState::kStarting);
    if (!factory_) {
        ChangeState(WebRtcSessionState::kFailed);
        FailOperation(operationId, "factory_unavailable",
                      "PeerConnectionFactory is null.");
        return operationId;
    }

    if (config.includeLoopbackAdapter) {
        webrtc::PeerConnectionFactoryInterface::Options options;
        options.network_ignore_mask = 0;
        factory_->SetOptions(options);
    }

    webrtc::PeerConnectionInterface::RTCConfiguration rtcConfiguration;
    rtcConfiguration.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    const bool hasConfiguredPortRange =
        config.iceMinPort != 0 || config.iceMaxPort != 0;
    if (hasConfiguredPortRange &&
        (config.iceMinPort < 1 || config.iceMaxPort > 65535 ||
         config.iceMinPort > config.iceMaxPort)) {
        ChangeState(WebRtcSessionState::kFailed);
        FailOperation(operationId, "invalid_ice_port_range",
                      "The ICE UDP port range must be between 1 and 65535, "
                      "and the minimum must not exceed the maximum.");
        return operationId;
    }
    if (hasConfiguredPortRange) {
        rtcConfiguration.set_min_port(config.iceMinPort);
        rtcConfiguration.set_max_port(config.iceMaxPort);
    }
    for (const auto& configuredServer : config.iceServers) {
        webrtc::PeerConnectionInterface::IceServer server;
        server.urls = configuredServer.urls;
        server.username = configuredServer.username;
        server.password = configuredServer.password;
        rtcConfiguration.servers.push_back(std::move(server));
    }

    auto peerOrError = factory_->CreatePeerConnectionOrError(
        rtcConfiguration, webrtc::PeerConnectionDependencies(this));
    if (!peerOrError.ok()) {
        ChangeState(WebRtcSessionState::kFailed);
        FailOperation(operationId, "peer_connection_create_failed",
                      peerOrError.error().message());
        return operationId;
    }

    auto peer = peerOrError.MoveValue();
    bool installed = false;
    {
        std::lock_guard lock(mutex_);
        if (!peerConnection_) {
            peerConnection_ = std::move(peer);
            installed = true;
        }
    }
    if (!installed) {
        FailOperation(operationId, "concurrent_start",
                      "Another thread started the WebRTC session first.");
        return operationId;
    }
    ChangeState(WebRtcSessionState::kReady);
    CompleteOperation(operationId);
    return operationId;
}

}  // namespace remote
