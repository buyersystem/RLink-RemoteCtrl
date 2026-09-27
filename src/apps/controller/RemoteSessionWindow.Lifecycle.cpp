// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RemoteSessionWindow.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <utility>

#include <QAbstractAnimation>
#include <QLabel>
#include <QMetaObject>
#include <QPointer>
#include <QSettings>
#include <QTimer>
#include <QVariant>
#include <QWidget>

#include "RemoteDesktopCanvas.h"
#include "RemoteInputDispatcher.h"
#include "RemoteSessionWindowConstants.h"
#include "src/apps/remote/ISessionMediaAccess.h"
#include "src/protocol/DataChannelCatalog.h"

namespace remote::controller {

    RemoteSessionWindow::RemoteSessionWindow(RemoteSessionBinding binding,
        IRemoteSessionControl* sessionControl,
        app::ISessionMediaAccess* sessionMedia,
        QWidget* parent)
        : FramelessMainWindow(parent),
        remoteInputDispatcher_(
            std::make_shared<RemoteInputDispatcher>()),
        binding_(std::move(binding))
    {
        const QSettings settings;
        selectedFrameRate_ = kDefaultScreenFrameRate;
        const std::uint32_t configuredDragSampleRate =
            settings.value(
                QString::fromLatin1(
                    kDragPointerSampleRateSetting),
                QVariant::fromValue(240u)).toUInt();
        if (std::find(
                kSupportedDragPointerSampleRates.begin(),
                kSupportedDragPointerSampleRates.end(),
                configuredDragSampleRate) !=
            kSupportedDragPointerSampleRates.end()) {
            dragPointerSampleRateHz_ =
                configuredDragSampleRate;
        }
        const int savedQuality = settings.value(
            QString::fromLatin1(kRemoteScreenQualitySetting),
            static_cast<int>(ScreenQualityTier::kOriginal)).toInt();
        if (savedQuality >=
                static_cast<int>(ScreenQualityTier::kAutomatic) &&
            savedQuality <=
                static_cast<int>(ScreenQualityTier::kOriginal)) {
            selectedQuality_ =
                static_cast<ScreenQualityTier>(savedQuality);
        }
        BuildUi();
        BindSessionVideo(sessionControl, sessionMedia, binding_);
        sessionElapsed_.start();
        durationTimer_->start(1000);
    }

    RemoteSessionWindow::~RemoteSessionWindow()
    {
        if (releaseFileTransferHostHandler_) {
            releaseFileTransferHostHandler_();
        }
        if (remotePasteAnimation_) {
            remotePasteAnimation_->stop();
        }
        delete remotePasteAnimationOverlay_;
        remotePasteAnimationOverlay_ = nullptr;
        if (desktopCanvas_) {
            static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
                ->ShutdownInputScheduler();
        }
        ReleaseRemoteInputs();
        remoteInputDispatcher_->Clear();
        BindSessionVideo(nullptr, nullptr, binding_);
    }

    void RemoteSessionWindow::SetDisconnectHandler(
        std::function<void()> handler)
    {
        disconnectHandler_ = std::move(handler);
    }

    void RemoteSessionWindow::SetRemotePasteHandler(
        std::function<bool(const QStringList& localFiles,
                           bool keyboardPaste)> handler)
    {
        remotePasteHandler_ = std::move(handler);
    }

    void RemoteSessionWindow::SetRemotePasteCancelHandler(
        std::function<void()> handler)
    {
        remotePasteCancelHandler_ = std::move(handler);
    }

    void RemoteSessionWindow::SetFileTransferHandlers(
        std::function<void()> openHandler,
        std::function<void()> releaseHostHandler)
    {
        fileTransferHandler_ = std::move(openHandler);
        releaseFileTransferHostHandler_ =
            std::move(releaseHostHandler);
    }

    void RemoteSessionWindow::SetDragPointerSampleRate(
        std::uint32_t hertz)
    {
        if (std::find(
                kSupportedDragPointerSampleRates.begin(),
                kSupportedDragPointerSampleRates.end(),
                hertz) ==
            kSupportedDragPointerSampleRates.end()) {
            hertz = 240;
        }
        dragPointerSampleRateHz_ = hertz;
        if (desktopCanvas_) {
            static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
                ->SetDragPointerSampleRate(hertz);
        }
    }

    void RemoteSessionWindow::SetRoomOnlineMemberCount(
        std::size_t onlineMemberCount)
    {
        const std::uint32_t maximumFrameRate =
            onlineMemberCount > kHighOccupancyRoomMemberThreshold
            ? kMultiMemberMaximumScreenFrameRate
            : kMaximumScreenFrameRate;
        if (roomMaximumFrameRate_ == maximumFrameRate) {
            return;
        }
        roomMaximumFrameRate_ = maximumFrameRate;
        const std::uint32_t previousSelection = selectedFrameRate_;
        RebuildFrameRateMenu();
        if (selectedFrameRate_ != previousSelection && sessionControl_ &&
            binding_.IsRoom()) {
            (void)RequestStreamPreference(false);
        }
    }

    void RemoteSessionWindow::BindSessionVideo(
        IRemoteSessionControl* sessionControl,
        app::ISessionMediaAccess* media,
        RemoteSessionBinding binding)
    {
        if (sessionControl_ == sessionControl && sessionMedia_ == media &&
            binding_.SameTransport(binding) &&
            sessionVideoSinkBound_) {
            binding_ = std::move(binding);
            if (sessionSourceLabel_) {
                sessionSourceLabel_->setText(
                    QStringLiteral("%1 · %2")
                        .arg(binding_.peerDeviceId,
                             binding_.SourceText()));
            }
            return;
        }
        const bool bindingChanged =
            sessionControl_ != sessionControl || sessionMedia_ != media ||
            !binding_.SameTransport(binding);
        if (bindingChanged && sessionMedia_) {
            sessionMedia_->SetRemoteCursorCallback({});
        }
        if (bindingChanged) {
            pairWasActive_ = false;
            if (desktopCanvas_) {
                static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
                    ->SetControlEnabled(false);
            }
            remoteInputDispatcher_->SetEnabled(false);
        }
        if (bindingChanged && sessionVideoSinkBound_ && sessionMedia_ &&
            desktopCanvas_) {
            if (binding_.IsDirect()) {
                (void)sessionMedia_->SetDirectRemoteVideoSink(nullptr);
            } else {
                (void)sessionMedia_->SetRoomRemoteVideoSink(
                    binding_.roomPairId.toStdString(),
                    kScreenMainVideoSlot, nullptr);
            }
        }

        sessionControl_ = sessionControl;
        sessionMedia_ = media;
        binding_ = std::move(binding);
        if (sessionMedia_) {
            const QPointer<RemoteSessionWindow> self(this);
            sessionMedia_->SetRemoteCursorCallback(
                [self](const std::string& pairId,
                       const RemoteCursorEnvelope& envelope) {
                    if (!self) return;
                    QMetaObject::invokeMethod(
                        self,
                        [self, pairId, envelope] {
                            if (self) {
                                self->HandleRemoteCursorMessage(
                                    pairId, envelope);
                            }
                        },
                        Qt::QueuedConnection);
                });
        }
        if (sessionSourceLabel_) {
            sessionSourceLabel_->setText(
                QStringLiteral("%1 · %2")
                    .arg(binding_.peerDeviceId, binding_.SourceText()));
        }
        remoteInputDispatcher_->SetMediaAccess(media);
        if (bindingChanged) {
            sessionVideoSinkBound_ = false;
            sessionVideoSinkRetryScheduled_ = false;
            roomScreenPreferenceRetryScheduled_ = false;
            preferenceRequestedScreenShareEpoch_ = 0;
            preferenceSentScreenShareEpoch_ = 0;
            screenStartupGeneration_ = 0;
            screenStartupRefreshAttempts_ = 0;
            screenFirstFramePresented_ = false;
            remoteSourceWidth_ = 0;
            remoteSourceHeight_ = 0;
            RebuildQualityMenu();
            if (desktopCanvas_) {
                static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
                    ->BeginPresentationGeneration(0);
                static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
                    ->ResetRemoteCursor();
            }
            reportedRemoteMaximumFrameRate_ = kMaximumScreenFrameRate;
            RebuildFrameRateMenu();
        }
        if (!sessionControl_ || !sessionMedia_ || !desktopCanvas_) {
            sessionVideoSinkBound_ = false;
            if (previewBadge_) {
                previewBadge_->setText(QStringLiteral("等待画面"));
            }
            RefreshControlState();
            return;
        }

        auto* sink = static_cast<RemoteDesktopCanvas*>(desktopCanvas_);
        if (binding_.IsDirect()) {
            const auto result =
                sessionMedia_->SetDirectRemoteVideoSink(sink);
            sessionVideoSinkBound_ = result.accepted;
            if (previewBadge_) {
                previewBadge_->setText(
                    result.accepted ? QStringLiteral("实时画面")
                                    : QStringLiteral("等待视频"));
            }
            if (!result.accepted && !sessionVideoSinkRetryScheduled_) {
                sessionVideoSinkRetryScheduled_ = true;
                const auto* expectedControl = sessionControl_;
                QTimer::singleShot(120, this,
                    [this, expectedControl] {
                        sessionVideoSinkRetryScheduled_ = false;
                        if (sessionControl_ == expectedControl &&
                            binding_.IsDirect() &&
                            !sessionVideoSinkBound_) {
                            BindSessionVideo(
                                sessionControl_, sessionMedia_, binding_);
                        }
                    });
            }
            RefreshControlState();
            return;
        }

        const auto bindingSnapshot = sessionControl_->Snapshot();
        if (bindingSnapshot.room.screenShareState ==
                RoomScreenShareState::kActive &&
            bindingSnapshot.room.screenShareEpoch != 0 &&
            bindingSnapshot.room.screenSharerDeviceId ==
                binding_.peerDeviceId.toStdString() &&
            screenStartupGeneration_ !=
                bindingSnapshot.room.screenShareEpoch) {
            BeginScreenStartup(
                bindingSnapshot.room.screenShareEpoch);
        }
        const auto result = sessionMedia_->SetRoomRemoteVideoSink(
            binding_.roomPairId.toStdString(), kScreenMainVideoSlot, sink);
        sessionVideoSinkBound_ = result.accepted;
        if (previewBadge_) {
            previewBadge_->setText(
                result.accepted ? QStringLiteral("实时画面")
                : QStringLiteral("等待视频"));
        }
        if (!result.accepted && !sessionVideoSinkRetryScheduled_) {
            sessionVideoSinkRetryScheduled_ = true;
            const auto* expectedControl = sessionControl_;
            const RemoteSessionBinding expectedBinding = binding_;
            QTimer::singleShot(120, this,
                [this, expectedControl, expectedBinding] {
                    sessionVideoSinkRetryScheduled_ = false;
                    if (sessionControl_ == expectedControl &&
                        binding_.SameTransport(expectedBinding) &&
                        !sessionVideoSinkBound_) {
                        BindSessionVideo(
                            sessionControl_, sessionMedia_, binding_);
                    }
                });
        }
        RefreshControlState();
    }

    void RemoteSessionWindow::HandleRemoteCursorMessage(
        const std::string& pairId,
        const RemoteCursorEnvelope& envelope)
    {
        if (!desktopCanvas_ ||
            envelope.senderDeviceId !=
                binding_.peerDeviceId.toStdString()) {
            return;
        }
        if (binding_.IsDirect()) {
            if (!pairId.empty()) return;
        } else if (pairId != binding_.roomPairId.toStdString()) {
            return;
        }
        static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
            ->ApplyRemoteCursor(envelope);
    }

    void RemoteSessionWindow::BeginScreenStartup(
        std::uint64_t screenShareGeneration)
    {
        if (screenShareGeneration == 0 || !desktopCanvas_) {
            return;
        }
        screenStartupGeneration_ = screenShareGeneration;
        // Frame-rate preference is scoped to one share generation. Start
        // every new share from the product default instead of carrying the
        // previous sharer's last acknowledged selection into this stream.
        selectedFrameRate_ = kDefaultScreenFrameRate;
        reportedRemoteMaximumFrameRate_ = kMaximumScreenFrameRate;
        RebuildFrameRateMenu();
        screenStartupRefreshAttempts_ = 0;
        screenFirstFramePresented_ = false;
        screenStartupElapsed_.restart();
        static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
            ->BeginPresentationGeneration(screenShareGeneration);

        // Fast machines normally present before the first retry. Keep a few
        // bounded late retries for old hardware encoders whose first IDR or
        // driver initialization can take several seconds. Each retry is
        // generation-bound and stops as soon as a frame is presented.
        constexpr std::array<std::pair<int, std::uint32_t>, 5>
            kStartupRetries = {{{300, 1u}, {800, 2u}, {1500, 3u},
                                {3000, 4u}, {6000, 5u}}};
        for (const auto [delayMs, stage] : kStartupRetries) {
            QTimer::singleShot(delayMs, this,
                [this, screenShareGeneration, stage] {
                    RequestScreenStartupRefresh(
                        screenShareGeneration, stage);
                });
        }
    }

    void RemoteSessionWindow::RequestScreenStartupRefresh(
        std::uint64_t screenShareGeneration,
        std::uint32_t retryStage)
    {
        if (screenFirstFramePresented_ ||
            screenStartupGeneration_ != screenShareGeneration ||
            retryStage <= screenStartupRefreshAttempts_ ||
            !sessionControl_ || !sessionMedia_ || !binding_.IsRoom() ||
            !desktopCanvas_) {
            return;
        }
        const auto snapshot = sessionControl_->Snapshot();
        if (snapshot.room.screenShareState !=
                RoomScreenShareState::kActive ||
            snapshot.room.screenShareEpoch != screenShareGeneration ||
            snapshot.room.screenSharerDeviceId !=
                binding_.peerDeviceId.toStdString()) {
            return;
        }

        screenStartupRefreshAttempts_ = retryStage;
        auto* sink = static_cast<RemoteDesktopCanvas*>(desktopCanvas_);
        // A bounded remove/add rebind makes libwebrtc issue a fresh PLI and
        // also sends the generation-bound type-10 full-frame request. It does
        // not rebuild ICE or the PeerConnection.
        (void)sessionMedia_->SetRoomRemoteVideoSink(
            binding_.roomPairId.toStdString(), kScreenMainVideoSlot, nullptr);
        const auto result = sessionMedia_->SetRoomRemoteVideoSink(
            binding_.roomPairId.toStdString(), kScreenMainVideoSlot, sink);
        sessionVideoSinkBound_ = result.accepted;
    }

    void RemoteSessionWindow::HandleFirstScreenPresentation(
        std::uint64_t screenShareGeneration)
    {
        if (screenFirstFramePresented_ ||
            screenStartupGeneration_ != screenShareGeneration ||
            !sessionMedia_ || !binding_.IsRoom()) {
            return;
        }
        screenFirstFramePresented_ = true;
        const qint64 elapsed = screenStartupElapsed_.isValid()
            ? screenStartupElapsed_.elapsed() : 0;
        const auto elapsedMs = static_cast<std::uint32_t>(
            std::clamp<qint64>(
                elapsed, 0,
                static_cast<qint64>(
                    std::numeric_limits<std::uint32_t>::max())));
        (void)sessionMedia_->NotifyRoomScreenFirstFramePresented(
            binding_.roomPairId.toStdString(),
            screenShareGeneration,
            elapsedMs);
    }

}  // namespace remote::controller
