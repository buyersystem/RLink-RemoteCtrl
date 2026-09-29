# 应用入口与软件更新

> 自动生成于 2026-09-28，源码树 `f12aea4209d9-dirty`。请运行 `tools/Generate-SourceSymbolReference.ps1` 刷新。

顶层应用入口、更新清单控制器与独立更新器。

本册共收录 5 个源码文件。函数与变量的中文作用优先采用源码紧邻注释；无注释时根据符号命名生成阅读提示，最终语义仍以源码为准。

## `src/apps/AgentHostMain.cpp`

[打开源码](../src/apps/AgentHostMain.cpp) · **文件作用：** 实现 agent host main 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L23](../src/apps/AgentHostMain.cpp#L23) | `ParsePort` | 定义 | `std::uint16_t ParsePort(int argc, char* argv[])` | 解码或解析 parse port 相关逻辑。 |
| [L48](../src/apps/AgentHostMain.cpp#L48) | `PrintFailure` | 定义 | `void PrintFailure(const std::string& stage, const std::string& detail)` | 实现 print failure 对应的业务或工具逻辑。 |
| [L59](../src/apps/AgentHostMain.cpp#L59) | `main` | 定义 | `int main(int argc, char* argv[])` | 实现 main 对应的业务或工具逻辑。 |

## `src/apps/ControllerAppMain.cpp`

[打开源码](../src/apps/ControllerAppMain.cpp) · **文件作用：** 实现 controller app main 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L21](../src/apps/ControllerAppMain.cpp#L21) | `ParsePort` | 定义 | `std::uint16_t ParsePort(int argc, char* argv[])` | 解码或解析 parse port 相关逻辑。 |
| [L45](../src/apps/ControllerAppMain.cpp#L45) | `PrintFailure` | 定义 | `void PrintFailure(const std::string& stage, const std::string& detail)` | 实现 print failure 对应的业务或工具逻辑。 |
| [L56](../src/apps/ControllerAppMain.cpp#L56) | `main` | 定义 | `int main(int argc, char* argv[])` | 实现 main 对应的业务或工具逻辑。 |

## `src/apps/update/RLinkUpdaterMain.cpp`

[打开源码](../src/apps/update/RLinkUpdaterMain.cpp) · **文件作用：** 实现 r link updater main 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L37](../src/apps/update/RLinkUpdaterMain.cpp#L37) | `Options` | struct | 定义 Options 的 struct 类型和相关状态。 |
| [L48](../src/apps/update/RLinkUpdaterMain.cpp#L48) | `UiState` | struct | 定义 UiState 的 struct 类型和相关状态。 |
| [L600](../src/apps/update/RLinkUpdaterMain.cpp#L600) | `ReplacedFile` | struct | 定义 ReplacedFile 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L35](../src/apps/update/RLinkUpdaterMain.cpp#L35) | `kStateChangedMessage` | `constexpr UINT kStateChangedMessage = WM_APP + 41;` | 定义 state changed message 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L60](../src/apps/update/RLinkUpdaterMain.cpp#L60) | `QuoteArgument` | 定义 | `std::wstring QuoteArgument(const std::wstring& value)` | 实现 quote argument 对应的业务或工具逻辑。 |
| [L103](../src/apps/update/RLinkUpdaterMain.cpp#L103) | `PublishFailure` | 定义 | `void PublishFailure(const std::wstring& detail)` | 发送或发布 publish failure 相关逻辑。 |
| [L117](../src/apps/update/RLinkUpdaterMain.cpp#L117) | `ParseOptions` | 定义 | `std::optional<Options> ParseOptions()` | 解码或解析 parse options 相关逻辑。 |
| [L162](../src/apps/update/RLinkUpdaterMain.cpp#L162) | `IsAllowedHttpsUrl` | 定义 | `bool IsAllowedHttpsUrl(const std::wstring& url)` | 判断 is allowed https url 相关逻辑。 |
| [L179](../src/apps/update/RLinkUpdaterMain.cpp#L179) | `IsAllowedPackageUrl` | 定义 | `bool IsAllowedPackageUrl(const std::wstring& url)` | 判断 is allowed package url 相关逻辑。 |
| [L194](../src/apps/update/RLinkUpdaterMain.cpp#L194) | `LastErrorText` | 定义 | `std::wstring LastErrorText(const wchar_t* operation)` | 实现 last error text 对应的业务或工具逻辑。 |
| [L200](../src/apps/update/RLinkUpdaterMain.cpp#L200) | `HashFileSha256` | 定义 | `bool HashFileSha256(const std::filesystem::path& path, std::wstring* hexDigest)` | 判断 hash file sha256 相关逻辑。 |
| [L263](../src/apps/update/RLinkUpdaterMain.cpp#L263) | `DownloadPackage` | 定义 | `bool DownloadPackage(const Options& options, const std::wstring& packageUrl, const std::filesystem::path& destination, std::wstring* error)` | 实现 download package 对应的业务或工具逻辑。 |
| [L408](../src/apps/update/RLinkUpdaterMain.cpp#L408) | `RunHiddenProcess` | 定义 | `bool RunHiddenProcess(const std::wstring& commandLine, DWORD timeoutMs, std::wstring* error)` | 执行后台循环或调度 run hidden process 相关逻辑。 |
| [L434](../src/apps/update/RLinkUpdaterMain.cpp#L434) | `RunHiddenProcessCapture` | 定义 | `bool RunHiddenProcessCapture(const std::wstring& commandLine, const std::filesystem::path& outputPath, DWORD timeoutMs, std::wstring* error)` | 执行后台循环或调度 run hidden process capture 相关逻辑。 |
| [L486](../src/apps/update/RLinkUpdaterMain.cpp#L486) | `IsRLinkInstallation` | 定义 | `bool IsRLinkInstallation(const std::filesystem::path& target)` | 判断 is r link installation 相关逻辑。 |
| [L506](../src/apps/update/RLinkUpdaterMain.cpp#L506) | `ValidateArchiveListing` | 定义 | `bool ValidateArchiveListing(const std::filesystem::path& tar, const std::filesystem::path& package, const std::filesystem::path& updateDirectory, std::wstring* error)` | 判断 validate archive listing 相关逻辑。 |
| [L548](../src/apps/update/RLinkUpdaterMain.cpp#L548) | `ExtractPackage` | 定义 | `bool ExtractPackage(const std::filesystem::path& package, const std::filesystem::path& staging, std::wstring* error)` | 实现 extract package 对应的业务或工具逻辑。 |
| [L585](../src/apps/update/RLinkUpdaterMain.cpp#L585) | `WaitForParent` | 定义 | `bool WaitForParent(DWORD processId, std::wstring* error)` | 实现 wait for parent 对应的业务或工具逻辑。 |
| [L605](../src/apps/update/RLinkUpdaterMain.cpp#L605) | `RollBackFiles` | 定义 | `void RollBackFiles(const std::filesystem::path& target, const std::filesystem::path& backup, const std::vector<ReplacedFile>& files)` | 实现 roll back files 对应的业务或工具逻辑。 |
| [L623](../src/apps/update/RLinkUpdaterMain.cpp#L623) | `ReplaceFiles` | 定义 | `bool ReplaceFiles(const std::filesystem::path& staging, const std::filesystem::path& target, const std::filesystem::path& backup, std::wstring* error)` | 实现 replace files 对应的业务或工具逻辑。 |
| [L695](../src/apps/update/RLinkUpdaterMain.cpp#L695) | `RestartApplication` | 定义 | `bool RestartApplication(const Options& options, std::wstring* error)` | 实现 restart application 对应的业务或工具逻辑。 |
| [L716](../src/apps/update/RLinkUpdaterMain.cpp#L716) | `ReadSmallFile` | 定义 | `std::string ReadSmallFile(const std::filesystem::path& path)` | 读取或恢复 read small file 相关逻辑。 |
| [L723](../src/apps/update/RLinkUpdaterMain.cpp#L723) | `RunSelfTest` | 定义 | `bool RunSelfTest()` | 执行后台循环或调度 run self test 相关逻辑。 |
| [L793](../src/apps/update/RLinkUpdaterMain.cpp#L793) | `RunUpdate` | 定义 | `void RunUpdate(Options options)` | 执行后台循环或调度 run update 相关逻辑。 |
| [L865](../src/apps/update/RLinkUpdaterMain.cpp#L865) | `DrawTextLine` | 定义 | `void DrawTextLine(HDC dc, const std::wstring& text, RECT bounds, HFONT font, COLORREF color, UINT format)` | 准备或呈现 draw text line 相关逻辑。 |
| [L875](../src/apps/update/RLinkUpdaterMain.cpp#L875) | `WindowProcedure` | 定义 | `LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam)` | 实现 window procedure 对应的业务或工具逻辑。 |
| [L992](../src/apps/update/RLinkUpdaterMain.cpp#L992) | `wWinMain` | 定义 | `int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)` | 实现 w win main 对应的业务或工具逻辑。 |

## `src/apps/update/SoftwareUpdateController.cpp`

[打开源码](../src/apps/update/SoftwareUpdateController.cpp) · **文件作用：** 实现 software update controller 相关函数与文件级辅助逻辑。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L42](../src/apps/update/SoftwareUpdateController.cpp#L42) | `kAutomaticCheckIntervalSeconds` | `constexpr qint64 kAutomaticCheckIntervalSeconds = 24 * 60 * 60;` | 定义 automatic check interval seconds 的编译期常量或产品边界。 |
| [L43](../src/apps/update/SoftwareUpdateController.cpp#L43) | `kMaximumManifestBytes` | `constexpr qint64 kMaximumManifestBytes = 256 * 1024;` | 定义 maximum manifest bytes 的编译期常量或产品边界。 |
| [L44](../src/apps/update/SoftwareUpdateController.cpp#L44) | `kMaximumManifestUrls` | `constexpr qsizetype kMaximumManifestUrls = 4;` | 定义 maximum manifest urls 的编译期常量或产品边界。 |
| [L45](../src/apps/update/SoftwareUpdateController.cpp#L45) | `kMaximumPackageUrls` | `constexpr qsizetype kMaximumPackageUrls = 8;` | 定义 maximum package urls 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L47](../src/apps/update/SoftwareUpdateController.cpp#L47) | `IsCleanHttpsUrl` | 定义 | `bool IsCleanHttpsUrl(const QUrl& url)` | 判断 is clean https url 相关逻辑。 |
| [L56](../src/apps/update/SoftwareUpdateController.cpp#L56) | `IsAllowedRepositoryUrl` | 定义 | `bool IsAllowedRepositoryUrl(const QUrl& url)` | 判断 is allowed repository url 相关逻辑。 |
| [L71](../src/apps/update/SoftwareUpdateController.cpp#L71) | `IsAllowedManifestUrl` | 定义 | `bool IsAllowedManifestUrl(const QUrl& url)` | 判断 is allowed manifest url 相关逻辑。 |
| [L79](../src/apps/update/SoftwareUpdateController.cpp#L79) | `IsAllowedManifestResponseUrl` | 定义 | `bool IsAllowedManifestResponseUrl(const QUrl& url)` | 判断 is allowed manifest response url 相关逻辑。 |
| [L86](../src/apps/update/SoftwareUpdateController.cpp#L86) | `IsAllowedPackageUrl` | 定义 | `bool IsAllowedPackageUrl(const QUrl& url)` | 判断 is allowed package url 相关逻辑。 |
| [L91](../src/apps/update/SoftwareUpdateController.cpp#L91) | `AppendUniqueUrl` | 定义 | `void AppendUniqueUrl(QList<QUrl>* urls, const QUrl& url)` | 实现 append unique url 对应的业务或工具逻辑。 |
| [L102](../src/apps/update/SoftwareUpdateController.cpp#L102) | `LoadManifestUrls` | 定义 | `QList<QUrl> LoadManifestUrls()` | 读取或恢复 load manifest urls 相关逻辑。 |
| [L135](../src/apps/update/SoftwareUpdateController.cpp#L135) | `ParseVersion` | 定义 | `QVector<int> ParseVersion(const QString& version)` | 解码或解析 parse version 相关逻辑。 |
| [L156](../src/apps/update/SoftwareUpdateController.cpp#L156) | `CompareVersions` | 定义 | `int CompareVersions(const QString& left, const QString& right)` | 实现 compare versions 对应的业务或工具逻辑。 |
| [L171](../src/apps/update/SoftwareUpdateController.cpp#L171) | `NormalizedSha256` | 定义 | `QString NormalizedSha256(const QString& value)` | 实现 normalized sha256 对应的业务或工具逻辑。 |
| [L180](../src/apps/update/SoftwareUpdateController.cpp#L180) | `QuoteWindowsArgument` | 定义 | `QString QuoteWindowsArgument(const QString& value)` | 实现 quote windows argument 对应的业务或工具逻辑。 |
| [L212](../src/apps/update/SoftwareUpdateController.cpp#L212) | `SoftwareUpdateController::SoftwareUpdateController` | 定义 | `SoftwareUpdateController::SoftwareUpdateController(QObject* parent) : QObject(parent), network_(new QNetworkAccessManager(this)), manifestUrls_(LoadManifestUrls())` | 构造并初始化 SoftwareUpdateController 实例。 |
| [L221](../src/apps/update/SoftwareUpdateController.cpp#L221) | `SoftwareUpdateController::SetStateChangedCallback` | 定义 | `void SoftwareUpdateController::SetStateChangedCallback( StateChangedCallback callback)` | 更新或应用 set state changed callback 相关逻辑。 |
| [L228](../src/apps/update/SoftwareUpdateController.cpp#L228) | `SoftwareUpdateController::ScheduleAutomaticCheck` | 定义 | `void SoftwareUpdateController::ScheduleAutomaticCheck()` | 执行后台循环或调度 schedule automatic check 相关逻辑。 |
| [L244](../src/apps/update/SoftwareUpdateController.cpp#L244) | `SoftwareUpdateController::CheckForUpdates` | 定义 | `void SoftwareUpdateController::CheckForUpdates(bool manualRequest)` | 校验 check for updates 相关逻辑。 |
| [L284](../src/apps/update/SoftwareUpdateController.cpp#L284) | `SoftwareUpdateController::StartNextManifestRequest` | 定义 | `void SoftwareUpdateController::StartNextManifestRequest()` | 启动 start next manifest request 相关逻辑。 |
| [L320](../src/apps/update/SoftwareUpdateController.cpp#L320) | `SoftwareUpdateController::FailCurrentManifest` | 定义 | `void SoftwareUpdateController::FailCurrentManifest(const QString& message)` | 实现 fail current manifest 对应的业务或工具逻辑。 |
| [L327](../src/apps/update/SoftwareUpdateController.cpp#L327) | `SoftwareUpdateController::FinishCheck` | 定义 | `void SoftwareUpdateController::FinishCheck(QNetworkReply* reply)` | 停止 finish check 相关逻辑。 |
| [L454](../src/apps/update/SoftwareUpdateController.cpp#L454) | `SoftwareUpdateController::LaunchUpdater` | 定义 | `bool SoftwareUpdateController::LaunchUpdater(QString* errorMessage)` | 启动 launch updater 相关逻辑。 |
| [L554](../src/apps/update/SoftwareUpdateController.cpp#L554) | `SoftwareUpdateController::PublishSnapshot` | 定义 | `void SoftwareUpdateController::PublishSnapshot()` | 发送或发布 publish snapshot 相关逻辑。 |
| [L561](../src/apps/update/SoftwareUpdateController.cpp#L561) | `SoftwareUpdateController::SetFailure` | 定义 | `void SoftwareUpdateController::SetFailure( const QString& message, bool manualRequest)` | 更新或应用 set failure 相关逻辑。 |

## `src/apps/update/SoftwareUpdateController.h`

[打开源码](../src/apps/update/SoftwareUpdateController.h) · **文件作用：** 声明 software update controller 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L15](../src/apps/update/SoftwareUpdateController.h#L15) | `QNetworkAccessManager` | class | 定义 QNetworkAccessManager 的 class 类型和相关状态。 |
| [L16](../src/apps/update/SoftwareUpdateController.h#L16) | `QNetworkReply` | class | 定义 QNetworkReply 的 class 类型和相关状态。 |
| [L20](../src/apps/update/SoftwareUpdateController.h#L20) | `SoftwareUpdateController` | class | 定义 SoftwareUpdateController 的 class 类型和相关状态。 |
| [L22](../src/apps/update/SoftwareUpdateController.h#L22) | `State` | enum class | 定义 State 的 enum class 类型和相关状态。 |
| [L31](../src/apps/update/SoftwareUpdateController.h#L31) | `Snapshot` | struct | 定义 Snapshot 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L15](../src/apps/update/SoftwareUpdateController.h#L15) | `QNetworkAccessManager` | `class QNetworkAccessManager;` | 保存 q network access manager 相关配置或运行状态。 |
| [L16](../src/apps/update/SoftwareUpdateController.h#L16) | `QNetworkReply` | `class QNetworkReply;` | 保存 q network reply 相关配置或运行状态。 |
| [L32](../src/apps/update/SoftwareUpdateController.h#L32) | `state` | `State state = State::kIdle;` | 保存状态机当前状态：state。 |
| [L33](../src/apps/update/SoftwareUpdateController.h#L33) | `installedVersion` | `QString installedVersion;` | 保存 installed version 相关配置或运行状态。 |
| [L34](../src/apps/update/SoftwareUpdateController.h#L34) | `latestVersion` | `QString latestVersion;` | 保存 latest version 相关配置或运行状态。 |
| [L35](../src/apps/update/SoftwareUpdateController.h#L35) | `minimumSupportedVersion` | `QString minimumSupportedVersion;` | 保存 minimum supported version 相关配置或运行状态。 |
| [L36](../src/apps/update/SoftwareUpdateController.h#L36) | `summary` | `QString summary;` | 保存 summary 相关配置或运行状态。 |
| [L37](../src/apps/update/SoftwareUpdateController.h#L37) | `releaseNotes` | `QStringList releaseNotes;` | 保存 release notes 相关配置或运行状态。 |
| [L38](../src/apps/update/SoftwareUpdateController.h#L38) | `errorMessage` | `QString errorMessage;` | 保存 error message 相关配置或运行状态。 |
| [L39](../src/apps/update/SoftwareUpdateController.h#L39) | `releaseNotesUrl` | `QUrl releaseNotesUrl;` | 保存路径、地址或显示名称：release notes url。 |
| [L40](../src/apps/update/SoftwareUpdateController.h#L40) | `manualRequest` | `bool manualRequest = false;` | 保存 manual request 相关配置或运行状态。 |
| [L41](../src/apps/update/SoftwareUpdateController.h#L41) | `mandatory` | `bool mandatory = false;` | 保存 mandatory 相关配置或运行状态。 |
| [L62](../src/apps/update/SoftwareUpdateController.h#L62) | `network_` | `QNetworkAccessManager* network_ = nullptr;` | 保存 network 相关配置或运行状态。 |
| [L63](../src/apps/update/SoftwareUpdateController.h#L63) | `activeReply_` | `QPointer<QNetworkReply> activeReply_;` | 保存 active reply 相关配置或运行状态。 |
| [L64](../src/apps/update/SoftwareUpdateController.h#L64) | `responseBody_` | `QByteArray responseBody_;` | 保存 response body 相关配置或运行状态。 |
| [L65](../src/apps/update/SoftwareUpdateController.h#L65) | `snapshot_` | `Snapshot snapshot_;` | 保存可跨层读取的状态快照：snapshot。 |
| [L66](../src/apps/update/SoftwareUpdateController.h#L66) | `stateChangedCallback_` | `StateChangedCallback stateChangedCallback_;` | 保存回调或观察者入口：state changed callback。 |
| [L67](../src/apps/update/SoftwareUpdateController.h#L67) | `manifestUrls_` | `QList<QUrl> manifestUrls_;` | 保存 manifest urls 相关配置或运行状态。 |
| [L68](../src/apps/update/SoftwareUpdateController.h#L68) | `packageUrls_` | `QList<QUrl> packageUrls_;` | 保存 package urls 相关配置或运行状态。 |
| [L69](../src/apps/update/SoftwareUpdateController.h#L69) | `activeManifestUrl_` | `QUrl activeManifestUrl_;` | 保存路径、地址或显示名称：active manifest url。 |
| [L70](../src/apps/update/SoftwareUpdateController.h#L70) | `nextManifestIndex_` | `qsizetype nextManifestIndex_ = 0;` | 保存 next manifest index 相关配置或运行状态。 |
| [L71](../src/apps/update/SoftwareUpdateController.h#L71) | `manifestErrors_` | `QStringList manifestErrors_;` | 保存 manifest errors 相关配置或运行状态。 |
| [L72](../src/apps/update/SoftwareUpdateController.h#L72) | `packageSha256_` | `QString packageSha256_;` | 保存 package sha256 相关配置或运行状态。 |
| [L73](../src/apps/update/SoftwareUpdateController.h#L73) | `packageSize_` | `qint64 packageSize_ = -1;` | 保存计数、尺寸或速率指标：package size。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L46](../src/apps/update/SoftwareUpdateController.h#L46) | `SoftwareUpdateController` | 声明 | `explicit SoftwareUpdateController(QObject* parent = nullptr)` | 实现 software update controller 对应的业务或工具逻辑。 |
| [L48](../src/apps/update/SoftwareUpdateController.h#L48) | `SetStateChangedCallback` | 声明 | `void SetStateChangedCallback(StateChangedCallback callback)` | 更新或应用 set state changed callback 相关逻辑。 |
| [L49](../src/apps/update/SoftwareUpdateController.h#L49) | `ScheduleAutomaticCheck` | 声明 | `void ScheduleAutomaticCheck()` | 执行后台循环或调度 schedule automatic check 相关逻辑。 |
| [L50](../src/apps/update/SoftwareUpdateController.h#L50) | `CheckForUpdates` | 声明 | `void CheckForUpdates(bool manualRequest)` | 校验 check for updates 相关逻辑。 |
| [L51](../src/apps/update/SoftwareUpdateController.h#L51) | `LaunchUpdater` | 声明 | `bool LaunchUpdater(QString* errorMessage = nullptr)` | 启动 launch updater 相关逻辑。 |
| [L53](../src/apps/update/SoftwareUpdateController.h#L53) | `CurrentSnapshot` | 定义 | `const Snapshot& CurrentSnapshot() const { return snapshot_; }` | 实现 current snapshot 对应的业务或工具逻辑。 |
| [L56](../src/apps/update/SoftwareUpdateController.h#L56) | `PublishSnapshot` | 声明 | `void PublishSnapshot()` | 发送或发布 publish snapshot 相关逻辑。 |
| [L57](../src/apps/update/SoftwareUpdateController.h#L57) | `StartNextManifestRequest` | 声明 | `void StartNextManifestRequest()` | 启动 start next manifest request 相关逻辑。 |
| [L58](../src/apps/update/SoftwareUpdateController.h#L58) | `FinishCheck` | 声明 | `void FinishCheck(QNetworkReply* reply)` | 停止 finish check 相关逻辑。 |
| [L59](../src/apps/update/SoftwareUpdateController.h#L59) | `FailCurrentManifest` | 声明 | `void FailCurrentManifest(const QString& message)` | 实现 fail current manifest 对应的业务或工具逻辑。 |
| [L60](../src/apps/update/SoftwareUpdateController.h#L60) | `SetFailure` | 声明 | `void SetFailure(const QString& message, bool manualRequest)` | 更新或应用 set failure 相关逻辑。 |
