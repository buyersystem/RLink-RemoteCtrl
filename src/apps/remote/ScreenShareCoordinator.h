// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <optional>
#include <memory>
#include <string>

#include "api/scoped_refptr.h"
#include "src/core/DesktopCaptureTypes.h"
#include "src/core/DisplayTopology.h"
#include "src/core/ScreenStreamPolicy.h"
#include "src/protocol/ScreenShareControlProtocol.h"

namespace remote {

class WindowsDesktopCaptureSource;

namespace app {

// Owns the policy and monotonically increasing identity of a local screen
// share. The engine still owns the capture transport; this coordinator keeps
// direct and room sessions from implementing subtly different selection and
// generation rules.
class ScreenShareCoordinator final {
public:
    ScreenShareCoordinator();
    ~ScreenShareCoordinator();

    ScreenShareCoordinator(const ScreenShareCoordinator&) = delete;
    ScreenShareCoordinator& operator=(const ScreenShareCoordinator&) = delete;

    [[nodiscard]] std::uint64_t BeginShare();
    [[nodiscard]] std::uint64_t NextGeneration() const;
    void CommitGeneration(std::uint64_t generation);
    [[nodiscard]] bool IsCurrentGeneration(
        std::uint64_t generation) const;

    [[nodiscard]] bool HasCaptureSource() const;
    [[nodiscard]] bool CaptureSourceIs(
        const WindowsDesktopCaptureSource* source) const;
    [[nodiscard]] webrtc::scoped_refptr<WindowsDesktopCaptureSource>
        CaptureSource() const;
    void SetCaptureSource(
        webrtc::scoped_refptr<WindowsDesktopCaptureSource> source);
    [[nodiscard]] webrtc::scoped_refptr<WindowsDesktopCaptureSource>
        TakeCaptureSource();

    [[nodiscard]] static std::optional<DisplayDescriptor> SelectDisplay(
        const DisplayTopologySnapshot& topology,
        const std::string& preferredStableDisplayKey);

    [[nodiscard]] static ScreenStreamPolicyResult ResolvePolicy(
        std::uint32_t sourceWidth,
        std::uint32_t sourceHeight,
        const ScreenStreamPreferenceRequest& request);

    [[nodiscard]] static std::uint32_t MaximumCaptureFrameRate(
        DesktopCaptureImplementation implementation,
        const WindowsDesktopCaptureSource* source = nullptr);

private:
    struct State;
    std::unique_ptr<State> state_;
};

}  // namespace app
}  // namespace remote
