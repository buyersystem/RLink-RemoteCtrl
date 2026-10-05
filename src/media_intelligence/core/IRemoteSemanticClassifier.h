// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <functional>
#include <string>

#include "media_intelligence/core/EncodedImageView.h"
#include "media_intelligence/core/SemanticClassification.h"

namespace remote::media_intelligence {

enum class RemoteClassificationStatus : std::uint8_t {
    kSuccess,
    kInvalidRequest,
    kInvalidConfiguration,
    kCredentialUnavailable,
    kBusy,
    kCircuitOpen,
    kCanceled,
    kTransportError,
    kTimeout,
    kHttpError,
    kResponseTooLarge,
    kInvalidResponse,
};

struct RemoteSemanticRequest {
    std::uint64_t generation = 0;
    std::uint64_t sourceFrameId = 0;
    std::uint64_t timestampMs = 0;
    EncodedImage image;
};

struct RemoteSemanticResult {
    RemoteClassificationStatus status =
        RemoteClassificationStatus::kInvalidRequest;
    std::uint64_t generation = 0;
    std::uint64_t sourceFrameId = 0;
    SemanticClassification classification;
    std::uint32_t httpStatus = 0;
    std::uint32_t retryAfterMs = 0;
    std::string error;
};

enum class RemoteSubmitStatus : std::uint8_t {
    kStarted,
    kQueued,
    kReplacedPending,
    kRejected,
};

class IRemoteSemanticClassifier {
public:
    using Completion = std::function<void(RemoteSemanticResult)>;

    virtual ~IRemoteSemanticClassifier() = default;

    virtual RemoteSubmitStatus Submit(
        RemoteSemanticRequest request,
        Completion completion) = 0;
    virtual void Reset(std::uint64_t generation) = 0;
    virtual void Stop() = 0;
};

}  // namespace remote::media_intelligence
