// SPDX-License-Identifier: GPL-3.0-only
#include "media_intelligence/core/H264ReferenceQualityModel.h"
#include <array>
#include <iostream>
#include <limits>
using namespace remote::media_intelligence;
int main()
{
    unsigned checks = 0, failures = 0;
    const auto check = [&](bool value, const char* name) {
        ++checks; if (!value) { ++failures; std::cerr << "FAIL: " << name << '\n'; }
    };
    const StreamQualityRequest request{1920, 1080, 60, ScreenScene::kCodeTerminal};
    std::uint64_t commonRate = 0;
    for (const auto backend : {"FFmpeg/NVENC (p4)", "FFmpeg/QSV", "FFmpeg/AMF (balanced)",
            "FFmpeg/libx264 (medium)", "OpenH264", "Builtin/OpenH264"}) {
        H264ReferenceQualityContext model({"video/H264", backend, "medium"});
        const auto estimate = H264ReferenceQualityContext::Estimate(request, &model);
        check(model.IsValid(), "supported backend");
        check(estimate.available && estimate.reference && !estimate.calibrated, "reference never claims measurement");
        if (!commonRate) commonRate = estimate.requiredVideoBitrateBps;
        check(estimate.requiredVideoBitrateBps == commonRate, "common reference across vendors");
        check(model.VerifyCurrentQuality(request, 25).acceptable &&
            !model.VerifyCurrentQuality(request, 25.01).acceptable, "code scene heuristic boundary");
        check(!model.VerifyCurrentQuality(request, -1).available &&
            !model.VerifyCurrentQuality(request, std::numeric_limits<double>::quiet_NaN()).available &&
            !model.VerifyCurrentQuality(request, 52).available, "invalid QP unavailable");
        for (unsigned scene = 1; scene <= 9; ++scene) {
            auto value = request; value.scene = static_cast<ScreenScene>(scene);
            check(H264ReferenceQualityContext::Estimate(value, &model).reference, "all scenes");
            const ContentAwareStreamConfig seeds;
            const auto qp = seeds.profiles[scene].maximumReferenceAverageQp;
            check(model.VerifyCurrentQuality(value, qp).acceptable &&
                !model.VerifyCurrentQuality(value, qp + .01).acceptable,
                "independent scene QP threshold");
            const auto predicted = static_cast<double>(value.width) * value.height * value.frameRate *
                seeds.profiles[scene].seedBitsPerPixelPerFrame * 1.15;
            const auto rate = H264ReferenceQualityContext::Estimate(value, &model).requiredVideoBitrateBps;
            check(rate >= predicted && rate - predicted <= 1,
                "scene bitrate matches its independent seed and margin");
        }
    }
    H264ReferenceQualityContext model({"video/H264", "FFmpeg/QSV", "medium"});
    check(commonRate >= 21'461'759 && commonRate <= 21'461'761, "explicit seed plus 15 percent margin");
    for (const auto backend : {"Unknown", "FFmpeg/NVENC2", "FFmpeg/QSVbroken", "FFmpeg/AMF ()"}) {
        H264ReferenceQualityContext unknown({"video/H264", backend, "medium"});
        check(!unknown.IsValid() && !H264ReferenceQualityContext::Estimate(request, &unknown).available,
            "unknown backend stays unavailable");
    }
    H264ReferenceQualityContext wrongCodec({"video/H265", "FFmpeg/QSV", "medium"});
    check(!wrongCodec.IsValid(), "no codec extrapolation");
    for (const auto invalid : std::array{StreamQualityRequest{0,1080,60,ScreenScene::kCodeTerminal},
            StreamQualityRequest{1921,1080,60,ScreenScene::kCodeTerminal},
            StreamQualityRequest{1920,1080,121,ScreenScene::kCodeTerminal},
            StreamQualityRequest{1920,1080,0,ScreenScene::kCodeTerminal},
            StreamQualityRequest{1920,1080,60,ScreenScene::kUnknown},
            StreamQualityRequest{16386,1080,60,ScreenScene::kCodeTerminal}}) {
        check(!H264ReferenceQualityContext::Estimate(invalid, &model).available, "invalid request");
    }
    H264ReferenceQualityConfig custom;
    custom.maximumAverageQp = 24; custom.bitrateSafetyMargin = 1.25;
    H264ReferenceQualityContext configured({"video/H264", "FFmpeg/AMF", "medium"}, custom);
    check(configured.VerifyCurrentQuality(request,24).acceptable &&
        !configured.VerifyCurrentQuality(request,24.1).acceptable, "host configurable threshold");
    check(H264ReferenceQualityContext::Estimate(request,&configured).requiredVideoBitrateBps > commonRate,
        "host configurable margin");
    custom.bitrateSafetyMargin = std::numeric_limits<double>::infinity();
    check(!H264ReferenceQualityContext({"video/H264", "FFmpeg/QSV", "medium"},custom).IsValid(), "invalid configuration");
    auto high = request; high.width=16384; high.height=16384; high.frameRate=120;
    check(H264ReferenceQualityContext::Estimate(high,&model).requiredVideoBitrateBps > 100'000'000,
        "never clamp requirement to make impossible candidate feasible");
    auto sceneRequest = request;
    sceneRequest.scene = ScreenScene::kPhotoGraphics;
    check(!model.VerifyCurrentQuality(sceneRequest, 25).acceptable &&
        model.VerifyCurrentQuality(request, 25).acceptable,
        "photo quality stricter than code at same QP");
    sceneRequest.scene = ScreenScene::kVideo;
    check(model.VerifyCurrentQuality(sceneRequest, 29).acceptable &&
        !model.VerifyCurrentQuality(request, 29).acceptable,
        "video quality proxy distinct from text");
    H264ReferenceQualityConfig independent;
    independent.sceneMaximumAverageQp[static_cast<unsigned>(ScreenScene::kCodeTerminal)] = 20;
    H264ReferenceQualityContext independentModel({"video/H264", "FFmpeg/QSV", "medium"}, independent);
    check(!independentModel.VerifyCurrentQuality(request, 21).acceptable &&
        independentModel.VerifyCurrentQuality(sceneRequest, 29).acceptable,
        "changing one scene threshold leaves other scenes intact");
    independent.sceneMaximumAverageQp[8] = std::numeric_limits<double>::quiet_NaN();
    check(!H264ReferenceQualityContext({"video/H264", "FFmpeg/QSV", "medium"}, independent).IsValid(),
        "invalid scene threshold rejects configuration");
    std::cout << "REFERENCE_MODEL_CHECKS=" << checks << " FAILURES=" << failures << '\n';
    return failures ? 1 : 0;
}
