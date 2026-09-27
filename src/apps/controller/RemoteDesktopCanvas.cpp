// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RemoteDesktopCanvas.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <limits>
#include <utility>

#include <QApplication>
#include <QClipboard>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFocusEvent>
#include <QFont>
#include <QKeyEvent>
#include <QLinearGradient>
#include <QMetaObject>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QSettings>
#include <QSizePolicy>
#include <QUrl>
#include <QWheelEvent>
#include <QWindow>

#include "D3D11VideoSurface.h"
#include "RemoteCursorRenderState.h"
#include "RemoteInputDispatcher.h"
#include "api/video/i420_buffer.h"
#include "libyuv/convert_argb.h"
#include "libyuv/convert_from.h"
#include "libyuv/scale.h"
#include "rtc_base/time_utils.h"
#include "src/core/RemoteInputTelemetry.h"
#include "src/core/VideoPresentationTelemetry.h"
#include "src/platform/win/D3D11NativeFrameBuffer.h"

namespace remote::controller {
namespace {
constexpr std::array<std::uint32_t, 5> kSupportedDragPointerSampleRates = {
    60u, 80u, 120u, 170u, 240u};
}

RemoteDesktopCanvas::RemoteDesktopCanvas(InputSender inputSender,
    PasteSender pasteSender,
    FirstPresentationCallback firstPresentationCallback,
    std::string telemetryPeerDeviceId,
    QWidget* parent)
    : QWidget(parent),
    inputSender_(std::move(inputSender)),
    pasteSender_(std::move(pasteSender)),
    firstPresentationCallback_(
        std::move(firstPresentationCallback)),
    presentationTelemetryId_(
        VideoPresentationTelemetryRegistry::Instance().Register(
            std::move(telemetryPeerDeviceId))),
    nativeRenderingEnabled_(
        QSettings().value(
            QStringLiteral("media/videoRendererPreference"),
            QStringLiteral("auto")).toString() !=
        QStringLiteral("cpu"))
{
    setMinimumSize(720, 430);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::ArrowCursor);
    setAttribute(Qt::WA_NativeWindow);
    pointerSamplingWindow_ =
        reinterpret_cast<HWND>(winId());

    pointerMoveScheduler_ =
        std::make_unique<
            HighResolutionPointerMoveScheduler>(
            inputSender_,
            [this](const RemoteInputEvent& fallback) {
                return SampleCurrentDragPointer(
                    fallback);
            });
    SetTargetFrameRate(30);

    nativeSurface_ = new D3D11VideoSurface(this);
    nativeSurface_->hide();
    cursorRenderState_ =
        std::make_unique<RemoteCursorRenderState>();
    UpdateCpuCanvasMetrics();
    UpdateCpuConversionInterval();

    // Window moving/resizing enters a Windows modal loop on the
    // Qt GUI thread. A QTimer-driven Present loop therefore loses
    // most frames while any RemoteC window is dragged. Keep D3D11
    // presentation on a persistent worker that consumes the same
    // latest-frame mailbox independently of the GUI event loop.
    nativePresentationThread_ = std::jthread(
        [this](std::stop_token stopToken) {
            NativePresentationLoop(stopToken);
        });

    // FFmpeg produces CPU I420 frames. Converting a 2K frame to
    // ARGB on the Qt thread allocates and writes roughly 14 MiB
    // before every paint, which starves mouse/window events at a
    // high frame rate. A single persistent worker converts only
    // the newest frame at the local display cadence.
    cpuConversionThread_ = std::jthread(
        [this](std::stop_token stopToken) {
            CpuConversionLoop(stopToken);
        });
}

RemoteDesktopCanvas::~RemoteDesktopCanvas()
{
    ShutdownInputScheduler();
    nativePresentationThread_.request_stop();
    nativeFrameCondition_.notify_all();
    if (nativePresentationThread_.joinable()) {
        nativePresentationThread_.join();
    }
    cpuConversionThread_.request_stop();
    cpuFrameCondition_.notify_all();
    if (cpuConversionThread_.joinable()) {
        cpuConversionThread_.join();
    }
    VideoPresentationTelemetryRegistry::Instance().Unregister(
        presentationTelemetryId_);
}

void RemoteDesktopCanvas::SetControlEnabled(bool enabled)
{
    const bool changed = controlEnabled_ != enabled;
    controlEnabled_ = enabled;
    // The responsive pointer is composed from the independent
    // cursor stream. Hide the local QWidget cursor only while
    // remote input is active to avoid a double cursor.
    setCursor(actualPixelPanning_
        ? Qt::ClosedHandCursor
        : (enabled ? Qt::BlankCursor : Qt::ArrowCursor));
    if (cursorRenderState_) {
        cursorRenderState_->SetRenderingEnabled(enabled);
    }
    if (!enabled) {
        CancelPendingPointerMove();
        clearFocus();
    }
    if (changed) {
        CursorVisualChanged();
    }
}

void RemoteDesktopCanvas::SetActualPixelDisplayMode(bool enabled)
{
    if (actualPixelDisplayMode_.exchange(
            enabled, std::memory_order_acq_rel) == enabled) {
        return;
    }
    actualPixelPanning_ = false;
    actualPixelPanOffset_ = {};
    setCursor(controlEnabled_
        ? Qt::BlankCursor
        : Qt::ArrowCursor);
    // Full source resolution is retained in 100% mode; fit mode
    // returns to the existing worker-side pre-scale path.
    UpdateCpuCanvasMetrics();
    UpdateNativeSurfaceGeometry();
    RequestNativeRedraw();
    update();
}

bool RemoteDesktopCanvas::ActualPixelDisplayMode() const
{
    return actualPixelDisplayMode_.load(
        std::memory_order_acquire);
}

void RemoteDesktopCanvas::ApplyRemoteCursor(const RemoteCursorEnvelope& envelope)
{
    if (!cursorRenderState_) return;
    switch (envelope.type) {
    case RemoteCursorMessageType::kShape:
        cursorRenderState_->SetShape(envelope.shape);
        break;
    case RemoteCursorMessageType::kPosition:
        if (envelope.position.displayId == remoteDisplayId_ &&
            envelope.position.displayLayoutVersion ==
                remoteDisplayLayoutVersion_) {
            cursorRenderState_->ApplyRemotePosition(
                envelope.position);
        }
        break;
    case RemoteCursorMessageType::kReset:
        cursorRenderState_->Reset();
        break;
    }
    CursorVisualChanged();
}

void RemoteDesktopCanvas::ResetRemoteCursor()
{
    if (!cursorRenderState_) return;
    cursorRenderState_->Reset();
    CursorVisualChanged();
}

void RemoteDesktopCanvas::SetConnectionStage(QString stage)
{
    stage = stage.trimmed();
    if (stage.isEmpty()) {
        stage = QStringLiteral("正在建立远程会话");
    }
    if (connectionStage_ == stage) {
        return;
    }
    connectionStage_ = std::move(stage);
    update();
}

void RemoteDesktopCanvas::BeginPresentationGeneration(std::uint64_t generation)
{
    presentationGeneration_.store(
        generation, std::memory_order_release);
    firstPresentationNotified_.store(
        false, std::memory_order_release);
}

void RemoteDesktopCanvas::NotifyFirstPresentation()
{
    const std::uint64_t generation =
        presentationGeneration_.load(
            std::memory_order_acquire);
    if (generation == 0 ||
        firstPresentationNotified_.exchange(
            true, std::memory_order_acq_rel)) {
        return;
    }
    if (firstPresentationCallback_) {
        firstPresentationCallback_(generation);
    }
}

void RemoteDesktopCanvas::SetTargetFrameRate(std::uint32_t framesPerSecond)
{
    // Input is sampled at roughly twice the visible frame rate,
    // with a useful 120-Hz floor and a bounded 240-Hz ceiling.
    // Button-held dragging always uses the 240-Hz ceiling.
    pointerMoveRateLimitHz_ = std::clamp(
        framesPerSecond * 2u, 120u, 240u);
    RemoteInputTelemetry::Instance()
        .SetMoveDispatchRateLimit(pointerMoveRateLimitHz_);
}

void RemoteDesktopCanvas::SetDragPointerSampleRate(std::uint32_t hertz)
{
    if (std::find(
            kSupportedDragPointerSampleRates.begin(),
            kSupportedDragPointerSampleRates.end(),
            hertz) ==
        kSupportedDragPointerSampleRates.end()) {
        hertz = 240;
    }
    dragPointerSampleRateHz_ = hertz;
    if (pointerMoveScheduler_) {
        pointerMoveScheduler_
            ->SetActiveDragSampleRate(hertz);
    }
}

void RemoteDesktopCanvas::ShutdownInputScheduler()
{
    if (pointerMoveScheduler_) {
        pointerMoveScheduler_->Shutdown();
    }
}

void RemoteDesktopCanvas::SetRemoteDisplayIdentity(
    std::uint32_t displayId,
    std::uint64_t layoutVersion)
{
    if (remoteDisplayId_ == displayId &&
        remoteDisplayLayoutVersion_ == layoutVersion) {
        return;
    }
    ReleaseRemoteInputs();
    remoteDisplayId_ = displayId;
    remoteDisplayLayoutVersion_ = layoutVersion;
    ResetRemoteCursor();
}

void RemoteDesktopCanvas::ReleaseRemoteInputs()
{
    CancelPendingPointerMove();
    if (!controlEnabled_ || !inputSender_) {
        return;
    }
    RemoteInputEvent input;
    input.type = RemoteInputMessageType::kReleaseAll;
    (void)inputSender_(input);
}

void RemoteDesktopCanvas::OnFrame(const webrtc::VideoFrame& frame)
{
    std::optional<std::int64_t> latestPacketReceiveUs;
    for (const auto& packetInfo : frame.packet_infos()) {
        const auto receiveTime = packetInfo.receive_time();
        if (!receiveTime.IsFinite()) {
            continue;
        }
        const std::int64_t receiveUs = receiveTime.us();
        if (!latestPacketReceiveUs ||
            receiveUs > *latestPacketReceiveUs) {
            latestPacketReceiveUs = receiveUs;
        }
    }
    if (latestPacketReceiveUs) {
        const std::int64_t nowUs = webrtc::TimeMicros();
        if (nowUs >= *latestPacketReceiveUs) {
            VideoPresentationTelemetryRegistry::Instance()
                .RecordReceiverPipeline(
                    presentationTelemetryId_,
                    static_cast<std::uint64_t>(
                        nowUs - *latestPacketReceiveUs));
        }
    }
    const auto source = frame.video_frame_buffer();
    if (source) {
        pointerSourceWidth_.store(
            source->width(), std::memory_order_release);
        pointerSourceHeight_.store(
            source->height(), std::memory_order_release);
    }
    if (nativeRenderingEnabled_ &&
        D3D11NativeFrameBuffer::From(source.get())) {
        QueueNativeFrame(source);
        return;
    }
    if (source) {
        if (nativeRenderingEnabled_ &&
            !cpuI420D3D11PresentationFailed_.load(
                std::memory_order_acquire)) {
            QueueI420Frame(source);
        }
        else {
            QueueCpuFrame(source);
        }
    }
}

void RemoteDesktopCanvas::mouseMoveEvent(QMouseEvent* event)
{
    if (actualPixelPanning_) {
        const QPoint delta =
            (event->position() -
                actualPixelPanStartPosition_).toPoint();
        actualPixelPanOffset_ =
            actualPixelPanStartOffset_ + delta;
        UpdateNativeSurfaceGeometry();
        RequestNativeRedraw();
        update();
        event->accept();
        return;
    }
    const auto point = MapPoint(
        event->position(), event->buttons() != Qt::NoButton);
    if (point && QueuePointerMove(
        *point, MapMouseButtons(event->buttons()))) {
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void RemoteDesktopCanvas::mousePressEvent(QMouseEvent* event)
{
    if (ActualPixelDisplayMode() &&
        event->button() == Qt::LeftButton &&
        event->modifiers().testFlag(Qt::AltModifier)) {
        ReleaseRemoteInputs();
        actualPixelPanning_ = true;
        actualPixelPanStartPosition_ = event->position();
        actualPixelPanStartOffset_ = actualPixelPanOffset_;
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    const auto button = MapMouseButton(event->button());
    const auto point = MapPoint(event->position(), false);
    if (button && point && controlEnabled_) {
        FlushPendingPointerMove();
        setFocus(Qt::MouseFocusReason);
        RemoteInputEvent input;
        input.type = RemoteInputMessageType::kMouseButton;
        input.displayId = remoteDisplayId_;
        input.displayLayoutVersion =
            remoteDisplayLayoutVersion_;
        input.normalizedX = point->first;
        input.normalizedY = point->second;
        input.mouseButton = *button;
        input.pressed = true;
        input.pressedMouseButtons =
            MapMouseButtons(event->buttons());
        RemoteInputTelemetry::Instance()
            .RecordGeneratedMouseButton();
        if (inputSender_ && inputSender_(input)) {
            if (pointerMoveScheduler_) {
                RemoteInputEvent dragInput = input;
                dragInput.type =
                    RemoteInputMessageType::kMouseMove;
                pointerMoveScheduler_->BeginDrag(
                    dragInput,
                    dragPointerSampleRateHz_);
            }
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void RemoteDesktopCanvas::mouseReleaseEvent(QMouseEvent* event)
{
    if (actualPixelPanning_ &&
        event->button() == Qt::LeftButton) {
        actualPixelPanning_ = false;
        setCursor(controlEnabled_
            ? Qt::BlankCursor
            : Qt::ArrowCursor);
        event->accept();
        return;
    }
    const auto button = MapMouseButton(event->button());
    const auto point = MapPoint(event->position(), true);
    if (button && point && controlEnabled_) {
        FlushPendingPointerMove();
        if (pointerMoveScheduler_) {
            pointerMoveScheduler_->EndDrag();
        }
        RemoteInputEvent input;
        input.type = RemoteInputMessageType::kMouseButton;
        input.displayId = remoteDisplayId_;
        input.displayLayoutVersion =
            remoteDisplayLayoutVersion_;
        input.normalizedX = point->first;
        input.normalizedY = point->second;
        input.mouseButton = *button;
        input.pressed = false;
        input.pressedMouseButtons =
            MapMouseButtons(event->buttons());
        RemoteInputTelemetry::Instance()
            .RecordGeneratedMouseButton();
        if (inputSender_ && inputSender_(input)) {
            if (pointerMoveScheduler_ &&
                input.pressedMouseButtons != 0) {
                RemoteInputEvent dragInput = input;
                dragInput.type =
                    RemoteInputMessageType::kMouseMove;
                pointerMoveScheduler_->BeginDrag(
                    dragInput,
                    dragPointerSampleRateHz_);
            }
            event->accept();
            return;
        }
    }
    QWidget::mouseReleaseEvent(event);
}

void RemoteDesktopCanvas::wheelEvent(QWheelEvent* event)
{
    const auto point = MapPoint(event->position(), false);
    if (point && controlEnabled_) {
        FlushPendingPointerMove();
        const QPoint delta = event->angleDelta();
        RemoteInputEvent input;
        input.type = RemoteInputMessageType::kMouseWheel;
        input.displayId = remoteDisplayId_;
        input.displayLayoutVersion =
            remoteDisplayLayoutVersion_;
        input.normalizedX = point->first;
        input.normalizedY = point->second;
        input.wheelDeltaX = ClampWheelDelta(delta.x());
        input.wheelDeltaY = ClampWheelDelta(delta.y());
        input.pressedMouseButtons =
            MapMouseButtons(event->buttons());
        if ((input.wheelDeltaX != 0 || input.wheelDeltaY != 0) &&
            inputSender_) {
            RemoteInputTelemetry::Instance()
                .RecordGeneratedMouseWheel();
            if (inputSender_(input)) {
                event->accept();
                return;
            }
        }
    }
    QWidget::wheelEvent(event);
}

void RemoteDesktopCanvas::keyPressEvent(QKeyEvent* event)
{
    if (controlEnabled_ && !event->isAutoRepeat() &&
        event->key() == Qt::Key_V &&
        event->modifiers().testFlag(Qt::ControlModifier) &&
        !event->modifiers().testFlag(Qt::ShiftModifier) &&
        pasteSender_) {
        QStringList clipboardFiles;
        if (const QClipboard* clipboard =
                QApplication::clipboard()) {
            if (const QMimeData* mimeData =
                    clipboard->mimeData(
                        QClipboard::Clipboard);
                mimeData && mimeData->hasUrls()) {
                for (const QUrl& url : mimeData->urls()) {
                    if (!url.isLocalFile()) continue;
                    const QString path = url.toLocalFile();
                    if (!path.isEmpty()) {
                        clipboardFiles.push_back(path);
                    }
                }
            }
        }
        // File URLs are captured synchronously on the UI thread.
        // This remains stable across repeated Ctrl+V even when an
        // RDP/delayed IDataObject changes its native formats after
        // the first read. Text and virtual formats still use the
        // one-shot native clipboard service below.
        if (pasteSender_(clipboardFiles, true)) {
            // Ctrl may already have reached the controlled
            // machine. Release it before the asynchronous
            // transfer; a fresh Ctrl+V is injected only after the
            // remote clipboard is ready.
            ReleaseRemoteInputs();
            suppressPasteKeyRelease_ = true;
            event->accept();
            return;
        }
    }
    if (SendKeyEvent(event, true)) {
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void RemoteDesktopCanvas::keyReleaseEvent(QKeyEvent* event)
{
    if (suppressPasteKeyRelease_ &&
        event->key() == Qt::Key_V) {
        suppressPasteKeyRelease_ = false;
        event->accept();
        return;
    }
    // Qt may emit synthetic auto-repeat releases. The real release follows
    // with isAutoRepeat() == false, so sending the synthetic one would make
    // held keys flicker on the controlled machine.
    if (controlEnabled_ && event->isAutoRepeat()) {
        event->accept();
        return;
    }
    if (SendKeyEvent(event, false)) {
        event->accept();
        return;
    }
    QWidget::keyReleaseEvent(event);
}

void RemoteDesktopCanvas::dragEnterEvent(QDragEnterEvent* event)
{
    if (controlEnabled_ && pasteSender_ && event->mimeData() &&
        event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        const bool hasLocalFile = std::any_of(
            urls.begin(), urls.end(),
            [](const QUrl& url) { return url.isLocalFile(); });
        if (hasLocalFile) {
            event->setDropAction(Qt::CopyAction);
            event->accept();
            return;
        }
    }
    QWidget::dragEnterEvent(event);
}

void RemoteDesktopCanvas::dropEvent(QDropEvent* event)
{
    QStringList files;
    if (event->mimeData()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl& url : urls) {
            if (url.isLocalFile() && !url.toLocalFile().isEmpty()) {
                files.push_back(url.toLocalFile());
            }
        }
    }
    if (!files.isEmpty() && controlEnabled_ && pasteSender_) {
        if (pasteSender_(files, false)) {
            FocusRemotePasteTarget(event->position());
            event->setDropAction(Qt::CopyAction);
            event->accept();
            return;
        }
    }
    QWidget::dropEvent(event);
}

void RemoteDesktopCanvas::focusOutEvent(QFocusEvent* event)
{
    actualPixelPanning_ = false;
    ReleaseRemoteInputs();
    setCursor(controlEnabled_
        ? Qt::BlankCursor
        : Qt::ArrowCursor);
    QWidget::focusOutEvent(event);
}

void RemoteDesktopCanvas::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    UpdateCpuCanvasMetrics();
    pointerStretchToCanvas_.store(
        false,
        std::memory_order_release);
    UpdateCpuConversionInterval();
    UpdateNativeSurfaceGeometry();
    RequestNativeRedraw();
}

bool RemoteDesktopCanvas::event(QEvent* event)
{
                const bool handled = QWidget::event(event);
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
                if (event &&
                    event->type() == QEvent::DevicePixelRatioChange) {
                    // A cross-monitor move can change DPR without changing
                    // the QWidget's logical dimensions.
                    UpdateCpuCanvasMetrics();
                }
#endif
                return handled;
            }

void RemoteDesktopCanvas::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (nativeFrameActive_) {
        painter.fillRect(rect(), Qt::black);
        return;
    }
    if (!frameImage_.isNull()) {
        painter.fillRect(rect(), Qt::black);
        const QRect target = VideoContentRect();
        // The CPU worker normally hands Qt an image already sized
        // for this content rectangle. Avoid enabling the costly
        // painter scaling path for the steady-state 1:1 blit.
        painter.setRenderHint(
            QPainter::SmoothPixmapTransform,
            target.size() !=
                frameImage_.deviceIndependentSize().toSize());
        const auto drawStartedAt =
            std::chrono::steady_clock::now();
        painter.drawImage(target, frameImage_);
        if (cursorRenderState_) {
            const QSize cursorSourceSize(
                pointerSourceWidth_.load(
                    std::memory_order_acquire),
                pointerSourceHeight_.load(
                    std::memory_order_acquire));
            cursorRenderState_->Paint(
                painter, target,
                cursorSourceSize.isValid()
                    ? cursorSourceSize
                    : frameImage_.size());
        }
        if (frameImageSequence_ != 0 &&
            frameImageSequence_ != lastPresentedCpuSequence_) {
            const auto drawUs = static_cast<std::uint64_t>(
                std::chrono::duration_cast<
                    std::chrono::microseconds>(
                    std::chrono::steady_clock::now() -
                    drawStartedAt)
                    .count());
            lastPresentedCpuSequence_ = frameImageSequence_;
            VideoPresentationTelemetryRegistry::Instance()
                .RecordPresented(presentationTelemetryId_,
                    VideoPresentationPath::kCpuQt,
                    drawUs, 0);
            NotifyFirstPresentation();
        }
        return;
    }

    QLinearGradient background(0, 0, width(), height());
    background.setColorAt(0.0, QColor(24, 49, 91));
    background.setColorAt(0.52, QColor(34, 79, 139));
    background.setColorAt(1.0, QColor(16, 30, 56));
    painter.fillRect(rect(), background);

    QPainterPath glow;
    glow.addEllipse(QPointF(width() * 0.73, height() * 0.38),
        width() * 0.29, height() * 0.48);
    QLinearGradient glowGradient(width() * 0.5, height() * 0.2,
        width(), height() * 0.75);
    glowGradient.setColorAt(0.0, QColor(96, 143, 255, 155));
    glowGradient.setColorAt(1.0, QColor(82, 93, 217, 15));
    painter.fillPath(glow, glowGradient);

    painter.setPen(QPen(QColor(255, 255, 255, 25), 1));
    for (int x = -height(); x < width(); x += 84) {
        painter.drawLine(x, height(), x + height(), 0);
    }

    const int taskbarHeight = 42;
    painter.fillRect(0, height() - taskbarHeight, width(), taskbarHeight,
        QColor(9, 17, 30, 220));
    painter.setBrush(QColor(90, 108, 242));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(width() / 2 - 58, height() - 32, 22, 22, 5, 5);
    painter.setBrush(QColor(232, 238, 248, 210));
    painter.drawEllipse(width() / 2 - 25, height() - 30, 18, 18);
    painter.drawRoundedRect(width() / 2 + 5, height() - 30, 24, 18, 4, 4);
    painter.setBrush(QColor(232, 238, 248, 170));
    painter.drawRoundedRect(width() / 2 + 42, height() - 30, 20, 18, 4, 4);

    QRectF overlay(24, 22, 190, 34);
    painter.setBrush(QColor(8, 14, 25, 150));
    painter.setPen(QPen(QColor(255, 255, 255, 30)));
    painter.drawRoundedRect(overlay, 8, 8);
    painter.setPen(QColor(221, 229, 242));
    QFont displayFont = QApplication::font();
    displayFont.setPixelSize(12);
    displayFont.setWeight(QFont::Medium);
    painter.setFont(displayFont);
    painter.drawText(overlay.adjusted(12, 0, -10, 0), Qt::AlignVCenter,
        QStringLiteral("远程画面  ·  连接中"));

    const int panelWidth = qMin(480, width() - 80);
    QRectF emptyState((width() - panelWidth) / 2.0,
        (height() - 155) / 2.0 - 12, panelWidth, 155);
    painter.setBrush(QColor(8, 16, 30, 165));
    painter.setPen(QPen(QColor(255, 255, 255, 32)));
    painter.drawRoundedRect(emptyState, 16, 16);

    painter.setPen(QColor(245, 248, 253));
    QFont previewTitleFont = QApplication::font();
    previewTitleFont.setPixelSize(21);
    previewTitleFont.setWeight(QFont::DemiBold);
    painter.setFont(previewTitleFont);
    painter.drawText(emptyState.adjusted(25, 28, -25, -70),
        Qt::AlignHCenter | Qt::AlignTop,
        QStringLiteral("正在连接中"));
    painter.setPen(QColor(167, 181, 203));
    QFont previewHintFont = QApplication::font();
    previewHintFont.setPixelSize(13);
    painter.setFont(previewHintFont);
    painter.drawText(emptyState.adjusted(34, 75, -34, -24),
        Qt::AlignHCenter | Qt::AlignTop,
        connectionStage_);
}

bool RemoteDesktopCanvas::IsCpuNv12Image(const QImage& image)
{
    return !image.isNull() &&
        image.format() == QImage::Format_Grayscale8 &&
        image.height() > 0 && image.height() % 3 == 0;
}

void RemoteDesktopCanvas::UpdateCpuCanvasMetrics()
{
    cpuCanvasWidth_.store(width(), std::memory_order_release);
    cpuCanvasHeight_.store(height(), std::memory_order_release);
    cpuCanvasDevicePixelRatioMilli_.store(
        (std::clamp)(static_cast<int>(std::lround(
            devicePixelRatioF() * 1000.0)), 500, 8000),
        std::memory_order_release);
}

QSize RemoteDesktopCanvas::CpuQtOutputSize(int sourceWidth, int sourceHeight) const
{
    // At 100%, retain every decoded source pixel. QImage's DPR
    // maps these physical pixels to the correct logical QWidget
    // size without any resampling.
    if (actualPixelDisplayMode_.load(
            std::memory_order_acquire)) {
        return QSize(sourceWidth, sourceHeight);
    }
    const int logicalCanvasWidth = cpuCanvasWidth_.load(
        std::memory_order_acquire);
    const int logicalCanvasHeight = cpuCanvasHeight_.load(
        std::memory_order_acquire);
    const int dprMilli = (std::clamp)(
        cpuCanvasDevicePixelRatioMilli_.load(
            std::memory_order_acquire), 500, 8000);
    const int canvasWidth = static_cast<int>(
        (static_cast<std::int64_t>(logicalCanvasWidth) *
            dprMilli + 500) / 1000);
    const int canvasHeight = static_cast<int>(
        (static_cast<std::int64_t>(logicalCanvasHeight) *
            dprMilli + 500) / 1000);
    if (sourceWidth <= 0 || sourceHeight <= 0 ||
        canvasWidth <= 0 || canvasHeight <= 0) {
        return QSize(sourceWidth, sourceHeight);
    }

    int outputWidth = canvasWidth;
    int outputHeight = canvasHeight;
    if (!pointerStretchToCanvas_.load(
            std::memory_order_acquire)) {
        const std::int64_t widthLimitedHeight =
            static_cast<std::int64_t>(canvasWidth) *
            sourceHeight / sourceWidth;
        if (widthLimitedHeight <= canvasHeight) {
            outputHeight = static_cast<int>(widthLimitedHeight);
        }
        else {
            outputWidth = static_cast<int>(
                static_cast<std::int64_t>(canvasHeight) *
                sourceWidth / sourceHeight);
        }
    }

    // I420 chroma planes are most efficient and least surprising
    // when both dimensions are even. The Qt canvas is never
    // smaller than two pixels in normal operation, but keep the
    // fallback valid during transient layout changes as well.
    outputWidth = (std::max)(2, outputWidth & ~1);
    outputHeight = (std::max)(2, outputHeight & ~1);
    return QSize(outputWidth, outputHeight);
}

void RemoteDesktopCanvas::QueueCpuFrame(
    webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame,
    bool recordArrival)
{
    if (recordArrival) {
        const bool d3d11Available =
            nativeRenderingEnabled_ &&
            !cpuD3D11PresentationFailed_.load(
                std::memory_order_acquire);
        VideoPresentationTelemetryRegistry::Instance()
            .RecordArrival(presentationTelemetryId_,
                !d3d11Available
                    ? VideoPresentationPath::kCpuQt
                    : (!cpuNv12D3D11PresentationFailed_.load(
                            std::memory_order_acquire)
                        ? VideoPresentationPath::kCpuNv12D3D11
                        : VideoPresentationPath::kCpuD3D11));
    }
    const std::uint64_t sequence =
        receivedFrameSequence_.fetch_add(1, std::memory_order_acq_rel) + 1;
    bool superseded = false;
    {
        std::lock_guard lock(cpuFrameMutex_);
        superseded = pendingCpuFrame_ != nullptr;
        pendingCpuFrame_ = std::move(frame);
        pendingCpuSequence_ = sequence;
    }
    if (superseded) {
        VideoPresentationTelemetryRegistry::Instance()
            .RecordSuperseded(presentationTelemetryId_);
    }
    cpuFrameCondition_.notify_one();
}

void RemoteDesktopCanvas::CpuConversionLoop(std::stop_token stopToken)
{
    using Clock = std::chrono::steady_clock;
    SetThreadDescription(
        GetCurrentThread(), L"RemoteC CPU Frame Conversion");
    SetThreadPriority(
        GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
    Clock::time_point nextConversionAt{};
    bool hasDeadline = false;
    webrtc::scoped_refptr<webrtc::I420Buffer>
        scaledI420Buffer;

    while (!stopToken.stop_requested()) {
        webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame;
        std::uint64_t sequence = 0;
        {
            std::unique_lock lock(cpuFrameMutex_);
            cpuFrameCondition_.wait(lock, [this, &stopToken] {
                return stopToken.stop_requested() ||
                    pendingCpuFrame_ != nullptr;
            });
            if (stopToken.stop_requested()) {
                break;
            }

            if (hasDeadline) {
                cpuFrameCondition_.wait_until(
                    lock, nextConversionAt, [&stopToken] {
                        return stopToken.stop_requested();
                    });
                if (stopToken.stop_requested()) {
                    break;
                }
            }

            // Frames that arrived while waiting replaced the old
            // mailbox value. Only the frame that can actually be
            // shown at this display deadline is converted.
            frame = std::move(pendingCpuFrame_);
            sequence = pendingCpuSequence_;
            pendingCpuSequence_ = 0;
        }

        const auto interval = std::chrono::microseconds(
            cpuConversionIntervalUs_.load(
                std::memory_order_acquire));
        const auto now = Clock::now();
        if (!hasDeadline || now > nextConversionAt + interval) {
            nextConversionAt = now + interval;
            hasDeadline = true;
        }
        else {
            nextConversionAt += interval;
        }

        if (!frame || sequence != receivedFrameSequence_.load(
                std::memory_order_acquire)) {
            if (frame) {
                VideoPresentationTelemetryRegistry::Instance()
                    .RecordSuperseded(presentationTelemetryId_);
            }
            continue;
        }
        const auto conversionStartedAt = Clock::now();
        const auto sourceI420 = frame->ToI420();
        if (!sourceI420) {
            VideoPresentationTelemetryRegistry::Instance()
                .RecordPresentFailure(presentationTelemetryId_);
            continue;
        }

        const bool useNv12GpuPath =
            nativeRenderingEnabled_ &&
            (sourceI420->width() & 1) == 0 &&
            (sourceI420->height() & 1) == 0 &&
            !cpuNv12D3D11PresentationFailed_.load(
                std::memory_order_acquire) &&
            !cpuD3D11PresentationFailed_.load(
                std::memory_order_acquire);
        const bool useQtCpuPath =
            !nativeRenderingEnabled_ ||
            cpuD3D11PresentationFailed_.load(
                std::memory_order_acquire);
        webrtc::scoped_refptr<webrtc::I420BufferInterface> i420 =
            sourceI420;
        if (!useNv12GpuPath && useQtCpuPath) {
            const QSize outputSize = CpuQtOutputSize(
                sourceI420->width(), sourceI420->height());
            if (outputSize.width() != sourceI420->width() ||
                outputSize.height() != sourceI420->height()) {
                if (!scaledI420Buffer ||
                    scaledI420Buffer->width() !=
                        outputSize.width() ||
                    scaledI420Buffer->height() !=
                        outputSize.height()) {
                    scaledI420Buffer =
                        webrtc::I420Buffer::Create(
                            outputSize.width(),
                            outputSize.height());
                }
                if (!scaledI420Buffer || libyuv::I420Scale(
                        sourceI420->DataY(), sourceI420->StrideY(),
                        sourceI420->DataU(), sourceI420->StrideU(),
                        sourceI420->DataV(), sourceI420->StrideV(),
                        sourceI420->width(), sourceI420->height(),
                        scaledI420Buffer->MutableDataY(),
                        scaledI420Buffer->StrideY(),
                        scaledI420Buffer->MutableDataU(),
                        scaledI420Buffer->StrideU(),
                        scaledI420Buffer->MutableDataV(),
                        scaledI420Buffer->StrideV(),
                        scaledI420Buffer->width(),
                        scaledI420Buffer->height(),
                        libyuv::kFilterBilinear) != 0) {
                    VideoPresentationTelemetryRegistry::Instance()
                        .RecordPresentFailure(
                            presentationTelemetryId_);
                    continue;
                }
                i420 = scaledI420Buffer;
            }
        }
        const QImage::Format targetFormat = useNv12GpuPath
            ? QImage::Format_Grayscale8
            : QImage::Format_ARGB32;
        const QSize targetStorageSize(
            i420->width(),
            useNv12GpuPath
                ? i420->height() + i420->height() / 2
                : i420->height());
        QImage image;
        {
            std::lock_guard lock(cpuImageMutex_);
            if (recycledCpuImage_.size() ==
                    targetStorageSize &&
                recycledCpuImage_.format() == targetFormat) {
                image = std::move(recycledCpuImage_);
            }
        }
        if (image.isNull()) {
            image = QImage(targetStorageSize, targetFormat);
        }
        // The storage size is in physical pixels. Recording DPR
        // keeps its device-independent size equal to the QWidget
        // content rectangle and prevents a second enlargement.
        image.setDevicePixelRatio(useQtCpuPath
            ? static_cast<qreal>((std::clamp)(
                cpuCanvasDevicePixelRatioMilli_.load(
                    std::memory_order_acquire),
                500, 8000)) / 1000.0
            : 1.0);
        int conversionResult = -1;
        if (!image.isNull()) {
            if (useNv12GpuPath) {
                const int rowPitch = image.bytesPerLine();
                conversionResult = libyuv::I420ToNV12(
                    i420->DataY(), i420->StrideY(),
                    i420->DataU(), i420->StrideU(),
                    i420->DataV(), i420->StrideV(),
                    image.bits(), rowPitch,
                    image.bits() +
                        rowPitch * i420->height(),
                    rowPitch,
                    i420->width(), i420->height());
            }
            else {
                conversionResult = libyuv::I420ToARGB(
                    i420->DataY(), i420->StrideY(),
                    i420->DataU(), i420->StrideU(),
                    i420->DataV(), i420->StrideV(),
                    image.bits(), image.bytesPerLine(),
                    i420->width(), i420->height());
            }
        }
        if (image.isNull() || conversionResult != 0 ||
            sequence != receivedFrameSequence_.load(
                std::memory_order_acquire)) {
            if (sequence != receivedFrameSequence_.load(
                    std::memory_order_acquire)) {
                VideoPresentationTelemetryRegistry::Instance()
                    .RecordSuperseded(presentationTelemetryId_);
            }
            else {
                VideoPresentationTelemetryRegistry::Instance()
                    .RecordPresentFailure(
                        presentationTelemetryId_);
            }
            RecycleCpuImage(std::move(image));
            continue;
        }
        const auto conversionUs = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(
                Clock::now() - conversionStartedAt)
                .count());
        VideoPresentationTelemetryRegistry::Instance()
            .RecordConversion(
                presentationTelemetryId_, conversionUs);
        QueueConvertedCpuImage(std::move(image), sequence);
    }
}

void RemoteDesktopCanvas::RecycleCpuImage(QImage image)
{
    if (image.isNull()) {
        return;
    }
    std::lock_guard lock(cpuImageMutex_);
    recycledCpuImage_ = std::move(image);
}

void RemoteDesktopCanvas::QueueConvertedCpuImage(QImage image, std::uint64_t sequence)
{
    if (nativeRenderingEnabled_ &&
        !cpuD3D11PresentationFailed_.load(
            std::memory_order_acquire)) {
        QueueD3D11CpuImage(std::move(image), sequence);
        return;
    }
    QueueQtCpuImage(std::move(image), sequence);
}

void RemoteDesktopCanvas::QueueD3D11CpuImage(QImage image, std::uint64_t sequence)
{
    const int frameWidth = image.width();
    // NV12 is stored in one owned byte buffer whose QImage height
    // covers both the Y plane and the half-height interleaved UV
    // plane. Geometry and aspect-ratio calculations must use the
    // actual video height rather than this 3/2 storage height.
    const int frameHeight = IsCpuNv12Image(image)
        ? image.height() * 2 / 3
        : image.height();
    bool superseded = false;
    QImage supersededCpuImage;
    {
        std::lock_guard lock(nativeFrameMutex_);
        superseded = pendingNativeFrame_ != nullptr ||
            !pendingD3D11CpuImage_.isNull();
        pendingNativeFrame_ = nullptr;
        pendingNativeSequence_ = 0;
        supersededCpuImage =
            std::move(pendingD3D11CpuImage_);
        pendingD3D11CpuImage_ = std::move(image);
        pendingD3D11CpuSequence_ = sequence;
    }
    if (superseded) {
        VideoPresentationTelemetryRegistry::Instance()
            .RecordSuperseded(presentationTelemetryId_);
    }
    RecycleCpuImage(std::move(supersededCpuImage));

    const int oldWidth = nativeFrameWidth_.exchange(
        frameWidth, std::memory_order_acq_rel);
    const int oldHeight = nativeFrameHeight_.exchange(
        frameHeight, std::memory_order_acq_rel);
    const bool firstActivation =
        !nativeUiActiveRequested_.exchange(
            true, std::memory_order_acq_rel);
    if (firstActivation || oldWidth != frameWidth ||
        oldHeight != frameHeight) {
        ScheduleNativeUiUpdate();
    }
    nativeFrameCondition_.notify_one();
}

void RemoteDesktopCanvas::QueueQtCpuImage(QImage image, std::uint64_t sequence)
{
    bool dispatch = false;
    {
        std::lock_guard lock(cpuImageMutex_);
        if (!pendingCpuImage_.isNull()) {
            VideoPresentationTelemetryRegistry::Instance()
                .RecordSuperseded(presentationTelemetryId_);
            recycledCpuImage_ = std::move(pendingCpuImage_);
        }
        pendingCpuImage_ = std::move(image);
        pendingCpuImageSequence_ = sequence;
        if (!cpuImageDispatchPending_) {
            cpuImageDispatchPending_ = true;
            dispatch = true;
        }
    }
    if (dispatch) {
        QMetaObject::invokeMethod(
            this, [this] { ApplyPendingCpuImage(); },
            Qt::QueuedConnection);
    }
}

void RemoteDesktopCanvas::ApplyPendingCpuImage()
{
    QImage image;
    std::uint64_t sequence = 0;
    {
        std::lock_guard lock(cpuImageMutex_);
        image = std::move(pendingCpuImage_);
        sequence = pendingCpuImageSequence_;
        pendingCpuImageSequence_ = 0;
        cpuImageDispatchPending_ = false;
    }
    if (image.isNull() ||
        sequence != receivedFrameSequence_.load(
            std::memory_order_acquire)) {
        if (!image.isNull()) {
            VideoPresentationTelemetryRegistry::Instance()
                .RecordSuperseded(presentationTelemetryId_);
        }
        RecycleCpuImage(std::move(image));
        return;
    }

    if (!DeactivateNativePresentation(sequence)) {
        VideoPresentationTelemetryRegistry::Instance()
            .RecordSuperseded(presentationTelemetryId_);
        RecycleCpuImage(std::move(image));
        return;
    }
    nativeFrameSize_ = {};
    nativeFrameActive_ = false;
    if (nativeSurface_) {
        nativeSurface_->hide();
    }

    QImage previous = std::move(frameImage_);
    frameImage_ = std::move(image);
    frameImageSequence_ = sequence;
    RecycleCpuImage(std::move(previous));
    update();
}

void RemoteDesktopCanvas::QueueNativeFrame(
    webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame)
{
    auto* native = D3D11NativeFrameBuffer::From(frame.get());
    if (!native) {
        QueueCpuFrame(std::move(frame));
        return;
    }
    const int frameWidth = native->width();
    const int frameHeight = native->height();
    VideoPresentationTelemetryRegistry::Instance().RecordArrival(
        presentationTelemetryId_,
        VideoPresentationPath::kD3D11);
    const std::uint64_t sequence =
        receivedFrameSequence_.fetch_add(1, std::memory_order_acq_rel) + 1;
    bool superseded = false;
    QImage supersededCpuImage;
    {
        std::lock_guard lock(nativeFrameMutex_);
        superseded = pendingNativeFrame_ != nullptr ||
            !pendingD3D11CpuImage_.isNull();
        supersededCpuImage =
            std::move(pendingD3D11CpuImage_);
        pendingD3D11CpuSequence_ = 0;
        pendingNativeFrame_ = std::move(frame);
        pendingNativeSequence_ = sequence;
    }
    if (superseded) {
        VideoPresentationTelemetryRegistry::Instance()
            .RecordSuperseded(presentationTelemetryId_);
    }
    RecycleCpuImage(std::move(supersededCpuImage));

    const int oldWidth = nativeFrameWidth_.exchange(
        frameWidth, std::memory_order_acq_rel);
    const int oldHeight = nativeFrameHeight_.exchange(
        frameHeight, std::memory_order_acq_rel);
    const bool firstActivation =
        !nativeUiActiveRequested_.exchange(
            true, std::memory_order_acq_rel);
    if (firstActivation || oldWidth != frameWidth ||
        oldHeight != frameHeight) {
        ScheduleNativeUiUpdate();
    }
    nativeFrameCondition_.notify_one();
}

void RemoteDesktopCanvas::QueueI420Frame(
    webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame)
{
    if (!frame || frame->width() <= 0 || frame->height() <= 0) {
        return;
    }
    const int frameWidth = frame->width();
    const int frameHeight = frame->height();
    VideoPresentationTelemetryRegistry::Instance().RecordArrival(
        presentationTelemetryId_,
        VideoPresentationPath::kCpuI420D3D11);
    const std::uint64_t sequence =
        receivedFrameSequence_.fetch_add(
            1, std::memory_order_acq_rel) + 1;
    bool superseded = false;
    QImage supersededCpuImage;
    {
        std::lock_guard lock(nativeFrameMutex_);
        superseded = pendingNativeFrame_ != nullptr ||
            !pendingD3D11CpuImage_.isNull();
        supersededCpuImage =
            std::move(pendingD3D11CpuImage_);
        pendingD3D11CpuSequence_ = 0;
        pendingNativeFrame_ = std::move(frame);
        pendingNativeSequence_ = sequence;
    }
    if (superseded) {
        VideoPresentationTelemetryRegistry::Instance()
            .RecordSuperseded(presentationTelemetryId_);
    }
    RecycleCpuImage(std::move(supersededCpuImage));

    const int oldWidth = nativeFrameWidth_.exchange(
        frameWidth, std::memory_order_acq_rel);
    const int oldHeight = nativeFrameHeight_.exchange(
        frameHeight, std::memory_order_acq_rel);
    const bool firstActivation =
        !nativeUiActiveRequested_.exchange(
            true, std::memory_order_acq_rel);
    if (firstActivation || oldWidth != frameWidth ||
        oldHeight != frameHeight) {
        ScheduleNativeUiUpdate();
    }
    nativeFrameCondition_.notify_one();
}

void RemoteDesktopCanvas::ScheduleNativeUiUpdate()
{
    nativeUiRevision_.fetch_add(1, std::memory_order_acq_rel);
    if (nativeUiDispatchPending_.exchange(
            true, std::memory_order_acq_rel)) {
        return;
    }
    QMetaObject::invokeMethod(this, [this] {
        const std::uint64_t revision = nativeUiRevision_.load(
            std::memory_order_acquire);
        if (nativeUiActiveRequested_.load(
                std::memory_order_acquire)) {
            const int width = nativeFrameWidth_.load(
                std::memory_order_acquire);
            const int height = nativeFrameHeight_.load(
                std::memory_order_acquire);
            if (width > 0 && height > 0) {
                nativeFrameSize_ = QSize(width, height);
                nativeFrameActive_ = true;
                frameImage_ = {};
                UpdateNativeSurfaceGeometry();
                nativeSurface_->show();
                nativeSurface_->raise();
                nativeSurfaceReady_.store(
                    true, std::memory_order_release);
                nativeFrameCondition_.notify_one();
                update();
            }
        }
        nativeUiDispatchPending_.store(
            false, std::memory_order_release);
        if (revision != nativeUiRevision_.load(
                std::memory_order_acquire)) {
            ScheduleNativeUiUpdate();
        }
    }, Qt::QueuedConnection);
}

void RemoteDesktopCanvas::UpdateCpuConversionInterval()
{
    const QScreen* currentScreen = screen();
    const qreal reportedRefreshRate = currentScreen
        ? currentScreen->refreshRate()
        : 60.0;
    const qreal refreshRate = (std::clamp)(
        reportedRefreshRate, 30.0, 240.0);
    VideoPresentationTelemetryRegistry::Instance()
        .SetLocalRefreshRate(
            presentationTelemetryId_, reportedRefreshRate);
    cpuConversionIntervalUs_.store(
        static_cast<std::uint32_t>((std::max)(
            1.0, std::round(1'000'000.0 / refreshRate))),
        std::memory_order_release);
    nativePresentationIntervalUs_.store(
        static_cast<std::uint32_t>((std::max)(
            1.0, std::round(1'000'000.0 / refreshRate))),
        std::memory_order_release);
}

void RemoteDesktopCanvas::RequestNativeRedraw()
{
    if (!nativeUiActiveRequested_.load(
            std::memory_order_acquire)) {
        return;
    }
    {
        std::lock_guard lock(nativeFrameMutex_);
        nativeRedrawRequested_ = true;
    }
    nativeFrameCondition_.notify_one();
}

void RemoteDesktopCanvas::CursorVisualChanged()
{
    // Qt painting reuses frameImage_; D3D11 presentation reuses
    // the retained last frame. No video frame or extra cursor
    // window is required for a pointer-only update.
    update();
    RequestNativeRedraw();
}

bool RemoteDesktopCanvas::DeactivateNativePresentation(std::uint64_t cpuSequence)
{
    QImage retiredCpuImage;
    {
        std::lock_guard lock(nativeFrameMutex_);
        if (cpuSequence != receivedFrameSequence_.load(
                std::memory_order_acquire)) {
            return false;
        }
        pendingNativeFrame_ = nullptr;
        pendingNativeSequence_ = 0;
        retiredCpuImage =
            std::move(pendingD3D11CpuImage_);
        pendingD3D11CpuSequence_ = 0;
        nativeRedrawRequested_ = false;
        nativeResetRequested_ = true;
        nativeUiActiveRequested_.store(
            false, std::memory_order_release);
        nativeSurfaceReady_.store(
            false, std::memory_order_release);
    }
    RecycleCpuImage(std::move(retiredCpuImage));
    nativeFrameCondition_.notify_one();
    return true;
}

void RemoteDesktopCanvas::NativePresentationLoop(std::stop_token stopToken)
{
    using Clock = std::chrono::steady_clock;
    const HRESULT comResult = CoInitializeEx(
        nullptr, COINIT_MULTITHREADED);
    SetThreadDescription(
        GetCurrentThread(), L"RemoteC D3D11 Present");
    SetThreadPriority(
        GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

    webrtc::scoped_refptr<webrtc::VideoFrameBuffer>
        lastNativeFrame;
    QImage lastCpuImage;
    std::uint64_t lastSequence = 0;
    bool lastWasCpu = false;
    Clock::time_point nextPresentationAt{};
    bool hasDeadline = false;

    while (!stopToken.stop_requested()) {
        webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame;
        QImage cpuImage;
        std::uint64_t sequence = 0;
        bool newFrame = false;
        bool cpuFrame = false;
        bool cpuNv12Frame = false;
        bool cpuI420Frame = false;
        bool presentationFallbackQueued = false;
        {
            std::unique_lock lock(nativeFrameMutex_);
            nativeFrameCondition_.wait(lock, [this, &stopToken] {
                return stopToken.stop_requested() ||
                    pendingNativeFrame_ != nullptr ||
                    !pendingD3D11CpuImage_.isNull() ||
                    nativeRedrawRequested_ ||
                    nativeResetRequested_;
            });
            if (stopToken.stop_requested()) {
                break;
            }

            if (nativeResetRequested_) {
                nativeResetRequested_ = false;
                lastNativeFrame = nullptr;
                lastCpuImage = {};
                lastSequence = 0;
                lastWasCpu = false;
                hasDeadline = false;
            }

            nativeFrameCondition_.wait(lock,
                [this, &stopToken] {
                    return stopToken.stop_requested() ||
                        nativeSurfaceReady_.load(
                            std::memory_order_acquire) ||
                        nativeResetRequested_;
                });
            if (stopToken.stop_requested()) {
                break;
            }
            if (nativeResetRequested_) {
                nativeResetRequested_ = false;
                lastNativeFrame = nullptr;
                lastCpuImage = {};
                lastSequence = 0;
                lastWasCpu = false;
                hasDeadline = false;
                continue;
            }

            if (hasDeadline &&
                Clock::now() < nextPresentationAt) {
                nativeFrameCondition_.wait_until(
                    lock, nextPresentationAt, [&stopToken] {
                        return stopToken.stop_requested();
                    });
                if (stopToken.stop_requested()) {
                    break;
                }
                if (nativeResetRequested_) {
                    nativeResetRequested_ = false;
                    lastNativeFrame = nullptr;
                    lastCpuImage = {};
                    lastSequence = 0;
                    lastWasCpu = false;
                    hasDeadline = false;
                }
            }

            const bool redraw = nativeRedrawRequested_;
            nativeRedrawRequested_ = false;
            if (pendingNativeFrame_) {
                frame = std::move(pendingNativeFrame_);
                sequence = pendingNativeSequence_;
                pendingNativeSequence_ = 0;
                newFrame = true;
            }
            else if (!pendingD3D11CpuImage_.isNull()) {
                cpuImage = std::move(pendingD3D11CpuImage_);
                sequence = pendingD3D11CpuSequence_;
                pendingD3D11CpuSequence_ = 0;
                newFrame = true;
                cpuFrame = true;
                cpuNv12Frame = IsCpuNv12Image(cpuImage);
            }
            else if (redraw && lastWasCpu &&
                !lastCpuImage.isNull()) {
                cpuImage = lastCpuImage;
                sequence = lastSequence;
                cpuFrame = true;
                cpuNv12Frame = IsCpuNv12Image(cpuImage);
            }
            else if (redraw && lastNativeFrame) {
                frame = lastNativeFrame;
                sequence = lastSequence;
            }
        }

        if (!frame && cpuImage.isNull()) {
            continue;
        }
        if (newFrame &&
            sequence != receivedFrameSequence_.load(
                std::memory_order_acquire)) {
            VideoPresentationTelemetryRegistry::Instance()
                .RecordSuperseded(presentationTelemetryId_);
            if (cpuFrame) {
                RecycleCpuImage(std::move(cpuImage));
            }
            continue;
        }

        if (cursorRenderState_) {
            const auto cursor =
                cursorRenderState_->GetSnapshot();
            int cursorSourceWidth = 0;
            int cursorSourceHeight = 0;
            if (cpuFrame) {
                cursorSourceWidth = cpuImage.width();
                cursorSourceHeight = cpuNv12Frame
                    ? cpuImage.height() * 2 / 3
                    : cpuImage.height();
            }
            else if (frame) {
                cursorSourceWidth = frame->width();
                cursorSourceHeight = frame->height();
            }
            nativeSurface_->SetCursorFrame(cursor,
                cursorSourceWidth, cursorSourceHeight);
        }

        D3D11VideoSurface::PresentTiming presentTiming;
        if (cpuFrame) {
            presentTiming = cpuNv12Frame
                ? nativeSurface_->PresentCpuNv12(cpuImage)
                : nativeSurface_->PresentCpuBgra(cpuImage);
        }
        else {
            auto* native =
                D3D11NativeFrameBuffer::From(frame.get());
            if (native) {
                presentTiming = nativeSurface_->Present(native);
            }
            else if (!cpuI420D3D11PresentationFailed_.load(
                         std::memory_order_acquire)) {
                const auto i420 = frame->ToI420();
                if (i420) {
                    cpuI420Frame = true;
                    presentTiming =
                        nativeSurface_->PresentCpuI420(i420.get());
                }
                else if (newFrame) {
                    QueueCpuFrame(std::move(frame), false);
                    presentationFallbackQueued = true;
                }
            }
            else if (newFrame) {
                QueueCpuFrame(std::move(frame), false);
                presentationFallbackQueued = true;
            }
        }
        if (presentationFallbackQueued) {
            continue;
        }
        if (newFrame && presentTiming.cpuPreparationUs > 0) {
            VideoPresentationTelemetryRegistry::Instance()
                .RecordConversion(presentationTelemetryId_,
                    presentTiming.cpuPreparationUs);
        }

        if (presentTiming.succeeded) {
            if (newFrame) {
                if (cpuFrame) {
                    QImage previousCpuImage =
                        std::move(lastCpuImage);
                    lastCpuImage = std::move(cpuImage);
                    lastNativeFrame = nullptr;
                    RecycleCpuImage(
                        std::move(previousCpuImage));
                }
                else {
                    QImage previousCpuImage =
                        std::move(lastCpuImage);
                    lastNativeFrame = frame;
                    RecycleCpuImage(
                        std::move(previousCpuImage));
                }
                lastSequence = sequence;
                lastWasCpu = cpuFrame;
                VideoPresentationTelemetryRegistry::Instance()
                    .RecordPresented(presentationTelemetryId_,
                        cpuNv12Frame
                            ? VideoPresentationPath::kCpuNv12D3D11
                            : (cpuFrame
                                ? VideoPresentationPath::kCpuD3D11
                                : (cpuI420Frame
                                    ? VideoPresentationPath::kCpuI420D3D11
                                    : VideoPresentationPath::kD3D11)),
                        presentTiming.videoProcessorSubmitUs,
                        presentTiming.presentCallUs);
                NotifyFirstPresentation();
            }
        }
        else if (newFrame) {
            VideoPresentationTelemetryRegistry::Instance()
                .RecordPresentFailure(
                    presentationTelemetryId_);
            if (cpuFrame) {
                if (cpuNv12Frame) {
                    // Some older drivers expose D3D11 video but
                    // reject application-uploaded NV12 input
                    // views. Drop this one frame and preserve the
                    // proven ARGB/D3D11 path for subsequent frames.
                    cpuNv12D3D11PresentationFailed_.store(
                        true, std::memory_order_release);
                    RecycleCpuImage(std::move(cpuImage));
                }
                else {
                    cpuD3D11PresentationFailed_.store(
                        true, std::memory_order_release);
                    QueueQtCpuImage(
                        std::move(cpuImage), sequence);
                }
            }
            else {
                if (cpuI420Frame) {
                    cpuI420D3D11PresentationFailed_.store(
                        true, std::memory_order_release);
                }
                QueueCpuFrame(std::move(frame), false);
            }
        }

        const auto interval = std::chrono::microseconds(
            nativePresentationIntervalUs_.load(
                std::memory_order_acquire));
        const auto now = Clock::now();
        if (!hasDeadline ||
            now > nextPresentationAt + interval) {
            nextPresentationAt = now + interval;
            hasDeadline = true;
        }
        else {
            nextPresentationAt += interval;
        }
    }

    RecycleCpuImage(std::move(lastCpuImage));

    if (SUCCEEDED(comResult)) {
        CoUninitialize();
    }
}

void RemoteDesktopCanvas::UpdateNativeSurfaceGeometry()
{
    if (nativeSurface_ && nativeFrameActive_) {
        nativeSurface_->setGeometry(VideoContentRect());
    }
}

QRect RemoteDesktopCanvas::VideoContentRect() const
{
    const QSize sourceSize = VideoSourceSize();
    if (!sourceSize.isValid()) {
        return {};
    }
    if (actualPixelDisplayMode_.load(
            std::memory_order_acquire)) {
        const qreal dpr = (std::max)(
            devicePixelRatioF(), 0.5);
        const QSize targetSize(
            (std::max)(1, static_cast<int>(std::lround(
                sourceSize.width() / dpr))),
            (std::max)(1, static_cast<int>(std::lround(
                sourceSize.height() / dpr))));
        int left = (width() - targetSize.width()) / 2 +
            actualPixelPanOffset_.x();
        int top = (height() - targetSize.height()) / 2 +
            actualPixelPanOffset_.y();
        left = targetSize.width() <= width()
            ? (width() - targetSize.width()) / 2
            : std::clamp(left,
                width() - targetSize.width(), 0);
        top = targetSize.height() <= height()
            ? (height() - targetSize.height()) / 2
            : std::clamp(top,
                height() - targetSize.height(), 0);
        return QRect(QPoint(left, top), targetSize);
    }
    const QSize targetSize =
        sourceSize.scaled(size(), Qt::KeepAspectRatio);
    return QRect(
        QPoint((width() - targetSize.width()) / 2,
            (height() - targetSize.height()) / 2),
        targetSize);
}

QSize RemoteDesktopCanvas::VideoSourceSize() const
{
    const QSize reportedSourceSize(
        pointerSourceWidth_.load(std::memory_order_acquire),
        pointerSourceHeight_.load(std::memory_order_acquire));
    if (reportedSourceSize.isValid()) {
        return reportedSourceSize;
    }
    return nativeFrameActive_
        ? nativeFrameSize_
        : frameImage_.size();
}

std::optional<RemoteInputEvent>
    RemoteDesktopCanvas::SampleCurrentDragPointer(
        const RemoteInputEvent& fallback) const
{
    if (!pointerSamplingWindow_ ||
        !IsWindow(pointerSamplingWindow_)) {
        return std::nullopt;
    }
    POINT pointer{};
    RECT client{};
    if (!GetCursorPos(&pointer) ||
        !ScreenToClient(
            pointerSamplingWindow_, &pointer) ||
        !GetClientRect(pointerSamplingWindow_, &client)) {
        return std::nullopt;
    }

    const int clientWidth = client.right - client.left;
    const int clientHeight = client.bottom - client.top;
    const int sourceWidth = pointerSourceWidth_.load(
        std::memory_order_acquire);
    const int sourceHeight = pointerSourceHeight_.load(
        std::memory_order_acquire);
    if (clientWidth <= 0 || clientHeight <= 0 ||
        sourceWidth <= 0 || sourceHeight <= 0) {
        return std::nullopt;
    }

    int contentLeft = 0;
    int contentTop = 0;
    int contentWidth = clientWidth;
    int contentHeight = clientHeight;
    if (!pointerStretchToCanvas_.load(
            std::memory_order_acquire)) {
        const std::int64_t widthLimitedHeight =
            static_cast<std::int64_t>(clientWidth) *
            sourceHeight / sourceWidth;
        if (widthLimitedHeight <= clientHeight) {
            contentHeight = static_cast<int>(
                widthLimitedHeight);
            contentTop =
                (clientHeight - contentHeight) / 2;
        } else {
            contentWidth = static_cast<int>(
                static_cast<std::int64_t>(
                    clientHeight) *
                sourceWidth / sourceHeight);
            contentLeft =
                (clientWidth - contentWidth) / 2;
        }
    }
    if (contentWidth <= 1 || contentHeight <= 1) {
        return std::nullopt;
    }

    pointer.x = std::clamp<LONG>(
        pointer.x,
        contentLeft,
        contentLeft + contentWidth - 1);
    pointer.y = std::clamp<LONG>(
        pointer.y,
        contentTop,
        contentTop + contentHeight - 1);
    const auto normalize = [](
        LONG position, int origin, int extent) {
        const auto offset = static_cast<std::uint64_t>(
            position - origin);
        return static_cast<std::uint16_t>(
            offset * 65'535ULL /
            static_cast<std::uint64_t>(extent - 1));
    };

    RemoteInputEvent sampled = fallback;
    sampled.normalizedX = normalize(
        pointer.x, contentLeft, contentWidth);
    sampled.normalizedY = normalize(
        pointer.y, contentTop, contentHeight);
    return sampled;
}

std::optional<std::pair<std::uint16_t, std::uint16_t>> RemoteDesktopCanvas::MapPoint(
    const QPointF& point,
    bool clampToContent) const
{
    if (!controlEnabled_) {
        return std::nullopt;
    }
    const QRect content = VideoContentRect();
    if (content.width() <= 0 || content.height() <= 0) {
        return std::nullopt;
    }

    QPointF mapped = point;
    if (!content.contains(mapped.toPoint())) {
        if (!clampToContent) {
            return std::nullopt;
        }
        mapped.setX(std::clamp(
            mapped.x(), static_cast<qreal>(content.left()),
            static_cast<qreal>(content.right())));
        mapped.setY(std::clamp(
            mapped.y(), static_cast<qreal>(content.top()),
            static_cast<qreal>(content.bottom())));
    }

    const auto normalize = [](qreal position, int origin, int extent) {
        if (extent <= 1) {
            return std::uint16_t{ 0 };
        }
        const double ratio = std::clamp(
            (position - origin) / static_cast<double>(extent - 1),
            0.0, 1.0);
        return static_cast<std::uint16_t>(
            std::lround(ratio * 65535.0));
        };
    return std::pair{
        normalize(mapped.x(), content.left(), content.width()),
        normalize(mapped.y(), content.top(), content.height()) };
}

std::optional<RemoteMouseButton> RemoteDesktopCanvas::MapMouseButton(
    Qt::MouseButton button)
{
    switch (button) {
    case Qt::LeftButton:
        return RemoteMouseButton::kLeft;
    case Qt::RightButton:
        return RemoteMouseButton::kRight;
    case Qt::MiddleButton:
        return RemoteMouseButton::kMiddle;
    case Qt::BackButton:
        return RemoteMouseButton::kX1;
    case Qt::ForwardButton:
        return RemoteMouseButton::kX2;
    default:
        return std::nullopt;
    }
}

std::uint8_t RemoteDesktopCanvas::MapMouseButtons(Qt::MouseButtons buttons)
{
    std::uint8_t mask = 0;
    if (buttons.testFlag(Qt::LeftButton)) {
        mask |= 1u << 0;
    }
    if (buttons.testFlag(Qt::RightButton)) {
        mask |= 1u << 1;
    }
    if (buttons.testFlag(Qt::MiddleButton)) {
        mask |= 1u << 2;
    }
    if (buttons.testFlag(Qt::BackButton)) {
        mask |= 1u << 3;
    }
    if (buttons.testFlag(Qt::ForwardButton)) {
        mask |= 1u << 4;
    }
    return mask;
}

std::int16_t RemoteDesktopCanvas::ClampWheelDelta(int value)
{
    return static_cast<std::int16_t>(std::clamp(
        value,
        static_cast<int>(std::numeric_limits<std::int16_t>::min()),
        static_cast<int>(std::numeric_limits<std::int16_t>::max())));
}

bool RemoteDesktopCanvas::IsExtendedVirtualKey(std::uint32_t virtualKey)
{
    switch (virtualKey) {
    case VK_RCONTROL:
    case VK_RMENU:
    case VK_INSERT:
    case VK_DELETE:
    case VK_HOME:
    case VK_END:
    case VK_PRIOR:
    case VK_NEXT:
    case VK_LEFT:
    case VK_RIGHT:
    case VK_UP:
    case VK_DOWN:
    case VK_NUMLOCK:
    case VK_DIVIDE:
    case VK_SNAPSHOT:
    case VK_LWIN:
    case VK_RWIN:
    case VK_APPS:
        return true;
    default:
        return false;
    }
}

bool RemoteDesktopCanvas::QueuePointerMove(
    const std::pair<std::uint16_t, std::uint16_t>& point,
    std::uint8_t pressedMouseButtons)
{
    if (!controlEnabled_ || !inputSender_ ||
        !pointerMoveScheduler_) {
        return false;
    }
    RemoteInputEvent input;
    input.type = RemoteInputMessageType::kMouseMove;
    input.displayId = remoteDisplayId_;
    input.displayLayoutVersion =
        remoteDisplayLayoutVersion_;
    input.normalizedX = point.first;
    input.normalizedY = point.second;
    input.pressedMouseButtons = pressedMouseButtons;
    auto& telemetry = RemoteInputTelemetry::Instance();
    telemetry.RecordGeneratedMouseMove();
    if (cursorRenderState_) {
        RemoteCursorPosition predicted;
        predicted.displayId = remoteDisplayId_;
        predicted.displayLayoutVersion =
            remoteDisplayLayoutVersion_;
        predicted.normalizedX = point.first;
        predicted.normalizedY = point.second;
        predicted.visible = true;
        cursorRenderState_->SetPosition(predicted, true);
        CursorVisualChanged();
    }
    const std::uint32_t rateLimitHz =
        pressedMouseButtons
        ? dragPointerSampleRateHz_
        : pointerMoveRateLimitHz_;
    return pointerMoveScheduler_->Queue(
        input, rateLimitHz);
}

void RemoteDesktopCanvas::FlushPendingPointerMove()
{
    if (pointerMoveScheduler_) {
        (void)pointerMoveScheduler_->Flush();
    }
}

void RemoteDesktopCanvas::CancelPendingPointerMove()
{
    if (pointerMoveScheduler_) {
        pointerMoveScheduler_->Cancel();
    }
}

bool RemoteDesktopCanvas::SendKeyEvent(QKeyEvent* event, bool pressed)
{
    if (!controlEnabled_ || !inputSender_) {
        return false;
    }
    const std::uint32_t nativeVirtualKey = event->nativeVirtualKey();
    const std::uint32_t nativeScanCode = event->nativeScanCode();
    if ((nativeVirtualKey == 0 && nativeScanCode == 0) ||
        nativeVirtualKey > std::numeric_limits<std::uint16_t>::max()) {
        return false;
    }

    RemoteInputEvent input;
    input.type = RemoteInputMessageType::kKey;
    input.displayId = 0;
    input.virtualKey = static_cast<std::uint16_t>(nativeVirtualKey);
    input.scanCode = static_cast<std::uint16_t>(nativeScanCode & 0xffu);
    input.extendedKey = (nativeScanCode & 0x100u) != 0 ||
        IsExtendedVirtualKey(nativeVirtualKey);
    input.pressed = pressed;
    input.repeat = pressed && event->isAutoRepeat();
    RemoteInputTelemetry::Instance().RecordGeneratedKey();
    return inputSender_(input);
}

void RemoteDesktopCanvas::FocusRemotePasteTarget(const QPointF& position)
{
    const auto point = MapPoint(position, false);
    if (!point || !controlEnabled_ || !inputSender_) return;
    FlushPendingPointerMove();
    setFocus(Qt::MouseFocusReason);
    for (const bool pressed : {true, false}) {
        RemoteInputEvent input;
        input.type = RemoteInputMessageType::kMouseButton;
        input.displayId = remoteDisplayId_;
        input.displayLayoutVersion = remoteDisplayLayoutVersion_;
        input.normalizedX = point->first;
        input.normalizedY = point->second;
        input.mouseButton = RemoteMouseButton::kLeft;
        input.pressed = pressed;
        input.pressedMouseButtons = pressed ? 1u : 0u;
        RemoteInputTelemetry::Instance()
            .RecordGeneratedMouseButton();
        if (!inputSender_(input)) break;
    }
}

}  // namespace remote::controller
