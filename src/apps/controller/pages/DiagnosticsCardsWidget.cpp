// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "DiagnosticsCardsWidget.h"

#include <QApplication>
#include <QClipboard>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include <QStringList>
#include <QStyle>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

namespace remote::controller::detail {
namespace {

QString DiagnosticsMetricExplanation(const QString &label) {
  static const QHash<QString, QString> explanations{
      {QStringLiteral("估算视频预算"), QStringLiteral("估计可用上行的 95%，最多达到用户视频码率上限；这是分配预算，不是实际流量。")},
      {QStringLiteral("候选参考需求"), QStringLiteral("模型估算候选规格的画质需求。健康网络保持已有参数，因此参考需求可能高于期望码率；弱网可执行候选必须满足预算。")},
      {QStringLiteral("候选期望码率"), QStringLiteral("策略准备分配的编码预算；经连续窗口确认后应用，不等于 WebRTC 实际目标码率。")},
      {QStringLiteral("候选发送上限"), QStringLiteral("候选确认后准备写入 RTP 的码率上限；当前实际应用值见“当前发送上限”，编码器不一定跑满。")},
      {QStringLiteral("状态"),
       QStringLiteral(
           "当前对象的运行状态；等待、连接中或关闭时会随实时状态更新。")},
      {QStringLiteral("连接路径"),
       QStringLiteral(
           "当前选中的 ICE 传输路径：局域网直连、STUN 公网直连或 TURN 中继。")},
      {QStringLiteral("当前 RTT"),
       QStringLiteral(
           "当前候选对的网络往返时间，不包含视频编解码和界面显示时间。")},
      {QStringLiteral("采样窗上行"),
       QStringLiteral("相邻两次 Stats "
                      "采样之间实际发送字节的平均速率，不是会话累计平均值。")},
      {QStringLiteral("采样窗下行"),
       QStringLiteral("相邻两次 Stats "
                      "采样之间实际接收字节的平均速率，不是会话累计平均值。")},
      {QStringLiteral("ICE"),
       QStringLiteral("ICE 连接状态，反映候选地址检查和 P2P 网络连通情况。")},
      {QStringLiteral("DTLS"),
       QStringLiteral("DTLS 安全传输状态；connected 表示密钥协商已经完成。")},
      {QStringLiteral("连接角色"),
       QStringLiteral("本机在 ICE 协商中的 controlling 或 controlled "
                      "角色，不代表远控中的控制端身份。")},
      {QStringLiteral("候选对"),
       QStringLiteral(
           "当前 ICE 候选地址对的状态；succeeded 表示该路径已验证可用。")},
      {QStringLiteral("本地候选"),
       QStringLiteral(
           "当前传输路径使用的本机地址、端口、候选类型、协议和网络适配器。")},
      {QStringLiteral("远端候选"),
       QStringLiteral("当前传输路径使用的对端地址、端口、候选类型和协议。")},
      {QStringLiteral("估计可用上行"),
       QStringLiteral(
           "WebRTC 拥塞控制估计的当前可用发送带宽，不等于运营商标称带宽。")},
      {QStringLiteral("估计可用下行"),
       QStringLiteral("WebRTC 在可获得该信息时报告的当前可用接收带宽。")},
      {QStringLiteral("安全套件"),
       QStringLiteral(
           "当前信令/媒体连接协商使用的 TLS、DTLS 和 SRTP 加密算法。")},
      {QStringLiteral("累计路径切换"),
       QStringLiteral("本次成员对会话中选中的 ICE 候选对发生变化的累计次数。")},
      {QStringLiteral("累计发送丢弃"),
       QStringLiteral("数据交给系统网络套接字前因错误或缓冲不足而被本机丢弃的累"
                      "计包数和字节数。")},
      {QStringLiteral("编码格式"),
       QStringLiteral("当前 RTP 媒体流实际协商并使用的编码格式。")},
      {QStringLiteral("采样窗码率"),
       QStringLiteral("相邻两次 Stats 采样之间该 RTP "
                      "流的有效负载码率；静止画面可能非常低。")},
      {QStringLiteral("编码器"),
       QStringLiteral(
           "当前 RTP 发送流实际使用的编码器实现，不是设置中的期望值。")},
      {QStringLiteral("解码器"),
       QStringLiteral(
           "当前 RTP 接收流实际使用的解码器实现，可用于确认硬件或软件路径。")},
      {QStringLiteral("累计数据包"),
       QStringLiteral(
           "从该 RTP "
           "流建立以来累计发送或接收的数据包数量；接收侧包含重传包。")},
      {QStringLiteral("累计丢包"),
       QStringLiteral(
           "依据 RTP "
           "序号估算的累计丢失包数及比例；重传恢复后仍应结合重传数据判断。")},
      {QStringLiteral("累计重传"),
       QStringLiteral(
           "本次 RTP 流累计发送或接收的 RTX 重传包数和有效负载字节数。")},
      {QStringLiteral("分辨率"),
       QStringLiteral("最近一帧实际编码或解码完成的视频尺寸。")},
      {QStringLiteral("WebRTC当前帧率"),
       QStringLiteral("最近一秒编码或解码完成的帧数；这是媒体吞吐率，不是 Qt "
                      "窗口实际 Present 帧率。")},
      {QStringLiteral("DXGI原始采集"),
       QStringLiteral("屏幕采集线程最近采样窗内每秒调用 CaptureFrame "
                      "的次数、成功交付的原始桌面帧数，以及观看端要求的采集目标"
                      "。它位于 WebRTC 自适应和编码之前。")},
      {QStringLiteral("WebRTC源输出"),
       QStringLiteral(
           "WebRTC RTCVideoSourceStats 报告的媒体源帧率。与 DXGI "
           "成功交付帧率之间的差值，通常来自 WebRTC 源适配或调度。")},
      {QStringLiteral("编码完成帧率"),
       QStringLiteral(
           "根据相邻两次 Stats 的 framesEncoded "
           "增量独立计算，表示编码器在该采样窗内每秒实际完成的帧数。")},
      {QStringLiteral("RTP发送帧率"),
       QStringLiteral(
           "根据相邻两次 Stats 的 framesSent "
           "增量独立计算，表示编码完成后实际交给 RTP 发送链路的帧数。")},
      {QStringLiteral("采集调用耗时"),
       QStringLiteral("屏幕采集线程最近一次 CaptureFrame "
                      "调用的同步耗时；持续接近或超过目标帧间隔表示采集或显卡驱"
                      "动可能成为瓶颈。")},
      {QStringLiteral("累计编码帧"),
       QStringLiteral(
           "本次 RTP 发送流成功编码的累计帧数，以及其中的关键帧数量。")},
      {QStringLiteral("累计解码帧"),
       QStringLiteral(
           "本次 RTP "
           "接收流成功解码的累计帧数、关键帧数和解码前或超时丢弃的帧数。")},
      {QStringLiteral("最近一帧 QP"),
       QStringLiteral("最近完成帧的量化参数；数值通常越低画质越高，部分硬件路径"
                      "不会报告。")},
      {QStringLiteral("累计反馈"),
       QStringLiteral("NACK 请求重传；PLI 请求新关键帧；FIR "
                      "强制请求完整帧。三项均为会话累计值。")},
      {QStringLiteral("WebRTC目标码率"),
       QStringLiteral("WebRTC 拥塞控制当前分配给该发送流的目标编码码率。")},
      {QStringLiteral("WebRTC启动码率"),
       QStringLiteral("本应用在分辨率或目标帧率发生变化时写入 PeerConnection "
                      "的带宽估计起始值；它会重置当前估计，但仍可被拥塞控制继续"
                      "升高或降低。")},
      {QStringLiteral("发送策略上限"),
       QStringLiteral("当前观看端为该成员对请求并成功应用的最大输出分辨率和最大"
                      "帧率；实际值仍可能被 WebRTC 自适应降低。")},
      {QStringLiteral("应用码率上限"),
       QStringLiteral("本应用根据输出分辨率和目标帧率计算后写入 RTP Sender "
                      "的码率天花板；它不是 WebRTC 必须跑满的目标码率。")},
      {QStringLiteral("当前媒体 RTT"),
       QStringLiteral("由远端 RTP/RTCP 反馈得到的该媒体流往返时间。")},
      {QStringLiteral("采集源"),
       QStringLiteral(
           "编码前的本地采集分辨率和采集帧率，用于区分采集与编码瓶颈。")},
      {QStringLiteral("最近一帧编码"),
       QStringLiteral("最近一帧从调用 Encode "
                      "到编码结果回调的精确流水线时间，包含内部排队，不是纯硬件"
                      "执行时间。")},
      {QStringLiteral("采样窗平均编码"),
       QStringLiteral("最近采样窗内 totalEncodeTime "
                      "增量除以新增编码帧数得到的平均编码流水线时间。")},
      {QStringLiteral("会话平均编码"),
       QStringLiteral("从流建立至今累计编码时间除以累计编码帧数。")},
      {QStringLiteral("采样窗平均 QP"),
       QStringLiteral("最近采样窗内新增帧的平均量化参数；未报告表示当前编解码路"
                      "径没有提供 QP。")},
      {QStringLiteral("当前质量限制"),
       QStringLiteral(
           "WebRTC 当前降低画质的主要原因，例如带宽、CPU 或其他限制。")},
      {QStringLiteral("网络抖动"),
       QStringLiteral(
           "按 RTP 到达时间计算的包间隔波动，不是 RTT，也不包含解码时间。")},
      {QStringLiteral("采样窗抖动缓冲"),
       QStringLiteral(
           "最近采样窗内，每帧从进入到离开 WebRTC 抖动缓冲区的平均等待时间。")},
      {QStringLiteral("最近一帧解码"),
       QStringLiteral(
           "最近一帧从调用 Decode 到解码帧回调的精确流水线时间，包含异步队列和 "
           "MFT 内部排队，不是纯 GPU 执行时间。")},
      {QStringLiteral("输入准备耗时"),
       QStringLiteral("硬件解码器最近一帧从进入 Decode 到 H264 数据复制进 MF "
                      "输入样本并准备完成的时间。")},
      {QStringLiteral("输入排队耗时"),
       QStringLiteral(
           "硬件解码器最近一帧从输入样本准备完成，到 MFT 接受 ProcessInput "
           "的等待时间；异步路径包含等待 NeedInput 事件。")},
      {QStringLiteral("MFT输出等待"),
       QStringLiteral("最近一帧从 MFT 接受 ProcessInput 到 ProcessOutput "
                      "返回对应输出样本的流水线时间；包含 MFT、驱动、DXVA/GPU "
                      "内部排队，不等同于纯 GPU 核心执行时间。")},
      {QStringLiteral("纹理交付耗时"),
       QStringLiteral("最近一帧从 ProcessOutput 返回，到取得 D3D11 NV12 "
                      "纹理、构造 WebRTC 原生帧并准备调用解码回调的时间。")},
      {QStringLiteral("硬解积压"),
       QStringLiteral(
           "当前等待 MFT "
           "接收的输入帧、已经提交但尚未输出的帧，以及本次硬解实例出现过的峰值"
           "总积压；达到 8 帧会完整回退软件解码，不会静默丢弃 H264 参考帧。")},
      {QStringLiteral("采样窗平均解码"),
       QStringLiteral("最近采样窗内 totalDecodeTime "
                      "增量除以新增解码帧数；口径是送入解码器到返回完整帧。")},
      {QStringLiteral("会话平均解码"),
       QStringLiteral("从流建立至今累计解码时间除以累计成功解码帧数。")},
      {QStringLiteral("采样窗处理延迟"),
       QStringLiteral(
           "最近采样窗内从一帧首个 RTP 包到达，到完整解码完成的平均时间。")},
      {QStringLiteral("会话平均处理延迟"),
       QStringLiteral(
           "从流建立至今，首个 RTP 包到完整解码完成的历史平均时间。")},
      {QStringLiteral("会话平均抖动缓冲"),
       QStringLiteral(
           "从流建立至今，每帧在 WebRTC 抖动缓冲区中的历史平均等待时间。")},
      {QStringLiteral("累计冻结/暂停"),
       QStringLiteral("冻结是异常长的帧间隔；暂停是超过 5 "
                      "秒没有新渲染帧。分别显示累计次数和持续时间。")},
      {QStringLiteral("本机显示路径"),
       QStringLiteral("远端解码帧在本机实际采用的显示路径，以及 Qt "
                      "从当前显示器读取到的刷新率。D3D11 表示硬解原生 NV12 "
                      "纹理直达 VideoProcessor；CPU I420/D3D11 表示软件解码的 "
                      "Y/U/V 三平面直接上传并由 Pixel Shader 转换显示；CPU "
                      "NV12/D3D11、CPU/D3D11 和 CPU/Qt 依次是兼容回退。")},
      {QStringLiteral("应用帧到达"),
       QStringLiteral("RemoteDesktopCanvas 最近 1 "
                      "秒实际收到的解码后视频帧数及窗口生命周期累计值。它位于 "
                      "WebRTC 解码之后、应用显示邮箱之前。")},
      {QStringLiteral("显示提交帧率"),
       QStringLiteral("最近 1 秒首次提交到 D3D11 交换链或首次由 Qt "
                      "绘制的新视频帧数及累计值。它是应用能够精确测量的最终显示"
                      "提交率，不冒充显示器物理扫描率。")},
      {QStringLiteral("应用覆盖丢帧"),
       QStringLiteral("显示线程尚未消费旧帧时，新帧覆盖应用最新帧邮箱，或转换完"
                      "成后已被更新帧淘汰的累计数量。该值不包含在 WebRTC Stats "
                      "的 framesDropped 中。")},
      {QStringLiteral("呈现帧间隔"),
       QStringLiteral("最近 240 个成功呈现帧之间的墙钟间隔：平均值、P95 "
                      "和最大值。大幅拖动时若最大值或 P95 "
                      "突增，说明卡顿发生在解码之后的显示调度阶段。")},
      {QStringLiteral("CPU帧格式整理"),
       QStringLiteral("软件显示路径最近一帧和最近 240 帧的 CPU 准备耗时。I420 "
                      "三平面路径统计 Map、逐行上传 Y/U/V 和 "
                      "Unmap；兼容回退统计 I420ToNV12 或 I420ToARGB。")},
      {QStringLiteral("I420三平面上传"),
       QStringLiteral("软件解码帧的 Y、U、V 三个平面通过动态 D3D11 "
                      "纹理环上传所需的 CPU 墙钟时间，包含 Map、逐行复制和 "
                      "Unmap，不包含 Pixel Shader 执行时间。")},
      {QStringLiteral("Pixel Shader提交"),
       QStringLiteral(
           "I420 三平面显示路径调用 D3D11 Draw 的 CPU 命令提交耗时；YUV 转 RGB "
           "与缩放实际在 GPU Pixel Shader 中执行。")},
      {QStringLiteral("VideoProcessor提交"),
       QStringLiteral(
           "D3D11 原生路径调用 VideoProcessorBlt 的最近一次和最近 240 次 CPU "
           "墙钟耗时。它是驱动命令提交时间，不冒充 GPU 核心执行时间。")},
      {QStringLiteral("Qt绘制提交"),
       QStringLiteral("CPU/Qt 路径第一次为新帧执行 QPainter::drawImage "
                      "的最近一次和最近 240 次 CPU 墙钟耗时。")},
      {QStringLiteral("Present调用"),
       QStringLiteral("D3D11 原生路径调用交换链 Present(0,0) 的最近一次和最近 "
                      "240 次 CPU 墙钟耗时；同步或驱动阻塞会直接反映在这里。")},
      {QStringLiteral("显示失败"),
       QStringLiteral("本次远程画面窗口生命周期内，颜色转换、D3D11 "
                      "VideoProcessor 或交换链 Present 失败的累计次数。")},
      {QStringLiteral("当前电平/累计隐藏"),
       QStringLiteral("当前音频电平，以及因丢包而由 WebRTC "
                      "补偿或隐藏的累计音频样本和事件。")},
      {QStringLiteral("协议"),
       QStringLiteral("当前 DataChannel 使用的上层协议标识；底层传输为 WebRTC "
                      "SCTP/DTLS。")},
      {QStringLiteral("发送缓冲"),
       QStringLiteral("尚未由 SCTP 发出的 DataChannel "
                      "数据量；持续升高表示发送端产生数据快于网络发送。")},
      {QStringLiteral("累计消息"),
       QStringLiteral("该 DataChannel 从建立以来累计发送和接收的消息条数。")},
      {QStringLiteral("累计流量"),
       QStringLiteral(
           "该 DataChannel 从建立以来累计发送和接收的有效负载字节数。")},
  };
  return explanations.value(
      label, QStringLiteral("该调试指标的当前实时或累计统计值。"));
}

QString DiagnosticsMetricToolTip(const DiagnosticsChip &chip) {
  return QStringLiteral("%1\n%2").arg(chip.label,
                                      DiagnosticsMetricExplanation(chip.label));
}

} // namespace

DiagnosticsCardsWidget::DiagnosticsCardsWidget(QWidget *parent)
    : QWidget(parent), layout_(new QVBoxLayout(this)) {
  setObjectName(QStringLiteral("statsCardsHost"));
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
  setMinimumHeight(160);
  layout_->setContentsMargins(0, 0, 0, 0);
  layout_->setSpacing(14);
  Rebuild({}, QStringLiteral("正在等待 WebRTC 连接质量数据…"));
  // Force the first real snapshot, including an empty one, to replace
  // the startup placeholder instead of comparing equal to an empty
  // structure key.
  structure_ = QStringLiteral("__startup_placeholder__");
}

void DiagnosticsCardsWidget::SetSections(
    const QVector<DiagnosticsSection> &sections, const QString &emptyText) {
  if (sections_ == sections && emptyText_ == emptyText &&
      structure_ != QStringLiteral("__startup_placeholder__")) return;
  sections_ = sections;
  emptyText_ = emptyText;
  QStringList structureParts;
  cardCopyTexts_.clear();
  for (const auto &section : sections) {
    structureParts << section.key;
    for (const auto &card : section.cards) {
      structureParts << card.key << QString::number(card.stackedMetrics)
                     << QString::number(card.copyable);
      QStringList copyLines{card.title};
      copyLines << (card.copyable ? section.title : card.subtitle.isEmpty()
                          ? QStringLiteral("成员：未报告")
                          : QStringLiteral("成员：%1").arg(card.subtitle));
      for (const auto &chip : card.chips) {
        structureParts << QStringLiteral("%1:%2").arg(chip.key).arg(
            chip.wide ? 1 : 0);
        copyLines << QStringLiteral("%1：%2").arg(chip.label, chip.value);
      }
      cardCopyTexts_.insert(section.key + QLatin1Char('/') + card.key,
                            copyLines.join(QLatin1Char('\n')));
    }
  }
  const QString structure = structureParts.join(QLatin1Char('|'));
  if (structure != structure_ || layout_->count() == 0) {
    Rebuild(sections, emptyText);
    structure_ = structure;
  }

  if (sections.isEmpty()) {
    if (auto* empty = findChild<QLabel*>(QStringLiteral("statsEmptyText"))) {
      if (empty->text() != emptyText) empty->setText(emptyText);
    }
  }
  RefreshValues();
}

void DiagnosticsCardsWidget::RefreshValues() {
  for (const auto &section : sections_) {
    if (auto* title = sectionButtons_.value(section.key)) {
      if (title->text() != section.title) title->setText(section.title);
    }
    if (auto* description = sectionDescriptions_.value(section.key)) {
      if (description->text() != section.description) description->setText(section.description);
    }
    for (const auto &card : section.cards) {
      const QString cardPrefix = section.key + QLatin1Char('/') + card.key;
      if (auto *title = titleButtons_.value(cardPrefix + "/title", nullptr)) {
        if (title->text() != card.title) title->setText(card.title);
      }
      if (auto *subtitle =
              textLabels_.value(cardPrefix + "/subtitle", nullptr)) {
        if (subtitle->text() != card.subtitle) subtitle->setText(card.subtitle);
        subtitle->setVisible(!card.subtitle.isEmpty());
      }
      for (const auto &chip : card.chips) {
        const QString chipKey = cardPrefix + QLatin1Char('/') + chip.key;
        auto* frame = chipFrames_.value(chipKey, nullptr);
        if (!frame || !frame->isVisibleTo(this)) continue;
        const QString toolTip = DiagnosticsMetricToolTip(chip);
        if (auto *value = textLabels_.value(chipKey, nullptr)) {
          if (value->text() != chip.value) value->setText(chip.value);
          if (value->toolTip() != toolTip) value->setToolTip(toolTip);
        }
        if (auto *label = chipNameLabels_.value(chipKey, nullptr)) {
          if (label->text() != chip.label) label->setText(chip.label);
          if (label->toolTip() != toolTip) label->setToolTip(toolTip);
        }
        {
          if (frame->toolTip() != toolTip) frame->setToolTip(toolTip);
          if (frame->property("tone").toByteArray() != chip.tone) {
            frame->setProperty("tone", chip.tone);
            frame->style()->unpolish(frame);
            frame->style()->polish(frame);
          }
        }
      }
    }
  }
}

void DiagnosticsCardsWidget::Rebuild(
    const QVector<DiagnosticsSection> &sections, const QString &emptyText) {
  setUpdatesEnabled(false);
  while (auto *item = layout_->takeAt(0)) {
    delete item->widget();
    delete item;
  }
  textLabels_.clear();
  chipNameLabels_.clear();
  chipFrames_.clear();
  titleButtons_.clear();
  sectionButtons_.clear();
  sectionDescriptions_.clear();

  if (sections.isEmpty()) {
    auto *empty = new QLabel(emptyText, this);
    empty->setWordWrap(true);
    empty->setObjectName(QStringLiteral("statsEmptyText"));
    empty->setAlignment(Qt::AlignCenter);
    empty->setMinimumHeight(120);
    layout_->addWidget(empty);
    setUpdatesEnabled(true);
    update();
    return;
  }

  for (const auto &section : sections) {
    auto *sectionHost = new QWidget(this);
    auto *sectionLayout = new QVBoxLayout(sectionHost);
    sectionLayout->setContentsMargins(0, 0, 0, 0);
    sectionLayout->setSpacing(10);

    auto *sectionHeader = new QWidget(sectionHost);
    auto *sectionHeaderLayout = new QHBoxLayout(sectionHeader);
    sectionHeaderLayout->setContentsMargins(2, 4, 2, 2);
    sectionHeaderLayout->setSpacing(10);
    const bool sectionExpanded = sectionExpanded_.value(
        section.key, section.initiallyExpanded || section.key == QStringLiteral("connection"));
    sectionExpanded_.insert(section.key, sectionExpanded);
    auto *sectionToggle = new QToolButton(sectionHeader);
    sectionToggle->setObjectName(QStringLiteral("statsSectionToggle"));
    sectionToggle->setText(section.title);
    sectionToggle->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    sectionToggle->setArrowType(sectionExpanded ? Qt::DownArrow
                                                : Qt::RightArrow);
    sectionToggle->setCheckable(true);
    sectionToggle->setChecked(sectionExpanded);
    sectionToggle->setCursor(Qt::PointingHandCursor);
    sectionHeaderLayout->addWidget(sectionToggle);
    sectionButtons_.insert(section.key, sectionToggle);
    auto *sectionDescription = new QLabel(section.description, sectionHeader);
    sectionDescription->setWordWrap(true);
    sectionDescription->setObjectName(
        QStringLiteral("statsSectionDescription"));
    sectionHeaderLayout->addWidget(sectionDescription);
    sectionDescriptions_.insert(section.key, sectionDescription);
    sectionHeaderLayout->addStretch(1);
    auto *count = new QLabel(QStringLiteral("%1 项").arg(section.cards.size()),
                             sectionHeader);
    count->setObjectName(QStringLiteral("statsCountPill"));
    sectionHeaderLayout->addWidget(count);
    sectionLayout->addWidget(sectionHeader);

    auto *sectionContent = new QWidget(sectionHost);
    auto *sectionContentLayout = new QVBoxLayout(sectionContent);
    sectionContentLayout->setContentsMargins(0, 0, 0, 0);
    sectionContentLayout->setSpacing(10);
    sectionContent->setVisible(sectionExpanded);
    connect(sectionToggle, &QToolButton::toggled, sectionHost,
            [this, sectionContent, sectionToggle,
             sectionKey = section.key](bool expanded) {
              sectionExpanded_.insert(sectionKey, expanded);
              if (expanded) PopulateSection(sectionContent, sectionKey);
              sectionContent->setVisible(expanded);
              if (expanded) RefreshValues();
              sectionToggle->setArrowType(expanded ? Qt::DownArrow
                                                   : Qt::RightArrow);
              updateGeometry();
            });

    if (sectionExpanded) PopulateSection(sectionContent, section.key);
    sectionLayout->addWidget(sectionContent);
    layout_->addWidget(sectionHost);
  }
  layout_->addStretch(1);
  setUpdatesEnabled(true);
  layout_->invalidate();
  layout_->activate();
  updateGeometry();
  if (parentWidget()) {
    parentWidget()->updateGeometry();
  }
  update();
}


// Materialize only expanded sections/cards. Collapsed groups retain their full
// latest model (including copy text), without hundreds of hidden QLabel layouts.
void DiagnosticsCardsWidget::PopulateSection(QWidget* sectionContent,
                                             const QString& sectionKey) {
  auto* sectionContentLayout = qobject_cast<QVBoxLayout*>(sectionContent->layout());
  if (!sectionContentLayout || sectionContentLayout->count() != 0) return;
  for (const auto& section : sections_) {
    if (section.key != sectionKey) continue;
    for (const auto &card : section.cards) {
      auto *cardFrame = new QFrame(sectionContent);
      cardFrame->setObjectName(QStringLiteral("statsMetricCard"));
      auto *cardLayout = new QVBoxLayout(cardFrame);
      cardLayout->setContentsMargins(16, 14, 16, 16);
      cardLayout->setSpacing(11);

      auto *cardHeader = new QWidget(cardFrame);
      auto *cardHeaderLayout = new QHBoxLayout(cardHeader);
      cardHeaderLayout->setContentsMargins(0, 0, 0, 0);
      cardHeaderLayout->setSpacing(9);
      const QString cardPrefix = section.key + QLatin1Char('/') + card.key;
      const bool cardExpanded = cardExpanded_.value(cardPrefix, card.initiallyExpanded);
      cardExpanded_.insert(cardPrefix, cardExpanded);
      auto *cardToggle = new QToolButton(cardHeader);
      cardToggle->setObjectName(QStringLiteral("statsCardToggle"));
      cardToggle->setText(card.title);
      cardToggle->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
      cardToggle->setArrowType(cardExpanded ? Qt::DownArrow : Qt::RightArrow);
      cardToggle->setCheckable(true);
      cardToggle->setChecked(cardExpanded);
      cardToggle->setCursor(Qt::PointingHandCursor);
      cardHeaderLayout->addWidget(cardToggle);
      auto *subtitle = new QLabel(card.subtitle, cardHeader);
      subtitle->setObjectName(QStringLiteral("statsPeerPill"));
      subtitle->setVisible(!card.subtitle.isEmpty());
      cardHeaderLayout->addWidget(subtitle);
      cardHeaderLayout->addStretch(1);
      if (card.copyable || card.title == QStringLiteral("屏幕接收") ||
          card.title == QStringLiteral("屏幕发送")) {
        const bool outboundScreen = card.title == QStringLiteral("屏幕发送");
        auto *copyButton =
            new QPushButton(QStringLiteral("复制信息"), cardHeader);
        copyButton->setObjectName(QStringLiteral("softButton"));
        copyButton->setCursor(Qt::PointingHandCursor);
        copyButton->setToolTip(
            card.copyable ? QStringLiteral("复制当前卡片中的全部指标") : outboundScreen
                ? QStringLiteral("复制当前屏幕发送卡片中的全部指标")
                : QStringLiteral("复制当前屏幕接收卡片中的全部指标"));
        connect(copyButton, &QPushButton::clicked, copyButton,
                [this, copyButton, cardKey = cardPrefix] {
                  QApplication::clipboard()->setText(
                      cardCopyTexts_.value(cardKey));
                  copyButton->setText(QStringLiteral("已复制"));
                  QTimer::singleShot(1200, copyButton, [copyButton] {
                    copyButton->setText(QStringLiteral("复制信息"));
                  });
                });
        cardHeaderLayout->addWidget(copyButton);
      }
      cardLayout->addWidget(cardHeader);

      titleButtons_.insert(cardPrefix + "/title", cardToggle);
      textLabels_.insert(cardPrefix + "/subtitle", subtitle);

      auto *chipsHost = new QWidget(cardFrame);
      if (cardExpanded) PopulateCard(chipsHost, cardPrefix);
      chipsHost->setVisible(cardExpanded);
      cardLayout->addWidget(chipsHost);
      connect(
          cardToggle, &QToolButton::toggled, cardFrame,
          [this, chipsHost, cardToggle, cardKey = cardPrefix](bool expanded) {
            cardExpanded_.insert(cardKey, expanded);
            if (expanded) PopulateCard(chipsHost, cardKey);
            chipsHost->setVisible(expanded);
            if (expanded) RefreshValues();
            cardToggle->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);
            updateGeometry();
          });
      sectionContentLayout->addWidget(cardFrame);
    }
    break;
  }
}

void DiagnosticsCardsWidget::PopulateCard(QWidget* chipsHost,
                                          const QString& cardPrefix) {
  if (chipsHost->layout()) return;
  for (const auto& section : sections_) {
    for (const auto& card : section.cards) {
      if (section.key + QLatin1Char('/') + card.key != cardPrefix) continue;
      auto *chips = new QGridLayout(chipsHost);
      chips->setContentsMargins(0, 0, 0, 0);
      chips->setHorizontalSpacing(9);
      chips->setVerticalSpacing(9);
      int row = 0;
      int column = 0;
      for (const auto &chip : card.chips) {
        if (chip.wide && column != 0) {
          ++row;
          column = 0;
        }
        auto *chipFrame = new QFrame(chipsHost);
        chipFrame->setObjectName(QStringLiteral("statsMetricChip"));
        chipFrame->setProperty("tone", chip.tone);
        chipFrame->setProperty("stackedMetrics", card.stackedMetrics);
        QBoxLayout *chipLayout = card.stackedMetrics
            ? static_cast<QBoxLayout *>(new QVBoxLayout(chipFrame))
            : static_cast<QBoxLayout *>(new QHBoxLayout(chipFrame));
        chipLayout->setContentsMargins(11, 8, 11, 8);
        chipLayout->setSpacing(7);
        auto *chipLabel = new QLabel(chip.label, chipFrame);
        chipLabel->setObjectName(QStringLiteral("statsChipLabel"));
        auto *chipValue = new QLabel(chip.value, chipFrame);
        chipValue->setTextFormat(Qt::PlainText);
        chipValue->setObjectName(QStringLiteral("statsChipValue"));
        chipValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
        chipValue->setWordWrap(chip.wide || card.stackedMetrics);
        const QString toolTip = DiagnosticsMetricToolTip(chip);
        chipFrame->setToolTip(toolTip);
        chipLabel->setToolTip(toolTip);
        chipValue->setToolTip(toolTip);
        chipLayout->addWidget(chipLabel);
        chipLayout->addWidget(chipValue, 1);

        if (chip.wide) {
          chips->addWidget(chipFrame, row, 0, 1, 2);
          ++row;
          column = 0;
        } else {
          chips->addWidget(chipFrame, row, column);
          if (++column == 2) {
            ++row;
            column = 0;
          }
        }
        const QString chipKey = cardPrefix + QLatin1Char('/') + chip.key;
        textLabels_.insert(chipKey, chipValue);
        chipNameLabels_.insert(chipKey, chipLabel);
        chipFrames_.insert(chipKey, chipFrame);
      }
      chips->setColumnStretch(0, 1);
      chips->setColumnStretch(1, 1);
      return;
    }
  }
}

} // namespace remote::controller::detail
