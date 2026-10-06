// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ControllerMainWindow.h"
#include "ControllerMainWindowSupport.h"

#include <QDateTime>
#include <QSettings>
#include <QStringList>
#include <algorithm>

#include "pages/DiagnosticsPage.h"

namespace remote::controller {
using namespace detail;

// Presentation only: opening a diagnostics category must not replay room,
// device, media or approval business actions from OnSessionEngineSnapshot.
void ControllerMainWindow::RefreshDiagnosticsSnapshotUi(
    const SessionEngineSnapshot& snapshot)
{
    if (!engine_) return;
    const bool debugPageVisible =
        debugPage_ && debugPage_->isVisibleTo(this);
    if (debugPage_ && debugPage_->HasValues() &&
        (debugPageVisible || diagnosticsCopyTextRequested_)) {
        const auto capabilities = engine_->Capabilities();
        const auto onlineMembers = std::count_if(
            snapshot.room.members.begin(), snapshot.room.members.end(),
            [](const RoomMemberSnapshot& member) { return member.online; });
        const auto activePeers = std::count_if(
            snapshot.roomActivity.peerConnections.begin(),
            snapshot.roomActivity.peerConnections.end(),
            [](const RoomPeerConnectionSnapshot& peer) {
                return peer.state == RoomPeerConnectionState::kActive;
            });
        const auto setDebugValue =
            [this](const QString& key, const QString& value,
                   const char* tone = "normal") {
                debugPage_->SetValue(key, value, tone);
            };
        const QString deviceId = snapshot.localDeviceId.empty()
                                     ? QStringLiteral("未注册")
                                     : QString::fromStdString(
                                           snapshot.localDeviceId);
        const QString signaling =
            ConnectivityDebugName(snapshot.connectivity);
        const bool signalingOnline =
            snapshot.connectivity == SessionConnectivityState::kOnline;
        const QString webRtc =
            capabilities.webRtcReady ? QStringLiteral("运行正常")
                                     : QStringLiteral("不可用");
        const QString systemEnvironment =
            QStringLiteral("%1 · %2")
                .arg(
                    capabilities.operatingSystemDescription.empty()
                        ? QStringLiteral("Windows 版本未报告")
                        : QString::fromStdString(
                              capabilities.operatingSystemDescription),
                    capabilities.nativeArchitecture.empty()
                        ? QStringLiteral("架构未知")
                        : QString::fromStdString(
                              capabilities.nativeArchitecture));
        const QString sessionEnvironment =
            capabilities.remoteSession
                ? QStringLiteral("Windows RDP 远程会话")
                : QStringLiteral("本地控制台会话");
        QStringList graphicsAdapters;
        for (const auto& adapter :
             capabilities.graphicsAdapterDescriptions) {
            graphicsAdapters
                << QString::fromStdString(adapter);
        }
        if (!capabilities.graphicsEnumerationError.empty()) {
            graphicsAdapters
                << QStringLiteral("错误：%1")
                       .arg(QString::fromStdString(
                           capabilities.graphicsEnumerationError));
        }
        if (graphicsAdapters.isEmpty()) {
            graphicsAdapters
                << QStringLiteral("没有枚举到 DXGI 图形适配器。");
        }
        const QString hardwareFingerprint =
            capabilities.hardwareFingerprint.empty()
                ? QStringLiteral("未生成")
                : QString::fromStdString(
                      capabilities.hardwareFingerprint);
        QString encoderProbeSource;
        if (capabilities.h264HardwareEncoderProbeFromCache) {
            encoderProbeSource =
                QStringLiteral("已复用当前硬件指纹的缓存结果");
        } else if (
            capabilities.h264HardwareEncoderProbeSucceeded) {
            encoderProbeSource =
                QStringLiteral("本次启动重新探测成功");
        } else if (
            capabilities.videoEncoderPreference == "software" ||
            capabilities.videoEncoderPreference == "ffmpeg") {
            encoderProbeSource =
                QStringLiteral("仅软件模式，未执行硬件探测");
        } else {
            encoderProbeSource =
                QStringLiteral("探测失败或尚未完成");
        }
        QString audioDeviceModule =
            capabilities.audioDeviceModuleCreated
                ? QStringLiteral("ADM 已创建并接入")
                : QStringLiteral("ADM 不可用");
        if (!capabilities.audioDeviceError.empty()) {
            audioDeviceModule += QStringLiteral("\n说明：%1")
                .arg(QString::fromStdString(
                    capabilities.audioDeviceError));
        }
        const QString cameraDevice =
            MediaDeviceCategoryDebugText(
                snapshot.media.localMediaDevices.camera);
        const QString microphoneDevice =
            MediaDeviceCategoryDebugText(
                snapshot.media.localMediaDevices.microphone);
        const QString speakerDevice =
            MediaDeviceCategoryDebugText(
                snapshot.media.localMediaDevices.speaker);
        const QString builtinEncoder =
            capabilities.hasH264Encoder
                ? QStringLiteral("可用")
                : QStringLiteral("不可用");
        const QString desktopCapture =
            capabilities.desktopCapturePreference == "libwebrtc"
                ? QStringLiteral("libwebrtc（像素差分）")
                : QStringLiteral("自研 DXGI（GPU 原生纹理）");
        QString encoderPreference = QStringLiteral("自动（硬件优先）");
        if (capabilities.videoEncoderPreference == "hardware") {
            encoderPreference =
                QStringLiteral("硬件编码（失败回退 FFmpeg/libx264）");
        } else if (capabilities.videoEncoderPreference ==
                   "ffmpeg_hardware") {
            encoderPreference =
                QStringLiteral(
                    "FFmpeg 硬件编码（失败回退 FFmpeg/libx264）");
        } else if (capabilities.videoEncoderPreference == "software") {
            encoderPreference = QStringLiteral("软件编码（OpenH264）");
        } else if (capabilities.videoEncoderPreference == "ffmpeg") {
            encoderPreference = QStringLiteral("软件编码（FFmpeg/libx264）");
        }
        const QString configuredEncoderQuality = QSettings().value(
            QString::fromLatin1(kFfmpegX264PresetSetting),
            QStringLiteral("medium")).toString().toLower();
        QString encoderQuality = QStringLiteral(
            "中 · libx264 medium · NVENC p4 · QSV medium · AMF balanced · MFT 50 · OpenH264 MEDIUM");
        if (configuredEncoderQuality == QStringLiteral("veryslow")) {
            encoderQuality = QStringLiteral(
                "极高 · libx264 veryslow · NVENC p7 · QSV veryslow · AMF quality · MFT 100 · OpenH264 HIGH");
        } else if (configuredEncoderQuality == QStringLiteral("slow")) {
            encoderQuality = QStringLiteral(
                "高 · libx264 slow · NVENC p6 · QSV slow · AMF quality · MFT 75 · OpenH264 HIGH");
        } else if (configuredEncoderQuality == QStringLiteral("veryfast")) {
            encoderQuality = QStringLiteral(
                "低 · libx264 veryfast · NVENC p2 · QSV faster · AMF speed · MFT 25 · OpenH264 LOW");
        } else if (configuredEncoderQuality == QStringLiteral("ultrafast")) {
            encoderQuality = QStringLiteral(
                "极低 · libx264 ultrafast · NVENC p1 · QSV veryfast · AMF speed · MFT 0 · OpenH264 LOW");
        }
        const QString hardwareEncoderCount =
            capabilities.h264HardwareEncoderCount == 0
                ? QStringLiteral("未检测到")
                : QStringLiteral("检测到 %1 个")
                      .arg(capabilities.h264HardwareEncoderCount);
        const QString cpuNv12 =
            capabilities.h264HardwareEncoderCpuNv12InputSupported
                ? QStringLiteral("支持")
                : QStringLiteral("暂未确认");
        const QString d3d11Encoder =
            capabilities.h264HardwareEncoderWired
                ? QStringLiteral("已接入（硬件优先）")
                : (capabilities.videoEncoderPreference == "software" ||
                   capabilities.videoEncoderPreference == "ffmpeg")
                      ? QStringLiteral("当前模式已禁用")
                      : QStringLiteral("未接入");
        const QString ffmpegHardwareEncoder =
            capabilities.ffmpegHardwareEncoderWired
                ? QStringLiteral("已接入并作为当前硬件路径")
                : capabilities.ffmpegHardwareEncoderAvailable
                    ? QStringLiteral("运行时可用，当前模式未选择")
                    : capabilities.ffmpegHardwareEncoderError.empty()
                        ? QStringLiteral("未接入")
                        : QStringLiteral("未接入：%1").arg(
                              QString::fromStdString(
                                  capabilities.ffmpegHardwareEncoderError));
        QString encoderFallback;
        if (capabilities.videoEncoderPreference == "hardware") {
            encoderFallback = QStringLiteral("当前模式已禁用");
        } else if (!capabilities.h264SoftwareEncoderWired) {
            encoderFallback =
                capabilities.videoEncoderPreference == "ffmpeg" &&
                        !capabilities.ffmpegX264EncoderError.empty()
                    ? QStringLiteral("未接入：%1")
                          .arg(QString::fromStdString(
                              capabilities.ffmpegX264EncoderError))
                    : QStringLiteral("未接入");
        } else if (capabilities.videoEncoderPreference == "software") {
            encoderFallback = QStringLiteral("OpenH264 已接入（显式选择）");
        } else if (capabilities.videoEncoderPreference == "ffmpeg") {
            encoderFallback = QStringLiteral("FFmpeg/libx264 已接入（显式选择）");
        } else if (capabilities.h264HardwareEncoderWired) {
            encoderFallback =
                QStringLiteral("已接入（硬件失败时回退）");
        } else {
            encoderFallback =
                QStringLiteral("已接入（当前主编码路径）");
        }
        const QString decoder =
            capabilities.hasH264Decoder ? QStringLiteral("可用")
                                        : QStringLiteral("不可用");
        QString decoderPreference =
            QStringLiteral("自动（FFmpeg 硬件优先，软件回退）");
        if (capabilities.videoDecoderPreference == "hardware") {
            decoderPreference = QStringLiteral("仅硬件");
        } else if (capabilities.videoDecoderPreference == "software") {
            decoderPreference = QStringLiteral("仅软件");
        }
        QString mfDecoderType;
        if (capabilities.mfD3D11DecoderHardware) {
            mfDecoderType = QStringLiteral("MF 硬件解码");
        } else if (capabilities.mfD3D11DecoderSoftware) {
            mfDecoderType =
                QStringLiteral("MF 软件解码（支持 D3D11 输出）");
        } else if (capabilities.mfD3D11DecoderConfigured) {
            mfDecoderType = QStringLiteral("MF 解码（类型未识别）");
        } else {
            mfDecoderType = QStringLiteral("未配置");
        }
        const QString mfDecoderName =
            capabilities.mfD3D11DecoderName.empty()
                ? QStringLiteral("当前没有可用的 MF D3D11 H264 解码器。")
                : QString::fromStdString(
                      capabilities.mfD3D11DecoderName);
        const QString d3d11Output =
            capabilities.d3d11NativeDecoderOutput
                ? QStringLiteral("支持")
                : QStringLiteral("不支持或未启用");
        const QString mfDecoderAsync =
            capabilities.mfD3D11DecoderHardware
                ? capabilities.mfD3D11DecoderAsynchronous
                      ? QStringLiteral("异步事件驱动")
                      : QStringLiteral("同步调用")
                : QStringLiteral("当前未使用硬件 MFT");
        const QString softwareFallback =
            capabilities.ffmpegSoftwareH264Decoder
                ? QStringLiteral("已接入")
                : QStringLiteral("未接入");
        const QString mfDecoderError =
            capabilities.mfD3D11DecoderError.empty()
                ? QStringLiteral("当前没有 MF 解码检测错误。")
                : QString::fromStdString(
                      capabilities.mfD3D11DecoderError);

        QStringList encoderDetails;
        for (const auto& encoder :
             capabilities.h264HardwareEncoderDescriptions) {
            encoderDetails
                << QString::fromStdString(encoder);
        }
        for (const auto& warning :
             capabilities.h264HardwareEncoderWarnings) {
            encoderDetails
                << QStringLiteral("警告：%1")
                       .arg(QString::fromStdString(warning));
        }
        for (const auto& encoder :
             capabilities.ffmpegHardwareEncoderDescriptions) {
            encoderDetails
                << QStringLiteral("FFmpeg 硬编：%1")
                       .arg(QString::fromStdString(encoder));
        }
        if (encoderDetails.isEmpty()) {
            encoderDetails
                << QStringLiteral("当前机器没有报告可用的硬件 H264 编码器。");
        }
        QStringList encoderRuntimeDetails;
        for (const auto& detail :
             capabilities.videoEncoderRuntimeDetails) {
            encoderRuntimeDetails
                << QString::fromStdString(detail);
        }
        if (encoderRuntimeDetails.isEmpty()) {
            encoderRuntimeDetails
                << QStringLiteral("当前没有活动的视频编码实例。");
        }
        const QString encoderFallbackReason =
            capabilities.videoEncoderLastFallbackReason.empty()
                ? QStringLiteral("当前没有发生硬件编码回退。")
                : QString::fromStdString(
                      capabilities.videoEncoderLastFallbackReason);

        const QString membership =
            RoomMembershipDebugName(snapshot.room.membership);
        const QString roomId = snapshot.room.roomId.empty()
                                   ? QStringLiteral("当前未加入房间")
                                   : QString::fromStdString(
                                         snapshot.room.roomId);
        const QString members =
            QStringLiteral("%1 人在线 · %2 个席位已占用 · 上限 %3 人")
                .arg(static_cast<qulonglong>(onlineMembers))
                .arg(static_cast<qulonglong>(
                    snapshot.room.members.size()))
                .arg(snapshot.room.capacity);
        const QString screenSharer =
            snapshot.room.screenSharerDeviceId.empty()
                ? QStringLiteral("无人共享")
                : MemberDisplayName(snapshot.room,
                                    snapshot.room.screenSharerDeviceId,
                                    snapshot.localDeviceId);
        const QString controller =
            snapshot.room.activeControllerDeviceId.empty()
                ? QStringLiteral("无人控制")
                : MemberDisplayName(snapshot.room,
                                    snapshot.room.activeControllerDeviceId,
                                    snapshot.localDeviceId);
        const QString controlGrant =
            snapshot.room.activeControllerDeviceId.empty()
                ? QStringLiteral("当前没有控制租约")
                : snapshot.roomControlGrantActive
                      ? QStringLiteral("授权令牌已同步")
                      : QStringLiteral("正在等待信令授权令牌");
        const QString peerSummary =
            QStringLiteral("%1 / %2 个成员对已连接")
                .arg(static_cast<qulonglong>(activePeers))
                .arg(static_cast<qulonglong>(
                    snapshot.roomActivity.peerConnections.size()));

        QStringList peerDetails;
        for (const auto& peer : snapshot.roomActivity.peerConnections) {
            const QString screenStartup =
                peer.screenFirstFramePresentedGeneration != 0
                    ? QStringLiteral("首屏已显示 · %1 ms · 刷新请求 %2 次")
                          .arg(peer.screenFirstFrameStartupMs)
                          .arg(peer.screenStartupRefreshRequests)
                    : peer.screenStartupRefreshRequests > 0
                        ? QStringLiteral("等待首屏 · 刷新请求 %1 次")
                              .arg(peer.screenStartupRefreshRequests)
                        : QStringLiteral("尚未启动");
            peerDetails
                << QStringLiteral(
                       "%1\n状态：%2 · DataChannel：%3 条 · 媒体槽：%4 个\n输入通道：键盘/可靠 %5 · 鼠标/快速 %6 · 显示映射 %7\n协商代次：%8 · ICE Restart：%9/3\n视频启动：%10\n成员对 ID：%11")
                       .arg(MemberDisplayName(
                                snapshot.room,
                                peer.peerDeviceId,
                                snapshot.localDeviceId),
                            PeerConnectionDebugName(peer.state))
                       .arg(peer.openDataChannelCount)
                       .arg(peer.preparedVideoSlotCount)
                       .arg(peer.controlReliableChannelOpen
                                ? QStringLiteral("已就绪")
                                : QStringLiteral("未就绪"))
                       .arg(peer.inputFastChannelOpen
                                ? QStringLiteral("已就绪")
                                : QStringLiteral("未就绪"))
                       .arg(peer.sharedDisplayId != 0 &&
                                    peer.sharedDisplayLayoutVersion != 0
                                ? QStringLiteral("已同步")
                                : QStringLiteral("未同步"))
                       .arg(peer.negotiationGeneration)
                       .arg(peer.iceRestartAttempt)
                       .arg(screenStartup)
                       .arg(QString::fromStdString(peer.pairId));
            if (!peer.errorMessage.empty()) {
                peerDetails
                    << QStringLiteral("错误：%1")
                           .arg(QString::fromStdString(
                               peer.errorMessage));
            }
        }
        if (peerDetails.isEmpty()) {
            peerDetails
                << QStringLiteral("当前没有需要显示的成员对连接。");
        }

        const QString sessionError =
            snapshot.error.message.empty()
                ? QStringLiteral("暂无错误")
                : QString::fromStdString(snapshot.error.message);
        const QString roomError =
            snapshot.room.errorMessage.empty()
                ? QStringLiteral("暂无错误")
                : QString::fromStdString(
                      snapshot.room.errorMessage);
        const app::ClipboardControllerSnapshot clipboard =
            clipboardController_
                ? clipboardController_->Snapshot()
                : app::ClipboardControllerSnapshot{};
        const QString clipboardState =
            !clipboard.enabled
                ? QStringLiteral("已关闭")
                : !clipboard.sessionActive
                      ? QStringLiteral("等待远程控制权限")
                      : QString::fromStdString(clipboard.state);
        const QString clipboardPeer = clipboard.peerDeviceId.empty()
            ? QStringLiteral("当前没有活动成员对")
            : QString::fromStdString(clipboard.peerDeviceId);
        const QString clipboardTransfer = clipboard.transferActive
            ? QStringLiteral(
                  "%1 · %2 · %3 / %4 字节 · %5 个项目 · "
                  "%6 KB/s · 预计剩余 %7 秒")
                  .arg(clipboard.transferOutgoing
                           ? QStringLiteral("发送")
                           : QStringLiteral("接收"))
                  .arg(QString::fromStdString(clipboard.state))
                  .arg(static_cast<qulonglong>(
                      clipboard.transferCompletedBytes))
                  .arg(static_cast<qulonglong>(
                      clipboard.transferTotalBytes))
                  .arg(clipboard.transferItemCount)
                  .arg(clipboard.transferBytesPerSecond / 1024.0,
                       0, 'f', 1)
                  .arg(static_cast<qulonglong>(
                      clipboard.transferEstimatedRemainingSeconds))
            : QStringLiteral("当前没有活动传输");
        const QString clipboardLast = clipboard.lastFormat.empty()
            ? QStringLiteral("尚未执行远程粘贴")
            : QStringLiteral("%1 · %2 字节")
                  .arg(QString::fromStdString(clipboard.lastFormat))
                  .arg(static_cast<qulonglong>(clipboard.lastBytes));
        const QString clipboardCounters = QStringLiteral(
            "发起粘贴 %1 项 · 接收粘贴 %2 项 · 失败/拒绝 %3 项")
            .arg(static_cast<qulonglong>(clipboard.sentItems))
            .arg(static_cast<qulonglong>(clipboard.receivedItems))
            .arg(static_cast<qulonglong>(clipboard.rejectedItems));
        const QString clipboardError =
            clipboard.lastErrorMessage.empty()
                ? QStringLiteral("暂无错误")
                : QString::fromStdString(clipboard.lastErrorMessage);

        setDebugValue(QStringLiteral("deviceId"), deviceId,
                      snapshot.localDeviceId.empty() ? "warning"
                                                     : "normal");
        setDebugValue(QStringLiteral("signaling"), signaling,
                      signalingOnline ? "good" : "warning");
        setDebugValue(QStringLiteral("webrtc"), webRtc,
                      capabilities.webRtcReady ? "good" : "error");
        setDebugValue(QStringLiteral("systemEnvironment"),
                      systemEnvironment);
        setDebugValue(
            QStringLiteral("sessionEnvironment"),
            sessionEnvironment,
            capabilities.remoteSession ? "warning" : "good");
        setDebugValue(
            QStringLiteral("graphicsAdapters"),
            graphicsAdapters.join(QStringLiteral("\n")),
            capabilities.graphicsEnumerationError.empty()
                ? "normal"
                : "warning");
        setDebugValue(QStringLiteral("hardwareFingerprint"),
                      hardwareFingerprint,
                      capabilities.hardwareFingerprint.empty()
                          ? "warning"
                          : "muted");
        setDebugValue(
            QStringLiteral("encoderProbeSource"),
            encoderProbeSource,
            capabilities.h264HardwareEncoderProbeSucceeded
                ? "good"
                : (capabilities.videoEncoderPreference == "software" ||
                   capabilities.videoEncoderPreference == "ffmpeg")
                      ? "normal"
                      : "warning");
        setDebugValue(
            QStringLiteral("audioDeviceModule"),
            audioDeviceModule,
            capabilities.audioDeviceModuleCreated ? "good"
                                                  : "warning");
        setDebugValue(QStringLiteral("cameraDevice"),
                      cameraDevice);
        setDebugValue(QStringLiteral("microphoneDevice"),
                      microphoneDevice);
        setDebugValue(QStringLiteral("speakerDevice"),
                      speakerDevice);
        setDebugValue(QStringLiteral("builtinEncoder"), builtinEncoder,
                      capabilities.hasH264Encoder ? "good" : "error");
        setDebugValue(QStringLiteral("desktopCapture"),
                      desktopCapture, "good");
        setDebugValue(QStringLiteral("encoderPreference"),
                      encoderPreference, "good");
        setDebugValue(QStringLiteral("encoderQuality"),
                      encoderQuality, "good");
        setDebugValue(QStringLiteral("hardwareEncoderCount"),
                      hardwareEncoderCount,
                      capabilities.h264HardwareEncoderAvailable
                          ? "good"
                          : "warning");
        setDebugValue(QStringLiteral("cpuNv12"), cpuNv12,
                      capabilities
                              .h264HardwareEncoderCpuNv12InputSupported
                          ? "good"
                          : "warning");
        setDebugValue(QStringLiteral("d3d11Encoder"), d3d11Encoder,
                      capabilities.h264HardwareEncoderWired
                          ? "good"
                          : "warning");
        setDebugValue(QStringLiteral("encoderFallback"),
                      encoderFallback,
                      capabilities.h264SoftwareEncoderWired
                          ? "good"
                          : capabilities.videoEncoderPreference ==
                                    "hardware"
                                ? "normal"
                                : "error");
        setDebugValue(QStringLiteral("encoderRuntime"),
                      encoderRuntimeDetails.join(
                          QStringLiteral("\n\n")));
        setDebugValue(
            QStringLiteral("encoderFallbackReason"),
            encoderFallbackReason,
            capabilities.videoEncoderLastFallbackReason.empty()
                ? "muted"
                : "warning");
        setDebugValue(QStringLiteral("decoder"), decoder,
                      capabilities.hasH264Decoder ? "good" : "error");
        setDebugValue(QStringLiteral("decoderPreference"),
                      decoderPreference, "good");
        setDebugValue(
            QStringLiteral("mfDecoderType"), mfDecoderType,
            capabilities.mfD3D11DecoderHardware
                ? "good"
                : capabilities.mfD3D11DecoderSoftware
                      ? "warning"
                      : "normal");
        setDebugValue(QStringLiteral("mfDecoderName"),
                      mfDecoderName,
                      capabilities.mfD3D11DecoderConfigured
                          ? "good"
                          : "muted");
        setDebugValue(QStringLiteral("d3d11Output"), d3d11Output,
                      capabilities.d3d11NativeDecoderOutput
                          ? "good"
                          : "warning");
        setDebugValue(QStringLiteral("mfDecoderAsync"),
                      mfDecoderAsync,
                      capabilities.mfD3D11DecoderHardware
                          ? "good"
                          : "muted");
        setDebugValue(QStringLiteral("softwareFallback"),
                      softwareFallback,
                      capabilities.ffmpegSoftwareH264Decoder
                          ? "good"
                          : "error");
        setDebugValue(
            QStringLiteral("mfDecoderError"), mfDecoderError,
            capabilities.mfD3D11DecoderError.empty()
                ? "muted"
                : "warning");
        setDebugValue(QStringLiteral("encoderDetails"),
                      encoderDetails.join(QStringLiteral("\n\n")));
        setDebugValue(QStringLiteral("membership"), membership,
                      snapshot.room.membership ==
                              RoomMembershipState::kActive
                          ? "good"
                          : "normal");
        setDebugValue(QStringLiteral("roomId"), roomId);
        setDebugValue(QStringLiteral("members"), members);
        setDebugValue(QStringLiteral("screenSharer"), screenSharer);
        setDebugValue(
            QStringLiteral("screenShareGeneration"),
            QString::number(snapshot.screenShare.generation));
        setDebugValue(QStringLiteral("controller"), controller);
        setDebugValue(QStringLiteral("controlGrant"), controlGrant,
                      snapshot.room.activeControllerDeviceId.empty()
                          ? "muted"
                          : snapshot.roomControlGrantActive
                                ? "good" : "warning");
        setDebugValue(QStringLiteral("peerSummary"), peerSummary,
                      activePeers > 0 ? "good" : "normal");
        setDebugValue(QStringLiteral("peerDetails"),
                      peerDetails.join(QStringLiteral("\n\n")));
        setDebugValue(QStringLiteral("clipboardState"), clipboardState,
                      clipboard.enabled && clipboard.sessionActive
                          ? "good" : "normal");
        setDebugValue(QStringLiteral("clipboardPeer"), clipboardPeer);
        setDebugValue(QStringLiteral("clipboardTransfer"),
                      clipboardTransfer,
                      clipboard.transferActive ? "warning" : "muted");
        setDebugValue(QStringLiteral("clipboardLast"), clipboardLast);
        setDebugValue(QStringLiteral("clipboardCounters"),
                      clipboardCounters);
        setDebugValue(QStringLiteral("clipboardError"), clipboardError,
                      clipboard.lastErrorMessage.empty() ? "muted"
                                                         : "warning");
        setDebugValue(QStringLiteral("sessionError"), sessionError,
                      snapshot.error.message.empty() ? "muted"
                                                    : "error");
        setDebugValue(QStringLiteral("roomError"), roomError,
                      snapshot.room.errorMessage.empty() ? "muted"
                                                         : "error");
        if (diagnosticsCopyTextRequested_) {
        const QSettings encoderSelectionSettings;
        const QString encoderSelectionCaptureBackend =
            encoderSelectionSettings.value(
                QString::fromLatin1(kDesktopCaptureBackendSetting),
                QStringLiteral("libwebrtc")).toString();
        const QString encoderSelectionX264Preset =
            encoderSelectionSettings.value(
                QString::fromLatin1(kFfmpegX264PresetSetting),
                QStringLiteral("medium")).toString();
        const bool encoderSelectionCurrent =
            encoderSelectionSettings.value(
                QString::fromLatin1(kEncoderBenchmarkCompletedSetting),
                false).toBool() &&
            encoderSelectionSettings.value(
                QString::fromLatin1(kEncoderBenchmarkPassedSetting),
                false).toBool() &&
            encoderSelectionSettings.value(
                QString::fromLatin1(kEncoderBenchmarkPolicyVersionSetting),
                0).toInt() == kEncoderBenchmarkPolicyVersion &&
            encoderSelectionSettings.value(
                QString::fromLatin1(kEncoderHardwareFingerprintSetting))
                .toString() == hardwareFingerprint &&
            encoderSelectionSettings.value(
                QString::fromLatin1(kEncoderCaptureBackendSetting))
                .toString() == encoderSelectionCaptureBackend &&
            encoderSelectionSettings.value(
                QString::fromLatin1(kEncoderX264PresetSetting))
                .toString() == encoderSelectionX264Preset;
        const QString automaticEncoderSelection = encoderSelectionCurrent
            ? QStringLiteral("%1 [%2]")
                  .arg(encoderSelectionSettings.value(
                           QString::fromLatin1(kBestEncoderNameSetting))
                           .toString(),
                       encoderSelectionSettings.value(
                           QString::fromLatin1(kBestEncoderIdSetting))
                           .toString())
            : QStringLiteral("等待当前采集方式的性能检测");
        QStringList copyLines;
        copyLines
            << QStringLiteral("RLink 调试信息")
            << QStringLiteral("================")
            << QString()
            << QStringLiteral("【设备与信令】")
            << QStringLiteral("本机设备 ID：%1").arg(deviceId)
            << QStringLiteral("信令连接：%1").arg(signaling)
            << QStringLiteral("WebRTC 运行时：%1").arg(webRtc)
            << QString()
            << QStringLiteral("【兼容性环境】")
            << QStringLiteral("系统与架构：%1")
                   .arg(systemEnvironment)
            << QStringLiteral("会话类型：%1")
                   .arg(sessionEnvironment)
            << QStringLiteral("图形适配器：%1")
                   .arg(graphicsAdapters.join(
                       QStringLiteral("；")))
            << QStringLiteral("硬件能力指纹：%1")
                   .arg(hardwareFingerprint)
            << QStringLiteral("编码能力探测：%1")
                   .arg(encoderProbeSource)
            << QStringLiteral("WebRTC 音频模块：%1")
                   .arg(audioDeviceModule.replace(
                       QLatin1Char('\n'), QStringLiteral("；")))
            << QStringLiteral("摄像头：%1")
                   .arg(QString(cameraDevice).replace(
                       QLatin1Char('\n'), QStringLiteral("；")))
            << QStringLiteral("麦克风：%1")
                   .arg(QString(microphoneDevice).replace(
                       QLatin1Char('\n'), QStringLiteral("；")))
            << QStringLiteral("扬声器：%1")
                   .arg(QString(speakerDevice).replace(
                       QLatin1Char('\n'), QStringLiteral("；")))
            << QString()
            << QStringLiteral("【媒体能力】")
            << QStringLiteral("H264 编码链路：%1")
                   .arg(builtinEncoder)
            << QStringLiteral("屏幕采集模式：%1")
                   .arg(desktopCapture)
            << QStringLiteral("视频编码模式：%1")
                   .arg(encoderPreference)
            << QStringLiteral("编码质量：%1")
                   .arg(encoderQuality)
            << QStringLiteral("自动编码实测选择：%1")
                   .arg(automaticEncoderSelection)
            << QStringLiteral("硬件编码器：%1")
                   .arg(hardwareEncoderCount)
            << QStringLiteral("CPU NV12 输入：%1").arg(cpuNv12)
            << QStringLiteral("D3D11 硬件编码：%1")
                   .arg(d3d11Encoder)
            << QStringLiteral("FFmpeg 硬件编码：%1")
                   .arg(ffmpegHardwareEncoder)
            << QStringLiteral("软件编码链路：%1")
                   .arg(encoderFallback)
            << QStringLiteral("当前编码实例：%1")
                   .arg(encoderRuntimeDetails.join(
                       QStringLiteral("；")))
            << QStringLiteral("最近回退原因：%1")
                   .arg(encoderFallbackReason)
            << QStringLiteral("H264 解码：%1").arg(decoder)
            << QStringLiteral("视频解码模式：%1")
                   .arg(decoderPreference)
            << QStringLiteral("MF D3D11 解码类型：%1")
                   .arg(mfDecoderType)
            << QStringLiteral("MF 解码器实例：%1")
                   .arg(mfDecoderName)
            << QStringLiteral("D3D11 原生输出：%1")
                   .arg(d3d11Output)
            << QStringLiteral("硬件 MFT 驱动：%1")
                   .arg(mfDecoderAsync)
            << QStringLiteral("FFmpeg 软件解码：%1")
                   .arg(softwareFallback)
            << QStringLiteral("MF 解码检测说明：%1")
                   .arg(mfDecoderError)
            << encoderDetails
            << QString()
            << QStringLiteral("【房间状态】")
            << QStringLiteral("状态：%1").arg(membership)
            << QStringLiteral("房间 ID：%1").arg(roomId)
            << QStringLiteral("成员与席位：%1").arg(members)
            << QStringLiteral("当前主机器：%1").arg(screenSharer)
            << QStringLiteral("当前控制者：%1").arg(controller)
            << QStringLiteral("控制授权令牌：%1").arg(controlGrant)
            << QString()
            << QStringLiteral("【成员连接】")
            << peerSummary
            << peerDetails
            << QString()
            << QStringLiteral("【远程粘贴】")
            << QStringLiteral("状态：%1").arg(clipboardState)
            << QStringLiteral("成员对：%1").arg(clipboardPeer)
            << QStringLiteral("当前传输：%1").arg(clipboardTransfer)
            << QStringLiteral("最近项目：%1").arg(clipboardLast)
            << QStringLiteral("累计统计：%1").arg(clipboardCounters)
            << QStringLiteral("最近错误：%1").arg(clipboardError)
            << QString()
            << QStringLiteral("【最近错误】")
            << QStringLiteral("会话错误：%1").arg(sessionError)
            << QStringLiteral("房间错误：%1").arg(roomError);
        debugCopyText_ = copyLines.join(QLatin1Char('\n'));

        QStringList mediaCopyLines;
        mediaCopyLines
            << QStringLiteral("RLink 媒体能力")
            << QStringLiteral("================")
            << QStringLiteral("H264 编码链路：%1")
                   .arg(builtinEncoder)
            << QStringLiteral("屏幕采集模式：%1")
                   .arg(desktopCapture)
            << QStringLiteral("视频编码模式：%1")
                   .arg(encoderPreference)
            << QStringLiteral("编码质量：%1")
                   .arg(encoderQuality)
            << QStringLiteral("自动编码实测选择：%1")
                   .arg(automaticEncoderSelection)
            << QStringLiteral("硬件编码器：%1")
                   .arg(hardwareEncoderCount)
            << QStringLiteral("CPU NV12 输入：%1").arg(cpuNv12)
            << QStringLiteral("D3D11 硬件编码：%1")
                   .arg(d3d11Encoder)
            << QStringLiteral("FFmpeg 硬件编码：%1")
                   .arg(ffmpegHardwareEncoder)
            << QStringLiteral("软件编码链路：%1")
                   .arg(encoderFallback)
            << QStringLiteral("当前编码实例：%1")
                   .arg(encoderRuntimeDetails.join(
                       QStringLiteral("；")))
            << QStringLiteral("最近回退原因：%1")
                   .arg(encoderFallbackReason)
            << QStringLiteral("H264 解码：%1").arg(decoder)
            << QStringLiteral("视频解码模式：%1")
                   .arg(decoderPreference)
            << QStringLiteral("MF D3D11 解码类型：%1")
                   .arg(mfDecoderType)
            << QStringLiteral("MF 解码器实例：%1")
                   .arg(mfDecoderName)
            << QStringLiteral("D3D11 原生输出：%1")
                   .arg(d3d11Output)
            << QStringLiteral("硬件 MFT 驱动：%1")
                   .arg(mfDecoderAsync)
            << QStringLiteral("FFmpeg 软件解码：%1")
                   .arg(softwareFallback)
            << QStringLiteral("MF 解码检测说明：%1")
                   .arg(mfDecoderError)
            << encoderDetails;
        mediaDebugCopyText_ =
            mediaCopyLines.join(QLatin1Char('\n'));
        }
    }
}

}  // namespace remote::controller
