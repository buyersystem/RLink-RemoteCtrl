// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "src/apps/remote/adapters/QtVisionApiExecutor.h"

#include <QMetaObject>
#include <QTimer>

#include <algorithm>
#include <limits>
#include <memory>
#include <utility>

namespace remote::app {

QtVisionApiExecutor::QtVisionApiExecutor(QObject* parent)
    : QObject(parent)
{
}

void QtVisionApiExecutor::Post(std::function<void()> task)
{
    if (!task) {
        return;
    }
    auto sharedTask =
        std::make_shared<std::function<void()>>(std::move(task));
    QMetaObject::invokeMethod(
        this,
        [sharedTask] {
            if (*sharedTask) {
                (*sharedTask)();
            }
        },
        Qt::QueuedConnection);
}

void QtVisionApiExecutor::PostAfter(
    std::uint32_t delayMs,
    std::function<void()> task)
{
    if (!task) {
        return;
    }
    auto sharedTask =
        std::make_shared<std::function<void()>>(std::move(task));
    const int boundedDelay = static_cast<int>(std::min<std::uint32_t>(
        delayMs,
        static_cast<std::uint32_t>(std::numeric_limits<int>::max())));
    QMetaObject::invokeMethod(
        this,
        [this, boundedDelay, sharedTask] {
            QTimer::singleShot(
                boundedDelay,
                this,
                [sharedTask] {
                    if (*sharedTask) {
                        (*sharedTask)();
                    }
                });
        },
        Qt::QueuedConnection);
}

}  // namespace remote::app
