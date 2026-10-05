// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "WindowsPreferredVideoEncoderFactory.h"
#include "GoogCcTelemetry.h"
#include "src/core/ScreenFrameQualityPolicy.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>

#include "api/environment/environment_factory.h"
#include "api/video_codecs/video_encoder_software_fallback_wrapper.h"
#include "common_video/h264/h264_bitstream_parser.h"
#include "common_video/h264/h264_common.h"
#include "modules/video_coding/include/video_error_codes.h"
#include "src/core/ScreenStreamPolicy.h"
#include "VideoCodecTimingTelemetry.h"

namespace remote {
namespace {

// libwebrtc's generic software-fallback wrapper may be configured through
// WebRTC-Video-EncoderFallbackSettings to force software below a pixel-count
// threshold. That policy is useful for some camera/mobile products, but it is
// wrong for a Windows remote desktop: changing 1080p -> 720p must not replace
// a healthy NVENC/QSV/AMF encoder with libx264. Preserve every other field
// trial and disable only the resolution-based forced fallback for the wrapper
// we construct here. Real InitEncode/Encode failures still use the normal
// software fallback path.
class HardwareFailureOnlyFieldTrials final
    : public webrtc::FieldTrialsView {
public:
    explicit HardwareFailureOnlyFieldTrials(
        const webrtc::FieldTrialsView& base)
        : base_(base)
    {}

    std::string Lookup(absl::string_view key) const override
    {
        if (key == "WebRTC-Video-EncoderFallbackSettings") {
            return {};
        }
        return base_.Lookup(key);
    }

private:
    const webrtc::FieldTrialsView& base_;
};

// Resolution changes make VideoStreamEncoder release and immediately
// initialize the same hardware encoder wrapper with the new dimensions.
// Some Windows hardware drivers finish destroying the previous session a few
// milliseconds after Release(), so the first NVENC/QSV/AMF initialization may
// fail even though an immediate retry succeeds. libwebrtc's stock fallback
// wrapper permanently selects software after that first failure. Absorb only
// this short transient here; repeated failures are still returned to the
// stock wrapper and therefore retain the normal software safety net.
class TransientFailureRetryVideoEncoder final
    : public webrtc::VideoEncoder {
public:
    explicit TransientFailureRetryVideoEncoder(
        std::unique_ptr<webrtc::VideoEncoder> encoder)
        : encoder_(std::move(encoder))
    {}

    void SetFecControllerOverride(
        webrtc::FecControllerOverride* fecControllerOverride) override
    {
        fecControllerOverride_ = fecControllerOverride;
        encoder_->SetFecControllerOverride(fecControllerOverride);
    }

    int InitEncode(
        const webrtc::VideoCodec* codecSettings,
        const Settings& settings) override
    {
        if (!codecSettings) {
            return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
        }
        codecSettings_ = *codecSettings;
        settings_ = settings;
        rates_.reset();
        initialized_ = false;
        const int result = InitializeWithRetry(/*attempts=*/3);
        initialized_ = result == WEBRTC_VIDEO_CODEC_OK;
        return result;
    }

    int32_t RegisterEncodeCompleteCallback(
        webrtc::EncodedImageCallback* callback) override
    {
        callback_ = callback;
        return encoder_->RegisterEncodeCompleteCallback(callback);
    }

    int32_t Release() override
    {
        initialized_ = false;
        callback_ = nullptr;
        rates_.reset();
        return encoder_->Release();
    }

    int32_t Encode(
        const webrtc::VideoFrame& frame,
        const std::vector<webrtc::VideoFrameType>* frameTypes) override
    {
        int32_t result = encoder_->Encode(frame, frameTypes);
        if (result != WEBRTC_VIDEO_CODEC_FALLBACK_SOFTWARE ||
            !initialized_ || !codecSettings_ || !settings_) {
            return result;
        }

        // A resolution reconfiguration can also expose the delayed driver
        // teardown on the first submitted frame. Recreate the hardware path
        // once before allowing the outer wrapper to switch to software.
        encoder_->Release();
        initialized_ = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        result = InitializeWithRetry(/*attempts=*/2);
        if (result != WEBRTC_VIDEO_CODEC_OK) {
            return result;
        }
        initialized_ = true;
        PrimeEncoder();
        return encoder_->Encode(frame, frameTypes);
    }

    void OnPacketLossRateUpdate(float packetLossRate) override
    {
        packetLossRate_ = packetLossRate;
        encoder_->OnPacketLossRateUpdate(packetLossRate);
    }

    void OnRttUpdate(int64_t rttMs) override
    {
        rttMs_ = rttMs;
        encoder_->OnRttUpdate(rttMs);
    }

    void OnLossNotification(
        const LossNotification& lossNotification) override
    {
        lossNotification_ = lossNotification;
        encoder_->OnLossNotification(lossNotification);
    }

    void SetRates(const RateControlParameters& parameters) override
    {
        rates_ = parameters;
        encoder_->SetRates(parameters);
    }

    EncoderInfo GetEncoderInfo() const override
    {
        return encoder_->GetEncoderInfo();
    }

private:
    int InitializeWithRetry(int attempts)
    {
        int result = WEBRTC_VIDEO_CODEC_ERROR;
        for (int attempt = 0; attempt < attempts; ++attempt) {
            if (attempt > 0) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(20 * attempt));
            }
            result = encoder_->InitEncode(
                &*codecSettings_, *settings_);
            if (result == WEBRTC_VIDEO_CODEC_OK ||
                result ==
                    WEBRTC_VIDEO_CODEC_ERR_SIMULCAST_PARAMETERS_NOT_SUPPORTED) {
                return result;
            }
            encoder_->Release();
        }
        return result;
    }

    void PrimeEncoder()
    {
        encoder_->SetFecControllerOverride(fecControllerOverride_);
        if (callback_) {
            encoder_->RegisterEncodeCompleteCallback(callback_);
        }
        if (rates_) {
            encoder_->SetRates(*rates_);
        }
        if (rttMs_) {
            encoder_->OnRttUpdate(*rttMs_);
        }
        if (packetLossRate_) {
            encoder_->OnPacketLossRateUpdate(*packetLossRate_);
        }
        if (lossNotification_) {
            encoder_->OnLossNotification(*lossNotification_);
        }
    }

    std::unique_ptr<webrtc::VideoEncoder> encoder_;
    webrtc::FecControllerOverride* fecControllerOverride_ = nullptr;
    webrtc::EncodedImageCallback* callback_ = nullptr;
    bool initialized_ = false;
    std::optional<webrtc::VideoCodec> codecSettings_;
    std::optional<Settings> settings_;
    std::optional<RateControlParameters> rates_;
    std::optional<float> packetLossRate_;
    std::optional<int64_t> rttMs_;
    std::optional<LossNotification> lossNotification_;
};

class RuntimeTrackedSoftwareEncoder final
    : public webrtc::VideoEncoder,
      public webrtc::EncodedImageCallback {
public:
    RuntimeTrackedSoftwareEncoder(
        std::unique_ptr<webrtc::VideoEncoder> encoder,
        std::shared_ptr<VideoEncoderRuntimeState> runtimeState)
        : encoder_(std::move(encoder))
        , runtimeState_(std::move(runtimeState))
    {}

    ~RuntimeTrackedSoftwareEncoder() override
    {
        Unregister();
    }

    int InitEncode(
        const webrtc::VideoCodec* codecSettings,
        const Settings& settings) override
    {
        Unregister();
        {
            std::lock_guard lock(frameStatsMutex_);
            inputRateWindowStart_ = {};
            outputRateWindowStart_ = {};
            inputRateWindowFrames_ = 0;
            outputRateWindowFrames_ = 0;
            observedInputFrameRate_ = 0;
            observedOutputFrameRate_ = 0;
            totalInputFrames_ = 0;
            totalOutputFrames_ = 0;
            totalDroppedFrames_ = 0;
        }
        const int result =
            encoder_->InitEncode(codecSettings, settings);
        if (result == WEBRTC_VIDEO_CODEC_OK &&
            runtimeState_ && codecSettings) {
            const auto info = encoder_->GetEncoderInfo();
            std::uint32_t effectiveStartBitrateBps =
                codecSettings->startBitrate * 1000;
            if (codecSettings->mode ==
                    webrtc::VideoCodecMode::kScreensharing &&
                info.implementation_name.starts_with(
                    "FFmpeg/libx264")) {
                const auto screenPolicy = ResolveScreenStreamPolicy(
                    codecSettings->width,
                    codecSettings->height,
                    {codecSettings->width,
                     codecSettings->height,
                     (std::max<std::uint32_t>)(
                         codecSettings->maxFramerate, 1)});
                effectiveStartBitrateBps = (std::min)(
                    (std::max)(effectiveStartBitrateBps,
                               screenPolicy.startBitrateBps),
                    codecSettings->maxBitrate * 1000);
            }
            runtimeInstanceId_ =
                runtimeState_->RegisterSoftwareEncoder(
                    info.implementation_name);
            runtimeState_->MarkSoftwareInitialized(
                runtimeInstanceId_,
                info.implementation_name,
                codecSettings->width,
                codecSettings->height,
                (std::max<std::uint32_t>)(
                    codecSettings->maxFramerate, 1),
                codecSettings->minBitrate * 1000,
                effectiveStartBitrateBps,
                codecSettings->maxBitrate * 1000);
        }
        return result;
    }

    int32_t RegisterEncodeCompleteCallback(
        webrtc::EncodedImageCallback* callback) override
    {
        {
            std::lock_guard lock(frameStatsMutex_);
            callback_ = callback;
        }
        return encoder_->RegisterEncodeCompleteCallback(
            callback ? this : nullptr);
    }

    int32_t Release() override
    {
        const int32_t result = encoder_->Release();
        {
            std::lock_guard lock(frameStatsMutex_);
            callback_ = nullptr;
        }
        Unregister();
        return result;
    }

    int32_t Encode(
        const webrtc::VideoFrame& frame,
        const std::vector<webrtc::VideoFrameType>* frameTypes) override
    {
        ReportInputFormat(frame.video_frame_buffer());
        RecordInputFrame();
        const int32_t result = encoder_->Encode(frame, frameTypes);
        if (result != WEBRTC_VIDEO_CODEC_OK) {
            RecordDroppedFrame();
        }
        return result;
    }

    void SetRates(
        const RateControlParameters& parameters) override
    {
        encoder_->SetRates(parameters);
        if (runtimeState_ && runtimeInstanceId_ != 0) {
            runtimeState_->MarkRates(
                runtimeInstanceId_,
                parameters.framerate_fps > 0.0
                    ? static_cast<std::uint32_t>(
                          parameters.framerate_fps)
                    : 0,
                parameters.bitrate.get_sum_bps());
        }
    }

    EncoderInfo GetEncoderInfo() const override
    {
        return encoder_->GetEncoderInfo();
    }

    Result OnEncodedImage(
        const webrtc::EncodedImage& encodedImage,
        const webrtc::CodecSpecificInfo* codecSpecificInfo) override
    {
        RecordOutputFrame();
        webrtc::EncodedImageCallback* callback = nullptr;
        {
            std::lock_guard lock(frameStatsMutex_);
            callback = callback_;
        }
        return callback
                   ? callback->OnEncodedImage(
                         encodedImage, codecSpecificInfo)
                   : Result(Result::ERROR_SEND_FAILED);
    }

    void OnFrameDropped(
        uint32_t rtpTimestamp,
        int spatialId,
        bool isEndOfTemporalUnit) override
    {
        RecordDroppedFrame();
        webrtc::EncodedImageCallback* callback = nullptr;
        {
            std::lock_guard lock(frameStatsMutex_);
            callback = callback_;
        }
        if (callback) {
            callback->OnFrameDropped(
                rtpTimestamp, spatialId, isEndOfTemporalUnit);
        }
    }

private:
    void PublishFramePipeline()
    {
        if (!runtimeState_ || runtimeInstanceId_ == 0) {
            return;
        }
        std::uint32_t inputRate = 0;
        std::uint32_t outputRate = 0;
        std::uint64_t inputFrames = 0;
        std::uint64_t outputFrames = 0;
        std::uint64_t droppedFrames = 0;
        {
            std::lock_guard lock(frameStatsMutex_);
            inputRate = observedInputFrameRate_;
            outputRate = observedOutputFrameRate_;
            inputFrames = totalInputFrames_;
            outputFrames = totalOutputFrames_;
            droppedFrames = totalDroppedFrames_;
        }
        runtimeState_->MarkFramePipeline(
            runtimeInstanceId_, inputRate, outputRate,
            inputFrames, outputFrames, droppedFrames);
    }

    void RecordInputFrame()
    {
        bool publish = false;
        {
            std::lock_guard lock(frameStatsMutex_);
            const auto now = std::chrono::steady_clock::now();
            if (inputRateWindowFrames_ == 0) {
                inputRateWindowStart_ = now;
            }
            ++inputRateWindowFrames_;
            ++totalInputFrames_;
            const auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - inputRateWindowStart_);
            if (elapsed >= std::chrono::milliseconds(1000)) {
                observedInputFrameRate_ =
                    static_cast<std::uint32_t>((std::max<long>)(
                        1L, std::lround(
                            static_cast<double>(
                                inputRateWindowFrames_) *
                            1000.0 /
                            static_cast<double>((std::max<std::int64_t>)(
                                elapsed.count(), 1)))));
                inputRateWindowFrames_ = 0;
                inputRateWindowStart_ = now;
                publish = true;
            }
        }
        if (publish) {
            PublishFramePipeline();
        }
    }

    void RecordOutputFrame()
    {
        bool publish = false;
        {
            std::lock_guard lock(frameStatsMutex_);
            const auto now = std::chrono::steady_clock::now();
            if (outputRateWindowFrames_ == 0) {
                outputRateWindowStart_ = now;
            }
            ++outputRateWindowFrames_;
            ++totalOutputFrames_;
            const auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - outputRateWindowStart_);
            if (elapsed >= std::chrono::milliseconds(1000)) {
                observedOutputFrameRate_ =
                    static_cast<std::uint32_t>((std::max<long>)(
                        1L, std::lround(
                            static_cast<double>(
                                outputRateWindowFrames_) *
                            1000.0 /
                            static_cast<double>((std::max<std::int64_t>)(
                                elapsed.count(), 1)))));
                outputRateWindowFrames_ = 0;
                outputRateWindowStart_ = now;
                publish = true;
            }
        }
        if (publish) {
            PublishFramePipeline();
        }
    }

    void RecordDroppedFrame()
    {
        {
            std::lock_guard lock(frameStatsMutex_);
            ++totalDroppedFrames_;
        }
        PublishFramePipeline();
    }

    void Unregister()
    {
        if (runtimeState_ && runtimeInstanceId_ != 0) {
            runtimeState_->UnregisterEncoder(runtimeInstanceId_);
            runtimeInstanceId_ = 0;
        }
    }

    void ReportInputFormat(
        const webrtc::scoped_refptr<webrtc::VideoFrameBuffer>& buffer)
    {
        if (!runtimeState_ || runtimeInstanceId_ == 0 || !buffer) {
            return;
        }
        std::string inputFormat;
        if (buffer->type() ==
                webrtc::VideoFrameBuffer::Type::kNative &&
            buffer->storage_representation() ==
                "D3D11_BGRA_DESKTOP") {
            inputFormat = "D3D11 BGRA desktop texture";
        } else if (buffer->type() ==
                webrtc::VideoFrameBuffer::Type::kNative &&
            buffer->storage_representation() ==
                "CPU_BGRA_DESKTOP") {
            inputFormat = "CPU BGRA desktop";
        } else if (
            buffer->type() ==
            webrtc::VideoFrameBuffer::Type::kI420) {
            inputFormat = "CPU I420";
        } else if (
            buffer->type() ==
            webrtc::VideoFrameBuffer::Type::kNV12) {
            inputFormat = "CPU NV12";
        } else {
            inputFormat = buffer->storage_representation();
        }
        if (inputFormat != lastInputFormat_) {
            lastInputFormat_ = inputFormat;
            runtimeState_->MarkInputFormat(
                runtimeInstanceId_, std::move(inputFormat));
        }
    }

    std::unique_ptr<webrtc::VideoEncoder> encoder_;
    std::shared_ptr<VideoEncoderRuntimeState> runtimeState_;
    std::uint64_t runtimeInstanceId_ = 0;
    std::string lastInputFormat_;
    std::mutex frameStatsMutex_;
    webrtc::EncodedImageCallback* callback_ = nullptr;
    std::chrono::steady_clock::time_point inputRateWindowStart_{};
    std::chrono::steady_clock::time_point outputRateWindowStart_{};
    std::uint32_t inputRateWindowFrames_ = 0;
    std::uint32_t outputRateWindowFrames_ = 0;
    std::uint32_t observedInputFrameRate_ = 0;
    std::uint32_t observedOutputFrameRate_ = 0;
    std::uint64_t totalInputFrames_ = 0;
    std::uint64_t totalOutputFrames_ = 0;
    std::uint64_t totalDroppedFrames_ = 0;
};

// Only headers are needed for QP. The upstream parser unescapes its whole NAL,
// so cap each supplied prefix instead of copying an encoded desktop frame.
// SPS/PPS survive across frames, while a missing slice must never reuse old QP.
class HeaderOnlyH264QpParser final : public webrtc::H264BitstreamParser {
public:
    std::optional<int> ParseFrame(std::span<const std::uint8_t> bitstream,
                                bool needsQp)
    {
        last_slice_qp_delta_.reset();
        constexpr std::size_t kMaximumHeaderBytes = 4096;
        for (const auto& index : webrtc::H264::FindNaluIndices(bitstream)) {
            auto nalu = bitstream.subspan(index.payload_start_offset,
                                         index.payload_size);
            if (nalu.empty()) continue;
            const auto type = webrtc::H264::ParseNaluType(nalu.front());
            if (type == webrtc::H264::kSps || type == webrtc::H264::kPps) {
                // An oversized parameter set is not a trusted substitute for
                // the previous one. Invalidate it and wait for valid headers.
                if (nalu.size() > kMaximumHeaderBytes) {
                    if (type == webrtc::H264::kSps) sps_.reset();
                    else pps_.reset();
                    continue;
                }
                ParseSlice(nalu);
            } else if (needsQp &&
                       (type == webrtc::H264::kSlice ||
                        type == webrtc::H264::kIdr)) {
                ParseSlice(nalu.first((std::min)(nalu.size(), kMaximumHeaderBytes)));
            }
        }
        return needsQp ? GetLastSliceQp() : std::nullopt;
    }
};

class FrameTimedVideoEncoder final
    : public webrtc::VideoEncoder,
      public webrtc::EncodedImageCallback {
public:
    explicit FrameTimedVideoEncoder(
        std::unique_ptr<webrtc::VideoEncoder> encoder,
        std::shared_ptr<GoogCcTelemetryState> qualityControl,
        bool nativeFrameDropperAvailable)
        : encoder_(std::move(encoder)),
          qualityControl_(std::move(qualityControl)),
          nativeFrameDropperAvailable_(nativeFrameDropperAvailable),
          timingInstanceId_(
              VideoCodecTimingRegistry::Instance().Register(
                  VideoCodecTimingDirection::kEncode))
    {}

    ~FrameTimedVideoEncoder() override
    {
        VideoCodecTimingRegistry::Instance().Unregister(
            timingInstanceId_);
    }

    void SetFecControllerOverride(
        webrtc::FecControllerOverride* fecControllerOverride) override
    {
        encoder_->SetFecControllerOverride(fecControllerOverride);
    }

    int InitEncode(
        const webrtc::VideoCodec* codecSettings,
        const Settings& settings) override
    {
        ClearPending();
        {
            std::lock_guard lock(mutex_);
            rawRates_.reset();
            rawRatesDirty_ = false;
            appliedReferenceBps_ = 0;
            appliedDeficitShareHundredths_ = 0;
            publishedQualityDecision_.reset();
            qualityEligible_ = codecSettings && nativeFrameDropperAvailable_ &&
                codecSettings->codecType == webrtc::kVideoCodecH264 &&
                codecSettings->mode == webrtc::VideoCodecMode::kScreensharing &&
                codecSettings->numberOfSimulcastStreams <= 1 &&
                codecSettings->H264().numberOfTemporalLayers <= 1;
            nominalFrameRate_ = codecSettings ? codecSettings->maxFramerate : 0;
            qualityCapBps_ = codecSettings ? static_cast<std::uint32_t>(
                (std::min<std::uint64_t>)(codecSettings->maxBitrate * 1000ull,
                    std::numeric_limits<std::uint32_t>::max())) : 0;
            qualityStartBps_ = codecSettings ? static_cast<std::uint32_t>(
                (std::min<std::uint64_t>)(codecSettings->startBitrate * 1000ull,
                    qualityCapBps_)) : 0;
            qualityPolicy_.Reset(nominalFrameRate_, qualityCapBps_, qualityStartBps_);
            qualityEnabled_ = false;
            codecSettings_ = codecSettings ? std::make_optional(*codecSettings) : std::nullopt;
            encoderSettings_ = settings;
            reconfigureError_ = WEBRTC_VIDEO_CODEC_OK;
            restoreUserCodec_ = false;
            forceKeyFrame_ = false;
            qpParser_ = codecSettings &&
                                codecSettings->codecType == webrtc::kVideoCodecH264
                            ? std::make_unique<HeaderOnlyH264QpParser>()
                            : nullptr;
        }
        lowLatencyScreenshare_.store(
            codecSettings &&
                codecSettings->mode ==
                    webrtc::VideoCodecMode::kScreensharing,
            std::memory_order_release);
        const int result = encoder_->InitEncode(codecSettings, settings);
        nativeRateControllerTrusted_.store(
            encoder_->GetEncoderInfo().has_trusted_rate_controller, std::memory_order_release);
        return result;
    }

    int32_t RegisterEncodeCompleteCallback(
        webrtc::EncodedImageCallback* callback) override
    {
        {
            std::lock_guard lock(mutex_);
            callback_ = callback;
        }
        return encoder_->RegisterEncodeCompleteCallback(
            callback ? this : nullptr);
    }

    int32_t Release() override
    {
        ClearPending();
        lowLatencyScreenshare_.store(false, std::memory_order_release);
        {
            std::lock_guard lock(mutex_);
            callback_ = nullptr;
            qpParser_.reset();
            rawRates_.reset();
            rawRatesDirty_ = false;
            publishedQualityDecision_.reset();
            qualityEligible_ = false;
            codecSettings_.reset();
            encoderSettings_.reset();
        }
        return encoder_->Release();
    }

    int32_t Encode(
        const webrtc::VideoFrame& frame,
        const std::vector<webrtc::VideoFrameType>* frameTypes) override
    {
        RefreshQualityRates();
        if (reconfigureError_ != WEBRTC_VIDEO_CODEC_OK) return reconfigureError_;
        const std::uint32_t timestamp = frame.rtp_timestamp();
        {
            std::lock_guard lock(mutex_);
            if (pendingStartsUs_.size() >= 512) {
                pendingStartsUs_.erase(pendingStartsUs_.begin());
            }
            pendingStartsUs_[timestamp] =
                VideoCodecTimingRegistry::SteadyNowUs();
        }
        std::vector<webrtc::VideoFrameType> keyFrameTypes;
        if (forceKeyFrame_) {
            keyFrameTypes = frameTypes ? *frameTypes : std::vector<webrtc::VideoFrameType>{webrtc::VideoFrameType::kVideoFrameKey};
            if (keyFrameTypes.empty()) keyFrameTypes.push_back(webrtc::VideoFrameType::kVideoFrameKey);
            std::fill(keyFrameTypes.begin(), keyFrameTypes.end(), webrtc::VideoFrameType::kVideoFrameKey);
        }
        const int32_t result = encoder_->Encode(frame, forceKeyFrame_ ? &keyFrameTypes : frameTypes);
        if (result == WEBRTC_VIDEO_CODEC_OK) forceKeyFrame_ = false;
        if (result != WEBRTC_VIDEO_CODEC_OK) {
            std::lock_guard lock(mutex_);
            pendingStartsUs_.erase(timestamp);
        }
        return result;
    }

    void SetRates(
        const RateControlParameters& parameters) override
    {
        {
            std::lock_guard lock(mutex_);
            rawRates_ = parameters;
            rawRatesDirty_ = true;
        }
        RefreshQualityRates();
    }

    void OnPacketLossRateUpdate(float packetLossRate) override
    {
        encoder_->OnPacketLossRateUpdate(packetLossRate);
    }

    void OnRttUpdate(int64_t rttMs) override
    {
        encoder_->OnRttUpdate(rttMs);
    }

    void OnLossNotification(
        const LossNotification& lossNotification) override
    {
        encoder_->OnLossNotification(lossNotification);
    }

    EncoderInfo GetEncoderInfo() const override
    {
        auto info = encoder_->GetEncoderInfo();
        nativeRateControllerTrusted_.store(info.has_trusted_rate_controller,
                                          std::memory_order_release);
        info.implementation_name =
            VideoCodecTimingRegistry::TaggedImplementation(
                std::move(info.implementation_name),
                timingInstanceId_);
        return info;
    }

    Result OnEncodedImage(
        const webrtc::EncodedImage& encodedImage,
        const webrtc::CodecSpecificInfo* codecSpecificInfo) override
    {
        // EncodedImage copies share the ref-counted bitstream buffer. Adding
        // metadata here therefore adds no image copy and covers hardware and
        // software fallback without changing either backend's ownership.
        webrtc::EncodedImage outputImage = encodedImage;
        webrtc::EncodedImageCallback* callback = nullptr;
        {
            std::lock_guard lock(mutex_);
            if (qpParser_) {
                const auto qp = qpParser_->ParseFrame(
                    {encodedImage.data(), encodedImage.size()},
                    encodedImage.qp_ < 0);
                if (encodedImage.qp_ < 0 && qp) outputImage.qp_ = *qp;
            }
            if (qualityEnabled_) qualityPolicy_.ObserveEncoded(
                outputImage.size(), outputImage.qp_, outputImage.IsKey());
            callback = callback_;
        }
        RecordCompleted(outputImage);
        if (!callback) {
            return Result(Result::ERROR_SEND_FAILED);
        }

        if (!lowLatencyScreenshare_.load(std::memory_order_acquire)) {
            return callback->OnEncodedImage(
                outputImage, codecSpecificInfo);
        }

        // Desktop control values freshness over replaying every old frame.
        // Advertising a zero playout-delay range activates libwebrtc's
        // low-latency receive path. If a hardware decoder temporarily blocks,
        // the frame buffer then fast-forwards to the newest decodable frame
        // instead of growing a seconds-long queue. Camera video keeps the
        // normal adaptive jitter buffer because only screenshare encoders set
        // this flag during InitEncode().
        outputImage.SetPlayoutDelay(
            webrtc::VideoPlayoutDelay::Minimal());
        return callback->OnEncodedImage(
            outputImage, codecSpecificInfo);
    }

    void OnFrameDropped(
        uint32_t rtpTimestamp,
        int spatialId,
        bool isEndOfTemporalUnit) override
    {
        if (isEndOfTemporalUnit) {
            std::lock_guard lock(mutex_);
            pendingStartsUs_.erase(rtpTimestamp);
            VideoCodecTimingRegistry::Instance().RecordDropped(
                timingInstanceId_);
        }

        webrtc::EncodedImageCallback* callback = nullptr;
        {
            std::lock_guard lock(mutex_);
            callback = callback_;
        }
        if (callback) {
            callback->OnFrameDropped(
                rtpTimestamp, spatialId, isEndOfTemporalUnit);
        }
    }

private:
    void RefreshQualityRates()
    {
        if (reconfigureError_ != WEBRTC_VIDEO_CODEC_OK) return;
        const bool mode = qualityControl_ &&
            qualityControl_->ScreenQualityProtectionEnabled();
        const auto now = GoogCcTelemetryState::NowMs();
        const auto network = mode ? qualityControl_->Snapshot(now) : GoogCcNetworkDiagnostics{};
        const bool recentOveruse = network.lastDelayOveruseAtMs != 0 &&
            now >= network.lastDelayOveruseAtMs && now - network.lastDelayOveruseAtMs <= 1500;
        const bool pressure = network.feedbackFresh &&
            (network.delayState == "overuse" || recentOveruse ||
             network.congestionWindowReduction > 0.01 || network.lossPercent >= 5.0);
        const bool supported = !nativeRateControllerTrusted_.load(std::memory_order_acquire);
        const auto target = qualityControl_ ? qualityControl_->ScreenQualityTarget()
                                           : std::pair<std::uint32_t, std::uint32_t>{};
        std::optional<RateControlParameters> output;
        std::optional<webrtc::VideoCodec> reconfigure;
        std::optional<Settings> settings;
        ScreenFrameQualityDecision decision;
        bool enabled = false;
        bool publishDecision = false;
        {
            std::lock_guard lock(mutex_);
            if (!rawRates_) return;
            enabled = mode && qualityEligible_ && supported;
            if (qualityEnabled_ && !enabled) restoreUserCodec_ = true;
            const bool newUserTarget = target.first != 0 && target.second != 0 &&
                (target.first != nominalFrameRate_ || target.second != qualityCapBps_);
            // Codec caps are rounded to Kbps. A sub-Kbps profile difference
            // must not reopen the GPU encoder on its first allocation.
            const bool backendTargetChanged = newUserTarget && codecSettings_ &&
                (target.first != codecSettings_->maxFramerate ||
                 std::abs(static_cast<std::int64_t>(target.second) -
                     static_cast<std::int64_t>(codecSettings_->maxBitrate) * 1000) >= 1000);
            if (backendTargetChanged && !enabled && qualityEligible_) restoreUserCodec_ = true;
            if (enabled && codecSettings_ && encoderSettings_ &&
                (backendTargetChanged || restoreUserCodec_)) {
                reconfigure = *codecSettings_;
                reconfigure->maxFramerate = (std::clamp)(target.first != 0 ? target.first : nominalFrameRate_, 1u, 120u);
                const auto cap = target.second != 0 ? target.second : qualityCapBps_;
                reconfigure->maxBitrate = (std::max)(1u, (cap + 999u) / 1000u);
                reconfigure->minBitrate = (std::min)(reconfigure->minBitrate, reconfigure->maxBitrate);
                // Never reinstall the connection's old startup prior. This
                // reinitializes only the codec, at the CURRENT allocation.
                reconfigure->startBitrate = (std::max)(1u,
                    (std::min)(rawRates_->bitrate.get_sum_bps(), cap) / 1000u);
                if (reconfigure->numberOfSimulcastStreams == 1) {
                    auto& layer = reconfigure->simulcastStream[0];
                    layer.maxFramerate = reconfigure->maxFramerate;
                    layer.maxBitrate = reconfigure->maxBitrate;
                    layer.minBitrate = (std::min)(layer.minBitrate, layer.maxBitrate);
                    layer.targetBitrate = (std::min)(layer.targetBitrate, layer.maxBitrate);
                }
                settings = *encoderSettings_;
                appliedReferenceBps_ = 0;
                restoreUserCodec_ = false;
            }
            if (target.first != 0 && target.second != 0 &&
                (target.first != nominalFrameRate_ || target.second != qualityCapBps_)) {
                qualityStartBps_ = static_cast<std::uint32_t>(
                    (std::min<std::uint64_t>)(target.second,
                        static_cast<std::uint64_t>(qualityStartBps_) * target.first /
                        (std::max)(nominalFrameRate_, 1u)));
                nominalFrameRate_ = (std::clamp)(target.first, 1u, 120u);
                qualityCapBps_ = target.second;
                qualityPolicy_.Retarget(nominalFrameRate_, qualityCapBps_);
            }
            if (enabled != qualityEnabled_ || network.routeRevision != qualityRouteRevision_) {
                qualityPolicy_.Reset(nominalFrameRate_, qualityCapBps_, qualityStartBps_);
                qualityEnabled_ = enabled;
                qualityRouteRevision_ = network.routeRevision;
            }
            // target_bitrate is the allocation BEFORE EncoderBitrateAdjuster.
            // Native FrameDropper uses that unadjusted target too. Using the
            // adjusted bitrate here applies its overshoot discount a second
            // time to our intentional larger-frame / lower-FPS tradeoff.
            // Explicit zero remains paused; bandwidth_allocation is not a
            // substitute for this stream's target allocation.
            const auto budget = enabled ? (std::min)(rawRates_->target_bitrate.get_sum_bps(), qualityCapBps_)
                                        : rawRates_->bitrate.get_sum_bps();
            decision = qualityPolicy_.Update(budget, enabled, pressure,
                qualityControl_ ? qualityControl_->ScreenQualityDeficitShare()
                                : kDefaultScreenQualityDeficitShareHundredths);
            decision.encoderAdjustedBudgetBps = rawRates_->bitrate.get_sum_bps();
            decision.bandwidthAllocationBps = rawRates_->bandwidth_allocation.IsFinite()
                ? static_cast<std::uint32_t>((std::min<std::int64_t>)(
                    rawRates_->bandwidth_allocation.bps(), std::numeric_limits<std::uint32_t>::max()))
                : 0;
            publishDecision = !publishedQualityDecision_ ||
                decision.protecting != publishedQualityDecision_->protecting ||
                decision.qualityLimited != publishedQualityDecision_->qualityLimited ||
                decision.networkBudgetBps != publishedQualityDecision_->networkBudgetBps ||
                decision.encoderAdjustedBudgetBps != publishedQualityDecision_->encoderAdjustedBudgetBps ||
                decision.bandwidthAllocationBps != publishedQualityDecision_->bandwidthAllocationBps ||
                decision.referenceBps != publishedQualityDecision_->referenceBps ||
                decision.deficitShareHundredths != appliedDeficitShareHundredths_;
            publishedQualityDecision_ = decision;
            appliedDeficitShareHundredths_ = decision.deficitShareHundredths;
            if (reconfigure) {
                // A held quality reference is permitted only after real GCC
                // pressure; otherwise startup uses exactly the current budget.
                reconfigure->startBitrate = (std::max)(1u, decision.encoderReferenceBps / 1000u);
            }
            const double fps = enabled ? nominalFrameRate_ : rawRates_->framerate_fps;
            if (rawRatesDirty_ || decision.encoderReferenceBps != appliedReferenceBps_ || fps != appliedRateFps_) {
                output = *rawRates_;
                if (enabled) {
                    // This rate describes the cost at the ORIGINAL cadence.
                    // GCC/native FrameDropper/pacer retain the unmodified
                    // network allocation. No second frame dropper is added.
                    output->bitrate.SetBitrate(0, 0, decision.encoderReferenceBps);
                    output->framerate_fps = fps;
                }
                appliedReferenceBps_ = decision.encoderReferenceBps;
                appliedRateFps_ = fps;
                rawRatesDirty_ = false;
            }
        }
        // Backends may synchronously drain old frames when changing rates.
        // Never hold the callback mutex across that reentrant operation.
        if (reconfigure) {
            int result = encoder_->Release();
            std::vector<std::uint32_t> dropped;
            webrtc::EncodedImageCallback* callback = nullptr;
            {
                std::lock_guard lock(mutex_);
                for (const auto& [timestamp, start] : pendingStartsUs_) dropped.push_back(timestamp);
                pendingStartsUs_.clear();
                callback = callback_;
            }
            // Release may already have completed some asynchronous frames.
            // Notify only those that remain, exactly once, outside the lock.
            for (const auto timestamp : dropped) {
                VideoCodecTimingRegistry::Instance().RecordDropped(timingInstanceId_);
                if (callback) callback->OnFrameDropped(timestamp, 0, true);
            }
            if (result == WEBRTC_VIDEO_CODEC_OK) result = encoder_->InitEncode(&*reconfigure, *settings);
            if (result == WEBRTC_VIDEO_CODEC_OK && callback)
                result = encoder_->RegisterEncodeCompleteCallback(this);
            reconfigureError_ = result;
            if (result != WEBRTC_VIDEO_CODEC_OK) return;
            forceKeyFrame_ = true;
            nativeRateControllerTrusted_.store(
                encoder_->GetEncoderInfo().has_trusted_rate_controller, std::memory_order_release);
            {
                std::lock_guard lock(mutex_);
                codecSettings_ = *reconfigure;
                // Discard old SPS/PPS only after the old backend is drained.
                qpParser_ = std::make_unique<HeaderOnlyH264QpParser>();
            }
        }
        if (output) {
            encoder_->SetRates(*output);
        }
        if (output || publishDecision) {
            VideoCodecTimingRegistry::Instance().RecordScreenQualityRates(
                timingInstanceId_, enabled, decision);
        }
    }

    void ClearPending()
    {
        std::lock_guard lock(mutex_);
        pendingStartsUs_.clear();
    }

    void RecordCompleted(const webrtc::EncodedImage& encodedImage)
    {
        const std::uint32_t timestamp =
            encodedImage.RtpTimestamp();
        std::int64_t startUs = 0;
        {
            std::lock_guard lock(mutex_);
            const auto found = pendingStartsUs_.find(timestamp);
            if (found == pendingStartsUs_.end()) {
                return;
            }
            startUs = found->second;
            pendingStartsUs_.erase(found);
        }
        const std::int64_t nowUs =
            VideoCodecTimingRegistry::SteadyNowUs();
        if (nowUs < startUs) {
            return;
        }
        VideoCodecTimingRegistry::Instance().RecordCompleted(
            timingInstanceId_,
            VideoCodecTimingRegistry::UntaggedImplementation(
                encoder_->GetEncoderInfo().implementation_name),
            timestamp,
            static_cast<std::uint64_t>(nowUs - startUs),
            encodedImage._encodedWidth,
            encodedImage._encodedHeight,
            encodedImage.size(),
            encodedImage.qp_ >= 0
                ? std::make_optional<std::int32_t>(
                      encodedImage.qp_)
                : std::nullopt);
    }

    std::unique_ptr<webrtc::VideoEncoder> encoder_;
    std::shared_ptr<GoogCcTelemetryState> qualityControl_;
    const bool nativeFrameDropperAvailable_;
    mutable std::atomic<bool> nativeRateControllerTrusted_{false};
    ScreenFrameQualityPolicy qualityPolicy_;
    std::optional<RateControlParameters> rawRates_;
    bool rawRatesDirty_ = false;
    std::optional<webrtc::VideoCodec> codecSettings_;
    std::optional<Settings> encoderSettings_;
    int reconfigureError_ = WEBRTC_VIDEO_CODEC_OK;
    bool restoreUserCodec_ = false;
    bool forceKeyFrame_ = false;
    bool qualityEligible_ = false, qualityEnabled_ = false;
    std::uint64_t qualityRouteRevision_ = 0;
    std::uint32_t nominalFrameRate_ = 0, qualityCapBps_ = 0, qualityStartBps_ = 0;
    std::uint32_t appliedReferenceBps_ = 0;
    std::uint32_t appliedDeficitShareHundredths_ = 0;
    std::optional<ScreenFrameQualityDecision> publishedQualityDecision_;
    double appliedRateFps_ = 0;
    const std::uint64_t timingInstanceId_;
    mutable std::mutex mutex_;
    std::unordered_map<std::uint32_t, std::int64_t> pendingStartsUs_;
    webrtc::EncodedImageCallback* callback_ = nullptr;
    std::unique_ptr<HeaderOnlyH264QpParser> qpParser_;
    std::atomic_bool lowLatencyScreenshare_{false};
};

void AppendUniqueFormats(
    std::vector<webrtc::SdpVideoFormat>& destination,
    const std::vector<webrtc::SdpVideoFormat>& source)
{
    for (const auto& format : source) {
        if (!format.IsCodecInList(destination)) {
            destination.push_back(format);
        }
    }
}

}  // namespace

class WindowsPreferredVideoEncoderFactoryState final {
public:
    WindowsPreferredVideoEncoderFactoryState(
        std::unique_ptr<webrtc::VideoEncoderFactory> softwareFactory,
        std::unique_ptr<webrtc::VideoEncoderFactory> hardwareFactory,
        std::shared_ptr<VideoEncoderRuntimeState> runtimeState)
        : softwareFactory_(std::move(softwareFactory)),
          hardwareFactory_(std::move(hardwareFactory)),
          runtimeState_(std::move(runtimeState))
    {}

    std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const
    {
        std::lock_guard lock(mutex_);
        std::vector<webrtc::SdpVideoFormat> formats;
        if (hardwareFactory_) {
            AppendUniqueFormats(
                formats, hardwareFactory_->GetSupportedFormats());
        }
        if (softwareFactory_) {
            AppendUniqueFormats(
                formats, softwareFactory_->GetSupportedFormats());
        }
        return formats;
    }

    webrtc::VideoEncoderFactory::CodecSupport QueryCodecSupport(
        const webrtc::SdpVideoFormat& format,
        std::optional<std::string> scalabilityMode,
        std::optional<webrtc::Resolution> resolution) const
    {
        std::lock_guard lock(mutex_);
        if (hardwareFactory_) {
            const auto support = hardwareFactory_->QueryCodecSupport(
                format, scalabilityMode, resolution);
            if (support.is_supported) {
                return support;
            }
        }
        return softwareFactory_
                   ? softwareFactory_->QueryCodecSupport(
                         format, scalabilityMode, resolution)
                   : webrtc::VideoEncoderFactory::CodecSupport{};
    }

    std::unique_ptr<webrtc::VideoEncoder> CreateSelected(
        const webrtc::Environment& environment,
        const webrtc::SdpVideoFormat& format,
        std::uint64_t* revision) const
    {
        std::lock_guard lock(mutex_);
        std::unique_ptr<webrtc::VideoEncoder> hardware;
        if (hardwareFactory_) {
            hardware = hardwareFactory_->Create(environment, format);
        }
        std::unique_ptr<webrtc::VideoEncoder> software;
        if (softwareFactory_) {
            software = softwareFactory_->Create(environment, format);
            if (software && runtimeState_) {
                software =
                    std::make_unique<RuntimeTrackedSoftwareEncoder>(
                        std::move(software), runtimeState_);
            }
        }
        if (revision) {
            *revision = revision_;
        }
        if (hardware && software) {
            hardware =
                std::make_unique<TransientFailureRetryVideoEncoder>(
                    std::move(hardware));
            webrtc::EnvironmentFactory fallbackEnvironmentFactory(
                environment);
            fallbackEnvironmentFactory.Set(
                std::make_unique<HardwareFailureOnlyFieldTrials>(
                    environment.field_trials()));
            const webrtc::Environment fallbackEnvironment =
                fallbackEnvironmentFactory.Create();
            return webrtc::CreateVideoEncoderSoftwareFallbackWrapper(
                fallbackEnvironment,
                std::move(software), std::move(hardware),
                false);
        }
        return hardware ? std::move(hardware) : std::move(software);
    }

    std::uint64_t Revision() const
    {
        std::lock_guard lock(mutex_);
        return revision_;
    }

    bool HasHardwareEncoderFor(
        const webrtc::SdpVideoFormat& format) const
    {
        std::lock_guard lock(mutex_);
        return hardwareFactory_ && format.IsCodecInList(
            hardwareFactory_->GetSupportedFormats());
    }

    bool HasSoftwareEncoderFor(
        const webrtc::SdpVideoFormat& format) const
    {
        std::lock_guard lock(mutex_);
        return softwareFactory_ && format.IsCodecInList(
            softwareFactory_->GetSupportedFormats());
    }

    void ReplaceFactories(
        std::unique_ptr<webrtc::VideoEncoderFactory> softwareFactory,
        std::unique_ptr<webrtc::VideoEncoderFactory> hardwareFactory)
    {
        std::lock_guard lock(mutex_);
        softwareFactory_ = std::move(softwareFactory);
        hardwareFactory_ = std::move(hardwareFactory);
        ++revision_;
    }

private:
    mutable std::mutex mutex_;
    std::unique_ptr<webrtc::VideoEncoderFactory> softwareFactory_;
    std::unique_ptr<webrtc::VideoEncoderFactory> hardwareFactory_;
    std::shared_ptr<VideoEncoderRuntimeState> runtimeState_;
    std::uint64_t revision_ = 1;
};

namespace {

class ReconfigurableVideoEncoder final : public webrtc::VideoEncoder {
public:
    ReconfigurableVideoEncoder(
        std::shared_ptr<WindowsPreferredVideoEncoderFactoryState> state,
        webrtc::Environment environment,
        webrtc::SdpVideoFormat format)
        : state_(std::move(state)),
          environment_(std::move(environment)),
          format_(std::move(format)),
          encoder_(state_->CreateSelected(
              environment_, format_, &revision_))
    {}

    void SetFecControllerOverride(
        webrtc::FecControllerOverride* fecControllerOverride) override
    {
        fecControllerOverride_ = fecControllerOverride;
        if (encoder_) {
            encoder_->SetFecControllerOverride(fecControllerOverride);
        }
    }

    int InitEncode(
        const webrtc::VideoCodec* codecSettings,
        const Settings& settings) override
    {
        if (!codecSettings || !EnsureCurrent(false)) {
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        codecSettings_ = *codecSettings;
        settings_ = settings;
        const int result = encoder_->InitEncode(codecSettings, settings);
        initialized_ = result == WEBRTC_VIDEO_CODEC_OK;
        return result;
    }

    int32_t RegisterEncodeCompleteCallback(
        webrtc::EncodedImageCallback* callback) override
    {
        callback_ = callback;
        return encoder_
                   ? encoder_->RegisterEncodeCompleteCallback(callback)
                   : WEBRTC_VIDEO_CODEC_ERROR;
    }

    int32_t Release() override
    {
        initialized_ = false;
        callback_ = nullptr;
        rates_.reset();
        return encoder_ ? encoder_->Release() : WEBRTC_VIDEO_CODEC_OK;
    }

    int32_t Encode(
        const webrtc::VideoFrame& frame,
        const std::vector<webrtc::VideoFrameType>* frameTypes) override
    {
        return EnsureCurrent(true)
                   ? encoder_->Encode(frame, frameTypes)
                   : WEBRTC_VIDEO_CODEC_ERROR;
    }

    void SetRates(const RateControlParameters& parameters) override
    {
        rates_ = parameters;
        if (encoder_) {
            encoder_->SetRates(parameters);
        }
    }

    void OnPacketLossRateUpdate(float packetLossRate) override
    {
        if (encoder_) {
            encoder_->OnPacketLossRateUpdate(packetLossRate);
        }
    }

    void OnRttUpdate(int64_t rttMs) override
    {
        if (encoder_) {
            encoder_->OnRttUpdate(rttMs);
        }
    }

    void OnLossNotification(
        const LossNotification& lossNotification) override
    {
        if (encoder_) {
            encoder_->OnLossNotification(lossNotification);
        }
    }

    EncoderInfo GetEncoderInfo() const override
    {
        return encoder_ ? encoder_->GetEncoderInfo() : EncoderInfo{};
    }

private:
    bool EnsureCurrent(bool initialize)
    {
        if (!state_) {
            return false;
        }
        const std::uint64_t currentRevision = state_->Revision();
        if (encoder_ && currentRevision == revision_) {
            return true;
        }
        std::uint64_t newRevision = 0;
        auto replacement = state_->CreateSelected(
            environment_, format_, &newRevision);
        if (!replacement) {
            return false;
        }
        replacement->SetFecControllerOverride(fecControllerOverride_);
        if (initialize && initialized_ && codecSettings_ && settings_) {
            const int initResult = replacement->InitEncode(
                &*codecSettings_, *settings_);
            if (initResult != WEBRTC_VIDEO_CODEC_OK) {
                replacement->Release();
                return false;
            }
            if (callback_ &&
                replacement->RegisterEncodeCompleteCallback(callback_) !=
                    WEBRTC_VIDEO_CODEC_OK) {
                replacement->Release();
                return false;
            }
            if (rates_) {
                replacement->SetRates(*rates_);
            }
        }
        if (encoder_) {
            encoder_->Release();
        }
        encoder_ = std::move(replacement);
        revision_ = newRevision;
        return true;
    }

    std::shared_ptr<WindowsPreferredVideoEncoderFactoryState> state_;
    webrtc::Environment environment_;
    webrtc::SdpVideoFormat format_;
    std::unique_ptr<webrtc::VideoEncoder> encoder_;
    std::uint64_t revision_ = 0;
    webrtc::FecControllerOverride* fecControllerOverride_ = nullptr;
    webrtc::EncodedImageCallback* callback_ = nullptr;
    bool initialized_ = false;
    std::optional<webrtc::VideoCodec> codecSettings_;
    std::optional<Settings> settings_;
    std::optional<RateControlParameters> rates_;
};

}  // namespace

WindowsPreferredVideoEncoderFactory::
WindowsPreferredVideoEncoderFactory(
    std::unique_ptr<webrtc::VideoEncoderFactory> softwareFactory,
    std::unique_ptr<webrtc::VideoEncoderFactory> hardwareFactory,
    std::shared_ptr<VideoEncoderRuntimeState> runtimeState,
    std::shared_ptr<GoogCcTelemetryState> qualityControl)
    : state_(std::make_shared<WindowsPreferredVideoEncoderFactoryState>(
          std::move(softwareFactory), std::move(hardwareFactory),
          std::move(runtimeState))), qualityControl_(std::move(qualityControl))
{}

WindowsPreferredVideoEncoderFactory::
~WindowsPreferredVideoEncoderFactory() = default;

std::vector<webrtc::SdpVideoFormat>
WindowsPreferredVideoEncoderFactory::GetSupportedFormats() const
{
    return state_->GetSupportedFormats();
}

webrtc::VideoEncoderFactory::CodecSupport
WindowsPreferredVideoEncoderFactory::QueryCodecSupport(
    const webrtc::SdpVideoFormat& format,
    std::optional<std::string> scalabilityMode,
    std::optional<webrtc::Resolution> resolution) const
{
    return state_->QueryCodecSupport(
        format, std::move(scalabilityMode), resolution);
}

std::unique_ptr<webrtc::VideoEncoder>
WindowsPreferredVideoEncoderFactory::Create(
    const webrtc::Environment& environment,
    const webrtc::SdpVideoFormat& format)
{
    auto selected = std::make_unique<ReconfigurableVideoEncoder>(
        state_, environment, format);
    if (selected->GetEncoderInfo().implementation_name.empty()) {
        return nullptr;
    }
    return std::make_unique<FrameTimedVideoEncoder>(
        std::move(selected), qualityControl_ ? qualityControl_ : GoogCcTelemetryForEnvironment(environment),
        !environment.field_trials().IsDisabled("WebRTC-FrameDropper"));
}

bool WindowsPreferredVideoEncoderFactory::HasHardwareEncoderFor(
    const webrtc::SdpVideoFormat& format) const
{
    return state_->HasHardwareEncoderFor(format);
}

bool WindowsPreferredVideoEncoderFactory::HasSoftwareEncoderFor(
    const webrtc::SdpVideoFormat& format) const
{
    return state_->HasSoftwareEncoderFor(format);
}

void WindowsPreferredVideoEncoderFactory::ReplaceFactories(
    std::unique_ptr<webrtc::VideoEncoderFactory> softwareFactory,
    std::unique_ptr<webrtc::VideoEncoderFactory> hardwareFactory)
{
    state_->ReplaceFactories(
        std::move(softwareFactory), std::move(hardwareFactory));
}

}  // namespace remote
