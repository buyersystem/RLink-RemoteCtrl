// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include "api/environment/environment_factory.h"
#include "api/video/i420_buffer.h"
#include "api/video_codecs/video_codec.h"
#include "api/video_codecs/video_encoder.h"
#include "modules/video_coding/include/video_error_codes.h"
#include "src/webrtc/WindowsPreferredVideoEncoderFactory.h"
#include "src/webrtc/VideoCodecTimingTelemetry.h"
#include "src/platform/win/FfmpegHardwareH264EncoderFactory.h"

// Exercise the exact collector window implementation without exposing a
// mutable production stats API. Link this test with the transport library;
// its separate collector object is not needed by this executable.
#include "src/webrtc/PeerConnectionStatsCollector.cpp"

namespace {
bool Check(bool valid, const char* name)
{
    std::cout << name << '=' << (valid ? "PASS" : "FAIL") << '\n';
    return valid;
}

// Header fixtures from WebRTC's BSD-licensed H264BitstreamParser unit test.
// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
// Source: common_video/h264/h264_bitstream_parser_unittest.cc.
// The upstream LICENSE and PATENTS supplied with our WebRTC dependency apply.
const std::vector<std::uint8_t> kKeyFrame = {
    0, 0, 0, 1, 0x67, 0x42, 0x80, 0x20, 0xda, 0x01, 0x40, 0x16,
    0xe8, 0x06, 0xd0, 0xa1, 0x35, 0, 0, 0, 1, 0x68, 0xce, 0x06,
    0xe2, 0, 0, 0, 1, 0x65, 0xb8, 0x40, 0xf0, 0x8c, 0x03, 0xf2,
    0x75, 0x67, 0xad, 0x41, 0x64, 0x24, 0x0e, 0xa0, 0xb2, 0x12, 0x1e, 0xf8
};
const std::vector<std::uint8_t> kDeltaFrame = {
    0, 0, 0, 1, 0x41, 0xe2, 0x01, 0x16, 0x0e, 0x3e, 0x2b, 0x86
};

struct Fixture {
    std::vector<std::uint8_t> bytes = kKeyFrame;
    int qp = -1;
    const std::uint8_t* inputBuffer = nullptr;
};

class HeaderEncoder final : public webrtc::VideoEncoder {
public:
    explicit HeaderEncoder(std::shared_ptr<Fixture> fixture)
        : fixture_(std::move(fixture)) {}
    void SetFecControllerOverride(webrtc::FecControllerOverride*) override {}
    int InitEncode(const webrtc::VideoCodec*, const Settings&) override
    { return WEBRTC_VIDEO_CODEC_OK; }
    int32_t RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* callback) override
    { callback_ = callback; return WEBRTC_VIDEO_CODEC_OK; }
    int32_t Release() override { callback_ = nullptr; return WEBRTC_VIDEO_CODEC_OK; }
    int32_t Encode(const webrtc::VideoFrame& frame,
                   const std::vector<webrtc::VideoFrameType>*) override
    {
        if (!callback_) return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
        webrtc::EncodedImage image;
        image.SetEncodedData(webrtc::EncodedImageBuffer::Create(
            fixture_->bytes.data(), fixture_->bytes.size()));
        image.SetRtpTimestamp(frame.rtp_timestamp());
        image._encodedWidth = 640;
        image._encodedHeight = 360;
        image.qp_ = fixture_->qp;
        fixture_->inputBuffer = image.data();
        callback_->OnEncodedImage(image, nullptr);
        return WEBRTC_VIDEO_CODEC_OK;
    }
    void SetRates(const RateControlParameters&) override {}
    EncoderInfo GetEncoderInfo() const override
    {
        EncoderInfo info;
        info.implementation_name = "HeaderFixture";
        return info;
    }
private:
    std::shared_ptr<Fixture> fixture_;
    webrtc::EncodedImageCallback* callback_ = nullptr;
};

class HeaderFactory final : public webrtc::VideoEncoderFactory {
public:
    explicit HeaderFactory(std::shared_ptr<Fixture> fixture)
        : fixture_(std::move(fixture)) {}
    std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override
    { return {webrtc::SdpVideoFormat("H264")}; }
    std::unique_ptr<webrtc::VideoEncoder> Create(
        const webrtc::Environment&, const webrtc::SdpVideoFormat&) override
    { return std::make_unique<HeaderEncoder>(fixture_); }
private:
    std::shared_ptr<Fixture> fixture_;
};

class Output final : public webrtc::EncodedImageCallback {
public:
    Result OnEncodedImage(const webrtc::EncodedImage& image,
                          const webrtc::CodecSpecificInfo*) override
    {
        qp = image.qp_;
        data = image.data();
        return Result(Result::OK);
    }
    void OnFrameDropped(std::uint32_t, int, bool) override {}
    int qp = -1;
    const std::uint8_t* data = nullptr;
};

bool EncoderChecks()
{
    bool ok = true;
    auto fixture = std::make_shared<Fixture>();
    remote::WindowsPreferredVideoEncoderFactory factory(
        std::make_unique<HeaderFactory>(fixture), nullptr);
    auto encoder = factory.Create(webrtc::CreateEnvironment(), webrtc::SdpVideoFormat("H264"));
    webrtc::VideoCodec codec{};
    codec.codecType = webrtc::kVideoCodecH264;
    codec.width = 640;
    codec.height = 360;
    codec.maxFramerate = 60;
    codec.mode = webrtc::VideoCodecMode::kScreensharing;
    const webrtc::VideoEncoder::Settings settings(
        webrtc::VideoEncoder::Capabilities(false), 1, 1200);
    Output output;
    ok &= Check(encoder && encoder->InitEncode(&codec, settings) == WEBRTC_VIDEO_CODEC_OK,
                "ENCODER_INITIALIZED");
    if (!encoder) return false;
    encoder->RegisterEncodeCompleteCallback(&output);
    auto frame = webrtc::VideoFrame::Builder()
        .set_video_frame_buffer(webrtc::I420Buffer::Create(640, 360))
        .set_rtp_timestamp(9000).build();
    std::uint32_t timestamp = 9000;
    const auto encode = [&] {
        frame.set_rtp_timestamp(++timestamp);
        return encoder->Encode(frame, nullptr) == WEBRTC_VIDEO_CODEC_OK;
    };
    ok &= Check(encode() && output.qp == 35, "H264_QP_FROM_KEYFRAME_HEADERS");
    ok &= Check(output.data == fixture->inputBuffer, "BITSTREAM_BUFFER_IS_SHARED");
    auto timing = remote::VideoCodecTimingRegistry::Instance()
        .SnapshotForImplementation(encoder->GetEncoderInfo().implementation_name);
    ok &= Check(timing && timing->qpAvailable && timing->qp == 35,
                "PARSED_QP_REACHES_PER_ENCODER_TELEMETRY");
    fixture->bytes = kDeltaFrame;
    ok &= Check(encode() && output.qp == 37, "SPS_PPS_RETAINED_FOR_DELTA_FRAME");
    fixture->bytes = {0, 0, 0, 1, 0x09, 0xf0};
    ok &= Check(encode() && output.qp == -1, "HEADER_ONLY_PACKET_HAS_NO_STALE_QP");
    fixture->bytes = {0, 0, 0, 1, 0x41};
    ok &= Check(encode() && output.qp == -1, "TRUNCATED_SLICE_HAS_NO_STALE_QP");
    fixture->bytes = kKeyFrame;
    fixture->qp = 19;
    ok &= Check(encode() && output.qp == 19, "NATIVE_QP_PRESERVED");
    fixture->bytes = kDeltaFrame;
    fixture->qp = -1;
    ok &= Check(encode() && output.qp == 37, "NATIVE_QP_FRAMES_STILL_UPDATE_PARAMETER_SETS");
    fixture->bytes.assign(5000, 0xab);
    fixture->bytes[0] = fixture->bytes[1] = fixture->bytes[2] = 0;
    fixture->bytes[3] = 1;
    fixture->bytes[4] = 0x67;
    ok &= Check(encode() && output.qp == -1,
                "OVERSIZED_PARAMETER_SET_IS_UNAVAILABLE");
    fixture->bytes = kDeltaFrame;
    ok &= Check(encode() && output.qp == -1,
                "OVERSIZED_PARAMETER_SET_INVALIDATES_OLD_STATE");
    fixture->bytes = kKeyFrame;
    fixture->bytes.resize(2 * 1024 * 1024, 0xab);
    ok &= Check(encode() && output.qp == 35 && output.data == fixture->inputBuffer,
                "LARGE_PAYLOAD_PARSES_BOUNDED_HEADER_WITH_SHARED_BUFFER");
    encoder->Release();
    encoder->InitEncode(&codec, settings);
    encoder->RegisterEncodeCompleteCallback(&output);
    fixture->bytes = kDeltaFrame;
    ok &= Check(encode() && output.qp == -1, "REINITIALIZATION_DISCARDS_OLD_PARAMETER_SETS");
    codec.codecType = webrtc::kVideoCodecVP8;
    encoder->Release();
    encoder->InitEncode(&codec, settings);
    encoder->RegisterEncodeCompleteCallback(&output);
    fixture->bytes = kKeyFrame;
    ok &= Check(encode() && output.qp == -1, "NON_H264_IS_NOT_PARSED_AS_H264");
    encoder->Release();
    return ok;
}

bool WindowChecks()
{
    bool ok = true;
    std::unordered_map<std::string, remote::AggregateSample> history;
    const auto window = [&](std::optional<double> total,
                            std::optional<std::uint64_t> count,
                            std::int64_t timestamp) {
        return remote::CalculateOptionalWindowAverage(history, "encode:test",
                                                       total, count, timestamp, 1000.0);
    };
    ok &= Check(!window(1.0, 100, 1000000).available, "FIRST_WINDOW_IS_BASELINE");
    const auto measured = window(1.05, 110, 2000000);
    ok &= Check(measured.available && std::abs(measured.value - 5.0) < 1e-9,
                "TRUE_COUNTER_DELTA_MEASURES_ENCODE_TIME");
    ok &= Check(!window(std::nullopt, 120, 3000000).available && history.empty(),
                "MISSING_TOTAL_BREAKS_COUNTER_CONTINUITY");
    ok &= Check(!window(1.15, 130, 4000000).available,
                "FIRST_SAMPLE_AFTER_MISSING_TOTAL_IS_BASELINE");
    ok &= Check(!window(1.20, std::nullopt, 5000000).available && history.empty(),
                "MISSING_FRAME_COUNT_BREAKS_COUNTER_CONTINUITY");
    ok &= Check(!window(std::numeric_limits<double>::quiet_NaN(), 140, 6000000).available &&
                    history.empty(), "NAN_TOTAL_IS_UNAVAILABLE");
    ok &= Check(!window(std::numeric_limits<double>::infinity(), 140, 6000000).available &&
                    history.empty(), "INFINITE_TOTAL_IS_UNAVAILABLE");
    ok &= Check(!window(-1.0, 140, 6000000).available && history.empty(),
                "NEGATIVE_TOTAL_IS_UNAVAILABLE");
    window(0.0, 0, 7000000);
    const auto trueZero = window(0.0, 10, 8000000);
    ok &= Check(trueZero.available && trueZero.value == 0.0,
                "REPORTED_ZERO_TOTAL_IS_VALID");
    ok &= Check(!window(0.0, 10, 9000000).available,
                "NO_NEW_FRAMES_IS_UNAVAILABLE");
    ok &= Check(!window(0.0, 0, 10000000).available,
                "COUNTER_RESET_IS_A_NEW_BASELINE");
    return ok;
}

bool NvencChecks()
{
    class HardwareOutput final : public webrtc::EncodedImageCallback {
    public:
        Result OnEncodedImage(const webrtc::EncodedImage& image,
                              const webrtc::CodecSpecificInfo*) override
        {
            ++frames;
            if (image.qp_ >= 0 && image.qp_ <= 51) ++qpFrames;
            return Result(Result::OK);
        }
        void OnFrameDropped(std::uint32_t, int, bool) override {}
        std::uint32_t frames = 0, qpFrames = 0;
    } output;
    remote::WindowsPreferredVideoEncoderFactory factory(nullptr,
        std::make_unique<remote::FfmpegHardwareH264EncoderFactory>(
            remote::FfmpegHardwareBackend::kNvenc));
    const auto formats = factory.GetSupportedFormats();
    if (formats.empty()) return Check(false, "NVENC_H264_FORMAT_AVAILABLE");
    auto encoder = factory.Create(webrtc::CreateEnvironment(), formats.front());
    webrtc::VideoCodec codec{};
    codec.codecType = webrtc::kVideoCodecH264;
    codec.width = 1280;
    codec.height = 720;
    codec.maxFramerate = 30;
    codec.minBitrate = 300;
    codec.startBitrate = 2500;
    codec.maxBitrate = 8000;
    codec.mode = webrtc::VideoCodecMode::kScreensharing;
    const webrtc::VideoEncoder::Settings settings(
        webrtc::VideoEncoder::Capabilities(false), 4, 1200);
    if (!encoder || encoder->InitEncode(&codec, settings) != WEBRTC_VIDEO_CODEC_OK)
        return Check(false, "NVENC_ENCODER_INITIALIZED");
    encoder->RegisterEncodeCompleteCallback(&output);
    webrtc::VideoBitrateAllocation allocation;
    allocation.SetBitrate(0, 0, 2500000);
    encoder->SetRates(webrtc::VideoEncoder::RateControlParameters(allocation, 30.0));
    auto buffer = webrtc::I420Buffer::Create(1280, 720);
    bool ok = true;
    for (int i = 0; i < 10; ++i) {
        std::memset(buffer->MutableDataY(), 32 + i * 16,
                    buffer->StrideY() * buffer->height());
        std::memset(buffer->MutableDataU(), 100,
                    buffer->StrideU() * ((buffer->height() + 1) / 2));
        std::memset(buffer->MutableDataV(), 150,
                    buffer->StrideV() * ((buffer->height() + 1) / 2));
        auto frame = webrtc::VideoFrame::Builder().set_video_frame_buffer(buffer)
            .set_timestamp_us(1000000 + i * 33333)
            .set_rtp_timestamp(90000 + i * 3000).build();
        const std::vector<webrtc::VideoFrameType> types{
            i == 0 ? webrtc::VideoFrameType::kVideoFrameKey
                   : webrtc::VideoFrameType::kVideoFrameDelta};
        ok &= encoder->Encode(frame, &types) == WEBRTC_VIDEO_CODEC_OK;
    }
    ok &= Check(output.frames >= 3 && output.qpFrames == output.frames,
                "REAL_NVENC_OUTPUT_HAS_PARSED_QP");
    const auto timing = remote::VideoCodecTimingRegistry::Instance()
        .SnapshotForImplementation(encoder->GetEncoderInfo().implementation_name);
    ok &= Check(timing && timing->qpAvailable && timing->qp >= 0 && timing->qp <= 51,
                "REAL_NVENC_QP_REACHES_TELEMETRY");
    std::cout << "nvenc_frames=" << output.frames << ",qp_frames=" << output.qpFrames
              << ",latest_qp=" << (timing ? timing->qp : -1) << '\n';
    encoder->Release();
    return ok;
}
}  // namespace

int main(int argc, char** argv)
{
    const bool encoder = EncoderChecks();
    const bool counters = WindowChecks();
    const bool hardware = argc < 2 || std::string_view(argv[1]) != "--nvenc" || NvencChecks();
    return encoder && counters && hardware ? 0 : 1;
}
