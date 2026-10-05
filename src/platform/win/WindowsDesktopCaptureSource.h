// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stop_token>
#include <string>
#include <thread>
#include <unordered_map>

#include "api/video/adapted_video_track_source.h"
#include "modules/desktop_capture/desktop_capturer.h"
#include "src/core/DesktopCaptureTypes.h"
#include "src/core/DisplayTopology.h"
#include "src/media_intelligence/core/ContentState.h"
#include "src/platform/win/IRemoteVisionFrameAnalyzer.h"

namespace remote::media_intelligence {
class ContentAnalysisWorker;
}

namespace remote {

// Captures one Windows display on a dedicated thread. WebRTC selects DXGI
// Desktop Duplication once when the capturer is created and keeps GDI as the
// permanent-error fallback; there is no per-frame capability probing.
class WindowsDesktopCaptureSource
    : public webrtc::AdaptedVideoTrackSource,
      private webrtc::DesktopCapturer::Callback {
public:
    enum class CaptureActivityState : std::uint8_t {
        kStarting,
        kActive,
        kIdle,
    };

    struct RuntimeStats {
        std::uint32_t targetFrameRate = 0;
        CaptureActivityState activityState =
            CaptureActivityState::kStarting;
        bool adaptiveFrameDeliveryEnabled = false;
        double captureAttemptsPerSecond = 0.0;
        double deliveredFramesPerSecond = 0.0;
        double changedFramesPerSecond = 0.0;
        double changedAreaRatio = 0.0;
        double idleHeartbeatFramesPerSecond = 0.0;
        std::uint64_t totalCaptureAttempts = 0;
        std::uint64_t totalDeliveredFrames = 0;
        std::uint64_t totalChangedFrames = 0;
        std::uint64_t totalIdleHeartbeatFrames = 0;
        std::uint64_t totalSuppressedUnchangedFrames = 0;
        std::uint64_t totalActivityTransitions = 0;
        std::uint64_t totalFailedCaptures = 0;
        bool inputBoostActive = false;
        std::uint64_t totalInputBoosts = 0;
        std::uint64_t totalForcedRefreshFrames = 0;
        double latestCaptureCallMs = 0.0;
        bool contentAnalyzerEnabled = false;
        std::string contentAnalyzerBackend = "disabled";
        std::uint64_t contentAnalysisGeneration = 0;
        media_intelligence::ContentState contentState;
        std::uint32_t contentStateAgeMs = 0;
        std::uint32_t contentLatestAnalysisTimeUs = 0;
        std::uint32_t contentLatestScaleConvertTimeUs = 0;
        std::uint32_t contentLatestJpegEncodeTimeUs = 0;
        std::uint64_t contentLatestJpegBytes = 0;
        media_intelligence::SemanticClassification contentLatestReturnedClassification;
        std::uint32_t contentLatestReturnedAgeMs = 0;
        std::uint64_t contentSubmittedSamples = 0;
        std::uint64_t contentReplacedSamples = 0;
        std::uint64_t contentProcessedSamples = 0;
        std::uint64_t contentRejectedSamples = 0;
        std::uint64_t contentDiscardedResults = 0;
    };

    enum class CaptureBackend {
        kDxgiNativeTexture,
        kDxgiPreferred,
        kGdi
    };

    explicit WindowsDesktopCaptureSource(
        DesktopCaptureImplementation implementation =
            DesktopCaptureImplementation::kNativeDxgi,
        DisplayDescriptor captureTarget = {},
        bool contentAnalyzerEnabled = false,
        std::uint32_t contentAnalyzerRateHz = 3,
        std::shared_ptr<IRemoteVisionFrameAnalyzer>
            remoteVisionAnalyzer = {});
    ~WindowsDesktopCaptureSource() override;

    bool StartCapture(
        std::chrono::milliseconds firstFrameTimeout =
            std::chrono::seconds(3));
    void StopCapture();
    bool SetTargetFrameRate(std::uint32_t framesPerSecond);
    // Chrome Remote Desktop-style short capture boost. The caller invokes
    // this only after the remote input has been injected into Windows.
    void NotifyRemoteInputActivity();
    // Forces the next successful capture to be delivered as a full update.
    // Used after ICE recovery and sender/track reactivation.
    void RequestRefreshFrame();
    // Delivers a short, bounded run of full frames after a new sender or sink
    // becomes active. This gives WebRTC enough stable input for its first
    // keyframe and startup bandwidth probe without changing the steady-state
    // desktop activity policy.
    void RequestStartupFrameBurst(
        std::uint32_t frameCount = 6,
        std::function<void()> firstDeliveredFrameCallback = {},
        std::function<void()> burstCompletedCallback = {});
    std::uint32_t TargetFrameRate() const noexcept;
    std::uint32_t CapturedWidth() const noexcept;
    std::uint32_t CapturedHeight() const noexcept;
    RuntimeStats CaptureRuntimeStats() const noexcept;

    DesktopCaptureImplementation ConfiguredImplementation() const noexcept;
    CaptureBackend Backend() const;
    std::string FallbackReason() const;
    std::string LastError() const;
    const DisplayDescriptor& CaptureTarget() const noexcept;

    SourceState state() const override;
    bool remote() const override;
    bool is_screencast() const override;
    std::optional<bool> needs_denoising() const override;
    void AddOrUpdateSink(webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink,
        const webrtc::VideoSinkWants& wants) override;
    void RemoveSink(webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink) override;
    bool GetStats(Stats* stats) override;
    void ProcessConstraints(const webrtc::VideoTrackSourceConstraints& constraints) override;

private:
    class FrameRateSinkProxy;
    friend class DesktopCaptureDeliveryTestAccess;
    enum class FrameDeliveryReason : std::uint8_t {
        kInitial,
        kDesktopChanged,
        kStartupPrime,
        kScheduledRepeat,
        kIdleHeartbeat,
        kForcedRefresh,
    };

    struct FrameUpdateRegion {
        int offsetX = 0;
        int offsetY = 0;
        int width = 0;
        int height = 0;
    };

    void CaptureLoop(std::stop_token stopToken);
    void OnCaptureResult(
        webrtc::DesktopCapturer::Result result,
        std::unique_ptr<webrtc::DesktopFrame> frame) override;
    void SetInitializationFailure(std::string message);
    void ResetActivityTracking();
    bool ShouldDeliverFrame(
        bool desktopChanged,
        bool forceRefresh,
        std::chrono::steady_clock::time_point now,
        FrameDeliveryReason* reason);
    bool ShouldDeliverLibWebRtcFrame(
        bool desktopChanged,
        bool forceRefresh,
        std::chrono::steady_clock::time_point now,
        FrameDeliveryReason* reason);
    void DeliverFrame(
        webrtc::scoped_refptr<webrtc::VideoFrameBuffer> buffer,
        FrameDeliveryReason reason,
        std::int64_t timestampUs = 0);
    void DeliverLibWebRtcFrame(
        webrtc::scoped_refptr<webrtc::VideoFrameBuffer> buffer,
        FrameDeliveryReason reason,
        const FrameUpdateRegion& updateRegion,
        bool repeatFrame,
        std::int64_t timestampUs = 0);
    bool AcceptFrameForDelivery(
        const webrtc::scoped_refptr<webrtc::VideoFrameBuffer>& buffer,
        std::int64_t timestampUs);
    bool ConsumeForcedRefreshFrame();
    void ScheduleForcedRefreshFrames(std::uint32_t frameCount);
    void SignalCaptureSchedule();
    void RecordChangedAreaRatio(float ratio) noexcept;
    void PublishChangedAreaRatioWindow() noexcept;
    void MaybeSubmitContentAnalysis(float changedAreaRatio) noexcept;
    void MaybeSubmitRemoteVisionFrame(
        webrtc::scoped_refptr<webrtc::VideoFrameBuffer> buffer) noexcept;

    mutable std::mutex mutex_;
    // Control-thread ownership only. VideoBroadcaster synchronizes dispatch
    // with RemoveSink; capture callbacks never acquire this map mutex.
    std::mutex sinkProxyMutex_;
    std::unordered_map<webrtc::VideoSinkInterface<webrtc::VideoFrame>*,
        std::unique_ptr<FrameRateSinkProxy>> sinkProxies_;
    webrtc::VideoBroadcaster sinkBroadcaster_;
    std::atomic<std::uint64_t> statsInputDimensions_{0};
    std::condition_variable firstFrameCondition_;
    std::jthread captureThread_;
    const DesktopCaptureImplementation configuredImplementation_;
    const DisplayDescriptor captureTarget_;
    const std::uint32_t contentAnalyzerRateHz_ = 3;
    std::unique_ptr<media_intelligence::ContentAnalysisWorker>
        contentAnalysisWorker_;
    const std::shared_ptr<IRemoteVisionFrameAnalyzer>
        remoteVisionAnalyzer_;
    CaptureBackend backend_ = CaptureBackend::kGdi;
    bool initializationFinished_ = false;
    bool firstFrameReady_ = false;
    // Capture-thread state: a discarded partial update requires the next
    // delivered CPU frame to cover the full desktop.
    bool deliveryUpdateRegionInvalidated_ = false;
    bool running_ = false;
    std::string fallbackReason_;
    std::string lastError_;
    std::atomic<std::uint32_t> targetFrameRate_{60};
    std::atomic<CaptureActivityState> activityState_{
        CaptureActivityState::kStarting};
    std::atomic<bool> adaptiveFrameDeliveryEnabled_{false};
    std::atomic<std::uint32_t> capturedWidth_{0};
    std::atomic<std::uint32_t> capturedHeight_{0};
    std::atomic<std::uint64_t> totalCaptureAttempts_{0};
    std::atomic<std::uint64_t> totalDeliveredFrames_{0};
    std::atomic<std::uint64_t> totalChangedFrames_{0};
    std::atomic<std::uint64_t> totalIdleHeartbeatFrames_{0};
    std::atomic<std::uint64_t> totalSuppressedUnchangedFrames_{0};
    std::atomic<std::uint64_t> totalActivityTransitions_{0};
    std::atomic<std::uint64_t> totalFailedCaptures_{0};
    std::atomic<std::uint64_t> totalInputBoosts_{0};
    std::atomic<std::uint64_t> totalForcedRefreshFrames_{0};
    std::atomic<std::uint64_t> captureAttemptsPerSecondMilli_{0};
    std::atomic<std::uint64_t> deliveredFramesPerSecondMilli_{0};
    std::atomic<std::uint64_t> changedFramesPerSecondMilli_{0};
    std::atomic<std::uint64_t> changedAreaRatioPpm_{0};
    std::atomic<std::uint64_t> changedAreaWindowPpmTotal_{0};
    std::atomic<std::uint64_t> changedAreaWindowSamples_{0};
    std::atomic<std::uint64_t> idleHeartbeatFramesPerSecondMilli_{0};
    std::atomic<std::uint64_t> latestCaptureCallUs_{0};
    std::atomic<std::int64_t> inputBoostUntilSteadyUs_{0};
    std::atomic<std::int64_t> startupPrimeUntilSteadyUs_{0};
    std::atomic<std::uint32_t> forcedRefreshFramesRemaining_{0};
    std::atomic<std::uint64_t> contentAnalysisGeneration_{0};
    std::atomic<std::uint64_t> contentAnalysisSourceFrameId_{0};
    std::atomic<std::int64_t> nextContentAnalysisSubmitSteadyUs_{0};
    std::atomic<std::uint64_t> remoteVisionSessionToken_{0};
    // HANDLE is kept opaque in the header. It is created and destroyed by the
    // active capture thread while mutex_ protects publication/lifetime.
    void* captureScheduleWakeEvent_ = nullptr;
    bool activityHasDeliveredFrame_ = false;
    std::chrono::steady_clock::time_point activityLastChangedAt_{};
    std::chrono::steady_clock::time_point activityLastDeliveredAt_{};
    std::function<void()> startupFrameDeliveredCallback_;
    std::function<void()> startupBurstCompletedCallback_;
    std::uint32_t startupCallbackFramesRemaining_ = 0;
};

}  // namespace remote
