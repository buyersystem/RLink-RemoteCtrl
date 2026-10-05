// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "media_intelligence/core/CalibratedStreamQualityModel.h"

#include <array>
#include <iostream>
#include <limits>
#include <string>

namespace {
using namespace remote::media_intelligence;

bool Check(bool value, const char* name)
{
    std::cout << name << '=' << (value ? "PASS" : "FAIL") << '\n';
    return value;
}

constexpr StreamEncoderProfile kProfile{"H264", "NVENC", "high-p4"};
constexpr StreamQualityRequest kRequest{1920, 1080, 60, ScreenScene::kGame3d};
constexpr StreamQualityCalibrationSample kSample{
    ScreenScene::kGame3d, 1920, 1080, 60, 12'000'000, 300, true, 28};

// All measurements here are TEST fixtures. They must never be shipped as
// production calibration data or interpreted as a visual-quality benchmark.
bool Validation()
{
    bool ok = true;
    CalibratedStreamQualityModel model;
    ok &= Check(!model.IsValid() && !model.Estimate(kRequest, kProfile).available,
        "EMPTY_MODEL_UNAVAILABLE");
    const std::array samples{kSample};
    ok &= Check(model.Build(samples, kProfile, "test-fixture-only") == StreamCalibrationError::kNone &&
        model.IsValid() && model.SampleCount() == 1 &&
        model.QualityEvidenceId() == "test-fixture-only", "VALID_FIXTURE_BUILDS");
    ok &= Check(model.Build({}, kProfile, "test-fixture-only") ==
        StreamCalibrationError::kInvalidSampleCount && !model.IsValid(), "EMPTY_IMPORT_INVALIDATES_OLD_MODEL");
    std::array<StreamQualityCalibrationSample, CalibratedStreamQualityModel::kMaximumSamples + 1> excess{};
    ok &= Check(model.Build(excess, kProfile, "test-fixture-only") ==
        StreamCalibrationError::kInvalidSampleCount, "TABLE_CAPACITY_BOUNDED");
    ok &= Check(model.Build(samples, {"", "NVENC", "high-p4"}, "test-fixture-only") ==
        StreamCalibrationError::kInvalidProfile, "EMPTY_PROFILE_REJECTED");
    const std::string longText(64, 'x');
    ok &= Check(model.Build(samples, {longText, "NVENC", "high-p4"}, "test-fixture-only") ==
        StreamCalibrationError::kInvalidProfile, "PROFILE_LENGTH_BOUNDED");
    ok &= Check(model.Build(samples, {std::string_view("H\0x", 3), "NVENC", "high-p4"}, "test-fixture-only") ==
        StreamCalibrationError::kInvalidProfile, "EMBEDDED_NUL_REJECTED");
    ok &= Check(model.Build(samples, kProfile, "") == StreamCalibrationError::kMissingEvidence,
        "OFFLINE_EVIDENCE_REQUIRED");
    const std::string longEvidence(128, 'x');
    ok &= Check(model.Build(samples, kProfile, longEvidence) == StreamCalibrationError::kMissingEvidence,
        "EVIDENCE_LENGTH_BOUNDED");
    const auto invalid = [&](StreamQualityCalibrationSample sample, StreamCalibrationError error,
        const char* label) {
        return Check(model.Build(std::array{sample}, kProfile, "test-fixture-only") == error &&
            !model.IsValid(), label);
    };
    auto sample = kSample;
    sample.scene = ScreenScene::kUnknown;
    ok &= invalid(sample, StreamCalibrationError::kInvalidScene, "UNKNOWN_SCENE_REJECTED");
    sample.scene = static_cast<ScreenScene>(255);
    ok &= invalid(sample, StreamCalibrationError::kInvalidScene, "INVALID_SCENE_REJECTED");
    sample = kSample;
    sample.width = 1919;
    ok &= invalid(sample, StreamCalibrationError::kInvalidDimensions, "UNALIGNED_SIZE_REJECTED");
    sample.width = std::numeric_limits<std::uint32_t>::max();
    ok &= invalid(sample, StreamCalibrationError::kInvalidDimensions, "DIMENSION_OVERFLOW_REJECTED");
    sample = kSample;
    sample.frameRate = 121;
    ok &= invalid(sample, StreamCalibrationError::kInvalidFrameRate, "FPS_ABOVE_120_REJECTED");
    sample.frameRate = 0;
    ok &= invalid(sample, StreamCalibrationError::kInvalidFrameRate, "ZERO_FPS_REJECTED");
    sample = kSample;
    sample.measuredRequiredBitrateBps = std::numeric_limits<std::uint64_t>::max();
    ok &= invalid(sample, StreamCalibrationError::kInvalidBitrate, "BITRATE_OVERFLOW_REJECTED");
    sample.measuredRequiredBitrateBps = 0;
    ok &= invalid(sample, StreamCalibrationError::kInvalidBitrate, "ZERO_BITRATE_REJECTED");
    sample = kSample;
    sample.measuredFrameCount = 29;
    ok &= invalid(sample, StreamCalibrationError::kInsufficientMeasurement, "INSUFFICIENT_FRAMES_REJECTED");
    sample = kSample;
    sample.measuredVisualQualityPassed = false;
    ok &= invalid(sample, StreamCalibrationError::kVisualQualityNotPassed, "QUALITY_PASS_REQUIRED");
    sample = kSample;
    sample.maximumVerifiedAverageQp = 52;
    ok &= invalid(sample, StreamCalibrationError::kInvalidQualityBoundary, "INVALID_QP_BOUNDARY_REJECTED");
    ok &= Check(model.Build(std::array{kSample, kSample}, kProfile, "test-fixture-only") ==
        StreamCalibrationError::kDuplicateSpecification && !model.IsValid(), "DUPLICATE_SPECIFICATION_REJECTED");
    return ok;
}

bool EnvelopeAndQuality()
{
    bool ok = true;
    CalibratedStreamQualityModel model;
    // The smaller 720p/30 measurement has a HIGHER bitrate than 1080p/60.
    // This is intentional: the envelope must not erase non-monotonic data.
    const std::array samples{
        kSample,
        StreamQualityCalibrationSample{ScreenScene::kGame3d, 1280, 720, 30, 14'000'000, 300, true, 24},
        StreamQualityCalibrationSample{ScreenScene::kGame3d, 3840, 2160, 120, 60'000'000, 300, true, 30},
        StreamQualityCalibrationSample{ScreenScene::kDocument, 1920, 1080, 60, 5'000'000, 300, true, 20},
        StreamQualityCalibrationSample{ScreenScene::kGame3d, 1440, 1080, 60, 40'000'000, 300, true, 10}};
    ok &= Check(model.Build(samples, kProfile, "test-fixture-only") == StreamCalibrationError::kNone,
        "MULTISCENE_FIXTURE_BUILDS");
    const auto estimate = model.Estimate(kRequest, kProfile);
    ok &= Check(estimate.available && estimate.calibrated && estimate.requiredVideoBitrateBps == 14'000'000,
        "NON_MONOTONIC_SMALLER_SAMPLE_PRESERVED");
    ok &= Check(model.Estimate({1760, 990, 45, ScreenScene::kGame3d}, kProfile).requiredVideoBitrateBps == 14'000'000,
        "COVERING_ENVELOPE_NO_OPTIMISTIC_INTERPOLATION");
    ok &= Check(model.Estimate({3840, 2160, 120, ScreenScene::kGame3d}, kProfile).requiredVideoBitrateBps == 60'000'000,
        "MEASURED_4K_120_COVERED");
    ok &= Check(model.Estimate({1920, 1080, 60, ScreenScene::kDocument}, kProfile).requiredVideoBitrateBps == 5'000'000,
        "SCENE_TABLES_ISOLATED");
    ok &= Check(!model.Estimate({4096, 2304, 120, ScreenScene::kGame3d}, kProfile).available,
        "SIZE_EXTRAPOLATION_REJECTED");
    ok &= Check(!model.Estimate({1920, 1080, 121, ScreenScene::kGame3d}, kProfile).calibrated,
        "FPS_EXTRAPOLATION_REJECTED");
    ok &= Check(!model.Estimate({1920, 1080, 60, ScreenScene::kVideo}, kProfile).available,
        "UNMEASURED_SCENE_REJECTED");
    ok &= Check(!model.Estimate({1080, 1920, 60, ScreenScene::kGame3d}, kProfile).available,
        "PORTRAIT_NOT_INFERRED_FROM_LANDSCAPE");
    ok &= Check(model.Estimate({1822, 1024, 60, ScreenScene::kGame3d}, kProfile).available &&
        model.VerifyCurrentQuality({1822, 1024, 60, ScreenScene::kGame3d}, kProfile, 24).acceptable,
        "COMMON_SCALE_EVEN_PIXEL_ROUNDING_COVERED");
    ok &= Check(!model.Estimate({1824, 1000, 60, ScreenScene::kGame3d}, kProfile).available &&
        !model.VerifyCurrentQuality({1824, 1000, 60, ScreenScene::kGame3d}, kProfile, 24).available,
        "DIFFERENT_ASPECT_RATIO_NOT_COVERED");
    ok &= Check(!model.Estimate({1920, 1080, 60, ScreenScene::kUnknown}, kProfile).available,
        "UNKNOWN_REQUEST_REJECTED");
    ok &= Check(!model.Estimate(kRequest, {"H265", "NVENC", "high-p4"}).available &&
        !model.Estimate(kRequest, {"H264", "QSV", "high-p4"}).available &&
        !model.Estimate(kRequest, {"H264", "NVENC", "high-p5"}).available,
        "CODEC_ENCODER_PROFILE_MISMATCH_REJECTED");
    const auto quality = model.VerifyCurrentQuality(kRequest, kProfile, 24);
    ok &= Check(quality.available && quality.acceptable &&
        !model.VerifyCurrentQuality(kRequest, kProfile, 24.01).acceptable,
        "QUALITY_PROXY_USES_CONSERVATIVE_RELATED_BOUNDARY");
    ok &= Check(!model.VerifyCurrentQuality(kRequest, kProfile,
        std::numeric_limits<double>::quiet_NaN()).available &&
        !model.VerifyCurrentQuality(kRequest, kProfile, std::numeric_limits<double>::infinity()).available &&
        !model.VerifyCurrentQuality(kRequest, kProfile, -1).available &&
        !model.VerifyCurrentQuality(kRequest, kProfile, 52).available,
        "INVALID_CURRENT_QP_REJECTED");
    ok &= Check(!model.VerifyCurrentQuality({4096, 2304, 120, ScreenScene::kGame3d}, kProfile, 0).available,
        "UNCOVERED_CURRENT_QUALITY_UNAVAILABLE");
    ok &= Check(!model.VerifyCurrentQuality(kRequest, {"H264", "QSV", "high-p4"}, 0).available,
        "CURRENT_QUALITY_PROVENANCE_REQUIRED");
    auto withoutQp = kSample;
    withoutQp.maximumVerifiedAverageQp = 0;
    CalibratedStreamQualityModel demandOnly;
    ok &= Check(demandOnly.Build(std::array{withoutQp}, kProfile, "test-fixture-only") == StreamCalibrationError::kNone &&
        demandOnly.Estimate(kRequest, kProfile).calibrated &&
        !demandOnly.VerifyCurrentQuality(kRequest, kProfile, 20).available,
        "DEMAND_CALIBRATION_DOES_NOT_INVENT_QP_BOUNDARY");

    CalibratedStreamQualityContext context(model, kProfile);
    CalibratedStreamQualityContext wrongContext(model, {"H264", "AMF", "high-p4"});
    ok &= Check(context.MatchesActiveProfile() &&
        CalibratedStreamQualityContext::Estimate(kRequest, &context).requiredVideoBitrateBps == 14'000'000 &&
        context.VerifyCurrentQuality(kRequest, 24).acceptable, "CALLBACK_AND_QUALITY_CONTEXT_MATCH");
    ok &= Check(!wrongContext.MatchesActiveProfile() &&
        !CalibratedStreamQualityContext::Estimate(kRequest, &wrongContext).available &&
        !wrongContext.VerifyCurrentQuality(kRequest, 0).available &&
        !CalibratedStreamQualityContext::Estimate(kRequest, nullptr).available,
        "MISMATCHED_OR_NULL_CONTEXT_UNAVAILABLE");
    model.Build({}, kProfile, "test-fixture-only");
    ok &= Check(CalibratedStreamQualityContext::Estimate(kRequest, &context).calibrated,
        "CONTEXT_OWNS_IMMUTABLE_MODEL_COPY");
    return ok;
}
}  // namespace

int main()
{
    return Validation() && EnvelopeAndQuality() ? 0 : 1;
}
