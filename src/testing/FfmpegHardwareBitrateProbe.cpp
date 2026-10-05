// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <string_view>
#include <thread>
#include <vector>

#include "api/video/i420_buffer.h"
#include "api/video/video_frame.h"
#include "api/video_codecs/video_codec.h"
#include "modules/video_coding/include/video_error_codes.h"
#include "src/core/ScreenStreamPolicy.h"
#include "src/platform/win/FfmpegHardwareH264Encoder.h"

namespace {
using namespace std::chrono;

class Output final : public webrtc::EncodedImageCallback {
public:
    Result OnEncodedImage(const webrtc::EncodedImage& image,
                          const webrtc::CodecSpecificInfo*) override
    {
        bytes += image.size();
        ++frames;
        if (image._frameType == webrtc::VideoFrameType::kVideoFrameKey) ++keys;
        return Result(Result::OK);
    }
    void OnFrameDropped(std::uint32_t, int, bool) override { ++drops; }
    std::uint64_t bytes = 0;
    std::uint32_t frames = 0;
    std::uint32_t keys = 0;
    std::uint32_t drops = 0;
};

bool Run(remote::FfmpegHardwareBackend backend,
         std::uint32_t initialFps, bool changeFps, bool idleRecovery, bool fpsOnly,
         bool mediaBudget)
{
    Output output;
    auto runtime = std::make_shared<remote::VideoEncoderRuntimeState>(
        remote::VideoEncoderPreference::kAutomatic);
    remote::FfmpegHardwareH264Encoder encoder(
        backend, runtime);
    webrtc::VideoCodec codec{};
    codec.codecType = webrtc::kVideoCodecH264;
    codec.width = mediaBudget ? 1920 : 1280;
    codec.height = mediaBudget ? 1080 : 720;
    codec.maxFramerate = initialFps;
    codec.minBitrate = 100;
    const auto initialPolicy = remote::ResolveScreenStreamPolicy(
        codec.width, codec.height, {codec.width, codec.height, initialFps});
    codec.startBitrate = mediaBudget ? initialPolicy.maxBitrateBps / 1000 : 8000;
    codec.maxBitrate = mediaBudget ? initialPolicy.networkProbeMaxBitrateBps / 1000 : 24000;
    codec.mode = webrtc::VideoCodecMode::kScreensharing;
    encoder.RegisterEncodeCompleteCallback(&output);
    if (encoder.InitEncode(&codec, webrtc::VideoEncoder::Settings(
            webrtc::VideoEncoder::Capabilities(false), 4, 1200)) !=
        WEBRTC_VIDEO_CODEC_OK) {
        std::cout << "INIT_ERROR=" << encoder.LastError() << std::endl;
        return false;
    }
    // Fixed deterministic textured motion supplies enough complexity to test
    // rate control without capturing or uploading any user's screen content.
    std::vector<webrtc::scoped_refptr<webrtc::I420Buffer>> buffers;
    for (unsigned n = 0; n < 8; ++n) {
        auto buffer = webrtc::I420Buffer::Create(codec.width, codec.height);
        for (int y = 0; y < buffer->height(); ++y) {
            for (int x = 0; x < buffer->width(); ++x) {
                const unsigned value = (x + n * 17) * 2654435761u ^
                    (y + n * 7) * 2246822519u;
                buffer->MutableDataY()[y * buffer->StrideY() + x] =
                    static_cast<std::uint8_t>(16 + value % 220);
            }
        }
        std::memset(buffer->MutableDataU(), 100,
            buffer->StrideU() * ((buffer->height() + 1) / 2));
        std::memset(buffer->MutableDataV(), 150,
            buffer->StrideV() * ((buffer->height() + 1) / 2));
        buffers.push_back(std::move(buffer));
    }
    std::uint64_t frameId = 0;
    const auto start = steady_clock::now();
    struct Phase { std::uint32_t bitrate, fps, allocationFps; bool checkRate = true; };
    const auto mediaPhase = [&](std::uint32_t fps) {
        const auto policy = remote::ResolveScreenStreamPolicy(
            codec.width, codec.height, {codec.width, codec.height, fps});
        std::cout << "policy_fps=" << fps << ",media_cap_bps=" << policy.maxBitrateBps
                  << ",connection_cap_bps=" << policy.networkProbeMaxBitrateBps << std::endl;
        return Phase{policy.maxBitrateBps, fps, fps};
    };
    const std::vector<Phase> phases = mediaBudget
        ? std::vector<Phase>{mediaPhase(120), mediaPhase(30), mediaPhase(120)}
        : fpsOnly
        ? std::vector<Phase>{{8'000'000, 60, 60}, {8'000'000, 120, 120},
                            {8'000'000, 60, 60}, {8'000'000, 120, 120}}
        : idleRecovery
        ? std::vector<Phase>{{8'000'000, 60, 60}, {8'000'000, 1, 1, false},
                            {8'000'000, 120, 1}, {4'000'000, 120, 120}}
        : changeFps
            ? std::vector<Phase>{{8'000'000, 60, 60}, {4'000'000, 120, 120},
                                {12'000'000, 120, 120}, {8'000'000, 60, 60},
                                {4'000'000, 120, 120}}
            : std::vector<Phase>{{8'000'000, initialFps, initialFps},
                                {4'000'000, initialFps, initialFps},
                                {12'000'000, initialFps, initialFps}};
    bool rateChecksPassed = true;
    std::uint32_t checkedPhases = 0;
    // Hold each allocation long enough for the adapter's 5 s catch-up path.
    for (const auto& phase : phases) {
        const auto bitrate = phase.bitrate;
        const auto fps = phase.fps;
        webrtc::VideoBitrateAllocation allocation;
        allocation.SetBitrate(0, 0, bitrate);
        const auto ratesStart = steady_clock::now();
        encoder.SetRates(webrtc::VideoEncoder::RateControlParameters(
            allocation, static_cast<double>(phase.allocationFps)));
        const auto ratesUs = duration_cast<microseconds>(steady_clock::now() - ratesStart).count();
        const auto phaseStart = steady_clock::now();
        auto next = phaseStart;
        std::uint64_t stableBytes = 0;
        double stableSeconds = 0;
        std::int64_t maximumEncodeUs = 0;
        for (int second = 0; second < 7; ++second) {
            const auto before = output;
            const auto sampleStart = steady_clock::now();
            for (unsigned frame = 0; frame < fps; ++frame) {
                const auto timestamp = duration_cast<microseconds>(
                    steady_clock::now() - start).count() + 1'000'000;
                auto input = webrtc::VideoFrame::Builder()
                    .set_video_frame_buffer(buffers[frameId++ % buffers.size()])
                    .set_timestamp_us(timestamp)
                    .set_rtp_timestamp(static_cast<std::uint32_t>(timestamp * 9 / 100))
                    .build();
                const auto encodeStart = steady_clock::now();
                const auto encodeResult = encoder.Encode(input, nullptr);
                maximumEncodeUs = (std::max)(maximumEncodeUs,
                    duration_cast<microseconds>(steady_clock::now() - encodeStart).count());
                if (encodeResult != WEBRTC_VIDEO_CODEC_OK) {
                    std::cout << "ENCODE_ERROR=" << encoder.LastError() << std::endl;
                    encoder.Release();
                    return false;
                }
                next += microseconds(1'000'000 / fps);
                std::this_thread::sleep_until(next);
            }
            const double elapsed = duration<double>(
                steady_clock::now() - sampleStart).count();
            std::uint32_t configuredRate = 0;
            std::uint32_t configuredFps = 0;
            const auto status = runtime->Snapshot();
            for (const auto& instance : status.instances) {
                configuredRate = instance.configuredBitrateBps;
                configuredFps = instance.configuredFrameRate;
            }
            if (second >= 5) {
                stableBytes += output.bytes - before.bytes;
                stableSeconds += elapsed;
            }
            std::cout << "initial_fps=" << initialFps << ",input_fps=" << fps
                << ",requested_bps=" << bitrate << ",adapter_bps=" << configuredRate
                << ",adapter_fps=" << configuredFps
                << ",phase_second=" << second + 1
                << ",actual_bps=" << static_cast<std::uint64_t>(
                    (output.bytes - before.bytes) * 8.0 / elapsed)
                << ",output_frames=" << output.frames - before.frames
                << ",keyframes=" << output.keys - before.keys << std::endl;
        }
        const double stableRatio = stableBytes * 8.0 / stableSeconds / bitrate;
        if (phase.checkRate) {
            ++checkedPhases;
            const bool pass = stableRatio >= 0.85 && stableRatio <= 1.15;
            rateChecksPassed = rateChecksPassed && pass;
            std::cout << "RATE_ASSERT=" << (pass ? "PASS" : "FAIL")
                << ",stable_ratio=" << stableRatio << std::endl;
        }
        std::cout << "set_rates_us=" << ratesUs << ",max_encode_us=" << maximumEncodeUs
                  << ",total_drops=" << output.drops << std::endl;
    }
    encoder.Release();
    std::cout << "checked_phases=" << checkedPhases << std::endl;
    return rateChecksPassed && output.drops == 0;
}
}  // namespace

int main(int argc, char** argv)
{
    auto backend = remote::FfmpegHardwareBackend::kNvenc;
    bool changeFps = false;
    bool idleRecovery = false;
    bool fpsOnly = false;
    bool mediaBudget = false;
    for (int index = 1; index < argc; ++index) {
        const std::string_view argument(argv[index]);
        if (argument == "fps-change") changeFps = true;
        else if (argument == "idle-recovery") idleRecovery = true;
        else if (argument == "fps-only") fpsOnly = true;
        else if (argument == "media-budget") mediaBudget = true;
        else if (argument == "qsv") backend = remote::FfmpegHardwareBackend::kQsv;
        else if (argument == "amf") backend = remote::FfmpegHardwareBackend::kAmf;
        else if (argument == "nvenc") backend = remote::FfmpegHardwareBackend::kNvenc;
        else if (argument == "list") {
            for (const auto& candidate :
                 remote::FfmpegHardwareH264Encoder::EnumerateAvailability()) {
                std::cout << candidate.implementation
                    << ",compiled=" << candidate.compiled
                    << ",driver_runtime=" << candidate.driverRuntimePresent
                    << ",detail=" << candidate.detail << std::endl;
            }
            return 0;
        } else {
            std::cerr << "Usage: FfmpegHardwareBitrateProbe [nvenc|qsv|amf] "
                         "[fps-change|idle-recovery|fps-only|media-budget] | list" << std::endl;
            return 2;
        }
    }
    // Optional second run investigates the encoder's configured FPS boundary.
    const bool result = Run(backend, changeFps || idleRecovery || fpsOnly ? 60 : 120,
                            changeFps, idleRecovery, fpsOnly, mediaBudget);
    std::cout << "BITRATE_PROBE_COMPLETED=" << (result ? "PASS" : "FAIL") << std::endl;
    return result ? 0 : 1;
}
