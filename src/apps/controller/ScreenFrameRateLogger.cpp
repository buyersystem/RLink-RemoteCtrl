// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ScreenFrameRateLogger.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QString>
#include <QStringConverter>
#include <QTextStream>

#include <condition_variable>
#include <cstdint>
#include <deque>
#include <mutex>
#include <stop_token>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "src/core/SessionDiagnostics.h"
#include "src/webrtc/IWebRtcSession.h"

namespace remote::controller::detail {
namespace {

QString CsvCell(const std::string &value) {
  QString escaped = QString::fromStdString(value);
  escaped.replace('"', QStringLiteral("\"\""));
  return QStringLiteral("\"%1\"").arg(escaped);
}

struct ScreenFrameRateLogRecord {
  qint64 timestampMs = 0;
  std::string pairId;
  std::string peerDeviceId;
  bool outbound = false;
  std::string configuredCaptureBackend;
  std::string activeCaptureBackend;
  std::string captureFallbackReason;
  std::string activity;
  std::uint32_t captureTargetFps = 0;
  double captureAttemptFps = 0.0;
  double captureDeliveredFps = 0.0;
  double captureChangedFps = 0.0;
  double captureHeartbeatFps = 0.0;
  std::uint64_t captureSuppressedTotal = 0;
  bool captureInputBoostActive = false;
  std::uint64_t captureInputBoostTotal = 0;
  std::uint64_t captureForcedRefreshTotal = 0;
  double sourceFps = 0.0;
  double rtpFps = 0.0;
  double encodedFps = 0.0;
  double sentFps = 0.0;
  double presentedFps = 0.0;
  std::uint64_t bitrateBps = 0;
};

class AsyncScreenFrameRateLogger final {
public:
  static AsyncScreenFrameRateLogger &Instance() {
    static AsyncScreenFrameRateLogger logger;
    return logger;
  }

  void Enqueue(std::vector<ScreenFrameRateLogRecord> records) {
    if (records.empty()) {
      return;
    }
    {
      std::lock_guard lock(mutex_);
      for (auto &record : records) {
        if (records_.size() >= kMaximumQueuedRecords) {
          records_.pop_front();
        }
        records_.push_back(std::move(record));
      }
    }
    condition_.notify_one();
  }

private:
  static constexpr std::size_t kMaximumQueuedRecords = 4096;

  AsyncScreenFrameRateLogger()
      : worker_([this](std::stop_token stopToken) { Run(stopToken); }) {}

  ~AsyncScreenFrameRateLogger() {
    worker_.request_stop();
    condition_.notify_all();
    if (worker_.joinable()) {
      worker_.join();
    }
  }

  void Run(std::stop_token stopToken) {
    const QString directory =
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (directory.isEmpty() || !QDir().mkpath(directory)) {
      return;
    }
    const QString logPath =
        QDir(directory).filePath(QStringLiteral("screen-frame-rate-%1.csv")
                                     .arg(QCoreApplication::applicationPid()));
    QFile logFile(logPath);
    const bool writeHeader = !logFile.exists() || logFile.size() == 0;
    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Append |
                      QIODevice::Text)) {
      return;
    }

    QTextStream output(&logFile);
    output.setEncoding(QStringConverter::Utf8);
    if (writeHeader) {
      output << "time,pair_id,peer_device_id,direction,"
                "configured_capture_backend,active_capture_backend,"
                "capture_fallback_reason,activity,"
                "capture_target_fps,capture_attempt_fps,"
                "capture_delivered_fps,capture_changed_fps,"
                "capture_heartbeat_fps,capture_suppressed_total,"
                "capture_input_boost_active,capture_input_boost_total,"
                "capture_forced_refresh_total,"
                "source_fps,rtp_fps,encoded_fps,sent_fps,"
                "presented_fps,bitrate_bps\n";
    }

    for (;;) {
      std::deque<ScreenFrameRateLogRecord> pending;
      {
        std::unique_lock lock(mutex_);
        condition_.wait(lock, [this, &stopToken] {
          return stopToken.stop_requested() || !records_.empty();
        });
        pending.swap(records_);
      }
      for (const auto &record : pending) {
        const QString timestamp =
            QDateTime::fromMSecsSinceEpoch(record.timestampMs)
                .toString(Qt::ISODateWithMs);
        output << timestamp << ',' << CsvCell(record.pairId) << ','
               << CsvCell(record.peerDeviceId) << ','
               << (record.outbound ? "outbound" : "inbound") << ','
               << CsvCell(record.configuredCaptureBackend) << ','
               << CsvCell(record.activeCaptureBackend) << ','
               << CsvCell(record.captureFallbackReason) << ','
               << CsvCell(record.activity) << ',' << record.captureTargetFps
               << ',' << QString::number(record.captureAttemptFps, 'f', 3)
               << ',' << QString::number(record.captureDeliveredFps, 'f', 3)
               << ',' << QString::number(record.captureChangedFps, 'f', 3)
               << ',' << QString::number(record.captureHeartbeatFps, 'f', 3)
               << ',' << record.captureSuppressedTotal << ','
               << (record.captureInputBoostActive ? 1 : 0) << ','
               << record.captureInputBoostTotal << ','
               << record.captureForcedRefreshTotal << ','
               << QString::number(record.sourceFps, 'f', 3) << ','
               << QString::number(record.rtpFps, 'f', 3) << ','
               << QString::number(record.encodedFps, 'f', 3) << ','
               << QString::number(record.sentFps, 'f', 3) << ','
               << QString::number(record.presentedFps, 'f', 3) << ','
               << record.bitrateBps << '\n';
      }
      output.flush();
      if (stopToken.stop_requested()) {
        std::lock_guard lock(mutex_);
        if (records_.empty()) {
          break;
        }
      }
    }
  }

  std::mutex mutex_;
  std::condition_variable condition_;
  std::deque<ScreenFrameRateLogRecord> records_;
  std::jthread worker_;
};

} // namespace

void AppendScreenFrameRateLog(const SessionDiagnosticsSnapshot &diagnostics) {
  std::vector<ScreenFrameRateLogRecord> records;
  const qint64 timestampMs = QDateTime::currentMSecsSinceEpoch();
  for (const auto &peer : diagnostics.peerConnections) {
    for (const auto &stream : peer.stats.rtpStreams) {
      if (stream.kind != "video" || stream.slot != kScreenMainVideoSlot) {
        continue;
      }
      ScreenFrameRateLogRecord record;
      record.timestampMs = timestampMs;
      record.pairId = peer.pairId;
      record.peerDeviceId = peer.peerDeviceId;
      record.outbound = stream.direction == RtpStreamDirection::kOutbound;
      record.configuredCaptureBackend = stream.captureConfiguredBackend;
      record.activeCaptureBackend = stream.captureActiveBackend;
      record.captureFallbackReason = stream.captureFallbackReason;
      record.activity = stream.captureActivityState;
      record.captureTargetFps = stream.captureTargetFrameRate;
      record.captureAttemptFps = stream.captureAttemptsPerSecond;
      record.captureDeliveredFps = stream.captureDeliveredFramesPerSecond;
      record.captureChangedFps = stream.captureChangedFramesPerSecond;
      record.captureHeartbeatFps = stream.captureIdleHeartbeatFramesPerSecond;
      record.captureSuppressedTotal = stream.captureSuppressedUnchangedFrames;
      record.captureInputBoostActive = stream.captureInputBoostActive;
      record.captureInputBoostTotal = stream.captureInputBoosts;
      record.captureForcedRefreshTotal = stream.captureForcedRefreshFrames;
      record.sourceFps = stream.sourceFramesPerSecond;
      record.rtpFps = stream.framesPerSecond;
      record.encodedFps = stream.encodedFramesPerSecond;
      record.sentFps = stream.sentFramesPerSecond;
      record.presentedFps = stream.presentedFramesPerSecond;
      record.bitrateBps = stream.bitrateBps;
      records.push_back(std::move(record));
    }
  }
  AsyncScreenFrameRateLogger::Instance().Enqueue(std::move(records));
}

} // namespace remote::controller::detail
