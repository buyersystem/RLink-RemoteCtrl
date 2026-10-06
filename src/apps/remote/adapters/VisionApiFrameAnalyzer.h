// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QString>

#include <functional>
#include <cstdint>
#include <memory>
#include <string>

#include "media_intelligence/remote/VisionApiTypes.h"
#include "src/platform/win/IRemoteVisionFrameAnalyzer.h"

namespace remote::app {

class VisionApiFrameAnalyzer final
    : public IRemoteVisionFrameAnalyzer {
public:
    using ActivationCheck = std::function<bool()>;

    struct FrameEncodingOptions {
        std::uint32_t maximumImageDimension =
            media_intelligence::kDefaultVisionApiMaximumImageDimension;
        int jpegQuality = media_intelligence::kDefaultVisionApiJpegQuality;
    };
    using EncodingOptionsProvider =
        std::function<FrameEncodingOptions()>;

    struct HostOptions {
        ActivationCheck activationCheck;
        EncodingOptionsProvider encodingOptionsProvider;
        QString credentialDirectory;
        std::uint32_t maximumImageDimension =
            media_intelligence::kDefaultVisionApiMaximumImageDimension;
        int jpegQuality = media_intelligence::kDefaultVisionApiJpegQuality;
    };

    static std::shared_ptr<VisionApiFrameAnalyzer> Create(
        media_intelligence::VisionApiRuntimeConfig config,
        HostOptions hostOptions = {},
        std::string* error = nullptr);

    ~VisionApiFrameAnalyzer() override;

    VisionApiFrameAnalyzer(const VisionApiFrameAnalyzer&) = delete;
    VisionApiFrameAnalyzer& operator=(const VisionApiFrameAnalyzer&) = delete;

    [[nodiscard]] std::uint64_t BeginSession() noexcept override;
    void EndSession(std::uint64_t sessionToken) noexcept override;
    bool SubmitFrame(
        std::uint64_t sessionToken,
        webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame)
        noexcept override;
    void InvalidateResult(
        std::uint64_t sessionToken) noexcept override;
    [[nodiscard]] RemoteVisionFrameAnalyzerSnapshot Snapshot()
        const noexcept override;
    [[nodiscard]] std::uint64_t MaximumSceneAgeMs() const noexcept override;

private:
    struct Impl;
    explicit VisionApiFrameAnalyzer(std::unique_ptr<Impl> impl);

    std::unique_ptr<Impl> impl_;
};

}  // namespace remote::app
