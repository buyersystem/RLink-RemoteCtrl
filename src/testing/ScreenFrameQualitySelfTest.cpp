// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)
#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <string_view>
#include <thread>
#include "api/environment/environment_factory.h"
#include "api/video/i420_buffer.h"
#include "modules/video_coding/utility/frame_dropper.h"
#include "modules/video_coding/include/video_error_codes.h"
#include "system_wrappers/include/clock.h"
#include "video/encoder_bitrate_adjuster.h"
#include "src/core/ScreenFrameQualityPolicy.h"
#include "src/webrtc/GoogCcTelemetry.h"
#include "src/webrtc/WindowsPreferredVideoEncoderFactory.h"
#include "src/webrtc/VideoCodecTimingTelemetry.h"
#include "src/platform/win/FfmpegHardwareH264EncoderFactory.h"
#include "src/platform/win/FfmpegX264H264EncoderFactory.h"

namespace {
using namespace std::chrono;
bool Check(bool pass, const char* label)
{ std::cout << label << '=' << (pass ? "PASS" : "FAIL") << std::endl; return pass; }

class DisabledDropperTrials final : public webrtc::FieldTrialsView {
public:
    std::string Lookup(absl::string_view key) const override
    { return key == "WebRTC-FrameDropper" ? "Disabled" : ""; }
};

class DefaultTrials final : public webrtc::FieldTrialsView {
public:
    std::string Lookup(absl::string_view) const override { return {}; }
};

struct Fixture {
    std::uint32_t rate = 0; double fps = 0; bool trusted = false;
    unsigned inits = 0, releases = 0, codecFps = 0, codecCap = 0;
    bool failInit = false, pending = false, keyRequested = false;
};
class Encoder final : public webrtc::VideoEncoder {
public:
    explicit Encoder(std::shared_ptr<Fixture> state) : state_(std::move(state)) {}
    void SetFecControllerOverride(webrtc::FecControllerOverride*) override {}
    int InitEncode(const webrtc::VideoCodec* codec, const Settings&) override
    { ++state_->inits; state_->codecFps = codec->maxFramerate; state_->codecCap = codec->maxBitrate;
      return state_->failInit ? WEBRTC_VIDEO_CODEC_ERROR : 0; }
    int32_t RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* cb) override { cb_ = cb; return 0; }
    int32_t Release() override { ++state_->releases; cb_ = nullptr; return 0; }
    int32_t Encode(const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* types) override
    {
        if (!cb_) return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
        state_->keyRequested = types && !types->empty() && (*types)[0] == webrtc::VideoFrameType::kVideoFrameKey;
        if (state_->pending) return 0;
        webrtc::EncodedImage image;
        image.SetEncodedData(webrtc::EncodedImageBuffer::Create(
            static_cast<std::size_t>(state_->rate / (std::max)(1.0, state_->fps) / 8.0)));
        image.SetRtpTimestamp(frame.rtp_timestamp());
        image._encodedWidth = 1280; image._encodedHeight = 720;
        image._frameType = state_->keyRequested ? webrtc::VideoFrameType::kVideoFrameKey
                                              : webrtc::VideoFrameType::kVideoFrameDelta;
        image.qp_ = 28 + static_cast<int>((8'000'000 - (std::min)(state_->rate, 8'000'000u)) * 20ull / 6'000'000);
        cb_->OnEncodedImage(image, nullptr);
        return 0;
    }
    void SetRates(const RateControlParameters& rates) override
    { state_->rate = rates.bitrate.get_sum_bps(); state_->fps = rates.framerate_fps; }
    EncoderInfo GetEncoderInfo() const override
    { EncoderInfo info; info.implementation_name = "ReferenceEncoder"; info.has_trusted_rate_controller = state_->trusted; return info; }
private:
    std::shared_ptr<Fixture> state_;
    webrtc::EncodedImageCallback* cb_ = nullptr;
};
class Factory final : public webrtc::VideoEncoderFactory {
public:
    explicit Factory(std::shared_ptr<Fixture> state) : state_(std::move(state)) {}
    std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override { return {webrtc::SdpVideoFormat("H264")}; }
    std::unique_ptr<webrtc::VideoEncoder> Create(const webrtc::Environment&, const webrtc::SdpVideoFormat&) override
    { return std::make_unique<Encoder>(state_); }
private: std::shared_ptr<Fixture> state_;
};
struct Output final : webrtc::EncodedImageCallback {
    webrtc::FrameDropper* dropper = nullptr;
    webrtc::EncoderBitrateAdjuster* adjuster = nullptr;
    std::uint64_t bytes = 0, frames = 0, qpFrames = 0, qpSum = 0, dropped = 0;
    Result OnEncodedImage(const webrtc::EncodedImage& image, const webrtc::CodecSpecificInfo*) override
    {
        bytes += image.size(); ++frames;
        if (image.qp_ >= 0) { ++qpFrames; qpSum += image.qp_; }
        if (dropper) dropper->Fill(image.size(), !image.IsKey());
        if (adjuster) adjuster->OnEncodedFrame(webrtc::DataSize::Bytes(image.size()), 0, 0);
        return Result(Result::OK);
    }
    void OnFrameDropped(std::uint32_t, int, bool) override { ++dropped; }
};

void Evidence(const std::shared_ptr<remote::GoogCcTelemetryState>& control, bool pressure)
{
    const auto now = remote::GoogCcTelemetryState::NowMs();
    webrtc::NetworkControlUpdate update;
    update.target_rate.emplace();
    update.target_rate->target_rate = webrtc::DataRate::BitsPerSec(pressure ? 2'000'000 : 8'000'000);
    update.target_rate->network_estimate.round_trip_time = webrtc::TimeDelta::Millis(30);
    control->ObserveUpdate(update, now);
    control->ConfirmController(now);
    control->ConfirmFeedback(now, true);
    control->ObserveDelay(pressure ? webrtc::BandwidthUsage::kBwOverusing : webrtc::BandwidthUsage::kBwNormal, now);
}
void SetRates(webrtc::VideoEncoder& encoder, std::uint32_t budget, unsigned fps)
{
    webrtc::VideoBitrateAllocation rate;
    rate.SetBitrate(0, 0, budget);
    webrtc::VideoEncoder::RateControlParameters parameters(rate, fps);
    // Constructors initialize only the adjusted bitrate; VideoStreamEncoder
    // explicitly supplies the pre-adjustment target allocation separately.
    parameters.target_bitrate = rate;
    encoder.SetRates(parameters);
}

void SetAllocatedRates(webrtc::VideoEncoder& encoder, std::uint32_t target,
                       std::uint32_t adjusted, unsigned fps,
                       std::uint32_t bandwidth)
{
    webrtc::VideoBitrateAllocation rate;
    rate.SetBitrate(0, 0, adjusted);
    webrtc::VideoEncoder::RateControlParameters parameters(rate, fps,
        webrtc::DataRate::BitsPerSec(bandwidth));
    parameters.target_bitrate.SetBitrate(0, 0, target);
    encoder.SetRates(parameters);
}

bool Run(bool hardware, bool protection, bool software)
{
    bool ok = true;
    constexpr unsigned fps = 120;
    constexpr std::uint32_t normal = 8'000'000, weak = 2'000'000;
    auto control = std::make_shared<remote::GoogCcTelemetryState>();
    control->SetScreenQualityProtection(protection);
    // Keep the stricter 0.20 quality regression; default and endpoints have
    // separate coverage, so changing the default cannot weaken this check.
    control->SetScreenQualityDeficitShare(20);
    auto fixture = std::make_shared<Fixture>();
    std::unique_ptr<webrtc::VideoEncoderFactory> child;
    if (hardware) child = std::make_unique<remote::FfmpegHardwareH264EncoderFactory>(
        remote::FfmpegHardwareBackend::kNvenc);
    else if (software) child = std::make_unique<remote::FfmpegX264H264EncoderFactory>(remote::FfmpegX264Preset::kUltraFast);
    else child = std::make_unique<Factory>(fixture);
    remote::WindowsPreferredVideoEncoderFactory factory(nullptr, std::move(child), nullptr, control);
    auto encoder = factory.Create(webrtc::CreateEnvironment(), webrtc::SdpVideoFormat("H264"));
    webrtc::VideoCodec codec{};
    codec.codecType = webrtc::kVideoCodecH264;
    codec.width = 1280; codec.height = 720; codec.maxFramerate = fps;
    codec.mode = webrtc::VideoCodecMode::kScreensharing;
    codec.startBitrate = codec.maxBitrate = normal / 1000; codec.minBitrate = 30;
    codec.H264()->numberOfTemporalLayers = 1;
    Output output;
    webrtc::FrameDropper dropper;
    output.dropper = &dropper;
    ok &= Check(encoder && encoder->InitEncode(&codec, webrtc::VideoEncoder::Settings(
        webrtc::VideoEncoder::Capabilities(false), 4, 1200)) == 0, "ENCODER_READY");
    if (!ok) return false;
    encoder->RegisterEncodeCompleteCallback(&output);
    std::vector<webrtc::scoped_refptr<webrtc::I420Buffer>> buffers;
    for (unsigned n = 0; n < 8; ++n) {
        auto buffer = webrtc::I420Buffer::Create(codec.width, codec.height);
        for (int y = 0; y < buffer->height(); ++y)
            for (int x = 0; x < buffer->width(); ++x) {
                const unsigned value = (x + n * 17) * 2654435761u ^ (y + n * 7) * 2246822519u;
                // Moderate detail with changing edges; no user's screen is read.
                buffer->MutableDataY()[y * buffer->StrideY() + x] =
                    static_cast<std::uint8_t>(16 + (value % 64) + ((x / 12 + y / 12 + n) % 3) * 60);
            }
        std::memset(buffer->MutableDataU(), 110, buffer->StrideU() * buffer->height() / 2);
        std::memset(buffer->MutableDataV(), 145, buffer->StrideV() * buffer->height() / 2);
        buffers.push_back(std::move(buffer));
    }
    unsigned frameId = 0;
    double normalQp = -1;
    auto next = steady_clock::now();
    struct Phase { const char* name; std::uint32_t rate; bool pressure; unsigned seconds; };
    for (const auto& phase : {Phase{"normal", normal, false, 3}, Phase{"weak", weak, true, 7}, Phase{"recover", normal, false, 3}}) {
        Evidence(control, phase.pressure);
        SetRates(*encoder, phase.rate, fps);
        dropper.SetRates(phase.rate / 1000.0f, fps);
        for (unsigned second = 0; second < phase.seconds; ++second) {
            const auto beforeBytes = output.bytes, beforeFrames = output.frames;
            const auto beforeQpCount = output.qpFrames, beforeQp = output.qpSum;
            for (unsigned f = 0; f < fps; ++f) {
                if (f % 24 == 0) { Evidence(control, phase.pressure); SetRates(*encoder, phase.rate, fps); }
                dropper.Leak(fps);
                if (!dropper.DropFrame()) {
                    auto frame = webrtc::VideoFrame::Builder().set_video_frame_buffer(buffers[frameId % 8])
                        .set_rtp_timestamp(++frameId * 750).set_timestamp_us(frameId * 8333ll).build();
                    ok &= encoder->Encode(frame, nullptr) == 0;
                } else ++frameId;
                if (hardware || software) { next += microseconds(1'000'000 / fps); std::this_thread::sleep_until(next); }
            }
            const auto count = output.frames - beforeFrames;
            const double qp = output.qpFrames == beforeQpCount ? -1 :
                static_cast<double>(output.qpSum - beforeQp) / (output.qpFrames - beforeQpCount);
            const auto timing = remote::VideoCodecTimingRegistry::Instance().SnapshotForImplementation(
                encoder->GetEncoderInfo().implementation_name);
            if (std::string_view(phase.name) == "normal") normalQp = qp;
            if (std::string_view(phase.name) == "recover" && second == 0)
                ok &= Check(count >= fps * .9, "FIRST_RECOVERY_SECOND_REACHES_TARGET");
            std::cout << "mode=" << (protection ? "quality" : "gcc_only") << ",phase=" << phase.name
                << ",second=" << second + 1 << ",input_fps=" << fps << ",encoded_frames=" << count
                << ",bytes_bps=" << (output.bytes - beforeBytes) * 8 << ",qp=" << qp
                << ",encoder_reference=" << (timing ? timing->screenQuality.encoderReferenceBps : 0) << std::endl;
            if (second + 1 == phase.seconds) {
                if (std::string_view(phase.name) == "weak") {
                    ok &= Check((output.bytes - beforeBytes) * 8 <= weak * 1.25, "NATIVE_DROPPER_BOUNDS_ACTUAL_OUTPUT");
                    if (!hardware && !software) {
                        ok &= Check(protection ? count < fps / 2 && qp <= 35 : count >= fps * .9 && qp >= 40,
                            "QUALITY_FIRST_VS_GCC_ONLY");
                        ok &= Check(fixture->fps == fps, "ENCODER_NOMINAL_CADENCE_UNCHANGED");
                    }
                    else if (protection) ok &= Check(count < fps * .75 && qp >= 0 && qp <= normalQp + 4,
                        "REAL_ENCODER_PRESERVES_FRAME_QUALITY_BY_NATIVE_DROPS");
                }
                if (std::string_view(phase.name) == "recover") ok &= Check(count >= fps * .9, "NATIVE_RECOVERY_WITHOUT_FPS_LADDER_WAIT");
            }
        }
    }
    if (protection) {
        // Exercise the real backend's cached time base and maxBitrate, not
        // just the reference policy. Network updates above must not reopen it.
        for (const unsigned userFps : {60u, 120u}) {
            const auto cap = userFps == 60 ? 4'000'000u : 8'000'000u;
            control->SetScreenQualityTarget(userFps, cap);
            Evidence(control, false);
            SetRates(*encoder, cap, userFps);
            const auto before = output.frames;
            for (unsigned n = 0; n < 10; ++n) {
                auto frame = webrtc::VideoFrame::Builder().set_video_frame_buffer(buffers[frameId % 8])
                    .set_rtp_timestamp(++frameId * 750).set_timestamp_us(frameId * 8333ll).build();
                ok &= encoder->Encode(frame, nullptr) == 0;
            }
            const auto current = remote::VideoCodecTimingRegistry::Instance().SnapshotForImplementation(
                encoder->GetEncoderInfo().implementation_name);
            ok &= Check(output.frames > before && current && current->screenQualityProtectionAvailable &&
                current->screenQuality.encoderReferenceBps <= cap,
                "BACKEND_CONTINUES_AFTER_DYNAMIC_USER_FPS_AND_CAP_CHANGE");
        }
    }
    // Runtime ON switch must release the held frame-quality reference before
    // encoding the next frame, even without another network allocation update.
    control->SetScreenQualityProtection(false);
    SetRates(*encoder, weak, fps);
    const auto timing = remote::VideoCodecTimingRegistry::Instance().SnapshotForImplementation(encoder->GetEncoderInfo().implementation_name);
    ok &= Check(timing && !timing->screenQualityProtectionAvailable &&
        timing->screenQuality.encoderReferenceBps == weak, "MODE_SWITCH_RELEASES_REFERENCE");
    encoder->Release();
    return ok;
}

bool Core()
{
    remote::ScreenFrameQualityPolicy policy;
    policy.Reset(120, 8'000'000, 8'000'000);
    bool ok = true;
    ok &= Check(!policy.Update(2'000'000, true, false).protecting, "LOW_BUDGET_ALONE_IS_NOT_CONGESTION");
    policy.Update(8'000'000, true, false);
    for (int i = 0; i < 8; ++i) policy.ObserveEncoded(5000, 28, false);
    auto result = policy.Update(2'000'000, true, true);
    ok &= Check(result.protecting && result.referenceBps == 4'800'000 &&
        result.encoderReferenceBps == 3'400'000 && result.deficitShareHundredths == 50,
        "REAL_FRAME_REFERENCE_WITH_DEFAULT_HALF_DEFICIT_SHARE");
    policy.ObserveEncoded(1'000'000, 28, true);
    ok &= Check(policy.Update(2'000'000, true, false).referenceBps == 4'800'000, "KEYFRAME_DOES_NOT_INFLATE_REFERENCE");
    ok &= Check(policy.Update(8'000'000, true, false).encoderReferenceBps == 8'000'000, "FULL_BUDGET_RECOVERY_IMMEDIATE");
    result = policy.Update(10'000, true, true);
    ok &= Check(result.qualityLimited && result.encoderReferenceBps == 2'405'000,
        "EXTREME_SCARCITY_STILL_USES_REQUESTED_DEFICIT_FORMULA");
    ok &= Check(policy.Update(0, true, true).encoderReferenceBps == 0, "ZERO_BUDGET_STAYS_PAUSED");
    ok &= Check(policy.Update(10'000, false, true).encoderReferenceBps == 10'000, "DISABLED_NEVER_AMPLIFIES");
    policy.Reset(120, 8'000'000, 8'000'000);
    policy.Update(2'000'000, true, true);
    ok &= Check(!policy.Update(8'000'000, true, false).protecting, "FIRST_WEAK_WINDOW_CAN_RECOVER");
    for (int i = 0; i < 8; ++i) policy.ObserveEncoded(5000, 28, false);
    policy.Retarget(60, 4'000'000);
    ok &= Check(policy.Update(1'000'000, true, true).referenceBps == 2'400'000,
        "USER_FPS_CHANGE_RETAINS_PER_FRAME_COST");
    policy.Retarget(60, 1'500'000);
    ok &= Check(policy.Update(1'000'000, true, true).encoderReferenceBps <= 1'500'000,
        "USER_CAP_CHANGE_BOUNDS_REFERENCE");
    policy.Reset(80, 24'880'000, 24'880'000);
    auto blend = policy.Update(6'890'000, true, true, 20);
    ok &= Check(blend.encoderReferenceBps == 21'282'000 && blend.referenceBps == 24'880'000,
        "USER_EXAMPLE_A_MINUS_DEFICIT_TIMES_POINT_TWO");
    ok &= Check(policy.Update(6'890'000, true, true, 20).encoderReferenceBps == blend.encoderReferenceBps,
        "REPEATED_BUDGET_UPDATES_DO_NOT_COMPOUND_DISCOUNT");
    const auto low = policy.Update(6'890'000, true, true, 5);
    const auto high = policy.Update(6'890'000, true, true, 50);
    const auto upper = policy.Update(6'890'000, true, true, 80);
    ok &= Check(low.encoderReferenceBps == 23'980'500 && high.encoderReferenceBps == 15'885'000 &&
        high.encoderReferenceBps < low.encoderReferenceBps,
        "HIGHER_SHARE_PRIORITIZES_FPS_LOWER_SHARE_PRIORITIZES_FRAME_QUALITY");
    ok &= Check(upper.encoderReferenceBps == 10'488'000 &&
        upper.deficitShareHundredths == 80 && upper.encoderReferenceBps < high.encoderReferenceBps,
        "POINT_EIGHT_SHARE_PRIORITIZES_FPS_WITHOUT_CHANGING_NETWORK_BUDGET");
    const auto zero = policy.Update(6'890'000, true, true, 0);
    const auto one = policy.Update(6'890'000, true, true, 100);
    ok &= Check(zero.deficitShareHundredths == 0 && zero.encoderReferenceBps == 24'880'000 &&
        zero.networkBudgetBps == 6'890'000 && one.deficitShareHundredths == 100 &&
        one.encoderReferenceBps == 6'890'000 && one.networkBudgetBps == 6'890'000,
        "ZERO_PRESERVES_REFERENCE_ONE_USES_NETWORK_BUDGET");
    ok &= Check(policy.Update(6'890'000, true, true, 101).encoderReferenceBps == one.encoderReferenceBps &&
        remote::NormalizeScreenQualityDeficitShareHundredths(0) == 0 &&
        remote::NormalizeScreenQualityDeficitShareHundredths(101) == 100,
        "DEFICIT_SHARE_RANGE_NORMALIZED_AT_API_BOUNDARY");
    ok &= Check(policy.Update(0, true, true, 0).encoderReferenceBps == 0 &&
        policy.Update(0, true, true, 100).encoderReferenceBps == 0,
        "ZERO_NETWORK_BUDGET_PAUSES_AT_BOTH_COEFFICIENT_ENDPOINTS");
    ok &= Check(policy.Update(30'000'000, true, false, 20).encoderReferenceBps == 30'000'000,
        "BUDGET_ABOVE_REFERENCE_USES_RAW_ALLOCATION");
    policy.Reset(120, 8'000'000, 8'000'000);
    policy.Update(8'000'000, true, false);
    for (int i = 0; i < 8; ++i) policy.ObserveEncoded(5000, 28, false);
    policy.Update(2'000'000, true, true);
    for (int i = 0; i < 240; ++i) policy.ObserveEncoded(10000, 50, false);
    ok &= Check(policy.Update(2'000'000, true, true).referenceBps == 4'800'000,
        "INTENTIONAL_QP_LOSS_DOES_NOT_INFLATE_HELD_QUALITY_REFERENCE");
    ok &= Check(!policy.Update(4'800'000, true, false).protecting &&
        policy.Update(4'800'000, true, false).encoderReferenceBps == 4'800'000,
        "RECOVERY_NEEDS_REAL_FRAME_COST_NOT_PREVIOUS_EXCESS_ALLOCATION");
    return ok;
}

bool UnadjustedBudgetAndRecovery()
{
    bool ok = true;
    // Include mode, eligibility, native-dropper and controller bypasses. Their
    // adjusted bitrate must retain its original meaning; only the supported
    // screen-quality path uses the allocation before EncoderBitrateAdjuster.
    for (int variant = 0; variant < 6; ++variant) {
        auto control = std::make_shared<remote::GoogCcTelemetryState>();
        auto fixture = std::make_shared<Fixture>();
        fixture->trusted = variant == 2;
        control->SetScreenQualityProtection(variant != 4);
        const auto env = variant == 5
            ? webrtc::CreateEnvironment(std::make_unique<DisabledDropperTrials>())
            : webrtc::CreateEnvironment();
        remote::WindowsPreferredVideoEncoderFactory factory(nullptr,
            std::make_unique<Factory>(fixture), nullptr, control);
        auto encoder = factory.Create(env, webrtc::SdpVideoFormat("H264"));
        webrtc::VideoCodec codec{};
        codec.codecType = webrtc::kVideoCodecH264;
        codec.width = 1280; codec.height = 720; codec.maxFramerate = 80;
        codec.mode = variant == 1 ? webrtc::VideoCodecMode::kRealtimeVideo
                                  : webrtc::VideoCodecMode::kScreensharing;
        codec.maxBitrate = codec.startBitrate = 24000; codec.minBitrate = 30;
        codec.H264()->numberOfTemporalLayers = variant == 3 ? 2 : 1;
        ok &= Check(encoder->InitEncode(&codec, webrtc::VideoEncoder::Settings(
            webrtc::VideoEncoder::Capabilities(false), 4, 1200)) == 0,
            "UNADJUSTED_BUDGET_ENCODER_READY");
        Output output;
        encoder->RegisterEncodeCompleteCallback(&output);
        Evidence(control, false);
        SetAllocatedRates(*encoder, 24'000'000, 12'000'000, 80, 30'000'000);
        // Timing becomes valid only after a completed frame. A key frame
        // establishes the telemetry without changing the learned A reference.
        auto frame = webrtc::VideoFrame::Builder().set_video_frame_buffer(
            webrtc::I420Buffer::Create(16, 16)).set_rtp_timestamp(1).build();
        const std::vector<webrtc::VideoFrameType> keyFrame{webrtc::VideoFrameType::kVideoFrameKey};
        ok &= encoder->Encode(frame, &keyFrame) == 0;
        auto timing = remote::VideoCodecTimingRegistry::Instance().SnapshotForImplementation(
            encoder->GetEncoderInfo().implementation_name);
        if (variant != 0) {
            ok &= Check(fixture->rate == 12'000'000 && fixture->fps == 80 &&
                timing && !timing->screenQualityProtectionAvailable,
                "UNSUPPORTED_PATHS_PRESERVE_ADJUSTED_ALLOCATION");
            encoder->Release();
            continue;
        }
        ok &= Check(fixture->rate == 24'000'000 && timing &&
            timing->screenQuality.networkBudgetBps == 24'000'000 &&
            timing->screenQuality.encoderAdjustedBudgetBps == 12'000'000 &&
            timing->screenQuality.bandwidthAllocationBps == 30'000'000 &&
            !timing->screenQuality.protecting,
            "HEALTHY_TARGET_NOT_HALVED_BY_ADJUSTER_OR_CONNECTION_ESTIMATE");
        Evidence(control, true);
        for (const unsigned share : {0u, 5u, 20u, 50u, 80u, 100u}) {
            control->SetScreenQualityDeficitShare(share);
            SetAllocatedRates(*encoder, 2'000'000, 1'000'000, 40, 30'000'000);
            const auto expected = 24'000'000u - 22'000'000u * share / 100u;
            timing = remote::VideoCodecTimingRegistry::Instance().SnapshotForImplementation(
                encoder->GetEncoderInfo().implementation_name);
            ok &= Check(fixture->rate == expected && fixture->fps == 80 && timing &&
                timing->screenQuality.networkBudgetBps == 2'000'000 &&
                timing->screenQuality.encoderAdjustedBudgetBps == 1'000'000 &&
                fixture->inits == 1,
                "DEFICIT_BLEND_USES_UNADJUSTED_TARGET_FOR_ALL_COEFFICIENTS");
        }
        Evidence(control, false);
        SetAllocatedRates(*encoder, 24'000'000, 12'000'000, 40, 30'000'000);
        timing = remote::VideoCodecTimingRegistry::Instance().SnapshotForImplementation(
            encoder->GetEncoderInfo().implementation_name);
        ok &= Check(fixture->rate == 24'000'000 && fixture->fps == 80 && fixture->inits == 1 &&
            timing && !timing->screenQuality.protecting,
            "RESTORED_TARGET_RELEASES_QUALITY_HOLD_WITHOUT_USER_FPS_CLICK");
        SetAllocatedRates(*encoder, 0, 1'000'000, 80, 30'000'000);
        ok &= Check(fixture->rate == 0,
            "EXPLICIT_ZERO_TARGET_IS_PAUSED_DESPITE_ADJUSTED_OR_AVAILABLE_BANDWIDTH");
        encoder->Release();
    }
    return ok;
}

bool CoupledAdjusterAndNativeDropperRecovery()
{
    constexpr unsigned fps = 120;
    constexpr std::uint32_t normal = 8'000'000, weak = 2'000'000;
    auto control = std::make_shared<remote::GoogCcTelemetryState>();
    control->SetScreenQualityProtection(true);
    auto fixture = std::make_shared<Fixture>();
    remote::WindowsPreferredVideoEncoderFactory factory(nullptr,
        std::make_unique<Factory>(fixture), nullptr, control);
    auto encoder = factory.Create(webrtc::CreateEnvironment(), webrtc::SdpVideoFormat("H264"));
    webrtc::VideoCodec codec{};
    codec.codecType = webrtc::kVideoCodecH264;
    codec.width = 1280; codec.height = 720; codec.maxFramerate = fps;
    codec.mode = webrtc::VideoCodecMode::kScreensharing;
    codec.startBitrate = codec.maxBitrate = normal / 1000; codec.minBitrate = 30;
    codec.H264()->numberOfTemporalLayers = 1;
    bool ok = Check(encoder->InitEncode(&codec, webrtc::VideoEncoder::Settings(
        webrtc::VideoEncoder::Capabilities(false), 4, 1200)) == 0,
        "COUPLED_ADJUSTER_DROPPER_ENCODER_READY");
    if (!ok) return false;
    webrtc::SimulatedClock clock(100'000);
    DefaultTrials trials;
    webrtc::EncoderBitrateAdjuster adjuster(codec, trials, clock);
    adjuster.OnEncoderInfo(encoder->GetEncoderInfo());
    webrtc::FrameDropper dropper;
    Output output;
    output.dropper = &dropper;
    output.adjuster = &adjuster;
    encoder->RegisterEncodeCompleteCallback(&output);
    auto buffer = webrtc::I420Buffer::Create(16, 16);
    unsigned frameId = 0;
    std::uint32_t lowestWeakAdjusted = weak;
    std::uint32_t firstRecoveryAdjusted = normal;
    struct Phase { const char* name; std::uint32_t rate; bool pressure; unsigned seconds; };
    for (const auto& phase : {Phase{"normal", normal, false, 3},
                              Phase{"short_limit", weak, true, 2},
                              Phase{"recover", normal, false, 3}}) {
        dropper.SetRates(phase.rate / 1000.0f, fps);
        Evidence(control, phase.pressure);
        for (unsigned second = 0; second < phase.seconds; ++second) {
            const auto beforeFrames = output.frames;
            const auto beforeBytes = output.bytes;
            for (unsigned n = 0; n < fps; ++n) {
                if (n % 24 == 0) {
                    Evidence(control, phase.pressure);
                    webrtc::VideoBitrateAllocation allocation;
                    allocation.SetBitrate(0, 0, phase.rate);
                    webrtc::VideoEncoder::RateControlParameters rates(allocation, fps,
                        webrtc::DataRate::BitsPerSec(phase.rate));
                    // Same order as VideoStreamEncoder::UpdateBitrateAllocation:
                    // keep target_bitrate, replace only bitrate with adjustment.
                    rates.target_bitrate = allocation;
                    rates.bitrate = adjuster.AdjustRateAllocation(rates);
                    encoder->SetRates(rates);
                    if (phase.pressure) lowestWeakAdjusted = (std::min)(
                        lowestWeakAdjusted, rates.bitrate.get_sum_bps());
                    if (std::string_view(phase.name) == "recover" && second == 0 && n == 0)
                        firstRecoveryAdjusted = rates.bitrate.get_sum_bps();
                }
                dropper.Leak(fps);
                ++frameId;
                if (!dropper.DropFrame()) {
                    auto frame = webrtc::VideoFrame::Builder().set_video_frame_buffer(buffer)
                        .set_rtp_timestamp(frameId * 750).set_timestamp_us(clock.TimeInMicroseconds()).build();
                    ok &= encoder->Encode(frame, nullptr) == 0;
                } else {
                    adjuster.OnFrameDropped();
                }
                clock.AdvanceTimeMicroseconds(1'000'000 / fps);
            }
            const auto frames = output.frames - beforeFrames;
            std::cout << "coupled_phase=" << phase.name << ",second=" << second + 1
                << ",encoded_frames=" << frames << ",bytes_bps=" << (output.bytes - beforeBytes) * 8
                << ",backend_reference=" << fixture->rate << std::endl;
            if (std::string_view(phase.name) == "recover") {
                ok &= Check(fixture->rate == normal && fixture->inits == 1,
                    "COUPLED_RECOVERY_USES_FULL_TARGET_WITHOUT_ENCODER_RESTART");
                if (second == 0) ok &= Check(frames >= fps * .9,
                    "COUPLED_SHORT_OUTAGE_RECOVERS_NATIVE_FPS_IN_FIRST_SECOND");
                if (second + 1 == phase.seconds) ok &= Check(frames == fps,
                    "COUPLED_RECOVERY_REACHES_USER_TARGET_WITHOUT_CLICK");
            }
        }
    }
    ok &= Check(lowestWeakAdjusted <= weak * .55,
        "REAL_BITRATE_ADJUSTER_REPRODUCES_HALF_ALLOCATION");
    ok &= Check(firstRecoveryAdjusted < normal * .9,
        "REAL_ADJUSTER_STILL_DISCOUNTED_WHEN_RECOVERED_TARGET_REACHES_ENCODER");
    encoder->Release();
    return ok;
}

bool DynamicAndEligibility()
{
    bool ok = true;
    for (int variant = 0; variant < 6; ++variant) {
        auto control = std::make_shared<remote::GoogCcTelemetryState>();
        auto fixture = std::make_shared<Fixture>();
        fixture->trusted = variant == 2;
        control->SetScreenQualityProtection(variant != 4);
        remote::WindowsPreferredVideoEncoderFactory factory(nullptr, std::make_unique<Factory>(fixture), nullptr, control);
        auto encoder = factory.Create(webrtc::CreateEnvironment(), webrtc::SdpVideoFormat("H264"));
        webrtc::VideoCodec codec{};
        codec.codecType = webrtc::kVideoCodecH264;
        codec.width = 1280; codec.height = 720; codec.maxFramerate = variant == 4 ? 30 : 120;
        codec.mode = variant == 1 ? webrtc::VideoCodecMode::kRealtimeVideo : webrtc::VideoCodecMode::kScreensharing;
        codec.maxBitrate = codec.startBitrate = 8000; codec.minBitrate = 30;
        codec.H264()->numberOfTemporalLayers = variant == 3 ? 2 : 1;
        Output output;
        ok &= Check(encoder->InitEncode(&codec, webrtc::VideoEncoder::Settings(
            webrtc::VideoEncoder::Capabilities(false), 4, 1200)) == 0, "PROFILE_ENCODER_READY");
        encoder->RegisterEncodeCompleteCallback(&output);
        control->SetScreenQualityTarget(120, 8'000'400);
        Evidence(control, true);
        SetRates(*encoder, 2'000'000, 120);
        if (variant >= 1 && variant <= 3) {
            ok &= Check(fixture->rate == 2'000'000 && fixture->inits == 1,
                "CAMERA_TRUSTED_AND_LAYERED_ENCODERS_BYPASS");
            encoder->Release();
            continue;
        }
        if (variant == 4) {
            ok &= Check(fixture->inits == 1 && fixture->codecFps == 30,
                "SCENE_MODE_RETAINS_ITS_ENCODER_SPECIFICATION");
            control->SetScreenQualityProtection(true);
            SetRates(*encoder, 2'000'000, 120);
            ok &= Check(fixture->inits == 2 && fixture->codecFps == 120,
                "OFF_AFTER_SCENE_START_RESTORES_USER_CODEC");
        } else ok &= Check(fixture->inits == 1, "SUB_KBPS_ROUNDING_DOES_NOT_REOPEN_ENCODER");
        const auto inits = fixture->inits;
        control->SetScreenQualityDeficitShare(50);
        SetRates(*encoder, 2'000'000, 120);
        ok &= Check(fixture->inits == inits && fixture->rate >= 5'000'000 && fixture->rate <= 5'000'400,
            "COEFFICIENT_UPDATE_REACHES_ENCODER_WITHOUT_RESTART_OR_RTP_EDIT");
        control->SetScreenQualityDeficitShare(80);
        SetRates(*encoder, 2'000'000, 120);
        ok &= Check(fixture->inits == inits && fixture->rate >= 3'200'000 && fixture->rate <= 3'200'400,
            "POINT_EIGHT_UPDATE_REACHES_ENCODER_WITHOUT_RESTART_OR_RTP_EDIT");
        control->SetScreenQualityDeficitShare(0);
        SetRates(*encoder, 2'000'000, 120);
        ok &= Check(fixture->inits == inits && fixture->fps == 120 &&
            fixture->rate >= 8'000'000 && fixture->rate <= 8'000'400,
            "ZERO_SHARE_REACHES_ENCODER_WITH_ORIGINAL_REFERENCE_AND_FPS");
        control->SetScreenQualityDeficitShare(100);
        auto endpointFrame = webrtc::VideoFrame::Builder().set_video_frame_buffer(
            webrtc::I420Buffer::Create(16, 16)).set_rtp_timestamp(98).build();
        encoder->Encode(endpointFrame, nullptr);
        ok &= Check(fixture->inits == inits && fixture->fps == 120 && fixture->rate == 2'000'000,
            "ONE_SHARE_USES_NETWORK_BUDGET_NEXT_FRAME_WITHOUT_RESTART");
        control->SetScreenQualityDeficitShare(5);
        auto coefficientFrame = webrtc::VideoFrame::Builder().set_video_frame_buffer(
            webrtc::I420Buffer::Create(16, 16)).set_rtp_timestamp(99).build();
        encoder->Encode(coefficientFrame, nullptr);
        ok &= Check(fixture->rate >= 7'700'000 && fixture->rate <= 7'700'400 && fixture->inits == inits,
            "COEFFICIENT_TAKES_EFFECT_NEXT_FRAME_WITHOUT_NEW_GCC_UPDATE");
        control->SetScreenQualityDeficitShare(20);
        SetRates(*encoder, 1'000'000, 30);
        ok &= Check(fixture->inits == inits && fixture->fps == 120,
            "NETWORK_BUDGET_NEVER_REOPENS_OR_REWRITES_NOMINAL_FPS");
        auto buffer = webrtc::I420Buffer::Create(1280, 720);
        auto frame = webrtc::VideoFrame::Builder().set_video_frame_buffer(buffer).set_rtp_timestamp(123).build();
        fixture->pending = true;
        encoder->Encode(frame, nullptr);
        fixture->pending = false;
        fixture->failInit = variant == 5;
        control->SetScreenQualityTarget(60, 4'000'000);
        SetRates(*encoder, 1'000'000, 60);
        ok &= Check(fixture->inits == inits + 1 && fixture->codecFps == 60 && fixture->codecCap == 4000 && output.dropped == 1,
            "USER_CHANGE_REOPENS_ONCE_AND_COMPLETES_PENDING_FRAME");
        const auto result = encoder->Encode(frame, nullptr);
        if (variant == 5) ok &= Check(result == WEBRTC_VIDEO_CODEC_ERROR,
            "REINITIALIZATION_FAILURE_PROPAGATES_TO_NATIVE_ENCODER_LOOP");
        else {
            ok &= Check(result == 0 && fixture->keyRequested, "USER_PROFILE_RESTART_FORCES_KEYFRAME");
            SetRates(*encoder, 2'000'000, 60);
            ok &= Check(fixture->inits == inits + 1 && fixture->fps == 60,
                "UPDATED_USER_PROFILE_HAS_NO_REPEATED_REINITIALIZATION");
            control->SetScreenQualityProtection(false);
            SetRates(*encoder, 900'000, 30);
            ok &= Check(fixture->rate == 900'000 && fixture->fps == 30,
                "SCENE_MODE_RECEIVES_UNMODIFIED_GCC_ALLOCATION");
        }
        encoder->Release();
    }
    return ok;
}
} // namespace
int main(int argc, char** argv)
{
    const bool hardware = argc > 1 && std::string_view(argv[1]) == "--nvenc";
    const bool software = argc > 1 && std::string_view(argv[1]) == "--x264";
    const bool ok = Core() && UnadjustedBudgetAndRecovery() &&
        CoupledAdjusterAndNativeDropperRecovery() && DynamicAndEligibility() &&
        Run(hardware, true, software) && Run(hardware, false, software);
    std::cout << "SCREEN_FRAME_QUALITY_COMPLETED=" << (ok ? "PASS" : "FAIL") << std::endl;
    return ok ? 0 : 1;
}
