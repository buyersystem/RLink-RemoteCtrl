# 内容策略核心最小示例

此示例只链接 `RLinkMediaIntelligence::core`，不使用 RLink 会话、Qt、WebRTC、Windows ML 或 GPU 接口。默认估算模型未标定，输出仅作为建议，`automaticControlEligible` 必须为 false。

仓库内构建运行：

```powershell
cmake --build build --config Release --target ContentAwarePolicyExample
.\x64\Release\ContentAwarePolicyExample.exe
```

安装组件后，其他 C++20 工程可使用：

```cmake
find_package(RLinkMediaIntelligence CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE RLinkMediaIntelligence::core)
```

宿主提供源尺寸、当前规格、用户边界、细场景、真实采集活动和同一连接的视频安全预算。`nowMs` 与 `networkTimestampMs` 必须使用同一单调时钟；`generation` 用于隔离会话统计。缺少容量时设置 `networkBudgetAvailable=false`，不能把实际已发送码率当作容量。

`networkConstrained` 默认为 false：只有宿主从新鲜窗口确认真实网络压力，才可置 true 并允许按场景降低尺寸/FPS。参考需求超过预算或静止转运动不构成该证据。没有压力时保留当前规格；恢复更高规格仍需质量、处理能力、预算余量和连续窗口确认。保持现有发送上限不等于要求发送这些流量，实际分配仍由拥塞控制负责。

尺寸和 FPS 候选使用固定容量配置数组，评价最多遍历 20×14 个候选；策略本身不分配堆内存，不创建线程，不处理帧。自定义 `qualityEstimator` 回调需自行保证无阻塞、无额外分配和有限计算开销，并基于实际编码器标定设置 `calibrated`。自动执行还要求真实画质和处理能力反馈，不能为方便接线将这些标志写成常量 true。

需要迟滞时，每连接保存独立 `ContentAwareStreamPolicyState`，调用 `EvaluateContentAwareStream`；仅在 `confirmed` 后由宿主执行，并在观察到尺寸、FPS 和两项码率值确实匹配后调用 `ConfirmContentAwareStreamApplied`。参考模型不作为新锚点的实测视觉证明。该模块不调用 sender，不替代 GoogCC，也不调整静态采集、心跳或变化抑制。执行、回退与后续画质验证由宿主负责。

可选 `media_intelligence/core/H264ReferenceQualityModel.h` 提供共用 H.264 参考初值，支持 NVENC/QSV/AMF/libx264/OpenH264。宿主保留 `H264ReferenceQualityContext`，设置 `qualityEstimator=H264ReferenceQualityContext::Estimate`、`qualityEstimatorContext=&context` 和 `allowReferenceModel=true`，才能显式允许参考执行；同时提供完整实时 QP/负载反馈。参考标记为 `reference=true, calibrated=false`，阈值与余量可调，不代表相应显卡实測结果。默认核心配置和原示例仍只观察，RLink 宿主默认允许参考模式。

核心还提供 `media_intelligence/core/CalibratedStreamQualityModel.h`。宿主把离线测量表、codec/encoder/profile 与证据标识交给 `Build`，发布不可变模型；使用 `CalibratedStreamQualityContext` 持有模型副本并提供 estimator 回调。表最多 128 条，查询无堆分配，只覆盖已测量的相同场景和长宽比，不外推。实时窗口 QP 只有在同一视觉标定提供了明确边界时才能用于 `VerifyCurrentQuality`；没有边界、编码器不匹配或场景未覆盖时仍不可执行。示例和自测的合成表不能作为实际应用的标定数据。
