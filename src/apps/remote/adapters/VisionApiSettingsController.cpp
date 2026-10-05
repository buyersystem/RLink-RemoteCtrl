// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "src/apps/remote/adapters/VisionApiSettingsController.h"

#include <QBuffer>
#include <QByteArray>
#include <QColor>
#include <QImage>
#include <QPainter>
#include <QPointer>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "media_intelligence/backends/openai_compatible/OpenAiCompatibleVisionProtocol.h"
#include "media_intelligence/remote/IClock.h"
#include "media_intelligence/remote/VisionApiRuntime.h"
#include "src/apps/remote/adapters/QtVisionApiExecutor.h"
#include "src/apps/remote/adapters/QtVisionApiHttpTransport.h"
#include "src/platform/win/DpapiContentAnalyzerSecretStore.h"

namespace remote::app {
namespace {

class SteadyClock final : public media_intelligence::IClock {
public:
    [[nodiscard]] std::uint64_t NowMs() const noexcept override
    {
        return static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch())
                .count());
    }
};

media_intelligence::EncodedImage CreateSyntheticTestImage(
    int maximumImageDimension,
    int jpegQuality)
{
    const int width = std::clamp(maximumImageDimension, 256, 1280);
    const int height = width * 3 / 5;
    QImage image(width, height, QImage::Format_RGB888);
    image.fill(QColor(246, 248, 252));
    QPainter painter(&image);
    const double scale = static_cast<double>(width) / 160.0;
    const auto scaledRect = [scale](int x, int y, int w, int h) {
        return QRect(
            qRound(x * scale), qRound(y * scale),
            qRound(w * scale), qRound(h * scale));
    };
    painter.fillRect(scaledRect(0, 0, 160, 18), QColor(40, 48, 64));
    painter.fillRect(scaledRect(10, 30, 92, 7), QColor(35, 42, 55));
    painter.fillRect(scaledRect(10, 44, 132, 5), QColor(95, 106, 122));
    painter.fillRect(scaledRect(10, 56, 118, 5), QColor(95, 106, 122));
    painter.fillRect(scaledRect(10, 72, 48, 14), QColor(42, 109, 232));
    painter.end();

    QByteArray encoded;
    QBuffer buffer(&encoded);
    if (!buffer.open(QIODevice::WriteOnly) ||
        !image.save(
            &buffer, "JPEG", std::clamp(jpegQuality, 30, 90)) ||
        encoded.isEmpty()) {
        return {};
    }
    auto storage = std::make_shared<const media_intelligence::EncodedImage::Storage>(
        reinterpret_cast<const std::uint8_t*>(encoded.constData()),
        reinterpret_cast<const std::uint8_t*>(encoded.constData()) +
            encoded.size());
    return media_intelligence::EncodedImage(
        std::move(storage),
        media_intelligence::EncodedImageFormat::kJpeg,
        static_cast<std::uint32_t>(image.width()),
        static_cast<std::uint32_t>(image.height()));
}

}  // namespace

struct VisionApiSettingsController::Impl {
    Impl()
        : executor(nullptr),
          transport(nullptr)
    {
    }

    SteadyClock clock;
    QtVisionApiExecutor executor;
    QtVisionApiHttpTransport transport;
    DpapiContentAnalyzerSecretStore secrets;
    std::shared_ptr<const media_intelligence::IVisionApiProtocol> protocol =
        std::make_shared<
            media_intelligence::OpenAiCompatibleVisionProtocol>();
    std::unique_ptr<media_intelligence::VisionApiRuntime> runtime;
    std::uint64_t generation = 0;
};

VisionApiSettingsController::VisionApiSettingsController(QObject* parent)
    : QObject(parent),
      impl_(std::make_unique<Impl>())
{
}

VisionApiSettingsController::~VisionApiSettingsController()
{
    CancelTest();
}

bool VisionApiSettingsController::SaveCredential(
    std::string_view credentialId,
    std::string_view secret,
    std::string* error)
{
    return impl_->secrets.SaveSecret(credentialId, secret, error);
}

bool VisionApiSettingsController::DeleteCredential(
    std::string_view credentialId,
    std::string* error)
{
    return impl_->secrets.DeleteSecret(credentialId, error);
}

bool VisionApiSettingsController::HasCredential(
    std::string_view credentialId) const
{
    return impl_->secrets.HasSecret(credentialId);
}

bool VisionApiSettingsController::TestConnection(
    media_intelligence::VisionApiEndpointConfig endpoint,
    int maximumImageDimension,
    int jpegQuality,
    TestCompletion completion,
    std::string* error)
{
    media_intelligence::EncodedImage testImage = CreateSyntheticTestImage(
        maximumImageDimension, jpegQuality);
    if (!testImage.IsValid()) {
        if (error) {
            *error = "synthetic_image_encode_failed";
        }
        return false;
    }
    return TestConnectionWithImage(
        std::move(endpoint), std::move(testImage),
        std::move(completion), error);
}

bool VisionApiSettingsController::TestConnectionWithImage(
    media_intelligence::VisionApiEndpointConfig endpoint,
    media_intelligence::EncodedImage testImage,
    TestCompletion completion,
    std::string* error)
{
    if (!completion) {
        if (error) {
            *error = "test_completion_missing";
        }
        return false;
    }
    if (!media_intelligence::ValidateVisionApiEndpoint(endpoint, error)) {
        return false;
    }
    if (!impl_->secrets.HasSecret(endpoint.credentialId)) {
        if (error) {
            *error = "credential_not_configured";
        }
        return false;
    }
    if (!testImage.IsValid()) {
        if (error) {
            *error = "test_image_invalid";
        }
        return false;
    }

    CancelTest();
    media_intelligence::VisionApiRuntimeConfig runtimeConfig;
    runtimeConfig.endpoint = std::move(endpoint);
    runtimeConfig.minimumRequestIntervalMs = 1;
    runtimeConfig.maximumConsecutiveFailures = 1;
    runtimeConfig.circuitBreakDurationMs = 1000;
    impl_->runtime =
        std::make_unique<media_intelligence::VisionApiRuntime>(
            std::move(runtimeConfig),
            impl_->protocol,
            impl_->transport,
            impl_->secrets,
            impl_->clock,
            impl_->executor);
    const std::uint64_t generation = ++impl_->generation;
    impl_->runtime->Reset(generation);

    media_intelligence::RemoteSemanticRequest request;
    request.generation = generation;
    request.sourceFrameId = 1;
    request.timestampMs = impl_->clock.NowMs();
    request.image = std::move(testImage);
    QPointer<VisionApiSettingsController> owner(this);
    const auto status = impl_->runtime->Submit(
        std::move(request),
        [owner, generation, completion = std::move(completion)](
            media_intelligence::RemoteSemanticResult result) mutable {
            if (!owner || owner->impl_->generation != generation) {
                return;
            }
            owner->impl_->runtime.reset();
            completion(std::move(result));
        });
    if (status == media_intelligence::RemoteSubmitStatus::kRejected) {
        CancelTest();
        if (error) {
            *error = "test_request_rejected";
        }
        return false;
    }
    if (error) {
        error->clear();
    }
    return true;
}

void VisionApiSettingsController::CancelTest()
{
    ++impl_->generation;
    if (impl_->runtime) {
        impl_->runtime->Stop();
        impl_->runtime.reset();
    }
}

}  // namespace remote::app
