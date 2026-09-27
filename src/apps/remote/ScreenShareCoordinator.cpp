// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ScreenShareCoordinator.h"

#include <utility>

#include "src/platform/win/WindowsDesktopCaptureSource.h"

namespace remote::app {

struct ScreenShareCoordinator::State {
    webrtc::scoped_refptr<WindowsDesktopCaptureSource> captureSource;
    std::uint64_t generation = 0;
};

ScreenShareCoordinator::ScreenShareCoordinator()
    : state_(std::make_unique<State>())
{
}

ScreenShareCoordinator::~ScreenShareCoordinator() = default;

std::uint64_t ScreenShareCoordinator::BeginShare()
{
    return ++state_->generation;
}

std::uint64_t ScreenShareCoordinator::NextGeneration() const
{
    return state_->generation + 1;
}

void ScreenShareCoordinator::CommitGeneration(std::uint64_t generation)
{
    state_->generation = generation;
}

bool ScreenShareCoordinator::IsCurrentGeneration(
    std::uint64_t generation) const
{
    return state_->generation == generation;
}

bool ScreenShareCoordinator::HasCaptureSource() const
{
    return state_->captureSource != nullptr;
}

bool ScreenShareCoordinator::CaptureSourceIs(
    const WindowsDesktopCaptureSource* source) const
{
    return state_->captureSource.get() == source;
}

webrtc::scoped_refptr<WindowsDesktopCaptureSource>
ScreenShareCoordinator::CaptureSource() const
{
    return state_->captureSource;
}

void ScreenShareCoordinator::SetCaptureSource(
    webrtc::scoped_refptr<WindowsDesktopCaptureSource> source)
{
    state_->captureSource = std::move(source);
}

webrtc::scoped_refptr<WindowsDesktopCaptureSource>
ScreenShareCoordinator::TakeCaptureSource()
{
    return std::exchange(state_->captureSource, nullptr);
}

std::optional<DisplayDescriptor> ScreenShareCoordinator::SelectDisplay(
    const DisplayTopologySnapshot& topology,
    const std::string& preferredStableDisplayKey)
{
    const auto* selected = FindDisplayByStableKey(
        topology, preferredStableDisplayKey);
    if (!selected) {
        selected = FindPrimaryDisplay(topology);
    }
    return selected ? std::optional<DisplayDescriptor>(*selected)
                    : std::nullopt;
}

ScreenStreamPolicyResult ScreenShareCoordinator::ResolvePolicy(
    std::uint32_t sourceWidth,
    std::uint32_t sourceHeight,
    const ScreenStreamPreferenceRequest& request)
{
    return ResolveScreenStreamPolicy(
        sourceWidth,
        sourceHeight,
        {request.maxWidth, request.maxHeight, request.framesPerSecond});
}

std::uint32_t ScreenShareCoordinator::MaximumCaptureFrameRate(
    DesktopCaptureImplementation implementation,
    const WindowsDesktopCaptureSource* source)
{
    if (implementation == DesktopCaptureImplementation::kLibWebRtc) {
        return kMaximumScreenFrameRate;
    }
    if (source && source->Backend() !=
            WindowsDesktopCaptureSource::CaptureBackend::
                kDxgiNativeTexture) {
        return 60u;
    }
    return kMaximumScreenFrameRate;
}

}  // namespace remote::app
