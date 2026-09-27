// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "SessionDiagnosticsFormatting.h"

#include <chrono>
#include <cstdint>
#include <sstream>
#include <string>

namespace remote::app {

std::string DesktopCaptureBackendName(
    WindowsDesktopCaptureSource::CaptureBackend backend)
{
    switch (backend) {
    case WindowsDesktopCaptureSource::CaptureBackend::kDxgiNativeTexture:
        return "native_dxgi_texture";
    case WindowsDesktopCaptureSource::CaptureBackend::kDxgiPreferred:
        return "libwebrtc_dxgi_with_gdi_fallback";
    case WindowsDesktopCaptureSource::CaptureBackend::kGdi:
    default:
        return "libwebrtc_gdi";
    }
}

std::string DescribeEncoderRuntimeInstance(
    const VideoEncoderInstanceRuntimeStatus& status)
{
    std::string state = status.state;
    if (state == "hardware_created") {
        state = "硬件编码器已创建";
    } else if (state == "hardware_active") {
        state = "硬件编码中";
    } else if (state == "hardware_stopped") {
        state = "硬件编码已停止";
    } else if (state == "software_fallback") {
        state = "已回退软件编码";
    } else if (state == "software_active") {
        state = "软件编码中";
    }
    std::string input = status.inputFormat;
    if (input == "D3D11 BGRA desktop texture") {
        input = "桌面 D3D11 BGRA 原生纹理";
    } else if (input == "CPU BGRA desktop") {
        input = "桌面 CPU BGRA";
    } else if (input == "CPU I420") {
        input = "摄像头/CPU I420";
    } else if (input == "CPU NV12") {
        input = "CPU NV12";
    }
    std::ostringstream stream;
    stream << "实例 #" << status.instanceId << " · "
           << (status.implementation.empty()
                   ? "未知编码器"
                   : status.implementation)
           << " | 状态：" << state;
    if (!input.empty()) {
        stream << " | 输入：" << input;
    }
    if (status.width != 0 && status.height != 0) {
        stream << " | " << status.width << 'x' << status.height;
    }
    if (status.initMaxFrameRate != 0) {
        stream << " | 初始化参数（历史）："
               << status.initMinBitrateBps << '/'
               << status.initStartBitrateBps << '/'
               << status.initMaxBitrateBps
               << " bps · 最大 "
               << status.initMaxFrameRate << " FPS";
    }
    if (status.frameRate != 0) {
        stream << " | WebRTC 参考帧率："
               << status.frameRate << " FPS";
    }
    if (status.observedInputFrameRate != 0) {
        stream << " | Encode 输入："
               << status.observedInputFrameRate << " FPS";
    }
    if (status.observedOutputFrameRate != 0) {
        stream << " | 编码回调输出："
               << status.observedOutputFrameRate << " FPS";
    }
    if (status.totalInputFrames != 0 ||
        status.totalOutputFrames != 0) {
        stream << " | 帧计数：输入 "
               << status.totalInputFrames
               << " / 输出 " << status.totalOutputFrames
               << " / 编码器拒绝 " << status.totalDroppedFrames;
    }
    if (status.configuredFrameRate != 0) {
        stream << " | MFT 声明帧率："
               << status.configuredFrameRate << " FPS";
    }
    if (status.targetBitrateBps != 0) {
        stream << " | SetRates #" << status.rateUpdateSequence
               << " 请求码率："
               << status.targetBitrateBps << " bps";
        if (status.lastRateUpdateUnixMs != 0) {
            const auto nowMs = static_cast<std::uint64_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count());
            stream << "（"
                   << (nowMs >= status.lastRateUpdateUnixMs
                           ? nowMs - status.lastRateUpdateUnixMs
                           : 0)
                   << " ms 前）";
        }
    }
    if (status.implementation == "MediaFoundationD3D11H264") {
        stream << " | MFT 动态码率设置："
               << (status.bitrateConfigurationAccepted ? "成功" : "失败");
        if (status.bitrateReadbackAvailable) {
            stream << " | MFT 读回码率："
                   << status.configuredBitrateBps << " bps";
        } else {
            stream << " | MFT 读回码率：驱动未提供";
        }
    }
    if (!status.fallbackReason.empty()) {
        stream << " | 回退原因：" << status.fallbackReason;
    }
    return stream.str();
}

}  // namespace remote::app
