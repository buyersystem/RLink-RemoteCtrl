# Target definitions for the RemoteC components and applications.
#
# Windows-only for now; platform-specific sources are grouped so a future
# cross-platform port can add e.g. src/platform/posix alongside src/platform/win.

set(_src "${CMAKE_SOURCE_DIR}/src")

include(GNUInstallDirs)

# These targets must remain consumable without the repository-wide
# rlink_project interface target, which carries application-only include paths.
function(media_intelligence_apply_common target)
  target_compile_features(${target} PUBLIC cxx_std_20)
  target_include_directories(${target} PUBLIC
    "$<BUILD_INTERFACE:${_src}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
  if(MSVC)
    target_compile_options(${target} PRIVATE
      /utf-8 /Zc:__cplusplus /permissive- /MP /W3 /sdl /Zi)
    target_compile_definitions(${target} PRIVATE
      _UNICODE UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
    target_link_options(${target} PRIVATE /DEBUG)
  endif()
endfunction()

# ---------------------------------------------------------------------------
# Static libraries
# ---------------------------------------------------------------------------

# Media intelligence is split from RLink adapters so the policy/runtime can
# be embedded by another native C++ application without Qt, WebRTC or D3D11.
add_library(media_intelligence_core STATIC
  "${_src}/media_intelligence/core/ContentState.cpp"
  "${_src}/media_intelligence/core/ContentState.h"
  "${_src}/media_intelligence/core/ScreenScene.cpp"
  "${_src}/media_intelligence/core/ScreenScene.h"
  "${_src}/media_intelligence/core/ContentAwareStreamPolicy.cpp"
  "${_src}/media_intelligence/core/ContentAwareStreamPolicy.h"
  "${_src}/media_intelligence/core/GoogCcNetworkPressure.cpp"
  "${_src}/media_intelligence/core/GoogCcNetworkPressure.h"
  "${_src}/media_intelligence/core/CalibratedStreamQualityModel.cpp"
  "${_src}/media_intelligence/core/CalibratedStreamQualityModel.h"
  "${_src}/media_intelligence/core/H264ReferenceQualityModel.cpp"
  "${_src}/media_intelligence/core/H264ReferenceQualityModel.h"
  "${_src}/media_intelligence/core/ContentMotionAnalyzer.cpp"
  "${_src}/media_intelligence/core/ContentMotionAnalyzer.h"
  "${_src}/media_intelligence/core/EncodedImageView.h"
  "${_src}/media_intelligence/core/IRemoteSemanticClassifier.h"
  "${_src}/media_intelligence/core/SemanticClassification.h")
media_intelligence_apply_common(media_intelligence_core)
set_target_properties(media_intelligence_core PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF
  EXPORT_NAME core
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
add_library(RLinkMediaIntelligence::core ALIAS media_intelligence_core)

add_library(media_intelligence_runtime STATIC
  "${_src}/media_intelligence/runtime/ContentAnalysisWorker.cpp"
  "${_src}/media_intelligence/runtime/ContentAnalysisWorker.h")
media_intelligence_apply_common(media_intelligence_runtime)
set_target_properties(media_intelligence_runtime PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF
  EXPORT_NAME runtime
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
target_link_libraries(media_intelligence_runtime PUBLIC
  media_intelligence_core)
add_library(RLinkMediaIntelligence::runtime ALIAS media_intelligence_runtime)

add_library(media_intelligence_remote STATIC
  "${_src}/media_intelligence/remote/IClock.h"
  "${_src}/media_intelligence/remote/IExecutor.h"
  "${_src}/media_intelligence/remote/IHttpTransport.h"
  "${_src}/media_intelligence/remote/ILogger.h"
  "${_src}/media_intelligence/remote/ISecretProvider.h"
  "${_src}/media_intelligence/remote/IVisionApiProtocol.h"
  "${_src}/media_intelligence/remote/VisionApiRuntime.cpp"
  "${_src}/media_intelligence/remote/VisionApiRuntime.h"
  "${_src}/media_intelligence/remote/VisionApiTypes.cpp"
  "${_src}/media_intelligence/remote/VisionApiTypes.h")
media_intelligence_apply_common(media_intelligence_remote)
set_target_properties(media_intelligence_remote PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF
  EXPORT_NAME remote
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
target_link_libraries(media_intelligence_remote PUBLIC
  media_intelligence_core)
add_library(RLinkMediaIntelligence::remote ALIAS media_intelligence_remote)

add_library(media_intelligence_openai_compatible STATIC
  "${_src}/media_intelligence/backends/openai_compatible/OpenAiCompatibleVisionProtocol.cpp"
  "${_src}/media_intelligence/backends/openai_compatible/OpenAiCompatibleVisionProtocol.h")
media_intelligence_apply_common(media_intelligence_openai_compatible)
set_target_properties(media_intelligence_openai_compatible PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF
  EXPORT_NAME openai_compatible
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
target_link_libraries(media_intelligence_openai_compatible PUBLIC
  media_intelligence_remote)
add_library(RLinkMediaIntelligence::openai_compatible ALIAS
  media_intelligence_openai_compatible)

set(_turbojpeg_root "${CMAKE_SOURCE_DIR}/third_party/libjpeg-turbo")
if(NOT EXISTS "${_turbojpeg_root}/CMakeLists.txt" OR
   NOT EXISTS "${_turbojpeg_root}/src/turbojpeg.h")
  message(FATAL_ERROR
    "Bundled libjpeg-turbo source is incomplete under ${_turbojpeg_root}")
endif()
set(_turbojpeg_nasm "${RLINK_WEBRTC_OUT}/nasm.exe")
if(WIN32 AND NOT EXISTS "${_turbojpeg_nasm}")
  message(FATAL_ERROR
    "Bundled libjpeg-turbo requires NASM; expected ${_turbojpeg_nasm}")
endif()
include(ExternalProject)
set(_turbojpeg_build
  "${CMAKE_CURRENT_BINARY_DIR}/third_party/libjpeg-turbo")
ExternalProject_Add(rlink_turbojpeg_external
  SOURCE_DIR "${_turbojpeg_root}"
  BINARY_DIR "${_turbojpeg_build}"
  CMAKE_GENERATOR "${CMAKE_GENERATOR}"
  CMAKE_GENERATOR_INSTANCE "${CMAKE_GENERATOR_INSTANCE}"
  CMAKE_GENERATOR_PLATFORM "${CMAKE_GENERATOR_PLATFORM}"
  CMAKE_GENERATOR_TOOLSET "${CMAKE_GENERATOR_TOOLSET}"
  CMAKE_ARGS
    "-DENABLE_SHARED:BOOL=ON"
    "-DENABLE_STATIC:BOOL=OFF"
    "-DWITH_CRT_DLL:BOOL=ON"
    "-DWITH_TURBOJPEG:BOOL=ON"
    "-DWITH_TOOLS:BOOL=OFF"
    "-DWITH_TESTS:BOOL=OFF"
    "-DREQUIRE_SIMD:BOOL=ON"
    "-DCMAKE_SYSTEM_PROCESSOR:STRING=AMD64"
    "-DCMAKE_ASM_NASM_COMPILER:FILEPATH=${_turbojpeg_nasm}"
  BUILD_COMMAND
    "${CMAKE_COMMAND}" --build <BINARY_DIR>
    --config $<CONFIG> --target turbojpeg
  INSTALL_COMMAND ""
  BUILD_BYPRODUCTS
    "${_turbojpeg_build}/Release/turbojpeg.lib"
    "${_turbojpeg_build}/Release/turbojpeg.dll"
    "${_turbojpeg_build}/Debug/turbojpeg.lib"
    "${_turbojpeg_build}/Debug/turbojpeg.dll"
    "${_turbojpeg_build}/RelWithDebInfo/turbojpeg.lib"
    "${_turbojpeg_build}/RelWithDebInfo/turbojpeg.dll"
    "${_turbojpeg_build}/MinSizeRel/turbojpeg.lib"
    "${_turbojpeg_build}/MinSizeRel/turbojpeg.dll")
add_library(rlink_bundled_turbojpeg SHARED IMPORTED GLOBAL)
set_target_properties(rlink_bundled_turbojpeg PROPERTIES
  IMPORTED_LOCATION_RELEASE
    "${_turbojpeg_build}/Release/turbojpeg.dll"
  IMPORTED_IMPLIB_RELEASE
    "${_turbojpeg_build}/Release/turbojpeg.lib"
  IMPORTED_LOCATION_DEBUG
    "${_turbojpeg_build}/Debug/turbojpeg.dll"
  IMPORTED_IMPLIB_DEBUG
    "${_turbojpeg_build}/Debug/turbojpeg.lib"
  IMPORTED_LOCATION_RELWITHDEBINFO
    "${_turbojpeg_build}/RelWithDebInfo/turbojpeg.dll"
  IMPORTED_IMPLIB_RELWITHDEBINFO
    "${_turbojpeg_build}/RelWithDebInfo/turbojpeg.lib"
  IMPORTED_LOCATION_MINSIZEREL
    "${_turbojpeg_build}/MinSizeRel/turbojpeg.dll"
  IMPORTED_IMPLIB_MINSIZEREL
    "${_turbojpeg_build}/MinSizeRel/turbojpeg.lib"
  INTERFACE_INCLUDE_DIRECTORIES "${_turbojpeg_root}/src")
add_dependencies(rlink_bundled_turbojpeg rlink_turbojpeg_external)

add_library(media_intelligence_turbojpeg STATIC
  "${_src}/media_intelligence/backends/turbojpeg/TurboJpegI420Encoder.cpp"
  "${_src}/media_intelligence/backends/turbojpeg/TurboJpegI420Encoder.h")
media_intelligence_apply_common(media_intelligence_turbojpeg)
set_target_properties(media_intelligence_turbojpeg PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
target_link_libraries(media_intelligence_turbojpeg PUBLIC
  media_intelligence_core
  rlink_bundled_turbojpeg)

# RLink host adapters. These deliberately sit outside the exported reusable
# component because they bind the generic interfaces to Qt and Windows DPAPI.
add_library(rlink_vision_api_adapters STATIC
  "${_src}/apps/remote/adapters/QtVisionApiExecutor.cpp"
  "${_src}/apps/remote/adapters/QtVisionApiExecutor.h"
  "${_src}/apps/remote/adapters/QtVisionApiHttpTransport.cpp"
  "${_src}/apps/remote/adapters/QtVisionApiHttpTransport.h"
  "${_src}/apps/remote/adapters/VisionApiFrameAnalyzer.cpp"
  "${_src}/apps/remote/adapters/VisionApiFrameAnalyzer.h"
  "${_src}/apps/remote/adapters/VisionApiSettingsController.cpp"
  "${_src}/apps/remote/adapters/VisionApiSettingsController.h"
  "${_src}/apps/remote/adapters/WebRtcVisionFrameEncoder.cpp"
  "${_src}/apps/remote/adapters/WebRtcVisionFrameEncoder.h"
  "${_src}/platform/win/DpapiContentAnalyzerSecretStore.cpp"
  "${_src}/platform/win/DpapiContentAnalyzerSecretStore.h")
rlink_apply_common(rlink_vision_api_adapters)
set_target_properties(rlink_vision_api_adapters PROPERTIES
  AUTOMOC ON AUTOUIC OFF AUTORCC OFF
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
target_link_libraries(rlink_vision_api_adapters PUBLIC
  media_intelligence_openai_compatible
  media_intelligence_turbojpeg
  rlink_webrtc
  Qt6::Core Qt6::Gui Qt6::Network
  crypt32)

install(TARGETS
    media_intelligence_core
    media_intelligence_runtime
    media_intelligence_remote
    media_intelligence_openai_compatible
  EXPORT RLinkMediaIntelligenceTargets
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(DIRECTORY "${_src}/media_intelligence/"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/media_intelligence"
  FILES_MATCHING PATTERN "*.h")
install(EXPORT RLinkMediaIntelligenceTargets
  FILE RLinkMediaIntelligenceTargets.cmake
  NAMESPACE RLinkMediaIntelligence::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/RLinkMediaIntelligence")

include(CMakePackageConfigHelpers)
configure_package_config_file(
  "${CMAKE_SOURCE_DIR}/cmake/RLinkMediaIntelligenceConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/RLinkMediaIntelligenceConfig.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/RLinkMediaIntelligence")
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/RLinkMediaIntelligenceConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/RLinkMediaIntelligenceConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/RLinkMediaIntelligenceConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/RLinkMediaIntelligence")

# RemoteCore: protocols + core session policy (no Qt, uses WebRTC headers).
add_library(rlink_core STATIC
  "${_src}/core/DesktopCaptureTypes.h"
  "${_src}/core/IRemoteSessionControl.h"
  "${_src}/core/ScreenNetworkPolicy.cpp"
  "${_src}/core/ScreenStreamPolicy.cpp"
  "${_src}/core/SessionController.cpp"
  "${_src}/protocol/BinaryProtocol.cpp"
  "${_src}/protocol/ClipboardProtocol.cpp"
  "${_src}/protocol/DataChannelCatalog.cpp"
  "${_src}/protocol/FileTransferProtocol.cpp"
  "${_src}/protocol/RemoteInputProtocol.cpp"
  "${_src}/protocol/RemoteCursorProtocol.cpp"
  "${_src}/protocol/RoomMemberControlProtocol.cpp"
  "${_src}/protocol/ScreenShareControlProtocol.cpp")
target_sources(rlink_core PRIVATE "${_src}/protocol/ScreenReceiverFeedbackProtocol.cpp")
rlink_apply_common(rlink_core)
target_link_libraries(rlink_core PUBLIC rlink_webrtc)

# RemoteAuth: OIDC/OAuth via Qt NetworkAuthorization.
add_library(rlink_auth STATIC
  "${_src}/auth/AuthConfig.cpp"
  "${_src}/auth/AuthManager.cpp"
  "${_src}/auth/DpapiTokenStore.cpp"
  "${_src}/auth/OidcDiscovery.cpp")
rlink_apply_common(rlink_auth)
set_target_properties(rlink_auth PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)
target_link_libraries(rlink_auth PUBLIC
  Qt6::Core Qt6::Gui Qt6::Network Qt6::NetworkAuth)

# SignalingTransport: Qt WebSocket signaling client.
add_library(rlink_signaling STATIC
  "${_src}/signaling/ISignalingClient.h"
  "${_src}/signaling/SignalingObservers.h"
  "${_src}/signaling/SignalingTypes.h"
  "${_src}/signaling/SignalingJsonCodec.cpp"
  "${_src}/signaling/QtWebSocketSignalingClient.cpp"
  "${_src}/signaling/QtWebSocketSignalingClient.Connection.cpp"
  "${_src}/signaling/QtWebSocketSignalingClient.Dispatch.cpp"
  "${_src}/signaling/QtWebSocketSignalingClient.DispatchConnection.cpp"
  "${_src}/signaling/QtWebSocketSignalingClient.DispatchDirect.cpp"
  "${_src}/signaling/QtWebSocketSignalingClient.DispatchRoom.cpp"
  "${_src}/signaling/QtWebSocketSignalingClient.DispatchTransport.cpp"
  "${_src}/signaling/QtWebSocketSignalingClient.Internal.h"
  "${_src}/signaling/QtWebSocketSignalingClient.LegacySession.cpp"
  "${_src}/signaling/QtWebSocketSignalingClient.Lifecycle.cpp"
  "${_src}/signaling/QtWebSocketSignalingClient.Negotiation.cpp"
  "${_src}/signaling/QtWebSocketSignalingClient.OwnedDevices.cpp"
  "${_src}/signaling/QtWebSocketSignalingClient.PairTransport.cpp"
  "${_src}/signaling/QtWebSocketSignalingClient.Recovery.cpp"
  "${_src}/signaling/QtWebSocketSignalingClient.RoomCommands.cpp")
rlink_apply_common(rlink_signaling)
set_target_properties(rlink_signaling PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)
target_link_libraries(rlink_signaling PUBLIC
  Qt6::Core Qt6::Network Qt6::WebSockets)

# WebRtcTransport: libwebrtc session + Windows capture/encode backends.
add_library(rlink_webrtc_transport STATIC
  "${_src}/platform/win/D3D11DesktopFrameBuffer.cpp"
  "${_src}/platform/win/DesktopBgraFrameBuffer.cpp"
  "${_src}/platform/win/D3D11NativeFrameBuffer.cpp"
  "${_src}/platform/win/DxgiNativeDesktopCapturer.cpp"
  "${_src}/platform/win/IRemoteVisionFrameAnalyzer.h"
  "${_src}/platform/win/WindowsDesktopCaptureSource.cpp"
  "${_src}/platform/win/WindowsDisplayTopology.cpp"
  "${_src}/platform/win/WindowsHardwareFingerprint.cpp"
  "${_src}/platform/win/WindowsCameraCaptureSource.cpp"
  "${_src}/platform/win/MfH264EncoderCapabilityProbe.cpp"
  "${_src}/platform/win/MfD3D11H264Encoder.cpp"
  "${_src}/platform/win/MfD3D11H264EncoderFactory.cpp"
  "${_src}/platform/win/MfH264EncoderSelfTest.cpp"
  "${_src}/platform/win/MfD3D11H264Decoder.cpp"
  "${_src}/platform/win/FfmpegD3D11H264Decoder.cpp"
  "${_src}/platform/win/FfmpegHardwareH264Encoder.cpp"
  "${_src}/platform/win/FfmpegHardwareRateControl.cpp"
  "${_src}/platform/win/FfmpegHardwareH264EncoderFactory.cpp"
  "${_src}/platform/win/FfmpegX264H264Encoder.cpp"
  "${_src}/platform/win/FfmpegX264H264EncoderFactory.cpp"
  "${_src}/platform/win/QualityOpenH264Encoder.cpp"
  "${_src}/platform/win/H264EncoderBenchmark.cpp"
  "${_src}/platform/win/MfD3D11H264DecoderBenchmark.cpp"
  "${_src}/platform/win/MfD3D11H264DecoderFactory.cpp"
  "${_src}/webrtc/LibWebRtcSession.cpp"
  "${_src}/webrtc/LibWebRtcSession.AudioSlot.cpp"
  "${_src}/webrtc/LibWebRtcSession.ContentPolicy.cpp"
  "${_src}/webrtc/LibWebRtcSession.ReceiverFeedback.cpp"
  "${_src}/webrtc/LibWebRtcSession.DataChannels.cpp"
  "${_src}/webrtc/LibWebRtcSession.Internal.h"
  "${_src}/webrtc/LibWebRtcSession.Lifecycle.cpp"
  "${_src}/webrtc/LibWebRtcSession.Negotiation.cpp"
  "${_src}/webrtc/LibWebRtcSession.NegotiationInternals.cpp"
  "${_src}/webrtc/LibWebRtcSession.Observers.cpp"
  "${_src}/webrtc/LibWebRtcSession.State.cpp"
  "${_src}/webrtc/LibWebRtcSession.StatsClose.cpp"
  "${_src}/webrtc/LibWebRtcSession.VideoSlots.cpp"
  "${_src}/webrtc/DataChannelManager.cpp"
  "${_src}/webrtc/DataChannelManager.h"
  "${_src}/webrtc/PeerNegotiator.cpp"
  "${_src}/webrtc/PeerNegotiator.h"
  "${_src}/webrtc/MediaSlotManager.h"
  "${_src}/webrtc/PeerConnectionStatsCollector.cpp"
  "${_src}/webrtc/WebRtcRuntime.cpp"
  "${_src}/webrtc/GoogCcTelemetry.cpp"
  "${_src}/webrtc/GoogCcTelemetry.h"
  "${_src}/webrtc/WindowsPreferredVideoDecoderFactory.cpp"
  "${_src}/webrtc/WindowsPreferredVideoEncoderFactory.cpp")
rlink_apply_common(rlink_webrtc_transport)
target_link_libraries(rlink_webrtc_transport PUBLIC rlink_webrtc rlink_ffmpeg rlink_core)
target_link_libraries(rlink_webrtc_transport PUBLIC
  media_intelligence_runtime)

# RemoteSessionEngine: in-process session engine + Windows platform services.
add_library(rlink_session_engine STATIC
  "${_src}/apps/remote/InProcessSessionEngine.ContentFeedback.cpp"
  "${_src}/apps/remote/adapters/ContentAwarePolicyDiagnostics.cpp"
  "${_src}/apps/remote/adapters/ContentAwarePolicyDiagnostics.h"
  "${_src}/apps/remote/ScreenShareCoordinator.cpp"
  "${_src}/apps/remote/ScreenShareCoordinator.h"
  "${_src}/apps/remote/LocalMediaCoordinator.cpp"
  "${_src}/apps/remote/LocalMediaCoordinator.h"
  "${_src}/apps/remote/VideoPipelinePreferenceNames.h"
  "${_src}/apps/remote/FileTransferController.cpp"
  "${_src}/apps/remote/FileTransferController.Chunking.cpp"
  "${_src}/apps/remote/FileTransferController.Commands.cpp"
  "${_src}/apps/remote/FileTransferController.Lifecycle.cpp"
  "${_src}/apps/remote/FileTransferController.Receive.cpp"
  "${_src}/apps/remote/FileTransferController.Reliability.cpp"
  "${_src}/apps/remote/FileTransferController.Send.cpp"
  "${_src}/apps/remote/FileTransferController.Worker.cpp"
  "${_src}/apps/remote/FileTransferControllerInternal.h"
  "${_src}/apps/remote/ClipboardCacheManager.cpp"
  "${_src}/apps/remote/ClipboardController.cpp"
  "${_src}/apps/remote/FileTransferStorage.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.Audio.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.Camera.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.DirectBridge.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.DirectCallbacks.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.DirectData.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.DirectMedia.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.DirectScreen.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.DirectSession.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.LeaseCallbacks.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.Lifecycle.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.MediaDevices.cpp"
  "${_src}/apps/remote/InProcessSessionMediaAdapter.cpp"
  "${_src}/apps/remote/InProcessSessionMediaAdapter.h"
  "${_src}/apps/remote/InProcessSessionEngine.RemoteControl.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.RemoteCursor.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.Room.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.RoomCallbacks.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.RoomPairDataDispatch.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.RoomPairInputDispatch.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.RoomPairReliableDispatch.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.RoomPairScreenDispatch.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.RoomPairTransferDispatch.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.RoomPairChannels.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.RoomPairLifecycle.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.RoomPairState.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.RoomSessionCommands.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.RuntimeHelpers.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.ScreenShare.cpp"
  "${_src}/apps/remote/DirectSessionCoordinator.cpp"
  "${_src}/apps/remote/DirectSessionCoordinator.h"
  "${_src}/apps/remote/ISessionMediaAccess.h"
  "${_src}/apps/remote/RoomSessionCoordinator.cpp"
  "${_src}/apps/remote/RoomSessionCoordinator.h"
  "${_src}/apps/remote/RoomMediaSlots.h"
  "${_src}/apps/remote/SessionDataChannelPolicy.h"
  "${_src}/apps/remote/SessionDiagnosticsFormatting.cpp"
  "${_src}/apps/remote/SessionDiagnosticsFormatting.h"
  "${_src}/apps/remote/SessionStatsPoller.cpp"
  "${_src}/apps/remote/SessionStatsPoller.h"
  "${_src}/platform/win/WindowsFileTransferService.cpp"
  "${_src}/platform/win/WindowsClipboardService.cpp"
  "${_src}/platform/win/WindowsCursorMonitor.cpp")
rlink_apply_common(rlink_session_engine)
target_link_libraries(rlink_session_engine PUBLIC
  rlink_core rlink_webrtc_transport rlink_signaling)

# ---------------------------------------------------------------------------
# Applications
# ---------------------------------------------------------------------------

# RLinkAPP: Qt Widgets controller/agent client.
add_executable(RLinkAPP WIN32
  "${_src}/apps/remote/RemoteCProductMain.cpp"
  "${_src}/apps/remote/RemoteCApplicationCoordinator.cpp"
  "${_src}/apps/update/SoftwareUpdateController.cpp"
  "${_src}/apps/controller/LoginWindow.cpp"
  "${_src}/apps/controller/MorphIconToolButton.cpp"
  "${_src}/apps/controller/MorphIconToolButton.h"
  "${_src}/apps/controller/ui/RemoteCTheme.cpp"
  "${_src}/apps/controller/ui/morph/MorphIconCore.cpp"
  "${_src}/apps/controller/ui/morph/MorphIconButtonBinding.cpp"
  "${_src}/apps/controller/CameraWindow.cpp"
  "${_src}/apps/controller/D3D11VideoSurface.cpp"
  "${_src}/apps/controller/D3D11VideoSurface.h"
  "${_src}/apps/controller/ControllerMainWindow.cpp"
  "${_src}/apps/controller/ControllerMainWindow.AppShell.cpp"
  "${_src}/apps/controller/ControllerMainWindow.Session.cpp"
  "${_src}/apps/controller/ControllerMainWindowSupport.cpp"
  "${_src}/apps/controller/ControllerMainWindowSupport.h"
  "${_src}/apps/controller/CurrentPageStack.cpp"
  "${_src}/apps/controller/CurrentPageStack.h"
  "${_src}/apps/controller/MediaControls.cpp"
  "${_src}/apps/controller/MediaControls.h"
  "${_src}/apps/controller/RoomStatusIndicator.cpp"
  "${_src}/apps/controller/RoomStatusIndicator.h"
  "${_src}/apps/controller/ScreenFrameRateLogger.cpp"
  "${_src}/apps/controller/ScreenFrameRateLogger.h"
  "${_src}/apps/controller/ControllerMainWindow.Ui.cpp"
  "${_src}/apps/controller/ControllerMainWindow.UiConnections.cpp"
  "${_src}/apps/controller/ControllerMainWindow.NavigationRecent.cpp"
  "${_src}/apps/controller/ControllerMainWindow.Media.cpp"
  "${_src}/apps/controller/ControllerMainWindow.Diagnostics.cpp"
  "${_src}/apps/controller/ControllerMainWindow.DiagnosticsSnapshot.cpp"
  "${_src}/apps/controller/ControllerMainWindow.DiagnosticsPage.cpp"
  "${_src}/apps/controller/ControllerMainWindow.SessionState.cpp"
  "${_src}/apps/controller/ControllerMainWindow.DeviceRecentPages.cpp"
  "${_src}/apps/controller/ControllerMainWindow.Room.cpp"
  "${_src}/apps/controller/ControllerMainWindow.RoomUi.cpp"
  "${_src}/apps/controller/ControllerMainWindow.ShellRoomPage.cpp"
  "${_src}/apps/controller/ControllerMainWindow.SettingsPage.cpp"
  "${_src}/apps/controller/ControllerMainWindow.OwnedDevices.cpp"
  "${_src}/apps/controller/pages/DirectConnectPage.cpp"
  "${_src}/apps/controller/pages/DirectConnectPage.Workspace.cpp"
  "${_src}/apps/controller/pages/DirectConnectPage.h"
  "${_src}/apps/controller/pages/DiagnosticsCardsWidget.cpp"
  "${_src}/apps/controller/pages/DiagnosticsCardsWidget.h"
  "${_src}/apps/controller/pages/ContentPolicyCards.cpp"
  "${_src}/apps/controller/pages/ContentPolicyCards.h"
  "${_src}/apps/controller/pages/DiagnosticsPage.cpp"
  "${_src}/apps/controller/pages/DiagnosticsPage.Workspace.cpp"
  "${_src}/apps/controller/pages/DiagnosticsPage.h"
  "${_src}/apps/controller/pages/OwnedDevicesPage.cpp"
  "${_src}/apps/controller/pages/OwnedDevicesPage.h"
  "${_src}/apps/controller/pages/RecentConnectionsPage.cpp"
  "${_src}/apps/controller/pages/RecentConnectionsPage.h"
  "${_src}/apps/controller/pages/RoomPage.cpp"
  "${_src}/apps/controller/pages/RoomPage.ActiveRoom.cpp"
  "${_src}/apps/controller/pages/RoomPage.h"
  "${_src}/apps/controller/pages/SettingsPage.cpp"
  "${_src}/apps/controller/pages/SettingsPage.ContentAwareness.cpp"
  "${_src}/apps/controller/pages/SettingsPage.FileTransfer.cpp"
  "${_src}/apps/controller/pages/SettingsPage.Media.cpp"
  "${_src}/apps/controller/pages/SettingsPage.RemoteDesktop.cpp"
  "${_src}/apps/controller/pages/SettingsPage.RemotePaste.cpp"
  "${_src}/apps/controller/pages/SettingsPage.Shortcuts.cpp"
  "${_src}/apps/controller/pages/SettingsPage.h"
  "${_src}/apps/controller/pages/SettingsPageUi.cpp"
  "${_src}/apps/controller/pages/SettingsPageUi.h"
  "${_src}/apps/controller/RemoteDesktopCanvas.cpp"
  "${_src}/apps/controller/RemoteDesktopCanvas.h"
  "${_src}/apps/controller/FileTransferWindow.cpp"
  "${_src}/apps/controller/FramelessWindow.cpp"
  "${_src}/apps/controller/RemoteCDialog.cpp"
  "${_src}/apps/controller/RemoteCComboBox.cpp"
  "${_src}/apps/controller/RemoteCToast.cpp"
  "${_src}/apps/controller/RoundedPopupMenu.cpp"
  "${_src}/apps/controller/RemoteCursorRenderState.cpp"
  "${_src}/apps/controller/RemoteCursorRenderState.h"
  "${_src}/apps/controller/RemoteSessionActionTile.cpp"
  "${_src}/apps/controller/RemoteSessionActionTile.h"
  "${_src}/apps/controller/RemoteTransferStatusButton.cpp"
  "${_src}/apps/controller/RemoteTransferStatusButton.h"
  "${_src}/apps/controller/RemoteSessionNetworkIndicator.cpp"
  "${_src}/apps/controller/RemoteSessionNetworkIndicator.h"
  "${_src}/apps/controller/RemoteSessionWindow.Diagnostics.cpp"
  "${_src}/apps/controller/RemoteSessionWindow.DisplayMenu.cpp"
  "${_src}/apps/controller/RemoteSessionWindow.Layout.cpp"
  "${_src}/apps/controller/RemoteSessionWindow.Lifecycle.cpp"
  "${_src}/apps/controller/RemoteSessionWindow.MediaMenu.cpp"
  "${_src}/apps/controller/RemoteSessionWindow.MediaSelection.cpp"
  "${_src}/apps/controller/RemoteSessionWindow.SessionActions.cpp"
  "${_src}/apps/controller/RemoteSessionWindow.SessionState.cpp"
  "${_src}/apps/controller/RemoteSessionWindow.TransferProgress.cpp"
  "${_src}/apps/controller/RemoteSessionWindow.Ui.cpp"
  "${_src}/apps/controller/RemoteSessionWindow.WindowActions.cpp"
  "${_src}/apps/controller/RemoteSessionWindowConstants.h"
  "${_src}/apps/controller/RemoteSessionWindowHelpers.cpp"
  "${_src}/apps/controller/RemoteSessionWindowHelpers.h"
  "${_src}/apps/controller/RoomCameraWindow.cpp"
  "${_src}/platform/win/WindowsInputExecutor.cpp"
  "${_src}/apps/controller/RemoteCResources.qrc"
  "${CMAKE_SOURCE_DIR}/assets/branding/RemoteCApp.rc"
  "${_src}/apps/remote/RemoteCApp.manifest")
rlink_apply_common(RLinkAPP)
set_target_properties(RLinkAPP PROPERTIES
  AUTOMOC ON AUTOUIC ON AUTORCC ON
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
target_link_libraries(RLinkAPP PRIVATE
  rlink_session_engine rlink_auth rlink_vision_api_adapters
  Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::WebSockets Qt6::NetworkAuth
  d3dcompiler)
target_compile_definitions(RLinkAPP PRIVATE
  RLINK_ENABLE_CONTENT_ANALYZER=$<BOOL:${RLINK_ENABLE_CONTENT_ANALYZER}>
  RLINK_ENABLE_REMOTE_VISION_API=$<BOOL:${RLINK_ENABLE_REMOTE_VISION_API}>)
target_link_options(RLinkAPP PRIVATE
  "/ENTRY:mainCRTStartup"
  "/MAP:$<TARGET_FILE_DIR:RLinkAPP>/RLinkAPP.map")
add_custom_command(TARGET RLinkAPP POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "$<TARGET_FILE:rlink_bundled_turbojpeg>"
    "$<TARGET_FILE_DIR:RLinkAPP>"
  VERBATIM)
rlink_deploy_qt(RLinkAPP)
rlink_copy_ffmpeg_runtime(RLinkAPP)
rlink_copy_licenses(RLinkAPP)

# Focused component test. It is excluded from the default application build
# and can be built without starting Qt or WebRTC runtime services.
add_executable(MediaIntelligenceSelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/MediaIntelligenceSelfTest.cpp")
rlink_apply_common(MediaIntelligenceSelfTest)
set_target_properties(MediaIntelligenceSelfTest PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
target_link_libraries(MediaIntelligenceSelfTest PRIVATE
  media_intelligence_runtime)

add_executable(ContentAwareStreamPolicySelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/ContentAwareStreamPolicySelfTest.cpp")
rlink_apply_common(ContentAwareStreamPolicySelfTest)
set_target_properties(ContentAwareStreamPolicySelfTest PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
target_link_libraries(ContentAwareStreamPolicySelfTest PRIVATE
  media_intelligence_core)

add_executable(GoogCcNetworkPressureSelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/GoogCcNetworkPressureSelfTest.cpp")
rlink_apply_common(GoogCcNetworkPressureSelfTest)
set_target_properties(GoogCcNetworkPressureSelfTest PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_link_libraries(GoogCcNetworkPressureSelfTest PRIVATE media_intelligence_core)

add_executable(GoogCcTelemetrySelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/GoogCcTelemetrySelfTest.cpp")
rlink_apply_common(GoogCcTelemetrySelfTest)
set_target_properties(GoogCcTelemetrySelfTest PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_link_libraries(GoogCcTelemetrySelfTest PRIVATE rlink_webrtc_transport)

# Actual localhost ICE/DTLS/RTP traffic exercises the injected controller gate.
# Mock controller tests cannot establish that the real Call uses this factory.
add_executable(GoogCcLiveTransportSelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/GoogCcLiveTransportSelfTest.cpp")
rlink_apply_common(GoogCcLiveTransportSelfTest)
set_target_properties(GoogCcLiveTransportSelfTest PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_link_libraries(GoogCcLiveTransportSelfTest PRIVATE
  rlink_webrtc_transport rlink_core)

# TransportPacketsFeedback has a copy constructor but no move constructor in
# this SDK. Explicit NRVO keeps the observer's ownership-transfer helper from
# adding a vector copy, including in debug builds. A storage-identity test checks
# the actual Release binary rather than assuming the optimizer removed it.
if(MSVC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.34)
  set_source_files_properties(
    "${_src}/webrtc/GoogCcTelemetry.cpp"
    "${_src}/testing/GoogCcTelemetrySelfTest.cpp"
    PROPERTIES COMPILE_OPTIONS "/Zc:nrvo")
endif()

add_executable(ContentAwarePolicyDiagnosticsSelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/ContentAwarePolicyDiagnosticsSelfTest.cpp"
  "${_src}/apps/remote/adapters/ContentAwarePolicyDiagnostics.cpp")
rlink_apply_common(ContentAwarePolicyDiagnosticsSelfTest)
set_target_properties(ContentAwarePolicyDiagnosticsSelfTest PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
target_link_libraries(ContentAwarePolicyDiagnosticsSelfTest PRIVATE
  media_intelligence_core)

# Real libwebrtc RTP transactions with synthetic, explicitly test-only evidence.
add_executable(ContentAwareStreamExecutionSelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/ContentAwareStreamExecutionSelfTest.cpp")
rlink_apply_common(ContentAwareStreamExecutionSelfTest)
set_target_properties(ContentAwareStreamExecutionSelfTest PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_link_libraries(ContentAwareStreamExecutionSelfTest PRIVATE
  rlink_webrtc_transport rlink_core)

add_executable(ScreenReceiverFeedbackProtocolSelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/ScreenReceiverFeedbackProtocolSelfTest.cpp")
rlink_apply_common(ScreenReceiverFeedbackProtocolSelfTest)
target_link_libraries(ScreenReceiverFeedbackProtocolSelfTest PRIVATE rlink_core)

add_executable(ScreenReceiverFeedbackPipelineSelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/ScreenReceiverFeedbackPipelineSelfTest.cpp")
rlink_apply_common(ScreenReceiverFeedbackPipelineSelfTest)
set_target_properties(ScreenReceiverFeedbackPipelineSelfTest PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_link_libraries(ScreenReceiverFeedbackPipelineSelfTest PRIVATE
  rlink_webrtc_transport rlink_core)

add_executable(ScreenFeedbackPreferenceSelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/ScreenFeedbackPreferenceSelfTest.cpp")
rlink_apply_common(ScreenFeedbackPreferenceSelfTest)
set_target_properties(ScreenFeedbackPreferenceSelfTest PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_link_libraries(ScreenFeedbackPreferenceSelfTest PRIVATE rlink_session_engine)

add_executable(CalibratedStreamQualityModelSelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/CalibratedStreamQualityModelSelfTest.cpp")
media_intelligence_apply_common(CalibratedStreamQualityModelSelfTest)
target_link_libraries(CalibratedStreamQualityModelSelfTest PRIVATE media_intelligence_core)

add_executable(H264ReferenceQualityModelSelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/H264ReferenceQualityModelSelfTest.cpp")
media_intelligence_apply_common(H264ReferenceQualityModelSelfTest)
set_target_properties(H264ReferenceQualityModelSelfTest PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_link_libraries(H264ReferenceQualityModelSelfTest PRIVATE media_intelligence_core)

add_executable(EncoderQualityEvidenceSelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/EncoderQualityEvidenceSelfTest.cpp")
rlink_apply_common(EncoderQualityEvidenceSelfTest)
target_link_libraries(EncoderQualityEvidenceSelfTest PRIVATE rlink_webrtc_transport)

add_executable(ContentAwarePolicyExample EXCLUDE_FROM_ALL
  "${CMAKE_SOURCE_DIR}/examples/content_aware_policy/main.cpp")
media_intelligence_apply_common(ContentAwarePolicyExample)
set_target_properties(ContentAwarePolicyExample PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
target_link_libraries(ContentAwarePolicyExample PRIVATE
  RLinkMediaIntelligence::core)

add_executable(FfmpegHardwareBitrateProbe EXCLUDE_FROM_ALL
  "${_src}/testing/FfmpegHardwareBitrateProbe.cpp"
  "${_src}/core/ScreenStreamPolicy.cpp")
rlink_apply_common(FfmpegHardwareBitrateProbe)
set_target_properties(FfmpegHardwareBitrateProbe PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_link_libraries(FfmpegHardwareBitrateProbe PRIVATE rlink_webrtc_transport)

add_executable(FfmpegHardwareRateControlSelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/FfmpegHardwareRateControlSelfTest.cpp"
  "${_src}/platform/win/FfmpegHardwareRateControl.cpp")
rlink_apply_common(FfmpegHardwareRateControlSelfTest)
set_target_properties(FfmpegHardwareRateControlSelfTest PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

add_executable(ScreenStreamPolicySelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/ScreenStreamPolicySelfTest.cpp"
  "${_src}/core/ScreenNetworkPolicy.cpp"
  "${_src}/core/ScreenStreamPolicy.cpp")
rlink_apply_common(ScreenStreamPolicySelfTest)
set_target_properties(ScreenStreamPolicySelfTest PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_link_libraries(ScreenStreamPolicySelfTest PRIVATE media_intelligence_core)

add_executable(ScreenFrameQualitySelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/ScreenFrameQualitySelfTest.cpp")
rlink_apply_common(ScreenFrameQualitySelfTest)
set_target_properties(ScreenFrameQualitySelfTest PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_link_libraries(ScreenFrameQualitySelfTest PRIVATE rlink_webrtc_transport)

add_executable(VisionApiSelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/VisionApiSelfTest.cpp")
rlink_apply_common(VisionApiSelfTest)
set_target_properties(VisionApiSelfTest PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
target_link_libraries(VisionApiSelfTest PRIVATE
  media_intelligence_openai_compatible)

add_executable(VisionApiAdaptersSelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/VisionApiAdaptersSelfTest.cpp")
rlink_apply_common(VisionApiAdaptersSelfTest)
set_target_properties(VisionApiAdaptersSelfTest PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
target_link_libraries(VisionApiAdaptersSelfTest PRIVATE
  rlink_vision_api_adapters
  Qt6::Core Qt6::Network)
add_custom_command(TARGET VisionApiAdaptersSelfTest POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "$<TARGET_FILE:rlink_bundled_turbojpeg>"
    "$<TARGET_FILE_DIR:VisionApiAdaptersSelfTest>"
  VERBATIM)

add_executable(VisionApiClassifierExample EXCLUDE_FROM_ALL
  "${CMAKE_SOURCE_DIR}/examples/vision_api_classifier/main.cpp")
set_target_properties(VisionApiClassifierExample PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
target_link_libraries(VisionApiClassifierExample PRIVATE
  RLinkMediaIntelligence::openai_compatible)

# RemoteCSignalServer: Qt HTTP/WebSocket signaling server (console).
add_executable(RemoteCSignalServer
  "${_src}/server/auth/LogtoUserInfoClient.cpp"
  "${_src}/server/auth/LogtoManagementClient.cpp"
  "${_src}/server/auth/LogtoWebhookServer.cpp"
  "${_src}/server/persistence/IdentityStore.cpp"
  "${_src}/server/signaling/DirectSessionRegistry.cpp"
  "${_src}/server/signaling/DirectSessionRegistry.h"
  "${_src}/server/signaling/RoomRegistry.cpp"
  "${_src}/server/signaling/RoomRegistry.h"
  "${_src}/server/signaling/AccessTokenService.cpp"
  "${_src}/server/signaling/SignalServer.cpp"
  "${_src}/server/signaling/SignalServer.AccountManagement.cpp"
  "${_src}/server/signaling/SignalServer.Authentication.cpp"
  "${_src}/server/signaling/SignalServer.Connection.cpp"
  "${_src}/server/signaling/SignalServer.Internal.h"
  "${_src}/server/signaling/SignalServer.LegacySession.cpp"
  "${_src}/server/signaling/SignalServer.Lifecycle.cpp"
  "${_src}/server/signaling/SignalServer.OwnedDevices.cpp"
  "${_src}/server/signaling/SignalServer.Recovery.cpp"
  "${_src}/server/signaling/SignalServer.Relay.cpp"
  "${_src}/server/signaling/SignalServer.RoomLeases.cpp"
  "${_src}/server/signaling/SignalServer.RoomMembership.cpp"
  "${_src}/server/signaling/SignalServer.RoomPairs.cpp"
  "${_src}/server/signaling/SignalServer.RoomState.cpp"
  "${_src}/server/signaling/SignalServerSupport.cpp"
  "${_src}/server/signaling/SignalServerMain.cpp")
rlink_apply_common(RemoteCSignalServer)
set_target_properties(RemoteCSignalServer PROPERTIES
  AUTOMOC ON AUTOUIC ON AUTORCC ON
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
target_link_libraries(RemoteCSignalServer PRIVATE
  Qt6::Core Qt6::Network Qt6::WebSockets Qt6::HttpServer Qt6::Sql)
rlink_deploy_qt(RemoteCSignalServer)

# RLinkUpdater: standalone self-contained updater (no Qt, static CRT /MT).
add_executable(RLinkUpdater WIN32
  "${_src}/apps/update/RLinkUpdaterMain.cpp"
  "${_src}/apps/update/RLinkUpdater.manifest")
set_target_properties(RLinkUpdater PROPERTIES
  MSVC_RUNTIME_LIBRARY "MultiThreaded"
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
if(MSVC)
  target_compile_options(RLinkUpdater PRIVATE /utf-8 /Zc:__cplusplus /W4 /permissive- /MP)
  target_compile_definitions(RLinkUpdater PRIVATE
    WIN32 _WINDOWS UNICODE _UNICODE NOMINMAX)
endif()
target_link_libraries(RLinkUpdater PRIVATE bcrypt dwmapi winhttp)

# UI-only responsiveness regression probe; excluded from normal product builds.
add_executable(DiagnosticsUiSelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/DiagnosticsUiSelfTest.cpp"
  "${_src}/apps/controller/pages/DiagnosticsCardsWidget.cpp"
  "${_src}/apps/controller/pages/DiagnosticsPage.cpp"
  "${_src}/apps/controller/pages/DiagnosticsPage.Workspace.cpp"
  "${_src}/apps/controller/CurrentPageStack.cpp"
  "${_src}/apps/controller/pages/RecentConnectionsPage.cpp"
  "${_src}/apps/controller/MediaControls.cpp"
  "${_src}/apps/controller/FramelessWindow.cpp"
  "${_src}/apps/controller/ui/RemoteCTheme.cpp"
  "${_src}/apps/controller/ui/morph/MorphIconCore.cpp"
  "${_src}/apps/controller/ui/morph/MorphIconButtonBinding.cpp"
  "${_src}/apps/controller/RemoteCResources.qrc")
rlink_apply_common(DiagnosticsUiSelfTest)
target_compile_definitions(DiagnosticsUiSelfTest PRIVATE
  RLINK_UI_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
target_link_libraries(DiagnosticsUiSelfTest PRIVATE Qt6::Widgets dwmapi)

# Synthetic camera presentation probe; does not open devices or start sessions.
add_executable(RoomCameraUiSelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/RoomCameraUiSelfTest.cpp"
  "${_src}/apps/controller/RoomCameraWindow.cpp"
  "${_src}/apps/controller/FramelessWindow.cpp"
  "${_src}/apps/controller/ui/RemoteCTheme.cpp"
  "${_src}/apps/controller/ui/morph/MorphIconCore.cpp"
  "${_src}/apps/controller/ui/morph/MorphIconButtonBinding.cpp"
  "${_src}/apps/controller/RemoteCResources.qrc")
rlink_apply_common(RoomCameraUiSelfTest)
target_link_libraries(RoomCameraUiSelfTest PRIVATE Qt6::Widgets rlink_webrtc dwmapi)

# Deterministic transport queue and UI completion probes; no network/device use.
# SessionControllerQueueSelfTest includes SessionController.cpp in its TU to
# exercise the private executor. Do not also link rlink_core into this target.
add_executable(SessionControllerQueueSelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/SessionControllerQueueSelfTest.cpp")
rlink_apply_common(SessionControllerQueueSelfTest)
set_target_properties(SessionControllerQueueSelfTest PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

add_executable(StreamPreferenceRequestStateSelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/StreamPreferenceRequestStateSelfTest.cpp")
rlink_apply_common(StreamPreferenceRequestStateSelfTest)
set_target_properties(StreamPreferenceRequestStateSelfTest PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_link_libraries(StreamPreferenceRequestStateSelfTest PRIVATE Qt6::Core)

add_executable(StreamPreferenceEngineSelfTest EXCLUDE_FROM_ALL
  "${_src}/testing/StreamPreferenceEngineSelfTest.cpp")
rlink_apply_common(StreamPreferenceEngineSelfTest)
set_target_properties(StreamPreferenceEngineSelfTest PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_link_libraries(StreamPreferenceEngineSelfTest PRIVATE rlink_session_engine)
