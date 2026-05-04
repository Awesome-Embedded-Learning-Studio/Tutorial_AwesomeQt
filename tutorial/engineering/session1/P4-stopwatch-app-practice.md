# 实战练习 · StopwatchApp — 从零开始做你的第一个完整应用

## 前言：从控件到应用的跨越

前面三篇我们练的都是"零件"——一个 LED、一个搜索框、一个表单布局。它们教会了你自绘、组合、布局这三种基本手法。但零件归零件，真正让人有成就感的是把这些零件组装成一个**完整可用的程序**。秒表就是这样一个最简单的"完整应用"：它有启动/停止的状态切换，有持续更新的显示，有圈数记录，有重置功能。麻雀虽小，五脏俱全——它具备了几乎所有应用都会有的核心要素：状态管理、定时刷新、用户交互、数据展示。

而且秒表有个额外的好处：它只需要 QTimer 和 QLabel 这两个最基础的类就能做出来，不需要学新的 API。你之前在定时器那篇讲座里学的东西，这里终于派上用场了。

---

## 出发前的装备清单

- **QTimer** — 周期性触发 timeout() 信号，用来驱动显示更新。如果你还没读过定时器那一篇，这是唯一的前置要求，建议先去补一下。
- **QElapsedTimer** — 高精度计时器，用来精确测量经过的时间。和 QTimer 的区别：QTimer 是"到时间了告诉我"，QElapsedTimer 是"告诉我过了多久"。秒表需要的是后者。
- **QLabel** — 显示时间文本。我们可以给它设一个大的等宽字体让它看起来更像数字显示器。
- **QPushButton / QListWidget** — 按钮和列表控件，用来做"开始/停止"、"圈"、"重置"按钮和圈数记录。

---

## 我们的目标长什么样

```
┌──────────────────────────────────┐
│                                  │
│           00:00.00               │
│                                  │
│   [ 开始 ]   [ 圈 ]   [ 重置 ]   │
│                                  │
│  圈 1     00:03.24               │
│  圈 2     00:07.81               │
│  圈 3     00:12.05               │
│                                  │
└──────────────────────────────────┘
```

中间一个大号时间显示，下面三个按钮，再下面是圈数列表。开始按钮点击后变成"停止"，停止后再点又变成"开始"。点"圈"记录当前时间。点"重置"清零一切。

完成标准：时间显示精确到百分之一秒（MM:SS.cc 格式），开始/停止切换正确，圈数时间记录到列表，重置清空所有状态。

---

## 第一步 — 主窗口结构：布局先行

### 思考题

秒表的界面结构很清晰：上面是时间显示，中间是按钮行，下面是列表。你觉得用什么布局最合适？提示：三个部分垂直排列，按钮行内部水平排列。QVBoxLayout 套 QHBoxLayout 是不是就够了？

### 动手写

先搭窗口类骨架。这次我们用 QWidget 而不是 QMainWindow，因为秒表不需要菜单栏、工具栏、状态栏这些重型结构——一个简单的 QWidget 就够了。

```cpp
#pragma once

#include <QElapsedTimer>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTimer>
#include <QWidget>

class StopwatchApp : public QWidget
{
    Q_OBJECT

public:
    explicit StopwatchApp(QWidget *parent = nullptr);

private slots:
    void onStartStop();
    void onLap();
    void onReset();
    void onTimeout();

private:
    void setupUi();
    void updateDisplay();

    // 显示和时间
    QLabel *m_timeLabel;

    // 按钮
    QPushButton *m_startStopBtn;
    QPushButton *m_lapBtn;
    QPushButton *m_resetBtn;

    // 圈数列表
    QListWidget *m_lapList;

    // 计时相关
    QTimer *m_updateTimer;
    QElapsedTimer m_elapsedTimer;

    // TODO: 你还需要哪些状态变量？
    //       提示：需要记录"上次停表时的累计时间"，
    //       因为 QElapsedTimer 每次 start() 都从零开始。
    //       如果用户 start → 跑了 3 秒 → stop → 再 start，
    //       显示应该从 3 秒继续而不是从 0 开始。
};
```

关于 `m_elapsedTimer`，这里有个设计上的要点需要提前想清楚。QElapsedTimer 的 `start()` 方法会重置计时器从零开始，`elapsed()` 返回的是自上次 start() 以来经过的毫秒数。但秒表需要支持"暂停后继续"——用户可能 start，跑几秒，stop，然后再 start，时间应该接着上次停的地方继续走。这就意味着你不能只用 elapsed() 的值，还得维护一个"累计已经过时间"的变量。

### 你可能会遇到的坑

QElapsedTimer 没有 stop 或 pause 方法。它就是一个纯粹的计时器——start 之后就一直在走。所以"暂停"的逻辑需要我们自己实现：记录 stop 时的 elapsed() 值，加到一个累计变量里。下次 start 时，elapsed() 又从零开始，但显示时间 = 累计值 + 当前 elapsed()。

### 检查点

编译运行。你应该能看到一个窗口，上面有一个 "00:00.00" 的标签，三个按钮，一个空列表。按钮点了没反应是正常的——我们还没写槽函数。

---

## 第二步 — 时间显示：大字体、等宽、居中

### 思考题

时间显示用 QLabel 显示一个形如 "00:03.24" 的字符串。我们需要多频繁地更新这个显示？如果精度是百分之一秒（10ms），那 QTimer 的间隔是不是应该设成 10ms？这个间隔会不会太频繁导致性能问题？提示：10ms 对现代 CPU 来说完全不算什么，但你可以先设成 30ms 或 50ms 试试，感受一下更新频率对视觉流畅度的影响。

### 动手写

给 QLabel 设一个大的等宽字体。为什么一定要等宽？因为如果用比例字体（比如默认字体），数字从 "00:00.00" 变成 "00:00.10" 时字符宽度不同，整个标签会发生微小的宽度跳动，看起来很不舒服。等宽字体的每个字符宽度一样，就不会有这个问题。

```cpp
// 在 setupUi 里创建 QLabel：
// TODO: 创建 m_timeLabel，设置初始文本 "00:00.00"
// TODO: 设置字体——QFont font("Monospace", 36, QFont::Bold);
//       或者用 QFontDatabase::systemFont(QFontDatabase::FixedFont)
//       然后设大号、粗体
// TODO: 设置对齐方式为居中 Qt::AlignCenter
```

然后实现 `updateDisplay()` 方法。这个方法会读取当前经过的时间，格式化为 MM:SS.cc 字符串，设置到 QLabel 上：

```cpp
void StopwatchApp::updateDisplay()
{
    // TODO: 计算总经过时间（毫秒）
    //       = m_accumulatedTime + m_elapsedTimer.elapsed()
    //       注意：只有在计时器运行时才加 elapsed()，
    //       停止状态下直接用 m_accumulatedTime。

    // TODO: 把毫秒转换成分:秒.百分之一秒
    //       totalMs / 60000 得到分钟
    //       (totalMs % 60000) / 1000 得到秒
    //       (totalMs % 1000) / 10 得到百分之一秒

    // TODO: 用 QString::asprintf("%02d:%02d.%02d", ...) 格式化
    //       或者用 QString::arg()，注意补零对齐

    // TODO: m_timeLabel->setText(formatted);
}
```

关于时间格式化，`QString::asprintf` 是最直观的方式，类似 C 的 sprintf。但 Qt 风格更推荐用 `QString::arg()`，它的类型安全更好。选哪种都行，只要格式正确——特别是分钟和秒要补零到两位，百分之一秒也要两位。

### 检查点

在 main.cpp 里创建 StopwatchApp 并 show。你应该能看到一个窗口，中间有 "00:00.00" 大字显示。字体应该是等宽的、居中的。

---

## 第三步 — 开始和停止：状态切换

### 思考题

"开始/停止"其实是同一个按钮，只是在不同状态下显示不同的文字。我们需要一个 bool 变量来跟踪当前是"运行中"还是"已停止"吗？还是可以通过 QTimer::isActive() 来判断？两种方式哪种更清晰？

### 动手写

实现 `onStartStop()` 槽函数。这是整个秒表的核心逻辑：

```cpp
void StopwatchApp::onStartStop()
{
    // TODO: 判断当前状态（运行中 or 已停止）
    //
    // 如果当前已停止 → 开始：
    //   1. m_elapsedTimer.start()    ← QElapsedTimer 从零开始计时
    //   2. m_updateTimer->start(10)  ← QTimer 每 10ms 触发一次 timeout
    //   3. 按钮文字改成 "停止"
    //
    // 如果当前运行中 → 停止：
    //   1. m_accumulatedTime += m_elapsedTimer.elapsed()  ← 记住已经过的时间
    //   2. m_updateTimer->stop()
    //   3. 按钮文字改回 "开始"
}
```

这里最关键的一行是 `m_accumulatedTime += m_elapsedTimer.elapsed()`。这行代码把当前这段计时器跑过的时间累加到总数里。下次再 start 时，QElapsedTimer 从零重新开始，但 m_accumulatedTime 里已经存了之前跑的总时间，所以显示 = 累计 + 当前。

`onTimeout()` 是 QTimer 每 10ms 触发一次的槽函数，它的逻辑就一行——调用 `updateDisplay()`：

```cpp
void StopwatchApp::onTimeout()
{
    updateDisplay();
}
```

### 你可能会遇到的坑

如果你不用 `m_accumulatedTime` 累计时间，而是每次 start 都让 QElapsedTimer 从零开始、只用 elapsed() 显示，那暂停后再继续就会从零开始计时。这是初学者最常犯的逻辑错误——"QElapsedTimer 每次 start 都重置"这个行为不符合直觉，但它确实就是这样设计的。

### 检查点

点"开始"，时间开始走动。点"停止"，时间停住。再点"开始"，时间从停住的地方继续走而不是从零开始。按钮文字在"开始"和"停止"之间正确切换。

---

## 第四步 — 重置：清空一切

### 动手写

```cpp
void StopwatchApp::onReset()
{
    // TODO: 停止计时器
    //       m_updateTimer->stop()

    // TODO: 重置累计时间
    //       m_accumulatedTime = 0

    // TODO: 更新显示为 "00:00.00"

    // TODO: 清空圈数列表
    //       m_lapList->clear()

    // TODO: 恢复按钮状态
    //       m_startStopBtn->setText("开始")
}
```

没什么花哨的，就是归零。但注意一个顺序问题：如果你先清显示再停计时器，可能有一帧的闪烁。虽然 10ms 的闪烁人眼几乎看不到，但养成好习惯，先停再清。

### 检查点

跑一段时间后点重置。显示回到 00:00.00，圈列表清空，按钮恢复为"开始"。再次点开始，从零开始计时。

---

## 第五步 — 圈功能：记录分段时间

### 思考题

圈时间有两种显示方式：一种是"从上次圈到这次的间隔时间"（分段时间），另一种是"从开始到现在的总时间"（累计时间）。你觉得应该显示哪种？或者两种都显示？手机上的秒表 App 通常怎么做？

### 动手写

```cpp
void StopwatchApp::onLap()
{
    // TODO: 只有在计时器运行时才能记圈
    //       如果没在跑就直接 return

    // TODO: 计算当前总时间
    //       和 updateDisplay 一样的计算方式

    // TODO: 格式化圈数文本
    //       "圈 N     MM:SS.cc"
    //       N 是 m_lapList->count() + 1

    // TODO: 添加到 QListWidget
    //       new QListWidgetItem(text, m_lapList)
    //       或者 m_lapList->addItem(text)

    // TODO: 自动滚动到最新一条
    //       m_lapList->scrollToBottom()
}
```

### 检查点

开始计时后点几次"圈"按钮。每次点击列表里应该多一条记录，显示当前的圈数和时间。最新的记录自动滚动到底部可见。停止后再点"圈"没反应（因为计时器没在跑）。

---

## 最终组装

main.cpp 非常简单：

```cpp
#include <QApplication>
#include "stopwatch_app.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    StopwatchApp stopwatch;
    stopwatch.resize(320, 400);
    stopwatch.show();

    return app.exec();
}
```

CMakeLists.txt 需要链接 `Qt6::Core`、`Qt6::Gui`、`Qt6::Widgets`，开启 AUTOMOC。

---

## 验收标准

开始/停止按钮切换正常，停止后再开始从停住的地方继续计时。时间显示格式为 MM:SS.cc，数字不跳动（等宽字体）。圈功能在运行时可用，每次点圈记录一条带编号和时间的数据。重置清空一切回到初始状态。连续快速点击开始/停止不会导致状态错乱或计时器叠加。

---

## 进阶挑战

给开始/停止按钮不同状态设置不同颜色——运行时红色（提醒你可以停了），停止时绿色。或者加键盘快捷键：空格键 = 开始/停止，L 键 = 记圈，R 键 = 重置——查一下 keyPressEvent。再或者，把圈时间导出到文件——用 QTextStream 写到一个 .txt 文件里。

---

## 踩坑预防清单

> **坑 #1：不用 QElapsedTimer 而用计数器**
> 如果你用 "QTimer 每 10ms 触发一次，维护一个 counter++" 的方式计时，长时间运行后误差会累积——因为 QTimer 的实际触发间隔不是精确的 10ms，可能是 11ms、12ms 甚至 15ms，跑一分钟就差好几秒。QElapsedTimer 用的是系统高精度计时器，误差在微秒级。

> **坑 #2：忘记累计暂停时间**
> QElapsedTimer::start() 会重置。如果你不维护 m_accumulatedTime，暂停后继续就会从零开始。记住：显示时间 = 累计历史时间 + 当前 elapsed()。

> **坑 #3：快速连续点击导致状态错乱**
> 如果用户快速连续点击开始/停止，可能出现多次 start() 或 stop() 叠加的问题。解决办法：在 onStartStop 开头检查当前状态，如果已经在目标状态就 return。

---

## 官方文档参考

- [QTimer Class](https://doc.qt.io/qt-6/qtimer.html) — 周期性定时器
- [QElapsedTimer Class](https://doc.qt.io/qt-6/qelapseedtimer.html) — 高精度经过时间测量
- [QListWidget Class](https://doc.qt.io/qt-6/qlistwidget.html) — 列表控件

到这里就大功告成了。如果你一路做到了这里，恭喜——你已经从零写出了一个完整可用的应用程序。虽然它很简单，但它具备了几乎所有应用都需要的核心要素：状态管理、定时刷新、用户交互。接下来我们回到控件方向，练一个稍微复杂点的：自定义确认对话框。
