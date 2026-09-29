// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "SoftwareUpdateController.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <shellapi.h>
#endif

namespace remote::update {
namespace {

constexpr auto kCnbStableManifestUrl =
    "https://cnb.cool/dyh-Rlink/RLink-RemoteCtrl/"
    "-/releases/latest/download/RLink-update.json";
constexpr auto kGithubStableManifestUrl =
    "https://github.com/dyhwdnmd/RLink-RemoteCtrl/"
    "releases/latest/download/RLink-update.json";
constexpr auto kLastAutomaticCheckSetting =
    "updates/lastAutomaticCheckUtc";
constexpr qint64 kAutomaticCheckIntervalSeconds = 24 * 60 * 60;
constexpr qint64 kMaximumManifestBytes = 256 * 1024;
constexpr qsizetype kMaximumManifestUrls = 4;
constexpr qsizetype kMaximumPackageUrls = 8;

bool IsCleanHttpsUrl(const QUrl& url)
{
    if (!url.isValid() || url.scheme().compare(
            QStringLiteral("https"), Qt::CaseInsensitive) != 0) {
        return false;
    }
    return url.userInfo().isEmpty() && url.fragment().isEmpty();
}

bool IsAllowedRepositoryUrl(const QUrl& url)
{
    if (!IsCleanHttpsUrl(url)) {
        return false;
    }
    const QString host = url.host().toLower();
    const QString path = url.path().toLower();
    return (host == QStringLiteral("github.com") &&
            path.startsWith(QStringLiteral(
                "/dyhwdnmd/rlink-remotectrl/"))) ||
        (host == QStringLiteral("cnb.cool") &&
         path.startsWith(QStringLiteral(
             "/dyh-rlink/rlink-remotectrl/")));
}

bool IsAllowedManifestUrl(const QUrl& url)
{
    return IsAllowedRepositoryUrl(url) &&
        url.query().isEmpty() &&
        url.fileName().compare(QStringLiteral("RLink-update.json"),
                               Qt::CaseInsensitive) == 0;
}

bool IsAllowedManifestResponseUrl(const QUrl& url)
{
    // Every request starts from a repository URL checked above. Both GitHub
    // and CNB may redirect release assets to a short-lived HTTPS CDN URL.
    return IsCleanHttpsUrl(url);
}

bool IsAllowedPackageUrl(const QUrl& url)
{
    return IsAllowedRepositoryUrl(url);
}

void AppendUniqueUrl(QList<QUrl>* urls, const QUrl& url)
{
    for (const QUrl& existing : *urls) {
        if (existing.matches(url, QUrl::NormalizePathSegments |
                                  QUrl::StripTrailingSlash)) {
            return;
        }
    }
    urls->push_back(url);
}

QList<QUrl> LoadManifestUrls()
{
    QList<QUrl> urls;
    const QString configurationPath = QDir(
        QCoreApplication::applicationDirPath())
        .filePath(QStringLiteral("RemoteC.bootstrap.json"));
    QFile file(configurationPath);
    if (file.open(QIODevice::ReadOnly) && file.size() <= 64 * 1024) {
        QJsonParseError parseError;
        const QJsonDocument document =
            QJsonDocument::fromJson(file.readAll(), &parseError);
        if (parseError.error == QJsonParseError::NoError &&
            document.isObject()) {
            const QJsonArray configured = document.object()
                .value(QStringLiteral("updateManifestUrls")).toArray();
            for (const QJsonValue& value : configured) {
                if (urls.size() >= kMaximumManifestUrls - 2) {
                    break;
                }
                const QUrl url(value.toString().trimmed());
                if (IsAllowedManifestUrl(url)) {
                    AppendUniqueUrl(&urls, url);
                }
            }
        }
    }
    AppendUniqueUrl(
        &urls, QUrl(QString::fromLatin1(kCnbStableManifestUrl)));
    AppendUniqueUrl(
        &urls, QUrl(QString::fromLatin1(kGithubStableManifestUrl)));
    return urls;
}

QVector<int> ParseVersion(const QString& version)
{
    static const QRegularExpression pattern(
        QStringLiteral("^([0-9]+)\\.([0-9]+)\\.([0-9]+)$"));
    const auto match = pattern.match(version.trimmed());
    if (!match.hasMatch()) {
        return {};
    }
    QVector<int> result;
    result.reserve(3);
    for (int index = 1; index <= 3; ++index) {
        bool ok = false;
        const qlonglong value = match.captured(index).toLongLong(&ok);
        if (!ok || value < 0 || value > 999999) {
            return {};
        }
        result.push_back(static_cast<int>(value));
    }
    return result;
}

int CompareVersions(const QString& left, const QString& right)
{
    const QVector<int> leftParts = ParseVersion(left);
    const QVector<int> rightParts = ParseVersion(right);
    if (leftParts.size() != 3 || rightParts.size() != 3) {
        return 0;
    }
    for (int index = 0; index < 3; ++index) {
        if (leftParts.at(index) != rightParts.at(index)) {
            return leftParts.at(index) < rightParts.at(index) ? -1 : 1;
        }
    }
    return 0;
}

QString NormalizedSha256(const QString& value)
{
    const QString normalized = value.trimmed().toLower();
    static const QRegularExpression pattern(
        QStringLiteral("^[0-9a-f]{64}$"));
    return pattern.match(normalized).hasMatch() ? normalized : QString();
}

#ifdef Q_OS_WIN
QString QuoteWindowsArgument(const QString& value)
{
    if (!value.contains(QLatin1Char(' ')) &&
        !value.contains(QLatin1Char('\t')) &&
        !value.contains(QLatin1Char('"'))) {
        return value;
    }
    QString result = QStringLiteral("\"");
    qsizetype backslashCount = 0;
    for (const QChar character : value) {
        if (character == QLatin1Char('\\')) {
            ++backslashCount;
            continue;
        }
        if (character == QLatin1Char('"')) {
            result += QString(backslashCount * 2 + 1, QLatin1Char('\\'));
            result += character;
            backslashCount = 0;
            continue;
        }
        result += QString(backslashCount, QLatin1Char('\\'));
        backslashCount = 0;
        result += character;
    }
    result += QString(backslashCount * 2, QLatin1Char('\\'));
    result += QLatin1Char('"');
    return result;
}
#endif

}  // namespace

SoftwareUpdateController::SoftwareUpdateController(QObject* parent)
    : QObject(parent),
      network_(new QNetworkAccessManager(this)),
      manifestUrls_(LoadManifestUrls())
{
    snapshot_.installedVersion =
        QCoreApplication::applicationVersion().trimmed();
}

void SoftwareUpdateController::SetStateChangedCallback(
    StateChangedCallback callback)
{
    stateChangedCallback_ = std::move(callback);
    PublishSnapshot();
}

void SoftwareUpdateController::ScheduleAutomaticCheck()
{
    if (snapshot_.installedVersion.isEmpty()) {
        return;
    }
    // Every launch must see a newly raised minimumSupportedVersion.  The
    // request is delayed so it never competes with the interactive startup.
    QTimer::singleShot(10000, this, [this] {
        if (snapshot_.state == State::kIdle ||
            snapshot_.state == State::kUpToDate ||
            snapshot_.state == State::kFailed) {
            CheckForUpdates(false);
        }
    });
}

void SoftwareUpdateController::CheckForUpdates(bool manualRequest)
{
    if (activeReply_) {
        if (manualRequest) {
            snapshot_.manualRequest = true;
            PublishSnapshot();
        }
        return;
    }
    if (snapshot_.installedVersion.isEmpty()) {
        SetFailure(QStringLiteral(
            "未检测到安装版本。请使用 RLink 安装程序安装后再检查更新。"),
            manualRequest);
        return;
    }
    if (ParseVersion(snapshot_.installedVersion).size() != 3) {
        SetFailure(QStringLiteral("当前安装版本号格式无效，无法比较更新。"),
                   manualRequest);
        return;
    }

    packageUrls_.clear();
    packageSha256_.clear();
    packageSize_ = -1;
    responseBody_.clear();
    manifestErrors_.clear();
    nextManifestIndex_ = 0;
    activeManifestUrl_ = {};
    snapshot_.state = State::kChecking;
    snapshot_.manualRequest = manualRequest;
    snapshot_.mandatory = false;
    snapshot_.minimumSupportedVersion.clear();
    snapshot_.releaseNotes.clear();
    snapshot_.errorMessage.clear();
    snapshot_.summary = QStringLiteral("正在检查更新…");
    PublishSnapshot();

    StartNextManifestRequest();
}

void SoftwareUpdateController::StartNextManifestRequest()
{
    if (nextManifestIndex_ >= manifestUrls_.size()) {
        const QString detail = manifestErrors_.isEmpty()
            ? QStringLiteral("没有可用的更新清单地址。")
            : manifestErrors_.join(QStringLiteral("；"));
        SetFailure(QStringLiteral("检查更新失败：%1").arg(detail),
                   snapshot_.manualRequest);
        return;
    }
    responseBody_.clear();
    activeManifestUrl_ = manifestUrls_.at(nextManifestIndex_++);
    QNetworkRequest request(activeManifestUrl_);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("RLink/%1 Windows")
                          .arg(snapshot_.installedVersion));
    request.setAttribute(
        QNetworkRequest::RedirectPolicyAttribute,
        QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setTransferTimeout(15000);
    QNetworkReply* reply = network_->get(request);
    activeReply_ = reply;
    connect(reply, &QNetworkReply::readyRead, this, [this, reply] {
        if (reply != activeReply_) {
            return;
        }
        responseBody_.append(reply->readAll());
        if (responseBody_.size() > kMaximumManifestBytes) {
            reply->abort();
        }
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        FinishCheck(reply);
    });
}

void SoftwareUpdateController::FailCurrentManifest(const QString& message)
{
    manifestErrors_.push_back(QStringLiteral("%1：%2")
        .arg(activeManifestUrl_.host(), message));
    StartNextManifestRequest();
}

void SoftwareUpdateController::FinishCheck(QNetworkReply* reply)
{
    if (!reply || reply != activeReply_) {
        if (reply) {
            reply->deleteLater();
        }
        return;
    }
    activeReply_ = nullptr;
    responseBody_.append(reply->readAll());
    const QUrl finalUrl = reply->url();
    const auto error = reply->error();
    const int httpStatus = reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QString networkError = reply->errorString();
    reply->deleteLater();

    if (responseBody_.size() > kMaximumManifestBytes) {
        FailCurrentManifest(QStringLiteral("更新清单过大"));
        return;
    }
    if (httpStatus == 404) {
        FailCurrentManifest(QStringLiteral("未找到更新清单"));
        return;
    }
    if (error != QNetworkReply::NoError) {
        FailCurrentManifest(networkError);
        return;
    }
    if (!IsAllowedManifestResponseUrl(finalUrl)) {
        FailCurrentManifest(QStringLiteral("被重定向到不受信任的地址"));
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument document =
        QJsonDocument::fromJson(responseBody_, &parseError);
    if (parseError.error != QJsonParseError::NoError ||
        !document.isObject()) {
        FailCurrentManifest(QStringLiteral("更新清单格式无效"));
        return;
    }
    const QJsonObject root = document.object();
    const QJsonObject package = root.value(QStringLiteral("package")).toObject();
    const QString latestVersion =
        root.value(QStringLiteral("version")).toString().trimmed();
    const QString minimumSupportedVersion = root
        .value(QStringLiteral("minimumSupportedVersion"))
        .toString().trimmed();
    QList<QUrl> packageUrls;
    const QJsonArray configuredPackageUrls =
        package.value(QStringLiteral("urls")).toArray();
    if (configuredPackageUrls.size() > kMaximumPackageUrls) {
        packageUrls.clear();
    }
    for (const QJsonValue& value : configuredPackageUrls) {
        if (configuredPackageUrls.size() > kMaximumPackageUrls) {
            break;
        }
        const QUrl url(value.toString().trimmed());
        if (!IsAllowedPackageUrl(url)) {
            packageUrls.clear();
            break;
        }
        AppendUniqueUrl(&packageUrls, url);
    }
    const QUrl legacyPackageUrl(
        package.value(QStringLiteral("url")).toString().trimmed());
    if (IsAllowedPackageUrl(legacyPackageUrl)) {
        AppendUniqueUrl(&packageUrls, legacyPackageUrl);
    }
    const QString packageSha256 = NormalizedSha256(
        package.value(QStringLiteral("sha256")).toString());
    const qint64 packageSize =
        package.value(QStringLiteral("size")).toVariant().toLongLong();

    if (root.value(QStringLiteral("schemaVersion")).toInt() != 1 ||
        root.value(QStringLiteral("product")).toString() !=
            QStringLiteral("RLink") ||
        root.value(QStringLiteral("channel")).toString() !=
            QStringLiteral("stable") ||
        ParseVersion(latestVersion).size() != 3 ||
        (!minimumSupportedVersion.isEmpty() &&
         ParseVersion(minimumSupportedVersion).size() != 3) ||
        packageUrls.isEmpty() ||
        packageSha256.isEmpty() || packageSize <= 0) {
        FailCurrentManifest(
            QStringLiteral("缺少有效的版本或安装包信息"));
        return;
    }

    QSettings settings;
    settings.setValue(QString::fromLatin1(kLastAutomaticCheckSetting),
                      QDateTime::currentDateTimeUtc());
    snapshot_.latestVersion = latestVersion;
    snapshot_.minimumSupportedVersion = minimumSupportedVersion;
    snapshot_.releaseNotes.clear();
    const QJsonArray releaseNotes =
        root.value(QStringLiteral("releaseNotes")).toArray();
    for (const QJsonValue& note : releaseNotes) {
        const QString text = note.toString().trimmed();
        if (!text.isEmpty() && snapshot_.releaseNotes.size() < 12) {
            snapshot_.releaseNotes.push_back(text.left(300));
        }
    }
    snapshot_.releaseNotesUrl = QUrl(
        root.value(QStringLiteral("releaseNotesUrl")).toString());
    snapshot_.errorMessage.clear();
    packageUrls_ = packageUrls;
    packageSha256_ = packageSha256;
    packageSize_ = packageSize;

    if (CompareVersions(snapshot_.installedVersion, latestVersion) < 0) {
        snapshot_.state = State::kUpdateAvailable;
        snapshot_.mandatory = !minimumSupportedVersion.isEmpty() &&
            CompareVersions(snapshot_.installedVersion,
                            minimumSupportedVersion) < 0;
        snapshot_.summary = root.value(QStringLiteral("summary"))
            .toString(QStringLiteral("新版本已准备好。"))
            .trimmed();
    } else {
        snapshot_.state = State::kUpToDate;
        snapshot_.summary = QStringLiteral("当前已是最新版本");
    }
    PublishSnapshot();
}

bool SoftwareUpdateController::LaunchUpdater(QString* errorMessage)
{
    if (snapshot_.state != State::kUpdateAvailable ||
        packageUrls_.isEmpty() || packageSha256_.isEmpty() ||
        packageSize_ <= 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("当前没有可安装的更新。" );
        }
        return false;
    }

    const QString applicationDirectory =
        QCoreApplication::applicationDirPath();
    const QString updaterSource =
        QDir(applicationDirectory).filePath(QStringLiteral("RLinkUpdater.exe"));
    if (!QFileInfo::exists(updaterSource)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral(
                "未找到 RLinkUpdater.exe，请重新安装当前版本后再试。" );
        }
        return false;
    }

    const QString updateRoot = QDir(
        QStandardPaths::writableLocation(QStandardPaths::TempLocation))
        .filePath(QStringLiteral("RLinkUpdate/%1-%2")
            .arg(snapshot_.latestVersion)
            .arg(QCoreApplication::applicationPid()));
    QDir directory;
    if (!directory.mkpath(updateRoot)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("无法创建更新临时目录。" );
        }
        return false;
    }
    const QString updaterCopy =
        QDir(updateRoot).filePath(QStringLiteral("RLinkUpdater.exe"));
    QFile::remove(updaterCopy);
    if (!QFile::copy(updaterSource, updaterCopy)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("无法准备独立更新程序。" );
        }
        return false;
    }

    QStringList arguments = {
        QStringLiteral("--manifest-url"), activeManifestUrl_.toString(),
        QStringLiteral("--package-sha256"), packageSha256_,
        QStringLiteral("--package-size"), QString::number(packageSize_),
        QStringLiteral("--target-dir"), applicationDirectory,
        QStringLiteral("--restart-exe"), QStringLiteral("RLinkAPP.exe"),
        QStringLiteral("--current-version"), snapshot_.installedVersion,
        QStringLiteral("--target-version"), snapshot_.latestVersion,
        QStringLiteral("--parent-pid"),
            QString::number(QCoreApplication::applicationPid())};
    for (const QUrl& packageUrl : packageUrls_) {
        arguments.push_back(QStringLiteral("--package-url"));
        arguments.push_back(packageUrl.toString());
    }
#ifdef Q_OS_WIN
    QStringList quotedArguments;
    quotedArguments.reserve(arguments.size());
    for (const QString& argument : arguments) {
        quotedArguments.push_back(QuoteWindowsArgument(argument));
    }
    const std::wstring nativeUpdater = updaterCopy.toStdWString();
    const std::wstring nativeArguments =
        quotedArguments.join(QLatin1Char(' ')).toStdWString();
    const std::wstring nativeWorkingDirectory = updateRoot.toStdWString();
    SHELLEXECUTEINFOW execute{};
    execute.cbSize = sizeof(execute);
    execute.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC;
    execute.hwnd = nullptr;
    execute.lpVerb = L"runas";
    execute.lpFile = nativeUpdater.c_str();
    execute.lpParameters = nativeArguments.c_str();
    execute.lpDirectory = nativeWorkingDirectory.c_str();
    execute.nShow = SW_SHOWNORMAL;
    const bool launched = ShellExecuteExW(&execute) != FALSE;
    if (execute.hProcess) {
        CloseHandle(execute.hProcess);
    }
#else
    const bool launched = QProcess::startDetached(
        updaterCopy, arguments, updateRoot);
#endif
    if (!launched) {
        if (errorMessage) {
            *errorMessage = QStringLiteral(
                "无法启动独立更新程序，可能未授予管理员权限。" );
        }
        return false;
    }

    snapshot_.state = State::kLaunchingUpdater;
    snapshot_.summary = QStringLiteral("正在启动安装程序…");
    PublishSnapshot();
    return true;
}

void SoftwareUpdateController::PublishSnapshot()
{
    if (stateChangedCallback_) {
        stateChangedCallback_(snapshot_);
    }
}

void SoftwareUpdateController::SetFailure(
    const QString& message, bool manualRequest)
{
    snapshot_.state = State::kFailed;
    snapshot_.manualRequest = manualRequest;
    snapshot_.errorMessage = message;
    snapshot_.summary = message;
    PublishSnapshot();
}

}  // namespace remote::update
