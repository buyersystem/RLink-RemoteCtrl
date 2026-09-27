// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "LocalMediaCoordinator.h"

#include <algorithm>
#include <utility>

#include "src/platform/win/WindowsCameraCaptureSource.h"

namespace remote::app {
namespace {

const char *DeviceName(LocalMediaDeviceKind kind) {
  switch (kind) {
  case LocalMediaDeviceKind::kCamera:
    return "camera";
  case LocalMediaDeviceKind::kMicrophone:
    return "microphone";
  case LocalMediaDeviceKind::kSpeaker:
    return "speaker";
  }
  return "media";
}

std::string ErrorCode(LocalMediaDeviceKind kind, const char *suffix) {
  return std::string(DeviceName(kind)) + suffix;
}

std::string UnavailableMessage(LocalMediaDeviceKind kind, bool noDevices) {
  if (noDevices) {
    return std::string("No ") + DeviceName(kind) + " device is available.";
  }
  return std::string("The selected ") + DeviceName(kind) +
         " is no longer available.";
}

std::string DisconnectedMessage(LocalMediaDeviceKind kind) {
  return std::string("The selected ") + DeviceName(kind) + " is not connected.";
}

} // namespace

struct LocalMediaCoordinator::State {
  webrtc::scoped_refptr<WindowsCameraCaptureSource> cameraCaptureSource;
  std::uint64_t cameraGeneration = 0;
};

LocalMediaCoordinator::LocalMediaCoordinator()
    : state_(std::make_unique<State>()) {}

LocalMediaCoordinator::~LocalMediaCoordinator() = default;

std::string
LocalMediaCoordinator::NormalizeDeviceId(const std::string &deviceId) {
  return deviceId.empty() ? std::string(kSystemDefaultMediaDeviceId) : deviceId;
}

SessionCommandResult LocalMediaCoordinator::ValidateSelection(
    const MediaDeviceCategorySnapshot &category,
    const std::string &normalizedDeviceId, LocalMediaDeviceKind kind) const {
  if (category.state == MediaDeviceSelectionState::kSwitching) {
    return {false, ErrorCode(kind, "_device_switch_in_progress"),
            std::string("A ") + DeviceName(kind) +
                " switch is already in progress."};
  }
  const bool followsDefault = normalizedDeviceId == kSystemDefaultMediaDeviceId;
  if ((followsDefault && category.devices.empty()) ||
      (!followsDefault &&
       !ContainsDevice(category.devices, normalizedDeviceId))) {
    return {false, ErrorCode(kind, "_device_unavailable"),
            UnavailableMessage(kind, followsDefault)};
  }
  return {true, {}, {}};
}

void LocalMediaCoordinator::UpdateAvailability(
    MediaDeviceCategorySnapshot &category, LocalMediaDeviceKind kind) const {
  if (category.state == MediaDeviceSelectionState::kSwitching) {
    return;
  }
  const bool followsDefault =
      category.preferredDeviceId.empty() ||
      category.preferredDeviceId == kSystemDefaultMediaDeviceId;
  const bool available =
      followsDefault ? !category.devices.empty()
                     : ContainsDevice(category.devices,
                                      category.preferredDeviceId);
  if (available) {
    category.state = MediaDeviceSelectionState::kReady;
    category.errorCode.clear();
    category.errorMessage.clear();
    return;
  }
  category.state = MediaDeviceSelectionState::kUnavailable;
  category.errorCode = ErrorCode(kind, "_device_unavailable");
  category.errorMessage = DisconnectedMessage(kind);
}

bool LocalMediaCoordinator::ContainsDevice(
    const std::vector<MediaDeviceDescriptor> &devices,
    const std::string &deviceId) {
  return std::any_of(devices.begin(), devices.end(),
                     [&deviceId](const MediaDeviceDescriptor &device) {
                       return device.id == deviceId && device.available;
                     });
}

std::uint64_t LocalMediaCoordinator::BeginCameraOperation() {
  return ++state_->cameraGeneration;
}

bool LocalMediaCoordinator::IsCurrentCameraOperation(
    std::uint64_t generation) const {
  return state_->cameraGeneration == generation;
}

webrtc::scoped_refptr<WindowsCameraCaptureSource>
LocalMediaCoordinator::CameraCaptureSource() const {
  return state_->cameraCaptureSource;
}

void LocalMediaCoordinator::SetCameraCaptureSource(
    webrtc::scoped_refptr<WindowsCameraCaptureSource> source) {
  state_->cameraCaptureSource = std::move(source);
}

webrtc::scoped_refptr<WindowsCameraCaptureSource>
LocalMediaCoordinator::TakeCameraCaptureSource() {
  return std::exchange(state_->cameraCaptureSource, nullptr);
}

} // namespace remote::app
