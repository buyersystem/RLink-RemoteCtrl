# 信令服务器

> 自动生成于 2026-09-28，源码树 `f12aea4209d9-dirty`。请运行 `tools/Generate-SourceSymbolReference.ps1` 刷新。

WSS 认证、设备注册、direct session、协作房间、持久化、限流和诊断。

本册共收录 34 个源码文件。函数与变量的中文作用优先采用源码紧邻注释；无注释时根据符号命名生成阅读提示，最终语义仍以源码为准。

## `src/server/auth/AuthTypes.h`

[打开源码](../src/server/auth/AuthTypes.h) · **文件作用：** 声明 auth types 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L10](../src/server/auth/AuthTypes.h#L10) | `UserInfoClaims` | struct | 定义 UserInfoClaims 的 struct 类型和相关状态。 |
| [L17](../src/server/auth/AuthTypes.h#L17) | `UserInfoStatus` | enum class | 定义 UserInfoStatus 的 enum class 类型和相关状态。 |
| [L25](../src/server/auth/AuthTypes.h#L25) | `UserInfoResult` | struct | 定义 UserInfoResult 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L11](../src/server/auth/AuthTypes.h#L11) | `subject` | `QString subject;` | 保存 subject 相关配置或运行状态。 |
| [L12](../src/server/auth/AuthTypes.h#L12) | `username` | `QString username;` | 保存路径、地址或显示名称：username。 |
| [L13](../src/server/auth/AuthTypes.h#L13) | `displayName` | `QString displayName;` | 保存路径、地址或显示名称：display name。 |
| [L14](../src/server/auth/AuthTypes.h#L14) | `email` | `QString email;` | 保存 email 相关配置或运行状态。 |
| [L26](../src/server/auth/AuthTypes.h#L26) | `status` | `UserInfoStatus status = UserInfoStatus::kInvalidResponse;` | 保存状态机当前状态：status。 |
| [L27](../src/server/auth/AuthTypes.h#L27) | `claims` | `UserInfoClaims claims;` | 保存 claims 相关配置或运行状态。 |
| [L28](../src/server/auth/AuthTypes.h#L28) | `code` | `QString code;` | 保存 code 相关配置或运行状态。 |
| [L29](../src/server/auth/AuthTypes.h#L29) | `message` | `QString message;` | 保存 message 相关配置或运行状态。 |
| [L30](../src/server/auth/AuthTypes.h#L30) | `retryable` | `bool retryable = false;` | 保存 retryable 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L32](../src/server/auth/AuthTypes.h#L32) | `authenticated` | 定义 | `bool authenticated() const` | 实现 authenticated 对应的业务或工具逻辑。 |

## `src/server/auth/LogtoManagementClient.cpp`

[打开源码](../src/server/auth/LogtoManagementClient.cpp) · **文件作用：** 实现 logto management client 相关函数与文件级辅助逻辑。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L18](../src/server/auth/LogtoManagementClient.cpp#L18) | `kMaximumResponseBytes` | `constexpr qsizetype kMaximumResponseBytes = 64 * 1024;` | 定义 maximum response bytes 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L20](../src/server/auth/LogtoManagementClient.cpp#L20) | `SetError` | 定义 | `void SetError(QString* errorMessage, const QString& message)` | 更新或应用 set error 相关逻辑。 |
| [L26](../src/server/auth/LogtoManagementClient.cpp#L26) | `Failure` | 定义 | `UserDeletionResult Failure(UserDeletionStatus status, QString code, QString message, bool retryable)` | 实现 failure 对应的业务或工具逻辑。 |
| [L40](../src/server/auth/LogtoManagementClient.cpp#L40) | `LogtoManagementClient::LogtoManagementClient` | 定义 | `LogtoManagementClient::LogtoManagementClient(QObject* parent) : QObject(parent), networkManager_(std::make_unique<QNetworkAccessManager>(this)) {}` | 构造并初始化 LogtoManagementClient 实例。 |
| [L44](../src/server/auth/LogtoManagementClient.cpp#L44) | `LogtoManagementClient::~LogtoManagementClient` | 定义 | `LogtoManagementClient::~LogtoManagementClient()` | 停止相关活动并释放 LogtoManagementClient 实例拥有的资源。 |
| [L49](../src/server/auth/LogtoManagementClient.cpp#L49) | `LogtoManagementClient::Configure` | 定义 | `bool LogtoManagementClient::Configure( const QUrl& issuer, QString clientId, QByteArray clientSecret, int timeoutMs, QString* errorMessage)` | 更新或应用 configure 相关逻辑。 |
| [L83](../src/server/auth/LogtoManagementClient.cpp#L83) | `LogtoManagementClient::IsConfigured` | 定义 | `bool LogtoManagementClient::IsConfigured() const` | 判断 is configured 相关逻辑。 |
| [L88](../src/server/auth/LogtoManagementClient.cpp#L88) | `LogtoManagementClient::DeleteUser` | 定义 | `void LogtoManagementClient::DeleteUser( const QString& subject, Completion completion)` | 实现 delete user 对应的业务或工具逻辑。 |
| [L107](../src/server/auth/LogtoManagementClient.cpp#L107) | `LogtoManagementClient::CancelAll` | 定义 | `void LogtoManagementClient::CancelAll()` | 判断 cancel all 相关逻辑。 |
| [L132](../src/server/auth/LogtoManagementClient.cpp#L132) | `LogtoManagementClient::StartNext` | 定义 | `void LogtoManagementClient::StartNext()` | 启动 start next 相关逻辑。 |
| [L145](../src/server/auth/LogtoManagementClient.cpp#L145) | `LogtoManagementClient::RequestAccessToken` | 定义 | `void LogtoManagementClient::RequestAccessToken()` | 发起请求或查询 request access token 相关逻辑。 |
| [L170](../src/server/auth/LogtoManagementClient.cpp#L170) | `LogtoManagementClient::FinishTokenRequest` | 定义 | `void LogtoManagementClient::FinishTokenRequest(QNetworkReply* reply)` | 停止 finish token request 相关逻辑。 |
| [L215](../src/server/auth/LogtoManagementClient.cpp#L215) | `LogtoManagementClient::StartDeleteRequest` | 定义 | `void LogtoManagementClient::StartDeleteRequest()` | 启动 start delete request 相关逻辑。 |
| [L233](../src/server/auth/LogtoManagementClient.cpp#L233) | `LogtoManagementClient::FinishDeleteRequest` | 定义 | `void LogtoManagementClient::FinishDeleteRequest(QNetworkReply* reply)` | 停止 finish delete request 相关逻辑。 |
| [L279](../src/server/auth/LogtoManagementClient.cpp#L279) | `LogtoManagementClient::CompleteCurrent` | 定义 | `void LogtoManagementClient::CompleteCurrent(UserDeletionResult result)` | 实现 complete current 对应的业务或工具逻辑。 |

## `src/server/auth/LogtoManagementClient.h`

[打开源码](../src/server/auth/LogtoManagementClient.h) · **文件作用：** 声明 logto management client 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L17](../src/server/auth/LogtoManagementClient.h#L17) | `QNetworkAccessManager` | class | 定义 QNetworkAccessManager 的 class 类型和相关状态。 |
| [L22](../src/server/auth/LogtoManagementClient.h#L22) | `UserDeletionStatus` | enum class | 定义 UserDeletionStatus 的 enum class 类型和相关状态。 |
| [L31](../src/server/auth/LogtoManagementClient.h#L31) | `UserDeletionResult` | struct | 定义 UserDeletionResult 的 struct 类型和相关状态。 |
| [L43](../src/server/auth/LogtoManagementClient.h#L43) | `LogtoManagementClient` | class | 定义 LogtoManagementClient 的 class 类型和相关状态。 |
| [L60](../src/server/auth/LogtoManagementClient.h#L60) | `PendingDeletion` | struct | 定义 PendingDeletion 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L17](../src/server/auth/LogtoManagementClient.h#L17) | `QNetworkAccessManager` | `class QNetworkAccessManager;` | 保存 q network access manager 相关配置或运行状态。 |
| [L32](../src/server/auth/LogtoManagementClient.h#L32) | `status` | `UserDeletionStatus status = UserDeletionStatus::kRejected;` | 保存状态机当前状态：status。 |
| [L33](../src/server/auth/LogtoManagementClient.h#L33) | `code` | `QString code;` | 保存 code 相关配置或运行状态。 |
| [L34](../src/server/auth/LogtoManagementClient.h#L34) | `message` | `QString message;` | 保存 message 相关配置或运行状态。 |
| [L35](../src/server/auth/LogtoManagementClient.h#L35) | `retryable` | `bool retryable = false;` | 保存 retryable 相关配置或运行状态。 |
| [L61](../src/server/auth/LogtoManagementClient.h#L61) | `subject` | `QString subject;` | 保存 subject 相关配置或运行状态。 |
| [L62](../src/server/auth/LogtoManagementClient.h#L62) | `completion` | `Completion completion;` | 保存 completion 相关配置或运行状态。 |
| [L72](../src/server/auth/LogtoManagementClient.h#L72) | `networkManager_` | `std::unique_ptr<QNetworkAccessManager> networkManager_;` | 保存 network manager 相关配置或运行状态。 |
| [L73](../src/server/auth/LogtoManagementClient.h#L73) | `queue_` | `QQueue<PendingDeletion> queue_;` | 保存待处理队列或请求：queue。 |
| [L74](../src/server/auth/LogtoManagementClient.h#L74) | `current_` | `std::unique_ptr<PendingDeletion> current_;` | 保存 current 相关配置或运行状态。 |
| [L75](../src/server/auth/LogtoManagementClient.h#L75) | `activeReply_` | `QNetworkReply* activeReply_ = nullptr;` | 保存 active reply 相关配置或运行状态。 |
| [L76](../src/server/auth/LogtoManagementClient.h#L76) | `tokenEndpoint_` | `QUrl tokenEndpoint_;` | 保存 token endpoint 相关配置或运行状态。 |
| [L77](../src/server/auth/LogtoManagementClient.h#L77) | `managementEndpoint_` | `QUrl managementEndpoint_;` | 保存 management endpoint 相关配置或运行状态。 |
| [L78](../src/server/auth/LogtoManagementClient.h#L78) | `clientId_` | `QString clientId_;` | 保存身份或作用域标识：client id。 |
| [L79](../src/server/auth/LogtoManagementClient.h#L79) | `clientSecret_` | `QByteArray clientSecret_;` | 保存 client secret 相关配置或运行状态。 |
| [L80](../src/server/auth/LogtoManagementClient.h#L80) | `accessToken_` | `QByteArray accessToken_;` | 保存 access token 相关配置或运行状态。 |
| [L81](../src/server/auth/LogtoManagementClient.h#L81) | `accessTokenExpiresAtSeconds_` | `qint64 accessTokenExpiresAtSeconds_ = 0;` | 保存 access token expires at seconds 相关配置或运行状态。 |
| [L82](../src/server/auth/LogtoManagementClient.h#L82) | `timeoutMs_` | `int timeoutMs_ = 10000;` | 保存 timeout ms 相关配置或运行状态。 |
| [L83](../src/server/auth/LogtoManagementClient.h#L83) | `cancelling_` | `bool cancelling_ = false;` | 保存 cancelling 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L37](../src/server/auth/LogtoManagementClient.h#L37) | `deleted` | 定义 | `bool deleted() const` | 实现 deleted 对应的业务或工具逻辑。 |
| [L47](../src/server/auth/LogtoManagementClient.h#L47) | `LogtoManagementClient` | 声明 | `explicit LogtoManagementClient(QObject* parent = nullptr)` | 实现 logto management client 对应的业务或工具逻辑。 |
| [L48](../src/server/auth/LogtoManagementClient.h#L48) | `~LogtoManagementClient` | 声明 | `~LogtoManagementClient() override` | 停止相关活动并释放 LogtoManagementClient 实例拥有的资源。 |
| [L50](../src/server/auth/LogtoManagementClient.h#L50) | `Configure` | 声明 | `bool Configure(const QUrl& issuer, QString clientId, QByteArray clientSecret, int timeoutMs, QString* errorMessage = nullptr)` | 更新或应用 configure 相关逻辑。 |
| [L55](../src/server/auth/LogtoManagementClient.h#L55) | `IsConfigured` | 声明 | `bool IsConfigured() const` | 判断 is configured 相关逻辑。 |
| [L56](../src/server/auth/LogtoManagementClient.h#L56) | `DeleteUser` | 声明 | `void DeleteUser(const QString& subject, Completion completion)` | 实现 delete user 对应的业务或工具逻辑。 |
| [L57](../src/server/auth/LogtoManagementClient.h#L57) | `CancelAll` | 声明 | `void CancelAll()` | 判断 cancel all 相关逻辑。 |
| [L65](../src/server/auth/LogtoManagementClient.h#L65) | `StartNext` | 声明 | `void StartNext()` | 启动 start next 相关逻辑。 |
| [L66](../src/server/auth/LogtoManagementClient.h#L66) | `RequestAccessToken` | 声明 | `void RequestAccessToken()` | 发起请求或查询 request access token 相关逻辑。 |
| [L67](../src/server/auth/LogtoManagementClient.h#L67) | `FinishTokenRequest` | 声明 | `void FinishTokenRequest(QNetworkReply* reply)` | 停止 finish token request 相关逻辑。 |
| [L68](../src/server/auth/LogtoManagementClient.h#L68) | `StartDeleteRequest` | 声明 | `void StartDeleteRequest()` | 启动 start delete request 相关逻辑。 |
| [L69](../src/server/auth/LogtoManagementClient.h#L69) | `FinishDeleteRequest` | 声明 | `void FinishDeleteRequest(QNetworkReply* reply)` | 停止 finish delete request 相关逻辑。 |
| [L70](../src/server/auth/LogtoManagementClient.h#L70) | `CompleteCurrent` | 声明 | `void CompleteCurrent(UserDeletionResult result)` | 实现 complete current 对应的业务或工具逻辑。 |

## `src/server/auth/LogtoUserInfoClient.cpp`

[打开源码](../src/server/auth/LogtoUserInfoClient.cpp) · **文件作用：** 实现 logto user info client 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L39](../src/server/auth/LogtoUserInfoClient.cpp#L39) | `LogtoUserInfoClient::PendingRequest` | struct | 定义 LogtoUserInfoClient::PendingRequest 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L17](../src/server/auth/LogtoUserInfoClient.cpp#L17) | `kMaximumUserInfoBytes` | `constexpr qsizetype kMaximumUserInfoBytes = 64 * 1024;` | 定义 maximum user info bytes 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L19](../src/server/auth/LogtoUserInfoClient.cpp#L19) | `SetError` | 定义 | `void SetError(QString* errorMessage, const QString& message)` | 更新或应用 set error 相关逻辑。 |
| [L25](../src/server/auth/LogtoUserInfoClient.cpp#L25) | `Failure` | 定义 | `UserInfoResult Failure(UserInfoStatus status, QString code, QString message, bool retryable)` | 实现 failure 对应的业务或工具逻辑。 |
| [L47](../src/server/auth/LogtoUserInfoClient.cpp#L47) | `LogtoUserInfoClient::LogtoUserInfoClient` | 定义 | `LogtoUserInfoClient::LogtoUserInfoClient(QObject* parent) : QObject(parent), networkManager_(std::make_unique<QNetworkAccessManager>(this)) {}` | 构造并初始化 LogtoUserInfoClient 实例。 |
| [L51](../src/server/auth/LogtoUserInfoClient.cpp#L51) | `LogtoUserInfoClient::~LogtoUserInfoClient` | 定义 | `LogtoUserInfoClient::~LogtoUserInfoClient()` | 停止相关活动并释放 LogtoUserInfoClient 实例拥有的资源。 |
| [L55](../src/server/auth/LogtoUserInfoClient.cpp#L55) | `LogtoUserInfoClient::Configure` | 定义 | `bool LogtoUserInfoClient::Configure( const QUrl& issuer, int timeoutMs, QString* errorMessage)` | 更新或应用 configure 相关逻辑。 |
| [L96](../src/server/auth/LogtoUserInfoClient.cpp#L96) | `LogtoUserInfoClient::Fetch` | 定义 | `QNetworkReply* LogtoUserInfoClient::Fetch( const QByteArray& accessToken, Completion completion)` | 实现 fetch 对应的业务或工具逻辑。 |
| [L153](../src/server/auth/LogtoUserInfoClient.cpp#L153) | `LogtoUserInfoClient::Cancel` | 定义 | `void LogtoUserInfoClient::Cancel(QNetworkReply* reply)` | 判断 cancel 相关逻辑。 |
| [L163](../src/server/auth/LogtoUserInfoClient.cpp#L163) | `LogtoUserInfoClient::CancelAll` | 定义 | `void LogtoUserInfoClient::CancelAll()` | 判断 cancel all 相关逻辑。 |
| [L170](../src/server/auth/LogtoUserInfoClient.cpp#L170) | `LogtoUserInfoClient::ParseResponseForTesting` | 定义 | `UserInfoResult LogtoUserInfoClient::ParseResponseForTesting( int httpStatus, const QByteArray& body, QNetworkReply::NetworkError networkError, bool timedOut, bool tooLarge)` | 解码或解析 parse response for testing 相关逻辑。 |
| [L180](../src/server/auth/LogtoUserInfoClient.cpp#L180) | `LogtoUserInfoClient::ParseResponse` | 定义 | `UserInfoResult LogtoUserInfoClient::ParseResponse( int httpStatus, const QByteArray& body, QNetworkReply::NetworkError networkError, bool timedOut, bool tooLarge)` | 解码或解析 parse response 相关逻辑。 |
| [L262](../src/server/auth/LogtoUserInfoClient.cpp#L262) | `LogtoUserInfoClient::Finish` | 定义 | `void LogtoUserInfoClient::Finish(QNetworkReply* reply)` | 停止 finish 相关逻辑。 |

## `src/server/auth/LogtoUserInfoClient.h`

[打开源码](../src/server/auth/LogtoUserInfoClient.h) · **文件作用：** 声明 logto user info client 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L18](../src/server/auth/LogtoUserInfoClient.h#L18) | `QNetworkAccessManager` | class | 定义 QNetworkAccessManager 的 class 类型和相关状态。 |
| [L23](../src/server/auth/LogtoUserInfoClient.h#L23) | `LogtoUserInfoClient` | class | 定义 LogtoUserInfoClient 的 class 类型和相关状态。 |
| [L49](../src/server/auth/LogtoUserInfoClient.h#L49) | `PendingRequest` | struct | 定义 PendingRequest 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L18](../src/server/auth/LogtoUserInfoClient.h#L18) | `QNetworkAccessManager` | `class QNetworkAccessManager;` | 保存 q network access manager 相关配置或运行状态。 |
| [L49](../src/server/auth/LogtoUserInfoClient.h#L49) | `PendingRequest` | `struct PendingRequest;` | 保存 pending request 相关配置或运行状态。 |
| [L59](../src/server/auth/LogtoUserInfoClient.h#L59) | `networkManager_` | `std::unique_ptr<QNetworkAccessManager> networkManager_;` | 保存 network manager 相关配置或运行状态。 |
| [L60](../src/server/auth/LogtoUserInfoClient.h#L60) | `pending_` | `QHash<QNetworkReply*, std::shared_ptr<PendingRequest>> pending_;` | 保存待处理队列或请求：pending。 |
| [L61](../src/server/auth/LogtoUserInfoClient.h#L61) | `userInfoEndpoint_` | `QUrl userInfoEndpoint_;` | 保存 user info endpoint 相关配置或运行状态。 |
| [L62](../src/server/auth/LogtoUserInfoClient.h#L62) | `timeoutMs_` | `int timeoutMs_ = 10000;` | 保存 timeout ms 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L27](../src/server/auth/LogtoUserInfoClient.h#L27) | `LogtoUserInfoClient` | 声明 | `explicit LogtoUserInfoClient(QObject* parent = nullptr)` | 实现 logto user info client 对应的业务或工具逻辑。 |
| [L28](../src/server/auth/LogtoUserInfoClient.h#L28) | `~LogtoUserInfoClient` | 声明 | `~LogtoUserInfoClient() override` | 停止相关活动并释放 LogtoUserInfoClient 实例拥有的资源。 |
| [L30](../src/server/auth/LogtoUserInfoClient.h#L30) | `Configure` | 声明 | `bool Configure(const QUrl& issuer, int timeoutMs, QString* errorMessage = nullptr)` | 更新或应用 configure 相关逻辑。 |
| [L33](../src/server/auth/LogtoUserInfoClient.h#L33) | `Fetch` | 声明 | `QNetworkReply* Fetch(const QByteArray& accessToken, Completion completion)` | 实现 fetch 对应的业务或工具逻辑。 |
| [L35](../src/server/auth/LogtoUserInfoClient.h#L35) | `Cancel` | 声明 | `void Cancel(QNetworkReply* reply)` | 判断 cancel 相关逻辑。 |
| [L36](../src/server/auth/LogtoUserInfoClient.h#L36) | `CancelAll` | 声明 | `void CancelAll()` | 判断 cancel all 相关逻辑。 |
| [L38](../src/server/auth/LogtoUserInfoClient.h#L38) | `userInfoEndpoint` | 定义 | `QUrl userInfoEndpoint() const { return userInfoEndpoint_; }` | 实现 user info endpoint 对应的业务或工具逻辑。 |
| [L40](../src/server/auth/LogtoUserInfoClient.h#L40) | `ParseResponseForTesting` | 声明 | `static UserInfoResult ParseResponseForTesting( int httpStatus, const QByteArray& body, QNetworkReply::NetworkError networkError = QNetworkReply::NoError, bool timedOut = false, bool tooLarge = false)` | 解码或解析 parse response for testing 相关逻辑。 |
| [L51](../src/server/auth/LogtoUserInfoClient.h#L51) | `ParseResponse` | 声明 | `static UserInfoResult ParseResponse( int httpStatus, const QByteArray& body, QNetworkReply::NetworkError networkError, bool timedOut, bool tooLarge)` | 解码或解析 parse response 相关逻辑。 |
| [L57](../src/server/auth/LogtoUserInfoClient.h#L57) | `Finish` | 声明 | `void Finish(QNetworkReply* reply)` | 停止 finish 相关逻辑。 |

## `src/server/auth/LogtoWebhookServer.cpp`

[打开源码](../src/server/auth/LogtoWebhookServer.cpp) · **文件作用：** 实现 logto webhook server 相关函数与文件级辅助逻辑。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L14](../src/server/auth/LogtoWebhookServer.cpp#L14) | `kMaximumWebhookBytes` | `constexpr qsizetype kMaximumWebhookBytes = 64 * 1024;` | 定义 maximum webhook bytes 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L16](../src/server/auth/LogtoWebhookServer.cpp#L16) | `SetError` | 定义 | `void SetError(QString* errorMessage, const QString& message)` | 更新或应用 set error 相关逻辑。 |
| [L22](../src/server/auth/LogtoWebhookServer.cpp#L22) | `JsonResponse` | 定义 | `QHttpServerResponse JsonResponse( QHttpServerResponse::StatusCode status, const QString& code)` | 实现 json response 对应的业务或工具逻辑。 |
| [L34](../src/server/auth/LogtoWebhookServer.cpp#L34) | `LogtoWebhookServer::LogtoWebhookServer` | 定义 | `LogtoWebhookServer::LogtoWebhookServer()` | 构造并初始化 LogtoWebhookServer 实例。 |
| [L44](../src/server/auth/LogtoWebhookServer.cpp#L44) | `LogtoWebhookServer::~LogtoWebhookServer` | 定义 | `LogtoWebhookServer::~LogtoWebhookServer()` | 停止相关活动并释放 LogtoWebhookServer 实例拥有的资源。 |
| [L48](../src/server/auth/LogtoWebhookServer.cpp#L48) | `LogtoWebhookServer::Start` | 定义 | `bool LogtoWebhookServer::Start( const QHostAddress& listenAddress, quint16 port, QByteArray signingKey, UserDeletedHandler handler, QString* errorMessage)` | 启动 start 相关逻辑。 |
| [L85](../src/server/auth/LogtoWebhookServer.cpp#L85) | `LogtoWebhookServer::Stop` | 定义 | `void LogtoWebhookServer::Stop()` | 停止 stop 相关逻辑。 |
| [L92](../src/server/auth/LogtoWebhookServer.cpp#L92) | `LogtoWebhookServer::IsListening` | 定义 | `bool LogtoWebhookServer::IsListening() const` | 判断 is listening 相关逻辑。 |
| [L96](../src/server/auth/LogtoWebhookServer.cpp#L96) | `LogtoWebhookServer::ServerPort` | 定义 | `quint16 LogtoWebhookServer::ServerPort() const` | 实现 server port 对应的业务或工具逻辑。 |
| [L100](../src/server/auth/LogtoWebhookServer.cpp#L100) | `LogtoWebhookServer::ConstantTimeEquals` | 定义 | `bool LogtoWebhookServer::ConstantTimeEquals( const QByteArray& first, const QByteArray& second)` | 实现 constant time equals 对应的业务或工具逻辑。 |
| [L114](../src/server/auth/LogtoWebhookServer.cpp#L114) | `LogtoWebhookServer::VerifySignatureForTesting` | 定义 | `bool LogtoWebhookServer::VerifySignatureForTesting( const QByteArray& body, const QByteArray& signingKey, const QByteArray& signature)` | 校验 verify signature for testing 相关逻辑。 |
| [L127](../src/server/auth/LogtoWebhookServer.cpp#L127) | `LogtoWebhookServer::HandleRequest` | 定义 | `QHttpServerResponse LogtoWebhookServer::HandleRequest( const QHttpServerRequest& request)` | 接收并处理 handle request 相关逻辑。 |

## `src/server/auth/LogtoWebhookServer.h`

[打开源码](../src/server/auth/LogtoWebhookServer.h) · **文件作用：** 声明 logto webhook server 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L15](../src/server/auth/LogtoWebhookServer.h#L15) | `LogtoWebhookServer` | class | 定义 LogtoWebhookServer 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L44](../src/server/auth/LogtoWebhookServer.h#L44) | `tcpServer_` | `QTcpServer tcpServer_;` | 保存 tcp server 相关配置或运行状态。 |
| [L45](../src/server/auth/LogtoWebhookServer.h#L45) | `httpServer_` | `QHttpServer httpServer_;` | 保存 http server 相关配置或运行状态。 |
| [L46](../src/server/auth/LogtoWebhookServer.h#L46) | `signingKey_` | `QByteArray signingKey_;` | 保存 signing key 相关配置或运行状态。 |
| [L47](../src/server/auth/LogtoWebhookServer.h#L47) | `userDeletedHandler_` | `UserDeletedHandler userDeletedHandler_;` | 保存回调或观察者入口：user deleted handler。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L20](../src/server/auth/LogtoWebhookServer.h#L20) | `LogtoWebhookServer` | 声明 | `LogtoWebhookServer()` | 实现 logto webhook server 对应的业务或工具逻辑。 |
| [L21](../src/server/auth/LogtoWebhookServer.h#L21) | `~LogtoWebhookServer` | 声明 | `~LogtoWebhookServer()` | 停止相关活动并释放 LogtoWebhookServer 实例拥有的资源。 |
| [L23](../src/server/auth/LogtoWebhookServer.h#L23) | `LogtoWebhookServer` | 声明 | `LogtoWebhookServer(const LogtoWebhookServer&) = delete` | 实现 logto webhook server 对应的业务或工具逻辑。 |
| [L26](../src/server/auth/LogtoWebhookServer.h#L26) | `Start` | 声明 | `bool Start(const QHostAddress& listenAddress, quint16 port, QByteArray signingKey, UserDeletedHandler handler, QString* errorMessage = nullptr)` | 启动 start 相关逻辑。 |
| [L31](../src/server/auth/LogtoWebhookServer.h#L31) | `Stop` | 声明 | `void Stop()` | 停止 stop 相关逻辑。 |
| [L32](../src/server/auth/LogtoWebhookServer.h#L32) | `IsListening` | 声明 | `bool IsListening() const` | 判断 is listening 相关逻辑。 |
| [L33](../src/server/auth/LogtoWebhookServer.h#L33) | `ServerPort` | 声明 | `quint16 ServerPort() const` | 实现 server port 对应的业务或工具逻辑。 |
| [L35](../src/server/auth/LogtoWebhookServer.h#L35) | `VerifySignatureForTesting` | 声明 | `static bool VerifySignatureForTesting(const QByteArray& body, const QByteArray& signingKey, const QByteArray& signature)` | 校验 verify signature for testing 相关逻辑。 |
| [L40](../src/server/auth/LogtoWebhookServer.h#L40) | `HandleRequest` | 声明 | `QHttpServerResponse HandleRequest(const QHttpServerRequest& request)` | 接收并处理 handle request 相关逻辑。 |
| [L41](../src/server/auth/LogtoWebhookServer.h#L41) | `ConstantTimeEquals` | 声明 | `static bool ConstantTimeEquals(const QByteArray& first, const QByteArray& second)` | 实现 constant time equals 对应的业务或工具逻辑。 |

## `src/server/persistence/IdentityStore.cpp`

[打开源码](../src/server/persistence/IdentityStore.cpp) · **文件作用：** 实现 identity store 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L24](../src/server/persistence/IdentityStore.cpp#L24) | `SetError` | 定义 | `void SetError(QString* errorMessage, const QString& message)` | 更新或应用 set error 相关逻辑。 |
| [L30](../src/server/persistence/IdentityStore.cpp#L30) | `QueryError` | 定义 | `QString QueryError(const QString& operation, const QSqlQuery& query)` | 发起请求或查询 query error 相关逻辑。 |
| [L35](../src/server/persistence/IdentityStore.cpp#L35) | `IsNineDigitPublicCode` | 定义 | `bool IsNineDigitPublicCode(const QString& value)` | 判断 is nine digit public code 相关逻辑。 |
| [L41](../src/server/persistence/IdentityStore.cpp#L41) | `AllocatePublicCode` | 定义 | `QString AllocatePublicCode(QSqlDatabase database, QString* errorMessage)` | 实现 allocate public code 对应的业务或工具逻辑。 |
| [L69](../src/server/persistence/IdentityStore.cpp#L69) | `DeviceTableSql` | 定义 | `QString DeviceTableSql(const QString& tableName)` | 实现 device table sql 对应的业务或工具逻辑。 |
| [L84](../src/server/persistence/IdentityStore.cpp#L84) | `HasUniqueDeviceIndex` | 定义 | `bool HasUniqueDeviceIndex(QSqlDatabase database, const QStringList& expectedColumns, bool* found, QString* errorMessage)` | 判断 has unique device index 相关逻辑。 |
| [L125](../src/server/persistence/IdentityStore.cpp#L125) | `HasStableInstallationDeviceSchema` | 定义 | `bool HasStableInstallationDeviceSchema(QSqlDatabase database, bool* found, QString* errorMessage)` | 判断 has stable installation device schema 相关逻辑。 |
| [L170](../src/server/persistence/IdentityStore.cpp#L170) | `RebuildDeviceTableForStableInstallation` | 定义 | `bool RebuildDeviceTableForStableInstallation(QSqlDatabase database, QString* errorMessage)` | 更新或应用 rebuild device table for stable installation 相关逻辑。 |
| [L208](../src/server/persistence/IdentityStore.cpp#L208) | `IdentityStore::IdentityStore` | 定义 | `IdentityStore::IdentityStore() : connectionName_(QStringLiteral("remotec-identity-%1").arg( QUuid::createUuid().toString(QUuid::WithoutBraces))) {}` | 构造并初始化 IdentityStore 实例。 |
| [L212](../src/server/persistence/IdentityStore.cpp#L212) | `IdentityStore::~IdentityStore` | 定义 | `IdentityStore::~IdentityStore()` | 停止相关活动并释放 IdentityStore 实例拥有的资源。 |
| [L216](../src/server/persistence/IdentityStore.cpp#L216) | `IdentityStore::Open` | 定义 | `bool IdentityStore::Open(const QString& databaseFile, QString* errorMessage)` | 启动 open 相关逻辑。 |
| [L270](../src/server/persistence/IdentityStore.cpp#L270) | `IdentityStore::Close` | 定义 | `void IdentityStore::Close()` | 关闭并清理 close 相关逻辑。 |
| [L283](../src/server/persistence/IdentityStore.cpp#L283) | `IdentityStore::IsOpen` | 定义 | `bool IdentityStore::IsOpen() const` | 判断 is open 相关逻辑。 |
| [L290](../src/server/persistence/IdentityStore.cpp#L290) | `IdentityStore::EnsureSchema` | 定义 | `bool IdentityStore::EnsureSchema(QString* errorMessage)` | 实现 ensure schema 对应的业务或工具逻辑。 |
| [L472](../src/server/persistence/IdentityStore.cpp#L472) | `IdentityStore::UpsertUser` | 定义 | `RemoteUser IdentityStore::UpsertUser( const remote::server_auth::UserInfoClaims& claims, QString* errorMessage)` | 实现 upsert user 对应的业务或工具逻辑。 |
| [L526](../src/server/persistence/IdentityStore.cpp#L526) | `IdentityStore::RegisterDevice` | 定义 | `DeviceRegistrationResult IdentityStore::RegisterDevice( qint64 ownerUserId, const QString& installationId, const QString& deviceName)` | 实现 register device 对应的业务或工具逻辑。 |
| [L606](../src/server/persistence/IdentityStore.cpp#L606) | `IdentityStore::ListDevicesForUser` | 定义 | `QList<OwnedDeviceRecord> IdentityStore::ListDevicesForUser( qint64 ownerUserId, QString* errorMessage) const` | 实现 list devices for user 对应的业务或工具逻辑。 |
| [L645](../src/server/persistence/IdentityStore.cpp#L645) | `IdentityStore::DeleteUserByLogtoSubject` | 定义 | `bool IdentityStore::DeleteUserByLogtoSubject( const QString& logtoSubject, bool* existed, QString* errorMessage)` | 实现 delete user by logto subject 对应的业务或工具逻辑。 |

## `src/server/persistence/IdentityStore.h`

[打开源码](../src/server/persistence/IdentityStore.h) · **文件作用：** 声明 identity store 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L12](../src/server/persistence/IdentityStore.h#L12) | `QSqlDatabase` | class | 定义 QSqlDatabase 的 class 类型和相关状态。 |
| [L17](../src/server/persistence/IdentityStore.h#L17) | `RemoteUser` | struct | 定义 RemoteUser 的 struct 类型和相关状态。 |
| [L27](../src/server/persistence/IdentityStore.h#L27) | `DeviceRegistrationStatus` | enum class | 定义 DeviceRegistrationStatus 的 enum class 类型和相关状态。 |
| [L33](../src/server/persistence/IdentityStore.h#L33) | `DeviceRegistrationResult` | struct | 定义 DeviceRegistrationResult 的 struct 类型和相关状态。 |
| [L43](../src/server/persistence/IdentityStore.h#L43) | `OwnedDeviceRecord` | struct | 定义 OwnedDeviceRecord 的 struct 类型和相关状态。 |
| [L53](../src/server/persistence/IdentityStore.h#L53) | `IdentityStore` | class | 定义 IdentityStore 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L12](../src/server/persistence/IdentityStore.h#L12) | `QSqlDatabase` | `class QSqlDatabase;` | 保存 q sql database 相关配置或运行状态。 |
| [L18](../src/server/persistence/IdentityStore.h#L18) | `id` | `qint64 id = 0;` | 保存身份或作用域标识：id。 |
| [L19](../src/server/persistence/IdentityStore.h#L19) | `logtoSubject` | `QString logtoSubject;` | 保存 logto subject 相关配置或运行状态。 |
| [L20](../src/server/persistence/IdentityStore.h#L20) | `username` | `QString username;` | 保存路径、地址或显示名称：username。 |
| [L21](../src/server/persistence/IdentityStore.h#L21) | `displayName` | `QString displayName;` | 保存路径、地址或显示名称：display name。 |
| [L22](../src/server/persistence/IdentityStore.h#L22) | `email` | `QString email;` | 保存 email 相关配置或运行状态。 |
| [L34](../src/server/persistence/IdentityStore.h#L34) | `status` | `DeviceRegistrationStatus status = DeviceRegistrationStatus::kStoreError;` | 保存状态机当前状态：status。 |
| [L35](../src/server/persistence/IdentityStore.h#L35) | `publicDeviceId` | `QString publicDeviceId;` | 保存身份或作用域标识：public device id。 |
| [L36](../src/server/persistence/IdentityStore.h#L36) | `errorMessage` | `QString errorMessage;` | 保存 error message 相关配置或运行状态。 |
| [L44](../src/server/persistence/IdentityStore.h#L44) | `publicDeviceId` | `QString publicDeviceId;` | 保存身份或作用域标识：public device id。 |
| [L45](../src/server/persistence/IdentityStore.h#L45) | `installationId` | `QString installationId;` | 保存身份或作用域标识：installation id。 |
| [L46](../src/server/persistence/IdentityStore.h#L46) | `ownerUserId` | `qint64 ownerUserId = 0;` | 保存身份或作用域标识：owner user id。 |
| [L47](../src/server/persistence/IdentityStore.h#L47) | `deviceName` | `QString deviceName;` | 保存路径、地址或显示名称：device name。 |
| [L48](../src/server/persistence/IdentityStore.h#L48) | `createdAt` | `qint64 createdAt = 0;` | 保存 created at 相关配置或运行状态。 |
| [L49](../src/server/persistence/IdentityStore.h#L49) | `lastSeenAt` | `qint64 lastSeenAt = 0;` | 保存 last seen at 相关配置或运行状态。 |
| [L50](../src/server/persistence/IdentityStore.h#L50) | `revoked` | `bool revoked = false;` | 保存 revoked 相关配置或运行状态。 |
| [L83](../src/server/persistence/IdentityStore.h#L83) | `connectionName_` | `QString connectionName_;` | 保存路径、地址或显示名称：connection name。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L24](../src/server/persistence/IdentityStore.h#L24) | `isValid` | 定义 | `bool isValid() const { return id > 0 && !logtoSubject.isEmpty(); }` | 判断 is valid 相关逻辑。 |
| [L38](../src/server/persistence/IdentityStore.h#L38) | `registered` | 定义 | `bool registered() const` | 实现 registered 对应的业务或工具逻辑。 |
| [L55](../src/server/persistence/IdentityStore.h#L55) | `IdentityStore` | 声明 | `IdentityStore()` | 实现 identity store 对应的业务或工具逻辑。 |
| [L56](../src/server/persistence/IdentityStore.h#L56) | `~IdentityStore` | 声明 | `~IdentityStore()` | 停止相关活动并释放 IdentityStore 实例拥有的资源。 |
| [L58](../src/server/persistence/IdentityStore.h#L58) | `IdentityStore` | 声明 | `IdentityStore(const IdentityStore&) = delete` | 实现 identity store 对应的业务或工具逻辑。 |
| [L61](../src/server/persistence/IdentityStore.h#L61) | `Open` | 声明 | `bool Open(const QString& databaseFile, QString* errorMessage = nullptr)` | 启动 open 相关逻辑。 |
| [L62](../src/server/persistence/IdentityStore.h#L62) | `Close` | 声明 | `void Close()` | 关闭并清理 close 相关逻辑。 |
| [L63](../src/server/persistence/IdentityStore.h#L63) | `IsOpen` | 声明 | `bool IsOpen() const` | 判断 is open 相关逻辑。 |
| [L65](../src/server/persistence/IdentityStore.h#L65) | `UpsertUser` | 声明 | `RemoteUser UpsertUser( const remote::server_auth::UserInfoClaims& claims, QString* errorMessage = nullptr)` | 实现 upsert user 对应的业务或工具逻辑。 |
| [L68](../src/server/persistence/IdentityStore.h#L68) | `RegisterDevice` | 声明 | `DeviceRegistrationResult RegisterDevice( qint64 ownerUserId, const QString& installationId, const QString& deviceName)` | 实现 register device 对应的业务或工具逻辑。 |
| [L72](../src/server/persistence/IdentityStore.h#L72) | `ListDevicesForUser` | 声明 | `QList<OwnedDeviceRecord> ListDevicesForUser( qint64 ownerUserId, QString* errorMessage = nullptr) const` | 实现 list devices for user 对应的业务或工具逻辑。 |
| [L75](../src/server/persistence/IdentityStore.h#L75) | `DeleteUserByLogtoSubject` | 声明 | `bool DeleteUserByLogtoSubject( const QString& logtoSubject, bool* existed = nullptr, QString* errorMessage = nullptr)` | 实现 delete user by logto subject 对应的业务或工具逻辑。 |
| [L81](../src/server/persistence/IdentityStore.h#L81) | `EnsureSchema` | 声明 | `bool EnsureSchema(QString* errorMessage)` | 实现 ensure schema 对应的业务或工具逻辑。 |

## `src/server/signaling/AccessTokenService.cpp`

[打开源码](../src/server/signaling/AccessTokenService.cpp) · **文件作用：** 实现 access token service 相关函数与文件级辅助逻辑。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L18](../src/server/signaling/AccessTokenService.cpp#L18) | `kMaximumTokenLifetimeSeconds` | `constexpr qint64 kMaximumTokenLifetimeSeconds = 7 * 24 * 60 * 60;` | 定义 maximum token lifetime seconds 的编译期常量或产品边界。 |
| [L19](../src/server/signaling/AccessTokenService.cpp#L19) | `kClockSkewSeconds` | `constexpr qint64 kClockSkewSeconds = 60;` | 定义 clock skew seconds 的编译期常量或产品边界。 |
| [L20](../src/server/signaling/AccessTokenService.cpp#L20) | `kTokenAudience` | `constexpr auto kTokenAudience = "remotec-signaling";` | 定义 token audience 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L22](../src/server/signaling/AccessTokenService.cpp#L22) | `Base64UrlEncode` | 定义 | `QByteArray Base64UrlEncode(const QByteArray& value)` | 实现 base64 url encode 对应的业务或工具逻辑。 |
| [L28](../src/server/signaling/AccessTokenService.cpp#L28) | `Base64UrlDecode` | 定义 | `QByteArray Base64UrlDecode(const QByteArray& value)` | 实现 base64 url decode 对应的业务或工具逻辑。 |
| [L35](../src/server/signaling/AccessTokenService.cpp#L35) | `ConstantTimeEqual` | 定义 | `bool ConstantTimeEqual(const QByteArray& left, const QByteArray& right)` | 实现 constant time equal 对应的业务或工具逻辑。 |
| [L48](../src/server/signaling/AccessTokenService.cpp#L48) | `SetError` | 定义 | `void SetError(QString* error, const QString& message)` | 更新或应用 set error 相关逻辑。 |
| [L57](../src/server/signaling/AccessTokenService.cpp#L57) | `AccessTokenService::AccessTokenService` | 定义 | `AccessTokenService::AccessTokenService(QByteArray secret) : secret_(std::move(secret)) {}` | 构造并初始化 AccessTokenService 实例。 |
| [L61](../src/server/signaling/AccessTokenService.cpp#L61) | `AccessTokenService::IsConfigured` | 定义 | `bool AccessTokenService::IsConfigured() const` | 判断 is configured 相关逻辑。 |
| [L66](../src/server/signaling/AccessTokenService.cpp#L66) | `AccessTokenService::Issue` | 定义 | `QByteArray AccessTokenService::Issue(const QString& deviceId, qint64 lifetimeSeconds, QString* error) const` | 判断 issue 相关逻辑。 |
| [L103](../src/server/signaling/AccessTokenService.cpp#L103) | `AccessTokenService::Verify` | 定义 | `bool AccessTokenService::Verify(const QByteArray& token, AccessTokenClaims* claims, QString* error) const` | 校验 verify 相关逻辑。 |
| [L167](../src/server/signaling/AccessTokenService.cpp#L167) | `AccessTokenService::IsValidDeviceId` | 定义 | `bool AccessTokenService::IsValidDeviceId(const QString& deviceId)` | 判断 is valid device id 相关逻辑。 |
| [L174](../src/server/signaling/AccessTokenService.cpp#L174) | `AccessTokenService::Sign` | 定义 | `QByteArray AccessTokenService::Sign(const QByteArray& encodedPayload) const` | 实现 sign 对应的业务或工具逻辑。 |

## `src/server/signaling/AccessTokenService.h`

[打开源码](../src/server/signaling/AccessTokenService.h) · **文件作用：** 声明 access token service 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L11](../src/server/signaling/AccessTokenService.h#L11) | `AccessTokenClaims` | struct | 定义 AccessTokenClaims 的 struct 类型和相关状态。 |
| [L18](../src/server/signaling/AccessTokenService.h#L18) | `AccessTokenService` | class | 定义 AccessTokenService 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L12](../src/server/signaling/AccessTokenService.h#L12) | `deviceId` | `QString deviceId;` | 保存身份或作用域标识：device id。 |
| [L13](../src/server/signaling/AccessTokenService.h#L13) | `tokenId` | `QString tokenId;` | 保存身份或作用域标识：token id。 |
| [L14](../src/server/signaling/AccessTokenService.h#L14) | `issuedAtSeconds` | `qint64 issuedAtSeconds = 0;` | 保存 issued at seconds 相关配置或运行状态。 |
| [L15](../src/server/signaling/AccessTokenService.h#L15) | `expiresAtSeconds` | `qint64 expiresAtSeconds = 0;` | 保存 expires at seconds 相关配置或运行状态。 |
| [L36](../src/server/signaling/AccessTokenService.h#L36) | `secret_` | `QByteArray secret_;` | 保存 secret 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L20](../src/server/signaling/AccessTokenService.h#L20) | `AccessTokenService` | 声明 | `explicit AccessTokenService(QByteArray secret)` | 实现 access token service 对应的业务或工具逻辑。 |
| [L22](../src/server/signaling/AccessTokenService.h#L22) | `IsConfigured` | 声明 | `bool IsConfigured() const` | 判断 is configured 相关逻辑。 |
| [L24](../src/server/signaling/AccessTokenService.h#L24) | `Issue` | 声明 | `QByteArray Issue(const QString& deviceId, qint64 lifetimeSeconds, QString* error) const` | 判断 issue 相关逻辑。 |
| [L27](../src/server/signaling/AccessTokenService.h#L27) | `Verify` | 声明 | `bool Verify(const QByteArray& token, AccessTokenClaims* claims, QString* error) const` | 校验 verify 相关逻辑。 |
| [L31](../src/server/signaling/AccessTokenService.h#L31) | `IsValidDeviceId` | 声明 | `static bool IsValidDeviceId(const QString& deviceId)` | 判断 is valid device id 相关逻辑。 |
| [L34](../src/server/signaling/AccessTokenService.h#L34) | `Sign` | 声明 | `QByteArray Sign(const QByteArray& encodedPayload) const` | 实现 sign 对应的业务或工具逻辑。 |

## `src/server/signaling/DirectSessionRegistry.cpp`

[打开源码](../src/server/signaling/DirectSessionRegistry.cpp) · **文件作用：** 实现 direct session registry 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L10](../src/server/signaling/DirectSessionRegistry.cpp#L10) | `DirectSessionRegistry::Reserve` | 定义 | `void DirectSessionRegistry::Reserve(qsizetype capacity)` | 实现 reserve 对应的业务或工具逻辑。 |
| [L16](../src/server/signaling/DirectSessionRegistry.cpp#L16) | `DirectSessionRegistry::HasDeviceSession` | 定义 | `bool DirectSessionRegistry::HasDeviceSession(const QString& deviceId) const` | 判断 has device session 相关逻辑。 |
| [L21](../src/server/signaling/DirectSessionRegistry.cpp#L21) | `DirectSessionRegistry::SessionIdForDevice` | 定义 | `QString DirectSessionRegistry::SessionIdForDevice( const QString& deviceId) const` | 实现 session id for device 对应的业务或工具逻辑。 |
| [L27](../src/server/signaling/DirectSessionRegistry.cpp#L27) | `DirectSessionRegistry::Insert` | 定义 | `bool DirectSessionRegistry::Insert(DirectSessionState session)` | 实现 insert 对应的业务或工具逻辑。 |
| [L46](../src/server/signaling/DirectSessionRegistry.cpp#L46) | `DirectSessionRegistry::Remove` | 定义 | `void DirectSessionRegistry::Remove(const QString& sessionId)` | 重置或移除 remove 相关逻辑。 |
| [L63](../src/server/signaling/DirectSessionRegistry.cpp#L63) | `DirectSessionRegistry::Clear` | 定义 | `void DirectSessionRegistry::Clear()` | 重置或移除 clear 相关逻辑。 |
| [L74](../src/server/signaling/DirectSessionRegistry.cpp#L74) | `DirectSessionRegistry::Sessions` | 定义 | `DirectSessionRegistry::SessionMap& DirectSessionRegistry::Sessions()` | 实现 sessions 对应的业务或工具逻辑。 |
| [L80](../src/server/signaling/DirectSessionRegistry.cpp#L80) | `DirectSessionRegistry::Sessions` | 定义 | `DirectSessionRegistry::Sessions() const` | 实现 sessions 对应的业务或工具逻辑。 |
| [L85](../src/server/signaling/DirectSessionRegistry.cpp#L85) | `RunDirectSessionRegistrySelfTest` | 定义 | `bool RunDirectSessionRegistrySelfTest(QString* errorMessage)` | 执行后台循环或调度 run direct session registry self test 相关逻辑。 |

## `src/server/signaling/DirectSessionRegistry.h`

[打开源码](../src/server/signaling/DirectSessionRegistry.h) · **文件作用：** 声明 direct session registry 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L12](../src/server/signaling/DirectSessionRegistry.h#L12) | `DirectSessionState` | struct | 定义 DirectSessionState 的 struct 类型和相关状态。 |
| [L13](../src/server/signaling/DirectSessionRegistry.h#L13) | `Phase` | enum class | 定义 Phase 的 enum class 类型和相关状态。 |
| [L31](../src/server/signaling/DirectSessionRegistry.h#L31) | `DirectSessionRegistry` | class | 定义 DirectSessionRegistry 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L18](../src/server/signaling/DirectSessionRegistry.h#L18) | `sessionId` | `QString sessionId;` | 保存身份或作用域标识：session id。 |
| [L19](../src/server/signaling/DirectSessionRegistry.h#L19) | `requesterDeviceId` | `QString requesterDeviceId;` | 保存身份或作用域标识：requester device id。 |
| [L20](../src/server/signaling/DirectSessionRegistry.h#L20) | `targetDeviceId` | `QString targetDeviceId;` | 保存身份或作用域标识：target device id。 |
| [L21](../src/server/signaling/DirectSessionRegistry.h#L21) | `purpose` | `QString purpose;` | 保存 purpose 相关配置或运行状态。 |
| [L22](../src/server/signaling/DirectSessionRegistry.h#L22) | `permissions` | `QJsonArray permissions;` | 保存 permissions 相关配置或运行状态。 |
| [L23](../src/server/signaling/DirectSessionRegistry.h#L23) | `phase` | `Phase phase = Phase::kPending;` | 保存状态机当前状态：phase。 |
| [L24](../src/server/signaling/DirectSessionRegistry.h#L24) | `requesterRecoveryToken` | `QString requesterRecoveryToken;` | 保存 requester recovery token 相关配置或运行状态。 |
| [L25](../src/server/signaling/DirectSessionRegistry.h#L25) | `targetRecoveryToken` | `QString targetRecoveryToken;` | 保存 target recovery token 相关配置或运行状态。 |
| [L26](../src/server/signaling/DirectSessionRegistry.h#L26) | `requesterDisconnectedAtMs` | `qint64 requesterDisconnectedAtMs = 0;` | 保存 requester disconnected at ms 相关配置或运行状态。 |
| [L27](../src/server/signaling/DirectSessionRegistry.h#L27) | `targetDisconnectedAtMs` | `qint64 targetDisconnectedAtMs = 0;` | 保存 target disconnected at ms 相关配置或运行状态。 |
| [L28](../src/server/signaling/DirectSessionRegistry.h#L28) | `pendingExpiresAtMs` | `qint64 pendingExpiresAtMs = 0;` | 保存 pending expires at ms 相关配置或运行状态。 |
| [L47](../src/server/signaling/DirectSessionRegistry.h#L47) | `sessions_` | `SessionMap sessions_;` | 保存 sessions 相关配置或运行状态。 |
| [L48](../src/server/signaling/DirectSessionRegistry.h#L48) | `deviceSessions_` | `QHash<QString, QString> deviceSessions_;` | 保存 device sessions 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L35](../src/server/signaling/DirectSessionRegistry.h#L35) | `Reserve` | 声明 | `void Reserve(qsizetype capacity)` | 实现 reserve 对应的业务或工具逻辑。 |
| [L36](../src/server/signaling/DirectSessionRegistry.h#L36) | `HasDeviceSession` | 声明 | `bool HasDeviceSession(const QString& deviceId) const` | 判断 has device session 相关逻辑。 |
| [L37](../src/server/signaling/DirectSessionRegistry.h#L37) | `SessionIdForDevice` | 声明 | `QString SessionIdForDevice(const QString& deviceId) const` | 实现 session id for device 对应的业务或工具逻辑。 |
| [L38](../src/server/signaling/DirectSessionRegistry.h#L38) | `Insert` | 声明 | `bool Insert(DirectSessionState session)` | 实现 insert 对应的业务或工具逻辑。 |
| [L39](../src/server/signaling/DirectSessionRegistry.h#L39) | `Remove` | 声明 | `void Remove(const QString& sessionId)` | 重置或移除 remove 相关逻辑。 |
| [L40](../src/server/signaling/DirectSessionRegistry.h#L40) | `Clear` | 声明 | `void Clear()` | 重置或移除 clear 相关逻辑。 |
| [L43](../src/server/signaling/DirectSessionRegistry.h#L43) | `Sessions` | 声明 | `SessionMap& Sessions()` | 实现 sessions 对应的业务或工具逻辑。 |
| [L44](../src/server/signaling/DirectSessionRegistry.h#L44) | `Sessions` | 声明 | `const SessionMap& Sessions() const` | 实现 sessions 对应的业务或工具逻辑。 |
| [L51](../src/server/signaling/DirectSessionRegistry.h#L51) | `RunDirectSessionRegistrySelfTest` | 声明 | `bool RunDirectSessionRegistrySelfTest(QString* errorMessage)` | 执行后台循环或调度 run direct session registry self test 相关逻辑。 |

## `src/server/signaling/RoomRegistry.cpp`

[打开源码](../src/server/signaling/RoomRegistry.cpp) · **文件作用：** 实现 room registry 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L8](../src/server/signaling/RoomRegistry.cpp#L8) | `RoomRegistry::Reserve` | 定义 | `void RoomRegistry::Reserve(qsizetype capacity)` | 实现 reserve 对应的业务或工具逻辑。 |
| [L17](../src/server/signaling/RoomRegistry.cpp#L17) | `RoomRegistry::Clear` | 定义 | `void RoomRegistry::Clear()` | 重置或移除 clear 相关逻辑。 |
| [L31](../src/server/signaling/RoomRegistry.cpp#L31) | `RoomRegistry::PairSize` | 定义 | `qsizetype RoomRegistry::PairSize() const` | 实现 pair size 对应的业务或工具逻辑。 |
| [L36](../src/server/signaling/RoomRegistry.cpp#L36) | `RoomRegistry::JoinRequestSize` | 定义 | `qsizetype RoomRegistry::JoinRequestSize() const` | 实现 join request size 对应的业务或工具逻辑。 |
| [L41](../src/server/signaling/RoomRegistry.cpp#L41) | `RoomRegistry::Rooms` | 定义 | `RoomRegistry::RoomMap& RoomRegistry::Rooms()` | 实现 rooms 对应的业务或工具逻辑。 |
| [L46](../src/server/signaling/RoomRegistry.cpp#L46) | `RoomRegistry::Rooms` | 定义 | `const RoomRegistry::RoomMap& RoomRegistry::Rooms() const` | 实现 rooms 对应的业务或工具逻辑。 |
| [L51](../src/server/signaling/RoomRegistry.cpp#L51) | `RoomRegistry::DeviceRooms` | 定义 | `RoomRegistry::DeviceRoomMap& RoomRegistry::DeviceRooms()` | 实现 device rooms 对应的业务或工具逻辑。 |
| [L56](../src/server/signaling/RoomRegistry.cpp#L56) | `RoomRegistry::DeviceRooms` | 定义 | `const RoomRegistry::DeviceRoomMap& RoomRegistry::DeviceRooms() const` | 实现 device rooms 对应的业务或工具逻辑。 |
| [L61](../src/server/signaling/RoomRegistry.cpp#L61) | `RoomRegistry::PairRooms` | 定义 | `RoomRegistry::PairRoomMap& RoomRegistry::PairRooms()` | 实现 pair rooms 对应的业务或工具逻辑。 |
| [L66](../src/server/signaling/RoomRegistry.cpp#L66) | `RoomRegistry::PairRooms` | 定义 | `const RoomRegistry::PairRoomMap& RoomRegistry::PairRooms() const` | 实现 pair rooms 对应的业务或工具逻辑。 |
| [L71](../src/server/signaling/RoomRegistry.cpp#L71) | `RoomRegistry::JoinRequests` | 定义 | `RoomRegistry::JoinRequestMap& RoomRegistry::JoinRequests()` | 实现 join requests 对应的业务或工具逻辑。 |
| [L76](../src/server/signaling/RoomRegistry.cpp#L76) | `RoomRegistry::JoinRequests` | 定义 | `const RoomRegistry::JoinRequestMap& RoomRegistry::JoinRequests() const` | 实现 join requests 对应的业务或工具逻辑。 |
| [L82](../src/server/signaling/RoomRegistry.cpp#L82) | `RoomRegistry::PendingJoinRequestsByDevice` | 定义 | `RoomRegistry::PendingJoinRequestsByDevice()` | 实现 pending join requests by device 对应的业务或工具逻辑。 |
| [L88](../src/server/signaling/RoomRegistry.cpp#L88) | `RoomRegistry::PendingJoinRequestsByDevice` | 定义 | `RoomRegistry::PendingJoinRequestsByDevice() const` | 实现 pending join requests by device 对应的业务或工具逻辑。 |
| [L93](../src/server/signaling/RoomRegistry.cpp#L93) | `RunRoomRegistrySelfTest` | 定义 | `bool RunRoomRegistrySelfTest(QString* errorMessage)` | 执行后台循环或调度 run room registry self test 相关逻辑。 |

## `src/server/signaling/RoomRegistry.h`

[打开源码](../src/server/signaling/RoomRegistry.h) · **文件作用：** 声明 room registry 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L11](../src/server/signaling/RoomRegistry.h#L11) | `RoomMemberState` | struct | 定义 RoomMemberState 的 struct 类型和相关状态。 |
| [L20](../src/server/signaling/RoomRegistry.h#L20) | `RoomPairState` | struct | 定义 RoomPairState 的 struct 类型和相关状态。 |
| [L27](../src/server/signaling/RoomRegistry.h#L27) | `RoomState` | struct | 定义 RoomState 的 struct 类型和相关状态。 |
| [L48](../src/server/signaling/RoomRegistry.h#L48) | `RoomJoinRequestState` | struct | 定义 RoomJoinRequestState 的 struct 类型和相关状态。 |
| [L56](../src/server/signaling/RoomRegistry.h#L56) | `RoomRegistry` | class | 定义 RoomRegistry 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L12](../src/server/signaling/RoomRegistry.h#L12) | `deviceId` | `QString deviceId;` | 保存身份或作用域标识：device id。 |
| [L13](../src/server/signaling/RoomRegistry.h#L13) | `deviceName` | `QString deviceName;` | 保存路径、地址或显示名称：device name。 |
| [L14](../src/server/signaling/RoomRegistry.h#L14) | `recoveryToken` | `QString recoveryToken;` | 保存 recovery token 相关配置或运行状态。 |
| [L15](../src/server/signaling/RoomRegistry.h#L15) | `disconnectedAtMs` | `qint64 disconnectedAtMs = 0;` | 保存 disconnected at ms 相关配置或运行状态。 |
| [L16](../src/server/signaling/RoomRegistry.h#L16) | `cameraPublishing` | `bool cameraPublishing = false;` | 保存 camera publishing 相关配置或运行状态。 |
| [L17](../src/server/signaling/RoomRegistry.h#L17) | `microphonePublishing` | `bool microphonePublishing = false;` | 保存 microphone publishing 相关配置或运行状态。 |
| [L21](../src/server/signaling/RoomRegistry.h#L21) | `pairId` | `QString pairId;` | 保存身份或作用域标识：pair id。 |
| [L22](../src/server/signaling/RoomRegistry.h#L22) | `firstDeviceId` | `QString firstDeviceId;` | 保存身份或作用域标识：first device id。 |
| [L23](../src/server/signaling/RoomRegistry.h#L23) | `secondDeviceId` | `QString secondDeviceId;` | 保存身份或作用域标识：second device id。 |
| [L24](../src/server/signaling/RoomRegistry.h#L24) | `offererDeviceId` | `QString offererDeviceId;` | 保存身份或作用域标识：offerer device id。 |
| [L28](../src/server/signaling/RoomRegistry.h#L28) | `roomId` | `QString roomId;` | 保存身份或作用域标识：room id。 |
| [L29](../src/server/signaling/RoomRegistry.h#L29) | `ownerDeviceId` | `QString ownerDeviceId;` | 保存身份或作用域标识：owner device id。 |
| [L30](../src/server/signaling/RoomRegistry.h#L30) | `capacity` | `int capacity = 2;` | 保存 capacity 相关配置或运行状态。 |
| [L32](../src/server/signaling/RoomRegistry.h#L32) | `screenSharerDeviceId` | `QString screenSharerDeviceId;` | 保存身份或作用域标识：screen sharer device id。 |
| [L33](../src/server/signaling/RoomRegistry.h#L33) | `pendingScreenSharerDeviceId` | `QString pendingScreenSharerDeviceId;` | 保存身份或作用域标识：pending screen sharer device id。 |
| [L34](../src/server/signaling/RoomRegistry.h#L34) | `screenShareEpoch` | `quint64 screenShareEpoch = 0;` | 标记当前世代，用于拒绝过期异步结果：screen share epoch。 |
| [L35](../src/server/signaling/RoomRegistry.h#L35) | `screenShareGrantId` | `QString screenShareGrantId;` | 保存身份或作用域标识：screen share grant id。 |
| [L36](../src/server/signaling/RoomRegistry.h#L36) | `screenShareSwitchRequestId` | `QString screenShareSwitchRequestId;` | 保存身份或作用域标识：screen share switch request id。 |
| [L37](../src/server/signaling/RoomRegistry.h#L37) | `screenShareSwitchRequesterDeviceId` | `QString screenShareSwitchRequesterDeviceId;` | 保存身份或作用域标识：screen share switch requester device id。 |
| [L38](../src/server/signaling/RoomRegistry.h#L38) | `screenShareSwitchRequestExpiresAtMs` | `qint64 screenShareSwitchRequestExpiresAtMs = 0;` | 保存 screen share switch request expires at ms 相关配置或运行状态。 |
| [L39](../src/server/signaling/RoomRegistry.h#L39) | `pendingControllerDeviceId` | `QString pendingControllerDeviceId;` | 保存身份或作用域标识：pending controller device id。 |
| [L40](../src/server/signaling/RoomRegistry.h#L40) | `controlRequestId` | `QString controlRequestId;` | 保存身份或作用域标识：control request id。 |
| [L41](../src/server/signaling/RoomRegistry.h#L41) | `controlRequestExpiresAtMs` | `qint64 controlRequestExpiresAtMs = 0;` | 保存 control request expires at ms 相关配置或运行状态。 |
| [L42](../src/server/signaling/RoomRegistry.h#L42) | `activeControllerDeviceId` | `QString activeControllerDeviceId;` | 保存身份或作用域标识：active controller device id。 |
| [L43](../src/server/signaling/RoomRegistry.h#L43) | `controlGrantId` | `QString controlGrantId;` | 保存身份或作用域标识：control grant id。 |
| [L44](../src/server/signaling/RoomRegistry.h#L44) | `members` | `QHash<QString, RoomMemberState> members;` | 保存 members 相关配置或运行状态。 |
| [L45](../src/server/signaling/RoomRegistry.h#L45) | `pairs` | `QHash<QString, RoomPairState> pairs;` | 保存 pairs 相关配置或运行状态。 |
| [L49](../src/server/signaling/RoomRegistry.h#L49) | `requestId` | `QString requestId;` | 保存身份或作用域标识：request id。 |
| [L50](../src/server/signaling/RoomRegistry.h#L50) | `roomId` | `QString roomId;` | 保存身份或作用域标识：room id。 |
| [L51](../src/server/signaling/RoomRegistry.h#L51) | `requesterDeviceId` | `QString requesterDeviceId;` | 保存身份或作用域标识：requester device id。 |
| [L52](../src/server/signaling/RoomRegistry.h#L52) | `requesterDeviceName` | `QString requesterDeviceName;` | 保存路径、地址或显示名称：requester device name。 |
| [L53](../src/server/signaling/RoomRegistry.h#L53) | `expiresAtMs` | `qint64 expiresAtMs = 0;` | 保存 expires at ms 相关配置或运行状态。 |
| [L82](../src/server/signaling/RoomRegistry.h#L82) | `rooms_` | `RoomMap rooms_;` | 保存 rooms 相关配置或运行状态。 |
| [L83](../src/server/signaling/RoomRegistry.h#L83) | `deviceRooms_` | `DeviceRoomMap deviceRooms_;` | 保存 device rooms 相关配置或运行状态。 |
| [L84](../src/server/signaling/RoomRegistry.h#L84) | `pairRooms_` | `PairRoomMap pairRooms_;` | 保存 pair rooms 相关配置或运行状态。 |
| [L85](../src/server/signaling/RoomRegistry.h#L85) | `joinRequests_` | `JoinRequestMap joinRequests_;` | 保存 join requests 相关配置或运行状态。 |
| [L86](../src/server/signaling/RoomRegistry.h#L86) | `pendingJoinRequestsByDevice_` | `PendingJoinRequestMap pendingJoinRequestsByDevice_;` | 保存 pending join requests by device 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L64](../src/server/signaling/RoomRegistry.h#L64) | `Reserve` | 声明 | `void Reserve(qsizetype capacity)` | 实现 reserve 对应的业务或工具逻辑。 |
| [L65](../src/server/signaling/RoomRegistry.h#L65) | `Clear` | 声明 | `void Clear()` | 重置或移除 clear 相关逻辑。 |
| [L67](../src/server/signaling/RoomRegistry.h#L67) | `PairSize` | 声明 | `qsizetype PairSize() const` | 实现 pair size 对应的业务或工具逻辑。 |
| [L68](../src/server/signaling/RoomRegistry.h#L68) | `JoinRequestSize` | 声明 | `qsizetype JoinRequestSize() const` | 实现 join request size 对应的业务或工具逻辑。 |
| [L70](../src/server/signaling/RoomRegistry.h#L70) | `Rooms` | 声明 | `RoomMap& Rooms()` | 实现 rooms 对应的业务或工具逻辑。 |
| [L71](../src/server/signaling/RoomRegistry.h#L71) | `Rooms` | 声明 | `const RoomMap& Rooms() const` | 实现 rooms 对应的业务或工具逻辑。 |
| [L72](../src/server/signaling/RoomRegistry.h#L72) | `DeviceRooms` | 声明 | `DeviceRoomMap& DeviceRooms()` | 实现 device rooms 对应的业务或工具逻辑。 |
| [L73](../src/server/signaling/RoomRegistry.h#L73) | `DeviceRooms` | 声明 | `const DeviceRoomMap& DeviceRooms() const` | 实现 device rooms 对应的业务或工具逻辑。 |
| [L74](../src/server/signaling/RoomRegistry.h#L74) | `PairRooms` | 声明 | `PairRoomMap& PairRooms()` | 实现 pair rooms 对应的业务或工具逻辑。 |
| [L75](../src/server/signaling/RoomRegistry.h#L75) | `PairRooms` | 声明 | `const PairRoomMap& PairRooms() const` | 实现 pair rooms 对应的业务或工具逻辑。 |
| [L76](../src/server/signaling/RoomRegistry.h#L76) | `JoinRequests` | 声明 | `JoinRequestMap& JoinRequests()` | 实现 join requests 对应的业务或工具逻辑。 |
| [L77](../src/server/signaling/RoomRegistry.h#L77) | `JoinRequests` | 声明 | `const JoinRequestMap& JoinRequests() const` | 实现 join requests 对应的业务或工具逻辑。 |
| [L78](../src/server/signaling/RoomRegistry.h#L78) | `PendingJoinRequestsByDevice` | 声明 | `PendingJoinRequestMap& PendingJoinRequestsByDevice()` | 实现 pending join requests by device 对应的业务或工具逻辑。 |
| [L79](../src/server/signaling/RoomRegistry.h#L79) | `PendingJoinRequestsByDevice` | 声明 | `const PendingJoinRequestMap& PendingJoinRequestsByDevice() const` | 实现 pending join requests by device 对应的业务或工具逻辑。 |
| [L89](../src/server/signaling/RoomRegistry.h#L89) | `RunRoomRegistrySelfTest` | 声明 | `bool RunRoomRegistrySelfTest(QString* errorMessage)` | 执行后台循环或调度 run room registry self test 相关逻辑。 |

## `src/server/signaling/SignalServer.AccountManagement.cpp`

[打开源码](../src/server/signaling/SignalServer.AccountManagement.cpp) · **文件作用：** 实现 signal server account management 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/server/signaling/SignalServer.AccountManagement.cpp#L9) | `SignalServer::Impl::SendAccountDeletionResult` | 定义 | `void SignalServer::Impl::SendAccountDeletionResult( QWebSocket* socket, bool deleted, const QString& code, const QString& message, bool retryable)` | 发送或发布 send account deletion result 相关逻辑。 |
| [L25](../src/server/signaling/SignalServer.AccountManagement.cpp#L25) | `SignalServer::Impl::HandleAccountDelete` | 定义 | `void SignalServer::Impl::HandleAccountDelete(ClientState* client)` | 接收并处理 handle account delete 相关逻辑。 |

## `src/server/signaling/SignalServer.Authentication.cpp`

[打开源码](../src/server/signaling/SignalServer.Authentication.cpp) · **文件作用：** 实现 signal server authentication 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/server/signaling/SignalServer.Authentication.cpp#L9) | `SignalServer::Impl::HandleMessageAuthentication` | 定义 | `void SignalServer::Impl::HandleMessageAuthentication( ClientState* client, const QJsonObject& payload)` | 接收并处理 handle message authentication 相关逻辑。 |
| [L166](../src/server/signaling/SignalServer.Authentication.cpp#L166) | `SignalServer::Impl::SendAuthenticationError` | 定义 | `void SignalServer::Impl::SendAuthenticationError( QWebSocket* socket, const QString& reason, const QString& message, bool retryable)` | 发送或发布 send authentication error 相关逻辑。 |

## `src/server/signaling/SignalServer.Connection.cpp`

[打开源码](../src/server/signaling/SignalServer.Connection.cpp) · **文件作用：** 实现 signal server connection 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/server/signaling/SignalServer.Connection.cpp#L9) | `SignalServer::Impl::AcceptConnections` | 定义 | `void SignalServer::Impl::AcceptConnections()` | 处理并回复 accept connections 相关逻辑。 |
| [L126](../src/server/signaling/SignalServer.Connection.cpp#L126) | `SignalServer::Impl::RejectConnection` | 定义 | `void SignalServer::Impl::RejectConnection(QWebSocket* socket, const QString& reason)` | 处理并回复 reject connection 相关逻辑。 |
| [L134](../src/server/signaling/SignalServer.Connection.cpp#L134) | `SignalServer::Impl::OnTextMessage` | 定义 | `void SignalServer::Impl::OnTextMessage(QWebSocket* socket, const QString& message)` | 接收并处理 on text message 相关逻辑。 |

## `src/server/signaling/SignalServer.cpp`

[打开源码](../src/server/signaling/SignalServer.cpp) · **文件作用：** 实现 signal server 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L8](../src/server/signaling/SignalServer.cpp#L8) | `SignalServer::SignalServer` | 定义 | `SignalServer::SignalServer(SignalServerConfig config) : impl_(std::make_unique<Impl>(std::move(config))) {}` | 构造并初始化 SignalServer 实例。 |
| [L12](../src/server/signaling/SignalServer.cpp#L12) | `SignalServer::~SignalServer` | 定义 | `SignalServer::~SignalServer()` | 停止相关活动并释放 SignalServer 实例拥有的资源。 |
| [L17](../src/server/signaling/SignalServer.cpp#L17) | `SignalServer::Start` | 定义 | `bool SignalServer::Start(QString* error)` | 启动 start 相关逻辑。 |
| [L22](../src/server/signaling/SignalServer.cpp#L22) | `SignalServer::Stop` | 定义 | `void SignalServer::Stop()` | 停止 stop 相关逻辑。 |
| [L27](../src/server/signaling/SignalServer.cpp#L27) | `SignalServer::IsListening` | 定义 | `bool SignalServer::IsListening() const` | 判断 is listening 相关逻辑。 |
| [L32](../src/server/signaling/SignalServer.cpp#L32) | `SignalServer::ServerPort` | 定义 | `quint16 SignalServer::ServerPort() const` | 实现 server port 对应的业务或工具逻辑。 |
| [L37](../src/server/signaling/SignalServer.cpp#L37) | `SignalServer::WebhookPort` | 定义 | `quint16 SignalServer::WebhookPort() const` | 实现 webhook port 对应的业务或工具逻辑。 |

## `src/server/signaling/SignalServer.h`

[打开源码](../src/server/signaling/SignalServer.h) · **文件作用：** 声明 signal server 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L16](../src/server/signaling/SignalServer.h#L16) | `SignalServerAuthenticationMode` | enum class | 定义 SignalServerAuthenticationMode 的 enum class 类型和相关状态。 |
| [L27](../src/server/signaling/SignalServer.h#L27) | `SignalServerConfig` | struct | 定义 SignalServerConfig 的 struct 类型和相关状态。 |
| [L60](../src/server/signaling/SignalServer.h#L60) | `SignalServer` | class | 定义 SignalServer 的 class 类型和相关状态。 |
| [L75](../src/server/signaling/SignalServer.h#L75) | `Impl` | class | 定义 Impl 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L28](../src/server/signaling/SignalServer.h#L28) | `listenAddress` | `QHostAddress listenAddress = QHostAddress::LocalHost;` | 保存 listen address 相关配置或运行状态。 |
| [L29](../src/server/signaling/SignalServer.h#L29) | `port` | `quint16 port = 9443;` | 保存 port 相关配置或运行状态。 |
| [L30](../src/server/signaling/SignalServer.h#L30) | `certificateFile` | `QString certificateFile;` | 保存 certificate file 相关配置或运行状态。 |
| [L31](../src/server/signaling/SignalServer.h#L31) | `privateKeyFile` | `QString privateKeyFile;` | 保存 private key file 相关配置或运行状态。 |
| [L32](../src/server/signaling/SignalServer.h#L32) | `tokenSecret` | `QByteArray tokenSecret;` | 保存 token secret 相关配置或运行状态。 |
| [L34](../src/server/signaling/SignalServer.h#L34) | `kLegacyUpgradeBearer` | `SignalServerAuthenticationMode::kLegacyUpgradeBearer;` | 定义 legacy upgrade bearer 的编译期常量或产品边界。 |
| [L35](../src/server/signaling/SignalServer.h#L35) | `iceServerUrls` | `QStringList iceServerUrls;` | 保存 ice server urls 相关配置或运行状态。 |
| [L36](../src/server/signaling/SignalServer.h#L36) | `maximumPendingConnections` | `int maximumPendingConnections = 128;` | 保存 maximum pending connections 相关配置或运行状态。 |
| [L37](../src/server/signaling/SignalServer.h#L37) | `maximumConnections` | `int maximumConnections = 5000;` | 保存 maximum connections 相关配置或运行状态。 |
| [L38](../src/server/signaling/SignalServer.h#L38) | `maximumUnauthenticatedConnections` | `int maximumUnauthenticatedConnections = 512;` | 保存 maximum unauthenticated connections 相关配置或运行状态。 |
| [L39](../src/server/signaling/SignalServer.h#L39) | `maximumConnectionsPerIp` | `int maximumConnectionsPerIp = 0;` | 保存 maximum connections per ip 相关配置或运行状态。 |
| [L40](../src/server/signaling/SignalServer.h#L40) | `authenticationIpLimitPerMinute` | `int authenticationIpLimitPerMinute = 20;` | 保存 authentication ip limit per minute 相关配置或运行状态。 |
| [L41](../src/server/signaling/SignalServer.h#L41) | `disableBusinessRateLimitsForTest` | `bool disableBusinessRateLimitsForTest = false;` | 保存 disable business rate limits for test 相关配置或运行状态。 |
| [L42](../src/server/signaling/SignalServer.h#L42) | `diagnosticsIntervalMs` | `int diagnosticsIntervalMs = 0;` | 保存 diagnostics interval ms 相关配置或运行状态。 |
| [L43](../src/server/signaling/SignalServer.h#L43) | `diagnosticsLogFile` | `QString diagnosticsLogFile;` | 保存 diagnostics log file 相关配置或运行状态。 |
| [L44](../src/server/signaling/SignalServer.h#L44) | `authenticationTimeoutMs` | `int authenticationTimeoutMs = 10000;` | 保存 authentication timeout ms 相关配置或运行状态。 |
| [L45](../src/server/signaling/SignalServer.h#L45) | `logtoIssuer` | `QUrl logtoIssuer;` | 保存 logto issuer 相关配置或运行状态。 |
| [L46](../src/server/signaling/SignalServer.h#L46) | `identityDatabaseFile` | `QString identityDatabaseFile;` | 保存 identity database file 相关配置或运行状态。 |
| [L47](../src/server/signaling/SignalServer.h#L47) | `userInfoTimeoutMs` | `int userInfoTimeoutMs = 10000;` | 保存 user info timeout ms 相关配置或运行状态。 |
| [L48](../src/server/signaling/SignalServer.h#L48) | `logtoManagementClientId` | `QString logtoManagementClientId;` | 保存身份或作用域标识：logto management client id。 |
| [L49](../src/server/signaling/SignalServer.h#L49) | `logtoManagementClientSecret` | `QByteArray logtoManagementClientSecret;` | 保存 logto management client secret 相关配置或运行状态。 |
| [L50](../src/server/signaling/SignalServer.h#L50) | `managementTimeoutMs` | `int managementTimeoutMs = 10000;` | 保存 management timeout ms 相关配置或运行状态。 |
| [L51](../src/server/signaling/SignalServer.h#L51) | `webhookListenAddress` | `QHostAddress webhookListenAddress = QHostAddress::LocalHost;` | 保存 webhook listen address 相关配置或运行状态。 |
| [L52](../src/server/signaling/SignalServer.h#L52) | `webhookPort` | `quint16 webhookPort = 0;` | 保存 webhook port 相关配置或运行状态。 |
| [L53](../src/server/signaling/SignalServer.h#L53) | `webhookSigningKey` | `QByteArray webhookSigningKey;` | 保存 webhook signing key 相关配置或运行状态。 |
| [L54](../src/server/signaling/SignalServer.h#L54) | `clientIdleTimeoutMs` | `int clientIdleTimeoutMs = 45000;` | 保存 client idle timeout ms 相关配置或运行状态。 |
| [L55](../src/server/signaling/SignalServer.h#L55) | `pendingSessionTimeoutMs` | `int pendingSessionTimeoutMs = 20000;` | 保存 pending session timeout ms 相关配置或运行状态。 |
| [L56](../src/server/signaling/SignalServer.h#L56) | `sessionRecoveryWindowMs` | `int sessionRecoveryWindowMs = 60000;` | 保存 session recovery window ms 相关配置或运行状态。 |
| [L57](../src/server/signaling/SignalServer.h#L57) | `maximumRoomMembers` | `int maximumRoomMembers = 5;` | 保存 maximum room members 相关配置或运行状态。 |
| [L75](../src/server/signaling/SignalServer.h#L75) | `Impl` | `class Impl;` | 保存 impl 相关配置或运行状态。 |
| [L76](../src/server/signaling/SignalServer.h#L76) | `impl_` | `std::unique_ptr<Impl> impl_;` | 保存 impl 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L62](../src/server/signaling/SignalServer.h#L62) | `SignalServer` | 声明 | `explicit SignalServer(SignalServerConfig config)` | 实现 signal server 对应的业务或工具逻辑。 |
| [L63](../src/server/signaling/SignalServer.h#L63) | `~SignalServer` | 声明 | `~SignalServer()` | 停止相关活动并释放 SignalServer 实例拥有的资源。 |
| [L65](../src/server/signaling/SignalServer.h#L65) | `SignalServer` | 声明 | `SignalServer(const SignalServer&) = delete` | 实现 signal server 对应的业务或工具逻辑。 |
| [L68](../src/server/signaling/SignalServer.h#L68) | `Start` | 声明 | `bool Start(QString* error)` | 启动 start 相关逻辑。 |
| [L69](../src/server/signaling/SignalServer.h#L69) | `Stop` | 声明 | `void Stop()` | 停止 stop 相关逻辑。 |
| [L70](../src/server/signaling/SignalServer.h#L70) | `IsListening` | 声明 | `bool IsListening() const` | 判断 is listening 相关逻辑。 |
| [L71](../src/server/signaling/SignalServer.h#L71) | `ServerPort` | 声明 | `quint16 ServerPort() const` | 实现 server port 对应的业务或工具逻辑。 |
| [L72](../src/server/signaling/SignalServer.h#L72) | `WebhookPort` | 声明 | `quint16 WebhookPort() const` | 实现 webhook port 对应的业务或工具逻辑。 |

## `src/server/signaling/SignalServer.Internal.h`

[打开源码](../src/server/signaling/SignalServer.Internal.h) · **文件作用：** 声明 signal server internal 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L51](../src/server/signaling/SignalServer.Internal.h#L51) | `SignalServer::Impl` | class | 定义 SignalServer::Impl 的 class 类型和相关状态。 |
| [L60](../src/server/signaling/SignalServer.Internal.h#L60) | `ClientState` | struct | 定义 ClientState 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L61](../src/server/signaling/SignalServer.Internal.h#L61) | `socket` | `QWebSocket* socket = nullptr;` | 保存 socket 相关配置或运行状态。 |
| [L62](../src/server/signaling/SignalServer.Internal.h#L62) | `claims` | `AccessTokenClaims claims;` | 保存 claims 相关配置或运行状态。 |
| [L63](../src/server/signaling/SignalServer.Internal.h#L63) | `authenticated` | `bool authenticated = false;` | 保存 authenticated 相关配置或运行状态。 |
| [L64](../src/server/signaling/SignalServer.Internal.h#L64) | `authenticationPending` | `bool authenticationPending = false;` | 保存待处理队列或请求：authentication pending。 |
| [L65](../src/server/signaling/SignalServer.Internal.h#L65) | `accountDeletionPending` | `bool accountDeletionPending = false;` | 保存待处理队列或请求：account deletion pending。 |
| [L66](../src/server/signaling/SignalServer.Internal.h#L66) | `messageAuthenticated` | `bool messageAuthenticated = false;` | 保存 message authenticated 相关配置或运行状态。 |
| [L67](../src/server/signaling/SignalServer.Internal.h#L67) | `registered` | `bool registered = false;` | 保存 registered 相关配置或运行状态。 |
| [L68](../src/server/signaling/SignalServer.Internal.h#L68) | `userInfoReply` | `QPointer<QNetworkReply> userInfoReply;` | 保存 user info reply 相关配置或运行状态。 |
| [L69](../src/server/signaling/SignalServer.Internal.h#L69) | `trustedUserId` | `qint64 trustedUserId = 0;` | 保存身份或作用域标识：trusted user id。 |
| [L70](../src/server/signaling/SignalServer.Internal.h#L70) | `logtoSubject` | `QString logtoSubject;` | 保存 logto subject 相关配置或运行状态。 |
| [L71](../src/server/signaling/SignalServer.Internal.h#L71) | `peerKey` | `QString peerKey;` | 保存 peer key 相关配置或运行状态。 |
| [L72](../src/server/signaling/SignalServer.Internal.h#L72) | `deviceName` | `QString deviceName;` | 保存路径、地址或显示名称：device name。 |
| [L73](../src/server/signaling/SignalServer.Internal.h#L73) | `receivedMessageIds` | `QSet<QByteArray> receivedMessageIds;` | 保存 received message ids 相关配置或运行状态。 |
| [L74](../src/server/signaling/SignalServer.Internal.h#L74) | `receivedMessageOrder` | `QQueue<QByteArray> receivedMessageOrder;` | 保存 received message order 相关配置或运行状态。 |
| [L75](../src/server/signaling/SignalServer.Internal.h#L75) | `lastActivityMs` | `qint64 lastActivityMs = 0;` | 保存 last activity ms 相关配置或运行状态。 |
| [L76](../src/server/signaling/SignalServer.Internal.h#L76) | `authenticationDeadlineMs` | `qint64 authenticationDeadlineMs = 0;` | 保存 authentication deadline ms 相关配置或运行状态。 |
| [L862](../src/server/signaling/SignalServer.Internal.h#L862) | `config_` | `SignalServerConfig config_;` | 保存 config 相关配置或运行状态。 |
| [L863](../src/server/signaling/SignalServer.Internal.h#L863) | `tokenService_` | `AccessTokenService tokenService_;` | 保存 token service 相关配置或运行状态。 |
| [L864](../src/server/signaling/SignalServer.Internal.h#L864) | `userInfoClient_` | `remote::server_auth::LogtoUserInfoClient userInfoClient_;` | 保存 user info client 相关配置或运行状态。 |
| [L865](../src/server/signaling/SignalServer.Internal.h#L865) | `managementClient_` | `remote::server_auth::LogtoManagementClient managementClient_;` | 保存 management client 相关配置或运行状态。 |
| [L866](../src/server/signaling/SignalServer.Internal.h#L866) | `webhookServer_` | `remote::server_auth::LogtoWebhookServer webhookServer_;` | 保存 webhook server 相关配置或运行状态。 |
| [L867](../src/server/signaling/SignalServer.Internal.h#L867) | `identityStore_` | `remote::server_persistence::IdentityStore identityStore_;` | 保存 identity store 相关配置或运行状态。 |
| [L868](../src/server/signaling/SignalServer.Internal.h#L868) | `authenticationIpRateLimiter_` | `SlidingWindowRateLimiter authenticationIpRateLimiter_;` | 保存 authentication ip rate limiter 相关配置或运行状态。 |
| [L869](../src/server/signaling/SignalServer.Internal.h#L869) | `availabilityIpRateLimiter_` | `SlidingWindowRateLimiter availabilityIpRateLimiter_;` | 保存 availability ip rate limiter 相关配置或运行状态。 |
| [L870](../src/server/signaling/SignalServer.Internal.h#L870) | `availabilityUserRateLimiter_` | `SlidingWindowRateLimiter availabilityUserRateLimiter_;` | 保存 availability user rate limiter 相关配置或运行状态。 |
| [L871](../src/server/signaling/SignalServer.Internal.h#L871) | `availabilityDeviceRateLimiter_` | `SlidingWindowRateLimiter availabilityDeviceRateLimiter_;` | 保存 availability device rate limiter 相关配置或运行状态。 |
| [L872](../src/server/signaling/SignalServer.Internal.h#L872) | `roomJoinIpRateLimiter_` | `SlidingWindowRateLimiter roomJoinIpRateLimiter_;` | 保存 room join ip rate limiter 相关配置或运行状态。 |
| [L873](../src/server/signaling/SignalServer.Internal.h#L873) | `roomJoinUserRateLimiter_` | `SlidingWindowRateLimiter roomJoinUserRateLimiter_;` | 保存 room join user rate limiter 相关配置或运行状态。 |
| [L874](../src/server/signaling/SignalServer.Internal.h#L874) | `roomJoinDeviceRateLimiter_` | `SlidingWindowRateLimiter roomJoinDeviceRateLimiter_;` | 保存 room join device rate limiter 相关配置或运行状态。 |
| [L883](../src/server/signaling/SignalServer.Internal.h#L883) | `server_` | `QWebSocketServer server_;` | 保存 server 相关配置或运行状态。 |
| [L884](../src/server/signaling/SignalServer.Internal.h#L884) | `maintenanceTimer_` | `QTimer maintenanceTimer_;` | 保存定时、截止或超时状态：maintenance timer。 |
| [L885](../src/server/signaling/SignalServer.Internal.h#L885) | `diagnosticProbeTimer_` | `QTimer diagnosticProbeTimer_;` | 保存定时、截止或超时状态：diagnostic probe timer。 |
| [L886](../src/server/signaling/SignalServer.Internal.h#L886) | `diagnosticsTimer_` | `QTimer diagnosticsTimer_;` | 保存定时、截止或超时状态：diagnostics timer。 |
| [L887](../src/server/signaling/SignalServer.Internal.h#L887) | `diagnosticsFile_` | `QFile diagnosticsFile_;` | 保存 diagnostics file 相关配置或运行状态。 |
| [L888](../src/server/signaling/SignalServer.Internal.h#L888) | `diagnosticsClock_` | `QElapsedTimer diagnosticsClock_;` | 保护跨线程共享状态：diagnostics clock。 |
| [L889](../src/server/signaling/SignalServer.Internal.h#L889) | `diagnosticProbeExpectedMs_` | `qint64 diagnosticProbeExpectedMs_ = 0;` | 保存 diagnostic probe expected ms 相关配置或运行状态。 |
| [L890](../src/server/signaling/SignalServer.Internal.h#L890) | `maximumEventLoopLagMs_` | `qint64 maximumEventLoopLagMs_ = 0;` | 保存 maximum event loop lag ms 相关配置或运行状态。 |
| [L891](../src/server/signaling/SignalServer.Internal.h#L891) | `lastDiagnosticsAtMs_` | `qint64 lastDiagnosticsAtMs_ = 0;` | 保存 last diagnostics at ms 相关配置或运行状态。 |
| [L892](../src/server/signaling/SignalServer.Internal.h#L892) | `lastReceivedMessagesTotal_` | `qint64 lastReceivedMessagesTotal_ = 0;` | 保存 last received messages total 相关配置或运行状态。 |
| [L893](../src/server/signaling/SignalServer.Internal.h#L893) | `lastReceivedBytesTotal_` | `qint64 lastReceivedBytesTotal_ = 0;` | 保存 last received bytes total 相关配置或运行状态。 |
| [L894](../src/server/signaling/SignalServer.Internal.h#L894) | `lastSentMessagesTotal_` | `qint64 lastSentMessagesTotal_ = 0;` | 保存 last sent messages total 相关配置或运行状态。 |
| [L895](../src/server/signaling/SignalServer.Internal.h#L895) | `lastSentBytesTotal_` | `qint64 lastSentBytesTotal_ = 0;` | 保存 last sent bytes total 相关配置或运行状态。 |
| [L896](../src/server/signaling/SignalServer.Internal.h#L896) | `acceptedConnectionsTotal_` | `qint64 acceptedConnectionsTotal_ = 0;` | 保存 accepted connections total 相关配置或运行状态。 |
| [L897](../src/server/signaling/SignalServer.Internal.h#L897) | `rejectedConnectionsTotal_` | `qint64 rejectedConnectionsTotal_ = 0;` | 保存 rejected connections total 相关配置或运行状态。 |
| [L898](../src/server/signaling/SignalServer.Internal.h#L898) | `receivedMessagesTotal_` | `qint64 receivedMessagesTotal_ = 0;` | 保存 received messages total 相关配置或运行状态。 |
| [L899](../src/server/signaling/SignalServer.Internal.h#L899) | `receivedBytesTotal_` | `qint64 receivedBytesTotal_ = 0;` | 保存 received bytes total 相关配置或运行状态。 |
| [L900](../src/server/signaling/SignalServer.Internal.h#L900) | `sentMessagesTotal_` | `qint64 sentMessagesTotal_ = 0;` | 保存 sent messages total 相关配置或运行状态。 |
| [L901](../src/server/signaling/SignalServer.Internal.h#L901) | `sentBytesTotal_` | `qint64 sentBytesTotal_ = 0;` | 保存 sent bytes total 相关配置或运行状态。 |
| [L902](../src/server/signaling/SignalServer.Internal.h#L902) | `clients_` | `std::unordered_map<QWebSocket*, std::unique_ptr<ClientState>> clients_;` | 保存 clients 相关配置或运行状态。 |
| [L903](../src/server/signaling/SignalServer.Internal.h#L903) | `unauthenticatedConnectionCount_` | `int unauthenticatedConnectionCount_ = 0;` | 保存计数、尺寸或速率指标：unauthenticated connection count。 |
| [L904](../src/server/signaling/SignalServer.Internal.h#L904) | `connectionCountByIp_` | `QHash<QString, int> connectionCountByIp_;` | 保存 connection count by ip 相关配置或运行状态。 |
| [L905](../src/server/signaling/SignalServer.Internal.h#L905) | `totalRememberedMessageIds_` | `qint64 totalRememberedMessageIds_ = 0;` | 保存 total remembered message ids 相关配置或运行状态。 |
| [L906](../src/server/signaling/SignalServer.Internal.h#L906) | `devices_` | `QHash<QString, QWebSocket*> devices_;` | 保存 devices 相关配置或运行状态。 |
| [L907](../src/server/signaling/SignalServer.Internal.h#L907) | `accountConnections_` | `QHash<qint64, QSet<QWebSocket*>> accountConnections_;` | 保存 account connections 相关配置或运行状态。 |
| [L908](../src/server/signaling/SignalServer.Internal.h#L908) | `ownedDevicesRevision_` | `quint64 ownedDevicesRevision_ = 1;` | 标记当前世代，用于拒绝过期异步结果：owned devices revision。 |
| [L909](../src/server/signaling/SignalServer.Internal.h#L909) | `directSessions_` | `DirectSessionRegistry directSessions_;` | 保存 direct sessions 相关配置或运行状态。 |
| [L910](../src/server/signaling/SignalServer.Internal.h#L910) | `roomRegistry_` | `RoomRegistry roomRegistry_;` | 保存 room registry 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L53](../src/server/signaling/SignalServer.Internal.h#L53) | `Impl` | 声明 | `explicit Impl(SignalServerConfig config)` | 实现 impl 对应的业务或工具逻辑。 |
| [L54](../src/server/signaling/SignalServer.Internal.h#L54) | `Start` | 声明 | `bool Start(QString* error)` | 启动 start 相关逻辑。 |
| [L55](../src/server/signaling/SignalServer.Internal.h#L55) | `Stop` | 声明 | `void Stop()` | 停止 stop 相关逻辑。 |
| [L56](../src/server/signaling/SignalServer.Internal.h#L56) | `IsListening` | 声明 | `bool IsListening() const` | 判断 is listening 相关逻辑。 |
| [L57](../src/server/signaling/SignalServer.Internal.h#L57) | `ServerPort` | 声明 | `quint16 ServerPort() const` | 实现 server port 对应的业务或工具逻辑。 |
| [L58](../src/server/signaling/SignalServer.Internal.h#L58) | `WebhookPort` | 声明 | `quint16 WebhookPort() const` | 实现 webhook port 对应的业务或工具逻辑。 |
| [L81](../src/server/signaling/SignalServer.Internal.h#L81) | `HandleMessageAuthentication` | 声明 | `void HandleMessageAuthentication( ClientState* client, const QJsonObject& payload)` | 接收并处理 handle message authentication 相关逻辑。 |
| [L84](../src/server/signaling/SignalServer.Internal.h#L84) | `SendAuthenticationError` | 声明 | `void SendAuthenticationError( QWebSocket* socket, const QString& reason, const QString& message, bool retryable)` | 发送或发布 send authentication error 相关逻辑。 |
| [L89](../src/server/signaling/SignalServer.Internal.h#L89) | `SendAccountDeletionResult` | 声明 | `void SendAccountDeletionResult( QWebSocket* socket, bool deleted, const QString& code, const QString& message, bool retryable)` | 发送或发布 send account deletion result 相关逻辑。 |
| [L95](../src/server/signaling/SignalServer.Internal.h#L95) | `HandleAccountDelete` | 声明 | `void HandleAccountDelete(ClientState* client)` | 接收并处理 handle account delete 相关逻辑。 |
| [L96](../src/server/signaling/SignalServer.Internal.h#L96) | `AcceptConnections` | 声明 | `void AcceptConnections()` | 处理并回复 accept connections 相关逻辑。 |
| [L97](../src/server/signaling/SignalServer.Internal.h#L97) | `RejectConnection` | 声明 | `void RejectConnection(QWebSocket* socket, const QString& reason)` | 处理并回复 reject connection 相关逻辑。 |
| [L98](../src/server/signaling/SignalServer.Internal.h#L98) | `OnTextMessage` | 声明 | `void OnTextMessage(QWebSocket* socket, const QString& message)` | 接收并处理 on text message 相关逻辑。 |
| [L99](../src/server/signaling/SignalServer.Internal.h#L99) | `BuildOwnedDevicesPayloadFromRecords` | 声明 | `QJsonObject BuildOwnedDevicesPayloadFromRecords( ClientState* recipient, quint64 revision, const QList<remote::server_persistence::OwnedDeviceRecord>& owned, const QString& storeError)` | 创建或初始化 build owned devices payload from records 相关逻辑。 |
| [L104](../src/server/signaling/SignalServer.Internal.h#L104) | `BuildOwnedDevicesPayload` | 声明 | `QJsonObject BuildOwnedDevicesPayload(ClientState* recipient, quint64 revision)` | 创建或初始化 build owned devices payload 相关逻辑。 |
| [L106](../src/server/signaling/SignalServer.Internal.h#L106) | `HandleMyDevicesRequest` | 声明 | `void HandleMyDevicesRequest(ClientState* client)` | 接收并处理 handle my devices request 相关逻辑。 |
| [L107](../src/server/signaling/SignalServer.Internal.h#L107) | `BroadcastOwnedDevices` | 声明 | `void BroadcastOwnedDevices(qint64 trustedUserId)` | 实现 broadcast owned devices 对应的业务或工具逻辑。 |
| [L108](../src/server/signaling/SignalServer.Internal.h#L108) | `HandleRegister` | 声明 | `void HandleRegister(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle register 相关逻辑。 |
| [L109](../src/server/signaling/SignalServer.Internal.h#L109) | `HandleRoomCreate` | 声明 | `void HandleRoomCreate(ClientState* creator, const QJsonObject& payload)` | 接收并处理 handle room create 相关逻辑。 |
| [L111](../src/server/signaling/SignalServer.Internal.h#L111) | `HandleRoomAvailabilityQuery` | 声明 | `void HandleRoomAvailabilityQuery(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room availability query 相关逻辑。 |
| [L113](../src/server/signaling/SignalServer.Internal.h#L113) | `HandleRoomJoinRequest` | 声明 | `void HandleRoomJoinRequest(ClientState* requester, const QJsonObject& payload)` | 接收并处理 handle room join request 相关逻辑。 |
| [L115](../src/server/signaling/SignalServer.Internal.h#L115) | `HandleRoomJoinResponse` | 声明 | `void HandleRoomJoinResponse(ClientState* owner, const QJsonObject& payload)` | 接收并处理 handle room join response 相关逻辑。 |
| [L117](../src/server/signaling/SignalServer.Internal.h#L117) | `HandleRoomSetCapacity` | 声明 | `void HandleRoomSetCapacity(ClientState* owner, const QJsonObject& payload)` | 接收并处理 handle room set capacity 相关逻辑。 |
| [L119](../src/server/signaling/SignalServer.Internal.h#L119) | `HandleRoomLeave` | 声明 | `void HandleRoomLeave(ClientState* memberClient, const QJsonObject& payload)` | 接收并处理 handle room leave 相关逻辑。 |
| [L121](../src/server/signaling/SignalServer.Internal.h#L121) | `HandleRoomMediaState` | 声明 | `void HandleRoomMediaState(ClientState* memberClient, const QJsonObject& payload)` | 接收并处理 handle room media state 相关逻辑。 |
| [L123](../src/server/signaling/SignalServer.Internal.h#L123) | `HandleRoomResume` | 声明 | `void HandleRoomResume(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room resume 相关逻辑。 |
| [L125](../src/server/signaling/SignalServer.Internal.h#L125) | `IsOnlineRoomMember` | 声明 | `bool IsOnlineRoomMember(const RoomState& room, const QString& deviceId) const` | 判断 is online room member 相关逻辑。 |
| [L127](../src/server/signaling/SignalServer.Internal.h#L127) | `SendRoomScreenShareGranted` | 声明 | `void SendRoomScreenShareGranted(QWebSocket* socket, const RoomState& room)` | 发送或发布 send room screen share granted 相关逻辑。 |
| [L129](../src/server/signaling/SignalServer.Internal.h#L129) | `SendRoomScreenShareSwitchPending` | 声明 | `void SendRoomScreenShareSwitchPending(QWebSocket* socket, const RoomState& room)` | 发送或发布 send room screen share switch pending 相关逻辑。 |
| [L131](../src/server/signaling/SignalServer.Internal.h#L131) | `SendRoomScreenShareSwitchRequested` | 声明 | `void SendRoomScreenShareSwitchRequested(QWebSocket* socket, const RoomState& room)` | 发送或发布 send room screen share switch requested 相关逻辑。 |
| [L133](../src/server/signaling/SignalServer.Internal.h#L133) | `SendRoomScreenShareSwitchResult` | 声明 | `void SendRoomScreenShareSwitchResult( QWebSocket* socket, const RoomState& room, const QString& requestId, bool accepted, const QString& reasonCode, const QString& reasonMessage)` | 发送或发布 send room screen share switch result 相关逻辑。 |
| [L140](../src/server/signaling/SignalServer.Internal.h#L140) | `CancelPendingRoomScreenShareSwitch` | 声明 | `void CancelPendingRoomScreenShareSwitch( RoomState& room, const QString& reasonCode, const QString& reasonMessage)` | 判断 cancel pending room screen share switch 相关逻辑。 |
| [L144](../src/server/signaling/SignalServer.Internal.h#L144) | `SendRoomControlResult` | 声明 | `void SendRoomControlResult(QWebSocket* socket, const RoomState& room, const QString& requestId, bool accepted, const QString& reasonCode, const QString& reasonMessage)` | 发送或发布 send room control result 相关逻辑。 |
| [L150](../src/server/signaling/SignalServer.Internal.h#L150) | `SendRoomControlGranted` | 声明 | `void SendRoomControlGranted(QWebSocket* socket, const RoomState& room)` | 发送或发布 send room control granted 相关逻辑。 |
| [L152](../src/server/signaling/SignalServer.Internal.h#L152) | `SendRoomControlRevoked` | 声明 | `void SendRoomControlRevoked(QWebSocket* socket, const RoomState& room, const QString& initiatorDeviceId, const QString& reasonCode)` | 发送或发布 send room control revoked 相关逻辑。 |
| [L156](../src/server/signaling/SignalServer.Internal.h#L156) | `CancelPendingRoomControl` | 声明 | `void CancelPendingRoomControl(RoomState& room, const QString& reasonCode, const QString& reasonMessage)` | 判断 cancel pending room control 相关逻辑。 |
| [L159](../src/server/signaling/SignalServer.Internal.h#L159) | `RevokeRoomControl` | 声明 | `void RevokeRoomControl(RoomState& room, const QString& initiatorDeviceId, const QString& reasonCode)` | 实现 revoke room control 对应的业务或工具逻辑。 |
| [L162](../src/server/signaling/SignalServer.Internal.h#L162) | `ResetRoomScreenShare` | 声明 | `void ResetRoomScreenShare(RoomState& room, const QString& initiatorDeviceId, const QString& reasonCode)` | 重置或移除 reset room screen share 相关逻辑。 |
| [L165](../src/server/signaling/SignalServer.Internal.h#L165) | `HandleRoomScreenShareRequest` | 声明 | `void HandleRoomScreenShareRequest(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room screen share request 相关逻辑。 |
| [L167](../src/server/signaling/SignalServer.Internal.h#L167) | `HandleRoomScreenShareReady` | 声明 | `void HandleRoomScreenShareReady(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room screen share ready 相关逻辑。 |
| [L169](../src/server/signaling/SignalServer.Internal.h#L169) | `HandleRoomScreenShareSwitchResponse` | 声明 | `void HandleRoomScreenShareSwitchResponse( ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room screen share switch response 相关逻辑。 |
| [L172](../src/server/signaling/SignalServer.Internal.h#L172) | `HandleRoomScreenShareSwitchCancel` | 声明 | `void HandleRoomScreenShareSwitchCancel( ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room screen share switch cancel 相关逻辑。 |
| [L175](../src/server/signaling/SignalServer.Internal.h#L175) | `HandleRoomScreenShareStop` | 声明 | `void HandleRoomScreenShareStop(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room screen share stop 相关逻辑。 |
| [L177](../src/server/signaling/SignalServer.Internal.h#L177) | `HandleRoomControlRequest` | 声明 | `void HandleRoomControlRequest(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room control request 相关逻辑。 |
| [L179](../src/server/signaling/SignalServer.Internal.h#L179) | `HandleRoomControlResponse` | 声明 | `void HandleRoomControlResponse(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room control response 相关逻辑。 |
| [L181](../src/server/signaling/SignalServer.Internal.h#L181) | `HandleRoomControlRelease` | 声明 | `void HandleRoomControlRelease(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room control release 相关逻辑。 |
| [L183](../src/server/signaling/SignalServer.Internal.h#L183) | `SerializeIceServers` | 声明 | `QJsonArray SerializeIceServers() const` | 编码 serialize ice servers 相关逻辑。 |
| [L184](../src/server/signaling/SignalServer.Internal.h#L184) | `CreateRoomPair` | 声明 | `void CreateRoomPair(RoomState& room, const QString& firstDeviceId, const QString& secondDeviceId)` | 创建或初始化 create room pair 相关逻辑。 |
| [L187](../src/server/signaling/SignalServer.Internal.h#L187) | `PairPeerDeviceId` | 声明 | `QString PairPeerDeviceId(const RoomPairState& pair, const QString& deviceId) const` | 实现 pair peer device id 对应的业务或工具逻辑。 |
| [L189](../src/server/signaling/SignalServer.Internal.h#L189) | `RoomPairMembersAreOnline` | 声明 | `bool RoomPairMembersAreOnline(const RoomState& room, const RoomPairState& pair) const` | 实现 room pair members are online 对应的业务或工具逻辑。 |
| [L191](../src/server/signaling/SignalServer.Internal.h#L191) | `SendRoomPairReadyToMember` | 声明 | `void SendRoomPairReadyToMember(const RoomState& room, const RoomPairState& pair, const QString& deviceId)` | 发送或发布 send room pair ready to member 相关逻辑。 |
| [L194](../src/server/signaling/SignalServer.Internal.h#L194) | `SendRoomPairReady` | 声明 | `void SendRoomPairReady(const RoomState& room, const RoomPairState& pair)` | 发送或发布 send room pair ready 相关逻辑。 |
| [L196](../src/server/signaling/SignalServer.Internal.h#L196) | `SendRoomPairsReadyForMember` | 声明 | `void SendRoomPairsReadyForMember(const RoomState& room, const QString& deviceId)` | 发送或发布 send room pairs ready for member 相关逻辑。 |
| [L198](../src/server/signaling/SignalServer.Internal.h#L198) | `SendRoomPairClosedToMember` | 声明 | `void SendRoomPairClosedToMember(const RoomState& room, const RoomPairState& pair, const QString& deviceId, const QString& initiatorDeviceId, const QString& reasonCode)` | 发送或发布 send room pair closed to member 相关逻辑。 |
| [L203](../src/server/signaling/SignalServer.Internal.h#L203) | `CloseRoomPair` | 声明 | `void CloseRoomPair(RoomState& room, const QString& pairId, const QString& initiatorDeviceId, const QString& reasonCode)` | 关闭并清理 close room pair 相关逻辑。 |
| [L207](../src/server/signaling/SignalServer.Internal.h#L207) | `CloseRoomPairsForMember` | 声明 | `void CloseRoomPairsForMember(RoomState& room, const QString& deviceId, const QString& initiatorDeviceId, const QString& reasonCode)` | 关闭并清理 close room pairs for member 相关逻辑。 |
| [L211](../src/server/signaling/SignalServer.Internal.h#L211) | `CloseAllRoomPairs` | 声明 | `void CloseAllRoomPairs(RoomState& room, const QString& initiatorDeviceId, const QString& reasonCode)` | 关闭并清理 close all room pairs 相关逻辑。 |
| [L214](../src/server/signaling/SignalServer.Internal.h#L214) | `SerializeRoom` | 声明 | `QJsonObject SerializeRoom(const RoomState& room) const` | 编码 serialize room 相关逻辑。 |
| [L215](../src/server/signaling/SignalServer.Internal.h#L215) | `SendRoomReady` | 声明 | `void SendRoomReady(QWebSocket* socket, const RoomState& room, const QString& recoveryToken)` | 发送或发布 send room ready 相关逻辑。 |
| [L218](../src/server/signaling/SignalServer.Internal.h#L218) | `BroadcastRoomState` | 声明 | `void BroadcastRoomState(const RoomState& room)` | 实现 broadcast room state 对应的业务或工具逻辑。 |
| [L219](../src/server/signaling/SignalServer.Internal.h#L219) | `SendRoomJoinPending` | 声明 | `void SendRoomJoinPending(QWebSocket* socket, const QString& roomId, const QString& requestId)` | 发送或发布 send room join pending 相关逻辑。 |
| [L222](../src/server/signaling/SignalServer.Internal.h#L222) | `SendRoomJoinResult` | 声明 | `void SendRoomJoinResult(QWebSocket* socket, const QString& roomId, const QString& requestId, bool accepted, const QString& reasonCode, const QString& reasonMessage)` | 发送或发布 send room join result 相关逻辑。 |
| [L228](../src/server/signaling/SignalServer.Internal.h#L228) | `SendRoomClosed` | 声明 | `void SendRoomClosed(QWebSocket* socket, const QString& roomId, const QString& initiatorDeviceId, const QString& reasonCode)` | 发送或发布 send room closed 相关逻辑。 |
| [L232](../src/server/signaling/SignalServer.Internal.h#L232) | `CloseRoom` | 声明 | `void CloseRoom(const QString& roomId, const QString& initiatorDeviceId, const QString& reasonCode)` | 关闭并清理 close room 相关逻辑。 |
| [L235](../src/server/signaling/SignalServer.Internal.h#L235) | `CancelPendingRoomJoinForDevice` | 声明 | `void CancelPendingRoomJoinForDevice(const QString& deviceId)` | 判断 cancel pending room join for device 相关逻辑。 |
| [L236](../src/server/signaling/SignalServer.Internal.h#L236) | `HandleSessionRequest` | 声明 | `void HandleSessionRequest(ClientState* requester, const QJsonObject& payload)` | 接收并处理 handle session request 相关逻辑。 |
| [L238](../src/server/signaling/SignalServer.Internal.h#L238) | `HandleSessionResponse` | 声明 | `void HandleSessionResponse(ClientState* target, const QString& sessionId, const QJsonObject& payload)` | 接收并处理 handle session response 相关逻辑。 |
| [L241](../src/server/signaling/SignalServer.Internal.h#L241) | `SendSessionReady` | 声明 | `void SendSessionReady(QWebSocket* socket, const QString& sessionId, const QString& peerDeviceId, const QString& recoveryToken)` | 发送或发布 send session ready 相关逻辑。 |
| [L245](../src/server/signaling/SignalServer.Internal.h#L245) | `HandleSessionResume` | 声明 | `void HandleSessionResume(ClientState* client, const QString& sessionId, const QJsonObject& payload)` | 接收并处理 handle session resume 相关逻辑。 |
| [L248](../src/server/signaling/SignalServer.Internal.h#L248) | `SendSessionResumed` | 声明 | `void SendSessionResumed(QWebSocket* socket, const QString& sessionId, const QString& peerDeviceId, const QString& resumedDeviceId)` | 发送或发布 send session resumed 相关逻辑。 |
| [L252](../src/server/signaling/SignalServer.Internal.h#L252) | `RelayRoomPairMessage` | 声明 | `bool RelayRoomPairMessage(ClientState* sender, const QString& pairId, const QString& type, const QJsonObject& payload)` | 发送或发布 relay room pair message 相关逻辑。 |
| [L256](../src/server/signaling/SignalServer.Internal.h#L256) | `RelaySessionMessage` | 声明 | `void RelaySessionMessage(ClientState* sender, const QString& sessionId, const QString& type, const QJsonObject& payload)` | 发送或发布 relay session message 相关逻辑。 |
| [L260](../src/server/signaling/SignalServer.Internal.h#L260) | `CloseSession` | 声明 | `void CloseSession(ClientState* sender, const QString& sessionId, const QString& type, const QJsonObject& payload)` | 关闭并清理 close session 相关逻辑。 |
| [L264](../src/server/signaling/SignalServer.Internal.h#L264) | `SendSessionEnded` | 声明 | `void SendSessionEnded(QWebSocket* socket, const QString& sessionId, const QString& initiatorDeviceId, const QString& reasonCode, const QString& disposition)` | 发送或发布 send session ended 相关逻辑。 |
| [L269](../src/server/signaling/SignalServer.Internal.h#L269) | `OnDisconnected` | 声明 | `void OnDisconnected(QWebSocket* socket)` | 接收并处理 on disconnected 相关逻辑。 |
| [L270](../src/server/signaling/SignalServer.Internal.h#L270) | `MarkDeviceDisconnected` | 声明 | `void MarkDeviceDisconnected(const QString& deviceId)` | 实现 mark device disconnected 对应的业务或工具逻辑。 |
| [L271](../src/server/signaling/SignalServer.Internal.h#L271) | `SweepIdleClients` | 声明 | `void SweepIdleClients()` | 实现 sweep idle clients 对应的业务或工具逻辑。 |
| [L272](../src/server/signaling/SignalServer.Internal.h#L272) | `SweepPendingSessions` | 声明 | `void SweepPendingSessions(qint64 now)` | 实现 sweep pending sessions 对应的业务或工具逻辑。 |
| [L273](../src/server/signaling/SignalServer.Internal.h#L273) | `SweepRecoverySessions` | 声明 | `void SweepRecoverySessions(qint64 now)` | 实现 sweep recovery sessions 对应的业务或工具逻辑。 |
| [L274](../src/server/signaling/SignalServer.Internal.h#L274) | `SweepRoomJoinRequests` | 声明 | `void SweepRoomJoinRequests(qint64 now)` | 实现 sweep room join requests 对应的业务或工具逻辑。 |
| [L275](../src/server/signaling/SignalServer.Internal.h#L275) | `SweepRoomControlRequests` | 声明 | `void SweepRoomControlRequests(qint64 now)` | 实现 sweep room control requests 对应的业务或工具逻辑。 |
| [L276](../src/server/signaling/SignalServer.Internal.h#L276) | `SweepRoomScreenShareSwitchRequests` | 声明 | `void SweepRoomScreenShareSwitchRequests(qint64 now)` | 实现 sweep room screen share switch requests 对应的业务或工具逻辑。 |
| [L277](../src/server/signaling/SignalServer.Internal.h#L277) | `SweepRoomRecoveries` | 声明 | `void SweepRoomRecoveries(qint64 now)` | 实现 sweep room recoveries 对应的业务或工具逻辑。 |
| [L278](../src/server/signaling/SignalServer.Internal.h#L278) | `FindClient` | 声明 | `ClientState* FindClient(QWebSocket* socket)` | 查询并返回 find client 相关逻辑。 |
| [L279](../src/server/signaling/SignalServer.Internal.h#L279) | `RememberMessageId` | 定义 | `bool RememberMessageId(ClientState* client, const QByteArray& messageId)` | 实现 remember message id 对应的业务或工具逻辑。 |
| [L297](../src/server/signaling/SignalServer.Internal.h#L297) | `CompleteClientAuthentication` | 定义 | `void CompleteClientAuthentication(ClientState* client)` | 实现 complete client authentication 对应的业务或工具逻辑。 |
| [L308](../src/server/signaling/SignalServer.Internal.h#L308) | `DeviceHasSession` | 定义 | `bool DeviceHasSession(const QString& deviceId) const` | 实现 device has session 对应的业务或工具逻辑。 |
| [L313](../src/server/signaling/SignalServer.Internal.h#L313) | `InsertSession` | 定义 | `bool InsertSession(SessionState session)` | 实现 insert session 对应的业务或工具逻辑。 |
| [L318](../src/server/signaling/SignalServer.Internal.h#L318) | `RemoveSession` | 定义 | `void RemoveSession(const QString& sessionId)` | 重置或移除 remove session 相关逻辑。 |
| [L323](../src/server/signaling/SignalServer.Internal.h#L323) | `ClearSessions` | 定义 | `void ClearSessions()` | 重置或移除 clear sessions 相关逻辑。 |
| [L328](../src/server/signaling/SignalServer.Internal.h#L328) | `AddAccountConnection` | 定义 | `void AddAccountConnection(ClientState* client)` | 实现 add account connection 对应的业务或工具逻辑。 |
| [L336](../src/server/signaling/SignalServer.Internal.h#L336) | `RemoveAccountConnection` | 定义 | `void RemoveAccountConnection(const ClientState* client)` | 重置或移除 remove account connection 相关逻辑。 |
| [L351](../src/server/signaling/SignalServer.Internal.h#L351) | `DeviceHasRoom` | 定义 | `bool DeviceHasRoom(const QString& deviceId) const` | 实现 device has room 对应的业务或工具逻辑。 |
| [L356](../src/server/signaling/SignalServer.Internal.h#L356) | `ReleaseDisconnectedRoomForFreshStart` | 定义 | `bool ReleaseDisconnectedRoomForFreshStart(const QString& deviceId)` | 释放或取消 release disconnected room for fresh start 相关逻辑。 |
| [L398](../src/server/signaling/SignalServer.Internal.h#L398) | `DeleteLocalAccount` | 定义 | `bool DeleteLocalAccount(const QString& subject, QString* error)` | 实现 delete local account 对应的业务或工具逻辑。 |
| [L486](../src/server/signaling/SignalServer.Internal.h#L486) | `SendError` | 定义 | `void SendError(QWebSocket* socket, const QString& sessionId, const QString& code, const QString& message)` | 发送或发布 send error 相关逻辑。 |
| [L497](../src/server/signaling/SignalServer.Internal.h#L497) | `PeerRateLimitKey` | 定义 | `QString PeerRateLimitKey(const QWebSocket* socket) const` | 实现 peer rate limit key 对应的业务或工具逻辑。 |
| [L512](../src/server/signaling/SignalServer.Internal.h#L512) | `AllowAuthenticationAttempt` | 定义 | `bool AllowAuthenticationAttempt(QWebSocket* socket, qint64 now)` | 实现 allow authentication attempt 对应的业务或工具逻辑。 |
| [L518](../src/server/signaling/SignalServer.Internal.h#L518) | `AllowRoomAvailabilityQuery` | 定义 | `bool AllowRoomAvailabilityQuery(ClientState* client, int requestedRoomCount, qint64 now)` | 实现 allow room availability query 对应的业务或工具逻辑。 |
| [L554](../src/server/signaling/SignalServer.Internal.h#L554) | `AllowRoomJoinAttempt` | 定义 | `bool AllowRoomJoinAttempt(ClientState* client, qint64 now)` | 实现 allow room join attempt 对应的业务或工具逻辑。 |
| [L583](../src/server/signaling/SignalServer.Internal.h#L583) | `AllowAccountDeletion` | 定义 | `bool AllowAccountDeletion(ClientState* client, qint64 now)` | 实现 allow account deletion 对应的业务或工具逻辑。 |
| [L590](../src/server/signaling/SignalServer.Internal.h#L590) | `AllowAssistanceAttempt` | 定义 | `bool AllowAssistanceAttempt(ClientState* client, const QString& targetDeviceId, qint64 now)` | 实现 allow assistance attempt 对应的业务或工具逻辑。 |
| [L616](../src/server/signaling/SignalServer.Internal.h#L616) | `PruneRateLimiters` | 定义 | `void PruneRateLimiters(qint64 now)` | 实现 prune rate limiters 对应的业务或工具逻辑。 |
| [L631](../src/server/signaling/SignalServer.Internal.h#L631) | `ClearRateLimiters` | 定义 | `void ClearRateLimiters()` | 重置或移除 clear rate limiters 相关逻辑。 |
| [L646](../src/server/signaling/SignalServer.Internal.h#L646) | `RateLimiterKeyCount` | 定义 | `qsizetype RateLimiterKeyCount() const` | 实现 rate limiter key count 对应的业务或工具逻辑。 |
| [L661](../src/server/signaling/SignalServer.Internal.h#L661) | `ProbeEventLoopLag` | 定义 | `void ProbeEventLoopLag()` | 实现 probe event loop lag 对应的业务或工具逻辑。 |
| [L675](../src/server/signaling/SignalServer.Internal.h#L675) | `WriteDiagnostics` | 定义 | `void WriteDiagnostics()` | 保存或写入 write diagnostics 相关逻辑。 |
| [L835](../src/server/signaling/SignalServer.Internal.h#L835) | `SendEnvelope` | 定义 | `void SendEnvelope(QWebSocket* socket, const QString& type, const QString& sessionId, const QJsonObject& payload)` | 发送或发布 send envelope 相关逻辑。 |

## `src/server/signaling/SignalServer.LegacySession.cpp`

[打开源码](../src/server/signaling/SignalServer.LegacySession.cpp) · **文件作用：** 实现 signal server legacy session 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/server/signaling/SignalServer.LegacySession.cpp#L9) | `SignalServer::Impl::HandleSessionRequest` | 定义 | `void SignalServer::Impl::HandleSessionRequest(ClientState* requester, const QJsonObject& payload)` | 接收并处理 handle session request 相关逻辑。 |
| [L130](../src/server/signaling/SignalServer.LegacySession.cpp#L130) | `SignalServer::Impl::HandleSessionResponse` | 定义 | `void SignalServer::Impl::HandleSessionResponse(ClientState* target, const QString& sessionId, const QJsonObject& payload)` | 接收并处理 handle session response 相关逻辑。 |
| [L190](../src/server/signaling/SignalServer.LegacySession.cpp#L190) | `SignalServer::Impl::SendSessionReady` | 定义 | `void SignalServer::Impl::SendSessionReady(QWebSocket* socket, const QString& sessionId, const QString& peerDeviceId, const QString& recoveryToken)` | 发送或发布 send session ready 相关逻辑。 |
| [L203](../src/server/signaling/SignalServer.LegacySession.cpp#L203) | `SignalServer::Impl::HandleSessionResume` | 定义 | `void SignalServer::Impl::HandleSessionResume(ClientState* client, const QString& sessionId, const QJsonObject& payload)` | 接收并处理 handle session resume 相关逻辑。 |
| [L259](../src/server/signaling/SignalServer.LegacySession.cpp#L259) | `SignalServer::Impl::SendSessionResumed` | 定义 | `void SignalServer::Impl::SendSessionResumed(QWebSocket* socket, const QString& sessionId, const QString& peerDeviceId, const QString& resumedDeviceId)` | 发送或发布 send session resumed 相关逻辑。 |

## `src/server/signaling/SignalServer.Lifecycle.cpp`

[打开源码](../src/server/signaling/SignalServer.Lifecycle.cpp) · **文件作用：** 实现 signal server lifecycle 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/server/signaling/SignalServer.Lifecycle.cpp#L9) | `SignalServer::Impl::Impl` | 定义 | `SignalServer::Impl::Impl(SignalServerConfig config) : config_(std::move(config)) , tokenService_(config_.tokenSecret) , authenticationIpRateLimiter_(SlidingWindowRateLimitPolicy` | 构造并初始化 Impl 实例。 |
| [L16](../src/server/signaling/SignalServer.Lifecycle.cpp#L16) | `availabilityIpRateLimiter_` | 定义 | `, availabilityIpRateLimiter_(kAvailabilityIpRateLimit) , availabilityUserRateLimiter_(kAvailabilityUserRateLimit) , availabilityDeviceRateLimiter_(kAvailabilityDeviceRateLimit) , roomJoinIpRateLimiter_(kRoomJoinIpRate...` | 实现 availability ip rate limiter 对应的业务或工具逻辑。 |
| [L46](../src/server/signaling/SignalServer.Lifecycle.cpp#L46) | `SignalServer::Impl::Start` | 定义 | `bool SignalServer::Impl::Start(QString* error)` | 启动 start 相关逻辑。 |
| [L240](../src/server/signaling/SignalServer.Lifecycle.cpp#L240) | `SignalServer::Impl::Stop` | 定义 | `void SignalServer::Impl::Stop()` | 停止 stop 相关逻辑。 |
| [L280](../src/server/signaling/SignalServer.Lifecycle.cpp#L280) | `SignalServer::Impl::IsListening` | 定义 | `bool SignalServer::Impl::IsListening() const` | 判断 is listening 相关逻辑。 |
| [L285](../src/server/signaling/SignalServer.Lifecycle.cpp#L285) | `SignalServer::Impl::ServerPort` | 定义 | `quint16 SignalServer::Impl::ServerPort() const` | 实现 server port 对应的业务或工具逻辑。 |
| [L290](../src/server/signaling/SignalServer.Lifecycle.cpp#L290) | `SignalServer::Impl::WebhookPort` | 定义 | `quint16 SignalServer::Impl::WebhookPort() const` | 实现 webhook port 对应的业务或工具逻辑。 |

## `src/server/signaling/SignalServer.OwnedDevices.cpp`

[打开源码](../src/server/signaling/SignalServer.OwnedDevices.cpp) · **文件作用：** 实现 signal server owned devices 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/server/signaling/SignalServer.OwnedDevices.cpp#L9) | `SignalServer::Impl::BuildOwnedDevicesPayloadFromRecords` | 定义 | `QJsonObject SignalServer::Impl::BuildOwnedDevicesPayloadFromRecords( ClientState* recipient, quint64 revision, const QList<remote::server_persistence::OwnedDeviceRecord>& owned, const QString& storeError)` | 创建或初始化 build owned devices payload from records 相关逻辑。 |
| [L52](../src/server/signaling/SignalServer.OwnedDevices.cpp#L52) | `SignalServer::Impl::BuildOwnedDevicesPayload` | 定义 | `QJsonObject SignalServer::Impl::BuildOwnedDevicesPayload(ClientState* recipient, quint64 revision)` | 创建或初始化 build owned devices payload 相关逻辑。 |
| [L62](../src/server/signaling/SignalServer.OwnedDevices.cpp#L62) | `SignalServer::Impl::HandleMyDevicesRequest` | 定义 | `void SignalServer::Impl::HandleMyDevicesRequest(ClientState* client)` | 接收并处理 handle my devices request 相关逻辑。 |
| [L76](../src/server/signaling/SignalServer.OwnedDevices.cpp#L76) | `SignalServer::Impl::BroadcastOwnedDevices` | 定义 | `void SignalServer::Impl::BroadcastOwnedDevices(qint64 trustedUserId)` | 实现 broadcast owned devices 对应的业务或工具逻辑。 |

## `src/server/signaling/SignalServer.Recovery.cpp`

[打开源码](../src/server/signaling/SignalServer.Recovery.cpp) · **文件作用：** 实现 signal server recovery 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/server/signaling/SignalServer.Recovery.cpp#L9) | `SignalServer::Impl::OnDisconnected` | 定义 | `void SignalServer::Impl::OnDisconnected(QWebSocket* socket)` | 接收并处理 on disconnected 相关逻辑。 |
| [L54](../src/server/signaling/SignalServer.Recovery.cpp#L54) | `SignalServer::Impl::MarkDeviceDisconnected` | 定义 | `void SignalServer::Impl::MarkDeviceDisconnected(const QString& deviceId)` | 实现 mark device disconnected 对应的业务或工具逻辑。 |
| [L136](../src/server/signaling/SignalServer.Recovery.cpp#L136) | `SignalServer::Impl::SweepIdleClients` | 定义 | `void SignalServer::Impl::SweepIdleClients()` | 实现 sweep idle clients 对应的业务或工具逻辑。 |
| [L166](../src/server/signaling/SignalServer.Recovery.cpp#L166) | `SignalServer::Impl::SweepPendingSessions` | 定义 | `void SignalServer::Impl::SweepPendingSessions(qint64 now)` | 实现 sweep pending sessions 对应的业务或工具逻辑。 |
| [L203](../src/server/signaling/SignalServer.Recovery.cpp#L203) | `SignalServer::Impl::SweepRecoverySessions` | 定义 | `void SignalServer::Impl::SweepRecoverySessions(qint64 now)` | 实现 sweep recovery sessions 对应的业务或工具逻辑。 |
| [L243](../src/server/signaling/SignalServer.Recovery.cpp#L243) | `SignalServer::Impl::SweepRoomJoinRequests` | 定义 | `void SignalServer::Impl::SweepRoomJoinRequests(qint64 now)` | 实现 sweep room join requests 对应的业务或工具逻辑。 |
| [L295](../src/server/signaling/SignalServer.Recovery.cpp#L295) | `SignalServer::Impl::SweepRoomControlRequests` | 定义 | `void SignalServer::Impl::SweepRoomControlRequests(qint64 now)` | 实现 sweep room control requests 对应的业务或工具逻辑。 |
| [L309](../src/server/signaling/SignalServer.Recovery.cpp#L309) | `SignalServer::Impl::SweepRoomScreenShareSwitchRequests` | 定义 | `void SignalServer::Impl::SweepRoomScreenShareSwitchRequests(qint64 now)` | 实现 sweep room screen share switch requests 对应的业务或工具逻辑。 |
| [L324](../src/server/signaling/SignalServer.Recovery.cpp#L324) | `SignalServer::Impl::SweepRoomRecoveries` | 定义 | `void SignalServer::Impl::SweepRoomRecoveries(qint64 now)` | 实现 sweep room recoveries 对应的业务或工具逻辑。 |
| [L394](../src/server/signaling/SignalServer.Recovery.cpp#L394) | `SignalServer::Impl::FindClient` | 定义 | `SignalServer::Impl::ClientState* SignalServer::Impl::FindClient( QWebSocket* socket)` | 查询并返回 find client 相关逻辑。 |

## `src/server/signaling/SignalServer.Relay.cpp`

[打开源码](../src/server/signaling/SignalServer.Relay.cpp) · **文件作用：** 实现 signal server relay 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/server/signaling/SignalServer.Relay.cpp#L9) | `SignalServer::Impl::RelayRoomPairMessage` | 定义 | `bool SignalServer::Impl::RelayRoomPairMessage(ClientState* sender, const QString& pairId, const QString& type, const QJsonObject& payload)` | 发送或发布 relay room pair message 相关逻辑。 |
| [L98](../src/server/signaling/SignalServer.Relay.cpp#L98) | `SignalServer::Impl::RelaySessionMessage` | 定义 | `void SignalServer::Impl::RelaySessionMessage(ClientState* sender, const QString& sessionId, const QString& type, const QJsonObject& payload)` | 发送或发布 relay session message 相关逻辑。 |
| [L154](../src/server/signaling/SignalServer.Relay.cpp#L154) | `SignalServer::Impl::CloseSession` | 定义 | `void SignalServer::Impl::CloseSession(ClientState* sender, const QString& sessionId, const QString& type, const QJsonObject& payload)` | 关闭并清理 close session 相关逻辑。 |
| [L206](../src/server/signaling/SignalServer.Relay.cpp#L206) | `SignalServer::Impl::SendSessionEnded` | 定义 | `void SignalServer::Impl::SendSessionEnded(QWebSocket* socket, const QString& sessionId, const QString& initiatorDeviceId, const QString& reasonCode, const QString& disposition)` | 发送或发布 send session ended 相关逻辑。 |

## `src/server/signaling/SignalServer.RoomLeases.cpp`

[打开源码](../src/server/signaling/SignalServer.RoomLeases.cpp) · **文件作用：** 实现 signal server room leases 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/server/signaling/SignalServer.RoomLeases.cpp#L9) | `SignalServer::Impl::IsOnlineRoomMember` | 定义 | `bool SignalServer::Impl::IsOnlineRoomMember(const RoomState& room, const QString& deviceId) const` | 判断 is online room member 相关逻辑。 |
| [L18](../src/server/signaling/SignalServer.RoomLeases.cpp#L18) | `SignalServer::Impl::SendRoomScreenShareGranted` | 定义 | `void SignalServer::Impl::SendRoomScreenShareGranted(QWebSocket* socket, const RoomState& room)` | 发送或发布 send room screen share granted 相关逻辑。 |
| [L34](../src/server/signaling/SignalServer.RoomLeases.cpp#L34) | `SignalServer::Impl::SendRoomScreenShareSwitchPending` | 定义 | `void SignalServer::Impl::SendRoomScreenShareSwitchPending(QWebSocket* socket, const RoomState& room)` | 发送或发布 send room screen share switch pending 相关逻辑。 |
| [L52](../src/server/signaling/SignalServer.RoomLeases.cpp#L52) | `SignalServer::Impl::SendRoomScreenShareSwitchRequested` | 定义 | `void SignalServer::Impl::SendRoomScreenShareSwitchRequested(QWebSocket* socket, const RoomState& room)` | 发送或发布 send room screen share switch requested 相关逻辑。 |
| [L75](../src/server/signaling/SignalServer.RoomLeases.cpp#L75) | `SignalServer::Impl::SendRoomScreenShareSwitchResult` | 定义 | `void SignalServer::Impl::SendRoomScreenShareSwitchResult( QWebSocket* socket, const RoomState& room, const QString& requestId, bool accepted, const QString& reasonCode, const QString& reasonMessage)` | 发送或发布 send room screen share switch result 相关逻辑。 |
| [L97](../src/server/signaling/SignalServer.RoomLeases.cpp#L97) | `SignalServer::Impl::CancelPendingRoomScreenShareSwitch` | 定义 | `void SignalServer::Impl::CancelPendingRoomScreenShareSwitch( RoomState& room, const QString& reasonCode, const QString& reasonMessage)` | 判断 cancel pending room screen share switch 相关逻辑。 |
| [L123](../src/server/signaling/SignalServer.RoomLeases.cpp#L123) | `SignalServer::Impl::SendRoomControlResult` | 定义 | `void SignalServer::Impl::SendRoomControlResult(QWebSocket* socket, const RoomState& room, const QString& requestId, bool accepted, const QString& reasonCode, const QString& reasonMessage)` | 发送或发布 send room control result 相关逻辑。 |
| [L143](../src/server/signaling/SignalServer.RoomLeases.cpp#L143) | `SignalServer::Impl::SendRoomControlGranted` | 定义 | `void SignalServer::Impl::SendRoomControlGranted(QWebSocket* socket, const RoomState& room)` | 发送或发布 send room control granted 相关逻辑。 |
| [L162](../src/server/signaling/SignalServer.RoomLeases.cpp#L162) | `SignalServer::Impl::SendRoomControlRevoked` | 定义 | `void SignalServer::Impl::SendRoomControlRevoked(QWebSocket* socket, const RoomState& room, const QString& initiatorDeviceId, const QString& reasonCode)` | 发送或发布 send room control revoked 相关逻辑。 |
| [L179](../src/server/signaling/SignalServer.RoomLeases.cpp#L179) | `SignalServer::Impl::CancelPendingRoomControl` | 定义 | `void SignalServer::Impl::CancelPendingRoomControl(RoomState& room, const QString& reasonCode, const QString& reasonMessage)` | 判断 cancel pending room control 相关逻辑。 |
| [L205](../src/server/signaling/SignalServer.RoomLeases.cpp#L205) | `SignalServer::Impl::RevokeRoomControl` | 定义 | `void SignalServer::Impl::RevokeRoomControl(RoomState& room, const QString& initiatorDeviceId, const QString& reasonCode)` | 实现 revoke room control 对应的业务或工具逻辑。 |
| [L227](../src/server/signaling/SignalServer.RoomLeases.cpp#L227) | `SignalServer::Impl::ResetRoomScreenShare` | 定义 | `void SignalServer::Impl::ResetRoomScreenShare(RoomState& room, const QString& initiatorDeviceId, const QString& reasonCode)` | 重置或移除 reset room screen share 相关逻辑。 |
| [L249](../src/server/signaling/SignalServer.RoomLeases.cpp#L249) | `SignalServer::Impl::HandleRoomScreenShareRequest` | 定义 | `void SignalServer::Impl::HandleRoomScreenShareRequest(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room screen share request 相关逻辑。 |
| [L339](../src/server/signaling/SignalServer.RoomLeases.cpp#L339) | `SignalServer::Impl::HandleRoomScreenShareReady` | 定义 | `void SignalServer::Impl::HandleRoomScreenShareReady(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room screen share ready 相关逻辑。 |
| [L371](../src/server/signaling/SignalServer.RoomLeases.cpp#L371) | `SignalServer::Impl::HandleRoomScreenShareSwitchResponse` | 定义 | `void SignalServer::Impl::HandleRoomScreenShareSwitchResponse( ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room screen share switch response 相关逻辑。 |
| [L456](../src/server/signaling/SignalServer.RoomLeases.cpp#L456) | `SignalServer::Impl::HandleRoomScreenShareSwitchCancel` | 定义 | `void SignalServer::Impl::HandleRoomScreenShareSwitchCancel( ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room screen share switch cancel 相关逻辑。 |
| [L494](../src/server/signaling/SignalServer.RoomLeases.cpp#L494) | `SignalServer::Impl::HandleRoomScreenShareStop` | 定义 | `void SignalServer::Impl::HandleRoomScreenShareStop(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room screen share stop 相关逻辑。 |
| [L530](../src/server/signaling/SignalServer.RoomLeases.cpp#L530) | `SignalServer::Impl::HandleRoomControlRequest` | 定义 | `void SignalServer::Impl::HandleRoomControlRequest(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room control request 相关逻辑。 |
| [L601](../src/server/signaling/SignalServer.RoomLeases.cpp#L601) | `SignalServer::Impl::HandleRoomControlResponse` | 定义 | `void SignalServer::Impl::HandleRoomControlResponse(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room control response 相关逻辑。 |
| [L675](../src/server/signaling/SignalServer.RoomLeases.cpp#L675) | `SignalServer::Impl::HandleRoomControlRelease` | 定义 | `void SignalServer::Impl::HandleRoomControlRelease(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room control release 相关逻辑。 |
| [L708](../src/server/signaling/SignalServer.RoomLeases.cpp#L708) | `SignalServer::Impl::SerializeIceServers` | 定义 | `QJsonArray SignalServer::Impl::SerializeIceServers() const` | 编码 serialize ice servers 相关逻辑。 |

## `src/server/signaling/SignalServer.RoomMembership.cpp`

[打开源码](../src/server/signaling/SignalServer.RoomMembership.cpp) · **文件作用：** 实现 signal server room membership 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/server/signaling/SignalServer.RoomMembership.cpp#L9) | `SignalServer::Impl::HandleRegister` | 定义 | `void SignalServer::Impl::HandleRegister(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle register 相关逻辑。 |
| [L82](../src/server/signaling/SignalServer.RoomMembership.cpp#L82) | `SignalServer::Impl::HandleRoomCreate` | 定义 | `void SignalServer::Impl::HandleRoomCreate(ClientState* creator, const QJsonObject& payload)` | 接收并处理 handle room create 相关逻辑。 |
| [L153](../src/server/signaling/SignalServer.RoomMembership.cpp#L153) | `SignalServer::Impl::HandleRoomAvailabilityQuery` | 定义 | `void SignalServer::Impl::HandleRoomAvailabilityQuery(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room availability query 相关逻辑。 |
| [L213](../src/server/signaling/SignalServer.RoomMembership.cpp#L213) | `SignalServer::Impl::HandleRoomJoinRequest` | 定义 | `void SignalServer::Impl::HandleRoomJoinRequest(ClientState* requester, const QJsonObject& payload)` | 接收并处理 handle room join request 相关逻辑。 |
| [L315](../src/server/signaling/SignalServer.RoomMembership.cpp#L315) | `SignalServer::Impl::HandleRoomJoinResponse` | 定义 | `void SignalServer::Impl::HandleRoomJoinResponse(ClientState* owner, const QJsonObject& payload)` | 接收并处理 handle room join response 相关逻辑。 |
| [L445](../src/server/signaling/SignalServer.RoomMembership.cpp#L445) | `SignalServer::Impl::HandleRoomSetCapacity` | 定义 | `void SignalServer::Impl::HandleRoomSetCapacity(ClientState* owner, const QJsonObject& payload)` | 接收并处理 handle room set capacity 相关逻辑。 |
| [L497](../src/server/signaling/SignalServer.RoomMembership.cpp#L497) | `SignalServer::Impl::HandleRoomLeave` | 定义 | `void SignalServer::Impl::HandleRoomLeave(ClientState* memberClient, const QJsonObject& payload)` | 接收并处理 handle room leave 相关逻辑。 |
| [L573](../src/server/signaling/SignalServer.RoomMembership.cpp#L573) | `SignalServer::Impl::HandleRoomMediaState` | 定义 | `void SignalServer::Impl::HandleRoomMediaState(ClientState* memberClient, const QJsonObject& payload)` | 接收并处理 handle room media state 相关逻辑。 |
| [L616](../src/server/signaling/SignalServer.RoomMembership.cpp#L616) | `SignalServer::Impl::HandleRoomResume` | 定义 | `void SignalServer::Impl::HandleRoomResume(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room resume 相关逻辑。 |

## `src/server/signaling/SignalServer.RoomPairs.cpp`

[打开源码](../src/server/signaling/SignalServer.RoomPairs.cpp) · **文件作用：** 实现 signal server room pairs 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/server/signaling/SignalServer.RoomPairs.cpp#L9) | `SignalServer::Impl::CreateRoomPair` | 定义 | `void SignalServer::Impl::CreateRoomPair(RoomState& room, const QString& firstDeviceId, const QString& secondDeviceId)` | 创建或初始化 create room pair 相关逻辑。 |
| [L39](../src/server/signaling/SignalServer.RoomPairs.cpp#L39) | `SignalServer::Impl::PairPeerDeviceId` | 定义 | `QString SignalServer::Impl::PairPeerDeviceId(const RoomPairState& pair, const QString& deviceId) const` | 实现 pair peer device id 对应的业务或工具逻辑。 |
| [L51](../src/server/signaling/SignalServer.RoomPairs.cpp#L51) | `SignalServer::Impl::RoomPairMembersAreOnline` | 定义 | `bool SignalServer::Impl::RoomPairMembersAreOnline(const RoomState& room, const RoomPairState& pair) const` | 实现 room pair members are online 对应的业务或工具逻辑。 |
| [L64](../src/server/signaling/SignalServer.RoomPairs.cpp#L64) | `SignalServer::Impl::SendRoomPairReadyToMember` | 定义 | `void SignalServer::Impl::SendRoomPairReadyToMember(const RoomState& room, const RoomPairState& pair, const QString& deviceId)` | 发送或发布 send room pair ready to member 相关逻辑。 |
| [L84](../src/server/signaling/SignalServer.RoomPairs.cpp#L84) | `SignalServer::Impl::SendRoomPairReady` | 定义 | `void SignalServer::Impl::SendRoomPairReady(const RoomState& room, const RoomPairState& pair)` | 发送或发布 send room pair ready 相关逻辑。 |
| [L101](../src/server/signaling/SignalServer.RoomPairs.cpp#L101) | `SignalServer::Impl::SendRoomPairsReadyForMember` | 定义 | `void SignalServer::Impl::SendRoomPairsReadyForMember(const RoomState& room, const QString& deviceId)` | 发送或发布 send room pairs ready for member 相关逻辑。 |
| [L115](../src/server/signaling/SignalServer.RoomPairs.cpp#L115) | `SignalServer::Impl::SendRoomPairClosedToMember` | 定义 | `void SignalServer::Impl::SendRoomPairClosedToMember(const RoomState& room, const RoomPairState& pair, const QString& deviceId, const QString& initiatorDeviceId, const QString& reasonCode)` | 发送或发布 send room pair closed to member 相关逻辑。 |
| [L136](../src/server/signaling/SignalServer.RoomPairs.cpp#L136) | `SignalServer::Impl::CloseRoomPair` | 定义 | `void SignalServer::Impl::CloseRoomPair(RoomState& room, const QString& pairId, const QString& initiatorDeviceId, const QString& reasonCode)` | 关闭并清理 close room pair 相关逻辑。 |
| [L157](../src/server/signaling/SignalServer.RoomPairs.cpp#L157) | `SignalServer::Impl::CloseRoomPairsForMember` | 定义 | `void SignalServer::Impl::CloseRoomPairsForMember(RoomState& room, const QString& deviceId, const QString& initiatorDeviceId, const QString& reasonCode)` | 关闭并清理 close room pairs for member 相关逻辑。 |
| [L175](../src/server/signaling/SignalServer.RoomPairs.cpp#L175) | `SignalServer::Impl::CloseAllRoomPairs` | 定义 | `void SignalServer::Impl::CloseAllRoomPairs(RoomState& room, const QString& initiatorDeviceId, const QString& reasonCode)` | 关闭并清理 close all room pairs 相关逻辑。 |

## `src/server/signaling/SignalServer.RoomState.cpp`

[打开源码](../src/server/signaling/SignalServer.RoomState.cpp) · **文件作用：** 实现 signal server room state 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/server/signaling/SignalServer.RoomState.cpp#L9) | `SignalServer::Impl::SerializeRoom` | 定义 | `QJsonObject SignalServer::Impl::SerializeRoom(const RoomState& room) const` | 编码 serialize room 相关逻辑。 |
| [L49](../src/server/signaling/SignalServer.RoomState.cpp#L49) | `SignalServer::Impl::SendRoomReady` | 定义 | `void SignalServer::Impl::SendRoomReady(QWebSocket* socket, const RoomState& room, const QString& recoveryToken)` | 发送或发布 send room ready 相关逻辑。 |
| [L59](../src/server/signaling/SignalServer.RoomState.cpp#L59) | `SignalServer::Impl::BroadcastRoomState` | 定义 | `void SignalServer::Impl::BroadcastRoomState(const RoomState& room)` | 实现 broadcast room state 对应的业务或工具逻辑。 |
| [L76](../src/server/signaling/SignalServer.RoomState.cpp#L76) | `SignalServer::Impl::SendRoomJoinPending` | 定义 | `void SignalServer::Impl::SendRoomJoinPending(QWebSocket* socket, const QString& roomId, const QString& requestId)` | 发送或发布 send room join pending 相关逻辑。 |
| [L87](../src/server/signaling/SignalServer.RoomState.cpp#L87) | `SignalServer::Impl::SendRoomJoinResult` | 定义 | `void SignalServer::Impl::SendRoomJoinResult(QWebSocket* socket, const QString& roomId, const QString& requestId, bool accepted, const QString& reasonCode, const QString& reasonMessage)` | 发送或发布 send room join result 相关逻辑。 |
| [L104](../src/server/signaling/SignalServer.RoomState.cpp#L104) | `SignalServer::Impl::SendRoomClosed` | 定义 | `void SignalServer::Impl::SendRoomClosed(QWebSocket* socket, const QString& roomId, const QString& initiatorDeviceId, const QString& reasonCode)` | 发送或发布 send room closed 相关逻辑。 |
| [L117](../src/server/signaling/SignalServer.RoomState.cpp#L117) | `SignalServer::Impl::CloseRoom` | 定义 | `void SignalServer::Impl::CloseRoom(const QString& roomId, const QString& initiatorDeviceId, const QString& reasonCode)` | 关闭并清理 close room 相关逻辑。 |
| [L174](../src/server/signaling/SignalServer.RoomState.cpp#L174) | `SignalServer::Impl::CancelPendingRoomJoinForDevice` | 定义 | `void SignalServer::Impl::CancelPendingRoomJoinForDevice(const QString& deviceId)` | 判断 cancel pending room join for device 相关逻辑。 |

## `src/server/signaling/SignalServerMain.cpp`

[打开源码](../src/server/signaling/SignalServerMain.cpp) · **文件作用：** 实现 signal server main 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L29](../src/server/signaling/SignalServerMain.cpp#L29) | `ReadSecret` | 定义 | `QByteArray ReadSecret(const QString& path, QString* error)` | 读取或恢复 read secret 相关逻辑。 |
| [L42](../src/server/signaling/SignalServerMain.cpp#L42) | `Fail` | 定义 | `int Fail(const QString& message)` | 实现 fail 对应的业务或工具逻辑。 |
| [L48](../src/server/signaling/SignalServerMain.cpp#L48) | `IsNineDigitPublicId` | 定义 | `bool IsNineDigitPublicId(const QString& value)` | 判断 is nine digit public id 相关逻辑。 |
| [L55](../src/server/signaling/SignalServerMain.cpp#L55) | `RunAuthStorageSelfTest` | 定义 | `int RunAuthStorageSelfTest()` | 执行后台循环或调度 run auth storage self test 相关逻辑。 |
| [L330](../src/server/signaling/SignalServerMain.cpp#L330) | `main` | 定义 | `int main(int argc, char* argv[])` | 实现 main 对应的业务或工具逻辑。 |

## `src/server/signaling/SignalServerSupport.cpp`

[打开源码](../src/server/signaling/SignalServerSupport.cpp) · **文件作用：** 实现 signal server support 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L23](../src/server/signaling/SignalServerSupport.cpp#L23) | `SetError` | 定义 | `void SetError(QString* error, const QString& message)` | 更新或应用 set error 相关逻辑。 |
| [L30](../src/server/signaling/SignalServerSupport.cpp#L30) | `ReadFile` | 定义 | `bool ReadFile(const QString& path, QByteArray* contents, QString* error)` | 读取或恢复 read file 相关逻辑。 |
| [L44](../src/server/signaling/SignalServerSupport.cpp#L44) | `CreateRecoveryToken` | 定义 | `QString CreateRecoveryToken()` | 创建或初始化 create recovery token 相关逻辑。 |
| [L57](../src/server/signaling/SignalServerSupport.cpp#L57) | `StdoutIsTerminal` | 定义 | `bool StdoutIsTerminal()` | 实现 stdout is terminal 对应的业务或工具逻辑。 |
| [L66](../src/server/signaling/SignalServerSupport.cpp#L66) | `ClearStdoutTerminal` | 定义 | `void ClearStdoutTerminal()` | 重置或移除 clear stdout terminal 相关逻辑。 |
| [L88](../src/server/signaling/SignalServerSupport.cpp#L88) | `FormatBytes` | 定义 | `QString FormatBytes(qint64 bytes)` | 实现 format bytes 对应的业务或工具逻辑。 |
| [L104](../src/server/signaling/SignalServerSupport.cpp#L104) | `FormatBitRate` | 定义 | `QString FormatBitRate(double bitsPerSecond)` | 实现 format bit rate 对应的业务或工具逻辑。 |
| [L120](../src/server/signaling/SignalServerSupport.cpp#L120) | `FormatDuration` | 定义 | `QString FormatDuration(qint64 milliseconds)` | 实现 format duration 对应的业务或工具逻辑。 |
| [L138](../src/server/signaling/SignalServerSupport.cpp#L138) | `IsValidPurpose` | 定义 | `bool IsValidPurpose(const QString& purpose)` | 判断 is valid purpose 相关逻辑。 |
| [L144](../src/server/signaling/SignalServerSupport.cpp#L144) | `IsNineDigitPublicId` | 定义 | `bool IsNineDigitPublicId(const QString& value)` | 判断 is nine digit public id 相关逻辑。 |
| [L151](../src/server/signaling/SignalServerSupport.cpp#L151) | `GenerateNineDigitPublicId` | 定义 | `QString GenerateNineDigitPublicId()` | 实现 generate nine digit public id 对应的业务或工具逻辑。 |

## `src/server/signaling/SignalServerSupport.h`

[打开源码](../src/server/signaling/SignalServerSupport.h) · **文件作用：** 声明 signal server support 相关类型、接口、配置和成员状态。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L15](../src/server/signaling/SignalServerSupport.h#L15) | `kProtocolVersion` | `inline constexpr int kProtocolVersion = 5;` | 定义 protocol version 的编译期常量或产品边界。 |
| [L16](../src/server/signaling/SignalServerSupport.h#L16) | `kMaximumMessageBytes` | `inline constexpr quint64 kMaximumMessageBytes = 1024 * 1024;` | 定义 maximum message bytes 的编译期常量或产品边界。 |
| [L17](../src/server/signaling/SignalServerSupport.h#L17) | `kMaximumRememberedMessageIds` | `inline constexpr int kMaximumRememberedMessageIds = 4096;` | 定义 maximum remembered message ids 的编译期常量或产品边界。 |
| [L18](../src/server/signaling/SignalServerSupport.h#L18) | `kMaximumSessions` | `inline constexpr int kMaximumSessions = 4096;` | 定义 maximum sessions 的编译期常量或产品边界。 |
| [L19](../src/server/signaling/SignalServerSupport.h#L19) | `kMaximumRooms` | `inline constexpr int kMaximumRooms = 4096;` | 定义 maximum rooms 的编译期常量或产品边界。 |
| [L20](../src/server/signaling/SignalServerSupport.h#L20) | `kInitialStateReserve` | `inline constexpr qsizetype kInitialStateReserve = 2048;` | 定义 initial state reserve 的编译期常量或产品边界。 |
| [L21](../src/server/signaling/SignalServerSupport.h#L21) | `kMaximumPermissions` | `inline constexpr int kMaximumPermissions = 16;` | 定义 maximum permissions 的编译期常量或产品边界。 |
| [L22](../src/server/signaling/SignalServerSupport.h#L22) | `kProtocolMaximumRoomMembers` | `inline constexpr int kProtocolMaximumRoomMembers = 5;` | 定义 protocol maximum room members 的编译期常量或产品边界。 |
| [L23](../src/server/signaling/SignalServerSupport.h#L23) | `kRoomJoinRequestTimeoutMs` | `inline constexpr int kRoomJoinRequestTimeoutMs = 30000;` | 定义 room join request timeout ms 的编译期常量或产品边界。 |
| [L24](../src/server/signaling/SignalServerSupport.h#L24) | `kRoomControlRequestTimeoutMs` | `inline constexpr int kRoomControlRequestTimeoutMs = 30000;` | 定义 room control request timeout ms 的编译期常量或产品边界。 |
| [L25](../src/server/signaling/SignalServerSupport.h#L25) | `kRoomScreenShareSwitchRequestTimeoutMs` | `inline constexpr int kRoomScreenShareSwitchRequestTimeoutMs = 30000;` | 定义 room screen share switch request timeout ms 的编译期常量或产品边界。 |
| [L26](../src/server/signaling/SignalServerSupport.h#L26) | `kRateLimitWindowMs` | `inline constexpr qint64 kRateLimitWindowMs = 60000;` | 定义 rate limit window ms 的编译期常量或产品边界。 |
| [L27](../src/server/signaling/SignalServerSupport.h#L27) | `kRateLimitMaximumKeys` | `inline constexpr int kRateLimitMaximumKeys = 4096;` | 定义 rate limit maximum keys 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L49](../src/server/signaling/SignalServerSupport.h#L49) | `ReadFile` | 声明 | `bool ReadFile(const QString& path, QByteArray* contents, QString* error)` | 读取或恢复 read file 相关逻辑。 |
| [L52](../src/server/signaling/SignalServerSupport.h#L52) | `CreateRecoveryToken` | 声明 | `QString CreateRecoveryToken()` | 创建或初始化 create recovery token 相关逻辑。 |
| [L53](../src/server/signaling/SignalServerSupport.h#L53) | `StdoutIsTerminal` | 声明 | `bool StdoutIsTerminal()` | 实现 stdout is terminal 对应的业务或工具逻辑。 |
| [L54](../src/server/signaling/SignalServerSupport.h#L54) | `ClearStdoutTerminal` | 声明 | `void ClearStdoutTerminal()` | 重置或移除 clear stdout terminal 相关逻辑。 |
| [L55](../src/server/signaling/SignalServerSupport.h#L55) | `FormatBytes` | 声明 | `QString FormatBytes(qint64 bytes)` | 实现 format bytes 对应的业务或工具逻辑。 |
| [L56](../src/server/signaling/SignalServerSupport.h#L56) | `FormatBitRate` | 声明 | `QString FormatBitRate(double bitsPerSecond)` | 实现 format bit rate 对应的业务或工具逻辑。 |
| [L57](../src/server/signaling/SignalServerSupport.h#L57) | `FormatDuration` | 声明 | `QString FormatDuration(qint64 milliseconds)` | 实现 format duration 对应的业务或工具逻辑。 |
| [L58](../src/server/signaling/SignalServerSupport.h#L58) | `SetError` | 声明 | `void SetError(QString* error, const QString& message)` | 更新或应用 set error 相关逻辑。 |
| [L59](../src/server/signaling/SignalServerSupport.h#L59) | `IsValidPurpose` | 声明 | `bool IsValidPurpose(const QString& purpose)` | 判断 is valid purpose 相关逻辑。 |
| [L60](../src/server/signaling/SignalServerSupport.h#L60) | `IsNineDigitPublicId` | 声明 | `bool IsNineDigitPublicId(const QString& value)` | 判断 is nine digit public id 相关逻辑。 |
| [L61](../src/server/signaling/SignalServerSupport.h#L61) | `GenerateNineDigitPublicId` | 声明 | `QString GenerateNineDigitPublicId()` | 实现 generate nine digit public id 对应的业务或工具逻辑。 |

## `src/server/signaling/SlidingWindowRateLimiter.h`

[打开源码](../src/server/signaling/SlidingWindowRateLimiter.h) · **文件作用：** 声明 sliding window rate limiter 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L15](../src/server/signaling/SlidingWindowRateLimiter.h#L15) | `SlidingWindowRateLimitPolicy` | struct | 定义 SlidingWindowRateLimitPolicy 的 struct 类型和相关状态。 |
| [L25](../src/server/signaling/SlidingWindowRateLimiter.h#L25) | `SlidingWindowRateLimiter` | class | In-memory, single-threaded rate limiter for the signaling event loop. Each key keeps only the accepted events inside the active window, while the number of keys is capped to pre... |
| [L120](../src/server/signaling/SlidingWindowRateLimiter.h#L120) | `Bucket` | struct | 定义 Bucket 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L16](../src/server/signaling/SlidingWindowRateLimiter.h#L16) | `maximumCost` | `int maximumCost = 1;` | 保存 maximum cost 相关配置或运行状态。 |
| [L17](../src/server/signaling/SlidingWindowRateLimiter.h#L17) | `windowMs` | `qint64 windowMs = 60000;` | 保存 window ms 相关配置或运行状态。 |
| [L18](../src/server/signaling/SlidingWindowRateLimiter.h#L18) | `maximumKeys` | `int maximumKeys = 4096;` | 保存 maximum keys 相关配置或运行状态。 |
| [L121](../src/server/signaling/SlidingWindowRateLimiter.h#L121) | `events` | `QQueue<qint64> events;` | 保存 events 相关配置或运行状态。 |
| [L122](../src/server/signaling/SlidingWindowRateLimiter.h#L122) | `lastSeenMs` | `qint64 lastSeenMs = 0;` | 保存 last seen ms 相关配置或运行状态。 |
| [L172](../src/server/signaling/SlidingWindowRateLimiter.h#L172) | `policy_` | `SlidingWindowRateLimitPolicy policy_;` | 保存 policy 相关配置或运行状态。 |
| [L173](../src/server/signaling/SlidingWindowRateLimiter.h#L173) | `buckets_` | `QHash<QString, Bucket> buckets_;` | 保存 buckets 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L27](../src/server/signaling/SlidingWindowRateLimiter.h#L27) | `SlidingWindowRateLimiter` | 定义 | `explicit SlidingWindowRateLimiter( SlidingWindowRateLimitPolicy policy) : policy_(policy) {}` | 实现 sliding window rate limiter 对应的业务或工具逻辑。 |
| [L32](../src/server/signaling/SlidingWindowRateLimiter.h#L32) | `CanAcquire` | 定义 | `bool CanAcquire(const QString& key, int cost, qint64 nowMs, qint64* retryAfterMs = nullptr)` | 判断 can acquire 相关逻辑。 |
| [L66](../src/server/signaling/SlidingWindowRateLimiter.h#L66) | `Record` | 定义 | `void Record(const QString& key, int cost, qint64 nowMs)` | 实现 record 对应的业务或工具逻辑。 |
| [L82](../src/server/signaling/SlidingWindowRateLimiter.h#L82) | `TryAcquire` | 定义 | `bool TryAcquire(const QString& key, int cost, qint64 nowMs, qint64* retryAfterMs = nullptr)` | 实现 try acquire 对应的业务或工具逻辑。 |
| [L94](../src/server/signaling/SlidingWindowRateLimiter.h#L94) | `Prune` | 定义 | `void Prune(qint64 nowMs)` | 实现 prune 对应的业务或工具逻辑。 |
| [L109](../src/server/signaling/SlidingWindowRateLimiter.h#L109) | `Clear` | 定义 | `void Clear()` | 重置或移除 clear 相关逻辑。 |
| [L114](../src/server/signaling/SlidingWindowRateLimiter.h#L114) | `keyCount` | 定义 | `qsizetype keyCount() const` | 实现 key count 对应的业务或工具逻辑。 |
| [L125](../src/server/signaling/SlidingWindowRateLimiter.h#L125) | `SetRetryAfter` | 定义 | `static void SetRetryAfter(qint64* destination, qint64 value)` | 更新或应用 set retry after 相关逻辑。 |
| [L132](../src/server/signaling/SlidingWindowRateLimiter.h#L132) | `Expire` | 定义 | `void Expire(Bucket& bucket, qint64 nowMs) const` | 实现 expire 对应的业务或工具逻辑。 |
| [L141](../src/server/signaling/SlidingWindowRateLimiter.h#L141) | `EnsureCapacityFor` | 定义 | `void EnsureCapacityFor(const QString& key, qint64 nowMs)` | 实现 ensure capacity for 对应的业务或工具逻辑。 |
