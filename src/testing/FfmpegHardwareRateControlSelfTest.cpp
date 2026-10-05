// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "src/platform/win/FfmpegHardwareRateControl.h"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>

namespace {

bool Check(bool value, const char* name)
{
    std::cout << name << '=' << (value ? "PASS" : "FAIL") << '\n';
    return value;
}

} // namespace

int main()
{
    using remote::FfmpegHardwareRateControl;
    using remote::FfmpegHardwareRateControlConfig;
    bool ok = true;
    constexpr auto limit = std::numeric_limits<std::uint32_t>::max();
    ok &= Check(FfmpegHardwareRateControl::ClampSubmittedBitrate(1'000'000, limit) == 1'000'000,
        "REQUESTED_BITRATE_IS_NOT_AMPLIFIED");
    ok &= Check(FfmpegHardwareRateControl::ClampSubmittedBitrate(limit, limit) == limit,
        "UINT32_MAX_BITRATE_HAS_NO_OVERFLOW");
    ok &= Check(FfmpegHardwareRateControl::ClampSubmittedBitrate(limit, 100'000'000) == 100'000'000,
        "SUBMITTED_BITRATE_LIMIT_IS_ENFORCED");
    ok &= Check(FfmpegHardwareRateControl::ClampSubmittedBitrate(0, limit) == 0 &&
        FfmpegHardwareRateControl::ClampSubmittedBitrate(1, 0) == 0,
        "ZERO_REQUEST_OR_ZERO_SUBMISSION_LIMIT_STAYS_ZERO");

    FfmpegHardwareRateControl control;
    control.Reset(60, 2'000'000, 0);
    control.Observe(2'000'000, 59.94, 100);
    auto decision = control.Recommend(10'000);
    ok &= Check(!decision.nominalFrameRateChangeNeeded && decision.nominalFrameRate == 60 &&
        decision.submittedBitrateBps == 2'000'000,
        "FRACTIONAL_FPS_JITTER_DOES_NOT_REQUEST_REOPEN");
    control.Observe(2'000'000, 59.0, 11'000);
    const bool lowerJitterIgnored = !control.Recommend(20'000).nominalFrameRateChangeNeeded;
    control.Observe(2'000'000, 61.0, 20'001);
    ok &= Check(lowerJitterIgnored && !control.Recommend(20'001).nominalFrameRateChangeNeeded,
        "59_61_FPS_JITTER_DOES_NOT_REQUEST_REOPEN");

    control.Observe(3'000'000, 120.0, 21'000);
    decision = control.Recommend(21'000);
    ok &= Check(decision.nominalFrameRateChangeNeeded && decision.proposedNominalFrameRate == 120 &&
        decision.nominalFrameRate == 60 && decision.requestedBitrateBps == 3'000'000 &&
        decision.submittedBitrateBps == 3'000'000,
        "MATERIAL_UPWARD_FPS_IS_IMMEDIATE_AND_PROPOSAL_IS_NOT_COMMITTED");
    ok &= Check(control.Recommend(21'001).nominalFrameRate == 60,
        "FAILED_OR_UNAPPLIED_BACKEND_CHANGE_KEEPS_ACTUAL_NOMINAL");
    control.CommitNominalFrameRate(120, 21'001);
    decision = control.Recommend(21'001);
    ok &= Check(!decision.nominalFrameRateChangeNeeded && decision.submittedBitrateBps == 3'000'000,
        "SUCCESSFUL_COMMIT_KEEPS_REQUESTED_RATE");

    control.Observe(1'000'000, 60.0, 22'000);
    ok &= Check(!control.Recommend(23'999).nominalFrameRateChangeNeeded &&
        !control.Recommend(25'999).nominalFrameRateChangeNeeded,
        "DOWNWARD_FPS_WAITS_FOR_STABILITY_AND_LAST_CHANGE_COOLDOWN");
    decision = control.Recommend(26'001);
    ok &= Check(decision.nominalFrameRateChangeNeeded && decision.proposedNominalFrameRate == 60 &&
        decision.submittedBitrateBps == 1'000'000,
        "STABLE_120_TO_60_FPS_EVENTUALLY_REQUESTS_NOMINAL_CHANGE");
    control.CommitNominalFrameRate(60, 26'001);
    control.Observe(2'000'000, 120.0, 26'002);
    ok &= Check(control.Recommend(26'002).proposedNominalFrameRate == 120 &&
        control.Recommend(26'002).nominalFrameRateChangeNeeded,
        "60_TO_120_UPWARD_FPS_BYPASSES_DOWNWARD_COOLDOWN");

    control.Reset(120, 2'000'000, 0);
    control.Observe(2'000'000, 1.0, 10'000);
    decision = control.Recommend(60'000);
    ok &= Check(!decision.nominalFrameRateChangeNeeded && decision.nominalFrameRate == 120 &&
        decision.submittedBitrateBps == 2'000'000,
        "STATIC_ONE_FPS_RETAINS_NOMINAL_WITHOUT_BITRATE_AMPLIFICATION");
    control.Observe(2'000'000, 120.0, 60'001);
    ok &= Check(!control.Recommend(60'001).nominalFrameRateChangeNeeded,
        "STATIC_TO_120_MOTION_NEEDS_NO_REOPEN_IF_NOMINAL_ALREADY_120");
    control.Reset(1, 2'000'000, 0);
    control.Observe(2'000'000, 120.0, 1);
    ok &= Check(control.Recommend(1).proposedNominalFrameRate == 120 &&
        control.Recommend(1).nominalFrameRateChangeNeeded,
        "ONE_TO_120_UPWARD_FPS_IS_IMMEDIATE");

    control.Reset(120, 1'000'000, 0);
    control.Observe(1'000'000, 60.0, 10'000);
    control.Observe(1'000'000, 60.5, 11'000);
    ok &= Check(control.Recommend(12'000).nominalFrameRateChangeNeeded,
        "SMALL_PENDING_DOWN_JITTER_DOES_NOT_RESET_STABILITY_TIMER");
    control.Observe(1'000'000, 30.0, 12'001);
    ok &= Check(!control.Recommend(14'000).nominalFrameRateChangeNeeded &&
        control.Recommend(14'001).proposedNominalFrameRate == 30,
        "MATERIAL_PENDING_DOWN_CHANGE_RESTARTS_STABILITY_TIMER");
    control.Observe(1'000'000, 120.0, 14'002);
    ok &= Check(!control.Recommend(20'000).nominalFrameRateChangeNeeded,
        "MOTION_RECOVERY_CANCELS_STALE_DOWNWARD_REQUEST");

    control.Observe(0, 1.0, 21'000);
    decision = control.Recommend(30'000);
    ok &= Check(decision.requestedBitrateBps == 0 && decision.submittedBitrateBps == 0 &&
        !decision.nominalFrameRateChangeNeeded,
        "PAUSED_ALLOCATION_DOES_NOT_TRIGGER_NOMINAL_CHANGE");
    control.Observe(1'000'000, 0.0, 31'000);
    control.Observe(1'000'000, std::numeric_limits<double>::quiet_NaN(), 31'001);
    control.Observe(1'000'000, std::numeric_limits<double>::infinity(), 31'002);
    ok &= Check(control.Recommend(31'002).activeFrameRate == 1.0,
        "INVALID_OBSERVATIONS_PRESERVE_LAST_VALID_ACTIVE_FPS");

    FfmpegHardwareRateControlConfig config;
    config.maximumRequestedBitrateBps = 8'000'000;
    config.maximumSubmittedBitrateBps = 20'000'000;
    config.maximumFrameRate = 60;
    FfmpegHardwareRateControl limited(config);
    limited.Reset(120, limit, 0);
    limited.Observe(limit, 1000.0, 1000);
    decision = limited.Recommend(1000);
    ok &= Check(decision.nominalFrameRate == 60 && decision.activeFrameRate == 60.0 &&
        decision.requestedBitrateBps == 8'000'000 && decision.submittedBitrateBps == 8'000'000,
        "CONFIGURED_REQUEST_AND_FPS_LIMITS_ARE_ENFORCED");
    limited.Observe(8'000'000, 0.25, 2000);
    ok &= Check(limited.Recommend(2000).activeFrameRate == 1.0 &&
        limited.Recommend(2000).submittedBitrateBps == 8'000'000,
        "SUB_ONE_FPS_AND_SUBMITTED_RATE_ARE_BOUNDED");

    control.Reset(60, 1'000'000, 10'000);
    control.Observe(1'000'000, 30.0, 11'000);
    control.Observe(1'000'000, 30.0, 0);
    ok &= Check(!control.Recommend(1000).nominalFrameRateChangeNeeded,
        "REGRESSED_CLOCK_CANNOT_BYPASS_LAST_CHANGE_COOLDOWN");
    control.Reset(0, 1'000'000, std::numeric_limits<std::int64_t>::min());
    control.CommitNominalFrameRate(120, std::numeric_limits<std::int64_t>::min());
    control.Observe(1'000'000, 60.0, std::numeric_limits<std::int64_t>::min());
    ok &= Check(control.Recommend(std::numeric_limits<std::int64_t>::max()).nominalFrameRateChangeNeeded,
        "FULL_TIMESTAMP_DOMAIN_HAS_NO_SIGNED_SUBTRACTION_OVERFLOW");

    std::cout << "FFMPEG_HARDWARE_RATE_CONTROL_SELF_TEST=" << (ok ? "PASS" : "FAIL") << '\n';
    return ok ? 0 : 1;
}
