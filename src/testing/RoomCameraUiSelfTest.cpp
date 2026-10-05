// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

// Synthetic video/UI regression probe: no camera, engine, network or account.
#include "src/apps/controller/RoomCameraWindow.h"
#include "src/apps/controller/ui/RemoteCTheme.h"
#include "src/apps/remote/ISessionMediaAccess.h"

#include "api/video/i420_buffer.h"
#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"

#include <QApplication>
#include <QColor>
#include <QElapsedTimer>
#include <QEvent>
#include <QImage>
#include <QLabel>
#include <QPointer>
#include <QPixmap>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryDir>
#include <QTextStream>

#include <algorithm>
#include <memory>
#include <thread>
#include <unordered_map>

namespace {
using namespace remote;
using namespace remote::controller;
using VideoSink = webrtc::VideoSinkInterface<webrtc::VideoFrame>;

int failures = 0;

void Check(bool condition, const char* message)
{
    QTextStream(condition ? stdout : stderr)
        << (condition ? "PASS " : "FAIL ") << message << '\n';
    if (!condition) {
        ++failures;
    }
}

void FlushUi()
{
    QApplication::sendPostedEvents();
    QApplication::processEvents();
}

class MediaAccess final : public app::ISessionMediaAccess {
public:
    VideoSink* preview = nullptr;
    std::unordered_map<std::string, VideoSink*> remoteSinks;
    int previewBindings = 0;
    int remoteBindings = 0;

    SessionCommandResult SetRoomRemoteVideoSink(
        const std::string& pairId, const std::string&, VideoSink* sink) override
    {
        remoteSinks[pairId] = sink;
        ++remoteBindings;
        return {};
    }
    SessionCommandResult SetDirectRemoteVideoSink(VideoSink*) override { return {}; }
    SessionCommandResult NotifyRoomScreenFirstFramePresented(
        const std::string&, std::uint64_t, std::uint32_t) override { return {}; }
    void SetLocalCameraPreviewSink(VideoSink* sink) override
    {
        preview = sink;
        ++previewBindings;
    }
    void SetRemoteInputSink(IRemoteInputSink*) override {}
    void SetRemoteFileTransferSink(IFileTransferSink*) override {}
    void SetRemoteClipboardSink(IClipboardSink*) override {}
    void SetRemoteCursorCallback(RemoteCursorCallback) override {}
    SessionCommandResult SendRemoteInput(const RemoteInputEvent&) override { return {}; }
    SessionCommandResult SendRemoteFileMessage(
        const std::string&, const FileTransferMessage&) override { return {}; }
    SessionCommandResult SendRemoteClipboardMessage(
        const std::string&, const std::string&, const ClipboardMessage&) override { return {}; }
    SessionCommandResult SetRemoteAudioPlaybackMuted(bool) override { return {}; }
    SessionCommandResult SetDirectScreenStreamPreference(
        const ScreenStreamPreferenceRequest&) override { return {}; }
    SessionCommandResult RequestDirectSharedDisplaySwitch(const std::string&) override { return {}; }
    void SetPreferredHardwareDecoderName(std::string) override {}
    SessionCommandResult ApplyVideoPipelinePreferences(
        DesktopCaptureImplementation, VideoEncoderPreference, FfmpegX264Preset,
        FfmpegHardwareBackend, std::string, VideoDecoderPreference) override { return {}; }
};

class EventCounter final : public QObject {
public:
    int metaCalls = 0;
    int layoutChanges = 0;

    void Reset() { metaCalls = layoutChanges = 0; }

protected:
    bool eventFilter(QObject*, QEvent* event) override
    {
        if (event->type() == QEvent::MetaCall) {
            ++metaCalls;
        }
        if (event->type() == QEvent::ParentChange ||
            event->type() == QEvent::PolishRequest ||
            event->type() == QEvent::Polish ||
            event->type() == QEvent::LayoutRequest) {
            ++layoutChanges;
        }
        return false;
    }
};

webrtc::VideoFrame Frame(std::uint8_t luma)
{
    auto buffer = webrtc::I420Buffer::Create(64, 36);
    std::fill_n(buffer->MutableDataY(), buffer->StrideY() * buffer->height(), luma);
    std::fill_n(buffer->MutableDataU(), buffer->StrideU() * 18, std::uint8_t{128});
    std::fill_n(buffer->MutableDataV(), buffer->StrideV() * 18, std::uint8_t{128});
    return webrtc::VideoFrame::Builder()
        .set_video_frame_buffer(buffer)
        .set_timestamp_us(0)
        .build();
}

bool ShowsWhiteFrame(QWidget* tile)
{
    const QImage image = tile->grab().toImage();
    const QColor center = image.pixelColor(image.width() / 2, image.height() / 2);
    return center.red() > 240 && center.green() > 240 && center.blue() > 240;
}

SessionEngineSnapshot Snapshot()
{
    SessionEngineSnapshot snapshot;
    snapshot.localDeviceId = "local";
    snapshot.room.membership = RoomMembershipState::kActive;
    snapshot.media.localCamera = LocalCameraState::kPublishing;
    snapshot.room.members = {
        {"local", "local camera", true, true, false},
        {"peer", "remote camera", true, true, false},
    };
    RoomPeerConnectionSnapshot pair;
    pair.pairId = "pair";
    pair.peerDeviceId = "peer";
    pair.state = RoomPeerConnectionState::kActive;
    snapshot.roomActivity.peerConnections.push_back(pair);
    return snapshot;
}

void TestCameraUi()
{
    MediaAccess media;
    auto window = std::make_unique<RoomCameraWindow>(&media);
    auto snapshot = Snapshot();
    window->SyncSnapshot(snapshot);
    window->show();
    FlushUi();

    auto* localTile = dynamic_cast<QWidget*>(media.preview);
    auto* remoteTile = dynamic_cast<QWidget*>(media.remoteSinks["pair"]);
    Check(localTile && remoteTile, "local and remote camera sinks bind once");
    if (!localTile || !remoteTile) {
        return;
    }
    EventCounter counter;
    localTile->installEventFilter(&counter);
    for (auto* widget : window->findChildren<QWidget*>()) {
        if (widget != localTile) {
            widget->installEventFilter(&counter);
        }
    }
    const auto localGeometry = localTile->geometry();
    const auto remoteGeometry = remoteTile->geometry();
    const int previewBindings = media.previewBindings;
    const int remoteBindings = media.remoteBindings;
    counter.Reset();
    QElapsedTimer timer;
    timer.start();
    for (int i = 0; i < 100; ++i) {
        window->SyncSnapshot(snapshot);
    }
    FlushUi();
    QTextStream(stdout) << "TIMING camera_identical_snapshot_100 "
                        << timer.nsecsElapsed() / 1000000.0 << " ms\n";
    Check(counter.layoutChanges == 0,
          "identical snapshots do not repolish, reparent or relayout tiles");
    Check(localTile->geometry() == localGeometry && remoteTile->geometry() == remoteGeometry &&
          media.previewBindings == previewBindings && media.remoteBindings == remoteBindings,
          "identical snapshots preserve geometry and sink bindings");

    snapshot.room.members[0].deviceName = "renamed local";
    window->SyncSnapshot(snapshot);
    FlushUi();
    Check(localTile->findChild<QLabel*>(QStringLiteral("cameraName"))->text() ==
              QStringLiteral("renamed local"),
          "name changes still update existing camera tiles");

    auto* mode = window->findChild<QPushButton*>(QStringLiteral("galleryModeButton"));
    mode->click();
    FlushUi();
    Check(!localTile->property("thumbnail").toBool() &&
          !remoteTile->property("thumbnail").toBool(),
          "overview mode still changes tile presentation");
    mode->click();
    FlushUi();
    Check(localTile->property("thumbnail").toBool() &&
          !remoteTile->property("thumbnail").toBool(),
          "focus mode restores the local thumbnail and remote focus");

    const QString galleryStyle = window->styleSheet();
    for (const auto preference : {ui::ThemePreference::kDark, ui::ThemePreference::kLight}) {
        ui::RemoteCTheme::SavePreference(preference);
        window->SyncSnapshot(snapshot);
        FlushUi();
        Check(window->styleSheet() == galleryStyle &&
              localTile->property("thumbnail").toBool(),
              "theme preference round trip preserves camera gallery style and presentation");
    }

    // The GUI is deliberately not pumped while a video producer delivers frames.
    // Exactly one context-bound event should apply the last frame, not 101 images.
    counter.Reset();
    VideoSink* const preview = media.preview;
    std::thread producer([preview] {
        const auto black = Frame(16);
        const auto white = Frame(235);
        for (int i = 0; i < 100; ++i) {
            preview->OnFrame(black);
        }
        preview->OnFrame(white);
    });
    producer.join();
    QApplication::sendPostedEvents(localTile, QEvent::MetaCall);
    Check(counter.metaCalls == 1, "busy GUI queues exactly one camera-frame dispatch");
    Check(ShowsWhiteFrame(localTile), "coalesced dispatch presents the newest frame");

    window->close();
    Check(!window->isVisible() && media.preview == preview && media.remoteSinks["pair"],
          "hiding the gallery preserves camera subscriptions");
    preview->OnFrame(Frame(235));
    QApplication::sendPostedEvents(localTile, QEvent::MetaCall);
    Check(ShowsWhiteFrame(localTile), "hidden gallery still receives its newest frame");

    snapshot.roomActivity.peerConnections[0].pairId = "reconnected-pair";
    window->SyncSnapshot(snapshot);
    Check(!media.remoteSinks["pair"] &&
          media.remoteSinks["reconnected-pair"] == dynamic_cast<VideoSink*>(remoteTile),
          "pair replacement rebinds the existing remote tile even without relayout");

    QPointer<QWidget> deletedRemote(remoteTile);
    media.remoteSinks["reconnected-pair"]->OnFrame(Frame(235));
    snapshot.room.members.pop_back();
    window->SyncSnapshot(snapshot);
    FlushUi();
    Check(deletedRemote.isNull() && !media.remoteSinks["reconnected-pair"] && media.preview == preview,
          "removing a camera detaches its sink and cancels pending widget callbacks");

    QPointer<QWidget> deletedLocal(localTile);
    preview->OnFrame(Frame(235));
    window.reset();
    FlushUi();
    Check(deletedLocal.isNull() && !media.preview,
          "window destruction detaches sinks and safely drops pending frame dispatch");
}
}  // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QTemporaryDir settingsDirectory;
    if (!settingsDirectory.isValid()) {
        return 2;
    }
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDirectory.path());
    QCoreApplication::setOrganizationName(QStringLiteral("RLinkSyntheticUiTests"));
    QCoreApplication::setApplicationName(QStringLiteral("RoomCameraUiSelfTest"));
    QSettings settings;
    settings.setValue(QStringLiteral("ui/animationLevel"), 0);
    settings.sync();
    remote::controller::ui::RemoteCTheme::SavePreference(
        remote::controller::ui::ThemePreference::kLight);
    TestCameraUi();
    QTextStream(stdout) << "RESULT room_camera_ui " << failures << " failures\n";
    return failures == 0 ? 0 : 1;
}
