// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "PeerNegotiator.h"

#include <memory>
#include <utility>

#include "api/jsep.h"
#include "api/make_ref_counted.h"

namespace remote {
namespace {

class CreateDescriptionCallback
    : public webrtc::CreateSessionDescriptionObserver {
public:
    using SuccessCallback = std::function<void(
        std::unique_ptr<webrtc::SessionDescriptionInterface>)>;
    using FailureCallback = std::function<void(webrtc::RTCError)>;

    CreateDescriptionCallback(SuccessCallback success,
                              FailureCallback failure)
        : success_(std::move(success)), failure_(std::move(failure))
    {}

    void OnSuccess(webrtc::SessionDescriptionInterface* description) override
    {
        success_(
            std::unique_ptr<webrtc::SessionDescriptionInterface>(description));
    }

    void OnFailure(webrtc::RTCError error) override
    {
        failure_(std::move(error));
    }

private:
    SuccessCallback success_;
    FailureCallback failure_;
};

class SetDescriptionCallback
    : public webrtc::SetSessionDescriptionObserver {
public:
    using SuccessCallback = std::function<void()>;
    using FailureCallback = std::function<void(webrtc::RTCError)>;

    SetDescriptionCallback(SuccessCallback success, FailureCallback failure)
        : success_(std::move(success)), failure_(std::move(failure))
    {}

    void OnSuccess() override { success_(); }

    void OnFailure(webrtc::RTCError error) override
    {
        failure_(std::move(error));
    }

private:
    SuccessCallback success_;
    FailureCallback failure_;
};

webrtc::SdpType ToNativeSdpType(SessionDescriptionType type)
{
    return type == SessionDescriptionType::kOffer ? webrtc::SdpType::kOffer
                                                  : webrtc::SdpType::kAnswer;
}

void SetLocalDescription(
    const PeerNegotiatorCallbacks& callbacks,
    OperationId operationId,
    SessionDescription description,
    std::unique_ptr<webrtc::SessionDescriptionInterface> nativeDescription)
{
    auto peer = callbacks.peerConnection();
    if (!peer) {
        callbacks.failOperation(
            operationId, "session_closed",
            "The session closed before local SDP was applied.");
        return;
    }

    auto callback = webrtc::make_ref_counted<SetDescriptionCallback>(
        [callbacks, operationId,
         description = std::move(description)] {
            callbacks.localDescriptionCompleted(operationId, description);
        },
        [callbacks, operationId](webrtc::RTCError error) {
            callbacks.failOperation(operationId,
                                    "set_local_sdp_failed",
                                    error.message());
        });
    peer->SetLocalDescription(callback.get(), nativeDescription.release());
}

}  // namespace

PeerNegotiator::PeerNegotiator(PeerNegotiatorCallbacks callbacks)
    : callbacks_(std::move(callbacks))
{}

void PeerNegotiator::CreateLocalDescription(
    webrtc::scoped_refptr<webrtc::PeerConnectionInterface> peer,
    OperationId operationId,
    SessionDescriptionType type,
    bool iceRestart) const
{
    if (!peer) {
        callbacks_.failOperation(
            operationId, "session_not_started",
            "Cannot create SDP before the session is started.");
        return;
    }
    callbacks_.changeState(WebRtcSessionState::kConnecting);

    const auto callbacks = callbacks_;
    auto callback = webrtc::make_ref_counted<CreateDescriptionCallback>(
        [callbacks, operationId, type](
            std::unique_ptr<webrtc::SessionDescriptionInterface>
                nativeDescription) mutable {
            std::string sdp;
            if (!nativeDescription || !nativeDescription->ToString(&sdp)) {
                callbacks.failOperation(
                    operationId, "local_sdp_serialize_failed",
                    "Failed to serialize the local SDP.");
                return;
            }
            SetLocalDescription(callbacks, operationId,
                                {type, std::move(sdp)},
                                std::move(nativeDescription));
        },
        [callbacks, operationId](webrtc::RTCError error) {
            callbacks.failOperation(operationId,
                                    "create_local_sdp_failed",
                                    error.message());
        });
    if (type == SessionDescriptionType::kOffer) {
        webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
        options.ice_restart = iceRestart;
        peer->CreateOffer(callback.get(), options);
    } else {
        peer->CreateAnswer(callback.get(), {});
    }
}

void PeerNegotiator::ApplyRemoteDescription(
    webrtc::scoped_refptr<webrtc::PeerConnectionInterface> peer,
    OperationId operationId,
    const SessionDescription& description) const
{
    if (!peer) {
        callbacks_.failOperation(
            operationId, "session_not_started",
            "Cannot apply SDP before the session is started.");
        return;
    }

    webrtc::SdpParseError parseError;
    auto nativeDescription = webrtc::CreateSessionDescription(
        ToNativeSdpType(description.type), description.sdp, &parseError);
    if (!nativeDescription) {
        callbacks_.failOperation(operationId, "remote_sdp_parse_failed",
                                 parseError.description);
        return;
    }

    const auto callbacks = callbacks_;
    auto callback = webrtc::make_ref_counted<SetDescriptionCallback>(
        [callbacks, operationId] {
            callbacks.completeOperation(operationId);
        },
        [callbacks, operationId](webrtc::RTCError error) {
            callbacks.failOperation(operationId,
                                    "set_remote_sdp_failed",
                                    error.message());
        });
    peer->SetRemoteDescription(callback.get(), nativeDescription.release());
}

void PeerNegotiator::AddRemoteIceCandidate(
    webrtc::scoped_refptr<webrtc::PeerConnectionInterface> peer,
    OperationId operationId,
    const IceCandidate& candidate) const
{
    if (!peer) {
        callbacks_.failOperation(
            operationId, "session_not_started",
            "Cannot add ICE before the session is started.");
        return;
    }

    webrtc::SdpParseError parseError;
    std::unique_ptr<webrtc::IceCandidate> nativeCandidate(
        webrtc::CreateIceCandidate(candidate.sdpMid,
                                   candidate.sdpMLineIndex,
                                   candidate.candidate, &parseError));
    if (!nativeCandidate) {
        callbacks_.failOperation(operationId,
                                 "ice_candidate_parse_failed",
                                 parseError.description);
        return;
    }
    if (!peer->AddIceCandidate(nativeCandidate.get())) {
        callbacks_.failOperation(
            operationId, "add_ice_candidate_failed",
            "PeerConnection rejected the ICE candidate.");
        return;
    }
    callbacks_.completeOperation(operationId);
}

}  // namespace remote
