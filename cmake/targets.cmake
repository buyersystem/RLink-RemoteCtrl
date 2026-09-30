# Target definitions for the RemoteC components and applications.
#
# Windows-only for now; platform-specific sources are grouped so a future
# cross-platform port can add e.g. src/platform/posix alongside src/platform/win.

set(_src "${CMAKE_SOURCE_DIR}/src")

# ---------------------------------------------------------------------------
# Static libraries
# ---------------------------------------------------------------------------

# Media intelligence is split from RLink adapters so the policy/runtime can
# be embedded by another native C++ application without Qt, WebRTC or D3D11.
add_library(media_intelligence_core STATIC
  "${_src}/media_intelligence/core/ContentState.cpp"
  "${_src}/media_intelligence/core/ContentState.h"
  "${_src}/media_intelligence/core/ContentMotionAnalyzer.cpp"
  "${_src}/media_intelligence/core/ContentMotionAnalyzer.h")
rlink_apply_common(media_intelligence_core)
set_target_properties(media_intelligence_core PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

add_library(media_intelligence_runtime STATIC
  "${_src}/media_intelligence/runtime/ContentAnalysisWorker.cpp"
  "${_src}/media_intelligence/runtime/ContentAnalysisWorker.h")
rlink_apply_common(media_intelligence_runtime)
set_target_properties(media_intelligence_runtime PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
target_link_libraries(media_intelligence_runtime PUBLIC
  media_intelligence_core)

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
  "${_src}/platform/win/FfmpegHardwareH264EncoderFactory.cpp"
  "${_src}/platform/win/FfmpegX264H264Encoder.cpp"
  "${_src}/platform/win/FfmpegX264H264EncoderFactory.cpp"
  "${_src}/platform/win/QualityOpenH264Encoder.cpp"
  "${_src}/platform/win/H264EncoderBenchmark.cpp"
  "${_src}/platform/win/MfD3D11H264DecoderBenchmark.cpp"
  "${_src}/platform/win/MfD3D11H264DecoderFactory.cpp"
  "${_src}/webrtc/LibWebRtcSession.cpp"
  "${_src}/webrtc/LibWebRtcSession.AudioSlot.cpp"
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
  "${_src}/webrtc/WindowsPreferredVideoDecoderFactory.cpp"
  "${_src}/webrtc/WindowsPreferredVideoEncoderFactory.cpp")
rlink_apply_common(rlink_webrtc_transport)
target_link_libraries(rlink_webrtc_transport PUBLIC rlink_webrtc rlink_ffmpeg)
target_link_libraries(rlink_webrtc_transport PUBLIC
  media_intelligence_runtime)

# RemoteSessionEngine: in-process session engine + Windows platform services.
add_library(rlink_session_engine STATIC
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
  rlink_session_engine rlink_auth
  Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::WebSockets Qt6::NetworkAuth
  d3dcompiler)
target_compile_definitions(RLinkAPP PRIVATE
  RLINK_ENABLE_CONTENT_ANALYZER=$<BOOL:${RLINK_ENABLE_CONTENT_ANALYZER}>)
target_link_options(RLinkAPP PRIVATE
  "/ENTRY:mainCRTStartup"
  "/MAP:$<TARGET_FILE_DIR:RLinkAPP>/RLinkAPP.map")
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
