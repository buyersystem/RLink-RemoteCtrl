// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)
#include "ContentPolicyCards.h"
#include "src/apps/controller/ControllerMainWindowSupport.h"
#include <QStringList>
#include <map>
#include <tuple>
#include <utility>

namespace remote::controller::detail {
namespace {
QString GoogCcReasonText(const std::string& reason)
{
    if (reason == "delay_overuse") return QStringLiteral("GoogCC 延迟检测：发送过量");
    if (reason == "congestion_window_pushback") return QStringLiteral("GoogCC 拥塞窗口要求缩减负载");
    if (reason == "delay_and_congestion_window") return QStringLiteral("延迟过量与拥塞窗口缩减同时出现");
    if (reason == "episode_retained") return QStringLiteral("当前负载已稳定，仍按受限预算运行");
    if (reason == "stale_feedback") return QStringLiteral("传输反馈过期，等待新反馈");
    if (reason == "stale_controller") return QStringLiteral("控制器输出过期，等待新输出");
    if (reason == "normal") return QStringLiteral("未确认拥塞，保持用户规格");
    return QStringLiteral("等待 GoogCC 网络证据");
}
QString GoogCcDelayText(const std::string& state)
{
    if (state == "overuse") return QStringLiteral("Overuse · 发送过量");
    if (state == "underuse") return QStringLiteral("Underuse · 队列回落");
    if (state == "normal") return QStringLiteral("Normal · 延迟趋势稳定");
    return QStringLiteral("尚未观察到延迟状态");
}
QString ConfirmationText(const std::string& reason)
{
    if (reason == "awaiting_fresh_sample") return QStringLiteral("等待下一份新鲜统计窗口");
    if (reason == "awaiting_evidence") return QStringLiteral("等待满足预算与画质要求的候选");
    if (reason == "awaiting_samples") return QStringLiteral("等待连续可行窗口");
    if (reason == "awaiting_stable_time") return QStringLiteral("候选可行，正在确认稳定时间");
    if (reason == "awaiting_residence") return QStringLiteral("候选可行，正在确认恢复驻留时间");
    if (reason == "awaiting_apply_interval") return QStringLiteral("等待最短调整间隔");
    if (reason == "ready") return QStringLiteral("确认通过");
    return QStringLiteral("当前没有待确认的参数调整");
}
} // namespace
DiagnosticsSection MakeContentPolicySection(const QString& key,
    const QString& peerName, const RtpStreamStatsSnapshot& stream)
{
    const auto& shadow = stream.contentPolicyShadow;
    const auto& execution = stream.contentPolicyExecution;
    const bool waiting = !shadow.observed || shadow.reason == "invalid_input" ||
        shadow.reason == "activity_unavailable" || shadow.reason == "capacity_unavailable" ||
        shadow.reason == "stale_network" || shadow.reason == "generation_mismatch";
    const bool holding = waiting || shadow.reason == "idle_hold" || shadow.reason == "healthy_hold";
    const bool knownScene = !stream.contentScene.empty() && stream.contentScene != "unknown";
    const auto size = [](std::uint32_t width, std::uint32_t height) {
        return width && height ? QStringLiteral("%1 × %2").arg(width).arg(height)
                               : QStringLiteral("等待数据");
    };
    const auto fps = [](std::uint32_t value) {
        return value ? QStringLiteral("%1 FPS").arg(value) : QStringLiteral("未报告");
    };
    const auto candidate = shadow.observed && shadow.hasRecommendation
        ? QStringLiteral("%1 · %2").arg(size(shadow.width, shadow.height), fps(shadow.senderMaxFps))
        : QStringLiteral("尚未评估");
    const auto status = ContentPolicyExecutionDisplayText(execution.status);
    const QByteArray statusTone = !execution.error.empty() ? "error"
        : shadow.reason == "healthy_hold" || execution.status == "applied" ? "good" : "normal";
    DiagnosticsSection section{key,
        QStringLiteral("%1 · %2").arg(peerName.isEmpty() ? QStringLiteral("未知成员") : peerName,
                                      SlotDisplayName(stream.slot, stream.kind)),
        QStringLiteral("每秒更新"), {}, true};
    const auto card = [](const QString& key, const QString& title, bool expanded = true) {
        return DiagnosticsCard{key, title, {}, {}, expanded, true, true};
    };
    auto overview = card(QStringLiteral("overview"), QStringLiteral("场景与执行"));
    overview.chips = {
        {"scene", QStringLiteral("当前采用场景"), knownScene ? ContentSceneDisplayText(stream.contentScene)
            : QStringLiteral("等待稳定分类"), knownScene ? "normal" : "muted"},
        {"user-target", QStringLiteral("用户目标帧率"), fps(stream.configuredMaxFrameRate)},
        {"current", QStringLiteral("实际编码画面"),
            QStringLiteral("%1 · %2 FPS").arg(size(stream.frameWidth, stream.frameHeight))
                .arg(stream.encodedFramesPerSecond, 0, 'f', 1)},
        {"candidate", holding ? QStringLiteral("保留规格") : QStringLiteral("候选规格（尚非实际值）"), candidate},
        {"limit", QStringLiteral("当前发送上限"), QStringLiteral("%1 · %2")
            .arg(fps(stream.effectiveNetworkFrameRate), stream.configuredMaxBitrateBps
                ? FormatBitrate(stream.configuredMaxBitrateBps) : QStringLiteral("未报告"))},
        {"status", QStringLiteral("执行状态"), status.isEmpty() ? QStringLiteral("等待策略数据") : status, statusTone, true},
        {"reason", QStringLiteral("决策依据"), shadow.observed
            ? ContentPolicyReasonDisplayText(shadow.reason) : QStringLiteral("等待统计窗口"), "normal", true}};
    if (!knownScene) overview.chips.push_back({"sceneHint", QStringLiteral("分类说明"),
        stream.contentAnalyzerBackend == "rules" ? QStringLiteral("本地规则仅分析运动，尚无细场景分类")
            : QStringLiteral("尚未取得稳定的大模型分类；未知场景不会当作已识别场景"), "muted", true});
    overview.chips.push_back({"confirmation", QStringLiteral("调整确认"),
        execution.status == "awaiting_candidate_budget"
            ? QStringLiteral("尚未开始：当前预算不足")
            : ConfirmationText(execution.confirmationBlock), "normal", true});
    if (execution.confirmRequiredSamples) {
        overview.chips.push_back({"confirmation-samples", QStringLiteral("连续确认窗口"),
            QStringLiteral("%1 / %2").arg(execution.confirmObservedSamples).arg(execution.confirmRequiredSamples)});
        overview.chips.push_back({"confirmation-time", QStringLiteral("内部确认剩余时间"),
            QStringLiteral("%1 秒（不含等待网络预算）").arg(execution.confirmationRemainingMs / 1000.0, 0, 'f', 1)});
    }
    if (!execution.error.empty()) overview.chips.push_back({"error", QStringLiteral("执行错误"),
        QString::fromStdString(execution.error), "error", true});
    section.cards.push_back(std::move(overview));

    const auto& gcc = stream.googCc;
    const bool networkWaiting = execution.networkStatus.empty() ||
        execution.networkStatus == "unavailable" || execution.networkStatus == "stale_feedback" ||
        execution.networkStatus == "stale_controller";
    const auto networkTone = networkWaiting ? QByteArray("muted")
        : execution.networkPressure ? QByteArray("normal") : QByteArray("good");
    auto network = card(QStringLiteral("network"), QStringLiteral("GoogCC 网络判断"));
    network.chips = {
        {"state", QStringLiteral("适应状态"), !gcc.controllerObserved && gcc.delayObserved
            ? QStringLiteral("控制器观察尚未接通，场景调整暂停")
            : GoogCcReasonText(execution.networkStatus), networkTone, true},
        {"trigger", QStringLiteral("本轮拥塞依据"), execution.networkTrigger.empty()
            ? QStringLiteral("尚无明确拥塞依据") : GoogCcReasonText(execution.networkTrigger), "normal", true},
        {"delay", QStringLiteral("GoogCC 延迟状态"), gcc.delayObserved
            ? GoogCcDelayText(gcc.delayState) : QStringLiteral("尚未观察到延迟状态")},
        {"overuse-event", QStringLiteral("最近延迟过载事件"), gcc.lastDelayOveruseAtMs
            ? QStringLiteral("已记录，按新鲜度消费") : QStringLiteral("尚未发生")},
        {"pushback", QStringLiteral("窗口负载缩减"), gcc.controllerObserved
            ? QStringLiteral("%1%").arg(gcc.congestionWindowReduction * 100, 0, 'f', 1)
            : QStringLiteral("未报告")},
        {"target", QStringLiteral("GoogCC 发送预算"), gcc.controllerObserved
            ? FormatBitrate(gcc.targetRateBps) : QStringLiteral("未报告")},
        {"effective", QStringLiteral("窗口缩减后的预算"), gcc.controllerObserved
            ? FormatBitrate(gcc.effectiveTargetRateBps) : QStringLiteral("未报告")},
        {"fresh", QStringLiteral("传输反馈"), gcc.feedbackAtMs
            ? QStringLiteral("%1 · %2 ms 前").arg(gcc.feedbackFresh
                ? QStringLiteral("新鲜") : QStringLiteral("已过期")).arg(gcc.feedbackAgeMs)
            : QStringLiteral("尚无有效反馈"), gcc.feedbackFresh ? "good" : "muted", true},
        {"hint", QStringLiteral("判断说明"), !gcc.controllerObserved && gcc.delayObserved
            ? QStringLiteral("仅收到延迟事件，尚无控制器预算与反馈；Normal 不代表完整网络判断。")
            : execution.networkPressure
            ? networkWaiting ? QStringLiteral("保留本轮恢复状态；证据更新前不执行新的场景调整。")
                : QStringLiteral("Normal/Underuse 只描述队列趋势，不代表限速解除。预算接近用户视频上限时恢复原规格；画质另行验证。")
            : QStringLiteral("理论需求、QP、低流量和运动变化不会单独触发规格下降。"), "muted", true}};
    section.cards.push_back(std::move(network));

    auto measurements = card(QStringLiteral("network-reference"), QStringLiteral("网络测量参考"), false);
    measurements.chips = {
        {"rtt", QStringLiteral("GoogCC RTT"), gcc.controllerObserved && gcc.roundTripTimeMs > 0
            ? QStringLiteral("%1 ms").arg(gcc.roundTripTimeMs, 0, 'f', 1) : QStringLiteral("未报告")},
        {"loss", QStringLiteral("GoogCC 丢包率"), gcc.controllerObserved
            ? QStringLiteral("%1%").arg(gcc.lossPercent, 0, 'f', 2) : QStringLiteral("未报告")},
        {"alr", QStringLiteral("应用受限（ALR）"), gcc.controllerObserved
            ? gcc.applicationLimited ? QStringLiteral("是 · 应用发送需求偏低")
                : QStringLiteral("否 · 接近估计容量") : QStringLiteral("未报告")},
        {"hint", QStringLiteral("说明"), QStringLiteral("RTT、丢包率与 ALR 仅作参考，不设本地弱网阈值。未公开的丢包降速与 RTT 回退状态尚未单独接入。"), "muted", true}};
    section.cards.push_back(std::move(measurements));

    auto budget = card(QStringLiteral("budget"), QStringLiteral("码率预算"));
    const auto bit = [&](std::uint64_t value) {
        return shadow.observed ? FormatBitrate(value) : QStringLiteral("等待数据");
    };
    budget.chips = {
        {"user-bpp", QStringLiteral("用户视频系数"), stream.userVideoBitrateBppHundredths
            ? QString::number(stream.userVideoBitrateBppHundredths / 100.0, 'f', 2)
            : QStringLiteral("未报告")},
        {"user-cap", QStringLiteral("用户视频总上限"), stream.userVideoBitrateLimitBps
            ? FormatBitrate(stream.userVideoBitrateLimitBps) : QStringLiteral("未报告")},
        {"available", QStringLiteral("估算视频预算"), bit(shadow.estimatedSafeVideoBudgetBps)},
        {"required", QStringLiteral("候选参考需求"), shadow.requiredVideoBitrateBps
            ? FormatBitrate(shadow.requiredVideoBitrateBps) : QStringLiteral("尚未评估")},
        {"desired", QStringLiteral("候选期望码率"), bit(shadow.desiredVideoBitrateBps)},
        {"max", QStringLiteral("候选发送上限"), bit(shadow.senderMaxBitrateBps)},
        {"verified", QStringLiteral("原规格实测恢复需求"), execution.userSpecificationVerifiedBitrateBps
            ? FormatBitrate(execution.userSpecificationVerifiedBitrateBps) : QStringLiteral("等待运行验证")},
        {"feasibility", QStringLiteral("预算评估"), waiting ? QStringLiteral("等待数据，尚未评估候选")
            : holding ? QStringLiteral("保持当前规格，参考需求不强制触发降级")
            : shadow.estimatedFeasible ? QStringLiteral("候选符合估算预算")
            : shadow.reason == "user_specification_restore"
                ? QStringLiteral("预算已接近用户视频上限，恢复原规格；参考画质仍待验证")
            : shadow.reason == "emergency_network_reduction"
                ? QStringLiteral("按场景逐步降低负载，当前预算仍不足以满足参考画质")
            : QStringLiteral("当前预算及质量要求下未找到可行候选"), "normal", true},
        {"model", QStringLiteral("需求模型"), shadow.modelCalibrated ? QStringLiteral("已标定模型")
            : shadow.modelReference ? QStringLiteral("参考模型，结合实时指标执行")
            : QStringLiteral("等待场景数据或可用需求模型"), "muted", true}};
    section.cards.push_back(std::move(budget));

    auto quality = card(QStringLiteral("quality"), QStringLiteral("质量与耗时参考"), false);
    quality.chips = {
        {"qp", QStringLiteral("采样窗平均 QP"), stream.windowQpAvailable
            ? QString::number(stream.windowQp, 'f', 1) : QStringLiteral("未报告")},
        {"quality", QStringLiteral("质量门槛"), !stream.contentQualityMetricAvailable ? QStringLiteral("缺少指标")
            : stream.contentQualityVerified ? QStringLiteral("通过") : QStringLiteral("未通过"),
            stream.contentQualityVerified ? "good" : "normal"},
        {"processing", QStringLiteral("处理耗时参考"), !stream.contentProcessingEvidenceAvailable
            ? QStringLiteral("缺少完整耗时数据") : stream.contentProcessingHealthy
                ? QStringLiteral("正常") : QStringLiteral("偏高"), "normal"},
        {"changes", QStringLiteral("累计策略调整"), QStringLiteral("%1 次").arg(execution.successfulChanges)},
        {"hint", QStringLiteral("说明"), QStringLiteral("处理耗时仅作诊断，不参与执行门控。"), "muted", true}};
    quality.chips.push_back({"applied", QStringLiteral("最近成功应用规格"), execution.applied
        ? QStringLiteral("%1 · %2 · %3").arg(size(execution.appliedWidth, execution.appliedHeight),
            fps(execution.appliedMaxFps), FormatBitrate(execution.appliedMaxBitrateBps))
        : QStringLiteral("尚无场景调整"), "normal", true});
    section.cards.push_back(std::move(quality));

    auto feedback = card(QStringLiteral("feedback"), QStringLiteral("接收端反馈"), false);
    const auto time = [&](bool available, double ms) {
        return stream.receiverFeedbackAvailable && available
            ? QStringLiteral("%1 ms").arg(ms, 0, 'f', 2) : QStringLiteral("未报告");
    };
    feedback.chips = {
        {"state", QStringLiteral("反馈状态"), stream.receiverFeedbackAvailable
            ? QStringLiteral("有效 · %1 ms 前").arg(stream.receiverFeedbackAgeMs)
            : QStringLiteral("尚未收到有效反馈或已过期"), "normal", true},
        {"size", QStringLiteral("反馈分辨率"), stream.receiverFeedbackAvailable
            ? size(stream.receiverFeedbackWidth, stream.receiverFeedbackHeight) : QStringLiteral("未报告")},
        {"frames", QStringLiteral("采样窗帧数"), stream.receiverFeedbackAvailable
            ? QStringLiteral("解码 %1 · 丢弃 %2").arg(stream.receiverFeedbackDecodedFrames)
                .arg(stream.receiverFeedbackDroppedFrames) : QStringLiteral("未报告")},
        {"decode", QStringLiteral("平均解码耗时"), time(stream.receiverFeedbackDecodeTimeAvailable, stream.receiverFeedbackDecodeTimeMs)},
        {"processing", QStringLiteral("平均处理耗时"), time(stream.receiverFeedbackProcessingTimeAvailable, stream.receiverFeedbackProcessingTimeMs)}};
    section.cards.push_back(std::move(feedback));
    return section;
}

QVector<DiagnosticsSection> MakeContentPolicySections(const QString& peerKey,
    const QString& peerName, const std::vector<RtpStreamStatsSnapshot>& streams)
{
    // Slot policy is mirrored onto multiple RTP stats objects. Display the
    // current primary sender once; stats IDs can change after a restart.
    std::map<std::string, const RtpStreamStatsSnapshot*> bySlot;
    const auto rank = [](const RtpStreamStatsSnapshot& stream) {
        const bool dimensions = stream.frameWidth > 0 && stream.frameHeight > 0;
        const bool activity = stream.bitrateBps > 0 || stream.encodedFramesPerSecond > 0 ||
            stream.sentFramesPerSecond > 0;
        const bool window = stream.sampleWindowMs >= 500;
        return std::tuple(window && dimensions && activity, dimensions && activity,
            dimensions, window && activity, stream.bytes);
    };
    for (const auto& stream : streams) {
        if (stream.direction != RtpStreamDirection::kOutbound || stream.kind != "video" ||
            stream.slot.empty()) continue;
        const auto codec = QString::fromStdString(stream.codec).toLower();
        if (codec.endsWith(QStringLiteral("/rtx")) || codec == QStringLiteral("rtx") ||
            codec.endsWith(QStringLiteral("/red")) || codec == QStringLiteral("red") ||
            codec.contains(QStringLiteral("ulpfec")) || codec.contains(QStringLiteral("flexfec"))) continue;
        const auto& shadow = stream.contentPolicyShadow;
        const auto& execution = stream.contentPolicyExecution;
        if (!shadow.observed && !execution.observed && !execution.applied &&
            !execution.successfulChanges && execution.error.empty()) continue;
        auto [entry, inserted] = bySlot.emplace(stream.slot, &stream);
        if (!inserted && (rank(stream) > rank(*entry->second) ||
            (rank(stream) == rank(*entry->second) && stream.statsId < entry->second->statsId)))
            entry->second = &stream;
    }
    QVector<DiagnosticsSection> sections;
    for (const auto& [slot, stream] : bySlot) {
        sections.push_back(MakeContentPolicySection(peerKey + QStringLiteral("/content-policy/") +
            QString::fromStdString(slot), peerName, *stream));
    }
    return sections;
}

QString ContentPolicyCopyText(const QVector<DiagnosticsSection>& sections)
{
    QStringList lines;
    for (const auto& section : sections) {
        lines << section.title;
        for (const auto& card : section.cards) {
            lines << QStringLiteral("【%1】").arg(card.title);
            for (const auto& chip : card.chips)
                lines << QStringLiteral("%1：%2").arg(chip.label, chip.value);
        }
        lines << QString();
    }
    return lines.join(QLatin1Char('\n'));
}
} // namespace remote::controller::detail
