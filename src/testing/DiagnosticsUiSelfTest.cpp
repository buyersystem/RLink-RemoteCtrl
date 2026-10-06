// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

// UI-only regression/performance test: no engine, network, settings or account data.
#include "src/apps/controller/CurrentPageStack.h"
#include "src/apps/controller/FramelessWindow.h"
#include "src/apps/controller/MediaControls.h"
#include "src/apps/controller/RecentDevicesRefreshState.h"
#include "src/apps/controller/pages/DiagnosticsCardsWidget.h"
#include "src/apps/controller/pages/DiagnosticsPage.h"
#include "src/apps/controller/pages/RecentConnectionsPage.h"
#include "src/apps/controller/ui/RemoteCTheme.h"

#include <QApplication>
#include <QClipboard>
#include <QElapsedTimer>
#include <QFile>
#include <QFrame>
#include <QLabel>
#include <QPixmap>
#include <QPointer>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSettings>
#include <QTemporaryDir>
#include <QTextStream>
#include <QToolButton>
#include <QVBoxLayout>

#include <functional>

using namespace remote::controller::detail;

namespace remote::controller::detail {
// Identical key fixtures avoid linking the broad application Support.cpp TU;
// the actual DiagnosticsPage implementation and input telemetry remain unmocked.
extern const char kScreenFrameRateLogEnabledSetting[] =
    "diagnostics/screenFrameRateCsvEnabled";
extern const char kInputEventStatsEnabledSetting[] =
    "diagnostics/inputEventStatsEnabled";
}

namespace {
int failures = 0;
void Check(bool condition, const char *message) {
  QTextStream(condition ? stdout : stderr)
      << (condition ? "PASS " : "FAIL ") << message << '\n';
  if (!condition)
    ++failures;
}

double Measure(const char *label, const std::function<void()> &action) {
  QElapsedTimer timer;
  timer.start();
  action();
  const double ms = timer.nsecsElapsed() / 1000000.0;
  QTextStream(stdout) << "TIMING " << label << " " << ms << " ms\n";
  return ms;
}

void FlushUi() {
  QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  QApplication::processEvents();
}

void TestUiConfigurationCache() {
  using namespace remote::controller;
  using ui::RemoteCTheme;
  using ui::ThemePreference;
  QSettings settings;
  const ThemePreference originalTheme = RemoteCTheme::LoadPreference();
  const int originalAnimation = CurrentUiAnimationLevel();

  for (const auto theme : {ThemePreference::kLight, ThemePreference::kDark,
                           ThemePreference::kSystem}) {
    RemoteCTheme::SavePreference(theme);
    Check(RemoteCTheme::LoadPreference() == theme &&
              settings.value(QStringLiteral("ui/themeMode")).toString() ==
                  RemoteCTheme::PreferenceValue(theme),
          "theme preference cache follows saves and persists selection");
  }
  Check(RemoteCTheme::IsDark(ThemePreference::kLight) == false &&
            RemoteCTheme::IsDark(ThemePreference::kDark) == true &&
            RemoteCTheme::IsDark(RemoteCTheme::LoadPreference()) ==
                RemoteCTheme::IsDark(ThemePreference::kSystem),
        "cached system preference still resolves current system appearance");
  settings.setValue(QStringLiteral("ui/themeMode"), QStringLiteral("dark"));
  RemoteCTheme::ReloadPreference();
  Check(RemoteCTheme::LoadPreference() == ThemePreference::kDark,
        "theme preference explicit reload observes external settings change");

  for (const int level : {0, 1, 2, -1, 3}) {
    SaveUiAnimationLevel(level);
    const int expected = level < 0 ? 0 : (level > 2 ? 2 : level);
    Check(CurrentUiAnimationLevel() == expected &&
              settings.value(QStringLiteral("ui/animationLevel")).toInt() == expected &&
              settings.value(QStringLiteral("ui/animationsEnabled")).toBool() ==
                  (expected > 0),
          "animation cache updates immediately with clamping and legacy persistence");
  }
  settings.remove(QStringLiteral("ui/animationLevel"));
  settings.setValue(QStringLiteral("ui/animationsEnabled"), false);
  ReloadUiAnimationLevel();
  Check(CurrentUiAnimationLevel() == 0,
        "animation cache reload preserves legacy disabled fallback");
  settings.setValue(QStringLiteral("ui/animationsEnabled"), true);
  ReloadUiAnimationLevel();
  Check(CurrentUiAnimationLevel() == 2,
        "animation cache reload preserves legacy complete fallback");

  int checksum = 0;
  Measure("ui_config_settings_read_10000", [&] {
    for (int i = 0; i < 10000; ++i) {
      QSettings current;
      checksum += current.value(QStringLiteral("ui/animationsEnabled"), true).toBool();
      checksum += static_cast<int>(RemoteCTheme::PreferenceFromValue(
          current.value(QStringLiteral("ui/themeMode"), QStringLiteral("system")).toString()));
    }
  });
  Measure("ui_config_cached_read_10000", [&] {
    for (int i = 0; i < 10000; ++i) {
      checksum += CurrentUiAnimationLevel();
      checksum += static_cast<int>(RemoteCTheme::LoadPreference());
    }
  });
  Check(checksum > 0, "configuration timing loops executed");
  RemoteCTheme::SavePreference(originalTheme);
  SaveUiAnimationLevel(originalAnimation);
}

void TestRecentDevicesRefreshState() {
  remote::controller::RecentDevicesRefreshState state;
  const QString accountA = QStringLiteral("account A");
  const QString accountB = QStringLiteral("account B");
  remote::OwnedDevicesSnapshot owned;
  owned.revision = 1;
  auto connectivity = remote::SessionConnectivityState::kOnline;
  const auto accept = [&](const QString& account, bool ready, bool dark, bool force = false) {
    return state.Accept(account, owned, connectivity, ready, dark, force);
  };
  Check(accept(accountA, true, false),
        "recent devices first snapshot refreshes display");
  Check(!accept(accountA, true, false),
        "recent devices identical snapshot skips display preparation");
  Check(accept(accountB, true, false),
        "recent devices account change refreshes display");
  owned.revision = 2;
  Check(accept(accountB, true, false),
        "recent devices owned revision change refreshes display");
  Check(accept(accountB, false, false),
        "recent devices engine readiness change refreshes display");
  Check(accept(accountB, false, true),
        "recent devices theme change refreshes display");
  Check(!accept(accountB, false, true),
        "recent devices updated snapshot skips only unchanged display state");
  Check(accept(accountB, false, true, true),
        "recent devices explicit navigation or history write forces refresh");
  Check(!accept(accountB, false, true),
        "recent devices forced refresh still records latest display state");
  connectivity = remote::SessionConnectivityState::kOffline;
  Check(accept(accountB, false, true),
        "recent devices disconnect refreshes even when readiness and revision are unchanged");
  owned.loaded = true;
  Check(accept(accountB, false, true),
        "recent devices loaded-state changes invalidate unchanged revisions");
  owned.devices.push_back({"987654321", "test device", true, false});
  Check(accept(accountB, false, true),
        "recent devices membership changes invalidate unchanged revisions");
  owned.devices[0].online = false;
  Check(accept(accountB, false, true),
        "recent devices offline availability invalidates unchanged revisions");
  owned.devices[0].current = true;
  Check(accept(accountB, false, true),
        "recent devices current-device changes invalidate unchanged revisions");
  Check(!accept(accountB, false, true),
        "recent devices unchanged availability still skips repeated preparation");
}

void TestRecentConnections() {
  using namespace remote::controller;
  RecentConnectionsPage page;
  QVector<RecentRoomCardData> rooms{{QStringLiteral("123456789"),
      QStringLiteral("test room"), QStringLiteral("available"),
      QStringLiteral("success"), QStringLiteral("join test room"), true}};
  QVector<RecentDeviceCardData> devices{{QStringLiteral("987654321"),
      QStringLiteral("test device"), QStringLiteral("test details"),
      QStringLiteral("connect test device"), true, true, true}};
  page.SetRooms(rooms);
  page.SetDevices(devices);
  QPointer<QPushButton> roomButton = page.findChild<QPushButton *>(QStringLiteral("softButton"));
  QPointer<QPushButton> deviceButton = page.findChild<QPushButton *>(QStringLiteral("primaryButton"));
  Measure("recent_identical_snapshot_100", [&] {
    for (int i = 0; i < 100; ++i) {
      page.SetRooms(rooms);
      page.SetDevices(devices);
    }
    FlushUi();
  });
  Check(roomButton && deviceButton &&
      roomButton == page.findChild<QPushButton *>(QStringLiteral("softButton")) &&
      deviceButton == page.findChild<QPushButton *>(QStringLiteral("primaryButton")),
      "recent unchanged snapshots preserve card and button identity");
  QString requestedRoom;
  QObject::connect(&page, &RecentConnectionsPage::joinRoomRequested,
                   [&requestedRoom](const QString &id) { requestedRoom = id; });
  if (roomButton)
    roomButton->click();
  Check(requestedRoom == rooms[0].roomId, "cached recent room remains clickable");

  rooms[0].canJoin = false;
  rooms[0].actionText = QStringLiteral("room closed");
  devices[0].actionEnabled = false;
  devices[0].actionText = QStringLiteral("device offline");
  page.SetRooms(rooms);
  page.SetDevices(devices);
  FlushUi();
  auto *newRoom = page.findChild<QPushButton *>(QStringLiteral("softButton"));
  auto *newDevice = page.findChild<QPushButton *>(QStringLiteral("primaryButton"));
  Check(roomButton.isNull() && deviceButton.isNull() && newRoom && newDevice &&
          !newRoom->isEnabled() && !newDevice->isEnabled() &&
          newRoom->text() == rooms[0].actionText && newDevice->text() == devices[0].actionText,
        "recent enable/text changes refresh cards");
  QPointer<QPushButton> beforeTheme = newRoom;
  const auto previousTheme = ui::RemoteCTheme::LoadPreference();
  ui::RemoteCTheme::SavePreference(ui::RemoteCTheme::IsDark(previousTheme)
      ? ui::ThemePreference::kLight : ui::ThemePreference::kDark);
  page.SetRooms(rooms);
  page.SetDevices(devices);
  FlushUi();
  Check(beforeTheme.isNull(), "recent same data still refreshes on theme change");
  page.SetRooms({});
  page.SetDevices({});
  FlushUi();
  page.SetRooms({});
  page.SetDevices({});
  Check(page.findChildren<QPushButton *>().isEmpty(),
        "recent repeated empty state has no stale action buttons");
  ui::RemoteCTheme::SavePreference(previousTheme);
}

void TestMediaControls() {
  using namespace remote::controller;
  QPushButton button;
  SetMediaStateButton(&button, MediaStateIcon::kCamera, true, QStringLiteral("first tip"));
  auto iconKey = button.icon().cacheKey();
  Measure("media_identical_state_100", [&] {
    for (int i = 0; i < 100; ++i)
      SetMediaStateButton(&button, MediaStateIcon::kCamera, true, QStringLiteral("updated tip"));
  });
  Check(button.icon().cacheKey() == iconKey && button.toolTip() == QStringLiteral("updated tip") &&
          button.accessibleName() == QStringLiteral("updated tip"),
        "media cached state keeps icon but updates tooltip/accessibility");
  button.setProperty("deviceMenu", true);
  SetMediaStateButton(&button, MediaStateIcon::kCamera, true, QStringLiteral("menu tip"));
  Check(button.toolTip().contains(QStringLiteral("点击右侧箭头选择设备")),
        "media cache preserves updated device menu hint");
  button.setIconSize(QSize(12, 12));
  SetMediaStateButton(&button, MediaStateIcon::kCamera, true, QStringLiteral("size tip"));
  Check(button.iconSize() == QSize(25, 25) && button.icon().cacheKey() != iconKey,
        "media externally changed icon size invalidates cached rendering");
  const auto state = button.property("remoteCMediaIconState");
  SetMediaStateButton(&button, MediaStateIcon::kCamera, false, QStringLiteral("inactive"));
  Check(button.property("remoteCMediaIconState") != state,
        "media active state change invalidates cache");
  const auto previousTheme = ui::RemoteCTheme::LoadPreference();
  const auto preTheme = button.property("remoteCMediaIconState");
  ui::RemoteCTheme::SavePreference(ui::RemoteCTheme::IsDark(previousTheme)
      ? ui::ThemePreference::kLight : ui::ThemePreference::kDark);
  SetMediaStateButton(&button, MediaStateIcon::kCamera, false, QStringLiteral("theme"));
  Check(button.property("remoteCMediaIconState") != preTheme,
        "media theme change invalidates cache");
  ui::RemoteCTheme::SavePreference(previousTheme);
  QPushButton gallery;
  SetCameraGalleryStateButton(&gallery, true, false, QStringLiteral("gallery first"));
  iconKey = gallery.icon().cacheKey();
  SetCameraGalleryStateButton(&gallery, true, false, QStringLiteral("gallery latest"));
  Check(gallery.icon().cacheKey() == iconKey &&
          gallery.toolTip() == QStringLiteral("gallery latest"),
        "gallery cache updates tooltip while keeping unchanged icon");
  SetCameraGalleryStateButton(&gallery, true, true, QStringLiteral("gallery opened"));
  Check(gallery.icon().cacheKey() != iconKey,
        "gallery visibility change invalidates cached icon");
}

class LayoutRequestCounter final : public QObject {
public:
  int count = 0;
protected:
  bool eventFilter(QObject *, QEvent *event) override {
    if (event->type() == QEvent::LayoutRequest)
      ++count;
    return false;
  }
};

void TestDiagnosticsPage() {
  using namespace remote::controller;
  DiagnosticsPage *page = nullptr;
  Measure("diagnostics_workspace_construct", [&] { page = new DiagnosticsPage; });
  page->resize(1100, 760);
  page->SetValue(QStringLiteral("deviceId"), QStringLiteral("device <test>"));
  auto *media = page->ValueLabel(QStringLiteral("encoderDetails"));
  Check(media != nullptr, "diagnostics media details label exists");
  const QString initialMedia = media ? media->text() : QString();
  page->SetValue(QStringLiteral("encoderDetails"), QStringLiteral("stale hidden text"));
  page->SetValue(QStringLiteral("encoderDetails"), QStringLiteral("latest <codec> details"), "warning");
  Check(media && media->text() == initialMedia,
        "hidden category snapshots update model without writing label");
  Measure("diagnostics_workspace_first_show_paint", [&] {
    page->show();
    FlushUi();
    QPixmap image(page->size());
    page->render(&image);
  });
  auto *device = page->ValueLabel(QStringLiteral("deviceId"));
  Check(device && device->textFormat() == Qt::PlainText &&
          device->text() == QStringLiteral("device <test>"),
        "diagnostics device names retain literal angle brackets");
  const QStringList categoryNames{QStringLiteral("设备与信令"), QStringLiteral("媒体能力"),
      QStringLiteral("连接质量"), QStringLiteral("房间状态"), QStringLiteral("成员连接"),
      QStringLiteral("鼠标与键盘"), QStringLiteral("远程粘贴"), QStringLiteral("最近错误"),
      QStringLiteral("场景识别性能"), QStringLiteral("场景优化状态")};
  int refreshes = 0;
  QObject::connect(page, &DiagnosticsPage::RefreshRequested, [&] { ++refreshes; });
  for (int i = 0; i < categoryNames.size(); ++i) {
    QPushButton *category = nullptr;
    for (auto *button : page->findChildren<QPushButton *>(QStringLiteral("settingsCategoryButton"))) {
      if (button->text() == categoryNames[i]) {
        category = button;
        break;
      }
    }
    const QByteArray timingName = QStringLiteral("diagnostics_category_%1_first_click_paint").arg(i).toLatin1();
    Measure(timingName.constData(), [&] {
      if (category)
        category->click();
      FlushUi();
      QPixmap image(page->size());
      page->render(&image);
    });
    Check(category && page->CurrentCategory() == i && refreshes == i + 1,
          "each category click selects page and requests fresh diagnostics");
    if (i == 1) {
      Check(media && media->text() == QStringLiteral("latest <codec> details") &&
              media->toolTip() == media->text() &&
              media->property("tone").toByteArray() == "warning" &&
              media->textFormat() == Qt::PlainText,
            "category opening applies latest cached text, tooltip and tone");
      LayoutRequestCounter counter;
      media->parentWidget()->installEventFilter(&counter);
      Measure("diagnostics_same_visible_value_1000", [&] {
        for (int n = 0; n < 1000; ++n)
          page->SetValue(QStringLiteral("encoderDetails"),
                         QStringLiteral("latest <codec> details"), "warning");
        FlushUi();
      });
      Check(counter.count == 0, "identical visible value avoids repeated layout requests");
      QString longValue;
      for (int line = 0; line < 80; ++line)
        longValue += QStringLiteral("codec line %1 <literal>\n").arg(line);
      page->SetValue(QStringLiteral("encoderDetails"), longValue, "warning");
      FlushUi();
      const QRect labelInContent(media->mapTo(page->widget(), QPoint()), media->size());
      Check(page->verticalScrollBar()->maximum() > 0 &&
              labelInContent.bottom() < page->widget()->height(),
            "long diagnostics details fit scroll content rather than clipped fixed stack");
      page->verticalScrollBar()->setValue(page->verticalScrollBar()->maximum());
      FlushUi();
      const int labelBottomInViewport = media->mapTo(page->viewport(), QPoint(0, media->height())).y();
      Check(labelBottomInViewport <= page->viewport()->height(),
            "bottom of long diagnostics value is reachable by scrolling");
      page->SetValue(QStringLiteral("encoderDetails"), QStringLiteral("short <codec>"), "normal");
      FlushUi();
    }
  }
  delete page;
}

QToolButton *FindToggle(QWidget &host, const QString &objectName,
                        const QString &text) {
  for (auto *button : host.findChildren<QToolButton *>(objectName)) {
    if (button->text() == text)
      return button;
  }
  return nullptr;
}

bool HasLabel(QWidget &host, const QString &objectName, const QString &text) {
  for (auto *label : host.findChildren<QLabel *>(objectName)) {
    if (label->text() == text)
      return true;
  }
  return false;
}

QVector<DiagnosticsSection> SampleSections() {
  QVector<DiagnosticsSection> sections;
  for (int s = 0; s < 6; ++s) {
    DiagnosticsSection section;
    section.key = s == 0 ? QStringLiteral("connection")
                         : QStringLiteral("section%1").arg(s);
    section.title = QStringLiteral("诊断分类 %1").arg(s);
    section.description = QStringLiteral("诊断采样窗口、媒体流水线与网络状态");
    for (int c = 0; c < 3; ++c) {
      DiagnosticsCard card;
      card.key = QStringLiteral("card%1").arg(c);
      card.title = QStringLiteral("指标卡片 %1-%2").arg(s).arg(c);
      card.subtitle = QStringLiteral("测试成员（非实际账号）");
      card.copyable = true;
      card.stackedMetrics = c == 1;
      for (int m = 0; m < 18; ++m) {
        card.chips.push_back({QStringLiteral("metric%1").arg(m),
                             QStringLiteral("网络指标 %1").arg(m),
                             QStringLiteral("采样值 %1 ms / 60 FPS").arg(m),
                             m == 2 ? QByteArray("good") : QByteArray("normal"),
                             m % 7 == 0});
      }
      section.cards.push_back(card);
    }
    sections.push_back(section);
  }
  return sections;
}

class HeightProbe final : public QWidget {
public:
  mutable int hintCalls = 0;
  mutable int hfwCalls = 0;
  QSize sizeHint() const override {
    ++hintCalls;
    return QSize(700, 300);
  }
  QSize minimumSizeHint() const override { return QSize(100, 100); }
  bool hasHeightForWidth() const override { return true; }
  int heightForWidth(int width) const override {
    ++hfwCalls;
    return 100 + 70000 / qMax(1, width);
  }
};

void TestStack(bool baseline) {
  CurrentPageStack stack;
  stack.resize(900, 600);
  auto *active = new HeightProbe;
  auto *hidden = new HeightProbe;
  stack.addWidget(active);
  stack.addWidget(hidden);
  stack.setCurrentWidget(active);
  stack.show();
  FlushUi();
  active->hintCalls = hidden->hintCalls = 0;
  active->hfwCalls = hidden->hfwCalls = 0;
  Measure("stack_hint_hfw_100", [&] {
    for (int i = 0; i < 100; ++i) {
      stack.sizeHint();
      stack.minimumSizeHint();
      stack.heightForWidth(600 + i);
    }
  });
  QTextStream(stdout) << "COUNTS hidden_hints=" << hidden->hintCalls
                      << " hidden_hfw=" << hidden->hfwCalls << '\n';
  if (!baseline)
    Check(hidden->hintCalls == 0 && hidden->hfwCalls == 0,
          "stack sizing never measures a hidden page");
  const int narrow = stack.heightForWidth(300);
  const int wide = stack.heightForWidth(1000);
  Check(narrow > wide, "current page height-for-width follows requested width");
  stack.setCurrentWidget(hidden);
  Check(stack.sizeHint() == hidden->sizeHint(),
        "stack size hint follows newly selected page");
  stack.RefreshCurrentHeight();
  Check(stack.minimumHeight() == stack.maximumHeight(),
        "room stack retains explicit fixed-height tracking");
  CurrentPageStack flowingStack(nullptr, false);
  flowingStack.addWidget(new HeightProbe);
  flowingStack.resize(500, 200);
  flowingStack.show();
  FlushUi();
  Check(flowingStack.minimumHeight() < flowingStack.maximumHeight(),
        "diagnostics stack does not freeze its height on resize");

  QWidget host;
  auto *hostLayout = new QVBoxLayout(&host);
  auto *nestedStack = new CurrentPageStack(&host, false);
  auto *nestedActive = new HeightProbe;
  auto *nestedHidden = new HeightProbe;
  nestedStack->addWidget(nestedActive);
  nestedStack->addWidget(nestedHidden);
  hostLayout->addItem(new remote::controller::detail::CurrentPageStackItem(nestedStack));
  host.resize(900, 600);
  host.show();
  FlushUi();
  nestedHidden->hfwCalls = 0;
  Measure("nested_stack_layout_hfw_100", [&] {
    for (int width = 600; width < 700; ++width)
      hostLayout->heightForWidth(width);
  });
  Check(nestedHidden->hfwCalls == 0,
        "actual outer layout never bypasses current-page height calculation");
  nestedStack->setCurrentWidget(nestedHidden);
  hostLayout->heightForWidth(705);
  Check(nestedHidden->hfwCalls > 0,
        "outer layout follows the newly selected category");
}

void TestCards(bool baseline) {
  QScrollArea scroll;
  scroll.resize(1100, 760);
  scroll.setWidgetResizable(true);
  DiagnosticsCardsWidget *cards = nullptr;
  auto sections = SampleSections();
  Measure("cold_construct_and_snapshot_324_metrics", [&] {
    cards = new DiagnosticsCardsWidget;
    scroll.setWidget(cards);
    cards->SetSections(sections, QStringLiteral("暂无数据"));
  });
  QTextStream(stdout) << "COUNTS cold_children="
                      << cards->findChildren<QWidget *>().size() << '\n';
  if (!baseline) {
    Check(cards->findChildren<QWidget *>().size() < 300,
          "cold snapshot does not construct collapsed section metrics");
  }
  Measure("first_show_layout_paint", [&] {
    scroll.show();
    FlushUi();
    QPixmap image(scroll.size());
    scroll.render(&image);
  });
  const auto initialSize = cards->sizeHint();
  Measure("unchanged_snapshot_30", [&] {
    for (int i = 0; i < 30; ++i)
      cards->SetSections(sections, QStringLiteral("暂无数据"));
    FlushUi();
  });
  Check(cards->sizeHint() == initialSize,
        "unchanged snapshots preserve page layout");
  Measure("changed_values_30", [&] {
    for (int i = 0; i < 30; ++i) {
      sections[0].cards[0].chips[0].value = QStringLiteral("fresh %1").arg(i);
      cards->SetSections(sections, QStringLiteral("暂无数据"));
    }
    FlushUi();
  });
  Check(HasLabel(*cards, QStringLiteral("statsChipValue"), QStringLiteral("fresh 29")),
        "visible metrics display latest sample");
  sections[0].cards[0].chips[0].tone = "warning";
  cards->SetSections(sections, QStringLiteral("暂无数据"));
  bool foundUpdatedTone = false;
  for (auto *label : cards->findChildren<QLabel *>(QStringLiteral("statsChipValue"))) {
    if (label->text() == QStringLiteral("fresh 29")) {
      foundUpdatedTone = label->parentWidget()->property("tone").toByteArray() == "warning";
      break;
    }
  }
  Check(foundUpdatedTone, "visible tone changes are not suppressed by refresh caching");
  auto *firstCard = FindToggle(*cards, QStringLiteral("statsCardToggle"),
                               sections[0].cards[0].title);
  Check(firstCard != nullptr, "expanded section contains card toggle");
  if (firstCard)
    firstCard->setChecked(false);

  sections[0].cards[0].chips[0].value = QStringLiteral("latest collapsed card");

  sections[1].cards[0].chips[0].value = QStringLiteral("latest hidden sample");
  cards->SetSections(sections, QStringLiteral("暂无数据"));
  auto *hiddenSection = FindToggle(*cards, QStringLiteral("statsSectionToggle"),
                                   sections[1].title);
  Check(hiddenSection != nullptr, "collapsed section toggle remains available");
  Measure("first_expand_hidden_section", [&] {
    if (hiddenSection)
      hiddenSection->setChecked(true);
    FlushUi();
  });
  Check(HasLabel(*cards, QStringLiteral("statsChipValue"),
                 QStringLiteral("latest hidden sample")),
        "first expansion uses latest hidden-section sample");
  auto *copy = cards->findChild<QPushButton *>(QStringLiteral("softButton"));
  if (copy)
    copy->click();
  Check(copy && QApplication::clipboard()->text().contains(QStringLiteral("latest collapsed card")),
        "copy includes latest metrics even while card is collapsed");

  sections[0].cards[0].chips.push_back({QStringLiteral("added"),
      QStringLiteral("新指标"), QStringLiteral("结构更新后最新值")});
  Measure("structure_change", [&] {
    cards->SetSections(sections, QStringLiteral("暂无数据"));
    FlushUi();
  });
  firstCard = FindToggle(*cards, QStringLiteral("statsCardToggle"),
                          sections[0].cards[0].title);
  hiddenSection = FindToggle(*cards, QStringLiteral("statsSectionToggle"),
                              sections[1].title);
  Check(firstCard && !firstCard->isChecked() && hiddenSection && hiddenSection->isChecked(),
        "structure rebuild preserves user card/section expansion state");
  if (firstCard)
    firstCard->setChecked(true);
  Check(HasLabel(*cards, QStringLiteral("statsChipValue"),
                 QStringLiteral("latest collapsed card")),
        "collapsed card receives latest model before reopening");
  Check(HasLabel(*cards, QStringLiteral("statsChipValue"),
                 QStringLiteral("结构更新后最新值")),
        "new metric appears after expanding rebuilt card");

  cards->SetSections({}, QStringLiteral("empty first"));
  cards->SetSections({}, QStringLiteral("empty updated"));
  if (!baseline)
    Check(HasLabel(*cards, QStringLiteral("statsEmptyText"), QStringLiteral("empty updated")),
          "empty-state text updates without structure change");
}
} // namespace

int main(int argc, char **argv) {
  QApplication application(argc, argv);
  // Tests own a disposable settings store and never read/write RLink user preferences.
  QTemporaryDir settingsDirectory;
  if (!settingsDirectory.isValid())
    return 2;
  QCoreApplication::setOrganizationName(QStringLiteral("RLinkUiRegressionTest"));
  QCoreApplication::setApplicationName(QStringLiteral("DiagnosticsUiSelfTest"));
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDirectory.path());
  QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, settingsDirectory.path());
  QSettings().setValue(QStringLiteral("ui/animationLevel"), 0);
  const QStringList args = application.arguments();
  const bool baseline = args.contains(QStringLiteral("--baseline"));
  if (!baseline)
    TestUiConfigurationCache();
  const QString theme = args.contains(QStringLiteral("--dark"))
                            ? QStringLiteral("dark") : QStringLiteral("light");
  QString style;
  for (const QString &name : {QStringLiteral("base"), theme}) {
    QFile file(QStringLiteral(RLINK_UI_TEST_SOURCE_DIR) +
               QStringLiteral("/assets/ui/theme/") + name + QStringLiteral(".qss"));
    if (!file.open(QIODevice::ReadOnly)) {
      QTextStream(stderr) << "FAIL cannot read test stylesheet\n";
      return 2;
    }
    style += QString::fromUtf8(file.readAll());
  }
  application.setStyleSheet(style);
  remote::controller::ui::RemoteCTheme::SavePreference(theme == QStringLiteral("dark")
      ? remote::controller::ui::ThemePreference::kDark
      : remote::controller::ui::ThemePreference::kLight);
  QTextStream(stdout) << "THEME " << theme << " PLATFORM "
                      << application.platformName() << '\n';
  TestCards(baseline);
  TestStack(baseline);
  if (!baseline) {
    TestRecentDevicesRefreshState();
    TestDiagnosticsPage();
    TestRecentConnections();
    TestMediaControls();
  }
  QTextStream(stdout) << "RESULT " << failures << " failures\n";
  return failures == 0 ? 0 : 1;
}
