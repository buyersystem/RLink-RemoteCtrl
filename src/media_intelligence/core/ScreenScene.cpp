// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ScreenScene.h"

#include <array>
#include <utility>

#include "ContentState.h"

namespace remote::media_intelligence {
namespace {

constexpr std::array<std::pair<ScreenScene, std::string_view>, 9> kSceneNames{{
    {ScreenScene::kCodeTerminal, "code_terminal"},
    {ScreenScene::kDocument, "document"},
    {ScreenScene::kSpreadsheet, "spreadsheet"},
    {ScreenScene::kWebApp, "web_app"},
    {ScreenScene::kPhotoGraphics, "photo_graphics"},
    {ScreenScene::kCadDiagram, "cad_diagram"},
    {ScreenScene::kVideo, "video"},
    {ScreenScene::kGame3d, "game_3d"},
    {ScreenScene::kMixed, "mixed"},
}};

}  // namespace

const char* ScreenSceneName(ScreenScene scene) noexcept
{
    for (const auto& [value, name] : kSceneNames) {
        if (scene == value) {
            return name.data();
        }
    }
    return "unknown";
}

ScreenScene ParseScreenScene(std::string_view name) noexcept
{
    for (const auto& [value, wireName] : kSceneNames) {
        if (name == wireName) {
            return value;
        }
    }
    return ScreenScene::kUnknown;
}

ScreenSemanticType SceneToSemanticType(ScreenScene scene) noexcept
{
    switch (scene) {
    case ScreenScene::kCodeTerminal:
    case ScreenScene::kDocument:
    case ScreenScene::kSpreadsheet:
    case ScreenScene::kCadDiagram:
        return ScreenSemanticType::kTextUi;
    case ScreenScene::kWebApp:
    case ScreenScene::kPhotoGraphics:
    case ScreenScene::kMixed:
        return ScreenSemanticType::kMixedUi;
    case ScreenScene::kVideo:
    case ScreenScene::kGame3d:
        return ScreenSemanticType::kVideo;
    case ScreenScene::kUnknown:
    default:
        return ScreenSemanticType::kUnknown;
    }
}

}  // namespace remote::media_intelligence
