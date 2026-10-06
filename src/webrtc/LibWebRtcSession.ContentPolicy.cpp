// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)
#include "LibWebRtcSession.Internal.h"
#include "GoogCcTelemetry.h"
#include <cmath>

namespace remote {
using namespace webrtc_session_detail;

void LibWebRtcSession::SetSceneQualityObservation(const SceneQualityObservation& observation)
{
    std::lock_guard lock(mutex_);
    // A concurrent older capture snapshot cannot resurrect a previous scene.
    if (observation.observedAtMs < sceneQualityObservation_.observedAtMs) return;
    const auto now = (std::max)((std::max)(SteadyNowMs(), observation.observedAtMs), sceneQualityLastTickMs_);
    const bool changedSession = observation.generation != sceneQualityObservation_.generation;
    const bool enabling = observation.enabled && !sceneQualityObservation_.enabled;
    if (enabling) {
        sceneQualitySmoother_.Reset(screenQualityDeficitShareHundredths_ / 100.0, now);
        sceneQualityLastResultMs_ = 0;
    } else if (changedSession) {
        sceneQualitySmoother_.SetTarget(screenQualityDeficitShareHundredths_ / 100.0, now);
        sceneQualityLastResultMs_ = 0;
    }
    sceneQualityObservation_ = observation;
    sceneQualityObservation_.maximumSceneAgeMs = (std::max<std::uint64_t>)(1000, observation.maximumSceneAgeMs);
    const auto range = media_intelligence::SceneQualityCoefficientSmoother::RangeForScene(observation.scene);
    if (observation.enabled && range.known && observation.sceneObservedAtMs != 0 &&
        observation.sceneObservedAtMs <= now &&
        now - observation.sceneObservedAtMs <= sceneQualityObservation_.maximumSceneAgeMs &&
        observation.sceneObservedAtMs >= sceneQualityLastResultMs_) {
        sceneQualitySmoother_.SetScene(observation.scene, now);
        sceneQualityLastResultMs_ = observation.sceneObservedAtMs;
    }
    if (!observation.enabled) {
        sceneQualityLastResultMs_ = 0;
        sceneQualitySmoother_.Reset(screenQualityDeficitShareHundredths_ / 100.0, now);
    }
    UpdateScreenQualityProtectionLocked();
}

void LibWebRtcSession::UpdateSceneQualityCoefficient(std::uint64_t nowMs)
{
    std::lock_guard lock(mutex_);
    if (!sceneQualityObservation_.enabled) return;
    UpdateSceneQualityCoefficientLocked(nowMs);
}

void LibWebRtcSession::UpdateSceneQualityCoefficientLocked(std::uint64_t nowMs)
{
    // Statistics callbacks and the existing poller may interleave. Never rewind
    // a transition when a previously captured timestamp arrives.
    const auto now = (std::max)(sceneQualityLastTickMs_, nowMs);
    sceneQualityLastTickMs_ = now;
    if (sceneQualityObservation_.enabled && sceneQualityLastResultMs_ &&
        now >= sceneQualityLastResultMs_ &&
        now - sceneQualityLastResultMs_ > sceneQualityObservation_.maximumSceneAgeMs) {
        sceneQualityLastResultMs_ = 0;
        sceneQualitySmoother_.SetTarget(screenQualityDeficitShareHundredths_ / 100.0, now);
    }
    const auto coefficient = sceneQualityObservation_.enabled
        ? static_cast<std::uint32_t>(std::lround(sceneQualitySmoother_.Evaluate(now) * 100.0))
        : screenQualityDeficitShareHundredths_;
    // This is the only encoder-facing scene output. No capture call, RTP
    // SetParameters, connection SetBitrate, or recovery restart occurs here.
    if (googCcTelemetry_ && googCcTelemetry_->ScreenQualityDeficitShare() != coefficient)
        googCcTelemetry_->SetScreenQualityDeficitShare(coefficient);
}

SceneQualitySmoothingSnapshot LibWebRtcSession::SceneQualitySnapshotLocked(std::uint64_t nowMs) const
{
    SceneQualitySmoothingSnapshot snapshot;
    snapshot.enabled = sceneQualityObservation_.enabled;
    snapshot.manualCoefficientHundredths = screenQualityDeficitShareHundredths_;
    const auto now = (std::max)(sceneQualityLastTickMs_, nowMs);
    const auto range = sceneQualitySmoother_.Range();
    snapshot.observed = snapshot.enabled && range.known;
    snapshot.scene = media_intelligence::ScreenSceneName(sceneQualitySmoother_.Scene());
    snapshot.minimumCoefficient = range.minimum;
    snapshot.maximumCoefficient = range.maximum;
    snapshot.targetCoefficient = snapshot.enabled ? sceneQualitySmoother_.Target()
        : screenQualityDeficitShareHundredths_ / 100.0;
    // Report what the codec actually reads, rather than a future tick's value.
    snapshot.currentCoefficient = (googCcTelemetry_ ? googCcTelemetry_->ScreenQualityDeficitShare()
        : screenQualityDeficitShareHundredths_) / 100.0;
    snapshot.transitioning = snapshot.enabled && sceneQualitySmoother_.IsTransitioning(now);
    snapshot.remainingMs = snapshot.transitioning ? sceneQualitySmoother_.RemainingMs(now) : 0;
    snapshot.status = !snapshot.enabled ? "AI 场景优化已关闭，使用手动设置"
        : !snapshot.observed ? (snapshot.transitioning ? "场景识别结果已过期，正在恢复手动设置"
            : "等待场景识别，使用手动设置")
        : snapshot.transitioning ? "正在调整到场景推荐值" : "已应用场景推荐值";
    return snapshot;
}
} // namespace remote
