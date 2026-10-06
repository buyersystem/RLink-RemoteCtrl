// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ControllerMainWindow.h"
#include "ControllerMainWindowSupport.h"

#include <QDateTime>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QStyle>
#include <QStringList>
#include <QTimer>
#include <QThread>
#include <QVariant>
#include <QMetaObject>
#include <utility>
#include <algorithm>
#include "CameraWindow.h"
#include "RemoteCDialog.h"
#include "RemoteSessionWindow.h"
#include "RemoteCToast.h"
#include "RoomCameraWindow.h"
#include "src/apps/remote/ClipboardController.h"
#include "src/platform/win/WindowsHardwareFingerprint.h"
#include "src/platform/win/WindowsInputExecutor.h"
#include "ScreenFrameRateLogger.h"
#include "pages/DirectConnectPage.h"
#include "pages/DiagnosticsCardsWidget.h"
#include "pages/ContentPolicyCards.h"
#include "pages/DiagnosticsPage.h"
#include "pages/SettingsPage.h"

namespace remote::controller {
using namespace detail;

void ControllerMainWindow::ScheduleDiagnosticsUiRefresh()
{
    if (diagnosticsUiRefreshPending_) return;
    diagnosticsUiRefreshPending_ = true;
    // Leave the input handler promptly so the selected page/indicator can be
    // painted first. Multiple navigation requests only refresh the final page.
    QTimer::singleShot(16, this, [this] {
        diagnosticsUiRefreshPending_ = false;
        if (!engine_ || !debugPage_ || !debugPage_->isVisibleTo(this)) return;
        RefreshDiagnosticsSnapshotUi(engine_->Snapshot());
        RefreshDiagnosticsUi();
    });
}

void ControllerMainWindow::RefreshDiagnosticsUi()
{
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(
            this, [this] { RefreshDiagnosticsUi(); },
            Qt::QueuedConnection);
        return;
    }
    if (!engine_) {
        return;
    }

    const bool diagnosticsPageVisible =
        debugPage_ && debugPage_->isVisibleTo(this);
    const bool sessionHudNeeded =
        remoteSessionWindow_ && remoteSessionBinding_;
    const bool settingsTrafficNeeded = settingsPage_ && settingsPage_->isVisibleTo(this) &&
        settingsPage_->CurrentCategory() == 1;
    if (!diagnosticsPageVisible && !sessionHudNeeded && !settingsTrafficNeeded &&
        (!debugPage_ || !debugPage_->ScreenFrameRateLogEnabled()) &&
        !diagnosticsCopyTextRequested_) {
        return;
    }

    if (diagnosticsPageVisible && !debugPage_->NeedsRealtimeDiagnostics() &&
        !sessionHudNeeded && !settingsTrafficNeeded &&
        !debugPage_->ScreenFrameRateLogEnabled() &&
        !diagnosticsCopyTextRequested_) return;

    const auto diagnostics = engine_->Diagnostics();
    if (settingsTrafficNeeded) settingsPage_->UpdateScreenVideoTrafficEstimate(diagnostics);
    if (debugPage_ && debugPage_->ScreenFrameRateLogEnabled()) {
        AppendScreenFrameRateLog(diagnostics);
    }
    // The session HUD is independent from the optional diagnostics page.
    // Feed the explicitly bound transport before the optional diagnostics
    // page early return so latency, resolution and FPS always remain live.
    if (remoteSessionWindow_ && remoteSessionBinding_) {
        for (const auto& peer : diagnostics.peerConnections) {
            const bool directPeer =
                remoteSessionBinding_->IsDirect() && peer.pairId.empty();
            const bool roomPeer =
                remoteSessionBinding_->IsRoom() &&
                remoteSessionBinding_->roomPairId ==
                    QString::fromStdString(peer.pairId);
            if (directPeer || roomPeer) {
                remoteSessionWindow_->UpdateDiagnostics(peer);
                break;
            }
        }
    }
    if ((!diagnosticsPageVisible && !diagnosticsCopyTextRequested_) ||
        !debugPage_ || !debugPage_->HasValues()) {
        return;
    }
    if (!debugPage_->NeedsRealtimeDiagnostics() && !diagnosticsCopyTextRequested_) return;

    const auto setInputDebugValue =
        [this](const QString& key, const QString& value,
               const char* tone = "normal") {
            debugPage_->SetValue(key, value, tone);
        };
    const auto& input = diagnostics.remoteInput;
    const std::uint32_t dragSampleRateHz =
        QSettings().value(
            QString::fromLatin1(
                kDragPointerSampleRateSetting),
            QVariant::fromValue(240u)).toUInt();
    QString inputDebugText;
    if (!input.enabled) {
        const QString disabled =
            QStringLiteral("统计已关闭，远控输入功能仍正常运行。");
        for (const QString& key : {
                 QStringLiteral("inputMoveGenerated"),
                 QStringLiteral("inputMoveDispatched"),
                 QStringLiteral("inputImmediateEvents"),
                 QStringLiteral("inputPacketsSent"),
                 QStringLiteral("inputPacketsReceived"),
                 QStringLiteral("inputInjected")}) {
            setInputDebugValue(key, disabled, "muted");
        }
        setInputDebugValue(
            QStringLiteral("inputMovePolicy"),
            QStringLiteral(
                "高精度独立调度 · 普通移动 120–240 Hz · 按住拖动主动采样 %1 Hz")
                .arg(dragSampleRateHz),
            "good");
        inputDebugText =
            QStringLiteral("鼠标与键盘统计：已关闭");
    } else {
        const auto rateAndTotal =
            [](std::uint64_t rate, std::uint64_t total) {
                return QStringLiteral("%1 次/秒 · 累计 %2")
                    .arg(static_cast<qulonglong>(rate))
                    .arg(static_cast<qulonglong>(total));
            };
        const auto& rate = input.perSecond;
        const auto& total = input.totals;
        const QString movePolicy =
            QStringLiteral(
                "高精度独立调度 · 当前上限 %1 Hz · 采样窗 %2 ms\n"
                "普通移动按画面帧率两倍调度；按住拖动以 %3 Hz 主动读取 Windows 真实光标；按下、松开、滚轮和键盘立即发送。")
                .arg(input.moveDispatchRateLimitHz)
                .arg(input.sampleWindowMs)
                .arg(dragSampleRateHz);
        const QString generatedMoves = rateAndTotal(
            rate.generatedMouseMoves,
            total.generatedMouseMoves);
        const QString dispatchedMoves =
            QStringLiteral(
                "%1\n合并覆盖 %2\n"
                "发送间隔 平均 %3 ms · P95 %4 ms · 最大 %5 ms")
                .arg(
                    rateAndTotal(
                        rate.dispatchedMouseMoves,
                        total.dispatchedMouseMoves),
                    rateAndTotal(
                        rate.coalescedMouseMoves,
                        total.coalescedMouseMoves))
                .arg(
                    static_cast<double>(
                        input.moveDispatchIntervalAverageUs) /
                        1000.0,
                    0, 'f', 3)
                .arg(
                    static_cast<double>(
                        input.moveDispatchIntervalP95Us) /
                        1000.0,
                    0, 'f', 3)
                .arg(
                    static_cast<double>(
                        input.moveDispatchIntervalMaximumUs) /
                        1000.0,
                    0, 'f', 3);
        const QString immediateEvents =
            QStringLiteral(
                "鼠标按键 %1/%2 · 滚轮 %3/%4 · 键盘 %5/%6（每秒/累计）")
                .arg(
                    static_cast<qulonglong>(
                        rate.generatedMouseButtons))
                .arg(
                    static_cast<qulonglong>(
                        total.generatedMouseButtons))
                .arg(
                    static_cast<qulonglong>(
                        rate.generatedMouseWheels))
                .arg(
                    static_cast<qulonglong>(
                        total.generatedMouseWheels))
                .arg(
                    static_cast<qulonglong>(rate.generatedKeys))
                .arg(
                    static_cast<qulonglong>(total.generatedKeys));
        const QString packetsSent =
            QStringLiteral(
                "快速队列 %1/%2 · 可靠队列 %3/%4\n"
                "发送完成 %5/%6 · 失败 %7/%8（每秒/累计）")
                .arg(
                    static_cast<qulonglong>(rate.fastPacketsQueued))
                .arg(
                    static_cast<qulonglong>(total.fastPacketsQueued))
                .arg(
                    static_cast<qulonglong>(
                        rate.reliablePacketsQueued))
                .arg(
                    static_cast<qulonglong>(
                        total.reliablePacketsQueued))
                .arg(
                    static_cast<qulonglong>(rate.packetsSent))
                .arg(
                    static_cast<qulonglong>(total.packetsSent))
                .arg(
                    static_cast<qulonglong>(
                        rate.packetsSendFailed))
                .arg(
                    static_cast<qulonglong>(
                        total.packetsSendFailed));
        const QString packetsReceived =
            QStringLiteral(
                "收到 %1/%2 · 去重或拒绝 %3/%4（每秒/累计）")
                .arg(
                    static_cast<qulonglong>(rate.packetsReceived))
                .arg(
                    static_cast<qulonglong>(total.packetsReceived))
                .arg(
                    static_cast<qulonglong>(rate.packetsDropped))
                .arg(
                    static_cast<qulonglong>(total.packetsDropped));
        const QString injected =
            QStringLiteral(
                "移动 %1/%2 · 按键 %3/%4 · 滚轮 %5/%6 · 键盘 %7/%8\n"
                "注入失败 %9/%10（每秒/累计）")
                .arg(
                    static_cast<qulonglong>(
                        rate.injectedMouseMoves))
                .arg(
                    static_cast<qulonglong>(
                        total.injectedMouseMoves))
                .arg(
                    static_cast<qulonglong>(
                        rate.injectedMouseButtons))
                .arg(
                    static_cast<qulonglong>(
                        total.injectedMouseButtons))
                .arg(
                    static_cast<qulonglong>(
                        rate.injectedMouseWheels))
                .arg(
                    static_cast<qulonglong>(
                        total.injectedMouseWheels))
                .arg(
                    static_cast<qulonglong>(rate.injectedKeys))
                .arg(
                    static_cast<qulonglong>(total.injectedKeys))
                .arg(
                    static_cast<qulonglong>(
                        rate.injectionFailures))
                .arg(
                    static_cast<qulonglong>(
                        total.injectionFailures));
        setInputDebugValue(
            QStringLiteral("inputMovePolicy"), movePolicy, "good");
        setInputDebugValue(
            QStringLiteral("inputMoveGenerated"), generatedMoves);
        setInputDebugValue(
            QStringLiteral("inputMoveDispatched"), dispatchedMoves);
        setInputDebugValue(
            QStringLiteral("inputImmediateEvents"), immediateEvents);
        setInputDebugValue(
            QStringLiteral("inputPacketsSent"), packetsSent,
            total.packetsSendFailed == 0 ? "good" : "warning");
        setInputDebugValue(
            QStringLiteral("inputPacketsReceived"), packetsReceived);
        setInputDebugValue(
            QStringLiteral("inputInjected"), injected,
            total.injectionFailures == 0 ? "good" : "warning");
        inputDebugText =
            QStringList{
                QStringLiteral("调度：%1").arg(movePolicy),
                QStringLiteral("Qt 移动：%1").arg(generatedMoves),
                QStringLiteral("移动调度：%1").arg(dispatchedMoves),
                QStringLiteral("即时事件：%1").arg(immediateEvents),
                QStringLiteral("发送：%1").arg(packetsSent),
                QStringLiteral("接收：%1").arg(packetsReceived),
                QStringLiteral("注入：%1").arg(injected)}
                .join(QStringLiteral("\n"));
    }
    const auto& cursor = diagnostics.remoteCursor;
    inputDebugText += QStringLiteral(
        "\n\n独立远程光标：%1 · 形状 %2 · 显示器 %3 / 布局 %4\n"
        "发布 位置 %5 / 形状 %6 · 接收 位置 %7 / 形状 %8 · 远端已执行输入 #%9")
        .arg(cursor.publishing ? QStringLiteral("正在采集")
                               : QStringLiteral("未采集"),
             cursor.shapeAvailable ? QStringLiteral("已缓存")
                                   : QStringLiteral("等待"))
        .arg(cursor.displayId)
        .arg(static_cast<qulonglong>(cursor.displayLayoutVersion))
        .arg(static_cast<qulonglong>(cursor.positionMessagesPublished))
        .arg(static_cast<qulonglong>(cursor.shapeMessagesPublished))
        .arg(static_cast<qulonglong>(cursor.positionMessagesReceived))
        .arg(static_cast<qulonglong>(cursor.shapeMessagesReceived))
        .arg(static_cast<qulonglong>(cursor.lastAppliedInputSequence));
    QStringList overviewLines;
    QStringList iceLines;
    QStringList outboundLines;
    QStringList inboundLines;
    QStringList dataChannelLines;
    QVector<DiagnosticsCard> connectionCards;
    QVector<DiagnosticsCard> outboundCards;
    QVector<DiagnosticsCard> inboundCards;
    QVector<DiagnosticsCard> dataChannelCards;
    bool visionPerformanceAvailable = false;
    std::uint32_t visionScaleConvertTimeUs = 0;
    std::uint32_t visionJpegEncodeTimeUs = 0;
    std::uint64_t visionJpegBytes = 0;
    QString visionReturnedScene;
    QString visionAcceptedScene;
    std::uint32_t visionResultAgeMs = 0;
    QVector<DiagnosticsSection> policySections;
    double visionReturnedConfidence = 0.0;
    const bool connectionDetailsNeeded =
        debugPage_->CurrentCategory() == 2 || diagnosticsCopyTextRequested_;
    const bool policyDetailsNeeded =
        debugPage_->CurrentCategory() == 9 || diagnosticsCopyTextRequested_;

    for (const auto& peer : diagnostics.peerConnections) {
        const QString peerName =
            peer.peerDeviceId.empty()
                ? QString::fromStdString(peer.pairId)
                : QString::fromStdString(peer.peerDeviceId);
        const QString peerHeader =
            QStringLiteral("对端：%1").arg(
                peerName.isEmpty() ? QStringLiteral("未知成员") : peerName);
        const QString peerKey =
            peer.pairId.empty()
                ? (peerName.isEmpty() ? QStringLiteral("unknown")
                                      : peerName)
                : QString::fromStdString(peer.pairId);
        const auto& stats = peer.stats;
        if (policyDetailsNeeded) {
            policySections += MakeContentPolicySections(peerKey, peerName, stats.rtpStreams);
        }
        for (const auto& stream : stats.rtpStreams) {
            if (stream.contentAnalyzerBackend.find("vision_api") !=
                    std::string::npos &&
                (stream.contentLatestScaleConvertTimeUs > 0 ||
                 stream.contentLatestJpegEncodeTimeUs > 0 ||
                 stream.contentLatestJpegBytes > 0)) {
                visionPerformanceAvailable = true;
                visionScaleConvertTimeUs =
                    stream.contentLatestScaleConvertTimeUs;
                visionJpegEncodeTimeUs =
                    stream.contentLatestJpegEncodeTimeUs;
                visionJpegBytes = stream.contentLatestJpegBytes;
                visionReturnedScene = ContentSceneDisplayText(
                    stream.contentLatestReturnedScene);
                visionAcceptedScene = ContentSceneDisplayText(
                    stream.contentScene);
                visionResultAgeMs = stream.contentLatestReturnedAgeMs;
                visionReturnedConfidence =
                    stream.contentLatestReturnedSemanticConfidence;
            }
        }
        // Input/vision/policy tabs do not need hundreds of RTP chips and
        // copy-only text strings from the connection-quality workspace.
        if (!connectionDetailsNeeded) continue;
        const auto& transport = stats.transport;
        if (!transport.collected) {
            overviewLines
                << QStringLiteral("%1\n正在等待第一份 WebRTC Stats…")
                       .arg(peerHeader);
            connectionCards.push_back(DiagnosticsCard{
                peerKey,
                QStringLiteral("连接正在建立"),
                peerName,
                {{QStringLiteral("state"),
                  QStringLiteral("状态"),
                  QStringLiteral("正在等待第一份 WebRTC Stats"),
                  "warning",
                  true}}});
            continue;
        }
        QString collectedTime = QStringLiteral("未知");
        if (transport.timestampMs > 0) {
            collectedTime =
                QDateTime::fromMSecsSinceEpoch(transport.timestampMs)
                    .toString(QStringLiteral("HH:mm:ss"));
        }
        overviewLines
            << QStringLiteral(
                   "%1\n↑ %2 · ↓ %3 · 累计发送 %4 · 累计接收 %5 · RTT %6 ms · %7")
                   .arg(peerHeader)
                   .arg(FormatBitrate(transport.sendBitrateBps))
                   .arg(FormatBitrate(transport.receiveBitrateBps))
                   .arg(FormatByteCount(transport.bytesSent))
                   .arg(FormatByteCount(transport.bytesReceived))
                   .arg(transport.currentRoundTripTimeMs, 0, 'f', 1)
                   .arg(collectedTime);

        QString availableBandwidth =
            QStringLiteral("可用上行：%1 · 可用下行：%2")
                .arg(transport.availableOutgoingBitrateBps == 0
                         ? QStringLiteral("未报告")
                         : FormatBitrate(
                               transport.availableOutgoingBitrateBps))
                .arg(transport.availableIncomingBitrateBps == 0
                         ? QStringLiteral("未报告")
                         : FormatBitrate(
                               transport.availableIncomingBitrateBps));
        iceLines
            << QStringLiteral(
                   "%1\n%2 · 候选对 %3 · ICE %4 · DTLS %5 · 角色 %6\n"
                   "本地：%7\n远端：%8\n%9\n"
                   "TLS %10 · DTLS %11 · SRTP %12 · 候选对切换 %13 次")
                   .arg(peerHeader)
                   .arg(RouteDisplayName(transport.routeType))
                   .arg(QString::fromStdString(
                       transport.candidatePairState))
                   .arg(QString::fromStdString(transport.iceState))
                   .arg(QString::fromStdString(transport.dtlsState))
                   .arg(QString::fromStdString(transport.iceRole))
                   .arg(CandidateDisplayText(transport.localCandidate))
                   .arg(CandidateDisplayText(transport.remoteCandidate))
                   .arg(availableBandwidth)
                   .arg(QString::fromStdString(transport.tlsVersion))
                   .arg(QString::fromStdString(transport.dtlsCipher))
                   .arg(QString::fromStdString(transport.srtpCipher))
                   .arg(transport.selectedCandidatePairChanges);

        DiagnosticsCard connectionCard;
        connectionCard.key = peerKey;
        connectionCard.title = QStringLiteral("连接与安全");
        connectionCard.subtitle = peerName;
        connectionCard.chips = {
            {QStringLiteral("route"), QStringLiteral("连接路径"),
             RouteDisplayName(transport.routeType), "good", false},
            {QStringLiteral("rtt"), QStringLiteral("当前 RTT"),
             QStringLiteral("%1 ms").arg(
                 transport.currentRoundTripTimeMs, 0, 'f', 1),
             transport.currentRoundTripTimeMs >= 250.0
                 ? QByteArray("warning")
                 : QByteArray("good"),
             false},
            {QStringLiteral("sendRate"), QStringLiteral("采样窗上行"),
             FormatBitrate(transport.sendBitrateBps) +
                 SampleWindowSuffix(transport.sampleWindowMs),
             "normal", false},
            {QStringLiteral("receiveRate"), QStringLiteral("采样窗下行"),
             FormatBitrate(transport.receiveBitrateBps) +
                 SampleWindowSuffix(transport.sampleWindowMs),
             "normal", false},
            {QStringLiteral("ice"), QStringLiteral("ICE"),
             QString::fromStdString(transport.iceState), "good", false},
            {QStringLiteral("dtls"), QStringLiteral("DTLS"),
             QString::fromStdString(transport.dtlsState), "good", false},
            {QStringLiteral("role"), QStringLiteral("连接角色"),
             QString::fromStdString(transport.iceRole), "normal", false},
            {QStringLiteral("pair"), QStringLiteral("候选对"),
             QString::fromStdString(transport.candidatePairState),
             "good", false},
            {QStringLiteral("local"), QStringLiteral("本地候选"),
             CandidateDisplayText(transport.localCandidate), "normal",
             true},
            {QStringLiteral("remote"), QStringLiteral("远端候选"),
             CandidateDisplayText(transport.remoteCandidate), "normal",
             true},
            {QStringLiteral("availableUp"),
             QStringLiteral("估计可用上行"),
             transport.availableOutgoingBitrateBps == 0
                 ? QStringLiteral("未报告")
                 : FormatBitrate(
                       transport.availableOutgoingBitrateBps),
             "normal", false},
            {QStringLiteral("availableDown"),
             QStringLiteral("估计可用下行"),
             transport.availableIncomingBitrateBps == 0
                 ? QStringLiteral("未报告")
                 : FormatBitrate(
                       transport.availableIncomingBitrateBps),
             "normal", false},
            {QStringLiteral("security"), QStringLiteral("安全套件"),
             QStringLiteral("TLS %1 · DTLS %2 · SRTP %3")
                 .arg(QString::fromStdString(transport.tlsVersion),
                      QString::fromStdString(transport.dtlsCipher),
                      QString::fromStdString(transport.srtpCipher)),
             "normal", true},
            {QStringLiteral("pairChanges"),
             QStringLiteral("累计路径切换"),
             QStringLiteral("%1 次").arg(
                 transport.selectedCandidatePairChanges),
             transport.selectedCandidatePairChanges > 2
                 ? QByteArray("warning")
                 : QByteArray("normal"),
             false},
            {QStringLiteral("discarded"),
             QStringLiteral("累计发送丢弃"),
             QStringLiteral("%1 包 / %2")
                 .arg(transport.packetsDiscardedOnSend)
                 .arg(FormatByteCount(
                     transport.bytesDiscardedOnSend)),
             transport.packetsDiscardedOnSend > 0
                 ? QByteArray("warning")
                 : QByteArray("normal"),
             false}};
        const auto& recovery = transport.googCc;
        if (recovery.recoveryProbeHistoricalBudgetBps != 0 ||
            recovery.recoveryProbeEpisodes != 0) {
            connectionCard.chips << DiagnosticsChip{
                QStringLiteral("screenRecoveryProbe"),
                QStringLiteral("网络恢复探测"),
                QStringLiteral("%1\n本轮原生探测 %2 次 · 剩余 %3 次 · 累计触发 %4 次")
                    .arg(recovery.recoveryProbeActive
                        ? QStringLiteral("原生恢复探测窗口 · 提示预算上界 %1")
                            .arg(FormatBitrate(recovery.recoveryProbeHistoricalBudgetBps))
                        : QStringLiteral("待机 · 无活动提示"))
                    .arg(recovery.recoveryProbeAttempts)
                    .arg(recovery.recoveryProbeRemainingAttempts)
                    .arg(recovery.recoveryProbeEpisodes),
                recovery.recoveryProbeActive ? QByteArray("warning") : QByteArray("normal"),
                true};
        }
        connectionCards.push_back(std::move(connectionCard));

        bool hasOutbound = false;
        bool hasInbound = false;
        for (const auto& stream : stats.rtpStreams) {
            QStringList details;
            details
                << QStringLiteral("[%1] %2")
                       .arg(SlotDisplayName(stream.slot, stream.kind),
                            stream.codec.empty()
                                ? QStringLiteral("未知编码")
                                : QString::fromStdString(stream.codec));
            if (stream.frameWidth > 0 && stream.frameHeight > 0) {
                details
                    << QStringLiteral("%1×%2")
                           .arg(stream.frameWidth)
                           .arg(stream.frameHeight);
            }
            if (stream.framesPerSecond > 0.0) {
                details
                    << QStringLiteral("%1 fps")
                           .arg(stream.framesPerSecond, 0, 'f', 1);
            }
            if (stream.direction == RtpStreamDirection::kOutbound &&
                stream.sourceFramesPerSecond > 0.0) {
                details
                    << QStringLiteral("采集 %1×%2@%3")
                           .arg(stream.sourceWidth)
                           .arg(stream.sourceHeight)
                           .arg(stream.sourceFramesPerSecond, 0, 'f', 1);
            }
            details << QStringLiteral("码率 %1")
                           .arg(FormatBitrate(stream.bitrateBps));
            if (stream.direction == RtpStreamDirection::kOutbound &&
                stream.targetBitrateBps > 0) {
                details
                    << QStringLiteral("目标 %1")
                           .arg(FormatBitrate(
                               stream.targetBitrateBps));
            }
            const std::string& implementation =
                stream.direction == RtpStreamDirection::kOutbound
                    ? stream.encoderImplementation
                    : stream.decoderImplementation;
            if (!implementation.empty()) {
                details
                    << QStringLiteral("%1：%2")
                           .arg(stream.direction ==
                                        RtpStreamDirection::kOutbound
                                    ? QStringLiteral("编码器")
                                    : QStringLiteral("解码器"),
                                QString::fromStdString(implementation));
            }
            details
                << QStringLiteral("累计包 %1 · 累计丢包 %2（%3%）")
                       .arg(stream.packets)
                       .arg(stream.packetsLost)
                       .arg(stream.lossPercent, 0, 'f', 2);
            if (stream.direction == RtpStreamDirection::kOutbound &&
                stream.roundTripTimeMs > 0.0) {
                details
                    << QStringLiteral("RTT %1 ms")
                           .arg(stream.roundTripTimeMs, 0, 'f', 1);
            }
            if (stream.direction == RtpStreamDirection::kInbound) {
                details
                    << QStringLiteral("抖动 %1 ms")
                           .arg(stream.jitterMs, 0, 'f', 1);
                if (stream.kind == "video") {
                    details
                        << QStringLiteral(
                               "已解码 %1 · 关键帧 %2 · 丢帧 %3")
                               .arg(stream.framesDecoded)
                               .arg(stream.keyFrames)
                               .arg(stream.framesDropped);
                    details
                        << QStringLiteral(
                               "最近帧解码 %1 · 采样窗平均 %2 ms · 会话平均 %3 ms")
                               .arg(
                                   LatestFrameTimingText(
                                       stream,
                                       QStringLiteral("解码")))
                               .arg(
                                   stream.windowDecodeTimeMs,
                                   0, 'f', 2)
                               .arg(
                                   stream.averageDecodeTimeMs,
                                   0, 'f', 2);
                    if (stream.freezeCount > 0 ||
                        stream.pauseCount > 0) {
                        details
                            << QStringLiteral(
                                   "冻结 %1 次/%2 ms · 暂停 %3 次/%4 ms")
                                   .arg(stream.freezeCount)
                                   .arg(stream.totalFreezeDurationMs,
                                        0, 'f', 0)
                                   .arg(stream.pauseCount)
                                   .arg(stream.totalPauseDurationMs,
                                        0, 'f', 0);
                    }
                } else if (stream.totalAudioSamples > 0) {
                    details
                        << QStringLiteral(
                               "音频电平 %1 · 隐藏样本 %2/%3 · 隐藏事件 %4")
                               .arg(stream.audioLevel, 0, 'f', 3)
                               .arg(stream.concealedAudioSamples)
                               .arg(stream.totalAudioSamples)
                               .arg(stream.concealmentEvents);
                }
            } else if (stream.kind == "video") {
                details
                    << QStringLiteral(
                           "已编码 %1 · 关键帧 %2 · 最近帧编码 %3 · 采样窗平均 %4 ms · 会话平均 %5 ms")
                           .arg(stream.framesEncoded)
                           .arg(stream.keyFrames)
                           .arg(
                               LatestFrameTimingText(
                                   stream,
                                   QStringLiteral("编码")))
                           .arg(stream.windowEncodeTimeMs, 0, 'f', 2)
                           .arg(stream.averageEncodeTimeMs, 0, 'f', 2);
            }
            if (stream.averageQp > 0.0) {
                details
                    << QStringLiteral("会话平均 QP %1")
                           .arg(stream.averageQp, 0, 'f', 1);
            }
            if (stream.retransmittedPackets > 0 ||
                stream.retransmittedBytes > 0) {
                details
                    << QStringLiteral("累计重传 %1 包/%2")
                           .arg(stream.retransmittedPackets)
                           .arg(FormatByteCount(
                               stream.retransmittedBytes));
            }
            if (stream.nackCount > 0 || stream.pliCount > 0 ||
                stream.firCount > 0) {
                details
                    << QStringLiteral("NACK/PLI/FIR %1/%2/%3")
                           .arg(stream.nackCount)
                           .arg(stream.pliCount)
                           .arg(stream.firCount);
            }
            if (!stream.qualityLimitationReason.empty() &&
                stream.qualityLimitationReason != "none") {
                details
                    << QStringLiteral("质量受限：%1")
                           .arg(QString::fromStdString(
                               stream.qualityLimitationReason));
            }
            if (stream.powerEfficient) {
                details << QStringLiteral("节能硬件路径");
            }
            DiagnosticsCard streamCard;
            streamCard.key =
                peerKey + QLatin1Char('/') +
                QString::fromStdString(
                    stream.statsId.empty() ? stream.mid
                                           : stream.statsId);
            streamCard.title =
                QStringLiteral("%1%2")
                    .arg(SlotDisplayName(stream.slot, stream.kind),
                         stream.direction ==
                                  RtpStreamDirection::kOutbound
                              ? QStringLiteral("发送")
                              : QStringLiteral("接收"));
            streamCard.subtitle = peerName;
            streamCard.chips = {
                {QStringLiteral("codec"), QStringLiteral("编码格式"),
                 stream.codec.empty()
                     ? QStringLiteral("未知")
                     : QString::fromStdString(stream.codec),
                 "normal", false},
                {QStringLiteral("rate"), QStringLiteral("采样窗码率"),
                 FormatBitrate(stream.bitrateBps) +
                     SampleWindowSuffix(stream.sampleWindowMs),
                 stream.bitrateBps > 0 ? QByteArray("good")
                                       : QByteArray("normal"),
                 false},
                {QStringLiteral("implementation"),
                 stream.direction == RtpStreamDirection::kOutbound
                     ? QStringLiteral("编码器")
                     : QStringLiteral("解码器"),
                 implementation.empty()
                     ? QStringLiteral("未报告")
                     : QString::fromStdString(implementation),
                 stream.powerEfficient ? QByteArray("good")
                                       : QByteArray("normal"),
                 false},
                {QStringLiteral("packets"), QStringLiteral("累计数据包"),
                 QStringLiteral("%1").arg(stream.packets), "normal",
                 false},
                {QStringLiteral("loss"), QStringLiteral("累计丢包"),
                 QStringLiteral("%1 包 · %2%")
                     .arg(stream.packetsLost)
                     .arg(stream.lossPercent, 0, 'f', 2),
                 stream.lossPercent >= 3.0
                     ? QByteArray("warning")
                     : QByteArray("normal"),
                 false},
                {QStringLiteral("retransmit"), QStringLiteral("累计重传"),
                 QStringLiteral("%1 包 · %2")
                     .arg(stream.retransmittedPackets)
                     .arg(FormatByteCount(
                         stream.retransmittedBytes)),
                 stream.retransmittedPackets > 0
                     ? QByteArray("warning")
                     : QByteArray("normal"),
                 false}};

            if (stream.kind == "video") {
                streamCard.chips
                    << DiagnosticsChip{
                           QStringLiteral("resolution"),
                           QStringLiteral("分辨率"),
                           stream.frameWidth > 0 &&
                                   stream.frameHeight > 0
                               ? QStringLiteral("%1 × %2")
                                     .arg(stream.frameWidth)
                                     .arg(stream.frameHeight)
                               : QStringLiteral("未报告"),
                           "normal", false}
                    << DiagnosticsChip{
                           QStringLiteral("fps"),
                           QStringLiteral("WebRTC当前帧率"),
                           QStringLiteral("%1 FPS").arg(
                               stream.framesPerSecond, 0, 'f', 1),
                           stream.framesPerSecond > 0.0
                               ? QByteArray("good")
                               : QByteArray("normal"),
                           false}
                    << DiagnosticsChip{
                           QStringLiteral("frames"),
                           stream.direction ==
                                   RtpStreamDirection::kOutbound
                               ? QStringLiteral("累计编码帧")
                               : QStringLiteral("累计解码帧"),
                           stream.direction ==
                                   RtpStreamDirection::kOutbound
                               ? QStringLiteral(
                                     "编码 %1 · 发送 %2 · 关键帧 %3")
                                     .arg(stream.framesEncoded)
                                     .arg(stream.framesSent)
                                     .arg(stream.keyFrames)
                               : QStringLiteral(
                                     "%1 · 关键帧 %2 · 丢帧 %3")
                                     .arg(stream.framesDecoded)
                                     .arg(stream.keyFrames)
                                     .arg(stream.framesDropped),
                           "normal", false}
                    << DiagnosticsChip{
                           QStringLiteral("qp"),
                           QStringLiteral("最近一帧 QP"),
                           stream.latestFrameQpAvailable
                               ? QStringLiteral("%1").arg(
                                     stream.latestFrameQp)
                               : QStringLiteral("未报告"),
                           "normal", false}
                    << DiagnosticsChip{
                           QStringLiteral("feedback"),
                           QStringLiteral("累计反馈"),
                           QStringLiteral("NACK %1 · PLI %2 · FIR %3")
                               .arg(stream.nackCount)
                               .arg(stream.pliCount)
                               .arg(stream.firCount),
                           stream.pliCount > 10
                               ? QByteArray("warning")
                               : QByteArray("normal"),
                           true};
            }
            if (stream.direction == RtpStreamDirection::kOutbound) {
                streamCard.chips
                    << DiagnosticsChip{
                           QStringLiteral("target"),
                           QStringLiteral("WebRTC目标码率"),
                           stream.targetBitrateBps > 0
                               ? FormatBitrate(
                                     stream.targetBitrateBps)
                               : QStringLiteral("未报告"),
                           "normal", false}
                    << DiagnosticsChip{
                           QStringLiteral("senderPolicy"),
                           QStringLiteral("发送策略上限"),
                           stream.configuredOutputWidth > 0 &&
                                   stream.configuredOutputHeight > 0
                               ? QStringLiteral("%1 × %2 · %3 FPS")
                                     .arg(stream.configuredOutputWidth)
                                     .arg(stream.configuredOutputHeight)
                                     .arg(stream.configuredMaxFrameRate)
                               : QStringLiteral("未配置"),
                           "normal", false}
                    << DiagnosticsChip{
                           QStringLiteral("senderStartBitrate"),
                           QStringLiteral("WebRTC启动码率"),
                           stream.configuredStartBitrateBps > 0
                               ? FormatBitrate(
                                     stream.configuredStartBitrateBps)
                               : QStringLiteral("未配置"),
                           "normal", false}
                    << DiagnosticsChip{
                           QStringLiteral("senderBitrateCeiling"),
                           QStringLiteral("视频编码码率上限"),
                           stream.configuredMaxBitrateBps > 0
                               ? FormatBitrate(
                                     stream.configuredMaxBitrateBps)
                               : QStringLiteral("未配置"),
                           "normal", false}
                    << DiagnosticsChip{
                           QStringLiteral("senderBitrateBootstrap"),
                           QStringLiteral("启动码率引导"),
                           QStringLiteral(
                               "重启 %1 · SetBitrate %2/%3 · 分配探测 %4 · "
                               "2 Mbps探测底座 %5%6")
                               .arg(stream.mediaReadyBitrateRestarts)
                               .arg(stream.bitrateBootstrapSuccesses)
                               .arg(stream.bitrateBootstrapAttempts)
                               .arg(stream.allocationProbePulses)
                               .arg(stream.bitrateProbeFloorActive
                                        ? QStringLiteral("活动")
                                        : QStringLiteral("已释放%1次").arg(
                                              stream.bitrateProbeFloorReleases))
                               .arg(stream.bitrateBootstrapError.empty()
                                        ? QString()
                                        : QStringLiteral(" · %1").arg(
                                              QString::fromStdString(
                                                  stream.bitrateBootstrapError))),
                           stream.bitrateBootstrapError.empty()
                               ? QByteArray("good")
                               : QByteArray("warning"),
                           true}
                    << DiagnosticsChip{
                           QStringLiteral("rtt"),
                           QStringLiteral("当前媒体 RTT"),
                           QStringLiteral("%1 ms").arg(
                               stream.roundTripTimeMs, 0, 'f', 1),
                           stream.roundTripTimeMs >= 250.0
                                ? QByteArray("warning")
                                : QByteArray("normal"),
                           false};
                if (stream.progressiveBitrateCeilingEnabled) {
                    QString ceilingState = QStringLiteral("稳定中");
                    if (stream.bitrateCeilingStatus == "waiting_for_stats") {
                        ceilingState = QStringLiteral("等待带宽统计");
                    } else if (stream.bitrateCeilingStatus == "stable") {
                        ceilingState = QStringLiteral("已稳定");
                    } else if (stream.bitrateCeilingStatus ==
                               "probe_pending") {
                        ceilingState = QStringLiteral("等待探测结果");
                    } else if (stream.bitrateCeilingStatus == "cooldown") {
                        ceilingState = QStringLiteral("失败冷却");
                    }
                    QString ceilingDetail = QStringLiteral(
                        "当前 %1 · 目标 %2 · 平滑估计 %3 · 稳定样本 %4/3 · %5")
                        .arg(FormatBitrate(
                            stream.appliedPeerConnectionMaxBitrateBps))
                        .arg(FormatBitrate(
                            stream.desiredPeerConnectionMaxBitrateBps))
                        .arg(stream.smoothedOutgoingCapacityBps > 0
                                 ? FormatBitrate(
                                       stream.smoothedOutgoingCapacityBps)
                                 : QStringLiteral("未报告"))
                        .arg(stream.bitrateCeilingStableSamples)
                        .arg(ceilingState);
                    if (stream.bitrateCeilingCooldownRemainingMs > 0) {
                        ceilingDetail += QStringLiteral(" · 剩余 %1 秒")
                            .arg(
                                stream.bitrateCeilingCooldownRemainingMs /
                                1000.0,
                                0,
                                'f',
                                1);
                    }
                    if (!stream.bitrateCeilingError.empty()) {
                        ceilingDetail += QStringLiteral("\n错误：%1").arg(
                            QString::fromStdString(
                                stream.bitrateCeilingError));
                    }
                    streamCard.chips << DiagnosticsChip{
                        QStringLiteral("progressiveBitrateCeiling"),
                        QStringLiteral("渐进码率上限"),
                        ceilingDetail,
                        stream.bitrateCeilingLastProbeFailed ||
                                !stream.bitrateCeilingError.empty()
                            ? QByteArray("warning")
                            : QByteArray("good"),
                        true};
                }
                if (stream.screenQualityProtectionAvailable) {
                    streamCard.chips << DiagnosticsChip{
                        QStringLiteral("nativeScreenQualityProtection"),
                        QStringLiteral("网络波动时的画质保护"),
                        QStringLiteral("%1 · 用户上限 %2 FPS\n视频目标预算 B：%3 · 单帧画质参考 A：%4\n编码器名义速率 C：%5（不是实际发送码率）\n当前取舍系数 %6\nWebRTC 编码修正值：%7 · 视频带宽分配：%8")
                            .arg(stream.screenQualityProtectionActive
                                ? QStringLiteral("保护画质，由 WebRTC 临时丢帧")
                                : QStringLiteral("按当前网络预算编码"))
                            .arg(stream.configuredMaxFrameRate)
                            .arg(FormatBitrate(stream.screenQualityNetworkBudgetBps))
                            .arg(FormatBitrate(stream.screenQualityReferenceBps))
                            .arg(FormatBitrate(stream.screenQualityEncoderReferenceBps))
                            .arg(stream.screenQualityDeficitShareHundredths / 100.0, 0, 'f', 2)
                            .arg(FormatBitrate(stream.screenQualityEncoderAdjustedBudgetBps))
                            .arg(FormatBitrate(stream.screenQualityBandwidthAllocationBps)),
                        stream.screenQualityProtectionActive ? QByteArray("warning") : QByteArray("good"),
                        true};
                }
                if (stream.adaptiveNetworkFrameRateEnabled) {
                    QString frameRateState = QStringLiteral("稳定");
                    if (stream.adaptiveNetworkFrameRateStatus ==
                        "waiting_for_activity") {
                        frameRateState = QStringLiteral("等待桌面活动");
                    } else if (stream.adaptiveNetworkFrameRateStatus ==
                               "idle_suspended") {
                        frameRateState = QStringLiteral("静止暂停判断");
                    } else if (stream.adaptiveNetworkFrameRateStatus ==
                               "startup_grace") {
                        frameRateState = QStringLiteral("启动保护期");
                    } else if (stream.adaptiveNetworkFrameRateStatus ==
                               "waiting_for_capacity") {
                        frameRateState = QStringLiteral("等待带宽估计");
                    } else if (stream.adaptiveNetworkFrameRateStatus ==
                               "reducing") {
                        frameRateState = QStringLiteral("弱网降帧判断");
                    } else if (stream.adaptiveNetworkFrameRateStatus ==
                               "recovering") {
                        frameRateState = QStringLiteral("带宽恢复判断");
                    }
                    QString frameRateDetail = QStringLiteral(
                        "用户请求 %1 FPS · 当前连接有效 %2 FPS · %3 · "
                        "平滑容量 %4 · 降帧样本 %5/2 · 恢复跟随当前网络预算")
                        .arg(stream.configuredMaxFrameRate)
                        .arg(stream.effectiveNetworkFrameRate)
                        .arg(frameRateState)
                        .arg(stream.adaptiveNetworkFrameRateCapacityBps > 0
                                 ? FormatBitrate(
                                       stream.adaptiveNetworkFrameRateCapacityBps)
                                 : QStringLiteral("未报告"))
                        .arg(stream.adaptiveNetworkFrameRateReductionSamples);
                    if (!stream.adaptiveNetworkFrameRateError.empty()) {
                        frameRateDetail += QStringLiteral("\n错误：%1").arg(
                            QString::fromStdString(
                                stream.adaptiveNetworkFrameRateError));
                    }
                    streamCard.chips << DiagnosticsChip{
                        QStringLiteral("adaptiveNetworkFrameRate"),
                        QStringLiteral("网络有效帧率"),
                        frameRateDetail,
                        stream.adaptiveNetworkFrameRateError.empty()
                            ? QByteArray("good")
                            : QByteArray("warning"),
                        true};
                }
                if (stream.kind == "video") {
                    if (!diagnostics.videoEncoderLastFallbackReason.empty()) {
                        const bool hardwareRecovered =
                            implementation.find("FFmpeg/QSV") !=
                                std::string::npos ||
                            implementation.find("FFmpeg/NVENC") !=
                                std::string::npos ||
                            implementation.find("FFmpeg/AMF") !=
                                std::string::npos ||
                            implementation.find("MediaFoundation") !=
                                std::string::npos;
                        streamCard.chips << DiagnosticsChip{
                            QStringLiteral("encoderFallbackReason"),
                            hardwareRecovered
                                ? QStringLiteral(
                                      "最近硬件编码异常（已自动恢复）")
                                : QStringLiteral("最近硬件编码异常"),
                            QString::fromStdString(
                                diagnostics.videoEncoderLastFallbackReason),
                            hardwareRecovered ? QByteArray("good")
                                              : QByteArray("warning"),
                            true};
                    }
                    const QString configuredCaptureBackend =
                        stream.captureConfiguredBackend == "libwebrtc"
                            ? QStringLiteral("libwebrtc")
                            : stream.captureConfiguredBackend == "native_dxgi"
                                  ? QStringLiteral("自研 DXGI")
                                  : QStringLiteral("未报告");
                    QString activeCaptureBackend;
                    if (stream.captureActiveBackend ==
                        "native_dxgi_texture") {
                        activeCaptureBackend =
                            QStringLiteral("自研 DXGI 纹理");
                    } else if (stream.captureActiveBackend ==
                               "libwebrtc_dxgi_with_gdi_fallback") {
                        activeCaptureBackend =
                            QStringLiteral("libwebrtc DXGI/GDI");
                    } else if (stream.captureActiveBackend ==
                               "libwebrtc_gdi") {
                        activeCaptureBackend =
                            QStringLiteral("libwebrtc GDI");
                    } else {
                        activeCaptureBackend = QStringLiteral("未报告");
                    }
                    QString captureBackendText =
                        QStringLiteral("配置 %1 · 实际 %2")
                            .arg(configuredCaptureBackend,
                                 activeCaptureBackend);
                    if (!stream.captureFallbackReason.empty()) {
                        captureBackendText += QStringLiteral("\n回退：%1")
                            .arg(QString::fromStdString(
                                stream.captureFallbackReason));
                    }
                    streamCard.chips
                        << DiagnosticsChip{
                               QStringLiteral("captureBackend"),
                               QStringLiteral("屏幕采集器"),
                               captureBackendText,
                               stream.captureFallbackReason.empty()
                                   ? QByteArray("good")
                                   : QByteArray("warning"),
                               true}
                        << DiagnosticsChip{
                               QStringLiteral("capture"),
                               QStringLiteral("桌面采集调度"),
                               stream.captureTargetFrameRate > 0
                                    ? QStringLiteral(
                                          "目标 %1 · 调用 %2 · 交付 %3 FPS")
                                         .arg(
                                             stream.captureTargetFrameRate)
                                         .arg(
                                             stream.captureAttemptsPerSecond,
                                             0, 'f', 1)
                                         .arg(
                                             stream.captureDeliveredFramesPerSecond,
                                             0, 'f', 1)
                                    : QStringLiteral("未报告"),
                               !stream.captureAdaptiveFrameDeliveryEnabled &&
                                       stream.captureDeliveredFramesPerSecond +
                                               1.0 <
                                           static_cast<double>(
                                               stream.captureTargetFrameRate)
                                    ? QByteArray("warning")
                                    : QByteArray("good"),
                               true}
                        << DiagnosticsChip{
                               QStringLiteral("captureActivity"),
                               QStringLiteral("静态桌面降帧"),
                               stream.captureAdaptiveFrameDeliveryEnabled
                                   ? QStringLiteral(
                                         "%1 · 变化 %2 · 心跳 %3 FPS")
                                         .arg(
                                             stream.captureActivityState ==
                                                     "idle"
                                                 ? QStringLiteral("静止")
                                                 : (stream.captureActivityState ==
                                                            "active"
                                                        ? QStringLiteral("活动")
                                                        : QStringLiteral("启动")))
                                         .arg(
                                             stream.captureChangedFramesPerSecond,
                                             0, 'f', 1)
                                         .arg(
                                             stream.captureIdleHeartbeatFramesPerSecond,
                                             0, 'f', 1)
                                   : QStringLiteral("回退路径未启用"),
                               stream.captureAdaptiveFrameDeliveryEnabled
                                   ? QByteArray("good")
                                   : QByteArray("normal"),
                               true}
                        << DiagnosticsChip{
                               QStringLiteral("captureChangedArea"),
                               QStringLiteral("画面变化面积"),
                               stream.contentAnalyzerEnabled
                                   ? QStringLiteral("%1% · 变化频率 %2 FPS")
                                         .arg(
                                             stream.captureChangedAreaRatio *
                                                 100.0,
                                             0, 'f', 2)
                                         .arg(
                                             stream.captureChangedFramesPerSecond,
                                             0, 'f', 1)
                                   : QStringLiteral("画面分析已关闭"),
                               "normal", true}
                        << DiagnosticsChip{
                               QStringLiteral("contentMotion"),
                               QStringLiteral("画面运动情况"),
                               stream.contentAnalyzerEnabled &&
                                       stream.contentSourceFrameId > 0
                                   ? QStringLiteral(
                                         "%1 · 分数 %2 · 帧 #%3 · %4 ms")
                                         .arg(
                                             stream.contentMotionLevel == "high"
                                                 ? QStringLiteral("高")
                                                 : (stream.contentMotionLevel ==
                                                            "medium"
                                                        ? QStringLiteral("中")
                                                        : (stream.contentMotionLevel ==
                                                                   "low"
                                                               ? QStringLiteral("低")
                                                               : (stream.contentMotionLevel ==
                                                                          "idle"
                                                                      ? QStringLiteral("静止")
                                                                      : QStringLiteral("未知")))))
                                         .arg(
                                             stream.contentMotionScore,
                                             0, 'f', 3)
                                         .arg(stream.contentSourceFrameId)
                                         .arg(stream.contentStateAgeMs)
                                   : (stream.contentAnalyzerEnabled
                                          ? QStringLiteral("等待首次分析")
                                          : QStringLiteral("画面运动分析已关闭")),
                               stream.contentAnalyzerEnabled
                                   ? QByteArray("good")
                                   : QByteArray("normal"),
                               true}
                        << DiagnosticsChip{
                               QStringLiteral("contentSemantic"),
                               QStringLiteral("场景识别结果"),
                               stream.contentAnalyzerEnabled &&
                                       stream.contentScene != "unknown"
                                   ? QStringLiteral("%1 · 置信度 %2 · %3")
                                         .arg(
                                             ContentSceneDisplayText(
                                                 stream.contentScene))
                                         .arg(
                                             stream.contentSemanticConfidence,
                                             0, 'f', 3)
                                         .arg(
                                             stream.contentAnalyzerBackend ==
                                                     "rules+vision_api"
                                                 ? QStringLiteral("AI 模型 API")
                                                 : QStringLiteral("本地分析"))
                                   : (stream.contentAnalyzerBackend ==
                                              "rules+vision_api"
                                          ? QStringLiteral(
                                                "等待确认场景；识别失败时仍保留画面运动分析")
                                          : QStringLiteral(
                                                "本地模型开发中，暂不支持场景识别")),
                               stream.contentScene != "unknown"
                                   ? QByteArray("good")
                                   : QByteArray("normal"),
                               true}
                        << DiagnosticsChip{
                               QStringLiteral("contentAnalysisQueue"),
                               QStringLiteral("画面分析任务"),
                               stream.contentAnalyzerEnabled
                                   ? QStringLiteral(
                                         "提交 %1 · 完成 %2 · 替换 %3 · %4 us")
                                         .arg(stream.contentSubmittedSamples)
                                         .arg(stream.contentProcessedSamples)
                                         .arg(stream.contentReplacedSamples)
                                         .arg(
                                             stream.contentLatestAnalysisTimeUs)
                                   : QStringLiteral("分析任务未启动"),
                               stream.contentRejectedSamples > 0 ||
                                       stream.contentDiscardedResults > 0
                                   ? QByteArray("warning")
                                   : QByteArray("normal"),
                               false}
                        << DiagnosticsChip{
                               QStringLiteral("captureSuppression"),
                               QStringLiteral("累计抑制重复帧"),
                               stream.captureAdaptiveFrameDeliveryEnabled
                                   ? QStringLiteral("%1 · 状态切换 %2")
                                         .arg(
                                             stream.captureSuppressedUnchangedFrames)
                                         .arg(
                                             stream.captureActivityTransitions)
                                   : QStringLiteral("不适用"),
                               "normal", false}
                        << DiagnosticsChip{
                               QStringLiteral("captureInteraction"),
                               QStringLiteral("交互加速 / 强制刷新"),
                               stream.captureConfiguredBackend == "libwebrtc"
                                   ? QStringLiteral(
                                         "%1 · 输入加速 %2 次 · 全帧 %3 次")
                                         .arg(
                                             stream.captureInputBoostActive
                                                 ? QStringLiteral("加速中")
                                                 : QStringLiteral("常规"))
                                         .arg(stream.captureInputBoosts)
                                         .arg(
                                             stream.captureForcedRefreshFrames)
                                   : QStringLiteral("仅 libwebrtc 路径启用"),
                               stream.captureInputBoostActive
                                   ? QByteArray("good")
                                   : QByteArray("normal"),
                               false}
                        << DiagnosticsChip{
                               QStringLiteral("captureCall"),
                               QStringLiteral("采集调用耗时"),
                               stream.captureTargetFrameRate > 0
                                   ? QStringLiteral(
                                         "%1 ms · 失败 %2/%3")
                                         .arg(
                                             stream.latestCaptureCallMs,
                                             0, 'f', 3)
                                         .arg(stream.captureFailures)
                                         .arg(stream.captureAttempts)
                                   : QStringLiteral("未报告"),
                               stream.latestCaptureCallMs >= 15.0
                                   ? QByteArray("warning")
                                   : QByteArray("normal"),
                               false}
                        << DiagnosticsChip{
                               QStringLiteral("source"),
                               QStringLiteral("WebRTC源输出"),
                               stream.sourceWidth > 0 &&
                                       stream.sourceHeight > 0
                                   ? QStringLiteral("%1 × %2 · %3 FPS")
                                         .arg(stream.sourceWidth)
                                         .arg(stream.sourceHeight)
                                         .arg(
                                             stream.sourceFramesPerSecond,
                                             0, 'f', 1)
                                   : QStringLiteral("未报告"),
                               "normal", true}
                        << DiagnosticsChip{
                               QStringLiteral("encodedFps"),
                               QStringLiteral("编码完成帧率"),
                               QStringLiteral("%1 FPS")
                                   .arg(
                                       stream.encodedFramesPerSecond,
                                       0, 'f', 1),
                               stream.encodedFramesPerSecond > 0.0
                                   ? QByteArray("good")
                                   : QByteArray("normal"),
                               false}
                        << DiagnosticsChip{
                               QStringLiteral("sentFps"),
                               QStringLiteral("RTP发送帧率"),
                               QStringLiteral("%1 FPS")
                                   .arg(
                                       stream.sentFramesPerSecond,
                                       0, 'f', 1),
                               stream.sentFramesPerSecond > 0.0
                                   ? QByteArray("good")
                                   : QByteArray("normal"),
                               false}
                        << DiagnosticsChip{
                               QStringLiteral("encodeTime"),
                               QStringLiteral("最近一帧编码"),
                               LatestFrameTimingText(
                                   stream,
                                   QStringLiteral("编码")),
                               stream.latestFrameTimeMs >= 30.0
                                   ? QByteArray("warning")
                                   : QByteArray("normal"),
                               true}
                        << DiagnosticsChip{
                               QStringLiteral("windowEncodeTime"),
                               QStringLiteral("采样窗平均编码"),
                               stream.windowEncodeTimeAvailable
                                   ? QStringLiteral("%1 ms%2")
                                         .arg(
                                             stream.windowEncodeTimeMs,
                                             0, 'f', 3)
                                         .arg(SampleWindowSuffix(
                                             stream.sampleWindowMs))
                                   : QStringLiteral("等待第二次采样"),
                               stream.windowEncodeTimeMs >= 30.0
                                   ? QByteArray("warning")
                                   : QByteArray("normal"),
                               false}
                        << DiagnosticsChip{
                               QStringLiteral("sessionEncodeTime"),
                               QStringLiteral("会话平均编码"),
                               QStringLiteral("%1 ms").arg(
                                   stream.averageEncodeTimeMs,
                                   0, 'f', 3),
                               "normal", false}
                        << DiagnosticsChip{
                               QStringLiteral("windowQp"),
                               QStringLiteral("采样窗平均 QP"),
                               stream.windowQpAvailable
                                   ? QStringLiteral("%1").arg(
                                         stream.windowQp, 0, 'f', 2)
                                   : QStringLiteral("未报告"),
                               "normal", false}
                        << DiagnosticsChip{
                               QStringLiteral("limitation"),
                               QStringLiteral("当前质量限制"),
                               stream.qualityLimitationReason.empty() ||
                                       stream.qualityLimitationReason ==
                                           "none"
                                   ? QStringLiteral("无")
                                   : QString::fromStdString(
                                         stream
                                             .qualityLimitationReason),
                               stream.qualityLimitationReason.empty() ||
                                       stream.qualityLimitationReason ==
                                           "none"
                                   ? QByteArray("good")
                                   : QByteArray("warning"),
                               false};
                }
                outboundCards.push_back(std::move(streamCard));
            } else {
                streamCard.chips
                    << DiagnosticsChip{
                           QStringLiteral("jitter"),
                           QStringLiteral("网络抖动"),
                           QStringLiteral("%1 ms").arg(
                               stream.jitterMs, 0, 'f', 1),
                           stream.jitterMs >= 30.0
                               ? QByteArray("warning")
                               : QByteArray("normal"),
                           false}
                    << DiagnosticsChip{
                           QStringLiteral("buffer"),
                           QStringLiteral("采样窗抖动缓冲"),
                           stream.windowJitterBufferDelayAvailable
                               ? QStringLiteral("%1 ms").arg(
                                     stream
                                         .windowJitterBufferDelayMs,
                                     0, 'f', 3)
                               : QStringLiteral("等待第二次采样"),
                           "normal", false};
                if (stream.kind == "video") {
                    streamCard.chips
                        << DiagnosticsChip{
                               QStringLiteral("decodeTime"),
                               QStringLiteral("最近一帧解码"),
                               LatestFrameTimingText(
                                   stream,
                                   QStringLiteral("解码")),
                               stream.latestFrameTimeMs >= 30.0
                                   ? QByteArray("warning")
                                   : QByteArray("normal"),
                               true}
                        << DiagnosticsChip{
                               QStringLiteral("windowDecodeTime"),
                               QStringLiteral("采样窗平均解码"),
                               stream.windowDecodeTimeAvailable
                                   ? QStringLiteral("%1 ms").arg(
                                         stream.windowDecodeTimeMs,
                                         0, 'f', 3)
                                   : QStringLiteral("等待第二次采样"),
                               stream.windowDecodeTimeMs >= 30.0
                                   ? QByteArray("warning")
                                   : QByteArray("normal"),
                               false}
                        << DiagnosticsChip{
                               QStringLiteral("decoderTimingWindow"),
                               QStringLiteral("逐帧解码耗时窗口"),
                               QStringLiteral(
                                   "平均 %1 · P95 %2 · 最大 %3 ms")
                                   .arg(stream.averageFrameTimingMs,
                                       0, 'f', 3)
                                   .arg(stream.p95FrameTimingMs,
                                       0, 'f', 3)
                                   .arg(stream.maximumFrameTimingMs,
                                       0, 'f', 3),
                               stream.p95FrameTimingMs >= 30.0
                                   ? QByteArray("warning")
                                   : QByteArray("normal"),
                               true}
                        << DiagnosticsChip{
                               QStringLiteral("sessionDecodeTime"),
                               QStringLiteral("会话平均解码"),
                               QStringLiteral("%1 ms").arg(
                                   stream.averageDecodeTimeMs,
                                   0, 'f', 3),
                               "normal", false}
                        << DiagnosticsChip{
                               QStringLiteral("processing"),
                               QStringLiteral("采样窗处理延迟"),
                               stream.windowProcessingDelayAvailable
                                   ? QStringLiteral("%1 ms").arg(
                                         stream
                                             .windowProcessingDelayMs,
                                         0, 'f', 3)
                                   : QStringLiteral("等待第二次采样"),
                               "normal", false}
                        << DiagnosticsChip{
                               QStringLiteral("sessionProcessing"),
                               QStringLiteral("会话平均处理延迟"),
                               QStringLiteral("%1 ms").arg(
                                   stream.averageProcessingDelayMs,
                                   0, 'f', 3),
                               "normal", false}
                        << DiagnosticsChip{
                               QStringLiteral("sessionBuffer"),
                               QStringLiteral("会话平均抖动缓冲"),
                               QStringLiteral("%1 ms").arg(
                                   stream
                                       .averageJitterBufferDelayMs,
                                   0, 'f', 3),
                               "normal", false}
                        << DiagnosticsChip{
                               QStringLiteral("windowQp"),
                               QStringLiteral("采样窗平均 QP"),
                               stream.windowQpAvailable
                                   ? QStringLiteral("%1").arg(
                                         stream.windowQp, 0, 'f', 2)
                                   : QStringLiteral("未报告"),
                               "normal", false}
                        << DiagnosticsChip{
                               QStringLiteral("freeze"),
                               QStringLiteral("累计冻结/暂停"),
                               QStringLiteral(
                                   "%1 次/%2 ms · %3 次/%4 ms")
                                   .arg(stream.freezeCount)
                                   .arg(
                                       stream.totalFreezeDurationMs,
                                       0, 'f', 0)
                                   .arg(stream.pauseCount)
                                   .arg(
                                       stream.totalPauseDurationMs,
                                       0, 'f', 0),
                               stream.freezeCount > 0
                                   ? QByteArray("warning")
                                   : QByteArray("normal"),
                               true};
                    if (stream.presentationTimingAvailable) {
                        const double supersededPercent =
                            stream.presentationArrivedFrames > 0
                                ? static_cast<double>(
                                      stream.presentationSupersededFrames) *
                                      100.0 /
                                      static_cast<double>(
                                          stream.presentationArrivedFrames)
                                : 0.0;
                        const bool nativePresentation =
                            stream.presentationPath == "D3D11" ||
                            stream.presentationPath == "CPU/D3D11" ||
                            stream.presentationPath == "CPU NV12/D3D11" ||
                            stream.presentationPath == "CPU I420/D3D11";
                        const bool i420ShaderPresentation =
                            stream.presentationPath == "CPU I420/D3D11";
                        streamCard.chips
                            << DiagnosticsChip{
                                   QStringLiteral("presentationPath"),
                                   QStringLiteral("本机显示路径"),
                                   QStringLiteral("%1 · %2 Hz")
                                       .arg(QString::fromStdString(
                                           stream.presentationPath))
                                       .arg(stream.localRefreshRateHz,
                                           0, 'f', 1),
                                   nativePresentation
                                       ? QByteArray("good")
                                       : QByteArray("normal"),
                                   false}
                            << DiagnosticsChip{
                                   QStringLiteral("presentationArrival"),
                                   QStringLiteral("应用帧到达"),
                                   QStringLiteral("%1 FPS · 累计 %2")
                                       .arg(
                                           stream
                                               .presentationArrivalFramesPerSecond,
                                           0, 'f', 1)
                                       .arg(
                                           stream.presentationArrivedFrames),
                                   stream.presentationArrivalFramesPerSecond >
                                           0.0
                                       ? QByteArray("good")
                                       : QByteArray("normal"),
                                   false}
                            << DiagnosticsChip{
                                   QStringLiteral("presentationFps"),
                                   QStringLiteral("显示提交帧率"),
                                   QStringLiteral("%1 FPS · 累计 %2")
                                       .arg(stream.presentedFramesPerSecond,
                                           0, 'f', 1)
                                       .arg(stream.presentedFrames),
                                   stream.presentedFramesPerSecond > 0.0
                                       ? QByteArray("good")
                                       : QByteArray("normal"),
                                   false}
                            << DiagnosticsChip{
                                   QStringLiteral("presentationSuperseded"),
                                   QStringLiteral("应用覆盖丢帧"),
                                   QStringLiteral("%1 帧 · %2%")
                                       .arg(
                                           stream.presentationSupersededFrames)
                                       .arg(supersededPercent, 0, 'f', 2),
                                   supersededPercent >= 3.0
                                       ? QByteArray("warning")
                                       : QByteArray("normal"),
                                   false}
                            << DiagnosticsChip{
                                   QStringLiteral("presentationInterval"),
                                   QStringLiteral("呈现帧间隔"),
                                   QStringLiteral(
                                       "平均 %1 · P95 %2 · 最大 %3 ms")
                                       .arg(
                                           stream.averagePresentedIntervalMs,
                                           0, 'f', 3)
                                       .arg(stream.p95PresentedIntervalMs,
                                           0, 'f', 3)
                                       .arg(stream.maximumPresentedIntervalMs,
                                           0, 'f', 3),
                                   stream.p95PresentedIntervalMs >= 30.0
                                       ? QByteArray("warning")
                                       : QByteArray("normal"),
                                   true}
                            << DiagnosticsChip{
                                   QStringLiteral("receiverPipeline"),
                                   QStringLiteral("末包到应用画面"),
                                   QStringLiteral(
                                       "最近 %1 · 平均 %2 · P95 %3 · 最大 %4 ms")
                                       .arg(stream.latestReceiverPipelineMs,
                                           0, 'f', 3)
                                       .arg(stream.averageReceiverPipelineMs,
                                           0, 'f', 3)
                                       .arg(stream.p95ReceiverPipelineMs,
                                           0, 'f', 3)
                                       .arg(stream.maximumReceiverPipelineMs,
                                           0, 'f', 3),
                                   stream.p95ReceiverPipelineMs >= 150.0
                                       ? QByteArray("warning")
                                       : QByteArray("normal"),
                                   true};
                        if (stream.presentationConvertedFrames > 0) {
                            streamCard.chips
                                << DiagnosticsChip{
                                       QStringLiteral(
                                           "presentationConversion"),
                                       i420ShaderPresentation
                                           ? QStringLiteral("I420三平面上传")
                                           : QStringLiteral("CPU帧格式整理"),
                                       QStringLiteral(
                                           "最近 %1 · 平均 %2 ms · %3 帧")
                                           .arg(
                                               stream
                                                   .latestPresentationConversionMs,
                                               0, 'f', 3)
                                           .arg(
                                               stream
                                                   .averagePresentationConversionMs,
                                               0, 'f', 3)
                                           .arg(
                                               stream
                                                   .presentationConvertedFrames),
                                       stream
                                                   .averagePresentationConversionMs >=
                                               8.0
                                           ? QByteArray("warning")
                                           : QByteArray("normal"),
                                       true};
                        }
                        streamCard.chips
                            << DiagnosticsChip{
                                   QStringLiteral("presentationRenderSubmit"),
                                   i420ShaderPresentation
                                       ? QStringLiteral("Pixel Shader提交")
                                       : (nativePresentation
                                       ? QStringLiteral(
                                             "VideoProcessor提交")
                                       : QStringLiteral("Qt绘制提交")),
                                   QStringLiteral("最近 %1 · 平均 %2 ms")
                                       .arg(
                                           stream
                                               .latestPresentationRenderSubmitMs,
                                           0, 'f', 3)
                                       .arg(
                                           stream
                                               .averagePresentationRenderSubmitMs,
                                           0, 'f', 3),
                                   stream
                                               .averagePresentationRenderSubmitMs >=
                                           8.0
                                       ? QByteArray("warning")
                                       : QByteArray("normal"),
                                   false};
                        if (nativePresentation) {
                            streamCard.chips
                                << DiagnosticsChip{
                                       QStringLiteral("presentationCall"),
                                       QStringLiteral("Present调用"),
                                       QStringLiteral(
                                           "最近 %1 · 平均 %2 ms")
                                           .arg(
                                               stream.latestPresentCallMs,
                                               0, 'f', 3)
                                           .arg(
                                               stream.averagePresentCallMs,
                                               0, 'f', 3),
                                       stream.averagePresentCallMs >= 8.0
                                           ? QByteArray("warning")
                                           : QByteArray("normal"),
                                       false};
                        }
                        streamCard.chips
                            << DiagnosticsChip{
                                   QStringLiteral("presentationFailures"),
                                   QStringLiteral("显示失败"),
                                   QStringLiteral("%1 次").arg(
                                       stream.presentationFailures),
                                   stream.presentationFailures > 0
                                       ? QByteArray("warning")
                                       : QByteArray("normal"),
                                   false};
                    }
                    if (stream.decodePipelineTimingAvailable) {
                        streamCard.chips
                            << DiagnosticsChip{
                                   QStringLiteral("decodePreparation"),
                                   QStringLiteral("输入准备耗时"),
                                   QStringLiteral("%1 ms").arg(
                                       stream.decodeInputPreparationMs,
                                       0, 'f', 3),
                                   "normal", false}
                            << DiagnosticsChip{
                                   QStringLiteral("decodeQueue"),
                                   QStringLiteral("输入排队耗时"),
                                   QStringLiteral("%1 ms").arg(
                                       stream.decodeInputQueueWaitMs,
                                       0, 'f', 3),
                                   stream.decodeInputQueueWaitMs >= 20.0
                                       ? QByteArray("warning")
                                       : QByteArray("normal"),
                                   false}
                            << DiagnosticsChip{
                                   QStringLiteral("decodeTransform"),
                                   QStringLiteral("MFT输出等待"),
                                   QStringLiteral(
                                       "最近 %1 · 平均 %2 · P95 %3 · 最大 %4 ms")
                                       .arg(stream.decodeTransformWaitMs,
                                           0, 'f', 3)
                                       .arg(
                                           stream.averageDecodeTransformWaitMs,
                                           0, 'f', 3)
                                       .arg(stream.p95DecodeTransformWaitMs,
                                           0, 'f', 3)
                                       .arg(
                                           stream.maximumDecodeTransformWaitMs,
                                           0, 'f', 3),
                                   stream.p95DecodeTransformWaitMs >= 30.0
                                       ? QByteArray("warning")
                                       : QByteArray("normal"),
                                   true}
                            << DiagnosticsChip{
                                   QStringLiteral("decodeDelivery"),
                                   QStringLiteral("纹理交付耗时"),
                                   QStringLiteral("%1 ms").arg(
                                       stream.decodeOutputDeliveryMs,
                                       0, 'f', 3),
                                   stream.decodeOutputDeliveryMs >= 10.0
                                       ? QByteArray("warning")
                                       : QByteArray("normal"),
                                   false}
                            << DiagnosticsChip{
                                   QStringLiteral("decodeBacklog"),
                                   QStringLiteral("硬解积压"),
                                   QStringLiteral("排队 %1 · MFT内 %2 · 峰值 %3%4")
                                       .arg(stream.decodeQueuedInputFrames)
                                       .arg(stream.decodeInFlightFrames)
                                       .arg(stream.decodePeakBacklogFrames)
                                       .arg(
                                           stream.decodePipelineAsynchronous
                                               ? QStringLiteral(" · 异步")
                                               : QStringLiteral(" · 同步")),
                                   stream.decodePeakBacklogFrames >= 6
                                       ? QByteArray("warning")
                                       : QByteArray("normal"),
                                   true};
                    }
                } else {
                    streamCard.chips
                        << DiagnosticsChip{
                               QStringLiteral("audio"),
                               QStringLiteral("当前电平/累计隐藏"),
                               QStringLiteral(
                                   "电平 %1 · 隐藏样本 %2/%3 · %4 次")
                                   .arg(stream.audioLevel, 0,
                                        'f', 3)
                                   .arg(
                                       stream.concealedAudioSamples)
                                   .arg(stream.totalAudioSamples)
                                   .arg(stream.concealmentEvents),
                               stream.concealmentEvents > 0
                                   ? QByteArray("warning")
                                   : QByteArray("normal"),
                               true};
                }
                inboundCards.push_back(std::move(streamCard));
            }

            const QString line =
                QStringLiteral("%1\n%2")
                    .arg(peerHeader, details.join(QStringLiteral(" · ")));
            if (stream.direction == RtpStreamDirection::kOutbound) {
                outboundLines << line;
                hasOutbound = true;
            } else {
                inboundLines << line;
                hasInbound = true;
            }
        }
        if (!hasOutbound) {
            outboundLines
                << QStringLiteral("%1\n当前没有发送中的 RTP 媒体流。")
                       .arg(peerHeader);
            outboundCards.push_back(DiagnosticsCard{
                peerKey + QStringLiteral("/outbound-empty"),
                QStringLiteral("暂无发送媒体"),
                peerName,
                {{QStringLiteral("state"), QStringLiteral("状态"),
                  QStringLiteral("当前没有发送中的 RTP 媒体流"),
                  "normal", true}}});
        }
        if (!hasInbound) {
            inboundLines
                << QStringLiteral("%1\n当前没有接收中的 RTP 媒体流。")
                       .arg(peerHeader);
            inboundCards.push_back(DiagnosticsCard{
                peerKey + QStringLiteral("/inbound-empty"),
                QStringLiteral("暂无接收媒体"),
                peerName,
                {{QStringLiteral("state"), QStringLiteral("状态"),
                  QStringLiteral("当前没有接收中的 RTP 媒体流"),
                  "normal", true}}});
        }

        if (stats.dataChannels.empty()) {
            dataChannelLines
                << QStringLiteral("%1\n当前没有 DataChannel Stats。")
                       .arg(peerHeader);
            dataChannelCards.push_back(DiagnosticsCard{
                peerKey + QStringLiteral("/data-empty"),
                QStringLiteral("暂无数据通道"),
                peerName,
                {{QStringLiteral("state"), QStringLiteral("状态"),
                  QStringLiteral("当前没有 DataChannel Stats"),
                  "normal", true}}});
        } else {
            for (const auto& channel : stats.dataChannels) {
                dataChannelLines
                    << QStringLiteral(
                           "%1\n%2 · %3 · ↑ %4 / ↓ %5 · 缓冲 %6\n"
                           "累计 ↑ %7（%8 条）/ ↓ %9（%10 条）")
                           .arg(peerHeader)
                           .arg(QString::fromStdString(channel.label))
                           .arg(QString::fromStdString(channel.state))
                           .arg(FormatBitrate(channel.sendBitrateBps))
                           .arg(FormatBitrate(channel.receiveBitrateBps))
                           .arg(FormatByteCount(
                               channel.bufferedAmountBytes))
                           .arg(FormatByteCount(channel.bytesSent))
                           .arg(channel.messagesSent)
                           .arg(FormatByteCount(channel.bytesReceived))
                           .arg(channel.messagesReceived);

                DiagnosticsCard channelCard;
                channelCard.key =
                    peerKey + QLatin1Char('/') +
                    QString::fromStdString(channel.label);
                channelCard.title =
                    QString::fromStdString(channel.label);
                channelCard.subtitle = peerName;
                channelCard.chips = {
                    {QStringLiteral("state"), QStringLiteral("状态"),
                     QString::fromStdString(channel.state),
                     channel.state == "open" ? QByteArray("good")
                                             : QByteArray("warning"),
                     false},
                    {QStringLiteral("protocol"), QStringLiteral("协议"),
                     channel.protocol.empty()
                         ? QStringLiteral("SCTP")
                         : QString::fromStdString(channel.protocol),
                     "normal", false},
                    {QStringLiteral("sendRate"),
                     QStringLiteral("采样窗上行"),
                     FormatBitrate(channel.sendBitrateBps) +
                         SampleWindowSuffix(channel.sampleWindowMs),
                     "normal",
                     false},
                    {QStringLiteral("receiveRate"),
                     QStringLiteral("采样窗下行"),
                     FormatBitrate(channel.receiveBitrateBps) +
                         SampleWindowSuffix(channel.sampleWindowMs),
                     "normal",
                     false},
                    {QStringLiteral("buffer"), QStringLiteral("发送缓冲"),
                     FormatByteCount(channel.bufferedAmountBytes),
                     channel.bufferedAmountBytes >= 4 * 1024 * 1024
                         ? QByteArray("warning")
                         : QByteArray("good"),
                     false},
                    {QStringLiteral("messages"),
                     QStringLiteral("累计消息"),
                     QStringLiteral("↑ %1 · ↓ %2")
                         .arg(channel.messagesSent)
                         .arg(channel.messagesReceived),
                     "normal", false},
                    {QStringLiteral("bytes"), QStringLiteral("累计流量"),
                     QStringLiteral("↑ %1 · ↓ %2")
                         .arg(FormatByteCount(channel.bytesSent),
                              FormatByteCount(channel.bytesReceived)),
                     "normal", true}};
                dataChannelCards.push_back(std::move(channelCard));
            }
        }
    }

    setInputDebugValue(
        QStringLiteral("visionReturnedScene"),
        visionReturnedScene.isEmpty()
            ? QStringLiteral("暂无识别结果") : visionReturnedScene,
        visionReturnedScene.isEmpty() ? "muted" : "normal");
    setInputDebugValue(QStringLiteral("visionAcceptedScene"),
        visionAcceptedScene.isEmpty() ? QStringLiteral("等待确认场景")
                                     : visionAcceptedScene,
        visionAcceptedScene.isEmpty() ? "muted" : "normal");
    setInputDebugValue(QStringLiteral("visionResultAge"),
        visionReturnedScene.isEmpty() ? QStringLiteral("暂无结果")
            : QStringLiteral("%1 秒").arg(visionResultAgeMs / 1000.0, 0, 'f', 1),
        visionReturnedScene.isEmpty() ? "muted" : "normal");
    if (diagnosticsPageVisible && debugPage_->CurrentCategory() == 9 &&
        debugPage_->PolicyCardsWidget()) {
        static_cast<DiagnosticsCardsWidget*>(debugPage_->PolicyCardsWidget())->SetSections(
            policySections, QStringLiteral("暂无场景优化数据\n开启 AI 场景优化并共享屏幕后，可查看每个连接的场景和取舍设置。"));
    }
    setInputDebugValue(
        QStringLiteral("visionReturnedConfidence"),
        visionReturnedScene.isEmpty()
            ? QStringLiteral("暂无结果")
            : QStringLiteral("%1%（%2）")
                  .arg(visionReturnedConfidence * 100.0, 0, 'f', 1)
                  .arg(visionReturnedConfidence, 0, 'f', 3),
        visionReturnedScene.isEmpty() ? "muted" : "normal");
    if (visionPerformanceAvailable) {
        setInputDebugValue(
            QStringLiteral("visionScaleConvertTime"),
            QStringLiteral("%1 us（%2 ms）")
                .arg(visionScaleConvertTimeUs)
                .arg(visionScaleConvertTimeUs / 1000.0, 0, 'f', 3),
            "normal");
        setInputDebugValue(
            QStringLiteral("visionJpegEncodeTime"),
            QStringLiteral("%1 us（%2 ms）")
                .arg(visionJpegEncodeTimeUs)
                .arg(visionJpegEncodeTimeUs / 1000.0, 0, 'f', 3),
            "normal");
        setInputDebugValue(
            QStringLiteral("visionJpegSize"),
            QStringLiteral("%1 KB（%2 字节）")
                .arg(visionJpegBytes / 1024.0, 0, 'f', 2)
                .arg(visionJpegBytes),
            "normal");
    } else {
        const QString waiting = QStringLiteral("尚无远控识别数据");
        setInputDebugValue(
            QStringLiteral("visionScaleConvertTime"), waiting, "muted");
        setInputDebugValue(
            QStringLiteral("visionJpegEncodeTime"), waiting, "muted");
        setInputDebugValue(
            QStringLiteral("visionJpegSize"), waiting, "muted");
    }

    if (diagnostics.peerConnections.empty()) {
        overviewLines
            << QStringLiteral("当前没有成员对 P2P 连接。");
    }
    if (iceLines.isEmpty()) {
        iceLines << QStringLiteral("尚未采集到 ICE 与传输层状态。");
    }
    if (outboundLines.isEmpty()) {
        outboundLines << QStringLiteral("尚未采集到发送媒体流。");
    }
    if (inboundLines.isEmpty()) {
        inboundLines << QStringLiteral("尚未采集到接收媒体流。");
    }
    if (dataChannelLines.isEmpty()) {
        dataChannelLines << QStringLiteral("尚未采集到 DataChannel 状态。");
    }

    const QString overviewText =
        overviewLines.join(QStringLiteral("\n\n"));
    const QString iceText = iceLines.join(QStringLiteral("\n\n"));
    const QString outboundText =
        outboundLines.join(QStringLiteral("\n\n"));
    const QString inboundText =
        inboundLines.join(QStringLiteral("\n\n"));
    const QString dataChannelText =
        dataChannelLines.join(QStringLiteral("\n\n"));

    QVector<DiagnosticsSection> sections;
    if (!connectionCards.isEmpty()) {
        sections.push_back(DiagnosticsSection{
            QStringLiteral("connection"),
            QStringLiteral("连接概览"),
            QStringLiteral("传输路径、候选地址、带宽与安全协商"),
            std::move(connectionCards)});
    }
    if (!outboundCards.isEmpty()) {
        sections.push_back(DiagnosticsSection{
            QStringLiteral("outbound"),
            QStringLiteral("发送媒体"),
            QStringLiteral("本机发往房间成员的音视频流"),
            std::move(outboundCards)});
    }
    if (!inboundCards.isEmpty()) {
        sections.push_back(DiagnosticsSection{
            QStringLiteral("inbound"),
            QStringLiteral("接收媒体"),
            QStringLiteral("从房间成员接收并解码的音视频流"),
            std::move(inboundCards)});
    }
    if (!dataChannelCards.isEmpty()) {
        sections.push_back(DiagnosticsSection{
            QStringLiteral("data"),
            QStringLiteral("数据通道"),
            QStringLiteral("输入、控制、文件与遥测通道"),
            std::move(dataChannelCards)});
    }
    if (diagnosticsPageVisible && debugPage_->CurrentCategory() == 2 &&
        debugPage_->StatsCardsWidget()) {
        static_cast<DiagnosticsCardsWidget*>(debugPage_->StatsCardsWidget())
            ->SetSections(
                sections,
                QStringLiteral("当前没有成员对 P2P 连接"));
    }

    if (diagnosticsCopyTextRequested_) {
        statsDebugCopyText_ =
            QStringList{
                QStringLiteral("【WebRTC 采样与逐帧遥测】"),
                QStringLiteral("传输快照：\n%1").arg(overviewText),
                QStringLiteral("ICE 与安全：\n%1").arg(iceText),
                QStringLiteral("发送媒体流：\n%1").arg(outboundText),
                QStringLiteral("接收媒体流：\n%1").arg(inboundText),
                QStringLiteral("DataChannel：\n%1").arg(dataChannelText),
                QStringLiteral("场景优化：\n%1").arg(ContentPolicyCopyText(policySections)),
                QStringLiteral("鼠标与键盘：\n%1").arg(inputDebugText)}
                .join(QStringLiteral("\n\n"));
    }
}

}  // namespace remote::controller
