// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <string_view>

namespace remote::media_intelligence {

enum class ScreenSemanticType : std::uint8_t;

// Scene describes content, independently from its current motion or the
// application's transport policy. Values are deliberately not FPS presets.
enum class ScreenScene : std::uint8_t {
    kUnknown,
    kCodeTerminal,
    kDocument,
    kSpreadsheet,
    kWebApp,
    kPhotoGraphics,
    kCadDiagram,
    kVideo,
    kGame3d,
    kMixed,
};

const char* ScreenSceneName(ScreenScene scene) noexcept;
// Exact wire names only. Unknown/unrecognized input never selects a policy.
ScreenScene ParseScreenScene(std::string_view name) noexcept;
ScreenSemanticType SceneToSemanticType(ScreenScene scene) noexcept;

}  // namespace remote::media_intelligence
