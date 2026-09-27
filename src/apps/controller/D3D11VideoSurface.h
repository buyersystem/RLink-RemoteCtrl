// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#include <array>
#include <cstddef>
#include <cstdint>

#include <QByteArray>
#include <QImage>
#include <QPoint>
#include <QWidget>

#include "RemoteCursorRenderState.h"
#include "api/video/i420_buffer.h"
#include "src/platform/win/D3D11NativeFrameBuffer.h"

class QPaintEngine;
class QPaintEvent;

namespace remote::controller {

// Presents decoder-owned or CPU-uploaded frames through a swap chain created
// on the matching D3D11 device.
class D3D11VideoSurface final : public QWidget {
public:
    struct PresentTiming {
        bool succeeded = false;
        std::uint64_t cpuPreparationUs = 0;
        std::uint64_t videoProcessorSubmitUs = 0;
        std::uint64_t presentCallUs = 0;
    };

    explicit D3D11VideoSurface(QWidget* parent = nullptr);

    void SetCursorFrame(
        const RemoteCursorRenderState::Snapshot& snapshot,
        int sourceWidth,
        int sourceHeight);
    PresentTiming Present(D3D11NativeFrameBuffer* frame);
    PresentTiming PresentCpuBgra(const QImage& image);
    PresentTiming PresentCpuNv12(const QImage& image);
    PresentTiming PresentCpuI420(webrtc::I420BufferInterface* frame);

protected:
    bool nativeEvent(const QByteArray& eventType,
                     void* message,
                     qintptr* result) override;
    QPaintEngine* paintEngine() const override;
    void paintEvent(QPaintEvent* event) override;

private:
    struct I420PlaneTexture {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> view;
    };

    struct I420UploadSlot {
        I420PlaneTexture y;
        I420PlaneTexture u;
        I420PlaneTexture v;
    };

    struct CursorConstants {
        float left;
        float top;
        float right;
        float bottom;
    };

    bool EnsureCursorPipeline();
    bool UpdateCursorTexture();
    void DrawCursorOverlay(ID3D11RenderTargetView* renderTarget,
                           UINT outputWidth,
                           UINT outputHeight);
    void ResetCursorResources();
    bool EnsureI420ShaderPipeline();
    bool CreateI420PlaneTexture(UINT width,
                                UINT height,
                                I420PlaneTexture* plane);
    bool EnsureI420UploadTextures(UINT width, UINT height);
    bool UploadI420Plane(I420PlaneTexture& plane,
                         const std::uint8_t* source,
                         int sourceStride,
                         UINT width,
                         UINT height);
    bool EnsureI420BackBufferView();
    void ResetI420UploadTextures();
    void ResetI420ShaderResources();
    PresentTiming PresentTexture(ID3D11Device* device,
                                 ID3D11Texture2D* texture,
                                 UINT subresourceIndex,
                                 int sourceWidth,
                                 int sourceHeight,
                                 DXGI_FORMAT expectedFormat);
    bool EnsureCpuDevice();
    bool EnsureCpuUploadTexture(UINT width,
                                UINT height,
                                DXGI_FORMAT format);
    bool EnsureDevice(ID3D11Device* device);
    bool EnsureSwapChain(HWND window, UINT width, UINT height);
    bool EnsureVideoProcessor(UINT sourceWidth,
                              UINT sourceHeight,
                              UINT outputWidth,
                              UINT outputHeight);
    void ResetDevice();

    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<ID3D11VideoDevice> videoDevice_;
    Microsoft::WRL::ComPtr<ID3D11VideoContext> videoContext_;
    Microsoft::WRL::ComPtr<IDXGIFactory2> factory_;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain_;
    Microsoft::WRL::ComPtr<ID3D11VideoProcessorEnumerator>
        processorEnumerator_;
    Microsoft::WRL::ComPtr<ID3D11VideoProcessor> processor_;
    Microsoft::WRL::ComPtr<ID3D11Device> cpuPresentationDevice_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> cpuUploadTexture_;
    std::array<I420UploadSlot, 3> i420UploadSlots_;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> i420VertexShader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> i420PixelShader_;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> i420SamplerState_;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> i420RasterizerState_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> i420BackBufferView_;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> cursorVertexShader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> cursorPixelShader_;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> cursorSamplerState_;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> cursorRasterizerState_;
    Microsoft::WRL::ComPtr<ID3D11BlendState> cursorBlendState_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> cursorConstantBuffer_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> cursorTexture_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cursorTextureView_;
    HWND window_ = nullptr;
    UINT swapWidth_ = 0;
    UINT swapHeight_ = 0;
    UINT sourceWidth_ = 0;
    UINT sourceHeight_ = 0;
    UINT processorWidth_ = 0;
    UINT processorHeight_ = 0;
    UINT frameIndex_ = 0;
    UINT cpuUploadWidth_ = 0;
    UINT cpuUploadHeight_ = 0;
    DXGI_FORMAT cpuUploadFormat_ = DXGI_FORMAT_UNKNOWN;
    UINT i420UploadWidth_ = 0;
    UINT i420UploadHeight_ = 0;
    std::size_t i420UploadSlotIndex_ = 0;
    QImage cursorImage_;
    QPoint cursorHotspot_;
    QPoint cursorPosition_;
    std::uint64_t cursorShapeRevision_ = 0;
    std::uint64_t uploadedCursorShapeRevision_ = 0;
    UINT cursorTextureWidth_ = 0;
    UINT cursorTextureHeight_ = 0;
    int cursorSourceWidth_ = 0;
    int cursorSourceHeight_ = 0;
    bool cursorVisible_ = false;
};

}  // namespace remote::controller
