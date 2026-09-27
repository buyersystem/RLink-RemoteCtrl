// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "api/scoped_refptr.h"
#include "src/core/SessionEngineTypes.h"

namespace remote {

class WindowsCameraCaptureSource;

namespace app {

enum class LocalMediaDeviceKind {
  kCamera,
  kMicrophone,
  kSpeaker,
};

class LocalMediaCoordinator final {
public:
  LocalMediaCoordinator();
  ~LocalMediaCoordinator();

  LocalMediaCoordinator(const LocalMediaCoordinator &) = delete;
  LocalMediaCoordinator &operator=(const LocalMediaCoordinator &) = delete;

  static std::string NormalizeDeviceId(const std::string &deviceId);
  static bool ContainsDevice(
      const std::vector<MediaDeviceDescriptor> &devices,
      const std::string &deviceId);
  SessionCommandResult
  ValidateSelection(const MediaDeviceCategorySnapshot &category,
                    const std::string &normalizedDeviceId,
                    LocalMediaDeviceKind kind) const;
  void UpdateAvailability(MediaDeviceCategorySnapshot &category,
                          LocalMediaDeviceKind kind) const;

  [[nodiscard]] std::uint64_t BeginCameraOperation();
  [[nodiscard]] bool
  IsCurrentCameraOperation(std::uint64_t generation) const;
  [[nodiscard]] webrtc::scoped_refptr<WindowsCameraCaptureSource>
  CameraCaptureSource() const;
  void SetCameraCaptureSource(
      webrtc::scoped_refptr<WindowsCameraCaptureSource> source);
  [[nodiscard]] webrtc::scoped_refptr<WindowsCameraCaptureSource>
  TakeCameraCaptureSource();

private:
  struct State;
  std::unique_ptr<State> state_;
};

} // namespace app
} // namespace remote
