# 认证与客户端信令

> 自动生成于 2026-09-28，源码树 `f12aea4209d9-dirty`。请运行 `tools/Generate-SourceSymbolReference.ps1` 刷新。

Logto/DPAPI 身份、启动配置、WSS 客户端、JSON 消息与恢复。

本册共收录 33 个源码文件。函数与变量的中文作用优先采用源码紧邻注释；无注释时根据符号命名生成阅读提示，最终语义仍以源码为准。

## `src/auth/AuthConfig.cpp`

[打开源码](../src/auth/AuthConfig.cpp) · **文件作用：** 实现 auth config 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L12](../src/auth/AuthConfig.cpp#L12) | `SetError` | 定义 | `void SetError(QString* errorMessage, const QString& message)` | 更新或应用 set error 相关逻辑。 |
| [L18](../src/auth/AuthConfig.cpp#L18) | `HasForbiddenSecret` | 定义 | `bool HasForbiddenSecret(const QJsonObject& object)` | 判断 has forbidden secret 相关逻辑。 |
| [L44](../src/auth/AuthConfig.cpp#L44) | `AuthConfig::Validate` | 定义 | `bool AuthConfig::Validate(QString* errorMessage) const` | 判断 validate 相关逻辑。 |
| [L90](../src/auth/AuthConfig.cpp#L90) | `AuthConfig::FromJson` | 定义 | `std::optional<AuthConfig> AuthConfig::FromJson( const QJsonObject& object, QString* errorMessage)` | 实现 from json 对应的业务或工具逻辑。 |

## `src/auth/AuthConfig.h`

[打开源码](../src/auth/AuthConfig.h) · **文件作用：** 声明 auth config 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L15](../src/auth/AuthConfig.h#L15) | `AuthConfig` | struct | 定义 AuthConfig 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L16](../src/auth/AuthConfig.h#L16) | `issuer` | `QUrl issuer;` | 保存 issuer 相关配置或运行状态。 |
| [L17](../src/auth/AuthConfig.h#L17) | `clientId` | `QString clientId;` | 保存身份或作用域标识：client id。 |
| [L26](../src/auth/AuthConfig.h#L26) | `networkTimeoutMs` | `int networkTimeoutMs = 10000;` | 保存 network timeout ms 相关配置或运行状态。 |
| [L29](../src/auth/AuthConfig.h#L29) | `interactiveTimeoutMs` | `int interactiveTimeoutMs = 900000;` | Interactive sign-up can require email verification and profile setup. Keep the loopback callback alive without affecting silent refreshes. |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L21](../src/auth/AuthConfig.h#L21) | `QByteArrayLiteral` | 声明 | `QByteArrayLiteral("openid"), QByteArrayLiteral("profile"), QByteArrayLiteral("email"), QByteArrayLiteral("offline_access"), }` | 实现 q byte array literal 对应的业务或工具逻辑。 |
| [L22](../src/auth/AuthConfig.h#L22) | `QByteArrayLiteral` | 声明 | `QByteArrayLiteral("profile"), QByteArrayLiteral("email"), QByteArrayLiteral("offline_access"), }` | 实现 q byte array literal 对应的业务或工具逻辑。 |
| [L23](../src/auth/AuthConfig.h#L23) | `QByteArrayLiteral` | 声明 | `QByteArrayLiteral("email"), QByteArrayLiteral("offline_access"), }` | 实现 q byte array literal 对应的业务或工具逻辑。 |
| [L24](../src/auth/AuthConfig.h#L24) | `QByteArrayLiteral` | 声明 | `QByteArrayLiteral("offline_access"), }` | 实现 q byte array literal 对应的业务或工具逻辑。 |
| [L31](../src/auth/AuthConfig.h#L31) | `Validate` | 声明 | `bool Validate(QString* errorMessage = nullptr) const` | 判断 validate 相关逻辑。 |
| [L33](../src/auth/AuthConfig.h#L33) | `FromJson` | 声明 | `static std::optional<AuthConfig> FromJson( const QJsonObject& object, QString* errorMessage = nullptr)` | 实现 from json 对应的业务或工具逻辑。 |

## `src/auth/AuthManager.cpp`

[打开源码](../src/auth/AuthManager.cpp) · **文件作用：** 实现 auth manager 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L22](../src/auth/AuthManager.cpp#L22) | `OAuthErrorMessage` | 定义 | `QString OAuthErrorMessage(QAbstractOAuth::Error error)` | 实现 o auth error message 对应的业务或工具逻辑。 |
| [L41](../src/auth/AuthManager.cpp#L41) | `IsTransientOAuthError` | 定义 | `bool IsTransientOAuthError(QAbstractOAuth::Error error)` | 判断 is transient o auth error 相关逻辑。 |
| [L48](../src/auth/AuthManager.cpp#L48) | `AuthManager::AuthManager` | 定义 | `AuthManager::AuthManager( std::unique_ptr<TokenStore> tokenStore, QObject* parent) : QObject(parent), tokenStore_(std::move(tokenStore)), discovery_(&networkManager_, this)` | 构造并初始化 AuthManager 实例。 |
| [L80](../src/auth/AuthManager.cpp#L80) | `AuthManager::~AuthManager` | 定义 | `AuthManager::~AuthManager()` | 停止相关活动并释放 AuthManager 实例拥有的资源。 |
| [L85](../src/auth/AuthManager.cpp#L85) | `AuthManager::Configure` | 定义 | `bool AuthManager::Configure( const AuthConfig& config, QString* errorMessage)` | 更新或应用 configure 相关逻辑。 |
| [L106](../src/auth/AuthManager.cpp#L106) | `AuthManager::SetCallbacks` | 定义 | `void AuthManager::SetCallbacks(Callbacks callbacks)` | 更新或应用 set callbacks 相关逻辑。 |
| [L110](../src/auth/AuthManager.cpp#L110) | `AuthManager::StartInteractiveLogin` | 定义 | `void AuthManager::StartInteractiveLogin()` | 启动 start interactive login 相关逻辑。 |
| [L120](../src/auth/AuthManager.cpp#L120) | `AuthManager::RestoreSession` | 定义 | `void AuthManager::RestoreSession()` | 读取或恢复 restore session 相关逻辑。 |
| [L175](../src/auth/AuthManager.cpp#L175) | `AuthManager::RefreshAccessToken` | 定义 | `void AuthManager::RefreshAccessToken()` | 刷新 refresh access token 相关逻辑。 |
| [L189](../src/auth/AuthManager.cpp#L189) | `AuthManager::SignOut` | 定义 | `void AuthManager::SignOut()` | 实现 sign out 对应的业务或工具逻辑。 |
| [L208](../src/auth/AuthManager.cpp#L208) | `AuthManager::Cancel` | 定义 | `void AuthManager::Cancel()` | 判断 cancel 相关逻辑。 |
| [L224](../src/auth/AuthManager.cpp#L224) | `AuthManager::BeginDiscovery` | 定义 | `void AuthManager::BeginDiscovery(Operation operation)` | 启动 begin discovery 相关逻辑。 |
| [L252](../src/auth/AuthManager.cpp#L252) | `AuthManager::BuildFlow` | 定义 | `void AuthManager::BuildFlow()` | 创建或初始化 build flow 相关逻辑。 |
| [L362](../src/auth/AuthManager.cpp#L362) | `AuthManager::BeginInteractiveGrant` | 定义 | `void AuthManager::BeginInteractiveGrant()` | 启动 begin interactive grant 相关逻辑。 |
| [L513](../src/auth/AuthManager.cpp#L513) | `AuthManager::BeginStoredTokenRefresh` | 定义 | `void AuthManager::BeginStoredTokenRefresh( const StoredRefreshToken& storedToken)` | 启动 begin stored token refresh 相关逻辑。 |
| [L520](../src/auth/AuthManager.cpp#L520) | `AuthManager::CompleteAuthentication` | 定义 | `void AuthManager::CompleteAuthentication()` | 实现 complete authentication 对应的业务或工具逻辑。 |
| [L574](../src/auth/AuthManager.cpp#L574) | `AuthManager::SetState` | 定义 | `void AuthManager::SetState(AuthState state)` | 更新或应用 set state 相关逻辑。 |
| [L584](../src/auth/AuthManager.cpp#L584) | `AuthManager::Fail` | 定义 | `void AuthManager::Fail( AuthErrorCode code, QString message, bool transient)` | 实现 fail 对应的业务或工具逻辑。 |
| [L613](../src/auth/AuthManager.cpp#L613) | `AuthManager::CloseCallbackListener` | 定义 | `void AuthManager::CloseCallbackListener()` | 关闭并清理 close callback listener 相关逻辑。 |
| [L619](../src/auth/AuthManager.cpp#L619) | `AuthManager::AbortOutstandingNetworkReplies` | 定义 | `void AuthManager::AbortOutstandingNetworkReplies()` | 释放或取消 abort outstanding network replies 相关逻辑。 |
| [L629](../src/auth/AuthManager.cpp#L629) | `AuthManager::GenerateState` | 定义 | `QString AuthManager::GenerateState()` | 实现 generate state 对应的业务或工具逻辑。 |

## `src/auth/AuthManager.h`

[打开源码](../src/auth/AuthManager.h) · **文件作用：** 声明 auth manager 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L19](../src/auth/AuthManager.h#L19) | `QOAuth2AuthorizationCodeFlow` | class | 定义 QOAuth2AuthorizationCodeFlow 的 class 类型和相关状态。 |
| [L20](../src/auth/AuthManager.h#L20) | `QOAuthHttpServerReplyHandler` | class | 定义 QOAuthHttpServerReplyHandler 的 class 类型和相关状态。 |
| [L25](../src/auth/AuthManager.h#L25) | `AuthManager` | class | 定义 AuthManager 的 class 类型和相关状态。 |
| [L27](../src/auth/AuthManager.h#L27) | `Callbacks` | struct | 定义 Callbacks 的 struct 类型和相关状态。 |
| [L52](../src/auth/AuthManager.h#L52) | `Operation` | enum class | 定义 Operation 的 enum class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L19](../src/auth/AuthManager.h#L19) | `QOAuth2AuthorizationCodeFlow` | `class QOAuth2AuthorizationCodeFlow;` | 保存 qo auth2 authorization code flow 相关配置或运行状态。 |
| [L20](../src/auth/AuthManager.h#L20) | `QOAuthHttpServerReplyHandler` | `class QOAuthHttpServerReplyHandler;` | 保存回调或观察者入口：qo auth http server reply handler。 |
| [L70](../src/auth/AuthManager.h#L70) | `tokenStore_` | `std::unique_ptr<TokenStore> tokenStore_;` | 保存 token store 相关配置或运行状态。 |
| [L71](../src/auth/AuthManager.h#L71) | `networkManager_` | `QNetworkAccessManager networkManager_;` | 保存 network manager 相关配置或运行状态。 |
| [L72](../src/auth/AuthManager.h#L72) | `discovery_` | `OidcDiscovery discovery_;` | 保存 discovery 相关配置或运行状态。 |
| [L73](../src/auth/AuthManager.h#L73) | `flow_` | `std::unique_ptr<QOAuth2AuthorizationCodeFlow> flow_;` | 保存 flow 相关配置或运行状态。 |
| [L74](../src/auth/AuthManager.h#L74) | `replyHandler_` | `std::unique_ptr<QOAuthHttpServerReplyHandler> replyHandler_;` | 保存回调或观察者入口：reply handler。 |
| [L75](../src/auth/AuthManager.h#L75) | `operationTimer_` | `QTimer operationTimer_;` | 保存定时、截止或超时状态：operation timer。 |
| [L76](../src/auth/AuthManager.h#L76) | `refreshTimer_` | `QTimer refreshTimer_;` | 保存定时、截止或超时状态：refresh timer。 |
| [L77](../src/auth/AuthManager.h#L77) | `config_` | `AuthConfig config_;` | 保存 config 相关配置或运行状态。 |
| [L78](../src/auth/AuthManager.h#L78) | `metadata_` | `OidcMetadata metadata_;` | 保存 metadata 相关配置或运行状态。 |
| [L79](../src/auth/AuthManager.h#L79) | `tokens_` | `AuthTokenSnapshot tokens_;` | 保存 tokens 相关配置或运行状态。 |
| [L80](../src/auth/AuthManager.h#L80) | `callbacks_` | `Callbacks callbacks_;` | 保存 callbacks 相关配置或运行状态。 |
| [L81](../src/auth/AuthManager.h#L81) | `state_` | `AuthState state_ = AuthState::kUnconfigured;` | 保存状态机当前状态：state。 |
| [L82](../src/auth/AuthManager.h#L82) | `operation_` | `Operation operation_ = Operation::kNone;` | 保存 operation 相关配置或运行状态。 |
| [L83](../src/auth/AuthManager.h#L83) | `expectedState_` | `QString expectedState_;` | 保存状态机当前状态：expected state。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L33](../src/auth/AuthManager.h#L33) | `AuthManager` | 声明 | `explicit AuthManager( std::unique_ptr<TokenStore> tokenStore, QObject* parent = nullptr)` | 实现 auth manager 对应的业务或工具逻辑。 |
| [L36](../src/auth/AuthManager.h#L36) | `~AuthManager` | 声明 | `~AuthManager() override` | 停止相关活动并释放 AuthManager 实例拥有的资源。 |
| [L38](../src/auth/AuthManager.h#L38) | `Configure` | 声明 | `bool Configure(const AuthConfig& config, QString* errorMessage = nullptr)` | 更新或应用 configure 相关逻辑。 |
| [L39](../src/auth/AuthManager.h#L39) | `SetCallbacks` | 声明 | `void SetCallbacks(Callbacks callbacks)` | 更新或应用 set callbacks 相关逻辑。 |
| [L41](../src/auth/AuthManager.h#L41) | `StartInteractiveLogin` | 声明 | `void StartInteractiveLogin()` | 启动 start interactive login 相关逻辑。 |
| [L42](../src/auth/AuthManager.h#L42) | `RestoreSession` | 声明 | `void RestoreSession()` | 读取或恢复 restore session 相关逻辑。 |
| [L43](../src/auth/AuthManager.h#L43) | `RefreshAccessToken` | 声明 | `void RefreshAccessToken()` | 刷新 refresh access token 相关逻辑。 |
| [L44](../src/auth/AuthManager.h#L44) | `SignOut` | 声明 | `void SignOut()` | 实现 sign out 对应的业务或工具逻辑。 |
| [L45](../src/auth/AuthManager.h#L45) | `Cancel` | 声明 | `void Cancel()` | 判断 cancel 相关逻辑。 |
| [L47](../src/auth/AuthManager.h#L47) | `state` | 定义 | `AuthState state() const { return state_; }` | 实现 state 对应的业务或工具逻辑。 |
| [L48](../src/auth/AuthManager.h#L48) | `tokens` | 定义 | `const AuthTokenSnapshot& tokens() const { return tokens_; }` | 实现 tokens 对应的业务或工具逻辑。 |
| [L49](../src/auth/AuthManager.h#L49) | `config` | 定义 | `const AuthConfig& config() const { return config_; }` | 实现 config 对应的业务或工具逻辑。 |
| [L59](../src/auth/AuthManager.h#L59) | `BeginDiscovery` | 声明 | `void BeginDiscovery(Operation operation)` | 启动 begin discovery 相关逻辑。 |
| [L60](../src/auth/AuthManager.h#L60) | `BuildFlow` | 声明 | `void BuildFlow()` | 创建或初始化 build flow 相关逻辑。 |
| [L61](../src/auth/AuthManager.h#L61) | `BeginInteractiveGrant` | 声明 | `void BeginInteractiveGrant()` | 启动 begin interactive grant 相关逻辑。 |
| [L62](../src/auth/AuthManager.h#L62) | `BeginStoredTokenRefresh` | 声明 | `void BeginStoredTokenRefresh(const StoredRefreshToken& storedToken)` | 启动 begin stored token refresh 相关逻辑。 |
| [L63](../src/auth/AuthManager.h#L63) | `CompleteAuthentication` | 声明 | `void CompleteAuthentication()` | 实现 complete authentication 对应的业务或工具逻辑。 |
| [L64](../src/auth/AuthManager.h#L64) | `SetState` | 声明 | `void SetState(AuthState state)` | 更新或应用 set state 相关逻辑。 |
| [L65](../src/auth/AuthManager.h#L65) | `Fail` | 声明 | `void Fail(AuthErrorCode code, QString message, bool transient)` | 实现 fail 对应的业务或工具逻辑。 |
| [L66](../src/auth/AuthManager.h#L66) | `CloseCallbackListener` | 声明 | `void CloseCallbackListener()` | 关闭并清理 close callback listener 相关逻辑。 |
| [L67](../src/auth/AuthManager.h#L67) | `AbortOutstandingNetworkReplies` | 声明 | `void AbortOutstandingNetworkReplies()` | 释放或取消 abort outstanding network replies 相关逻辑。 |
| [L68](../src/auth/AuthManager.h#L68) | `GenerateState` | 声明 | `static QString GenerateState()` | 实现 generate state 对应的业务或工具逻辑。 |

## `src/auth/AuthTypes.h`

[打开源码](../src/auth/AuthTypes.h) · **文件作用：** 声明 auth types 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L12](../src/auth/AuthTypes.h#L12) | `AuthState` | enum class | 定义 AuthState 的 enum class 类型和相关状态。 |
| [L23](../src/auth/AuthTypes.h#L23) | `AuthErrorCode` | enum class | 定义 AuthErrorCode 的 enum class 类型和相关状态。 |
| [L38](../src/auth/AuthTypes.h#L38) | `AuthError` | struct | 定义 AuthError 的 struct 类型和相关状态。 |
| [L44](../src/auth/AuthTypes.h#L44) | `AuthTokenSnapshot` | struct | 定义 AuthTokenSnapshot 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L39](../src/auth/AuthTypes.h#L39) | `code` | `AuthErrorCode code = AuthErrorCode::kNone;` | 保存 code 相关配置或运行状态。 |
| [L40](../src/auth/AuthTypes.h#L40) | `message` | `QString message;` | 保存 message 相关配置或运行状态。 |
| [L41](../src/auth/AuthTypes.h#L41) | `transient` | `bool transient = false;` | 保存 transient 相关配置或运行状态。 |
| [L45](../src/auth/AuthTypes.h#L45) | `accessToken` | `QString accessToken;` | 保存 access token 相关配置或运行状态。 |
| [L46](../src/auth/AuthTypes.h#L46) | `idToken` | `QString idToken;` | 保存 id token 相关配置或运行状态。 |
| [L47](../src/auth/AuthTypes.h#L47) | `tokenType` | `QString tokenType;` | 保存 token type 相关配置或运行状态。 |
| [L48](../src/auth/AuthTypes.h#L48) | `grantedScopes` | `QSet<QByteArray> grantedScopes;` | 保存 granted scopes 相关配置或运行状态。 |
| [L49](../src/auth/AuthTypes.h#L49) | `expiresAtUtc` | `QDateTime expiresAtUtc;` | 保存 expires at utc 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L51](../src/auth/AuthTypes.h#L51) | `HasUsableAccessToken` | 定义 | `bool HasUsableAccessToken() const` | 判断 has usable access token 相关逻辑。 |

## `src/auth/DpapiTokenStore.cpp`

[打开源码](../src/auth/DpapiTokenStore.cpp) · **文件作用：** 实现 dpapi token store 相关函数与文件级辅助逻辑。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L22](../src/auth/DpapiTokenStore.cpp#L22) | `kEnvelopeMagic` | `constexpr char kEnvelopeMagic[] = "RCAUTH";` | 定义 envelope magic 的编译期常量或产品边界。 |
| [L23](../src/auth/DpapiTokenStore.cpp#L23) | `kEnvelopeVersion` | `constexpr char kEnvelopeVersion = 1;` | 定义 envelope version 的编译期常量或产品边界。 |
| [L24](../src/auth/DpapiTokenStore.cpp#L24) | `kEntropy` | `constexpr char kEntropy[] = "RemoteC.Logto.RefreshToken.v1";` | 定义 entropy 的编译期常量或产品边界。 |
| [L25](../src/auth/DpapiTokenStore.cpp#L25) | `kMaximumCredentialsBytes` | `constexpr qint64 kMaximumCredentialsBytes = 1024 * 1024;` | 定义 maximum credentials bytes 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L27](../src/auth/DpapiTokenStore.cpp#L27) | `SetError` | 定义 | `void SetError(QString* errorMessage, const QString& message)` | 更新或应用 set error 相关逻辑。 |
| [L33](../src/auth/DpapiTokenStore.cpp#L33) | `WindowsErrorMessage` | 定义 | `QString WindowsErrorMessage(const QString& operation)` | 实现 windows error message 对应的业务或工具逻辑。 |
| [L39](../src/auth/DpapiTokenStore.cpp#L39) | `SecureClear` | 定义 | `void SecureClear(QByteArray* data)` | 实现 secure clear 对应的业务或工具逻辑。 |
| [L46](../src/auth/DpapiTokenStore.cpp#L46) | `ToBlob` | 定义 | `bool ToBlob(QByteArray* data, DATA_BLOB* blob)` | 实现 to blob 对应的业务或工具逻辑。 |
| [L57](../src/auth/DpapiTokenStore.cpp#L57) | `Protect` | 定义 | `QByteArray Protect(QByteArray* plaintext, QString* errorMessage)` | 实现 protect 对应的业务或工具逻辑。 |
| [L90](../src/auth/DpapiTokenStore.cpp#L90) | `Unprotect` | 定义 | `QByteArray Unprotect(QByteArray* encrypted, QString* errorMessage)` | 实现 unprotect 对应的业务或工具逻辑。 |
| [L124](../src/auth/DpapiTokenStore.cpp#L124) | `DpapiTokenStore::DpapiTokenStore` | 定义 | `DpapiTokenStore::DpapiTokenStore(QString credentialsPath) : credentialsPath_(credentialsPath.isEmpty() ? DefaultCredentialsPath() : std::move(credentialsPath)) {}` | 构造并初始化 DpapiTokenStore 实例。 |
| [L129](../src/auth/DpapiTokenStore.cpp#L129) | `DpapiTokenStore::DefaultCredentialsPath` | 定义 | `QString DpapiTokenStore::DefaultCredentialsPath()` | 实现 default credentials path 对应的业务或工具逻辑。 |
| [L135](../src/auth/DpapiTokenStore.cpp#L135) | `DpapiTokenStore::Load` | 定义 | `TokenStoreLoadStatus DpapiTokenStore::Load( StoredRefreshToken* token, QString* errorMessage)` | 读取或恢复 load 相关逻辑。 |
| [L211](../src/auth/DpapiTokenStore.cpp#L211) | `DpapiTokenStore::Save` | 定义 | `bool DpapiTokenStore::Save( const StoredRefreshToken& token, QString* errorMessage)` | 保存或写入 save 相关逻辑。 |
| [L270](../src/auth/DpapiTokenStore.cpp#L270) | `DpapiTokenStore::Clear` | 定义 | `bool DpapiTokenStore::Clear(QString* errorMessage)` | 重置或移除 clear 相关逻辑。 |

## `src/auth/DpapiTokenStore.h`

[打开源码](../src/auth/DpapiTokenStore.h) · **文件作用：** 声明 dpapi token store 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L12](../src/auth/DpapiTokenStore.h#L12) | `DpapiTokenStore` | class | 定义 DpapiTokenStore 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L29](../src/auth/DpapiTokenStore.h#L29) | `credentialsPath_` | `QString credentialsPath_;` | 保存路径、地址或显示名称：credentials path。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L16](../src/auth/DpapiTokenStore.h#L16) | `Load` | 声明 | `TokenStoreLoadStatus Load( StoredRefreshToken* token, QString* errorMessage = nullptr) override` | 读取或恢复 load 相关逻辑。 |
| [L19](../src/auth/DpapiTokenStore.h#L19) | `Save` | 声明 | `bool Save( const StoredRefreshToken& token, QString* errorMessage = nullptr) override` | 保存或写入 save 相关逻辑。 |
| [L22](../src/auth/DpapiTokenStore.h#L22) | `Clear` | 声明 | `bool Clear(QString* errorMessage = nullptr) override` | 重置或移除 clear 相关逻辑。 |
| [L24](../src/auth/DpapiTokenStore.h#L24) | `credentialsPath` | 定义 | `const QString& credentialsPath() const { return credentialsPath_; }` | 实现 credentials path 对应的业务或工具逻辑。 |
| [L26](../src/auth/DpapiTokenStore.h#L26) | `DefaultCredentialsPath` | 声明 | `static QString DefaultCredentialsPath()` | 实现 default credentials path 对应的业务或工具逻辑。 |

## `src/auth/OidcDiscovery.cpp`

[打开源码](../src/auth/OidcDiscovery.cpp) · **文件作用：** 实现 oidc discovery 相关函数与文件级辅助逻辑。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L19](../src/auth/OidcDiscovery.cpp#L19) | `kMaximumDiscoveryBytes` | `constexpr qint64 kMaximumDiscoveryBytes = 1024 * 1024;` | 定义 maximum discovery bytes 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L21](../src/auth/OidcDiscovery.cpp#L21) | `NormalizeIssuer` | 定义 | `QUrl NormalizeIssuer(QUrl issuer)` | 实现 normalize issuer 对应的业务或工具逻辑。 |
| [L30](../src/auth/OidcDiscovery.cpp#L30) | `IsSecureEndpoint` | 定义 | `bool IsSecureEndpoint(const QUrl& url)` | 判断 is secure endpoint 相关逻辑。 |
| [L35](../src/auth/OidcDiscovery.cpp#L35) | `StringArrayContains` | 定义 | `bool StringArrayContains(const QJsonValue& value, const QString& expected)` | 实现 string array contains 对应的业务或工具逻辑。 |
| [L49](../src/auth/OidcDiscovery.cpp#L49) | `OidcDiscovery::OidcDiscovery` | 定义 | `OidcDiscovery::OidcDiscovery( QNetworkAccessManager* networkManager, QObject* parent) : QObject(parent), networkManager_(networkManager) {}` | 构造并初始化 OidcDiscovery 实例。 |
| [L54](../src/auth/OidcDiscovery.cpp#L54) | `OidcDiscovery::~OidcDiscovery` | 定义 | `OidcDiscovery::~OidcDiscovery()` | 停止相关活动并释放 OidcDiscovery 实例拥有的资源。 |
| [L58](../src/auth/OidcDiscovery.cpp#L58) | `OidcDiscovery::Discover` | 定义 | `void OidcDiscovery::Discover( const QUrl& issuer, int timeoutMs, Completion completion)` | 实现 discover 对应的业务或工具逻辑。 |
| [L197](../src/auth/OidcDiscovery.cpp#L197) | `OidcDiscovery::Cancel` | 定义 | `void OidcDiscovery::Cancel()` | 判断 cancel 相关逻辑。 |
| [L208](../src/auth/OidcDiscovery.cpp#L208) | `OidcDiscovery::Finish` | 定义 | `void OidcDiscovery::Finish( bool success, OidcMetadata metadata, QString errorMessage, bool transient)` | 停止 finish 相关逻辑。 |

## `src/auth/OidcDiscovery.h`

[打开源码](../src/auth/OidcDiscovery.h) · **文件作用：** 声明 oidc discovery 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L14](../src/auth/OidcDiscovery.h#L14) | `QNetworkAccessManager` | class | 定义 QNetworkAccessManager 的 class 类型和相关状态。 |
| [L15](../src/auth/OidcDiscovery.h#L15) | `QNetworkReply` | class | 定义 QNetworkReply 的 class 类型和相关状态。 |
| [L20](../src/auth/OidcDiscovery.h#L20) | `OidcMetadata` | struct | 定义 OidcMetadata 的 struct 类型和相关状态。 |
| [L27](../src/auth/OidcDiscovery.h#L27) | `OidcDiscovery` | class | 定义 OidcDiscovery 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L14](../src/auth/OidcDiscovery.h#L14) | `QNetworkAccessManager` | `class QNetworkAccessManager;` | 保存 q network access manager 相关配置或运行状态。 |
| [L15](../src/auth/OidcDiscovery.h#L15) | `QNetworkReply` | `class QNetworkReply;` | 保存 q network reply 相关配置或运行状态。 |
| [L21](../src/auth/OidcDiscovery.h#L21) | `issuer` | `QUrl issuer;` | 保存 issuer 相关配置或运行状态。 |
| [L22](../src/auth/OidcDiscovery.h#L22) | `authorizationEndpoint` | `QUrl authorizationEndpoint;` | 保存 authorization endpoint 相关配置或运行状态。 |
| [L23](../src/auth/OidcDiscovery.h#L23) | `tokenEndpoint` | `QUrl tokenEndpoint;` | 保存 token endpoint 相关配置或运行状态。 |
| [L24](../src/auth/OidcDiscovery.h#L24) | `userInfoEndpoint` | `QUrl userInfoEndpoint;` | 保存 user info endpoint 相关配置或运行状态。 |
| [L50](../src/auth/OidcDiscovery.h#L50) | `networkManager_` | `QNetworkAccessManager* networkManager_ = nullptr;` | 保存 network manager 相关配置或运行状态。 |
| [L51](../src/auth/OidcDiscovery.h#L51) | `activeReply_` | `QPointer<QNetworkReply> activeReply_;` | 保存 active reply 相关配置或运行状态。 |
| [L52](../src/auth/OidcDiscovery.h#L52) | `completion_` | `Completion completion_;` | 保存 completion 相关配置或运行状态。 |
| [L53](../src/auth/OidcDiscovery.h#L53) | `expectedIssuer_` | `QUrl expectedIssuer_;` | 保存 expected issuer 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L32](../src/auth/OidcDiscovery.h#L32) | `OidcDiscovery` | 声明 | `explicit OidcDiscovery( QNetworkAccessManager* networkManager, QObject* parent = nullptr)` | 实现 oidc discovery 对应的业务或工具逻辑。 |
| [L35](../src/auth/OidcDiscovery.h#L35) | `~OidcDiscovery` | 声明 | `~OidcDiscovery() override` | 停止相关活动并释放 OidcDiscovery 实例拥有的资源。 |
| [L37](../src/auth/OidcDiscovery.h#L37) | `Discover` | 声明 | `void Discover( const QUrl& issuer, int timeoutMs, Completion completion)` | 实现 discover 对应的业务或工具逻辑。 |
| [L41](../src/auth/OidcDiscovery.h#L41) | `Cancel` | 声明 | `void Cancel()` | 判断 cancel 相关逻辑。 |
| [L44](../src/auth/OidcDiscovery.h#L44) | `Finish` | 声明 | `void Finish( bool success, OidcMetadata metadata, QString errorMessage, bool transient)` | 停止 finish 相关逻辑。 |

## `src/auth/TokenStore.h`

[打开源码](../src/auth/TokenStore.h) · **文件作用：** 声明 token store 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L11](../src/auth/TokenStore.h#L11) | `StoredRefreshToken` | struct | 定义 StoredRefreshToken 的 struct 类型和相关状态。 |
| [L18](../src/auth/TokenStore.h#L18) | `TokenStoreLoadStatus` | enum class | 定义 TokenStoreLoadStatus 的 enum class 类型和相关状态。 |
| [L24](../src/auth/TokenStore.h#L24) | `TokenStore` | class | 定义 TokenStore 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L12](../src/auth/TokenStore.h#L12) | `issuer` | `QString issuer;` | 保存 issuer 相关配置或运行状态。 |
| [L13](../src/auth/TokenStore.h#L13) | `clientId` | `QString clientId;` | 保存身份或作用域标识：client id。 |
| [L14](../src/auth/TokenStore.h#L14) | `refreshToken` | `QString refreshToken;` | 保存 refresh token 相关配置或运行状态。 |
| [L15](../src/auth/TokenStore.h#L15) | `savedAtUtc` | `QDateTime savedAtUtc;` | 保存 saved at utc 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L26](../src/auth/TokenStore.h#L26) | `~TokenStore` | 声明 | `virtual ~TokenStore() = default` | 停止相关活动并释放 TokenStore 实例拥有的资源。 |
| [L28](../src/auth/TokenStore.h#L28) | `Load` | 声明 | `virtual TokenStoreLoadStatus Load( StoredRefreshToken* token, QString* errorMessage = nullptr) = 0` | 读取或恢复 load 相关逻辑。 |
| [L31](../src/auth/TokenStore.h#L31) | `Save` | 声明 | `virtual bool Save( const StoredRefreshToken& token, QString* errorMessage = nullptr) = 0` | 保存或写入 save 相关逻辑。 |
| [L34](../src/auth/TokenStore.h#L34) | `Clear` | 声明 | `virtual bool Clear(QString* errorMessage = nullptr) = 0` | 重置或移除 clear 相关逻辑。 |

## `src/signaling/ISignalingClient.h`

[打开源码](../src/signaling/ISignalingClient.h) · **文件作用：** 声明 i signaling client 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L10](../src/signaling/ISignalingClient.h#L10) | `ISignalingClient` | class | 定义 ISignalingClient 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L12](../src/signaling/ISignalingClient.h#L12) | `~ISignalingClient` | 声明 | `virtual ~ISignalingClient() = default` | 停止相关活动并释放 ISignalingClient 实例拥有的资源。 |
| [L14](../src/signaling/ISignalingClient.h#L14) | `SetObserver` | 声明 | `virtual void SetObserver(ISignalingClientObserver* observer) = 0` | 更新或应用 set observer 相关逻辑。 |
| [L15](../src/signaling/ISignalingClient.h#L15) | `Connect` | 声明 | `virtual SignalingOperationResult Connect( const SignalingClientConfig& config) = 0` | 建立连接 connect 相关逻辑。 |
| [L19](../src/signaling/ISignalingClient.h#L19) | `UpdateAccessToken` | 声明 | `virtual SignalingOperationResult UpdateAccessToken( const std::string& accessToken) = 0` | Replaces the in-memory token used by the next message authentication or reconnect. It deliberately does not tear down an active WebRTC session. |
| [L21](../src/signaling/ISignalingClient.h#L21) | `RequestAccountDeletion` | 定义 | `virtual SignalingOperationResult RequestAccountDeletion()` | 发起请求或查询 request account deletion 相关逻辑。 |
| [L26](../src/signaling/ISignalingClient.h#L26) | `RequestOwnedDevices` | 定义 | `virtual SignalingOperationResult RequestOwnedDevices()` | 发起请求或查询 request owned devices 相关逻辑。 |
| [L31](../src/signaling/ISignalingClient.h#L31) | `Disconnect` | 声明 | `virtual void Disconnect() = 0` | 断开连接 disconnect 相关逻辑。 |
| [L32](../src/signaling/ISignalingClient.h#L32) | `State` | 声明 | `virtual SignalingConnectionState State() const = 0` | 实现 state 对应的业务或工具逻辑。 |
| [L34](../src/signaling/ISignalingClient.h#L34) | `RequestSession` | 声明 | `virtual SignalingOperationResult RequestSession( const std::string& targetDeviceId, SessionPurpose purpose, const std::vector<std::string>& permissions) = 0` | 发起请求或查询 request session 相关逻辑。 |
| [L38](../src/signaling/ISignalingClient.h#L38) | `RequestOwnedDeviceSession` | 定义 | `virtual SignalingOperationResult RequestOwnedDeviceSession( const std::string& targetDeviceId, SessionPurpose purpose, const std::vector<std::string>& permissions)` | 发起请求或查询 request owned device session 相关逻辑。 |
| [L45](../src/signaling/ISignalingClient.h#L45) | `RequestAssistedSession` | 定义 | `virtual SignalingOperationResult RequestAssistedSession( const std::string& targetDeviceId, const std::string& verificationCode, const std::vector<std::string>& permissions)` | 发起请求或查询 request assisted session 相关逻辑。 |
| [L54](../src/signaling/ISignalingClient.h#L54) | `RespondToSession` | 声明 | `virtual SignalingOperationResult RespondToSession( const std::string& sessionId, bool accepted, const std::string& reasonCode) = 0` | 处理并回复 respond to session 相关逻辑。 |
| [L58](../src/signaling/ISignalingClient.h#L58) | `CancelSession` | 声明 | `virtual SignalingOperationResult CancelSession( const std::string& sessionId, const std::string& reasonCode) = 0` | 判断 cancel session 相关逻辑。 |
| [L61](../src/signaling/ISignalingClient.h#L61) | `CloseSession` | 声明 | `virtual SignalingOperationResult CloseSession( const std::string& sessionId, const std::string& reasonCode) = 0` | 关闭并清理 close session 相关逻辑。 |
| [L64](../src/signaling/ISignalingClient.h#L64) | `ResumeSession` | 声明 | `virtual SignalingOperationResult ResumeSession( const std::string& sessionId, const std::string& recoveryToken) = 0` | 实现 resume session 对应的业务或工具逻辑。 |
| [L68](../src/signaling/ISignalingClient.h#L68) | `CreateRoom` | 声明 | `virtual SignalingOperationResult CreateRoom( std::uint32_t capacity) = 0` | 创建或初始化 create room 相关逻辑。 |
| [L70](../src/signaling/ISignalingClient.h#L70) | `RequestRoomJoin` | 声明 | `virtual SignalingOperationResult RequestRoomJoin( const std::string& roomId) = 0` | 发起请求或查询 request room join 相关逻辑。 |
| [L72](../src/signaling/ISignalingClient.h#L72) | `QueryRoomAvailability` | 定义 | `virtual SignalingOperationResult QueryRoomAvailability( const std::vector<std::string>& roomIds)` | 发起请求或查询 query room availability 相关逻辑。 |
| [L79](../src/signaling/ISignalingClient.h#L79) | `RespondToRoomJoin` | 声明 | `virtual SignalingOperationResult RespondToRoomJoin( const std::string& roomId, const std::string& requestId, bool accepted, const std::string& reasonCode) = 0` | 处理并回复 respond to room join 相关逻辑。 |
| [L84](../src/signaling/ISignalingClient.h#L84) | `SetRoomCapacity` | 声明 | `virtual SignalingOperationResult SetRoomCapacity( const std::string& roomId, std::uint32_t capacity) = 0` | 更新或应用 set room capacity 相关逻辑。 |
| [L87](../src/signaling/ISignalingClient.h#L87) | `LeaveRoom` | 声明 | `virtual SignalingOperationResult LeaveRoom( const std::string& roomId, const std::string& reasonCode) = 0` | 实现 leave room 对应的业务或工具逻辑。 |
| [L90](../src/signaling/ISignalingClient.h#L90) | `ResumeRoom` | 声明 | `virtual SignalingOperationResult ResumeRoom( const std::string& roomId, const std::string& recoveryToken) = 0` | 实现 resume room 对应的业务或工具逻辑。 |
| [L93](../src/signaling/ISignalingClient.h#L93) | `SetRoomMediaState` | 声明 | `virtual SignalingOperationResult SetRoomMediaState( const std::string& roomId, bool cameraPublishing, bool microphonePublishing) = 0` | 更新或应用 set room media state 相关逻辑。 |
| [L97](../src/signaling/ISignalingClient.h#L97) | `RequestRoomScreenShare` | 声明 | `virtual SignalingOperationResult RequestRoomScreenShare( const std::string& roomId) = 0` | 发起请求或查询 request room screen share 相关逻辑。 |
| [L99](../src/signaling/ISignalingClient.h#L99) | `ConfirmRoomScreenShare` | 声明 | `virtual SignalingOperationResult ConfirmRoomScreenShare( const std::string& roomId, const std::string& grantId) = 0` | 实现 confirm room screen share 对应的业务或工具逻辑。 |
| [L102](../src/signaling/ISignalingClient.h#L102) | `StopRoomScreenShare` | 声明 | `virtual SignalingOperationResult StopRoomScreenShare( const std::string& roomId, const std::string& grantId, const std::string& reasonCode) = 0` | 停止 stop room screen share 相关逻辑。 |
| [L106](../src/signaling/ISignalingClient.h#L106) | `RespondToRoomScreenShareSwitch` | 定义 | `virtual SignalingOperationResult RespondToRoomScreenShareSwitch( const std::string& roomId, const std::string& requestId, bool accepted, const std::string& reasonCode)` | 处理并回复 respond to room screen share switch 相关逻辑。 |
| [L119](../src/signaling/ISignalingClient.h#L119) | `CancelRoomScreenShareSwitch` | 定义 | `virtual SignalingOperationResult CancelRoomScreenShareSwitch( const std::string& roomId, const std::string& requestId, const std::string& reasonCode)` | 判断 cancel room screen share switch 相关逻辑。 |
| [L130](../src/signaling/ISignalingClient.h#L130) | `RequestRoomControl` | 声明 | `virtual SignalingOperationResult RequestRoomControl( const std::string& roomId) = 0` | 发起请求或查询 request room control 相关逻辑。 |
| [L132](../src/signaling/ISignalingClient.h#L132) | `RespondToRoomControl` | 声明 | `virtual SignalingOperationResult RespondToRoomControl( const std::string& roomId, const std::string& requestId, bool accepted, const std::string& reasonCode) = 0` | 处理并回复 respond to room control 相关逻辑。 |
| [L137](../src/signaling/ISignalingClient.h#L137) | `ReleaseRoomControl` | 声明 | `virtual SignalingOperationResult ReleaseRoomControl( const std::string& roomId, const std::string& grantId, const std::string& reasonCode) = 0` | 释放或取消 release room control 相关逻辑。 |
| [L142](../src/signaling/ISignalingClient.h#L142) | `SendDescription` | 声明 | `virtual SignalingOperationResult SendDescription( const SignalingSessionDescription& description) = 0` | 发送或发布 send description 相关逻辑。 |
| [L144](../src/signaling/ISignalingClient.h#L144) | `SendIceCandidate` | 声明 | `virtual SignalingOperationResult SendIceCandidate( const SignalingIceCandidate& candidate) = 0` | 发送或发布 send ice candidate 相关逻辑。 |
| [L146](../src/signaling/ISignalingClient.h#L146) | `SendIceRestartRequest` | 定义 | `virtual SignalingOperationResult SendIceRestartRequest( const SignalingIceRestartRequest& request)` | 发送或发布 send ice restart request 相关逻辑。 |
| [L153](../src/signaling/ISignalingClient.h#L153) | `SendIceRestartCancel` | 定义 | `virtual SignalingOperationResult SendIceRestartCancel( const SignalingIceRestartCancel& cancel)` | 发送或发布 send ice restart cancel 相关逻辑。 |

## `src/signaling/LocalTcpSignaling.cpp`

[打开源码](../src/signaling/LocalTcpSignaling.cpp) · **文件作用：** 实现 local tcp signaling 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L30](../src/signaling/LocalTcpSignaling.cpp#L30) | `MessageType` | enum class | 定义 MessageType 的 enum class 类型和相关状态。 |
| [L129](../src/signaling/LocalTcpSignaling.cpp#L129) | `LocalTcpSignaling::Impl` | class | 定义 LocalTcpSignaling::Impl 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L27](../src/signaling/LocalTcpSignaling.cpp#L27) | `kProtocolVersion` | `constexpr std::uint16_t kProtocolVersion = 1;` | 定义 protocol version 的编译期常量或产品边界。 |
| [L28](../src/signaling/LocalTcpSignaling.cpp#L28) | `kMaxPayloadBytes` | `constexpr std::uint32_t kMaxPayloadBytes = 1024 * 1024;` | 定义 max payload bytes 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L35](../src/signaling/LocalTcpSignaling.cpp#L35) | `AppendU32` | 定义 | `void AppendU32(std::vector<std::uint8_t>& output, std::uint32_t value)` | 实现 append u32 对应的业务或工具逻辑。 |
| [L42](../src/signaling/LocalTcpSignaling.cpp#L42) | `AppendString` | 定义 | `bool AppendString(std::vector<std::uint8_t>& output, const std::string& value)` | 实现 append string 对应的业务或工具逻辑。 |
| [L53](../src/signaling/LocalTcpSignaling.cpp#L53) | `ReadU32` | 定义 | `bool ReadU32(std::span<const std::uint8_t> input, std::size_t& offset, std::uint32_t& value)` | 读取或恢复 read u32 相关逻辑。 |
| [L66](../src/signaling/LocalTcpSignaling.cpp#L66) | `ReadString` | 定义 | `bool ReadString(std::span<const std::uint8_t> input, std::size_t& offset, std::string& value)` | 读取或恢复 read string 相关逻辑。 |
| [L81](../src/signaling/LocalTcpSignaling.cpp#L81) | `SendAll` | 定义 | `bool SendAll(SOCKET socket, std::span<const std::uint8_t> bytes)` | 发送或发布 send all 相关逻辑。 |
| [L101](../src/signaling/LocalTcpSignaling.cpp#L101) | `ReceiveAll` | 定义 | `bool ReceiveAll(SOCKET socket, std::span<std::uint8_t> bytes)` | 接收并处理 receive all 相关逻辑。 |
| [L121](../src/signaling/LocalTcpSignaling.cpp#L121) | `SocketErrorText` | 定义 | `std::string SocketErrorText(const char* operation)` | 实现 socket error text 对应的业务或工具逻辑。 |
| [L131](../src/signaling/LocalTcpSignaling.cpp#L131) | `Impl` | 定义 | `Impl()` | 实现 impl 对应的业务或工具逻辑。 |
| [L141](../src/signaling/LocalTcpSignaling.cpp#L141) | `~Impl` | 定义 | `~Impl()` | 停止相关活动并释放 Impl 实例拥有的资源。 |
| [L149](../src/signaling/LocalTcpSignaling.cpp#L149) | `SetReceiver` | 定义 | `void SetReceiver(SessionControllerBase* receiver)` | 更新或应用 set receiver 相关逻辑。 |
| [L155](../src/signaling/LocalTcpSignaling.cpp#L155) | `StartListening` | 定义 | `bool StartListening(std::uint16_t port)` | 启动 start listening 相关逻辑。 |
| [L206](../src/signaling/LocalTcpSignaling.cpp#L206) | `Connect` | 定义 | `bool Connect(const std::string& ipv4Address, std::uint16_t port, std::chrono::milliseconds timeout)` | 建立连接 connect 相关逻辑。 |
| [L259](../src/signaling/LocalTcpSignaling.cpp#L259) | `WaitUntilConnected` | 定义 | `bool WaitUntilConnected(std::chrono::milliseconds timeout)` | 实现 wait until connected 对应的业务或工具逻辑。 |
| [L268](../src/signaling/LocalTcpSignaling.cpp#L268) | `SendDescription` | 定义 | `bool SendDescription(const SessionDescription& description)` | 发送或发布 send description 相关逻辑。 |
| [L282](../src/signaling/LocalTcpSignaling.cpp#L282) | `SendIceCandidate` | 定义 | `bool SendIceCandidate(const IceCandidate& candidate)` | 发送或发布 send ice candidate 相关逻辑。 |
| [L297](../src/signaling/LocalTcpSignaling.cpp#L297) | `Stop` | 定义 | `void Stop()` | 停止 stop 相关逻辑。 |
| [L326](../src/signaling/LocalTcpSignaling.cpp#L326) | `LastError` | 定义 | `std::string LastError() const` | 实现 last error 对应的业务或工具逻辑。 |
| [L333](../src/signaling/LocalTcpSignaling.cpp#L333) | `SendFrame` | 定义 | `bool SendFrame(MessageType type, std::span<const std::uint8_t> payload)` | 发送或发布 send frame 相关逻辑。 |
| [L370](../src/signaling/LocalTcpSignaling.cpp#L370) | `AcceptAndRead` | 定义 | `void AcceptAndRead(SOCKET listener)` | 处理并回复 accept and read 相关逻辑。 |
| [L408](../src/signaling/LocalTcpSignaling.cpp#L408) | `ReadLoop` | 定义 | `void ReadLoop(SOCKET peer)` | 读取或恢复 read loop 相关逻辑。 |
| [L454](../src/signaling/LocalTcpSignaling.cpp#L454) | `Dispatch` | 定义 | `bool Dispatch(MessageType type, std::span<const std::uint8_t> payload)` | 接收并处理 dispatch 相关逻辑。 |
| [L506](../src/signaling/LocalTcpSignaling.cpp#L506) | `SetError` | 定义 | `void SetError(std::string error)` | 更新或应用 set error 相关逻辑。 |
| [L515](../src/signaling/LocalTcpSignaling.cpp#L515) | `SetErrorLocked` | 定义 | `void SetErrorLocked(std::string error)` | 更新或应用 set error locked 相关逻辑。 |
| [L535](../src/signaling/LocalTcpSignaling.cpp#L535) | `LocalTcpSignaling::LocalTcpSignaling` | 定义 | `LocalTcpSignaling::LocalTcpSignaling() : impl_(std::make_unique<Impl>()) {}` | 构造并初始化 LocalTcpSignaling 实例。 |
| [L539](../src/signaling/LocalTcpSignaling.cpp#L539) | `LocalTcpSignaling::SetReceiver` | 定义 | `void LocalTcpSignaling::SetReceiver(SessionControllerBase* receiver)` | 更新或应用 set receiver 相关逻辑。 |
| [L544](../src/signaling/LocalTcpSignaling.cpp#L544) | `LocalTcpSignaling::StartListening` | 定义 | `bool LocalTcpSignaling::StartListening(std::uint16_t port)` | 启动 start listening 相关逻辑。 |
| [L549](../src/signaling/LocalTcpSignaling.cpp#L549) | `LocalTcpSignaling::Connect` | 定义 | `bool LocalTcpSignaling::Connect(const std::string& ipv4Address, std::uint16_t port, std::chrono::milliseconds timeout)` | 建立连接 connect 相关逻辑。 |
| [L556](../src/signaling/LocalTcpSignaling.cpp#L556) | `LocalTcpSignaling::WaitUntilConnected` | 定义 | `bool LocalTcpSignaling::WaitUntilConnected(std::chrono::milliseconds timeout)` | 实现 wait until connected 对应的业务或工具逻辑。 |
| [L561](../src/signaling/LocalTcpSignaling.cpp#L561) | `LocalTcpSignaling::SendDescription` | 定义 | `bool LocalTcpSignaling::SendDescription( const SessionDescription& description)` | 发送或发布 send description 相关逻辑。 |
| [L567](../src/signaling/LocalTcpSignaling.cpp#L567) | `LocalTcpSignaling::SendIceCandidate` | 定义 | `bool LocalTcpSignaling::SendIceCandidate(const IceCandidate& candidate)` | 发送或发布 send ice candidate 相关逻辑。 |
| [L572](../src/signaling/LocalTcpSignaling.cpp#L572) | `LocalTcpSignaling::Stop` | 定义 | `void LocalTcpSignaling::Stop()` | 停止 stop 相关逻辑。 |
| [L577](../src/signaling/LocalTcpSignaling.cpp#L577) | `LocalTcpSignaling::LastError` | 定义 | `std::string LocalTcpSignaling::LastError() const` | 实现 last error 对应的业务或工具逻辑。 |

## `src/signaling/LocalTcpSignaling.h`

[打开源码](../src/signaling/LocalTcpSignaling.h) · **文件作用：** 声明 local tcp signaling 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L18](../src/signaling/LocalTcpSignaling.h#L18) | `LocalTcpSignaling` | class | Local, process-to-process signaling transport used by the first vertical slice. Only SDP and trickle ICE travel over this connection; WebRTC media and DataChannels continue to u... |
| [L42](../src/signaling/LocalTcpSignaling.h#L42) | `Impl` | class | 定义 Impl 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L42](../src/signaling/LocalTcpSignaling.h#L42) | `Impl` | `class Impl;` | 保存 impl 相关配置或运行状态。 |
| [L43](../src/signaling/LocalTcpSignaling.h#L43) | `impl_` | `std::unique_ptr<Impl> impl_;` | 保存 impl 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L20](../src/signaling/LocalTcpSignaling.h#L20) | `LocalTcpSignaling` | 声明 | `LocalTcpSignaling()` | 实现 local tcp signaling 对应的业务或工具逻辑。 |
| [L21](../src/signaling/LocalTcpSignaling.h#L21) | `~LocalTcpSignaling` | 声明 | `~LocalTcpSignaling() override` | 停止相关活动并释放 LocalTcpSignaling 实例拥有的资源。 |
| [L23](../src/signaling/LocalTcpSignaling.h#L23) | `LocalTcpSignaling` | 声明 | `LocalTcpSignaling(const LocalTcpSignaling&) = delete` | 实现 local tcp signaling 对应的业务或工具逻辑。 |
| [L26](../src/signaling/LocalTcpSignaling.h#L26) | `SetReceiver` | 声明 | `void SetReceiver(SessionControllerBase* receiver)` | 更新或应用 set receiver 相关逻辑。 |
| [L28](../src/signaling/LocalTcpSignaling.h#L28) | `StartListening` | 声明 | `bool StartListening(std::uint16_t port)` | 启动 start listening 相关逻辑。 |
| [L29](../src/signaling/LocalTcpSignaling.h#L29) | `Connect` | 声明 | `bool Connect(const std::string& ipv4Address, std::uint16_t port, std::chrono::milliseconds timeout)` | 建立连接 connect 相关逻辑。 |
| [L32](../src/signaling/LocalTcpSignaling.h#L32) | `WaitUntilConnected` | 声明 | `bool WaitUntilConnected(std::chrono::milliseconds timeout)` | 实现 wait until connected 对应的业务或工具逻辑。 |
| [L34](../src/signaling/LocalTcpSignaling.h#L34) | `SendDescription` | 声明 | `bool SendDescription( const SessionDescription& description) override` | 发送或发布 send description 相关逻辑。 |
| [L36](../src/signaling/LocalTcpSignaling.h#L36) | `SendIceCandidate` | 声明 | `bool SendIceCandidate(const IceCandidate& candidate) override` | 发送或发布 send ice candidate 相关逻辑。 |
| [L38](../src/signaling/LocalTcpSignaling.h#L38) | `Stop` | 声明 | `void Stop()` | 停止 stop 相关逻辑。 |
| [L39](../src/signaling/LocalTcpSignaling.h#L39) | `LastError` | 声明 | `std::string LastError() const` | 实现 last error 对应的业务或工具逻辑。 |

## `src/signaling/QtWebSocketSignalingClient.Connection.cpp`

[打开源码](../src/signaling/QtWebSocketSignalingClient.Connection.cpp) · **文件作用：** 实现 qt web socket signaling client connection 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/signaling/QtWebSocketSignalingClient.Connection.cpp#L9) | `QtWebSocketSignalingClient::Impl::SendHeartbeat` | 定义 | `void QtWebSocketSignalingClient::Impl::SendHeartbeat()` | 发送或发布 send heartbeat 相关逻辑。 |
| [L31](../src/signaling/QtWebSocketSignalingClient.Connection.cpp#L31) | `QtWebSocketSignalingClient::Impl::OpenConfiguredSocket` | 定义 | `void QtWebSocketSignalingClient::Impl::OpenConfiguredSocket()` | 启动 open configured socket 相关逻辑。 |
| [L51](../src/signaling/QtWebSocketSignalingClient.Connection.cpp#L51) | `QtWebSocketSignalingClient::Impl::ScheduleReconnect` | 定义 | `void QtWebSocketSignalingClient::Impl::ScheduleReconnect()` | 执行后台循环或调度 schedule reconnect 相关逻辑。 |

## `src/signaling/QtWebSocketSignalingClient.cpp`

[打开源码](../src/signaling/QtWebSocketSignalingClient.cpp) · **文件作用：** 实现 qt web socket signaling client 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L40](../src/signaling/QtWebSocketSignalingClient.cpp#L40) | `Success` | 定义 | `SignalingOperationResult Success()` | 实现 success 对应的业务或工具逻辑。 |
| [L45](../src/signaling/QtWebSocketSignalingClient.cpp#L45) | `Failure` | 定义 | `SignalingOperationResult Failure(std::string code, std::string message)` | 实现 failure 对应的业务或工具逻辑。 |
| [L50](../src/signaling/QtWebSocketSignalingClient.cpp#L50) | `IsNineDigitPublicId` | 定义 | `bool IsNineDigitPublicId(std::string_view value)` | 判断 is nine digit public id 相关逻辑。 |
| [L59](../src/signaling/QtWebSocketSignalingClient.cpp#L59) | `ShouldBypassProxy` | 定义 | `bool ShouldBypassProxy(const QUrl& endpoint)` | 判断 should bypass proxy 相关逻辑。 |
| [L66](../src/signaling/QtWebSocketSignalingClient.cpp#L66) | `PurposeToString` | 定义 | `QString PurposeToString(SessionPurpose purpose)` | 实现 purpose to string 对应的业务或工具逻辑。 |
| [L78](../src/signaling/QtWebSocketSignalingClient.cpp#L78) | `PurposeFromString` | 定义 | `SessionPurpose PurposeFromString(const QString& purpose)` | 实现 purpose from string 对应的业务或工具逻辑。 |
| [L89](../src/signaling/QtWebSocketSignalingClient.cpp#L89) | `ScreenShareStateFromString` | 定义 | `RoomScreenShareState ScreenShareStateFromString(const QString& state, bool* valid)` | 实现 screen share state from string 对应的业务或工具逻辑。 |
| [L109](../src/signaling/QtWebSocketSignalingClient.cpp#L109) | `ReadRoomSnapshot` | 定义 | `bool ReadRoomSnapshot(const QJsonValue& value, RoomSnapshot* snapshot)` | 读取或恢复 read room snapshot 相关逻辑。 |
| [L225](../src/signaling/QtWebSocketSignalingClient.cpp#L225) | `QtWebSocketSignalingClient::QtWebSocketSignalingClient` | 定义 | `QtWebSocketSignalingClient::QtWebSocketSignalingClient() : impl_(std::make_unique<Impl>()) {}` | 构造并初始化 QtWebSocketSignalingClient 实例。 |
| [L231](../src/signaling/QtWebSocketSignalingClient.cpp#L231) | `QtWebSocketSignalingClient::SetObserver` | 定义 | `void QtWebSocketSignalingClient::SetObserver( ISignalingClientObserver* observer)` | 更新或应用 set observer 相关逻辑。 |
| [L237](../src/signaling/QtWebSocketSignalingClient.cpp#L237) | `QtWebSocketSignalingClient::Connect` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Connect( const SignalingClientConfig& config)` | 建立连接 connect 相关逻辑。 |
| [L243](../src/signaling/QtWebSocketSignalingClient.cpp#L243) | `QtWebSocketSignalingClient::UpdateAccessToken` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::UpdateAccessToken( const std::string& accessToken)` | 更新或应用 update access token 相关逻辑。 |
| [L249](../src/signaling/QtWebSocketSignalingClient.cpp#L249) | `QtWebSocketSignalingClient::RequestAccountDeletion` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::RequestAccountDeletion()` | 发起请求或查询 request account deletion 相关逻辑。 |
| [L254](../src/signaling/QtWebSocketSignalingClient.cpp#L254) | `QtWebSocketSignalingClient::RequestOwnedDevices` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::RequestOwnedDevices()` | 发起请求或查询 request owned devices 相关逻辑。 |
| [L259](../src/signaling/QtWebSocketSignalingClient.cpp#L259) | `QtWebSocketSignalingClient::Disconnect` | 定义 | `void QtWebSocketSignalingClient::Disconnect()` | 断开连接 disconnect 相关逻辑。 |
| [L264](../src/signaling/QtWebSocketSignalingClient.cpp#L264) | `QtWebSocketSignalingClient::State` | 定义 | `SignalingConnectionState QtWebSocketSignalingClient::State() const` | 实现 state 对应的业务或工具逻辑。 |
| [L269](../src/signaling/QtWebSocketSignalingClient.cpp#L269) | `QtWebSocketSignalingClient::RequestSession` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::RequestSession( const std::string& targetDeviceId, SessionPurpose purpose, const std::vector<std::string>& permissions)` | 发起请求或查询 request session 相关逻辑。 |
| [L277](../src/signaling/QtWebSocketSignalingClient.cpp#L277) | `QtWebSocketSignalingClient::RequestOwnedDeviceSession` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::RequestOwnedDeviceSession( const std::string& targetDeviceId, SessionPurpose purpose, const std::vector<std::string>& permissions)` | 发起请求或查询 request owned device session 相关逻辑。 |
| [L286](../src/signaling/QtWebSocketSignalingClient.cpp#L286) | `QtWebSocketSignalingClient::RequestAssistedSession` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::RequestAssistedSession( const std::string& targetDeviceId, const std::string& verificationCode, const std::vector<std::string>& permissions)` | 发起请求或查询 request assisted session 相关逻辑。 |
| [L295](../src/signaling/QtWebSocketSignalingClient.cpp#L295) | `QtWebSocketSignalingClient::RespondToSession` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::RespondToSession( const std::string& sessionId, bool accepted, const std::string& reasonCode)` | 处理并回复 respond to session 相关逻辑。 |
| [L303](../src/signaling/QtWebSocketSignalingClient.cpp#L303) | `QtWebSocketSignalingClient::CancelSession` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::CancelSession( const std::string& sessionId, const std::string& reasonCode)` | 判断 cancel session 相关逻辑。 |
| [L310](../src/signaling/QtWebSocketSignalingClient.cpp#L310) | `QtWebSocketSignalingClient::CloseSession` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::CloseSession( const std::string& sessionId, const std::string& reasonCode)` | 关闭并清理 close session 相关逻辑。 |
| [L317](../src/signaling/QtWebSocketSignalingClient.cpp#L317) | `QtWebSocketSignalingClient::ResumeSession` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::ResumeSession( const std::string& sessionId, const std::string& recoveryToken)` | 实现 resume session 对应的业务或工具逻辑。 |
| [L324](../src/signaling/QtWebSocketSignalingClient.cpp#L324) | `QtWebSocketSignalingClient::CreateRoom` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::CreateRoom( std::uint32_t capacity)` | 创建或初始化 create room 相关逻辑。 |
| [L330](../src/signaling/QtWebSocketSignalingClient.cpp#L330) | `QtWebSocketSignalingClient::RequestRoomJoin` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::RequestRoomJoin( const std::string& roomId)` | 发起请求或查询 request room join 相关逻辑。 |
| [L336](../src/signaling/QtWebSocketSignalingClient.cpp#L336) | `QtWebSocketSignalingClient::QueryRoomAvailability` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::QueryRoomAvailability( const std::vector<std::string>& roomIds)` | 发起请求或查询 query room availability 相关逻辑。 |
| [L342](../src/signaling/QtWebSocketSignalingClient.cpp#L342) | `QtWebSocketSignalingClient::RespondToRoomJoin` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::RespondToRoomJoin( const std::string& roomId, const std::string& requestId, bool accepted, const std::string& reasonCode)` | 处理并回复 respond to room join 相关逻辑。 |
| [L351](../src/signaling/QtWebSocketSignalingClient.cpp#L351) | `QtWebSocketSignalingClient::SetRoomCapacity` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::SetRoomCapacity( const std::string& roomId, std::uint32_t capacity)` | 更新或应用 set room capacity 相关逻辑。 |
| [L358](../src/signaling/QtWebSocketSignalingClient.cpp#L358) | `QtWebSocketSignalingClient::LeaveRoom` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::LeaveRoom( const std::string& roomId, const std::string& reasonCode)` | 实现 leave room 对应的业务或工具逻辑。 |
| [L365](../src/signaling/QtWebSocketSignalingClient.cpp#L365) | `QtWebSocketSignalingClient::ResumeRoom` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::ResumeRoom( const std::string& roomId, const std::string& recoveryToken)` | 实现 resume room 对应的业务或工具逻辑。 |
| [L372](../src/signaling/QtWebSocketSignalingClient.cpp#L372) | `QtWebSocketSignalingClient::SetRoomMediaState` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::SetRoomMediaState( const std::string& roomId, bool cameraPublishing, bool microphonePublishing)` | 更新或应用 set room media state 相关逻辑。 |
| [L381](../src/signaling/QtWebSocketSignalingClient.cpp#L381) | `QtWebSocketSignalingClient::RequestRoomScreenShare` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::RequestRoomScreenShare( const std::string& roomId)` | 发起请求或查询 request room screen share 相关逻辑。 |
| [L387](../src/signaling/QtWebSocketSignalingClient.cpp#L387) | `QtWebSocketSignalingClient::ConfirmRoomScreenShare` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::ConfirmRoomScreenShare( const std::string& roomId, const std::string& grantId)` | 实现 confirm room screen share 对应的业务或工具逻辑。 |
| [L394](../src/signaling/QtWebSocketSignalingClient.cpp#L394) | `QtWebSocketSignalingClient::StopRoomScreenShare` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::StopRoomScreenShare( const std::string& roomId, const std::string& grantId, const std::string& reasonCode)` | 停止 stop room screen share 相关逻辑。 |
| [L403](../src/signaling/QtWebSocketSignalingClient.cpp#L403) | `QtWebSocketSignalingClient::RespondToRoomScreenShareSwitch` | 定义 | `QtWebSocketSignalingClient::RespondToRoomScreenShareSwitch( const std::string& roomId, const std::string& requestId, bool accepted, const std::string& reasonCode)` | 处理并回复 respond to room screen share switch 相关逻辑。 |
| [L414](../src/signaling/QtWebSocketSignalingClient.cpp#L414) | `QtWebSocketSignalingClient::CancelRoomScreenShareSwitch` | 定义 | `QtWebSocketSignalingClient::CancelRoomScreenShareSwitch( const std::string& roomId, const std::string& requestId, const std::string& reasonCode)` | 判断 cancel room screen share switch 相关逻辑。 |
| [L423](../src/signaling/QtWebSocketSignalingClient.cpp#L423) | `QtWebSocketSignalingClient::RequestRoomControl` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::RequestRoomControl( const std::string& roomId)` | 发起请求或查询 request room control 相关逻辑。 |
| [L429](../src/signaling/QtWebSocketSignalingClient.cpp#L429) | `QtWebSocketSignalingClient::RespondToRoomControl` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::RespondToRoomControl( const std::string& roomId, const std::string& requestId, bool accepted, const std::string& reasonCode)` | 处理并回复 respond to room control 相关逻辑。 |
| [L439](../src/signaling/QtWebSocketSignalingClient.cpp#L439) | `QtWebSocketSignalingClient::ReleaseRoomControl` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::ReleaseRoomControl( const std::string& roomId, const std::string& grantId, const std::string& reasonCode)` | 释放或取消 release room control 相关逻辑。 |
| [L447](../src/signaling/QtWebSocketSignalingClient.cpp#L447) | `QtWebSocketSignalingClient::SendDescription` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::SendDescription( const SignalingSessionDescription& description)` | 发送或发布 send description 相关逻辑。 |
| [L453](../src/signaling/QtWebSocketSignalingClient.cpp#L453) | `QtWebSocketSignalingClient::SendIceCandidate` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::SendIceCandidate( const SignalingIceCandidate& candidate)` | 发送或发布 send ice candidate 相关逻辑。 |
| [L459](../src/signaling/QtWebSocketSignalingClient.cpp#L459) | `QtWebSocketSignalingClient::SendIceRestartRequest` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::SendIceRestartRequest( const SignalingIceRestartRequest& request)` | 发送或发布 send ice restart request 相关逻辑。 |
| [L465](../src/signaling/QtWebSocketSignalingClient.cpp#L465) | `QtWebSocketSignalingClient::SendIceRestartCancel` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::SendIceRestartCancel( const SignalingIceRestartCancel& cancel)` | 发送或发布 send ice restart cancel 相关逻辑。 |
| [L471](../src/signaling/QtWebSocketSignalingClient.cpp#L471) | `QtWebSocketSignalingClient::AbortConnectionForTesting` | 定义 | `void QtWebSocketSignalingClient::AbortConnectionForTesting()` | 释放或取消 abort connection for testing 相关逻辑。 |

## `src/signaling/QtWebSocketSignalingClient.Dispatch.cpp`

[打开源码](../src/signaling/QtWebSocketSignalingClient.Dispatch.cpp) · **文件作用：** 实现 qt web socket signaling client dispatch 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/signaling/QtWebSocketSignalingClient.Dispatch.cpp#L9) | `QtWebSocketSignalingClient::Impl::DispatchMessage` | 定义 | `void QtWebSocketSignalingClient::Impl::DispatchMessage( const QString& type, const std::string& sessionId, const QJsonObject& payload)` | 接收并处理 dispatch message 相关逻辑。 |

## `src/signaling/QtWebSocketSignalingClient.DispatchConnection.cpp`

[打开源码](../src/signaling/QtWebSocketSignalingClient.DispatchConnection.cpp) · **文件作用：** 实现 qt web socket signaling client dispatch connection 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/signaling/QtWebSocketSignalingClient.DispatchConnection.cpp#L9) | `QtWebSocketSignalingClient::Impl::DispatchConnectionMessage` | 定义 | `bool QtWebSocketSignalingClient::Impl::DispatchConnectionMessage( const QString& type, const std::string& sessionId, const QJsonObject& payload)` | 接收并处理 dispatch connection message 相关逻辑。 |

## `src/signaling/QtWebSocketSignalingClient.DispatchDirect.cpp`

[打开源码](../src/signaling/QtWebSocketSignalingClient.DispatchDirect.cpp) · **文件作用：** 实现 qt web socket signaling client dispatch direct 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/signaling/QtWebSocketSignalingClient.DispatchDirect.cpp#L9) | `QtWebSocketSignalingClient::Impl::DispatchDirectSessionMessage` | 定义 | `bool QtWebSocketSignalingClient::Impl::DispatchDirectSessionMessage( const QString& type, const std::string& sessionId, const QJsonObject& payload)` | 接收并处理 dispatch direct session message 相关逻辑。 |

## `src/signaling/QtWebSocketSignalingClient.DispatchRoom.cpp`

[打开源码](../src/signaling/QtWebSocketSignalingClient.DispatchRoom.cpp) · **文件作用：** 实现 qt web socket signaling client dispatch room 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/signaling/QtWebSocketSignalingClient.DispatchRoom.cpp#L9) | `QtWebSocketSignalingClient::Impl::DispatchRoomMessage` | 定义 | `bool QtWebSocketSignalingClient::Impl::DispatchRoomMessage( const QString& type, const std::string& sessionId, const QJsonObject& payload)` | 接收并处理 dispatch room message 相关逻辑。 |

## `src/signaling/QtWebSocketSignalingClient.DispatchTransport.cpp`

[打开源码](../src/signaling/QtWebSocketSignalingClient.DispatchTransport.cpp) · **文件作用：** 实现 qt web socket signaling client dispatch transport 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/signaling/QtWebSocketSignalingClient.DispatchTransport.cpp#L9) | `QtWebSocketSignalingClient::Impl::DispatchTransportMessage` | 定义 | `bool QtWebSocketSignalingClient::Impl::DispatchTransportMessage( const QString& type, const std::string& sessionId, const QJsonObject& payload)` | 接收并处理 dispatch transport message 相关逻辑。 |

## `src/signaling/QtWebSocketSignalingClient.h`

[打开源码](../src/signaling/QtWebSocketSignalingClient.h) · **文件作用：** 声明 qt web socket signaling client 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L12](../src/signaling/QtWebSocketSignalingClient.h#L12) | `QtWebSocketSignalingClient` | class | 定义 QtWebSocketSignalingClient 的 class 类型和相关状态。 |
| [L123](../src/signaling/QtWebSocketSignalingClient.h#L123) | `Impl` | class | 定义 Impl 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L123](../src/signaling/QtWebSocketSignalingClient.h#L123) | `Impl` | `class Impl;` | 保存 impl 相关配置或运行状态。 |
| [L124](../src/signaling/QtWebSocketSignalingClient.h#L124) | `impl_` | `std::unique_ptr<Impl> impl_;` | 保存 impl 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L14](../src/signaling/QtWebSocketSignalingClient.h#L14) | `QtWebSocketSignalingClient` | 声明 | `QtWebSocketSignalingClient()` | 实现 qt web socket signaling client 对应的业务或工具逻辑。 |
| [L15](../src/signaling/QtWebSocketSignalingClient.h#L15) | `~QtWebSocketSignalingClient` | 声明 | `~QtWebSocketSignalingClient() override` | 停止相关活动并释放 QtWebSocketSignalingClient 实例拥有的资源。 |
| [L17](../src/signaling/QtWebSocketSignalingClient.h#L17) | `QtWebSocketSignalingClient` | 声明 | `QtWebSocketSignalingClient(const QtWebSocketSignalingClient&) = delete` | 实现 qt web socket signaling client 对应的业务或工具逻辑。 |
| [L21](../src/signaling/QtWebSocketSignalingClient.h#L21) | `SetObserver` | 声明 | `void SetObserver(ISignalingClientObserver* observer) override` | 更新或应用 set observer 相关逻辑。 |
| [L22](../src/signaling/QtWebSocketSignalingClient.h#L22) | `Connect` | 声明 | `SignalingOperationResult Connect( const SignalingClientConfig& config) override` | 建立连接 connect 相关逻辑。 |
| [L24](../src/signaling/QtWebSocketSignalingClient.h#L24) | `UpdateAccessToken` | 声明 | `SignalingOperationResult UpdateAccessToken( const std::string& accessToken) override` | 更新或应用 update access token 相关逻辑。 |
| [L26](../src/signaling/QtWebSocketSignalingClient.h#L26) | `RequestAccountDeletion` | 声明 | `SignalingOperationResult RequestAccountDeletion() override` | 发起请求或查询 request account deletion 相关逻辑。 |
| [L27](../src/signaling/QtWebSocketSignalingClient.h#L27) | `RequestOwnedDevices` | 声明 | `SignalingOperationResult RequestOwnedDevices() override` | 发起请求或查询 request owned devices 相关逻辑。 |
| [L28](../src/signaling/QtWebSocketSignalingClient.h#L28) | `Disconnect` | 声明 | `void Disconnect() override` | 断开连接 disconnect 相关逻辑。 |
| [L29](../src/signaling/QtWebSocketSignalingClient.h#L29) | `State` | 声明 | `SignalingConnectionState State() const override` | 实现 state 对应的业务或工具逻辑。 |
| [L31](../src/signaling/QtWebSocketSignalingClient.h#L31) | `RequestSession` | 声明 | `SignalingOperationResult RequestSession( const std::string& targetDeviceId, SessionPurpose purpose, const std::vector<std::string>& permissions) override` | 发起请求或查询 request session 相关逻辑。 |
| [L35](../src/signaling/QtWebSocketSignalingClient.h#L35) | `RequestOwnedDeviceSession` | 声明 | `SignalingOperationResult RequestOwnedDeviceSession( const std::string& targetDeviceId, SessionPurpose purpose, const std::vector<std::string>& permissions) override` | 发起请求或查询 request owned device session 相关逻辑。 |
| [L39](../src/signaling/QtWebSocketSignalingClient.h#L39) | `RequestAssistedSession` | 声明 | `SignalingOperationResult RequestAssistedSession( const std::string& targetDeviceId, const std::string& verificationCode, const std::vector<std::string>& permissions) override` | 发起请求或查询 request assisted session 相关逻辑。 |
| [L43](../src/signaling/QtWebSocketSignalingClient.h#L43) | `RespondToSession` | 声明 | `SignalingOperationResult RespondToSession( const std::string& sessionId, bool accepted, const std::string& reasonCode) override` | 处理并回复 respond to session 相关逻辑。 |
| [L47](../src/signaling/QtWebSocketSignalingClient.h#L47) | `CancelSession` | 声明 | `SignalingOperationResult CancelSession( const std::string& sessionId, const std::string& reasonCode) override` | 判断 cancel session 相关逻辑。 |
| [L50](../src/signaling/QtWebSocketSignalingClient.h#L50) | `CloseSession` | 声明 | `SignalingOperationResult CloseSession( const std::string& sessionId, const std::string& reasonCode) override` | 关闭并清理 close session 相关逻辑。 |
| [L53](../src/signaling/QtWebSocketSignalingClient.h#L53) | `ResumeSession` | 声明 | `SignalingOperationResult ResumeSession( const std::string& sessionId, const std::string& recoveryToken) override` | 实现 resume session 对应的业务或工具逻辑。 |
| [L56](../src/signaling/QtWebSocketSignalingClient.h#L56) | `CreateRoom` | 声明 | `SignalingOperationResult CreateRoom( std::uint32_t capacity) override` | 创建或初始化 create room 相关逻辑。 |
| [L58](../src/signaling/QtWebSocketSignalingClient.h#L58) | `RequestRoomJoin` | 声明 | `SignalingOperationResult RequestRoomJoin( const std::string& roomId) override` | 发起请求或查询 request room join 相关逻辑。 |
| [L60](../src/signaling/QtWebSocketSignalingClient.h#L60) | `QueryRoomAvailability` | 声明 | `SignalingOperationResult QueryRoomAvailability( const std::vector<std::string>& roomIds) override` | 发起请求或查询 query room availability 相关逻辑。 |
| [L62](../src/signaling/QtWebSocketSignalingClient.h#L62) | `RespondToRoomJoin` | 声明 | `SignalingOperationResult RespondToRoomJoin( const std::string& roomId, const std::string& requestId, bool accepted, const std::string& reasonCode) override` | 处理并回复 respond to room join 相关逻辑。 |
| [L67](../src/signaling/QtWebSocketSignalingClient.h#L67) | `SetRoomCapacity` | 声明 | `SignalingOperationResult SetRoomCapacity( const std::string& roomId, std::uint32_t capacity) override` | 更新或应用 set room capacity 相关逻辑。 |
| [L70](../src/signaling/QtWebSocketSignalingClient.h#L70) | `LeaveRoom` | 声明 | `SignalingOperationResult LeaveRoom( const std::string& roomId, const std::string& reasonCode) override` | 实现 leave room 对应的业务或工具逻辑。 |
| [L73](../src/signaling/QtWebSocketSignalingClient.h#L73) | `ResumeRoom` | 声明 | `SignalingOperationResult ResumeRoom( const std::string& roomId, const std::string& recoveryToken) override` | 实现 resume room 对应的业务或工具逻辑。 |
| [L76](../src/signaling/QtWebSocketSignalingClient.h#L76) | `SetRoomMediaState` | 声明 | `SignalingOperationResult SetRoomMediaState( const std::string& roomId, bool cameraPublishing, bool microphonePublishing) override` | 更新或应用 set room media state 相关逻辑。 |
| [L80](../src/signaling/QtWebSocketSignalingClient.h#L80) | `RequestRoomScreenShare` | 声明 | `SignalingOperationResult RequestRoomScreenShare( const std::string& roomId) override` | 发起请求或查询 request room screen share 相关逻辑。 |
| [L82](../src/signaling/QtWebSocketSignalingClient.h#L82) | `ConfirmRoomScreenShare` | 声明 | `SignalingOperationResult ConfirmRoomScreenShare( const std::string& roomId, const std::string& grantId) override` | 实现 confirm room screen share 对应的业务或工具逻辑。 |
| [L85](../src/signaling/QtWebSocketSignalingClient.h#L85) | `StopRoomScreenShare` | 声明 | `SignalingOperationResult StopRoomScreenShare( const std::string& roomId, const std::string& grantId, const std::string& reasonCode) override` | 停止 stop room screen share 相关逻辑。 |
| [L89](../src/signaling/QtWebSocketSignalingClient.h#L89) | `RespondToRoomScreenShareSwitch` | 声明 | `SignalingOperationResult RespondToRoomScreenShareSwitch( const std::string& roomId, const std::string& requestId, bool accepted, const std::string& reasonCode) override` | 处理并回复 respond to room screen share switch 相关逻辑。 |
| [L94](../src/signaling/QtWebSocketSignalingClient.h#L94) | `CancelRoomScreenShareSwitch` | 声明 | `SignalingOperationResult CancelRoomScreenShareSwitch( const std::string& roomId, const std::string& requestId, const std::string& reasonCode) override` | 判断 cancel room screen share switch 相关逻辑。 |
| [L98](../src/signaling/QtWebSocketSignalingClient.h#L98) | `RequestRoomControl` | 声明 | `SignalingOperationResult RequestRoomControl( const std::string& roomId) override` | 发起请求或查询 request room control 相关逻辑。 |
| [L100](../src/signaling/QtWebSocketSignalingClient.h#L100) | `RespondToRoomControl` | 声明 | `SignalingOperationResult RespondToRoomControl( const std::string& roomId, const std::string& requestId, bool accepted, const std::string& reasonCode) override` | 处理并回复 respond to room control 相关逻辑。 |
| [L105](../src/signaling/QtWebSocketSignalingClient.h#L105) | `ReleaseRoomControl` | 声明 | `SignalingOperationResult ReleaseRoomControl( const std::string& roomId, const std::string& grantId, const std::string& reasonCode) override` | 释放或取消 release room control 相关逻辑。 |
| [L109](../src/signaling/QtWebSocketSignalingClient.h#L109) | `SendDescription` | 声明 | `SignalingOperationResult SendDescription( const SignalingSessionDescription& description) override` | 发送或发布 send description 相关逻辑。 |
| [L111](../src/signaling/QtWebSocketSignalingClient.h#L111) | `SendIceCandidate` | 声明 | `SignalingOperationResult SendIceCandidate( const SignalingIceCandidate& candidate) override` | 发送或发布 send ice candidate 相关逻辑。 |
| [L113](../src/signaling/QtWebSocketSignalingClient.h#L113) | `SendIceRestartRequest` | 声明 | `SignalingOperationResult SendIceRestartRequest( const SignalingIceRestartRequest& request) override` | 发送或发布 send ice restart request 相关逻辑。 |
| [L115](../src/signaling/QtWebSocketSignalingClient.h#L115) | `SendIceRestartCancel` | 声明 | `SignalingOperationResult SendIceRestartCancel( const SignalingIceRestartCancel& cancel) override` | 发送或发布 send ice restart cancel 相关逻辑。 |
| [L120](../src/signaling/QtWebSocketSignalingClient.h#L120) | `AbortConnectionForTesting` | 声明 | `void AbortConnectionForTesting()` | Integration tests use this to emulate a transient transport failure without stopping the signaling server or the active PeerConnection. |

## `src/signaling/QtWebSocketSignalingClient.Internal.h`

[打开源码](../src/signaling/QtWebSocketSignalingClient.Internal.h) · **文件作用：** 声明 qt web socket signaling client internal 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L64](../src/signaling/QtWebSocketSignalingClient.Internal.h#L64) | `QtWebSocketSignalingClient::Impl` | class | 定义 QtWebSocketSignalingClient::Impl 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L42](../src/signaling/QtWebSocketSignalingClient.Internal.h#L42) | `kProtocolVersion` | `inline constexpr int kProtocolVersion = 5;` | 定义 protocol version 的编译期常量或产品边界。 |
| [L43](../src/signaling/QtWebSocketSignalingClient.Internal.h#L43) | `kMaximumMessageBytes` | `inline constexpr quint64 kMaximumMessageBytes = 1024 * 1024;` | 定义 maximum message bytes 的编译期常量或产品边界。 |
| [L44](../src/signaling/QtWebSocketSignalingClient.Internal.h#L44) | `kMaximumAccessTokenBytes` | `inline constexpr std::size_t kMaximumAccessTokenBytes = 16 * 1024;` | 定义 maximum access token bytes 的编译期常量或产品边界。 |
| [L45](../src/signaling/QtWebSocketSignalingClient.Internal.h#L45) | `kMaximumRememberedMessageIds` | `inline constexpr std::size_t kMaximumRememberedMessageIds = 4096;` | 定义 maximum remembered message ids 的编译期常量或产品边界。 |
| [L271](../src/signaling/QtWebSocketSignalingClient.Internal.h#L271) | `socket_` | `QWebSocket socket_;` | 保存 socket 相关配置或运行状态。 |
| [L272](../src/signaling/QtWebSocketSignalingClient.Internal.h#L272) | `heartbeatTimer_` | `QTimer heartbeatTimer_;` | 保存定时、截止或超时状态：heartbeat timer。 |
| [L273](../src/signaling/QtWebSocketSignalingClient.Internal.h#L273) | `heartbeatDeadline_` | `QTimer heartbeatDeadline_;` | 保存定时、截止或超时状态：heartbeat deadline。 |
| [L274](../src/signaling/QtWebSocketSignalingClient.Internal.h#L274) | `authenticationDeadline_` | `QTimer authenticationDeadline_;` | 保存定时、截止或超时状态：authentication deadline。 |
| [L275](../src/signaling/QtWebSocketSignalingClient.Internal.h#L275) | `reconnectTimer_` | `QTimer reconnectTimer_;` | 保存定时、截止或超时状态：reconnect timer。 |
| [L276](../src/signaling/QtWebSocketSignalingClient.Internal.h#L276) | `heartbeatRoundTrip_` | `QElapsedTimer heartbeatRoundTrip_;` | 保存 heartbeat round trip 相关配置或运行状态。 |
| [L277](../src/signaling/QtWebSocketSignalingClient.Internal.h#L277) | `heartbeatOutstanding_` | `bool heartbeatOutstanding_ = false;` | 保存 heartbeat outstanding 相关配置或运行状态。 |
| [L278](../src/signaling/QtWebSocketSignalingClient.Internal.h#L278) | `manualDisconnect_` | `bool manualDisconnect_ = true;` | 保存 manual disconnect 相关配置或运行状态。 |
| [L279](../src/signaling/QtWebSocketSignalingClient.Internal.h#L279) | `fatalDisconnect_` | `bool fatalDisconnect_ = false;` | 保存 fatal disconnect 相关配置或运行状态。 |
| [L280](../src/signaling/QtWebSocketSignalingClient.Internal.h#L280) | `reconnectAttempt_` | `std::uint32_t reconnectAttempt_ = 0;` | 保存 reconnect attempt 相关配置或运行状态。 |
| [L281](../src/signaling/QtWebSocketSignalingClient.Internal.h#L281) | `observer_` | `ISignalingClientObserver* observer_ = nullptr;` | 保存回调或观察者入口：observer。 |
| [L282](../src/signaling/QtWebSocketSignalingClient.Internal.h#L282) | `config_` | `SignalingClientConfig config_;` | 保存 config 相关配置或运行状态。 |
| [L283](../src/signaling/QtWebSocketSignalingClient.Internal.h#L283) | `endpoint_` | `QUrl endpoint_;` | 保存 endpoint 相关配置或运行状态。 |
| [L286](../src/signaling/QtWebSocketSignalingClient.Internal.h#L286) | `receivedMessageIds_` | `std::unordered_set<std::string> receivedMessageIds_;` | 保存 received message ids 相关配置或运行状态。 |
| [L287](../src/signaling/QtWebSocketSignalingClient.Internal.h#L287) | `receivedMessageOrder_` | `std::deque<std::string> receivedMessageOrder_;` | 保存 received message order 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L52](../src/signaling/QtWebSocketSignalingClient.Internal.h#L52) | `Success` | 声明 | `SignalingOperationResult Success()` | 实现 success 对应的业务或工具逻辑。 |
| [L53](../src/signaling/QtWebSocketSignalingClient.Internal.h#L53) | `Failure` | 声明 | `SignalingOperationResult Failure(std::string code, std::string message)` | 实现 failure 对应的业务或工具逻辑。 |
| [L54](../src/signaling/QtWebSocketSignalingClient.Internal.h#L54) | `IsNineDigitPublicId` | 声明 | `bool IsNineDigitPublicId(std::string_view value)` | 判断 is nine digit public id 相关逻辑。 |
| [L55](../src/signaling/QtWebSocketSignalingClient.Internal.h#L55) | `ShouldBypassProxy` | 声明 | `bool ShouldBypassProxy(const QUrl& endpoint)` | 判断 should bypass proxy 相关逻辑。 |
| [L56](../src/signaling/QtWebSocketSignalingClient.Internal.h#L56) | `PurposeToString` | 声明 | `QString PurposeToString(SessionPurpose purpose)` | 实现 purpose to string 对应的业务或工具逻辑。 |
| [L57](../src/signaling/QtWebSocketSignalingClient.Internal.h#L57) | `PurposeFromString` | 声明 | `SessionPurpose PurposeFromString(const QString& purpose)` | 实现 purpose from string 对应的业务或工具逻辑。 |
| [L58](../src/signaling/QtWebSocketSignalingClient.Internal.h#L58) | `ScreenShareStateFromString` | 声明 | `RoomScreenShareState ScreenShareStateFromString( const QString& state, bool* valid)` | 实现 screen share state from string 对应的业务或工具逻辑。 |
| [L60](../src/signaling/QtWebSocketSignalingClient.Internal.h#L60) | `ReadRoomSnapshot` | 声明 | `bool ReadRoomSnapshot(const QJsonValue& value, RoomSnapshot* snapshot)` | 读取或恢复 read room snapshot 相关逻辑。 |
| [L66](../src/signaling/QtWebSocketSignalingClient.Internal.h#L66) | `Impl` | 声明 | `Impl()` | 实现 impl 对应的业务或工具逻辑。 |
| [L67](../src/signaling/QtWebSocketSignalingClient.Internal.h#L67) | `~Impl` | 声明 | `~Impl()` | 停止相关活动并释放 Impl 实例拥有的资源。 |
| [L68](../src/signaling/QtWebSocketSignalingClient.Internal.h#L68) | `SetObserver` | 声明 | `void SetObserver(ISignalingClientObserver* observer)` | 更新或应用 set observer 相关逻辑。 |
| [L69](../src/signaling/QtWebSocketSignalingClient.Internal.h#L69) | `Connect` | 声明 | `SignalingOperationResult Connect(const SignalingClientConfig& config)` | 建立连接 connect 相关逻辑。 |
| [L70](../src/signaling/QtWebSocketSignalingClient.Internal.h#L70) | `UpdateAccessToken` | 声明 | `SignalingOperationResult UpdateAccessToken( const std::string& accessToken)` | 更新或应用 update access token 相关逻辑。 |
| [L72](../src/signaling/QtWebSocketSignalingClient.Internal.h#L72) | `RequestAccountDeletion` | 声明 | `SignalingOperationResult RequestAccountDeletion()` | 发起请求或查询 request account deletion 相关逻辑。 |
| [L73](../src/signaling/QtWebSocketSignalingClient.Internal.h#L73) | `Disconnect` | 声明 | `void Disconnect()` | 断开连接 disconnect 相关逻辑。 |
| [L74](../src/signaling/QtWebSocketSignalingClient.Internal.h#L74) | `State` | 声明 | `SignalingConnectionState State() const` | 实现 state 对应的业务或工具逻辑。 |
| [L75](../src/signaling/QtWebSocketSignalingClient.Internal.h#L75) | `RequestOwnedDevices` | 声明 | `SignalingOperationResult RequestOwnedDevices()` | 发起请求或查询 request owned devices 相关逻辑。 |
| [L76](../src/signaling/QtWebSocketSignalingClient.Internal.h#L76) | `RequestSession` | 声明 | `SignalingOperationResult RequestSession( const std::string& targetDeviceId, SessionPurpose purpose, const std::vector<std::string>& permissions)` | 发起请求或查询 request session 相关逻辑。 |
| [L80](../src/signaling/QtWebSocketSignalingClient.Internal.h#L80) | `RequestOwnedDeviceSession` | 声明 | `SignalingOperationResult RequestOwnedDeviceSession( const std::string& targetDeviceId, SessionPurpose purpose, const std::vector<std::string>& permissions)` | 发起请求或查询 request owned device session 相关逻辑。 |
| [L84](../src/signaling/QtWebSocketSignalingClient.Internal.h#L84) | `RequestAssistedSession` | 声明 | `SignalingOperationResult RequestAssistedSession( const std::string& targetDeviceId, const std::string& verificationCode, const std::vector<std::string>& permissions)` | 发起请求或查询 request assisted session 相关逻辑。 |
| [L88](../src/signaling/QtWebSocketSignalingClient.Internal.h#L88) | `RespondToSession` | 声明 | `SignalingOperationResult RespondToSession( const std::string& sessionId, bool accepted, const std::string& reasonCode)` | 处理并回复 respond to session 相关逻辑。 |
| [L92](../src/signaling/QtWebSocketSignalingClient.Internal.h#L92) | `CancelSession` | 声明 | `SignalingOperationResult CancelSession( const std::string& sessionId, const std::string& reasonCode)` | 判断 cancel session 相关逻辑。 |
| [L95](../src/signaling/QtWebSocketSignalingClient.Internal.h#L95) | `CloseSession` | 声明 | `SignalingOperationResult CloseSession( const std::string& sessionId, const std::string& reasonCode)` | 关闭并清理 close session 相关逻辑。 |
| [L98](../src/signaling/QtWebSocketSignalingClient.Internal.h#L98) | `ResumeSession` | 声明 | `SignalingOperationResult ResumeSession( const std::string& sessionId, const std::string& recoveryToken)` | 实现 resume session 对应的业务或工具逻辑。 |
| [L101](../src/signaling/QtWebSocketSignalingClient.Internal.h#L101) | `CreateRoom` | 声明 | `SignalingOperationResult CreateRoom(std::uint32_t capacity)` | 创建或初始化 create room 相关逻辑。 |
| [L102](../src/signaling/QtWebSocketSignalingClient.Internal.h#L102) | `RequestRoomJoin` | 声明 | `SignalingOperationResult RequestRoomJoin(const std::string& roomId)` | 发起请求或查询 request room join 相关逻辑。 |
| [L103](../src/signaling/QtWebSocketSignalingClient.Internal.h#L103) | `QueryRoomAvailability` | 声明 | `SignalingOperationResult QueryRoomAvailability( const std::vector<std::string>& roomIds)` | 发起请求或查询 query room availability 相关逻辑。 |
| [L105](../src/signaling/QtWebSocketSignalingClient.Internal.h#L105) | `RespondToRoomJoin` | 声明 | `SignalingOperationResult RespondToRoomJoin( const std::string& roomId, const std::string& requestId, bool accepted, const std::string& reasonCode)` | 处理并回复 respond to room join 相关逻辑。 |
| [L110](../src/signaling/QtWebSocketSignalingClient.Internal.h#L110) | `SetRoomCapacity` | 声明 | `SignalingOperationResult SetRoomCapacity(const std::string& roomId, std::uint32_t capacity)` | 更新或应用 set room capacity 相关逻辑。 |
| [L112](../src/signaling/QtWebSocketSignalingClient.Internal.h#L112) | `LeaveRoom` | 声明 | `SignalingOperationResult LeaveRoom(const std::string& roomId, const std::string& reasonCode)` | 实现 leave room 对应的业务或工具逻辑。 |
| [L114](../src/signaling/QtWebSocketSignalingClient.Internal.h#L114) | `ResumeRoom` | 声明 | `SignalingOperationResult ResumeRoom(const std::string& roomId, const std::string& recoveryToken)` | 实现 resume room 对应的业务或工具逻辑。 |
| [L116](../src/signaling/QtWebSocketSignalingClient.Internal.h#L116) | `SetRoomMediaState` | 声明 | `SignalingOperationResult SetRoomMediaState( const std::string& roomId, bool cameraPublishing, bool microphonePublishing)` | 更新或应用 set room media state 相关逻辑。 |
| [L120](../src/signaling/QtWebSocketSignalingClient.Internal.h#L120) | `RequestRoomScreenShare` | 声明 | `SignalingOperationResult RequestRoomScreenShare( const std::string& roomId)` | 发起请求或查询 request room screen share 相关逻辑。 |
| [L122](../src/signaling/QtWebSocketSignalingClient.Internal.h#L122) | `ConfirmRoomScreenShare` | 声明 | `SignalingOperationResult ConfirmRoomScreenShare( const std::string& roomId, const std::string& grantId)` | 实现 confirm room screen share 对应的业务或工具逻辑。 |
| [L125](../src/signaling/QtWebSocketSignalingClient.Internal.h#L125) | `StopRoomScreenShare` | 声明 | `SignalingOperationResult StopRoomScreenShare( const std::string& roomId, const std::string& grantId, const std::string& reasonCode)` | 停止 stop room screen share 相关逻辑。 |
| [L129](../src/signaling/QtWebSocketSignalingClient.Internal.h#L129) | `RespondToRoomScreenShareSwitch` | 声明 | `SignalingOperationResult RespondToRoomScreenShareSwitch( const std::string& roomId, const std::string& requestId, bool accepted, const std::string& reasonCode)` | 处理并回复 respond to room screen share switch 相关逻辑。 |
| [L134](../src/signaling/QtWebSocketSignalingClient.Internal.h#L134) | `CancelRoomScreenShareSwitch` | 声明 | `SignalingOperationResult CancelRoomScreenShareSwitch( const std::string& roomId, const std::string& requestId, const std::string& reasonCode)` | 判断 cancel room screen share switch 相关逻辑。 |
| [L138](../src/signaling/QtWebSocketSignalingClient.Internal.h#L138) | `RequestRoomControl` | 声明 | `SignalingOperationResult RequestRoomControl(const std::string& roomId)` | 发起请求或查询 request room control 相关逻辑。 |
| [L139](../src/signaling/QtWebSocketSignalingClient.Internal.h#L139) | `RespondToRoomControl` | 声明 | `SignalingOperationResult RespondToRoomControl( const std::string& roomId, const std::string& requestId, bool accepted, const std::string& reasonCode)` | 处理并回复 respond to room control 相关逻辑。 |
| [L144](../src/signaling/QtWebSocketSignalingClient.Internal.h#L144) | `ReleaseRoomControl` | 声明 | `SignalingOperationResult ReleaseRoomControl( const std::string& roomId, const std::string& grantId, const std::string& reasonCode)` | 释放或取消 release room control 相关逻辑。 |
| [L148](../src/signaling/QtWebSocketSignalingClient.Internal.h#L148) | `AbortConnectionForTesting` | 声明 | `void AbortConnectionForTesting()` | 释放或取消 abort connection for testing 相关逻辑。 |
| [L149](../src/signaling/QtWebSocketSignalingClient.Internal.h#L149) | `SendDescription` | 声明 | `SignalingOperationResult SendDescription( const SignalingSessionDescription& description)` | 发送或发布 send description 相关逻辑。 |
| [L151](../src/signaling/QtWebSocketSignalingClient.Internal.h#L151) | `SendIceCandidate` | 声明 | `SignalingOperationResult SendIceCandidate( const SignalingIceCandidate& candidate)` | 发送或发布 send ice candidate 相关逻辑。 |
| [L153](../src/signaling/QtWebSocketSignalingClient.Internal.h#L153) | `SendIceRestartRequest` | 声明 | `SignalingOperationResult SendIceRestartRequest( const SignalingIceRestartRequest& request)` | 发送或发布 send ice restart request 相关逻辑。 |
| [L155](../src/signaling/QtWebSocketSignalingClient.Internal.h#L155) | `SendIceRestartCancel` | 声明 | `SignalingOperationResult SendIceRestartCancel( const SignalingIceRestartCancel& cancel)` | 发送或发布 send ice restart cancel 相关逻辑。 |
| [L158](../src/signaling/QtWebSocketSignalingClient.Internal.h#L158) | `SendSessionEndAction` | 声明 | `SignalingOperationResult SendSessionEndAction( const QString& type, const std::string& sessionId, const std::string& reasonCode)` | 发送或发布 send session end action 相关逻辑。 |
| [L162](../src/signaling/QtWebSocketSignalingClient.Internal.h#L162) | `SendHeartbeat` | 声明 | `void SendHeartbeat()` | 发送或发布 send heartbeat 相关逻辑。 |
| [L163](../src/signaling/QtWebSocketSignalingClient.Internal.h#L163) | `OpenConfiguredSocket` | 声明 | `void OpenConfiguredSocket()` | 启动 open configured socket 相关逻辑。 |
| [L164](../src/signaling/QtWebSocketSignalingClient.Internal.h#L164) | `ScheduleReconnect` | 声明 | `void ScheduleReconnect()` | 执行后台循环或调度 schedule reconnect 相关逻辑。 |
| [L165](../src/signaling/QtWebSocketSignalingClient.Internal.h#L165) | `SendDescriptionOnOwnerThread` | 声明 | `SignalingOperationResult SendDescriptionOnOwnerThread( const SignalingSessionDescription& description)` | 发送或发布 send description on owner thread 相关逻辑。 |
| [L167](../src/signaling/QtWebSocketSignalingClient.Internal.h#L167) | `SendIceCandidateOnOwnerThread` | 声明 | `SignalingOperationResult SendIceCandidateOnOwnerThread( const SignalingIceCandidate& candidate)` | 发送或发布 send ice candidate on owner thread 相关逻辑。 |
| [L169](../src/signaling/QtWebSocketSignalingClient.Internal.h#L169) | `OnConnected` | 声明 | `void OnConnected()` | 接收并处理 on connected 相关逻辑。 |
| [L170](../src/signaling/QtWebSocketSignalingClient.Internal.h#L170) | `BeginDeviceRegistration` | 声明 | `void BeginDeviceRegistration()` | 启动 begin device registration 相关逻辑。 |
| [L171](../src/signaling/QtWebSocketSignalingClient.Internal.h#L171) | `OnDisconnected` | 声明 | `void OnDisconnected()` | 接收并处理 on disconnected 相关逻辑。 |
| [L172](../src/signaling/QtWebSocketSignalingClient.Internal.h#L172) | `OnTextMessage` | 声明 | `void OnTextMessage(const QString& message)` | 接收并处理 on text message 相关逻辑。 |
| [L173](../src/signaling/QtWebSocketSignalingClient.Internal.h#L173) | `DispatchMessage` | 声明 | `void DispatchMessage(const QString& type, const std::string& sessionId, const QJsonObject& payload)` | 接收并处理 dispatch message 相关逻辑。 |
| [L176](../src/signaling/QtWebSocketSignalingClient.Internal.h#L176) | `DispatchConnectionMessage` | 声明 | `bool DispatchConnectionMessage(const QString& type, const std::string& sessionId, const QJsonObject& payload)` | 接收并处理 dispatch connection message 相关逻辑。 |
| [L179](../src/signaling/QtWebSocketSignalingClient.Internal.h#L179) | `DispatchDirectSessionMessage` | 声明 | `bool DispatchDirectSessionMessage(const QString& type, const std::string& sessionId, const QJsonObject& payload)` | 接收并处理 dispatch direct session message 相关逻辑。 |
| [L182](../src/signaling/QtWebSocketSignalingClient.Internal.h#L182) | `DispatchRoomMessage` | 声明 | `bool DispatchRoomMessage(const QString& type, const std::string& sessionId, const QJsonObject& payload)` | 接收并处理 dispatch room message 相关逻辑。 |
| [L185](../src/signaling/QtWebSocketSignalingClient.Internal.h#L185) | `DispatchTransportMessage` | 声明 | `bool DispatchTransportMessage(const QString& type, const std::string& sessionId, const QJsonObject& payload)` | 接收并处理 dispatch transport message 相关逻辑。 |
| [L188](../src/signaling/QtWebSocketSignalingClient.Internal.h#L188) | `SendEnvelope` | 定义 | `SignalingOperationResult SendEnvelope(const QString& type, const std::string& sessionId, const QJsonObject& payload)` | 发送或发布 send envelope 相关逻辑。 |
| [L221](../src/signaling/QtWebSocketSignalingClient.Internal.h#L221) | `RequireRegistered` | 定义 | `SignalingOperationResult RequireRegistered() const` | 实现 require registered 对应的业务或工具逻辑。 |
| [L231](../src/signaling/QtWebSocketSignalingClient.Internal.h#L231) | `RememberMessageId` | 定义 | `bool RememberMessageId(const std::string& messageId)` | 实现 remember message id 对应的业务或工具逻辑。 |
| [L245](../src/signaling/QtWebSocketSignalingClient.Internal.h#L245) | `SetState` | 定义 | `void SetState(SignalingConnectionState state)` | 更新或应用 set state 相关逻辑。 |
| [L255](../src/signaling/QtWebSocketSignalingClient.Internal.h#L255) | `NotifyError` | 定义 | `void NotifyError(const std::string& code, const std::string& message)` | 通知或报告 notify error 相关逻辑。 |
| [L262](../src/signaling/QtWebSocketSignalingClient.Internal.h#L262) | `Fail` | 定义 | `void Fail(const std::string& code, const std::string& message)` | 实现 fail 对应的业务或工具逻辑。 |

## `src/signaling/QtWebSocketSignalingClient.LegacySession.cpp`

[打开源码](../src/signaling/QtWebSocketSignalingClient.LegacySession.cpp) · **文件作用：** 实现 qt web socket signaling client legacy session 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/signaling/QtWebSocketSignalingClient.LegacySession.cpp#L9) | `QtWebSocketSignalingClient::Impl::RequestSession` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::RequestSession( const std::string& targetDeviceId, SessionPurpose purpose, const std::vector<std::string>& permissions)` | 发起请求或查询 request session 相关逻辑。 |
| [L30](../src/signaling/QtWebSocketSignalingClient.LegacySession.cpp#L30) | `QtWebSocketSignalingClient::Impl::RequestOwnedDeviceSession` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::RequestOwnedDeviceSession( const std::string& targetDeviceId, SessionPurpose purpose, const std::vector<std::string>& permissions)` | 发起请求或查询 request owned device session 相关逻辑。 |
| [L52](../src/signaling/QtWebSocketSignalingClient.LegacySession.cpp#L52) | `QtWebSocketSignalingClient::Impl::RequestAssistedSession` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::RequestAssistedSession( const std::string& targetDeviceId, const std::string& verificationCode, const std::vector<std::string>& permissions)` | 发起请求或查询 request assisted session 相关逻辑。 |
| [L80](../src/signaling/QtWebSocketSignalingClient.LegacySession.cpp#L80) | `QtWebSocketSignalingClient::Impl::RespondToSession` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::RespondToSession( const std::string& sessionId, bool accepted, const std::string& reasonCode)` | 处理并回复 respond to session 相关逻辑。 |
| [L102](../src/signaling/QtWebSocketSignalingClient.LegacySession.cpp#L102) | `QtWebSocketSignalingClient::Impl::CancelSession` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::CancelSession( const std::string& sessionId, const std::string& reasonCode)` | 判断 cancel session 相关逻辑。 |
| [L110](../src/signaling/QtWebSocketSignalingClient.LegacySession.cpp#L110) | `QtWebSocketSignalingClient::Impl::CloseSession` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::CloseSession( const std::string& sessionId, const std::string& reasonCode)` | 关闭并清理 close session 相关逻辑。 |
| [L118](../src/signaling/QtWebSocketSignalingClient.LegacySession.cpp#L118) | `QtWebSocketSignalingClient::Impl::ResumeSession` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::ResumeSession( const std::string& sessionId, const std::string& recoveryToken)` | 实现 resume session 对应的业务或工具逻辑。 |

## `src/signaling/QtWebSocketSignalingClient.Lifecycle.cpp`

[打开源码](../src/signaling/QtWebSocketSignalingClient.Lifecycle.cpp) · **文件作用：** 实现 qt web socket signaling client lifecycle 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/signaling/QtWebSocketSignalingClient.Lifecycle.cpp#L9) | `QtWebSocketSignalingClient::Impl::Impl` | 定义 | `QtWebSocketSignalingClient::Impl::Impl()` | 构造并初始化 Impl 实例。 |
| [L77](../src/signaling/QtWebSocketSignalingClient.Lifecycle.cpp#L77) | `QtWebSocketSignalingClient::Impl::~Impl` | 定义 | `QtWebSocketSignalingClient::Impl::~Impl()` | 停止相关活动并释放 Impl 实例拥有的资源。 |
| [L86](../src/signaling/QtWebSocketSignalingClient.Lifecycle.cpp#L86) | `QtWebSocketSignalingClient::Impl::SetObserver` | 定义 | `void QtWebSocketSignalingClient::Impl::SetObserver(ISignalingClientObserver* observer)` | 更新或应用 set observer 相关逻辑。 |
| [L94](../src/signaling/QtWebSocketSignalingClient.Lifecycle.cpp#L94) | `QtWebSocketSignalingClient::Impl::Connect` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::Connect(const SignalingClientConfig& config)` | 建立连接 connect 相关逻辑。 |
| [L185](../src/signaling/QtWebSocketSignalingClient.Lifecycle.cpp#L185) | `QtWebSocketSignalingClient::Impl::UpdateAccessToken` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::UpdateAccessToken( const std::string& accessToken)` | 更新或应用 update access token 相关逻辑。 |
| [L197](../src/signaling/QtWebSocketSignalingClient.Lifecycle.cpp#L197) | `QtWebSocketSignalingClient::Impl::RequestAccountDeletion` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::RequestAccountDeletion()` | 发起请求或查询 request account deletion 相关逻辑。 |
| [L207](../src/signaling/QtWebSocketSignalingClient.Lifecycle.cpp#L207) | `QtWebSocketSignalingClient::Impl::Disconnect` | 定义 | `void QtWebSocketSignalingClient::Impl::Disconnect()` | 断开连接 disconnect 相关逻辑。 |
| [L229](../src/signaling/QtWebSocketSignalingClient.Lifecycle.cpp#L229) | `QtWebSocketSignalingClient::Impl::State` | 定义 | `SignalingConnectionState QtWebSocketSignalingClient::Impl::State() const` | 实现 state 对应的业务或工具逻辑。 |

## `src/signaling/QtWebSocketSignalingClient.Negotiation.cpp`

[打开源码](../src/signaling/QtWebSocketSignalingClient.Negotiation.cpp) · **文件作用：** 实现 qt web socket signaling client negotiation 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/signaling/QtWebSocketSignalingClient.Negotiation.cpp#L9) | `QtWebSocketSignalingClient::Impl::SendDescriptionOnOwnerThread` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::SendDescriptionOnOwnerThread( const SignalingSessionDescription& description)` | 发送或发布 send description on owner thread 相关逻辑。 |
| [L24](../src/signaling/QtWebSocketSignalingClient.Negotiation.cpp#L24) | `QtWebSocketSignalingClient::Impl::SendIceCandidateOnOwnerThread` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::SendIceCandidateOnOwnerThread( const SignalingIceCandidate& candidate)` | 发送或发布 send ice candidate on owner thread 相关逻辑。 |

## `src/signaling/QtWebSocketSignalingClient.OwnedDevices.cpp`

[打开源码](../src/signaling/QtWebSocketSignalingClient.OwnedDevices.cpp) · **文件作用：** 实现 qt web socket signaling client owned devices 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/signaling/QtWebSocketSignalingClient.OwnedDevices.cpp#L9) | `QtWebSocketSignalingClient::Impl::RequestOwnedDevices` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::RequestOwnedDevices()` | 发起请求或查询 request owned devices 相关逻辑。 |

## `src/signaling/QtWebSocketSignalingClient.PairTransport.cpp`

[打开源码](../src/signaling/QtWebSocketSignalingClient.PairTransport.cpp) · **文件作用：** 实现 qt web socket signaling client pair transport 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/signaling/QtWebSocketSignalingClient.PairTransport.cpp#L9) | `QtWebSocketSignalingClient::Impl::AbortConnectionForTesting` | 定义 | `void QtWebSocketSignalingClient::Impl::AbortConnectionForTesting()` | 释放或取消 abort connection for testing 相关逻辑。 |
| [L19](../src/signaling/QtWebSocketSignalingClient.PairTransport.cpp#L19) | `QtWebSocketSignalingClient::Impl::SendDescription` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::SendDescription( const SignalingSessionDescription& description)` | 发送或发布 send description 相关逻辑。 |
| [L50](../src/signaling/QtWebSocketSignalingClient.PairTransport.cpp#L50) | `QtWebSocketSignalingClient::Impl::SendIceCandidate` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::SendIceCandidate( const SignalingIceCandidate& candidate)` | 发送或发布 send ice candidate 相关逻辑。 |
| [L81](../src/signaling/QtWebSocketSignalingClient.PairTransport.cpp#L81) | `QtWebSocketSignalingClient::Impl::SendIceRestartRequest` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::SendIceRestartRequest( const SignalingIceRestartRequest& request)` | 发送或发布 send ice restart request 相关逻辑。 |
| [L116](../src/signaling/QtWebSocketSignalingClient.PairTransport.cpp#L116) | `QtWebSocketSignalingClient::Impl::SendIceRestartCancel` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::SendIceRestartCancel( const SignalingIceRestartCancel& cancel)` | 发送或发布 send ice restart cancel 相关逻辑。 |
| [L151](../src/signaling/QtWebSocketSignalingClient.PairTransport.cpp#L151) | `QtWebSocketSignalingClient::Impl::SendSessionEndAction` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::SendSessionEndAction( const QString& type, const std::string& sessionId, const std::string& reasonCode)` | 发送或发布 send session end action 相关逻辑。 |

## `src/signaling/QtWebSocketSignalingClient.Recovery.cpp`

[打开源码](../src/signaling/QtWebSocketSignalingClient.Recovery.cpp) · **文件作用：** 实现 qt web socket signaling client recovery 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/signaling/QtWebSocketSignalingClient.Recovery.cpp#L9) | `QtWebSocketSignalingClient::Impl::OnConnected` | 定义 | `void QtWebSocketSignalingClient::Impl::OnConnected()` | 接收并处理 on connected 相关逻辑。 |
| [L32](../src/signaling/QtWebSocketSignalingClient.Recovery.cpp#L32) | `QtWebSocketSignalingClient::Impl::BeginDeviceRegistration` | 定义 | `void QtWebSocketSignalingClient::Impl::BeginDeviceRegistration()` | 启动 begin device registration 相关逻辑。 |
| [L67](../src/signaling/QtWebSocketSignalingClient.Recovery.cpp#L67) | `QtWebSocketSignalingClient::Impl::OnDisconnected` | 定义 | `void QtWebSocketSignalingClient::Impl::OnDisconnected()` | 接收并处理 on disconnected 相关逻辑。 |
| [L88](../src/signaling/QtWebSocketSignalingClient.Recovery.cpp#L88) | `QtWebSocketSignalingClient::Impl::OnTextMessage` | 定义 | `void QtWebSocketSignalingClient::Impl::OnTextMessage(const QString& message)` | 接收并处理 on text message 相关逻辑。 |

## `src/signaling/QtWebSocketSignalingClient.RoomCommands.cpp`

[打开源码](../src/signaling/QtWebSocketSignalingClient.RoomCommands.cpp) · **文件作用：** 实现 qt web socket signaling client room commands 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/signaling/QtWebSocketSignalingClient.RoomCommands.cpp#L9) | `QtWebSocketSignalingClient::Impl::CreateRoom` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::CreateRoom(std::uint32_t capacity)` | 创建或初始化 create room 相关逻辑。 |
| [L25](../src/signaling/QtWebSocketSignalingClient.RoomCommands.cpp#L25) | `QtWebSocketSignalingClient::Impl::RequestRoomJoin` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::RequestRoomJoin(const std::string& roomId)` | 发起请求或查询 request room join 相关逻辑。 |
| [L39](../src/signaling/QtWebSocketSignalingClient.RoomCommands.cpp#L39) | `QtWebSocketSignalingClient::Impl::QueryRoomAvailability` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::QueryRoomAvailability( const std::vector<std::string>& roomIds)` | 发起请求或查询 query room availability 相关逻辑。 |
| [L63](../src/signaling/QtWebSocketSignalingClient.RoomCommands.cpp#L63) | `QtWebSocketSignalingClient::Impl::RespondToRoomJoin` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::RespondToRoomJoin( const std::string& roomId, const std::string& requestId, bool accepted, const std::string& reasonCode)` | 处理并回复 respond to room join 相关逻辑。 |
| [L88](../src/signaling/QtWebSocketSignalingClient.RoomCommands.cpp#L88) | `QtWebSocketSignalingClient::Impl::SetRoomCapacity` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::SetRoomCapacity(const std::string& roomId, std::uint32_t capacity)` | 更新或应用 set room capacity 相关逻辑。 |
| [L107](../src/signaling/QtWebSocketSignalingClient.RoomCommands.cpp#L107) | `QtWebSocketSignalingClient::Impl::LeaveRoom` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::LeaveRoom(const std::string& roomId, const std::string& reasonCode)` | 实现 leave room 对应的业务或工具逻辑。 |
| [L126](../src/signaling/QtWebSocketSignalingClient.RoomCommands.cpp#L126) | `QtWebSocketSignalingClient::Impl::ResumeRoom` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::ResumeRoom(const std::string& roomId, const std::string& recoveryToken)` | 实现 resume room 对应的业务或工具逻辑。 |
| [L144](../src/signaling/QtWebSocketSignalingClient.RoomCommands.cpp#L144) | `QtWebSocketSignalingClient::Impl::SetRoomMediaState` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::SetRoomMediaState( const std::string& roomId, bool cameraPublishing, bool microphonePublishing)` | 更新或应用 set room media state 相关逻辑。 |
| [L165](../src/signaling/QtWebSocketSignalingClient.RoomCommands.cpp#L165) | `QtWebSocketSignalingClient::Impl::RequestRoomScreenShare` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::RequestRoomScreenShare( const std::string& roomId)` | 发起请求或查询 request room screen share 相关逻辑。 |
| [L181](../src/signaling/QtWebSocketSignalingClient.RoomCommands.cpp#L181) | `QtWebSocketSignalingClient::Impl::ConfirmRoomScreenShare` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::ConfirmRoomScreenShare( const std::string& roomId, const std::string& grantId)` | 实现 confirm room screen share 对应的业务或工具逻辑。 |
| [L200](../src/signaling/QtWebSocketSignalingClient.RoomCommands.cpp#L200) | `QtWebSocketSignalingClient::Impl::StopRoomScreenShare` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::StopRoomScreenShare( const std::string& roomId, const std::string& grantId, const std::string& reasonCode)` | 停止 stop room screen share 相关逻辑。 |
| [L224](../src/signaling/QtWebSocketSignalingClient.RoomCommands.cpp#L224) | `QtWebSocketSignalingClient::Impl::RespondToRoomScreenShareSwitch` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::RespondToRoomScreenShareSwitch( const std::string& roomId, const std::string& requestId, bool accepted, const std::string& reasonCode)` | 处理并回复 respond to room screen share switch 相关逻辑。 |
| [L251](../src/signaling/QtWebSocketSignalingClient.RoomCommands.cpp#L251) | `QtWebSocketSignalingClient::Impl::CancelRoomScreenShareSwitch` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::CancelRoomScreenShareSwitch( const std::string& roomId, const std::string& requestId, const std::string& reasonCode)` | 判断 cancel room screen share switch 相关逻辑。 |
| [L276](../src/signaling/QtWebSocketSignalingClient.RoomCommands.cpp#L276) | `QtWebSocketSignalingClient::Impl::RequestRoomControl` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::RequestRoomControl(const std::string& roomId)` | 发起请求或查询 request room control 相关逻辑。 |
| [L291](../src/signaling/QtWebSocketSignalingClient.RoomCommands.cpp#L291) | `QtWebSocketSignalingClient::Impl::RespondToRoomControl` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::RespondToRoomControl( const std::string& roomId, const std::string& requestId, bool accepted, const std::string& reasonCode)` | 处理并回复 respond to room control 相关逻辑。 |
| [L317](../src/signaling/QtWebSocketSignalingClient.RoomCommands.cpp#L317) | `QtWebSocketSignalingClient::Impl::ReleaseRoomControl` | 定义 | `SignalingOperationResult QtWebSocketSignalingClient::Impl::ReleaseRoomControl( const std::string& roomId, const std::string& grantId, const std::string& reasonCode)` | 释放或取消 release room control 相关逻辑。 |

## `src/signaling/SignalingJsonCodec.cpp`

[打开源码](../src/signaling/SignalingJsonCodec.cpp) · **文件作用：** 实现 signaling json codec 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L10](../src/signaling/SignalingJsonCodec.cpp#L10) | `ToQString` | 定义 | `QString ToQString(const std::string& value)` | 实现 to q string 对应的业务或工具逻辑。 |
| [L16](../src/signaling/SignalingJsonCodec.cpp#L16) | `ToString` | 定义 | `std::string ToString(const QString& value)` | 实现 to string 对应的业务或工具逻辑。 |
| [L22](../src/signaling/SignalingJsonCodec.cpp#L22) | `StringArray` | 定义 | `QJsonArray StringArray(const std::vector<std::string>& values)` | 实现 string array 对应的业务或工具逻辑。 |
| [L31](../src/signaling/SignalingJsonCodec.cpp#L31) | `ReadStringArray` | 定义 | `std::vector<std::string> ReadStringArray(const QJsonValue& value)` | 读取或恢复 read string array 相关逻辑。 |
| [L47](../src/signaling/SignalingJsonCodec.cpp#L47) | `ReadRequiredString` | 定义 | `bool ReadRequiredString(const QJsonObject& object, const char* field, std::string* value)` | 读取或恢复 read required string 相关逻辑。 |
| [L60](../src/signaling/SignalingJsonCodec.cpp#L60) | `ReadRequiredBool` | 定义 | `bool ReadRequiredBool(const QJsonObject& object, const char* field, bool* value)` | 读取或恢复 read required bool 相关逻辑。 |
| [L73](../src/signaling/SignalingJsonCodec.cpp#L73) | `ReadSafeUnsignedInteger` | 定义 | `bool ReadSafeUnsignedInteger(const QJsonObject& object, const char* field, std::uint64_t* value)` | 读取或恢复 read safe unsigned integer 相关逻辑。 |

## `src/signaling/SignalingJsonCodec.h`

[打开源码](../src/signaling/SignalingJsonCodec.h) · **文件作用：** 声明 signaling json codec 相关类型、接口、配置和成员状态。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L17](../src/signaling/SignalingJsonCodec.h#L17) | `ToQString` | 声明 | `QString ToQString(const std::string& value)` | 实现 to q string 对应的业务或工具逻辑。 |
| [L18](../src/signaling/SignalingJsonCodec.h#L18) | `ToString` | 声明 | `std::string ToString(const QString& value)` | 实现 to string 对应的业务或工具逻辑。 |
| [L19](../src/signaling/SignalingJsonCodec.h#L19) | `StringArray` | 声明 | `QJsonArray StringArray(const std::vector<std::string>& values)` | 实现 string array 对应的业务或工具逻辑。 |
| [L20](../src/signaling/SignalingJsonCodec.h#L20) | `ReadStringArray` | 声明 | `std::vector<std::string> ReadStringArray(const QJsonValue& value)` | 读取或恢复 read string array 相关逻辑。 |
| [L22](../src/signaling/SignalingJsonCodec.h#L22) | `ReadRequiredString` | 声明 | `bool ReadRequiredString(const QJsonObject& object, const char* field, std::string* value)` | 读取或恢复 read required string 相关逻辑。 |
| [L25](../src/signaling/SignalingJsonCodec.h#L25) | `ReadRequiredBool` | 声明 | `bool ReadRequiredBool(const QJsonObject& object, const char* field, bool* value)` | 读取或恢复 read required bool 相关逻辑。 |
| [L28](../src/signaling/SignalingJsonCodec.h#L28) | `ReadSafeUnsignedInteger` | 声明 | `bool ReadSafeUnsignedInteger(const QJsonObject& object, const char* field, std::uint64_t* value)` | 读取或恢复 read safe unsigned integer 相关逻辑。 |

## `src/signaling/SignalingObservers.h`

[打开源码](../src/signaling/SignalingObservers.h) · **文件作用：** 声明 signaling observers 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L10](../src/signaling/SignalingObservers.h#L10) | `ISignalingConnectionObserver` | class | 定义 ISignalingConnectionObserver 的 class 类型和相关状态。 |
| [L22](../src/signaling/SignalingObservers.h#L22) | `IDirectSignalingObserver` | class | 定义 IDirectSignalingObserver 的 class 类型和相关状态。 |
| [L54](../src/signaling/SignalingObservers.h#L54) | `IAccountSignalingObserver` | class | 定义 IAccountSignalingObserver 的 class 类型和相关状态。 |
| [L70](../src/signaling/SignalingObservers.h#L70) | `IRoomSignalingObserver` | class | 定义 IRoomSignalingObserver 的 class 类型和相关状态。 |
| [L155](../src/signaling/SignalingObservers.h#L155) | `ISignalingClientObserver` | class | 定义 ISignalingClientObserver 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L12](../src/signaling/SignalingObservers.h#L12) | `~ISignalingConnectionObserver` | 声明 | `virtual ~ISignalingConnectionObserver() = default` | 停止相关活动并释放 ISignalingConnectionObserver 实例拥有的资源。 |
| [L14](../src/signaling/SignalingObservers.h#L14) | `OnSignalingStateChanged` | 声明 | `virtual void OnSignalingStateChanged( SignalingConnectionState state) = 0` | 接收并处理 on signaling state changed 相关逻辑。 |
| [L16](../src/signaling/SignalingObservers.h#L16) | `OnDeviceRegistered` | 声明 | `virtual void OnDeviceRegistered(const std::string& deviceId) = 0` | 接收并处理 on device registered 相关逻辑。 |
| [L17](../src/signaling/SignalingObservers.h#L17) | `OnHeartbeatAcknowledged` | 声明 | `virtual void OnHeartbeatAcknowledged(std::uint32_t roundTripMs) = 0` | 接收并处理 on heartbeat acknowledged 相关逻辑。 |
| [L18](../src/signaling/SignalingObservers.h#L18) | `OnSignalingError` | 声明 | `virtual void OnSignalingError(const std::string& code, const std::string& message) = 0` | 接收并处理 on signaling error 相关逻辑。 |
| [L24](../src/signaling/SignalingObservers.h#L24) | `~IDirectSignalingObserver` | 声明 | `virtual ~IDirectSignalingObserver() = default` | 停止相关活动并释放 IDirectSignalingObserver 实例拥有的资源。 |
| [L26](../src/signaling/SignalingObservers.h#L26) | `OnIncomingSessionRequest` | 声明 | `virtual void OnIncomingSessionRequest( const IncomingSessionRequest& request) = 0` | 接收并处理 on incoming session request 相关逻辑。 |
| [L28](../src/signaling/SignalingObservers.h#L28) | `OnSessionResponse` | 声明 | `virtual void OnSessionResponse( const SignalingSessionResponse& response) = 0` | 接收并处理 on session response 相关逻辑。 |
| [L30](../src/signaling/SignalingObservers.h#L30) | `OnSessionPending` | 声明 | `virtual void OnSessionPending( const SignalingSessionPending& pending) = 0` | 接收并处理 on session pending 相关逻辑。 |
| [L32](../src/signaling/SignalingObservers.h#L32) | `OnSessionReady` | 声明 | `virtual void OnSessionReady(const SignalingSessionReady& ready) = 0` | 接收并处理 on session ready 相关逻辑。 |
| [L33](../src/signaling/SignalingObservers.h#L33) | `OnSessionSuspended` | 声明 | `virtual void OnSessionSuspended( const SignalingSessionSuspended& suspended) = 0` | 接收并处理 on session suspended 相关逻辑。 |
| [L35](../src/signaling/SignalingObservers.h#L35) | `OnSessionResumed` | 声明 | `virtual void OnSessionResumed( const SignalingSessionResumed& resumed) = 0` | 接收并处理 on session resumed 相关逻辑。 |
| [L37](../src/signaling/SignalingObservers.h#L37) | `OnSessionEnded` | 声明 | `virtual void OnSessionEnded(const SignalingSessionEnded& ended) = 0` | 接收并处理 on session ended 相关逻辑。 |
| [L38](../src/signaling/SignalingObservers.h#L38) | `OnRemoteDescription` | 声明 | `virtual void OnRemoteDescription( const SignalingSessionDescription& description) = 0` | 接收并处理 on remote description 相关逻辑。 |
| [L40](../src/signaling/SignalingObservers.h#L40) | `OnRemoteIceCandidate` | 声明 | `virtual void OnRemoteIceCandidate( const SignalingIceCandidate& candidate) = 0` | 接收并处理 on remote ice candidate 相关逻辑。 |
| [L42](../src/signaling/SignalingObservers.h#L42) | `OnIceRestartRequested` | 定义 | `virtual void OnIceRestartRequested( const SignalingIceRestartRequest& request)` | 接收并处理 on ice restart requested 相关逻辑。 |
| [L47](../src/signaling/SignalingObservers.h#L47) | `OnIceRestartCancelled` | 定义 | `virtual void OnIceRestartCancelled( const SignalingIceRestartCancel& cancel)` | 接收并处理 on ice restart cancelled 相关逻辑。 |
| [L56](../src/signaling/SignalingObservers.h#L56) | `~IAccountSignalingObserver` | 声明 | `virtual ~IAccountSignalingObserver() = default` | 停止相关活动并释放 IAccountSignalingObserver 实例拥有的资源。 |
| [L58](../src/signaling/SignalingObservers.h#L58) | `OnAccountDeletionResult` | 定义 | `virtual void OnAccountDeletionResult( const SignalingAccountDeletionResult& result)` | 接收并处理 on account deletion result 相关逻辑。 |
| [L63](../src/signaling/SignalingObservers.h#L63) | `OnOwnedDevicesChanged` | 定义 | `virtual void OnOwnedDevicesChanged( const SignalingOwnedDevicesSnapshot& snapshot)` | 接收并处理 on owned devices changed 相关逻辑。 |
| [L72](../src/signaling/SignalingObservers.h#L72) | `~IRoomSignalingObserver` | 声明 | `virtual ~IRoomSignalingObserver() = default` | 停止相关活动并释放 IRoomSignalingObserver 实例拥有的资源。 |
| [L75](../src/signaling/SignalingObservers.h#L75) | `OnRoomReady` | 定义 | `virtual void OnRoomReady(const SignalingRoomReady& ready)` | Default no-op implementations keep direct-session-only observers small. |
| [L79](../src/signaling/SignalingObservers.h#L79) | `OnRoomState` | 定义 | `virtual void OnRoomState(const RoomSnapshot& room)` | 接收并处理 on room state 相关逻辑。 |
| [L83](../src/signaling/SignalingObservers.h#L83) | `OnRoomJoinPending` | 定义 | `virtual void OnRoomJoinPending( const SignalingRoomJoinPending& pending)` | 接收并处理 on room join pending 相关逻辑。 |
| [L88](../src/signaling/SignalingObservers.h#L88) | `OnRoomJoinRequested` | 定义 | `virtual void OnRoomJoinRequested(const RoomJoinRequest& request)` | 接收并处理 on room join requested 相关逻辑。 |
| [L92](../src/signaling/SignalingObservers.h#L92) | `OnRoomJoinResult` | 定义 | `virtual void OnRoomJoinResult( const SignalingRoomJoinResult& result)` | 接收并处理 on room join result 相关逻辑。 |
| [L97](../src/signaling/SignalingObservers.h#L97) | `OnRoomAvailabilityResult` | 定义 | `virtual void OnRoomAvailabilityResult( const SignalingRoomAvailabilityResult& result)` | 接收并处理 on room availability result 相关逻辑。 |
| [L102](../src/signaling/SignalingObservers.h#L102) | `OnRoomClosed` | 定义 | `virtual void OnRoomClosed(const SignalingRoomClosed& closed)` | 接收并处理 on room closed 相关逻辑。 |
| [L106](../src/signaling/SignalingObservers.h#L106) | `OnRoomPairReady` | 定义 | `virtual void OnRoomPairReady(const SignalingRoomPairReady& ready)` | 接收并处理 on room pair ready 相关逻辑。 |
| [L110](../src/signaling/SignalingObservers.h#L110) | `OnRoomPairClosed` | 定义 | `virtual void OnRoomPairClosed(const SignalingRoomPairClosed& closed)` | 接收并处理 on room pair closed 相关逻辑。 |
| [L114](../src/signaling/SignalingObservers.h#L114) | `OnRoomScreenShareGranted` | 定义 | `virtual void OnRoomScreenShareGranted( const SignalingRoomScreenShareGranted& granted)` | 接收并处理 on room screen share granted 相关逻辑。 |
| [L119](../src/signaling/SignalingObservers.h#L119) | `OnRoomScreenShareSwitchPending` | 定义 | `virtual void OnRoomScreenShareSwitchPending( const SignalingRoomScreenShareSwitchPending& pending)` | 接收并处理 on room screen share switch pending 相关逻辑。 |
| [L124](../src/signaling/SignalingObservers.h#L124) | `OnRoomScreenShareSwitchRequested` | 定义 | `virtual void OnRoomScreenShareSwitchRequested( const RoomScreenShareSwitchRequest& request)` | 接收并处理 on room screen share switch requested 相关逻辑。 |
| [L129](../src/signaling/SignalingObservers.h#L129) | `OnRoomScreenShareSwitchResult` | 定义 | `virtual void OnRoomScreenShareSwitchResult( const SignalingRoomScreenShareSwitchResult& result)` | 接收并处理 on room screen share switch result 相关逻辑。 |
| [L134](../src/signaling/SignalingObservers.h#L134) | `OnRoomControlRequested` | 定义 | `virtual void OnRoomControlRequested(const RoomControlRequest& request)` | 接收并处理 on room control requested 相关逻辑。 |
| [L138](../src/signaling/SignalingObservers.h#L138) | `OnRoomControlResult` | 定义 | `virtual void OnRoomControlResult( const SignalingRoomControlResult& result)` | 接收并处理 on room control result 相关逻辑。 |
| [L143](../src/signaling/SignalingObservers.h#L143) | `OnRoomControlGranted` | 定义 | `virtual void OnRoomControlGranted( const SignalingRoomControlGranted& granted)` | 接收并处理 on room control granted 相关逻辑。 |
| [L148](../src/signaling/SignalingObservers.h#L148) | `OnRoomControlRevoked` | 定义 | `virtual void OnRoomControlRevoked( const SignalingRoomControlRevoked& revoked)` | 接收并处理 on room control revoked 相关逻辑。 |
| [L160](../src/signaling/SignalingObservers.h#L160) | `~ISignalingClientObserver` | 声明 | `~ISignalingClientObserver() override = default` | 停止相关活动并释放 ISignalingClientObserver 实例拥有的资源。 |

## `src/signaling/SignalingTypes.h`

[打开源码](../src/signaling/SignalingTypes.h) · **文件作用：** 声明 signaling types 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L15](../src/signaling/SignalingTypes.h#L15) | `SignalingConnectionState` | enum class | 定义 SignalingConnectionState 的 enum class 类型和相关状态。 |
| [L25](../src/signaling/SignalingTypes.h#L25) | `SignalingAuthenticationMode` | enum class | 定义 SignalingAuthenticationMode 的 enum class 类型和相关状态。 |
| [L35](../src/signaling/SignalingTypes.h#L35) | `SignalingCapabilities` | struct | 定义 SignalingCapabilities 的 struct 类型和相关状态。 |
| [L45](../src/signaling/SignalingTypes.h#L45) | `SignalingClientConfig` | struct | 定义 SignalingClientConfig 的 struct 类型和相关状态。 |
| [L69](../src/signaling/SignalingTypes.h#L69) | `SignalingOperationResult` | struct | 定义 SignalingOperationResult 的 struct 类型和相关状态。 |
| [L75](../src/signaling/SignalingTypes.h#L75) | `SignalingAccountDeletionResult` | struct | 定义 SignalingAccountDeletionResult 的 struct 类型和相关状态。 |
| [L82](../src/signaling/SignalingTypes.h#L82) | `IncomingSessionRequest` | struct | 定义 IncomingSessionRequest 的 struct 类型和相关状态。 |
| [L94](../src/signaling/SignalingTypes.h#L94) | `SignalingOwnedDevice` | struct | 定义 SignalingOwnedDevice 的 struct 类型和相关状态。 |
| [L103](../src/signaling/SignalingTypes.h#L103) | `SignalingOwnedDevicesSnapshot` | struct | 定义 SignalingOwnedDevicesSnapshot 的 struct 类型和相关状态。 |
| [L108](../src/signaling/SignalingTypes.h#L108) | `SignalingSessionResponse` | struct | 定义 SignalingSessionResponse 的 struct 类型和相关状态。 |
| [L115](../src/signaling/SignalingTypes.h#L115) | `SignalingSessionPending` | struct | 定义 SignalingSessionPending 的 struct 类型和相关状态。 |
| [L120](../src/signaling/SignalingTypes.h#L120) | `SignalingSessionEndKind` | enum class | 定义 SignalingSessionEndKind 的 enum class 类型和相关状态。 |
| [L125](../src/signaling/SignalingTypes.h#L125) | `SignalingSessionEnded` | struct | 定义 SignalingSessionEnded 的 struct 类型和相关状态。 |
| [L132](../src/signaling/SignalingTypes.h#L132) | `SignalingIceServer` | struct | 定义 SignalingIceServer 的 struct 类型和相关状态。 |
| [L138](../src/signaling/SignalingTypes.h#L138) | `SignalingSessionReady` | struct | 定义 SignalingSessionReady 的 struct 类型和相关状态。 |
| [L145](../src/signaling/SignalingTypes.h#L145) | `SignalingSessionSuspended` | struct | 定义 SignalingSessionSuspended 的 struct 类型和相关状态。 |
| [L151](../src/signaling/SignalingTypes.h#L151) | `SignalingSessionResumed` | struct | 定义 SignalingSessionResumed 的 struct 类型和相关状态。 |
| [L157](../src/signaling/SignalingTypes.h#L157) | `SignalingSessionDescription` | struct | 定义 SignalingSessionDescription 的 struct 类型和相关状态。 |
| [L164](../src/signaling/SignalingTypes.h#L164) | `SignalingIceCandidate` | struct | 定义 SignalingIceCandidate 的 struct 类型和相关状态。 |
| [L172](../src/signaling/SignalingTypes.h#L172) | `SignalingIceRestartRequest` | struct | 定义 SignalingIceRestartRequest 的 struct 类型和相关状态。 |
| [L178](../src/signaling/SignalingTypes.h#L178) | `SignalingIceRestartCancel` | struct | 定义 SignalingIceRestartCancel 的 struct 类型和相关状态。 |
| [L184](../src/signaling/SignalingTypes.h#L184) | `SignalingRoomReady` | struct | 定义 SignalingRoomReady 的 struct 类型和相关状态。 |
| [L189](../src/signaling/SignalingTypes.h#L189) | `SignalingRoomJoinPending` | struct | 定义 SignalingRoomJoinPending 的 struct 类型和相关状态。 |
| [L194](../src/signaling/SignalingTypes.h#L194) | `SignalingRoomJoinResult` | struct | 定义 SignalingRoomJoinResult 的 struct 类型和相关状态。 |
| [L202](../src/signaling/SignalingTypes.h#L202) | `SignalingRoomAvailability` | struct | 定义 SignalingRoomAvailability 的 struct 类型和相关状态。 |
| [L208](../src/signaling/SignalingTypes.h#L208) | `SignalingRoomAvailabilityResult` | struct | 定义 SignalingRoomAvailabilityResult 的 struct 类型和相关状态。 |
| [L212](../src/signaling/SignalingTypes.h#L212) | `SignalingRoomClosed` | struct | 定义 SignalingRoomClosed 的 struct 类型和相关状态。 |
| [L218](../src/signaling/SignalingTypes.h#L218) | `SignalingRoomPairReady` | struct | 定义 SignalingRoomPairReady 的 struct 类型和相关状态。 |
| [L226](../src/signaling/SignalingTypes.h#L226) | `SignalingRoomPairClosed` | struct | 定义 SignalingRoomPairClosed 的 struct 类型和相关状态。 |
| [L234](../src/signaling/SignalingTypes.h#L234) | `SignalingRoomScreenShareGranted` | struct | 定义 SignalingRoomScreenShareGranted 的 struct 类型和相关状态。 |
| [L240](../src/signaling/SignalingTypes.h#L240) | `SignalingRoomScreenShareSwitchPending` | struct | 定义 SignalingRoomScreenShareSwitchPending 的 struct 类型和相关状态。 |
| [L246](../src/signaling/SignalingTypes.h#L246) | `SignalingRoomScreenShareSwitchResult` | struct | 定义 SignalingRoomScreenShareSwitchResult 的 struct 类型和相关状态。 |
| [L254](../src/signaling/SignalingTypes.h#L254) | `SignalingRoomControlResult` | struct | 定义 SignalingRoomControlResult 的 struct 类型和相关状态。 |
| [L262](../src/signaling/SignalingTypes.h#L262) | `SignalingRoomControlGranted` | struct | 定义 SignalingRoomControlGranted 的 struct 类型和相关状态。 |
| [L269](../src/signaling/SignalingTypes.h#L269) | `SignalingRoomControlRevoked` | struct | 定义 SignalingRoomControlRevoked 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L36](../src/signaling/SignalingTypes.h#L36) | `screenCapture` | `bool screenCapture = true;` | 保存 screen capture 相关配置或运行状态。 |
| [L37](../src/signaling/SignalingTypes.h#L37) | `cameraCapture` | `bool cameraCapture = true;` | 保存 camera capture 相关配置或运行状态。 |
| [L38](../src/signaling/SignalingTypes.h#L38) | `inputControl` | `bool inputControl = true;` | 保存 input control 相关配置或运行状态。 |
| [L39](../src/signaling/SignalingTypes.h#L39) | `h264Encode` | `bool h264Encode = false;` | 保存 h264 encode 相关配置或运行状态。 |
| [L40](../src/signaling/SignalingTypes.h#L40) | `h264Decode` | `bool h264Decode = false;` | 保存 h264 decode 相关配置或运行状态。 |
| [L41](../src/signaling/SignalingTypes.h#L41) | `d3d11NativeDecode` | `bool d3d11NativeDecode = false;` | 保存 d3d11 native decode 相关配置或运行状态。 |
| [L42](../src/signaling/SignalingTypes.h#L42) | `protocolVersion` | `std::uint32_t protocolVersion = 5;` | 保存 protocol version 相关配置或运行状态。 |
| [L46](../src/signaling/SignalingTypes.h#L46) | `endpoint` | `std::string endpoint;` | 保存 endpoint 相关配置或运行状态。 |
| [L47](../src/signaling/SignalingTypes.h#L47) | `accessToken` | `std::string accessToken;` | 保存 access token 相关配置或运行状态。 |
| [L49](../src/signaling/SignalingTypes.h#L49) | `kLegacyUpgradeBearer` | `SignalingAuthenticationMode::kLegacyUpgradeBearer;` | 定义 legacy upgrade bearer 的编译期常量或产品边界。 |
| [L50](../src/signaling/SignalingTypes.h#L50) | `deviceId` | `std::string deviceId;` | 保存身份或作用域标识：device id。 |
| [L51](../src/signaling/SignalingTypes.h#L51) | `deviceName` | `std::string deviceName;` | 保存路径、地址或显示名称：device name。 |
| [L54](../src/signaling/SignalingTypes.h#L54) | `deviceVerificationCode` | `std::string deviceVerificationCode;` | Six-digit code generated locally for this process. It is never stored by the signaling server and is only relayed over authenticated WSS. |
| [L55](../src/signaling/SignalingTypes.h#L55) | `appVersion` | `std::string appVersion;` | 保存 app version 相关配置或运行状态。 |
| [L58](../src/signaling/SignalingTypes.h#L58) | `trustedCaPem` | `std::string trustedCaPem;` | Optional PEM CA bundle for local/private deployments. Supplying it adds a trust anchor; certificate and hostname verification remain enabled. |
| [L59](../src/signaling/SignalingTypes.h#L59) | `heartbeatIntervalMs` | `std::uint32_t heartbeatIntervalMs = 15000;` | 保存 heartbeat interval ms 相关配置或运行状态。 |
| [L60](../src/signaling/SignalingTypes.h#L60) | `heartbeatTimeoutMs` | `std::uint32_t heartbeatTimeoutMs = 10000;` | 保存 heartbeat timeout ms 相关配置或运行状态。 |
| [L61](../src/signaling/SignalingTypes.h#L61) | `authenticationTimeoutMs` | `std::uint32_t authenticationTimeoutMs = 10000;` | 保存 authentication timeout ms 相关配置或运行状态。 |
| [L62](../src/signaling/SignalingTypes.h#L62) | `reconnectInitialDelayMs` | `std::uint32_t reconnectInitialDelayMs = 1000;` | 保存 reconnect initial delay ms 相关配置或运行状态。 |
| [L63](../src/signaling/SignalingTypes.h#L63) | `reconnectMaximumDelayMs` | `std::uint32_t reconnectMaximumDelayMs = 10000;` | 保存 reconnect maximum delay ms 相关配置或运行状态。 |
| [L65](../src/signaling/SignalingTypes.h#L65) | `reconnectAttemptLimit` | `std::uint32_t reconnectAttemptLimit = 8;` | Zero disables automatic reconnect. |
| [L66](../src/signaling/SignalingTypes.h#L66) | `capabilities` | `SignalingCapabilities capabilities;` | 保存 capabilities 相关配置或运行状态。 |
| [L70](../src/signaling/SignalingTypes.h#L70) | `accepted` | `bool accepted = false;` | 保存 accepted 相关配置或运行状态。 |
| [L71](../src/signaling/SignalingTypes.h#L71) | `errorCode` | `std::string errorCode;` | 保存 error code 相关配置或运行状态。 |
| [L72](../src/signaling/SignalingTypes.h#L72) | `errorMessage` | `std::string errorMessage;` | 保存 error message 相关配置或运行状态。 |
| [L76](../src/signaling/SignalingTypes.h#L76) | `deleted` | `bool deleted = false;` | 保存 deleted 相关配置或运行状态。 |
| [L77](../src/signaling/SignalingTypes.h#L77) | `errorCode` | `std::string errorCode;` | 保存 error code 相关配置或运行状态。 |
| [L78](../src/signaling/SignalingTypes.h#L78) | `errorMessage` | `std::string errorMessage;` | 保存 error message 相关配置或运行状态。 |
| [L79](../src/signaling/SignalingTypes.h#L79) | `retryable` | `bool retryable = false;` | 保存 retryable 相关配置或运行状态。 |
| [L83](../src/signaling/SignalingTypes.h#L83) | `sessionId` | `std::string sessionId;` | 保存身份或作用域标识：session id。 |
| [L84](../src/signaling/SignalingTypes.h#L84) | `requesterDeviceId` | `std::string requesterDeviceId;` | 保存身份或作用域标识：requester device id。 |
| [L85](../src/signaling/SignalingTypes.h#L85) | `purpose` | `SessionPurpose purpose = SessionPurpose::kNone;` | 保存 purpose 相关配置或运行状态。 |
| [L86](../src/signaling/SignalingTypes.h#L86) | `requestedPermissions` | `std::vector<std::string> requestedPermissions;` | 保存 requested permissions 相关配置或运行状态。 |
| [L87](../src/signaling/SignalingTypes.h#L87) | `sameAccount` | `bool sameAccount = false;` | 保存计数、尺寸或速率指标：same account。 |
| [L88](../src/signaling/SignalingTypes.h#L88) | `autoAccept` | `bool autoAccept = false;` | 保存 auto accept 相关配置或运行状态。 |
| [L90](../src/signaling/SignalingTypes.h#L90) | `kManualApproval` | `DirectAuthorizationMethod::kManualApproval;` | 定义 manual approval 的编译期常量或产品边界。 |
| [L91](../src/signaling/SignalingTypes.h#L91) | `verificationCode` | `std::string verificationCode;` | 保存 verification code 相关配置或运行状态。 |
| [L95](../src/signaling/SignalingTypes.h#L95) | `deviceId` | `std::string deviceId;` | 保存身份或作用域标识：device id。 |
| [L96](../src/signaling/SignalingTypes.h#L96) | `deviceName` | `std::string deviceName;` | 保存路径、地址或显示名称：device name。 |
| [L97](../src/signaling/SignalingTypes.h#L97) | `online` | `bool online = false;` | 保存 online 相关配置或运行状态。 |
| [L98](../src/signaling/SignalingTypes.h#L98) | `current` | `bool current = false;` | 保存 current 相关配置或运行状态。 |
| [L99](../src/signaling/SignalingTypes.h#L99) | `createdAt` | `std::int64_t createdAt = 0;` | 保存 created at 相关配置或运行状态。 |
| [L100](../src/signaling/SignalingTypes.h#L100) | `lastSeenAt` | `std::int64_t lastSeenAt = 0;` | 保存 last seen at 相关配置或运行状态。 |
| [L104](../src/signaling/SignalingTypes.h#L104) | `revision` | `std::uint64_t revision = 0;` | 标记当前世代，用于拒绝过期异步结果：revision。 |
| [L105](../src/signaling/SignalingTypes.h#L105) | `devices` | `std::vector<SignalingOwnedDevice> devices;` | 保存 devices 相关配置或运行状态。 |
| [L109](../src/signaling/SignalingTypes.h#L109) | `sessionId` | `std::string sessionId;` | 保存身份或作用域标识：session id。 |
| [L110](../src/signaling/SignalingTypes.h#L110) | `accepted` | `bool accepted = false;` | 保存 accepted 相关配置或运行状态。 |
| [L111](../src/signaling/SignalingTypes.h#L111) | `reasonCode` | `std::string reasonCode;` | 保存 reason code 相关配置或运行状态。 |
| [L112](../src/signaling/SignalingTypes.h#L112) | `reasonMessage` | `std::string reasonMessage;` | 保存 reason message 相关配置或运行状态。 |
| [L116](../src/signaling/SignalingTypes.h#L116) | `sessionId` | `std::string sessionId;` | 保存身份或作用域标识：session id。 |
| [L117](../src/signaling/SignalingTypes.h#L117) | `peerDeviceId` | `std::string peerDeviceId;` | 保存身份或作用域标识：peer device id。 |
| [L126](../src/signaling/SignalingTypes.h#L126) | `sessionId` | `std::string sessionId;` | 保存身份或作用域标识：session id。 |
| [L127](../src/signaling/SignalingTypes.h#L127) | `initiatorDeviceId` | `std::string initiatorDeviceId;` | 保存身份或作用域标识：initiator device id。 |
| [L128](../src/signaling/SignalingTypes.h#L128) | `reasonCode` | `std::string reasonCode;` | 保存 reason code 相关配置或运行状态。 |
| [L129](../src/signaling/SignalingTypes.h#L129) | `kind` | `SignalingSessionEndKind kind = SignalingSessionEndKind::kClosed;` | 保存 kind 相关配置或运行状态。 |
| [L133](../src/signaling/SignalingTypes.h#L133) | `urls` | `std::vector<std::string> urls;` | 保存 urls 相关配置或运行状态。 |
| [L134](../src/signaling/SignalingTypes.h#L134) | `username` | `std::string username;` | 保存路径、地址或显示名称：username。 |
| [L135](../src/signaling/SignalingTypes.h#L135) | `credential` | `std::string credential;` | 保存 credential 相关配置或运行状态。 |
| [L139](../src/signaling/SignalingTypes.h#L139) | `sessionId` | `std::string sessionId;` | 保存身份或作用域标识：session id。 |
| [L140](../src/signaling/SignalingTypes.h#L140) | `peerDeviceId` | `std::string peerDeviceId;` | 保存身份或作用域标识：peer device id。 |
| [L141](../src/signaling/SignalingTypes.h#L141) | `recoveryToken` | `std::string recoveryToken;` | 保存 recovery token 相关配置或运行状态。 |
| [L142](../src/signaling/SignalingTypes.h#L142) | `iceServers` | `std::vector<SignalingIceServer> iceServers;` | 保存 ice servers 相关配置或运行状态。 |
| [L146](../src/signaling/SignalingTypes.h#L146) | `sessionId` | `std::string sessionId;` | 保存身份或作用域标识：session id。 |
| [L147](../src/signaling/SignalingTypes.h#L147) | `peerDeviceId` | `std::string peerDeviceId;` | 保存身份或作用域标识：peer device id。 |
| [L148](../src/signaling/SignalingTypes.h#L148) | `recoveryWindowMs` | `std::uint32_t recoveryWindowMs = 0;` | 保存 recovery window ms 相关配置或运行状态。 |
| [L152](../src/signaling/SignalingTypes.h#L152) | `sessionId` | `std::string sessionId;` | 保存身份或作用域标识：session id。 |
| [L153](../src/signaling/SignalingTypes.h#L153) | `peerDeviceId` | `std::string peerDeviceId;` | 保存身份或作用域标识：peer device id。 |
| [L154](../src/signaling/SignalingTypes.h#L154) | `resumedDeviceId` | `std::string resumedDeviceId;` | 保存身份或作用域标识：resumed device id。 |
| [L158](../src/signaling/SignalingTypes.h#L158) | `sessionId` | `std::string sessionId;` | 保存身份或作用域标识：session id。 |
| [L159](../src/signaling/SignalingTypes.h#L159) | `type` | `std::string type;` | 保存 type 相关配置或运行状态。 |
| [L160](../src/signaling/SignalingTypes.h#L160) | `sdp` | `std::string sdp;` | 保存 sdp 相关配置或运行状态。 |
| [L161](../src/signaling/SignalingTypes.h#L161) | `negotiationGeneration` | `std::uint64_t negotiationGeneration = 1;` | 标记当前世代，用于拒绝过期异步结果：negotiation generation。 |
| [L165](../src/signaling/SignalingTypes.h#L165) | `sessionId` | `std::string sessionId;` | 保存身份或作用域标识：session id。 |
| [L166](../src/signaling/SignalingTypes.h#L166) | `candidate` | `std::string candidate;` | 保存 candidate 相关配置或运行状态。 |
| [L167](../src/signaling/SignalingTypes.h#L167) | `sdpMid` | `std::string sdpMid;` | 保存身份或作用域标识：sdp mid。 |
| [L168](../src/signaling/SignalingTypes.h#L168) | `sdpMLineIndex` | `std::int32_t sdpMLineIndex = -1;` | 保存 sdp m line index 相关配置或运行状态。 |
| [L169](../src/signaling/SignalingTypes.h#L169) | `negotiationGeneration` | `std::uint64_t negotiationGeneration = 1;` | 标记当前世代，用于拒绝过期异步结果：negotiation generation。 |
| [L173](../src/signaling/SignalingTypes.h#L173) | `sessionId` | `std::string sessionId;` | 保存身份或作用域标识：session id。 |
| [L174](../src/signaling/SignalingTypes.h#L174) | `observedGeneration` | `std::uint64_t observedGeneration = 1;` | 标记当前世代，用于拒绝过期异步结果：observed generation。 |
| [L175](../src/signaling/SignalingTypes.h#L175) | `requestSequence` | `std::uint64_t requestSequence = 0;` | 保存单调序号，用于排序或去重：request sequence。 |
| [L179](../src/signaling/SignalingTypes.h#L179) | `sessionId` | `std::string sessionId;` | 保存身份或作用域标识：session id。 |
| [L180](../src/signaling/SignalingTypes.h#L180) | `observedGeneration` | `std::uint64_t observedGeneration = 1;` | 标记当前世代，用于拒绝过期异步结果：observed generation。 |
| [L181](../src/signaling/SignalingTypes.h#L181) | `requestSequence` | `std::uint64_t requestSequence = 0;` | 保存单调序号，用于排序或去重：request sequence。 |
| [L185](../src/signaling/SignalingTypes.h#L185) | `room` | `RoomSnapshot room;` | 保存 room 相关配置或运行状态。 |
| [L186](../src/signaling/SignalingTypes.h#L186) | `recoveryToken` | `std::string recoveryToken;` | 保存 recovery token 相关配置或运行状态。 |
| [L190](../src/signaling/SignalingTypes.h#L190) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L191](../src/signaling/SignalingTypes.h#L191) | `requestId` | `std::string requestId;` | 保存身份或作用域标识：request id。 |
| [L195](../src/signaling/SignalingTypes.h#L195) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L196](../src/signaling/SignalingTypes.h#L196) | `requestId` | `std::string requestId;` | 保存身份或作用域标识：request id。 |
| [L197](../src/signaling/SignalingTypes.h#L197) | `accepted` | `bool accepted = false;` | 保存 accepted 相关配置或运行状态。 |
| [L198](../src/signaling/SignalingTypes.h#L198) | `reasonCode` | `std::string reasonCode;` | 保存 reason code 相关配置或运行状态。 |
| [L199](../src/signaling/SignalingTypes.h#L199) | `reasonMessage` | `std::string reasonMessage;` | 保存 reason message 相关配置或运行状态。 |
| [L203](../src/signaling/SignalingTypes.h#L203) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L204](../src/signaling/SignalingTypes.h#L204) | `exists` | `bool exists = false;` | 保存 exists 相关配置或运行状态。 |
| [L205](../src/signaling/SignalingTypes.h#L205) | `joinable` | `bool joinable = false;` | 保存 joinable 相关配置或运行状态。 |
| [L209](../src/signaling/SignalingTypes.h#L209) | `rooms` | `std::vector<SignalingRoomAvailability> rooms;` | 保存 rooms 相关配置或运行状态。 |
| [L213](../src/signaling/SignalingTypes.h#L213) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L214](../src/signaling/SignalingTypes.h#L214) | `initiatorDeviceId` | `std::string initiatorDeviceId;` | 保存身份或作用域标识：initiator device id。 |
| [L215](../src/signaling/SignalingTypes.h#L215) | `reasonCode` | `std::string reasonCode;` | 保存 reason code 相关配置或运行状态。 |
| [L219](../src/signaling/SignalingTypes.h#L219) | `pairId` | `std::string pairId;` | 保存身份或作用域标识：pair id。 |
| [L220](../src/signaling/SignalingTypes.h#L220) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L221](../src/signaling/SignalingTypes.h#L221) | `peerDeviceId` | `std::string peerDeviceId;` | 保存身份或作用域标识：peer device id。 |
| [L222](../src/signaling/SignalingTypes.h#L222) | `localIsOfferer` | `bool localIsOfferer = false;` | 保存 local is offerer 相关配置或运行状态。 |
| [L223](../src/signaling/SignalingTypes.h#L223) | `iceServers` | `std::vector<SignalingIceServer> iceServers;` | 保存 ice servers 相关配置或运行状态。 |
| [L227](../src/signaling/SignalingTypes.h#L227) | `pairId` | `std::string pairId;` | 保存身份或作用域标识：pair id。 |
| [L228](../src/signaling/SignalingTypes.h#L228) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L229](../src/signaling/SignalingTypes.h#L229) | `peerDeviceId` | `std::string peerDeviceId;` | 保存身份或作用域标识：peer device id。 |
| [L230](../src/signaling/SignalingTypes.h#L230) | `initiatorDeviceId` | `std::string initiatorDeviceId;` | 保存身份或作用域标识：initiator device id。 |
| [L231](../src/signaling/SignalingTypes.h#L231) | `reasonCode` | `std::string reasonCode;` | 保存 reason code 相关配置或运行状态。 |
| [L235](../src/signaling/SignalingTypes.h#L235) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L236](../src/signaling/SignalingTypes.h#L236) | `grantId` | `std::string grantId;` | 保存身份或作用域标识：grant id。 |
| [L237](../src/signaling/SignalingTypes.h#L237) | `epoch` | `std::uint64_t epoch = 0;` | 标记当前世代，用于拒绝过期异步结果：epoch。 |
| [L241](../src/signaling/SignalingTypes.h#L241) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L242](../src/signaling/SignalingTypes.h#L242) | `requestId` | `std::string requestId;` | 保存身份或作用域标识：request id。 |
| [L243](../src/signaling/SignalingTypes.h#L243) | `screenSharerDeviceId` | `std::string screenSharerDeviceId;` | 保存身份或作用域标识：screen sharer device id。 |
| [L247](../src/signaling/SignalingTypes.h#L247) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L248](../src/signaling/SignalingTypes.h#L248) | `requestId` | `std::string requestId;` | 保存身份或作用域标识：request id。 |
| [L249](../src/signaling/SignalingTypes.h#L249) | `accepted` | `bool accepted = false;` | 保存 accepted 相关配置或运行状态。 |
| [L250](../src/signaling/SignalingTypes.h#L250) | `reasonCode` | `std::string reasonCode;` | 保存 reason code 相关配置或运行状态。 |
| [L251](../src/signaling/SignalingTypes.h#L251) | `reasonMessage` | `std::string reasonMessage;` | 保存 reason message 相关配置或运行状态。 |
| [L255](../src/signaling/SignalingTypes.h#L255) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L256](../src/signaling/SignalingTypes.h#L256) | `requestId` | `std::string requestId;` | 保存身份或作用域标识：request id。 |
| [L257](../src/signaling/SignalingTypes.h#L257) | `accepted` | `bool accepted = false;` | 保存 accepted 相关配置或运行状态。 |
| [L258](../src/signaling/SignalingTypes.h#L258) | `reasonCode` | `std::string reasonCode;` | 保存 reason code 相关配置或运行状态。 |
| [L259](../src/signaling/SignalingTypes.h#L259) | `reasonMessage` | `std::string reasonMessage;` | 保存 reason message 相关配置或运行状态。 |
| [L263](../src/signaling/SignalingTypes.h#L263) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L264](../src/signaling/SignalingTypes.h#L264) | `grantId` | `std::string grantId;` | 保存身份或作用域标识：grant id。 |
| [L265](../src/signaling/SignalingTypes.h#L265) | `screenSharerDeviceId` | `std::string screenSharerDeviceId;` | 保存身份或作用域标识：screen sharer device id。 |
| [L266](../src/signaling/SignalingTypes.h#L266) | `controllerDeviceId` | `std::string controllerDeviceId;` | 保存身份或作用域标识：controller device id。 |
| [L270](../src/signaling/SignalingTypes.h#L270) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L271](../src/signaling/SignalingTypes.h#L271) | `initiatorDeviceId` | `std::string initiatorDeviceId;` | 保存身份或作用域标识：initiator device id。 |
| [L272](../src/signaling/SignalingTypes.h#L272) | `reasonCode` | `std::string reasonCode;` | 保存 reason code 相关配置或运行状态。 |
