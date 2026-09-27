// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <chrono>
#include <cstdint>
#include <mutex>

#include <QImage>
#include <QPoint>
#include <QRect>
#include <QSize>

#include "src/protocol/RemoteCursorProtocol.h"

class QPainter;

namespace remote::controller {

// Thread-safe cursor state shared by the Qt paint path and the dedicated
// D3D11 presentation thread. It intentionally is not a QWidget: a translucent
// native cursor window cannot reliably compose above a child swap chain.
class RemoteCursorRenderState final {
public:
    struct Snapshot {
        QImage image;
        QPoint hotspot;
        QPoint normalizedPosition;
        std::uint64_t shapeRevision = 0;
        bool visible = false;
    };

    RemoteCursorRenderState();

    void SetShape(const RemoteCursorShape& shape);
    void SetPosition(const RemoteCursorPosition& position, bool predicted);
    void ApplyRemotePosition(const RemoteCursorPosition& position);
    void Reset();
    void SetRenderingEnabled(bool enabled);
    Snapshot GetSnapshot() const;
    void Paint(QPainter& painter,
               const QRect& content,
               const QSize& sourceSize) const;

private:
    static QRect TargetRect(const Snapshot& snapshot,
                            const QRect& content,
                            const QSize& sourceSize);
    void BuildFallbackArrow();

    mutable std::mutex mutex_;
    QImage image_;
    QPoint hotspot_;
    QPoint normalizedPosition_;
    std::chrono::steady_clock::time_point localPredictionStartedAt_{};
    std::uint64_t shapeRevision_ = 0;
    bool hasLocalPrediction_ = false;
    bool visible_ = false;
    bool renderingEnabled_ = false;
};

}  // namespace remote::controller
