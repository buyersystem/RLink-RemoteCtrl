// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QHostAddress>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTemporaryDir>
#include <QThread>
#include <QTimer>

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include "api/video/i420_buffer.h"
#include "media_intelligence/remote/IHttpTransport.h"
#include "src/apps/remote/adapters/QtVisionApiHttpTransport.h"
#include "src/apps/remote/adapters/VisionApiFrameAnalyzer.h"
#include "src/apps/remote/adapters/WebRtcVisionFrameEncoder.h"
#include "src/platform/win/DpapiContentAnalyzerSecretStore.h"

namespace {

using namespace remote::media_intelligence;

bool Check(bool value, const char* name)
{
    std::cout << name << '=' << (value ? "PASS" : "FAIL") << '\n';
    return value;
}

struct LocalResponse {
    QByteArray body = "{\"ok\":true}";
    QByteArray extraHeaders;
};

class LocalHttpServer final {
public:
    bool Start()
    {
        QObject::connect(
            &server_,
            &QTcpServer::newConnection,
            &server_,
            [this] {
                QTcpSocket* socket = server_.nextPendingConnection();
                if (!socket) {
                    return;
                }
                QObject::connect(
                    socket,
                    &QTcpSocket::readyRead,
                    socket,
                    [this, socket] {
                        request_.append(socket->readAll());
                        const qsizetype headerEnd = request_.indexOf("\r\n\r\n");
                        if (headerEnd < 0 || responded_) {
                            return;
                        }
                        // Larger JPEGs arrive in several reads. Consume the
                        // complete body before responding and closing the socket.
                        qsizetype contentLength = 0;
                        for (const auto& line : request_.left(headerEnd).split('\n')) {
                            const auto header = line.trimmed();
                            if (header.toLower().startsWith("content-length:")) {
                                bool validLength = false;
                                contentLength = header.mid(15).trimmed().toLongLong(
                                    &validLength);
                                if (!validLength || contentLength < 0) {
                                    return;
                                }
                            }
                        }
                        if (request_.size() - (headerEnd + 4) < contentLength) {
                            return;
                        }
                        responded_ = true;
                        const QByteArray response =
                            "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n" +
                            response_.extraHeaders +
                            "Content-Length: " +
                            QByteArray::number(response_.body.size()) +
                            "\r\nConnection: close\r\n\r\n" +
                            response_.body;
                        socket->write(response);
                        socket->disconnectFromHost();
                    });
            });
        return server_.listen(QHostAddress::LocalHost, 0);
    }

    void Prepare(LocalResponse response)
    {
        response_ = std::move(response);
        request_.clear();
        responded_ = false;
    }

    [[nodiscard]] QString Url() const
    {
        return QStringLiteral("http://127.0.0.1:%1/test")
            .arg(server_.serverPort());
    }

    [[nodiscard]] QString BaseUrl() const
    {
        return QStringLiteral("http://127.0.0.1:%1")
            .arg(server_.serverPort());
    }

    [[nodiscard]] const QByteArray& Request() const noexcept
    {
        return request_;
    }

private:
    QTcpServer server_;
    LocalResponse response_;
    QByteArray request_;
    bool responded_ = false;
};

bool WaitFor(
    std::function<void(std::function<void(HttpResponse)>)> begin,
    HttpResponse* output)
{
    QEventLoop loop;
    bool completed = false;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    begin([&](HttpResponse response) {
        *output = std::move(response);
        completed = true;
        loop.quit();
    });
    timeout.start(3000);
    loop.exec();
    return completed;
}

bool WaitUntil(std::function<bool()> predicate, int timeoutMs = 3000)
{
    QElapsedTimer elapsed;
    elapsed.start();
    while (elapsed.elapsed() < timeoutMs) {
        QCoreApplication::processEvents(
            QEventLoop::AllEvents, 25);
        if (predicate()) {
            return true;
        }
        QThread::msleep(5);
    }
    return predicate();
}

}  // namespace

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    bool passed = true;

    QTemporaryDir secretDirectory;
    passed &= Check(secretDirectory.isValid(), "TEMP_SECRET_DIRECTORY");
    remote::DpapiContentAnalyzerSecretStore secrets(
        secretDirectory.path());
    std::string error;
    constexpr std::string_view kCredentialId = "test-credential";
    constexpr std::string_view kSecret = "secret-value-not-plaintext";
    passed &= Check(
        secrets.SaveSecret(kCredentialId, kSecret, &error),
        "DPAPI_SAVE");
    const QStringList files = QDir(secretDirectory.path()).entryList(
        QDir::Files | QDir::NoDotAndDotDot);
    passed &= Check(files.size() == 1, "DPAPI_FILE_CREATED");
    if (files.size() == 1) {
        QFile encryptedFile(
            QDir(secretDirectory.path()).filePath(files.front()));
        passed &= Check(
            encryptedFile.open(QIODevice::ReadOnly) &&
                !encryptedFile.readAll().contains(
                    QByteArray(kSecret.data(),
                               static_cast<qsizetype>(kSecret.size()))),
            "DPAPI_FILE_HAS_NO_PLAINTEXT");
    }
    SecretValue loaded;
    passed &= Check(
        secrets.LoadSecret(kCredentialId, &loaded, &error) &&
            loaded.View() == kSecret,
        "DPAPI_LOAD");
    loaded.Clear();
    passed &= Check(
        !secrets.SaveSecret("../escape", kSecret, &error),
        "DPAPI_REJECT_PATH_TRAVERSAL");
    passed &= Check(
        secrets.DeleteSecret(kCredentialId, &error) &&
            !secrets.HasSecret(kCredentialId),
        "DPAPI_DELETE");

    auto i420 = webrtc::I420Buffer::Create(1280, 720);
    for (int row = 0; row < i420->height(); ++row) {
        std::fill_n(
            i420->MutableDataY() + row * i420->StrideY(),
            i420->width(),
            static_cast<std::uint8_t>(32 + row % 160));
    }
    for (int row = 0; row < (i420->height() + 1) / 2; ++row) {
        std::fill_n(
            i420->MutableDataU() + row * i420->StrideU(),
            (i420->width() + 1) / 2,
            static_cast<std::uint8_t>(110));
        std::fill_n(
            i420->MutableDataV() + row * i420->StrideV(),
            (i420->width() + 1) / 2,
            static_cast<std::uint8_t>(145));
    }
    remote::app::VisionFrameEncodingMetrics encodingMetrics;
    const auto encodedFrame =
        remote::app::EncodeWebRtcFrameAsJpeg(
            i420, 512, 60, &encodingMetrics);
    const auto encodedView = encodedFrame.View();
    passed &= Check(
        encodedView.IsValid() && encodedView.width == 512 &&
            encodedView.height == 288 && encodedView.bytes.size() > 4 &&
            encodingMetrics.scaleConvertTimeUs > 0 &&
            encodingMetrics.jpegEncodeTimeUs > 0 &&
            encodingMetrics.jpegBytes == encodedView.bytes.size() &&
            encodedView.bytes[0] == 0xff &&
            encodedView.bytes[1] == 0xd8,
        "WEBRTC_FRAME_JPEG_ENCODE");

    LocalHttpServer server;
    passed &= Check(server.Start(), "LOOPBACK_SERVER_START");
    remote::app::QtVisionApiHttpTransport transport;

    HttpRequest request;
    request.url = server.Url().toStdString();
    request.headers.push_back({"Content-Type", "application/json"});
    request.body = "{}";
    request.timeoutMs = 2000;
    request.maximumResponseBytes = 1024;
    HttpAuthorization authorization;
    authorization.credential = SecretValue("transport-secret");
    server.Prepare({"{\"result\":\"ok\"}", {}});
    HttpResponse response;
    passed &= Check(
        WaitFor(
            [&](auto completion) {
                transport.Start(
                    request,
                    std::move(authorization),
                    std::move(completion));
            },
            &response),
        "QT_TRANSPORT_COMPLETES");
    passed &= Check(
        response.transportStatus == HttpTransportStatus::kSuccess &&
            response.statusCode == 200 &&
            response.body == "{\"result\":\"ok\"}",
        "QT_TRANSPORT_RESPONSE");
    passed &= Check(
        server.Request().contains("Authorization: Bearer transport-secret") &&
            server.Request().contains("POST /test HTTP/1.1"),
        "QT_TRANSPORT_AUTH_BOUNDARY");

    server.Prepare({QByteArray(2048, 'x'), {}});
    HttpRequest limitedRequest = request;
    limitedRequest.maximumResponseBytes = 64;
    HttpAuthorization limitedAuthorization;
    limitedAuthorization.credential = SecretValue("transport-secret");
    passed &= Check(
        WaitFor(
            [&](auto completion) {
                transport.Start(
                    std::move(limitedRequest),
                    std::move(limitedAuthorization),
                    std::move(completion));
            },
            &response) &&
            response.transportStatus ==
                HttpTransportStatus::kResponseTooLarge,
        "QT_TRANSPORT_RESPONSE_LIMIT");

    HttpAuthorization canceledAuthorization;
    canceledAuthorization.credential = SecretValue("transport-secret");
    passed &= Check(
        WaitFor(
            [&](auto completion) {
                const auto requestId = transport.Start(
                    request,
                    std::move(canceledAuthorization),
                    std::move(completion));
                transport.Cancel(requestId);
            },
            &response) &&
            response.transportStatus == HttpTransportStatus::kCanceled,
        "QT_TRANSPORT_CANCEL");

    constexpr std::string_view kAnalyzerCredentialId =
        "frame-analyzer-credential";
    passed &= Check(
        secrets.SaveSecret(
            kAnalyzerCredentialId, "frame-analyzer-secret", &error),
        "FRAME_ANALYZER_SECRET_SAVE");
    VisionApiRuntimeConfig analyzerConfig;
    analyzerConfig.endpoint.baseUrl = server.BaseUrl().toStdString();
    analyzerConfig.endpoint.model = "loopback-vision";
    analyzerConfig.endpoint.credentialId =
        std::string(kAnalyzerCredentialId);
    analyzerConfig.endpoint.timeoutMs = 2000;
    analyzerConfig.endpoint.maximumResponseBytes = 4096;
    analyzerConfig.minimumRequestIntervalMs = 500;
    bool uploadAllowed = true;
    remote::app::VisionApiFrameAnalyzer::HostOptions hostOptions;
    hostOptions.activationCheck = [&uploadAllowed] {
        return uploadAllowed;
    };
    hostOptions.credentialDirectory = secretDirectory.path();
    // Sampling cadence changes the grace period, without refreshing the actual
    // classification timestamp or issuing any request in this check.
    bool allIntervalsHaveGrace = true;
    for (const auto [interval, expectedAge] : {
        std::pair{500u, 15000ull}, {1000u, 15000ull}, {2000u, 15000ull},
        {5000u, 17000ull}, {10000u, 32000ull}, {20000u, 62000ull},
        {30000u, 92000ull}, {60000u, 182000ull}}) {
        auto ageConfig = analyzerConfig;
        ageConfig.minimumRequestIntervalMs = interval;
        auto ageAnalyzer = remote::app::VisionApiFrameAnalyzer::Create(ageConfig, hostOptions, &error);
        allIntervalsHaveGrace &= ageAnalyzer && ageAnalyzer->MaximumSceneAgeMs() == expectedAge;
    }
    passed &= Check(allIntervalsHaveGrace, "ALL_SAMPLING_INTERVALS_USE_MODEL_RESULT_GRACE");
    auto analyzer = remote::app::VisionApiFrameAnalyzer::Create(
        analyzerConfig, std::move(hostOptions), &error);
    passed &= Check(
        analyzer != nullptr,
        "FRAME_ANALYZER_CREATE");
    if (analyzer) {
        server.Prepare({
            R"({"choices":[{"message":{"content":"{\"scene\":\"code_terminal\",\"confidence\":0.95}"}}]})",
            {}});
        const std::uint64_t sessionToken = analyzer->BeginSession();
        passed &= Check(
            sessionToken != 0 && analyzer->SubmitFrame(
                sessionToken, i420),
            "FRAME_ANALYZER_SUBMIT");
        passed &= Check(!analyzer->SubmitFrame(sessionToken, i420),
            "SUBSECOND_INTERVAL_PREVENTS_BACK_TO_BACK_SAMPLES");
        analyzer->InvalidateResult(sessionToken);
        passed &= Check(!analyzer->SubmitFrame(sessionToken, i420),
            "SCENE_INVALIDATION_DOES_NOT_BYPASS_REQUEST_INTERVAL");
        passed &= Check(
            WaitUntil([&] {
                const auto snapshot = analyzer->Snapshot();
                return snapshot.processedSamples == 1 &&
                    snapshot.latestReturnedClassification.ResolvedSemantic() ==
                        ScreenSemanticType::kTextUi &&
                    snapshot.classification.ResolvedSemantic() ==
                        ScreenSemanticType::kTextUi &&
                    snapshot.classification.scene == ScreenScene::kCodeTerminal &&
                    snapshot.latestReturnedClassification.scene ==
                        ScreenScene::kCodeTerminal && snapshot.latestReturnedAtMs != 0;
            }),
            "FRAME_ANALYZER_END_TO_END");
        passed &= Check(
            server.Request().contains(
                "Authorization: Bearer frame-analyzer-secret") &&
                server.Request().contains(
                    "POST /chat/completions HTTP/1.1"),
            "FRAME_ANALYZER_REQUEST_BOUNDARY");
        const auto submitNext = [&] {
            return WaitUntil([&] {
                return analyzer->SubmitFrame(sessionToken, i420);
            }, 2500);
        };
        server.Prepare({
            R"({"choices":[{"message":{"content":"{\"scene\":\"document\",\"confidence\":0.95}"}}]})",
            {}});
        passed &= Check(submitNext() && WaitUntil([&] {
            const auto snapshot = analyzer->Snapshot();
            return snapshot.processedSamples == 2 &&
                snapshot.latestReturnedClassification.scene == ScreenScene::kDocument &&
                snapshot.classification.scene == ScreenScene::kDocument;
        }), "HIGH_CONFIDENCE_NEW_SCENE_SWITCHES_AFTER_ONE_RESULT");
        server.Prepare({
            R"({"choices":[{"message":{"content":"{\"scene\":\"code_terminal\",\"confidence\":0.75}"}}]})",
            {}});
        passed &= Check(submitNext() && WaitUntil([&] {
            const auto snapshot = analyzer->Snapshot();
            return snapshot.processedSamples == 3 &&
                snapshot.classification.scene == ScreenScene::kDocument;
        }), "MEDIUM_CONFIDENCE_NEW_SCENE_RETAINS_CURRENT_UNTIL_CONFIRMED");
        server.Prepare({
            R"({"choices":[{"message":{"content":"{\"scene\":\"code_terminal\",\"confidence\":0.75}"}}]})",
            {}});
        passed &= Check(submitNext() && WaitUntil([&] {
            const auto snapshot = analyzer->Snapshot();
            return snapshot.processedSamples == 4 &&
                snapshot.classification.scene == ScreenScene::kCodeTerminal;
        }), "SECOND_MEDIUM_CONFIDENCE_RESULT_SWITCHES_SCENE");
        server.Prepare({
            R"({"choices":[{"message":{"content":"{\"scene\":\"game_3d\",\"confidence\":0.5}"}}]})",
            {}});
        passed &= Check(submitNext() && WaitUntil([&] {
            const auto snapshot = analyzer->Snapshot();
            return snapshot.processedSamples == 5 &&
                snapshot.latestReturnedClassification.scene == ScreenScene::kGame3d &&
                snapshot.classification.scene == ScreenScene::kCodeTerminal;
        }), "LOW_CONFIDENCE_DIAGNOSTIC_DOES_NOT_REPLACE_ACCEPTED_SCENE");
        const auto rejectedBefore = analyzer->Snapshot().rejectedSamples;
        server.Prepare({R"({"choices":[{"message":{"content":"invalid classification"}}]})", {}});
        passed &= Check(submitNext() && WaitUntil([&] {
            const auto snapshot = analyzer->Snapshot();
            return snapshot.rejectedSamples > rejectedBefore &&
                snapshot.classification.scene == ScreenScene::kCodeTerminal;
        }), "API_FAILURE_PRESERVES_LAST_CONFIRMED_SCENE");
        analyzer->InvalidateResult(sessionToken);
        passed &= Check(!analyzer->Snapshot().classification.IsValid() &&
            analyzer->Snapshot().latestReturnedClassification.scene == ScreenScene::kGame3d,
            "INVALIDATION_CLEARS_CONTROL_STATE_PRESERVES_RAW_DIAGNOSTIC");
        uploadAllowed = false;
        passed &= Check(
            WaitUntil([&] { return !analyzer->Snapshot().running; }),
            "FRAME_ANALYZER_PRIVACY_SUSPEND");
        passed &= Check(
            !analyzer->SubmitFrame(sessionToken, i420),
            "FRAME_ANALYZER_REJECTS_WHEN_SUSPENDED");
        analyzer->EndSession(sessionToken);
        analyzer.reset();
    }
    passed &= Check(
        secrets.DeleteSecret(kAnalyzerCredentialId, &error),
        "FRAME_ANALYZER_SECRET_DELETE");

    std::cout << "VISION_API_ADAPTERS_SELF_TEST="
              << (passed ? "PASS" : "FAIL") << '\n';
    return passed ? 0 : 1;
}
