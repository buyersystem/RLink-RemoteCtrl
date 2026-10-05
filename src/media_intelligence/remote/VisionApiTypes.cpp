// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "media_intelligence/remote/VisionApiTypes.h"

#include <algorithm>
#include <cctype>
#include <cmath>

namespace remote::media_intelligence {
namespace {

bool Fail(std::string* error, std::string message)
{
    if (error) {
        *error = std::move(message);
    }
    return false;
}

bool ContainsWhitespace(std::string_view value)
{
    return std::any_of(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
}

bool IsLoopbackAuthority(std::string_view authority)
{
    if (authority.starts_with("[::1]")) {
        const std::string_view suffix = authority.substr(5);
        return suffix.empty() || suffix.front() == ':';
    }

    const std::size_t port = authority.find(':');
    const std::string_view host = authority.substr(0, port);
    return host == "localhost" || host == "127.0.0.1";
}

}  // namespace

std::uint32_t NormalizeVisionApiRequestIntervalMs(double seconds) noexcept
{
    if (!std::isfinite(seconds)) {
        return kDefaultVisionApiRequestIntervalMs;
    }
    const double boundedSeconds = std::clamp(seconds,
        kMinimumVisionApiRequestIntervalMs / 1000.0,
        kMaximumVisionApiRequestIntervalMs / 1000.0);
    return static_cast<std::uint32_t>(std::lround(boundedSeconds * 1000.0));
}

VisionApiEndpointConfig DeepSeekVisionApiPreset()
{
    VisionApiEndpointConfig config;
    config.providerId = "deepseek";
    config.baseUrl = "https://api.deepseek.com";
    config.chatCompletionsPath = "/chat/completions";
    config.model = "deepseek-flash";
    config.imageDetail = VisionImageDetail::kLow;
    config.requestJsonObjectResponse = true;
    config.disableThinking = true;
    return config;
}

bool ValidateVisionApiEndpoint(
    const VisionApiEndpointConfig& config,
    std::string* error)
{
    if (config.baseUrl.empty()) {
        return Fail(error, "base_url_empty");
    }
    if (config.model.empty()) {
        return Fail(error, "model_empty");
    }
    if (config.credentialId.empty()) {
        return Fail(error, "credential_id_empty");
    }
    if (ContainsWhitespace(config.baseUrl) ||
        config.baseUrl.find('?') != std::string::npos ||
        config.baseUrl.find('#') != std::string::npos) {
        return Fail(error, "base_url_contains_forbidden_component");
    }

    const bool https = config.baseUrl.starts_with("https://");
    const bool http = config.baseUrl.starts_with("http://");
    if (!https && !http) {
        return Fail(error, "base_url_scheme_not_allowed");
    }

    const std::size_t authorityStart = https ? 8 : 7;
    const std::size_t authorityEnd = config.baseUrl.find('/', authorityStart);
    const std::string_view authority(
        config.baseUrl.data() + authorityStart,
        (authorityEnd == std::string::npos ? config.baseUrl.size()
                                          : authorityEnd) - authorityStart);
    if (authority.empty() || authority.find('@') != std::string_view::npos) {
        return Fail(error, "base_url_authority_invalid");
    }
    if (http && !IsLoopbackAuthority(authority)) {
        return Fail(error, "plain_http_requires_loopback");
    }

    if (config.chatCompletionsPath.empty() ||
        config.chatCompletionsPath.front() != '/' ||
        ContainsWhitespace(config.chatCompletionsPath) ||
        config.chatCompletionsPath.find('?') != std::string::npos ||
        config.chatCompletionsPath.find('#') != std::string::npos) {
        return Fail(error, "chat_completions_path_invalid");
    }
    if (config.timeoutMs < 1000 || config.timeoutMs > 60000) {
        return Fail(error, "timeout_out_of_range");
    }
    if (config.maximumResponseBytes < 1024 ||
        config.maximumResponseBytes > 1024 * 1024) {
        return Fail(error, "response_limit_out_of_range");
    }

    if (error) {
        error->clear();
    }
    return true;
}

std::string NormalizeVisionApiBaseUrl(std::string_view baseUrl)
{
    while (baseUrl.size() > 8 && baseUrl.back() == '/') {
        baseUrl.remove_suffix(1);
    }
    return std::string(baseUrl);
}

const char* VisionImageDetailName(VisionImageDetail detail) noexcept
{
    switch (detail) {
    case VisionImageDetail::kLow:
        return "low";
    case VisionImageDetail::kAuto:
        return "auto";
    case VisionImageDetail::kOriginal:
        return "original";
    }
    return "low";
}

}  // namespace remote::media_intelligence
