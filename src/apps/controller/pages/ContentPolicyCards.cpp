// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)
#include "ContentPolicyCards.h"
#include "src/apps/controller/ControllerMainWindowSupport.h"
#include <QStringList>
#include <map>
#include <tuple>
#include <utility>

namespace remote::controller::detail {
DiagnosticsSection MakeContentPolicySection(const QString& key,
    const QString& peerName, const RtpStreamStatsSnapshot& stream)
{
    const auto& smoothing = stream.sceneQualitySmoothing;
    const bool knownScene = smoothing.observed && !smoothing.scene.empty() &&
        smoothing.scene != "unknown";
    const auto coefficient = [](double value) { return QString::number(value, 'f', 2); };
    const QString status = !smoothing.status.empty() ? QString::fromStdString(smoothing.status)
        : !smoothing.enabled ? QStringLiteral("AI 场景优化已关闭，使用手动设置")
        : !knownScene ? QStringLiteral("等待场景识别，保持当前设置")
        : smoothing.transitioning ? QStringLiteral("正在调整取舍") : QStringLiteral("已应用场景推荐设置");
    DiagnosticsSection section{key,
        QStringLiteral("%1 · %2").arg(peerName.isEmpty() ? QStringLiteral("未知成员") : peerName,
                                      SlotDisplayName(stream.slot, stream.kind)),
        QStringLiteral("每秒更新"), {}, true};
    DiagnosticsCard coefficients{QStringLiteral("scene-coefficient"),
        QStringLiteral("当前场景与取舍设置"), {}, {}, true, true, true};
    coefficients.chips = {
        {"scene", QStringLiteral("当前场景"), knownScene
            ? ContentSceneDisplayText(smoothing.scene) : QStringLiteral("尚未确认"),
            knownScene ? "normal" : "muted"},
        {"current", QStringLiteral("当前取舍系数"), coefficient(smoothing.currentCoefficient), "good"},
        {"target", QStringLiteral("场景推荐系数"), knownScene
            ? coefficient(smoothing.targetCoefficient) : QStringLiteral("尚未确定")},
        {"range", QStringLiteral("场景推荐范围"), knownScene
            ? QStringLiteral("%1 ～ %2").arg(coefficient(smoothing.minimumCoefficient),
                                             coefficient(smoothing.maximumCoefficient))
            : QStringLiteral("尚未确定")},
        {"manual", QStringLiteral("手动设置值"),
            coefficient(smoothing.manualCoefficientHundredths / 100.0)},
        {"remaining", QStringLiteral("调整剩余时间"), smoothing.transitioning
            ? QStringLiteral("%1 秒").arg(smoothing.remainingMs / 1000.0, 0, 'f', 2)
            : QStringLiteral("0 秒")},
        {"status", QStringLiteral("状态"), status, "normal", true},
        {"behavior", QStringLiteral("调整说明"),
            QStringLiteral("网络波动时，数值越小越优先保画质，越大越优先保帧率。推荐值取推荐范围的中间值。"), "muted", true},
        {"scope", QStringLiteral("影响范围"),
            QStringLiteral("只调整网络波动时的画质与帧率取舍，不改变采集帧率和用户设置，原有网络控制保持不变。"),
            "muted", true}};
    section.cards.push_back(std::move(coefficients));
    return section;
}

QVector<DiagnosticsSection> MakeContentPolicySections(const QString& peerKey,
    const QString& peerName, const std::vector<RtpStreamStatsSnapshot>& streams)
{
    // A slot can have several RTP stats objects. Show the active primary sender
    // once, preserving the section identity when its stats ID changes.
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
        const auto& smoothing = stream.sceneQualitySmoothing;
        if (!smoothing.enabled && !smoothing.observed) continue;
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
