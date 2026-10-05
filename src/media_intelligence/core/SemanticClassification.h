// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cmath>

#include "media_intelligence/core/ContentState.h"

namespace remote::media_intelligence {

struct SemanticClassification {
    ScreenScene scene = ScreenScene::kUnknown;
    float confidence = 0.0f;

    [[nodiscard]] ScreenSemanticType ResolvedSemantic() const noexcept
    {
        return SceneToSemanticType(scene);
    }

    [[nodiscard]] bool IsValid() const noexcept
    {
        if (!std::isfinite(confidence) || confidence < 0.0f ||
            confidence > 1.0f) {
            return false;
        }
        const auto resolved = ResolvedSemantic();
        if (resolved != ScreenSemanticType::kTextUi &&
            resolved != ScreenSemanticType::kMixedUi &&
            resolved != ScreenSemanticType::kVideo) {
            return false;
        }
        return true;
    }
};

}  // namespace remote::media_intelligence
