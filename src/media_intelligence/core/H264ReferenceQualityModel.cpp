// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "media_intelligence/core/H264ReferenceQualityModel.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace remote::media_intelligence {
namespace {

bool ValidText(std::string_view value) noexcept
{
    return !value.empty() &&
        value.size() <= H264ReferenceQualityContext::kMaximumProfileText &&
        value.find('\0') == std::string_view::npos;
}

bool KnownH264Codec(std::string_view codec) noexcept
{
    return codec == "video/H264" || codec == "video/h264" ||
        codec == "H264" || codec == "h264";
}

bool NamedBackend(std::string_view encoder, std::string_view name) noexcept
{
    if (encoder == name) {
        return true;
    }
    // Actual factory names append a quality option, for example NVENC (p4).
    // A boundary is required: NVENC2 or a similar unknown backend is rejected.
    return encoder.starts_with(name) && encoder.size() > name.size() + 3 &&
        encoder.substr(name.size()).starts_with(" (") && encoder.back() == ')';
}

bool KnownH264Backend(std::string_view encoder) noexcept
{
    return NamedBackend(encoder, "FFmpeg/NVENC") ||
        NamedBackend(encoder, "FFmpeg/QSV") ||
        NamedBackend(encoder, "FFmpeg/AMF") ||
        NamedBackend(encoder, "FFmpeg/libx264") ||
        NamedBackend(encoder, "OpenH264") ||
        NamedBackend(encoder, "Builtin/OpenH264");
}

bool ValidConfig(const H264ReferenceQualityConfig& config) noexcept
{
    if (!std::isfinite(config.bitrateSafetyMargin) ||
        config.bitrateSafetyMargin < 1.0 || config.bitrateSafetyMargin > 4.0 ||
        !std::isfinite(config.maximumAverageQp) ||
        config.maximumAverageQp < 0.0 || config.maximumAverageQp > 51.0) {
        return false;
    }
    for (std::size_t index = 1; index < config.sceneBitsPerPixelPerFrame.size(); ++index) {
        const double seed = config.sceneBitsPerPixelPerFrame[index];
        const double qp = config.sceneMaximumAverageQp[index];
        if (!std::isfinite(seed) || seed <= 0.0 || seed > 100.0 ||
            !std::isfinite(qp) || qp < 0.0 || qp > 51.0) {
            return false;
        }
    }
    return true;
}

}  // namespace

H264ReferenceQualityConfig::H264ReferenceQualityConfig() noexcept
{
    const ContentAwareStreamConfig seed;
    for (std::size_t index = 0; index < sceneBitsPerPixelPerFrame.size(); ++index) {
        sceneBitsPerPixelPerFrame[index] = seed.profiles[index].seedBitsPerPixelPerFrame;
        sceneMaximumAverageQp[index] = seed.profiles[index].maximumReferenceAverageQp;
    }
}

H264ReferenceQualityContext::H264ReferenceQualityContext(
    StreamEncoderProfile activeProfile, const H264ReferenceQualityConfig& config) noexcept
    : config_(config)
{
    if (!ValidText(activeProfile.codec) || !ValidText(activeProfile.encoder) ||
        !ValidText(activeProfile.profile) || !KnownH264Codec(activeProfile.codec) ||
        !KnownH264Backend(activeProfile.encoder) || !ValidConfig(config)) {
        return;
    }
    std::copy(activeProfile.codec.begin(), activeProfile.codec.end(), codec_.begin());
    std::copy(activeProfile.encoder.begin(), activeProfile.encoder.end(), encoder_.begin());
    std::copy(activeProfile.profile.begin(), activeProfile.profile.end(), profile_.begin());
    valid_ = true;
}

StreamEncoderProfile H264ReferenceQualityContext::Profile() const noexcept
{
    return {codec_.data(), encoder_.data(), profile_.data()};
}

bool H264ReferenceQualityContext::MatchesProfile(StreamEncoderProfile activeProfile) const noexcept
{
    const auto stored = Profile();
    return valid_ && stored.codec == activeProfile.codec &&
        stored.encoder == activeProfile.encoder && stored.profile == activeProfile.profile;
}

bool H264ReferenceQualityContext::ValidRequest(const StreamQualityRequest& request) const noexcept
{
    const auto scene = static_cast<std::size_t>(request.scene);
    return valid_ && scene > 0 && scene < config_.sceneBitsPerPixelPerFrame.size() &&
        request.width >= 2 && request.height >= 2 &&
        request.width <= kMaximumDimension && request.height <= kMaximumDimension &&
        request.width % 2 == 0 && request.height % 2 == 0 &&
        request.frameRate >= 1 && request.frameRate <= 120;
}

StreamQualityEstimate H264ReferenceQualityContext::Estimate(
    const StreamQualityRequest& request, const void* context) noexcept
{
    StreamQualityEstimate result;
    if (!context) {
        return result;
    }
    const auto& model = *static_cast<const H264ReferenceQualityContext*>(context);
    if (!model.ValidRequest(request)) {
        return result;
    }
    // Widen before multiplication. Do not clamp requirements to a network or
    // media limit: that could make an infeasible candidate appear feasible.
    const auto pixels = static_cast<std::uint64_t>(request.width) * request.height;
    const double required = static_cast<double>(pixels) * request.frameRate *
        model.config_.sceneBitsPerPixelPerFrame[static_cast<std::size_t>(request.scene)] *
        model.config_.bitrateSafetyMargin;
    if (!std::isfinite(required) || required <= 0.0 ||
        required >= static_cast<double>(std::numeric_limits<std::uint64_t>::max())) {
        return result;
    }
    result.requiredVideoBitrateBps = static_cast<std::uint64_t>(std::ceil(required));
    result.available = true;
    result.calibrated = false;
    result.reference = true;
    return result;
}

StreamCurrentQualityVerification H264ReferenceQualityContext::VerifyCurrentQuality(
    const StreamQualityRequest& request, double windowAverageQp) const noexcept
{
    if (!ValidRequest(request) || !std::isfinite(windowAverageQp) ||
        windowAverageQp < 0.0 || windowAverageQp > 51.0) {
        return {};
    }
    const auto maximumQp = std::min(config_.maximumAverageQp,
        config_.sceneMaximumAverageQp[static_cast<std::size_t>(request.scene)]);
    return {true, windowAverageQp <= maximumQp};
}

}  // namespace remote::media_intelligence
