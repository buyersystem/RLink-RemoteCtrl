// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>

#include "api/video/video_frame_buffer.h"
#include "media_intelligence/core/SemanticClassification.h"

namespace remote {

struct RemoteVisionFrameAnalyzerSnapshot {
    bool running = false;
    std::uint64_t generation = 0;
    media_intelligence::SemanticClassification classification;
    media_intelligence::SemanticClassification latestReturnedClassification;
    std::uint64_t latestReturnedAtMs = 0;
    std::uint64_t sourceFrameId = 0;
    std::uint64_t completedAtMs = 0;
    std::uint32_t latestAnalysisTimeUs = 0;
    std::uint32_t latestScaleConvertTimeUs = 0;
    std::uint32_t latestJpegEncodeTimeUs = 0;
    std::uint64_t latestJpegBytes = 0;
    std::uint64_t submittedSamples = 0;
    std::uint64_t replacedSamples = 0;
    std::uint64_t processedSamples = 0;
    std::uint64_t rejectedSamples = 0;
    std::uint64_t discardedResults = 0;
};

// RLink-side boundary between desktop capture and a remote vision backend.
// SubmitFrame must remain non-blocking: the capture thread only transfers one
// reference to the latest immutable frame; scaling, JPEG encoding and network
// I/O are performed by the implementation away from the capture thread.
class IRemoteVisionFrameAnalyzer {
public:
    virtual ~IRemoteVisionFrameAnalyzer() = default;

    [[nodiscard]] virtual std::uint64_t BeginSession() noexcept = 0;
    virtual void EndSession(std::uint64_t sessionToken) noexcept = 0;
    virtual bool SubmitFrame(
        std::uint64_t sessionToken,
        webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame) noexcept = 0;
    virtual void InvalidateResult(
        std::uint64_t sessionToken) noexcept = 0;
    [[nodiscard]] virtual RemoteVisionFrameAnalyzerSnapshot Snapshot()
        const noexcept = 0;
    // A result may survive missed requests, but must eventually give way to
    // the host's manual coefficient. Backends can account for their cadence.
    [[nodiscard]] virtual std::uint64_t MaximumSceneAgeMs() const noexcept
    {
        return 65000;
    }
};

}  // namespace remote
