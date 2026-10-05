// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include <array>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "media_intelligence/backends/openai_compatible/OpenAiCompatibleVisionProtocol.h"
#include "media_intelligence/remote/VisionApiRuntime.h"

namespace {

using namespace remote::media_intelligence;

bool Check(bool value, const char* name)
{
    std::cout << name << '=' << (value ? "PASS" : "FAIL") << '\n';
    return value;
}

EncodedImage TestImage(std::uint8_t marker = 0xFF)
{
    auto bytes = std::make_shared<const EncodedImage::Storage>(
        EncodedImage::Storage{marker, 0xD8, 0xFF});
    return EncodedImage(bytes, EncodedImageFormat::kJpeg, 1, 1);
}

RemoteSemanticRequest TestRequest(
    std::uint64_t generation,
    std::uint64_t frameId)
{
    RemoteSemanticRequest request;
    request.generation = generation;
    request.sourceFrameId = frameId;
    request.timestampMs = frameId * 10;
    request.image = TestImage(static_cast<std::uint8_t>(frameId));
    return request;
}

HttpResponse SuccessResponse(std::string scene = "code_terminal")
{
    HttpResponse response;
    response.statusCode = 200;
    response.body =
        "{\"choices\":[{\"message\":{\"content\":\"{\\\"scene\\\":\\\"" +
        scene +
        "\\\",\\\"confidence\\\":0.91}\"}}]}";
    return response;
}

std::string ClassificationResponse(std::string_view classification)
{
    std::string escaped;
    for (const char ch : classification) {
        if (ch == '"' || ch == '\\') {
            escaped.push_back('\\');
        }
        escaped.push_back(ch);
    }
    return "{\"choices\":[{\"message\":{\"content\":\"" + escaped +
        "\"}}]}";
}

class FakeClock final : public IClock {
public:
    [[nodiscard]] std::uint64_t NowMs() const noexcept override
    {
        return nowMs;
    }

    std::uint64_t nowMs = 0;
};

class TestExecutor final : public IExecutor {
public:
    explicit TestExecutor(FakeClock& clock) : clock_(clock) {}

    void Post(std::function<void()> task) override
    {
        if (task) {
            task();
        }
    }

    void PostAfter(
        std::uint32_t delayMs,
        std::function<void()> task) override
    {
        delayed.push_back({clock_.nowMs + delayMs, std::move(task)});
    }

    void RunDue()
    {
        for (;;) {
            auto found = delayed.end();
            for (auto it = delayed.begin(); it != delayed.end(); ++it) {
                if (it->dueMs <= clock_.nowMs &&
                    (found == delayed.end() || it->dueMs < found->dueMs)) {
                    found = it;
                }
            }
            if (found == delayed.end()) {
                return;
            }
            std::function<void()> task = std::move(found->task);
            delayed.erase(found);
            task();
        }
    }

private:
    struct DelayedTask {
        std::uint64_t dueMs = 0;
        std::function<void()> task;
    };

    FakeClock& clock_;
    std::vector<DelayedTask> delayed;
};

class MemorySecrets final : public ISecretProvider {
public:
    bool LoadSecret(
        std::string_view credentialId,
        SecretValue* secret,
        std::string* error) override
    {
        const auto found = values.find(std::string(credentialId));
        if (found == values.end() || !secret) {
            if (error) {
                *error = "credential_not_found";
            }
            return false;
        }
        *secret = SecretValue(found->second);
        return true;
    }

    std::map<std::string, std::string> values;
};

class FakeTransport final : public IHttpTransport {
public:
    struct Pending {
        HttpRequest request;
        Completion completion;
    };

    RequestId Start(
        HttpRequest request,
        HttpAuthorization authorization,
        Completion completion) override
    {
        lastAuthorizationScheme = authorization.scheme;
        lastCredential = std::string(authorization.credential.View());
        const RequestId id = nextId++;
        pending.emplace(
            id,
            Pending{std::move(request), std::move(completion)});
        lastStartedId = id;
        return id;
    }

    void Cancel(RequestId requestId) override
    {
        canceled.push_back(requestId);
        pending.erase(requestId);
    }

    bool Complete(RequestId requestId, HttpResponse response)
    {
        const auto found = pending.find(requestId);
        if (found == pending.end()) {
            return false;
        }
        Completion completion = std::move(found->second.completion);
        pending.erase(found);
        completion(std::move(response));
        return true;
    }

    const HttpRequest* LastRequest() const
    {
        const auto found = pending.find(lastStartedId);
        return found == pending.end() ? nullptr : &found->second.request;
    }

    RequestId nextId = 1;
    RequestId lastStartedId = 0;
    std::map<RequestId, Pending> pending;
    std::vector<RequestId> canceled;
    std::string lastAuthorizationScheme;
    std::string lastCredential;
};

class NullLogger final : public ILogger {
public:
    void Log(VisionLogLevel, std::string_view message) override
    {
        events.emplace_back(message);
    }

    std::vector<std::string> events;
};

}  // namespace

int main()
{
    bool passed = true;
    passed &= Check(NormalizeVisionApiRequestIntervalMs(0.5) == 500 &&
        NormalizeVisionApiRequestIntervalMs(1.0) == 1000 &&
        NormalizeVisionApiRequestIntervalMs(2.0) == 2000 &&
        NormalizeVisionApiRequestIntervalMs(5.0) == 5000,
        "FRACTIONAL_AND_WHOLE_SECONDS_TO_MILLISECONDS");
    passed &= Check(NormalizeVisionApiRequestIntervalMs(0.0) == 500 &&
        NormalizeVisionApiRequestIntervalMs(1000.0) == 60000 &&
        NormalizeVisionApiRequestIntervalMs(std::numeric_limits<double>::quiet_NaN()) ==
            kDefaultVisionApiRequestIntervalMs &&
        NormalizeVisionApiRequestIntervalMs(std::numeric_limits<double>::infinity()) ==
            kDefaultVisionApiRequestIntervalMs,
        "INVALID_INTERVALS_ARE_BOUNDED_WITHOUT_UNSAFE_CONVERSION");
    const auto protocol =
        std::make_shared<OpenAiCompatibleVisionProtocol>();

    VisionApiEndpointConfig endpoint = DeepSeekVisionApiPreset();
    endpoint.credentialId = "deepseek-test";
    passed &= Check(
        endpoint.baseUrl == "https://api.deepseek.com" &&
            endpoint.model == "deepseek-flash" &&
            endpoint.requestJsonObjectResponse &&
            endpoint.disableThinking,
        "DEEPSEEK_PRESET");

    std::string validationError;
    passed &= Check(
        ValidateVisionApiEndpoint(endpoint, &validationError),
        "DEEPSEEK_ENDPOINT_VALID");

    VisionApiEndpointConfig unsafe = endpoint;
    unsafe.baseUrl = "http://example.com";
    passed &= Check(
        !ValidateVisionApiEndpoint(unsafe, &validationError) &&
            validationError == "plain_http_requires_loopback",
        "REJECT_REMOTE_PLAINTEXT_HTTP");
    unsafe.baseUrl = "http://127.0.0.1:11434/v1";
    passed &= Check(
        ValidateVisionApiEndpoint(unsafe, &validationError),
        "ALLOW_LOOPBACK_HTTP");

    const VisionProtocolBuildResult built =
        protocol->BuildRequest(endpoint, TestImage().View());
    passed &= Check(built.success, "BUILD_OPENAI_COMPATIBLE_REQUEST");
    passed &= Check(
        built.request.url ==
            "https://api.deepseek.com/chat/completions",
        "DEEPSEEK_REQUEST_URL");
    passed &= Check(
        built.request.body.find("\"model\":\"deepseek-flash\"") !=
                std::string::npos &&
            built.request.body.find(
                "\"response_format\":{\"type\":\"json_object\"}") !=
                std::string::npos &&
            built.request.body.find(
                "\"thinking\":{\"type\":\"disabled\"}") !=
                std::string::npos &&
            built.request.body.find("data:image/jpeg;base64,/9j/") !=
                std::string::npos &&
            built.request.body.find("api-key") == std::string::npos,
        "REQUEST_BODY_MODEL_IMAGE_NO_SECRET");

    const VisionProtocolParseResult parsed =
        protocol->ParseResponse(endpoint, SuccessResponse().body);
    passed &= Check(
        parsed.success &&
            parsed.classification.scene == ScreenScene::kCodeTerminal &&
            parsed.classification.confidence > 0.90f,
        "PARSE_STRICT_CLASSIFICATION");

    const std::array sceneMappings{
        std::pair{ScreenScene::kCodeTerminal, ScreenSemanticType::kTextUi},
        std::pair{ScreenScene::kDocument, ScreenSemanticType::kTextUi},
        std::pair{ScreenScene::kSpreadsheet, ScreenSemanticType::kTextUi},
        std::pair{ScreenScene::kWebApp, ScreenSemanticType::kMixedUi},
        std::pair{ScreenScene::kPhotoGraphics, ScreenSemanticType::kMixedUi},
        std::pair{ScreenScene::kCadDiagram, ScreenSemanticType::kTextUi},
        std::pair{ScreenScene::kVideo, ScreenSemanticType::kVideo},
        std::pair{ScreenScene::kGame3d, ScreenSemanticType::kVideo},
        std::pair{ScreenScene::kMixed, ScreenSemanticType::kMixedUi},
    };
    for (const auto& [scene, semantic] : sceneMappings) {
        const std::string name = ScreenSceneName(scene);
        const auto fine = protocol->ParseResponse(endpoint,
            ClassificationResponse("{\"scene\":\"" + name +
                "\",\"confidence\":0.92}"));
        passed &= Check(
            ParseScreenScene(name) == scene &&
                SceneToSemanticType(scene) == semantic && fine.success &&
                fine.classification.scene == scene &&
                fine.classification.ResolvedSemantic() == semantic &&
                fine.classification.IsValid(),
            ("PARSE_SCENE_" + name).c_str());
        const auto both = protocol->ParseResponse(endpoint,
            ClassificationResponse("{\"scene\":\"" + name +
                "\",\"semantic\":\"" + ScreenSemanticTypeName(semantic) +
                "\",\"confidence\":1}"));
        passed &= Check(!both.success &&
            both.error == "classification_schema_invalid",
            ("REJECT_REDUNDANT_SEMANTIC_" + name).c_str());
    }
    passed &= Check(
        built.request.body.find("code_terminal") != std::string::npos &&
            built.request.body.find("game_3d") != std::string::npos &&
            built.request.body.find("exactly scene and confidence") !=
                std::string::npos,
        "REQUESTS_FINE_SCENE_CONTRACT");
    for (const char* legacy : {"text_ui", "mixed_ui", "video"}) {
        const auto old = protocol->ParseResponse(endpoint,
            ClassificationResponse(std::string("{\"semantic\":\"") +
                legacy + "\",\"confidence\":0.91}"));
        passed &= Check(!old.success &&
            old.error == "classification_schema_invalid",
            (std::string("REJECT_OLD_CONTRACT_") + legacy).c_str());
    }
    const std::array<std::string_view, 19> invalidClassifications{{
        "{\"scene\":\"unknown\",\"confidence\":0.9}",
        "{\"scene\":\"Code_Terminal\",\"confidence\":0.9}",
        "{\"scene\":\"game_3d \",\"confidence\":0.9}",
        "{\"scene\":\"future_scene\",\"confidence\":0.9}",
        "{\"scene\":\"code_terminal\",\"semantic\":\"video\",\"confidence\":0.9}",
        "{\"scene\":\"game_3d\",\"semantic\":\"text_ui\",\"confidence\":0.9}",
        "{\"scene\":\"document\",\"semantic\":\"unknown\",\"confidence\":0.9}",
        "{\"scene\":3,\"confidence\":0.9}",
        "{\"scene\":\"document\",\"semantic\":null,\"confidence\":0.9}",
        "{\"scene\":\"document\",\"confidence\":\"0.9\"}",
        "{\"scene\":\"document\",\"confidence\":true}",
        "{\"scene\":\"document\",\"confidence\":null}",
        "{\"scene\":\"document\",\"confidence\":-0.001}",
        "{\"scene\":\"document\",\"confidence\":1.001}",
        "{\"scene\":\"document\",\"confidence\":1e999}",
        "{\"scene\":\"document\",\"confidence\":0.9,\"command\":\"run\"}",
        "{\"scene\":\"document\",\"scene\":\"video\",\"confidence\":0.9}",
        "{\"scene\":\"document\"}",
        "{\"confidence\":0.9,\"unused\":\"video\"}",
    }};
    bool rejectedInvalid = true;
    for (const auto content : invalidClassifications) {
        rejectedInvalid &= !protocol->ParseResponse(endpoint,
            ClassificationResponse(content)).success;
    }
    passed &= Check(rejectedInvalid, "REJECT_INVALID_SCENE_CONTRACTS");
    const auto zeroConfidence = protocol->ParseResponse(endpoint,
        ClassificationResponse("{\"scene\":\"document\",\"confidence\":0}"));
    passed &= Check(zeroConfidence.success &&
        zeroConfidence.classification.confidence == 0.0f,
        "ACCEPT_ZERO_CONFIDENCE_WITH_VALID_SCENE");
    const auto fullConfidence = protocol->ParseResponse(endpoint,
        ClassificationResponse("{\"scene\":\"document\",\"confidence\":1}"));
    passed &= Check(fullConfidence.success &&
        fullConfidence.classification.confidence == 1.0f,
        "ACCEPT_FULL_CONFIDENCE_WITH_VALID_SCENE");
    const SemanticClassification sceneOnly{ScreenScene::kDocument, 0.9f};
    const SemanticClassification invalidConfidence{ScreenScene::kDocument,
        std::numeric_limits<float>::quiet_NaN()};
    const SemanticClassification invalidScene{static_cast<ScreenScene>(255), 0.9f};
    passed &= Check(sceneOnly.IsValid() &&
        sceneOnly.ResolvedSemantic() == ScreenSemanticType::kTextUi &&
        !invalidConfidence.IsValid() && !invalidScene.IsValid() &&
        !SemanticClassification{}.IsValid(),
        "FINE_SCENE_CLASSIFICATION_VALIDITY");
    ContentState classified;
    classified.semantic = ScreenSemanticType::kTextUi;
    classified.scene = ScreenScene::kDocument;
    classified.semanticConfidence = 0.9f;
    classified.timestampMs = 100;
    classified.modelResultAvailable = true;
    classified.motion = ScreenMotionLevel::kHigh;
    const auto fresh = ApplyContentStateStaleness(classified, 2100);
    const auto stale = ApplyContentStateStaleness(classified, 2101);
    const auto reversed = ApplyContentStateStaleness(classified, 99);
    passed &= Check(fresh.scene == ScreenScene::kDocument &&
        stale.scene == ScreenScene::kUnknown &&
        stale.semantic == ScreenSemanticType::kUnknown &&
        stale.semanticConfidence == 0.0f && !stale.modelResultAvailable &&
        stale.motion == ScreenMotionLevel::kHigh &&
        reversed.scene == ScreenScene::kUnknown,
        "SCENE_STALENESS_PRESERVES_MOTION");

    const std::string extraField =
        "{\"choices\":[{\"message\":{\"content\":\"{\\\"scene\\\":"
        "\\\"video\\\",\\\"confidence\\\":0.9,\\\"command\\\":\\\"run\\\"}\"}}]}";
    passed &= Check(
        !protocol->ParseResponse(endpoint, extraField).success,
        "REJECT_EXTRA_CONTROL_FIELD");
    passed &= Check(
        !protocol->ParseResponse(
             endpoint,
             "{\"choices\":[{\"message\":{\"content\":\"```json{}\"}}]}")
             .success,
        "REJECT_MARKDOWN_RESPONSE");

    FakeClock clock;
    TestExecutor executor(clock);
    MemorySecrets secrets;
    secrets.values.emplace("deepseek-test", "api-key");
    FakeTransport transport;
    NullLogger logger;

    VisionApiRuntimeConfig runtimeConfig;
    runtimeConfig.endpoint = endpoint;
    runtimeConfig.minimumRequestIntervalMs = 10;
    runtimeConfig.maximumConsecutiveFailures = 3;
    runtimeConfig.circuitBreakDurationMs = 1000;
    VisionApiRuntime runtime(
        runtimeConfig,
        protocol,
        transport,
        secrets,
        clock,
        executor,
        &logger);
    runtime.Reset(7);

    std::vector<RemoteSemanticResult> results;
    auto collect = [&results](RemoteSemanticResult result) {
        results.push_back(std::move(result));
    };

    passed &= Check(
        runtime.Submit(TestRequest(7, 1), collect) ==
            RemoteSubmitStatus::kStarted,
        "RUNTIME_STARTS_FIRST_REQUEST");
    const auto firstId = transport.lastStartedId;
    passed &= Check(
        transport.lastCredential == "api-key" &&
            transport.lastAuthorizationScheme == "Bearer" &&
            transport.LastRequest() &&
            transport.LastRequest()->body.find("api-key") ==
                std::string::npos,
        "SECRET_ONLY_AT_TRANSPORT_BOUNDARY");
    passed &= Check(
        runtime.Submit(TestRequest(7, 2), collect) ==
            RemoteSubmitStatus::kQueued &&
            runtime.Submit(TestRequest(7, 3), collect) ==
                RemoteSubmitStatus::kReplacedPending,
        "RUNTIME_LATEST_PENDING_ONLY");
    passed &= Check(
        !results.empty() &&
            results.back().sourceFrameId == 2 &&
            results.back().status == RemoteClassificationStatus::kCanceled,
        "REPLACED_SAMPLE_COMPLETES_CANCELED");
    passed &= Check(
        transport.Complete(firstId, SuccessResponse("video")),
        "COMPLETE_FIRST_REQUEST");
    passed &= Check(
        results.back().sourceFrameId == 1 &&
            results.back().status == RemoteClassificationStatus::kSuccess &&
            results.back().classification.scene == ScreenScene::kVideo,
        "RUNTIME_RETURNS_CLASSIFICATION");

    clock.nowMs = 10;
    executor.RunDue();
    passed &= Check(
        transport.pending.size() == 1 &&
            transport.lastStartedId != firstId,
        "QUEUED_LATEST_STARTS_AT_INTERVAL");
    passed &= Check(
        runtime.Snapshot().requestInFlight &&
            !runtime.Snapshot().pendingSample,
        "AUTO_START_CONSUMES_PENDING");
    passed &= Check(
        runtime.Submit(TestRequest(7, 4), collect) ==
            RemoteSubmitStatus::kQueued,
        "NEWEST_WAITS_BEHIND_AUTO_STARTED_REQUEST");
    passed &= Check(
        results.back().sourceFrameId == 1 &&
            results.back().status == RemoteClassificationStatus::kSuccess,
        "AUTO_STARTED_SAMPLE_NOT_PREMATURELY_CANCELED");
    const auto resetRequestId = transport.lastStartedId;
    runtime.Reset(8);
    passed &= Check(
        !transport.canceled.empty() &&
            transport.canceled.back() == resetRequestId &&
            results.back().sourceFrameId == 4 &&
            results.back().status == RemoteClassificationStatus::kCanceled,
        "GENERATION_RESET_CANCELS_REQUEST");

    runtime.Reset(9);
    for (std::uint64_t frame = 10; frame < 13; ++frame) {
        clock.nowMs = runtime.Snapshot().nextEligibleAtMs;
        passed &= Check(
            runtime.Submit(TestRequest(9, frame), collect) ==
                RemoteSubmitStatus::kStarted,
            "FAILURE_REQUEST_STARTS");
        HttpResponse failure;
        failure.statusCode = 500;
        passed &= Check(
            transport.Complete(transport.lastStartedId, std::move(failure)),
            "FAILURE_REQUEST_COMPLETES");
    }
    passed &= Check(runtime.Snapshot().circuitOpen, "CIRCUIT_OPENS");
    passed &= Check(
        runtime.Submit(TestRequest(9, 20), collect) ==
                RemoteSubmitStatus::kRejected &&
            results.back().status == RemoteClassificationStatus::kCircuitOpen,
        "CIRCUIT_REJECTS_REQUEST");

    runtime.Stop();
    const VisionApiRuntimeSnapshot stopped = runtime.Snapshot();
    passed &= Check(
        !stopped.running && !stopped.requestInFlight &&
            !stopped.pendingSample,
        "RUNTIME_STOPPED");

    std::cout << "VISION_API_SELF_TEST="
              << (passed ? "PASS" : "FAIL") << '\n';
    return passed ? 0 : 1;
}
