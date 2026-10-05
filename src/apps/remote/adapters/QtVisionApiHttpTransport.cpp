// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "src/apps/remote/adapters/QtVisionApiHttpTransport.h"

#include <QByteArray>
#include <QHash>
#include <QMetaObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QUrl>

#include <algorithm>
#include <limits>
#include <string>
#include <utility>

namespace remote::app {
namespace {

using media_intelligence::HttpAuthorization;
using media_intelligence::HttpRequest;
using media_intelligence::HttpResponse;
using media_intelligence::HttpTransportStatus;
using RequestId = media_intelligence::IHttpTransport::RequestId;
using Completion = media_intelligence::IHttpTransport::Completion;

void SecureClear(QByteArray* bytes)
{
    if (bytes && !bytes->isEmpty()) {
        std::fill(bytes->begin(), bytes->end(), '\0');
        bytes->clear();
    }
}

void SecureClear(std::string* value)
{
    if (!value || value->empty()) {
        return;
    }
    volatile char* data = value->data();
    for (std::size_t index = 0; index < value->size(); ++index) {
        data[index] = '\0';
    }
    value->clear();
}

struct PendingStart {
    HttpRequest request;
    HttpAuthorization authorization;
    Completion completion;
};

struct ActiveRequest {
    QPointer<QNetworkReply> reply;
    Completion completion;
    QByteArray response;
    qsizetype maximumResponseBytes = 0;
    bool responseTooLarge = false;
    bool canceled = false;
};

void ReadAvailable(ActiveRequest* state)
{
    if (!state || !state->reply || state->responseTooLarge) {
        return;
    }
    const qsizetype remaining =
        state->maximumResponseBytes - state->response.size();
    if (remaining < 0) {
        state->responseTooLarge = true;
        state->reply->abort();
        return;
    }
    const QByteArray chunk = state->reply->read(remaining + 1);
    if (chunk.size() > remaining) {
        state->responseTooLarge = true;
        state->reply->abort();
        return;
    }
    state->response.append(chunk);
}

std::uint32_t ParseRetryAfterMs(const QByteArray& value)
{
    bool ok = false;
    const qulonglong seconds = value.trimmed().toULongLong(&ok);
    if (!ok) {
        return 0;
    }
    const qulonglong milliseconds = seconds * 1000ull;
    return static_cast<std::uint32_t>(std::min<qulonglong>(
        milliseconds,
        std::numeric_limits<std::uint32_t>::max()));
}

}  // namespace

struct QtVisionApiHttpTransport::Impl {
    explicit Impl(QObject* owner) : manager(owner) {}

    QNetworkAccessManager manager;
    QHash<RequestId, std::shared_ptr<ActiveRequest>> active;
};

QtVisionApiHttpTransport::QtVisionApiHttpTransport(QObject* parent)
    : QObject(parent),
      impl_(std::make_unique<Impl>(this))
{
}

QtVisionApiHttpTransport::~QtVisionApiHttpTransport()
{
    for (const auto& state : std::as_const(impl_->active)) {
        if (state && state->reply) {
            state->reply->abort();
        }
    }
    impl_->active.clear();
}

QtVisionApiHttpTransport::RequestId QtVisionApiHttpTransport::Start(
    HttpRequest request,
    HttpAuthorization authorization,
    Completion completion)
{
    if (!completion) {
        return 0;
    }
    const RequestId requestId =
        nextRequestId_.fetch_add(1, std::memory_order_relaxed);
    if (requestId == 0) {
        return 0;
    }
    auto pending = std::make_shared<PendingStart>();
    pending->request = std::move(request);
    pending->authorization = std::move(authorization);
    pending->completion = std::move(completion);
    {
        std::lock_guard lock(canceledMutex_);
        pendingStartIds_.insert(requestId);
    }

    QMetaObject::invokeMethod(
        this,
        [this, requestId, pending] {
            {
                std::lock_guard lock(canceledMutex_);
                pendingStartIds_.erase(requestId);
                if (canceledBeforeStart_.erase(requestId) != 0) {
                    HttpResponse response;
                    response.transportStatus = HttpTransportStatus::kCanceled;
                    pending->completion(std::move(response));
                    return;
                }
            }

            const QUrl url(QString::fromStdString(pending->request.url));
            if (!url.isValid() || url.scheme().isEmpty() || url.host().isEmpty()) {
                HttpResponse response;
                response.transportStatus = HttpTransportStatus::kNetworkError;
                response.error = "request_url_invalid";
                pending->completion(std::move(response));
                return;
            }

            QNetworkRequest networkRequest(url);
            networkRequest.setAttribute(
                QNetworkRequest::RedirectPolicyAttribute,
                QNetworkRequest::ManualRedirectPolicy);
            networkRequest.setTransferTimeout(
                static_cast<int>(std::min<std::uint32_t>(
                    pending->request.timeoutMs,
                    static_cast<std::uint32_t>(
                        std::numeric_limits<int>::max()))));
            for (const auto& header : pending->request.headers) {
                networkRequest.setRawHeader(
                    QByteArray::fromStdString(header.name),
                    QByteArray::fromStdString(header.value));
            }

            QByteArray authorization =
                QByteArray::fromStdString(pending->authorization.scheme);
            authorization.append(' ');
            authorization.append(
                pending->authorization.credential.View().data(),
                static_cast<qsizetype>(
                    pending->authorization.credential.View().size()));
            networkRequest.setRawHeader("Authorization", authorization);
            pending->authorization.credential.Clear();
            SecureClear(&authorization);

            const QByteArray method =
                QByteArray::fromStdString(pending->request.method);
            QByteArray body =
                QByteArray::fromStdString(pending->request.body);
            SecureClear(&pending->request.body);
            QNetworkReply* reply = impl_->manager.sendCustomRequest(
                networkRequest,
                method,
                body);
            SecureClear(&body);
            if (!reply) {
                HttpResponse response;
                response.transportStatus = HttpTransportStatus::kNetworkError;
                response.error = "network_manager_rejected_request";
                pending->completion(std::move(response));
                return;
            }

            auto active = std::make_shared<ActiveRequest>();
            active->reply = reply;
            active->completion = std::move(pending->completion);
            active->maximumResponseBytes =
                static_cast<qsizetype>(pending->request.maximumResponseBytes);
            impl_->active.insert(requestId, active);

            connect(reply, &QNetworkReply::readyRead, this,
                    [active] { ReadAvailable(active.get()); });
            connect(reply, &QNetworkReply::finished, this,
                    [this, requestId, active] {
                ReadAvailable(active.get());
                HttpResponse response;
                if (active->canceled) {
                    response.transportStatus = HttpTransportStatus::kCanceled;
                } else if (active->responseTooLarge) {
                    response.transportStatus =
                        HttpTransportStatus::kResponseTooLarge;
                } else if (active->reply &&
                           active->reply->error() ==
                               QNetworkReply::TimeoutError) {
                    response.transportStatus = HttpTransportStatus::kTimeout;
                } else if (active->reply &&
                           active->reply->error() !=
                               QNetworkReply::NoError) {
                    response.transportStatus =
                        HttpTransportStatus::kNetworkError;
                    response.error = "qt_network_error_" +
                        std::to_string(
                            static_cast<int>(active->reply->error()));
                } else {
                    response.transportStatus = HttpTransportStatus::kSuccess;
                }
                if (active->reply) {
                    response.statusCode = active->reply->attribute(
                        QNetworkRequest::HttpStatusCodeAttribute).toUInt();
                    response.retryAfterMs = ParseRetryAfterMs(
                        active->reply->rawHeader("Retry-After"));
                }
                response.body.assign(
                    active->response.constData(),
                    static_cast<std::size_t>(active->response.size()));
                impl_->active.remove(requestId);
                if (active->reply) {
                    active->reply->deleteLater();
                }
                Completion completion = std::move(active->completion);
                if (completion) {
                    completion(std::move(response));
                }
            });
        },
        Qt::QueuedConnection);
    return requestId;
}

void QtVisionApiHttpTransport::Cancel(RequestId requestId)
{
    if (requestId == 0) {
        return;
    }
    {
        std::lock_guard lock(canceledMutex_);
        if (pendingStartIds_.contains(requestId)) {
            canceledBeforeStart_.insert(requestId);
        }
    }
    QMetaObject::invokeMethod(
        this,
        [this, requestId] {
            const auto found = impl_->active.find(requestId);
            if (found == impl_->active.end()) {
                return;
            }
            {
                std::lock_guard lock(canceledMutex_);
                canceledBeforeStart_.erase(requestId);
            }
            const auto& active = found.value();
            if (active) {
                active->canceled = true;
                if (active->reply) {
                    active->reply->abort();
                }
            }
        },
        Qt::QueuedConnection);
}

}  // namespace remote::app
