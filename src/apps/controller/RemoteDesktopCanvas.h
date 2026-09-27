// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <Windows.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <utility>

#include <QImage>
#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QWidget>

#include "api/scoped_refptr.h"
#include "api/video/video_frame.h"
#include "api/video/video_frame_buffer.h"
#include "api/video/video_sink_interface.h"
#include "src/protocol/RemoteCursorProtocol.h"
#include "src/protocol/RemoteInputProtocol.h"

class QDragEnterEvent;
class QDropEvent;
class QEvent;
class QFocusEvent;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QWheelEvent;

namespace remote::controller {

class D3D11VideoSurface;
class HighResolutionPointerMoveScheduler;
class RemoteCursorRenderState;

class RemoteDesktopCanvas final
    : public QWidget,
    public webrtc::VideoSinkInterface<webrtc::VideoFrame> {
public:
    using InputSender = std::function<bool(const RemoteInputEvent&)>;
    // Returns true when RemoteC consumed the action. False means a
    // keyboard Ctrl+V must continue to the controlled machine as an
    // ordinary remote key event.
    using PasteSender =
        std::function<bool(const QStringList&, bool keyboardPaste)>;
    using FirstPresentationCallback =
        std::function<void(std::uint64_t)>;

    explicit RemoteDesktopCanvas(InputSender inputSender,
        PasteSender pasteSender,
        FirstPresentationCallback firstPresentationCallback,
        std::string telemetryPeerDeviceId,
        QWidget* parent = nullptr);

    ~RemoteDesktopCanvas() override;

    void SetControlEnabled(bool enabled);

    void SetActualPixelDisplayMode(bool enabled);

    bool ActualPixelDisplayMode() const;

    void ApplyRemoteCursor(const RemoteCursorEnvelope& envelope);

    void ResetRemoteCursor();

    void SetConnectionStage(QString stage);

    void BeginPresentationGeneration(std::uint64_t generation);

    void NotifyFirstPresentation();

    void SetTargetFrameRate(std::uint32_t framesPerSecond);

    void SetDragPointerSampleRate(std::uint32_t hertz);

    void ShutdownInputScheduler();

    void SetRemoteDisplayIdentity(
        std::uint32_t displayId,
        std::uint64_t layoutVersion);

    void ReleaseRemoteInputs();

    void OnFrame(const webrtc::VideoFrame& frame) override;

protected:
    void mouseMoveEvent(QMouseEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;

    void mouseReleaseEvent(QMouseEvent* event) override;

    void wheelEvent(QWheelEvent* event) override;

    void keyPressEvent(QKeyEvent* event) override;

    void keyReleaseEvent(QKeyEvent* event) override;

    void dragEnterEvent(QDragEnterEvent* event) override;

    void dropEvent(QDropEvent* event) override;

    void focusOutEvent(QFocusEvent* event) override;

    void resizeEvent(QResizeEvent* event) override;

    bool event(QEvent* event) override;

    void paintEvent(QPaintEvent*) override;

private:
    static bool IsCpuNv12Image(const QImage& image);

    void UpdateCpuCanvasMetrics();

    QSize CpuQtOutputSize(int sourceWidth, int sourceHeight) const;

    void QueueCpuFrame(
        webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame,
        bool recordArrival = true);

    void CpuConversionLoop(std::stop_token stopToken);

    void RecycleCpuImage(QImage image);

    void QueueConvertedCpuImage(QImage image, std::uint64_t sequence);

    void QueueD3D11CpuImage(QImage image, std::uint64_t sequence);

    void QueueQtCpuImage(QImage image, std::uint64_t sequence);

    void ApplyPendingCpuImage();

    void QueueNativeFrame(
        webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame);

    void QueueI420Frame(
        webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame);

    void ScheduleNativeUiUpdate();

    void UpdateCpuConversionInterval();

    void RequestNativeRedraw();

    void CursorVisualChanged();

    bool DeactivateNativePresentation(std::uint64_t cpuSequence);

    void NativePresentationLoop(std::stop_token stopToken);
    void UpdateNativeSurfaceGeometry();

    QRect VideoContentRect() const;

    QSize VideoSourceSize() const;

    std::optional<RemoteInputEvent>
        SampleCurrentDragPointer(
            const RemoteInputEvent& fallback) const;

    std::optional<std::pair<std::uint16_t, std::uint16_t>> MapPoint(
        const QPointF& point,
        bool clampToContent) const;

    static std::optional<RemoteMouseButton> MapMouseButton(
        Qt::MouseButton button);

    static std::uint8_t MapMouseButtons(Qt::MouseButtons buttons);

    static std::int16_t ClampWheelDelta(int value);

    static bool IsExtendedVirtualKey(std::uint32_t virtualKey);

    bool QueuePointerMove(
        const std::pair<std::uint16_t, std::uint16_t>& point,
        std::uint8_t pressedMouseButtons);

    void FlushPendingPointerMove();

    void CancelPendingPointerMove();

    bool SendKeyEvent(QKeyEvent* event, bool pressed);

    void FocusRemotePasteTarget(const QPointF& position);
    QImage frameImage_;
    std::uint64_t frameImageSequence_ = 0;
    std::uint64_t lastPresentedCpuSequence_ = 0;
    InputSender inputSender_;
    PasteSender pasteSender_;
    FirstPresentationCallback firstPresentationCallback_;
    std::atomic<std::uint64_t> presentationGeneration_{ 0 };
    std::atomic<bool> firstPresentationNotified_{ false };
    std::uint64_t presentationTelemetryId_ = 0;
    bool controlEnabled_ = false;
    std::atomic_bool actualPixelDisplayMode_{ false };
    QPoint actualPixelPanOffset_;
    QPointF actualPixelPanStartPosition_;
    QPoint actualPixelPanStartOffset_;
    bool actualPixelPanning_ = false;
    bool suppressPasteKeyRelease_ = false;
    QString connectionStage_ =
        QStringLiteral("正在建立远程会话");
    std::unique_ptr<HighResolutionPointerMoveScheduler>
        pointerMoveScheduler_;
    std::uint32_t pointerMoveRateLimitHz_ = 120;
    std::uint32_t dragPointerSampleRateHz_ = 240;
    HWND pointerSamplingWindow_ = nullptr;
    std::atomic<int> pointerSourceWidth_{0};
    std::atomic<int> pointerSourceHeight_{0};
    std::atomic_bool pointerStretchToCanvas_{false};
    std::uint32_t remoteDisplayId_ = 0;
    std::uint64_t remoteDisplayLayoutVersion_ = 0;
    bool nativeRenderingEnabled_ = true;
    D3D11VideoSurface* nativeSurface_ = nullptr;
    std::unique_ptr<RemoteCursorRenderState> cursorRenderState_;
    QSize nativeFrameSize_;
    bool nativeFrameActive_ = false;
    std::atomic<std::uint64_t> receivedFrameSequence_{ 0 };
    std::mutex cpuFrameMutex_;
    std::condition_variable cpuFrameCondition_;
    webrtc::scoped_refptr<webrtc::VideoFrameBuffer> pendingCpuFrame_;
    std::uint64_t pendingCpuSequence_ = 0;
    std::atomic<std::uint32_t> cpuConversionIntervalUs_{ 16'667 };
    std::atomic<int> cpuCanvasWidth_{ 0 };
    std::atomic<int> cpuCanvasHeight_{ 0 };
    // QWidget geometry uses device-independent pixels. The CPU
    // renderer needs the screen DPR because it pre-scales frames
    // before handing them back to the UI thread.
    std::atomic<int> cpuCanvasDevicePixelRatioMilli_{ 1000 };
    std::jthread cpuConversionThread_;
    std::mutex cpuImageMutex_;
    QImage pendingCpuImage_;
    std::uint64_t pendingCpuImageSequence_ = 0;
    QImage recycledCpuImage_;
    bool cpuImageDispatchPending_ = false;
    std::mutex nativeFrameMutex_;
    std::condition_variable nativeFrameCondition_;
    webrtc::scoped_refptr<webrtc::VideoFrameBuffer> pendingNativeFrame_;
    std::uint64_t pendingNativeSequence_ = 0;
    QImage pendingD3D11CpuImage_;
    std::uint64_t pendingD3D11CpuSequence_ = 0;
    bool nativeRedrawRequested_ = false;
    bool nativeResetRequested_ = false;
    std::atomic<std::uint32_t> nativePresentationIntervalUs_{ 16'667 };
    std::jthread nativePresentationThread_;
    std::atomic<int> nativeFrameWidth_{ 0 };
    std::atomic<int> nativeFrameHeight_{ 0 };
    std::atomic<bool> nativeUiActiveRequested_{ false };
    std::atomic<bool> nativeSurfaceReady_{ false };
    std::atomic<bool> nativeUiDispatchPending_{ false };
    std::atomic<std::uint64_t> nativeUiRevision_{ 0 };
    std::atomic<bool> cpuD3D11PresentationFailed_{ false };
    std::atomic<bool> cpuNv12D3D11PresentationFailed_{ false };
    std::atomic<bool> cpuI420D3D11PresentationFailed_{ false };
};

}  // namespace remote::controller
