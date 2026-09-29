// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <functional>

#include <QObject>
#include <QList>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

namespace remote::update {

class SoftwareUpdateController final : public QObject {
public:
    enum class State {
        kIdle,
        kChecking,
        kUpToDate,
        kUpdateAvailable,
        kFailed,
        kLaunchingUpdater,
    };

    struct Snapshot {
        State state = State::kIdle;
        QString installedVersion;
        QString latestVersion;
        QString minimumSupportedVersion;
        QString summary;
        QStringList releaseNotes;
        QString errorMessage;
        QUrl releaseNotesUrl;
        bool manualRequest = false;
        bool mandatory = false;
    };

    using StateChangedCallback = std::function<void(const Snapshot&)>;

    explicit SoftwareUpdateController(QObject* parent = nullptr);

    void SetStateChangedCallback(StateChangedCallback callback);
    void ScheduleAutomaticCheck();
    void CheckForUpdates(bool manualRequest);
    bool LaunchUpdater(QString* errorMessage = nullptr);

    const Snapshot& CurrentSnapshot() const { return snapshot_; }

private:
    void PublishSnapshot();
    void StartNextManifestRequest();
    void FinishCheck(QNetworkReply* reply);
    void FailCurrentManifest(const QString& message);
    void SetFailure(const QString& message, bool manualRequest);

    QNetworkAccessManager* network_ = nullptr;
    QPointer<QNetworkReply> activeReply_;
    QByteArray responseBody_;
    Snapshot snapshot_;
    StateChangedCallback stateChangedCallback_;
    QList<QUrl> manifestUrls_;
    QList<QUrl> packageUrls_;
    QUrl activeManifestUrl_;
    qsizetype nextManifestIndex_ = 0;
    QStringList manifestErrors_;
    QString packageSha256_;
    qint64 packageSize_ = -1;
};

}  // namespace remote::update
