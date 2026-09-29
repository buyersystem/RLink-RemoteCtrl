# 应用层：主界面与远程会话窗口

> 自动生成于 2026-09-28，源码树 `f12aea4209d9-dirty`。请运行 `tools/Generate-SourceSymbolReference.ps1` 刷新。

Qt 主窗口、远程会话窗口、画布、文件窗口、主题与交互控件。

本册共收录 107 个源码文件。函数与变量的中文作用优先采用源码紧邻注释；无注释时根据符号命名生成阅读提示，最终语义仍以源码为准。

## `src/apps/controller/CameraWindow.cpp`

[打开源码](../src/apps/controller/CameraWindow.cpp) · **文件作用：** 实现 camera window 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L132](../src/apps/controller/CameraWindow.cpp#L132) | `CameraPreviewWidget::CameraPreviewWidget` | 定义 | `CameraPreviewWidget::CameraPreviewWidget(QWidget* parent) : QWidget(parent)` | 构造并初始化 CameraPreviewWidget 实例。 |
| [L138](../src/apps/controller/CameraWindow.cpp#L138) | `CameraPreviewWidget::paintEvent` | 定义 | `void CameraPreviewWidget::paintEvent(QPaintEvent*)` | 准备或呈现 paint event 相关逻辑。 |
| [L190](../src/apps/controller/CameraWindow.cpp#L190) | `CameraOverlayWidget::CameraOverlayWidget` | 定义 | `CameraOverlayWidget::CameraOverlayWidget( QString deviceName, std::function<void()> detachAction, std::function<void()> closeAction, QWidget* parent) : QWidget(parent)` | 构造并初始化 CameraOverlayWidget 实例。 |
| [L288](../src/apps/controller/CameraWindow.cpp#L288) | `CameraOverlayWidget::eventFilter` | 定义 | `bool CameraOverlayWidget::eventFilter(QObject* watched, QEvent* event)` | 实现 event filter 对应的业务或工具逻辑。 |
| [L318](../src/apps/controller/CameraWindow.cpp#L318) | `CameraWindow::CameraWindow` | 定义 | `CameraWindow::CameraWindow(QString deviceId, QString deviceName, std::function<void()> reattachAction, QWidget* parent) : FramelessMainWindow(parent), deviceId_(std::move(deviceId)), deviceName_(std::move(deviceName))...` | 构造并初始化 CameraWindow 实例。 |
| [L330](../src/apps/controller/CameraWindow.cpp#L330) | `CameraWindow::BuildUi` | 定义 | `void CameraWindow::BuildUi()` | 创建或初始化 build ui 相关逻辑。 |
| [L492](../src/apps/controller/CameraWindow.cpp#L492) | `CameraWindow::ReturnToOverlay` | 定义 | `void CameraWindow::ReturnToOverlay()` | 实现 return to overlay 对应的业务或工具逻辑。 |

## `src/apps/controller/CameraWindow.h`

[打开源码](../src/apps/controller/CameraWindow.h) · **文件作用：** 声明 camera window 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L15](../src/apps/controller/CameraWindow.h#L15) | `CameraPreviewWidget` | class | 定义 CameraPreviewWidget 的 class 类型和相关状态。 |
| [L23](../src/apps/controller/CameraWindow.h#L23) | `CameraOverlayWidget` | class | 定义 CameraOverlayWidget 的 class 类型和相关状态。 |
| [L39](../src/apps/controller/CameraWindow.h#L39) | `CameraWindow` | class | 定义 CameraWindow 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L34](../src/apps/controller/CameraWindow.h#L34) | `dragHandle_` | `QWidget* dragHandle_ = nullptr;` | 保存 drag handle 相关配置或运行状态。 |
| [L35](../src/apps/controller/CameraWindow.h#L35) | `dragOffset_` | `QPoint dragOffset_;` | 保存 drag offset 相关配置或运行状态。 |
| [L36](../src/apps/controller/CameraWindow.h#L36) | `dragging_` | `bool dragging_ = false;` | 保存 dragging 相关配置或运行状态。 |
| [L54](../src/apps/controller/CameraWindow.h#L54) | `deviceId_` | `QString deviceId_;` | 保存身份或作用域标识：device id。 |
| [L55](../src/apps/controller/CameraWindow.h#L55) | `deviceName_` | `QString deviceName_;` | 保存路径、地址或显示名称：device name。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L17](../src/apps/controller/CameraWindow.h#L17) | `CameraPreviewWidget` | 声明 | `explicit CameraPreviewWidget(QWidget* parent = nullptr)` | 实现 camera preview widget 对应的业务或工具逻辑。 |
| [L20](../src/apps/controller/CameraWindow.h#L20) | `paintEvent` | 声明 | `void paintEvent(QPaintEvent* event) override` | 准备或呈现 paint event 相关逻辑。 |
| [L25](../src/apps/controller/CameraWindow.h#L25) | `CameraOverlayWidget` | 声明 | `CameraOverlayWidget(QString deviceName, std::function<void()> detachAction, std::function<void()> closeAction, QWidget* parent = nullptr)` | 实现 camera overlay widget 对应的业务或工具逻辑。 |
| [L31](../src/apps/controller/CameraWindow.h#L31) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L41](../src/apps/controller/CameraWindow.h#L41) | `CameraWindow` | 定义 | `CameraWindow(QString deviceId, QString deviceName, std::function<void()> reattachAction = {},` | 实现 camera window 对应的业务或工具逻辑。 |
| [L45](../src/apps/controller/CameraWindow.h#L45) | `~CameraWindow` | 声明 | `~CameraWindow() override = default` | 停止相关活动并释放 CameraWindow 实例拥有的资源。 |
| [L47](../src/apps/controller/CameraWindow.h#L47) | `CameraWindow` | 声明 | `CameraWindow(const CameraWindow&) = delete` | 实现 camera window 对应的业务或工具逻辑。 |
| [L51](../src/apps/controller/CameraWindow.h#L51) | `BuildUi` | 声明 | `void BuildUi()` | 创建或初始化 build ui 相关逻辑。 |
| [L52](../src/apps/controller/CameraWindow.h#L52) | `ReturnToOverlay` | 声明 | `void ReturnToOverlay()` | 实现 return to overlay 对应的业务或工具逻辑。 |

## `src/apps/controller/ControllerMainWindow.AppShell.cpp`

[打开源码](../src/apps/controller/ControllerMainWindow.AppShell.cpp) · **文件作用：** 实现 controller main window app shell 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L40](../src/apps/controller/ControllerMainWindow.AppShell.cpp#L40) | `ControllerMainWindow::ToggleAccountMenu` | 定义 | `void ControllerMainWindow::ToggleAccountMenu()` | 实现 toggle account menu 对应的业务或工具逻辑。 |
| [L47](../src/apps/controller/ControllerMainWindow.AppShell.cpp#L47) | `ControllerMainWindow::UpdateAccountMenuGeometry` | 定义 | `void ControllerMainWindow::UpdateAccountMenuGeometry()` | 更新或应用 update account menu geometry 相关逻辑。 |
| [L66](../src/apps/controller/ControllerMainWindow.AppShell.cpp#L66) | `ControllerMainWindow::UpdateAccountMenuHoverFromCursor` | 定义 | `void ControllerMainWindow::UpdateAccountMenuHoverFromCursor()` | 更新或应用 update account menu hover from cursor 相关逻辑。 |
| [L85](../src/apps/controller/ControllerMainWindow.AppShell.cpp#L85) | `ControllerMainWindow::ShowAccountMenu` | 定义 | `void ControllerMainWindow::ShowAccountMenu()` | 实现 show account menu 对应的业务或工具逻辑。 |
| [L145](../src/apps/controller/ControllerMainWindow.AppShell.cpp#L145) | `ControllerMainWindow::HideAccountMenu` | 定义 | `void ControllerMainWindow::HideAccountMenu(bool animated)` | 实现 hide account menu 对应的业务或工具逻辑。 |
| [L169](../src/apps/controller/ControllerMainWindow.AppShell.cpp#L169) | `ControllerMainWindow::StartAccountMenuMotion` | 定义 | `void ControllerMainWindow::StartAccountMenuMotion( const QPoint& targetPosition, int durationMs, bool hideWhenFinished)` | 启动 start account menu motion 相关逻辑。 |
| [L215](../src/apps/controller/ControllerMainWindow.AppShell.cpp#L215) | `ControllerMainWindow::StopAccountMenuMotion` | 定义 | `void ControllerMainWindow::StopAccountMenuMotion()` | 停止 stop account menu motion 相关逻辑。 |
| [L222](../src/apps/controller/ControllerMainWindow.AppShell.cpp#L222) | `ControllerMainWindow::BuildSystemTray` | 定义 | `void ControllerMainWindow::BuildSystemTray()` | 创建或初始化 build system tray 相关逻辑。 |
| [L310](../src/apps/controller/ControllerMainWindow.AppShell.cpp#L310) | `ControllerMainWindow::ShowFromSystemTray` | 定义 | `void ControllerMainWindow::ShowFromSystemTray()` | 实现 show from system tray 对应的业务或工具逻辑。 |
| [L338](../src/apps/controller/ControllerMainWindow.AppShell.cpp#L338) | `ControllerMainWindow::QuitFromSystemTray` | 定义 | `void ControllerMainWindow::QuitFromSystemTray()` | 实现 quit from system tray 对应的业务或工具逻辑。 |
| [L358](../src/apps/controller/ControllerMainWindow.AppShell.cpp#L358) | `ControllerMainWindow::DestroyAuxiliaryWindowsForExit` | 定义 | `void ControllerMainWindow::DestroyAuxiliaryWindowsForExit()` | 关闭并清理 destroy auxiliary windows for exit 相关逻辑。 |
| [L374](../src/apps/controller/ControllerMainWindow.AppShell.cpp#L374) | `ControllerMainWindow::CheckForSoftwareUpdates` | 定义 | `void ControllerMainWindow::CheckForSoftwareUpdates(bool manualRequest)` | 校验 check for software updates 相关逻辑。 |
| [L387](../src/apps/controller/ControllerMainWindow.AppShell.cpp#L387) | `ControllerMainWindow::HandleSoftwareUpdateState` | 定义 | `void ControllerMainWindow::HandleSoftwareUpdateState( const update::SoftwareUpdateController::Snapshot& snapshot)` | 接收并处理 handle software update state 相关逻辑。 |
| [L472](../src/apps/controller/ControllerMainWindow.AppShell.cpp#L472) | `ControllerMainWindow::OpenSoftwareUpdate` | 定义 | `void ControllerMainWindow::OpenSoftwareUpdate()` | 启动 open software update 相关逻辑。 |
| [L531](../src/apps/controller/ControllerMainWindow.AppShell.cpp#L531) | `ControllerMainWindow::QuitForSoftwareUpdate` | 定义 | `void ControllerMainWindow::QuitForSoftwareUpdate()` | 实现 quit for software update 对应的业务或工具逻辑。 |

## `src/apps/controller/ControllerMainWindow.cpp`

[打开源码](../src/apps/controller/ControllerMainWindow.cpp) · **文件作用：** 实现 controller main window 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L564](../src/apps/controller/ControllerMainWindow.cpp#L564) | `ScrollPosition` | struct | 定义 ScrollPosition 的 struct 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L74](../src/apps/controller/ControllerMainWindow.cpp#L74) | `ControllerMainWindow::SettingsControls` | 定义 | `SettingsPageControls& ControllerMainWindow::SettingsControls()` | 更新或应用 settings controls 相关逻辑。 |
| [L80](../src/apps/controller/ControllerMainWindow.cpp#L80) | `ControllerMainWindow::RoomControls` | 定义 | `RoomPageControls& ControllerMainWindow::RoomControls()` | 实现 room controls 对应的业务或工具逻辑。 |
| [L86](../src/apps/controller/ControllerMainWindow.cpp#L86) | `ControllerMainWindow::ControllerMainWindow` | 定义 | `ControllerMainWindow::ControllerMainWindow( std::unique_ptr<ISessionEngine> engine, bool startEngineImmediately, app::ISessionMediaAccess* sessionMedia, QWidget* parent) : FramelessMainWindow(parent), inputExecutor_(s...` | 构造并初始化 ControllerMainWindow 实例。 |
| [L317](../src/apps/controller/ControllerMainWindow.cpp#L317) | `ControllerMainWindow::~ControllerMainWindow` | 定义 | `ControllerMainWindow::~ControllerMainWindow()` | 停止相关活动并释放 ControllerMainWindow 实例拥有的资源。 |
| [L351](../src/apps/controller/ControllerMainWindow.cpp#L351) | `ControllerMainWindow::ActivateFromExternalLaunch` | 定义 | `void ControllerMainWindow::ActivateFromExternalLaunch()` | 实现 activate from external launch 对应的业务或工具逻辑。 |
| [L356](../src/apps/controller/ControllerMainWindow.cpp#L356) | `ControllerMainWindow::SetAccountInteractionCallback` | 定义 | `void ControllerMainWindow::SetAccountInteractionCallback( std::function<void()> callback)` | 更新或应用 set account interaction callback 相关逻辑。 |
| [L362](../src/apps/controller/ControllerMainWindow.cpp#L362) | `ControllerMainWindow::SetAccountSwitchCallback` | 定义 | `void ControllerMainWindow::SetAccountSwitchCallback( std::function<void()> callback)` | 更新或应用 set account switch callback 相关逻辑。 |
| [L368](../src/apps/controller/ControllerMainWindow.cpp#L368) | `ControllerMainWindow::SetAccountDeletionCallback` | 定义 | `void ControllerMainWindow::SetAccountDeletionCallback( std::function<void()> callback)` | 更新或应用 set account deletion callback 相关逻辑。 |
| [L374](../src/apps/controller/ControllerMainWindow.cpp#L374) | `ControllerMainWindow::SetAccountSignedOut` | 定义 | `void ControllerMainWindow::SetAccountSignedOut(const QString& message)` | 更新或应用 set account signed out 相关逻辑。 |
| [L411](../src/apps/controller/ControllerMainWindow.cpp#L411) | `ControllerMainWindow::SetAccountBusy` | 定义 | `void ControllerMainWindow::SetAccountBusy(const QString& message)` | 更新或应用 set account busy 相关逻辑。 |
| [L428](../src/apps/controller/ControllerMainWindow.cpp#L428) | `ControllerMainWindow::SetAccountSession` | 定义 | `void ControllerMainWindow::SetAccountSession( const QString& accountId, const QString& accountLabel, const QString& accountDetail, std::function<void()> signOutCallback)` | 更新或应用 set account session 相关逻辑。 |
| [L498](../src/apps/controller/ControllerMainWindow.cpp#L498) | `ControllerMainWindow::StartSessionEngine` | 定义 | `bool ControllerMainWindow::StartSessionEngine()` | 启动 start session engine 相关逻辑。 |
| [L512](../src/apps/controller/ControllerMainWindow.cpp#L512) | `ControllerMainWindow::StopSessionEngine` | 定义 | `void ControllerMainWindow::StopSessionEngine()` | 停止 stop session engine 相关逻辑。 |
| [L522](../src/apps/controller/ControllerMainWindow.cpp#L522) | `ControllerMainWindow::PrepareForApplicationExit` | 定义 | `void ControllerMainWindow::PrepareForApplicationExit()` | 实现 prepare for application exit 对应的业务或工具逻辑。 |
| [L531](../src/apps/controller/ControllerMainWindow.cpp#L531) | `ControllerMainWindow::ApplyAuthenticationAvailability` | 定义 | `void ControllerMainWindow::ApplyAuthenticationAvailability(bool authenticated)` | 更新或应用 apply authentication availability 相关逻辑。 |
| [L542](../src/apps/controller/ControllerMainWindow.cpp#L542) | `ControllerMainWindow::RequestMediaDeviceRefresh` | 定义 | `void ControllerMainWindow::RequestMediaDeviceRefresh( bool userInitiated)` | 发起请求或查询 request media device refresh 相关逻辑。 |
| [L557](../src/apps/controller/ControllerMainWindow.cpp#L557) | `ControllerMainWindow::ApplyInterfaceTheme` | 定义 | `void ControllerMainWindow::ApplyInterfaceTheme(bool showFeedback)` | 更新或应用 apply interface theme 相关逻辑。 |
| [L727](../src/apps/controller/ControllerMainWindow.cpp#L727) | `ControllerMainWindow::SetInterfaceThemePreference` | 定义 | `void ControllerMainWindow::SetInterfaceThemePreference( const QString& value)` | 更新或应用 set interface theme preference 相关逻辑。 |
| [L735](../src/apps/controller/ControllerMainWindow.cpp#L735) | `ControllerMainWindow::RunThemeRoundTripSelfTest` | 定义 | `bool ControllerMainWindow::RunThemeRoundTripSelfTest(QString* errorMessage)` | 执行后台循环或调度 run theme round trip self test 相关逻辑。 |
| [L839](../src/apps/controller/ControllerMainWindow.cpp#L839) | `ControllerMainWindow::ShowMediaDeviceMenu` | 定义 | `void ControllerMainWindow::ShowMediaDeviceMenu( MediaDeviceKind kind, QWidget* anchor)` | 实现 show media device menu 对应的业务或工具逻辑。 |
| [L1133](../src/apps/controller/ControllerMainWindow.cpp#L1133) | `ControllerMainWindow::BeginMediaDeviceSelection` | 定义 | `void ControllerMainWindow::BeginMediaDeviceSelection( MediaDeviceKind kind, const QString& deviceId)` | 启动 begin media device selection 相关逻辑。 |
| [L1204](../src/apps/controller/ControllerMainWindow.cpp#L1204) | `ControllerMainWindow::eventFilter` | 定义 | `bool ControllerMainWindow::eventFilter(QObject* watched, QEvent* event)` | 实现 event filter 对应的业务或工具逻辑。 |
| [L1292](../src/apps/controller/ControllerMainWindow.cpp#L1292) | `ControllerMainWindow::nativeEvent` | 定义 | `bool ControllerMainWindow::nativeEvent( const QByteArray& eventType, void* message, qintptr* result)` | 实现 native event 对应的业务或工具逻辑。 |
| [L1312](../src/apps/controller/ControllerMainWindow.cpp#L1312) | `ControllerMainWindow::closeEvent` | 定义 | `void ControllerMainWindow::closeEvent(QCloseEvent* event)` | 关闭并清理 close event 相关逻辑。 |
| [L1343](../src/apps/controller/ControllerMainWindow.cpp#L1343) | `ControllerMainWindow::changeEvent` | 定义 | `void ControllerMainWindow::changeEvent(QEvent* event)` | 实现 change event 对应的业务或工具逻辑。 |
| [L1355](../src/apps/controller/ControllerMainWindow.cpp#L1355) | `ControllerMainWindow::resizeEvent` | 定义 | `void ControllerMainWindow::resizeEvent(QResizeEvent* event)` | 实现 resize event 对应的业务或工具逻辑。 |

## `src/apps/controller/ControllerMainWindow.DeviceRecentPages.cpp`

[打开源码](../src/apps/controller/ControllerMainWindow.DeviceRecentPages.cpp) · **文件作用：** 实现 controller main window device recent pages 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L12](../src/apps/controller/ControllerMainWindow.DeviceRecentPages.cpp#L12) | `ControllerMainWindow::BuildDeviceAndRecentPages` | 定义 | `void ControllerMainWindow::BuildDeviceAndRecentPages()` | 创建或初始化 build device and recent pages 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.Diagnostics.cpp`

[打开源码](../src/apps/controller/ControllerMainWindow.Diagnostics.cpp) · **文件作用：** 实现 controller main window diagnostics 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L35](../src/apps/controller/ControllerMainWindow.Diagnostics.cpp#L35) | `ControllerMainWindow::RefreshDiagnosticsUi` | 定义 | `void ControllerMainWindow::RefreshDiagnosticsUi()` | 刷新 refresh diagnostics ui 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.DiagnosticsPage.cpp`

[打开源码](../src/apps/controller/ControllerMainWindow.DiagnosticsPage.cpp) · **文件作用：** 实现 controller main window diagnostics page 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L13](../src/apps/controller/ControllerMainWindow.DiagnosticsPage.cpp#L13) | `ControllerMainWindow::BuildDiagnosticsPage` | 定义 | `void ControllerMainWindow::BuildDiagnosticsPage()` | 创建或初始化 build diagnostics page 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.h`

[打开源码](../src/apps/controller/ControllerMainWindow.h) · **文件作用：** 声明 controller main window 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L25](../src/apps/controller/ControllerMainWindow.h#L25) | `WindowsInputExecutor` | class | 定义 WindowsInputExecutor 的 class 类型和相关状态。 |
| [L27](../src/apps/controller/ControllerMainWindow.h#L27) | `FileTransferController` | class | 定义 FileTransferController 的 class 类型和相关状态。 |
| [L28](../src/apps/controller/ControllerMainWindow.h#L28) | `ISessionMediaAccess` | class | 定义 ISessionMediaAccess 的 class 类型和相关状态。 |
| [L32](../src/apps/controller/ControllerMainWindow.h#L32) | `QAction` | class | 定义 QAction 的 class 类型和相关状态。 |
| [L33](../src/apps/controller/ControllerMainWindow.h#L33) | `QCloseEvent` | class | 定义 QCloseEvent 的 class 类型和相关状态。 |
| [L34](../src/apps/controller/ControllerMainWindow.h#L34) | `QEvent` | class | 定义 QEvent 的 class 类型和相关状态。 |
| [L35](../src/apps/controller/ControllerMainWindow.h#L35) | `QFrame` | class | 定义 QFrame 的 class 类型和相关状态。 |
| [L36](../src/apps/controller/ControllerMainWindow.h#L36) | `QGraphicsOpacityEffect` | class | 定义 QGraphicsOpacityEffect 的 class 类型和相关状态。 |
| [L37](../src/apps/controller/ControllerMainWindow.h#L37) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L38](../src/apps/controller/ControllerMainWindow.h#L38) | `QLineEdit` | class | 定义 QLineEdit 的 class 类型和相关状态。 |
| [L39](../src/apps/controller/ControllerMainWindow.h#L39) | `QPropertyAnimation` | class | 定义 QPropertyAnimation 的 class 类型和相关状态。 |
| [L40](../src/apps/controller/ControllerMainWindow.h#L40) | `QProcess` | class | 定义 QProcess 的 class 类型和相关状态。 |
| [L41](../src/apps/controller/ControllerMainWindow.h#L41) | `QPoint` | class | 定义 QPoint 的 class 类型和相关状态。 |
| [L42](../src/apps/controller/ControllerMainWindow.h#L42) | `QPushButton` | class | 定义 QPushButton 的 class 类型和相关状态。 |
| [L43](../src/apps/controller/ControllerMainWindow.h#L43) | `QResizeEvent` | class | 定义 QResizeEvent 的 class 类型和相关状态。 |
| [L44](../src/apps/controller/ControllerMainWindow.h#L44) | `QStackedWidget` | class | 定义 QStackedWidget 的 class 类型和相关状态。 |
| [L45](../src/apps/controller/ControllerMainWindow.h#L45) | `QSystemTrayIcon` | class | 定义 QSystemTrayIcon 的 class 类型和相关状态。 |
| [L46](../src/apps/controller/ControllerMainWindow.h#L46) | `QTimer` | class | 定义 QTimer 的 class 类型和相关状态。 |
| [L47](../src/apps/controller/ControllerMainWindow.h#L47) | `QToolButton` | class | 定义 QToolButton 的 class 类型和相关状态。 |
| [L48](../src/apps/controller/ControllerMainWindow.h#L48) | `QVBoxLayout` | class | 定义 QVBoxLayout 的 class 类型和相关状态。 |
| [L52](../src/apps/controller/ControllerMainWindow.h#L52) | `CameraWindow` | class | 定义 CameraWindow 的 class 类型和相关状态。 |
| [L53](../src/apps/controller/ControllerMainWindow.h#L53) | `FileTransferWindow` | class | 定义 FileTransferWindow 的 class 类型和相关状态。 |
| [L54](../src/apps/controller/ControllerMainWindow.h#L54) | `DiagnosticsPage` | class | 定义 DiagnosticsPage 的 class 类型和相关状态。 |
| [L55](../src/apps/controller/ControllerMainWindow.h#L55) | `DirectConnectPage` | class | 定义 DirectConnectPage 的 class 类型和相关状态。 |
| [L56](../src/apps/controller/ControllerMainWindow.h#L56) | `OwnedDevicesPage` | class | 定义 OwnedDevicesPage 的 class 类型和相关状态。 |
| [L57](../src/apps/controller/ControllerMainWindow.h#L57) | `RecentConnectionsPage` | class | 定义 RecentConnectionsPage 的 class 类型和相关状态。 |
| [L58](../src/apps/controller/ControllerMainWindow.h#L58) | `RemoteSessionWindow` | class | 定义 RemoteSessionWindow 的 class 类型和相关状态。 |
| [L59](../src/apps/controller/ControllerMainWindow.h#L59) | `RoomPage` | class | 定义 RoomPage 的 class 类型和相关状态。 |
| [L60](../src/apps/controller/ControllerMainWindow.h#L60) | `RoomPageControls` | struct | 定义 RoomPageControls 的 struct 类型和相关状态。 |
| [L61](../src/apps/controller/ControllerMainWindow.h#L61) | `RoomCameraWindow` | class | 定义 RoomCameraWindow 的 class 类型和相关状态。 |
| [L62](../src/apps/controller/ControllerMainWindow.h#L62) | `SettingsPage` | class | 定义 SettingsPage 的 class 类型和相关状态。 |
| [L63](../src/apps/controller/ControllerMainWindow.h#L63) | `SettingsPageControls` | struct | 定义 SettingsPageControls 的 struct 类型和相关状态。 |
| [L65](../src/apps/controller/ControllerMainWindow.h#L65) | `ControllerMainWindow` | class | 定义 ControllerMainWindow 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L25](../src/apps/controller/ControllerMainWindow.h#L25) | `WindowsInputExecutor` | `class WindowsInputExecutor;` | 保存 windows input executor 相关配置或运行状态。 |
| [L27](../src/apps/controller/ControllerMainWindow.h#L27) | `FileTransferController` | `class FileTransferController;` | 保存 file transfer controller 相关配置或运行状态。 |
| [L28](../src/apps/controller/ControllerMainWindow.h#L28) | `ISessionMediaAccess` | `class ISessionMediaAccess;` | 保存 i session media access 相关配置或运行状态。 |
| [L32](../src/apps/controller/ControllerMainWindow.h#L32) | `QAction` | `class QAction;` | 保存 q action 相关配置或运行状态。 |
| [L33](../src/apps/controller/ControllerMainWindow.h#L33) | `QCloseEvent` | `class QCloseEvent;` | 保存 q close event 相关配置或运行状态。 |
| [L34](../src/apps/controller/ControllerMainWindow.h#L34) | `QEvent` | `class QEvent;` | 保存 q event 相关配置或运行状态。 |
| [L35](../src/apps/controller/ControllerMainWindow.h#L35) | `QFrame` | `class QFrame;` | 保存媒体帧、图像或缓冲资源：q frame。 |
| [L36](../src/apps/controller/ControllerMainWindow.h#L36) | `QGraphicsOpacityEffect` | `class QGraphicsOpacityEffect;` | 保存 q graphics opacity effect 相关配置或运行状态。 |
| [L37](../src/apps/controller/ControllerMainWindow.h#L37) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L38](../src/apps/controller/ControllerMainWindow.h#L38) | `QLineEdit` | `class QLineEdit;` | 保存 q line edit 相关配置或运行状态。 |
| [L39](../src/apps/controller/ControllerMainWindow.h#L39) | `QPropertyAnimation` | `class QPropertyAnimation;` | 保存 q property animation 相关配置或运行状态。 |
| [L40](../src/apps/controller/ControllerMainWindow.h#L40) | `QProcess` | `class QProcess;` | 保存 q process 相关配置或运行状态。 |
| [L41](../src/apps/controller/ControllerMainWindow.h#L41) | `QPoint` | `class QPoint;` | 保存 q point 相关配置或运行状态。 |
| [L42](../src/apps/controller/ControllerMainWindow.h#L42) | `QPushButton` | `class QPushButton;` | 保存 q push button 相关配置或运行状态。 |
| [L43](../src/apps/controller/ControllerMainWindow.h#L43) | `QResizeEvent` | `class QResizeEvent;` | 保存 q resize event 相关配置或运行状态。 |
| [L44](../src/apps/controller/ControllerMainWindow.h#L44) | `QStackedWidget` | `class QStackedWidget;` | 保存 q stacked widget 相关配置或运行状态。 |
| [L45](../src/apps/controller/ControllerMainWindow.h#L45) | `QSystemTrayIcon` | `class QSystemTrayIcon;` | 保存 q system tray icon 相关配置或运行状态。 |
| [L46](../src/apps/controller/ControllerMainWindow.h#L46) | `QTimer` | `class QTimer;` | 保存定时、截止或超时状态：q timer。 |
| [L47](../src/apps/controller/ControllerMainWindow.h#L47) | `QToolButton` | `class QToolButton;` | 保存 q tool button 相关配置或运行状态。 |
| [L48](../src/apps/controller/ControllerMainWindow.h#L48) | `QVBoxLayout` | `class QVBoxLayout;` | 保存 qv box layout 相关配置或运行状态。 |
| [L52](../src/apps/controller/ControllerMainWindow.h#L52) | `CameraWindow` | `class CameraWindow;` | 保存 camera window 相关配置或运行状态。 |
| [L53](../src/apps/controller/ControllerMainWindow.h#L53) | `FileTransferWindow` | `class FileTransferWindow;` | 保存 file transfer window 相关配置或运行状态。 |
| [L54](../src/apps/controller/ControllerMainWindow.h#L54) | `DiagnosticsPage` | `class DiagnosticsPage;` | 保存 diagnostics page 相关配置或运行状态。 |
| [L55](../src/apps/controller/ControllerMainWindow.h#L55) | `DirectConnectPage` | `class DirectConnectPage;` | 保存 direct connect page 相关配置或运行状态。 |
| [L56](../src/apps/controller/ControllerMainWindow.h#L56) | `OwnedDevicesPage` | `class OwnedDevicesPage;` | 保存 owned devices page 相关配置或运行状态。 |
| [L57](../src/apps/controller/ControllerMainWindow.h#L57) | `RecentConnectionsPage` | `class RecentConnectionsPage;` | 保存 recent connections page 相关配置或运行状态。 |
| [L58](../src/apps/controller/ControllerMainWindow.h#L58) | `RemoteSessionWindow` | `class RemoteSessionWindow;` | 保存 remote session window 相关配置或运行状态。 |
| [L59](../src/apps/controller/ControllerMainWindow.h#L59) | `RoomPage` | `class RoomPage;` | 保存 room page 相关配置或运行状态。 |
| [L60](../src/apps/controller/ControllerMainWindow.h#L60) | `RoomPageControls` | `struct RoomPageControls;` | 保存 room page controls 相关配置或运行状态。 |
| [L61](../src/apps/controller/ControllerMainWindow.h#L61) | `RoomCameraWindow` | `class RoomCameraWindow;` | 保存 room camera window 相关配置或运行状态。 |
| [L62](../src/apps/controller/ControllerMainWindow.h#L62) | `SettingsPage` | `class SettingsPage;` | 保存 settings page 相关配置或运行状态。 |
| [L63](../src/apps/controller/ControllerMainWindow.h#L63) | `SettingsPageControls` | `struct SettingsPageControls;` | 保存 settings page controls 相关配置或运行状态。 |
| [L211](../src/apps/controller/ControllerMainWindow.h#L211) | `inputExecutor_` | `std::unique_ptr<WindowsInputExecutor> inputExecutor_;` | 保存 input executor 相关配置或运行状态。 |
| [L212](../src/apps/controller/ControllerMainWindow.h#L212) | `fileTransferController_` | `std::unique_ptr<app::FileTransferController> fileTransferController_;` | 保存 file transfer controller 相关配置或运行状态。 |
| [L213](../src/apps/controller/ControllerMainWindow.h#L213) | `clipboardController_` | `std::unique_ptr<app::ClipboardController> clipboardController_;` | 保存 clipboard controller 相关配置或运行状态。 |
| [L215](../src/apps/controller/ControllerMainWindow.h#L215) | `softwareUpdateController_` | `softwareUpdateController_;` | 保存 software update controller 相关配置或运行状态。 |
| [L216](../src/apps/controller/ControllerMainWindow.h#L216) | `softwareUpdatePromptOpen_` | `bool softwareUpdatePromptOpen_ = false;` | 保存能力或开关状态：software update prompt open。 |
| [L217](../src/apps/controller/ControllerMainWindow.h#L217) | `engine_` | `std::unique_ptr<ISessionEngine> engine_;` | 保存 engine 相关配置或运行状态。 |
| [L218](../src/apps/controller/ControllerMainWindow.h#L218) | `sessionMedia_` | `app::ISessionMediaAccess* sessionMedia_ = nullptr;` | 保存 session media 相关配置或运行状态。 |
| [L220](../src/apps/controller/ControllerMainWindow.h#L220) | `kStopped` | `SessionEngineState::kStopped;` | 定义 stopped 的编译期常量或产品边界。 |
| [L221](../src/apps/controller/ControllerMainWindow.h#L221) | `assistedSessionTimeoutTimer_` | `QTimer* assistedSessionTimeoutTimer_ = nullptr;` | 保存定时、截止或超时状态：assisted session timeout timer。 |
| [L222](../src/apps/controller/ControllerMainWindow.h#L222) | `assistedSessionPending_` | `bool assistedSessionPending_ = false;` | 保存待处理队列或请求：assisted session pending。 |
| [L223](../src/apps/controller/ControllerMainWindow.h#L223) | `assistedSessionActive_` | `bool assistedSessionActive_ = false;` | 保存能力或开关状态：assisted session active。 |
| [L224](../src/apps/controller/ControllerMainWindow.h#L224) | `assistedSessionCancellationPending_` | `bool assistedSessionCancellationPending_ = false;` | 保存待处理队列或请求：assisted session cancellation pending。 |
| [L225](../src/apps/controller/ControllerMainWindow.h#L225) | `assistedSessionTimedOut_` | `bool assistedSessionTimedOut_ = false;` | 保存 assisted session timed out 相关配置或运行状态。 |
| [L226](../src/apps/controller/ControllerMainWindow.h#L226) | `ownedDeviceSessionPending_` | `bool ownedDeviceSessionPending_ = false;` | 保存待处理队列或请求：owned device session pending。 |
| [L227](../src/apps/controller/ControllerMainWindow.h#L227) | `lastDirectSessionToastError_` | `QString lastDirectSessionToastError_;` | 保存最近错误或失败原因：last direct session toast error。 |
| [L228](../src/apps/controller/ControllerMainWindow.h#L228) | `connectivityPill_` | `QLabel* connectivityPill_ = nullptr;` | 保存 connectivity pill 相关配置或运行状态。 |
| [L229](../src/apps/controller/ControllerMainWindow.h#L229) | `profileCard_` | `QFrame* profileCard_ = nullptr;` | 保存 profile card 相关配置或运行状态。 |
| [L230](../src/apps/controller/ControllerMainWindow.h#L230) | `profileAvatar_` | `QLabel* profileAvatar_ = nullptr;` | 保存 profile avatar 相关配置或运行状态。 |
| [L231](../src/apps/controller/ControllerMainWindow.h#L231) | `profileName_` | `QLabel* profileName_ = nullptr;` | 保存路径、地址或显示名称：profile name。 |
| [L232](../src/apps/controller/ControllerMainWindow.h#L232) | `serviceStatus_` | `QLabel* serviceStatus_ = nullptr;` | 保存状态机当前状态：service status。 |
| [L233](../src/apps/controller/ControllerMainWindow.h#L233) | `profileUpdateButton_` | `QToolButton* profileUpdateButton_ = nullptr;` | 保存 profile update button 相关配置或运行状态。 |
| [L234](../src/apps/controller/ControllerMainWindow.h#L234) | `accountMenu_` | `QFrame* accountMenu_ = nullptr;` | 保存 account menu 相关配置或运行状态。 |
| [L235](../src/apps/controller/ControllerMainWindow.h#L235) | `accountMenuAvatar_` | `QLabel* accountMenuAvatar_ = nullptr;` | 保存 account menu avatar 相关配置或运行状态。 |
| [L236](../src/apps/controller/ControllerMainWindow.h#L236) | `accountMenuName_` | `QLabel* accountMenuName_ = nullptr;` | 保存路径、地址或显示名称：account menu name。 |
| [L237](../src/apps/controller/ControllerMainWindow.h#L237) | `accountMenuDetail_` | `QLabel* accountMenuDetail_ = nullptr;` | 保存 account menu detail 相关配置或运行状态。 |
| [L238](../src/apps/controller/ControllerMainWindow.h#L238) | `softwareUpdateAction_` | `QPushButton* softwareUpdateAction_ = nullptr;` | 保存 software update action 相关配置或运行状态。 |
| [L239](../src/apps/controller/ControllerMainWindow.h#L239) | `accountMenuOpacity_` | `QGraphicsOpacityEffect* accountMenuOpacity_ = nullptr;` | 保存 account menu opacity 相关配置或运行状态。 |
| [L240](../src/apps/controller/ControllerMainWindow.h#L240) | `accountMenuMotionTimer_` | `QTimer* accountMenuMotionTimer_ = nullptr;` | 保存定时、截止或超时状态：account menu motion timer。 |
| [L241](../src/apps/controller/ControllerMainWindow.h#L241) | `accountMenuMotionClock_` | `QElapsedTimer accountMenuMotionClock_;` | 保护跨线程共享状态：account menu motion clock。 |
| [L242](../src/apps/controller/ControllerMainWindow.h#L242) | `accountMenuMotionStart_` | `QPoint accountMenuMotionStart_;` | 保存 account menu motion start 相关配置或运行状态。 |
| [L243](../src/apps/controller/ControllerMainWindow.h#L243) | `accountMenuMotionTarget_` | `QPoint accountMenuMotionTarget_;` | 保存 account menu motion target 相关配置或运行状态。 |
| [L244](../src/apps/controller/ControllerMainWindow.h#L244) | `accountMenuMotionDurationMs_` | `int accountMenuMotionDurationMs_ = 0;` | 保存 account menu motion duration ms 相关配置或运行状态。 |
| [L245](../src/apps/controller/ControllerMainWindow.h#L245) | `accountMenuMotionHiding_` | `bool accountMenuMotionHiding_ = false;` | 保存 account menu motion hiding 相关配置或运行状态。 |
| [L246](../src/apps/controller/ControllerMainWindow.h#L246) | `accountMenuHoverTimer_` | `QTimer* accountMenuHoverTimer_ = nullptr;` | 保存定时、截止或超时状态：account menu hover timer。 |
| [L247](../src/apps/controller/ControllerMainWindow.h#L247) | `roomPage_` | `RoomPage* roomPage_ = nullptr;` | 保存 room page 相关配置或运行状态。 |
| [L248](../src/apps/controller/ControllerMainWindow.h#L248) | `roomWorkspaceTargetActive_` | `bool roomWorkspaceTargetActive_ = false;` | 保存能力或开关状态：room workspace target active。 |
| [L249](../src/apps/controller/ControllerMainWindow.h#L249) | `roomWorkspaceTransitionPending_` | `bool roomWorkspaceTransitionPending_ = false;` | 保存待处理队列或请求：room workspace transition pending。 |
| [L250](../src/apps/controller/ControllerMainWindow.h#L250) | `roomWorkspaceTransitionRequest_` | `quint64 roomWorkspaceTransitionRequest_ = 0;` | 保存 room workspace transition request 相关配置或运行状态。 |
| [L251](../src/apps/controller/ControllerMainWindow.h#L251) | `renderedRoomMemberKey_` | `QString renderedRoomMemberKey_;` | 保存 rendered room member key 相关配置或运行状态。 |
| [L252](../src/apps/controller/ControllerMainWindow.h#L252) | `fileTransferNavButton_` | `QPushButton* fileTransferNavButton_ = nullptr;` | 保存 file transfer nav button 相关配置或运行状态。 |
| [L253](../src/apps/controller/ControllerMainWindow.h#L253) | `titleBar_` | `CustomTitleBar* titleBar_ = nullptr;` | 保存 title bar 相关配置或运行状态。 |
| [L254](../src/apps/controller/ControllerMainWindow.h#L254) | `pageStack_` | `QStackedWidget* pageStack_ = nullptr;` | 保存 page stack 相关配置或运行状态。 |
| [L255](../src/apps/controller/ControllerMainWindow.h#L255) | `settingsPage_` | `SettingsPage* settingsPage_ = nullptr;` | 保存 settings page 相关配置或运行状态。 |
| [L256](../src/apps/controller/ControllerMainWindow.h#L256) | `roomNavButton_` | `QPushButton* roomNavButton_ = nullptr;` | 保存 room nav button 相关配置或运行状态。 |
| [L257](../src/apps/controller/ControllerMainWindow.h#L257) | `deviceNavButton_` | `QPushButton* deviceNavButton_ = nullptr;` | 保存 device nav button 相关配置或运行状态。 |
| [L258](../src/apps/controller/ControllerMainWindow.h#L258) | `myDevicesNavButton_` | `QPushButton* myDevicesNavButton_ = nullptr;` | 保存 my devices nav button 相关配置或运行状态。 |
| [L259](../src/apps/controller/ControllerMainWindow.h#L259) | `recentNavButton_` | `QPushButton* recentNavButton_ = nullptr;` | 保存 recent nav button 相关配置或运行状态。 |
| [L260](../src/apps/controller/ControllerMainWindow.h#L260) | `debugNavButton_` | `QPushButton* debugNavButton_ = nullptr;` | 保存 debug nav button 相关配置或运行状态。 |
| [L261](../src/apps/controller/ControllerMainWindow.h#L261) | `settingsNavButton_` | `QPushButton* settingsNavButton_ = nullptr;` | 保存 settings nav button 相关配置或运行状态。 |
| [L262](../src/apps/controller/ControllerMainWindow.h#L262) | `helpNavButton_` | `QPushButton* helpNavButton_ = nullptr;` | 保存 help nav button 相关配置或运行状态。 |
| [L263](../src/apps/controller/ControllerMainWindow.h#L263) | `authorNavButton_` | `QPushButton* authorNavButton_ = nullptr;` | 保存 author nav button 相关配置或运行状态。 |
| [L264](../src/apps/controller/ControllerMainWindow.h#L264) | `pageNavigationButtons_` | `QVector<QPushButton*> pageNavigationButtons_;` | 保存 page navigation buttons 相关配置或运行状态。 |
| [L265](../src/apps/controller/ControllerMainWindow.h#L265) | `navigationIndicator_` | `QFrame* navigationIndicator_ = nullptr;` | 保存 navigation indicator 相关配置或运行状态。 |
| [L266](../src/apps/controller/ControllerMainWindow.h#L266) | `navigationAnimation_` | `QPropertyAnimation* navigationAnimation_ = nullptr;` | 保存 navigation animation 相关配置或运行状态。 |
| [L267](../src/apps/controller/ControllerMainWindow.h#L267) | `trayIcon_` | `QSystemTrayIcon* trayIcon_ = nullptr;` | 保存 tray icon 相关配置或运行状态。 |
| [L268](../src/apps/controller/ControllerMainWindow.h#L268) | `trayIdentityLabel_` | `QLabel* trayIdentityLabel_ = nullptr;` | 保存路径、地址或显示名称：tray identity label。 |
| [L269](../src/apps/controller/ControllerMainWindow.h#L269) | `traySignOutAction_` | `QAction* traySignOutAction_ = nullptr;` | 保存 tray sign out action 相关配置或运行状态。 |
| [L274](../src/apps/controller/ControllerMainWindow.h#L274) | `sessionEngineStarted_` | `bool sessionEngineStarted_ = false;` | 保存 session engine started 相关配置或运行状态。 |
| [L275](../src/apps/controller/ControllerMainWindow.h#L275) | `applicationExitPrepared_` | `bool applicationExitPrepared_ = false;` | 保存 application exit prepared 相关配置或运行状态。 |
| [L276](../src/apps/controller/ControllerMainWindow.h#L276) | `authenticationAvailable_` | `bool authenticationAvailable_ = true;` | 保存能力或开关状态：authentication available。 |
| [L277](../src/apps/controller/ControllerMainWindow.h#L277) | `videoPipelineSettingsBusy_` | `bool videoPipelineSettingsBusy_ = false;` | 保存 video pipeline settings busy 相关配置或运行状态。 |
| [L278](../src/apps/controller/ControllerMainWindow.h#L278) | `videoPipelineSettingsApplyPending_` | `bool videoPipelineSettingsApplyPending_ = false;` | 保存待处理队列或请求：video pipeline settings apply pending。 |
| [L279](../src/apps/controller/ControllerMainWindow.h#L279) | `decoderBenchmarkProcess_` | `QProcess* decoderBenchmarkProcess_ = nullptr;` | 保存 decoder benchmark process 相关配置或运行状态。 |
| [L280](../src/apps/controller/ControllerMainWindow.h#L280) | `decoderBenchmarkManualRequest_` | `bool decoderBenchmarkManualRequest_ = false;` | 保存 decoder benchmark manual request 相关配置或运行状态。 |
| [L281](../src/apps/controller/ControllerMainWindow.h#L281) | `decoderBenchmarkHardwareFingerprint_` | `QString decoderBenchmarkHardwareFingerprint_;` | 保存 decoder benchmark hardware fingerprint 相关配置或运行状态。 |
| [L282](../src/apps/controller/ControllerMainWindow.h#L282) | `encoderBenchmarkProcess_` | `QProcess* encoderBenchmarkProcess_ = nullptr;` | 保存 encoder benchmark process 相关配置或运行状态。 |
| [L283](../src/apps/controller/ControllerMainWindow.h#L283) | `encoderBenchmarkManualRequest_` | `bool encoderBenchmarkManualRequest_ = false;` | 保存 encoder benchmark manual request 相关配置或运行状态。 |
| [L284](../src/apps/controller/ControllerMainWindow.h#L284) | `encoderBenchmarkHardwareFingerprint_` | `QString encoderBenchmarkHardwareFingerprint_;` | 保存 encoder benchmark hardware fingerprint 相关配置或运行状态。 |
| [L285](../src/apps/controller/ControllerMainWindow.h#L285) | `encoderBenchmarkCaptureBackend_` | `QString encoderBenchmarkCaptureBackend_;` | 保存 encoder benchmark capture backend 相关配置或运行状态。 |
| [L286](../src/apps/controller/ControllerMainWindow.h#L286) | `encoderBenchmarkX264Preset_` | `QString encoderBenchmarkX264Preset_;` | 保存 encoder benchmark x264 preset 相关配置或运行状态。 |
| [L287](../src/apps/controller/ControllerMainWindow.h#L287) | `mediaDeviceRefreshDebounceTimer_` | `QTimer* mediaDeviceRefreshDebounceTimer_ = nullptr;` | 保存定时、截止或超时状态：media device refresh debounce timer。 |
| [L288](../src/apps/controller/ControllerMainWindow.h#L288) | `localDevicePage_` | `QWidget* localDevicePage_ = nullptr;` | 保存 local device page 相关配置或运行状态。 |
| [L289](../src/apps/controller/ControllerMainWindow.h#L289) | `directConnectPage_` | `DirectConnectPage* directConnectPage_ = nullptr;` | 保存 direct connect page 相关配置或运行状态。 |
| [L290](../src/apps/controller/ControllerMainWindow.h#L290) | `ownedDevicesPage_` | `OwnedDevicesPage* ownedDevicesPage_ = nullptr;` | 保存 owned devices page 相关配置或运行状态。 |
| [L291](../src/apps/controller/ControllerMainWindow.h#L291) | `recentConnectionsPage_` | `RecentConnectionsPage* recentConnectionsPage_ = nullptr;` | 保存 recent connections page 相关配置或运行状态。 |
| [L292](../src/apps/controller/ControllerMainWindow.h#L292) | `debugPage_` | `DiagnosticsPage* debugPage_ = nullptr;` | 保存 debug page 相关配置或运行状态。 |
| [L293](../src/apps/controller/ControllerMainWindow.h#L293) | `darkInterfaceTheme_` | `bool darkInterfaceTheme_ = false;` | 保存 dark interface theme 相关配置或运行状态。 |
| [L294](../src/apps/controller/ControllerMainWindow.h#L294) | `debugCopyText_` | `QString debugCopyText_;` | 保存 debug copy text 相关配置或运行状态。 |
| [L295](../src/apps/controller/ControllerMainWindow.h#L295) | `mediaDebugCopyText_` | `QString mediaDebugCopyText_;` | 保存 media debug copy text 相关配置或运行状态。 |
| [L296](../src/apps/controller/ControllerMainWindow.h#L296) | `statsDebugCopyText_` | `QString statsDebugCopyText_;` | 保存 stats debug copy text 相关配置或运行状态。 |
| [L297](../src/apps/controller/ControllerMainWindow.h#L297) | `diagnosticsCopyTextRequested_` | `bool diagnosticsCopyTextRequested_ = false;` | 保存 diagnostics copy text requested 相关配置或运行状态。 |
| [L298](../src/apps/controller/ControllerMainWindow.h#L298) | `diagnosticsRefreshTimer_` | `QTimer* diagnosticsRefreshTimer_ = nullptr;` | 保存定时、截止或超时状态：diagnostics refresh timer。 |
| [L299](../src/apps/controller/ControllerMainWindow.h#L299) | `lastRememberedRoomId_` | `QString lastRememberedRoomId_;` | 保存身份或作用域标识：last remembered room id。 |
| [L300](../src/apps/controller/ControllerMainWindow.h#L300) | `lastRememberedDirectSessionId_` | `QString lastRememberedDirectSessionId_;` | 保存身份或作用域标识：last remembered direct session id。 |
| [L301](../src/apps/controller/ControllerMainWindow.h#L301) | `recentHistoryAccountKey_` | `QString recentHistoryAccountKey_;` | 保存 recent history account key 相关配置或运行状态。 |
| [L302](../src/apps/controller/ControllerMainWindow.h#L302) | `recentRoomAvailabilityRequested_` | `bool recentRoomAvailabilityRequested_ = false;` | 保存 recent room availability requested 相关配置或运行状态。 |
| [L303](../src/apps/controller/ControllerMainWindow.h#L303) | `quitting_` | `bool quitting_ = false;` | 保存 quitting 相关配置或运行状态。 |
| [L304](../src/apps/controller/ControllerMainWindow.h#L304) | `pendingDeviceName_` | `QString pendingDeviceName_;` | 保存路径、地址或显示名称：pending device name。 |
| [L305](../src/apps/controller/ControllerMainWindow.h#L305) | `promptedSessionId_` | `QString promptedSessionId_;` | 保存身份或作用域标识：prompted session id。 |
| [L306](../src/apps/controller/ControllerMainWindow.h#L306) | `approvalRoomId_` | `QString approvalRoomId_;` | 保存身份或作用域标识：approval room id。 |
| [L307](../src/apps/controller/ControllerMainWindow.h#L307) | `promptedRoomJoinRequestIds_` | `QSet<QString> promptedRoomJoinRequestIds_;` | 保存 prompted room join request ids 相关配置或运行状态。 |
| [L308](../src/apps/controller/ControllerMainWindow.h#L308) | `promptedRoomScreenShareSwitchRequestIds_` | `QSet<QString> promptedRoomScreenShareSwitchRequestIds_;` | 保存 prompted room screen share switch request ids 相关配置或运行状态。 |
| [L309](../src/apps/controller/ControllerMainWindow.h#L309) | `promptedRoomControlRequestIds_` | `QSet<QString> promptedRoomControlRequestIds_;` | 保存 prompted room control request ids 相关配置或运行状态。 |
| [L310](../src/apps/controller/ControllerMainWindow.h#L310) | `promptedRoomScreenShareViewRequestIds_` | `QSet<QString> promptedRoomScreenShareViewRequestIds_;` | 保存 prompted room screen share view request ids 相关配置或运行状态。 |
| [L311](../src/apps/controller/ControllerMainWindow.h#L311) | `handledRoomMemberActionResultKeys_` | `QSet<QString> handledRoomMemberActionResultKeys_;` | 保存 handled room member action result keys 相关配置或运行状态。 |
| [L312](../src/apps/controller/ControllerMainWindow.h#L312) | `roomJoinApprovalPromptPending_` | `bool roomJoinApprovalPromptPending_ = false;` | 保存待处理队列或请求：room join approval prompt pending。 |
| [L313](../src/apps/controller/ControllerMainWindow.h#L313) | `roomScreenShareSwitchApprovalPromptPending_` | `bool roomScreenShareSwitchApprovalPromptPending_ = false;` | 保存待处理队列或请求：room screen share switch approval prompt pending。 |
| [L314](../src/apps/controller/ControllerMainWindow.h#L314) | `roomControlApprovalPromptPending_` | `bool roomControlApprovalPromptPending_ = false;` | 保存待处理队列或请求：room control approval prompt pending。 |
| [L315](../src/apps/controller/ControllerMainWindow.h#L315) | `roomScreenShareViewApprovalPromptPending_` | `bool roomScreenShareViewApprovalPromptPending_ = false;` | 保存待处理队列或请求：room screen share view approval prompt pending。 |
| [L316](../src/apps/controller/ControllerMainWindow.h#L316) | `lastRoomDecisionAlertKey_` | `QString lastRoomDecisionAlertKey_;` | 保存 last room decision alert key 相关配置或运行状态。 |
| [L317](../src/apps/controller/ControllerMainWindow.h#L317) | `cameraWindow_` | `QPointer<CameraWindow> cameraWindow_;` | 保存 camera window 相关配置或运行状态。 |
| [L318](../src/apps/controller/ControllerMainWindow.h#L318) | `fileTransferWindow_` | `QPointer<FileTransferWindow> fileTransferWindow_;` | 保存 file transfer window 相关配置或运行状态。 |
| [L319](../src/apps/controller/ControllerMainWindow.h#L319) | `remoteSessionWindow_` | `QPointer<RemoteSessionWindow> remoteSessionWindow_;` | 保存 remote session window 相关配置或运行状态。 |
| [L320](../src/apps/controller/ControllerMainWindow.h#L320) | `roomCameraWindow_` | `QPointer<RoomCameraWindow> roomCameraWindow_;` | 保存 room camera window 相关配置或运行状态。 |
| [L321](../src/apps/controller/ControllerMainWindow.h#L321) | `remoteSessionBinding_` | `std::optional<RemoteSessionBinding> remoteSessionBinding_;` | 保存 remote session binding 相关配置或运行状态。 |
| [L322](../src/apps/controller/ControllerMainWindow.h#L322) | `preflightScreenPreferenceEpoch_` | `quint64 preflightScreenPreferenceEpoch_ = 0;` | 标记当前世代，用于拒绝过期异步结果：preflight screen preference epoch。 |
| [L323](../src/apps/controller/ControllerMainWindow.h#L323) | `preflightScreenPreferenceAttemptEpoch_` | `quint64 preflightScreenPreferenceAttemptEpoch_ = 0;` | 标记当前世代，用于拒绝过期异步结果：preflight screen preference attempt epoch。 |
| [L324](../src/apps/controller/ControllerMainWindow.h#L324) | `preflightScreenPreferenceAttempts_` | `int preflightScreenPreferenceAttempts_ = 0;` | 保存 preflight screen preference attempts 相关配置或运行状态。 |
| [L325](../src/apps/controller/ControllerMainWindow.h#L325) | `preflightScreenPreferenceScheduled_` | `bool preflightScreenPreferenceScheduled_ = false;` | 保存 preflight screen preference scheduled 相关配置或运行状态。 |
| [L326](../src/apps/controller/ControllerMainWindow.h#L326) | `dismissedRemoteScreenSharerDeviceId_` | `QString dismissedRemoteScreenSharerDeviceId_;` | 保存身份或作用域标识：dismissed remote screen sharer device id。 |
| [L327](../src/apps/controller/ControllerMainWindow.h#L327) | `dismissedRemoteScreenShareEpoch_` | `quint64 dismissedRemoteScreenShareEpoch_ = 0;` | 标记当前世代，用于拒绝过期异步结果：dismissed remote screen share epoch。 |
| [L328](../src/apps/controller/ControllerMainWindow.h#L328) | `dismissedDirectSessionId_` | `QString dismissedDirectSessionId_;` | 保存身份或作用域标识：dismissed direct session id。 |
| [L329](../src/apps/controller/ControllerMainWindow.h#L329) | `localCameraStopRequested_` | `bool localCameraStopRequested_ = false;` | 保存 local camera stop requested 相关配置或运行状态。 |
| [L330](../src/apps/controller/ControllerMainWindow.h#L330) | `cameraGalleryManuallyHidden_` | `bool cameraGalleryManuallyHidden_ = false;` | 保存 camera gallery manually hidden 相关配置或运行状态。 |
| [L331](../src/apps/controller/ControllerMainWindow.h#L331) | `mediaDeviceRevision_` | `quint64 mediaDeviceRevision_ = 0;` | 标记当前世代，用于拒绝过期异步结果：media device revision。 |
| [L332](../src/apps/controller/ControllerMainWindow.h#L332) | `mediaDeviceSnapshotSeen_` | `bool mediaDeviceSnapshotSeen_ = false;` | 保存 media device snapshot seen 相关配置或运行状态。 |
| [L333](../src/apps/controller/ControllerMainWindow.h#L333) | `mediaActivitySnapshotSeen_` | `bool mediaActivitySnapshotSeen_ = false;` | 保存 media activity snapshot seen 相关配置或运行状态。 |
| [L335](../src/apps/controller/ControllerMainWindow.h#L335) | `kOff` | `LocalMicrophoneState::kOff;` | 定义 off 的编译期常量或产品边界。 |
| [L336](../src/apps/controller/ControllerMainWindow.h#L336) | `displayedRoomAudioPlaybackMuted_` | `bool displayedRoomAudioPlaybackMuted_ = false;` | 保存 displayed room audio playback muted 相关配置或运行状态。 |
| [L337](../src/apps/controller/ControllerMainWindow.h#L337) | `mediaDeviceRefreshUserRequested_` | `bool mediaDeviceRefreshUserRequested_ = false;` | 保存 media device refresh user requested 相关配置或运行状态。 |
| [L338](../src/apps/controller/ControllerMainWindow.h#L338) | `pendingCameraDeviceId_` | `QString pendingCameraDeviceId_;` | 保存身份或作用域标识：pending camera device id。 |
| [L339](../src/apps/controller/ControllerMainWindow.h#L339) | `pendingMicrophoneDeviceId_` | `QString pendingMicrophoneDeviceId_;` | 保存身份或作用域标识：pending microphone device id。 |
| [L340](../src/apps/controller/ControllerMainWindow.h#L340) | `pendingSpeakerDeviceId_` | `QString pendingSpeakerDeviceId_;` | 保存身份或作用域标识：pending speaker device id。 |
| [L341](../src/apps/controller/ControllerMainWindow.h#L341) | `lastMediaDeviceErrorKey_` | `QString lastMediaDeviceErrorKey_;` | 保存 last media device error key 相关配置或运行状态。 |
| [L342](../src/apps/controller/ControllerMainWindow.h#L342) | `displayedClipboardSentItems_` | `std::uint64_t displayedClipboardSentItems_ = 0;` | 保存 displayed clipboard sent items 相关配置或运行状态。 |
| [L343](../src/apps/controller/ControllerMainWindow.h#L343) | `displayedClipboardReceivedItems_` | `std::uint64_t displayedClipboardReceivedItems_ = 0;` | 保存 displayed clipboard received items 相关配置或运行状态。 |
| [L344](../src/apps/controller/ControllerMainWindow.h#L344) | `displayedClipboardRejectedItems_` | `std::uint64_t displayedClipboardRejectedItems_ = 0;` | 保存 displayed clipboard rejected items 相关配置或运行状态。 |
| [L345](../src/apps/controller/ControllerMainWindow.h#L345) | `displayedClipboardErrorCode_` | `QString displayedClipboardErrorCode_;` | 保存 displayed clipboard error code 相关配置或运行状态。 |
| [L346](../src/apps/controller/ControllerMainWindow.h#L346) | `pendingRemotePasteDialogId_` | `QString pendingRemotePasteDialogId_;` | 保存身份或作用域标识：pending remote paste dialog id。 |
| [L347](../src/apps/controller/ControllerMainWindow.h#L347) | `visibleRemotePasteDialogId_` | `QString visibleRemotePasteDialogId_;` | 保存身份或作用域标识：visible remote paste dialog id。 |
| [L348](../src/apps/controller/ControllerMainWindow.h#L348) | `promptedClipboardConflictId_` | `QString promptedClipboardConflictId_;` | 保存身份或作用域标识：prompted clipboard conflict id。 |
| [L349](../src/apps/controller/ControllerMainWindow.h#L349) | `clipboardUiUpdatePending_` | `std::atomic_bool clipboardUiUpdatePending_{false};` | 保存待处理队列或请求：clipboard ui update pending。 |
| [L350](../src/apps/controller/ControllerMainWindow.h#L350) | `clipboardAllowedForCurrentControl_` | `bool clipboardAllowedForCurrentControl_ = false;` | 保存 clipboard allowed for current control 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L69](../src/apps/controller/ControllerMainWindow.h#L69) | `ControllerMainWindow` | 声明 | `explicit ControllerMainWindow( std::unique_ptr<ISessionEngine> engine, bool startEngineImmediately = true, app::ISessionMediaAccess* sessionMedia = nullptr, QWidget* parent = nullptr)` | 实现 controller main window 对应的业务或工具逻辑。 |
| [L74](../src/apps/controller/ControllerMainWindow.h#L74) | `~ControllerMainWindow` | 声明 | `~ControllerMainWindow() override` | 停止相关活动并释放 ControllerMainWindow 实例拥有的资源。 |
| [L76](../src/apps/controller/ControllerMainWindow.h#L76) | `ActivateFromExternalLaunch` | 声明 | `void ActivateFromExternalLaunch()` | 实现 activate from external launch 对应的业务或工具逻辑。 |
| [L77](../src/apps/controller/ControllerMainWindow.h#L77) | `SetAccountInteractionCallback` | 声明 | `void SetAccountInteractionCallback( std::function<void()> callback)` | 更新或应用 set account interaction callback 相关逻辑。 |
| [L79](../src/apps/controller/ControllerMainWindow.h#L79) | `SetAccountSwitchCallback` | 声明 | `void SetAccountSwitchCallback(std::function<void()> callback)` | 更新或应用 set account switch callback 相关逻辑。 |
| [L80](../src/apps/controller/ControllerMainWindow.h#L80) | `SetAccountDeletionCallback` | 声明 | `void SetAccountDeletionCallback(std::function<void()> callback)` | 更新或应用 set account deletion callback 相关逻辑。 |
| [L83](../src/apps/controller/ControllerMainWindow.h#L83) | `SetAccountBusy` | 声明 | `void SetAccountBusy(const QString& message)` | 更新或应用 set account busy 相关逻辑。 |
| [L84](../src/apps/controller/ControllerMainWindow.h#L84) | `SetAccountSession` | 声明 | `void SetAccountSession( const QString& accountId, const QString& accountLabel, const QString& accountDetail, std::function<void()> signOutCallback)` | 更新或应用 set account session 相关逻辑。 |
| [L89](../src/apps/controller/ControllerMainWindow.h#L89) | `StartSessionEngine` | 声明 | `bool StartSessionEngine()` | 启动 start session engine 相关逻辑。 |
| [L90](../src/apps/controller/ControllerMainWindow.h#L90) | `StopSessionEngine` | 声明 | `void StopSessionEngine()` | 停止 stop session engine 相关逻辑。 |
| [L93](../src/apps/controller/ControllerMainWindow.h#L93) | `PrepareForApplicationExit` | 声明 | `void PrepareForApplicationExit()` | Executes the protocol-aware shutdown while Qt can still flush WSS messages. Safe to call repeatedly from every application-exit path. |
| [L94](../src/apps/controller/ControllerMainWindow.h#L94) | `RunThemeRoundTripSelfTest` | 声明 | `bool RunThemeRoundTripSelfTest(QString* errorMessage = nullptr)` | 执行后台循环或调度 run theme round trip self test 相关逻辑。 |
| [L96](../src/apps/controller/ControllerMainWindow.h#L96) | `ControllerMainWindow` | 声明 | `ControllerMainWindow(const ControllerMainWindow&) = delete` | 实现 controller main window 对应的业务或工具逻辑。 |
| [L100](../src/apps/controller/ControllerMainWindow.h#L100) | `OnSessionEngineSnapshot` | 声明 | `void OnSessionEngineSnapshot( const SessionEngineSnapshot& snapshot) override` | 接收并处理 on session engine snapshot 相关逻辑。 |
| [L102](../src/apps/controller/ControllerMainWindow.h#L102) | `OnClipboardStateChanged` | 声明 | `void OnClipboardStateChanged( const app::ClipboardControllerSnapshot& snapshot) override` | 接收并处理 on clipboard state changed 相关逻辑。 |
| [L104](../src/apps/controller/ControllerMainWindow.h#L104) | `BuildUi` | 声明 | `void BuildUi()` | 创建或初始化 build ui 相关逻辑。 |
| [L105](../src/apps/controller/ControllerMainWindow.h#L105) | `BuildShellAndRoomPage` | 声明 | `void BuildShellAndRoomPage()` | 创建或初始化 build shell and room page 相关逻辑。 |
| [L106](../src/apps/controller/ControllerMainWindow.h#L106) | `RoomControls` | 声明 | `RoomPageControls& RoomControls()` | 实现 room controls 对应的业务或工具逻辑。 |
| [L107](../src/apps/controller/ControllerMainWindow.h#L107) | `BuildDeviceAndRecentPages` | 声明 | `void BuildDeviceAndRecentPages()` | 创建或初始化 build device and recent pages 相关逻辑。 |
| [L108](../src/apps/controller/ControllerMainWindow.h#L108) | `BuildDiagnosticsPage` | 声明 | `void BuildDiagnosticsPage()` | 创建或初始化 build diagnostics page 相关逻辑。 |
| [L109](../src/apps/controller/ControllerMainWindow.h#L109) | `BuildSettingsPage` | 声明 | `void BuildSettingsPage()` | 创建或初始化 build settings page 相关逻辑。 |
| [L110](../src/apps/controller/ControllerMainWindow.h#L110) | `SettingsControls` | 声明 | `SettingsPageControls& SettingsControls()` | 更新或应用 settings controls 相关逻辑。 |
| [L111](../src/apps/controller/ControllerMainWindow.h#L111) | `BuildHelpAndAuthorPages` | 声明 | `void BuildHelpAndAuthorPages()` | 创建或初始化 build help and author pages 相关逻辑。 |
| [L112](../src/apps/controller/ControllerMainWindow.h#L112) | `ConnectUiSignals` | 声明 | `void ConnectUiSignals()` | 建立连接 connect ui signals 相关逻辑。 |
| [L113](../src/apps/controller/ControllerMainWindow.h#L113) | `BuildSystemTray` | 声明 | `void BuildSystemTray()` | 创建或初始化 build system tray 相关逻辑。 |
| [L114](../src/apps/controller/ControllerMainWindow.h#L114) | `ShowFromSystemTray` | 声明 | `void ShowFromSystemTray()` | 实现 show from system tray 对应的业务或工具逻辑。 |
| [L115](../src/apps/controller/ControllerMainWindow.h#L115) | `QuitFromSystemTray` | 声明 | `void QuitFromSystemTray()` | 实现 quit from system tray 对应的业务或工具逻辑。 |
| [L116](../src/apps/controller/ControllerMainWindow.h#L116) | `DestroyAuxiliaryWindowsForExit` | 声明 | `void DestroyAuxiliaryWindowsForExit()` | 关闭并清理 destroy auxiliary windows for exit 相关逻辑。 |
| [L117](../src/apps/controller/ControllerMainWindow.h#L117) | `CheckForSoftwareUpdates` | 声明 | `void CheckForSoftwareUpdates(bool manualRequest)` | 校验 check for software updates 相关逻辑。 |
| [L118](../src/apps/controller/ControllerMainWindow.h#L118) | `HandleSoftwareUpdateState` | 声明 | `void HandleSoftwareUpdateState( const update::SoftwareUpdateController::Snapshot& snapshot)` | 接收并处理 handle software update state 相关逻辑。 |
| [L120](../src/apps/controller/ControllerMainWindow.h#L120) | `OpenSoftwareUpdate` | 声明 | `void OpenSoftwareUpdate()` | 启动 open software update 相关逻辑。 |
| [L121](../src/apps/controller/ControllerMainWindow.h#L121) | `QuitForSoftwareUpdate` | 声明 | `void QuitForSoftwareUpdate()` | 实现 quit for software update 对应的业务或工具逻辑。 |
| [L122](../src/apps/controller/ControllerMainWindow.h#L122) | `InitializeEngine` | 声明 | `bool InitializeEngine()` | 创建或初始化 initialize engine 相关逻辑。 |
| [L123](../src/apps/controller/ControllerMainWindow.h#L123) | `ApplyEngineInitializationState` | 声明 | `void ApplyEngineInitializationState( const SessionEngineSnapshot& snapshot)` | 更新或应用 apply engine initialization state 相关逻辑。 |
| [L125](../src/apps/controller/ControllerMainWindow.h#L125) | `ApplyAuthenticationAvailability` | 声明 | `void ApplyAuthenticationAvailability(bool authenticated)` | 更新或应用 apply authentication availability 相关逻辑。 |
| [L126](../src/apps/controller/ControllerMainWindow.h#L126) | `StartSession` | 声明 | `void StartSession(const QString& deviceId, const QString& deviceName, SessionPurpose purpose)` | 启动 start session 相关逻辑。 |
| [L129](../src/apps/controller/ControllerMainWindow.h#L129) | `StartOwnedDeviceSession` | 声明 | `void StartOwnedDeviceSession(const QString& deviceId, const QString& deviceName)` | 启动 start owned device session 相关逻辑。 |
| [L131](../src/apps/controller/ControllerMainWindow.h#L131) | `StartAssistedSession` | 声明 | `void StartAssistedSession(const QString& deviceId, const QString& verificationCode)` | 启动 start assisted session 相关逻辑。 |
| [L133](../src/apps/controller/ControllerMainWindow.h#L133) | `OpenCameraWindow` | 声明 | `void OpenCameraWindow(const QString& deviceId, const QString& deviceName)` | 启动 open camera window 相关逻辑。 |
| [L134](../src/apps/controller/ControllerMainWindow.h#L134) | `OpenRemoteSession` | 声明 | `void OpenRemoteSession(RemoteSessionBinding binding)` | 启动 open remote session 相关逻辑。 |
| [L135](../src/apps/controller/ControllerMainWindow.h#L135) | `RefreshOwnedDevicesUi` | 声明 | `void RefreshOwnedDevicesUi( const SessionEngineSnapshot& snapshot)` | 刷新 refresh owned devices ui 相关逻辑。 |
| [L137](../src/apps/controller/ControllerMainWindow.h#L137) | `UpdateRoomUi` | 声明 | `void UpdateRoomUi(const SessionEngineSnapshot& snapshot)` | 更新或应用 update room ui 相关逻辑。 |
| [L138](../src/apps/controller/ControllerMainWindow.h#L138) | `QueueRoomJoinApproval` | 声明 | `void QueueRoomJoinApproval(const SessionEngineSnapshot& snapshot)` | 实现 queue room join approval 对应的业务或工具逻辑。 |
| [L139](../src/apps/controller/ControllerMainWindow.h#L139) | `QueueRoomScreenShareSwitchApproval` | 声明 | `void QueueRoomScreenShareSwitchApproval( const SessionEngineSnapshot& snapshot)` | 实现 queue room screen share switch approval 对应的业务或工具逻辑。 |
| [L141](../src/apps/controller/ControllerMainWindow.h#L141) | `QueueRoomControlApproval` | 声明 | `void QueueRoomControlApproval(const SessionEngineSnapshot& snapshot)` | 实现 queue room control approval 对应的业务或工具逻辑。 |
| [L142](../src/apps/controller/ControllerMainWindow.h#L142) | `QueueRoomScreenShareViewApproval` | 声明 | `void QueueRoomScreenShareViewApproval( const SessionEngineSnapshot& snapshot)` | 实现 queue room screen share view approval 对应的业务或工具逻辑。 |
| [L144](../src/apps/controller/ControllerMainWindow.h#L144) | `HandleRoomMemberActionResults` | 声明 | `void HandleRoomMemberActionResults( const SessionEngineSnapshot& snapshot)` | 接收并处理 handle room member action results 相关逻辑。 |
| [L146](../src/apps/controller/ControllerMainWindow.h#L146) | `ShowRoomMemberContextMenu` | 声明 | `void ShowRoomMemberContextMenu(const QPoint& position)` | 实现 show room member context menu 对应的业务或工具逻辑。 |
| [L147](../src/apps/controller/ControllerMainWindow.h#L147) | `HandleRemoteSessionDisconnect` | 声明 | `void HandleRemoteSessionDisconnect()` | 接收并处理 handle remote session disconnect 相关逻辑。 |
| [L148](../src/apps/controller/ControllerMainWindow.h#L148) | `SetRoomActionHint` | 声明 | `void SetRoomActionHint(const QString& text, bool error = false)` | 更新或应用 set room action hint 相关逻辑。 |
| [L149](../src/apps/controller/ControllerMainWindow.h#L149) | `SetRuntimeStatus` | 声明 | `void SetRuntimeStatus(const QString& status, const QString& color)` | 更新或应用 set runtime status 相关逻辑。 |
| [L150](../src/apps/controller/ControllerMainWindow.h#L150) | `SelectMainPage` | 声明 | `void SelectMainPage(int pageIndex, QPushButton* navigationButton, const QString& title)` | 查询并返回 select main page 相关逻辑。 |
| [L153](../src/apps/controller/ControllerMainWindow.h#L153) | `AnimateNavigationIndicator` | 声明 | `void AnimateNavigationIndicator(QPushButton* navigationButton)` | 实现 animate navigation indicator 对应的业务或工具逻辑。 |
| [L154](../src/apps/controller/ControllerMainWindow.h#L154) | `QueueRoomWorkspaceActive` | 声明 | `void QueueRoomWorkspaceActive(bool active)` | 实现 queue room workspace active 对应的业务或工具逻辑。 |
| [L155](../src/apps/controller/ControllerMainWindow.h#L155) | `SetRoomWorkspaceActive` | 声明 | `void SetRoomWorkspaceActive(bool active)` | 更新或应用 set room workspace active 相关逻辑。 |
| [L156](../src/apps/controller/ControllerMainWindow.h#L156) | `SetAnimationLevel` | 声明 | `void SetAnimationLevel(int level)` | 更新或应用 set animation level 相关逻辑。 |
| [L157](../src/apps/controller/ControllerMainWindow.h#L157) | `ToggleAccountMenu` | 声明 | `void ToggleAccountMenu()` | 实现 toggle account menu 对应的业务或工具逻辑。 |
| [L158](../src/apps/controller/ControllerMainWindow.h#L158) | `ShowAccountMenu` | 声明 | `void ShowAccountMenu()` | 实现 show account menu 对应的业务或工具逻辑。 |
| [L159](../src/apps/controller/ControllerMainWindow.h#L159) | `HideAccountMenu` | 声明 | `void HideAccountMenu(bool animated = true)` | 实现 hide account menu 对应的业务或工具逻辑。 |
| [L160](../src/apps/controller/ControllerMainWindow.h#L160) | `UpdateAccountMenuGeometry` | 声明 | `void UpdateAccountMenuGeometry()` | 更新或应用 update account menu geometry 相关逻辑。 |
| [L161](../src/apps/controller/ControllerMainWindow.h#L161) | `StartAccountMenuMotion` | 声明 | `void StartAccountMenuMotion(const QPoint& targetPosition, int durationMs, bool hideWhenFinished)` | 启动 start account menu motion 相关逻辑。 |
| [L163](../src/apps/controller/ControllerMainWindow.h#L163) | `StopAccountMenuMotion` | 声明 | `void StopAccountMenuMotion()` | 停止 stop account menu motion 相关逻辑。 |
| [L164](../src/apps/controller/ControllerMainWindow.h#L164) | `UpdateAccountMenuHoverFromCursor` | 声明 | `void UpdateAccountMenuHoverFromCursor()` | 更新或应用 update account menu hover from cursor 相关逻辑。 |
| [L165](../src/apps/controller/ControllerMainWindow.h#L165) | `ApplyInterfaceTheme` | 声明 | `void ApplyInterfaceTheme(bool showFeedback = false)` | 更新或应用 apply interface theme 相关逻辑。 |
| [L166](../src/apps/controller/ControllerMainWindow.h#L166) | `SetInterfaceThemePreference` | 声明 | `void SetInterfaceThemePreference(const QString& value)` | 更新或应用 set interface theme preference 相关逻辑。 |
| [L167](../src/apps/controller/ControllerMainWindow.h#L167) | `RememberRecentRoom` | 声明 | `void RememberRecentRoom(const SessionEngineSnapshot& snapshot)` | 实现 remember recent room 对应的业务或工具逻辑。 |
| [L168](../src/apps/controller/ControllerMainWindow.h#L168) | `RememberRecentDevice` | 声明 | `void RememberRecentDevice(const SessionEngineSnapshot& snapshot)` | 实现 remember recent device 对应的业务或工具逻辑。 |
| [L169](../src/apps/controller/ControllerMainWindow.h#L169) | `RefreshRecentRooms` | 声明 | `void RefreshRecentRooms()` | 刷新 refresh recent rooms 相关逻辑。 |
| [L170](../src/apps/controller/ControllerMainWindow.h#L170) | `RefreshRecentDevices` | 声明 | `void RefreshRecentDevices()` | 刷新 refresh recent devices 相关逻辑。 |
| [L171](../src/apps/controller/ControllerMainWindow.h#L171) | `RecentSettingsKey` | 声明 | `QString RecentSettingsKey(const QString& listName) const` | 实现 recent settings key 对应的业务或工具逻辑。 |
| [L172](../src/apps/controller/ControllerMainWindow.h#L172) | `MigrateLegacyRecentHistory` | 声明 | `void MigrateLegacyRecentHistory()` | 实现 migrate legacy recent history 对应的业务或工具逻辑。 |
| [L173](../src/apps/controller/ControllerMainWindow.h#L173) | `RequestRecentRoomAvailability` | 声明 | `void RequestRecentRoomAvailability()` | 发起请求或查询 request recent room availability 相关逻辑。 |
| [L174](../src/apps/controller/ControllerMainWindow.h#L174) | `RefreshDiagnosticsUi` | 声明 | `void RefreshDiagnosticsUi()` | 刷新 refresh diagnostics ui 相关逻辑。 |
| [L175](../src/apps/controller/ControllerMainWindow.h#L175) | `StartDecoderBenchmark` | 声明 | `void StartDecoderBenchmark(bool manualRequest)` | 启动 start decoder benchmark 相关逻辑。 |
| [L176](../src/apps/controller/ControllerMainWindow.h#L176) | `FinishDecoderBenchmark` | 声明 | `void FinishDecoderBenchmark(int exitCode)` | 停止 finish decoder benchmark 相关逻辑。 |
| [L177](../src/apps/controller/ControllerMainWindow.h#L177) | `RefreshDecoderBenchmarkSummary` | 声明 | `void RefreshDecoderBenchmarkSummary()` | 刷新 refresh decoder benchmark summary 相关逻辑。 |
| [L178](../src/apps/controller/ControllerMainWindow.h#L178) | `RefreshDecoderHardwareSelectionAvailability` | 声明 | `void RefreshDecoderHardwareSelectionAvailability()` | 刷新 refresh decoder hardware selection availability 相关逻辑。 |
| [L179](../src/apps/controller/ControllerMainWindow.h#L179) | `StartEncoderBenchmark` | 声明 | `void StartEncoderBenchmark(bool manualRequest)` | 启动 start encoder benchmark 相关逻辑。 |
| [L180](../src/apps/controller/ControllerMainWindow.h#L180) | `FinishEncoderBenchmark` | 声明 | `void FinishEncoderBenchmark(int exitCode)` | 停止 finish encoder benchmark 相关逻辑。 |
| [L181](../src/apps/controller/ControllerMainWindow.h#L181) | `RefreshEncoderBenchmarkSummary` | 声明 | `void RefreshEncoderBenchmarkSummary()` | 刷新 refresh encoder benchmark summary 相关逻辑。 |
| [L182](../src/apps/controller/ControllerMainWindow.h#L182) | `UpdateLocalMediaDevicesUi` | 声明 | `void UpdateLocalMediaDevicesUi( const SessionEngineSnapshot& snapshot)` | 更新或应用 update local media devices ui 相关逻辑。 |
| [L184](../src/apps/controller/ControllerMainWindow.h#L184) | `PersistHardwareCapabilityCache` | 声明 | `void PersistHardwareCapabilityCache()` | 保存或写入 persist hardware capability cache 相关逻辑。 |
| [L185](../src/apps/controller/ControllerMainWindow.h#L185) | `RequestMediaDeviceRefresh` | 声明 | `void RequestMediaDeviceRefresh(bool userInitiated = false)` | 发起请求或查询 request media device refresh 相关逻辑。 |
| [L189](../src/apps/controller/ControllerMainWindow.h#L189) | `UpdateVideoPipelineSettingsAvailability` | 声明 | `void UpdateVideoPipelineSettingsAvailability( const SessionEngineSnapshot& snapshot)` | 更新或应用 update video pipeline settings availability 相关逻辑。 |
| [L191](../src/apps/controller/ControllerMainWindow.h#L191) | `ShowMediaDeviceMenu` | 声明 | `void ShowMediaDeviceMenu( MediaDeviceKind kind, QWidget* anchor)` | 实现 show media device menu 对应的业务或工具逻辑。 |
| [L193](../src/apps/controller/ControllerMainWindow.h#L193) | `BeginMediaDeviceSelection` | 声明 | `void BeginMediaDeviceSelection( MediaDeviceKind kind, const QString& deviceId)` | 启动 begin media device selection 相关逻辑。 |
| [L195](../src/apps/controller/ControllerMainWindow.h#L195) | `ApplyClipboardConfigurationFromUi` | 声明 | `void ApplyClipboardConfigurationFromUi(bool showFeedback = false)` | 更新或应用 apply clipboard configuration from ui 相关逻辑。 |
| [L196](../src/apps/controller/ControllerMainWindow.h#L196) | `UpdateClipboardSession` | 声明 | `void UpdateClipboardSession(const SessionEngineSnapshot& snapshot)` | 更新或应用 update clipboard session 相关逻辑。 |
| [L197](../src/apps/controller/ControllerMainWindow.h#L197) | `UpdateDirectFileTransferSession` | 声明 | `void UpdateDirectFileTransferSession( const SessionEngineSnapshot& snapshot)` | 更新或应用 update direct file transfer session 相关逻辑。 |
| [L201](../src/apps/controller/ControllerMainWindow.h#L201) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L202](../src/apps/controller/ControllerMainWindow.h#L202) | `nativeEvent` | 声明 | `bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override` | 实现 native event 对应的业务或工具逻辑。 |
| [L205](../src/apps/controller/ControllerMainWindow.h#L205) | `changeEvent` | 声明 | `void changeEvent(QEvent* event) override` | 实现 change event 对应的业务或工具逻辑。 |
| [L206](../src/apps/controller/ControllerMainWindow.h#L206) | `closeEvent` | 声明 | `void closeEvent(QCloseEvent* event) override` | 关闭并清理 close event 相关逻辑。 |
| [L207](../src/apps/controller/ControllerMainWindow.h#L207) | `resizeEvent` | 声明 | `void resizeEvent(QResizeEvent* event) override` | 实现 resize event 对应的业务或工具逻辑。 |

## `src/apps/controller/ControllerMainWindow.Media.cpp`

[打开源码](../src/apps/controller/ControllerMainWindow.Media.cpp) · **文件作用：** 实现 controller main window media 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L46](../src/apps/controller/ControllerMainWindow.Media.cpp#L46) | `ControllerMainWindow::InitializeEngine` | 定义 | `bool ControllerMainWindow::InitializeEngine()` | 创建或初始化 initialize engine 相关逻辑。 |
| [L66](../src/apps/controller/ControllerMainWindow.Media.cpp#L66) | `ControllerMainWindow::ApplyEngineInitializationState` | 定义 | `void ControllerMainWindow::ApplyEngineInitializationState( const SessionEngineSnapshot& snapshot)` | 更新或应用 apply engine initialization state 相关逻辑。 |
| [L130](../src/apps/controller/ControllerMainWindow.Media.cpp#L130) | `ControllerMainWindow::PersistHardwareCapabilityCache` | 定义 | `void ControllerMainWindow::PersistHardwareCapabilityCache()` | 保存或写入 persist hardware capability cache 相关逻辑。 |
| [L184](../src/apps/controller/ControllerMainWindow.Media.cpp#L184) | `ControllerMainWindow::ApplyVideoPipelineSettingsFromUi` | 定义 | `void ControllerMainWindow::ApplyVideoPipelineSettingsFromUi( bool showFeedback, const QString& changedSettingName)` | 更新或应用 apply video pipeline settings from ui 相关逻辑。 |
| [L305](../src/apps/controller/ControllerMainWindow.Media.cpp#L305) | `ControllerMainWindow::UpdateVideoPipelineSettingsAvailability` | 定义 | `void ControllerMainWindow::UpdateVideoPipelineSettingsAvailability( const SessionEngineSnapshot& snapshot)` | 更新或应用 update video pipeline settings availability 相关逻辑。 |
| [L349](../src/apps/controller/ControllerMainWindow.Media.cpp#L349) | `ControllerMainWindow::UpdateLocalMediaDevicesUi` | 定义 | `void ControllerMainWindow::UpdateLocalMediaDevicesUi( const SessionEngineSnapshot& snapshot)` | 更新或应用 update local media devices ui 相关逻辑。 |
| [L665](../src/apps/controller/ControllerMainWindow.Media.cpp#L665) | `ControllerMainWindow::RefreshEncoderBenchmarkSummary` | 定义 | `void ControllerMainWindow::RefreshEncoderBenchmarkSummary()` | 刷新 refresh encoder benchmark summary 相关逻辑。 |
| [L811](../src/apps/controller/ControllerMainWindow.Media.cpp#L811) | `ControllerMainWindow::StartEncoderBenchmark` | 定义 | `void ControllerMainWindow::StartEncoderBenchmark(bool manualRequest)` | 启动 start encoder benchmark 相关逻辑。 |
| [L885](../src/apps/controller/ControllerMainWindow.Media.cpp#L885) | `ControllerMainWindow::FinishEncoderBenchmark` | 定义 | `void ControllerMainWindow::FinishEncoderBenchmark(int exitCode)` | 停止 finish encoder benchmark 相关逻辑。 |
| [L1028](../src/apps/controller/ControllerMainWindow.Media.cpp#L1028) | `ControllerMainWindow::RefreshDecoderBenchmarkSummary` | 定义 | `void ControllerMainWindow::RefreshDecoderBenchmarkSummary()` | 刷新 refresh decoder benchmark summary 相关逻辑。 |
| [L1241](../src/apps/controller/ControllerMainWindow.Media.cpp#L1241) | `ControllerMainWindow::RefreshDecoderHardwareSelectionAvailability` | 定义 | `void ControllerMainWindow::RefreshDecoderHardwareSelectionAvailability()` | 刷新 refresh decoder hardware selection availability 相关逻辑。 |
| [L1297](../src/apps/controller/ControllerMainWindow.Media.cpp#L1297) | `ControllerMainWindow::StartDecoderBenchmark` | 定义 | `void ControllerMainWindow::StartDecoderBenchmark(bool manualRequest)` | 启动 start decoder benchmark 相关逻辑。 |
| [L1380](../src/apps/controller/ControllerMainWindow.Media.cpp#L1380) | `ControllerMainWindow::FinishDecoderBenchmark` | 定义 | `void ControllerMainWindow::FinishDecoderBenchmark(int exitCode)` | 停止 finish decoder benchmark 相关逻辑。 |
| [L1533](../src/apps/controller/ControllerMainWindow.Media.cpp#L1533) | `ControllerMainWindow::ApplyClipboardConfigurationFromUi` | 定义 | `void ControllerMainWindow::ApplyClipboardConfigurationFromUi( bool showFeedback)` | 更新或应用 apply clipboard configuration from ui 相关逻辑。 |
| [L1602](../src/apps/controller/ControllerMainWindow.Media.cpp#L1602) | `ControllerMainWindow::UpdateClipboardSession` | 定义 | `void ControllerMainWindow::UpdateClipboardSession( const SessionEngineSnapshot& snapshot)` | 更新或应用 update clipboard session 相关逻辑。 |
| [L1665](../src/apps/controller/ControllerMainWindow.Media.cpp#L1665) | `ControllerMainWindow::UpdateDirectFileTransferSession` | 定义 | `void ControllerMainWindow::UpdateDirectFileTransferSession( const SessionEngineSnapshot& snapshot)` | 更新或应用 update direct file transfer session 相关逻辑。 |
| [L1702](../src/apps/controller/ControllerMainWindow.Media.cpp#L1702) | `ControllerMainWindow::OnClipboardStateChanged` | 定义 | `void ControllerMainWindow::OnClipboardStateChanged( const app::ClipboardControllerSnapshot& snapshot)` | 接收并处理 on clipboard state changed 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.NavigationRecent.cpp`

[打开源码](../src/apps/controller/ControllerMainWindow.NavigationRecent.cpp) · **文件作用：** 实现 controller main window navigation recent 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L36](../src/apps/controller/ControllerMainWindow.NavigationRecent.cpp#L36) | `ControllerMainWindow::RecentSettingsKey` | 定义 | `QString ControllerMainWindow::RecentSettingsKey( const QString& listName) const` | 实现 recent settings key 对应的业务或工具逻辑。 |
| [L46](../src/apps/controller/ControllerMainWindow.NavigationRecent.cpp#L46) | `ControllerMainWindow::MigrateLegacyRecentHistory` | 定义 | `void ControllerMainWindow::MigrateLegacyRecentHistory()` | 实现 migrate legacy recent history 对应的业务或工具逻辑。 |
| [L71](../src/apps/controller/ControllerMainWindow.NavigationRecent.cpp#L71) | `ControllerMainWindow::RememberRecentRoom` | 定义 | `void ControllerMainWindow::RememberRecentRoom( const SessionEngineSnapshot& snapshot)` | 实现 remember recent room 对应的业务或工具逻辑。 |
| [L131](../src/apps/controller/ControllerMainWindow.NavigationRecent.cpp#L131) | `ControllerMainWindow::RefreshRecentRooms` | 定义 | `void ControllerMainWindow::RefreshRecentRooms()` | 刷新 refresh recent rooms 相关逻辑。 |
| [L213](../src/apps/controller/ControllerMainWindow.NavigationRecent.cpp#L213) | `ControllerMainWindow::RememberRecentDevice` | 定义 | `void ControllerMainWindow::RememberRecentDevice( const SessionEngineSnapshot& snapshot)` | 实现 remember recent device 对应的业务或工具逻辑。 |
| [L275](../src/apps/controller/ControllerMainWindow.NavigationRecent.cpp#L275) | `ControllerMainWindow::RefreshRecentDevices` | 定义 | `void ControllerMainWindow::RefreshRecentDevices()` | 刷新 refresh recent devices 相关逻辑。 |
| [L349](../src/apps/controller/ControllerMainWindow.NavigationRecent.cpp#L349) | `ControllerMainWindow::RequestRecentRoomAvailability` | 定义 | `void ControllerMainWindow::RequestRecentRoomAvailability()` | 发起请求或查询 request recent room availability 相关逻辑。 |
| [L375](../src/apps/controller/ControllerMainWindow.NavigationRecent.cpp#L375) | `ControllerMainWindow::SelectMainPage` | 定义 | `void ControllerMainWindow::SelectMainPage( int pageIndex, QPushButton* navigationButton, const QString& title)` | 查询并返回 select main page 相关逻辑。 |
| [L407](../src/apps/controller/ControllerMainWindow.NavigationRecent.cpp#L407) | `ControllerMainWindow::AnimateNavigationIndicator` | 定义 | `void ControllerMainWindow::AnimateNavigationIndicator( QPushButton* navigationButton)` | 实现 animate navigation indicator 对应的业务或工具逻辑。 |
| [L432](../src/apps/controller/ControllerMainWindow.NavigationRecent.cpp#L432) | `ControllerMainWindow::QueueRoomWorkspaceActive` | 定义 | `void ControllerMainWindow::QueueRoomWorkspaceActive(bool active)` | 实现 queue room workspace active 对应的业务或工具逻辑。 |
| [L463](../src/apps/controller/ControllerMainWindow.NavigationRecent.cpp#L463) | `ControllerMainWindow::SetRoomWorkspaceActive` | 定义 | `void ControllerMainWindow::SetRoomWorkspaceActive(bool active)` | 更新或应用 set room workspace active 相关逻辑。 |
| [L613](../src/apps/controller/ControllerMainWindow.NavigationRecent.cpp#L613) | `ControllerMainWindow::SetAnimationLevel` | 定义 | `void ControllerMainWindow::SetAnimationLevel(int level)` | 更新或应用 set animation level 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.OwnedDevices.cpp`

[打开源码](../src/apps/controller/ControllerMainWindow.OwnedDevices.cpp) · **文件作用：** 实现 controller main window owned devices 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L13](../src/apps/controller/ControllerMainWindow.OwnedDevices.cpp#L13) | `ControllerMainWindow::RefreshOwnedDevicesUi` | 定义 | `void ControllerMainWindow::RefreshOwnedDevicesUi( const SessionEngineSnapshot& snapshot)` | 刷新 refresh owned devices ui 相关逻辑。 |
| [L23](../src/apps/controller/ControllerMainWindow.OwnedDevices.cpp#L23) | `ControllerMainWindow::StartOwnedDeviceSession` | 定义 | `void ControllerMainWindow::StartOwnedDeviceSession( const QString& deviceId, const QString& deviceName)` | 启动 start owned device session 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.Room.cpp`

[打开源码](../src/apps/controller/ControllerMainWindow.Room.cpp) · **文件作用：** 实现 controller main window room 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L23](../src/apps/controller/ControllerMainWindow.Room.cpp#L23) | `ControllerMainWindow::QueueRoomJoinApproval` | 定义 | `void ControllerMainWindow::QueueRoomJoinApproval( const SessionEngineSnapshot& snapshot)` | 实现 queue room join approval 对应的业务或工具逻辑。 |
| [L124](../src/apps/controller/ControllerMainWindow.Room.cpp#L124) | `ControllerMainWindow::QueueRoomScreenShareSwitchApproval` | 定义 | `void ControllerMainWindow::QueueRoomScreenShareSwitchApproval( const SessionEngineSnapshot& snapshot)` | 实现 queue room screen share switch approval 对应的业务或工具逻辑。 |
| [L237](../src/apps/controller/ControllerMainWindow.Room.cpp#L237) | `ControllerMainWindow::QueueRoomControlApproval` | 定义 | `void ControllerMainWindow::QueueRoomControlApproval( const SessionEngineSnapshot& snapshot)` | 实现 queue room control approval 对应的业务或工具逻辑。 |
| [L338](../src/apps/controller/ControllerMainWindow.Room.cpp#L338) | `ControllerMainWindow::QueueRoomScreenShareViewApproval` | 定义 | `void ControllerMainWindow::QueueRoomScreenShareViewApproval( const SessionEngineSnapshot& snapshot)` | 实现 queue room screen share view approval 对应的业务或工具逻辑。 |
| [L454](../src/apps/controller/ControllerMainWindow.Room.cpp#L454) | `ControllerMainWindow::HandleRoomMemberActionResults` | 定义 | `void ControllerMainWindow::HandleRoomMemberActionResults( const SessionEngineSnapshot& snapshot)` | 接收并处理 handle room member action results 相关逻辑。 |
| [L521](../src/apps/controller/ControllerMainWindow.Room.cpp#L521) | `ControllerMainWindow::ShowRoomMemberContextMenu` | 定义 | `void ControllerMainWindow::ShowRoomMemberContextMenu( const QPoint& position)` | 实现 show room member context menu 对应的业务或工具逻辑。 |

## `src/apps/controller/ControllerMainWindow.RoomUi.cpp`

[打开源码](../src/apps/controller/ControllerMainWindow.RoomUi.cpp) · **文件作用：** 实现 controller main window room ui 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L35](../src/apps/controller/ControllerMainWindow.RoomUi.cpp#L35) | `ControllerMainWindow::UpdateRoomUi` | 定义 | `void ControllerMainWindow::UpdateRoomUi( const SessionEngineSnapshot& snapshot)` | 更新或应用 update room ui 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.Session.cpp`

[打开源码](../src/apps/controller/ControllerMainWindow.Session.cpp) · **文件作用：** 实现 controller main window session 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L28](../src/apps/controller/ControllerMainWindow.Session.cpp#L28) | `ControllerMainWindow::StartSession` | 定义 | `void ControllerMainWindow::StartSession(const QString& deviceId, const QString& deviceName, SessionPurpose purpose)` | 启动 start session 相关逻辑。 |
| [L50](../src/apps/controller/ControllerMainWindow.Session.cpp#L50) | `ControllerMainWindow::StartAssistedSession` | 定义 | `void ControllerMainWindow::StartAssistedSession( const QString& deviceId, const QString& verificationCode)` | 启动 start assisted session 相关逻辑。 |
| [L83](../src/apps/controller/ControllerMainWindow.Session.cpp#L83) | `ControllerMainWindow::HandleRemoteSessionDisconnect` | 定义 | `void ControllerMainWindow::HandleRemoteSessionDisconnect()` | 接收并处理 handle remote session disconnect 相关逻辑。 |
| [L131](../src/apps/controller/ControllerMainWindow.Session.cpp#L131) | `ControllerMainWindow::SetRoomActionHint` | 定义 | `void ControllerMainWindow::SetRoomActionHint(const QString& text, bool error)` | 更新或应用 set room action hint 相关逻辑。 |
| [L143](../src/apps/controller/ControllerMainWindow.Session.cpp#L143) | `ControllerMainWindow::OpenCameraWindow` | 定义 | `void ControllerMainWindow::OpenCameraWindow(const QString& deviceId, const QString& deviceName)` | 启动 open camera window 相关逻辑。 |
| [L161](../src/apps/controller/ControllerMainWindow.Session.cpp#L161) | `ControllerMainWindow::OpenRemoteSession` | 定义 | `void ControllerMainWindow::OpenRemoteSession( RemoteSessionBinding binding)` | 启动 open remote session 相关逻辑。 |
| [L326](../src/apps/controller/ControllerMainWindow.Session.cpp#L326) | `ControllerMainWindow::SetRuntimeStatus` | 定义 | `void ControllerMainWindow::SetRuntimeStatus(const QString& status, const QString& color)` | 更新或应用 set runtime status 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.SessionState.cpp`

[打开源码](../src/apps/controller/ControllerMainWindow.SessionState.cpp) · **文件作用：** 实现 controller main window session state 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L35](../src/apps/controller/ControllerMainWindow.SessionState.cpp#L35) | `ControllerMainWindow::OnSessionEngineSnapshot` | 定义 | `void ControllerMainWindow::OnSessionEngineSnapshot( const SessionEngineSnapshot& snapshot)` | 接收并处理 on session engine snapshot 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.SettingsPage.cpp`

[打开源码](../src/apps/controller/ControllerMainWindow.SettingsPage.cpp) · **文件作用：** 实现 controller main window settings page 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L13](../src/apps/controller/ControllerMainWindow.SettingsPage.cpp#L13) | `ControllerMainWindow::BuildSettingsPage` | 定义 | `void ControllerMainWindow::BuildSettingsPage()` | 创建或初始化 build settings page 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.ShellRoomPage.cpp`

[打开源码](../src/apps/controller/ControllerMainWindow.ShellRoomPage.cpp) · **文件作用：** 实现 controller main window shell room page 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L29](../src/apps/controller/ControllerMainWindow.ShellRoomPage.cpp#L29) | `ControllerMainWindow::BuildShellAndRoomPage` | 定义 | `void ControllerMainWindow::BuildShellAndRoomPage()` | 创建或初始化 build shell and room page 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.Ui.cpp`

[打开源码](../src/apps/controller/ControllerMainWindow.Ui.cpp) · **文件作用：** 实现 controller main window ui 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L11](../src/apps/controller/ControllerMainWindow.Ui.cpp#L11) | `ControllerMainWindow::BuildUi` | 定义 | `void ControllerMainWindow::BuildUi()` | 创建或初始化 build ui 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.UiConnections.cpp`

[打开源码](../src/apps/controller/ControllerMainWindow.UiConnections.cpp) · **文件作用：** 实现 controller main window ui connections 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L50](../src/apps/controller/ControllerMainWindow.UiConnections.cpp#L50) | `ControllerMainWindow::BuildHelpAndAuthorPages` | 定义 | `void ControllerMainWindow::BuildHelpAndAuthorPages()` | 创建或初始化 build help and author pages 相关逻辑。 |
| [L166](../src/apps/controller/ControllerMainWindow.UiConnections.cpp#L166) | `ControllerMainWindow::ConnectUiSignals` | 定义 | `void ControllerMainWindow::ConnectUiSignals()` | 建立连接 connect ui signals 相关逻辑。 |

## `src/apps/controller/ControllerMainWindowSupport.cpp`

[打开源码](../src/apps/controller/ControllerMainWindowSupport.cpp) · **文件作用：** 实现 controller main window support 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L135](../src/apps/controller/ControllerMainWindowSupport.cpp#L135) | `IsNineDigitPublicId` | 定义 | `bool IsNineDigitPublicId(const QString& value)` | 判断 is nine digit public id 相关逻辑。 |
| [L142](../src/apps/controller/ControllerMainWindowSupport.cpp#L142) | `LocalizedDirectSessionError` | 定义 | `QString LocalizedDirectSessionError( const std::string& errorCode, const std::string& /*fallbackMessage*/, const SessionEngineSnapshot* snapshot)` | 实现 localized direct session error 对应的业务或工具逻辑。 |
| [L204](../src/apps/controller/ControllerMainWindowSupport.cpp#L204) | `IsDirectRecoveryFailureCode` | 定义 | `bool IsDirectRecoveryFailureCode(const std::string& errorCode)` | 判断 is direct recovery failure code 相关逻辑。 |
| [L284](../src/apps/controller/ControllerMainWindowSupport.cpp#L284) | `InstalledChineseInterfaceFonts` | 定义 | `QStringList InstalledChineseInterfaceFonts()` | 实现 installed chinese interface fonts 对应的业务或工具逻辑。 |
| [L302](../src/apps/controller/ControllerMainWindowSupport.cpp#L302) | `ApplyInterfaceFontPreference` | 定义 | `void ApplyInterfaceFontPreference(const QString& systemFamily, int pixelSize)` | 更新或应用 apply interface font preference 相关逻辑。 |
| [L321](../src/apps/controller/ControllerMainWindowSupport.cpp#L321) | `AdaptBenchmarkHtmlForTheme` | 定义 | `QString AdaptBenchmarkHtmlForTheme(QString html)` | 实现 adapt benchmark html for theme 对应的业务或工具逻辑。 |
| [L347](../src/apps/controller/ControllerMainWindowSupport.cpp#L347) | `VideoEncoderPreferenceFromSetting` | 定义 | `VideoEncoderPreference VideoEncoderPreferenceFromSetting( const QString& value)` | 实现 video encoder preference from setting 对应的业务或工具逻辑。 |
| [L365](../src/apps/controller/ControllerMainWindowSupport.cpp#L365) | `DesktopCaptureImplementationFromSetting` | 定义 | `DesktopCaptureImplementation DesktopCaptureImplementationFromSetting( const QString& value)` | 实现 desktop capture implementation from setting 对应的业务或工具逻辑。 |
| [L373](../src/apps/controller/ControllerMainWindowSupport.cpp#L373) | `FfmpegHardwareBackendFromSetting` | 定义 | `FfmpegHardwareBackend FfmpegHardwareBackendFromSetting( const QString& value)` | 实现 ffmpeg hardware backend from setting 对应的业务或工具逻辑。 |
| [L388](../src/apps/controller/ControllerMainWindowSupport.cpp#L388) | `EncoderQualityFromSetting` | 定义 | `FfmpegX264Preset EncoderQualityFromSetting(const QString& value)` | 编码 encoder quality from setting 相关逻辑。 |
| [L405](../src/apps/controller/ControllerMainWindowSupport.cpp#L405) | `VideoDecoderPreferenceFromSetting` | 定义 | `VideoDecoderPreference VideoDecoderPreferenceFromSetting( const QString& value)` | 实现 video decoder preference from setting 对应的业务或工具逻辑。 |
| [L435](../src/apps/controller/ControllerMainWindowSupport.cpp#L435) | `InitialClipboardCacheBaseDirectory` | 定义 | `QString InitialClipboardCacheBaseDirectory()` | 创建或初始化 initial clipboard cache base directory 相关逻辑。 |
| [L442](../src/apps/controller/ControllerMainWindowSupport.cpp#L442) | `ClipboardCacheRootForBase` | 定义 | `QString ClipboardCacheRootForBase(const QString& baseDirectory)` | 实现 clipboard cache root for base 对应的业务或工具逻辑。 |
| [L448](../src/apps/controller/ControllerMainWindowSupport.cpp#L448) | `SafeClipboardCacheCapacityGiB` | 定义 | `qulonglong SafeClipboardCacheCapacityGiB(const QString& baseDirectory)` | 实现 safe clipboard cache capacity gi b 对应的业务或工具逻辑。 |
| [L457](../src/apps/controller/ControllerMainWindowSupport.cpp#L457) | `WindowsAutoStartCommand` | 定义 | `QString WindowsAutoStartCommand()` | 实现 windows auto start command 对应的业务或工具逻辑。 |
| [L463](../src/apps/controller/ControllerMainWindowSupport.cpp#L463) | `WindowsAutoStartEnabled` | 定义 | `bool WindowsAutoStartEnabled()` | 实现 windows auto start enabled 对应的业务或工具逻辑。 |
| [L477](../src/apps/controller/ControllerMainWindowSupport.cpp#L477) | `SetWindowsAutoStartEnabled` | 定义 | `bool SetWindowsAutoStartEnabled(bool enabled)` | 更新或应用 set windows auto start enabled 相关逻辑。 |
| [L500](../src/apps/controller/ControllerMainWindowSupport.cpp#L500) | `SavedScreenQualityBounds` | 定义 | `std::pair<std::uint32_t, std::uint32_t> SavedScreenQualityBounds( ScreenQualityTier quality)` | 保存或写入 saved screen quality bounds 相关逻辑。 |
| [L517](../src/apps/controller/ControllerMainWindowSupport.cpp#L517) | `FormatBitrate` | 定义 | `QString FormatBitrate(std::uint64_t bitsPerSecond)` | 实现 format bitrate 对应的业务或工具逻辑。 |
| [L532](../src/apps/controller/ControllerMainWindowSupport.cpp#L532) | `FormatByteCount` | 定义 | `QString FormatByteCount(std::uint64_t bytes)` | 实现 format byte count 对应的业务或工具逻辑。 |
| [L553](../src/apps/controller/ControllerMainWindowSupport.cpp#L553) | `SampleWindowSuffix` | 定义 | `QString SampleWindowSuffix(std::uint32_t windowMs)` | 实现 sample window suffix 对应的业务或工具逻辑。 |
| [L560](../src/apps/controller/ControllerMainWindowSupport.cpp#L560) | `LatestFrameTimingText` | 定义 | `QString LatestFrameTimingText( const RtpStreamStatsSnapshot& stream, const QString& action)` | 实现 latest frame timing text 对应的业务或工具逻辑。 |
| [L586](../src/apps/controller/ControllerMainWindowSupport.cpp#L586) | `RouteDisplayName` | 定义 | `QString RouteDisplayName(const std::string& route)` | 实现 route display name 对应的业务或工具逻辑。 |
| [L600](../src/apps/controller/ControllerMainWindowSupport.cpp#L600) | `SlotDisplayName` | 定义 | `QString SlotDisplayName(const std::string& slot, const std::string& kind)` | 实现 slot display name 对应的业务或工具逻辑。 |
| [L616](../src/apps/controller/ControllerMainWindowSupport.cpp#L616) | `CandidateDisplayText` | 定义 | `QString CandidateDisplayText( const IceCandidateStatsSnapshot& candidate)` | 判断 candidate display text 相关逻辑。 |
| [L640](../src/apps/controller/ControllerMainWindowSupport.cpp#L640) | `InitialFileSaveDirectory` | 定义 | `QString InitialFileSaveDirectory()` | 创建或初始化 initial file save directory 相关逻辑。 |
| [L650](../src/apps/controller/ControllerMainWindowSupport.cpp#L650) | `SetBusyStatusAnimation` | 定义 | `void SetBusyStatusAnimation(QLabel* label, bool busy)` | 更新或应用 set busy status animation 相关逻辑。 |
| [L682](../src/apps/controller/ControllerMainWindowSupport.cpp#L682) | `AnimateSmallUiChange` | 定义 | `void AnimateSmallUiChange(QWidget* widget)` | 实现 animate small ui change 对应的业务或工具逻辑。 |
| [L1458](../src/apps/controller/ControllerMainWindowSupport.cpp#L1458) | `DrawNavigationIcon` | 定义 | `QPixmap DrawNavigationIcon(NavigationIcon icon, const QColor& color)` | 准备或呈现 draw navigation icon 相关逻辑。 |
| [L1502](../src/apps/controller/ControllerMainWindowSupport.cpp#L1502) | `MakeNavigationIcon` | 定义 | `QIcon MakeNavigationIcon(NavigationIcon icon, bool dark)` | 创建或初始化 make navigation icon 相关逻辑。 |
| [L1520](../src/apps/controller/ControllerMainWindowSupport.cpp#L1520) | `MakePageSurface` | 定义 | `QScrollArea* MakePageSurface(const QString& title, const QString& subtitle, QWidget* parent, QVBoxLayout** pageLayout)` | 创建或初始化 make page surface 相关逻辑。 |
| [L1549](../src/apps/controller/ControllerMainWindowSupport.cpp#L1549) | `MakeNavigationButton` | 定义 | `QPushButton* MakeNavigationButton(const QString& text, NavigationIcon icon, bool active, QWidget* parent)` | 创建或初始化 make navigation button 相关逻辑。 |
| [L1619](../src/apps/controller/ControllerMainWindowSupport.cpp#L1619) | `MakeDivider` | 定义 | `QFrame* MakeDivider(QWidget* parent)` | 创建或初始化 make divider 相关逻辑。 |
| [L1627](../src/apps/controller/ControllerMainWindowSupport.cpp#L1627) | `AddRoomCapacityItems` | 定义 | `void AddRoomCapacityItems(QComboBox* comboBox)` | 实现 add room capacity items 对应的业务或工具逻辑。 |
| [L1643](../src/apps/controller/ControllerMainWindowSupport.cpp#L1643) | `MemberDisplayName` | 定义 | `QString MemberDisplayName(const RoomSnapshot& room, const std::string& deviceId, const std::string& localDeviceId)` | 实现 member display name 对应的业务或工具逻辑。 |
| [L1667](../src/apps/controller/ControllerMainWindowSupport.cpp#L1667) | `ConnectivityDebugName` | 定义 | `QString ConnectivityDebugName(SessionConnectivityState state)` | 建立连接 connectivity debug name 相关逻辑。 |
| [L1684](../src/apps/controller/ControllerMainWindowSupport.cpp#L1684) | `RoomMembershipDebugName` | 定义 | `QString RoomMembershipDebugName(RoomMembershipState state)` | 实现 room membership debug name 对应的业务或工具逻辑。 |
| [L1703](../src/apps/controller/ControllerMainWindowSupport.cpp#L1703) | `PeerConnectionDebugName` | 定义 | `QString PeerConnectionDebugName(RoomPeerConnectionState state)` | 实现 peer connection debug name 对应的业务或工具逻辑。 |
| [L1726](../src/apps/controller/ControllerMainWindowSupport.cpp#L1726) | `MediaDeviceSelectionDebugName` | 定义 | `QString MediaDeviceSelectionDebugName( MediaDeviceSelectionState state)` | 实现 media device selection debug name 对应的业务或工具逻辑。 |
| [L1742](../src/apps/controller/ControllerMainWindowSupport.cpp#L1742) | `MediaDeviceDebugName` | 定义 | `QString MediaDeviceDebugName( const MediaDeviceCategorySnapshot& category, const std::string& deviceId)` | 实现 media device debug name 对应的业务或工具逻辑。 |
| [L1766](../src/apps/controller/ControllerMainWindowSupport.cpp#L1766) | `MediaDeviceCategoryDebugText` | 定义 | `QString MediaDeviceCategoryDebugText( const MediaDeviceCategorySnapshot& category)` | 实现 media device category debug text 对应的业务或工具逻辑。 |

## `src/apps/controller/ControllerMainWindowSupport.h`

[打开源码](../src/apps/controller/ControllerMainWindowSupport.h) · **文件作用：** 声明 controller main window support 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L24](../src/apps/controller/ControllerMainWindowSupport.h#L24) | `QComboBox` | class | 定义 QComboBox 的 class 类型和相关状态。 |
| [L25](../src/apps/controller/ControllerMainWindowSupport.h#L25) | `QFrame` | class | 定义 QFrame 的 class 类型和相关状态。 |
| [L26](../src/apps/controller/ControllerMainWindowSupport.h#L26) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L27](../src/apps/controller/ControllerMainWindowSupport.h#L27) | `QPushButton` | class | 定义 QPushButton 的 class 类型和相关状态。 |
| [L28](../src/apps/controller/ControllerMainWindowSupport.h#L28) | `QScrollArea` | class | 定义 QScrollArea 的 class 类型和相关状态。 |
| [L29](../src/apps/controller/ControllerMainWindowSupport.h#L29) | `QVBoxLayout` | class | 定义 QVBoxLayout 的 class 类型和相关状态。 |
| [L30](../src/apps/controller/ControllerMainWindowSupport.h#L30) | `QWidget` | class | 定义 QWidget 的 class 类型和相关状态。 |
| [L125](../src/apps/controller/ControllerMainWindowSupport.h#L125) | `NavigationIcon` | enum class | 定义 NavigationIcon 的 enum class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L24](../src/apps/controller/ControllerMainWindowSupport.h#L24) | `QComboBox` | `class QComboBox;` | 保存 q combo box 相关配置或运行状态。 |
| [L25](../src/apps/controller/ControllerMainWindowSupport.h#L25) | `QFrame` | `class QFrame;` | 保存媒体帧、图像或缓冲资源：q frame。 |
| [L26](../src/apps/controller/ControllerMainWindowSupport.h#L26) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L27](../src/apps/controller/ControllerMainWindowSupport.h#L27) | `QPushButton` | `class QPushButton;` | 保存 q push button 相关配置或运行状态。 |
| [L28](../src/apps/controller/ControllerMainWindowSupport.h#L28) | `QScrollArea` | `class QScrollArea;` | 保存 q scroll area 相关配置或运行状态。 |
| [L29](../src/apps/controller/ControllerMainWindowSupport.h#L29) | `QVBoxLayout` | `class QVBoxLayout;` | 保存 qv box layout 相关配置或运行状态。 |
| [L30](../src/apps/controller/ControllerMainWindowSupport.h#L30) | `QWidget` | `class QWidget;` | 保存 q widget 相关配置或运行状态。 |
| [L34](../src/apps/controller/ControllerMainWindowSupport.h#L34) | `kDefaultFileSaveDirectorySetting` | `extern const char kDefaultFileSaveDirectorySetting[];` | 定义 default file save directory setting 的编译期常量或产品边界。 |
| [L35](../src/apps/controller/ControllerMainWindowSupport.h#L35) | `kVideoEncoderPreferenceSetting` | `extern const char kVideoEncoderPreferenceSetting[];` | 定义 video encoder preference setting 的编译期常量或产品边界。 |
| [L36](../src/apps/controller/ControllerMainWindowSupport.h#L36) | `kFfmpegX264PresetSetting` | `extern const char kFfmpegX264PresetSetting[];` | 定义 ffmpeg x264 preset setting 的编译期常量或产品边界。 |
| [L37](../src/apps/controller/ControllerMainWindowSupport.h#L37) | `kFfmpegHardwareBackendSetting` | `extern const char kFfmpegHardwareBackendSetting[];` | 定义 ffmpeg hardware backend setting 的编译期常量或产品边界。 |
| [L38](../src/apps/controller/ControllerMainWindowSupport.h#L38) | `kVideoDecoderPreferenceSetting` | `extern const char kVideoDecoderPreferenceSetting[];` | 定义 video decoder preference setting 的编译期常量或产品边界。 |
| [L39](../src/apps/controller/ControllerMainWindowSupport.h#L39) | `kVideoRendererPreferenceSetting` | `extern const char kVideoRendererPreferenceSetting[];` | 定义 video renderer preference setting 的编译期常量或产品边界。 |
| [L40](../src/apps/controller/ControllerMainWindowSupport.h#L40) | `kDesktopCaptureBackendSetting` | `extern const char kDesktopCaptureBackendSetting[];` | 定义 desktop capture backend setting 的编译期常量或产品边界。 |
| [L41](../src/apps/controller/ControllerMainWindowSupport.h#L41) | `kScreenFrameRateLogEnabledSetting` | `extern const char kScreenFrameRateLogEnabledSetting[];` | 定义 screen frame rate log enabled setting 的编译期常量或产品边界。 |
| [L42](../src/apps/controller/ControllerMainWindowSupport.h#L42) | `kInputEventStatsEnabledSetting` | `extern const char kInputEventStatsEnabledSetting[];` | 定义 input event stats enabled setting 的编译期常量或产品边界。 |
| [L43](../src/apps/controller/ControllerMainWindowSupport.h#L43) | `kBestDecoderNameSetting` | `extern const char kBestDecoderNameSetting[];` | 定义 best decoder name setting 的编译期常量或产品边界。 |
| [L44](../src/apps/controller/ControllerMainWindowSupport.h#L44) | `kBestDecoderAverageSetting` | `extern const char kBestDecoderAverageSetting[];` | 定义 best decoder average setting 的编译期常量或产品边界。 |
| [L45](../src/apps/controller/ControllerMainWindowSupport.h#L45) | `kBestDecoderP95Setting` | `extern const char kBestDecoderP95Setting[];` | 定义 best decoder p95 setting 的编译期常量或产品边界。 |
| [L46](../src/apps/controller/ControllerMainWindowSupport.h#L46) | `kBestDecoderTestTimeSetting` | `extern const char kBestDecoderTestTimeSetting[];` | 定义 best decoder test time setting 的编译期常量或产品边界。 |
| [L47](../src/apps/controller/ControllerMainWindowSupport.h#L47) | `kDecoderCandidatesSetting` | `extern const char kDecoderCandidatesSetting[];` | 定义 decoder candidates setting 的编译期常量或产品边界。 |
| [L48](../src/apps/controller/ControllerMainWindowSupport.h#L48) | `kDecoderHardwareFingerprintSetting` | `extern const char kDecoderHardwareFingerprintSetting[];` | 定义 decoder hardware fingerprint setting 的编译期常量或产品边界。 |
| [L49](../src/apps/controller/ControllerMainWindowSupport.h#L49) | `kDecoderBenchmarkCompletedSetting` | `extern const char kDecoderBenchmarkCompletedSetting[];` | 定义 decoder benchmark completed setting 的编译期常量或产品边界。 |
| [L50](../src/apps/controller/ControllerMainWindowSupport.h#L50) | `kDecoderBenchmarkPassedSetting` | `extern const char kDecoderBenchmarkPassedSetting[];` | 定义 decoder benchmark passed setting 的编译期常量或产品边界。 |
| [L51](../src/apps/controller/ControllerMainWindowSupport.h#L51) | `kDecoderBenchmarkPolicyVersionSetting` | `extern const char kDecoderBenchmarkPolicyVersionSetting[];` | 定义 decoder benchmark policy version setting 的编译期常量或产品边界。 |
| [L52](../src/apps/controller/ControllerMainWindowSupport.h#L52) | `kDecoderBenchmarkPolicyVersion` | `extern const int kDecoderBenchmarkPolicyVersion;` | 定义 decoder benchmark policy version 的编译期常量或产品边界。 |
| [L53](../src/apps/controller/ControllerMainWindowSupport.h#L53) | `kBestEncoderIdSetting` | `extern const char kBestEncoderIdSetting[];` | 定义 best encoder id setting 的编译期常量或产品边界。 |
| [L54](../src/apps/controller/ControllerMainWindowSupport.h#L54) | `kBestEncoderNameSetting` | `extern const char kBestEncoderNameSetting[];` | 定义 best encoder name setting 的编译期常量或产品边界。 |
| [L55](../src/apps/controller/ControllerMainWindowSupport.h#L55) | `kEncoderCandidatesSetting` | `extern const char kEncoderCandidatesSetting[];` | 定义 encoder candidates setting 的编译期常量或产品边界。 |
| [L56](../src/apps/controller/ControllerMainWindowSupport.h#L56) | `kEncoderHardwareFingerprintSetting` | `extern const char kEncoderHardwareFingerprintSetting[];` | 定义 encoder hardware fingerprint setting 的编译期常量或产品边界。 |
| [L57](../src/apps/controller/ControllerMainWindowSupport.h#L57) | `kEncoderCaptureBackendSetting` | `extern const char kEncoderCaptureBackendSetting[];` | 定义 encoder capture backend setting 的编译期常量或产品边界。 |
| [L58](../src/apps/controller/ControllerMainWindowSupport.h#L58) | `kEncoderX264PresetSetting` | `extern const char kEncoderX264PresetSetting[];` | 定义 encoder x264 preset setting 的编译期常量或产品边界。 |
| [L59](../src/apps/controller/ControllerMainWindowSupport.h#L59) | `kEncoderBenchmarkCompletedSetting` | `extern const char kEncoderBenchmarkCompletedSetting[];` | 定义 encoder benchmark completed setting 的编译期常量或产品边界。 |
| [L60](../src/apps/controller/ControllerMainWindowSupport.h#L60) | `kEncoderBenchmarkPassedSetting` | `extern const char kEncoderBenchmarkPassedSetting[];` | 定义 encoder benchmark passed setting 的编译期常量或产品边界。 |
| [L61](../src/apps/controller/ControllerMainWindowSupport.h#L61) | `kEncoderBenchmarkPolicyVersionSetting` | `extern const char kEncoderBenchmarkPolicyVersionSetting[];` | 定义 encoder benchmark policy version setting 的编译期常量或产品边界。 |
| [L62](../src/apps/controller/ControllerMainWindowSupport.h#L62) | `kBestEncoderTestTimeSetting` | `extern const char kBestEncoderTestTimeSetting[];` | 定义 best encoder test time setting 的编译期常量或产品边界。 |
| [L63](../src/apps/controller/ControllerMainWindowSupport.h#L63) | `kEncoderBenchmarkPolicyVersion` | `extern const int kEncoderBenchmarkPolicyVersion;` | 定义 encoder benchmark policy version 的编译期常量或产品边界。 |
| [L64](../src/apps/controller/ControllerMainWindowSupport.h#L64) | `kCameraDeviceSetting` | `extern const char kCameraDeviceSetting[];` | 定义 camera device setting 的编译期常量或产品边界。 |
| [L65](../src/apps/controller/ControllerMainWindowSupport.h#L65) | `kMicrophoneDeviceSetting` | `extern const char kMicrophoneDeviceSetting[];` | 定义 microphone device setting 的编译期常量或产品边界。 |
| [L66](../src/apps/controller/ControllerMainWindowSupport.h#L66) | `kSpeakerDeviceSetting` | `extern const char kSpeakerDeviceSetting[];` | 定义 speaker device setting 的编译期常量或产品边界。 |
| [L67](../src/apps/controller/ControllerMainWindowSupport.h#L67) | `kDefaultRoomCapacitySetting` | `extern const char kDefaultRoomCapacitySetting[];` | 定义 default room capacity setting 的编译期常量或产品边界。 |
| [L68](../src/apps/controller/ControllerMainWindowSupport.h#L68) | `kCloseButtonBehaviorSetting` | `extern const char kCloseButtonBehaviorSetting[];` | 定义 close button behavior setting 的编译期常量或产品边界。 |
| [L69](../src/apps/controller/ControllerMainWindowSupport.h#L69) | `kStartupVisibilitySetting` | `extern const char kStartupVisibilitySetting[];` | 定义 startup visibility setting 的编译期常量或产品边界。 |
| [L70](../src/apps/controller/ControllerMainWindowSupport.h#L70) | `kWindowsAutoStartValueName` | `extern const char kWindowsAutoStartValueName[];` | 定义 windows auto start value name 的编译期常量或产品边界。 |
| [L71](../src/apps/controller/ControllerMainWindowSupport.h#L71) | `kAutoOpenCameraGallerySetting` | `extern const char kAutoOpenCameraGallerySetting[];` | 定义 auto open camera gallery setting 的编译期常量或产品边界。 |
| [L72](../src/apps/controller/ControllerMainWindowSupport.h#L72) | `kInterfaceFontFamilySetting` | `extern const char kInterfaceFontFamilySetting[];` | 定义 interface font family setting 的编译期常量或产品边界。 |
| [L73](../src/apps/controller/ControllerMainWindowSupport.h#L73) | `kRemoteScreenQualitySetting` | `extern const char kRemoteScreenQualitySetting[];` | 定义 remote screen quality setting 的编译期常量或产品边界。 |
| [L74](../src/apps/controller/ControllerMainWindowSupport.h#L74) | `kDragPointerSampleRateSetting` | `extern const char kDragPointerSampleRateSetting[];` | 定义 drag pointer sample rate setting 的编译期常量或产品边界。 |
| [L75](../src/apps/controller/ControllerMainWindowSupport.h#L75) | `kRemotePasteEnabledSetting` | `extern const char kRemotePasteEnabledSetting[];` | 定义 remote paste enabled setting 的编译期常量或产品边界。 |
| [L76](../src/apps/controller/ControllerMainWindowSupport.h#L76) | `kClipboardFormatsSetting` | `extern const char kClipboardFormatsSetting[];` | 定义 clipboard formats setting 的编译期常量或产品边界。 |
| [L77](../src/apps/controller/ControllerMainWindowSupport.h#L77) | `kClipboardFileLimitSetting` | `extern const char kClipboardFileLimitSetting[];` | 定义 clipboard file limit setting 的编译期常量或产品边界。 |
| [L78](../src/apps/controller/ControllerMainWindowSupport.h#L78) | `kClipboardCacheBaseDirectorySetting` | `extern const char kClipboardCacheBaseDirectorySetting[];` | 定义 clipboard cache base directory setting 的编译期常量或产品边界。 |
| [L79](../src/apps/controller/ControllerMainWindowSupport.h#L79) | `kClipboardCacheRetentionSetting` | `extern const char kClipboardCacheRetentionSetting[];` | 定义 clipboard cache retention setting 的编译期常量或产品边界。 |
| [L80](../src/apps/controller/ControllerMainWindowSupport.h#L80) | `kClipboardCacheCapacitySetting` | `extern const char kClipboardCacheCapacitySetting[];` | 定义 clipboard cache capacity setting 的编译期常量或产品边界。 |
| [L81](../src/apps/controller/ControllerMainWindowSupport.h#L81) | `kRemotePastePopupThresholdBytes` | `extern const std::uint64_t kRemotePastePopupThresholdBytes;` | 定义 remote paste popup threshold bytes 的编译期常量或产品边界。 |
| [L82](../src/apps/controller/ControllerMainWindowSupport.h#L82) | `kSettingsControlWidth` | `extern const int kSettingsControlWidth;` | 定义 settings control width 的编译期常量或产品边界。 |
| [L83](../src/apps/controller/ControllerMainWindowSupport.h#L83) | `kMainStyle` | `extern const char kMainStyle[];` | 定义 main style 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L85](../src/apps/controller/ControllerMainWindowSupport.h#L85) | `IsNineDigitPublicId` | 声明 | `bool IsNineDigitPublicId(const QString& value)` | 判断 is nine digit public id 相关逻辑。 |
| [L86](../src/apps/controller/ControllerMainWindowSupport.h#L86) | `LocalizedDirectSessionError` | 声明 | `QString LocalizedDirectSessionError( const std::string& errorCode, const std::string& fallbackMessage, const SessionEngineSnapshot* snapshot = nullptr)` | 实现 localized direct session error 对应的业务或工具逻辑。 |
| [L90](../src/apps/controller/ControllerMainWindowSupport.h#L90) | `IsDirectRecoveryFailureCode` | 声明 | `bool IsDirectRecoveryFailureCode(const std::string& errorCode)` | 判断 is direct recovery failure code 相关逻辑。 |
| [L91](../src/apps/controller/ControllerMainWindowSupport.h#L91) | `InstalledChineseInterfaceFonts` | 声明 | `QStringList InstalledChineseInterfaceFonts()` | 实现 installed chinese interface fonts 对应的业务或工具逻辑。 |
| [L92](../src/apps/controller/ControllerMainWindowSupport.h#L92) | `ApplyInterfaceFontPreference` | 声明 | `void ApplyInterfaceFontPreference( const QString& systemFamily, int pixelSize)` | 更新或应用 apply interface font preference 相关逻辑。 |
| [L94](../src/apps/controller/ControllerMainWindowSupport.h#L94) | `AdaptBenchmarkHtmlForTheme` | 声明 | `QString AdaptBenchmarkHtmlForTheme(QString html)` | 实现 adapt benchmark html for theme 对应的业务或工具逻辑。 |
| [L95](../src/apps/controller/ControllerMainWindowSupport.h#L95) | `VideoEncoderPreferenceFromSetting` | 声明 | `VideoEncoderPreference VideoEncoderPreferenceFromSetting( const QString& value)` | 实现 video encoder preference from setting 对应的业务或工具逻辑。 |
| [L97](../src/apps/controller/ControllerMainWindowSupport.h#L97) | `DesktopCaptureImplementationFromSetting` | 声明 | `DesktopCaptureImplementation DesktopCaptureImplementationFromSetting( const QString& value)` | 实现 desktop capture implementation from setting 对应的业务或工具逻辑。 |
| [L99](../src/apps/controller/ControllerMainWindowSupport.h#L99) | `FfmpegHardwareBackendFromSetting` | 声明 | `FfmpegHardwareBackend FfmpegHardwareBackendFromSetting( const QString& value)` | 实现 ffmpeg hardware backend from setting 对应的业务或工具逻辑。 |
| [L101](../src/apps/controller/ControllerMainWindowSupport.h#L101) | `EncoderQualityFromSetting` | 声明 | `FfmpegX264Preset EncoderQualityFromSetting(const QString& value)` | 编码 encoder quality from setting 相关逻辑。 |
| [L102](../src/apps/controller/ControllerMainWindowSupport.h#L102) | `VideoDecoderPreferenceFromSetting` | 声明 | `VideoDecoderPreference VideoDecoderPreferenceFromSetting( const QString& value)` | 实现 video decoder preference from setting 对应的业务或工具逻辑。 |
| [L104](../src/apps/controller/ControllerMainWindowSupport.h#L104) | `InitialClipboardCacheBaseDirectory` | 声明 | `QString InitialClipboardCacheBaseDirectory()` | 创建或初始化 initial clipboard cache base directory 相关逻辑。 |
| [L105](../src/apps/controller/ControllerMainWindowSupport.h#L105) | `ClipboardCacheRootForBase` | 声明 | `QString ClipboardCacheRootForBase(const QString& baseDirectory)` | 实现 clipboard cache root for base 对应的业务或工具逻辑。 |
| [L106](../src/apps/controller/ControllerMainWindowSupport.h#L106) | `SafeClipboardCacheCapacityGiB` | 声明 | `qulonglong SafeClipboardCacheCapacityGiB(const QString& baseDirectory)` | 实现 safe clipboard cache capacity gi b 对应的业务或工具逻辑。 |
| [L107](../src/apps/controller/ControllerMainWindowSupport.h#L107) | `WindowsAutoStartCommand` | 声明 | `QString WindowsAutoStartCommand()` | 实现 windows auto start command 对应的业务或工具逻辑。 |
| [L108](../src/apps/controller/ControllerMainWindowSupport.h#L108) | `WindowsAutoStartEnabled` | 声明 | `bool WindowsAutoStartEnabled()` | 实现 windows auto start enabled 对应的业务或工具逻辑。 |
| [L109](../src/apps/controller/ControllerMainWindowSupport.h#L109) | `SetWindowsAutoStartEnabled` | 声明 | `bool SetWindowsAutoStartEnabled(bool enabled)` | 更新或应用 set windows auto start enabled 相关逻辑。 |
| [L110](../src/apps/controller/ControllerMainWindowSupport.h#L110) | `SavedScreenQualityBounds` | 声明 | `std::pair<std::uint32_t, std::uint32_t> SavedScreenQualityBounds( ScreenQualityTier quality)` | 保存或写入 saved screen quality bounds 相关逻辑。 |
| [L112](../src/apps/controller/ControllerMainWindowSupport.h#L112) | `FormatBitrate` | 声明 | `QString FormatBitrate(std::uint64_t bitsPerSecond)` | 实现 format bitrate 对应的业务或工具逻辑。 |
| [L113](../src/apps/controller/ControllerMainWindowSupport.h#L113) | `FormatByteCount` | 声明 | `QString FormatByteCount(std::uint64_t bytes)` | 实现 format byte count 对应的业务或工具逻辑。 |
| [L114](../src/apps/controller/ControllerMainWindowSupport.h#L114) | `SampleWindowSuffix` | 声明 | `QString SampleWindowSuffix(std::uint32_t windowMs)` | 实现 sample window suffix 对应的业务或工具逻辑。 |
| [L115](../src/apps/controller/ControllerMainWindowSupport.h#L115) | `LatestFrameTimingText` | 声明 | `QString LatestFrameTimingText( const RtpStreamStatsSnapshot& stream, const QString& action)` | 实现 latest frame timing text 对应的业务或工具逻辑。 |
| [L117](../src/apps/controller/ControllerMainWindowSupport.h#L117) | `RouteDisplayName` | 声明 | `QString RouteDisplayName(const std::string& route)` | 实现 route display name 对应的业务或工具逻辑。 |
| [L118](../src/apps/controller/ControllerMainWindowSupport.h#L118) | `SlotDisplayName` | 声明 | `QString SlotDisplayName( const std::string& slot, const std::string& kind)` | 实现 slot display name 对应的业务或工具逻辑。 |
| [L120](../src/apps/controller/ControllerMainWindowSupport.h#L120) | `CandidateDisplayText` | 声明 | `QString CandidateDisplayText(const IceCandidateStatsSnapshot& candidate)` | 判断 candidate display text 相关逻辑。 |
| [L121](../src/apps/controller/ControllerMainWindowSupport.h#L121) | `InitialFileSaveDirectory` | 声明 | `QString InitialFileSaveDirectory()` | 创建或初始化 initial file save directory 相关逻辑。 |
| [L122](../src/apps/controller/ControllerMainWindowSupport.h#L122) | `SetBusyStatusAnimation` | 声明 | `void SetBusyStatusAnimation(QLabel* label, bool busy)` | 更新或应用 set busy status animation 相关逻辑。 |
| [L123](../src/apps/controller/ControllerMainWindowSupport.h#L123) | `AnimateSmallUiChange` | 声明 | `void AnimateSmallUiChange(QWidget* widget)` | 实现 animate small ui change 对应的业务或工具逻辑。 |
| [L137](../src/apps/controller/ControllerMainWindowSupport.h#L137) | `DrawNavigationIcon` | 声明 | `QPixmap DrawNavigationIcon(NavigationIcon icon, const QColor& color)` | 准备或呈现 draw navigation icon 相关逻辑。 |
| [L138](../src/apps/controller/ControllerMainWindowSupport.h#L138) | `MakeNavigationIcon` | 声明 | `QIcon MakeNavigationIcon(NavigationIcon icon, bool dark)` | 创建或初始化 make navigation icon 相关逻辑。 |
| [L139](../src/apps/controller/ControllerMainWindowSupport.h#L139) | `MakePageSurface` | 声明 | `QScrollArea* MakePageSurface( const QString& title, const QString& subtitle, QWidget* parent, QVBoxLayout** pageLayout)` | 创建或初始化 make page surface 相关逻辑。 |
| [L144](../src/apps/controller/ControllerMainWindowSupport.h#L144) | `MakeNavigationButton` | 声明 | `QPushButton* MakeNavigationButton( const QString& text, NavigationIcon icon, bool active, QWidget* parent)` | 创建或初始化 make navigation button 相关逻辑。 |
| [L149](../src/apps/controller/ControllerMainWindowSupport.h#L149) | `MakeDivider` | 声明 | `QFrame* MakeDivider(QWidget* parent)` | 创建或初始化 make divider 相关逻辑。 |
| [L150](../src/apps/controller/ControllerMainWindowSupport.h#L150) | `AddRoomCapacityItems` | 声明 | `void AddRoomCapacityItems(QComboBox* comboBox)` | 实现 add room capacity items 对应的业务或工具逻辑。 |
| [L151](../src/apps/controller/ControllerMainWindowSupport.h#L151) | `MemberDisplayName` | 声明 | `QString MemberDisplayName( const RoomSnapshot& room, const std::string& deviceId, const std::string& localDeviceId)` | 实现 member display name 对应的业务或工具逻辑。 |
| [L155](../src/apps/controller/ControllerMainWindowSupport.h#L155) | `ConnectivityDebugName` | 声明 | `QString ConnectivityDebugName(SessionConnectivityState state)` | 建立连接 connectivity debug name 相关逻辑。 |
| [L156](../src/apps/controller/ControllerMainWindowSupport.h#L156) | `RoomMembershipDebugName` | 声明 | `QString RoomMembershipDebugName(RoomMembershipState state)` | 实现 room membership debug name 对应的业务或工具逻辑。 |
| [L157](../src/apps/controller/ControllerMainWindowSupport.h#L157) | `PeerConnectionDebugName` | 声明 | `QString PeerConnectionDebugName(RoomPeerConnectionState state)` | 实现 peer connection debug name 对应的业务或工具逻辑。 |
| [L158](../src/apps/controller/ControllerMainWindowSupport.h#L158) | `MediaDeviceSelectionDebugName` | 声明 | `QString MediaDeviceSelectionDebugName(MediaDeviceSelectionState state)` | 实现 media device selection debug name 对应的业务或工具逻辑。 |
| [L159](../src/apps/controller/ControllerMainWindowSupport.h#L159) | `MediaDeviceDebugName` | 声明 | `QString MediaDeviceDebugName( const MediaDeviceCategorySnapshot& category, const std::string& deviceId)` | 实现 media device debug name 对应的业务或工具逻辑。 |
| [L162](../src/apps/controller/ControllerMainWindowSupport.h#L162) | `MediaDeviceCategoryDebugText` | 声明 | `QString MediaDeviceCategoryDebugText( const MediaDeviceCategorySnapshot& category)` | 实现 media device category debug text 对应的业务或工具逻辑。 |

## `src/apps/controller/ControllerProductMain.cpp`

[打开源码](../src/apps/controller/ControllerProductMain.cpp) · **文件作用：** 实现 controller product main 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L10](../src/apps/controller/ControllerProductMain.cpp#L10) | `main` | 定义 | `int main(int argc, char* argv[])` | 实现 main 对应的业务或工具逻辑。 |

## `src/apps/controller/CurrentPageStack.cpp`

[打开源码](../src/apps/controller/CurrentPageStack.cpp) · **文件作用：** 实现 current page stack 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L15](../src/apps/controller/CurrentPageStack.cpp#L15) | `CurrentPageStack::PageHeightForWidth` | 定义 | `int CurrentPageStack::PageHeightForWidth(QWidget* page) const` | 实现 page height for width 对应的业务或工具逻辑。 |
| [L35](../src/apps/controller/CurrentPageStack.cpp#L35) | `CurrentPageStack::RefreshCurrentHeight` | 定义 | `void CurrentPageStack::RefreshCurrentHeight()` | 刷新 refresh current height 相关逻辑。 |
| [L48](../src/apps/controller/CurrentPageStack.cpp#L48) | `CurrentPageStack::sizeHint` | 定义 | `QSize CurrentPageStack::sizeHint() const` | 实现 size hint 对应的业务或工具逻辑。 |
| [L54](../src/apps/controller/CurrentPageStack.cpp#L54) | `CurrentPageStack::minimumSizeHint` | 定义 | `QSize CurrentPageStack::minimumSizeHint() const` | 实现 minimum size hint 对应的业务或工具逻辑。 |
| [L60](../src/apps/controller/CurrentPageStack.cpp#L60) | `CurrentPageStack::resizeEvent` | 定义 | `void CurrentPageStack::resizeEvent(QResizeEvent* event)` | 实现 resize event 对应的业务或工具逻辑。 |

## `src/apps/controller/CurrentPageStack.h`

[打开源码](../src/apps/controller/CurrentPageStack.h) · **文件作用：** 声明 current page stack 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L10](../src/apps/controller/CurrentPageStack.h#L10) | `CurrentPageStack` | class | 定义 CurrentPageStack 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L14](../src/apps/controller/CurrentPageStack.h#L14) | `PageHeightForWidth` | 声明 | `int PageHeightForWidth(QWidget* page) const` | 实现 page height for width 对应的业务或工具逻辑。 |
| [L15](../src/apps/controller/CurrentPageStack.h#L15) | `RefreshCurrentHeight` | 声明 | `void RefreshCurrentHeight()` | 刷新 refresh current height 相关逻辑。 |
| [L16](../src/apps/controller/CurrentPageStack.h#L16) | `sizeHint` | 声明 | `QSize sizeHint() const override` | 实现 size hint 对应的业务或工具逻辑。 |
| [L17](../src/apps/controller/CurrentPageStack.h#L17) | `minimumSizeHint` | 声明 | `QSize minimumSizeHint() const override` | 实现 minimum size hint 对应的业务或工具逻辑。 |
| [L20](../src/apps/controller/CurrentPageStack.h#L20) | `resizeEvent` | 声明 | `void resizeEvent(QResizeEvent* event) override` | 实现 resize event 对应的业务或工具逻辑。 |

## `src/apps/controller/D3D11VideoSurface.cpp`

[打开源码](../src/apps/controller/D3D11VideoSurface.cpp) · **文件作用：** 实现 d3 d11 video surface 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L265](../src/apps/controller/D3D11VideoSurface.cpp#L265) | `VertexOutput` | struct | 定义 VertexOutput 的 struct 类型和相关状态。 |
| [L479](../src/apps/controller/D3D11VideoSurface.cpp#L479) | `VertexOutput` | struct | 定义 VertexOutput 的 struct 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L22](../src/apps/controller/D3D11VideoSurface.cpp#L22) | `D3D11VideoSurface::SetCursorFrame` | 定义 | `void D3D11VideoSurface::SetCursorFrame( const RemoteCursorRenderState::Snapshot& snapshot, int sourceWidth, int sourceHeight)` | 更新或应用 set cursor frame 相关逻辑。 |
| [L35](../src/apps/controller/D3D11VideoSurface.cpp#L35) | `D3D11VideoSurface::D3D11VideoSurface` | 定义 | `D3D11VideoSurface::D3D11VideoSurface(QWidget* parent) : QWidget(parent)` | 构造并初始化 D3D11VideoSurface 实例。 |
| [L49](../src/apps/controller/D3D11VideoSurface.cpp#L49) | `D3D11VideoSurface::nativeEvent` | 定义 | `bool D3D11VideoSurface::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)` | 实现 native event 对应的业务或工具逻辑。 |
| [L74](../src/apps/controller/D3D11VideoSurface.cpp#L74) | `D3D11VideoSurface::Present` | 定义 | `D3D11VideoSurface::PresentTiming D3D11VideoSurface::Present(D3D11NativeFrameBuffer* frame)` | 准备或呈现 present 相关逻辑。 |
| [L86](../src/apps/controller/D3D11VideoSurface.cpp#L86) | `D3D11VideoSurface::PresentCpuBgra` | 定义 | `D3D11VideoSurface::PresentTiming D3D11VideoSurface::PresentCpuBgra(const QImage& image)` | 准备或呈现 present cpu bgra 相关逻辑。 |
| [L108](../src/apps/controller/D3D11VideoSurface.cpp#L108) | `D3D11VideoSurface::PresentCpuNv12` | 定义 | `D3D11VideoSurface::PresentTiming D3D11VideoSurface::PresentCpuNv12(const QImage& image)` | 准备或呈现 present cpu nv12 相关逻辑。 |
| [L142](../src/apps/controller/D3D11VideoSurface.cpp#L142) | `D3D11VideoSurface::PresentCpuI420` | 定义 | `D3D11VideoSurface::PresentTiming D3D11VideoSurface::PresentCpuI420( webrtc::I420BufferInterface* frame)` | 准备或呈现 present cpu i420 相关逻辑。 |
| [L245](../src/apps/controller/D3D11VideoSurface.cpp#L245) | `D3D11VideoSurface::paintEngine` | 定义 | `QPaintEngine* D3D11VideoSurface::paintEngine() const { return nullptr; }` | 准备或呈现 paint engine 相关逻辑。 |
| [L248](../src/apps/controller/D3D11VideoSurface.cpp#L248) | `D3D11VideoSurface::paintEvent` | 定义 | `void D3D11VideoSurface::paintEvent(QPaintEvent*) {}` | 准备或呈现 paint event 相关逻辑。 |
| [L251](../src/apps/controller/D3D11VideoSurface.cpp#L251) | `D3D11VideoSurface::EnsureCursorPipeline` | 定义 | `bool D3D11VideoSurface::EnsureCursorPipeline()` | 实现 ensure cursor pipeline 对应的业务或工具逻辑。 |
| [L349](../src/apps/controller/D3D11VideoSurface.cpp#L349) | `D3D11VideoSurface::UpdateCursorTexture` | 定义 | `bool D3D11VideoSurface::UpdateCursorTexture()` | 更新或应用 update cursor texture 相关逻辑。 |
| [L394](../src/apps/controller/D3D11VideoSurface.cpp#L394) | `D3D11VideoSurface::DrawCursorOverlay` | 定义 | `void D3D11VideoSurface::DrawCursorOverlay(ID3D11RenderTargetView* renderTarget, UINT outputWidth, UINT outputHeight)` | 准备或呈现 draw cursor overlay 相关逻辑。 |
| [L452](../src/apps/controller/D3D11VideoSurface.cpp#L452) | `D3D11VideoSurface::ResetCursorResources` | 定义 | `void D3D11VideoSurface::ResetCursorResources()` | 重置或移除 reset cursor resources 相关逻辑。 |
| [L467](../src/apps/controller/D3D11VideoSurface.cpp#L467) | `D3D11VideoSurface::EnsureI420ShaderPipeline` | 定义 | `bool D3D11VideoSurface::EnsureI420ShaderPipeline()` | 实现 ensure i420 shader pipeline 对应的业务或工具逻辑。 |
| [L553](../src/apps/controller/D3D11VideoSurface.cpp#L553) | `D3D11VideoSurface::CreateI420PlaneTexture` | 定义 | `bool D3D11VideoSurface::CreateI420PlaneTexture( UINT width, UINT height, I420PlaneTexture* plane)` | 创建或初始化 create i420 plane texture 相关逻辑。 |
| [L575](../src/apps/controller/D3D11VideoSurface.cpp#L575) | `D3D11VideoSurface::EnsureI420UploadTextures` | 定义 | `bool D3D11VideoSurface::EnsureI420UploadTextures(UINT width, UINT height)` | 实现 ensure i420 upload textures 对应的业务或工具逻辑。 |
| [L599](../src/apps/controller/D3D11VideoSurface.cpp#L599) | `D3D11VideoSurface::UploadI420Plane` | 定义 | `bool D3D11VideoSurface::UploadI420Plane(I420PlaneTexture& plane, const std::uint8_t* source, int sourceStride, UINT width, UINT height)` | 准备或呈现 upload i420 plane 相关逻辑。 |
| [L622](../src/apps/controller/D3D11VideoSurface.cpp#L622) | `D3D11VideoSurface::EnsureI420BackBufferView` | 定义 | `bool D3D11VideoSurface::EnsureI420BackBufferView()` | 实现 ensure i420 back buffer view 对应的业务或工具逻辑。 |
| [L634](../src/apps/controller/D3D11VideoSurface.cpp#L634) | `D3D11VideoSurface::ResetI420UploadTextures` | 定义 | `void D3D11VideoSurface::ResetI420UploadTextures()` | 重置或移除 reset i420 upload textures 相关逻辑。 |
| [L646](../src/apps/controller/D3D11VideoSurface.cpp#L646) | `D3D11VideoSurface::ResetI420ShaderResources` | 定义 | `void D3D11VideoSurface::ResetI420ShaderResources()` | 重置或移除 reset i420 shader resources 相关逻辑。 |
| [L656](../src/apps/controller/D3D11VideoSurface.cpp#L656) | `D3D11VideoSurface::PresentTexture` | 定义 | `D3D11VideoSurface::PresentTiming D3D11VideoSurface::PresentTexture( ID3D11Device* device, ID3D11Texture2D* texture, UINT subresourceIndex, int sourceWidth, int sourceHeight, DXGI_FORMAT expectedFormat)` | 准备或呈现 present texture 相关逻辑。 |
| [L781](../src/apps/controller/D3D11VideoSurface.cpp#L781) | `D3D11VideoSurface::EnsureCpuDevice` | 定义 | `bool D3D11VideoSurface::EnsureCpuDevice()` | 实现 ensure cpu device 对应的业务或工具逻辑。 |
| [L814](../src/apps/controller/D3D11VideoSurface.cpp#L814) | `D3D11VideoSurface::EnsureCpuUploadTexture` | 定义 | `bool D3D11VideoSurface::EnsureCpuUploadTexture( UINT width, UINT height, DXGI_FORMAT format)` | 实现 ensure cpu upload texture 对应的业务或工具逻辑。 |
| [L843](../src/apps/controller/D3D11VideoSurface.cpp#L843) | `D3D11VideoSurface::EnsureDevice` | 定义 | `bool D3D11VideoSurface::EnsureDevice(ID3D11Device* device)` | 实现 ensure device 对应的业务或工具逻辑。 |
| [L884](../src/apps/controller/D3D11VideoSurface.cpp#L884) | `D3D11VideoSurface::EnsureSwapChain` | 定义 | `bool D3D11VideoSurface::EnsureSwapChain(HWND window, UINT width, UINT height)` | 实现 ensure swap chain 对应的业务或工具逻辑。 |
| [L926](../src/apps/controller/D3D11VideoSurface.cpp#L926) | `D3D11VideoSurface::EnsureVideoProcessor` | 定义 | `bool D3D11VideoSurface::EnsureVideoProcessor(UINT sourceWidth, UINT sourceHeight, UINT outputWidth, UINT outputHeight)` | 实现 ensure video processor 对应的业务或工具逻辑。 |
| [L964](../src/apps/controller/D3D11VideoSurface.cpp#L964) | `D3D11VideoSurface::ResetDevice` | 定义 | `void D3D11VideoSurface::ResetDevice()` | 重置或移除 reset device 相关逻辑。 |

## `src/apps/controller/D3D11VideoSurface.h`

[打开源码](../src/apps/controller/D3D11VideoSurface.h) · **文件作用：** 声明 d3 d11 video surface 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L24](../src/apps/controller/D3D11VideoSurface.h#L24) | `QPaintEngine` | class | 定义 QPaintEngine 的 class 类型和相关状态。 |
| [L25](../src/apps/controller/D3D11VideoSurface.h#L25) | `QPaintEvent` | class | 定义 QPaintEvent 的 class 类型和相关状态。 |
| [L31](../src/apps/controller/D3D11VideoSurface.h#L31) | `D3D11VideoSurface` | class | Presents decoder-owned or CPU-uploaded frames through a swap chain created on the matching D3D11 device. |
| [L33](../src/apps/controller/D3D11VideoSurface.h#L33) | `PresentTiming` | struct | 定义 PresentTiming 的 struct 类型和相关状态。 |
| [L59](../src/apps/controller/D3D11VideoSurface.h#L59) | `I420PlaneTexture` | struct | 定义 I420PlaneTexture 的 struct 类型和相关状态。 |
| [L64](../src/apps/controller/D3D11VideoSurface.h#L64) | `I420UploadSlot` | struct | 定义 I420UploadSlot 的 struct 类型和相关状态。 |
| [L70](../src/apps/controller/D3D11VideoSurface.h#L70) | `CursorConstants` | struct | 定义 CursorConstants 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L24](../src/apps/controller/D3D11VideoSurface.h#L24) | `QPaintEngine` | `class QPaintEngine;` | 保存 q paint engine 相关配置或运行状态。 |
| [L25](../src/apps/controller/D3D11VideoSurface.h#L25) | `QPaintEvent` | `class QPaintEvent;` | 保存 q paint event 相关配置或运行状态。 |
| [L34](../src/apps/controller/D3D11VideoSurface.h#L34) | `succeeded` | `bool succeeded = false;` | 保存 succeeded 相关配置或运行状态。 |
| [L35](../src/apps/controller/D3D11VideoSurface.h#L35) | `cpuPreparationUs` | `std::uint64_t cpuPreparationUs = 0;` | 保存 cpu preparation us 相关配置或运行状态。 |
| [L36](../src/apps/controller/D3D11VideoSurface.h#L36) | `videoProcessorSubmitUs` | `std::uint64_t videoProcessorSubmitUs = 0;` | 保存 video processor submit us 相关配置或运行状态。 |
| [L37](../src/apps/controller/D3D11VideoSurface.h#L37) | `presentCallUs` | `std::uint64_t presentCallUs = 0;` | 保存 present call us 相关配置或运行状态。 |
| [L60](../src/apps/controller/D3D11VideoSurface.h#L60) | `texture` | `Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;` | 保存媒体帧、图像或缓冲资源：texture。 |
| [L61](../src/apps/controller/D3D11VideoSurface.h#L61) | `view` | `Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> view;` | 保存 view 相关配置或运行状态。 |
| [L65](../src/apps/controller/D3D11VideoSurface.h#L65) | `y` | `I420PlaneTexture y;` | 保存 y 相关配置或运行状态。 |
| [L66](../src/apps/controller/D3D11VideoSurface.h#L66) | `u` | `I420PlaneTexture u;` | 保存 u 相关配置或运行状态。 |
| [L67](../src/apps/controller/D3D11VideoSurface.h#L67) | `v` | `I420PlaneTexture v;` | 保存 v 相关配置或运行状态。 |
| [L71](../src/apps/controller/D3D11VideoSurface.h#L71) | `left` | `float left;` | 保存 left 相关配置或运行状态。 |
| [L72](../src/apps/controller/D3D11VideoSurface.h#L72) | `top` | `float top;` | 保存 top 相关配置或运行状态。 |
| [L73](../src/apps/controller/D3D11VideoSurface.h#L73) | `right` | `float right;` | 保存 right 相关配置或运行状态。 |
| [L74](../src/apps/controller/D3D11VideoSurface.h#L74) | `bottom` | `float bottom;` | 保存 bottom 相关配置或运行状态。 |
| [L114](../src/apps/controller/D3D11VideoSurface.h#L114) | `device_` | `Microsoft::WRL::ComPtr<ID3D11Device> device_;` | 保存 device 相关配置或运行状态。 |
| [L115](../src/apps/controller/D3D11VideoSurface.h#L115) | `context_` | `Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;` | 保存 context 相关配置或运行状态。 |
| [L116](../src/apps/controller/D3D11VideoSurface.h#L116) | `videoDevice_` | `Microsoft::WRL::ComPtr<ID3D11VideoDevice> videoDevice_;` | 保存 video device 相关配置或运行状态。 |
| [L117](../src/apps/controller/D3D11VideoSurface.h#L117) | `videoContext_` | `Microsoft::WRL::ComPtr<ID3D11VideoContext> videoContext_;` | 保存 video context 相关配置或运行状态。 |
| [L118](../src/apps/controller/D3D11VideoSurface.h#L118) | `factory_` | `Microsoft::WRL::ComPtr<IDXGIFactory2> factory_;` | 保存 factory 相关配置或运行状态。 |
| [L119](../src/apps/controller/D3D11VideoSurface.h#L119) | `swapChain_` | `Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain_;` | 保存 swap chain 相关配置或运行状态。 |
| [L121](../src/apps/controller/D3D11VideoSurface.h#L121) | `processorEnumerator_` | `processorEnumerator_;` | 保存 processor enumerator 相关配置或运行状态。 |
| [L122](../src/apps/controller/D3D11VideoSurface.h#L122) | `processor_` | `Microsoft::WRL::ComPtr<ID3D11VideoProcessor> processor_;` | 保存 processor 相关配置或运行状态。 |
| [L123](../src/apps/controller/D3D11VideoSurface.h#L123) | `cpuPresentationDevice_` | `Microsoft::WRL::ComPtr<ID3D11Device> cpuPresentationDevice_;` | 保存 cpu presentation device 相关配置或运行状态。 |
| [L124](../src/apps/controller/D3D11VideoSurface.h#L124) | `cpuUploadTexture_` | `Microsoft::WRL::ComPtr<ID3D11Texture2D> cpuUploadTexture_;` | 保存媒体帧、图像或缓冲资源：cpu upload texture。 |
| [L125](../src/apps/controller/D3D11VideoSurface.h#L125) | `i420UploadSlots_` | `std::array<I420UploadSlot, 3> i420UploadSlots_;` | 保存 i420 upload slots 相关配置或运行状态。 |
| [L126](../src/apps/controller/D3D11VideoSurface.h#L126) | `i420VertexShader_` | `Microsoft::WRL::ComPtr<ID3D11VertexShader> i420VertexShader_;` | 保存 i420 vertex shader 相关配置或运行状态。 |
| [L127](../src/apps/controller/D3D11VideoSurface.h#L127) | `i420PixelShader_` | `Microsoft::WRL::ComPtr<ID3D11PixelShader> i420PixelShader_;` | 保存 i420 pixel shader 相关配置或运行状态。 |
| [L128](../src/apps/controller/D3D11VideoSurface.h#L128) | `i420SamplerState_` | `Microsoft::WRL::ComPtr<ID3D11SamplerState> i420SamplerState_;` | 保存状态机当前状态：i420 sampler state。 |
| [L129](../src/apps/controller/D3D11VideoSurface.h#L129) | `i420RasterizerState_` | `Microsoft::WRL::ComPtr<ID3D11RasterizerState> i420RasterizerState_;` | 保存状态机当前状态：i420 rasterizer state。 |
| [L130](../src/apps/controller/D3D11VideoSurface.h#L130) | `i420BackBufferView_` | `Microsoft::WRL::ComPtr<ID3D11RenderTargetView> i420BackBufferView_;` | 保存 i420 back buffer view 相关配置或运行状态。 |
| [L131](../src/apps/controller/D3D11VideoSurface.h#L131) | `cursorVertexShader_` | `Microsoft::WRL::ComPtr<ID3D11VertexShader> cursorVertexShader_;` | 保存 cursor vertex shader 相关配置或运行状态。 |
| [L132](../src/apps/controller/D3D11VideoSurface.h#L132) | `cursorPixelShader_` | `Microsoft::WRL::ComPtr<ID3D11PixelShader> cursorPixelShader_;` | 保存 cursor pixel shader 相关配置或运行状态。 |
| [L133](../src/apps/controller/D3D11VideoSurface.h#L133) | `cursorSamplerState_` | `Microsoft::WRL::ComPtr<ID3D11SamplerState> cursorSamplerState_;` | 保存状态机当前状态：cursor sampler state。 |
| [L134](../src/apps/controller/D3D11VideoSurface.h#L134) | `cursorRasterizerState_` | `Microsoft::WRL::ComPtr<ID3D11RasterizerState> cursorRasterizerState_;` | 保存状态机当前状态：cursor rasterizer state。 |
| [L135](../src/apps/controller/D3D11VideoSurface.h#L135) | `cursorBlendState_` | `Microsoft::WRL::ComPtr<ID3D11BlendState> cursorBlendState_;` | 保存状态机当前状态：cursor blend state。 |
| [L136](../src/apps/controller/D3D11VideoSurface.h#L136) | `cursorConstantBuffer_` | `Microsoft::WRL::ComPtr<ID3D11Buffer> cursorConstantBuffer_;` | 保存媒体帧、图像或缓冲资源：cursor constant buffer。 |
| [L137](../src/apps/controller/D3D11VideoSurface.h#L137) | `cursorTexture_` | `Microsoft::WRL::ComPtr<ID3D11Texture2D> cursorTexture_;` | 保存媒体帧、图像或缓冲资源：cursor texture。 |
| [L138](../src/apps/controller/D3D11VideoSurface.h#L138) | `cursorTextureView_` | `Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cursorTextureView_;` | 保存 cursor texture view 相关配置或运行状态。 |
| [L139](../src/apps/controller/D3D11VideoSurface.h#L139) | `window_` | `HWND window_ = nullptr;` | 保存 window 相关配置或运行状态。 |
| [L140](../src/apps/controller/D3D11VideoSurface.h#L140) | `swapWidth_` | `UINT swapWidth_ = 0;` | 保存计数、尺寸或速率指标：swap width。 |
| [L141](../src/apps/controller/D3D11VideoSurface.h#L141) | `swapHeight_` | `UINT swapHeight_ = 0;` | 保存计数、尺寸或速率指标：swap height。 |
| [L142](../src/apps/controller/D3D11VideoSurface.h#L142) | `sourceWidth_` | `UINT sourceWidth_ = 0;` | 保存计数、尺寸或速率指标：source width。 |
| [L143](../src/apps/controller/D3D11VideoSurface.h#L143) | `sourceHeight_` | `UINT sourceHeight_ = 0;` | 保存计数、尺寸或速率指标：source height。 |
| [L144](../src/apps/controller/D3D11VideoSurface.h#L144) | `processorWidth_` | `UINT processorWidth_ = 0;` | 保存计数、尺寸或速率指标：processor width。 |
| [L145](../src/apps/controller/D3D11VideoSurface.h#L145) | `processorHeight_` | `UINT processorHeight_ = 0;` | 保存计数、尺寸或速率指标：processor height。 |
| [L146](../src/apps/controller/D3D11VideoSurface.h#L146) | `frameIndex_` | `UINT frameIndex_ = 0;` | 保存 frame index 相关配置或运行状态。 |
| [L147](../src/apps/controller/D3D11VideoSurface.h#L147) | `cpuUploadWidth_` | `UINT cpuUploadWidth_ = 0;` | 保存计数、尺寸或速率指标：cpu upload width。 |
| [L148](../src/apps/controller/D3D11VideoSurface.h#L148) | `cpuUploadHeight_` | `UINT cpuUploadHeight_ = 0;` | 保存计数、尺寸或速率指标：cpu upload height。 |
| [L149](../src/apps/controller/D3D11VideoSurface.h#L149) | `cpuUploadFormat_` | `DXGI_FORMAT cpuUploadFormat_ = DXGI_FORMAT_UNKNOWN;` | 保存 cpu upload format 相关配置或运行状态。 |
| [L150](../src/apps/controller/D3D11VideoSurface.h#L150) | `i420UploadWidth_` | `UINT i420UploadWidth_ = 0;` | 保存计数、尺寸或速率指标：i420 upload width。 |
| [L151](../src/apps/controller/D3D11VideoSurface.h#L151) | `i420UploadHeight_` | `UINT i420UploadHeight_ = 0;` | 保存计数、尺寸或速率指标：i420 upload height。 |
| [L152](../src/apps/controller/D3D11VideoSurface.h#L152) | `i420UploadSlotIndex_` | `std::size_t i420UploadSlotIndex_ = 0;` | 保存 i420 upload slot index 相关配置或运行状态。 |
| [L153](../src/apps/controller/D3D11VideoSurface.h#L153) | `cursorImage_` | `QImage cursorImage_;` | 保存媒体帧、图像或缓冲资源：cursor image。 |
| [L154](../src/apps/controller/D3D11VideoSurface.h#L154) | `cursorHotspot_` | `QPoint cursorHotspot_;` | 保存 cursor hotspot 相关配置或运行状态。 |
| [L155](../src/apps/controller/D3D11VideoSurface.h#L155) | `cursorPosition_` | `QPoint cursorPosition_;` | 保存 cursor position 相关配置或运行状态。 |
| [L156](../src/apps/controller/D3D11VideoSurface.h#L156) | `cursorShapeRevision_` | `std::uint64_t cursorShapeRevision_ = 0;` | 标记当前世代，用于拒绝过期异步结果：cursor shape revision。 |
| [L157](../src/apps/controller/D3D11VideoSurface.h#L157) | `uploadedCursorShapeRevision_` | `std::uint64_t uploadedCursorShapeRevision_ = 0;` | 标记当前世代，用于拒绝过期异步结果：uploaded cursor shape revision。 |
| [L158](../src/apps/controller/D3D11VideoSurface.h#L158) | `cursorTextureWidth_` | `UINT cursorTextureWidth_ = 0;` | 保存计数、尺寸或速率指标：cursor texture width。 |
| [L159](../src/apps/controller/D3D11VideoSurface.h#L159) | `cursorTextureHeight_` | `UINT cursorTextureHeight_ = 0;` | 保存计数、尺寸或速率指标：cursor texture height。 |
| [L160](../src/apps/controller/D3D11VideoSurface.h#L160) | `cursorSourceWidth_` | `int cursorSourceWidth_ = 0;` | 保存计数、尺寸或速率指标：cursor source width。 |
| [L161](../src/apps/controller/D3D11VideoSurface.h#L161) | `cursorSourceHeight_` | `int cursorSourceHeight_ = 0;` | 保存计数、尺寸或速率指标：cursor source height。 |
| [L162](../src/apps/controller/D3D11VideoSurface.h#L162) | `cursorVisible_` | `bool cursorVisible_ = false;` | 保存 cursor visible 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L40](../src/apps/controller/D3D11VideoSurface.h#L40) | `D3D11VideoSurface` | 声明 | `explicit D3D11VideoSurface(QWidget* parent = nullptr)` | 实现 d3 d11 video surface 对应的业务或工具逻辑。 |
| [L42](../src/apps/controller/D3D11VideoSurface.h#L42) | `SetCursorFrame` | 声明 | `void SetCursorFrame( const RemoteCursorRenderState::Snapshot& snapshot, int sourceWidth, int sourceHeight)` | 更新或应用 set cursor frame 相关逻辑。 |
| [L46](../src/apps/controller/D3D11VideoSurface.h#L46) | `Present` | 声明 | `PresentTiming Present(D3D11NativeFrameBuffer* frame)` | 准备或呈现 present 相关逻辑。 |
| [L47](../src/apps/controller/D3D11VideoSurface.h#L47) | `PresentCpuBgra` | 声明 | `PresentTiming PresentCpuBgra(const QImage& image)` | 准备或呈现 present cpu bgra 相关逻辑。 |
| [L48](../src/apps/controller/D3D11VideoSurface.h#L48) | `PresentCpuNv12` | 声明 | `PresentTiming PresentCpuNv12(const QImage& image)` | 准备或呈现 present cpu nv12 相关逻辑。 |
| [L49](../src/apps/controller/D3D11VideoSurface.h#L49) | `PresentCpuI420` | 声明 | `PresentTiming PresentCpuI420(webrtc::I420BufferInterface* frame)` | 准备或呈现 present cpu i420 相关逻辑。 |
| [L52](../src/apps/controller/D3D11VideoSurface.h#L52) | `nativeEvent` | 声明 | `bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override` | 实现 native event 对应的业务或工具逻辑。 |
| [L55](../src/apps/controller/D3D11VideoSurface.h#L55) | `paintEngine` | 声明 | `QPaintEngine* paintEngine() const override` | 准备或呈现 paint engine 相关逻辑。 |
| [L56](../src/apps/controller/D3D11VideoSurface.h#L56) | `paintEvent` | 声明 | `void paintEvent(QPaintEvent* event) override` | 准备或呈现 paint event 相关逻辑。 |
| [L77](../src/apps/controller/D3D11VideoSurface.h#L77) | `EnsureCursorPipeline` | 声明 | `bool EnsureCursorPipeline()` | 实现 ensure cursor pipeline 对应的业务或工具逻辑。 |
| [L78](../src/apps/controller/D3D11VideoSurface.h#L78) | `UpdateCursorTexture` | 声明 | `bool UpdateCursorTexture()` | 更新或应用 update cursor texture 相关逻辑。 |
| [L79](../src/apps/controller/D3D11VideoSurface.h#L79) | `DrawCursorOverlay` | 声明 | `void DrawCursorOverlay(ID3D11RenderTargetView* renderTarget, UINT outputWidth, UINT outputHeight)` | 准备或呈现 draw cursor overlay 相关逻辑。 |
| [L82](../src/apps/controller/D3D11VideoSurface.h#L82) | `ResetCursorResources` | 声明 | `void ResetCursorResources()` | 重置或移除 reset cursor resources 相关逻辑。 |
| [L83](../src/apps/controller/D3D11VideoSurface.h#L83) | `EnsureI420ShaderPipeline` | 声明 | `bool EnsureI420ShaderPipeline()` | 实现 ensure i420 shader pipeline 对应的业务或工具逻辑。 |
| [L84](../src/apps/controller/D3D11VideoSurface.h#L84) | `CreateI420PlaneTexture` | 声明 | `bool CreateI420PlaneTexture(UINT width, UINT height, I420PlaneTexture* plane)` | 创建或初始化 create i420 plane texture 相关逻辑。 |
| [L87](../src/apps/controller/D3D11VideoSurface.h#L87) | `EnsureI420UploadTextures` | 声明 | `bool EnsureI420UploadTextures(UINT width, UINT height)` | 实现 ensure i420 upload textures 对应的业务或工具逻辑。 |
| [L88](../src/apps/controller/D3D11VideoSurface.h#L88) | `UploadI420Plane` | 声明 | `bool UploadI420Plane(I420PlaneTexture& plane, const std::uint8_t* source, int sourceStride, UINT width, UINT height)` | 准备或呈现 upload i420 plane 相关逻辑。 |
| [L93](../src/apps/controller/D3D11VideoSurface.h#L93) | `EnsureI420BackBufferView` | 声明 | `bool EnsureI420BackBufferView()` | 实现 ensure i420 back buffer view 对应的业务或工具逻辑。 |
| [L94](../src/apps/controller/D3D11VideoSurface.h#L94) | `ResetI420UploadTextures` | 声明 | `void ResetI420UploadTextures()` | 重置或移除 reset i420 upload textures 相关逻辑。 |
| [L95](../src/apps/controller/D3D11VideoSurface.h#L95) | `ResetI420ShaderResources` | 声明 | `void ResetI420ShaderResources()` | 重置或移除 reset i420 shader resources 相关逻辑。 |
| [L96](../src/apps/controller/D3D11VideoSurface.h#L96) | `PresentTexture` | 声明 | `PresentTiming PresentTexture(ID3D11Device* device, ID3D11Texture2D* texture, UINT subresourceIndex, int sourceWidth, int sourceHeight, DXGI_FORMAT expectedFormat)` | 准备或呈现 present texture 相关逻辑。 |
| [L102](../src/apps/controller/D3D11VideoSurface.h#L102) | `EnsureCpuDevice` | 声明 | `bool EnsureCpuDevice()` | 实现 ensure cpu device 对应的业务或工具逻辑。 |
| [L103](../src/apps/controller/D3D11VideoSurface.h#L103) | `EnsureCpuUploadTexture` | 声明 | `bool EnsureCpuUploadTexture(UINT width, UINT height, DXGI_FORMAT format)` | 实现 ensure cpu upload texture 对应的业务或工具逻辑。 |
| [L106](../src/apps/controller/D3D11VideoSurface.h#L106) | `EnsureDevice` | 声明 | `bool EnsureDevice(ID3D11Device* device)` | 实现 ensure device 对应的业务或工具逻辑。 |
| [L107](../src/apps/controller/D3D11VideoSurface.h#L107) | `EnsureSwapChain` | 声明 | `bool EnsureSwapChain(HWND window, UINT width, UINT height)` | 实现 ensure swap chain 对应的业务或工具逻辑。 |
| [L108](../src/apps/controller/D3D11VideoSurface.h#L108) | `EnsureVideoProcessor` | 声明 | `bool EnsureVideoProcessor(UINT sourceWidth, UINT sourceHeight, UINT outputWidth, UINT outputHeight)` | 实现 ensure video processor 对应的业务或工具逻辑。 |
| [L112](../src/apps/controller/D3D11VideoSurface.h#L112) | `ResetDevice` | 声明 | `void ResetDevice()` | 重置或移除 reset device 相关逻辑。 |

## `src/apps/controller/FileTransferWindow.cpp`

[打开源码](../src/apps/controller/FileTransferWindow.cpp) · **文件作用：** 实现 file transfer window 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L451](../src/apps/controller/FileTransferWindow.cpp#L451) | `FileTransferCard` | class | 定义 FileTransferCard 的 class 类型和相关状态。 |
| [L453](../src/apps/controller/FileTransferWindow.cpp#L453) | `Actions` | struct | 定义 Actions 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L51](../src/apps/controller/FileTransferWindow.cpp#L51) | `kDefaultFileTransferDrawerWidth` | `constexpr int kDefaultFileTransferDrawerWidth = 390;` | 定义 default file transfer drawer width 的编译期常量或产品边界。 |
| [L52](../src/apps/controller/FileTransferWindow.cpp#L52) | `kMinimumFileTransferDrawerWidth` | `constexpr int kMinimumFileTransferDrawerWidth = 340;` | 定义 minimum file transfer drawer width 的编译期常量或产品边界。 |
| [L53](../src/apps/controller/FileTransferWindow.cpp#L53) | `kDrawerResizeHandleWidth` | `constexpr int kDrawerResizeHandleWidth = 8;` | 定义 drawer resize handle width 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L55](../src/apps/controller/FileTransferWindow.cpp#L55) | `DrawerRevealEasing` | 定义 | `const QEasingCurve& DrawerRevealEasing()` | 准备或呈现 drawer reveal easing 相关逻辑。 |
| [L66](../src/apps/controller/FileTransferWindow.cpp#L66) | `DrawerDismissEasing` | 定义 | `const QEasingCurve& DrawerDismissEasing()` | 准备或呈现 drawer dismiss easing 相关逻辑。 |
| [L77](../src/apps/controller/FileTransferWindow.cpp#L77) | `InitialFileSaveDirectory` | 定义 | `QString InitialFileSaveDirectory()` | 创建或初始化 initial file save directory 相关逻辑。 |
| [L278](../src/apps/controller/FileTransferWindow.cpp#L278) | `FormatBytes` | 定义 | `QString FormatBytes(std::uint64_t bytes)` | 实现 format bytes 对应的业务或工具逻辑。 |
| [L295](../src/apps/controller/FileTransferWindow.cpp#L295) | `FormatRemainingTime` | 定义 | `QString FormatRemainingTime(std::uint64_t seconds)` | 实现 format remaining time 对应的业务或工具逻辑。 |
| [L310](../src/apps/controller/FileTransferWindow.cpp#L310) | `StateText` | 定义 | `QString StateText(app::FileTransferState state)` | 实现 state text 对应的业务或工具逻辑。 |
| [L343](../src/apps/controller/FileTransferWindow.cpp#L343) | `FileTransferErrorText` | 定义 | `QString FileTransferErrorText(const std::string& errorCode, const std::string& errorMessage)` | 实现 file transfer error text 对应的业务或工具逻辑。 |
| [L444](../src/apps/controller/FileTransferWindow.cpp#L444) | `FileTransferErrorText` | 定义 | `QString FileTransferErrorText(const app::FileTransferSnapshot& transfer)` | 实现 file transfer error text 对应的业务或工具逻辑。 |
| [L462](../src/apps/controller/FileTransferWindow.cpp#L462) | `FileTransferCard` | 定义 | `FileTransferCard(Actions actions, QWidget* parent) : QFrame(parent), actions_(std::move(actions))` | 实现 file transfer card 对应的业务或工具逻辑。 |
| [L518](../src/apps/controller/FileTransferWindow.cpp#L518) | `Update` | 定义 | `void Update(const app::FileTransferSnapshot& transfer, bool animate)` | 更新或应用 update 相关逻辑。 |
| [L617](../src/apps/controller/FileTransferWindow.cpp#L617) | `AddAction` | 定义 | `QPushButton* AddAction(const QString& text, const char* tone, std::function<void()> callback)` | 实现 add action 对应的业务或工具逻辑。 |
| [L706](../src/apps/controller/FileTransferWindow.cpp#L706) | `RebuildActions` | 定义 | `void RebuildActions(const app::FileTransferSnapshot& transfer)` | 更新或应用 rebuild actions 相关逻辑。 |
| [L742](../src/apps/controller/FileTransferWindow.cpp#L742) | `AnimateStateLabel` | 定义 | `void AnimateStateLabel()` | 实现 animate state label 对应的业务或工具逻辑。 |
| [L777](../src/apps/controller/FileTransferWindow.cpp#L777) | `FileTransferWindow::FileTransferWindow` | 定义 | `FileTransferWindow::FileTransferWindow( app::FileTransferController* controller, QWidget* parent) : FramelessMainWindow(parent, true), controller_(controller)` | 构造并初始化 FileTransferWindow 实例。 |
| [L788](../src/apps/controller/FileTransferWindow.cpp#L788) | `FileTransferWindow::~FileTransferWindow` | 定义 | `FileTransferWindow::~FileTransferWindow()` | 停止相关活动并释放 FileTransferWindow 实例拥有的资源。 |
| [L793](../src/apps/controller/FileTransferWindow.cpp#L793) | `FileTransferWindow::DetachController` | 定义 | `void FileTransferWindow::DetachController()` | 实现 detach controller 对应的业务或工具逻辑。 |
| [L802](../src/apps/controller/FileTransferWindow.cpp#L802) | `FileTransferWindow::SyncPeers` | 定义 | `void FileTransferWindow::SyncPeers( const std::vector<FileTransferPeer>& peers, const QRect& mainWindowGeometry)` | 实现 sync peers 对应的业务或工具逻辑。 |
| [L825](../src/apps/controller/FileTransferWindow.cpp#L825) | `FileTransferWindow::AttachAsDrawer` | 定义 | `void FileTransferWindow::AttachAsDrawer(QWidget* host)` | 实现 attach as drawer 对应的业务或工具逻辑。 |
| [L839](../src/apps/controller/FileTransferWindow.cpp#L839) | `FileTransferWindow::EffectiveDrawerWidth` | 定义 | `int FileTransferWindow::EffectiveDrawerWidth() const` | 实现 effective drawer width 对应的业务或工具逻辑。 |
| [L852](../src/apps/controller/FileTransferWindow.cpp#L852) | `FileTransferWindow::SetDrawerWidth` | 定义 | `void FileTransferWindow::SetDrawerWidth(int requestedWidth)` | 更新或应用 set drawer width 相关逻辑。 |
| [L867](../src/apps/controller/FileTransferWindow.cpp#L867) | `FileTransferWindow::OpenBesideMainWindow` | 定义 | `void FileTransferWindow::OpenBesideMainWindow( const QRect& mainWindowGeometry)` | 启动 open beside main window 相关逻辑。 |
| [L894](../src/apps/controller/FileTransferWindow.cpp#L894) | `FileTransferWindow::HideWithAnimation` | 定义 | `void FileTransferWindow::HideWithAnimation()` | 实现 hide with animation 对应的业务或工具逻辑。 |
| [L910](../src/apps/controller/FileTransferWindow.cpp#L910) | `FileTransferWindow::HideImmediately` | 定义 | `void FileTransferWindow::HideImmediately()` | 实现 hide immediately 对应的业务或工具逻辑。 |
| [L920](../src/apps/controller/FileTransferWindow.cpp#L920) | `FileTransferWindow::UpdateDrawerGeometry` | 定义 | `void FileTransferWindow::UpdateDrawerGeometry()` | 更新或应用 update drawer geometry 相关逻辑。 |
| [L939](../src/apps/controller/FileTransferWindow.cpp#L939) | `FileTransferWindow::IsHiding` | 定义 | `bool FileTransferWindow::IsHiding() const` | 判断 is hiding 相关逻辑。 |
| [L944](../src/apps/controller/FileTransferWindow.cpp#L944) | `FileTransferWindow::StartDrawerMotion` | 定义 | `void FileTransferWindow::StartDrawerMotion(const QPoint& targetPosition, int durationMs)` | 启动 start drawer motion 相关逻辑。 |
| [L985](../src/apps/controller/FileTransferWindow.cpp#L985) | `FileTransferWindow::StopDrawerAnimation` | 定义 | `void FileTransferWindow::StopDrawerAnimation()` | 停止 stop drawer animation 相关逻辑。 |
| [L991](../src/apps/controller/FileTransferWindow.cpp#L991) | `FileTransferWindow::closeEvent` | 定义 | `void FileTransferWindow::closeEvent(QCloseEvent* event)` | 关闭并清理 close event 相关逻辑。 |
| [L997](../src/apps/controller/FileTransferWindow.cpp#L997) | `FileTransferWindow::resizeEvent` | 定义 | `void FileTransferWindow::resizeEvent(QResizeEvent* event)` | 实现 resize event 对应的业务或工具逻辑。 |
| [L1007](../src/apps/controller/FileTransferWindow.cpp#L1007) | `FileTransferWindow::eventFilter` | 定义 | `bool FileTransferWindow::eventFilter(QObject* watched, QEvent* event)` | 实现 event filter 对应的业务或工具逻辑。 |
| [L1046](../src/apps/controller/FileTransferWindow.cpp#L1046) | `FileTransferWindow::OnFileTransfersChanged` | 定义 | `void FileTransferWindow::OnFileTransfersChanged( const std::vector<app::FileTransferSnapshot>& transfers)` | 接收并处理 on file transfers changed 相关逻辑。 |
| [L1070](../src/apps/controller/FileTransferWindow.cpp#L1070) | `FileTransferWindow::ScheduleTransferApply` | 定义 | `void FileTransferWindow::ScheduleTransferApply()` | 执行后台循环或调度 schedule transfer apply 相关逻辑。 |
| [L1087](../src/apps/controller/FileTransferWindow.cpp#L1087) | `FileTransferWindow::BuildUi` | 定义 | `void FileTransferWindow::BuildUi()` | 创建或初始化 build ui 相关逻辑。 |
| [L1198](../src/apps/controller/FileTransferWindow.cpp#L1198) | `FileTransferWindow::RefreshThemeStyle` | 定义 | `void FileTransferWindow::RefreshThemeStyle()` | 刷新 refresh theme style 相关逻辑。 |
| [L1213](../src/apps/controller/FileTransferWindow.cpp#L1213) | `FileTransferWindow::ApplyTransfers` | 定义 | `void FileTransferWindow::ApplyTransfers( std::vector<app::FileTransferSnapshot> transfers)` | 更新或应用 apply transfers 相关逻辑。 |
| [L1246](../src/apps/controller/FileTransferWindow.cpp#L1246) | `FileTransferWindow::UpdateTransferCardsIncrementally` | 定义 | `void FileTransferWindow::UpdateTransferCardsIncrementally()` | 更新或应用 update transfer cards incrementally 相关逻辑。 |
| [L1373](../src/apps/controller/FileTransferWindow.cpp#L1373) | `FileTransferWindow::RebuildTransferCards` | 定义 | `void FileTransferWindow::RebuildTransferCards()` | 更新或应用 rebuild transfer cards 相关逻辑。 |
| [L1587](../src/apps/controller/FileTransferWindow.cpp#L1587) | `FileTransferWindow::ChooseAndSendFile` | 定义 | `void FileTransferWindow::ChooseAndSendFile()` | 实现 choose and send file 对应的业务或工具逻辑。 |
| [L1607](../src/apps/controller/FileTransferWindow.cpp#L1607) | `FileTransferWindow::AcceptTransfer` | 定义 | `void FileTransferWindow::AcceptTransfer(const std::string& transferId)` | 处理并回复 accept transfer 相关逻辑。 |
| [L1623](../src/apps/controller/FileTransferWindow.cpp#L1623) | `FileTransferWindow::SaveTransferAs` | 定义 | `void FileTransferWindow::SaveTransferAs(const std::string& transferId)` | 保存或写入 save transfer as 相关逻辑。 |
| [L1637](../src/apps/controller/FileTransferWindow.cpp#L1637) | `FileTransferWindow::AcceptTransferToDirectory` | 定义 | `void FileTransferWindow::AcceptTransferToDirectory( const std::string& transferId, const QString& directory)` | 处理并回复 accept transfer to directory 相关逻辑。 |
| [L1677](../src/apps/controller/FileTransferWindow.cpp#L1677) | `FileTransferWindow::RejectTransfer` | 定义 | `void FileTransferWindow::RejectTransfer(const std::string& transferId)` | 处理并回复 reject transfer 相关逻辑。 |
| [L1690](../src/apps/controller/FileTransferWindow.cpp#L1690) | `FileTransferWindow::CancelTransfer` | 定义 | `void FileTransferWindow::CancelTransfer(const std::string& transferId)` | 判断 cancel transfer 相关逻辑。 |
| [L1703](../src/apps/controller/FileTransferWindow.cpp#L1703) | `FileTransferWindow::ResumeTransfer` | 定义 | `void FileTransferWindow::ResumeTransfer(const std::string& transferId)` | 实现 resume transfer 对应的业务或工具逻辑。 |
| [L1716](../src/apps/controller/FileTransferWindow.cpp#L1716) | `FileTransferWindow::OpenTransferFolder` | 定义 | `void FileTransferWindow::OpenTransferFolder( const std::filesystem::path& path)` | 启动 open transfer folder 相关逻辑。 |

## `src/apps/controller/FileTransferWindow.h`

[打开源码](../src/apps/controller/FileTransferWindow.h) · **文件作用：** 声明 file transfer window 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L20](../src/apps/controller/FileTransferWindow.h#L20) | `QCloseEvent` | class | 定义 QCloseEvent 的 class 类型和相关状态。 |
| [L21](../src/apps/controller/FileTransferWindow.h#L21) | `QComboBox` | class | 定义 QComboBox 的 class 类型和相关状态。 |
| [L22](../src/apps/controller/FileTransferWindow.h#L22) | `QEvent` | class | 定义 QEvent 的 class 类型和相关状态。 |
| [L23](../src/apps/controller/FileTransferWindow.h#L23) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L24](../src/apps/controller/FileTransferWindow.h#L24) | `QResizeEvent` | class | 定义 QResizeEvent 的 class 类型和相关状态。 |
| [L25](../src/apps/controller/FileTransferWindow.h#L25) | `QPushButton` | class | 定义 QPushButton 的 class 类型和相关状态。 |
| [L26](../src/apps/controller/FileTransferWindow.h#L26) | `QScrollArea` | class | 定义 QScrollArea 的 class 类型和相关状态。 |
| [L27](../src/apps/controller/FileTransferWindow.h#L27) | `QTimer` | class | 定义 QTimer 的 class 类型和相关状态。 |
| [L28](../src/apps/controller/FileTransferWindow.h#L28) | `QVBoxLayout` | class | 定义 QVBoxLayout 的 class 类型和相关状态。 |
| [L29](../src/apps/controller/FileTransferWindow.h#L29) | `QWidget` | class | 定义 QWidget 的 class 类型和相关状态。 |
| [L33](../src/apps/controller/FileTransferWindow.h#L33) | `FileTransferCard` | class | 定义 FileTransferCard 的 class 类型和相关状态。 |
| [L35](../src/apps/controller/FileTransferWindow.h#L35) | `FileTransferPeer` | struct | 定义 FileTransferPeer 的 struct 类型和相关状态。 |
| [L40](../src/apps/controller/FileTransferWindow.h#L40) | `FileTransferWindow` | class | 定义 FileTransferWindow 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L20](../src/apps/controller/FileTransferWindow.h#L20) | `QCloseEvent` | `class QCloseEvent;` | 保存 q close event 相关配置或运行状态。 |
| [L21](../src/apps/controller/FileTransferWindow.h#L21) | `QComboBox` | `class QComboBox;` | 保存 q combo box 相关配置或运行状态。 |
| [L22](../src/apps/controller/FileTransferWindow.h#L22) | `QEvent` | `class QEvent;` | 保存 q event 相关配置或运行状态。 |
| [L23](../src/apps/controller/FileTransferWindow.h#L23) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L24](../src/apps/controller/FileTransferWindow.h#L24) | `QResizeEvent` | `class QResizeEvent;` | 保存 q resize event 相关配置或运行状态。 |
| [L25](../src/apps/controller/FileTransferWindow.h#L25) | `QPushButton` | `class QPushButton;` | 保存 q push button 相关配置或运行状态。 |
| [L26](../src/apps/controller/FileTransferWindow.h#L26) | `QScrollArea` | `class QScrollArea;` | 保存 q scroll area 相关配置或运行状态。 |
| [L27](../src/apps/controller/FileTransferWindow.h#L27) | `QTimer` | `class QTimer;` | 保存定时、截止或超时状态：q timer。 |
| [L28](../src/apps/controller/FileTransferWindow.h#L28) | `QVBoxLayout` | `class QVBoxLayout;` | 保存 qv box layout 相关配置或运行状态。 |
| [L29](../src/apps/controller/FileTransferWindow.h#L29) | `QWidget` | `class QWidget;` | 保存 q widget 相关配置或运行状态。 |
| [L33](../src/apps/controller/FileTransferWindow.h#L33) | `FileTransferCard` | `class FileTransferCard;` | 保存 file transfer card 相关配置或运行状态。 |
| [L36](../src/apps/controller/FileTransferWindow.h#L36) | `deviceId` | `std::string deviceId;` | 保存身份或作用域标识：device id。 |
| [L37](../src/apps/controller/FileTransferWindow.h#L37) | `displayName` | `QString displayName;` | 保存路径、地址或显示名称：display name。 |
| [L87](../src/apps/controller/FileTransferWindow.h#L87) | `controller_` | `app::FileTransferController* controller_ = nullptr;` | 保存 controller 相关配置或运行状态。 |
| [L88](../src/apps/controller/FileTransferWindow.h#L88) | `peerSelector_` | `QComboBox* peerSelector_ = nullptr;` | 保存 peer selector 相关配置或运行状态。 |
| [L89](../src/apps/controller/FileTransferWindow.h#L89) | `titleBar_` | `CustomTitleBar* titleBar_ = nullptr;` | 保存 title bar 相关配置或运行状态。 |
| [L90](../src/apps/controller/FileTransferWindow.h#L90) | `sendButton_` | `QPushButton* sendButton_ = nullptr;` | 保存 send button 相关配置或运行状态。 |
| [L91](../src/apps/controller/FileTransferWindow.h#L91) | `peerHint_` | `QLabel* peerHint_ = nullptr;` | 保存 peer hint 相关配置或运行状态。 |
| [L92](../src/apps/controller/FileTransferWindow.h#L92) | `emptyState_` | `QLabel* emptyState_ = nullptr;` | 保存状态机当前状态：empty state。 |
| [L93](../src/apps/controller/FileTransferWindow.h#L93) | `transferContainer_` | `QWidget* transferContainer_ = nullptr;` | 保存 transfer container 相关配置或运行状态。 |
| [L94](../src/apps/controller/FileTransferWindow.h#L94) | `transferScroll_` | `QScrollArea* transferScroll_ = nullptr;` | 保存 transfer scroll 相关配置或运行状态。 |
| [L95](../src/apps/controller/FileTransferWindow.h#L95) | `transferLayout_` | `QVBoxLayout* transferLayout_ = nullptr;` | 保存 transfer layout 相关配置或运行状态。 |
| [L96](../src/apps/controller/FileTransferWindow.h#L96) | `transferCards_` | `QHash<QString, FileTransferCard*> transferCards_;` | 保存 transfer cards 相关配置或运行状态。 |
| [L97](../src/apps/controller/FileTransferWindow.h#L97) | `transfers_` | `std::vector<app::FileTransferSnapshot> transfers_;` | 保存 transfers 相关配置或运行状态。 |
| [L98](../src/apps/controller/FileTransferWindow.h#L98) | `pendingTransfersMutex_` | `std::mutex pendingTransfersMutex_;` | 保护跨线程共享状态：pending transfers mutex。 |
| [L99](../src/apps/controller/FileTransferWindow.h#L99) | `pendingTransfers_` | `std::vector<app::FileTransferSnapshot> pendingTransfers_;` | 保存 pending transfers 相关配置或运行状态。 |
| [L100](../src/apps/controller/FileTransferWindow.h#L100) | `transferApplyScheduled_` | `bool transferApplyScheduled_ = false;` | 保存 transfer apply scheduled 相关配置或运行状态。 |
| [L101](../src/apps/controller/FileTransferWindow.h#L101) | `anchorGeometry_` | `QRect anchorGeometry_;` | 保存 anchor geometry 相关配置或运行状态。 |
| [L102](../src/apps/controller/FileTransferWindow.h#L102) | `drawerHost_` | `QPointer<QWidget> drawerHost_;` | 保存 drawer host 相关配置或运行状态。 |
| [L103](../src/apps/controller/FileTransferWindow.h#L103) | `drawerMotionTimer_` | `QTimer* drawerMotionTimer_ = nullptr;` | 保存定时、截止或超时状态：drawer motion timer。 |
| [L104](../src/apps/controller/FileTransferWindow.h#L104) | `drawerMotionClock_` | `QElapsedTimer drawerMotionClock_;` | 保护跨线程共享状态：drawer motion clock。 |
| [L105](../src/apps/controller/FileTransferWindow.h#L105) | `drawerMotionStart_` | `QPoint drawerMotionStart_;` | 保存 drawer motion start 相关配置或运行状态。 |
| [L106](../src/apps/controller/FileTransferWindow.h#L106) | `drawerMotionTarget_` | `QPoint drawerMotionTarget_;` | 保存 drawer motion target 相关配置或运行状态。 |
| [L107](../src/apps/controller/FileTransferWindow.h#L107) | `drawerMotionDurationMs_` | `int drawerMotionDurationMs_ = 0;` | 保存 drawer motion duration ms 相关配置或运行状态。 |
| [L108](../src/apps/controller/FileTransferWindow.h#L108) | `drawerHiding_` | `bool drawerHiding_ = false;` | 保存 drawer hiding 相关配置或运行状态。 |
| [L109](../src/apps/controller/FileTransferWindow.h#L109) | `drawerResizeHandle_` | `QWidget* drawerResizeHandle_ = nullptr;` | 保存 drawer resize handle 相关配置或运行状态。 |
| [L110](../src/apps/controller/FileTransferWindow.h#L110) | `drawerWidth_` | `int drawerWidth_ = 390;` | 保存计数、尺寸或速率指标：drawer width。 |
| [L111](../src/apps/controller/FileTransferWindow.h#L111) | `drawerResizeDragging_` | `bool drawerResizeDragging_ = false;` | 保存 drawer resize dragging 相关配置或运行状态。 |
| [L112](../src/apps/controller/FileTransferWindow.h#L112) | `drawerResizeStartGlobalX_` | `int drawerResizeStartGlobalX_ = 0;` | 保存 drawer resize start global x 相关配置或运行状态。 |
| [L113](../src/apps/controller/FileTransferWindow.h#L113) | `drawerResizeStartWidth_` | `int drawerResizeStartWidth_ = 0;` | 保存计数、尺寸或速率指标：drawer resize start width。 |
| [L114](../src/apps/controller/FileTransferWindow.h#L114) | `announcedIncomingTransfers_` | `QSet<QString> announcedIncomingTransfers_;` | 保存 announced incoming transfers 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L44](../src/apps/controller/FileTransferWindow.h#L44) | `FileTransferWindow` | 声明 | `explicit FileTransferWindow(app::FileTransferController* controller, QWidget* parent = nullptr)` | 实现 file transfer window 对应的业务或工具逻辑。 |
| [L46](../src/apps/controller/FileTransferWindow.h#L46) | `~FileTransferWindow` | 声明 | `~FileTransferWindow() override` | 停止相关活动并释放 FileTransferWindow 实例拥有的资源。 |
| [L48](../src/apps/controller/FileTransferWindow.h#L48) | `SyncPeers` | 声明 | `void SyncPeers(const std::vector<FileTransferPeer>& peers, const QRect& anchorGeometry)` | 实现 sync peers 对应的业务或工具逻辑。 |
| [L50](../src/apps/controller/FileTransferWindow.h#L50) | `AttachAsDrawer` | 声明 | `void AttachAsDrawer(QWidget* host)` | 实现 attach as drawer 对应的业务或工具逻辑。 |
| [L51](../src/apps/controller/FileTransferWindow.h#L51) | `OpenBesideMainWindow` | 声明 | `void OpenBesideMainWindow(const QRect& mainWindowGeometry)` | 启动 open beside main window 相关逻辑。 |
| [L52](../src/apps/controller/FileTransferWindow.h#L52) | `HideWithAnimation` | 声明 | `void HideWithAnimation()` | 实现 hide with animation 对应的业务或工具逻辑。 |
| [L53](../src/apps/controller/FileTransferWindow.h#L53) | `HideImmediately` | 声明 | `void HideImmediately()` | 实现 hide immediately 对应的业务或工具逻辑。 |
| [L54](../src/apps/controller/FileTransferWindow.h#L54) | `UpdateDrawerGeometry` | 声明 | `void UpdateDrawerGeometry()` | 更新或应用 update drawer geometry 相关逻辑。 |
| [L55](../src/apps/controller/FileTransferWindow.h#L55) | `IsHiding` | 声明 | `bool IsHiding() const` | 判断 is hiding 相关逻辑。 |
| [L56](../src/apps/controller/FileTransferWindow.h#L56) | `DetachController` | 声明 | `void DetachController()` | 实现 detach controller 对应的业务或工具逻辑。 |
| [L57](../src/apps/controller/FileTransferWindow.h#L57) | `RefreshThemeStyle` | 声明 | `void RefreshThemeStyle()` | 刷新 refresh theme style 相关逻辑。 |
| [L60](../src/apps/controller/FileTransferWindow.h#L60) | `closeEvent` | 声明 | `void closeEvent(QCloseEvent* event) override` | 关闭并清理 close event 相关逻辑。 |
| [L61](../src/apps/controller/FileTransferWindow.h#L61) | `resizeEvent` | 声明 | `void resizeEvent(QResizeEvent* event) override` | 实现 resize event 对应的业务或工具逻辑。 |
| [L62](../src/apps/controller/FileTransferWindow.h#L62) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L65](../src/apps/controller/FileTransferWindow.h#L65) | `OnFileTransfersChanged` | 声明 | `void OnFileTransfersChanged( const std::vector<app::FileTransferSnapshot>& transfers) override` | 接收并处理 on file transfers changed 相关逻辑。 |
| [L67](../src/apps/controller/FileTransferWindow.h#L67) | `BuildUi` | 声明 | `void BuildUi()` | 创建或初始化 build ui 相关逻辑。 |
| [L68](../src/apps/controller/FileTransferWindow.h#L68) | `ApplyTransfers` | 声明 | `void ApplyTransfers( std::vector<app::FileTransferSnapshot> transfers)` | 更新或应用 apply transfers 相关逻辑。 |
| [L70](../src/apps/controller/FileTransferWindow.h#L70) | `ScheduleTransferApply` | 声明 | `void ScheduleTransferApply()` | 执行后台循环或调度 schedule transfer apply 相关逻辑。 |
| [L71](../src/apps/controller/FileTransferWindow.h#L71) | `RebuildTransferCards` | 声明 | `void RebuildTransferCards()` | 更新或应用 rebuild transfer cards 相关逻辑。 |
| [L72](../src/apps/controller/FileTransferWindow.h#L72) | `UpdateTransferCardsIncrementally` | 声明 | `void UpdateTransferCardsIncrementally()` | 更新或应用 update transfer cards incrementally 相关逻辑。 |
| [L73](../src/apps/controller/FileTransferWindow.h#L73) | `SetDrawerWidth` | 声明 | `void SetDrawerWidth(int requestedWidth)` | 更新或应用 set drawer width 相关逻辑。 |
| [L74](../src/apps/controller/FileTransferWindow.h#L74) | `EffectiveDrawerWidth` | 声明 | `int EffectiveDrawerWidth() const` | 实现 effective drawer width 对应的业务或工具逻辑。 |
| [L75](../src/apps/controller/FileTransferWindow.h#L75) | `StartDrawerMotion` | 声明 | `void StartDrawerMotion(const QPoint& targetPosition, int durationMs)` | 启动 start drawer motion 相关逻辑。 |
| [L76](../src/apps/controller/FileTransferWindow.h#L76) | `StopDrawerAnimation` | 声明 | `void StopDrawerAnimation()` | 停止 stop drawer animation 相关逻辑。 |
| [L77](../src/apps/controller/FileTransferWindow.h#L77) | `ChooseAndSendFile` | 声明 | `void ChooseAndSendFile()` | 实现 choose and send file 对应的业务或工具逻辑。 |
| [L78](../src/apps/controller/FileTransferWindow.h#L78) | `AcceptTransfer` | 声明 | `void AcceptTransfer(const std::string& transferId)` | 处理并回复 accept transfer 相关逻辑。 |
| [L79](../src/apps/controller/FileTransferWindow.h#L79) | `SaveTransferAs` | 声明 | `void SaveTransferAs(const std::string& transferId)` | 保存或写入 save transfer as 相关逻辑。 |
| [L80](../src/apps/controller/FileTransferWindow.h#L80) | `AcceptTransferToDirectory` | 声明 | `void AcceptTransferToDirectory(const std::string& transferId, const QString& directory)` | 处理并回复 accept transfer to directory 相关逻辑。 |
| [L82](../src/apps/controller/FileTransferWindow.h#L82) | `RejectTransfer` | 声明 | `void RejectTransfer(const std::string& transferId)` | 处理并回复 reject transfer 相关逻辑。 |
| [L83](../src/apps/controller/FileTransferWindow.h#L83) | `CancelTransfer` | 声明 | `void CancelTransfer(const std::string& transferId)` | 判断 cancel transfer 相关逻辑。 |
| [L84](../src/apps/controller/FileTransferWindow.h#L84) | `ResumeTransfer` | 声明 | `void ResumeTransfer(const std::string& transferId)` | 实现 resume transfer 对应的业务或工具逻辑。 |
| [L85](../src/apps/controller/FileTransferWindow.h#L85) | `OpenTransferFolder` | 声明 | `void OpenTransferFolder(const std::filesystem::path& path)` | 启动 open transfer folder 相关逻辑。 |

## `src/apps/controller/FramelessWindow.cpp`

[打开源码](../src/apps/controller/FramelessWindow.cpp) · **文件作用：** 实现 frameless window 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L93](../src/apps/controller/FramelessWindow.cpp#L93) | `ResizeBorderMetrics` | struct | 定义 ResizeBorderMetrics 的 struct 类型和相关状态。 |
| [L158](../src/apps/controller/FramelessWindow.cpp#L158) | `FramelessResizeOverlay` | class | 定义 FramelessResizeOverlay 的 class 类型和相关状态。 |
| [L396](../src/apps/controller/FramelessWindow.cpp#L396) | `SmoothWheelFilter` | class | 定义 SmoothWheelFilter 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L248](../src/apps/controller/FramelessWindow.cpp#L248) | `kGrip` | `static constexpr int kGrip = 8;` | 定义 grip 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L43](../src/apps/controller/FramelessWindow.cpp#L43) | `MakeCaptionButton` | 定义 | `QToolButton* MakeCaptionButton(const QString& iconResource, const QString& objectName, QWidget* parent)` | 创建或初始化 make caption button 相关逻辑。 |
| [L59](../src/apps/controller/FramelessWindow.cpp#L59) | `SetNativeRoundedCorners` | 定义 | `void SetNativeRoundedCorners(HWND window, bool enabled)` | 更新或应用 set native rounded corners 相关逻辑。 |
| [L71](../src/apps/controller/FramelessWindow.cpp#L71) | `EnableNativeWindowTransitions` | 定义 | `void EnableNativeWindowTransitions(HWND window)` | 实现 enable native window transitions 对应的业务或工具逻辑。 |
| [L98](../src/apps/controller/FramelessWindow.cpp#L98) | `ResizeBordersForWindow` | 定义 | `ResizeBorderMetrics ResizeBordersForWindow(HWND window)` | 实现 resize borders for window 对应的业务或工具逻辑。 |
| [L113](../src/apps/controller/FramelessWindow.cpp#L113) | `ResizeEdgesForHitTest` | 定义 | `Qt::Edges ResizeEdgesForHitTest(LRESULT hitTest)` | 实现 resize edges for hit test 对应的业务或工具逻辑。 |
| [L137](../src/apps/controller/FramelessWindow.cpp#L137) | `ResizeCursorForHitTest` | 定义 | `HCURSOR ResizeCursorForHitTest(LRESULT hitTest)` | 实现 resize cursor for hit test 对应的业务或工具逻辑。 |
| [L160](../src/apps/controller/FramelessWindow.cpp#L160) | `FramelessResizeOverlay` | 定义 | `explicit FramelessResizeOverlay(FramelessMainWindow* owner) : QWidget(owner, Qt::Tool \| Qt::FramelessWindowHint \| Qt::NoDropShadowWindowHint \| Qt::WindowDoesNotAcceptFocus), owner_(owner)` | 实现 frameless resize overlay 对应的业务或工具逻辑。 |
| [L178](../src/apps/controller/FramelessWindow.cpp#L178) | `eventFilter` | 定义 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L201](../src/apps/controller/FramelessWindow.cpp#L201) | `enterEvent` | 定义 | `void enterEvent(QEnterEvent* event) override` | 实现 enter event 对应的业务或工具逻辑。 |
| [L207](../src/apps/controller/FramelessWindow.cpp#L207) | `mouseMoveEvent` | 定义 | `void mouseMoveEvent(QMouseEvent* event) override` | 实现 mouse move event 对应的业务或工具逻辑。 |
| [L213](../src/apps/controller/FramelessWindow.cpp#L213) | `mousePressEvent` | 定义 | `void mousePressEvent(QMouseEvent* event) override` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L238](../src/apps/controller/FramelessWindow.cpp#L238) | `paintEvent` | 定义 | `void paintEvent(QPaintEvent*) override` | 准备或呈现 paint event 相关逻辑。 |
| [L251](../src/apps/controller/FramelessWindow.cpp#L251) | `NativeSizingEdge` | 定义 | `static WPARAM NativeSizingEdge(Qt::Edges edges)` | 实现 native sizing edge 对应的业务或工具逻辑。 |
| [L281](../src/apps/controller/FramelessWindow.cpp#L281) | `EdgesAt` | 定义 | `Qt::Edges EdgesAt(const QPoint& position) const` | 实现 edges at 对应的业务或工具逻辑。 |
| [L299](../src/apps/controller/FramelessWindow.cpp#L299) | `UpdateCursorForPosition` | 定义 | `void UpdateCursorForPosition(const QPoint& position)` | 更新或应用 update cursor for position 相关逻辑。 |
| [L324](../src/apps/controller/FramelessWindow.cpp#L324) | `SyncToOwner` | 定义 | `void SyncToOwner()` | 实现 sync to owner 对应的业务或工具逻辑。 |
| [L347](../src/apps/controller/FramelessWindow.cpp#L347) | `CreateRemoteCIcon` | 定义 | `QIcon CreateRemoteCIcon()` | 创建或初始化 create remote c icon 相关逻辑。 |
| [L356](../src/apps/controller/FramelessWindow.cpp#L356) | `ScaleUiStyleSheet` | 定义 | `QString ScaleUiStyleSheet(const QString& styleSheet)` | 转换或缩放 scale ui style sheet 相关逻辑。 |
| [L385](../src/apps/controller/FramelessWindow.cpp#L385) | `CurrentUiAnimationLevel` | 定义 | `int CurrentUiAnimationLevel()` | 实现 current ui animation level 对应的业务或工具逻辑。 |
| [L398](../src/apps/controller/FramelessWindow.cpp#L398) | `SmoothWheelFilter` | 定义 | `explicit SmoothWheelFilter(QAbstractScrollArea* area) : QObject(area), area_(area)` | 实现 smooth wheel filter 对应的业务或工具逻辑。 |
| [L408](../src/apps/controller/FramelessWindow.cpp#L408) | `eventFilter` | 定义 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L463](../src/apps/controller/FramelessWindow.cpp#L463) | `StopInertia` | 定义 | `void StopInertia()` | 停止 stop inertia 相关逻辑。 |
| [L470](../src/apps/controller/FramelessWindow.cpp#L470) | `AdvanceInertia` | 定义 | `void AdvanceInertia()` | 实现 advance inertia 对应的业务或工具逻辑。 |
| [L505](../src/apps/controller/FramelessWindow.cpp#L505) | `EnableSmoothWheelScrolling` | 定义 | `void EnableSmoothWheelScrolling(QAbstractScrollArea* scrollArea)` | 实现 enable smooth wheel scrolling 对应的业务或工具逻辑。 |
| [L516](../src/apps/controller/FramelessWindow.cpp#L516) | `FramelessMainWindow::FramelessMainWindow` | 定义 | `FramelessMainWindow::FramelessMainWindow(QWidget* parent, bool embedded) : QMainWindow(parent)` | 构造并初始化 FramelessMainWindow 实例。 |
| [L536](../src/apps/controller/FramelessWindow.cpp#L536) | `FramelessMainWindow::SetNativeRoundedCornersEnabled` | 定义 | `void FramelessMainWindow::SetNativeRoundedCornersEnabled(bool enabled)` | 更新或应用 set native rounded corners enabled 相关逻辑。 |
| [L546](../src/apps/controller/FramelessWindow.cpp#L546) | `FramelessMainWindow::changeEvent` | 定义 | `void FramelessMainWindow::changeEvent(QEvent* event)` | 实现 change event 对应的业务或工具逻辑。 |
| [L566](../src/apps/controller/FramelessWindow.cpp#L566) | `FramelessMainWindow::ApplyUiStyleSheet` | 定义 | `void FramelessMainWindow::ApplyUiStyleSheet(const QString& styleSheet)` | 更新或应用 apply ui style sheet 相关逻辑。 |
| [L572](../src/apps/controller/FramelessWindow.cpp#L572) | `FramelessMainWindow::RefreshWindowStyle` | 定义 | `void FramelessMainWindow::RefreshWindowStyle()` | 刷新 refresh window style 相关逻辑。 |
| [L579](../src/apps/controller/FramelessWindow.cpp#L579) | `FramelessMainWindow::RefreshAllWindowStyles` | 定义 | `void FramelessMainWindow::RefreshAllWindowStyles()` | 刷新 refresh all window styles 相关逻辑。 |
| [L595](../src/apps/controller/FramelessWindow.cpp#L595) | `FramelessMainWindow::MinimizeWithSystemAnimation` | 定义 | `void FramelessMainWindow::MinimizeWithSystemAnimation()` | 实现 minimize with system animation 对应的业务或工具逻辑。 |
| [L609](../src/apps/controller/FramelessWindow.cpp#L609) | `FramelessMainWindow::AnimateWindowEntrance` | 定义 | `void FramelessMainWindow::AnimateWindowEntrance( const QPoint& fullAnimationOffset, const QRect& globalClipRect, int durationOverrideMs)` | 实现 animate window entrance 对应的业务或工具逻辑。 |
| [L680](../src/apps/controller/FramelessWindow.cpp#L680) | `FramelessMainWindow::AnimateWindowExit` | 定义 | `void FramelessMainWindow::AnimateWindowExit( const QPoint& fullAnimationOffset, const QRect& globalClipRect, int durationOverrideMs)` | 实现 animate window exit 对应的业务或工具逻辑。 |
| [L751](../src/apps/controller/FramelessWindow.cpp#L751) | `FramelessMainWindow::nativeEvent` | 定义 | `bool FramelessMainWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)` | 实现 native event 对应的业务或工具逻辑。 |
| [L953](../src/apps/controller/FramelessWindow.cpp#L953) | `FramelessMainWindow::ConstrainResizeGeometry` | 定义 | `QRect FramelessMainWindow::ConstrainResizeGeometry( const QRect& proposedGeometry, Qt::Edges resizeEdges, qreal devicePixelRatio) const` | 实现 constrain resize geometry 对应的业务或工具逻辑。 |
| [L963](../src/apps/controller/FramelessWindow.cpp#L963) | `CustomTitleBar::CustomTitleBar` | 定义 | `CustomTitleBar::CustomTitleBar(FramelessMainWindow* window, QString title, QWidget* parent) : QWidget(parent), window_(window)` | 构造并初始化 CustomTitleBar 实例。 |
| [L1044](../src/apps/controller/FramelessWindow.cpp#L1044) | `CustomTitleBar::SetMinimizeAction` | 定义 | `void CustomTitleBar::SetMinimizeAction(std::function<void()> action)` | 更新或应用 set minimize action 相关逻辑。 |
| [L1049](../src/apps/controller/FramelessWindow.cpp#L1049) | `CustomTitleBar::SetEmbeddedMode` | 定义 | `void CustomTitleBar::SetEmbeddedMode(bool embedded)` | 更新或应用 set embedded mode 相关逻辑。 |
| [L1056](../src/apps/controller/FramelessWindow.cpp#L1056) | `CustomTitleBar::SetTitle` | 定义 | `void CustomTitleBar::SetTitle(const QString& title)` | 更新或应用 set title 相关逻辑。 |
| [L1061](../src/apps/controller/FramelessWindow.cpp#L1061) | `CustomTitleBar::RefreshThemeStyle` | 定义 | `void CustomTitleBar::RefreshThemeStyle(bool dark)` | 刷新 refresh theme style 相关逻辑。 |
| [L1087](../src/apps/controller/FramelessWindow.cpp#L1087) | `CustomTitleBar::eventFilter` | 定义 | `bool CustomTitleBar::eventFilter(QObject* watched, QEvent* event)` | 实现 event filter 对应的业务或工具逻辑。 |
| [L1095](../src/apps/controller/FramelessWindow.cpp#L1095) | `CustomTitleBar::mouseDoubleClickEvent` | 定义 | `void CustomTitleBar::mouseDoubleClickEvent(QMouseEvent* event)` | 实现 mouse double click event 对应的业务或工具逻辑。 |
| [L1109](../src/apps/controller/FramelessWindow.cpp#L1109) | `CustomTitleBar::mousePressEvent` | 定义 | `void CustomTitleBar::mousePressEvent(QMouseEvent* event)` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L1123](../src/apps/controller/FramelessWindow.cpp#L1123) | `CustomTitleBar::ToggleMaximized` | 定义 | `void CustomTitleBar::ToggleMaximized()` | 实现 toggle maximized 对应的业务或工具逻辑。 |
| [L1143](../src/apps/controller/FramelessWindow.cpp#L1143) | `CustomTitleBar::UpdateMaximizeButton` | 定义 | `void CustomTitleBar::UpdateMaximizeButton()` | 更新或应用 update maximize button 相关逻辑。 |

## `src/apps/controller/FramelessWindow.h`

[打开源码](../src/apps/controller/FramelessWindow.h) · **文件作用：** 声明 frameless window 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L14](../src/apps/controller/FramelessWindow.h#L14) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L15](../src/apps/controller/FramelessWindow.h#L15) | `QAbstractScrollArea` | class | 定义 QAbstractScrollArea 的 class 类型和相关状态。 |
| [L16](../src/apps/controller/FramelessWindow.h#L16) | `QToolButton` | class | 定义 QToolButton 的 class 类型和相关状态。 |
| [L25](../src/apps/controller/FramelessWindow.h#L25) | `FramelessMainWindow` | class | 定义 FramelessMainWindow 的 class 类型和相关状态。 |
| [L64](../src/apps/controller/FramelessWindow.h#L64) | `CustomTitleBar` | class | 定义 CustomTitleBar 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L14](../src/apps/controller/FramelessWindow.h#L14) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L15](../src/apps/controller/FramelessWindow.h#L15) | `QAbstractScrollArea` | `class QAbstractScrollArea;` | 保存 q abstract scroll area 相关配置或运行状态。 |
| [L16](../src/apps/controller/FramelessWindow.h#L16) | `QToolButton` | `class QToolButton;` | 保存 q tool button 相关配置或运行状态。 |
| [L55](../src/apps/controller/FramelessWindow.h#L55) | `baseStyleSheet_` | `QString baseStyleSheet_;` | 保存 base style sheet 相关配置或运行状态。 |
| [L56](../src/apps/controller/FramelessWindow.h#L56) | `resizeOverlay_` | `QWidget* resizeOverlay_ = nullptr;` | 保存 resize overlay 相关配置或运行状态。 |
| [L58](../src/apps/controller/FramelessWindow.h#L58) | `resizeEdges_` | `Qt::Edges resizeEdges_{};` | 保存 resize edges 相关配置或运行状态。 |
| [L59](../src/apps/controller/FramelessWindow.h#L59) | `resizeStartCursor_` | `QPoint resizeStartCursor_;` | 保存 resize start cursor 相关配置或运行状态。 |
| [L60](../src/apps/controller/FramelessWindow.h#L60) | `resizeStartWindowRect_` | `QRect resizeStartWindowRect_;` | 保存 resize start window rect 相关配置或运行状态。 |
| [L84](../src/apps/controller/FramelessWindow.h#L84) | `window_` | `FramelessMainWindow* window_ = nullptr;` | 保存 window 相关配置或运行状态。 |
| [L85](../src/apps/controller/FramelessWindow.h#L85) | `titleLabel_` | `QLabel* titleLabel_ = nullptr;` | 保存路径、地址或显示名称：title label。 |
| [L87](../src/apps/controller/FramelessWindow.h#L87) | `minimizeButton_` | `QToolButton* minimizeButton_ = nullptr;` | 保存 minimize button 相关配置或运行状态。 |
| [L88](../src/apps/controller/FramelessWindow.h#L88) | `maximizeButton_` | `QToolButton* maximizeButton_ = nullptr;` | 保存 maximize button 相关配置或运行状态。 |
| [L89](../src/apps/controller/FramelessWindow.h#L89) | `closeButton_` | `QToolButton* closeButton_ = nullptr;` | 保存 close button 相关配置或运行状态。 |
| [L90](../src/apps/controller/FramelessWindow.h#L90) | `embedded_` | `bool embedded_ = false;` | 保存 embedded 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L20](../src/apps/controller/FramelessWindow.h#L20) | `CreateRemoteCIcon` | 声明 | `QIcon CreateRemoteCIcon()` | 创建或初始化 create remote c icon 相关逻辑。 |
| [L21](../src/apps/controller/FramelessWindow.h#L21) | `ScaleUiStyleSheet` | 声明 | `QString ScaleUiStyleSheet(const QString& styleSheet)` | 转换或缩放 scale ui style sheet 相关逻辑。 |
| [L22](../src/apps/controller/FramelessWindow.h#L22) | `CurrentUiAnimationLevel` | 声明 | `int CurrentUiAnimationLevel()` | 实现 current ui animation level 对应的业务或工具逻辑。 |
| [L23](../src/apps/controller/FramelessWindow.h#L23) | `EnableSmoothWheelScrolling` | 声明 | `void EnableSmoothWheelScrolling(QAbstractScrollArea* scrollArea)` | 实现 enable smooth wheel scrolling 对应的业务或工具逻辑。 |
| [L27](../src/apps/controller/FramelessWindow.h#L27) | `FramelessMainWindow` | 声明 | `explicit FramelessMainWindow(QWidget* parent = nullptr, bool embedded = false)` | 实现 frameless main window 对应的业务或工具逻辑。 |
| [L29](../src/apps/controller/FramelessWindow.h#L29) | `~FramelessMainWindow` | 声明 | `~FramelessMainWindow() override = default` | 停止相关活动并释放 FramelessMainWindow 实例拥有的资源。 |
| [L31](../src/apps/controller/FramelessWindow.h#L31) | `RefreshAllWindowStyles` | 声明 | `static void RefreshAllWindowStyles()` | 刷新 refresh all window styles 相关逻辑。 |
| [L32](../src/apps/controller/FramelessWindow.h#L32) | `MinimizeWithSystemAnimation` | 声明 | `void MinimizeWithSystemAnimation()` | 实现 minimize with system animation 对应的业务或工具逻辑。 |
| [L33](../src/apps/controller/FramelessWindow.h#L33) | `AnimateWindowEntrance` | 声明 | `void AnimateWindowEntrance(const QPoint& fullAnimationOffset = QPoint(10, 0), const QRect& globalClipRect = QRect(), int durationOverrideMs = 0)` | 实现 animate window entrance 对应的业务或工具逻辑。 |
| [L34](../src/apps/controller/FramelessWindow.h#L34) | `QPoint` | 声明 | `QPoint(10, 0), const QRect& globalClipRect = QRect(), int durationOverrideMs = 0)` | 实现 q point 对应的业务或工具逻辑。 |
| [L37](../src/apps/controller/FramelessWindow.h#L37) | `AnimateWindowExit` | 声明 | `void AnimateWindowExit(const QPoint& fullAnimationOffset = QPoint(), const QRect& globalClipRect = QRect(), int durationOverrideMs = 0)` | 实现 animate window exit 对应的业务或工具逻辑。 |
| [L42](../src/apps/controller/FramelessWindow.h#L42) | `ApplyUiStyleSheet` | 声明 | `void ApplyUiStyleSheet(const QString& styleSheet)` | 更新或应用 apply ui style sheet 相关逻辑。 |
| [L43](../src/apps/controller/FramelessWindow.h#L43) | `SetNativeRoundedCornersEnabled` | 声明 | `void SetNativeRoundedCornersEnabled(bool enabled)` | 更新或应用 set native rounded corners enabled 相关逻辑。 |
| [L44](../src/apps/controller/FramelessWindow.h#L44) | `changeEvent` | 声明 | `void changeEvent(QEvent* event) override` | 实现 change event 对应的业务或工具逻辑。 |
| [L45](../src/apps/controller/FramelessWindow.h#L45) | `nativeEvent` | 声明 | `bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override` | 实现 native event 对应的业务或工具逻辑。 |
| [L48](../src/apps/controller/FramelessWindow.h#L48) | `ConstrainResizeGeometry` | 声明 | `virtual QRect ConstrainResizeGeometry(const QRect& proposedGeometry, Qt::Edges resizeEdges, qreal devicePixelRatio) const` | 实现 constrain resize geometry 对应的业务或工具逻辑。 |
| [L53](../src/apps/controller/FramelessWindow.h#L53) | `RefreshWindowStyle` | 声明 | `void RefreshWindowStyle()` | 刷新 refresh window style 相关逻辑。 |
| [L66](../src/apps/controller/FramelessWindow.h#L66) | `CustomTitleBar` | 声明 | `CustomTitleBar(FramelessMainWindow* window, QString title, QWidget* parent = nullptr)` | 实现 custom title bar 对应的业务或工具逻辑。 |
| [L70](../src/apps/controller/FramelessWindow.h#L70) | `SetMinimizeAction` | 声明 | `void SetMinimizeAction(std::function<void()> action)` | 更新或应用 set minimize action 相关逻辑。 |
| [L71](../src/apps/controller/FramelessWindow.h#L71) | `SetEmbeddedMode` | 声明 | `void SetEmbeddedMode(bool embedded)` | 更新或应用 set embedded mode 相关逻辑。 |
| [L72](../src/apps/controller/FramelessWindow.h#L72) | `SetTitle` | 声明 | `void SetTitle(const QString& title)` | 更新或应用 set title 相关逻辑。 |
| [L73](../src/apps/controller/FramelessWindow.h#L73) | `RefreshThemeStyle` | 声明 | `void RefreshThemeStyle(bool dark)` | 刷新 refresh theme style 相关逻辑。 |
| [L76](../src/apps/controller/FramelessWindow.h#L76) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L77](../src/apps/controller/FramelessWindow.h#L77) | `mouseDoubleClickEvent` | 声明 | `void mouseDoubleClickEvent(QMouseEvent* event) override` | 实现 mouse double click event 对应的业务或工具逻辑。 |
| [L78](../src/apps/controller/FramelessWindow.h#L78) | `mousePressEvent` | 声明 | `void mousePressEvent(QMouseEvent* event) override` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L81](../src/apps/controller/FramelessWindow.h#L81) | `ToggleMaximized` | 声明 | `void ToggleMaximized()` | 实现 toggle maximized 对应的业务或工具逻辑。 |
| [L82](../src/apps/controller/FramelessWindow.h#L82) | `UpdateMaximizeButton` | 声明 | `void UpdateMaximizeButton()` | 更新或应用 update maximize button 相关逻辑。 |

## `src/apps/controller/LoginWindow.cpp`

[打开源码](../src/apps/controller/LoginWindow.cpp) · **文件作用：** 实现 login window 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L41](../src/apps/controller/LoginWindow.cpp#L41) | `AccountRevealEasing` | 定义 | `const QEasingCurve& AccountRevealEasing()` | 实现 account reveal easing 对应的业务或工具逻辑。 |
| [L52](../src/apps/controller/LoginWindow.cpp#L52) | `AccountDismissEasing` | 定义 | `const QEasingCurve& AccountDismissEasing()` | 实现 account dismiss easing 对应的业务或工具逻辑。 |
| [L65](../src/apps/controller/LoginWindow.cpp#L65) | `LoginStatusWindow::LoginStatusWindow` | 定义 | `LoginStatusWindow::LoginStatusWindow(QWidget* parent) : QDialog(parent)` | 构造并初始化 LoginStatusWindow 实例。 |
| [L140](../src/apps/controller/LoginWindow.cpp#L140) | `LoginStatusWindow::SetAccountLabel` | 定义 | `void LoginStatusWindow::SetAccountLabel(const QString& accountLabel)` | 更新或应用 set account label 相关逻辑。 |
| [L146](../src/apps/controller/LoginWindow.cpp#L146) | `LoginStatusWindow::RefreshThemeStyle` | 定义 | `void LoginStatusWindow::RefreshThemeStyle()` | 刷新 refresh theme style 相关逻辑。 |
| [L166](../src/apps/controller/LoginWindow.cpp#L166) | `LoginStatusWindow::ShowAndActivate` | 定义 | `void LoginStatusWindow::ShowAndActivate()` | 实现 show and activate 对应的业务或工具逻辑。 |
| [L187](../src/apps/controller/LoginWindow.cpp#L187) | `LoginStatusWindow::HideAndReleaseTopmost` | 定义 | `void LoginStatusWindow::HideAndReleaseTopmost()` | 实现 hide and release topmost 对应的业务或工具逻辑。 |
| [L198](../src/apps/controller/LoginWindow.cpp#L198) | `LoginStatusWindow::mousePressEvent` | 定义 | `void LoginStatusWindow::mousePressEvent(QMouseEvent* event)` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L207](../src/apps/controller/LoginWindow.cpp#L207) | `LoginWindow::LoginWindow` | 定义 | `LoginWindow::LoginWindow(QWidget* parent) : QDialog(parent)` | 构造并初始化 LoginWindow 实例。 |
| [L218](../src/apps/controller/LoginWindow.cpp#L218) | `LoginWindow::SetLoginRequestedCallback` | 定义 | `void LoginWindow::SetLoginRequestedCallback( std::function<void()> callback)` | 更新或应用 set login requested callback 相关逻辑。 |
| [L223](../src/apps/controller/LoginWindow.cpp#L223) | `LoginWindow::SetCancelRequestedCallback` | 定义 | `void LoginWindow::SetCancelRequestedCallback( std::function<void()> callback)` | 更新或应用 set cancel requested callback 相关逻辑。 |
| [L228](../src/apps/controller/LoginWindow.cpp#L228) | `LoginWindow::SetExitRequestedCallback` | 定义 | `void LoginWindow::SetExitRequestedCallback( std::function<void()> callback)` | 更新或应用 set exit requested callback 相关逻辑。 |
| [L233](../src/apps/controller/LoginWindow.cpp#L233) | `LoginWindow::SetSignOutRequestedCallback` | 定义 | `void LoginWindow::SetSignOutRequestedCallback( std::function<void()> callback)` | 更新或应用 set sign out requested callback 相关逻辑。 |
| [L238](../src/apps/controller/LoginWindow.cpp#L238) | `LoginWindow::SetDeleteAccountRequestedCallback` | 定义 | `void LoginWindow::SetDeleteAccountRequestedCallback( std::function<void()> callback)` | 更新或应用 set delete account requested callback 相关逻辑。 |
| [L243](../src/apps/controller/LoginWindow.cpp#L243) | `LoginWindow::SetOwnerWindow` | 定义 | `void LoginWindow::SetOwnerWindow(QWidget* owner)` | 更新或应用 set owner window 相关逻辑。 |
| [L256](../src/apps/controller/LoginWindow.cpp#L256) | `LoginWindow::ShowReady` | 定义 | `void LoginWindow::ShowReady(const QString& message)` | 实现 show ready 对应的业务或工具逻辑。 |
| [L276](../src/apps/controller/LoginWindow.cpp#L276) | `LoginWindow::ShowBusy` | 定义 | `void LoginWindow::ShowBusy( const QString& title, const QString& message)` | 实现 show busy 对应的业务或工具逻辑。 |
| [L294](../src/apps/controller/LoginWindow.cpp#L294) | `LoginWindow::ShowAccountDeletionBusy` | 定义 | `void LoginWindow::ShowAccountDeletionBusy()` | 实现 show account deletion busy 对应的业务或工具逻辑。 |
| [L303](../src/apps/controller/LoginWindow.cpp#L303) | `LoginWindow::ShowError` | 定义 | `void LoginWindow::ShowError( const QString& message, bool retryable)` | 实现 show error 对应的业务或工具逻辑。 |
| [L322](../src/apps/controller/LoginWindow.cpp#L322) | `LoginWindow::ShowAuthenticated` | 定义 | `void LoginWindow::ShowAuthenticated( const QString& accountLabel, const QString& accountDetail)` | 实现 show authenticated 对应的业务或工具逻辑。 |
| [L345](../src/apps/controller/LoginWindow.cpp#L345) | `LoginWindow::ShowAndActivate` | 定义 | `void LoginWindow::ShowAndActivate()` | 实现 show and activate 对应的业务或工具逻辑。 |
| [L380](../src/apps/controller/LoginWindow.cpp#L380) | `LoginWindow::HideWithAnimation` | 定义 | `void LoginWindow::HideWithAnimation()` | 实现 hide with animation 对应的业务或工具逻辑。 |
| [L408](../src/apps/controller/LoginWindow.cpp#L408) | `LoginWindow::IsHiding` | 定义 | `bool LoginWindow::IsHiding() const` | 判断 is hiding 相关逻辑。 |
| [L412](../src/apps/controller/LoginWindow.cpp#L412) | `LoginWindow::CardTargetGeometry` | 定义 | `QRect LoginWindow::CardTargetGeometry() const` | 实现 card target geometry 对应的业务或工具逻辑。 |
| [L436](../src/apps/controller/LoginWindow.cpp#L436) | `LoginWindow::LayoutCardLayer` | 定义 | `void LoginWindow::LayoutCardLayer()` | 实现 layout card layer 对应的业务或工具逻辑。 |
| [L443](../src/apps/controller/LoginWindow.cpp#L443) | `LoginWindow::StartVisibilityMotion` | 定义 | `void LoginWindow::StartVisibilityMotion(const QPoint& targetPosition, int durationMs)` | 启动 start visibility motion 相关逻辑。 |
| [L486](../src/apps/controller/LoginWindow.cpp#L486) | `LoginWindow::StopVisibilityMotion` | 定义 | `void LoginWindow::StopVisibilityMotion()` | 停止 stop visibility motion 相关逻辑。 |
| [L492](../src/apps/controller/LoginWindow.cpp#L492) | `LoginWindow::IsVisibilityMotionActive` | 定义 | `bool LoginWindow::IsVisibilityMotionActive() const` | 判断 is visibility motion active 相关逻辑。 |
| [L497](../src/apps/controller/LoginWindow.cpp#L497) | `LoginWindow::closeEvent` | 定义 | `void LoginWindow::closeEvent(QCloseEvent* event)` | 关闭并清理 close event 相关逻辑。 |
| [L513](../src/apps/controller/LoginWindow.cpp#L513) | `LoginWindow::eventFilter` | 定义 | `bool LoginWindow::eventFilter(QObject* watched, QEvent* event)` | 实现 event filter 对应的业务或工具逻辑。 |
| [L558](../src/apps/controller/LoginWindow.cpp#L558) | `LoginWindow::SetStatePanelTone` | 定义 | `void LoginWindow::SetStatePanelTone(const QString& tone)` | 更新或应用 set state panel tone 相关逻辑。 |
| [L570](../src/apps/controller/LoginWindow.cpp#L570) | `LoginWindow::mousePressEvent` | 定义 | `void LoginWindow::mousePressEvent(QMouseEvent* event)` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L580](../src/apps/controller/LoginWindow.cpp#L580) | `LoginWindow::BuildUi` | 定义 | `void LoginWindow::BuildUi()` | 创建或初始化 build ui 相关逻辑。 |
| [L874](../src/apps/controller/LoginWindow.cpp#L874) | `LoginWindow::RefreshThemeStyle` | 定义 | `void LoginWindow::RefreshThemeStyle()` | 刷新 refresh theme style 相关逻辑。 |
| [L936](../src/apps/controller/LoginWindow.cpp#L936) | `LoginWindow::UpdateAuthenticatedAccountText` | 定义 | `void LoginWindow::UpdateAuthenticatedAccountText()` | 更新或应用 update authenticated account text 相关逻辑。 |

## `src/apps/controller/LoginWindow.h`

[打开源码](../src/apps/controller/LoginWindow.h) · **文件作用：** 声明 login window 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L15](../src/apps/controller/LoginWindow.h#L15) | `QCloseEvent` | class | 定义 QCloseEvent 的 class 类型和相关状态。 |
| [L16](../src/apps/controller/LoginWindow.h#L16) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L17](../src/apps/controller/LoginWindow.h#L17) | `QMouseEvent` | class | 定义 QMouseEvent 的 class 类型和相关状态。 |
| [L18](../src/apps/controller/LoginWindow.h#L18) | `QProgressBar` | class | 定义 QProgressBar 的 class 类型和相关状态。 |
| [L19](../src/apps/controller/LoginWindow.h#L19) | `QPushButton` | class | 定义 QPushButton 的 class 类型和相关状态。 |
| [L20](../src/apps/controller/LoginWindow.h#L20) | `QFrame` | class | 定义 QFrame 的 class 类型和相关状态。 |
| [L21](../src/apps/controller/LoginWindow.h#L21) | `QTimer` | class | 定义 QTimer 的 class 类型和相关状态。 |
| [L25](../src/apps/controller/LoginWindow.h#L25) | `LoginStatusWindow` | class | 定义 LoginStatusWindow 的 class 类型和相关状态。 |
| [L40](../src/apps/controller/LoginWindow.h#L40) | `LoginWindow` | class | 定义 LoginWindow 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L15](../src/apps/controller/LoginWindow.h#L15) | `QCloseEvent` | `class QCloseEvent;` | 保存 q close event 相关配置或运行状态。 |
| [L16](../src/apps/controller/LoginWindow.h#L16) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L17](../src/apps/controller/LoginWindow.h#L17) | `QMouseEvent` | `class QMouseEvent;` | 保存 q mouse event 相关配置或运行状态。 |
| [L18](../src/apps/controller/LoginWindow.h#L18) | `QProgressBar` | `class QProgressBar;` | 保存 q progress bar 相关配置或运行状态。 |
| [L19](../src/apps/controller/LoginWindow.h#L19) | `QPushButton` | `class QPushButton;` | 保存 q push button 相关配置或运行状态。 |
| [L20](../src/apps/controller/LoginWindow.h#L20) | `QFrame` | `class QFrame;` | 保存媒体帧、图像或缓冲资源：q frame。 |
| [L21](../src/apps/controller/LoginWindow.h#L21) | `QTimer` | `class QTimer;` | 保存定时、截止或超时状态：q timer。 |
| [L37](../src/apps/controller/LoginWindow.h#L37) | `accountLabel_` | `QLabel* accountLabel_ = nullptr;` | 保存路径、地址或显示名称：account label。 |
| [L77](../src/apps/controller/LoginWindow.h#L77) | `titleLabel_` | `QLabel* titleLabel_ = nullptr;` | 保存路径、地址或显示名称：title label。 |
| [L78](../src/apps/controller/LoginWindow.h#L78) | `stateIcon_` | `QLabel* stateIcon_ = nullptr;` | 保存 state icon 相关配置或运行状态。 |
| [L79](../src/apps/controller/LoginWindow.h#L79) | `statusLabel_` | `QLabel* statusLabel_ = nullptr;` | 保存路径、地址或显示名称：status label。 |
| [L80](../src/apps/controller/LoginWindow.h#L80) | `securityLabel_` | `QLabel* securityLabel_ = nullptr;` | 保存路径、地址或显示名称：security label。 |
| [L81](../src/apps/controller/LoginWindow.h#L81) | `progressBar_` | `QProgressBar* progressBar_ = nullptr;` | 保存 progress bar 相关配置或运行状态。 |
| [L82](../src/apps/controller/LoginWindow.h#L82) | `loginButton_` | `QPushButton* loginButton_ = nullptr;` | 保存 login button 相关配置或运行状态。 |
| [L83](../src/apps/controller/LoginWindow.h#L83) | `cancelButton_` | `QPushButton* cancelButton_ = nullptr;` | 保存 cancel button 相关配置或运行状态。 |
| [L84](../src/apps/controller/LoginWindow.h#L84) | `exitButton_` | `QPushButton* exitButton_ = nullptr;` | 保存 exit button 相关配置或运行状态。 |
| [L85](../src/apps/controller/LoginWindow.h#L85) | `signOutButton_` | `QPushButton* signOutButton_ = nullptr;` | 保存 sign out button 相关配置或运行状态。 |
| [L86](../src/apps/controller/LoginWindow.h#L86) | `deleteAccountButton_` | `QPushButton* deleteAccountButton_ = nullptr;` | 保存 delete account button 相关配置或运行状态。 |
| [L87](../src/apps/controller/LoginWindow.h#L87) | `statePanel_` | `QFrame* statePanel_ = nullptr;` | 保存 state panel 相关配置或运行状态。 |
| [L88](../src/apps/controller/LoginWindow.h#L88) | `cardLayer_` | `QWidget* cardLayer_ = nullptr;` | 保存 card layer 相关配置或运行状态。 |
| [L89](../src/apps/controller/LoginWindow.h#L89) | `card_` | `QFrame* card_ = nullptr;` | 保存 card 相关配置或运行状态。 |
| [L90](../src/apps/controller/LoginWindow.h#L90) | `dragHandle_` | `QWidget* dragHandle_ = nullptr;` | 保存 drag handle 相关配置或运行状态。 |
| [L91](../src/apps/controller/LoginWindow.h#L91) | `visibilityMotionTimer_` | `QTimer* visibilityMotionTimer_ = nullptr;` | 保存定时、截止或超时状态：visibility motion timer。 |
| [L92](../src/apps/controller/LoginWindow.h#L92) | `visibilityMotionClock_` | `QElapsedTimer visibilityMotionClock_;` | 保护跨线程共享状态：visibility motion clock。 |
| [L93](../src/apps/controller/LoginWindow.h#L93) | `visibilityMotionStart_` | `QPoint visibilityMotionStart_;` | 保存 visibility motion start 相关配置或运行状态。 |
| [L94](../src/apps/controller/LoginWindow.h#L94) | `visibilityMotionTarget_` | `QPoint visibilityMotionTarget_;` | 保存 visibility motion target 相关配置或运行状态。 |
| [L95](../src/apps/controller/LoginWindow.h#L95) | `visibilityMotionDurationMs_` | `int visibilityMotionDurationMs_ = 0;` | 保存 visibility motion duration ms 相关配置或运行状态。 |
| [L96](../src/apps/controller/LoginWindow.h#L96) | `ownerWindow_` | `QPointer<QWidget> ownerWindow_;` | 保存 owner window 相关配置或运行状态。 |
| [L97](../src/apps/controller/LoginWindow.h#L97) | `preferredCardSize_` | `QSize preferredCardSize_{620, 520};` | 保存计数、尺寸或速率指标：preferred card size。 |
| [L98](../src/apps/controller/LoginWindow.h#L98) | `dragOffset_` | `QPoint dragOffset_;` | 保存 drag offset 相关配置或运行状态。 |
| [L99](../src/apps/controller/LoginWindow.h#L99) | `lastCardPosition_` | `QPoint lastCardPosition_;` | 保存 last card position 相关配置或运行状态。 |
| [L100](../src/apps/controller/LoginWindow.h#L100) | `dragging_` | `bool dragging_ = false;` | 保存 dragging 相关配置或运行状态。 |
| [L101](../src/apps/controller/LoginWindow.h#L101) | `hasLastCardPosition_` | `bool hasLastCardPosition_ = false;` | 保存 has last card position 相关配置或运行状态。 |
| [L102](../src/apps/controller/LoginWindow.h#L102) | `hiding_` | `bool hiding_ = false;` | 保存 hiding 相关配置或运行状态。 |
| [L103](../src/apps/controller/LoginWindow.h#L103) | `authenticatedAccountLabel_` | `QString authenticatedAccountLabel_;` | 保存路径、地址或显示名称：authenticated account label。 |
| [L104](../src/apps/controller/LoginWindow.h#L104) | `authenticatedAccountDetail_` | `QString authenticatedAccountDetail_;` | 保存 authenticated account detail 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L27](../src/apps/controller/LoginWindow.h#L27) | `LoginStatusWindow` | 声明 | `explicit LoginStatusWindow(QWidget* parent = nullptr)` | 实现 login status window 对应的业务或工具逻辑。 |
| [L28](../src/apps/controller/LoginWindow.h#L28) | `SetAccountLabel` | 声明 | `void SetAccountLabel(const QString& accountLabel)` | 更新或应用 set account label 相关逻辑。 |
| [L29](../src/apps/controller/LoginWindow.h#L29) | `ShowAndActivate` | 声明 | `void ShowAndActivate()` | 实现 show and activate 对应的业务或工具逻辑。 |
| [L30](../src/apps/controller/LoginWindow.h#L30) | `HideAndReleaseTopmost` | 声明 | `void HideAndReleaseTopmost()` | 实现 hide and release topmost 对应的业务或工具逻辑。 |
| [L31](../src/apps/controller/LoginWindow.h#L31) | `RefreshThemeStyle` | 声明 | `void RefreshThemeStyle()` | 刷新 refresh theme style 相关逻辑。 |
| [L34](../src/apps/controller/LoginWindow.h#L34) | `mousePressEvent` | 声明 | `void mousePressEvent(QMouseEvent* event) override` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L42](../src/apps/controller/LoginWindow.h#L42) | `LoginWindow` | 声明 | `explicit LoginWindow(QWidget* parent = nullptr)` | 实现 login window 对应的业务或工具逻辑。 |
| [L44](../src/apps/controller/LoginWindow.h#L44) | `SetLoginRequestedCallback` | 声明 | `void SetLoginRequestedCallback(std::function<void()> callback)` | 更新或应用 set login requested callback 相关逻辑。 |
| [L45](../src/apps/controller/LoginWindow.h#L45) | `SetCancelRequestedCallback` | 声明 | `void SetCancelRequestedCallback(std::function<void()> callback)` | 更新或应用 set cancel requested callback 相关逻辑。 |
| [L46](../src/apps/controller/LoginWindow.h#L46) | `SetExitRequestedCallback` | 声明 | `void SetExitRequestedCallback(std::function<void()> callback)` | 更新或应用 set exit requested callback 相关逻辑。 |
| [L47](../src/apps/controller/LoginWindow.h#L47) | `SetSignOutRequestedCallback` | 声明 | `void SetSignOutRequestedCallback(std::function<void()> callback)` | 更新或应用 set sign out requested callback 相关逻辑。 |
| [L48](../src/apps/controller/LoginWindow.h#L48) | `SetDeleteAccountRequestedCallback` | 声明 | `void SetDeleteAccountRequestedCallback(std::function<void()> callback)` | 更新或应用 set delete account requested callback 相关逻辑。 |
| [L49](../src/apps/controller/LoginWindow.h#L49) | `SetOwnerWindow` | 声明 | `void SetOwnerWindow(QWidget* owner)` | 更新或应用 set owner window 相关逻辑。 |
| [L52](../src/apps/controller/LoginWindow.h#L52) | `ShowBusy` | 声明 | `void ShowBusy(const QString& title, const QString& message)` | 实现 show busy 对应的业务或工具逻辑。 |
| [L53](../src/apps/controller/LoginWindow.h#L53) | `ShowAccountDeletionBusy` | 声明 | `void ShowAccountDeletionBusy()` | 实现 show account deletion busy 对应的业务或工具逻辑。 |
| [L54](../src/apps/controller/LoginWindow.h#L54) | `ShowError` | 声明 | `void ShowError(const QString& message, bool retryable = true)` | 实现 show error 对应的业务或工具逻辑。 |
| [L55](../src/apps/controller/LoginWindow.h#L55) | `ShowAuthenticated` | 声明 | `void ShowAuthenticated(const QString& accountLabel, const QString& accountDetail)` | 实现 show authenticated 对应的业务或工具逻辑。 |
| [L57](../src/apps/controller/LoginWindow.h#L57) | `ShowAndActivate` | 声明 | `void ShowAndActivate()` | 实现 show and activate 对应的业务或工具逻辑。 |
| [L58](../src/apps/controller/LoginWindow.h#L58) | `HideWithAnimation` | 声明 | `void HideWithAnimation()` | 实现 hide with animation 对应的业务或工具逻辑。 |
| [L59](../src/apps/controller/LoginWindow.h#L59) | `IsHiding` | 声明 | `bool IsHiding() const` | 判断 is hiding 相关逻辑。 |
| [L60](../src/apps/controller/LoginWindow.h#L60) | `RefreshThemeStyle` | 声明 | `void RefreshThemeStyle()` | 刷新 refresh theme style 相关逻辑。 |
| [L63](../src/apps/controller/LoginWindow.h#L63) | `closeEvent` | 声明 | `void closeEvent(QCloseEvent* event) override` | 关闭并清理 close event 相关逻辑。 |
| [L64](../src/apps/controller/LoginWindow.h#L64) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L65](../src/apps/controller/LoginWindow.h#L65) | `mousePressEvent` | 声明 | `void mousePressEvent(QMouseEvent* event) override` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L68](../src/apps/controller/LoginWindow.h#L68) | `BuildUi` | 声明 | `void BuildUi()` | 创建或初始化 build ui 相关逻辑。 |
| [L69](../src/apps/controller/LoginWindow.h#L69) | `SetStatePanelTone` | 声明 | `void SetStatePanelTone(const QString& tone)` | 更新或应用 set state panel tone 相关逻辑。 |
| [L70](../src/apps/controller/LoginWindow.h#L70) | `UpdateAuthenticatedAccountText` | 声明 | `void UpdateAuthenticatedAccountText()` | 更新或应用 update authenticated account text 相关逻辑。 |
| [L71](../src/apps/controller/LoginWindow.h#L71) | `CardTargetGeometry` | 声明 | `QRect CardTargetGeometry() const` | 实现 card target geometry 对应的业务或工具逻辑。 |
| [L72](../src/apps/controller/LoginWindow.h#L72) | `LayoutCardLayer` | 声明 | `void LayoutCardLayer()` | 实现 layout card layer 对应的业务或工具逻辑。 |
| [L73](../src/apps/controller/LoginWindow.h#L73) | `StartVisibilityMotion` | 声明 | `void StartVisibilityMotion(const QPoint& targetPosition, int durationMs)` | 启动 start visibility motion 相关逻辑。 |
| [L74](../src/apps/controller/LoginWindow.h#L74) | `StopVisibilityMotion` | 声明 | `void StopVisibilityMotion()` | 停止 stop visibility motion 相关逻辑。 |
| [L75](../src/apps/controller/LoginWindow.h#L75) | `IsVisibilityMotionActive` | 声明 | `bool IsVisibilityMotionActive() const` | 判断 is visibility motion active 相关逻辑。 |

## `src/apps/controller/MediaControls.cpp`

[打开源码](../src/apps/controller/MediaControls.cpp) · **文件作用：** 实现 media controls 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L24](../src/apps/controller/MediaControls.cpp#L24) | `MediaDeviceButton::MediaDeviceButton` | 定义 | `MediaDeviceButton::MediaDeviceButton(QWidget *parent) : QPushButton(parent), arrowAnimation_(new QVariantAnimation(this))` | 构造并初始化 MediaDeviceButton 实例。 |
| [L34](../src/apps/controller/MediaControls.cpp#L34) | `MediaDeviceButton::SetDeviceMenuHandler` | 定义 | `void MediaDeviceButton::SetDeviceMenuHandler(std::function<void()> handler)` | 更新或应用 set device menu handler 相关逻辑。 |
| [L40](../src/apps/controller/MediaControls.cpp#L40) | `MediaDeviceButton::mousePressEvent` | 定义 | `void MediaDeviceButton::mousePressEvent(QMouseEvent *event)` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L53](../src/apps/controller/MediaControls.cpp#L53) | `MediaDeviceButton::paintEvent` | 定义 | `void MediaDeviceButton::paintEvent(QPaintEvent *event)` | 准备或呈现 paint event 相关逻辑。 |
| [L75](../src/apps/controller/MediaControls.cpp#L75) | `MediaDeviceButton::SetDeviceMenuOpen` | 定义 | `void MediaDeviceButton::SetDeviceMenuOpen(bool open)` | 更新或应用 set device menu open 相关逻辑。 |
| [L90](../src/apps/controller/MediaControls.cpp#L90) | `CreateMediaStateIcon` | 定义 | `QIcon CreateMediaStateIcon(MediaStateIcon type, bool active)` | 创建或初始化 create media state icon 相关逻辑。 |
| [L133](../src/apps/controller/MediaControls.cpp#L133) | `SetMediaStateButton` | 定义 | `void SetMediaStateButton(QPushButton *button, MediaStateIcon type, bool active, const QString &toolTip)` | 更新或应用 set media state button 相关逻辑。 |
| [L181](../src/apps/controller/MediaControls.cpp#L181) | `SetCameraGalleryStateButton` | 定义 | `void SetCameraGalleryStateButton(QPushButton *button, bool camerasAvailable, bool galleryVisible, const QString &toolTip)` | 更新或应用 set camera gallery state button 相关逻辑。 |
| [L205](../src/apps/controller/MediaControls.cpp#L205) | `CreateMemberMediaIndicator` | 定义 | `QToolButton *CreateMemberMediaIndicator(QWidget *parent, MediaStateIcon type, bool active, const QString &toolTip, bool hadPreviousState, bool previousActive)` | 创建或初始化 create member media indicator 相关逻辑。 |

## `src/apps/controller/MediaControls.h`

[打开源码](../src/apps/controller/MediaControls.h) · **文件作用：** 声明 media controls 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L12](../src/apps/controller/MediaControls.h#L12) | `QMouseEvent` | class | 定义 QMouseEvent 的 class 类型和相关状态。 |
| [L13](../src/apps/controller/MediaControls.h#L13) | `QPaintEvent` | class | 定义 QPaintEvent 的 class 类型和相关状态。 |
| [L14](../src/apps/controller/MediaControls.h#L14) | `QToolButton` | class | 定义 QToolButton 的 class 类型和相关状态。 |
| [L15](../src/apps/controller/MediaControls.h#L15) | `QVariantAnimation` | class | 定义 QVariantAnimation 的 class 类型和相关状态。 |
| [L19](../src/apps/controller/MediaControls.h#L19) | `MediaStateIcon` | enum class | 定义 MediaStateIcon 的 enum class 类型和相关状态。 |
| [L21](../src/apps/controller/MediaControls.h#L21) | `MediaDeviceButton` | class | 定义 MediaDeviceButton 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L12](../src/apps/controller/MediaControls.h#L12) | `QMouseEvent` | `class QMouseEvent;` | 保存 q mouse event 相关配置或运行状态。 |
| [L13](../src/apps/controller/MediaControls.h#L13) | `QPaintEvent` | `class QPaintEvent;` | 保存 q paint event 相关配置或运行状态。 |
| [L14](../src/apps/controller/MediaControls.h#L14) | `QToolButton` | `class QToolButton;` | 保存 q tool button 相关配置或运行状态。 |
| [L15](../src/apps/controller/MediaControls.h#L15) | `QVariantAnimation` | `class QVariantAnimation;` | 保存 q variant animation 相关配置或运行状态。 |
| [L19](../src/apps/controller/MediaControls.h#L19) | `MediaStateIcon` | `enum class MediaStateIcon { kCamera, kMicrophone, kSpeaker, kScreen };` | 保存 media state icon 相关配置或运行状态。 |
| [L34](../src/apps/controller/MediaControls.h#L34) | `kMenuAreaWidth` | `static constexpr int kMenuAreaWidth = 30;` | 定义 menu area width 的编译期常量或产品边界。 |
| [L36](../src/apps/controller/MediaControls.h#L36) | `arrowAnimation_` | `QVariantAnimation *arrowAnimation_ = nullptr;` | 保存 arrow animation 相关配置或运行状态。 |
| [L37](../src/apps/controller/MediaControls.h#L37) | `arrowRotation_` | `qreal arrowRotation_ = 0.0;` | 保存 arrow rotation 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L23](../src/apps/controller/MediaControls.h#L23) | `MediaDeviceButton` | 声明 | `explicit MediaDeviceButton(QWidget *parent = nullptr)` | 实现 media device button 对应的业务或工具逻辑。 |
| [L25](../src/apps/controller/MediaControls.h#L25) | `SetDeviceMenuHandler` | 声明 | `void SetDeviceMenuHandler(std::function<void()> handler)` | 更新或应用 set device menu handler 相关逻辑。 |
| [L28](../src/apps/controller/MediaControls.h#L28) | `mousePressEvent` | 声明 | `void mousePressEvent(QMouseEvent *event) override` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L29](../src/apps/controller/MediaControls.h#L29) | `paintEvent` | 声明 | `void paintEvent(QPaintEvent *event) override` | 准备或呈现 paint event 相关逻辑。 |
| [L32](../src/apps/controller/MediaControls.h#L32) | `SetDeviceMenuOpen` | 声明 | `void SetDeviceMenuOpen(bool open)` | 更新或应用 set device menu open 相关逻辑。 |
| [L40](../src/apps/controller/MediaControls.h#L40) | `CreateMediaStateIcon` | 声明 | `QIcon CreateMediaStateIcon(MediaStateIcon type, bool active)` | 创建或初始化 create media state icon 相关逻辑。 |
| [L41](../src/apps/controller/MediaControls.h#L41) | `SetMediaStateButton` | 声明 | `void SetMediaStateButton(QPushButton *button, MediaStateIcon type, bool active, const QString &toolTip)` | 更新或应用 set media state button 相关逻辑。 |
| [L43](../src/apps/controller/MediaControls.h#L43) | `SetCameraGalleryStateButton` | 声明 | `void SetCameraGalleryStateButton(QPushButton *button, bool camerasAvailable, bool galleryVisible, const QString &toolTip)` | 更新或应用 set camera gallery state button 相关逻辑。 |
| [L45](../src/apps/controller/MediaControls.h#L45) | `CreateMemberMediaIndicator` | 声明 | `QToolButton *CreateMemberMediaIndicator(QWidget *parent, MediaStateIcon type, bool active, const QString &toolTip, bool hadPreviousState = false, bool previousActive = false)` | 创建或初始化 create member media indicator 相关逻辑。 |

## `src/apps/controller/MorphIconToolButton.cpp`

[打开源码](../src/apps/controller/MorphIconToolButton.cpp) · **文件作用：** 实现 morph icon tool button 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L18](../src/apps/controller/MorphIconToolButton.cpp#L18) | `MorphIconToolButton::MorphIconToolButton` | 定义 | `MorphIconToolButton::MorphIconToolButton( const QString& sourceResource, const QString& targetResource, QWidget* parent) : QToolButton(parent), sourceResource_(sourceResource), targetResource_(targetResource)` | 构造并初始化 MorphIconToolButton 实例。 |
| [L51](../src/apps/controller/MorphIconToolButton.cpp#L51) | `MorphIconToolButton::SetTarget` | 定义 | `void MorphIconToolButton::SetTarget(bool target)` | 更新或应用 set target 相关逻辑。 |
| [L73](../src/apps/controller/MorphIconToolButton.cpp#L73) | `MorphIconToolButton::paintEvent` | 定义 | `void MorphIconToolButton::paintEvent(QPaintEvent* event)` | 准备或呈现 paint event 相关逻辑。 |

## `src/apps/controller/MorphIconToolButton.h`

[打开源码](../src/apps/controller/MorphIconToolButton.h) · **文件作用：** 声明 morph icon tool button 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L13](../src/apps/controller/MorphIconToolButton.h#L13) | `QPaintEvent` | class | 定义 QPaintEvent 的 class 类型和相关状态。 |
| [L14](../src/apps/controller/MorphIconToolButton.h#L14) | `QWidget` | class | 定义 QWidget 的 class 类型和相关状态。 |
| [L18](../src/apps/controller/MorphIconToolButton.h#L18) | `MorphIconToolButton` | class | 定义 MorphIconToolButton 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L13](../src/apps/controller/MorphIconToolButton.h#L13) | `QPaintEvent` | `class QPaintEvent;` | 保存 q paint event 相关配置或运行状态。 |
| [L14](../src/apps/controller/MorphIconToolButton.h#L14) | `QWidget` | `class QWidget;` | 保存 q widget 相关配置或运行状态。 |
| [L31](../src/apps/controller/MorphIconToolButton.h#L31) | `sourceResource_` | `QString sourceResource_;` | 保存 source resource 相关配置或运行状态。 |
| [L32](../src/apps/controller/MorphIconToolButton.h#L32) | `targetResource_` | `QString targetResource_;` | 保存 target resource 相关配置或运行状态。 |
| [L33](../src/apps/controller/MorphIconToolButton.h#L33) | `morphIcon_` | `remotec::ui::morph::MorphIconCore morphIcon_;` | 保存 morph icon 相关配置或运行状态。 |
| [L34](../src/apps/controller/MorphIconToolButton.h#L34) | `spring_` | `remotec::ui::morph::Spring spring_;` | 保存 spring 相关配置或运行状态。 |
| [L35](../src/apps/controller/MorphIconToolButton.h#L35) | `timer_` | `QTimer timer_;` | 保存定时、截止或超时状态：timer。 |
| [L36](../src/apps/controller/MorphIconToolButton.h#L36) | `elapsed_` | `QElapsedTimer elapsed_;` | 保存 elapsed 相关配置或运行状态。 |
| [L37](../src/apps/controller/MorphIconToolButton.h#L37) | `progress_` | `double progress_ = 0.0;` | 保存 progress 相关配置或运行状态。 |
| [L38](../src/apps/controller/MorphIconToolButton.h#L38) | `start_` | `double start_ = 0.0;` | 保存 start 相关配置或运行状态。 |
| [L39](../src/apps/controller/MorphIconToolButton.h#L39) | `end_` | `double end_ = 0.0;` | 保存 end 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L20](../src/apps/controller/MorphIconToolButton.h#L20) | `MorphIconToolButton` | 声明 | `MorphIconToolButton( const QString& sourceResource, const QString& targetResource, QWidget* parent = nullptr)` | 实现 morph icon tool button 对应的业务或工具逻辑。 |
| [L25](../src/apps/controller/MorphIconToolButton.h#L25) | `SetTarget` | 声明 | `void SetTarget(bool target)` | 更新或应用 set target 相关逻辑。 |
| [L28](../src/apps/controller/MorphIconToolButton.h#L28) | `paintEvent` | 声明 | `void paintEvent(QPaintEvent* event) override` | 准备或呈现 paint event 相关逻辑。 |

## `src/apps/controller/pages/DiagnosticsCardsWidget.cpp`

[打开源码](../src/apps/controller/pages/DiagnosticsCardsWidget.cpp) · **文件作用：** 实现 diagnostics cards widget 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L23](../src/apps/controller/pages/DiagnosticsCardsWidget.cpp#L23) | `DiagnosticsMetricExplanation` | 定义 | `QString DiagnosticsMetricExplanation(const QString &label)` | 查询并返回 diagnostics metric explanation 相关逻辑。 |
| [L272](../src/apps/controller/pages/DiagnosticsCardsWidget.cpp#L272) | `DiagnosticsMetricToolTip` | 定义 | `QString DiagnosticsMetricToolTip(const DiagnosticsChip &chip)` | 查询并返回 diagnostics metric tool tip 相关逻辑。 |
| [L279](../src/apps/controller/pages/DiagnosticsCardsWidget.cpp#L279) | `DiagnosticsCardsWidget::DiagnosticsCardsWidget` | 定义 | `DiagnosticsCardsWidget::DiagnosticsCardsWidget(QWidget *parent) : QWidget(parent), layout_(new QVBoxLayout(this))` | 构造并初始化 DiagnosticsCardsWidget 实例。 |
| [L293](../src/apps/controller/pages/DiagnosticsCardsWidget.cpp#L293) | `DiagnosticsCardsWidget::SetSections` | 定义 | `void DiagnosticsCardsWidget::SetSections( const QVector<DiagnosticsSection> &sections, const QString &emptyText)` | 更新或应用 set sections 相关逻辑。 |
| [L354](../src/apps/controller/pages/DiagnosticsCardsWidget.cpp#L354) | `DiagnosticsCardsWidget::Rebuild` | 定义 | `void DiagnosticsCardsWidget::Rebuild( const QVector<DiagnosticsSection> &sections, const QString &emptyText)` | 更新或应用 rebuild 相关逻辑。 |

## `src/apps/controller/pages/DiagnosticsCardsWidget.h`

[打开源码](../src/apps/controller/pages/DiagnosticsCardsWidget.h) · **文件作用：** 声明 diagnostics cards widget 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L12](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L12) | `QFrame` | class | 定义 QFrame 的 class 类型和相关状态。 |
| [L13](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L13) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L14](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L14) | `QToolButton` | class | 定义 QToolButton 的 class 类型和相关状态。 |
| [L15](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L15) | `QVBoxLayout` | class | 定义 QVBoxLayout 的 class 类型和相关状态。 |
| [L19](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L19) | `DiagnosticsChip` | struct | 定义 DiagnosticsChip 的 struct 类型和相关状态。 |
| [L27](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L27) | `DiagnosticsCard` | struct | 定义 DiagnosticsCard 的 struct 类型和相关状态。 |
| [L34](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L34) | `DiagnosticsSection` | struct | 定义 DiagnosticsSection 的 struct 类型和相关状态。 |
| [L41](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L41) | `DiagnosticsCardsWidget` | class | 定义 DiagnosticsCardsWidget 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L12](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L12) | `QFrame` | `class QFrame;` | 保存媒体帧、图像或缓冲资源：q frame。 |
| [L13](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L13) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L14](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L14) | `QToolButton` | `class QToolButton;` | 保存 q tool button 相关配置或运行状态。 |
| [L15](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L15) | `QVBoxLayout` | `class QVBoxLayout;` | 保存 qv box layout 相关配置或运行状态。 |
| [L20](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L20) | `key` | `QString key;` | 保存 key 相关配置或运行状态。 |
| [L21](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L21) | `label` | `QString label;` | 保存路径、地址或显示名称：label。 |
| [L22](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L22) | `value` | `QString value;` | 保存 value 相关配置或运行状态。 |
| [L23](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L23) | `tone` | `QByteArray tone = "normal";` | 保存 tone 相关配置或运行状态。 |
| [L24](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L24) | `wide` | `bool wide = false;` | 保存 wide 相关配置或运行状态。 |
| [L28](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L28) | `key` | `QString key;` | 保存 key 相关配置或运行状态。 |
| [L29](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L29) | `title` | `QString title;` | 保存 title 相关配置或运行状态。 |
| [L30](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L30) | `subtitle` | `QString subtitle;` | 保存 subtitle 相关配置或运行状态。 |
| [L31](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L31) | `chips` | `QVector<DiagnosticsChip> chips;` | 保存 chips 相关配置或运行状态。 |
| [L35](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L35) | `key` | `QString key;` | 保存 key 相关配置或运行状态。 |
| [L36](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L36) | `title` | `QString title;` | 保存 title 相关配置或运行状态。 |
| [L37](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L37) | `description` | `QString description;` | 保存 description 相关配置或运行状态。 |
| [L38](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L38) | `cards` | `QVector<DiagnosticsCard> cards;` | 保存 cards 相关配置或运行状态。 |
| [L52](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L52) | `layout_` | `QVBoxLayout *layout_ = nullptr;` | 保存 layout 相关配置或运行状态。 |
| [L53](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L53) | `structure_` | `QString structure_;` | 保存 structure 相关配置或运行状态。 |
| [L54](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L54) | `textLabels_` | `QHash<QString, QLabel *> textLabels_;` | 保存 text labels 相关配置或运行状态。 |
| [L55](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L55) | `chipNameLabels_` | `QHash<QString, QLabel *> chipNameLabels_;` | 保存 chip name labels 相关配置或运行状态。 |
| [L56](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L56) | `chipFrames_` | `QHash<QString, QFrame *> chipFrames_;` | 保存 chip frames 相关配置或运行状态。 |
| [L57](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L57) | `titleButtons_` | `QHash<QString, QToolButton *> titleButtons_;` | 保存 title buttons 相关配置或运行状态。 |
| [L58](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L58) | `cardCopyTexts_` | `QHash<QString, QString> cardCopyTexts_;` | 保存 card copy texts 相关配置或运行状态。 |
| [L59](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L59) | `sectionExpanded_` | `QHash<QString, bool> sectionExpanded_;` | 保存 section expanded 相关配置或运行状态。 |
| [L60](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L60) | `cardExpanded_` | `QHash<QString, bool> cardExpanded_;` | 保存 card expanded 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L43](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L43) | `DiagnosticsCardsWidget` | 声明 | `explicit DiagnosticsCardsWidget(QWidget *parent = nullptr)` | 查询并返回 diagnostics cards widget 相关逻辑。 |
| [L45](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L45) | `SetSections` | 声明 | `void SetSections(const QVector<DiagnosticsSection> &sections, const QString &emptyText)` | 更新或应用 set sections 相关逻辑。 |
| [L49](../src/apps/controller/pages/DiagnosticsCardsWidget.h#L49) | `Rebuild` | 声明 | `void Rebuild(const QVector<DiagnosticsSection> &sections, const QString &emptyText)` | 更新或应用 rebuild 相关逻辑。 |

## `src/apps/controller/pages/DiagnosticsPage.cpp`

[打开源码](../src/apps/controller/pages/DiagnosticsPage.cpp) · **文件作用：** 实现 diagnostics page 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L18](../src/apps/controller/pages/DiagnosticsPage.cpp#L18) | `DiagnosticsPage::DiagnosticsPage` | 定义 | `DiagnosticsPage::DiagnosticsPage(QWidget* parent) : QScrollArea(parent)` | 构造并初始化 DiagnosticsPage 实例。 |
| [L47](../src/apps/controller/pages/DiagnosticsPage.cpp#L47) | `DiagnosticsPage::AddDetailPage` | 定义 | `QVBoxLayout* DiagnosticsPage::AddDetailPage( QStackedWidget* stack, const QString& title, const QString& description, QPushButton** copyButton)` | 实现 add detail page 对应的业务或工具逻辑。 |
| [L80](../src/apps/controller/pages/DiagnosticsPage.cpp#L80) | `DiagnosticsPage::AddValue` | 定义 | `void DiagnosticsPage::AddValue(QVBoxLayout* layout, const QString& key, const QString& title, bool expanded)` | 实现 add value 对应的业务或工具逻辑。 |
| [L107](../src/apps/controller/pages/DiagnosticsPage.cpp#L107) | `DiagnosticsPage::RegisterValue` | 定义 | `void DiagnosticsPage::RegisterValue(const QString& key, QLabel* label)` | 实现 register value 对应的业务或工具逻辑。 |
| [L112](../src/apps/controller/pages/DiagnosticsPage.cpp#L112) | `DiagnosticsPage::ValueLabel` | 定义 | `QLabel* DiagnosticsPage::ValueLabel(const QString& key) const` | 实现 value label 对应的业务或工具逻辑。 |
| [L117](../src/apps/controller/pages/DiagnosticsPage.cpp#L117) | `DiagnosticsPage::HasValues` | 定义 | `bool DiagnosticsPage::HasValues() const` | 判断 has values 相关逻辑。 |
| [L122](../src/apps/controller/pages/DiagnosticsPage.cpp#L122) | `DiagnosticsPage::ScreenFrameRateLogEnabled` | 定义 | `bool DiagnosticsPage::ScreenFrameRateLogEnabled() const` | 实现 screen frame rate log enabled 对应的业务或工具逻辑。 |
| [L127](../src/apps/controller/pages/DiagnosticsPage.cpp#L127) | `DiagnosticsPage::InputEventStatsEnabled` | 定义 | `bool DiagnosticsPage::InputEventStatsEnabled() const` | 实现 input event stats enabled 对应的业务或工具逻辑。 |
| [L132](../src/apps/controller/pages/DiagnosticsPage.cpp#L132) | `DiagnosticsPage::StatsCardsWidget` | 定义 | `QWidget* DiagnosticsPage::StatsCardsWidget() const` | 实现 stats cards widget 对应的业务或工具逻辑。 |
| [L137](../src/apps/controller/pages/DiagnosticsPage.cpp#L137) | `DiagnosticsPage::CopyAllButton` | 定义 | `QPushButton* DiagnosticsPage::CopyAllButton() const` | 实现 copy all button 对应的业务或工具逻辑。 |
| [L142](../src/apps/controller/pages/DiagnosticsPage.cpp#L142) | `DiagnosticsPage::CopyMediaButton` | 定义 | `QPushButton* DiagnosticsPage::CopyMediaButton() const` | 实现 copy media button 对应的业务或工具逻辑。 |

## `src/apps/controller/pages/DiagnosticsPage.h`

[打开源码](../src/apps/controller/pages/DiagnosticsPage.h) · **文件作用：** 声明 diagnostics page 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L10](../src/apps/controller/pages/DiagnosticsPage.h#L10) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L11](../src/apps/controller/pages/DiagnosticsPage.h#L11) | `QPushButton` | class | 定义 QPushButton 的 class 类型和相关状态。 |
| [L12](../src/apps/controller/pages/DiagnosticsPage.h#L12) | `QStackedWidget` | class | 定义 QStackedWidget 的 class 类型和相关状态。 |
| [L13](../src/apps/controller/pages/DiagnosticsPage.h#L13) | `QVBoxLayout` | class | 定义 QVBoxLayout 的 class 类型和相关状态。 |
| [L17](../src/apps/controller/pages/DiagnosticsPage.h#L17) | `DiagnosticsPage` | class | 定义 DiagnosticsPage 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L10](../src/apps/controller/pages/DiagnosticsPage.h#L10) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L11](../src/apps/controller/pages/DiagnosticsPage.h#L11) | `QPushButton` | `class QPushButton;` | 保存 q push button 相关配置或运行状态。 |
| [L12](../src/apps/controller/pages/DiagnosticsPage.h#L12) | `QStackedWidget` | `class QStackedWidget;` | 保存 q stacked widget 相关配置或运行状态。 |
| [L13](../src/apps/controller/pages/DiagnosticsPage.h#L13) | `QVBoxLayout` | `class QVBoxLayout;` | 保存 qv box layout 相关配置或运行状态。 |
| [L48](../src/apps/controller/pages/DiagnosticsPage.h#L48) | `contentLayout_` | `QVBoxLayout* contentLayout_ = nullptr;` | 保存 content layout 相关配置或运行状态。 |
| [L49](../src/apps/controller/pages/DiagnosticsPage.h#L49) | `valueLabels_` | `QHash<QString, QLabel*> valueLabels_;` | 保存 value labels 相关配置或运行状态。 |
| [L50](../src/apps/controller/pages/DiagnosticsPage.h#L50) | `statsCardsWidget_` | `QWidget* statsCardsWidget_ = nullptr;` | 保存 stats cards widget 相关配置或运行状态。 |
| [L51](../src/apps/controller/pages/DiagnosticsPage.h#L51) | `copyAllButton_` | `QPushButton* copyAllButton_ = nullptr;` | 保存 copy all button 相关配置或运行状态。 |
| [L52](../src/apps/controller/pages/DiagnosticsPage.h#L52) | `copyMediaButton_` | `QPushButton* copyMediaButton_ = nullptr;` | 保存 copy media button 相关配置或运行状态。 |
| [L53](../src/apps/controller/pages/DiagnosticsPage.h#L53) | `screenFrameRateLogEnabled_` | `bool screenFrameRateLogEnabled_ = false;` | 保存能力或开关状态：screen frame rate log enabled。 |
| [L54](../src/apps/controller/pages/DiagnosticsPage.h#L54) | `inputEventStatsEnabled_` | `bool inputEventStatsEnabled_ = false;` | 保存能力或开关状态：input event stats enabled。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L21](../src/apps/controller/pages/DiagnosticsPage.h#L21) | `DiagnosticsPage` | 声明 | `explicit DiagnosticsPage(QWidget* parent = nullptr)` | 查询并返回 diagnostics page 相关逻辑。 |
| [L23](../src/apps/controller/pages/DiagnosticsPage.h#L23) | `AddDetailPage` | 声明 | `QVBoxLayout* AddDetailPage(QStackedWidget* stack, const QString& title, const QString& description, QPushButton** copyButton = nullptr)` | 实现 add detail page 对应的业务或工具逻辑。 |
| [L27](../src/apps/controller/pages/DiagnosticsPage.h#L27) | `AddValue` | 声明 | `void AddValue(QVBoxLayout* layout, const QString& key, const QString& title, bool expanded = false)` | 实现 add value 对应的业务或工具逻辑。 |
| [L31](../src/apps/controller/pages/DiagnosticsPage.h#L31) | `RegisterValue` | 声明 | `void RegisterValue(const QString& key, QLabel* label)` | 实现 register value 对应的业务或工具逻辑。 |
| [L32](../src/apps/controller/pages/DiagnosticsPage.h#L32) | `ValueLabel` | 声明 | `QLabel* ValueLabel(const QString& key) const` | 实现 value label 对应的业务或工具逻辑。 |
| [L33](../src/apps/controller/pages/DiagnosticsPage.h#L33) | `HasValues` | 声明 | `bool HasValues() const` | 判断 has values 相关逻辑。 |
| [L34](../src/apps/controller/pages/DiagnosticsPage.h#L34) | `ScreenFrameRateLogEnabled` | 声明 | `bool ScreenFrameRateLogEnabled() const` | 实现 screen frame rate log enabled 对应的业务或工具逻辑。 |
| [L35](../src/apps/controller/pages/DiagnosticsPage.h#L35) | `InputEventStatsEnabled` | 声明 | `bool InputEventStatsEnabled() const` | 实现 input event stats enabled 对应的业务或工具逻辑。 |
| [L36](../src/apps/controller/pages/DiagnosticsPage.h#L36) | `StatsCardsWidget` | 声明 | `QWidget* StatsCardsWidget() const` | 实现 stats cards widget 对应的业务或工具逻辑。 |
| [L37](../src/apps/controller/pages/DiagnosticsPage.h#L37) | `CopyAllButton` | 声明 | `QPushButton* CopyAllButton() const` | 实现 copy all button 对应的业务或工具逻辑。 |
| [L38](../src/apps/controller/pages/DiagnosticsPage.h#L38) | `CopyMediaButton` | 声明 | `QPushButton* CopyMediaButton() const` | 实现 copy media button 对应的业务或工具逻辑。 |
| [L41](../src/apps/controller/pages/DiagnosticsPage.h#L41) | `ScreenFrameRateLogToggled` | 声明 | `void ScreenFrameRateLogToggled(bool enabled)` | 实现 screen frame rate log toggled 对应的业务或工具逻辑。 |
| [L42](../src/apps/controller/pages/DiagnosticsPage.h#L42) | `InputEventStatsToggled` | 声明 | `void InputEventStatsToggled(bool enabled)` | 实现 input event stats toggled 对应的业务或工具逻辑。 |
| [L43](../src/apps/controller/pages/DiagnosticsPage.h#L43) | `RefreshRequested` | 声明 | `void RefreshRequested()` | 刷新 refresh requested 相关逻辑。 |
| [L46](../src/apps/controller/pages/DiagnosticsPage.h#L46) | `BuildWorkspace` | 声明 | `void BuildWorkspace()` | 创建或初始化 build workspace 相关逻辑。 |

## `src/apps/controller/pages/DiagnosticsPage.Workspace.cpp`

[打开源码](../src/apps/controller/pages/DiagnosticsPage.Workspace.cpp) · **文件作用：** 实现 diagnostics page workspace 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L23](../src/apps/controller/pages/DiagnosticsPage.Workspace.cpp#L23) | `DiagnosticsPage::BuildWorkspace` | 定义 | `void DiagnosticsPage::BuildWorkspace()` | 创建或初始化 build workspace 相关逻辑。 |

## `src/apps/controller/pages/DirectConnectPage.cpp`

[打开源码](../src/apps/controller/pages/DirectConnectPage.cpp) · **文件作用：** 实现 direct connect page 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L21](../src/apps/controller/pages/DirectConnectPage.cpp#L21) | `DirectConnectPage::DirectConnectPage` | 定义 | `DirectConnectPage::DirectConnectPage(QWidget* parent) : QScrollArea(parent)` | 构造并初始化 DirectConnectPage 实例。 |
| [L58](../src/apps/controller/pages/DirectConnectPage.cpp#L58) | `DirectConnectPage::BuildLoginPrompt` | 定义 | `void DirectConnectPage::BuildLoginPrompt()` | 创建或初始化 build login prompt 相关逻辑。 |
| [L168](../src/apps/controller/pages/DirectConnectPage.cpp#L168) | `DirectConnectPage::DeviceId` | 定义 | `QString DirectConnectPage::DeviceId() const` | 实现 device id 对应的业务或工具逻辑。 |
| [L173](../src/apps/controller/pages/DirectConnectPage.cpp#L173) | `DirectConnectPage::VerificationCode` | 定义 | `QString DirectConnectPage::VerificationCode() const` | 实现 verification code 对应的业务或工具逻辑。 |
| [L180](../src/apps/controller/pages/DirectConnectPage.cpp#L180) | `DirectConnectPage::SetActionEnabled` | 定义 | `void DirectConnectPage::SetActionEnabled(bool enabled)` | 更新或应用 set action enabled 相关逻辑。 |
| [L187](../src/apps/controller/pages/DirectConnectPage.cpp#L187) | `DirectConnectPage::SetActionState` | 定义 | `void DirectConnectPage::SetActionState(bool enabled, const QString& text)` | 更新或应用 set action state 相关逻辑。 |
| [L196](../src/apps/controller/pages/DirectConnectPage.cpp#L196) | `DirectConnectPage::PrepareCredentials` | 定义 | `void DirectConnectPage::PrepareCredentials(const QString& deviceId)` | 实现 prepare credentials 对应的业务或工具逻辑。 |
| [L206](../src/apps/controller/pages/DirectConnectPage.cpp#L206) | `DirectConnectPage::ShowSignedOut` | 定义 | `void DirectConnectPage::ShowSignedOut(const QString& message)` | 实现 show signed out 对应的业务或工具逻辑。 |
| [L217](../src/apps/controller/pages/DirectConnectPage.cpp#L217) | `DirectConnectPage::ShowLoginBusy` | 定义 | `void DirectConnectPage::ShowLoginBusy(const QString& message)` | 实现 show login busy 对应的业务或工具逻辑。 |
| [L225](../src/apps/controller/pages/DirectConnectPage.cpp#L225) | `DirectConnectPage::SetAuthenticated` | 定义 | `void DirectConnectPage::SetAuthenticated(bool authenticated)` | 更新或应用 set authenticated 相关逻辑。 |
| [L231](../src/apps/controller/pages/DirectConnectPage.cpp#L231) | `DirectConnectPage::SetAccountLabel` | 定义 | `void DirectConnectPage::SetAccountLabel(const QString& label)` | 更新或应用 set account label 相关逻辑。 |
| [L237](../src/apps/controller/pages/DirectConnectPage.cpp#L237) | `DirectConnectPage::SetLocalCredentials` | 定义 | `void DirectConnectPage::SetLocalCredentials( const QString& deviceId, const QString& verificationCode)` | 更新或应用 set local credentials 相关逻辑。 |
| [L251](../src/apps/controller/pages/DirectConnectPage.cpp#L251) | `DirectConnectPage::SetSignalStatus` | 定义 | `void DirectConnectPage::SetSignalStatus( const QString& text, const QString& styleSheet)` | 更新或应用 set signal status 相关逻辑。 |
| [L259](../src/apps/controller/pages/DirectConnectPage.cpp#L259) | `DirectConnectPage::SetRuntimeStatus` | 定义 | `void DirectConnectPage::SetRuntimeStatus( const QString& text, const QString& color)` | 更新或应用 set runtime status 相关逻辑。 |
| [L267](../src/apps/controller/pages/DirectConnectPage.cpp#L267) | `DirectConnectPage::SetDecoderStatus` | 定义 | `void DirectConnectPage::SetDecoderStatus( const QString& text, const QString& color)` | 更新或应用 set decoder status 相关逻辑。 |
| [L275](../src/apps/controller/pages/DirectConnectPage.cpp#L275) | `DirectConnectPage::SetAssistHint` | 定义 | `void DirectConnectPage::SetAssistHint( const QString& text, AssistHintTone tone)` | 更新或应用 set assist hint 相关逻辑。 |
| [L297](../src/apps/controller/pages/DirectConnectPage.cpp#L297) | `DirectConnectPage::HasAssistError` | 定义 | `bool DirectConnectPage::HasAssistError() const` | 判断 has assist error 相关逻辑。 |

## `src/apps/controller/pages/DirectConnectPage.h`

[打开源码](../src/apps/controller/pages/DirectConnectPage.h) · **文件作用：** 声明 direct connect page 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L9](../src/apps/controller/pages/DirectConnectPage.h#L9) | `QVBoxLayout` | class | 定义 QVBoxLayout 的 class 类型和相关状态。 |
| [L10](../src/apps/controller/pages/DirectConnectPage.h#L10) | `QLineEdit` | class | 定义 QLineEdit 的 class 类型和相关状态。 |
| [L11](../src/apps/controller/pages/DirectConnectPage.h#L11) | `QPushButton` | class | 定义 QPushButton 的 class 类型和相关状态。 |
| [L12](../src/apps/controller/pages/DirectConnectPage.h#L12) | `QFrame` | class | 定义 QFrame 的 class 类型和相关状态。 |
| [L13](../src/apps/controller/pages/DirectConnectPage.h#L13) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L17](../src/apps/controller/pages/DirectConnectPage.h#L17) | `DirectConnectPage` | class | 定义 DirectConnectPage 的 class 类型和相关状态。 |
| [L21](../src/apps/controller/pages/DirectConnectPage.h#L21) | `AssistHintTone` | enum class | 定义 AssistHintTone 的 enum class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L9](../src/apps/controller/pages/DirectConnectPage.h#L9) | `QVBoxLayout` | `class QVBoxLayout;` | 保存 qv box layout 相关配置或运行状态。 |
| [L10](../src/apps/controller/pages/DirectConnectPage.h#L10) | `QLineEdit` | `class QLineEdit;` | 保存 q line edit 相关配置或运行状态。 |
| [L11](../src/apps/controller/pages/DirectConnectPage.h#L11) | `QPushButton` | `class QPushButton;` | 保存 q push button 相关配置或运行状态。 |
| [L12](../src/apps/controller/pages/DirectConnectPage.h#L12) | `QFrame` | `class QFrame;` | 保存媒体帧、图像或缓冲资源：q frame。 |
| [L13](../src/apps/controller/pages/DirectConnectPage.h#L13) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L60](../src/apps/controller/pages/DirectConnectPage.h#L60) | `contentLayout_` | `QVBoxLayout* contentLayout_ = nullptr;` | 保存 content layout 相关配置或运行状态。 |
| [L61](../src/apps/controller/pages/DirectConnectPage.h#L61) | `loginPrompt_` | `QFrame* loginPrompt_ = nullptr;` | 保存 login prompt 相关配置或运行状态。 |
| [L62](../src/apps/controller/pages/DirectConnectPage.h#L62) | `loginStatus_` | `QLabel* loginStatus_ = nullptr;` | 保存状态机当前状态：login status。 |
| [L63](../src/apps/controller/pages/DirectConnectPage.h#L63) | `loginButton_` | `QPushButton* loginButton_ = nullptr;` | 保存 login button 相关配置或运行状态。 |
| [L64](../src/apps/controller/pages/DirectConnectPage.h#L64) | `signedInWorkspace_` | `QFrame* signedInWorkspace_ = nullptr;` | 保存 signed in workspace 相关配置或运行状态。 |
| [L65](../src/apps/controller/pages/DirectConnectPage.h#L65) | `accountLabel_` | `QLabel* accountLabel_ = nullptr;` | 保存路径、地址或显示名称：account label。 |
| [L66](../src/apps/controller/pages/DirectConnectPage.h#L66) | `localDeviceId_` | `QLabel* localDeviceId_ = nullptr;` | 保存身份或作用域标识：local device id。 |
| [L67](../src/apps/controller/pages/DirectConnectPage.h#L67) | `localVerificationCode_` | `QLabel* localVerificationCode_ = nullptr;` | 保存 local verification code 相关配置或运行状态。 |
| [L68](../src/apps/controller/pages/DirectConnectPage.h#L68) | `copyDeviceIdButton_` | `QPushButton* copyDeviceIdButton_ = nullptr;` | 保存 copy device id button 相关配置或运行状态。 |
| [L69](../src/apps/controller/pages/DirectConnectPage.h#L69) | `copyVerificationCodeButton_` | `QPushButton* copyVerificationCodeButton_ = nullptr;` | 保存 copy verification code button 相关配置或运行状态。 |
| [L70](../src/apps/controller/pages/DirectConnectPage.h#L70) | `shareLocalCredentialsButton_` | `QPushButton* shareLocalCredentialsButton_ = nullptr;` | 保存 share local credentials button 相关配置或运行状态。 |
| [L71](../src/apps/controller/pages/DirectConnectPage.h#L71) | `signalStatus_` | `QLabel* signalStatus_ = nullptr;` | 保存状态机当前状态：signal status。 |
| [L72](../src/apps/controller/pages/DirectConnectPage.h#L72) | `runtimeStatus_` | `QLabel* runtimeStatus_ = nullptr;` | 保存状态机当前状态：runtime status。 |
| [L73](../src/apps/controller/pages/DirectConnectPage.h#L73) | `decoderStatus_` | `QLabel* decoderStatus_ = nullptr;` | 保存状态机当前状态：decoder status。 |
| [L74](../src/apps/controller/pages/DirectConnectPage.h#L74) | `assistHint_` | `QLabel* assistHint_ = nullptr;` | 保存 assist hint 相关配置或运行状态。 |
| [L75](../src/apps/controller/pages/DirectConnectPage.h#L75) | `deviceIdEdit_` | `QLineEdit* deviceIdEdit_ = nullptr;` | 保存 device id edit 相关配置或运行状态。 |
| [L76](../src/apps/controller/pages/DirectConnectPage.h#L76) | `verificationCodeEdit_` | `QLineEdit* verificationCodeEdit_ = nullptr;` | 保存 verification code edit 相关配置或运行状态。 |
| [L77](../src/apps/controller/pages/DirectConnectPage.h#L77) | `connectButton_` | `QPushButton* connectButton_ = nullptr;` | 保存 connect button 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L28](../src/apps/controller/pages/DirectConnectPage.h#L28) | `DirectConnectPage` | 声明 | `explicit DirectConnectPage(QWidget* parent = nullptr)` | 实现 direct connect page 对应的业务或工具逻辑。 |
| [L30](../src/apps/controller/pages/DirectConnectPage.h#L30) | `DeviceId` | 声明 | `QString DeviceId() const` | 实现 device id 对应的业务或工具逻辑。 |
| [L31](../src/apps/controller/pages/DirectConnectPage.h#L31) | `VerificationCode` | 声明 | `QString VerificationCode() const` | 实现 verification code 对应的业务或工具逻辑。 |
| [L32](../src/apps/controller/pages/DirectConnectPage.h#L32) | `SetActionEnabled` | 声明 | `void SetActionEnabled(bool enabled)` | 更新或应用 set action enabled 相关逻辑。 |
| [L33](../src/apps/controller/pages/DirectConnectPage.h#L33) | `SetActionState` | 声明 | `void SetActionState(bool enabled, const QString& text)` | 更新或应用 set action state 相关逻辑。 |
| [L34](../src/apps/controller/pages/DirectConnectPage.h#L34) | `PrepareCredentials` | 声明 | `void PrepareCredentials(const QString& deviceId)` | 实现 prepare credentials 对应的业务或工具逻辑。 |
| [L35](../src/apps/controller/pages/DirectConnectPage.h#L35) | `ShowSignedOut` | 声明 | `void ShowSignedOut(const QString& message)` | 实现 show signed out 对应的业务或工具逻辑。 |
| [L36](../src/apps/controller/pages/DirectConnectPage.h#L36) | `ShowLoginBusy` | 声明 | `void ShowLoginBusy(const QString& message)` | 实现 show login busy 对应的业务或工具逻辑。 |
| [L37](../src/apps/controller/pages/DirectConnectPage.h#L37) | `SetAuthenticated` | 声明 | `void SetAuthenticated(bool authenticated)` | 更新或应用 set authenticated 相关逻辑。 |
| [L38](../src/apps/controller/pages/DirectConnectPage.h#L38) | `SetAccountLabel` | 声明 | `void SetAccountLabel(const QString& label)` | 更新或应用 set account label 相关逻辑。 |
| [L39](../src/apps/controller/pages/DirectConnectPage.h#L39) | `SetLocalCredentials` | 声明 | `void SetLocalCredentials(const QString& deviceId, const QString& verificationCode)` | 更新或应用 set local credentials 相关逻辑。 |
| [L41](../src/apps/controller/pages/DirectConnectPage.h#L41) | `SetSignalStatus` | 声明 | `void SetSignalStatus(const QString& text, const QString& styleSheet)` | 更新或应用 set signal status 相关逻辑。 |
| [L42](../src/apps/controller/pages/DirectConnectPage.h#L42) | `SetRuntimeStatus` | 声明 | `void SetRuntimeStatus(const QString& text, const QString& color)` | 更新或应用 set runtime status 相关逻辑。 |
| [L43](../src/apps/controller/pages/DirectConnectPage.h#L43) | `SetDecoderStatus` | 声明 | `void SetDecoderStatus(const QString& text, const QString& color)` | 更新或应用 set decoder status 相关逻辑。 |
| [L44](../src/apps/controller/pages/DirectConnectPage.h#L44) | `SetAssistHint` | 声明 | `void SetAssistHint(const QString& text, AssistHintTone tone)` | 更新或应用 set assist hint 相关逻辑。 |
| [L45](../src/apps/controller/pages/DirectConnectPage.h#L45) | `HasAssistError` | 声明 | `bool HasAssistError() const` | 判断 has assist error 相关逻辑。 |
| [L48](../src/apps/controller/pages/DirectConnectPage.h#L48) | `LoginRequested` | 声明 | `void LoginRequested()` | 实现 login requested 对应的业务或工具逻辑。 |
| [L49](../src/apps/controller/pages/DirectConnectPage.h#L49) | `ClipboardTextRequested` | 声明 | `void ClipboardTextRequested(const QString& text, const QString& successMessage)` | 实现 clipboard text requested 对应的业务或工具逻辑。 |
| [L51](../src/apps/controller/pages/DirectConnectPage.h#L51) | `CredentialsChanged` | 声明 | `void CredentialsChanged()` | 实现 credentials changed 对应的业务或工具逻辑。 |
| [L52](../src/apps/controller/pages/DirectConnectPage.h#L52) | `ActionRequested` | 声明 | `void ActionRequested(const QString& deviceId, const QString& verificationCode)` | 实现 action requested 对应的业务或工具逻辑。 |
| [L56](../src/apps/controller/pages/DirectConnectPage.h#L56) | `BuildLoginPrompt` | 声明 | `void BuildLoginPrompt()` | 创建或初始化 build login prompt 相关逻辑。 |
| [L57](../src/apps/controller/pages/DirectConnectPage.h#L57) | `BuildSignedInWorkspace` | 声明 | `void BuildSignedInWorkspace()` | 创建或初始化 build signed in workspace 相关逻辑。 |
| [L58](../src/apps/controller/pages/DirectConnectPage.h#L58) | `ShowCopiedFeedback` | 声明 | `void ShowCopiedFeedback(QPushButton* button, const QString& resetText)` | 实现 show copied feedback 对应的业务或工具逻辑。 |

## `src/apps/controller/pages/DirectConnectPage.Workspace.cpp`

[打开源码](../src/apps/controller/pages/DirectConnectPage.Workspace.cpp) · **文件作用：** 实现 direct connect page workspace 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L25](../src/apps/controller/pages/DirectConnectPage.Workspace.cpp#L25) | `DirectConnectPage::BuildSignedInWorkspace` | 定义 | `void DirectConnectPage::BuildSignedInWorkspace()` | 创建或初始化 build signed in workspace 相关逻辑。 |
| [L306](../src/apps/controller/pages/DirectConnectPage.Workspace.cpp#L306) | `DirectConnectPage::ShowCopiedFeedback` | 定义 | `void DirectConnectPage::ShowCopiedFeedback( QPushButton* button, const QString& resetText)` | 实现 show copied feedback 对应的业务或工具逻辑。 |

## `src/apps/controller/pages/OwnedDevicesPage.cpp`

[打开源码](../src/apps/controller/pages/OwnedDevicesPage.cpp) · **文件作用：** 实现 owned devices page 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L25](../src/apps/controller/pages/OwnedDevicesPage.cpp#L25) | `OwnedDevicesPage::OwnedDevicesPage` | 定义 | `OwnedDevicesPage::OwnedDevicesPage(QWidget* parent) : QScrollArea(parent)` | 构造并初始化 OwnedDevicesPage 实例。 |
| [L145](../src/apps/controller/pages/OwnedDevicesPage.cpp#L145) | `OwnedDevicesPage::UpdateSnapshot` | 定义 | `void OwnedDevicesPage::UpdateSnapshot( const OwnedDevicesSnapshot& ownedDevices, SessionConnectivityState connectivity)` | 更新或应用 update snapshot 相关逻辑。 |
| [L274](../src/apps/controller/pages/OwnedDevicesPage.cpp#L274) | `OwnedDevicesPage::RefreshThemeStyle` | 定义 | `void OwnedDevicesPage::RefreshThemeStyle()` | 刷新 refresh theme style 相关逻辑。 |
| [L298](../src/apps/controller/pages/OwnedDevicesPage.cpp#L298) | `OwnedDevicesPage::PulseRefreshIcon` | 定义 | `void OwnedDevicesPage::PulseRefreshIcon()` | 实现 pulse refresh icon 对应的业务或工具逻辑。 |

## `src/apps/controller/pages/OwnedDevicesPage.h`

[打开源码](../src/apps/controller/pages/OwnedDevicesPage.h) · **文件作用：** 声明 owned devices page 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L11](../src/apps/controller/pages/OwnedDevicesPage.h#L11) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L12](../src/apps/controller/pages/OwnedDevicesPage.h#L12) | `QFrame` | class | 定义 QFrame 的 class 类型和相关状态。 |
| [L13](../src/apps/controller/pages/OwnedDevicesPage.h#L13) | `QPushButton` | class | 定义 QPushButton 的 class 类型和相关状态。 |
| [L14](../src/apps/controller/pages/OwnedDevicesPage.h#L14) | `QVBoxLayout` | class | 定义 QVBoxLayout 的 class 类型和相关状态。 |
| [L18](../src/apps/controller/pages/OwnedDevicesPage.h#L18) | `OwnedDevicesPage` | class | 定义 OwnedDevicesPage 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L11](../src/apps/controller/pages/OwnedDevicesPage.h#L11) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L12](../src/apps/controller/pages/OwnedDevicesPage.h#L12) | `QFrame` | `class QFrame;` | 保存媒体帧、图像或缓冲资源：q frame。 |
| [L13](../src/apps/controller/pages/OwnedDevicesPage.h#L13) | `QPushButton` | `class QPushButton;` | 保存 q push button 相关配置或运行状态。 |
| [L14](../src/apps/controller/pages/OwnedDevicesPage.h#L14) | `QVBoxLayout` | `class QVBoxLayout;` | 保存 qv box layout 相关配置或运行状态。 |
| [L36](../src/apps/controller/pages/OwnedDevicesPage.h#L36) | `content_` | `QWidget* content_ = nullptr;` | 保存 content 相关配置或运行状态。 |
| [L37](../src/apps/controller/pages/OwnedDevicesPage.h#L37) | `cardsLayout_` | `QVBoxLayout* cardsLayout_ = nullptr;` | 保存 cards layout 相关配置或运行状态。 |
| [L38](../src/apps/controller/pages/OwnedDevicesPage.h#L38) | `summaryLabel_` | `QLabel* summaryLabel_ = nullptr;` | 保存路径、地址或显示名称：summary label。 |
| [L39](../src/apps/controller/pages/OwnedDevicesPage.h#L39) | `emptyState_` | `QFrame* emptyState_ = nullptr;` | 保存状态机当前状态：empty state。 |
| [L40](../src/apps/controller/pages/OwnedDevicesPage.h#L40) | `emptyArtwork_` | `QLabel* emptyArtwork_ = nullptr;` | 保存 empty artwork 相关配置或运行状态。 |
| [L41](../src/apps/controller/pages/OwnedDevicesPage.h#L41) | `refreshButton_` | `QPushButton* refreshButton_ = nullptr;` | 保存 refresh button 相关配置或运行状态。 |
| [L42](../src/apps/controller/pages/OwnedDevicesPage.h#L42) | `renderedRevision_` | `quint64 renderedRevision_ = 0;` | 标记当前世代，用于拒绝过期异步结果：rendered revision。 |
| [L44](../src/apps/controller/pages/OwnedDevicesPage.h#L44) | `kNotConfigured` | `SessionConnectivityState::kNotConfigured;` | 定义 not configured 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L22](../src/apps/controller/pages/OwnedDevicesPage.h#L22) | `OwnedDevicesPage` | 声明 | `explicit OwnedDevicesPage(QWidget* parent = nullptr)` | 实现 owned devices page 对应的业务或工具逻辑。 |
| [L24](../src/apps/controller/pages/OwnedDevicesPage.h#L24) | `UpdateSnapshot` | 声明 | `void UpdateSnapshot(const OwnedDevicesSnapshot& ownedDevices, SessionConnectivityState connectivity)` | 更新或应用 update snapshot 相关逻辑。 |
| [L26](../src/apps/controller/pages/OwnedDevicesPage.h#L26) | `RefreshThemeStyle` | 声明 | `void RefreshThemeStyle()` | 刷新 refresh theme style 相关逻辑。 |
| [L29](../src/apps/controller/pages/OwnedDevicesPage.h#L29) | `refreshRequested` | 声明 | `void refreshRequested()` | 刷新 refresh requested 相关逻辑。 |
| [L30](../src/apps/controller/pages/OwnedDevicesPage.h#L30) | `connectRequested` | 声明 | `void connectRequested(const QString& deviceId, const QString& deviceName)` | 建立连接 connect requested 相关逻辑。 |
| [L34](../src/apps/controller/pages/OwnedDevicesPage.h#L34) | `PulseRefreshIcon` | 声明 | `void PulseRefreshIcon()` | 实现 pulse refresh icon 对应的业务或工具逻辑。 |

## `src/apps/controller/pages/RecentConnectionsPage.cpp`

[打开源码](../src/apps/controller/pages/RecentConnectionsPage.cpp) · **文件作用：** 实现 recent connections page 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L23](../src/apps/controller/pages/RecentConnectionsPage.cpp#L23) | `MakeEmptyState` | 定义 | `QFrame* MakeEmptyState( QWidget* parent, const QString& iconPath, const QString& title, const QString& hint)` | 创建或初始化 make empty state 相关逻辑。 |
| [L57](../src/apps/controller/pages/RecentConnectionsPage.cpp#L57) | `ClearCards` | 定义 | `void ClearCards(QVBoxLayout* layout, QFrame* emptyState)` | 重置或移除 clear cards 相关逻辑。 |
| [L70](../src/apps/controller/pages/RecentConnectionsPage.cpp#L70) | `RecentConnectionsPage::RecentConnectionsPage` | 定义 | `RecentConnectionsPage::RecentConnectionsPage(QWidget* parent) : QScrollArea(parent)` | 构造并初始化 RecentConnectionsPage 实例。 |
| [L143](../src/apps/controller/pages/RecentConnectionsPage.cpp#L143) | `RecentConnectionsPage::SetRooms` | 定义 | `void RecentConnectionsPage::SetRooms( const QVector<RecentRoomCardData>& rooms)` | 更新或应用 set rooms 相关逻辑。 |
| [L218](../src/apps/controller/pages/RecentConnectionsPage.cpp#L218) | `RecentConnectionsPage::SetDevices` | 定义 | `void RecentConnectionsPage::SetDevices( const QVector<RecentDeviceCardData>& devices)` | 更新或应用 set devices 相关逻辑。 |

## `src/apps/controller/pages/RecentConnectionsPage.h`

[打开源码](../src/apps/controller/pages/RecentConnectionsPage.h) · **文件作用：** 声明 recent connections page 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L10](../src/apps/controller/pages/RecentConnectionsPage.h#L10) | `QFrame` | class | 定义 QFrame 的 class 类型和相关状态。 |
| [L11](../src/apps/controller/pages/RecentConnectionsPage.h#L11) | `QVBoxLayout` | class | 定义 QVBoxLayout 的 class 类型和相关状态。 |
| [L15](../src/apps/controller/pages/RecentConnectionsPage.h#L15) | `RecentRoomCardData` | struct | 定义 RecentRoomCardData 的 struct 类型和相关状态。 |
| [L24](../src/apps/controller/pages/RecentConnectionsPage.h#L24) | `RecentDeviceCardData` | struct | 定义 RecentDeviceCardData 的 struct 类型和相关状态。 |
| [L34](../src/apps/controller/pages/RecentConnectionsPage.h#L34) | `RecentConnectionsPage` | class | 定义 RecentConnectionsPage 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L10](../src/apps/controller/pages/RecentConnectionsPage.h#L10) | `QFrame` | `class QFrame;` | 保存媒体帧、图像或缓冲资源：q frame。 |
| [L11](../src/apps/controller/pages/RecentConnectionsPage.h#L11) | `QVBoxLayout` | `class QVBoxLayout;` | 保存 qv box layout 相关配置或运行状态。 |
| [L16](../src/apps/controller/pages/RecentConnectionsPage.h#L16) | `roomId` | `QString roomId;` | 保存身份或作用域标识：room id。 |
| [L17](../src/apps/controller/pages/RecentConnectionsPage.h#L17) | `detail` | `QString detail;` | 保存 detail 相关配置或运行状态。 |
| [L18](../src/apps/controller/pages/RecentConnectionsPage.h#L18) | `availabilityText` | `QString availabilityText;` | 保存 availability text 相关配置或运行状态。 |
| [L19](../src/apps/controller/pages/RecentConnectionsPage.h#L19) | `availabilityTone` | `QString availabilityTone;` | 保存 availability tone 相关配置或运行状态。 |
| [L20](../src/apps/controller/pages/RecentConnectionsPage.h#L20) | `actionText` | `QString actionText;` | 保存 action text 相关配置或运行状态。 |
| [L21](../src/apps/controller/pages/RecentConnectionsPage.h#L21) | `canJoin` | `bool canJoin = false;` | 保存 can join 相关配置或运行状态。 |
| [L25](../src/apps/controller/pages/RecentConnectionsPage.h#L25) | `deviceId` | `QString deviceId;` | 保存身份或作用域标识：device id。 |
| [L26](../src/apps/controller/pages/RecentConnectionsPage.h#L26) | `deviceName` | `QString deviceName;` | 保存路径、地址或显示名称：device name。 |
| [L27](../src/apps/controller/pages/RecentConnectionsPage.h#L27) | `detail` | `QString detail;` | 保存 detail 相关配置或运行状态。 |
| [L28](../src/apps/controller/pages/RecentConnectionsPage.h#L28) | `actionText` | `QString actionText;` | 保存 action text 相关配置或运行状态。 |
| [L29](../src/apps/controller/pages/RecentConnectionsPage.h#L29) | `ownedDevice` | `bool ownedDevice = false;` | 保存 owned device 相关配置或运行状态。 |
| [L30](../src/apps/controller/pages/RecentConnectionsPage.h#L30) | `ownedDeviceOnline` | `bool ownedDeviceOnline = false;` | 保存 owned device online 相关配置或运行状态。 |
| [L31](../src/apps/controller/pages/RecentConnectionsPage.h#L31) | `actionEnabled` | `bool actionEnabled = false;` | 保存能力或开关状态：action enabled。 |
| [L50](../src/apps/controller/pages/RecentConnectionsPage.h#L50) | `content_` | `QWidget* content_ = nullptr;` | 保存 content 相关配置或运行状态。 |
| [L51](../src/apps/controller/pages/RecentConnectionsPage.h#L51) | `roomsLayout_` | `QVBoxLayout* roomsLayout_ = nullptr;` | 保存 rooms layout 相关配置或运行状态。 |
| [L52](../src/apps/controller/pages/RecentConnectionsPage.h#L52) | `devicesLayout_` | `QVBoxLayout* devicesLayout_ = nullptr;` | 保存 devices layout 相关配置或运行状态。 |
| [L53](../src/apps/controller/pages/RecentConnectionsPage.h#L53) | `roomsEmptyState_` | `QFrame* roomsEmptyState_ = nullptr;` | 保存状态机当前状态：rooms empty state。 |
| [L54](../src/apps/controller/pages/RecentConnectionsPage.h#L54) | `devicesEmptyState_` | `QFrame* devicesEmptyState_ = nullptr;` | 保存状态机当前状态：devices empty state。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L38](../src/apps/controller/pages/RecentConnectionsPage.h#L38) | `RecentConnectionsPage` | 声明 | `explicit RecentConnectionsPage(QWidget* parent = nullptr)` | 实现 recent connections page 对应的业务或工具逻辑。 |
| [L40](../src/apps/controller/pages/RecentConnectionsPage.h#L40) | `SetRooms` | 声明 | `void SetRooms(const QVector<RecentRoomCardData>& rooms)` | 更新或应用 set rooms 相关逻辑。 |
| [L41](../src/apps/controller/pages/RecentConnectionsPage.h#L41) | `SetDevices` | 声明 | `void SetDevices(const QVector<RecentDeviceCardData>& devices)` | 更新或应用 set devices 相关逻辑。 |
| [L44](../src/apps/controller/pages/RecentConnectionsPage.h#L44) | `joinRoomRequested` | 声明 | `void joinRoomRequested(const QString& roomId)` | 实现 join room requested 对应的业务或工具逻辑。 |
| [L45](../src/apps/controller/pages/RecentConnectionsPage.h#L45) | `connectOwnedDeviceRequested` | 声明 | `void connectOwnedDeviceRequested(const QString& deviceId, const QString& deviceName)` | 建立连接 connect owned device requested 相关逻辑。 |
| [L47](../src/apps/controller/pages/RecentConnectionsPage.h#L47) | `reconnectAssistedDeviceRequested` | 声明 | `void reconnectAssistedDeviceRequested(const QString& deviceId)` | 实现 reconnect assisted device requested 对应的业务或工具逻辑。 |

## `src/apps/controller/pages/RoomPage.ActiveRoom.cpp`

[打开源码](../src/apps/controller/pages/RoomPage.ActiveRoom.cpp) · **文件作用：** 实现 room page active room 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L33](../src/apps/controller/pages/RoomPage.ActiveRoom.cpp#L33) | `RoomPage::BuildActiveRoom` | 定义 | `void RoomPage::BuildActiveRoom()` | 创建或初始化 build active room 相关逻辑。 |

## `src/apps/controller/pages/RoomPage.cpp`

[打开源码](../src/apps/controller/pages/RoomPage.cpp) · **文件作用：** 实现 room page 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L25](../src/apps/controller/pages/RoomPage.cpp#L25) | `RoomPage::RoomPage` | 定义 | `RoomPage::RoomPage(QWidget* parent) : QScrollArea(parent)` | 构造并初始化 RoomPage 实例。 |
| [L46](../src/apps/controller/pages/RoomPage.cpp#L46) | `RoomPage::BuildHeaderAndEntry` | 定义 | `void RoomPage::BuildHeaderAndEntry()` | 创建或初始化 build header and entry 相关逻辑。 |
| [L172](../src/apps/controller/pages/RoomPage.cpp#L172) | `RoomPage::Controls` | 定义 | `RoomPageControls& RoomPage::Controls()` | 实现 controls 对应的业务或工具逻辑。 |
| [L177](../src/apps/controller/pages/RoomPage.cpp#L177) | `RoomPage::Controls` | 定义 | `const RoomPageControls& RoomPage::Controls() const` | 实现 controls 对应的业务或工具逻辑。 |
| [L182](../src/apps/controller/pages/RoomPage.cpp#L182) | `RoomPage::SetEntryControls` | 定义 | `void RoomPage::SetEntryControls(QComboBox* capacity, QPushButton* createButton, QLineEdit* roomId, QPushButton* joinButton)` | 更新或应用 set entry controls 相关逻辑。 |
| [L202](../src/apps/controller/pages/RoomPage.cpp#L202) | `RoomPage::FocusRoomId` | 定义 | `void RoomPage::FocusRoomId()` | 实现 focus room id 对应的业务或工具逻辑。 |
| [L209](../src/apps/controller/pages/RoomPage.cpp#L209) | `RoomPage::SetEntryEnabled` | 定义 | `void RoomPage::SetEntryEnabled(bool enabled)` | 更新或应用 set entry enabled 相关逻辑。 |
| [L217](../src/apps/controller/pages/RoomPage.cpp#L217) | `RoomPage::SetEntryActionsEnabled` | 定义 | `void RoomPage::SetEntryActionsEnabled(bool enabled)` | 更新或应用 set entry actions enabled 相关逻辑。 |
| [L223](../src/apps/controller/pages/RoomPage.cpp#L223) | `RoomPage::SetEntryActionTexts` | 定义 | `void RoomPage::SetEntryActionTexts(const QString& createText, const QString& joinText)` | 更新或应用 set entry action texts 相关逻辑。 |
| [L230](../src/apps/controller/pages/RoomPage.cpp#L230) | `RoomPage::SetCreateActionText` | 定义 | `void RoomPage::SetCreateActionText(const QString& text)` | 更新或应用 set create action text 相关逻辑。 |
| [L235](../src/apps/controller/pages/RoomPage.cpp#L235) | `RoomPage::SetJoinActionText` | 定义 | `void RoomPage::SetJoinActionText(const QString& text)` | 更新或应用 set join action text 相关逻辑。 |
| [L240](../src/apps/controller/pages/RoomPage.cpp#L240) | `RoomPage::SetCapacity` | 定义 | `void RoomPage::SetCapacity(uint capacity)` | 更新或应用 set capacity 相关逻辑。 |
| [L249](../src/apps/controller/pages/RoomPage.cpp#L249) | `RoomPage::SetRoomId` | 定义 | `void RoomPage::SetRoomId(const QString& roomId)` | 更新或应用 set room id 相关逻辑。 |

## `src/apps/controller/pages/RoomPage.h`

[打开源码](../src/apps/controller/pages/RoomPage.h) · **文件作用：** 声明 room page 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L12](../src/apps/controller/pages/RoomPage.h#L12) | `QComboBox` | class | 定义 QComboBox 的 class 类型和相关状态。 |
| [L13](../src/apps/controller/pages/RoomPage.h#L13) | `QFrame` | class | 定义 QFrame 的 class 类型和相关状态。 |
| [L14](../src/apps/controller/pages/RoomPage.h#L14) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L15](../src/apps/controller/pages/RoomPage.h#L15) | `QLineEdit` | class | 定义 QLineEdit 的 class 类型和相关状态。 |
| [L16](../src/apps/controller/pages/RoomPage.h#L16) | `QListWidget` | class | 定义 QListWidget 的 class 类型和相关状态。 |
| [L17](../src/apps/controller/pages/RoomPage.h#L17) | `QParallelAnimationGroup` | class | 定义 QParallelAnimationGroup 的 class 类型和相关状态。 |
| [L18](../src/apps/controller/pages/RoomPage.h#L18) | `QPushButton` | class | 定义 QPushButton 的 class 类型和相关状态。 |
| [L19](../src/apps/controller/pages/RoomPage.h#L19) | `QStackedWidget` | class | 定义 QStackedWidget 的 class 类型和相关状态。 |
| [L20](../src/apps/controller/pages/RoomPage.h#L20) | `QVBoxLayout` | class | 定义 QVBoxLayout 的 class 类型和相关状态。 |
| [L24](../src/apps/controller/pages/RoomPage.h#L24) | `RoomPageControls` | struct | 定义 RoomPageControls 的 struct 类型和相关状态。 |
| [L55](../src/apps/controller/pages/RoomPage.h#L55) | `RoomPage` | class | 定义 RoomPage 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L12](../src/apps/controller/pages/RoomPage.h#L12) | `QComboBox` | `class QComboBox;` | 保存 q combo box 相关配置或运行状态。 |
| [L13](../src/apps/controller/pages/RoomPage.h#L13) | `QFrame` | `class QFrame;` | 保存媒体帧、图像或缓冲资源：q frame。 |
| [L14](../src/apps/controller/pages/RoomPage.h#L14) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L15](../src/apps/controller/pages/RoomPage.h#L15) | `QLineEdit` | `class QLineEdit;` | 保存 q line edit 相关配置或运行状态。 |
| [L16](../src/apps/controller/pages/RoomPage.h#L16) | `QListWidget` | `class QListWidget;` | 保存 q list widget 相关配置或运行状态。 |
| [L17](../src/apps/controller/pages/RoomPage.h#L17) | `QParallelAnimationGroup` | `class QParallelAnimationGroup;` | 保存 q parallel animation group 相关配置或运行状态。 |
| [L18](../src/apps/controller/pages/RoomPage.h#L18) | `QPushButton` | `class QPushButton;` | 保存 q push button 相关配置或运行状态。 |
| [L19](../src/apps/controller/pages/RoomPage.h#L19) | `QStackedWidget` | `class QStackedWidget;` | 保存 q stacked widget 相关配置或运行状态。 |
| [L20](../src/apps/controller/pages/RoomPage.h#L20) | `QVBoxLayout` | `class QVBoxLayout;` | 保存 qv box layout 相关配置或运行状态。 |
| [L25](../src/apps/controller/pages/RoomPage.h#L25) | `connectivityLabel` | `QLabel* connectivityLabel = nullptr;` | 保存路径、地址或显示名称：connectivity label。 |
| [L26](../src/apps/controller/pages/RoomPage.h#L26) | `actionHint` | `QLabel* actionHint = nullptr;` | 保存 action hint 相关配置或运行状态。 |
| [L27](../src/apps/controller/pages/RoomPage.h#L27) | `entryPanel` | `QFrame* entryPanel = nullptr;` | 保存 entry panel 相关配置或运行状态。 |
| [L28](../src/apps/controller/pages/RoomPage.h#L28) | `roomPanel` | `QFrame* roomPanel = nullptr;` | 保存 room panel 相关配置或运行状态。 |
| [L29](../src/apps/controller/pages/RoomPage.h#L29) | `workspaceStack` | `QStackedWidget* workspaceStack = nullptr;` | 保存 workspace stack 相关配置或运行状态。 |
| [L30](../src/apps/controller/pages/RoomPage.h#L30) | `workspaceAnimation` | `QParallelAnimationGroup* workspaceAnimation = nullptr;` | 保存 workspace animation 相关配置或运行状态。 |
| [L31](../src/apps/controller/pages/RoomPage.h#L31) | `workspaceTransitionLayer` | `QWidget* workspaceTransitionLayer = nullptr;` | 保存 workspace transition layer 相关配置或运行状态。 |
| [L32](../src/apps/controller/pages/RoomPage.h#L32) | `roomIdLabel` | `QLabel* roomIdLabel = nullptr;` | 保存路径、地址或显示名称：room id label。 |
| [L33](../src/apps/controller/pages/RoomPage.h#L33) | `copyRoomIdButton` | `QPushButton* copyRoomIdButton = nullptr;` | 保存 copy room id button 相关配置或运行状态。 |
| [L34](../src/apps/controller/pages/RoomPage.h#L34) | `occupancyLabel` | `QLabel* occupancyLabel = nullptr;` | 保存路径、地址或显示名称：occupancy label。 |
| [L35](../src/apps/controller/pages/RoomPage.h#L35) | `ownerLabel` | `QLabel* ownerLabel = nullptr;` | 保存路径、地址或显示名称：owner label。 |
| [L36](../src/apps/controller/pages/RoomPage.h#L36) | `screenSharerLabel` | `QLabel* screenSharerLabel = nullptr;` | 保存路径、地址或显示名称：screen sharer label。 |
| [L37](../src/apps/controller/pages/RoomPage.h#L37) | `controllerLabel` | `QLabel* controllerLabel = nullptr;` | 保存路径、地址或显示名称：controller label。 |
| [L38](../src/apps/controller/pages/RoomPage.h#L38) | `peerConnectivityLabel` | `QLabel* peerConnectivityLabel = nullptr;` | 保存路径、地址或显示名称：peer connectivity label。 |
| [L39](../src/apps/controller/pages/RoomPage.h#L39) | `seatUsageLabel` | `QLabel* seatUsageLabel = nullptr;` | 保存路径、地址或显示名称：seat usage label。 |
| [L40](../src/apps/controller/pages/RoomPage.h#L40) | `memberSummaryLabel` | `QLabel* memberSummaryLabel = nullptr;` | 保存路径、地址或显示名称：member summary label。 |
| [L41](../src/apps/controller/pages/RoomPage.h#L41) | `memberFooterLabel` | `QLabel* memberFooterLabel = nullptr;` | 保存路径、地址或显示名称：member footer label。 |
| [L42](../src/apps/controller/pages/RoomPage.h#L42) | `memberList` | `QListWidget* memberList = nullptr;` | 保存 member list 相关配置或运行状态。 |
| [L43](../src/apps/controller/pages/RoomPage.h#L43) | `activeRoomCapacity` | `QComboBox* activeRoomCapacity = nullptr;` | 保存 active room capacity 相关配置或运行状态。 |
| [L44](../src/apps/controller/pages/RoomPage.h#L44) | `applyRoomCapacityButton` | `QPushButton* applyRoomCapacityButton = nullptr;` | 保存 apply room capacity button 相关配置或运行状态。 |
| [L45](../src/apps/controller/pages/RoomPage.h#L45) | `screenShareButton` | `QPushButton* screenShareButton = nullptr;` | 保存 screen share button 相关配置或运行状态。 |
| [L46](../src/apps/controller/pages/RoomPage.h#L46) | `cameraButton` | `QPushButton* cameraButton = nullptr;` | 保存 camera button 相关配置或运行状态。 |
| [L47](../src/apps/controller/pages/RoomPage.h#L47) | `microphoneButton` | `QPushButton* microphoneButton = nullptr;` | 保存 microphone button 相关配置或运行状态。 |
| [L48](../src/apps/controller/pages/RoomPage.h#L48) | `speakerButton` | `QPushButton* speakerButton = nullptr;` | 保存 speaker button 相关配置或运行状态。 |
| [L49](../src/apps/controller/pages/RoomPage.h#L49) | `cameraGalleryButton` | `QPushButton* cameraGalleryButton = nullptr;` | 保存 camera gallery button 相关配置或运行状态。 |
| [L50](../src/apps/controller/pages/RoomPage.h#L50) | `fileTransferButton` | `QPushButton* fileTransferButton = nullptr;` | 保存 file transfer button 相关配置或运行状态。 |
| [L51](../src/apps/controller/pages/RoomPage.h#L51) | `leaveButton` | `QPushButton* leaveButton = nullptr;` | 保存 leave button 相关配置或运行状态。 |
| [L52](../src/apps/controller/pages/RoomPage.h#L52) | `stageLabel` | `QLabel* stageLabel = nullptr;` | 保存路径、地址或显示名称：stage label。 |
| [L87](../src/apps/controller/pages/RoomPage.h#L87) | `content_` | `QWidget* content_ = nullptr;` | 保存 content 相关配置或运行状态。 |
| [L88](../src/apps/controller/pages/RoomPage.h#L88) | `contentLayout_` | `QVBoxLayout* contentLayout_ = nullptr;` | 保存 content layout 相关配置或运行状态。 |
| [L89](../src/apps/controller/pages/RoomPage.h#L89) | `controls_` | `RoomPageControls controls_;` | 保存 controls 相关配置或运行状态。 |
| [L90](../src/apps/controller/pages/RoomPage.h#L90) | `capacity_` | `QComboBox* capacity_ = nullptr;` | 保存 capacity 相关配置或运行状态。 |
| [L91](../src/apps/controller/pages/RoomPage.h#L91) | `createButton_` | `QPushButton* createButton_ = nullptr;` | 保存 create button 相关配置或运行状态。 |
| [L92](../src/apps/controller/pages/RoomPage.h#L92) | `roomId_` | `QLineEdit* roomId_ = nullptr;` | 保存身份或作用域标识：room id。 |
| [L93](../src/apps/controller/pages/RoomPage.h#L93) | `joinButton_` | `QPushButton* joinButton_ = nullptr;` | 保存 join button 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L59](../src/apps/controller/pages/RoomPage.h#L59) | `RoomPage` | 声明 | `explicit RoomPage(QWidget* parent = nullptr)` | 实现 room page 对应的业务或工具逻辑。 |
| [L61](../src/apps/controller/pages/RoomPage.h#L61) | `Controls` | 声明 | `RoomPageControls& Controls()` | 实现 controls 对应的业务或工具逻辑。 |
| [L62](../src/apps/controller/pages/RoomPage.h#L62) | `Controls` | 声明 | `const RoomPageControls& Controls() const` | 实现 controls 对应的业务或工具逻辑。 |
| [L63](../src/apps/controller/pages/RoomPage.h#L63) | `SetEntryEnabled` | 声明 | `void SetEntryEnabled(bool enabled)` | 更新或应用 set entry enabled 相关逻辑。 |
| [L64](../src/apps/controller/pages/RoomPage.h#L64) | `SetEntryActionsEnabled` | 声明 | `void SetEntryActionsEnabled(bool enabled)` | 更新或应用 set entry actions enabled 相关逻辑。 |
| [L65](../src/apps/controller/pages/RoomPage.h#L65) | `SetEntryActionTexts` | 声明 | `void SetEntryActionTexts(const QString& createText, const QString& joinText)` | 更新或应用 set entry action texts 相关逻辑。 |
| [L67](../src/apps/controller/pages/RoomPage.h#L67) | `SetCreateActionText` | 声明 | `void SetCreateActionText(const QString& text)` | 更新或应用 set create action text 相关逻辑。 |
| [L68](../src/apps/controller/pages/RoomPage.h#L68) | `SetJoinActionText` | 声明 | `void SetJoinActionText(const QString& text)` | 更新或应用 set join action text 相关逻辑。 |
| [L69](../src/apps/controller/pages/RoomPage.h#L69) | `SetCapacity` | 声明 | `void SetCapacity(uint capacity)` | 更新或应用 set capacity 相关逻辑。 |
| [L70](../src/apps/controller/pages/RoomPage.h#L70) | `SetRoomId` | 声明 | `void SetRoomId(const QString& roomId)` | 更新或应用 set room id 相关逻辑。 |
| [L71](../src/apps/controller/pages/RoomPage.h#L71) | `FocusRoomId` | 声明 | `void FocusRoomId()` | 实现 focus room id 对应的业务或工具逻辑。 |
| [L74](../src/apps/controller/pages/RoomPage.h#L74) | `CreateRequested` | 声明 | `void CreateRequested(uint capacity)` | 创建或初始化 create requested 相关逻辑。 |
| [L75](../src/apps/controller/pages/RoomPage.h#L75) | `JoinRequested` | 声明 | `void JoinRequested(const QString& roomId)` | 实现 join requested 对应的业务或工具逻辑。 |
| [L76](../src/apps/controller/pages/RoomPage.h#L76) | `MemberContextMenuRequested` | 声明 | `void MemberContextMenuRequested(const QPoint& position)` | 实现 member context menu requested 对应的业务或工具逻辑。 |
| [L77](../src/apps/controller/pages/RoomPage.h#L77) | `MediaDeviceMenuRequested` | 声明 | `void MediaDeviceMenuRequested(MediaDeviceKind kind, QWidget* anchor)` | 实现 media device menu requested 对应的业务或工具逻辑。 |
| [L80](../src/apps/controller/pages/RoomPage.h#L80) | `BuildHeaderAndEntry` | 声明 | `void BuildHeaderAndEntry()` | 创建或初始化 build header and entry 相关逻辑。 |
| [L81](../src/apps/controller/pages/RoomPage.h#L81) | `BuildActiveRoom` | 声明 | `void BuildActiveRoom()` | 创建或初始化 build active room 相关逻辑。 |
| [L82](../src/apps/controller/pages/RoomPage.h#L82) | `SetEntryControls` | 声明 | `void SetEntryControls(QComboBox* capacity, QPushButton* createButton, QLineEdit* roomId, QPushButton* joinButton)` | 更新或应用 set entry controls 相关逻辑。 |

## `src/apps/controller/pages/SettingsPage.cpp`

[打开源码](../src/apps/controller/pages/SettingsPage.cpp) · **文件作用：** 实现 settings page 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L34](../src/apps/controller/pages/SettingsPage.cpp#L34) | `SettingsPage::SettingsPage` | 定义 | `SettingsPage::SettingsPage(QWidget* parent) : QScrollArea(parent)` | 构造并初始化 SettingsPage 实例。 |
| [L123](../src/apps/controller/pages/SettingsPage.cpp#L123) | `SettingsPage::BuildGeneralSettingsPage` | 定义 | `void SettingsPage::BuildGeneralSettingsPage()` | 创建或初始化 build general settings page 相关逻辑。 |
| [L369](../src/apps/controller/pages/SettingsPage.cpp#L369) | `SettingsPage::DetailStack` | 定义 | `QStackedWidget* SettingsPage::DetailStack() const` | 实现 detail stack 对应的业务或工具逻辑。 |
| [L374](../src/apps/controller/pages/SettingsPage.cpp#L374) | `SettingsPage::Controls` | 定义 | `SettingsPageControls& SettingsPage::Controls()` | 实现 controls 对应的业务或工具逻辑。 |
| [L379](../src/apps/controller/pages/SettingsPage.cpp#L379) | `SettingsPage::Controls` | 定义 | `const SettingsPageControls& SettingsPage::Controls() const` | 实现 controls 对应的业务或工具逻辑。 |
| [L384](../src/apps/controller/pages/SettingsPage.cpp#L384) | `SettingsPage::CurrentCategory` | 定义 | `int SettingsPage::CurrentCategory() const` | 实现 current category 对应的业务或工具逻辑。 |

## `src/apps/controller/pages/SettingsPage.FileTransfer.cpp`

[打开源码](../src/apps/controller/pages/SettingsPage.FileTransfer.cpp) · **文件作用：** 实现 settings page file transfer 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L25](../src/apps/controller/pages/SettingsPage.FileTransfer.cpp#L25) | `SettingsPage::BuildFileTransferSettingsPage` | 定义 | `void SettingsPage::BuildFileTransferSettingsPage()` | 创建或初始化 build file transfer settings page 相关逻辑。 |

## `src/apps/controller/pages/SettingsPage.h`

[打开源码](../src/apps/controller/pages/SettingsPage.h) · **文件作用：** 声明 settings page 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L8](../src/apps/controller/pages/SettingsPage.h#L8) | `QButtonGroup` | class | 定义 QButtonGroup 的 class 类型和相关状态。 |
| [L9](../src/apps/controller/pages/SettingsPage.h#L9) | `QComboBox` | class | 定义 QComboBox 的 class 类型和相关状态。 |
| [L10](../src/apps/controller/pages/SettingsPage.h#L10) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L11](../src/apps/controller/pages/SettingsPage.h#L11) | `QPushButton` | class | 定义 QPushButton 的 class 类型和相关状态。 |
| [L12](../src/apps/controller/pages/SettingsPage.h#L12) | `QStackedWidget` | class | 定义 QStackedWidget 的 class 类型和相关状态。 |
| [L13](../src/apps/controller/pages/SettingsPage.h#L13) | `QVBoxLayout` | class | 定义 QVBoxLayout 的 class 类型和相关状态。 |
| [L17](../src/apps/controller/pages/SettingsPage.h#L17) | `SettingsPageControls` | struct | 定义 SettingsPageControls 的 struct 类型和相关状态。 |
| [L55](../src/apps/controller/pages/SettingsPage.h#L55) | `SettingsPage` | class | 定义 SettingsPage 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L8](../src/apps/controller/pages/SettingsPage.h#L8) | `QButtonGroup` | `class QButtonGroup;` | 保存 q button group 相关配置或运行状态。 |
| [L9](../src/apps/controller/pages/SettingsPage.h#L9) | `QComboBox` | `class QComboBox;` | 保存 q combo box 相关配置或运行状态。 |
| [L10](../src/apps/controller/pages/SettingsPage.h#L10) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L11](../src/apps/controller/pages/SettingsPage.h#L11) | `QPushButton` | `class QPushButton;` | 保存 q push button 相关配置或运行状态。 |
| [L12](../src/apps/controller/pages/SettingsPage.h#L12) | `QStackedWidget` | `class QStackedWidget;` | 保存 q stacked widget 相关配置或运行状态。 |
| [L13](../src/apps/controller/pages/SettingsPage.h#L13) | `QVBoxLayout` | `class QVBoxLayout;` | 保存 qv box layout 相关配置或运行状态。 |
| [L18](../src/apps/controller/pages/SettingsPage.h#L18) | `animationLevelSelector` | `QComboBox* animationLevelSelector = nullptr;` | 保存 animation level selector 相关配置或运行状态。 |
| [L19](../src/apps/controller/pages/SettingsPage.h#L19) | `themeModeSelector` | `QComboBox* themeModeSelector = nullptr;` | 保存 theme mode selector 相关配置或运行状态。 |
| [L20](../src/apps/controller/pages/SettingsPage.h#L20) | `fontFamilySelector` | `QComboBox* fontFamilySelector = nullptr;` | 保存 font family selector 相关配置或运行状态。 |
| [L21](../src/apps/controller/pages/SettingsPage.h#L21) | `fontSizeSelector` | `QComboBox* fontSizeSelector = nullptr;` | 保存 font size selector 相关配置或运行状态。 |
| [L22](../src/apps/controller/pages/SettingsPage.h#L22) | `autoStartSelector` | `QComboBox* autoStartSelector = nullptr;` | 保存 auto start selector 相关配置或运行状态。 |
| [L23](../src/apps/controller/pages/SettingsPage.h#L23) | `startupVisibilitySelector` | `QComboBox* startupVisibilitySelector = nullptr;` | 保存 startup visibility selector 相关配置或运行状态。 |
| [L24](../src/apps/controller/pages/SettingsPage.h#L24) | `closeButtonBehaviorSelector` | `QComboBox* closeButtonBehaviorSelector = nullptr;` | 保存 close button behavior selector 相关配置或运行状态。 |
| [L25](../src/apps/controller/pages/SettingsPage.h#L25) | `defaultRoomCapacitySelector` | `QComboBox* defaultRoomCapacitySelector = nullptr;` | 保存 default room capacity selector 相关配置或运行状态。 |
| [L26](../src/apps/controller/pages/SettingsPage.h#L26) | `desktopCaptureSelector` | `QComboBox* desktopCaptureSelector = nullptr;` | 保存 desktop capture selector 相关配置或运行状态。 |
| [L27](../src/apps/controller/pages/SettingsPage.h#L27) | `videoEncoderSelector` | `QComboBox* videoEncoderSelector = nullptr;` | 保存 video encoder selector 相关配置或运行状态。 |
| [L28](../src/apps/controller/pages/SettingsPage.h#L28) | `ffmpegHardwareBackendSelector` | `QComboBox* ffmpegHardwareBackendSelector = nullptr;` | 保存 ffmpeg hardware backend selector 相关配置或运行状态。 |
| [L29](../src/apps/controller/pages/SettingsPage.h#L29) | `ffmpegX264PresetSelector` | `QComboBox* ffmpegX264PresetSelector = nullptr;` | 保存 ffmpeg x264 preset selector 相关配置或运行状态。 |
| [L30](../src/apps/controller/pages/SettingsPage.h#L30) | `videoDecoderSelector` | `QComboBox* videoDecoderSelector = nullptr;` | 保存 video decoder selector 相关配置或运行状态。 |
| [L31](../src/apps/controller/pages/SettingsPage.h#L31) | `videoRendererSelector` | `QComboBox* videoRendererSelector = nullptr;` | 保存 video renderer selector 相关配置或运行状态。 |
| [L32](../src/apps/controller/pages/SettingsPage.h#L32) | `dragPointerSampleRateSelector` | `QComboBox* dragPointerSampleRateSelector = nullptr;` | 保存 drag pointer sample rate selector 相关配置或运行状态。 |
| [L33](../src/apps/controller/pages/SettingsPage.h#L33) | `remotePasteEnabledSelector` | `QComboBox* remotePasteEnabledSelector = nullptr;` | 保存 remote paste enabled selector 相关配置或运行状态。 |
| [L34](../src/apps/controller/pages/SettingsPage.h#L34) | `clipboardFormatsSelector` | `QComboBox* clipboardFormatsSelector = nullptr;` | 保存 clipboard formats selector 相关配置或运行状态。 |
| [L35](../src/apps/controller/pages/SettingsPage.h#L35) | `clipboardLargeFileLimitSelector` | `QComboBox* clipboardLargeFileLimitSelector = nullptr;` | 保存 clipboard large file limit selector 相关配置或运行状态。 |
| [L36](../src/apps/controller/pages/SettingsPage.h#L36) | `clipboardCacheRetentionSelector` | `QComboBox* clipboardCacheRetentionSelector = nullptr;` | 保存 clipboard cache retention selector 相关配置或运行状态。 |
| [L37](../src/apps/controller/pages/SettingsPage.h#L37) | `clipboardCacheCapacitySelector` | `QComboBox* clipboardCacheCapacitySelector = nullptr;` | 保存 clipboard cache capacity selector 相关配置或运行状态。 |
| [L38](../src/apps/controller/pages/SettingsPage.h#L38) | `clipboardCachePathLabel` | `QLabel* clipboardCachePathLabel = nullptr;` | 保存路径、地址或显示名称：clipboard cache path label。 |
| [L39](../src/apps/controller/pages/SettingsPage.h#L39) | `clipboardCacheUsageLabel` | `QLabel* clipboardCacheUsageLabel = nullptr;` | 保存路径、地址或显示名称：clipboard cache usage label。 |
| [L40](../src/apps/controller/pages/SettingsPage.h#L40) | `clearClipboardCacheButton` | `QPushButton* clearClipboardCacheButton = nullptr;` | 保存 clear clipboard cache button 相关配置或运行状态。 |
| [L41](../src/apps/controller/pages/SettingsPage.h#L41) | `decoderBenchmarkSummary` | `QLabel* decoderBenchmarkSummary = nullptr;` | 保存 decoder benchmark summary 相关配置或运行状态。 |
| [L42](../src/apps/controller/pages/SettingsPage.h#L42) | `decoderBenchmarkButton` | `QPushButton* decoderBenchmarkButton = nullptr;` | 保存 decoder benchmark button 相关配置或运行状态。 |
| [L43](../src/apps/controller/pages/SettingsPage.h#L43) | `encoderBenchmarkSummary` | `QLabel* encoderBenchmarkSummary = nullptr;` | 保存 encoder benchmark summary 相关配置或运行状态。 |
| [L44](../src/apps/controller/pages/SettingsPage.h#L44) | `encoderBenchmarkButton` | `QPushButton* encoderBenchmarkButton = nullptr;` | 保存 encoder benchmark button 相关配置或运行状态。 |
| [L45](../src/apps/controller/pages/SettingsPage.h#L45) | `cameraGalleryBehaviorSelector` | `QComboBox* cameraGalleryBehaviorSelector = nullptr;` | 保存 camera gallery behavior selector 相关配置或运行状态。 |
| [L46](../src/apps/controller/pages/SettingsPage.h#L46) | `cameraDeviceSelector` | `QComboBox* cameraDeviceSelector = nullptr;` | 保存 camera device selector 相关配置或运行状态。 |
| [L47](../src/apps/controller/pages/SettingsPage.h#L47) | `microphoneDeviceSelector` | `QComboBox* microphoneDeviceSelector = nullptr;` | 保存 microphone device selector 相关配置或运行状态。 |
| [L48](../src/apps/controller/pages/SettingsPage.h#L48) | `speakerDeviceSelector` | `QComboBox* speakerDeviceSelector = nullptr;` | 保存 speaker device selector 相关配置或运行状态。 |
| [L49](../src/apps/controller/pages/SettingsPage.h#L49) | `mediaDeviceStatusLabel` | `QLabel* mediaDeviceStatusLabel = nullptr;` | 保存路径、地址或显示名称：media device status label。 |
| [L50](../src/apps/controller/pages/SettingsPage.h#L50) | `refreshMediaDevicesButton` | `QPushButton* refreshMediaDevicesButton = nullptr;` | 保存 refresh media devices button 相关配置或运行状态。 |
| [L51](../src/apps/controller/pages/SettingsPage.h#L51) | `softwareUpdateStatusLabel` | `QLabel* softwareUpdateStatusLabel = nullptr;` | 保存路径、地址或显示名称：software update status label。 |
| [L52](../src/apps/controller/pages/SettingsPage.h#L52) | `softwareUpdateCheckButton` | `QPushButton* softwareUpdateCheckButton = nullptr;` | 保存 software update check button 相关配置或运行状态。 |
| [L81](../src/apps/controller/pages/SettingsPage.h#L81) | `contentLayout_` | `QVBoxLayout* contentLayout_ = nullptr;` | 保存 content layout 相关配置或运行状态。 |
| [L82](../src/apps/controller/pages/SettingsPage.h#L82) | `controls_` | `SettingsPageControls controls_;` | 保存 controls 相关配置或运行状态。 |
| [L83](../src/apps/controller/pages/SettingsPage.h#L83) | `categoryGroup_` | `QButtonGroup* categoryGroup_ = nullptr;` | 保存 category group 相关配置或运行状态。 |
| [L84](../src/apps/controller/pages/SettingsPage.h#L84) | `detailStack_` | `QStackedWidget* detailStack_ = nullptr;` | 保存 detail stack 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L59](../src/apps/controller/pages/SettingsPage.h#L59) | `SettingsPage` | 声明 | `explicit SettingsPage(QWidget* parent = nullptr)` | 更新或应用 settings page 相关逻辑。 |
| [L61](../src/apps/controller/pages/SettingsPage.h#L61) | `DetailStack` | 声明 | `QStackedWidget* DetailStack() const` | 实现 detail stack 对应的业务或工具逻辑。 |
| [L62](../src/apps/controller/pages/SettingsPage.h#L62) | `Controls` | 声明 | `SettingsPageControls& Controls()` | 实现 controls 对应的业务或工具逻辑。 |
| [L63](../src/apps/controller/pages/SettingsPage.h#L63) | `Controls` | 声明 | `const SettingsPageControls& Controls() const` | 实现 controls 对应的业务或工具逻辑。 |
| [L64](../src/apps/controller/pages/SettingsPage.h#L64) | `RefreshClipboardCacheCapacityOptions` | 声明 | `void RefreshClipboardCacheCapacityOptions()` | 刷新 refresh clipboard cache capacity options 相关逻辑。 |
| [L65](../src/apps/controller/pages/SettingsPage.h#L65) | `CurrentCategory` | 声明 | `int CurrentCategory() const` | 实现 current category 对应的业务或工具逻辑。 |
| [L68](../src/apps/controller/pages/SettingsPage.h#L68) | `CategoryChanged` | 声明 | `void CategoryChanged(int index)` | 实现 category changed 对应的业务或工具逻辑。 |
| [L69](../src/apps/controller/pages/SettingsPage.h#L69) | `ClipboardCacheBaseDirectoryChanged` | 声明 | `void ClipboardCacheBaseDirectoryChanged()` | 实现 clipboard cache base directory changed 对应的业务或工具逻辑。 |
| [L70](../src/apps/controller/pages/SettingsPage.h#L70) | `SoftwareUpdateRequested` | 声明 | `void SoftwareUpdateRequested()` | 实现 software update requested 对应的业务或工具逻辑。 |
| [L73](../src/apps/controller/pages/SettingsPage.h#L73) | `ApplyClipboardCacheBaseDirectory` | 声明 | `void ApplyClipboardCacheBaseDirectory(const QString& selectedDirectory)` | 更新或应用 apply clipboard cache base directory 相关逻辑。 |
| [L74](../src/apps/controller/pages/SettingsPage.h#L74) | `BuildAudioVideoSettingsPage` | 声明 | `void BuildAudioVideoSettingsPage()` | 创建或初始化 build audio video settings page 相关逻辑。 |
| [L75](../src/apps/controller/pages/SettingsPage.h#L75) | `BuildFileTransferSettingsPage` | 声明 | `void BuildFileTransferSettingsPage()` | 创建或初始化 build file transfer settings page 相关逻辑。 |
| [L76](../src/apps/controller/pages/SettingsPage.h#L76) | `BuildGeneralSettingsPage` | 声明 | `void BuildGeneralSettingsPage()` | 创建或初始化 build general settings page 相关逻辑。 |
| [L77](../src/apps/controller/pages/SettingsPage.h#L77) | `BuildRemoteDesktopSettingsPage` | 声明 | `void BuildRemoteDesktopSettingsPage()` | 创建或初始化 build remote desktop settings page 相关逻辑。 |
| [L78](../src/apps/controller/pages/SettingsPage.h#L78) | `BuildRemotePasteSettingsPage` | 声明 | `void BuildRemotePasteSettingsPage()` | 创建或初始化 build remote paste settings page 相关逻辑。 |
| [L79](../src/apps/controller/pages/SettingsPage.h#L79) | `BuildShortcutSettingsPage` | 声明 | `void BuildShortcutSettingsPage()` | 创建或初始化 build shortcut settings page 相关逻辑。 |

## `src/apps/controller/pages/SettingsPage.Media.cpp`

[打开源码](../src/apps/controller/pages/SettingsPage.Media.cpp) · **文件作用：** 实现 settings page media 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L24](../src/apps/controller/pages/SettingsPage.Media.cpp#L24) | `SettingsPage::BuildAudioVideoSettingsPage` | 定义 | `void SettingsPage::BuildAudioVideoSettingsPage()` | 创建或初始化 build audio video settings page 相关逻辑。 |

## `src/apps/controller/pages/SettingsPage.RemoteDesktop.cpp`

[打开源码](../src/apps/controller/pages/SettingsPage.RemoteDesktop.cpp) · **文件作用：** 实现 settings page remote desktop 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L30](../src/apps/controller/pages/SettingsPage.RemoteDesktop.cpp#L30) | `ConfigurePreferenceSelector` | 定义 | `void ConfigurePreferenceSelector(QComboBox* selector, const QSettings& settings, const char* settingKey, const bool renderer)` | 更新或应用 configure preference selector 相关逻辑。 |
| [L61](../src/apps/controller/pages/SettingsPage.RemoteDesktop.cpp#L61) | `AddBenchmarkRow` | 定义 | `void AddBenchmarkRow(QWidget* page, QVBoxLayout* layout, const QString& title, const QString& hint, QLabel*& summary, QPushButton*& button, const QString& summaryObjectName, const QSizePolicy::Policy horizontalPolicy)` | 实现 add benchmark row 对应的业务或工具逻辑。 |
| [L107](../src/apps/controller/pages/SettingsPage.RemoteDesktop.cpp#L107) | `SettingsPage::BuildRemoteDesktopSettingsPage` | 定义 | `void SettingsPage::BuildRemoteDesktopSettingsPage()` | 创建或初始化 build remote desktop settings page 相关逻辑。 |

## `src/apps/controller/pages/SettingsPage.RemotePaste.cpp`

[打开源码](../src/apps/controller/pages/SettingsPage.RemotePaste.cpp) · **文件作用：** 实现 settings page remote paste 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L36](../src/apps/controller/pages/SettingsPage.RemotePaste.cpp#L36) | `SettingsPage::BuildRemotePasteSettingsPage` | 定义 | `void SettingsPage::BuildRemotePasteSettingsPage()` | 创建或初始化 build remote paste settings page 相关逻辑。 |
| [L261](../src/apps/controller/pages/SettingsPage.RemotePaste.cpp#L261) | `SettingsPage::RefreshClipboardCacheCapacityOptions` | 定义 | `void SettingsPage::RefreshClipboardCacheCapacityOptions()` | 刷新 refresh clipboard cache capacity options 相关逻辑。 |
| [L311](../src/apps/controller/pages/SettingsPage.RemotePaste.cpp#L311) | `SettingsPage::ApplyClipboardCacheBaseDirectory` | 定义 | `void SettingsPage::ApplyClipboardCacheBaseDirectory( const QString& selectedDirectory)` | 更新或应用 apply clipboard cache base directory 相关逻辑。 |

## `src/apps/controller/pages/SettingsPage.Shortcuts.cpp`

[打开源码](../src/apps/controller/pages/SettingsPage.Shortcuts.cpp) · **文件作用：** 实现 settings page shortcuts 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L14](../src/apps/controller/pages/SettingsPage.Shortcuts.cpp#L14) | `SettingsPage::BuildShortcutSettingsPage` | 定义 | `void SettingsPage::BuildShortcutSettingsPage()` | 创建或初始化 build shortcut settings page 相关逻辑。 |

## `src/apps/controller/pages/SettingsPageUi.cpp`

[打开源码](../src/apps/controller/pages/SettingsPageUi.cpp) · **文件作用：** 实现 settings page ui 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L15](../src/apps/controller/pages/SettingsPageUi.cpp#L15) | `AddSettingsDetailHeader` | 定义 | `void AddSettingsDetailHeader(QVBoxLayout* layout, QWidget* parent, const QString& title, const QString& description)` | 实现 add settings detail header 对应的业务或工具逻辑。 |
| [L30](../src/apps/controller/pages/SettingsPageUi.cpp#L30) | `CreateSettingsRow` | 定义 | `std::pair<QFrame*, QHBoxLayout*> CreateSettingsRow( QWidget* parent, const QString& title, const QString& description)` | 创建或初始化 create settings row 相关逻辑。 |
| [L58](../src/apps/controller/pages/SettingsPageUi.cpp#L58) | `CreatePathSettingsRow` | 定义 | `std::pair<QFrame*, QHBoxLayout*> CreatePathSettingsRow( QWidget* parent, const QString& title, const QString& description)` | 创建或初始化 create path settings row 相关逻辑。 |

## `src/apps/controller/pages/SettingsPageUi.h`

[打开源码](../src/apps/controller/pages/SettingsPageUi.h) · **文件作用：** 声明 settings page ui 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L8](../src/apps/controller/pages/SettingsPageUi.h#L8) | `QFrame` | class | 定义 QFrame 的 class 类型和相关状态。 |
| [L9](../src/apps/controller/pages/SettingsPageUi.h#L9) | `QHBoxLayout` | class | 定义 QHBoxLayout 的 class 类型和相关状态。 |
| [L10](../src/apps/controller/pages/SettingsPageUi.h#L10) | `QString` | class | 定义 QString 的 class 类型和相关状态。 |
| [L11](../src/apps/controller/pages/SettingsPageUi.h#L11) | `QVBoxLayout` | class | 定义 QVBoxLayout 的 class 类型和相关状态。 |
| [L12](../src/apps/controller/pages/SettingsPageUi.h#L12) | `QWidget` | class | 定义 QWidget 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L8](../src/apps/controller/pages/SettingsPageUi.h#L8) | `QFrame` | `class QFrame;` | 保存媒体帧、图像或缓冲资源：q frame。 |
| [L9](../src/apps/controller/pages/SettingsPageUi.h#L9) | `QHBoxLayout` | `class QHBoxLayout;` | 保存 qh box layout 相关配置或运行状态。 |
| [L10](../src/apps/controller/pages/SettingsPageUi.h#L10) | `QString` | `class QString;` | 保存 q string 相关配置或运行状态。 |
| [L11](../src/apps/controller/pages/SettingsPageUi.h#L11) | `QVBoxLayout` | `class QVBoxLayout;` | 保存 qv box layout 相关配置或运行状态。 |
| [L12](../src/apps/controller/pages/SettingsPageUi.h#L12) | `QWidget` | `class QWidget;` | 保存 q widget 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L16](../src/apps/controller/pages/SettingsPageUi.h#L16) | `AddSettingsDetailHeader` | 声明 | `void AddSettingsDetailHeader(QVBoxLayout* layout, QWidget* parent, const QString& title, const QString& description)` | 实现 add settings detail header 对应的业务或工具逻辑。 |
| [L21](../src/apps/controller/pages/SettingsPageUi.h#L21) | `CreateSettingsRow` | 声明 | `std::pair<QFrame*, QHBoxLayout*> CreateSettingsRow( QWidget* parent, const QString& title, const QString& description)` | 创建或初始化 create settings row 相关逻辑。 |
| [L26](../src/apps/controller/pages/SettingsPageUi.h#L26) | `CreatePathSettingsRow` | 声明 | `std::pair<QFrame*, QHBoxLayout*> CreatePathSettingsRow( QWidget* parent, const QString& title, const QString& description)` | 创建或初始化 create path settings row 相关逻辑。 |

## `src/apps/controller/RemoteCComboBox.cpp`

[打开源码](../src/apps/controller/RemoteCComboBox.cpp) · **文件作用：** 实现 remote c combo box 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L33](../src/apps/controller/RemoteCComboBox.cpp#L33) | `RemoteCComboItemDelegate` | class | 定义 RemoteCComboItemDelegate 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L37](../src/apps/controller/RemoteCComboBox.cpp#L37) | `sizeHint` | 定义 | `QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override` | 实现 size hint 对应的业务或工具逻辑。 |
| [L45](../src/apps/controller/RemoteCComboBox.cpp#L45) | `paint` | 定义 | `void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override` | 准备或呈现 paint 相关逻辑。 |
| [L99](../src/apps/controller/RemoteCComboBox.cpp#L99) | `RemoteCComboBox::RemoteCComboBox` | 定义 | `RemoteCComboBox::RemoteCComboBox(QWidget* parent) : QComboBox(parent), arrowAnimation_(new QVariantAnimation(this))` | 构造并初始化 RemoteCComboBox 实例。 |
| [L186](../src/apps/controller/RemoteCComboBox.cpp#L186) | `RemoteCComboBox::RefreshThemeStyle` | 定义 | `void RemoteCComboBox::RefreshThemeStyle()` | 刷新 refresh theme style 相关逻辑。 |
| [L254](../src/apps/controller/RemoteCComboBox.cpp#L254) | `RemoteCComboBox::SetWheelSelectionEnabled` | 定义 | `void RemoteCComboBox::SetWheelSelectionEnabled(bool enabled)` | 更新或应用 set wheel selection enabled 相关逻辑。 |
| [L259](../src/apps/controller/RemoteCComboBox.cpp#L259) | `RemoteCComboBox::wheelEvent` | 定义 | `void RemoteCComboBox::wheelEvent(QWheelEvent* event)` | 实现 wheel event 对应的业务或工具逻辑。 |
| [L268](../src/apps/controller/RemoteCComboBox.cpp#L268) | `RemoteCComboBox::paintEvent` | 定义 | `void RemoteCComboBox::paintEvent(QPaintEvent*)` | 准备或呈现 paint event 相关逻辑。 |
| [L295](../src/apps/controller/RemoteCComboBox.cpp#L295) | `RemoteCComboBox::showPopup` | 定义 | `void RemoteCComboBox::showPopup()` | 实现 show popup 对应的业务或工具逻辑。 |
| [L341](../src/apps/controller/RemoteCComboBox.cpp#L341) | `RemoteCComboBox::hidePopup` | 定义 | `void RemoteCComboBox::hidePopup()` | 实现 hide popup 对应的业务或工具逻辑。 |
| [L349](../src/apps/controller/RemoteCComboBox.cpp#L349) | `RemoteCComboBox::eventFilter` | 定义 | `bool RemoteCComboBox::eventFilter(QObject* watched, QEvent* event)` | 实现 event filter 对应的业务或工具逻辑。 |
| [L374](../src/apps/controller/RemoteCComboBox.cpp#L374) | `RemoteCComboBox::EnsurePopup` | 定义 | `void RemoteCComboBox::EnsurePopup()` | 实现 ensure popup 对应的业务或工具逻辑。 |
| [L446](../src/apps/controller/RemoteCComboBox.cpp#L446) | `RemoteCComboBox::SetPopupOpen` | 定义 | `void RemoteCComboBox::SetPopupOpen(bool open)` | 更新或应用 set popup open 相关逻辑。 |

## `src/apps/controller/RemoteCComboBox.h`

[打开源码](../src/apps/controller/RemoteCComboBox.h) · **文件作用：** 声明 remote c combo box 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L8](../src/apps/controller/RemoteCComboBox.h#L8) | `QVariantAnimation` | class | 定义 QVariantAnimation 的 class 类型和相关状态。 |
| [L9](../src/apps/controller/RemoteCComboBox.h#L9) | `QFrame` | class | 定义 QFrame 的 class 类型和相关状态。 |
| [L10](../src/apps/controller/RemoteCComboBox.h#L10) | `QListWidget` | class | 定义 QListWidget 的 class 类型和相关状态。 |
| [L11](../src/apps/controller/RemoteCComboBox.h#L11) | `QWheelEvent` | class | 定义 QWheelEvent 的 class 类型和相关状态。 |
| [L15](../src/apps/controller/RemoteCComboBox.h#L15) | `RemoteCComboBox` | class | 定义 RemoteCComboBox 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L8](../src/apps/controller/RemoteCComboBox.h#L8) | `QVariantAnimation` | `class QVariantAnimation;` | 保存 q variant animation 相关配置或运行状态。 |
| [L9](../src/apps/controller/RemoteCComboBox.h#L9) | `QFrame` | `class QFrame;` | 保存媒体帧、图像或缓冲资源：q frame。 |
| [L10](../src/apps/controller/RemoteCComboBox.h#L10) | `QListWidget` | `class QListWidget;` | 保存 q list widget 相关配置或运行状态。 |
| [L11](../src/apps/controller/RemoteCComboBox.h#L11) | `QWheelEvent` | `class QWheelEvent;` | 保存 q wheel event 相关配置或运行状态。 |
| [L32](../src/apps/controller/RemoteCComboBox.h#L32) | `arrowAnimation_` | `QVariantAnimation* arrowAnimation_ = nullptr;` | 保存 arrow animation 相关配置或运行状态。 |
| [L33](../src/apps/controller/RemoteCComboBox.h#L33) | `popup_` | `QFrame* popup_ = nullptr;` | 保存 popup 相关配置或运行状态。 |
| [L34](../src/apps/controller/RemoteCComboBox.h#L34) | `popupList_` | `QListWidget* popupList_ = nullptr;` | 保存 popup list 相关配置或运行状态。 |
| [L35](../src/apps/controller/RemoteCComboBox.h#L35) | `lightComboStyleSheet_` | `QString lightComboStyleSheet_;` | 保存 light combo style sheet 相关配置或运行状态。 |
| [L36](../src/apps/controller/RemoteCComboBox.h#L36) | `lightPopupStyleSheet_` | `QString lightPopupStyleSheet_;` | 保存 light popup style sheet 相关配置或运行状态。 |
| [L37](../src/apps/controller/RemoteCComboBox.h#L37) | `darkTheme_` | `bool darkTheme_ = false;` | 保存 dark theme 相关配置或运行状态。 |
| [L38](../src/apps/controller/RemoteCComboBox.h#L38) | `wheelSelectionEnabled_` | `bool wheelSelectionEnabled_ = true;` | 保存能力或开关状态：wheel selection enabled。 |
| [L39](../src/apps/controller/RemoteCComboBox.h#L39) | `arrowRotation_` | `qreal arrowRotation_ = 0.0;` | 保存 arrow rotation 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L17](../src/apps/controller/RemoteCComboBox.h#L17) | `RemoteCComboBox` | 声明 | `explicit RemoteCComboBox(QWidget* parent = nullptr)` | 实现 remote c combo box 对应的业务或工具逻辑。 |
| [L18](../src/apps/controller/RemoteCComboBox.h#L18) | `RefreshThemeStyle` | 声明 | `void RefreshThemeStyle()` | 刷新 refresh theme style 相关逻辑。 |
| [L19](../src/apps/controller/RemoteCComboBox.h#L19) | `SetWheelSelectionEnabled` | 声明 | `void SetWheelSelectionEnabled(bool enabled)` | 更新或应用 set wheel selection enabled 相关逻辑。 |
| [L22](../src/apps/controller/RemoteCComboBox.h#L22) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L23](../src/apps/controller/RemoteCComboBox.h#L23) | `paintEvent` | 声明 | `void paintEvent(QPaintEvent* event) override` | 准备或呈现 paint event 相关逻辑。 |
| [L24](../src/apps/controller/RemoteCComboBox.h#L24) | `wheelEvent` | 声明 | `void wheelEvent(QWheelEvent* event) override` | 实现 wheel event 对应的业务或工具逻辑。 |
| [L25](../src/apps/controller/RemoteCComboBox.h#L25) | `showPopup` | 声明 | `void showPopup() override` | 实现 show popup 对应的业务或工具逻辑。 |
| [L26](../src/apps/controller/RemoteCComboBox.h#L26) | `hidePopup` | 声明 | `void hidePopup() override` | 实现 hide popup 对应的业务或工具逻辑。 |
| [L29](../src/apps/controller/RemoteCComboBox.h#L29) | `EnsurePopup` | 声明 | `void EnsurePopup()` | 实现 ensure popup 对应的业务或工具逻辑。 |
| [L30](../src/apps/controller/RemoteCComboBox.h#L30) | `SetPopupOpen` | 声明 | `void SetPopupOpen(bool open)` | 更新或应用 set popup open 相关逻辑。 |

## `src/apps/controller/RemoteCDialog.cpp`

[打开源码](../src/apps/controller/RemoteCDialog.cpp) · **文件作用：** 实现 remote c dialog 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L200](../src/apps/controller/RemoteCDialog.cpp#L200) | `DialogDragHeader` | class | 定义 DialogDragHeader 的 class 类型和相关状态。 |
| [L224](../src/apps/controller/RemoteCDialog.cpp#L224) | `DialogActivityIndicator` | class | 定义 DialogActivityIndicator 的 class 类型和相关状态。 |
| [L293](../src/apps/controller/RemoteCDialog.cpp#L293) | `DialogProgressBar` | class | QProgressBar may delegate value transitions to the native Windows style. For a transfer dialog that makes the painted chunk visibly trail the byte counter when updates arrive qu... |
| [L339](../src/apps/controller/RemoteCDialog.cpp#L339) | `DialogOptionCheckBox` | class | 定义 DialogOptionCheckBox 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L171](../src/apps/controller/RemoteCDialog.cpp#L171) | `BadgeText` | 定义 | `QString BadgeText(RemoteCDialog::Tone tone)` | 实现 badge text 对应的业务或工具逻辑。 |
| [L185](../src/apps/controller/RemoteCDialog.cpp#L185) | `BadgeStyle` | 定义 | `QString BadgeStyle(RemoteCDialog::Tone tone)` | 实现 badge style 对应的业务或工具逻辑。 |
| [L202](../src/apps/controller/RemoteCDialog.cpp#L202) | `DialogDragHeader` | 定义 | `explicit DialogDragHeader(QDialog* dialog, QWidget* parent) : QWidget(parent), dialog_(dialog)` | 实现 dialog drag header 对应的业务或工具逻辑。 |
| [L209](../src/apps/controller/RemoteCDialog.cpp#L209) | `mousePressEvent` | 定义 | `void mousePressEvent(QMouseEvent* event) override` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L226](../src/apps/controller/RemoteCDialog.cpp#L226) | `DialogActivityIndicator` | 定义 | `explicit DialogActivityIndicator(QWidget* parent) : QWidget(parent)` | 实现 dialog activity indicator 对应的业务或工具逻辑。 |
| [L239](../src/apps/controller/RemoteCDialog.cpp#L239) | `showEvent` | 定义 | `void showEvent(QShowEvent* event) override` | 实现 show event 对应的业务或工具逻辑。 |
| [L246](../src/apps/controller/RemoteCDialog.cpp#L246) | `hideEvent` | 定义 | `void hideEvent(QHideEvent* event) override` | 实现 hide event 对应的业务或工具逻辑。 |
| [L252](../src/apps/controller/RemoteCDialog.cpp#L252) | `paintEvent` | 定义 | `void paintEvent(QPaintEvent*) override` | 准备或呈现 paint event 相关逻辑。 |
| [L295](../src/apps/controller/RemoteCDialog.cpp#L295) | `DialogProgressBar` | 定义 | `explicit DialogProgressBar(QWidget* parent = nullptr) : QProgressBar(parent)` | 实现 dialog progress bar 对应的业务或工具逻辑。 |
| [L302](../src/apps/controller/RemoteCDialog.cpp#L302) | `paintEvent` | 定义 | `void paintEvent(QPaintEvent*) override` | 准备或呈现 paint event 相关逻辑。 |
| [L341](../src/apps/controller/RemoteCDialog.cpp#L341) | `DialogOptionCheckBox` | 定义 | `explicit DialogOptionCheckBox( const QString& text, QWidget* parent = nullptr) : QCheckBox(text, parent)` | 实现 dialog option check box 对应的业务或工具逻辑。 |
| [L350](../src/apps/controller/RemoteCDialog.cpp#L350) | `paintEvent` | 定义 | `void paintEvent(QPaintEvent*) override` | 准备或呈现 paint event 相关逻辑。 |
| [L408](../src/apps/controller/RemoteCDialog.cpp#L408) | `RemoteCDialog::Confirm` | 定义 | `bool RemoteCDialog::Confirm(QWidget* parent, const QString& title, const QString& message, const QString& confirmText, const QString& cancelText, Tone tone, bool requestAttention, bool nonActivating, std::uintptr_t na...` | 实现 confirm 对应的业务或工具逻辑。 |
| [L427](../src/apps/controller/RemoteCDialog.cpp#L427) | `RemoteCDialog::ConfirmWithOption` | 定义 | `bool RemoteCDialog::ConfirmWithOption( QWidget* parent, const QString& title, const QString& message, const QString& confirmText, const QString& cancelText, const QString& optionText, bool optionInitiallyChecked, bool...` | 实现 confirm with option 对应的业务或工具逻辑。 |
| [L455](../src/apps/controller/RemoteCDialog.cpp#L455) | `RemoteCDialog::Alert` | 定义 | `void RemoteCDialog::Alert(QWidget* parent, const QString& title, const QString& message, const QString& buttonText, Tone tone, bool requestAttention)` | 实现 alert 对应的业务或工具逻辑。 |
| [L467](../src/apps/controller/RemoteCDialog.cpp#L467) | `RemoteCDialog::CreateStatus` | 定义 | `RemoteCDialog* RemoteCDialog::CreateStatus( QWidget* parent, const QString& title, const QString& message, const QString& buttonText, Tone tone)` | 创建或初始化 create status 相关逻辑。 |
| [L502](../src/apps/controller/RemoteCDialog.cpp#L502) | `RemoteCDialog::SetContent` | 定义 | `void RemoteCDialog::SetContent(const QString& title, const QString& message, const QString& buttonText, Tone tone)` | 更新或应用 set content 相关逻辑。 |
| [L552](../src/apps/controller/RemoteCDialog.cpp#L552) | `RemoteCDialog::SetStatusActionHandler` | 定义 | `void RemoteCDialog::SetStatusActionHandler(std::function<void()> handler)` | 更新或应用 set status action handler 相关逻辑。 |
| [L557](../src/apps/controller/RemoteCDialog.cpp#L557) | `RemoteCDialog::SetProgress` | 定义 | `void RemoteCDialog::SetProgress(double progress)` | 更新或应用 set progress 相关逻辑。 |
| [L569](../src/apps/controller/RemoteCDialog.cpp#L569) | `RemoteCDialog::SetNonActivatingWindow` | 定义 | `void RemoteCDialog::SetNonActivatingWindow(bool enabled)` | 更新或应用 set non activating window 相关逻辑。 |
| [L580](../src/apps/controller/RemoteCDialog.cpp#L580) | `RemoteCDialog::SetNativeAnchorWindow` | 定义 | `void RemoteCDialog::SetNativeAnchorWindow(std::uintptr_t windowHandle)` | 更新或应用 set native anchor window 相关逻辑。 |
| [L585](../src/apps/controller/RemoteCDialog.cpp#L585) | `RemoteCDialog::ApplyNonActivatingNativeStyle` | 定义 | `void RemoteCDialog::ApplyNonActivatingNativeStyle()` | 更新或应用 apply non activating native style 相关逻辑。 |
| [L606](../src/apps/controller/RemoteCDialog.cpp#L606) | `RemoteCDialog::RemoteCDialog` | 定义 | `RemoteCDialog::RemoteCDialog(QWidget* parent, const QString& title, const QString& message, const QString& confirmText, const QString& cancelText, Tone tone, bool showCancel, bool requestAttention) : QDialog(parent ? ...` | 构造并初始化 RemoteCDialog 实例。 |
| [L751](../src/apps/controller/RemoteCDialog.cpp#L751) | `RemoteCDialog::eventFilter` | 定义 | `bool RemoteCDialog::eventFilter(QObject* watched, QEvent* event)` | 实现 event filter 对应的业务或工具逻辑。 |
| [L800](../src/apps/controller/RemoteCDialog.cpp#L800) | `RemoteCDialog::showEvent` | 定义 | `void RemoteCDialog::showEvent(QShowEvent* event)` | 实现 show event 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteCDialog.h`

[打开源码](../src/apps/controller/RemoteCDialog.h) · **文件作用：** 声明 remote c dialog 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L13](../src/apps/controller/RemoteCDialog.h#L13) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L14](../src/apps/controller/RemoteCDialog.h#L14) | `QFrame` | class | 定义 QFrame 的 class 类型和相关状态。 |
| [L15](../src/apps/controller/RemoteCDialog.h#L15) | `QPushButton` | class | 定义 QPushButton 的 class 类型和相关状态。 |
| [L16](../src/apps/controller/RemoteCDialog.h#L16) | `QProgressBar` | class | 定义 QProgressBar 的 class 类型和相关状态。 |
| [L20](../src/apps/controller/RemoteCDialog.h#L20) | `RemoteCDialog` | class | 定义 RemoteCDialog 的 class 类型和相关状态。 |
| [L22](../src/apps/controller/RemoteCDialog.h#L22) | `Tone` | enum class | 定义 Tone 的 enum class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L13](../src/apps/controller/RemoteCDialog.h#L13) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L14](../src/apps/controller/RemoteCDialog.h#L14) | `QFrame` | `class QFrame;` | 保存媒体帧、图像或缓冲资源：q frame。 |
| [L15](../src/apps/controller/RemoteCDialog.h#L15) | `QPushButton` | `class QPushButton;` | 保存 q push button 相关配置或运行状态。 |
| [L16](../src/apps/controller/RemoteCDialog.h#L16) | `QProgressBar` | `class QProgressBar;` | 保存 q progress bar 相关配置或运行状态。 |
| [L96](../src/apps/controller/RemoteCDialog.h#L96) | `shown_` | `bool shown_ = false;` | 保存 shown 相关配置或运行状态。 |
| [L97](../src/apps/controller/RemoteCDialog.h#L97) | `statusDialog_` | `bool statusDialog_ = false;` | 保存 status dialog 相关配置或运行状态。 |
| [L98](../src/apps/controller/RemoteCDialog.h#L98) | `requestAttention_` | `bool requestAttention_ = false;` | 保存 request attention 相关配置或运行状态。 |
| [L99](../src/apps/controller/RemoteCDialog.h#L99) | `nonActivatingWindow_` | `bool nonActivatingWindow_ = false;` | 保存 non activating window 相关配置或运行状态。 |
| [L100](../src/apps/controller/RemoteCDialog.h#L100) | `nativeAnchorWindow_` | `std::uintptr_t nativeAnchorWindow_ = 0;` | 保存 native anchor window 相关配置或运行状态。 |
| [L101](../src/apps/controller/RemoteCDialog.h#L101) | `centerOnAnchorScreen_` | `bool centerOnAnchorScreen_ = false;` | 保存 center on anchor screen 相关配置或运行状态。 |
| [L102](../src/apps/controller/RemoteCDialog.h#L102) | `nonActivatingDragActive_` | `bool nonActivatingDragActive_ = false;` | 保存能力或开关状态：non activating drag active。 |
| [L103](../src/apps/controller/RemoteCDialog.h#L103) | `nonActivatingDragStartGlobal_` | `QPoint nonActivatingDragStartGlobal_;` | 保存 non activating drag start global 相关配置或运行状态。 |
| [L104](../src/apps/controller/RemoteCDialog.h#L104) | `nonActivatingDragStartTopLeft_` | `QPoint nonActivatingDragStartTopLeft_;` | 保存 non activating drag start top left 相关配置或运行状态。 |
| [L105](../src/apps/controller/RemoteCDialog.h#L105) | `card_` | `QFrame* card_ = nullptr;` | 保存 card 相关配置或运行状态。 |
| [L106](../src/apps/controller/RemoteCDialog.h#L106) | `badgeLabel_` | `QLabel* badgeLabel_ = nullptr;` | 保存路径、地址或显示名称：badge label。 |
| [L107](../src/apps/controller/RemoteCDialog.h#L107) | `titleLabel_` | `QLabel* titleLabel_ = nullptr;` | 保存路径、地址或显示名称：title label。 |
| [L108](../src/apps/controller/RemoteCDialog.h#L108) | `messageLabel_` | `QLabel* messageLabel_ = nullptr;` | 保存路径、地址或显示名称：message label。 |
| [L109](../src/apps/controller/RemoteCDialog.h#L109) | `activityIndicator_` | `QWidget* activityIndicator_ = nullptr;` | 保存 activity indicator 相关配置或运行状态。 |
| [L110](../src/apps/controller/RemoteCDialog.h#L110) | `progressBar_` | `QProgressBar* progressBar_ = nullptr;` | 保存 progress bar 相关配置或运行状态。 |
| [L111](../src/apps/controller/RemoteCDialog.h#L111) | `confirmButton_` | `QPushButton* confirmButton_ = nullptr;` | 保存 confirm button 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L29](../src/apps/controller/RemoteCDialog.h#L29) | `Confirm` | 声明 | `static bool Confirm(QWidget* parent, const QString& title, const QString& message, const QString& confirmText, const QString& cancelText = QStringLiteral("取消"), Tone tone = Tone::kQuestion, bool requestAttention = fal...` | 实现 confirm 对应的业务或工具逻辑。 |
| [L40](../src/apps/controller/RemoteCDialog.h#L40) | `ConfirmWithOption` | 声明 | `static bool ConfirmWithOption( QWidget* parent, const QString& title, const QString& message, const QString& confirmText, const QString& cancelText, const QString& optionText, bool optionInitiallyChecked, bool* option...` | 实现 confirm with option 对应的业务或工具逻辑。 |
| [L52](../src/apps/controller/RemoteCDialog.h#L52) | `Alert` | 声明 | `static void Alert(QWidget* parent, const QString& title, const QString& message, const QString& buttonText = QStringLiteral("知道了"), Tone tone = Tone::kWarning, bool requestAttention = false)` | 实现 alert 对应的业务或工具逻辑。 |
| [L61](../src/apps/controller/RemoteCDialog.h#L61) | `CreateStatus` | 声明 | `static RemoteCDialog* CreateStatus( QWidget* parent, const QString& title, const QString& message, const QString& buttonText = QStringLiteral("隐藏提示"), Tone tone = Tone::kWarning)` | Creates a movable, non-modal status dialog using the same visual language as room-join and control approval prompts. The parent owns it. |
| [L68](../src/apps/controller/RemoteCDialog.h#L68) | `SetContent` | 声明 | `void SetContent(const QString& title, const QString& message, const QString& buttonText, Tone tone)` | 更新或应用 set content 相关逻辑。 |
| [L72](../src/apps/controller/RemoteCDialog.h#L72) | `SetStatusActionHandler` | 声明 | `void SetStatusActionHandler(std::function<void()> handler)` | 更新或应用 set status action handler 相关逻辑。 |
| [L73](../src/apps/controller/RemoteCDialog.h#L73) | `SetProgress` | 声明 | `void SetProgress(double progress)` | 更新或应用 set progress 相关逻辑。 |
| [L77](../src/apps/controller/RemoteCDialog.h#L77) | `SetNonActivatingWindow` | 声明 | `void SetNonActivatingWindow(bool enabled)` | Keeps a status window above a foreign target (for example Explorer) without activating this dialog or bringing its Qt owner to the front. The dialog remains mouse-draggable thro... |
| [L78](../src/apps/controller/RemoteCDialog.h#L78) | `SetNativeAnchorWindow` | 声明 | `void SetNativeAnchorWindow(std::uintptr_t windowHandle)` | 更新或应用 set native anchor window 相关逻辑。 |
| [L81](../src/apps/controller/RemoteCDialog.h#L81) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L82](../src/apps/controller/RemoteCDialog.h#L82) | `showEvent` | 声明 | `void showEvent(QShowEvent* event) override` | 实现 show event 对应的业务或工具逻辑。 |
| [L85](../src/apps/controller/RemoteCDialog.h#L85) | `ApplyNonActivatingNativeStyle` | 声明 | `void ApplyNonActivatingNativeStyle()` | 更新或应用 apply non activating native style 相关逻辑。 |
| [L87](../src/apps/controller/RemoteCDialog.h#L87) | `RemoteCDialog` | 声明 | `RemoteCDialog(QWidget* parent, const QString& title, const QString& message, const QString& confirmText, const QString& cancelText, Tone tone, bool showCancel, bool requestAttention)` | 实现 remote c dialog 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteCToast.cpp`

[打开源码](../src/apps/controller/RemoteCToast.cpp) · **文件作用：** 实现 remote c toast 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L23](../src/apps/controller/RemoteCToast.cpp#L23) | `ToastHost` | 定义 | `QWidget* ToastHost(QWidget* parent)` | 实现 toast host 对应的业务或工具逻辑。 |
| [L37](../src/apps/controller/RemoteCToast.cpp#L37) | `AnimationLevel` | 定义 | `int AnimationLevel()` | 实现 animation level 对应的业务或工具逻辑。 |
| [L48](../src/apps/controller/RemoteCToast.cpp#L48) | `RemoteCToast::Show` | 定义 | `void RemoteCToast::Show(QWidget* parent, const QString& message, Tone tone)` | 实现 show 对应的业务或工具逻辑。 |
| [L71](../src/apps/controller/RemoteCToast.cpp#L71) | `RemoteCToast::ShowAbove` | 定义 | `void RemoteCToast::ShowAbove(QWidget* anchor, const QString& message, Tone tone)` | 实现 show above 对应的业务或工具逻辑。 |
| [L94](../src/apps/controller/RemoteCToast.cpp#L94) | `RemoteCToast::ShowAtGlobalCenter` | 定义 | `void RemoteCToast::ShowAtGlobalCenter( QWidget* parent, const QPoint& globalCenter, const QString& message, Tone tone, bool prominent)` | 实现 show at global center 对应的业务或工具逻辑。 |
| [L120](../src/apps/controller/RemoteCToast.cpp#L120) | `RemoteCToast::RemoteCToast` | 定义 | `RemoteCToast::RemoteCToast(QWidget* host) : QFrame(host), host_(host)` | 构造并初始化 RemoteCToast 实例。 |
| [L151](../src/apps/controller/RemoteCToast.cpp#L151) | `RemoteCToast::ShowMessage` | 定义 | `void RemoteCToast::ShowMessage(const QString& message, Tone tone, QWidget* anchor, const QPoint* globalCenter, bool prominent)` | 实现 show message 对应的业务或工具逻辑。 |
| [L247](../src/apps/controller/RemoteCToast.cpp#L247) | `RemoteCToast::FadeOut` | 定义 | `void RemoteCToast::FadeOut()` | 实现 fade out 对应的业务或工具逻辑。 |
| [L267](../src/apps/controller/RemoteCToast.cpp#L267) | `RemoteCToast::Reposition` | 定义 | `void RemoteCToast::Reposition()` | 实现 reposition 对应的业务或工具逻辑。 |
| [L300](../src/apps/controller/RemoteCToast.cpp#L300) | `RemoteCToast::eventFilter` | 定义 | `bool RemoteCToast::eventFilter(QObject* watched, QEvent* event)` | 实现 event filter 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteCToast.h`

[打开源码](../src/apps/controller/RemoteCToast.h) · **文件作用：** 声明 remote c toast 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L11](../src/apps/controller/RemoteCToast.h#L11) | `QEvent` | class | 定义 QEvent 的 class 类型和相关状态。 |
| [L12](../src/apps/controller/RemoteCToast.h#L12) | `QGraphicsOpacityEffect` | class | 定义 QGraphicsOpacityEffect 的 class 类型和相关状态。 |
| [L13](../src/apps/controller/RemoteCToast.h#L13) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L14](../src/apps/controller/RemoteCToast.h#L14) | `QPropertyAnimation` | class | 定义 QPropertyAnimation 的 class 类型和相关状态。 |
| [L15](../src/apps/controller/RemoteCToast.h#L15) | `QTimer` | class | 定义 QTimer 的 class 类型和相关状态。 |
| [L19](../src/apps/controller/RemoteCToast.h#L19) | `RemoteCToast` | class | 定义 RemoteCToast 的 class 类型和相关状态。 |
| [L21](../src/apps/controller/RemoteCToast.h#L21) | `Tone` | enum class | 定义 Tone 的 enum class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L11](../src/apps/controller/RemoteCToast.h#L11) | `QEvent` | `class QEvent;` | 保存 q event 相关配置或运行状态。 |
| [L12](../src/apps/controller/RemoteCToast.h#L12) | `QGraphicsOpacityEffect` | `class QGraphicsOpacityEffect;` | 保存 q graphics opacity effect 相关配置或运行状态。 |
| [L13](../src/apps/controller/RemoteCToast.h#L13) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L14](../src/apps/controller/RemoteCToast.h#L14) | `QPropertyAnimation` | `class QPropertyAnimation;` | 保存 q property animation 相关配置或运行状态。 |
| [L15](../src/apps/controller/RemoteCToast.h#L15) | `QTimer` | `class QTimer;` | 保存定时、截止或超时状态：q timer。 |
| [L53](../src/apps/controller/RemoteCToast.h#L53) | `host_` | `QWidget* host_ = nullptr;` | 保存 host 相关配置或运行状态。 |
| [L54](../src/apps/controller/RemoteCToast.h#L54) | `anchor_` | `QPointer<QWidget> anchor_;` | 保存 anchor 相关配置或运行状态。 |
| [L55](../src/apps/controller/RemoteCToast.h#L55) | `globalCenter_` | `QPoint globalCenter_;` | 保存 global center 相关配置或运行状态。 |
| [L56](../src/apps/controller/RemoteCToast.h#L56) | `useGlobalCenter_` | `bool useGlobalCenter_ = false;` | 保存 use global center 相关配置或运行状态。 |
| [L57](../src/apps/controller/RemoteCToast.h#L57) | `marker_` | `QLabel* marker_ = nullptr;` | 保存 marker 相关配置或运行状态。 |
| [L58](../src/apps/controller/RemoteCToast.h#L58) | `message_` | `QLabel* message_ = nullptr;` | 保存 message 相关配置或运行状态。 |
| [L59](../src/apps/controller/RemoteCToast.h#L59) | `timer_` | `QTimer* timer_ = nullptr;` | 保存定时、截止或超时状态：timer。 |
| [L60](../src/apps/controller/RemoteCToast.h#L60) | `opacity_` | `QGraphicsOpacityEffect* opacity_ = nullptr;` | 保存 opacity 相关配置或运行状态。 |
| [L61](../src/apps/controller/RemoteCToast.h#L61) | `animation_` | `QPropertyAnimation* animation_ = nullptr;` | 保存 animation 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L27](../src/apps/controller/RemoteCToast.h#L27) | `Show` | 声明 | `static void Show(QWidget* parent, const QString& message, Tone tone = Tone::kInformation)` | 实现 show 对应的业务或工具逻辑。 |
| [L30](../src/apps/controller/RemoteCToast.h#L30) | `ShowAbove` | 声明 | `static void ShowAbove(QWidget* anchor, const QString& message, Tone tone = Tone::kInformation)` | 实现 show above 对应的业务或工具逻辑。 |
| [L33](../src/apps/controller/RemoteCToast.h#L33) | `ShowAtGlobalCenter` | 声明 | `static void ShowAtGlobalCenter( QWidget* parent, const QPoint& globalCenter, const QString& message, Tone tone = Tone::kInformation, bool prominent = false)` | 实现 show at global center 对应的业务或工具逻辑。 |
| [L41](../src/apps/controller/RemoteCToast.h#L41) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L44](../src/apps/controller/RemoteCToast.h#L44) | `RemoteCToast` | 声明 | `explicit RemoteCToast(QWidget* host)` | 实现 remote c toast 对应的业务或工具逻辑。 |
| [L45](../src/apps/controller/RemoteCToast.h#L45) | `ShowMessage` | 声明 | `void ShowMessage(const QString& message, Tone tone, QWidget* anchor, const QPoint* globalCenter = nullptr, bool prominent = false)` | 实现 show message 对应的业务或工具逻辑。 |
| [L50](../src/apps/controller/RemoteCToast.h#L50) | `Reposition` | 声明 | `void Reposition()` | 实现 reposition 对应的业务或工具逻辑。 |
| [L51](../src/apps/controller/RemoteCToast.h#L51) | `FadeOut` | 声明 | `void FadeOut()` | 实现 fade out 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteCursorRenderState.cpp`

[打开源码](../src/apps/controller/RemoteCursorRenderState.cpp) · **文件作用：** 实现 remote cursor render state 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L17](../src/apps/controller/RemoteCursorRenderState.cpp#L17) | `RemoteCursorRenderState::RemoteCursorRenderState` | 定义 | `RemoteCursorRenderState::RemoteCursorRenderState()` | 构造并初始化 RemoteCursorRenderState 实例。 |
| [L22](../src/apps/controller/RemoteCursorRenderState.cpp#L22) | `RemoteCursorRenderState::SetShape` | 定义 | `void RemoteCursorRenderState::SetShape(const RemoteCursorShape& shape)` | 更新或应用 set shape 相关逻辑。 |
| [L54](../src/apps/controller/RemoteCursorRenderState.cpp#L54) | `RemoteCursorRenderState::SetPosition` | 定义 | `void RemoteCursorRenderState::SetPosition( const RemoteCursorPosition& position, bool predicted)` | 更新或应用 set position 相关逻辑。 |
| [L67](../src/apps/controller/RemoteCursorRenderState.cpp#L67) | `RemoteCursorRenderState::ApplyRemotePosition` | 定义 | `void RemoteCursorRenderState::ApplyRemotePosition( const RemoteCursorPosition& position)` | 更新或应用 apply remote position 相关逻辑。 |
| [L86](../src/apps/controller/RemoteCursorRenderState.cpp#L86) | `RemoteCursorRenderState::Reset` | 定义 | `void RemoteCursorRenderState::Reset()` | 重置或移除 reset 相关逻辑。 |
| [L94](../src/apps/controller/RemoteCursorRenderState.cpp#L94) | `RemoteCursorRenderState::SetRenderingEnabled` | 定义 | `void RemoteCursorRenderState::SetRenderingEnabled(bool enabled)` | 更新或应用 set rendering enabled 相关逻辑。 |
| [L100](../src/apps/controller/RemoteCursorRenderState.cpp#L100) | `RemoteCursorRenderState::GetSnapshot` | 定义 | `RemoteCursorRenderState::Snapshot RemoteCursorRenderState::GetSnapshot() const` | 查询并返回 get snapshot 相关逻辑。 |
| [L107](../src/apps/controller/RemoteCursorRenderState.cpp#L107) | `RemoteCursorRenderState::Paint` | 定义 | `void RemoteCursorRenderState::Paint(QPainter& painter, const QRect& content, const QSize& sourceSize) const` | 准备或呈现 paint 相关逻辑。 |
| [L123](../src/apps/controller/RemoteCursorRenderState.cpp#L123) | `RemoteCursorRenderState::TargetRect` | 定义 | `QRect RemoteCursorRenderState::TargetRect(const Snapshot& snapshot, const QRect& content, const QSize& sourceSize)` | 实现 target rect 对应的业务或工具逻辑。 |
| [L148](../src/apps/controller/RemoteCursorRenderState.cpp#L148) | `RemoteCursorRenderState::BuildFallbackArrow` | 定义 | `void RemoteCursorRenderState::BuildFallbackArrow()` | 创建或初始化 build fallback arrow 相关逻辑。 |

## `src/apps/controller/RemoteCursorRenderState.h`

[打开源码](../src/apps/controller/RemoteCursorRenderState.h) · **文件作用：** 声明 remote cursor render state 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L17](../src/apps/controller/RemoteCursorRenderState.h#L17) | `QPainter` | class | 定义 QPainter 的 class 类型和相关状态。 |
| [L24](../src/apps/controller/RemoteCursorRenderState.h#L24) | `RemoteCursorRenderState` | class | Thread-safe cursor state shared by the Qt paint path and the dedicated D3D11 presentation thread. It intentionally is not a QWidget: a translucent native cursor window cannot re... |
| [L26](../src/apps/controller/RemoteCursorRenderState.h#L26) | `Snapshot` | struct | 定义 Snapshot 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L17](../src/apps/controller/RemoteCursorRenderState.h#L17) | `QPainter` | `class QPainter;` | 保存 q painter 相关配置或运行状态。 |
| [L27](../src/apps/controller/RemoteCursorRenderState.h#L27) | `image` | `QImage image;` | 保存媒体帧、图像或缓冲资源：image。 |
| [L28](../src/apps/controller/RemoteCursorRenderState.h#L28) | `hotspot` | `QPoint hotspot;` | 保存 hotspot 相关配置或运行状态。 |
| [L29](../src/apps/controller/RemoteCursorRenderState.h#L29) | `normalizedPosition` | `QPoint normalizedPosition;` | 保存 normalized position 相关配置或运行状态。 |
| [L30](../src/apps/controller/RemoteCursorRenderState.h#L30) | `shapeRevision` | `std::uint64_t shapeRevision = 0;` | 标记当前世代，用于拒绝过期异步结果：shape revision。 |
| [L31](../src/apps/controller/RemoteCursorRenderState.h#L31) | `visible` | `bool visible = false;` | 保存 visible 相关配置或运行状态。 |
| [L52](../src/apps/controller/RemoteCursorRenderState.h#L52) | `mutex_` | `mutable std::mutex mutex_;` | 保护跨线程共享状态：mutex。 |
| [L53](../src/apps/controller/RemoteCursorRenderState.h#L53) | `image_` | `QImage image_;` | 保存媒体帧、图像或缓冲资源：image。 |
| [L54](../src/apps/controller/RemoteCursorRenderState.h#L54) | `hotspot_` | `QPoint hotspot_;` | 保存 hotspot 相关配置或运行状态。 |
| [L55](../src/apps/controller/RemoteCursorRenderState.h#L55) | `normalizedPosition_` | `QPoint normalizedPosition_;` | 保存 normalized position 相关配置或运行状态。 |
| [L56](../src/apps/controller/RemoteCursorRenderState.h#L56) | `localPredictionStartedAt_` | `std::chrono::steady_clock::time_point localPredictionStartedAt_{};` | 保存 local prediction started at 相关配置或运行状态。 |
| [L57](../src/apps/controller/RemoteCursorRenderState.h#L57) | `shapeRevision_` | `std::uint64_t shapeRevision_ = 0;` | 标记当前世代，用于拒绝过期异步结果：shape revision。 |
| [L58](../src/apps/controller/RemoteCursorRenderState.h#L58) | `hasLocalPrediction_` | `bool hasLocalPrediction_ = false;` | 保存 has local prediction 相关配置或运行状态。 |
| [L59](../src/apps/controller/RemoteCursorRenderState.h#L59) | `visible_` | `bool visible_ = false;` | 保存 visible 相关配置或运行状态。 |
| [L60](../src/apps/controller/RemoteCursorRenderState.h#L60) | `renderingEnabled_` | `bool renderingEnabled_ = false;` | 保存能力或开关状态：rendering enabled。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L34](../src/apps/controller/RemoteCursorRenderState.h#L34) | `RemoteCursorRenderState` | 声明 | `RemoteCursorRenderState()` | 实现 remote cursor render state 对应的业务或工具逻辑。 |
| [L36](../src/apps/controller/RemoteCursorRenderState.h#L36) | `SetShape` | 声明 | `void SetShape(const RemoteCursorShape& shape)` | 更新或应用 set shape 相关逻辑。 |
| [L37](../src/apps/controller/RemoteCursorRenderState.h#L37) | `SetPosition` | 声明 | `void SetPosition(const RemoteCursorPosition& position, bool predicted)` | 更新或应用 set position 相关逻辑。 |
| [L38](../src/apps/controller/RemoteCursorRenderState.h#L38) | `ApplyRemotePosition` | 声明 | `void ApplyRemotePosition(const RemoteCursorPosition& position)` | 更新或应用 apply remote position 相关逻辑。 |
| [L39](../src/apps/controller/RemoteCursorRenderState.h#L39) | `Reset` | 声明 | `void Reset()` | 重置或移除 reset 相关逻辑。 |
| [L40](../src/apps/controller/RemoteCursorRenderState.h#L40) | `SetRenderingEnabled` | 声明 | `void SetRenderingEnabled(bool enabled)` | 更新或应用 set rendering enabled 相关逻辑。 |
| [L41](../src/apps/controller/RemoteCursorRenderState.h#L41) | `GetSnapshot` | 声明 | `Snapshot GetSnapshot() const` | 查询并返回 get snapshot 相关逻辑。 |
| [L42](../src/apps/controller/RemoteCursorRenderState.h#L42) | `Paint` | 声明 | `void Paint(QPainter& painter, const QRect& content, const QSize& sourceSize) const` | 准备或呈现 paint 相关逻辑。 |
| [L47](../src/apps/controller/RemoteCursorRenderState.h#L47) | `TargetRect` | 声明 | `static QRect TargetRect(const Snapshot& snapshot, const QRect& content, const QSize& sourceSize)` | 实现 target rect 对应的业务或工具逻辑。 |
| [L50](../src/apps/controller/RemoteCursorRenderState.h#L50) | `BuildFallbackArrow` | 声明 | `void BuildFallbackArrow()` | 创建或初始化 build fallback arrow 相关逻辑。 |

## `src/apps/controller/RemoteDesktopCanvas.cpp`

[打开源码](../src/apps/controller/RemoteDesktopCanvas.cpp) · **文件作用：** 实现 remote desktop canvas 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L59](../src/apps/controller/RemoteDesktopCanvas.cpp#L59) | `pasteSender_` | 定义 | `pasteSender_(std::move(pasteSender)), firstPresentationCallback_( std::move(firstPresentationCallback)), presentationTelemetryId_( VideoPresentationTelemetryRegistry::Instance().Register( std::move(telemetryPeerDevice...` | 实现 paste sender 对应的业务或工具逻辑。 |
| [L119](../src/apps/controller/RemoteDesktopCanvas.cpp#L119) | `RemoteDesktopCanvas::~RemoteDesktopCanvas` | 定义 | `RemoteDesktopCanvas::~RemoteDesktopCanvas()` | 停止相关活动并释放 RemoteDesktopCanvas 实例拥有的资源。 |
| [L136](../src/apps/controller/RemoteDesktopCanvas.cpp#L136) | `RemoteDesktopCanvas::SetControlEnabled` | 定义 | `void RemoteDesktopCanvas::SetControlEnabled(bool enabled)` | 更新或应用 set control enabled 相关逻辑。 |
| [L158](../src/apps/controller/RemoteDesktopCanvas.cpp#L158) | `RemoteDesktopCanvas::SetActualPixelDisplayMode` | 定义 | `void RemoteDesktopCanvas::SetActualPixelDisplayMode(bool enabled)` | 更新或应用 set actual pixel display mode 相关逻辑。 |
| [L177](../src/apps/controller/RemoteDesktopCanvas.cpp#L177) | `RemoteDesktopCanvas::ActualPixelDisplayMode` | 定义 | `bool RemoteDesktopCanvas::ActualPixelDisplayMode() const` | 实现 actual pixel display mode 对应的业务或工具逻辑。 |
| [L183](../src/apps/controller/RemoteDesktopCanvas.cpp#L183) | `RemoteDesktopCanvas::ApplyRemoteCursor` | 定义 | `void RemoteDesktopCanvas::ApplyRemoteCursor(const RemoteCursorEnvelope& envelope)` | 更新或应用 apply remote cursor 相关逻辑。 |
| [L205](../src/apps/controller/RemoteDesktopCanvas.cpp#L205) | `RemoteDesktopCanvas::ResetRemoteCursor` | 定义 | `void RemoteDesktopCanvas::ResetRemoteCursor()` | 重置或移除 reset remote cursor 相关逻辑。 |
| [L212](../src/apps/controller/RemoteDesktopCanvas.cpp#L212) | `RemoteDesktopCanvas::SetConnectionStage` | 定义 | `void RemoteDesktopCanvas::SetConnectionStage(QString stage)` | 更新或应用 set connection stage 相关逻辑。 |
| [L225](../src/apps/controller/RemoteDesktopCanvas.cpp#L225) | `RemoteDesktopCanvas::BeginPresentationGeneration` | 定义 | `void RemoteDesktopCanvas::BeginPresentationGeneration(std::uint64_t generation)` | 启动 begin presentation generation 相关逻辑。 |
| [L233](../src/apps/controller/RemoteDesktopCanvas.cpp#L233) | `RemoteDesktopCanvas::NotifyFirstPresentation` | 定义 | `void RemoteDesktopCanvas::NotifyFirstPresentation()` | 通知或报告 notify first presentation 相关逻辑。 |
| [L248](../src/apps/controller/RemoteDesktopCanvas.cpp#L248) | `RemoteDesktopCanvas::SetTargetFrameRate` | 定义 | `void RemoteDesktopCanvas::SetTargetFrameRate(std::uint32_t framesPerSecond)` | 更新或应用 set target frame rate 相关逻辑。 |
| [L259](../src/apps/controller/RemoteDesktopCanvas.cpp#L259) | `RemoteDesktopCanvas::SetDragPointerSampleRate` | 定义 | `void RemoteDesktopCanvas::SetDragPointerSampleRate(std::uint32_t hertz)` | 更新或应用 set drag pointer sample rate 相关逻辑。 |
| [L275](../src/apps/controller/RemoteDesktopCanvas.cpp#L275) | `RemoteDesktopCanvas::ShutdownInputScheduler` | 定义 | `void RemoteDesktopCanvas::ShutdownInputScheduler()` | 关闭并清理 shutdown input scheduler 相关逻辑。 |
| [L282](../src/apps/controller/RemoteDesktopCanvas.cpp#L282) | `RemoteDesktopCanvas::SetRemoteDisplayIdentity` | 定义 | `void RemoteDesktopCanvas::SetRemoteDisplayIdentity( std::uint32_t displayId, std::uint64_t layoutVersion)` | 更新或应用 set remote display identity 相关逻辑。 |
| [L296](../src/apps/controller/RemoteDesktopCanvas.cpp#L296) | `RemoteDesktopCanvas::ReleaseRemoteInputs` | 定义 | `void RemoteDesktopCanvas::ReleaseRemoteInputs()` | 释放或取消 release remote inputs 相关逻辑。 |
| [L307](../src/apps/controller/RemoteDesktopCanvas.cpp#L307) | `RemoteDesktopCanvas::OnFrame` | 定义 | `void RemoteDesktopCanvas::OnFrame(const webrtc::VideoFrame& frame)` | 接收并处理 on frame 相关逻辑。 |
| [L355](../src/apps/controller/RemoteDesktopCanvas.cpp#L355) | `RemoteDesktopCanvas::mouseMoveEvent` | 定义 | `void RemoteDesktopCanvas::mouseMoveEvent(QMouseEvent* event)` | 实现 mouse move event 对应的业务或工具逻辑。 |
| [L379](../src/apps/controller/RemoteDesktopCanvas.cpp#L379) | `RemoteDesktopCanvas::mousePressEvent` | 定义 | `void RemoteDesktopCanvas::mousePressEvent(QMouseEvent* event)` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L426](../src/apps/controller/RemoteDesktopCanvas.cpp#L426) | `RemoteDesktopCanvas::mouseReleaseEvent` | 定义 | `void RemoteDesktopCanvas::mouseReleaseEvent(QMouseEvent* event)` | 实现 mouse release event 对应的业务或工具逻辑。 |
| [L474](../src/apps/controller/RemoteDesktopCanvas.cpp#L474) | `RemoteDesktopCanvas::wheelEvent` | 定义 | `void RemoteDesktopCanvas::wheelEvent(QWheelEvent* event)` | 实现 wheel event 对应的业务或工具逻辑。 |
| [L504](../src/apps/controller/RemoteDesktopCanvas.cpp#L504) | `RemoteDesktopCanvas::keyPressEvent` | 定义 | `void RemoteDesktopCanvas::keyPressEvent(QKeyEvent* event)` | 实现 key press event 对应的业务或工具逻辑。 |
| [L550](../src/apps/controller/RemoteDesktopCanvas.cpp#L550) | `RemoteDesktopCanvas::keyReleaseEvent` | 定义 | `void RemoteDesktopCanvas::keyReleaseEvent(QKeyEvent* event)` | 实现 key release event 对应的业务或工具逻辑。 |
| [L572](../src/apps/controller/RemoteDesktopCanvas.cpp#L572) | `RemoteDesktopCanvas::dragEnterEvent` | 定义 | `void RemoteDesktopCanvas::dragEnterEvent(QDragEnterEvent* event)` | 实现 drag enter event 对应的业务或工具逻辑。 |
| [L589](../src/apps/controller/RemoteDesktopCanvas.cpp#L589) | `RemoteDesktopCanvas::dropEvent` | 定义 | `void RemoteDesktopCanvas::dropEvent(QDropEvent* event)` | 实现 drop event 对应的业务或工具逻辑。 |
| [L611](../src/apps/controller/RemoteDesktopCanvas.cpp#L611) | `RemoteDesktopCanvas::focusOutEvent` | 定义 | `void RemoteDesktopCanvas::focusOutEvent(QFocusEvent* event)` | 实现 focus out event 对应的业务或工具逻辑。 |
| [L621](../src/apps/controller/RemoteDesktopCanvas.cpp#L621) | `RemoteDesktopCanvas::resizeEvent` | 定义 | `void RemoteDesktopCanvas::resizeEvent(QResizeEvent* event)` | 实现 resize event 对应的业务或工具逻辑。 |
| [L633](../src/apps/controller/RemoteDesktopCanvas.cpp#L633) | `RemoteDesktopCanvas::event` | 定义 | `bool RemoteDesktopCanvas::event(QEvent* event)` | 实现 event 对应的业务或工具逻辑。 |
| [L647](../src/apps/controller/RemoteDesktopCanvas.cpp#L647) | `RemoteDesktopCanvas::paintEvent` | 定义 | `void RemoteDesktopCanvas::paintEvent(QPaintEvent*)` | 准备或呈现 paint event 相关逻辑。 |
| [L767](../src/apps/controller/RemoteDesktopCanvas.cpp#L767) | `RemoteDesktopCanvas::IsCpuNv12Image` | 定义 | `bool RemoteDesktopCanvas::IsCpuNv12Image(const QImage& image)` | 判断 is cpu nv12 image 相关逻辑。 |
| [L774](../src/apps/controller/RemoteDesktopCanvas.cpp#L774) | `RemoteDesktopCanvas::UpdateCpuCanvasMetrics` | 定义 | `void RemoteDesktopCanvas::UpdateCpuCanvasMetrics()` | 更新或应用 update cpu canvas metrics 相关逻辑。 |
| [L784](../src/apps/controller/RemoteDesktopCanvas.cpp#L784) | `RemoteDesktopCanvas::CpuQtOutputSize` | 定义 | `QSize RemoteDesktopCanvas::CpuQtOutputSize(int sourceWidth, int sourceHeight) const` | 实现 cpu qt output size 对应的业务或工具逻辑。 |
| [L837](../src/apps/controller/RemoteDesktopCanvas.cpp#L837) | `RemoteDesktopCanvas::QueueCpuFrame` | 定义 | `void RemoteDesktopCanvas::QueueCpuFrame( webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame, bool recordArrival)` | 实现 queue cpu frame 对应的业务或工具逻辑。 |
| [L871](../src/apps/controller/RemoteDesktopCanvas.cpp#L871) | `RemoteDesktopCanvas::CpuConversionLoop` | 定义 | `void RemoteDesktopCanvas::CpuConversionLoop(std::stop_token stopToken)` | 实现 cpu conversion loop 对应的业务或工具逻辑。 |
| [L1072](../src/apps/controller/RemoteDesktopCanvas.cpp#L1072) | `RemoteDesktopCanvas::RecycleCpuImage` | 定义 | `void RemoteDesktopCanvas::RecycleCpuImage(QImage image)` | 实现 recycle cpu image 对应的业务或工具逻辑。 |
| [L1081](../src/apps/controller/RemoteDesktopCanvas.cpp#L1081) | `RemoteDesktopCanvas::QueueConvertedCpuImage` | 定义 | `void RemoteDesktopCanvas::QueueConvertedCpuImage(QImage image, std::uint64_t sequence)` | 实现 queue converted cpu image 对应的业务或工具逻辑。 |
| [L1092](../src/apps/controller/RemoteDesktopCanvas.cpp#L1092) | `RemoteDesktopCanvas::QueueD3D11CpuImage` | 定义 | `void RemoteDesktopCanvas::QueueD3D11CpuImage(QImage image, std::uint64_t sequence)` | 实现 queue d3 d11 cpu image 对应的业务或工具逻辑。 |
| [L1135](../src/apps/controller/RemoteDesktopCanvas.cpp#L1135) | `RemoteDesktopCanvas::QueueQtCpuImage` | 定义 | `void RemoteDesktopCanvas::QueueQtCpuImage(QImage image, std::uint64_t sequence)` | 实现 queue qt cpu image 对应的业务或工具逻辑。 |
| [L1159](../src/apps/controller/RemoteDesktopCanvas.cpp#L1159) | `RemoteDesktopCanvas::ApplyPendingCpuImage` | 定义 | `void RemoteDesktopCanvas::ApplyPendingCpuImage()` | 更新或应用 apply pending cpu image 相关逻辑。 |
| [L1200](../src/apps/controller/RemoteDesktopCanvas.cpp#L1200) | `RemoteDesktopCanvas::QueueNativeFrame` | 定义 | `void RemoteDesktopCanvas::QueueNativeFrame( webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame)` | 实现 queue native frame 对应的业务或工具逻辑。 |
| [L1247](../src/apps/controller/RemoteDesktopCanvas.cpp#L1247) | `RemoteDesktopCanvas::QueueI420Frame` | 定义 | `void RemoteDesktopCanvas::QueueI420Frame( webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame)` | 实现 queue i420 frame 对应的业务或工具逻辑。 |
| [L1293](../src/apps/controller/RemoteDesktopCanvas.cpp#L1293) | `RemoteDesktopCanvas::ScheduleNativeUiUpdate` | 定义 | `void RemoteDesktopCanvas::ScheduleNativeUiUpdate()` | 执行后台循环或调度 schedule native ui update 相关逻辑。 |
| [L1331](../src/apps/controller/RemoteDesktopCanvas.cpp#L1331) | `RemoteDesktopCanvas::UpdateCpuConversionInterval` | 定义 | `void RemoteDesktopCanvas::UpdateCpuConversionInterval()` | 更新或应用 update cpu conversion interval 相关逻辑。 |
| [L1352](../src/apps/controller/RemoteDesktopCanvas.cpp#L1352) | `RemoteDesktopCanvas::RequestNativeRedraw` | 定义 | `void RemoteDesktopCanvas::RequestNativeRedraw()` | 发起请求或查询 request native redraw 相关逻辑。 |
| [L1365](../src/apps/controller/RemoteDesktopCanvas.cpp#L1365) | `RemoteDesktopCanvas::CursorVisualChanged` | 定义 | `void RemoteDesktopCanvas::CursorVisualChanged()` | 实现 cursor visual changed 对应的业务或工具逻辑。 |
| [L1374](../src/apps/controller/RemoteDesktopCanvas.cpp#L1374) | `RemoteDesktopCanvas::DeactivateNativePresentation` | 定义 | `bool RemoteDesktopCanvas::DeactivateNativePresentation(std::uint64_t cpuSequence)` | 实现 deactivate native presentation 对应的业务或工具逻辑。 |
| [L1400](../src/apps/controller/RemoteDesktopCanvas.cpp#L1400) | `RemoteDesktopCanvas::NativePresentationLoop` | 定义 | `void RemoteDesktopCanvas::NativePresentationLoop(std::stop_token stopToken)` | 实现 native presentation loop 对应的业务或工具逻辑。 |
| [L1673](../src/apps/controller/RemoteDesktopCanvas.cpp#L1673) | `RemoteDesktopCanvas::UpdateNativeSurfaceGeometry` | 定义 | `void RemoteDesktopCanvas::UpdateNativeSurfaceGeometry()` | 更新或应用 update native surface geometry 相关逻辑。 |
| [L1680](../src/apps/controller/RemoteDesktopCanvas.cpp#L1680) | `RemoteDesktopCanvas::VideoContentRect` | 定义 | `QRect RemoteDesktopCanvas::VideoContentRect() const` | 实现 video content rect 对应的业务或工具逻辑。 |
| [L1717](../src/apps/controller/RemoteDesktopCanvas.cpp#L1717) | `RemoteDesktopCanvas::VideoSourceSize` | 定义 | `QSize RemoteDesktopCanvas::VideoSourceSize() const` | 实现 video source size 对应的业务或工具逻辑。 |
| [L1731](../src/apps/controller/RemoteDesktopCanvas.cpp#L1731) | `RemoteDesktopCanvas::SampleCurrentDragPointer` | 定义 | `RemoteDesktopCanvas::SampleCurrentDragPointer( const RemoteInputEvent& fallback) const` | 实现 sample current drag pointer 对应的业务或工具逻辑。 |
| [L1810](../src/apps/controller/RemoteDesktopCanvas.cpp#L1810) | `RemoteDesktopCanvas::MapPoint` | 定义 | `std::optional<std::pair<std::uint16_t, std::uint16_t>> RemoteDesktopCanvas::MapPoint( const QPointF& point, bool clampToContent) const` | 实现 map point 对应的业务或工具逻辑。 |
| [L1850](../src/apps/controller/RemoteDesktopCanvas.cpp#L1850) | `RemoteDesktopCanvas::MapMouseButton` | 定义 | `std::optional<RemoteMouseButton> RemoteDesktopCanvas::MapMouseButton( Qt::MouseButton button)` | 实现 map mouse button 对应的业务或工具逻辑。 |
| [L1869](../src/apps/controller/RemoteDesktopCanvas.cpp#L1869) | `RemoteDesktopCanvas::MapMouseButtons` | 定义 | `std::uint8_t RemoteDesktopCanvas::MapMouseButtons(Qt::MouseButtons buttons)` | 实现 map mouse buttons 对应的业务或工具逻辑。 |
| [L1890](../src/apps/controller/RemoteDesktopCanvas.cpp#L1890) | `RemoteDesktopCanvas::ClampWheelDelta` | 定义 | `std::int16_t RemoteDesktopCanvas::ClampWheelDelta(int value)` | 实现 clamp wheel delta 对应的业务或工具逻辑。 |
| [L1898](../src/apps/controller/RemoteDesktopCanvas.cpp#L1898) | `RemoteDesktopCanvas::IsExtendedVirtualKey` | 定义 | `bool RemoteDesktopCanvas::IsExtendedVirtualKey(std::uint32_t virtualKey)` | 判断 is extended virtual key 相关逻辑。 |
| [L1925](../src/apps/controller/RemoteDesktopCanvas.cpp#L1925) | `RemoteDesktopCanvas::QueuePointerMove` | 定义 | `bool RemoteDesktopCanvas::QueuePointerMove( const std::pair<std::uint16_t, std::uint16_t>& point, std::uint8_t pressedMouseButtons)` | 实现 queue pointer move 对应的业务或工具逻辑。 |
| [L1962](../src/apps/controller/RemoteDesktopCanvas.cpp#L1962) | `RemoteDesktopCanvas::FlushPendingPointerMove` | 定义 | `void RemoteDesktopCanvas::FlushPendingPointerMove()` | 实现 flush pending pointer move 对应的业务或工具逻辑。 |
| [L1969](../src/apps/controller/RemoteDesktopCanvas.cpp#L1969) | `RemoteDesktopCanvas::CancelPendingPointerMove` | 定义 | `void RemoteDesktopCanvas::CancelPendingPointerMove()` | 判断 cancel pending pointer move 相关逻辑。 |
| [L1976](../src/apps/controller/RemoteDesktopCanvas.cpp#L1976) | `RemoteDesktopCanvas::SendKeyEvent` | 定义 | `bool RemoteDesktopCanvas::SendKeyEvent(QKeyEvent* event, bool pressed)` | 发送或发布 send key event 相关逻辑。 |
| [L2001](../src/apps/controller/RemoteDesktopCanvas.cpp#L2001) | `RemoteDesktopCanvas::FocusRemotePasteTarget` | 定义 | `void RemoteDesktopCanvas::FocusRemotePasteTarget(const QPointF& position)` | 实现 focus remote paste target 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteDesktopCanvas.h`

[打开源码](../src/apps/controller/RemoteDesktopCanvas.h) · **文件作用：** 声明 remote desktop canvas 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L35](../src/apps/controller/RemoteDesktopCanvas.h#L35) | `QDragEnterEvent` | class | 定义 QDragEnterEvent 的 class 类型和相关状态。 |
| [L36](../src/apps/controller/RemoteDesktopCanvas.h#L36) | `QDropEvent` | class | 定义 QDropEvent 的 class 类型和相关状态。 |
| [L37](../src/apps/controller/RemoteDesktopCanvas.h#L37) | `QEvent` | class | 定义 QEvent 的 class 类型和相关状态。 |
| [L38](../src/apps/controller/RemoteDesktopCanvas.h#L38) | `QFocusEvent` | class | 定义 QFocusEvent 的 class 类型和相关状态。 |
| [L39](../src/apps/controller/RemoteDesktopCanvas.h#L39) | `QKeyEvent` | class | 定义 QKeyEvent 的 class 类型和相关状态。 |
| [L40](../src/apps/controller/RemoteDesktopCanvas.h#L40) | `QMouseEvent` | class | 定义 QMouseEvent 的 class 类型和相关状态。 |
| [L41](../src/apps/controller/RemoteDesktopCanvas.h#L41) | `QPaintEvent` | class | 定义 QPaintEvent 的 class 类型和相关状态。 |
| [L42](../src/apps/controller/RemoteDesktopCanvas.h#L42) | `QResizeEvent` | class | 定义 QResizeEvent 的 class 类型和相关状态。 |
| [L43](../src/apps/controller/RemoteDesktopCanvas.h#L43) | `QWheelEvent` | class | 定义 QWheelEvent 的 class 类型和相关状态。 |
| [L47](../src/apps/controller/RemoteDesktopCanvas.h#L47) | `D3D11VideoSurface` | class | 定义 D3D11VideoSurface 的 class 类型和相关状态。 |
| [L48](../src/apps/controller/RemoteDesktopCanvas.h#L48) | `HighResolutionPointerMoveScheduler` | class | 定义 HighResolutionPointerMoveScheduler 的 class 类型和相关状态。 |
| [L49](../src/apps/controller/RemoteDesktopCanvas.h#L49) | `RemoteCursorRenderState` | class | 定义 RemoteCursorRenderState 的 class 类型和相关状态。 |
| [L51](../src/apps/controller/RemoteDesktopCanvas.h#L51) | `RemoteDesktopCanvas` | class | 定义 RemoteDesktopCanvas 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L35](../src/apps/controller/RemoteDesktopCanvas.h#L35) | `QDragEnterEvent` | `class QDragEnterEvent;` | 保存 q drag enter event 相关配置或运行状态。 |
| [L36](../src/apps/controller/RemoteDesktopCanvas.h#L36) | `QDropEvent` | `class QDropEvent;` | 保存 q drop event 相关配置或运行状态。 |
| [L37](../src/apps/controller/RemoteDesktopCanvas.h#L37) | `QEvent` | `class QEvent;` | 保存 q event 相关配置或运行状态。 |
| [L38](../src/apps/controller/RemoteDesktopCanvas.h#L38) | `QFocusEvent` | `class QFocusEvent;` | 保存 q focus event 相关配置或运行状态。 |
| [L39](../src/apps/controller/RemoteDesktopCanvas.h#L39) | `QKeyEvent` | `class QKeyEvent;` | 保存 q key event 相关配置或运行状态。 |
| [L40](../src/apps/controller/RemoteDesktopCanvas.h#L40) | `QMouseEvent` | `class QMouseEvent;` | 保存 q mouse event 相关配置或运行状态。 |
| [L41](../src/apps/controller/RemoteDesktopCanvas.h#L41) | `QPaintEvent` | `class QPaintEvent;` | 保存 q paint event 相关配置或运行状态。 |
| [L42](../src/apps/controller/RemoteDesktopCanvas.h#L42) | `QResizeEvent` | `class QResizeEvent;` | 保存 q resize event 相关配置或运行状态。 |
| [L43](../src/apps/controller/RemoteDesktopCanvas.h#L43) | `QWheelEvent` | `class QWheelEvent;` | 保存 q wheel event 相关配置或运行状态。 |
| [L47](../src/apps/controller/RemoteDesktopCanvas.h#L47) | `D3D11VideoSurface` | `class D3D11VideoSurface;` | 保存 d3 d11 video surface 相关配置或运行状态。 |
| [L48](../src/apps/controller/RemoteDesktopCanvas.h#L48) | `HighResolutionPointerMoveScheduler` | `class HighResolutionPointerMoveScheduler;` | 保存 high resolution pointer move scheduler 相关配置或运行状态。 |
| [L49](../src/apps/controller/RemoteDesktopCanvas.h#L49) | `RemoteCursorRenderState` | `class RemoteCursorRenderState;` | 保存状态机当前状态：remote cursor render state。 |
| [L201](../src/apps/controller/RemoteDesktopCanvas.h#L201) | `frameImage_` | `QImage frameImage_;` | 保存媒体帧、图像或缓冲资源：frame image。 |
| [L202](../src/apps/controller/RemoteDesktopCanvas.h#L202) | `frameImageSequence_` | `std::uint64_t frameImageSequence_ = 0;` | 保存单调序号，用于排序或去重：frame image sequence。 |
| [L203](../src/apps/controller/RemoteDesktopCanvas.h#L203) | `lastPresentedCpuSequence_` | `std::uint64_t lastPresentedCpuSequence_ = 0;` | 保存单调序号，用于排序或去重：last presented cpu sequence。 |
| [L204](../src/apps/controller/RemoteDesktopCanvas.h#L204) | `inputSender_` | `InputSender inputSender_;` | 保存 input sender 相关配置或运行状态。 |
| [L205](../src/apps/controller/RemoteDesktopCanvas.h#L205) | `pasteSender_` | `PasteSender pasteSender_;` | 保存 paste sender 相关配置或运行状态。 |
| [L206](../src/apps/controller/RemoteDesktopCanvas.h#L206) | `firstPresentationCallback_` | `FirstPresentationCallback firstPresentationCallback_;` | 保存回调或观察者入口：first presentation callback。 |
| [L207](../src/apps/controller/RemoteDesktopCanvas.h#L207) | `presentationGeneration_` | `std::atomic<std::uint64_t> presentationGeneration_{ 0 };` | 标记当前世代，用于拒绝过期异步结果：presentation generation。 |
| [L208](../src/apps/controller/RemoteDesktopCanvas.h#L208) | `firstPresentationNotified_` | `std::atomic<bool> firstPresentationNotified_{ false };` | 保存 first presentation notified 相关配置或运行状态。 |
| [L209](../src/apps/controller/RemoteDesktopCanvas.h#L209) | `presentationTelemetryId_` | `std::uint64_t presentationTelemetryId_ = 0;` | 保存身份或作用域标识：presentation telemetry id。 |
| [L210](../src/apps/controller/RemoteDesktopCanvas.h#L210) | `controlEnabled_` | `bool controlEnabled_ = false;` | 保存能力或开关状态：control enabled。 |
| [L211](../src/apps/controller/RemoteDesktopCanvas.h#L211) | `actualPixelDisplayMode_` | `std::atomic_bool actualPixelDisplayMode_{ false };` | 保存 actual pixel display mode 相关配置或运行状态。 |
| [L212](../src/apps/controller/RemoteDesktopCanvas.h#L212) | `actualPixelPanOffset_` | `QPoint actualPixelPanOffset_;` | 保存 actual pixel pan offset 相关配置或运行状态。 |
| [L213](../src/apps/controller/RemoteDesktopCanvas.h#L213) | `actualPixelPanStartPosition_` | `QPointF actualPixelPanStartPosition_;` | 保存 actual pixel pan start position 相关配置或运行状态。 |
| [L214](../src/apps/controller/RemoteDesktopCanvas.h#L214) | `actualPixelPanStartOffset_` | `QPoint actualPixelPanStartOffset_;` | 保存 actual pixel pan start offset 相关配置或运行状态。 |
| [L215](../src/apps/controller/RemoteDesktopCanvas.h#L215) | `actualPixelPanning_` | `bool actualPixelPanning_ = false;` | 保存 actual pixel panning 相关配置或运行状态。 |
| [L216](../src/apps/controller/RemoteDesktopCanvas.h#L216) | `suppressPasteKeyRelease_` | `bool suppressPasteKeyRelease_ = false;` | 保存 suppress paste key release 相关配置或运行状态。 |
| [L220](../src/apps/controller/RemoteDesktopCanvas.h#L220) | `pointerMoveScheduler_` | `pointerMoveScheduler_;` | 保存 pointer move scheduler 相关配置或运行状态。 |
| [L221](../src/apps/controller/RemoteDesktopCanvas.h#L221) | `pointerMoveRateLimitHz_` | `std::uint32_t pointerMoveRateLimitHz_ = 120;` | 保存 pointer move rate limit hz 相关配置或运行状态。 |
| [L222](../src/apps/controller/RemoteDesktopCanvas.h#L222) | `dragPointerSampleRateHz_` | `std::uint32_t dragPointerSampleRateHz_ = 240;` | 保存 drag pointer sample rate hz 相关配置或运行状态。 |
| [L223](../src/apps/controller/RemoteDesktopCanvas.h#L223) | `pointerSamplingWindow_` | `HWND pointerSamplingWindow_ = nullptr;` | 保存 pointer sampling window 相关配置或运行状态。 |
| [L224](../src/apps/controller/RemoteDesktopCanvas.h#L224) | `pointerSourceWidth_` | `std::atomic<int> pointerSourceWidth_{0};` | 保存计数、尺寸或速率指标：pointer source width。 |
| [L225](../src/apps/controller/RemoteDesktopCanvas.h#L225) | `pointerSourceHeight_` | `std::atomic<int> pointerSourceHeight_{0};` | 保存计数、尺寸或速率指标：pointer source height。 |
| [L226](../src/apps/controller/RemoteDesktopCanvas.h#L226) | `pointerStretchToCanvas_` | `std::atomic_bool pointerStretchToCanvas_{false};` | 保存 pointer stretch to canvas 相关配置或运行状态。 |
| [L227](../src/apps/controller/RemoteDesktopCanvas.h#L227) | `remoteDisplayId_` | `std::uint32_t remoteDisplayId_ = 0;` | 保存身份或作用域标识：remote display id。 |
| [L228](../src/apps/controller/RemoteDesktopCanvas.h#L228) | `remoteDisplayLayoutVersion_` | `std::uint64_t remoteDisplayLayoutVersion_ = 0;` | 保存 remote display layout version 相关配置或运行状态。 |
| [L229](../src/apps/controller/RemoteDesktopCanvas.h#L229) | `nativeRenderingEnabled_` | `bool nativeRenderingEnabled_ = true;` | 保存能力或开关状态：native rendering enabled。 |
| [L230](../src/apps/controller/RemoteDesktopCanvas.h#L230) | `nativeSurface_` | `D3D11VideoSurface* nativeSurface_ = nullptr;` | 保存 native surface 相关配置或运行状态。 |
| [L231](../src/apps/controller/RemoteDesktopCanvas.h#L231) | `cursorRenderState_` | `std::unique_ptr<RemoteCursorRenderState> cursorRenderState_;` | 保存状态机当前状态：cursor render state。 |
| [L232](../src/apps/controller/RemoteDesktopCanvas.h#L232) | `nativeFrameSize_` | `QSize nativeFrameSize_;` | 保存计数、尺寸或速率指标：native frame size。 |
| [L233](../src/apps/controller/RemoteDesktopCanvas.h#L233) | `nativeFrameActive_` | `bool nativeFrameActive_ = false;` | 保存能力或开关状态：native frame active。 |
| [L234](../src/apps/controller/RemoteDesktopCanvas.h#L234) | `receivedFrameSequence_` | `std::atomic<std::uint64_t> receivedFrameSequence_{ 0 };` | 保存单调序号，用于排序或去重：received frame sequence。 |
| [L235](../src/apps/controller/RemoteDesktopCanvas.h#L235) | `cpuFrameMutex_` | `std::mutex cpuFrameMutex_;` | 保护跨线程共享状态：cpu frame mutex。 |
| [L236](../src/apps/controller/RemoteDesktopCanvas.h#L236) | `cpuFrameCondition_` | `std::condition_variable cpuFrameCondition_;` | 保存 cpu frame condition 相关配置或运行状态。 |
| [L237](../src/apps/controller/RemoteDesktopCanvas.h#L237) | `pendingCpuFrame_` | `webrtc::scoped_refptr<webrtc::VideoFrameBuffer> pendingCpuFrame_;` | 保存媒体帧、图像或缓冲资源：pending cpu frame。 |
| [L238](../src/apps/controller/RemoteDesktopCanvas.h#L238) | `pendingCpuSequence_` | `std::uint64_t pendingCpuSequence_ = 0;` | 保存单调序号，用于排序或去重：pending cpu sequence。 |
| [L239](../src/apps/controller/RemoteDesktopCanvas.h#L239) | `cpuConversionIntervalUs_` | `std::atomic<std::uint32_t> cpuConversionIntervalUs_{ 16'667 };` | 保存 cpu conversion interval us 相关配置或运行状态。 |
| [L240](../src/apps/controller/RemoteDesktopCanvas.h#L240) | `cpuCanvasWidth_` | `std::atomic<int> cpuCanvasWidth_{ 0 };` | 保存计数、尺寸或速率指标：cpu canvas width。 |
| [L241](../src/apps/controller/RemoteDesktopCanvas.h#L241) | `cpuCanvasHeight_` | `std::atomic<int> cpuCanvasHeight_{ 0 };` | 保存计数、尺寸或速率指标：cpu canvas height。 |
| [L245](../src/apps/controller/RemoteDesktopCanvas.h#L245) | `cpuCanvasDevicePixelRatioMilli_` | `std::atomic<int> cpuCanvasDevicePixelRatioMilli_{ 1000 };` | QWidget geometry uses device-independent pixels. The CPU renderer needs the screen DPR because it pre-scales frames before handing them back to the UI thread. |
| [L246](../src/apps/controller/RemoteDesktopCanvas.h#L246) | `cpuConversionThread_` | `std::jthread cpuConversionThread_;` | 拥有后台执行线程或工作器：cpu conversion thread。 |
| [L247](../src/apps/controller/RemoteDesktopCanvas.h#L247) | `cpuImageMutex_` | `std::mutex cpuImageMutex_;` | 保护跨线程共享状态：cpu image mutex。 |
| [L248](../src/apps/controller/RemoteDesktopCanvas.h#L248) | `pendingCpuImage_` | `QImage pendingCpuImage_;` | 保存媒体帧、图像或缓冲资源：pending cpu image。 |
| [L249](../src/apps/controller/RemoteDesktopCanvas.h#L249) | `pendingCpuImageSequence_` | `std::uint64_t pendingCpuImageSequence_ = 0;` | 保存单调序号，用于排序或去重：pending cpu image sequence。 |
| [L250](../src/apps/controller/RemoteDesktopCanvas.h#L250) | `recycledCpuImage_` | `QImage recycledCpuImage_;` | 保存媒体帧、图像或缓冲资源：recycled cpu image。 |
| [L251](../src/apps/controller/RemoteDesktopCanvas.h#L251) | `cpuImageDispatchPending_` | `bool cpuImageDispatchPending_ = false;` | 保存待处理队列或请求：cpu image dispatch pending。 |
| [L252](../src/apps/controller/RemoteDesktopCanvas.h#L252) | `nativeFrameMutex_` | `std::mutex nativeFrameMutex_;` | 保护跨线程共享状态：native frame mutex。 |
| [L253](../src/apps/controller/RemoteDesktopCanvas.h#L253) | `nativeFrameCondition_` | `std::condition_variable nativeFrameCondition_;` | 保存 native frame condition 相关配置或运行状态。 |
| [L254](../src/apps/controller/RemoteDesktopCanvas.h#L254) | `pendingNativeFrame_` | `webrtc::scoped_refptr<webrtc::VideoFrameBuffer> pendingNativeFrame_;` | 保存媒体帧、图像或缓冲资源：pending native frame。 |
| [L255](../src/apps/controller/RemoteDesktopCanvas.h#L255) | `pendingNativeSequence_` | `std::uint64_t pendingNativeSequence_ = 0;` | 保存单调序号，用于排序或去重：pending native sequence。 |
| [L256](../src/apps/controller/RemoteDesktopCanvas.h#L256) | `pendingD3D11CpuImage_` | `QImage pendingD3D11CpuImage_;` | 保存媒体帧、图像或缓冲资源：pending d3 d11 cpu image。 |
| [L257](../src/apps/controller/RemoteDesktopCanvas.h#L257) | `pendingD3D11CpuSequence_` | `std::uint64_t pendingD3D11CpuSequence_ = 0;` | 保存单调序号，用于排序或去重：pending d3 d11 cpu sequence。 |
| [L258](../src/apps/controller/RemoteDesktopCanvas.h#L258) | `nativeRedrawRequested_` | `bool nativeRedrawRequested_ = false;` | 保存 native redraw requested 相关配置或运行状态。 |
| [L259](../src/apps/controller/RemoteDesktopCanvas.h#L259) | `nativeResetRequested_` | `bool nativeResetRequested_ = false;` | 保存 native reset requested 相关配置或运行状态。 |
| [L260](../src/apps/controller/RemoteDesktopCanvas.h#L260) | `nativePresentationIntervalUs_` | `std::atomic<std::uint32_t> nativePresentationIntervalUs_{ 16'667 };` | 保存 native presentation interval us 相关配置或运行状态。 |
| [L261](../src/apps/controller/RemoteDesktopCanvas.h#L261) | `nativePresentationThread_` | `std::jthread nativePresentationThread_;` | 拥有后台执行线程或工作器：native presentation thread。 |
| [L262](../src/apps/controller/RemoteDesktopCanvas.h#L262) | `nativeFrameWidth_` | `std::atomic<int> nativeFrameWidth_{ 0 };` | 保存计数、尺寸或速率指标：native frame width。 |
| [L263](../src/apps/controller/RemoteDesktopCanvas.h#L263) | `nativeFrameHeight_` | `std::atomic<int> nativeFrameHeight_{ 0 };` | 保存计数、尺寸或速率指标：native frame height。 |
| [L264](../src/apps/controller/RemoteDesktopCanvas.h#L264) | `nativeUiActiveRequested_` | `std::atomic<bool> nativeUiActiveRequested_{ false };` | 保存 native ui active requested 相关配置或运行状态。 |
| [L265](../src/apps/controller/RemoteDesktopCanvas.h#L265) | `nativeSurfaceReady_` | `std::atomic<bool> nativeSurfaceReady_{ false };` | 保存能力或开关状态：native surface ready。 |
| [L266](../src/apps/controller/RemoteDesktopCanvas.h#L266) | `nativeUiDispatchPending_` | `std::atomic<bool> nativeUiDispatchPending_{ false };` | 保存待处理队列或请求：native ui dispatch pending。 |
| [L267](../src/apps/controller/RemoteDesktopCanvas.h#L267) | `nativeUiRevision_` | `std::atomic<std::uint64_t> nativeUiRevision_{ 0 };` | 标记当前世代，用于拒绝过期异步结果：native ui revision。 |
| [L268](../src/apps/controller/RemoteDesktopCanvas.h#L268) | `cpuD3D11PresentationFailed_` | `std::atomic<bool> cpuD3D11PresentationFailed_{ false };` | 保存 cpu d3 d11 presentation failed 相关配置或运行状态。 |
| [L269](../src/apps/controller/RemoteDesktopCanvas.h#L269) | `cpuNv12D3D11PresentationFailed_` | `std::atomic<bool> cpuNv12D3D11PresentationFailed_{ false };` | 保存 cpu nv12 d3 d11 presentation failed 相关配置或运行状态。 |
| [L270](../src/apps/controller/RemoteDesktopCanvas.h#L270) | `cpuI420D3D11PresentationFailed_` | `std::atomic<bool> cpuI420D3D11PresentationFailed_{ false };` | 保存 cpu i420 d3 d11 presentation failed 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L64](../src/apps/controller/RemoteDesktopCanvas.h#L64) | `RemoteDesktopCanvas` | 声明 | `explicit RemoteDesktopCanvas(InputSender inputSender, PasteSender pasteSender, FirstPresentationCallback firstPresentationCallback, std::string telemetryPeerDeviceId, QWidget* parent = nullptr)` | 实现 remote desktop canvas 对应的业务或工具逻辑。 |
| [L70](../src/apps/controller/RemoteDesktopCanvas.h#L70) | `~RemoteDesktopCanvas` | 声明 | `~RemoteDesktopCanvas() override` | 停止相关活动并释放 RemoteDesktopCanvas 实例拥有的资源。 |
| [L72](../src/apps/controller/RemoteDesktopCanvas.h#L72) | `SetControlEnabled` | 声明 | `void SetControlEnabled(bool enabled)` | 更新或应用 set control enabled 相关逻辑。 |
| [L74](../src/apps/controller/RemoteDesktopCanvas.h#L74) | `SetActualPixelDisplayMode` | 声明 | `void SetActualPixelDisplayMode(bool enabled)` | 更新或应用 set actual pixel display mode 相关逻辑。 |
| [L76](../src/apps/controller/RemoteDesktopCanvas.h#L76) | `ActualPixelDisplayMode` | 声明 | `bool ActualPixelDisplayMode() const` | 实现 actual pixel display mode 对应的业务或工具逻辑。 |
| [L78](../src/apps/controller/RemoteDesktopCanvas.h#L78) | `ApplyRemoteCursor` | 声明 | `void ApplyRemoteCursor(const RemoteCursorEnvelope& envelope)` | 更新或应用 apply remote cursor 相关逻辑。 |
| [L80](../src/apps/controller/RemoteDesktopCanvas.h#L80) | `ResetRemoteCursor` | 声明 | `void ResetRemoteCursor()` | 重置或移除 reset remote cursor 相关逻辑。 |
| [L82](../src/apps/controller/RemoteDesktopCanvas.h#L82) | `SetConnectionStage` | 声明 | `void SetConnectionStage(QString stage)` | 更新或应用 set connection stage 相关逻辑。 |
| [L84](../src/apps/controller/RemoteDesktopCanvas.h#L84) | `BeginPresentationGeneration` | 声明 | `void BeginPresentationGeneration(std::uint64_t generation)` | 启动 begin presentation generation 相关逻辑。 |
| [L86](../src/apps/controller/RemoteDesktopCanvas.h#L86) | `NotifyFirstPresentation` | 声明 | `void NotifyFirstPresentation()` | 通知或报告 notify first presentation 相关逻辑。 |
| [L88](../src/apps/controller/RemoteDesktopCanvas.h#L88) | `SetTargetFrameRate` | 声明 | `void SetTargetFrameRate(std::uint32_t framesPerSecond)` | 更新或应用 set target frame rate 相关逻辑。 |
| [L90](../src/apps/controller/RemoteDesktopCanvas.h#L90) | `SetDragPointerSampleRate` | 声明 | `void SetDragPointerSampleRate(std::uint32_t hertz)` | 更新或应用 set drag pointer sample rate 相关逻辑。 |
| [L92](../src/apps/controller/RemoteDesktopCanvas.h#L92) | `ShutdownInputScheduler` | 声明 | `void ShutdownInputScheduler()` | 关闭并清理 shutdown input scheduler 相关逻辑。 |
| [L94](../src/apps/controller/RemoteDesktopCanvas.h#L94) | `SetRemoteDisplayIdentity` | 声明 | `void SetRemoteDisplayIdentity( std::uint32_t displayId, std::uint64_t layoutVersion)` | 更新或应用 set remote display identity 相关逻辑。 |
| [L98](../src/apps/controller/RemoteDesktopCanvas.h#L98) | `ReleaseRemoteInputs` | 声明 | `void ReleaseRemoteInputs()` | 释放或取消 release remote inputs 相关逻辑。 |
| [L100](../src/apps/controller/RemoteDesktopCanvas.h#L100) | `OnFrame` | 声明 | `void OnFrame(const webrtc::VideoFrame& frame) override` | 接收并处理 on frame 相关逻辑。 |
| [L103](../src/apps/controller/RemoteDesktopCanvas.h#L103) | `mouseMoveEvent` | 声明 | `void mouseMoveEvent(QMouseEvent* event) override` | 实现 mouse move event 对应的业务或工具逻辑。 |
| [L105](../src/apps/controller/RemoteDesktopCanvas.h#L105) | `mousePressEvent` | 声明 | `void mousePressEvent(QMouseEvent* event) override` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L107](../src/apps/controller/RemoteDesktopCanvas.h#L107) | `mouseReleaseEvent` | 声明 | `void mouseReleaseEvent(QMouseEvent* event) override` | 实现 mouse release event 对应的业务或工具逻辑。 |
| [L109](../src/apps/controller/RemoteDesktopCanvas.h#L109) | `wheelEvent` | 声明 | `void wheelEvent(QWheelEvent* event) override` | 实现 wheel event 对应的业务或工具逻辑。 |
| [L111](../src/apps/controller/RemoteDesktopCanvas.h#L111) | `keyPressEvent` | 声明 | `void keyPressEvent(QKeyEvent* event) override` | 实现 key press event 对应的业务或工具逻辑。 |
| [L113](../src/apps/controller/RemoteDesktopCanvas.h#L113) | `keyReleaseEvent` | 声明 | `void keyReleaseEvent(QKeyEvent* event) override` | 实现 key release event 对应的业务或工具逻辑。 |
| [L115](../src/apps/controller/RemoteDesktopCanvas.h#L115) | `dragEnterEvent` | 声明 | `void dragEnterEvent(QDragEnterEvent* event) override` | 实现 drag enter event 对应的业务或工具逻辑。 |
| [L117](../src/apps/controller/RemoteDesktopCanvas.h#L117) | `dropEvent` | 声明 | `void dropEvent(QDropEvent* event) override` | 实现 drop event 对应的业务或工具逻辑。 |
| [L119](../src/apps/controller/RemoteDesktopCanvas.h#L119) | `focusOutEvent` | 声明 | `void focusOutEvent(QFocusEvent* event) override` | 实现 focus out event 对应的业务或工具逻辑。 |
| [L121](../src/apps/controller/RemoteDesktopCanvas.h#L121) | `resizeEvent` | 声明 | `void resizeEvent(QResizeEvent* event) override` | 实现 resize event 对应的业务或工具逻辑。 |
| [L123](../src/apps/controller/RemoteDesktopCanvas.h#L123) | `event` | 声明 | `bool event(QEvent* event) override` | 实现 event 对应的业务或工具逻辑。 |
| [L125](../src/apps/controller/RemoteDesktopCanvas.h#L125) | `paintEvent` | 声明 | `void paintEvent(QPaintEvent*) override` | 准备或呈现 paint event 相关逻辑。 |
| [L128](../src/apps/controller/RemoteDesktopCanvas.h#L128) | `IsCpuNv12Image` | 声明 | `static bool IsCpuNv12Image(const QImage& image)` | 判断 is cpu nv12 image 相关逻辑。 |
| [L130](../src/apps/controller/RemoteDesktopCanvas.h#L130) | `UpdateCpuCanvasMetrics` | 声明 | `void UpdateCpuCanvasMetrics()` | 更新或应用 update cpu canvas metrics 相关逻辑。 |
| [L132](../src/apps/controller/RemoteDesktopCanvas.h#L132) | `CpuQtOutputSize` | 声明 | `QSize CpuQtOutputSize(int sourceWidth, int sourceHeight) const` | 实现 cpu qt output size 对应的业务或工具逻辑。 |
| [L134](../src/apps/controller/RemoteDesktopCanvas.h#L134) | `QueueCpuFrame` | 声明 | `void QueueCpuFrame( webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame, bool recordArrival = true)` | 实现 queue cpu frame 对应的业务或工具逻辑。 |
| [L138](../src/apps/controller/RemoteDesktopCanvas.h#L138) | `CpuConversionLoop` | 声明 | `void CpuConversionLoop(std::stop_token stopToken)` | 实现 cpu conversion loop 对应的业务或工具逻辑。 |
| [L140](../src/apps/controller/RemoteDesktopCanvas.h#L140) | `RecycleCpuImage` | 声明 | `void RecycleCpuImage(QImage image)` | 实现 recycle cpu image 对应的业务或工具逻辑。 |
| [L142](../src/apps/controller/RemoteDesktopCanvas.h#L142) | `QueueConvertedCpuImage` | 声明 | `void QueueConvertedCpuImage(QImage image, std::uint64_t sequence)` | 实现 queue converted cpu image 对应的业务或工具逻辑。 |
| [L144](../src/apps/controller/RemoteDesktopCanvas.h#L144) | `QueueD3D11CpuImage` | 声明 | `void QueueD3D11CpuImage(QImage image, std::uint64_t sequence)` | 实现 queue d3 d11 cpu image 对应的业务或工具逻辑。 |
| [L146](../src/apps/controller/RemoteDesktopCanvas.h#L146) | `QueueQtCpuImage` | 声明 | `void QueueQtCpuImage(QImage image, std::uint64_t sequence)` | 实现 queue qt cpu image 对应的业务或工具逻辑。 |
| [L148](../src/apps/controller/RemoteDesktopCanvas.h#L148) | `ApplyPendingCpuImage` | 声明 | `void ApplyPendingCpuImage()` | 更新或应用 apply pending cpu image 相关逻辑。 |
| [L150](../src/apps/controller/RemoteDesktopCanvas.h#L150) | `QueueNativeFrame` | 声明 | `void QueueNativeFrame( webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame)` | 实现 queue native frame 对应的业务或工具逻辑。 |
| [L153](../src/apps/controller/RemoteDesktopCanvas.h#L153) | `QueueI420Frame` | 声明 | `void QueueI420Frame( webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame)` | 实现 queue i420 frame 对应的业务或工具逻辑。 |
| [L156](../src/apps/controller/RemoteDesktopCanvas.h#L156) | `ScheduleNativeUiUpdate` | 声明 | `void ScheduleNativeUiUpdate()` | 执行后台循环或调度 schedule native ui update 相关逻辑。 |
| [L158](../src/apps/controller/RemoteDesktopCanvas.h#L158) | `UpdateCpuConversionInterval` | 声明 | `void UpdateCpuConversionInterval()` | 更新或应用 update cpu conversion interval 相关逻辑。 |
| [L160](../src/apps/controller/RemoteDesktopCanvas.h#L160) | `RequestNativeRedraw` | 声明 | `void RequestNativeRedraw()` | 发起请求或查询 request native redraw 相关逻辑。 |
| [L162](../src/apps/controller/RemoteDesktopCanvas.h#L162) | `CursorVisualChanged` | 声明 | `void CursorVisualChanged()` | 实现 cursor visual changed 对应的业务或工具逻辑。 |
| [L164](../src/apps/controller/RemoteDesktopCanvas.h#L164) | `DeactivateNativePresentation` | 声明 | `bool DeactivateNativePresentation(std::uint64_t cpuSequence)` | 实现 deactivate native presentation 对应的业务或工具逻辑。 |
| [L166](../src/apps/controller/RemoteDesktopCanvas.h#L166) | `NativePresentationLoop` | 声明 | `void NativePresentationLoop(std::stop_token stopToken)` | 实现 native presentation loop 对应的业务或工具逻辑。 |
| [L167](../src/apps/controller/RemoteDesktopCanvas.h#L167) | `UpdateNativeSurfaceGeometry` | 声明 | `void UpdateNativeSurfaceGeometry()` | 更新或应用 update native surface geometry 相关逻辑。 |
| [L169](../src/apps/controller/RemoteDesktopCanvas.h#L169) | `VideoContentRect` | 声明 | `QRect VideoContentRect() const` | 实现 video content rect 对应的业务或工具逻辑。 |
| [L171](../src/apps/controller/RemoteDesktopCanvas.h#L171) | `VideoSourceSize` | 声明 | `QSize VideoSourceSize() const` | 实现 video source size 对应的业务或工具逻辑。 |
| [L174](../src/apps/controller/RemoteDesktopCanvas.h#L174) | `SampleCurrentDragPointer` | 声明 | `SampleCurrentDragPointer( const RemoteInputEvent& fallback) const` | 实现 sample current drag pointer 对应的业务或工具逻辑。 |
| [L177](../src/apps/controller/RemoteDesktopCanvas.h#L177) | `MapPoint` | 声明 | `std::optional<std::pair<std::uint16_t, std::uint16_t>> MapPoint( const QPointF& point, bool clampToContent) const` | 实现 map point 对应的业务或工具逻辑。 |
| [L181](../src/apps/controller/RemoteDesktopCanvas.h#L181) | `MapMouseButton` | 声明 | `static std::optional<RemoteMouseButton> MapMouseButton( Qt::MouseButton button)` | 实现 map mouse button 对应的业务或工具逻辑。 |
| [L184](../src/apps/controller/RemoteDesktopCanvas.h#L184) | `MapMouseButtons` | 声明 | `static std::uint8_t MapMouseButtons(Qt::MouseButtons buttons)` | 实现 map mouse buttons 对应的业务或工具逻辑。 |
| [L186](../src/apps/controller/RemoteDesktopCanvas.h#L186) | `ClampWheelDelta` | 声明 | `static std::int16_t ClampWheelDelta(int value)` | 实现 clamp wheel delta 对应的业务或工具逻辑。 |
| [L188](../src/apps/controller/RemoteDesktopCanvas.h#L188) | `IsExtendedVirtualKey` | 声明 | `static bool IsExtendedVirtualKey(std::uint32_t virtualKey)` | 判断 is extended virtual key 相关逻辑。 |
| [L190](../src/apps/controller/RemoteDesktopCanvas.h#L190) | `QueuePointerMove` | 声明 | `bool QueuePointerMove( const std::pair<std::uint16_t, std::uint16_t>& point, std::uint8_t pressedMouseButtons)` | 实现 queue pointer move 对应的业务或工具逻辑。 |
| [L194](../src/apps/controller/RemoteDesktopCanvas.h#L194) | `FlushPendingPointerMove` | 声明 | `void FlushPendingPointerMove()` | 实现 flush pending pointer move 对应的业务或工具逻辑。 |
| [L196](../src/apps/controller/RemoteDesktopCanvas.h#L196) | `CancelPendingPointerMove` | 声明 | `void CancelPendingPointerMove()` | 判断 cancel pending pointer move 相关逻辑。 |
| [L198](../src/apps/controller/RemoteDesktopCanvas.h#L198) | `SendKeyEvent` | 声明 | `bool SendKeyEvent(QKeyEvent* event, bool pressed)` | 发送或发布 send key event 相关逻辑。 |
| [L200](../src/apps/controller/RemoteDesktopCanvas.h#L200) | `FocusRemotePasteTarget` | 声明 | `void FocusRemotePasteTarget(const QPointF& position)` | 实现 focus remote paste target 对应的业务或工具逻辑。 |
| [L218](../src/apps/controller/RemoteDesktopCanvas.h#L218) | `QStringLiteral` | 声明 | `QStringLiteral("正在建立远程会话")` | 实现 q string literal 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteInputDispatcher.h`

[打开源码](../src/apps/controller/RemoteInputDispatcher.h) · **文件作用：** 声明 remote input dispatcher 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L20](../src/apps/controller/RemoteInputDispatcher.h#L20) | `RemoteInputDispatcher` | class | 定义 RemoteInputDispatcher 的 class 类型和相关状态。 |
| [L54](../src/apps/controller/RemoteInputDispatcher.h#L54) | `HighResolutionPointerMoveScheduler` | class | 定义 HighResolutionPointerMoveScheduler 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L49](../src/apps/controller/RemoteInputDispatcher.h#L49) | `mutex_` | `std::mutex mutex_;` | 保护跨线程共享状态：mutex。 |
| [L50](../src/apps/controller/RemoteInputDispatcher.h#L50) | `media_` | `app::ISessionMediaAccess* media_ = nullptr;` | 保存 media 相关配置或运行状态。 |
| [L51](../src/apps/controller/RemoteInputDispatcher.h#L51) | `enabled_` | `bool enabled_ = false;` | 保存能力或开关状态：enabled。 |
| [L471](../src/apps/controller/RemoteInputDispatcher.h#L471) | `sender_` | `InputSender sender_;` | 保存 sender 相关配置或运行状态。 |
| [L472](../src/apps/controller/RemoteInputDispatcher.h#L472) | `dragSampleProvider_` | `DragSampleProvider dragSampleProvider_;` | 保存 drag sample provider 相关配置或运行状态。 |
| [L473](../src/apps/controller/RemoteInputDispatcher.h#L473) | `wakeEvent_` | `HANDLE wakeEvent_ = nullptr;` | 保存 wake event 相关配置或运行状态。 |
| [L474](../src/apps/controller/RemoteInputDispatcher.h#L474) | `waitableTimer_` | `HANDLE waitableTimer_ = nullptr;` | 保存定时、截止或超时状态：waitable timer。 |
| [L475](../src/apps/controller/RemoteInputDispatcher.h#L475) | `worker_` | `std::jthread worker_;` | 拥有后台执行线程或工作器：worker。 |
| [L476](../src/apps/controller/RemoteInputDispatcher.h#L476) | `stateMutex_` | `std::mutex stateMutex_;` | 保护跨线程共享状态：state mutex。 |
| [L477](../src/apps/controller/RemoteInputDispatcher.h#L477) | `dispatchMutex_` | `std::mutex dispatchMutex_;` | 保护跨线程共享状态：dispatch mutex。 |
| [L478](../src/apps/controller/RemoteInputDispatcher.h#L478) | `pendingInput_` | `std::optional<RemoteInputEvent> pendingInput_;` | 保存 pending input 相关配置或运行状态。 |
| [L479](../src/apps/controller/RemoteInputDispatcher.h#L479) | `dragInput_` | `RemoteInputEvent dragInput_;` | 保存 drag input 相关配置或运行状态。 |
| [L480](../src/apps/controller/RemoteInputDispatcher.h#L480) | `pendingInterval_` | `std::chrono::microseconds pendingInterval_{8'333};` | 保存 pending interval 相关配置或运行状态。 |
| [L481](../src/apps/controller/RemoteInputDispatcher.h#L481) | `dragInterval_` | `std::chrono::microseconds dragInterval_{4'166};` | 保存 drag interval 相关配置或运行状态。 |
| [L482](../src/apps/controller/RemoteInputDispatcher.h#L482) | `lastDispatchAt_` | `std::chrono::steady_clock::time_point lastDispatchAt_;` | 保存 last dispatch at 相关配置或运行状态。 |
| [L483](../src/apps/controller/RemoteInputDispatcher.h#L483) | `nextDispatchAt_` | `std::chrono::steady_clock::time_point nextDispatchAt_;` | 保存 next dispatch at 相关配置或运行状态。 |
| [L484](../src/apps/controller/RemoteInputDispatcher.h#L484) | `lastDragX_` | `std::uint16_t lastDragX_ = 0;` | 保存 last drag x 相关配置或运行状态。 |
| [L485](../src/apps/controller/RemoteInputDispatcher.h#L485) | `lastDragY_` | `std::uint16_t lastDragY_ = 0;` | 保存 last drag y 相关配置或运行状态。 |
| [L486](../src/apps/controller/RemoteInputDispatcher.h#L486) | `hasCadence_` | `bool hasCadence_ = false;` | 保存 has cadence 相关配置或运行状态。 |
| [L487](../src/apps/controller/RemoteInputDispatcher.h#L487) | `dragActive_` | `bool dragActive_ = false;` | 保存能力或开关状态：drag active。 |
| [L488](../src/apps/controller/RemoteInputDispatcher.h#L488) | `lastDragPointValid_` | `bool lastDragPointValid_ = false;` | 保存身份或作用域标识：last drag point valid。 |
| [L489](../src/apps/controller/RemoteInputDispatcher.h#L489) | `stopping_` | `bool stopping_ = false;` | 保存 stopping 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L22](../src/apps/controller/RemoteInputDispatcher.h#L22) | `SetMediaAccess` | 定义 | `void SetMediaAccess(app::ISessionMediaAccess* media)` | 更新或应用 set media access 相关逻辑。 |
| [L28](../src/apps/controller/RemoteInputDispatcher.h#L28) | `SetEnabled` | 定义 | `void SetEnabled(bool enabled)` | 更新或应用 set enabled 相关逻辑。 |
| [L34](../src/apps/controller/RemoteInputDispatcher.h#L34) | `Clear` | 定义 | `void Clear()` | 重置或移除 clear 相关逻辑。 |
| [L41](../src/apps/controller/RemoteInputDispatcher.h#L41) | `Send` | 定义 | `bool Send(const RemoteInputEvent& event)` | 发送或发布 send 相关逻辑。 |
| [L62](../src/apps/controller/RemoteInputDispatcher.h#L62) | `HighResolutionPointerMoveScheduler` | 定义 | `explicit HighResolutionPointerMoveScheduler( InputSender sender, DragSampleProvider dragSampleProvider) : sender_(std::move(sender)), dragSampleProvider_( std::move(dragSampleProvider))` | 实现 high resolution pointer move scheduler 对应的业务或工具逻辑。 |
| [L90](../src/apps/controller/RemoteInputDispatcher.h#L90) | `~HighResolutionPointerMoveScheduler` | 定义 | `~HighResolutionPointerMoveScheduler()` | 停止相关活动并释放 HighResolutionPointerMoveScheduler 实例拥有的资源。 |
| [L95](../src/apps/controller/RemoteInputDispatcher.h#L95) | `HighResolutionPointerMoveScheduler` | 声明 | `HighResolutionPointerMoveScheduler( const HighResolutionPointerMoveScheduler&) = delete` | 实现 high resolution pointer move scheduler 对应的业务或工具逻辑。 |
| [L100](../src/apps/controller/RemoteInputDispatcher.h#L100) | `Queue` | 定义 | `bool Queue( const RemoteInputEvent& input, std::uint32_t rateLimitHz)` | 实现 queue 对应的业务或工具逻辑。 |
| [L161](../src/apps/controller/RemoteInputDispatcher.h#L161) | `BeginDrag` | 定义 | `void BeginDrag( const RemoteInputEvent& input, std::uint32_t sampleRateHz)` | 启动 begin drag 相关逻辑。 |
| [L194](../src/apps/controller/RemoteInputDispatcher.h#L194) | `SetActiveDragSampleRate` | 定义 | `void SetActiveDragSampleRate( std::uint32_t sampleRateHz)` | 更新或应用 set active drag sample rate 相关逻辑。 |
| [L219](../src/apps/controller/RemoteInputDispatcher.h#L219) | `EndDrag` | 定义 | `void EndDrag()` | 停止 end drag 相关逻辑。 |
| [L236](../src/apps/controller/RemoteInputDispatcher.h#L236) | `Flush` | 定义 | `bool Flush()` | 实现 flush 对应的业务或工具逻辑。 |
| [L277](../src/apps/controller/RemoteInputDispatcher.h#L277) | `Cancel` | 定义 | `void Cancel()` | 判断 cancel 相关逻辑。 |
| [L294](../src/apps/controller/RemoteInputDispatcher.h#L294) | `Shutdown` | 定义 | `void Shutdown()` | 关闭并清理 shutdown 相关逻辑。 |
| [L319](../src/apps/controller/RemoteInputDispatcher.h#L319) | `Dispatch` | 定义 | `bool Dispatch(const RemoteInputEvent& input)` | 接收并处理 dispatch 相关逻辑。 |
| [L326](../src/apps/controller/RemoteInputDispatcher.h#L326) | `SampleDragInput` | 定义 | `std::optional<RemoteInputEvent> SampleDragInput( const RemoteInputEvent& fallback) const` | 实现 sample drag input 对应的业务或工具逻辑。 |
| [L339](../src/apps/controller/RemoteInputDispatcher.h#L339) | `AcceptDragPoint` | 定义 | `bool AcceptDragPoint(const RemoteInputEvent& input)` | 处理并回复 accept drag point 相关逻辑。 |
| [L358](../src/apps/controller/RemoteInputDispatcher.h#L358) | `DispatchDueInput` | 定义 | `void DispatchDueInput()` | 接收并处理 dispatch due input 相关逻辑。 |
| [L403](../src/apps/controller/RemoteInputDispatcher.h#L403) | `ArmTimer` | 定义 | `bool ArmTimer( std::chrono::steady_clock::time_point deadline)` | 实现 arm timer 对应的业务或工具逻辑。 |
| [L430](../src/apps/controller/RemoteInputDispatcher.h#L430) | `Run` | 定义 | `void Run(std::stop_token stopToken)` | 执行后台循环或调度 run 相关逻辑。 |

## `src/apps/controller/RemoteSessionActionTile.cpp`

[打开源码](../src/apps/controller/RemoteSessionActionTile.cpp) · **文件作用：** 实现 remote session action tile 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L24](../src/apps/controller/RemoteSessionActionTile.cpp#L24) | `ActionTile::ActionTile` | 定义 | `ActionTile::ActionTile(const QString& icon, const QString& text, int fixedWidth, QWidget* parent) : QPushButton(parent)` | 构造并初始化 ActionTile 实例。 |
| [L67](../src/apps/controller/RemoteSessionActionTile.cpp#L67) | `ActionTile::SetIcon` | 定义 | `void ActionTile::SetIcon(const QString& icon)` | 更新或应用 set icon 相关逻辑。 |
| [L82](../src/apps/controller/RemoteSessionActionTile.cpp#L82) | `ActionTile::SetText` | 定义 | `void ActionTile::SetText(const QString& text)` | 更新或应用 set text 相关逻辑。 |
| [L90](../src/apps/controller/RemoteSessionActionTile.cpp#L90) | `ActionTile::SetTone` | 定义 | `void ActionTile::SetTone(const QString& tone)` | 更新或应用 set tone 相关逻辑。 |
| [L99](../src/apps/controller/RemoteSessionActionTile.cpp#L99) | `ActionTile::SetInteractive` | 定义 | `void ActionTile::SetInteractive(bool interactive)` | 更新或应用 set interactive 相关逻辑。 |
| [L110](../src/apps/controller/RemoteSessionActionTile.cpp#L110) | `ActionTile::SetSlashVisible` | 定义 | `void ActionTile::SetSlashVisible(bool visible)` | 更新或应用 set slash visible 相关逻辑。 |
| [L123](../src/apps/controller/RemoteSessionActionTile.cpp#L123) | `ActionTile::enterEvent` | 定义 | `void ActionTile::enterEvent(QEnterEvent* event)` | 实现 enter event 对应的业务或工具逻辑。 |
| [L132](../src/apps/controller/RemoteSessionActionTile.cpp#L132) | `ActionTile::leaveEvent` | 定义 | `void ActionTile::leaveEvent(QEvent* event)` | 实现 leave event 对应的业务或工具逻辑。 |
| [L140](../src/apps/controller/RemoteSessionActionTile.cpp#L140) | `ActionTile::paintEvent` | 定义 | `void ActionTile::paintEvent(QPaintEvent* event)` | 准备或呈现 paint event 相关逻辑。 |
| [L216](../src/apps/controller/RemoteSessionActionTile.cpp#L216) | `ActionTile::ConfigureMorphPair` | 定义 | `void ActionTile::ConfigureMorphPair()` | 更新或应用 configure morph pair 相关逻辑。 |
| [L254](../src/apps/controller/RemoteSessionActionTile.cpp#L254) | `ActionTile::StartMorphTransition` | 定义 | `void ActionTile::StartMorphTransition(bool target)` | 启动 start morph transition 相关逻辑。 |
| [L274](../src/apps/controller/RemoteSessionActionTile.cpp#L274) | `ActionTile::StartIconTransition` | 定义 | `void ActionTile::StartIconTransition()` | 启动 start icon transition 相关逻辑。 |
| [L291](../src/apps/controller/RemoteSessionActionTile.cpp#L291) | `ActionTile::ResolveIconResource` | 定义 | `QString ActionTile::ResolveIconResource() const` | 查询并返回 resolve icon resource 相关逻辑。 |
| [L331](../src/apps/controller/RemoteSessionActionTile.cpp#L331) | `ActionTile::Repolish` | 定义 | `void ActionTile::Repolish(QWidget* widget)` | 实现 repolish 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteSessionActionTile.h`

[打开源码](../src/apps/controller/RemoteSessionActionTile.h) · **文件作用：** 声明 remote session action tile 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L12](../src/apps/controller/RemoteSessionActionTile.h#L12) | `QEnterEvent` | class | 定义 QEnterEvent 的 class 类型和相关状态。 |
| [L13](../src/apps/controller/RemoteSessionActionTile.h#L13) | `QEvent` | class | 定义 QEvent 的 class 类型和相关状态。 |
| [L14](../src/apps/controller/RemoteSessionActionTile.h#L14) | `QPaintEvent` | class | 定义 QPaintEvent 的 class 类型和相关状态。 |
| [L15](../src/apps/controller/RemoteSessionActionTile.h#L15) | `QTimer` | class | 定义 QTimer 的 class 类型和相关状态。 |
| [L16](../src/apps/controller/RemoteSessionActionTile.h#L16) | `QVariantAnimation` | class | 定义 QVariantAnimation 的 class 类型和相关状态。 |
| [L17](../src/apps/controller/RemoteSessionActionTile.h#L17) | `QWidget` | class | 定义 QWidget 的 class 类型和相关状态。 |
| [L21](../src/apps/controller/RemoteSessionActionTile.h#L21) | `ActionTile` | class | 定义 ActionTile 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L12](../src/apps/controller/RemoteSessionActionTile.h#L12) | `QEnterEvent` | `class QEnterEvent;` | 保存 q enter event 相关配置或运行状态。 |
| [L13](../src/apps/controller/RemoteSessionActionTile.h#L13) | `QEvent` | `class QEvent;` | 保存 q event 相关配置或运行状态。 |
| [L14](../src/apps/controller/RemoteSessionActionTile.h#L14) | `QPaintEvent` | `class QPaintEvent;` | 保存 q paint event 相关配置或运行状态。 |
| [L15](../src/apps/controller/RemoteSessionActionTile.h#L15) | `QTimer` | `class QTimer;` | 保存定时、截止或超时状态：q timer。 |
| [L16](../src/apps/controller/RemoteSessionActionTile.h#L16) | `QVariantAnimation` | `class QVariantAnimation;` | 保存 q variant animation 相关配置或运行状态。 |
| [L17](../src/apps/controller/RemoteSessionActionTile.h#L17) | `QWidget` | `class QWidget;` | 保存 q widget 相关配置或运行状态。 |
| [L40](../src/apps/controller/RemoteSessionActionTile.h#L40) | `kTileHeight` | `static constexpr int kTileHeight = 56;` | 定义 tile height 的编译期常量或产品边界。 |
| [L48](../src/apps/controller/RemoteSessionActionTile.h#L48) | `icon_` | `QString icon_;` | 保存 icon 相关配置或运行状态。 |
| [L49](../src/apps/controller/RemoteSessionActionTile.h#L49) | `text_` | `QString text_;` | 保存 text 相关配置或运行状态。 |
| [L50](../src/apps/controller/RemoteSessionActionTile.h#L50) | `slashVisible_` | `bool slashVisible_ = false;` | 保存 slash visible 相关配置或运行状态。 |
| [L51](../src/apps/controller/RemoteSessionActionTile.h#L51) | `transitionAnimation_` | `QVariantAnimation* transitionAnimation_ = nullptr;` | 保存 transition animation 相关配置或运行状态。 |
| [L52](../src/apps/controller/RemoteSessionActionTile.h#L52) | `iconScale_` | `qreal iconScale_ = 1.0;` | 保存 icon scale 相关配置或运行状态。 |
| [L53](../src/apps/controller/RemoteSessionActionTile.h#L53) | `morphIcon_` | `remotec::ui::morph::MorphIconCore morphIcon_;` | 保存 morph icon 相关配置或运行状态。 |
| [L54](../src/apps/controller/RemoteSessionActionTile.h#L54) | `morphSpring_` | `remotec::ui::morph::Spring morphSpring_;` | 保存 morph spring 相关配置或运行状态。 |
| [L55](../src/apps/controller/RemoteSessionActionTile.h#L55) | `morphTimer_` | `QTimer* morphTimer_ = nullptr;` | 保存定时、截止或超时状态：morph timer。 |
| [L56](../src/apps/controller/RemoteSessionActionTile.h#L56) | `morphElapsed_` | `QElapsedTimer morphElapsed_;` | 保存 morph elapsed 相关配置或运行状态。 |
| [L57](../src/apps/controller/RemoteSessionActionTile.h#L57) | `morphProgress_` | `double morphProgress_ = 0.0;` | 保存 morph progress 相关配置或运行状态。 |
| [L58](../src/apps/controller/RemoteSessionActionTile.h#L58) | `morphStart_` | `double morphStart_ = 0.0;` | 保存 morph start 相关配置或运行状态。 |
| [L59](../src/apps/controller/RemoteSessionActionTile.h#L59) | `morphEnd_` | `double morphEnd_ = 0.0;` | 保存 morph end 相关配置或运行状态。 |
| [L60](../src/apps/controller/RemoteSessionActionTile.h#L60) | `hoverMorph_` | `bool hoverMorph_ = false;` | 保存 hover morph 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L23](../src/apps/controller/RemoteSessionActionTile.h#L23) | `ActionTile` | 声明 | `ActionTile(const QString& icon, const QString& text, int fixedWidth, QWidget* parent = nullptr)` | 实现 action tile 对应的业务或工具逻辑。 |
| [L28](../src/apps/controller/RemoteSessionActionTile.h#L28) | `SetIcon` | 声明 | `void SetIcon(const QString& icon)` | 更新或应用 set icon 相关逻辑。 |
| [L29](../src/apps/controller/RemoteSessionActionTile.h#L29) | `SetText` | 声明 | `void SetText(const QString& text)` | 更新或应用 set text 相关逻辑。 |
| [L30](../src/apps/controller/RemoteSessionActionTile.h#L30) | `SetTone` | 声明 | `void SetTone(const QString& tone)` | 更新或应用 set tone 相关逻辑。 |
| [L31](../src/apps/controller/RemoteSessionActionTile.h#L31) | `SetInteractive` | 声明 | `void SetInteractive(bool interactive)` | 更新或应用 set interactive 相关逻辑。 |
| [L32](../src/apps/controller/RemoteSessionActionTile.h#L32) | `SetSlashVisible` | 声明 | `void SetSlashVisible(bool visible)` | 更新或应用 set slash visible 相关逻辑。 |
| [L35](../src/apps/controller/RemoteSessionActionTile.h#L35) | `enterEvent` | 声明 | `void enterEvent(QEnterEvent* event) override` | 实现 enter event 对应的业务或工具逻辑。 |
| [L36](../src/apps/controller/RemoteSessionActionTile.h#L36) | `leaveEvent` | 声明 | `void leaveEvent(QEvent* event) override` | 实现 leave event 对应的业务或工具逻辑。 |
| [L37](../src/apps/controller/RemoteSessionActionTile.h#L37) | `paintEvent` | 声明 | `void paintEvent(QPaintEvent* event) override` | 准备或呈现 paint event 相关逻辑。 |
| [L42](../src/apps/controller/RemoteSessionActionTile.h#L42) | `ConfigureMorphPair` | 声明 | `void ConfigureMorphPair()` | 更新或应用 configure morph pair 相关逻辑。 |
| [L43](../src/apps/controller/RemoteSessionActionTile.h#L43) | `StartMorphTransition` | 声明 | `void StartMorphTransition(bool target)` | 启动 start morph transition 相关逻辑。 |
| [L44](../src/apps/controller/RemoteSessionActionTile.h#L44) | `StartIconTransition` | 声明 | `void StartIconTransition()` | 启动 start icon transition 相关逻辑。 |
| [L45](../src/apps/controller/RemoteSessionActionTile.h#L45) | `ResolveIconResource` | 声明 | `QString ResolveIconResource() const` | 查询并返回 resolve icon resource 相关逻辑。 |
| [L46](../src/apps/controller/RemoteSessionActionTile.h#L46) | `Repolish` | 声明 | `static void Repolish(QWidget* widget)` | 实现 repolish 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteSessionBinding.h`

[打开源码](../src/apps/controller/RemoteSessionBinding.h) · **文件作用：** 声明 remote session binding 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L14](../src/apps/controller/RemoteSessionBinding.h#L14) | `RemoteSessionMode` | enum class | 定义 RemoteSessionMode 的 enum class 类型和相关状态。 |
| [L22](../src/apps/controller/RemoteSessionBinding.h#L22) | `RemoteSessionBinding` | struct | Immutable-by-convention UI routing context. Authorization origin explains how a session was admitted; mode determines how media and control are bound. Owned-device and verificat... |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L23](../src/apps/controller/RemoteSessionBinding.h#L23) | `mode` | `RemoteSessionMode mode = RemoteSessionMode::kDirect;` | 保存 mode 相关配置或运行状态。 |
| [L24](../src/apps/controller/RemoteSessionBinding.h#L24) | `origin` | `SessionOrigin origin = SessionOrigin::kManualDeviceId;` | 保存 origin 相关配置或运行状态。 |
| [L25](../src/apps/controller/RemoteSessionBinding.h#L25) | `peerDeviceId` | `QString peerDeviceId;` | 保存身份或作用域标识：peer device id。 |
| [L26](../src/apps/controller/RemoteSessionBinding.h#L26) | `peerDeviceName` | `QString peerDeviceName;` | 保存路径、地址或显示名称：peer device name。 |
| [L27](../src/apps/controller/RemoteSessionBinding.h#L27) | `roomPairId` | `QString roomPairId;` | 保存身份或作用域标识：room pair id。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L29](../src/apps/controller/RemoteSessionBinding.h#L29) | `Direct` | 定义 | `static RemoteSessionBinding Direct( QString deviceId, QString deviceName, SessionOrigin sessionOrigin)` | 实现 direct 对应的业务或工具逻辑。 |
| [L41](../src/apps/controller/RemoteSessionBinding.h#L41) | `Room` | 定义 | `static RemoteSessionBinding Room( QString deviceId, QString deviceName, QString pairId)` | 实现 room 对应的业务或工具逻辑。 |
| [L53](../src/apps/controller/RemoteSessionBinding.h#L53) | `IsDirect` | 定义 | `bool IsDirect() const { return mode == RemoteSessionMode::kDirect; }` | 判断 is direct 相关逻辑。 |
| [L54](../src/apps/controller/RemoteSessionBinding.h#L54) | `IsRoom` | 定义 | `bool IsRoom() const { return mode == RemoteSessionMode::kRoom; }` | 判断 is room 相关逻辑。 |
| [L55](../src/apps/controller/RemoteSessionBinding.h#L55) | `IsValid` | 定义 | `bool IsValid() const` | 判断 is valid 相关逻辑。 |
| [L62](../src/apps/controller/RemoteSessionBinding.h#L62) | `SameTransport` | 定义 | `bool SameTransport(const RemoteSessionBinding& other) const` | 判断 same transport 相关逻辑。 |
| [L69](../src/apps/controller/RemoteSessionBinding.h#L69) | `SourceText` | 定义 | `QString SourceText() const` | 实现 source text 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteSessionNetworkIndicator.cpp`

[打开源码](../src/apps/controller/RemoteSessionNetworkIndicator.cpp) · **文件作用：** 实现 remote session network indicator 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L14](../src/apps/controller/RemoteSessionNetworkIndicator.cpp#L14) | `NetworkSignalIndicator::NetworkSignalIndicator` | 定义 | `NetworkSignalIndicator::NetworkSignalIndicator(QWidget* parent) : QWidget(parent)` | 构造并初始化 NetworkSignalIndicator 实例。 |
| [L21](../src/apps/controller/RemoteSessionNetworkIndicator.cpp#L21) | `NetworkSignalIndicator::paintEvent` | 定义 | `void NetworkSignalIndicator::paintEvent(QPaintEvent*)` | 准备或呈现 paint event 相关逻辑。 |

## `src/apps/controller/RemoteSessionNetworkIndicator.h`

[打开源码](../src/apps/controller/RemoteSessionNetworkIndicator.h) · **文件作用：** 声明 remote session network indicator 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L8](../src/apps/controller/RemoteSessionNetworkIndicator.h#L8) | `QPaintEvent` | class | 定义 QPaintEvent 的 class 类型和相关状态。 |
| [L12](../src/apps/controller/RemoteSessionNetworkIndicator.h#L12) | `NetworkSignalIndicator` | class | 定义 NetworkSignalIndicator 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L8](../src/apps/controller/RemoteSessionNetworkIndicator.h#L8) | `QPaintEvent` | `class QPaintEvent;` | 保存 q paint event 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L14](../src/apps/controller/RemoteSessionNetworkIndicator.h#L14) | `NetworkSignalIndicator` | 声明 | `explicit NetworkSignalIndicator(QWidget* parent = nullptr)` | 实现 network signal indicator 对应的业务或工具逻辑。 |
| [L17](../src/apps/controller/RemoteSessionNetworkIndicator.h#L17) | `paintEvent` | 声明 | `void paintEvent(QPaintEvent* event) override` | 准备或呈现 paint event 相关逻辑。 |

## `src/apps/controller/RemoteSessionWindow.Diagnostics.cpp`

[打开源码](../src/apps/controller/RemoteSessionWindow.Diagnostics.cpp) · **文件作用：** 实现 remote session window diagnostics 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L18](../src/apps/controller/RemoteSessionWindow.Diagnostics.cpp#L18) | `SessionBitrateText` | 定义 | `QString SessionBitrateText(std::uint64_t bitsPerSecond)` | 实现 session bitrate text 对应的业务或工具逻辑。 |
| [L30](../src/apps/controller/RemoteSessionWindow.Diagnostics.cpp#L30) | `SessionRouteText` | 定义 | `QString SessionRouteText(const std::string& route)` | 实现 session route text 对应的业务或工具逻辑。 |
| [L46](../src/apps/controller/RemoteSessionWindow.Diagnostics.cpp#L46) | `RemoteSessionWindow::UpdateDiagnostics` | 定义 | `void RemoteSessionWindow::UpdateDiagnostics( const PeerConnectionDiagnosticsSnapshot& diagnostics)` | 更新或应用 update diagnostics 相关逻辑。 |

## `src/apps/controller/RemoteSessionWindow.DisplayMenu.cpp`

[打开源码](../src/apps/controller/RemoteSessionWindow.DisplayMenu.cpp) · **文件作用：** 实现 remote session window display menu 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L24](../src/apps/controller/RemoteSessionWindow.DisplayMenu.cpp#L24) | `RemoteSessionWindow::ShowRemoteDisplayMenu` | 定义 | `void RemoteSessionWindow::ShowRemoteDisplayMenu()` | 实现 show remote display menu 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteSessionWindow.h`

[打开源码](../src/apps/controller/RemoteSessionWindow.h) · **文件作用：** 声明 remote session window 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L26](../src/apps/controller/RemoteSessionWindow.h#L26) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L27](../src/apps/controller/RemoteSessionWindow.h#L27) | `QMenu` | class | 定义 QMenu 的 class 类型和相关状态。 |
| [L28](../src/apps/controller/RemoteSessionWindow.h#L28) | `QActionGroup` | class | 定义 QActionGroup 的 class 类型和相关状态。 |
| [L29](../src/apps/controller/RemoteSessionWindow.h#L29) | `QCloseEvent` | class | 定义 QCloseEvent 的 class 类型和相关状态。 |
| [L30](../src/apps/controller/RemoteSessionWindow.h#L30) | `QEvent` | class | 定义 QEvent 的 class 类型和相关状态。 |
| [L31](../src/apps/controller/RemoteSessionWindow.h#L31) | `QMoveEvent` | class | 定义 QMoveEvent 的 class 类型和相关状态。 |
| [L32](../src/apps/controller/RemoteSessionWindow.h#L32) | `QAbstractAnimation` | class | 定义 QAbstractAnimation 的 class 类型和相关状态。 |
| [L33](../src/apps/controller/RemoteSessionWindow.h#L33) | `QPropertyAnimation` | class | 定义 QPropertyAnimation 的 class 类型和相关状态。 |
| [L34](../src/apps/controller/RemoteSessionWindow.h#L34) | `QResizeEvent` | class | 定义 QResizeEvent 的 class 类型和相关状态。 |
| [L35](../src/apps/controller/RemoteSessionWindow.h#L35) | `QTimer` | class | 定义 QTimer 的 class 类型和相关状态。 |
| [L36](../src/apps/controller/RemoteSessionWindow.h#L36) | `QToolButton` | class | 定义 QToolButton 的 class 类型和相关状态。 |
| [L37](../src/apps/controller/RemoteSessionWindow.h#L37) | `QWidget` | class | 定义 QWidget 的 class 类型和相关状态。 |
| [L40](../src/apps/controller/RemoteSessionWindow.h#L40) | `ISessionMediaAccess` | class | 定义 ISessionMediaAccess 的 class 类型和相关状态。 |
| [L45](../src/apps/controller/RemoteSessionWindow.h#L45) | `ActionTile` | class | 定义 ActionTile 的 class 类型和相关状态。 |
| [L46](../src/apps/controller/RemoteSessionWindow.h#L46) | `RemoteCDialog` | class | 定义 RemoteCDialog 的 class 类型和相关状态。 |
| [L47](../src/apps/controller/RemoteSessionWindow.h#L47) | `RemoteInputDispatcher` | class | 定义 RemoteInputDispatcher 的 class 类型和相关状态。 |
| [L49](../src/apps/controller/RemoteSessionWindow.h#L49) | `RemoteSessionWindow` | class | 定义 RemoteSessionWindow 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L26](../src/apps/controller/RemoteSessionWindow.h#L26) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L27](../src/apps/controller/RemoteSessionWindow.h#L27) | `QMenu` | `class QMenu;` | 保存 q menu 相关配置或运行状态。 |
| [L28](../src/apps/controller/RemoteSessionWindow.h#L28) | `QActionGroup` | `class QActionGroup;` | 保存 q action group 相关配置或运行状态。 |
| [L29](../src/apps/controller/RemoteSessionWindow.h#L29) | `QCloseEvent` | `class QCloseEvent;` | 保存 q close event 相关配置或运行状态。 |
| [L30](../src/apps/controller/RemoteSessionWindow.h#L30) | `QEvent` | `class QEvent;` | 保存 q event 相关配置或运行状态。 |
| [L31](../src/apps/controller/RemoteSessionWindow.h#L31) | `QMoveEvent` | `class QMoveEvent;` | 保存 q move event 相关配置或运行状态。 |
| [L32](../src/apps/controller/RemoteSessionWindow.h#L32) | `QAbstractAnimation` | `class QAbstractAnimation;` | 保存 q abstract animation 相关配置或运行状态。 |
| [L33](../src/apps/controller/RemoteSessionWindow.h#L33) | `QPropertyAnimation` | `class QPropertyAnimation;` | 保存 q property animation 相关配置或运行状态。 |
| [L34](../src/apps/controller/RemoteSessionWindow.h#L34) | `QResizeEvent` | `class QResizeEvent;` | 保存 q resize event 相关配置或运行状态。 |
| [L35](../src/apps/controller/RemoteSessionWindow.h#L35) | `QTimer` | `class QTimer;` | 保存定时、截止或超时状态：q timer。 |
| [L36](../src/apps/controller/RemoteSessionWindow.h#L36) | `QToolButton` | `class QToolButton;` | 保存 q tool button 相关配置或运行状态。 |
| [L37](../src/apps/controller/RemoteSessionWindow.h#L37) | `QWidget` | `class QWidget;` | 保存 q widget 相关配置或运行状态。 |
| [L40](../src/apps/controller/RemoteSessionWindow.h#L40) | `ISessionMediaAccess` | `class ISessionMediaAccess;` | 保存 i session media access 相关配置或运行状态。 |
| [L45](../src/apps/controller/RemoteSessionWindow.h#L45) | `ActionTile` | `class ActionTile;` | 保存 action tile 相关配置或运行状态。 |
| [L46](../src/apps/controller/RemoteSessionWindow.h#L46) | `RemoteCDialog` | `class RemoteCDialog;` | 保存 remote c dialog 相关配置或运行状态。 |
| [L47](../src/apps/controller/RemoteSessionWindow.h#L47) | `RemoteInputDispatcher` | `class RemoteInputDispatcher;` | 保存 remote input dispatcher 相关配置或运行状态。 |
| [L140](../src/apps/controller/RemoteSessionWindow.h#L140) | `remoteInputDispatcher_` | `remoteInputDispatcher_;` | 保存 remote input dispatcher 相关配置或运行状态。 |
| [L141](../src/apps/controller/RemoteSessionWindow.h#L141) | `binding_` | `RemoteSessionBinding binding_;` | 保存 binding 相关配置或运行状态。 |
| [L142](../src/apps/controller/RemoteSessionWindow.h#L142) | `sessionSourceLabel_` | `QLabel* sessionSourceLabel_ = nullptr;` | 保存路径、地址或显示名称：session source label。 |
| [L143](../src/apps/controller/RemoteSessionWindow.h#L143) | `durationLabel_` | `QLabel* durationLabel_ = nullptr;` | 保存路径、地址或显示名称：duration label。 |
| [L144](../src/apps/controller/RemoteSessionWindow.h#L144) | `previewBadge_` | `QLabel* previewBadge_ = nullptr;` | 保存 preview badge 相关配置或运行状态。 |
| [L145](../src/apps/controller/RemoteSessionWindow.h#L145) | `controlBadge_` | `ActionTile* controlBadge_ = nullptr;` | 保存 control badge 相关配置或运行状态。 |
| [L146](../src/apps/controller/RemoteSessionWindow.h#L146) | `connectionStatusLabel_` | `QLabel* connectionStatusLabel_ = nullptr;` | 保存路径、地址或显示名称：connection status label。 |
| [L147](../src/apps/controller/RemoteSessionWindow.h#L147) | `networkSignalIndicator_` | `QWidget* networkSignalIndicator_ = nullptr;` | 保存 network signal indicator 相关配置或运行状态。 |
| [L148](../src/apps/controller/RemoteSessionWindow.h#L148) | `latencyLabel_` | `QLabel* latencyLabel_ = nullptr;` | 保存路径、地址或显示名称：latency label。 |
| [L149](../src/apps/controller/RemoteSessionWindow.h#L149) | `codecLabel_` | `QLabel* codecLabel_ = nullptr;` | 保存路径、地址或显示名称：codec label。 |
| [L150](../src/apps/controller/RemoteSessionWindow.h#L150) | `resolutionLabel_` | `QLabel* resolutionLabel_ = nullptr;` | 保存路径、地址或显示名称：resolution label。 |
| [L151](../src/apps/controller/RemoteSessionWindow.h#L151) | `controlButton_` | `ActionTile* controlButton_ = nullptr;` | 保存 control button 相关配置或运行状态。 |
| [L152](../src/apps/controller/RemoteSessionWindow.h#L152) | `qualityButton_` | `QToolButton* qualityButton_ = nullptr;` | 保存 quality button 相关配置或运行状态。 |
| [L153](../src/apps/controller/RemoteSessionWindow.h#L153) | `qualityMenu_` | `QMenu* qualityMenu_ = nullptr;` | 保存 quality menu 相关配置或运行状态。 |
| [L154](../src/apps/controller/RemoteSessionWindow.h#L154) | `qualityGroup_` | `QActionGroup* qualityGroup_ = nullptr;` | 保存 quality group 相关配置或运行状态。 |
| [L155](../src/apps/controller/RemoteSessionWindow.h#L155) | `frameRateButton_` | `QToolButton* frameRateButton_ = nullptr;` | 保存 frame rate button 相关配置或运行状态。 |
| [L156](../src/apps/controller/RemoteSessionWindow.h#L156) | `frameRateMenu_` | `QMenu* frameRateMenu_ = nullptr;` | 保存 frame rate menu 相关配置或运行状态。 |
| [L157](../src/apps/controller/RemoteSessionWindow.h#L157) | `frameRateGroup_` | `QActionGroup* frameRateGroup_ = nullptr;` | 保存 frame rate group 相关配置或运行状态。 |
| [L158](../src/apps/controller/RemoteSessionWindow.h#L158) | `speakerButton_` | `ActionTile* speakerButton_ = nullptr;` | 保存 speaker button 相关配置或运行状态。 |
| [L159](../src/apps/controller/RemoteSessionWindow.h#L159) | `microphoneButton_` | `ActionTile* microphoneButton_ = nullptr;` | 保存 microphone button 相关配置或运行状态。 |
| [L160](../src/apps/controller/RemoteSessionWindow.h#L160) | `fileTransferButton_` | `ActionTile* fileTransferButton_ = nullptr;` | 保存 file transfer button 相关配置或运行状态。 |
| [L161](../src/apps/controller/RemoteSessionWindow.h#L161) | `mediaDeviceButton_` | `QToolButton* mediaDeviceButton_ = nullptr;` | 保存 media device button 相关配置或运行状态。 |
| [L162](../src/apps/controller/RemoteSessionWindow.h#L162) | `remoteDisplayButton_` | `QToolButton* remoteDisplayButton_ = nullptr;` | 保存 remote display button 相关配置或运行状态。 |
| [L163](../src/apps/controller/RemoteSessionWindow.h#L163) | `fullScreenButton_` | `ActionTile* fullScreenButton_ = nullptr;` | 保存 full screen button 相关配置或运行状态。 |
| [L164](../src/apps/controller/RemoteSessionWindow.h#L164) | `toolbarLockButton_` | `QToolButton* toolbarLockButton_ = nullptr;` | 保存 toolbar lock button 相关配置或运行状态。 |
| [L165](../src/apps/controller/RemoteSessionWindow.h#L165) | `remotePasteStatusButton_` | `QToolButton* remotePasteStatusButton_ = nullptr;` | 保存 remote paste status button 相关配置或运行状态。 |
| [L166](../src/apps/controller/RemoteSessionWindow.h#L166) | `hudFrameRateLabel_` | `QLabel* hudFrameRateLabel_ = nullptr;` | 保存路径、地址或显示名称：hud frame rate label。 |
| [L167](../src/apps/controller/RemoteSessionWindow.h#L167) | `sessionTitleBar_` | `CustomTitleBar* sessionTitleBar_ = nullptr;` | 保存 session title bar 相关配置或运行状态。 |
| [L168](../src/apps/controller/RemoteSessionWindow.h#L168) | `contentHost_` | `QWidget* contentHost_ = nullptr;` | 保存 content host 相关配置或运行状态。 |
| [L169](../src/apps/controller/RemoteSessionWindow.h#L169) | `toolbarRevealZone_` | `QWidget* toolbarRevealZone_ = nullptr;` | 保存 toolbar reveal zone 相关配置或运行状态。 |
| [L170](../src/apps/controller/RemoteSessionWindow.h#L170) | `sessionToolbar_` | `QWidget* sessionToolbar_ = nullptr;` | 保存 session toolbar 相关配置或运行状态。 |
| [L171](../src/apps/controller/RemoteSessionWindow.h#L171) | `sessionHud_` | `QWidget* sessionHud_ = nullptr;` | 保存 session hud 相关配置或运行状态。 |
| [L172](../src/apps/controller/RemoteSessionWindow.h#L172) | `remotePasteStatusHost_` | `QWidget* remotePasteStatusHost_ = nullptr;` | 保存 remote paste status host 相关配置或运行状态。 |
| [L173](../src/apps/controller/RemoteSessionWindow.h#L173) | `desktopCanvas_` | `QWidget* desktopCanvas_ = nullptr;` | 保存 desktop canvas 相关配置或运行状态。 |
| [L174](../src/apps/controller/RemoteSessionWindow.h#L174) | `toolbarHideTimer_` | `QTimer* toolbarHideTimer_ = nullptr;` | 保存定时、截止或超时状态：toolbar hide timer。 |
| [L175](../src/apps/controller/RemoteSessionWindow.h#L175) | `toolbarAnimation_` | `QPropertyAnimation* toolbarAnimation_ = nullptr;` | 保存 toolbar animation 相关配置或运行状态。 |
| [L176](../src/apps/controller/RemoteSessionWindow.h#L176) | `sessionElapsed_` | `QElapsedTimer sessionElapsed_;` | 保存 session elapsed 相关配置或运行状态。 |
| [L177](../src/apps/controller/RemoteSessionWindow.h#L177) | `screenStartupElapsed_` | `QElapsedTimer screenStartupElapsed_;` | 保存 screen startup elapsed 相关配置或运行状态。 |
| [L178](../src/apps/controller/RemoteSessionWindow.h#L178) | `durationTimer_` | `QTimer* durationTimer_ = nullptr;` | 保存定时、截止或超时状态：duration timer。 |
| [L179](../src/apps/controller/RemoteSessionWindow.h#L179) | `sessionControl_` | `IRemoteSessionControl* sessionControl_ = nullptr;` | 保存 session control 相关配置或运行状态。 |
| [L180](../src/apps/controller/RemoteSessionWindow.h#L180) | `sessionMedia_` | `app::ISessionMediaAccess* sessionMedia_ = nullptr;` | 保存 session media 相关配置或运行状态。 |
| [L181](../src/apps/controller/RemoteSessionWindow.h#L181) | `sessionVideoSinkBound_` | `bool sessionVideoSinkBound_ = false;` | 保存 session video sink bound 相关配置或运行状态。 |
| [L182](../src/apps/controller/RemoteSessionWindow.h#L182) | `sessionVideoSinkRetryScheduled_` | `bool sessionVideoSinkRetryScheduled_ = false;` | 保存 session video sink retry scheduled 相关配置或运行状态。 |
| [L183](../src/apps/controller/RemoteSessionWindow.h#L183) | `roomScreenPreferenceRetryScheduled_` | `bool roomScreenPreferenceRetryScheduled_ = false;` | 保存 room screen preference retry scheduled 相关配置或运行状态。 |
| [L184](../src/apps/controller/RemoteSessionWindow.h#L184) | `screenStartupGeneration_` | `std::uint64_t screenStartupGeneration_ = 0;` | 标记当前世代，用于拒绝过期异步结果：screen startup generation。 |
| [L185](../src/apps/controller/RemoteSessionWindow.h#L185) | `screenStartupRefreshAttempts_` | `std::uint32_t screenStartupRefreshAttempts_ = 0;` | 保存 screen startup refresh attempts 相关配置或运行状态。 |
| [L186](../src/apps/controller/RemoteSessionWindow.h#L186) | `screenFirstFramePresented_` | `bool screenFirstFramePresented_ = false;` | 保存 screen first frame presented 相关配置或运行状态。 |
| [L187](../src/apps/controller/RemoteSessionWindow.h#L187) | `preferenceRequestedScreenShareEpoch_` | `std::uint64_t preferenceRequestedScreenShareEpoch_ = 0;` | 标记当前世代，用于拒绝过期异步结果：preference requested screen share epoch。 |
| [L188](../src/apps/controller/RemoteSessionWindow.h#L188) | `preferenceSentScreenShareEpoch_` | `std::uint64_t preferenceSentScreenShareEpoch_ = 0;` | 标记当前世代，用于拒绝过期异步结果：preference sent screen share epoch。 |
| [L189](../src/apps/controller/RemoteSessionWindow.h#L189) | `pendingRemoteDisplaySwitchSequence_` | `std::uint64_t pendingRemoteDisplaySwitchSequence_ = 0;` | 保存单调序号，用于排序或去重：pending remote display switch sequence。 |
| [L190](../src/apps/controller/RemoteSessionWindow.h#L190) | `lastControlActionText_` | `QString lastControlActionText_;` | 保存 last control action text 相关配置或运行状态。 |
| [L191](../src/apps/controller/RemoteSessionWindow.h#L191) | `inputControlEnabled_` | `bool inputControlEnabled_ = false;` | 保存能力或开关状态：input control enabled。 |
| [L192](../src/apps/controller/RemoteSessionWindow.h#L192) | `closeWithoutConfirmation_` | `bool closeWithoutConfirmation_ = false;` | 保存 close without confirmation 相关配置或运行状态。 |
| [L193](../src/apps/controller/RemoteSessionWindow.h#L193) | `closeConfirmationVisible_` | `bool closeConfirmationVisible_ = false;` | 保存 close confirmation visible 相关配置或运行状态。 |
| [L194](../src/apps/controller/RemoteSessionWindow.h#L194) | `networkRecoveryDialog_` | `RemoteCDialog* networkRecoveryDialog_ = nullptr;` | 保存 network recovery dialog 相关配置或运行状态。 |
| [L195](../src/apps/controller/RemoteSessionWindow.h#L195) | `remotePasteDialog_` | `RemoteCDialog* remotePasteDialog_ = nullptr;` | 保存 remote paste dialog 相关配置或运行状态。 |
| [L196](../src/apps/controller/RemoteSessionWindow.h#L196) | `remotePasteTransferId_` | `QString remotePasteTransferId_;` | 保存身份或作用域标识：remote paste transfer id。 |
| [L197](../src/apps/controller/RemoteSessionWindow.h#L197) | `remotePastePromptDismissed_` | `bool remotePastePromptDismissed_ = false;` | 保存 remote paste prompt dismissed 相关配置或运行状态。 |
| [L198](../src/apps/controller/RemoteSessionWindow.h#L198) | `remotePasteDialogClosing_` | `bool remotePasteDialogClosing_ = false;` | 保存 remote paste dialog closing 相关配置或运行状态。 |
| [L199](../src/apps/controller/RemoteSessionWindow.h#L199) | `remotePasteMinimized_` | `bool remotePasteMinimized_ = false;` | 保存 remote paste minimized 相关配置或运行状态。 |
| [L200](../src/apps/controller/RemoteSessionWindow.h#L200) | `remotePasteRestoreAnimating_` | `bool remotePasteRestoreAnimating_ = false;` | 保存 remote paste restore animating 相关配置或运行状态。 |
| [L201](../src/apps/controller/RemoteSessionWindow.h#L201) | `remotePasteRestoreGeometry_` | `QRect remotePasteRestoreGeometry_;` | 保存 remote paste restore geometry 相关配置或运行状态。 |
| [L202](../src/apps/controller/RemoteSessionWindow.h#L202) | `remotePasteAnimation_` | `QAbstractAnimation* remotePasteAnimation_ = nullptr;` | 保存 remote paste animation 相关配置或运行状态。 |
| [L203](../src/apps/controller/RemoteSessionWindow.h#L203) | `remotePasteAnimationOverlay_` | `QWidget* remotePasteAnimationOverlay_ = nullptr;` | 保存 remote paste animation overlay 相关配置或运行状态。 |
| [L204](../src/apps/controller/RemoteSessionWindow.h#L204) | `remotePasteDialogSnapshot_` | `QPixmap remotePasteDialogSnapshot_;` | 保存可跨层读取的状态快照：remote paste dialog snapshot。 |
| [L205](../src/apps/controller/RemoteSessionWindow.h#L205) | `pairWasActive_` | `bool pairWasActive_ = false;` | 保存能力或开关状态：pair was active。 |
| [L206](../src/apps/controller/RemoteSessionWindow.h#L206) | `recoveryPromptWasVisible_` | `bool recoveryPromptWasVisible_ = false;` | 保存 recovery prompt was visible 相关配置或运行状态。 |
| [L207](../src/apps/controller/RemoteSessionWindow.h#L207) | `networkRecoveryPromptDismissed_` | `bool networkRecoveryPromptDismissed_ = false;` | 保存 network recovery prompt dismissed 相关配置或运行状态。 |
| [L208](../src/apps/controller/RemoteSessionWindow.h#L208) | `networkRecoveryFailureDismissed_` | `bool networkRecoveryFailureDismissed_ = false;` | 保存 network recovery failure dismissed 相关配置或运行状态。 |
| [L209](../src/apps/controller/RemoteSessionWindow.h#L209) | `networkRecoveryShowingFailure_` | `bool networkRecoveryShowingFailure_ = false;` | 保存最近错误或失败原因：network recovery showing failure。 |
| [L210](../src/apps/controller/RemoteSessionWindow.h#L210) | `selectedFrameRate_` | `std::uint32_t selectedFrameRate_ = kDefaultScreenFrameRate;` | 保存计数、尺寸或速率指标：selected frame rate。 |
| [L211](../src/apps/controller/RemoteSessionWindow.h#L211) | `dragPointerSampleRateHz_` | `std::uint32_t dragPointerSampleRateHz_ = 240;` | 保存 drag pointer sample rate hz 相关配置或运行状态。 |
| [L212](../src/apps/controller/RemoteSessionWindow.h#L212) | `remoteMaximumFrameRate_` | `std::uint32_t remoteMaximumFrameRate_ = 120;` | 保存计数、尺寸或速率指标：remote maximum frame rate。 |
| [L213](../src/apps/controller/RemoteSessionWindow.h#L213) | `reportedRemoteMaximumFrameRate_` | `std::uint32_t reportedRemoteMaximumFrameRate_ = 120;` | 保存计数、尺寸或速率指标：reported remote maximum frame rate。 |
| [L214](../src/apps/controller/RemoteSessionWindow.h#L214) | `roomMaximumFrameRate_` | `std::uint32_t roomMaximumFrameRate_ = 120;` | 保存计数、尺寸或速率指标：room maximum frame rate。 |
| [L215](../src/apps/controller/RemoteSessionWindow.h#L215) | `remoteSourceWidth_` | `std::uint32_t remoteSourceWidth_ = 0;` | 保存计数、尺寸或速率指标：remote source width。 |
| [L216](../src/apps/controller/RemoteSessionWindow.h#L216) | `remoteSourceHeight_` | `std::uint32_t remoteSourceHeight_ = 0;` | 保存计数、尺寸或速率指标：remote source height。 |
| [L217](../src/apps/controller/RemoteSessionWindow.h#L217) | `selectedQuality_` | `ScreenQualityTier selectedQuality_ = ScreenQualityTier::kOriginal;` | 保存 selected quality 相关配置或运行状态。 |
| [L218](../src/apps/controller/RemoteSessionWindow.h#L218) | `toolbarShown_` | `bool toolbarShown_ = true;` | 保存 toolbar shown 相关配置或运行状态。 |
| [L219](../src/apps/controller/RemoteSessionWindow.h#L219) | `toolbarLocked_` | `bool toolbarLocked_ = false;` | 保存 toolbar locked 相关配置或运行状态。 |
| [L220](../src/apps/controller/RemoteSessionWindow.h#L220) | `fullScreenMode_` | `bool fullScreenMode_ = false;` | 保存 full screen mode 相关配置或运行状态。 |
| [L221](../src/apps/controller/RemoteSessionWindow.h#L221) | `mediaDeviceMenuOpen_` | `bool mediaDeviceMenuOpen_ = false;` | 保存能力或开关状态：media device menu open。 |
| [L222](../src/apps/controller/RemoteSessionWindow.h#L222) | `pendingCameraDeviceId_` | `QString pendingCameraDeviceId_;` | 保存身份或作用域标识：pending camera device id。 |
| [L223](../src/apps/controller/RemoteSessionWindow.h#L223) | `pendingMicrophoneDeviceId_` | `QString pendingMicrophoneDeviceId_;` | 保存身份或作用域标识：pending microphone device id。 |
| [L224](../src/apps/controller/RemoteSessionWindow.h#L224) | `pendingSpeakerDeviceId_` | `QString pendingSpeakerDeviceId_;` | 保存身份或作用域标识：pending speaker device id。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L51](../src/apps/controller/RemoteSessionWindow.h#L51) | `RemoteSessionWindow` | 声明 | `RemoteSessionWindow(RemoteSessionBinding binding, IRemoteSessionControl* sessionControl = nullptr, app::ISessionMediaAccess* sessionMedia = nullptr, QWidget* parent = nullptr)` | 实现 remote session window 对应的业务或工具逻辑。 |
| [L55](../src/apps/controller/RemoteSessionWindow.h#L55) | `~RemoteSessionWindow` | 声明 | `~RemoteSessionWindow() override` | 停止相关活动并释放 RemoteSessionWindow 实例拥有的资源。 |
| [L57](../src/apps/controller/RemoteSessionWindow.h#L57) | `RemoteSessionWindow` | 声明 | `RemoteSessionWindow(const RemoteSessionWindow&) = delete` | 实现 remote session window 对应的业务或工具逻辑。 |
| [L60](../src/apps/controller/RemoteSessionWindow.h#L60) | `BindSessionVideo` | 声明 | `void BindSessionVideo(IRemoteSessionControl* sessionControl, app::ISessionMediaAccess* media, RemoteSessionBinding binding)` | 实现 bind session video 对应的业务或工具逻辑。 |
| [L63](../src/apps/controller/RemoteSessionWindow.h#L63) | `RefreshControlState` | 声明 | `void RefreshControlState()` | 刷新 refresh control state 相关逻辑。 |
| [L64](../src/apps/controller/RemoteSessionWindow.h#L64) | `UpdateDiagnostics` | 声明 | `void UpdateDiagnostics( const PeerConnectionDiagnosticsSnapshot& diagnostics)` | 更新或应用 update diagnostics 相关逻辑。 |
| [L66](../src/apps/controller/RemoteSessionWindow.h#L66) | `SetDisconnectHandler` | 声明 | `void SetDisconnectHandler(std::function<void()> handler)` | 更新或应用 set disconnect handler 相关逻辑。 |
| [L67](../src/apps/controller/RemoteSessionWindow.h#L67) | `SetRemotePasteHandler` | 声明 | `void SetRemotePasteHandler( std::function<bool(const QStringList& localFiles, bool keyboardPaste)> handler)` | 更新或应用 set remote paste handler 相关逻辑。 |
| [L70](../src/apps/controller/RemoteSessionWindow.h#L70) | `SetRemotePasteCancelHandler` | 声明 | `void SetRemotePasteCancelHandler(std::function<void()> handler)` | 更新或应用 set remote paste cancel handler 相关逻辑。 |
| [L71](../src/apps/controller/RemoteSessionWindow.h#L71) | `SetFileTransferHandlers` | 声明 | `void SetFileTransferHandlers( std::function<void()> openHandler, std::function<void()> releaseHostHandler)` | 更新或应用 set file transfer handlers 相关逻辑。 |
| [L74](../src/apps/controller/RemoteSessionWindow.h#L74) | `ShowRemotePasteProgress` | 声明 | `void ShowRemotePasteProgress( const QString& transferId, const QString& title, const QString& message, double progress, std::uintptr_t localTargetWindow = 0)` | 实现 show remote paste progress 对应的业务或工具逻辑。 |
| [L78](../src/apps/controller/RemoteSessionWindow.h#L78) | `CompleteRemotePasteProgress` | 声明 | `void CompleteRemotePasteProgress(const QString& transferId)` | 实现 complete remote paste progress 对应的业务或工具逻辑。 |
| [L79](../src/apps/controller/RemoteSessionWindow.h#L79) | `CloseRemotePasteProgress` | 声明 | `void CloseRemotePasteProgress(const QString& transferId)` | 关闭并清理 close remote paste progress 相关逻辑。 |
| [L80](../src/apps/controller/RemoteSessionWindow.h#L80) | `ShowRemotePasteFailure` | 声明 | `void ShowRemotePasteFailure( const QString& transferId, const QString& message)` | 实现 show remote paste failure 对应的业务或工具逻辑。 |
| [L82](../src/apps/controller/RemoteSessionWindow.h#L82) | `SetDragPointerSampleRate` | 声明 | `void SetDragPointerSampleRate(std::uint32_t hertz)` | 更新或应用 set drag pointer sample rate 相关逻辑。 |
| [L83](../src/apps/controller/RemoteSessionWindow.h#L83) | `SetRoomOnlineMemberCount` | 声明 | `void SetRoomOnlineMemberCount(std::size_t onlineMemberCount)` | 更新或应用 set room online member count 相关逻辑。 |
| [L86](../src/apps/controller/RemoteSessionWindow.h#L86) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L87](../src/apps/controller/RemoteSessionWindow.h#L87) | `changeEvent` | 声明 | `void changeEvent(QEvent* event) override` | 实现 change event 对应的业务或工具逻辑。 |
| [L88](../src/apps/controller/RemoteSessionWindow.h#L88) | `closeEvent` | 声明 | `void closeEvent(QCloseEvent* event) override` | 关闭并清理 close event 相关逻辑。 |
| [L89](../src/apps/controller/RemoteSessionWindow.h#L89) | `moveEvent` | 声明 | `void moveEvent(QMoveEvent* event) override` | 实现 move event 对应的业务或工具逻辑。 |
| [L90](../src/apps/controller/RemoteSessionWindow.h#L90) | `resizeEvent` | 声明 | `void resizeEvent(QResizeEvent* event) override` | 实现 resize event 对应的业务或工具逻辑。 |
| [L93](../src/apps/controller/RemoteSessionWindow.h#L93) | `BuildUi` | 声明 | `void BuildUi()` | 创建或初始化 build ui 相关逻辑。 |
| [L94](../src/apps/controller/RemoteSessionWindow.h#L94) | `LayoutSessionOverlays` | 声明 | `void LayoutSessionOverlays()` | 实现 layout session overlays 对应的业务或工具逻辑。 |
| [L95](../src/apps/controller/RemoteSessionWindow.h#L95) | `ShowSessionToolbar` | 声明 | `void ShowSessionToolbar(bool animated = true)` | 实现 show session toolbar 对应的业务或工具逻辑。 |
| [L96](../src/apps/controller/RemoteSessionWindow.h#L96) | `HideSessionToolbar` | 声明 | `void HideSessionToolbar(bool animated = true)` | 实现 hide session toolbar 对应的业务或工具逻辑。 |
| [L97](../src/apps/controller/RemoteSessionWindow.h#L97) | `ScheduleSessionToolbarHide` | 声明 | `void ScheduleSessionToolbarHide()` | 执行后台循环或调度 schedule session toolbar hide 相关逻辑。 |
| [L98](../src/apps/controller/RemoteSessionWindow.h#L98) | `UpdateSessionDuration` | 声明 | `void UpdateSessionDuration()` | 更新或应用 update session duration 相关逻辑。 |
| [L99](../src/apps/controller/RemoteSessionWindow.h#L99) | `UpdateNetworkRecoveryPrompt` | 声明 | `void UpdateNetworkRecoveryPrompt( bool recovering, std::uint32_t attempt, bool waitingForSignaling, bool failed = false)` | 更新或应用 update network recovery prompt 相关逻辑。 |
| [L104](../src/apps/controller/RemoteSessionWindow.h#L104) | `HandleControlAction` | 声明 | `void HandleControlAction()` | 接收并处理 handle control action 相关逻辑。 |
| [L105](../src/apps/controller/RemoteSessionWindow.h#L105) | `RebuildQualityMenu` | 声明 | `void RebuildQualityMenu()` | 更新或应用 rebuild quality menu 相关逻辑。 |
| [L106](../src/apps/controller/RemoteSessionWindow.h#L106) | `RebuildFrameRateMenu` | 声明 | `void RebuildFrameRateMenu()` | 更新或应用 rebuild frame rate menu 相关逻辑。 |
| [L107](../src/apps/controller/RemoteSessionWindow.h#L107) | `HandleFrameRateSelection` | 声明 | `void HandleFrameRateSelection(std::uint32_t framesPerSecond)` | 接收并处理 handle frame rate selection 相关逻辑。 |
| [L108](../src/apps/controller/RemoteSessionWindow.h#L108) | `HandleQualitySelection` | 声明 | `void HandleQualitySelection(ScreenQualityTier quality)` | 接收并处理 handle quality selection 相关逻辑。 |
| [L109](../src/apps/controller/RemoteSessionWindow.h#L109) | `RequestStreamPreference` | 声明 | `bool RequestStreamPreference(bool showError = true)` | 发起请求或查询 request stream preference 相关逻辑。 |
| [L110](../src/apps/controller/RemoteSessionWindow.h#L110) | `ToggleRemoteSound` | 声明 | `void ToggleRemoteSound()` | 实现 toggle remote sound 对应的业务或工具逻辑。 |
| [L111](../src/apps/controller/RemoteSessionWindow.h#L111) | `ToggleLocalMicrophone` | 声明 | `void ToggleLocalMicrophone()` | 实现 toggle local microphone 对应的业务或工具逻辑。 |
| [L112](../src/apps/controller/RemoteSessionWindow.h#L112) | `ShowMediaDeviceMenu` | 声明 | `void ShowMediaDeviceMenu()` | 实现 show media device menu 对应的业务或工具逻辑。 |
| [L113](../src/apps/controller/RemoteSessionWindow.h#L113) | `ShowRemoteDisplayMenu` | 声明 | `void ShowRemoteDisplayMenu()` | 实现 show remote display menu 对应的业务或工具逻辑。 |
| [L114](../src/apps/controller/RemoteSessionWindow.h#L114) | `BeginMediaDeviceSelection` | 声明 | `void BeginMediaDeviceSelection( MediaDeviceKind kind, const QString& deviceId)` | 启动 begin media device selection 相关逻辑。 |
| [L116](../src/apps/controller/RemoteSessionWindow.h#L116) | `CompleteMediaDeviceSelections` | 声明 | `void CompleteMediaDeviceSelections( const MediaDeviceSnapshot& media)` | 实现 complete media device selections 对应的业务或工具逻辑。 |
| [L118](../src/apps/controller/RemoteSessionWindow.h#L118) | `ToggleToolbarLock` | 声明 | `void ToggleToolbarLock()` | 实现 toggle toolbar lock 对应的业务或工具逻辑。 |
| [L119](../src/apps/controller/RemoteSessionWindow.h#L119) | `ToggleFullScreenMode` | 声明 | `void ToggleFullScreenMode()` | 实现 toggle full screen mode 对应的业务或工具逻辑。 |
| [L120](../src/apps/controller/RemoteSessionWindow.h#L120) | `HandleDisconnectAction` | 声明 | `void HandleDisconnectAction()` | 接收并处理 handle disconnect action 相关逻辑。 |
| [L121](../src/apps/controller/RemoteSessionWindow.h#L121) | `ReleaseRemoteInputs` | 声明 | `void ReleaseRemoteInputs()` | 释放或取消 release remote inputs 相关逻辑。 |
| [L122](../src/apps/controller/RemoteSessionWindow.h#L122) | `HandleRemoteCursorMessage` | 声明 | `void HandleRemoteCursorMessage( const std::string& pairId, const RemoteCursorEnvelope& envelope)` | 接收并处理 handle remote cursor message 相关逻辑。 |
| [L125](../src/apps/controller/RemoteSessionWindow.h#L125) | `BeginScreenStartup` | 声明 | `void BeginScreenStartup(std::uint64_t screenShareGeneration)` | 启动 begin screen startup 相关逻辑。 |
| [L126](../src/apps/controller/RemoteSessionWindow.h#L126) | `RequestScreenStartupRefresh` | 声明 | `void RequestScreenStartupRefresh( std::uint64_t screenShareGeneration, std::uint32_t retryStage)` | 发起请求或查询 request screen startup refresh 相关逻辑。 |
| [L129](../src/apps/controller/RemoteSessionWindow.h#L129) | `HandleFirstScreenPresentation` | 声明 | `void HandleFirstScreenPresentation( std::uint64_t screenShareGeneration)` | 接收并处理 handle first screen presentation 相关逻辑。 |
| [L131](../src/apps/controller/RemoteSessionWindow.h#L131) | `MinimizeRemotePasteProgress` | 声明 | `void MinimizeRemotePasteProgress()` | 实现 minimize remote paste progress 对应的业务或工具逻辑。 |
| [L132](../src/apps/controller/RemoteSessionWindow.h#L132) | `RestoreRemotePasteProgress` | 声明 | `void RestoreRemotePasteProgress()` | 读取或恢复 restore remote paste progress 相关逻辑。 |
| [L133](../src/apps/controller/RemoteSessionWindow.h#L133) | `SetRemotePasteStatusButtonVisible` | 声明 | `void SetRemotePasteStatusButtonVisible(bool visible)` | 更新或应用 set remote paste status button visible 相关逻辑。 |
| [L134](../src/apps/controller/RemoteSessionWindow.h#L134) | `RemotePasteDialogRestoreGeometry` | 声明 | `QRect RemotePasteDialogRestoreGeometry() const` | 实现 remote paste dialog restore geometry 对应的业务或工具逻辑。 |
| [L135](../src/apps/controller/RemoteSessionWindow.h#L135) | `CreateRemotePasteAnimationOverlay` | 声明 | `QLabel* CreateRemotePasteAnimationOverlay( const QPixmap& snapshot, const QRect& geometry, qreal opacity = 1.0)` | 创建或初始化 create remote paste animation overlay 相关逻辑。 |

## `src/apps/controller/RemoteSessionWindow.Layout.cpp`

[打开源码](../src/apps/controller/RemoteSessionWindow.Layout.cpp) · **文件作用：** 实现 remote session window layout 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L25](../src/apps/controller/RemoteSessionWindow.Layout.cpp#L25) | `RemoteSessionWindow::eventFilter` | 定义 | `bool RemoteSessionWindow::eventFilter(QObject* watched, QEvent* event)` | 实现 event filter 对应的业务或工具逻辑。 |
| [L64](../src/apps/controller/RemoteSessionWindow.Layout.cpp#L64) | `RemoteSessionWindow::resizeEvent` | 定义 | `void RemoteSessionWindow::resizeEvent(QResizeEvent* event)` | 实现 resize event 对应的业务或工具逻辑。 |
| [L70](../src/apps/controller/RemoteSessionWindow.Layout.cpp#L70) | `RemoteSessionWindow::changeEvent` | 定义 | `void RemoteSessionWindow::changeEvent(QEvent* event)` | 实现 change event 对应的业务或工具逻辑。 |
| [L94](../src/apps/controller/RemoteSessionWindow.Layout.cpp#L94) | `RemoteSessionWindow::moveEvent` | 定义 | `void RemoteSessionWindow::moveEvent(QMoveEvent* event)` | 实现 move event 对应的业务或工具逻辑。 |
| [L100](../src/apps/controller/RemoteSessionWindow.Layout.cpp#L100) | `RemoteSessionWindow::LayoutSessionOverlays` | 定义 | `void RemoteSessionWindow::LayoutSessionOverlays()` | 实现 layout session overlays 对应的业务或工具逻辑。 |
| [L244](../src/apps/controller/RemoteSessionWindow.Layout.cpp#L244) | `RemoteSessionWindow::ShowSessionToolbar` | 定义 | `void RemoteSessionWindow::ShowSessionToolbar(bool animated)` | 实现 show session toolbar 对应的业务或工具逻辑。 |
| [L280](../src/apps/controller/RemoteSessionWindow.Layout.cpp#L280) | `RemoteSessionWindow::HideSessionToolbar` | 定义 | `void RemoteSessionWindow::HideSessionToolbar(bool animated)` | 实现 hide session toolbar 对应的业务或工具逻辑。 |
| [L312](../src/apps/controller/RemoteSessionWindow.Layout.cpp#L312) | `RemoteSessionWindow::ScheduleSessionToolbarHide` | 定义 | `void RemoteSessionWindow::ScheduleSessionToolbarHide()` | 执行后台循环或调度 schedule session toolbar hide 相关逻辑。 |

## `src/apps/controller/RemoteSessionWindow.Lifecycle.cpp`

[打开源码](../src/apps/controller/RemoteSessionWindow.Lifecycle.cpp) · **文件作用：** 实现 remote session window lifecycle 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L32](../src/apps/controller/RemoteSessionWindow.Lifecycle.cpp#L32) | `RemoteSessionWindow::RemoteSessionWindow` | 定义 | `RemoteSessionWindow::RemoteSessionWindow(RemoteSessionBinding binding, IRemoteSessionControl* sessionControl, app::ISessionMediaAccess* sessionMedia, QWidget* parent) : FramelessMainWindow(parent), remoteInputDispatch...` | 构造并初始化 RemoteSessionWindow 实例。 |
| [L72](../src/apps/controller/RemoteSessionWindow.Lifecycle.cpp#L72) | `RemoteSessionWindow::~RemoteSessionWindow` | 定义 | `RemoteSessionWindow::~RemoteSessionWindow()` | 停止相关活动并释放 RemoteSessionWindow 实例拥有的资源。 |
| [L91](../src/apps/controller/RemoteSessionWindow.Lifecycle.cpp#L91) | `RemoteSessionWindow::SetDisconnectHandler` | 定义 | `void RemoteSessionWindow::SetDisconnectHandler( std::function<void()> handler)` | 更新或应用 set disconnect handler 相关逻辑。 |
| [L97](../src/apps/controller/RemoteSessionWindow.Lifecycle.cpp#L97) | `RemoteSessionWindow::SetRemotePasteHandler` | 定义 | `void RemoteSessionWindow::SetRemotePasteHandler( std::function<bool(const QStringList& localFiles, bool keyboardPaste)> handler)` | 更新或应用 set remote paste handler 相关逻辑。 |
| [L104](../src/apps/controller/RemoteSessionWindow.Lifecycle.cpp#L104) | `RemoteSessionWindow::SetRemotePasteCancelHandler` | 定义 | `void RemoteSessionWindow::SetRemotePasteCancelHandler( std::function<void()> handler)` | 更新或应用 set remote paste cancel handler 相关逻辑。 |
| [L110](../src/apps/controller/RemoteSessionWindow.Lifecycle.cpp#L110) | `RemoteSessionWindow::SetFileTransferHandlers` | 定义 | `void RemoteSessionWindow::SetFileTransferHandlers( std::function<void()> openHandler, std::function<void()> releaseHostHandler)` | 更新或应用 set file transfer handlers 相关逻辑。 |
| [L119](../src/apps/controller/RemoteSessionWindow.Lifecycle.cpp#L119) | `RemoteSessionWindow::SetDragPointerSampleRate` | 定义 | `void RemoteSessionWindow::SetDragPointerSampleRate( std::uint32_t hertz)` | 更新或应用 set drag pointer sample rate 相关逻辑。 |
| [L136](../src/apps/controller/RemoteSessionWindow.Lifecycle.cpp#L136) | `RemoteSessionWindow::SetRoomOnlineMemberCount` | 定义 | `void RemoteSessionWindow::SetRoomOnlineMemberCount( std::size_t onlineMemberCount)` | 更新或应用 set room online member count 相关逻辑。 |
| [L155](../src/apps/controller/RemoteSessionWindow.Lifecycle.cpp#L155) | `RemoteSessionWindow::BindSessionVideo` | 定义 | `void RemoteSessionWindow::BindSessionVideo( IRemoteSessionControl* sessionControl, app::ISessionMediaAccess* media, RemoteSessionBinding binding)` | 实现 bind session video 对应的业务或工具逻辑。 |
| [L318](../src/apps/controller/RemoteSessionWindow.Lifecycle.cpp#L318) | `RemoteSessionWindow::HandleRemoteCursorMessage` | 定义 | `void RemoteSessionWindow::HandleRemoteCursorMessage( const std::string& pairId, const RemoteCursorEnvelope& envelope)` | 接收并处理 handle remote cursor message 相关逻辑。 |
| [L336](../src/apps/controller/RemoteSessionWindow.Lifecycle.cpp#L336) | `RemoteSessionWindow::BeginScreenStartup` | 定义 | `void RemoteSessionWindow::BeginScreenStartup( std::uint64_t screenShareGeneration)` | 启动 begin screen startup 相关逻辑。 |
| [L371](../src/apps/controller/RemoteSessionWindow.Lifecycle.cpp#L371) | `RemoteSessionWindow::RequestScreenStartupRefresh` | 定义 | `void RemoteSessionWindow::RequestScreenStartupRefresh( std::uint64_t screenShareGeneration, std::uint32_t retryStage)` | 发起请求或查询 request screen startup refresh 相关逻辑。 |
| [L403](../src/apps/controller/RemoteSessionWindow.Lifecycle.cpp#L403) | `RemoteSessionWindow::HandleFirstScreenPresentation` | 定义 | `void RemoteSessionWindow::HandleFirstScreenPresentation( std::uint64_t screenShareGeneration)` | 接收并处理 handle first screen presentation 相关逻辑。 |

## `src/apps/controller/RemoteSessionWindow.MediaMenu.cpp`

[打开源码](../src/apps/controller/RemoteSessionWindow.MediaMenu.cpp) · **文件作用：** 实现 remote session window media menu 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L31](../src/apps/controller/RemoteSessionWindow.MediaMenu.cpp#L31) | `RemoteSessionWindow::ShowMediaDeviceMenu` | 定义 | `void RemoteSessionWindow::ShowMediaDeviceMenu()` | 实现 show media device menu 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteSessionWindow.MediaSelection.cpp`

[打开源码](../src/apps/controller/RemoteSessionWindow.MediaSelection.cpp) · **文件作用：** 实现 remote session window media selection 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L13](../src/apps/controller/RemoteSessionWindow.MediaSelection.cpp#L13) | `RemoteSessionWindow::BeginMediaDeviceSelection` | 定义 | `void RemoteSessionWindow::BeginMediaDeviceSelection( MediaDeviceKind kind, const QString& deviceId)` | 启动 begin media device selection 相关逻辑。 |
| [L87](../src/apps/controller/RemoteSessionWindow.MediaSelection.cpp#L87) | `RemoteSessionWindow::CompleteMediaDeviceSelections` | 定义 | `void RemoteSessionWindow::CompleteMediaDeviceSelections( const MediaDeviceSnapshot& media)` | 实现 complete media device selections 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteSessionWindow.SessionActions.cpp`

[打开源码](../src/apps/controller/RemoteSessionWindow.SessionActions.cpp) · **文件作用：** 实现 remote session window session actions 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L29](../src/apps/controller/RemoteSessionWindow.SessionActions.cpp#L29) | `RemoteSessionWindow::HandleDisconnectAction` | 定义 | `void RemoteSessionWindow::HandleDisconnectAction()` | 接收并处理 handle disconnect action 相关逻辑。 |
| [L39](../src/apps/controller/RemoteSessionWindow.SessionActions.cpp#L39) | `RemoteSessionWindow::closeEvent` | 定义 | `void RemoteSessionWindow::closeEvent(QCloseEvent* event)` | 关闭并清理 close event 相关逻辑。 |
| [L92](../src/apps/controller/RemoteSessionWindow.SessionActions.cpp#L92) | `RemoteSessionWindow::UpdateSessionDuration` | 定义 | `void RemoteSessionWindow::UpdateSessionDuration()` | 更新或应用 update session duration 相关逻辑。 |
| [L106](../src/apps/controller/RemoteSessionWindow.SessionActions.cpp#L106) | `RemoteSessionWindow::UpdateNetworkRecoveryPrompt` | 定义 | `void RemoteSessionWindow::UpdateNetworkRecoveryPrompt( bool recovering, std::uint32_t attempt, bool waitingForSignaling, bool failed)` | 更新或应用 update network recovery prompt 相关逻辑。 |
| [L217](../src/apps/controller/RemoteSessionWindow.SessionActions.cpp#L217) | `RemoteSessionWindow::HandleControlAction` | 定义 | `void RemoteSessionWindow::HandleControlAction()` | 接收并处理 handle control action 相关逻辑。 |
| [L246](../src/apps/controller/RemoteSessionWindow.SessionActions.cpp#L246) | `RemoteSessionWindow::RebuildFrameRateMenu` | 定义 | `void RemoteSessionWindow::RebuildFrameRateMenu()` | 更新或应用 rebuild frame rate menu 相关逻辑。 |
| [L288](../src/apps/controller/RemoteSessionWindow.SessionActions.cpp#L288) | `RemoteSessionWindow::RebuildQualityMenu` | 定义 | `void RemoteSessionWindow::RebuildQualityMenu()` | 更新或应用 rebuild quality menu 相关逻辑。 |
| [L333](../src/apps/controller/RemoteSessionWindow.SessionActions.cpp#L333) | `RemoteSessionWindow::HandleFrameRateSelection` | 定义 | `void RemoteSessionWindow::HandleFrameRateSelection( std::uint32_t framesPerSecond)` | 接收并处理 handle frame rate selection 相关逻辑。 |
| [L376](../src/apps/controller/RemoteSessionWindow.SessionActions.cpp#L376) | `RemoteSessionWindow::HandleQualitySelection` | 定义 | `void RemoteSessionWindow::HandleQualitySelection(ScreenQualityTier quality)` | 接收并处理 handle quality selection 相关逻辑。 |
| [L408](../src/apps/controller/RemoteSessionWindow.SessionActions.cpp#L408) | `RemoteSessionWindow::RequestStreamPreference` | 定义 | `bool RemoteSessionWindow::RequestStreamPreference(bool showError)` | 发起请求或查询 request stream preference 相关逻辑。 |
| [L437](../src/apps/controller/RemoteSessionWindow.SessionActions.cpp#L437) | `RemoteSessionWindow::ToggleRemoteSound` | 定义 | `void RemoteSessionWindow::ToggleRemoteSound()` | 实现 toggle remote sound 对应的业务或工具逻辑。 |
| [L460](../src/apps/controller/RemoteSessionWindow.SessionActions.cpp#L460) | `RemoteSessionWindow::ToggleLocalMicrophone` | 定义 | `void RemoteSessionWindow::ToggleLocalMicrophone()` | 实现 toggle local microphone 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteSessionWindow.SessionState.cpp`

[打开源码](../src/apps/controller/RemoteSessionWindow.SessionState.cpp) · **文件作用：** 实现 remote session window session state 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L28](../src/apps/controller/RemoteSessionWindow.SessionState.cpp#L28) | `IsDirectRecoveryFailureCode` | 定义 | `bool IsDirectRecoveryFailureCode(const std::string& errorCode)` | 判断 is direct recovery failure code 相关逻辑。 |
| [L36](../src/apps/controller/RemoteSessionWindow.SessionState.cpp#L36) | `RemoteSessionWindow::RefreshControlState` | 定义 | `void RemoteSessionWindow::RefreshControlState()` | 刷新 refresh control state 相关逻辑。 |

## `src/apps/controller/RemoteSessionWindow.TransferProgress.cpp`

[打开源码](../src/apps/controller/RemoteSessionWindow.TransferProgress.cpp) · **文件作用：** 实现 remote session window transfer progress 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L30](../src/apps/controller/RemoteSessionWindow.TransferProgress.cpp#L30) | `RemoteSessionWindow::ShowRemotePasteProgress` | 定义 | `void RemoteSessionWindow::ShowRemotePasteProgress( const QString& transferId, const QString& title, const QString& message, double progress, std::uintptr_t localTargetWindow)` | 实现 show remote paste progress 对应的业务或工具逻辑。 |
| [L143](../src/apps/controller/RemoteSessionWindow.TransferProgress.cpp#L143) | `RemoteSessionWindow::CompleteRemotePasteProgress` | 定义 | `void RemoteSessionWindow::CompleteRemotePasteProgress( const QString& transferId)` | 实现 complete remote paste progress 对应的业务或工具逻辑。 |
| [L170](../src/apps/controller/RemoteSessionWindow.TransferProgress.cpp#L170) | `RemoteSessionWindow::CloseRemotePasteProgress` | 定义 | `void RemoteSessionWindow::CloseRemotePasteProgress( const QString& transferId)` | 关闭并清理 close remote paste progress 相关逻辑。 |
| [L199](../src/apps/controller/RemoteSessionWindow.TransferProgress.cpp#L199) | `RemoteSessionWindow::ShowRemotePasteFailure` | 定义 | `void RemoteSessionWindow::ShowRemotePasteFailure( const QString& transferId, const QString& message)` | 实现 show remote paste failure 对应的业务或工具逻辑。 |
| [L251](../src/apps/controller/RemoteSessionWindow.TransferProgress.cpp#L251) | `RemoteSessionWindow::SetRemotePasteStatusButtonVisible` | 定义 | `void RemoteSessionWindow::SetRemotePasteStatusButtonVisible(bool visible)` | 更新或应用 set remote paste status button visible 相关逻辑。 |
| [L266](../src/apps/controller/RemoteSessionWindow.TransferProgress.cpp#L266) | `RemoteSessionWindow::RemotePasteDialogRestoreGeometry` | 定义 | `QRect RemoteSessionWindow::RemotePasteDialogRestoreGeometry() const` | 实现 remote paste dialog restore geometry 对应的业务或工具逻辑。 |
| [L280](../src/apps/controller/RemoteSessionWindow.TransferProgress.cpp#L280) | `RemoteSessionWindow::CreateRemotePasteAnimationOverlay` | 定义 | `QLabel* RemoteSessionWindow::CreateRemotePasteAnimationOverlay( const QPixmap& snapshot, const QRect& geometry, qreal opacity)` | 创建或初始化 create remote paste animation overlay 相关逻辑。 |
| [L333](../src/apps/controller/RemoteSessionWindow.TransferProgress.cpp#L333) | `RemoteSessionWindow::MinimizeRemotePasteProgress` | 定义 | `void RemoteSessionWindow::MinimizeRemotePasteProgress()` | 实现 minimize remote paste progress 对应的业务或工具逻辑。 |
| [L445](../src/apps/controller/RemoteSessionWindow.TransferProgress.cpp#L445) | `RemoteSessionWindow::RestoreRemotePasteProgress` | 定义 | `void RemoteSessionWindow::RestoreRemotePasteProgress()` | 读取或恢复 restore remote paste progress 相关逻辑。 |

## `src/apps/controller/RemoteSessionWindow.Ui.cpp`

[打开源码](../src/apps/controller/RemoteSessionWindow.Ui.cpp) · **文件作用：** 实现 remote session window ui 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L344](../src/apps/controller/RemoteSessionWindow.Ui.cpp#L344) | `RemoteSessionWindow::BuildUi` | 定义 | `void RemoteSessionWindow::BuildUi()` | 创建或初始化 build ui 相关逻辑。 |

## `src/apps/controller/RemoteSessionWindow.WindowActions.cpp`

[打开源码](../src/apps/controller/RemoteSessionWindow.WindowActions.cpp) · **文件作用：** 实现 remote session window window actions 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L22](../src/apps/controller/RemoteSessionWindow.WindowActions.cpp#L22) | `RemoteSessionWindow::ToggleToolbarLock` | 定义 | `void RemoteSessionWindow::ToggleToolbarLock()` | 实现 toggle toolbar lock 对应的业务或工具逻辑。 |
| [L87](../src/apps/controller/RemoteSessionWindow.WindowActions.cpp#L87) | `RemoteSessionWindow::ToggleFullScreenMode` | 定义 | `void RemoteSessionWindow::ToggleFullScreenMode()` | 实现 toggle full screen mode 对应的业务或工具逻辑。 |
| [L124](../src/apps/controller/RemoteSessionWindow.WindowActions.cpp#L124) | `RemoteSessionWindow::ReleaseRemoteInputs` | 定义 | `void RemoteSessionWindow::ReleaseRemoteInputs()` | 释放或取消 release remote inputs 相关逻辑。 |

## `src/apps/controller/RemoteSessionWindowConstants.h`

[打开源码](../src/apps/controller/RemoteSessionWindowConstants.h) · **文件作用：** 声明 remote session window constants 相关类型、接口、配置和成员状态。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L16](../src/apps/controller/RemoteSessionWindowConstants.h#L16) | `kSupportedDragPointerSampleRates` | `kSupportedDragPointerSampleRates = {60u, 80u, 120u, 170u, 240u};` | 定义 supported drag pointer sample rates 的编译期常量或产品边界。 |

## `src/apps/controller/RemoteSessionWindowHelpers.cpp`

[打开源码](../src/apps/controller/RemoteSessionWindowHelpers.cpp) · **文件作用：** 实现 remote session window helpers 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L14](../src/apps/controller/RemoteSessionWindowHelpers.cpp#L14) | `MakeToolButton` | 定义 | `QToolButton* MakeToolButton(const QString& text, const QString& tooltip, QWidget* parent)` | 创建或初始化 make tool button 相关逻辑。 |
| [L62](../src/apps/controller/RemoteSessionWindowHelpers.cpp#L62) | `ScreenQualityText` | 定义 | `QString ScreenQualityText(ScreenQualityTier quality)` | 实现 screen quality text 对应的业务或工具逻辑。 |
| [L79](../src/apps/controller/RemoteSessionWindowHelpers.cpp#L79) | `CaptureBackendText` | 定义 | `QString CaptureBackendText(const std::string& backend)` | 采集 capture backend text 相关逻辑。 |
| [L102](../src/apps/controller/RemoteSessionWindowHelpers.cpp#L102) | `ActiveMediaDeviceText` | 定义 | `QString ActiveMediaDeviceText( const MediaDeviceCategorySnapshot& category)` | 实现 active media device text 对应的业务或工具逻辑。 |
| [L127](../src/apps/controller/RemoteSessionWindowHelpers.cpp#L127) | `ScreenQualityBounds` | 定义 | `std::pair<std::uint32_t, std::uint32_t> ScreenQualityBounds( ScreenQualityTier quality)` | 实现 screen quality bounds 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteSessionWindowHelpers.h`

[打开源码](../src/apps/controller/RemoteSessionWindowHelpers.h) · **文件作用：** 声明 remote session window helpers 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L15](../src/apps/controller/RemoteSessionWindowHelpers.h#L15) | `QToolButton` | class | 定义 QToolButton 的 class 类型和相关状态。 |
| [L16](../src/apps/controller/RemoteSessionWindowHelpers.h#L16) | `QWidget` | class | 定义 QWidget 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L15](../src/apps/controller/RemoteSessionWindowHelpers.h#L15) | `QToolButton` | `class QToolButton;` | 保存 q tool button 相关配置或运行状态。 |
| [L16](../src/apps/controller/RemoteSessionWindowHelpers.h#L16) | `QWidget` | `class QWidget;` | 保存 q widget 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L20](../src/apps/controller/RemoteSessionWindowHelpers.h#L20) | `MakeToolButton` | 声明 | `QToolButton* MakeToolButton( const QString& text, const QString& tooltip, QWidget* parent)` | 创建或初始化 make tool button 相关逻辑。 |
| [L24](../src/apps/controller/RemoteSessionWindowHelpers.h#L24) | `ScreenQualityText` | 声明 | `QString ScreenQualityText(ScreenQualityTier quality)` | 实现 screen quality text 对应的业务或工具逻辑。 |
| [L25](../src/apps/controller/RemoteSessionWindowHelpers.h#L25) | `CaptureBackendText` | 声明 | `QString CaptureBackendText(const std::string& backend)` | 采集 capture backend text 相关逻辑。 |
| [L26](../src/apps/controller/RemoteSessionWindowHelpers.h#L26) | `ActiveMediaDeviceText` | 声明 | `QString ActiveMediaDeviceText( const MediaDeviceCategorySnapshot& category)` | 实现 active media device text 对应的业务或工具逻辑。 |
| [L28](../src/apps/controller/RemoteSessionWindowHelpers.h#L28) | `ScreenQualityBounds` | 声明 | `std::pair<std::uint32_t, std::uint32_t> ScreenQualityBounds( ScreenQualityTier quality)` | 实现 screen quality bounds 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteTransferStatusButton.cpp`

[打开源码](../src/apps/controller/RemoteTransferStatusButton.cpp) · **文件作用：** 实现 remote transfer status button 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L20](../src/apps/controller/RemoteTransferStatusButton.cpp#L20) | `RemoteTransferStatusButton::RemoteTransferStatusButton` | 定义 | `RemoteTransferStatusButton::RemoteTransferStatusButton(QWidget* parent) : QToolButton(parent)` | 构造并初始化 RemoteTransferStatusButton 实例。 |
| [L37](../src/apps/controller/RemoteTransferStatusButton.cpp#L37) | `RemoteTransferStatusButton::SetProgress` | 定义 | `void RemoteTransferStatusButton::SetProgress(double progress)` | 更新或应用 set progress 相关逻辑。 |
| [L68](../src/apps/controller/RemoteTransferStatusButton.cpp#L68) | `RemoteTransferStatusButton::paintEvent` | 定义 | `void RemoteTransferStatusButton::paintEvent(QPaintEvent*)` | 准备或呈现 paint event 相关逻辑。 |

## `src/apps/controller/RemoteTransferStatusButton.h`

[打开源码](../src/apps/controller/RemoteTransferStatusButton.h) · **文件作用：** 声明 remote transfer status button 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L9](../src/apps/controller/RemoteTransferStatusButton.h#L9) | `QPaintEvent` | class | 定义 QPaintEvent 的 class 类型和相关状态。 |
| [L10](../src/apps/controller/RemoteTransferStatusButton.h#L10) | `QVariantAnimation` | class | 定义 QVariantAnimation 的 class 类型和相关状态。 |
| [L11](../src/apps/controller/RemoteTransferStatusButton.h#L11) | `QWidget` | class | 定义 QWidget 的 class 类型和相关状态。 |
| [L15](../src/apps/controller/RemoteTransferStatusButton.h#L15) | `RemoteTransferStatusButton` | class | 定义 RemoteTransferStatusButton 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L9](../src/apps/controller/RemoteTransferStatusButton.h#L9) | `QPaintEvent` | `class QPaintEvent;` | 保存 q paint event 相关配置或运行状态。 |
| [L10](../src/apps/controller/RemoteTransferStatusButton.h#L10) | `QVariantAnimation` | `class QVariantAnimation;` | 保存 q variant animation 相关配置或运行状态。 |
| [L11](../src/apps/controller/RemoteTransferStatusButton.h#L11) | `QWidget` | `class QWidget;` | 保存 q widget 相关配置或运行状态。 |
| [L25](../src/apps/controller/RemoteTransferStatusButton.h#L25) | `waveTimer_` | `QTimer waveTimer_{this};` | 保存定时、截止或超时状态：wave timer。 |
| [L26](../src/apps/controller/RemoteTransferStatusButton.h#L26) | `progressAnimation_` | `QVariantAnimation* progressAnimation_ = nullptr;` | 保存 progress animation 相关配置或运行状态。 |
| [L27](../src/apps/controller/RemoteTransferStatusButton.h#L27) | `progress_` | `double progress_ = 0.0;` | 保存 progress 相关配置或运行状态。 |
| [L28](../src/apps/controller/RemoteTransferStatusButton.h#L28) | `targetProgress_` | `double targetProgress_ = 0.0;` | 保存 target progress 相关配置或运行状态。 |
| [L29](../src/apps/controller/RemoteTransferStatusButton.h#L29) | `wavePhase_` | `double wavePhase_ = 0.0;` | 保存状态机当前状态：wave phase。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L17](../src/apps/controller/RemoteTransferStatusButton.h#L17) | `RemoteTransferStatusButton` | 声明 | `explicit RemoteTransferStatusButton(QWidget* parent = nullptr)` | 实现 remote transfer status button 对应的业务或工具逻辑。 |
| [L19](../src/apps/controller/RemoteTransferStatusButton.h#L19) | `SetProgress` | 声明 | `void SetProgress(double progress)` | 更新或应用 set progress 相关逻辑。 |
| [L22](../src/apps/controller/RemoteTransferStatusButton.h#L22) | `paintEvent` | 声明 | `void paintEvent(QPaintEvent* event) override` | 准备或呈现 paint event 相关逻辑。 |

## `src/apps/controller/RoomCameraWindow.cpp`

[打开源码](../src/apps/controller/RoomCameraWindow.cpp) · **文件作用：** 实现 room camera window 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L144](../src/apps/controller/RoomCameraWindow.cpp#L144) | `RoomCameraTile` | class | 定义 RoomCameraTile 的 class 类型和相关状态。 |
| [L302](../src/apps/controller/RoomCameraWindow.cpp#L302) | `CameraAspectRatioHost` | class | 定义 CameraAspectRatioHost 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L107](../src/apps/controller/RoomCameraWindow.cpp#L107) | `MemberName` | 定义 | `QString MemberName(const RoomMemberSnapshot& member)` | 实现 member name 对应的业务或工具逻辑。 |
| [L114](../src/apps/controller/RoomCameraWindow.cpp#L114) | `ClearLayout` | 定义 | `void ClearLayout(QLayout* layout)` | 重置或移除 clear layout 相关逻辑。 |
| [L121](../src/apps/controller/RoomCameraWindow.cpp#L121) | `AnimateCameraTileEntrance` | 定义 | `void AnimateCameraTileEntrance(QWidget* widget)` | 实现 animate camera tile entrance 对应的业务或工具逻辑。 |
| [L148](../src/apps/controller/RoomCameraWindow.cpp#L148) | `RoomCameraTile` | 定义 | `RoomCameraTile(QString name, bool local, std::function<void(bool)> activate, QWidget* parent = nullptr) : QFrame(parent), name_(std::move(name)), local_(local), activate_(std::move(activate))` | 实现 room camera tile 对应的业务或工具逻辑。 |
| [L173](../src/apps/controller/RoomCameraWindow.cpp#L173) | `SetName` | 定义 | `void SetName(const QString& name)` | 更新或应用 set name 相关逻辑。 |
| [L181](../src/apps/controller/RoomCameraWindow.cpp#L181) | `IsLocal` | 定义 | `bool IsLocal() const { return local_; }` | 判断 is local 相关逻辑。 |
| [L183](../src/apps/controller/RoomCameraWindow.cpp#L183) | `SetPresentation` | 定义 | `void SetPresentation(bool thumbnail, bool overview)` | 更新或应用 set presentation 相关逻辑。 |
| [L205](../src/apps/controller/RoomCameraWindow.cpp#L205) | `OnFrame` | 定义 | `void OnFrame(const webrtc::VideoFrame& frame) override` | 接收并处理 on frame 相关逻辑。 |
| [L232](../src/apps/controller/RoomCameraWindow.cpp#L232) | `mousePressEvent` | 定义 | `void mousePressEvent(QMouseEvent* event) override` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L240](../src/apps/controller/RoomCameraWindow.cpp#L240) | `mouseDoubleClickEvent` | 定义 | `void mouseDoubleClickEvent(QMouseEvent* event) override` | 实现 mouse double click event 对应的业务或工具逻辑。 |
| [L250](../src/apps/controller/RoomCameraWindow.cpp#L250) | `resizeEvent` | 定义 | `void resizeEvent(QResizeEvent* event) override` | 实现 resize event 对应的业务或工具逻辑。 |
| [L261](../src/apps/controller/RoomCameraWindow.cpp#L261) | `paintEvent` | 定义 | `void paintEvent(QPaintEvent* event) override` | 准备或呈现 paint event 相关逻辑。 |
| [L304](../src/apps/controller/RoomCameraWindow.cpp#L304) | `CameraAspectRatioHost` | 定义 | `explicit CameraAspectRatioHost(RoomCameraTile* tile, QWidget* parent = nullptr) : QWidget(parent), tile_(tile)` | 实现 camera aspect ratio host 对应的业务或工具逻辑。 |
| [L312](../src/apps/controller/RoomCameraWindow.cpp#L312) | `SetPresentation` | 定义 | `void SetPresentation(bool thumbnail, bool overview)` | 更新或应用 set presentation 相关逻辑。 |
| [L327](../src/apps/controller/RoomCameraWindow.cpp#L327) | `resizeEvent` | 定义 | `void resizeEvent(QResizeEvent* event) override` | 实现 resize event 对应的业务或工具逻辑。 |
| [L334](../src/apps/controller/RoomCameraWindow.cpp#L334) | `UpdateTileGeometry` | 定义 | `void UpdateTileGeometry()` | 更新或应用 update tile geometry 相关逻辑。 |
| [L354](../src/apps/controller/RoomCameraWindow.cpp#L354) | `RoomCameraWindow::RoomCameraWindow` | 定义 | `RoomCameraWindow::RoomCameraWindow(app::ISessionMediaAccess* media, QWidget* parent) : FramelessMainWindow(parent), media_(media)` | 构造并初始化 RoomCameraWindow 实例。 |
| [L435](../src/apps/controller/RoomCameraWindow.cpp#L435) | `RoomCameraWindow::~RoomCameraWindow` | 定义 | `RoomCameraWindow::~RoomCameraWindow()` | 停止相关活动并释放 RoomCameraWindow 实例拥有的资源。 |
| [L440](../src/apps/controller/RoomCameraWindow.cpp#L440) | `RoomCameraWindow::SyncSnapshot` | 定义 | `void RoomCameraWindow::SyncSnapshot(const SessionEngineSnapshot& snapshot)` | 实现 sync snapshot 对应的业务或工具逻辑。 |
| [L545](../src/apps/controller/RoomCameraWindow.cpp#L545) | `RoomCameraWindow::OpenBesideMainWindow` | 定义 | `void RoomCameraWindow::OpenBesideMainWindow( const QRect& mainWindowGeometry)` | 启动 open beside main window 相关逻辑。 |
| [L565](../src/apps/controller/RoomCameraWindow.cpp#L565) | `RoomCameraWindow::SetHiddenByUserCallback` | 定义 | `void RoomCameraWindow::SetHiddenByUserCallback( std::function<void()> callback)` | 更新或应用 set hidden by user callback 相关逻辑。 |
| [L571](../src/apps/controller/RoomCameraWindow.cpp#L571) | `RoomCameraWindow::closeEvent` | 定义 | `void RoomCameraWindow::closeEvent(QCloseEvent* event)` | 关闭并清理 close event 相关逻辑。 |
| [L580](../src/apps/controller/RoomCameraWindow.cpp#L580) | `RoomCameraWindow::ConstrainResizeGeometry` | 定义 | `QRect RoomCameraWindow::ConstrainResizeGeometry( const QRect& proposedGeometry, Qt::Edges resizeEdges, qreal devicePixelRatio) const` | 实现 constrain resize geometry 对应的业务或工具逻辑。 |
| [L620](../src/apps/controller/RoomCameraWindow.cpp#L620) | `RoomCameraWindow::RebuildLayout` | 定义 | `void RoomCameraWindow::RebuildLayout()` | 更新或应用 rebuild layout 相关逻辑。 |
| [L635](../src/apps/controller/RoomCameraWindow.cpp#L635) | `RoomCameraWindow::RebuildFocusLayout` | 定义 | `void RoomCameraWindow::RebuildFocusLayout()` | 更新或应用 rebuild focus layout 相关逻辑。 |
| [L691](../src/apps/controller/RoomCameraWindow.cpp#L691) | `RoomCameraWindow::RebuildOverviewLayout` | 定义 | `void RoomCameraWindow::RebuildOverviewLayout()` | 更新或应用 rebuild overview layout 相关逻辑。 |
| [L742](../src/apps/controller/RoomCameraWindow.cpp#L742) | `RoomCameraWindow::SetFocusedDevice` | 定义 | `void RoomCameraWindow::SetFocusedDevice(const QString& deviceId, bool maximize)` | 更新或应用 set focused device 相关逻辑。 |
| [L757](../src/apps/controller/RoomCameraWindow.cpp#L757) | `RoomCameraWindow::SetOverviewMode` | 定义 | `void RoomCameraWindow::SetOverviewMode(bool overview)` | 更新或应用 set overview mode 相关逻辑。 |
| [L763](../src/apps/controller/RoomCameraWindow.cpp#L763) | `RoomCameraWindow::UpdateSingleParticipantMode` | 定义 | `void RoomCameraWindow::UpdateSingleParticipantMode(int participantCount)` | 更新或应用 update single participant mode 相关逻辑。 |
| [L791](../src/apps/controller/RoomCameraWindow.cpp#L791) | `RoomCameraWindow::SingleWindowHeightForWidth` | 定义 | `int RoomCameraWindow::SingleWindowHeightForWidth(int windowWidth) const` | 实现 single window height for width 对应的业务或工具逻辑。 |
| [L805](../src/apps/controller/RoomCameraWindow.cpp#L805) | `RoomCameraWindow::SingleWindowWidthForHeight` | 定义 | `int RoomCameraWindow::SingleWindowWidthForHeight(int windowHeight) const` | 实现 single window width for height 对应的业务或工具逻辑。 |
| [L819](../src/apps/controller/RoomCameraWindow.cpp#L819) | `RoomCameraWindow::PlaceBesideMainWindow` | 定义 | `void RoomCameraWindow::PlaceBesideMainWindow( const QRect& mainWindowGeometry)` | 实现 place beside main window 对应的业务或工具逻辑。 |
| [L851](../src/apps/controller/RoomCameraWindow.cpp#L851) | `RoomCameraWindow::RemoveTile` | 定义 | `void RoomCameraWindow::RemoveTile(const QString& deviceId)` | 重置或移除 remove tile 相关逻辑。 |
| [L873](../src/apps/controller/RoomCameraWindow.cpp#L873) | `RoomCameraWindow::DetachAllSinks` | 定义 | `void RoomCameraWindow::DetachAllSinks()` | 实现 detach all sinks 对应的业务或工具逻辑。 |

## `src/apps/controller/RoomCameraWindow.h`

[打开源码](../src/apps/controller/RoomCameraWindow.h) · **文件作用：** 声明 room camera window 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L16](../src/apps/controller/RoomCameraWindow.h#L16) | `QCloseEvent` | class | 定义 QCloseEvent 的 class 类型和相关状态。 |
| [L17](../src/apps/controller/RoomCameraWindow.h#L17) | `QGridLayout` | class | 定义 QGridLayout 的 class 类型和相关状态。 |
| [L18](../src/apps/controller/RoomCameraWindow.h#L18) | `QHBoxLayout` | class | 定义 QHBoxLayout 的 class 类型和相关状态。 |
| [L19](../src/apps/controller/RoomCameraWindow.h#L19) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L20](../src/apps/controller/RoomCameraWindow.h#L20) | `QPushButton` | class | 定义 QPushButton 的 class 类型和相关状态。 |
| [L21](../src/apps/controller/RoomCameraWindow.h#L21) | `QScrollArea` | class | 定义 QScrollArea 的 class 类型和相关状态。 |
| [L22](../src/apps/controller/RoomCameraWindow.h#L22) | `QStackedWidget` | class | 定义 QStackedWidget 的 class 类型和相关状态。 |
| [L23](../src/apps/controller/RoomCameraWindow.h#L23) | `QVBoxLayout` | class | 定义 QVBoxLayout 的 class 类型和相关状态。 |
| [L26](../src/apps/controller/RoomCameraWindow.h#L26) | `ISessionMediaAccess` | class | 定义 ISessionMediaAccess 的 class 类型和相关状态。 |
| [L31](../src/apps/controller/RoomCameraWindow.h#L31) | `RoomCameraTile` | class | 定义 RoomCameraTile 的 class 类型和相关状态。 |
| [L33](../src/apps/controller/RoomCameraWindow.h#L33) | `RoomCameraWindow` | class | 定义 RoomCameraWindow 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L16](../src/apps/controller/RoomCameraWindow.h#L16) | `QCloseEvent` | `class QCloseEvent;` | 保存 q close event 相关配置或运行状态。 |
| [L17](../src/apps/controller/RoomCameraWindow.h#L17) | `QGridLayout` | `class QGridLayout;` | 保存 q grid layout 相关配置或运行状态。 |
| [L18](../src/apps/controller/RoomCameraWindow.h#L18) | `QHBoxLayout` | `class QHBoxLayout;` | 保存 qh box layout 相关配置或运行状态。 |
| [L19](../src/apps/controller/RoomCameraWindow.h#L19) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L20](../src/apps/controller/RoomCameraWindow.h#L20) | `QPushButton` | `class QPushButton;` | 保存 q push button 相关配置或运行状态。 |
| [L21](../src/apps/controller/RoomCameraWindow.h#L21) | `QScrollArea` | `class QScrollArea;` | 保存 q scroll area 相关配置或运行状态。 |
| [L22](../src/apps/controller/RoomCameraWindow.h#L22) | `QStackedWidget` | `class QStackedWidget;` | 保存 q stacked widget 相关配置或运行状态。 |
| [L23](../src/apps/controller/RoomCameraWindow.h#L23) | `QVBoxLayout` | `class QVBoxLayout;` | 保存 qv box layout 相关配置或运行状态。 |
| [L26](../src/apps/controller/RoomCameraWindow.h#L26) | `ISessionMediaAccess` | `class ISessionMediaAccess;` | 保存 i session media access 相关配置或运行状态。 |
| [L31](../src/apps/controller/RoomCameraWindow.h#L31) | `RoomCameraTile` | `class RoomCameraTile;` | 保存 room camera tile 相关配置或运行状态。 |
| [L62](../src/apps/controller/RoomCameraWindow.h#L62) | `media_` | `app::ISessionMediaAccess* media_ = nullptr;` | 保存 media 相关配置或运行状态。 |
| [L63](../src/apps/controller/RoomCameraWindow.h#L63) | `viewStack_` | `QStackedWidget* viewStack_ = nullptr;` | 保存 view stack 相关配置或运行状态。 |
| [L64](../src/apps/controller/RoomCameraWindow.h#L64) | `galleryHeader_` | `QWidget* galleryHeader_ = nullptr;` | 保存 gallery header 相关配置或运行状态。 |
| [L65](../src/apps/controller/RoomCameraWindow.h#L65) | `focusPage_` | `QWidget* focusPage_ = nullptr;` | 保存 focus page 相关配置或运行状态。 |
| [L66](../src/apps/controller/RoomCameraWindow.h#L66) | `focusLayout_` | `QVBoxLayout* focusLayout_ = nullptr;` | 保存 focus layout 相关配置或运行状态。 |
| [L67](../src/apps/controller/RoomCameraWindow.h#L67) | `thumbnailHost_` | `QWidget* thumbnailHost_ = nullptr;` | 保存 thumbnail host 相关配置或运行状态。 |
| [L68](../src/apps/controller/RoomCameraWindow.h#L68) | `thumbnailLayout_` | `QHBoxLayout* thumbnailLayout_ = nullptr;` | 保存 thumbnail layout 相关配置或运行状态。 |
| [L69](../src/apps/controller/RoomCameraWindow.h#L69) | `thumbnailScroll_` | `QScrollArea* thumbnailScroll_ = nullptr;` | 保存 thumbnail scroll 相关配置或运行状态。 |
| [L70](../src/apps/controller/RoomCameraWindow.h#L70) | `overviewPage_` | `QWidget* overviewPage_ = nullptr;` | 保存 overview page 相关配置或运行状态。 |
| [L71](../src/apps/controller/RoomCameraWindow.h#L71) | `overviewGrid_` | `QGridLayout* overviewGrid_ = nullptr;` | 保存身份或作用域标识：overview grid。 |
| [L72](../src/apps/controller/RoomCameraWindow.h#L72) | `countLabel_` | `QLabel* countLabel_ = nullptr;` | 保存路径、地址或显示名称：count label。 |
| [L73](../src/apps/controller/RoomCameraWindow.h#L73) | `emptyLabel_` | `QLabel* emptyLabel_ = nullptr;` | 保存路径、地址或显示名称：empty label。 |
| [L74](../src/apps/controller/RoomCameraWindow.h#L74) | `viewModeButton_` | `QPushButton* viewModeButton_ = nullptr;` | 保存 view mode button 相关配置或运行状态。 |
| [L75](../src/apps/controller/RoomCameraWindow.h#L75) | `tiles_` | `QHash<QString, RoomCameraTile*> tiles_;` | 保存 tiles 相关配置或运行状态。 |
| [L76](../src/apps/controller/RoomCameraWindow.h#L76) | `tileHosts_` | `QHash<QString, QWidget*> tileHosts_;` | 保存 tile hosts 相关配置或运行状态。 |
| [L77](../src/apps/controller/RoomCameraWindow.h#L77) | `pairBindings_` | `QHash<QString, QString> pairBindings_;` | 保存 pair bindings 相关配置或运行状态。 |
| [L78](../src/apps/controller/RoomCameraWindow.h#L78) | `orderedDeviceIds_` | `QStringList orderedDeviceIds_;` | 保存 ordered device ids 相关配置或运行状态。 |
| [L79](../src/apps/controller/RoomCameraWindow.h#L79) | `focusedDeviceId_` | `QString focusedDeviceId_;` | 保存身份或作用域标识：focused device id。 |
| [L80](../src/apps/controller/RoomCameraWindow.h#L80) | `overviewMode_` | `bool overviewMode_ = false;` | 保存 overview mode 相关配置或运行状态。 |
| [L81](../src/apps/controller/RoomCameraWindow.h#L81) | `singleParticipantMode_` | `bool singleParticipantMode_ = false;` | 保存 single participant mode 相关配置或运行状态。 |
| [L82](../src/apps/controller/RoomCameraWindow.h#L82) | `initialPlacementDone_` | `bool initialPlacementDone_ = false;` | 保存 initial placement done 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L35](../src/apps/controller/RoomCameraWindow.h#L35) | `RoomCameraWindow` | 声明 | `explicit RoomCameraWindow(app::ISessionMediaAccess* media, QWidget* parent = nullptr)` | 实现 room camera window 对应的业务或工具逻辑。 |
| [L37](../src/apps/controller/RoomCameraWindow.h#L37) | `~RoomCameraWindow` | 声明 | `~RoomCameraWindow() override` | 停止相关活动并释放 RoomCameraWindow 实例拥有的资源。 |
| [L39](../src/apps/controller/RoomCameraWindow.h#L39) | `SyncSnapshot` | 声明 | `void SyncSnapshot(const SessionEngineSnapshot& snapshot)` | 实现 sync snapshot 对应的业务或工具逻辑。 |
| [L40](../src/apps/controller/RoomCameraWindow.h#L40) | `OpenBesideMainWindow` | 声明 | `void OpenBesideMainWindow(const QRect& mainWindowGeometry)` | 启动 open beside main window 相关逻辑。 |
| [L41](../src/apps/controller/RoomCameraWindow.h#L41) | `SetHiddenByUserCallback` | 声明 | `void SetHiddenByUserCallback(std::function<void()> callback)` | 更新或应用 set hidden by user callback 相关逻辑。 |
| [L44](../src/apps/controller/RoomCameraWindow.h#L44) | `closeEvent` | 声明 | `void closeEvent(QCloseEvent* event) override` | 关闭并清理 close event 相关逻辑。 |
| [L45](../src/apps/controller/RoomCameraWindow.h#L45) | `ConstrainResizeGeometry` | 声明 | `QRect ConstrainResizeGeometry(const QRect& proposedGeometry, Qt::Edges resizeEdges, qreal devicePixelRatio) const override` | 实现 constrain resize geometry 对应的业务或工具逻辑。 |
| [L50](../src/apps/controller/RoomCameraWindow.h#L50) | `RebuildLayout` | 声明 | `void RebuildLayout()` | 更新或应用 rebuild layout 相关逻辑。 |
| [L51](../src/apps/controller/RoomCameraWindow.h#L51) | `RebuildFocusLayout` | 声明 | `void RebuildFocusLayout()` | 更新或应用 rebuild focus layout 相关逻辑。 |
| [L52](../src/apps/controller/RoomCameraWindow.h#L52) | `RebuildOverviewLayout` | 声明 | `void RebuildOverviewLayout()` | 更新或应用 rebuild overview layout 相关逻辑。 |
| [L53](../src/apps/controller/RoomCameraWindow.h#L53) | `SetFocusedDevice` | 声明 | `void SetFocusedDevice(const QString& deviceId, bool maximize)` | 更新或应用 set focused device 相关逻辑。 |
| [L54](../src/apps/controller/RoomCameraWindow.h#L54) | `SetOverviewMode` | 声明 | `void SetOverviewMode(bool overview)` | 更新或应用 set overview mode 相关逻辑。 |
| [L55](../src/apps/controller/RoomCameraWindow.h#L55) | `UpdateSingleParticipantMode` | 声明 | `void UpdateSingleParticipantMode(int participantCount)` | 更新或应用 update single participant mode 相关逻辑。 |
| [L56](../src/apps/controller/RoomCameraWindow.h#L56) | `SingleWindowHeightForWidth` | 声明 | `int SingleWindowHeightForWidth(int windowWidth) const` | 实现 single window height for width 对应的业务或工具逻辑。 |
| [L57](../src/apps/controller/RoomCameraWindow.h#L57) | `SingleWindowWidthForHeight` | 声明 | `int SingleWindowWidthForHeight(int windowHeight) const` | 实现 single window width for height 对应的业务或工具逻辑。 |
| [L58](../src/apps/controller/RoomCameraWindow.h#L58) | `PlaceBesideMainWindow` | 声明 | `void PlaceBesideMainWindow(const QRect& mainWindowGeometry)` | 实现 place beside main window 对应的业务或工具逻辑。 |
| [L59](../src/apps/controller/RoomCameraWindow.h#L59) | `RemoveTile` | 声明 | `void RemoveTile(const QString& deviceId)` | 重置或移除 remove tile 相关逻辑。 |
| [L60](../src/apps/controller/RoomCameraWindow.h#L60) | `DetachAllSinks` | 声明 | `void DetachAllSinks()` | 实现 detach all sinks 对应的业务或工具逻辑。 |

## `src/apps/controller/RoomStatusIndicator.cpp`

[打开源码](../src/apps/controller/RoomStatusIndicator.cpp) · **文件作用：** 实现 room status indicator 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L17](../src/apps/controller/RoomStatusIndicator.cpp#L17) | `CreateRoomStatusIndicator` | 定义 | `QLabel* CreateRoomStatusIndicator(QWidget* parent, RoomStatusIcon type)` | 创建或初始化 create room status indicator 相关逻辑。 |

## `src/apps/controller/RoomStatusIndicator.h`

[打开源码](../src/apps/controller/RoomStatusIndicator.h) · **文件作用：** 声明 room status indicator 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L6](../src/apps/controller/RoomStatusIndicator.h#L6) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L7](../src/apps/controller/RoomStatusIndicator.h#L7) | `QWidget` | class | 定义 QWidget 的 class 类型和相关状态。 |
| [L11](../src/apps/controller/RoomStatusIndicator.h#L11) | `RoomStatusIcon` | enum class | 定义 RoomStatusIcon 的 enum class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L6](../src/apps/controller/RoomStatusIndicator.h#L6) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L7](../src/apps/controller/RoomStatusIndicator.h#L7) | `QWidget` | `class QWidget;` | 保存 q widget 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L19](../src/apps/controller/RoomStatusIndicator.h#L19) | `CreateRoomStatusIndicator` | 声明 | `QLabel* CreateRoomStatusIndicator(QWidget* parent, RoomStatusIcon type)` | 创建或初始化 create room status indicator 相关逻辑。 |

## `src/apps/controller/RoundedPopupMenu.cpp`

[打开源码](../src/apps/controller/RoundedPopupMenu.cpp) · **文件作用：** 实现 rounded popup menu 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L18](../src/apps/controller/RoundedPopupMenu.cpp#L18) | `RoundedPopupMenu::RoundedPopupMenu` | 定义 | `RoundedPopupMenu::RoundedPopupMenu( QWidget* parent, qreal cornerRadius) : QMenu(parent), cornerRadius_(cornerRadius)` | 构造并初始化 RoundedPopupMenu 实例。 |
| [L27](../src/apps/controller/RoundedPopupMenu.cpp#L27) | `RoundedPopupMenu::SetToggleAnchor` | 定义 | `void RoundedPopupMenu::SetToggleAnchor( QWidget* anchor, std::function<void()> anchorPressed)` | 更新或应用 set toggle anchor 相关逻辑。 |
| [L35](../src/apps/controller/RoundedPopupMenu.cpp#L35) | `RoundedPopupMenu::HandleToggleAnchorPress` | 定义 | `bool RoundedPopupMenu::HandleToggleAnchorPress(QMouseEvent* event)` | 接收并处理 handle toggle anchor press 相关逻辑。 |
| [L57](../src/apps/controller/RoundedPopupMenu.cpp#L57) | `RoundedPopupMenu::eventFilter` | 定义 | `bool RoundedPopupMenu::eventFilter(QObject* watched, QEvent* event)` | 实现 event filter 对应的业务或工具逻辑。 |
| [L68](../src/apps/controller/RoundedPopupMenu.cpp#L68) | `RoundedPopupMenu::hideEvent` | 定义 | `void RoundedPopupMenu::hideEvent(QHideEvent* event)` | 实现 hide event 对应的业务或工具逻辑。 |
| [L77](../src/apps/controller/RoundedPopupMenu.cpp#L77) | `RoundedPopupMenu::mousePressEvent` | 定义 | `void RoundedPopupMenu::mousePressEvent(QMouseEvent* event)` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L85](../src/apps/controller/RoundedPopupMenu.cpp#L85) | `RoundedPopupMenu::resizeEvent` | 定义 | `void RoundedPopupMenu::resizeEvent(QResizeEvent* event)` | 实现 resize event 对应的业务或工具逻辑。 |
| [L95](../src/apps/controller/RoundedPopupMenu.cpp#L95) | `RoundedPopupMenu::showEvent` | 定义 | `void RoundedPopupMenu::showEvent(QShowEvent* event)` | 实现 show event 对应的业务或工具逻辑。 |

## `src/apps/controller/RoundedPopupMenu.h`

[打开源码](../src/apps/controller/RoundedPopupMenu.h) · **文件作用：** 声明 rounded popup menu 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L11](../src/apps/controller/RoundedPopupMenu.h#L11) | `QEvent` | class | 定义 QEvent 的 class 类型和相关状态。 |
| [L12](../src/apps/controller/RoundedPopupMenu.h#L12) | `QHideEvent` | class | 定义 QHideEvent 的 class 类型和相关状态。 |
| [L13](../src/apps/controller/RoundedPopupMenu.h#L13) | `QMouseEvent` | class | 定义 QMouseEvent 的 class 类型和相关状态。 |
| [L14](../src/apps/controller/RoundedPopupMenu.h#L14) | `QShowEvent` | class | 定义 QShowEvent 的 class 类型和相关状态。 |
| [L18](../src/apps/controller/RoundedPopupMenu.h#L18) | `RoundedPopupMenu` | class | 定义 RoundedPopupMenu 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L11](../src/apps/controller/RoundedPopupMenu.h#L11) | `QEvent` | `class QEvent;` | 保存 q event 相关配置或运行状态。 |
| [L12](../src/apps/controller/RoundedPopupMenu.h#L12) | `QHideEvent` | `class QHideEvent;` | 保存 q hide event 相关配置或运行状态。 |
| [L13](../src/apps/controller/RoundedPopupMenu.h#L13) | `QMouseEvent` | `class QMouseEvent;` | 保存 q mouse event 相关配置或运行状态。 |
| [L14](../src/apps/controller/RoundedPopupMenu.h#L14) | `QShowEvent` | `class QShowEvent;` | 保存 q show event 相关配置或运行状态。 |
| [L38](../src/apps/controller/RoundedPopupMenu.h#L38) | `cornerRadius_` | `qreal cornerRadius_ = 12.0;` | 保存 corner radius 相关配置或运行状态。 |
| [L39](../src/apps/controller/RoundedPopupMenu.h#L39) | `toggleAnchor_` | `QPointer<QWidget> toggleAnchor_;` | 保存 toggle anchor 相关配置或运行状态。 |
| [L41](../src/apps/controller/RoundedPopupMenu.h#L41) | `applicationFilterInstalled_` | `bool applicationFilterInstalled_ = false;` | 保存 application filter installed 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L20](../src/apps/controller/RoundedPopupMenu.h#L20) | `RoundedPopupMenu` | 声明 | `explicit RoundedPopupMenu( QWidget* parent = nullptr, qreal cornerRadius = 12.0)` | 实现 rounded popup menu 对应的业务或工具逻辑。 |
| [L24](../src/apps/controller/RoundedPopupMenu.h#L24) | `SetToggleAnchor` | 定义 | `void SetToggleAnchor( QWidget* anchor, std::function<void()> anchorPressed = {})` | 更新或应用 set toggle anchor 相关逻辑。 |
| [L29](../src/apps/controller/RoundedPopupMenu.h#L29) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L30](../src/apps/controller/RoundedPopupMenu.h#L30) | `hideEvent` | 声明 | `void hideEvent(QHideEvent* event) override` | 实现 hide event 对应的业务或工具逻辑。 |
| [L31](../src/apps/controller/RoundedPopupMenu.h#L31) | `mousePressEvent` | 声明 | `void mousePressEvent(QMouseEvent* event) override` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L32](../src/apps/controller/RoundedPopupMenu.h#L32) | `resizeEvent` | 声明 | `void resizeEvent(QResizeEvent* event) override` | 实现 resize event 对应的业务或工具逻辑。 |
| [L33](../src/apps/controller/RoundedPopupMenu.h#L33) | `showEvent` | 声明 | `void showEvent(QShowEvent* event) override` | 实现 show event 对应的业务或工具逻辑。 |
| [L36](../src/apps/controller/RoundedPopupMenu.h#L36) | `HandleToggleAnchorPress` | 声明 | `bool HandleToggleAnchorPress(QMouseEvent* event)` | 接收并处理 handle toggle anchor press 相关逻辑。 |

## `src/apps/controller/ScreenFrameRateLogger.cpp`

[打开源码](../src/apps/controller/ScreenFrameRateLogger.cpp) · **文件作用：** 实现 screen frame rate logger 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L37](../src/apps/controller/ScreenFrameRateLogger.cpp#L37) | `ScreenFrameRateLogRecord` | struct | 定义 ScreenFrameRateLogRecord 的 struct 类型和相关状态。 |
| [L63](../src/apps/controller/ScreenFrameRateLogger.cpp#L63) | `AsyncScreenFrameRateLogger` | class | 定义 AsyncScreenFrameRateLogger 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L87](../src/apps/controller/ScreenFrameRateLogger.cpp#L87) | `kMaximumQueuedRecords` | `static constexpr std::size_t kMaximumQueuedRecords = 4096;` | 定义 maximum queued records 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L31](../src/apps/controller/ScreenFrameRateLogger.cpp#L31) | `CsvCell` | 定义 | `QString CsvCell(const std::string &value)` | 实现 csv cell 对应的业务或工具逻辑。 |
| [L65](../src/apps/controller/ScreenFrameRateLogger.cpp#L65) | `Instance` | 定义 | `static AsyncScreenFrameRateLogger &Instance()` | 实现 instance 对应的业务或工具逻辑。 |
| [L70](../src/apps/controller/ScreenFrameRateLogger.cpp#L70) | `Enqueue` | 定义 | `void Enqueue(std::vector<ScreenFrameRateLogRecord> records)` | 实现 enqueue 对应的业务或工具逻辑。 |
| [L89](../src/apps/controller/ScreenFrameRateLogger.cpp#L89) | `AsyncScreenFrameRateLogger` | 定义 | `AsyncScreenFrameRateLogger() : worker_([this](std::stop_token stopToken) { Run(stopToken); }) {}` | 实现 async screen frame rate logger 对应的业务或工具逻辑。 |
| [L92](../src/apps/controller/ScreenFrameRateLogger.cpp#L92) | `~AsyncScreenFrameRateLogger` | 定义 | `~AsyncScreenFrameRateLogger()` | 停止相关活动并释放 AsyncScreenFrameRateLogger 实例拥有的资源。 |
| [L100](../src/apps/controller/ScreenFrameRateLogger.cpp#L100) | `Run` | 定义 | `void Run(std::stop_token stopToken)` | 执行后台循环或调度 run 相关逻辑。 |
| [L184](../src/apps/controller/ScreenFrameRateLogger.cpp#L184) | `AppendScreenFrameRateLog` | 定义 | `void AppendScreenFrameRateLog(const SessionDiagnosticsSnapshot &diagnostics)` | 实现 append screen frame rate log 对应的业务或工具逻辑。 |

## `src/apps/controller/ScreenFrameRateLogger.h`

[打开源码](../src/apps/controller/ScreenFrameRateLogger.h) · **文件作用：** 声明 screen frame rate logger 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L7](../src/apps/controller/ScreenFrameRateLogger.h#L7) | `SessionDiagnosticsSnapshot` | struct | 定义 SessionDiagnosticsSnapshot 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L7](../src/apps/controller/ScreenFrameRateLogger.h#L7) | `SessionDiagnosticsSnapshot` | `struct SessionDiagnosticsSnapshot;` | 保存可跨层读取的状态快照：session diagnostics snapshot。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L12](../src/apps/controller/ScreenFrameRateLogger.h#L12) | `AppendScreenFrameRateLog` | 声明 | `void AppendScreenFrameRateLog(const SessionDiagnosticsSnapshot &diagnostics)` | 实现 append screen frame rate log 对应的业务或工具逻辑。 |

## `src/apps/controller/ui/morph/MorphIconButtonBinding.cpp`

[打开源码](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp) · **文件作用：** 实现 morph icon button binding 相关函数与文件级辅助逻辑。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L22](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L22) | `kBindingName` | `constexpr auto kBindingName = "remoteCMorphIconBinding";` | 定义 binding name 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L24](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L24) | `animationLevel` | 定义 | `int animationLevel()` | 实现 animation level 对应的业务或工具逻辑。 |
| [L29](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L29) | `mixColor` | 定义 | `QColor mixColor(const QColor& source, const QColor& target, double progress)` | 实现 mix color 对应的业务或工具逻辑。 |
| [L41](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L41) | `MorphIconButtonBinding::attach` | 定义 | `MorphIconButtonBinding* MorphIconButtonBinding::attach( QAbstractButton* button, const QString& sourceResource, const QString& targetResource, Interaction interaction, const QSize& iconSize, const QColor& sourceColor,...` | 实现 attach 对应的业务或工具逻辑。 |
| [L78](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L78) | `QObject` | 定义 | `: QObject(button), button_(button), sourceResource_(std::move(sourceResource)), targetResource_(std::move(targetResource)), interaction_(interaction), iconSize_(iconSize), sourceColor_(sourceColor), targetColor_(targe...` | 实现 q object 对应的业务或工具逻辑。 |
| [L113](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L113) | `MorphIconButtonBinding::setTarget` | 定义 | `void MorphIconButtonBinding::setTarget(bool target, bool animated)` | 更新或应用 set target 相关逻辑。 |
| [L127](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L127) | `MorphIconButtonBinding::pulse` | 定义 | `void MorphIconButtonBinding::pulse(int holdMilliseconds)` | 实现 pulse 对应的业务或工具逻辑。 |
| [L135](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L135) | `MorphIconButtonBinding::eventFilter` | 定义 | `bool MorphIconButtonBinding::eventFilter(QObject* watched, QEvent* event)` | 实现 event filter 对应的业务或工具逻辑。 |
| [L154](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L154) | `MorphIconButtonBinding::startTransition` | 定义 | `void MorphIconButtonBinding::startTransition(double targetProgress, bool animated)` | 启动 start transition 相关逻辑。 |
| [L178](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L178) | `MorphIconButtonBinding::currentColor` | 定义 | `QColor MorphIconButtonBinding::currentColor() const` | 实现 current color 对应的业务或工具逻辑。 |
| [L194](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L194) | `MorphIconButtonBinding::render` | 定义 | `void MorphIconButtonBinding::render()` | 准备或呈现 render 相关逻辑。 |

## `src/apps/controller/ui/morph/MorphIconButtonBinding.h`

[打开源码](../src/apps/controller/ui/morph/MorphIconButtonBinding.h) · **文件作用：** 声明 morph icon button binding 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L14](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L14) | `QAbstractButton` | class | 定义 QAbstractButton 的 class 类型和相关状态。 |
| [L15](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L15) | `QEvent` | class | 定义 QEvent 的 class 类型和相关状态。 |
| [L19](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L19) | `MorphIconButtonBinding` | class | 定义 MorphIconButtonBinding 的 class 类型和相关状态。 |
| [L21](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L21) | `Interaction` | enum class | 定义 Interaction 的 enum class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L14](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L14) | `QAbstractButton` | `class QAbstractButton;` | 保存 q abstract button 相关配置或运行状态。 |
| [L15](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L15) | `QEvent` | `class QEvent;` | 保存 q event 相关配置或运行状态。 |
| [L58](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L58) | `button_` | `QAbstractButton* button_ = nullptr;` | 保存 button 相关配置或运行状态。 |
| [L59](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L59) | `sourceResource_` | `QString sourceResource_;` | 保存 source resource 相关配置或运行状态。 |
| [L60](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L60) | `targetResource_` | `QString targetResource_;` | 保存 target resource 相关配置或运行状态。 |
| [L61](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L61) | `interaction_` | `Interaction interaction_ = Interaction::State;` | 保存 interaction 相关配置或运行状态。 |
| [L62](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L62) | `iconSize_` | `QSize iconSize_{18, 18};` | 保存计数、尺寸或速率指标：icon size。 |
| [L63](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L63) | `sourceColor_` | `QColor sourceColor_;` | 保存 source color 相关配置或运行状态。 |
| [L64](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L64) | `targetColor_` | `QColor targetColor_;` | 保存 target color 相关配置或运行状态。 |
| [L65](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L65) | `core_` | `MorphIconCore core_;` | 保存 core 相关配置或运行状态。 |
| [L66](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L66) | `spring_` | `Spring spring_;` | 保存 spring 相关配置或运行状态。 |
| [L67](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L67) | `timer_` | `QTimer timer_;` | 保存定时、截止或超时状态：timer。 |
| [L68](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L68) | `elapsed_` | `QElapsedTimer elapsed_;` | 保存 elapsed 相关配置或运行状态。 |
| [L69](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L69) | `progress_` | `double progress_ = 0.0;` | 保存 progress 相关配置或运行状态。 |
| [L70](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L70) | `start_` | `double start_ = 0.0;` | 保存 start 相关配置或运行状态。 |
| [L71](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L71) | `end_` | `double end_ = 0.0;` | 保存 end 相关配置或运行状态。 |
| [L72](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L72) | `target_` | `bool target_ = false;` | 保存 target 相关配置或运行状态。 |
| [L73](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L73) | `initialized_` | `bool initialized_ = false;` | 保存 initialized 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L27](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L27) | `attach` | 声明 | `static MorphIconButtonBinding* attach( QAbstractButton* button, const QString& sourceResource, const QString& targetResource, Interaction interaction, const QSize& iconSize = QSize(18, 18), const QColor& sourceColor =...` | 实现 attach 对应的业务或工具逻辑。 |
| [L36](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L36) | `setTarget` | 声明 | `void setTarget(bool target, bool animated = true)` | 更新或应用 set target 相关逻辑。 |
| [L37](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L37) | `pulse` | 声明 | `void pulse(int holdMilliseconds = 1100)` | 实现 pulse 对应的业务或工具逻辑。 |
| [L43](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L43) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L46](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L46) | `MorphIconButtonBinding` | 声明 | `MorphIconButtonBinding(QAbstractButton* button, QString sourceResource, QString targetResource, Interaction interaction, QSize iconSize, QColor sourceColor, QColor targetColor)` | 实现 morph icon button binding 对应的业务或工具逻辑。 |
| [L54](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L54) | `startTransition` | 声明 | `void startTransition(double targetProgress, bool animated)` | 启动 start transition 相关逻辑。 |
| [L55](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L55) | `render` | 声明 | `void render()` | 准备或呈现 render 相关逻辑。 |
| [L56](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L56) | `currentColor` | 声明 | `QColor currentColor() const` | 实现 current color 对应的业务或工具逻辑。 |

## `src/apps/controller/ui/morph/MorphIconCore.cpp`

[打开源码](../src/apps/controller/ui/morph/MorphIconCore.cpp) · **文件作用：** 实现 morph icon core 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L33](../src/apps/controller/ui/morph/MorphIconCore.cpp#L33) | `CachedMorphPlan` | struct | 定义 CachedMorphPlan 的 struct 类型和相关状态。 |
| [L98](../src/apps/controller/ui/morph/MorphIconCore.cpp#L98) | `RawSegment` | struct | 定义 RawSegment 的 struct 类型和相关状态。 |
| [L103](../src/apps/controller/ui/morph/MorphIconCore.cpp#L103) | `RawSubpath` | struct | 定义 RawSubpath 的 struct 类型和相关状态。 |
| [L109](../src/apps/controller/ui/morph/MorphIconCore.cpp#L109) | `PathParser` | class | 定义 PathParser 的 class 类型和相关状态。 |
| [L370](../src/apps/controller/ui/morph/MorphIconCore.cpp#L370) | `CubicBuilder` | class | 定义 CubicBuilder 的 class 类型和相关状态。 |
| [L818](../src/apps/controller/ui/morph/MorphIconCore.cpp#L818) | `Alignment` | struct | 定义 Alignment 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L25](../src/apps/controller/ui/morph/MorphIconCore.cpp#L25) | `kPi` | `constexpr double kPi = 3.1415926535897932384626433832795;` | 定义 pi 的编译期常量或产品边界。 |
| [L26](../src/apps/controller/ui/morph/MorphIconCore.cpp#L26) | `kTau` | `constexpr double kTau = 2.0 * kPi;` | 定义 tau 的编译期常量或产品边界。 |
| [L27](../src/apps/controller/ui/morph/MorphIconCore.cpp#L27) | `kCornerThreshold` | `constexpr double kCornerThreshold = kPi / 8.0;` | 定义 corner threshold 的编译期常量或产品边界。 |
| [L28](../src/apps/controller/ui/morph/MorphIconCore.cpp#L28) | `kLengthWeight` | `constexpr double kLengthWeight = 0.35;` | 定义 length weight 的编译期常量或产品边界。 |
| [L29](../src/apps/controller/ui/morph/MorphIconCore.cpp#L29) | `kRotationTieBreak` | `constexpr double kRotationTieBreak = 0.05;` | 定义 rotation tie break 的编译期常量或产品边界。 |
| [L30](../src/apps/controller/ui/morph/MorphIconCore.cpp#L30) | `kGlobalResidualThreshold` | `constexpr double kGlobalResidualThreshold = 5e-3;` | 定义 global residual threshold 的编译期常量或产品边界。 |
| [L31](../src/apps/controller/ui/morph/MorphIconCore.cpp#L31) | `kEpsilon` | `constexpr double kEpsilon = 1e-12;` | 定义 epsilon 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L38](../src/apps/controller/ui/morph/MorphIconCore.cpp#L38) | `morphPlanCacheMutex` | 定义 | `std::mutex& morphPlanCacheMutex()` | 实现 morph plan cache mutex 对应的业务或工具逻辑。 |
| [L44](../src/apps/controller/ui/morph/MorphIconCore.cpp#L44) | `morphPlanCache` | 定义 | `std::unordered_map<std::string, CachedMorphPlan>& morphPlanCache()` | 实现 morph plan cache 对应的业务或工具逻辑。 |
| [L50](../src/apps/controller/ui/morph/MorphIconCore.cpp#L50) | `cross` | 定义 | `double cross(const QPointF& a, const QPointF& b)` | 实现 cross 对应的业务或工具逻辑。 |
| [L55](../src/apps/controller/ui/morph/MorphIconCore.cpp#L55) | `dot` | 定义 | `double dot(const QPointF& a, const QPointF& b)` | 实现 dot 对应的业务或工具逻辑。 |
| [L60](../src/apps/controller/ui/morph/MorphIconCore.cpp#L60) | `norm` | 定义 | `double norm(const QPointF& p)` | 实现 norm 对应的业务或工具逻辑。 |
| [L65](../src/apps/controller/ui/morph/MorphIconCore.cpp#L65) | `centroid` | 定义 | `QPointF centroid(const std::vector<QPointF>& points)` | 实现 centroid 对应的业务或工具逻辑。 |
| [L74](../src/apps/controller/ui/morph/MorphIconCore.cpp#L74) | `polylineLength` | 定义 | `double polylineLength(const std::vector<QPointF>& points)` | 实现 polyline length 对应的业务或工具逻辑。 |
| [L83](../src/apps/controller/ui/morph/MorphIconCore.cpp#L83) | `reversePoints` | 定义 | `std::vector<QPointF> reversePoints(const std::vector<QPointF>& points)` | 实现 reverse points 对应的业务或工具逻辑。 |
| [L88](../src/apps/controller/ui/morph/MorphIconCore.cpp#L88) | `rotatePoints` | 定义 | `std::vector<QPointF> rotatePoints(const std::vector<QPointF>& points, int offset)` | 实现 rotate points 对应的业务或工具逻辑。 |
| [L111](../src/apps/controller/ui/morph/MorphIconCore.cpp#L111) | `PathParser` | 定义 | `explicit PathParser(QString data) : data_(std::move(data)) {}` | 实现 path parser 对应的业务或工具逻辑。 |
| [L113](../src/apps/controller/ui/morph/MorphIconCore.cpp#L113) | `parse` | 定义 | `std::vector<RawSubpath> parse()` | 解码或解析 parse 相关逻辑。 |
| [L244](../src/apps/controller/ui/morph/MorphIconCore.cpp#L244) | `atEnd` | 定义 | `bool atEnd() const { return index_ >= data_.size(); }` | 实现 at end 对应的业务或工具逻辑。 |
| [L245](../src/apps/controller/ui/morph/MorphIconCore.cpp#L245) | `peek` | 定义 | `QChar peek() const { return atEnd() ? QChar() : data_[index_]; }` | 实现 peek 对应的业务或工具逻辑。 |
| [L246](../src/apps/controller/ui/morph/MorphIconCore.cpp#L246) | `take` | 定义 | `QChar take() { return data_[index_++]; }` | 实现 take 对应的业务或工具逻辑。 |
| [L248](../src/apps/controller/ui/morph/MorphIconCore.cpp#L248) | `isCommand` | 定义 | `static bool isCommand(QChar value)` | 判断 is command 相关逻辑。 |
| [L253](../src/apps/controller/ui/morph/MorphIconCore.cpp#L253) | `skipSeparators` | 定义 | `void skipSeparators()` | 实现 skip separators 对应的业务或工具逻辑。 |
| [L269](../src/apps/controller/ui/morph/MorphIconCore.cpp#L269) | `number` | 定义 | `double number()` | 实现 number 对应的业务或工具逻辑。 |
| [L313](../src/apps/controller/ui/morph/MorphIconCore.cpp#L313) | `flag` | 定义 | `int flag()` | 实现 flag 对应的业务或工具逻辑。 |
| [L322](../src/apps/controller/ui/morph/MorphIconCore.cpp#L322) | `point` | 定义 | `QPointF point(bool relative)` | 实现 point 对应的业务或工具逻辑。 |
| [L327](../src/apps/controller/ui/morph/MorphIconCore.cpp#L327) | `pointFromOrigin` | 定义 | `QPointF pointFromOrigin(bool relative, const QPointF& origin)` | 实现 point from origin 对应的业务或工具逻辑。 |
| [L338](../src/apps/controller/ui/morph/MorphIconCore.cpp#L338) | `ensureOpen` | 定义 | `void ensureOpen()` | 实现 ensure open 对应的业务或工具逻辑。 |
| [L349](../src/apps/controller/ui/morph/MorphIconCore.cpp#L349) | `append` | 定义 | `void append(QChar kind, std::initializer_list<double> values)` | 实现 append 对应的业务或工具逻辑。 |
| [L372](../src/apps/controller/ui/morph/MorphIconCore.cpp#L372) | `CubicBuilder` | 定义 | `explicit CubicBuilder(QPointF start) : current_(start)` | 实现 cubic builder 对应的业务或工具逻辑。 |
| [L377](../src/apps/controller/ui/morph/MorphIconCore.cpp#L377) | `cubic` | 定义 | `void cubic(const QPointF& c1, const QPointF& c2, const QPointF& end)` | 实现 cubic 对应的业务或工具逻辑。 |
| [L385](../src/apps/controller/ui/morph/MorphIconCore.cpp#L385) | `line` | 定义 | `void line(const QPointF& end)` | 实现 line 对应的业务或工具逻辑。 |
| [L394](../src/apps/controller/ui/morph/MorphIconCore.cpp#L394) | `quadratic` | 定义 | `void quadratic(const QPointF& control, const QPointF& end)` | 实现 quadratic 对应的业务或工具逻辑。 |
| [L400](../src/apps/controller/ui/morph/MorphIconCore.cpp#L400) | `arc` | 定义 | `void arc(double rx0, double ry0, double rotationDegrees, int large, int sweep, const QPointF& end)` | 实现 arc 对应的业务或工具逻辑。 |
| [L475](../src/apps/controller/ui/morph/MorphIconCore.cpp#L475) | `finish` | 定义 | `CubicPath finish(bool closed)` | 停止 finish 相关逻辑。 |
| [L489](../src/apps/controller/ui/morph/MorphIconCore.cpp#L489) | `lowerPath` | 定义 | `CubicPath lowerPath(const RawSubpath& raw)` | 实现 lower path 对应的业务或工具逻辑。 |
| [L506](../src/apps/controller/ui/morph/MorphIconCore.cpp#L506) | `attributeNumber` | 定义 | `double attributeNumber(const QXmlStreamAttributes& attributes, const QString& name, double fallback = 0.0)` | 实现 attribute number 对应的业务或工具逻辑。 |
| [L518](../src/apps/controller/ui/morph/MorphIconCore.cpp#L518) | `parsePointList` | 定义 | `std::vector<double> parsePointList(QString text)` | 解码或解析 parse point list 相关逻辑。 |
| [L533](../src/apps/controller/ui/morph/MorphIconCore.cpp#L533) | `polyPath` | 定义 | `CubicPath polyPath(const std::vector<double>& values, bool closed)` | 实现 poly path 对应的业务或工具逻辑。 |
| [L545](../src/apps/controller/ui/morph/MorphIconCore.cpp#L545) | `ellipsePath` | 定义 | `CubicPath ellipsePath(double cx, double cy, double rx, double ry)` | 实现 ellipse path 对应的业务或工具逻辑。 |
| [L561](../src/apps/controller/ui/morph/MorphIconCore.cpp#L561) | `rectPath` | 定义 | `CubicPath rectPath(const QXmlStreamAttributes& attributes)` | 实现 rect path 对应的业务或工具逻辑。 |
| [L605](../src/apps/controller/ui/morph/MorphIconCore.cpp#L605) | `segmentCount` | 定义 | `int segmentCount(const CubicPath& path)` | 实现 segment count 对应的业务或工具逻辑。 |
| [L610](../src/apps/controller/ui/morph/MorphIconCore.cpp#L610) | `cubicPoint` | 定义 | `QPointF cubicPoint(const CubicPath& path, int segment, double t)` | 实现 cubic point 对应的业务或工具逻辑。 |
| [L622](../src/apps/controller/ui/morph/MorphIconCore.cpp#L622) | `cubicSpeed` | 定义 | `double cubicSpeed(const CubicPath& path, int segment, double t)` | 实现 cubic speed 对应的业务或工具逻辑。 |
| [L633](../src/apps/controller/ui/morph/MorphIconCore.cpp#L633) | `segmentLength` | 定义 | `double segmentLength(const CubicPath& path, int segment, double end = 1.0)` | 实现 segment length 对应的业务或工具逻辑。 |
| [L651](../src/apps/controller/ui/morph/MorphIconCore.cpp#L651) | `endpointTangent` | 定义 | `QPointF endpointTangent(const CubicPath& path, int segment, bool atEnd)` | 停止 endpoint tangent 相关逻辑。 |
| [L669](../src/apps/controller/ui/morph/MorphIconCore.cpp#L669) | `detectCorners` | 定义 | `std::vector<int> detectCorners(const CubicPath& path)` | 实现 detect corners 对应的业务或工具逻辑。 |
| [L696](../src/apps/controller/ui/morph/MorphIconCore.cpp#L696) | `invertLength` | 定义 | `double invertLength(const CubicPath& path, int segment, double target, double length)` | 实现 invert length 对应的业务或工具逻辑。 |
| [L715](../src/apps/controller/ui/morph/MorphIconCore.cpp#L715) | `resamplePath` | 定义 | `std::vector<QPointF> resamplePath(const CubicPath& path, int sampleCount)` | 实现 resample path 对应的业务或工具逻辑。 |
| [L826](../src/apps/controller/ui/morph/MorphIconCore.cpp#L826) | `alignPair` | 定义 | `Alignment alignPair(const SampledPath& source, const SampledPath& target)` | 实现 align pair 对应的业务或工具逻辑。 |
| [L859](../src/apps/controller/ui/morph/MorphIconCore.cpp#L859) | `costMatrix` | 定义 | `std::vector<std::vector<double>> costMatrix(const std::vector<SampledPath>& source, const std::vector<SampledPath>& target)` | 实现 cost matrix 对应的业务或工具逻辑。 |
| [L875](../src/apps/controller/ui/morph/MorphIconCore.cpp#L875) | `bestPermutation` | 定义 | `std::vector<int> bestPermutation(const std::vector<std::vector<double>>& costs)` | 实现 best permutation 对应的业务或工具逻辑。 |
| [L907](../src/apps/controller/ui/morph/MorphIconCore.cpp#L907) | `bestSurjection` | 定义 | `std::vector<int> bestSurjection(const std::vector<std::vector<double>>& costs)` | 实现 best surjection 对应的业务或工具逻辑。 |
| [L956](../src/apps/controller/ui/morph/MorphIconCore.cpp#L956) | `applyGlobalAlignment` | 定义 | `void applyGlobalAlignment(MorphPlan& plan)` | 更新或应用 apply global alignment 相关逻辑。 |
| [L1004](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1004) | `setError` | 定义 | `void setError(QString* output, const QString& value)` | 更新或应用 set error 相关逻辑。 |
| [L1011](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1011) | `MorphIconCore::loadSvg` | 定义 | `bool MorphIconCore::loadSvg(const QString& resource, std::vector<CubicPath>& paths, QRectF& viewBox, QString* errorMessage)` | 读取或恢复 load svg 相关逻辑。 |
| [L1091](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1091) | `MorphIconCore::resample` | 定义 | `std::vector<SampledPath> MorphIconCore::resample( const std::vector<CubicPath>& paths, int sampleCount)` | 实现 resample 对应的业务或工具逻辑。 |
| [L1102](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1102) | `MorphIconCore::procrustes` | 定义 | `Similarity MorphIconCore::procrustes(const std::vector<QPointF>& source, const std::vector<QPointF>& target, const QPointF& sourceCentroid, const QPointF& targetCentroid)` | 实现 procrustes 对应的业务或工具逻辑。 |
| [L1134](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1134) | `MorphIconCore::buildPlan` | 定义 | `MorphPlan MorphIconCore::buildPlan(const std::vector<SampledPath>& source, const std::vector<SampledPath>& target)` | 创建或初始化 build plan 相关逻辑。 |
| [L1190](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1190) | `MorphIconCore::configure` | 定义 | `bool MorphIconCore::configure(const QString& sourceSvgResource, const QString& targetSvgResource, int sampleCount, QString* errorMessage)` | 更新或应用 configure 相关逻辑。 |
| [L1249](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1249) | `MorphIconCore::interpolate` | 定义 | `void MorphIconCore::interpolate( double progress, std::vector<std::vector<QPointF>>& output) const` | 实现 interpolate 对应的业务或工具逻辑。 |
| [L1281](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1281) | `MorphIconCore::painterPath` | 定义 | `QPainterPath MorphIconCore::painterPath(double progress) const` | 准备或呈现 painter path 相关逻辑。 |
| [L1296](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1296) | `MorphIconCore::paint` | 定义 | `void MorphIconCore::paint(QPainter& painter, const QRectF& targetRect, const QColor& color, double progress, qreal strokeWidth) const` | 准备或呈现 paint 相关逻辑。 |
| [L1319](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1319) | `Spring::configure` | 定义 | `void Spring::configure(double stiffness, double damping) noexcept` | 更新或应用 configure 相关逻辑。 |
| [L1325](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1325) | `Spring::start` | 定义 | `void Spring::start() noexcept` | 启动 start 相关逻辑。 |
| [L1331](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1331) | `Spring::step` | 定义 | `bool Spring::step(double seconds) noexcept` | 实现 step 对应的业务或工具逻辑。 |

## `src/apps/controller/ui/morph/MorphIconCore.h`

[打开源码](../src/apps/controller/ui/morph/MorphIconCore.h) · **文件作用：** 声明 morph icon core 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L14](../src/apps/controller/ui/morph/MorphIconCore.h#L14) | `QPainter` | class | 定义 QPainter 的 class 类型和相关状态。 |
| [L18](../src/apps/controller/ui/morph/MorphIconCore.h#L18) | `CubicPath` | struct | 定义 CubicPath 的 struct 类型和相关状态。 |
| [L24](../src/apps/controller/ui/morph/MorphIconCore.h#L24) | `SampledPath` | struct | 定义 SampledPath 的 struct 类型和相关状态。 |
| [L29](../src/apps/controller/ui/morph/MorphIconCore.h#L29) | `Similarity` | struct | 定义 Similarity 的 struct 类型和相关状态。 |
| [L35](../src/apps/controller/ui/morph/MorphIconCore.h#L35) | `PlanItem` | struct | 定义 PlanItem 的 struct 类型和相关状态。 |
| [L51](../src/apps/controller/ui/morph/MorphIconCore.h#L51) | `MorphPlan` | struct | 定义 MorphPlan 的 struct 类型和相关状态。 |
| [L64](../src/apps/controller/ui/morph/MorphIconCore.h#L64) | `MorphIconCore` | class | MIT-licensed C++ port of Morphicons 1.7.1 core geometry. It intentionally has no DOM/web dependency: SVG resources are parsed once, then only sampled point clouds and a cached p... |
| [L102](../src/apps/controller/ui/morph/MorphIconCore.h#L102) | `Spring` | class | 定义 Spring 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L14](../src/apps/controller/ui/morph/MorphIconCore.h#L14) | `QPainter` | `class QPainter;` | 保存 q painter 相关配置或运行状态。 |
| [L20](../src/apps/controller/ui/morph/MorphIconCore.h#L20) | `points` | `std::vector<QPointF> points;` | p0, c1, c2, p1, c1, c2, p2, ...; adjacent cubics share an endpoint. |
| [L21](../src/apps/controller/ui/morph/MorphIconCore.h#L21) | `closed` | `bool closed = false;` | 保存 closed 相关配置或运行状态。 |
| [L25](../src/apps/controller/ui/morph/MorphIconCore.h#L25) | `points` | `std::vector<QPointF> points;` | 保存 points 相关配置或运行状态。 |
| [L26](../src/apps/controller/ui/morph/MorphIconCore.h#L26) | `closed` | `bool closed = false;` | 保存 closed 相关配置或运行状态。 |
| [L30](../src/apps/controller/ui/morph/MorphIconCore.h#L30) | `theta` | `double theta = 0.0;` | 保存 theta 相关配置或运行状态。 |
| [L31](../src/apps/controller/ui/morph/MorphIconCore.h#L31) | `sigma` | `double sigma = 1.0;` | 保存 sigma 相关配置或运行状态。 |
| [L32](../src/apps/controller/ui/morph/MorphIconCore.h#L32) | `residual` | `double residual = 0.0;` | 保存 residual 相关配置或运行状态。 |
| [L36](../src/apps/controller/ui/morph/MorphIconCore.h#L36) | `source` | `std::vector<QPointF> source;` | 保存 source 相关配置或运行状态。 |
| [L37](../src/apps/controller/ui/morph/MorphIconCore.h#L37) | `sourceCentered` | `std::vector<QPointF> sourceCentered;` | 保存 source centered 相关配置或运行状态。 |
| [L38](../src/apps/controller/ui/morph/MorphIconCore.h#L38) | `targetAligned` | `std::vector<QPointF> targetAligned;` | 保存 target aligned 相关配置或运行状态。 |
| [L39](../src/apps/controller/ui/morph/MorphIconCore.h#L39) | `targetOriented` | `std::vector<QPointF> targetOriented;` | 保存 target oriented 相关配置或运行状态。 |
| [L40](../src/apps/controller/ui/morph/MorphIconCore.h#L40) | `sourceCentroid` | `QPointF sourceCentroid;` | 保存身份或作用域标识：source centroid。 |
| [L41](../src/apps/controller/ui/morph/MorphIconCore.h#L41) | `targetCentroid` | `QPointF targetCentroid;` | 保存身份或作用域标识：target centroid。 |
| [L42](../src/apps/controller/ui/morph/MorphIconCore.h#L42) | `theta` | `double theta = 0.0;` | 保存 theta 相关配置或运行状态。 |
| [L43](../src/apps/controller/ui/morph/MorphIconCore.h#L43) | `logSigma` | `double logSigma = 0.0;` | 保存 log sigma 相关配置或运行状态。 |
| [L44](../src/apps/controller/ui/morph/MorphIconCore.h#L44) | `residual` | `double residual = 0.0;` | 保存 residual 相关配置或运行状态。 |
| [L45](../src/apps/controller/ui/morph/MorphIconCore.h#L45) | `closed` | `bool closed = false;` | 保存 closed 相关配置或运行状态。 |
| [L46](../src/apps/controller/ui/morph/MorphIconCore.h#L46) | `hasBlockTransport` | `bool hasBlockTransport = false;` | 保存 has block transport 相关配置或运行状态。 |
| [L47](../src/apps/controller/ui/morph/MorphIconCore.h#L47) | `blockOffset` | `QPointF blockOffset;` | 保存 block offset 相关配置或运行状态。 |
| [L48](../src/apps/controller/ui/morph/MorphIconCore.h#L48) | `blockDrift` | `QPointF blockDrift;` | 保存 block drift 相关配置或运行状态。 |
| [L52](../src/apps/controller/ui/morph/MorphIconCore.h#L52) | `items` | `std::vector<PlanItem> items;` | 保存 items 相关配置或运行状态。 |
| [L53](../src/apps/controller/ui/morph/MorphIconCore.h#L53) | `sampleCount` | `int sampleCount = 0;` | 保存计数、尺寸或速率指标：sample count。 |
| [L98](../src/apps/controller/ui/morph/MorphIconCore.h#L98) | `plan_` | `MorphPlan plan_;` | 保存 plan 相关配置或运行状态。 |
| [L99](../src/apps/controller/ui/morph/MorphIconCore.h#L99) | `sharedViewBox_` | `QRectF sharedViewBox_{0.0, 0.0, 24.0, 24.0};` | 保存 shared view box 相关配置或运行状态。 |
| [L112](../src/apps/controller/ui/morph/MorphIconCore.h#L112) | `value_` | `double value_ = 1.0;` | 保存 value 相关配置或运行状态。 |
| [L113](../src/apps/controller/ui/morph/MorphIconCore.h#L113) | `velocity_` | `double velocity_ = 0.0;` | 保存 velocity 相关配置或运行状态。 |
| [L114](../src/apps/controller/ui/morph/MorphIconCore.h#L114) | `stiffness_` | `double stiffness_ = 250.0;` | 保存 stiffness 相关配置或运行状态。 |
| [L115](../src/apps/controller/ui/morph/MorphIconCore.h#L115) | `damping_` | `double damping_ = 24.0;` | 保存 damping 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L66](../src/apps/controller/ui/morph/MorphIconCore.h#L66) | `configure` | 声明 | `bool configure(const QString& sourceSvgResource, const QString& targetSvgResource, int sampleCount = 64, QString* errorMessage = nullptr)` | 更新或应用 configure 相关逻辑。 |
| [L74](../src/apps/controller/ui/morph/MorphIconCore.h#L74) | `interpolate` | 声明 | `void interpolate(double progress, std::vector<std::vector<QPointF>>& output) const` | 实现 interpolate 对应的业务或工具逻辑。 |
| [L76](../src/apps/controller/ui/morph/MorphIconCore.h#L76) | `painterPath` | 声明 | `[[nodiscard]] QPainterPath painterPath(double progress) const` | 准备或呈现 painter path 相关逻辑。 |
| [L77](../src/apps/controller/ui/morph/MorphIconCore.h#L77) | `paint` | 声明 | `void paint(QPainter& painter, const QRectF& targetRect, const QColor& color, double progress, qreal strokeWidth = 2.0) const` | 准备或呈现 paint 相关逻辑。 |
| [L83](../src/apps/controller/ui/morph/MorphIconCore.h#L83) | `loadSvg` | 声明 | `static bool loadSvg(const QString& resource, std::vector<CubicPath>& paths, QRectF& viewBox, QString* errorMessage = nullptr)` | 读取或恢复 load svg 相关逻辑。 |
| [L87](../src/apps/controller/ui/morph/MorphIconCore.h#L87) | `resample` | 声明 | `static std::vector<SampledPath> resample( const std::vector<CubicPath>& paths, int sampleCount = 64)` | 实现 resample 对应的业务或工具逻辑。 |
| [L90](../src/apps/controller/ui/morph/MorphIconCore.h#L90) | `buildPlan` | 声明 | `static MorphPlan buildPlan(const std::vector<SampledPath>& source, const std::vector<SampledPath>& target)` | 创建或初始化 build plan 相关逻辑。 |
| [L92](../src/apps/controller/ui/morph/MorphIconCore.h#L92) | `procrustes` | 声明 | `static Similarity procrustes(const std::vector<QPointF>& source, const std::vector<QPointF>& target, const QPointF& sourceCentroid, const QPointF& targetCentroid)` | 实现 procrustes 对应的业务或工具逻辑。 |
| [L104](../src/apps/controller/ui/morph/MorphIconCore.h#L104) | `configure` | 声明 | `void configure(double stiffness, double damping) noexcept` | 更新或应用 configure 相关逻辑。 |
| [L105](../src/apps/controller/ui/morph/MorphIconCore.h#L105) | `start` | 声明 | `void start() noexcept` | 启动 start 相关逻辑。 |
| [L106](../src/apps/controller/ui/morph/MorphIconCore.h#L106) | `step` | 声明 | `bool step(double seconds) noexcept` | 实现 step 对应的业务或工具逻辑。 |

## `src/apps/controller/ui/RemoteCTheme.cpp`

[打开源码](../src/apps/controller/ui/RemoteCTheme.cpp) · **文件作用：** 实现 remote c theme 相关函数与文件级辅助逻辑。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L21](../src/apps/controller/ui/RemoteCTheme.cpp#L21) | `kThemeIconPath` | `constexpr auto kThemeIconPath = "remoteCThemeIconPath";` | 定义 theme icon path 的编译期常量或产品边界。 |
| [L22](../src/apps/controller/ui/RemoteCTheme.cpp#L22) | `kThemeIconTone` | `constexpr auto kThemeIconTone = "remoteCThemeIconTone";` | 定义 theme icon tone 的编译期常量或产品边界。 |
| [L23](../src/apps/controller/ui/RemoteCTheme.cpp#L23) | `kThemeIconSize` | `constexpr auto kThemeIconSize = "remoteCThemeIconSize";` | 定义 theme icon size 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L26](../src/apps/controller/ui/RemoteCTheme.cpp#L26) | `RemoteCTheme::Color` | 定义 | `QColor RemoteCTheme::Color(ThemeColor color)` | 实现 color 对应的业务或工具逻辑。 |
| [L59](../src/apps/controller/ui/RemoteCTheme.cpp#L59) | `RemoteCTheme::LoadStyleSheet` | 定义 | `QString RemoteCTheme::LoadStyleSheet(const QString& resourcePath)` | 读取或恢复 load style sheet 相关逻辑。 |
| [L68](../src/apps/controller/ui/RemoteCTheme.cpp#L68) | `RemoteCTheme::LightStyleSheet` | 定义 | `QString RemoteCTheme::LightStyleSheet()` | 实现 light style sheet 对应的业务或工具逻辑。 |
| [L74](../src/apps/controller/ui/RemoteCTheme.cpp#L74) | `RemoteCTheme::DarkStyleSheet` | 定义 | `QString RemoteCTheme::DarkStyleSheet()` | 实现 dark style sheet 对应的业务或工具逻辑。 |
| [L80](../src/apps/controller/ui/RemoteCTheme.cpp#L80) | `RemoteCTheme::MainWindowColorOverrides` | 定义 | `QString RemoteCTheme::MainWindowColorOverrides(bool dark)` | 实现 main window color overrides 对应的业务或工具逻辑。 |
| [L87](../src/apps/controller/ui/RemoteCTheme.cpp#L87) | `RemoteCTheme::RemoteDarkStyleSheet` | 定义 | `QString RemoteCTheme::RemoteDarkStyleSheet()` | 实现 remote dark style sheet 对应的业务或工具逻辑。 |
| [L93](../src/apps/controller/ui/RemoteCTheme.cpp#L93) | `RemoteCTheme::PageStyleSheet` | 定义 | `QString RemoteCTheme::PageStyleSheet(const QString& resourcePath, bool dark)` | 实现 page style sheet 对应的业务或工具逻辑。 |
| [L102](../src/apps/controller/ui/RemoteCTheme.cpp#L102) | `RemoteCTheme::LoadPreference` | 定义 | `ThemePreference RemoteCTheme::LoadPreference()` | 读取或恢复 load preference 相关逻辑。 |
| [L109](../src/apps/controller/ui/RemoteCTheme.cpp#L109) | `RemoteCTheme::SavePreference` | 定义 | `void RemoteCTheme::SavePreference(ThemePreference preference)` | 保存或写入 save preference 相关逻辑。 |
| [L116](../src/apps/controller/ui/RemoteCTheme.cpp#L116) | `RemoteCTheme::PreferenceValue` | 定义 | `QString RemoteCTheme::PreferenceValue(ThemePreference preference)` | 实现 preference value 对应的业务或工具逻辑。 |
| [L126](../src/apps/controller/ui/RemoteCTheme.cpp#L126) | `RemoteCTheme::PreferenceFromValue` | 定义 | `ThemePreference RemoteCTheme::PreferenceFromValue(const QString& value)` | 实现 preference from value 对应的业务或工具逻辑。 |
| [L138](../src/apps/controller/ui/RemoteCTheme.cpp#L138) | `RemoteCTheme::IsDark` | 定义 | `bool RemoteCTheme::IsDark(ThemePreference preference)` | 判断 is dark 相关逻辑。 |
| [L149](../src/apps/controller/ui/RemoteCTheme.cpp#L149) | `RemoteCTheme::IconColor` | 定义 | `QColor RemoteCTheme::IconColor(ThemeIconTone tone, bool dark)` | 实现 icon color 对应的业务或工具逻辑。 |
| [L174](../src/apps/controller/ui/RemoteCTheme.cpp#L174) | `RemoteCTheme::Icon` | 定义 | `QIcon RemoteCTheme::Icon(const QString& resourcePath, ThemeIconTone tone, QSize logicalSize)` | 实现 icon 对应的业务或工具逻辑。 |
| [L193](../src/apps/controller/ui/RemoteCTheme.cpp#L193) | `RemoteCTheme::SetIcon` | 定义 | `void RemoteCTheme::SetIcon(QAbstractButton* button, const QString& resourcePath, ThemeIconTone tone)` | 更新或应用 set icon 相关逻辑。 |
| [L203](../src/apps/controller/ui/RemoteCTheme.cpp#L203) | `RemoteCTheme::SetIcon` | 定义 | `void RemoteCTheme::SetIcon(QAction* action, const QString& resourcePath, ThemeIconTone tone)` | 更新或应用 set icon 相关逻辑。 |
| [L212](../src/apps/controller/ui/RemoteCTheme.cpp#L212) | `RemoteCTheme::SetPixmap` | 定义 | `void RemoteCTheme::SetPixmap(QLabel* label, const QString& resourcePath, QSize logicalSize, ThemeIconTone tone)` | 更新或应用 set pixmap 相关逻辑。 |
| [L223](../src/apps/controller/ui/RemoteCTheme.cpp#L223) | `RemoteCTheme::RefreshIcons` | 定义 | `void RemoteCTheme::RefreshIcons(QWidget* root)` | 刷新 refresh icons 相关逻辑。 |
| [L250](../src/apps/controller/ui/RemoteCTheme.cpp#L250) | `RemoteCTheme::ApplyLight` | 定义 | `void RemoteCTheme::ApplyLight(QWidget* widget)` | 更新或应用 apply light 相关逻辑。 |
| [L257](../src/apps/controller/ui/RemoteCTheme.cpp#L257) | `RemoteCTheme::ApplyDark` | 定义 | `void RemoteCTheme::ApplyDark(QWidget* widget)` | 更新或应用 apply dark 相关逻辑。 |
| [L264](../src/apps/controller/ui/RemoteCTheme.cpp#L264) | `RemoteCTheme::ApplyRemoteDark` | 定义 | `void RemoteCTheme::ApplyRemoteDark(QWidget* widget)` | 更新或应用 apply remote dark 相关逻辑。 |

## `src/apps/controller/ui/RemoteCTheme.h`

[打开源码](../src/apps/controller/ui/RemoteCTheme.h) · **文件作用：** 声明 remote c theme 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L11](../src/apps/controller/ui/RemoteCTheme.h#L11) | `QAbstractButton` | class | 定义 QAbstractButton 的 class 类型和相关状态。 |
| [L12](../src/apps/controller/ui/RemoteCTheme.h#L12) | `QAction` | class | 定义 QAction 的 class 类型和相关状态。 |
| [L13](../src/apps/controller/ui/RemoteCTheme.h#L13) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L14](../src/apps/controller/ui/RemoteCTheme.h#L14) | `QWidget` | class | 定义 QWidget 的 class 类型和相关状态。 |
| [L18](../src/apps/controller/ui/RemoteCTheme.h#L18) | `ThemePreference` | enum class | 定义 ThemePreference 的 enum class 类型和相关状态。 |
| [L24](../src/apps/controller/ui/RemoteCTheme.h#L24) | `ThemeColor` | enum class | 定义 ThemeColor 的 enum class 类型和相关状态。 |
| [L53](../src/apps/controller/ui/RemoteCTheme.h#L53) | `ThemeIconTone` | enum class | 定义 ThemeIconTone 的 enum class 类型和相关状态。 |
| [L62](../src/apps/controller/ui/RemoteCTheme.h#L62) | `RemoteCTheme` | class | 定义 RemoteCTheme 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L11](../src/apps/controller/ui/RemoteCTheme.h#L11) | `QAbstractButton` | `class QAbstractButton;` | 保存 q abstract button 相关配置或运行状态。 |
| [L12](../src/apps/controller/ui/RemoteCTheme.h#L12) | `QAction` | `class QAction;` | 保存 q action 相关配置或运行状态。 |
| [L13](../src/apps/controller/ui/RemoteCTheme.h#L13) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L14](../src/apps/controller/ui/RemoteCTheme.h#L14) | `QWidget` | `class QWidget;` | 保存 q widget 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L64](../src/apps/controller/ui/RemoteCTheme.h#L64) | `Color` | 声明 | `static QColor Color(ThemeColor color)` | 实现 color 对应的业务或工具逻辑。 |
| [L65](../src/apps/controller/ui/RemoteCTheme.h#L65) | `LightStyleSheet` | 声明 | `static QString LightStyleSheet()` | 实现 light style sheet 对应的业务或工具逻辑。 |
| [L66](../src/apps/controller/ui/RemoteCTheme.h#L66) | `DarkStyleSheet` | 声明 | `static QString DarkStyleSheet()` | 实现 dark style sheet 对应的业务或工具逻辑。 |
| [L69](../src/apps/controller/ui/RemoteCTheme.h#L69) | `MainWindowColorOverrides` | 声明 | `static QString MainWindowColorOverrides(bool dark)` | Main-window geometry lives in kMainStyle. Theme switching may append only color overrides so a round trip can never change layout metrics. |
| [L70](../src/apps/controller/ui/RemoteCTheme.h#L70) | `RemoteDarkStyleSheet` | 声明 | `static QString RemoteDarkStyleSheet()` | 实现 remote dark style sheet 对应的业务或工具逻辑。 |
| [L71](../src/apps/controller/ui/RemoteCTheme.h#L71) | `PageStyleSheet` | 声明 | `static QString PageStyleSheet(const QString& resourcePath, bool dark)` | 实现 page style sheet 对应的业务或工具逻辑。 |
| [L72](../src/apps/controller/ui/RemoteCTheme.h#L72) | `LoadPreference` | 声明 | `static ThemePreference LoadPreference()` | 读取或恢复 load preference 相关逻辑。 |
| [L73](../src/apps/controller/ui/RemoteCTheme.h#L73) | `SavePreference` | 声明 | `static void SavePreference(ThemePreference preference)` | 保存或写入 save preference 相关逻辑。 |
| [L74](../src/apps/controller/ui/RemoteCTheme.h#L74) | `PreferenceValue` | 声明 | `static QString PreferenceValue(ThemePreference preference)` | 实现 preference value 对应的业务或工具逻辑。 |
| [L75](../src/apps/controller/ui/RemoteCTheme.h#L75) | `PreferenceFromValue` | 声明 | `static ThemePreference PreferenceFromValue(const QString& value)` | 实现 preference from value 对应的业务或工具逻辑。 |
| [L76](../src/apps/controller/ui/RemoteCTheme.h#L76) | `IsDark` | 声明 | `static bool IsDark(ThemePreference preference)` | 判断 is dark 相关逻辑。 |
| [L77](../src/apps/controller/ui/RemoteCTheme.h#L77) | `Icon` | 声明 | `static QIcon Icon(const QString& resourcePath, ThemeIconTone tone = ThemeIconTone::kNeutral, QSize logicalSize = QSize(24, 24))` | 实现 icon 对应的业务或工具逻辑。 |
| [L80](../src/apps/controller/ui/RemoteCTheme.h#L80) | `SetIcon` | 声明 | `static void SetIcon(QAbstractButton* button, const QString& resourcePath, ThemeIconTone tone = ThemeIconTone::kNeutral)` | 更新或应用 set icon 相关逻辑。 |
| [L82](../src/apps/controller/ui/RemoteCTheme.h#L82) | `SetIcon` | 声明 | `static void SetIcon(QAction* action, const QString& resourcePath, ThemeIconTone tone = ThemeIconTone::kNeutral)` | 更新或应用 set icon 相关逻辑。 |
| [L84](../src/apps/controller/ui/RemoteCTheme.h#L84) | `SetPixmap` | 声明 | `static void SetPixmap(QLabel* label, const QString& resourcePath, QSize logicalSize, ThemeIconTone tone = ThemeIconTone::kNeutral)` | 更新或应用 set pixmap 相关逻辑。 |
| [L87](../src/apps/controller/ui/RemoteCTheme.h#L87) | `RefreshIcons` | 声明 | `static void RefreshIcons(QWidget* root)` | 刷新 refresh icons 相关逻辑。 |
| [L88](../src/apps/controller/ui/RemoteCTheme.h#L88) | `ApplyLight` | 声明 | `static void ApplyLight(QWidget* widget)` | 更新或应用 apply light 相关逻辑。 |
| [L89](../src/apps/controller/ui/RemoteCTheme.h#L89) | `ApplyDark` | 声明 | `static void ApplyDark(QWidget* widget)` | 更新或应用 apply dark 相关逻辑。 |
| [L90](../src/apps/controller/ui/RemoteCTheme.h#L90) | `ApplyRemoteDark` | 声明 | `static void ApplyRemoteDark(QWidget* widget)` | 更新或应用 apply remote dark 相关逻辑。 |
| [L93](../src/apps/controller/ui/RemoteCTheme.h#L93) | `LoadStyleSheet` | 声明 | `static QString LoadStyleSheet(const QString& resourcePath)` | 读取或恢复 load style sheet 相关逻辑。 |
| [L94](../src/apps/controller/ui/RemoteCTheme.h#L94) | `IconColor` | 声明 | `static QColor IconColor(ThemeIconTone tone, bool dark)` | 实现 icon color 对应的业务或工具逻辑。 |
