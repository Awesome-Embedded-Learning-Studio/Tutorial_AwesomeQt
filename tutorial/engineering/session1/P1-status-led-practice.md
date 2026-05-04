# 实战练习 · StatusLed — 从 paintEvent 画出你的第一个自定义控件

## 前言：为什么要造这个轮子

说实话，Qt 自带的控件库虽然种类不少，但真正到了工程里你总会发现缺东西。一个最典型的场景就是状态指示灯——设备在线了亮绿灯、离线了亮红灯、告警了黄灯闪烁——这种小东西 Qt 没有直接给你。你可能会说 QLabel 换个背景色不就完了？确实能凑合，但那个效果距离"工业级"的指示灯差得远：没有发光感、没有渐变、圆角不好控制、闪烁逻辑到处散落。所以我们干脆自己动手，从最简单的一个 LED 指示灯开始，把 Qt 自绘控件的整个流程走通。

这个练习我们会碰到自定义控件最核心的几个概念：继承 QWidget、重写 paintEvent、用 QPainter 画图、用 QRadialGradient 做发光效果、用 QTimer 做闪烁。这些知识点在你日后写任何自定义控件时都会反复用到，所以这个最不起眼的小灯泡，其实是我们整个自绘之旅的起点。

---

## 出发前的装备清单

开始之前，你需要对以下几个 Qt 类有基本的了解。不需要精通，知道它们干什么就行——如果你之前跟过我们的讲座教程，大部分应该已经眼熟了。

- **QWidget** — 所有可见控件的基类，我们要继承它来造自己的控件。如果你不确定 QWidget 的构造函数里那个 `parent` 参数是干嘛的，回去翻一下 QObject 的父子关系那一节。
- **QPainter** — Qt 的 2D 绘图引擎，可以在任何 QPaintDevice（QWidget 就是其中之一）上画线、画圆、画文字、画渐变。它的工作方式有点像你拿一支笔在画布上画：先 `begin()`，然后一笔一笔画，最后 `end()`。
- **QTimer** — 周期性触发 timeout() 信号的定时器，我们用它来让 LED 闪烁。如果你还没读过定时器那一篇，建议先去补一下，不长。
- **QRadialGradient** — 径向渐变，从一个中心点向外辐射颜色变化。这个东西就是让我们的 LED 看起来"发光"的关键，而不是一个扁平的色块。

---

## 我们的目标长什么样

我们要做一个大小约 20×20 像素的圆形指示灯，支持四种状态（绿色表示正常、红色表示错误、黄色表示警告、灰色表示离线），外加一个闪烁模式。效果大概是这样：

```
  ● 正常（绿）    ● 错误（红）    ● 警告（黄）    ● 离线（灰）
       ●●●●●          ●●●●●          ●●●●●          ●●●●●
     ●●●●●●●●●     ●●●●●●●●●     ●●●●●●●●●     ●●●●●●●●●
    ●●●●●●●●●●●   ●●●●●●●●●●●   ●●●●●●●●●●●   ●●●●●●●●●●●
    ●●●●●●●●●●●   ●●●●●●●●●●●   ●●●●●●●●●●●   ●●●●●●●●●●●
     ●●●●●●●●●     ●●●●●●●●●     ●●●●●●●●●     ●●●●●●●●●
       ●●●●●          ●●●●●          ●●●●●          ●●●●●
```

当然 ASCII 画不出渐变效果，真正的 LED 会有一个从中心亮到边缘暗的径向渐变，看起来像一颗真正在发光的小灯珠。

完成标准：运行程序后能看到四个不同颜色的 LED 排成一排，点击一个按钮可以让任意一个 LED 进入闪烁模式（亮/灭交替）。

---

## 第一步 — 打地基：把类结构搭起来

### 思考题

在动手之前，先想一个问题：我们的 StatusLed 应该继承自 QWidget，而不是 QLabel 或者 QPushButton。你觉得为什么？QLabel 也能显示颜色，为什么不直接用它？提示：想想 `paintEvent` 这个东西意味着什么程度的自由度。

### 动手写

现在我们来建一个名叫 `StatusLed` 的类。头文件的基本结构大概长这样——注意，我只给骨架，具体实现需要你自己填。每一处标了 `// TODO` 的地方，就是你需要思考和补全的：

```cpp
#pragma once

#include <QWidget>

class StatusLed : public QWidget
{
    Q_OBJECT

public:
    // TODO: 你觉得需要哪些状态？绿色、红色、黄色、灰色……
    //       用什么来表示这些状态？枚举是个好选择。
    //       试着声明一个 enum，放在 public 区域里。

    // TODO: 构造函数。QWidget 的子类构造函数通常需要一个 parent 参数。
    //       你需要初始化哪些东西？提示：初始状态、控件大小。

    // TODO: 你需要提供哪些公开方法让外部控制 LED？
    //       比如：设置状态、获取当前状态、开启/关闭闪烁。
    //       想想函数签名应该长什么样。

    // QWidget 有一个虚函数叫 sizeHint()，它告诉布局管理器
    // "我这个控件希望多大"。你需要重写它。
    QSize sizeHint() const override;

protected:
    // 这是最关键的一个函数重写——所有自定义绘制都在这里完成。
    // QWidget 的 paintEvent 在控件需要重绘时被 Qt 的事件循环调用。
    void paintEvent(QPaintEvent *event) override;

private:
    // TODO: 你需要哪些成员变量来存储当前状态？
    //       当前 LED 的颜色/状态是必须存的。
    //       闪烁相关的状态呢？
};
```

这里有一个东西你必须理解：`Q_OBJECT` 宏。这个东西展开之后会给你的类注入信号槽机制、元对象信息等一堆好东西。只要你的类继承了 QObject 并且要用信号槽或者 `qobject_cast`，就**必须**加这个宏。不加的话编译能过，但运行时信号槽会静默失败，调试到怀疑人生。别问我怎么知道的。

### 你可能会遇到的坑

如果你在类声明里写了 `Q_OBJECT` 但忘了让 CMake 开启 `AUTOMOC`，链接阶段会报一堆 `undefined reference to vtable` 的错误。这是因为 Qt 的 MOC（Meta-Object Compiler）需要处理 `Q_OBJECT` 宏生成额外的 C++ 代码。解决办法很简单，CMakeLists.txt 里加上 `set(CMAKE_AUTOMOC ON)` 就行了。

### 检查点

到这里你还不需要编译——我们只是搭了个空壳。但你可以试一下：在 main.cpp 里 `#include` 这个头文件，创建一个 StatusLed 对象并 show 出来。如果看到一个空白的小窗口，说明类结构没问题，地基打好了。

---

## 第二步 — 画一个圆：让 paintEvent 干活

### 思考题

`paintEvent` 里，我们要用 QPainter 画一个圆。Qt 画圆的方法是 `drawEllipse`，但这里有个问题：你是直接在 widget 的 (0, 0) 坐标画，还是需要做一些坐标计算？如果控件被 resize 成了 30×20（不是正方形），你画出来的圆会不会变成椭圆？想想怎么保证它始终是个正圆。

### 动手写

现在来实现 `paintEvent`。核心思路很简单：创建一个 QPainter，设置好画笔和画刷，然后画一个椭圆。关键是坐标系和尺寸的计算。

```cpp
void StatusLed::paintEvent(QPaintEvent * /*event*/)
{
    // TODO: 创建 QPainter。它有两种用法：
    //       1. QPainter painter(this); — 构造时传入 widget 指针，自动 begin/end
    //       2. 手动调用 begin() 和 end()
    //       推荐第一种，更安全。

    // TODO: 开启抗锯齿，不然画出来的圆会有锯齿。
    //       调用 painter.setRenderHint(QPainter::Antialiasing);

    // TODO: 计算绘制区域。
    //       width() 和 height() 给你控件的实际尺寸。
    //       取较小值的一半作为半径，保证是正圆。
    //       计算圆心坐标。

    // TODO: 用 QRadialGradient 创建渐变画刷。
    //       QRadialGradient 的构造函数需要中心和半径。
    //       你需要用 setColorAt() 设置至少两个色标：
    //       - 中心点 (0.0): 较亮的颜色
    //       - 边缘 (1.0): 较暗的颜色或透明
    //       这样才能产生"发光"的效果。

    // TODO: 设置画刷（QBrush）为你的渐变。
    //       可以设置一下画笔（QPen）为 Qt::NoPen，去掉边框线。

    // TODO: 调用 drawEllipse 画圆。
    //       参数可以是 QRectF 或者中心+半径的形式。
}
```

关于 QRadialGradient，多说两句。这个类的 `setColorAt(qreal position, QColor)` 方法用来定义渐变中某个位置的颜色。position 从 0.0（中心）到 1.0（边缘）。比如你想让中心亮白、中间是绿色、边缘暗绿，就设三个色标。建议你先查一下 Qt 文档里 QRadialGradient 的图示，理解了渐变原理再动手写，比盲猜参数快得多。

颜色怎么选？这里有个实用的技巧：中心用"加亮版"（比如绿色状态用亮绿 `#66ff66`），边缘用"正常版"或"暗版"（`#00aa00`），这样从中心到边缘会有一个从亮到暗的过渡，看起来就像真的在发光。

### 你可能会遇到的坑

一个非常经典的错误：在 `paintEvent` 里 `new` 了一个 QPainter 但忘记调用 `end()`，或者用栈上的 QPainter 但作用域没控制好。第一种写法（构造时传入 `this`）会自动处理这个问题，所以我强烈推荐。另外，`paintEvent` 里**绝对不能**调用 `update()` 或 `repaint()`，否则会无限递归，程序直接卡死——这个坑坑了我整整一个下午。

### 检查点

编译运行。你应该能看到一个带渐变效果的圆形 LED。颜色可以先写死成绿色——能画出来就说明 QPainter 的基本用法你掌握了。如果看到的是方形或者什么都没有，检查一下坐标计算和 drawEllipse 的参数。

---

## 第三步 — 让它变色：状态管理

### 思考题

现在 LED 只有一种颜色。我们要让它支持多种状态，每个状态对应不同的颜色。想一想：你打算怎么存储"状态→颜色"的映射关系？`switch-case`？`QMap`？还是直接在枚举里用位运算编码颜色？每种方案各有什么优劣？

### 动手写

我们回到头文件，把第一步留下的枚举和成员变量补上：

```cpp
// 在 public 区域声明枚举
enum class Status {
    // TODO: 定义你的状态。至少四种：正常、错误、警告、离线。
};

// 公开方法
void setStatus(Status status);
Status status() const;
```

然后实现 `setStatus`。这个函数要做两件事：记录新状态，然后触发重绘。触发重绘用的是 `update()`——这个函数不会立即重绘，而是告诉 Qt "我这个控件脏了，下次事件循环的时候帮我重绘一下"。这也是为什么它不会导致无限递归的原因，它只是往事件队列里投了一个"请重绘"的请求。

```cpp
void StatusLed::setStatus(Status status)
{
    // TODO: 检查新状态和旧状态是否相同，相同就不用折腾了。
    //       存储新状态到成员变量。
    //       调用 update() 请求重绘。
}
```

接下来修改 `paintEvent`，让它根据当前状态选择不同的渐变颜色。你可以写一个私有辅助函数，比如 `currentColor()`，根据 `m_status` 返回对应的 QColor。然后在 paintEvent 里用它来构建 QRadialGradient 的色标。

### 你可能会遇到的坑

如果你在 setStatus 里调用了 `repaint()` 而不是 `update()`，在某些情况下会导致闪烁或者性能问题。`repaint()` 是强制立即重绘，跳过了 Qt 的优化调度。绝大多数场景下 `update()` 才是正确的选择，除非你明确知道自己在做什么。

### 检查点

在 main.cpp 里创建一个 StatusLed，用代码分别设置四种状态，每次设置后你能看到 LED 变色。如果你嫌每次改代码重编译太麻烦，可以放一个 QPushButton，每次点击切换下一个状态——顺便练一下信号槽。

---

## 第四步 — 让它闪烁：QTimer 上场

### 思考题

闪烁效果的本质是什么？就是"亮 → 灭 → 亮 → 灭"的循环。这里有两种实现思路：第一种是用一个 bool 变量记录当前是亮还是灭，QTimer 每次 timeout 就 flip 一下这个变量然后 update()；第二种是直接 hide()/show() 整个控件。你觉得哪种更合理？为什么？提示：hide() 之后 QTimer 还会继续触发吗？如果这个 LED 控件在 hide 状态下，外部的布局会怎样？

### 动手写

在头文件里加上闪烁相关的成员和方法：

```cpp
// 公开方法
void setBlinking(bool enabled);
bool isBlinking() const;

// 私有成员
// TODO: 一个 QTimer* 用于闪烁定时
// TODO: 一个 bool 用于记录当前是否处于"可见"半周期
```

实现思路是这样的：`setBlinking(true)` 时启动一个间隔约 500ms 的 QTimer，每次 timeout 翻转"可见"标志，然后 update()。在 paintEvent 里，如果处于"不可见"半周期，就把整体透明度降到一个很低的值（或者干脆画一个更暗的圆）。`setBlinking(false)` 时停止定时器并恢复到正常显示。

```cpp
void StatusLed::setBlinking(bool enabled)
{
    // TODO: 如果 enabled 为 true 且定时器还没在跑：
    //       重置可见标志为 true
    //       启动定时器，间隔大约 400-600ms
    //
    //       如果 enabled 为 false：
    //       停止定时器
    //       恢复可见标志为 true
    //       调用 update() 确保恢复正常显示
}
```

QTimer 的 parent 一定要设成 this（也就是 StatusLed 自己）。这样一来，当 StatusLed 被销毁时，QTimer 会自动跟着被销毁，不会出现定时器在控件死后还触发的灵异事件。

### 你可能会遇到的坑

如果你在 setBlinking 里每次都 `new QTimer(this)` 而不是复用同一个 QTimer 对象，连续调用 setBlinking(true) 多次就会创建多个同时运行的定时器，LED 会以一种完全不可控的节奏狂闪。正确做法是在构造函数里创建一次 QTimer，之后只调用 start()/stop()。

### 检查点

让一个 LED 进入闪烁模式。它应该以大约每秒一次的频率亮/灭交替。关闭闪烁后恢复正常显示。如果你看到的闪烁节奏不均匀，检查一下是不是创建了多个定时器。

---

## 最终组装 — 把零件串起来

现在我们来写 main.cpp，把四个 LED 放到窗口里展示。这里给一个最小的 main 函数骨架：

```cpp
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "status_led.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // TODO: 创建一个 QWidget 作为主窗口
    // TODO: 创建一个 QHBoxLayout 把四个 LED 水平排列
    //       每个 LED 旁边放一个 QLabel 说明状态
    // TODO: 下面放一个 QPushButton "切换闪烁"
    //       点击后让第一个 LED 切换闪烁状态

    // 主窗口 show
    return app.exec();
}
```

布局方案建议用 QVBoxLayout 做主布局，上半部分放 QHBoxLayout 横排 LED 组，下半部分放按钮。

别忘了 CMakeLists.txt：你的项目需要链接 `Qt6::Core`、`Qt6::Gui` 和 `Qt6::Widgets`，开启 `CMAKE_AUTOMOC`。参考一下之前讲座教程里用的 CMakeLists 模板就行。

---

## 验收标准

运行你的程序，逐项确认以下这些：

四个 LED 显示了正确的颜色（绿、红、黄、灰），渐变效果看起来像在发光而不只是个色块。通过代码调用 setStatus 切换状态后颜色立即变化。点击闪烁按钮后 LED 以大约每秒两次的频率闪烁，再次点击闪烁停止并恢复正常显示。缩放窗口后 LED 仍然是正圆而不是被压扁成椭圆。连续快速点击闪烁按钮不会导致多个定时器叠加产生鬼畜效果。

---

## 进阶挑战

如果你觉得上面的内容太轻松，试试下面这些：给 LED 加一个 QToolTip，鼠标悬停时显示当前状态名称（比如"正常"、"告警"），提示——查一下 QWidget 的 event() 方法和 QToolTip。或者给 LED 加一个"呼吸"模式：用 QPropertyAnimation 做一个从亮到暗再到亮的循环动画，比简单开关看起来高级得多。再或者，让你的 StatusLed 支持自定义大小提示（sizeHint）和最小尺寸（minimumSizeHint），让布局管理器能正确处理它的尺寸。

---

## 踩坑预防清单

> **坑 #1：忘记 Q_OBJECT 宏**
> 如果你用了信号槽但忘记在类声明里加 Q_OBJECT，编译能过但信号槽不工作，运行时没有任何报错，调试到怀疑人生。解决办法：凡是继承 QObject 的自定义类，无脑加 Q_OBJECT。

> **坑 #2：paintEvent 里调用 update()**
> paintEvent → update() → 又触发 paintEvent → 无限递归 → 程序卡死。update() 只应该在 paintEvent 之外的地方调用（比如 setStatus 里）。在 paintEvent 里你只需要专心画图。

> **坑 #3：QPainter 没有正确管理生命周期**
> 如果你用 `new QPainter` 并忘记 `delete`，或者忘记调 `end()`，轻则内存泄漏，重则绘制异常。推荐用栈上构造的方式：`QPainter painter(this);`，出了作用域自动清理。

> **坑 #4：定时器 parent 没设置**
> QTimer 的 parent 不设成 this 的话，StatusLed 销毁后定时器还在跑，timeout 信号触发时访问的成员变量已经不存在了——典型的 use-after-free，轻则段错误，重则给你一个完全看不懂的崩溃栈。

---

## 官方文档参考

- [QWidget Class](https://doc.qt.io/qt-6/qwidget.html) — paintEvent、sizeHint、update 等
- [QPainter Class](https://doc.qt.io/qt-6/qpainter.html) — 绘图 API
- [QRadialGradient Class](https://doc.qt.io/qt-6/qradialgradient.html) — 径向渐变
- [QTimer Class](https://doc.qt.io/qt-6/qtimer.html) — 定时器

到这里就大功告成了。如果你一路做到了这里，你已经从零手搓了一个完整的自定义自绘控件——虽然只是一个小灯泡，但 paintEvent + QPainter + QTimer 这个组合，是日后所有复杂自绘控件的基石。下一篇我们会练 SearchEdit，换个思路：不用自绘，而是用"组合"的方式把现有的控件拼装成一个新控件。那又是另一种风景。
