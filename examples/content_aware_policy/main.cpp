// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include <iostream>
#include "media_intelligence/core/ContentAwareStreamPolicy.h"

int main()
{
    using namespace remote::media_intelligence;
    ContentAwareStreamInput input;
    input.sourceWidth = input.currentWidth = 1920;
    input.sourceHeight = input.currentHeight = 1080;
    input.maximumFrameRate = input.currentFrameRate = 120;
    input.currentDesiredVideoBitrateBps = 35'000'000;
    input.currentSenderMaxBitrateBps = 35'000'000;
    input.scene = ScreenScene::kGame3d;
    input.activityAvailable = input.active = true;
    input.networkBudgetAvailable = true;
    input.safeVideoBudgetBps = 35'000'000;
    input.nowMs = input.networkTimestampMs = 10'000;
    input.generation = input.networkGeneration = 1;

    const auto decision = RecommendContentAwareStream(input);
    std::cout << decision.width << 'x' << decision.height << " / "
        << decision.senderMaxFps << " FPS\nrequired_bps="
        << decision.requiredVideoBitrateBps << "\ndesired_bps="
        << decision.desiredVideoBitrateBps << "\nsender_max_bps="
        << decision.senderMaxBitrateBps << "\nreason="
        << ContentAwareStreamReasonName(decision.reason)
        << "\nautomatic_control_eligible=" << decision.automaticControlEligible
        << '\n';
    // This sample intentionally uses the uncalibrated seed. It must never
    // authorize automatic execution or pretend to have measured image quality.
    return decision.hasRecommendation && !decision.automaticControlEligible ? 0 : 1;
}
