// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "media_intelligence/core/SemanticClassification.h"

namespace remote::media_intelligence {

inline constexpr std::uint32_t kMinimumVisionApiRequestIntervalMs = 500;
inline constexpr std::uint32_t kMaximumVisionApiRequestIntervalMs = 60000;
inline constexpr std::uint32_t kDefaultVisionApiRequestIntervalMs = 1000;
inline constexpr std::uint32_t kDefaultVisionApiMaximumImageDimension = 1280;

// Settings are stored in seconds, including fractional values such as 0.5.
// Validate before converting to integral milliseconds.
[[nodiscard]] std::uint32_t NormalizeVisionApiRequestIntervalMs(
    double seconds) noexcept;

enum class VisionApiProtocolKind : std::uint8_t {
    kOpenAiChatCompletions,
};

enum class VisionImageDetail : std::uint8_t {
    kLow,
    kAuto,
    kOriginal,
};

struct VisionApiEndpointConfig {
    std::string providerId = "openai_compatible";
    std::string baseUrl;
    std::string chatCompletionsPath = "/chat/completions";
    std::string model;
    std::string credentialId;
    VisionApiProtocolKind protocol =
        VisionApiProtocolKind::kOpenAiChatCompletions;
    VisionImageDetail imageDetail = VisionImageDetail::kLow;
    bool requestJsonObjectResponse = false;
    bool disableThinking = false;
    std::uint32_t timeoutMs = 8000;
    std::uint32_t maximumResponseBytes = 16 * 1024;
};

struct VisionApiRuntimeConfig {
    VisionApiEndpointConfig endpoint;
    std::uint32_t minimumRequestIntervalMs = kDefaultVisionApiRequestIntervalMs;
    std::uint32_t maximumConsecutiveFailures = 3;
    std::uint32_t circuitBreakDurationMs = 5 * 60 * 1000;
};

struct HttpHeader {
    std::string name;
    std::string value;
};

struct HttpRequest {
    std::string method = "POST";
    std::string url;
    std::vector<HttpHeader> headers;
    std::string body;
    std::uint32_t timeoutMs = 8000;
    std::uint32_t maximumResponseBytes = 16 * 1024;
};

class SecretValue final {
public:
    SecretValue() = default;
    explicit SecretValue(std::string value) : value_(std::move(value)) {}
    ~SecretValue() { Clear(); }

    SecretValue(const SecretValue&) = delete;
    SecretValue& operator=(const SecretValue&) = delete;

    SecretValue(SecretValue&& other) noexcept
        : value_(std::move(other.value_))
    {
        other.Clear();
    }

    SecretValue& operator=(SecretValue&& other) noexcept
    {
        if (this != &other) {
            Clear();
            value_ = std::move(other.value_);
            other.Clear();
        }
        return *this;
    }

    [[nodiscard]] std::string_view View() const noexcept { return value_; }
    [[nodiscard]] bool Empty() const noexcept { return value_.empty(); }

    void Clear() noexcept
    {
        volatile char* data = value_.empty() ? nullptr : value_.data();
        for (std::size_t index = 0; data && index < value_.size(); ++index) {
            data[index] = '\0';
        }
        value_.clear();
    }

private:
    std::string value_;
};

struct HttpAuthorization {
    std::string scheme = "Bearer";
    SecretValue credential;
};

enum class HttpTransportStatus : std::uint8_t {
    kSuccess,
    kNetworkError,
    kTimeout,
    kCanceled,
    kResponseTooLarge,
};

struct HttpResponse {
    HttpTransportStatus transportStatus = HttpTransportStatus::kSuccess;
    std::uint32_t statusCode = 0;
    std::string body;
    std::uint32_t retryAfterMs = 0;
    std::string error;
};

struct VisionProtocolBuildResult {
    bool success = false;
    HttpRequest request;
    std::string error;
};

struct VisionProtocolParseResult {
    bool success = false;
    SemanticClassification classification;
    std::string error;
};

[[nodiscard]] VisionApiEndpointConfig DeepSeekVisionApiPreset();
[[nodiscard]] bool ValidateVisionApiEndpoint(
    const VisionApiEndpointConfig& config,
    std::string* error = nullptr);
[[nodiscard]] std::string NormalizeVisionApiBaseUrl(std::string_view baseUrl);
[[nodiscard]] const char* VisionImageDetailName(
    VisionImageDetail detail) noexcept;

}  // namespace remote::media_intelligence
