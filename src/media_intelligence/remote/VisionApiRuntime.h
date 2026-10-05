// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <memory>

#include "media_intelligence/core/IRemoteSemanticClassifier.h"
#include "media_intelligence/remote/IClock.h"
#include "media_intelligence/remote/IExecutor.h"
#include "media_intelligence/remote/IHttpTransport.h"
#include "media_intelligence/remote/ILogger.h"
#include "media_intelligence/remote/ISecretProvider.h"
#include "media_intelligence/remote/IVisionApiProtocol.h"
#include "media_intelligence/remote/VisionApiTypes.h"

namespace remote::media_intelligence {

struct VisionApiRuntimeState;

struct VisionApiRuntimeSnapshot {
    bool running = false;
    bool requestInFlight = false;
    bool pendingSample = false;
    bool circuitOpen = false;
    std::uint64_t generation = 0;
    std::uint64_t submittedRequests = 0;
    std::uint64_t startedRequests = 0;
    std::uint64_t replacedSamples = 0;
    std::uint64_t completedRequests = 0;
    std::uint64_t successfulRequests = 0;
    std::uint64_t failedRequests = 0;
    std::uint32_t consecutiveFailures = 0;
    std::uint64_t nextEligibleAtMs = 0;
    std::uint64_t circuitOpenUntilMs = 0;
};

class VisionApiRuntime final : public IRemoteSemanticClassifier {
public:
    VisionApiRuntime(
        VisionApiRuntimeConfig config,
        std::shared_ptr<const IVisionApiProtocol> protocol,
        IHttpTransport& transport,
        ISecretProvider& secrets,
        IClock& clock,
        IExecutor& completionExecutor,
        ILogger* logger = nullptr);
    ~VisionApiRuntime() override;

    VisionApiRuntime(const VisionApiRuntime&) = delete;
    VisionApiRuntime& operator=(const VisionApiRuntime&) = delete;

    RemoteSubmitStatus Submit(
        RemoteSemanticRequest request,
        Completion completion) override;
    void Reset(std::uint64_t generation) override;
    void Stop() override;

    [[nodiscard]] VisionApiRuntimeSnapshot Snapshot() const;

private:
    std::shared_ptr<VisionApiRuntimeState> state_;
};

}  // namespace remote::media_intelligence
