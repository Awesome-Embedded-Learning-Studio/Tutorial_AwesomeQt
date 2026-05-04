# 现代Qt开发教程（新手篇）3.9——属性动画框架基础

## 1. 前言 / 为什么你需要一套动画框架

如果你在界面上让一个按钮从左边"唰"地滑到右边，或者让一个窗口淡入淡出，最朴素的做法是在定时器回调里手动修改控件的 geometry 或 opacity，每一帧算一个中间值，然后调用 `update()`。这套流程写一个动画还行，写三个以上你就会发现重复代码堆积如山——而且你还得自己处理动画的暂停、恢复、串行衔接、缓动曲线等一堆细节。Qt 的动画框架（Animation Framework）把这些通用逻辑全部抽象出来了，核心思路非常简单：你告诉它"哪个属性的值从 A 变到 B，花多少毫秒，用什么缓动曲线"，剩下的插值计算和定时刷新全部由框架代劳。

这套框架的核心类是 `QPropertyAnimation`，它可以对任意 `Q_PROPERTY` 做动画——不限于 geometry，控件的 opacity、color、windowOpacity、甚至你自定义的属性，只要它有合理的 `READ`/`WRITE` 函数并且类型支持 `QVariant` 插值，就可以被驱动。在此基础上，`QSequentialAnimationGroup` 和 `QParallelAnimationGroup` 让你把多个动画组合成串行或并行的编排，`QEasingCurve` 则提供了四十多种缓动函数来控制动画的加速减速节奏。说实话，掌握这几个类的组合用法之后，日常开发中绝大多数界面动画需求都可以用不到五十行代码搞定。

这篇文章我们从最基础的 `QPropertyAnimation` 单个属性动画开始，逐步加入缓动曲线和串行动画组，最后用一个综合练习把所有知识点串起来。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。动画框架属于 QtCore 模块中的 `QPropertyAnimation`、`QAbstractAnimation` 等类以及 `QSequentialAnimationGroup` 等组合类，这些类全部在 Qt6::Core 中，但我们的示例界面会用到 QtWidgets，所以实际链接的是 `Qt6::Widgets`（它会自动拉取 Qt6::Core 和 Qt6::Gui）。动画框架在所有 Qt 支持的桌面平台上行为一致，不涉及平台相关的渲染差异。

## 3. 核心概念讲解

### 3.1 QPropertyAnimation：对一个属性做动画

`QPropertyAnimation` 的工作原理可以用一句话概括：它在指定的时间段内，按照指定的缓动曲线，线性地（或者非线性地）把一个 `Q_PROPERTY` 的值从 startValue 插值到 endValue。每过一帧，它会调用该属性的 WRITE 函数把当前插值写入目标对象。你不需要自己开定时器，不需要自己算插值，不需要自己调 `update()`——这些都由框架完成。

使用 `QPropertyAnimation` 的前提条件是目标对象的属性必须通过 `Q_PROPERTY` 宏声明。Qt 内置的 QWidget 子类已经为 `geometry`、`windowOpacity`、`pos`、`size` 等属性做了声明，所以你可以直接对这些属性做动画。如果你要对自定义属性做动画，需要在类声明中加上 `Q_PROPERTY` 宏。

我们先看最简单的例子——让一个按钮从窗口左侧滑到右侧：

```cpp
auto *button = new QPushButton("滑动按钮", this);
button->setGeometry(20, 100, 120, 40);

auto *animation = new QPropertyAnimation(button, "geometry");
animation->setDuration(1500);  // 1500 毫秒
animation->setStartValue(QRect(20, 100, 120, 40));
animation->setEndValue(QRect(500, 100, 120, 40));
animation->start(QAbstractAnimation::DeleteWhenStopped);
```

你会发现这段代码的逻辑非常直白：创建动画对象，指定目标和属性名，设置起止值和时长，启动。`QAbstractAnimation::DeleteWhenStopped` 表示动画播放完毕后自动删除动画对象，避免内存泄漏。这是最常用的启动方式。

这里有一个细节值得说明：`setStartValue` 和 `setEndValue` 接收的是 `QVariant`，这意味着你可以传任何支持 `QVariant` 插值的类型。对于 `geometry` 属性来说，类型是 `QRect`，Qt 内置了 `QRect` 的插值规则——每一帧在 x、y、width、height 四个维度上分别做线性插值。如果你想对 `pos` 属性做动画，传入 `QPoint` 即可；对 `windowOpacity` 做动画，传入 `double` 即可。

另一个常见的用法是对控件的 `pos`（位置）做动画，保持大小不变：

```cpp
auto *anim = new QPropertyAnimation(button, "pos");
anim->setDuration(1000);
anim->setStartValue(QPoint(20, 100));
anim->setEndValue(QPoint(500, 100));
anim->setEasingCurve(QEasingCurve::OutCubic);
anim->start(QAbstractAnimation::DeleteWhenStopped);
```

这段代码只改变按钮的位置而不改变大小。你可以根据需求选择对 `geometry`（位置+大小）还是 `pos`（仅位置）做动画。

### 3.2 setStartValue / setEndValue / setDuration 基础配置

这三个方法是 `QPropertyAnimation` 最基础的配置项，我们逐个拆解。

`setDuration(int msecs)` 设置动画的总时长，单位是毫秒。这个值直接影响动画的"快慢"感觉——300ms 通常给人"快速响应"的感觉，1000ms 是"平滑过渡"的节奏，3000ms 以上会让人等得有点不耐烦。对于大多数界面过渡动画，500-1500ms 是一个合理的范围。

`setStartValue(const QVariant &value)` 设置属性的起始值。如果你不调用这个方法，动画会使用属性当前的值作为起始值。这意味着你可以省略 `setStartValue`，让动画从当前状态开始——这对于"从当前位置滑到目标位置"的场景非常方便。

`setEndValue(const QVariant &value)` 设置属性的终止值。这个值是动画结束时属性应该达到的最终值。

```cpp
// 淡入效果：窗口从完全透明到完全不透明
auto *fadeIn = new QPropertyAnimation(this, "windowOpacity");
fadeIn->setDuration(800);
fadeIn->setStartValue(0.0);
fadeIn->setEndValue(1.0);
fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
```

这段代码实现了一个窗口淡入效果。`windowOpacity` 是 QWidget 的内置属性，类型是 `double`，取值范围 0.0 到 1.0。注意，要让 `windowOpacity` 动画生效，窗口需要启用 `Qt::WA_TranslucentBackground` 属性，并且窗口标志需要包含 `Qt::FramelessWindowHint`（否则标题栏的透明度行为可能不符合预期）。

一个容易忽略的点是，`setStartValue` 和 `setEndValue` 的类型必须和属性的声明类型匹配。如果你对一个 `QRect` 类型的属性传入 `QPoint`，动画在运行时会输出一条警告并且不做任何事。这一点在编译期不会报错，是运行时的行为，所以如果你发现动画没效果，第一件事就是检查值的类型是否和属性类型一致。

我们再来看一个对自定义属性做动画的例子。假设我们有一个自定义 widget，需要对其背景色的红绿蓝分量做动画：

```cpp
class ColorWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor backgroundColor READ backgroundColor
               WRITE setBackgroundColor NOTIFY backgroundColorChanged)

public:
    QColor backgroundColor() const { return m_bgColor; }

    void setBackgroundColor(const QColor &color)
    {
        if (m_bgColor != color) {
            m_bgColor = color;
            update();  // 触发重绘
            emit backgroundColorChanged(color);
        }
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.fillRect(rect(), m_bgColor);
    }

signals:
    void backgroundColorChanged(const QColor &color);

private:
    QColor m_bgColor{Qt::white};
};
```

声明了 `Q_PROPERTY` 之后，你就可以像操作内置属性一样对 `backgroundColor` 做动画了。`QPropertyAnimation` 会自动利用 `QColor` 的插值规则在两个颜色之间做平滑过渡。WRITE 函数里我们调用了 `update()`，这样每一帧颜色变化时控件都会重绘，视觉上就是颜色渐变的效果。

### 3.3 QEasingCurve：缓动函数效果对比

如果你只用默认的线性插值（`QEasingCurve::Linear`），动画的每一帧都会匀速推进——起点到终点的速度恒定不变。这在物理世界中几乎不存在：现实中物体从静止开始加速、到中间达到最大速度、接近终点时减速停下。缓动函数就是用来模拟这种"自然的运动节奏"的，它定义了动画进度（0.0 到 1.0）和时间进度（0.0 到 1.0）之间的映射关系。

`QEasingCurve` 提供了大约四十种预设曲线，日常开发中最常用的大概就这几种：

`QEasingCurve::Linear` 是默认值，匀速直线运动，适合进度条、数值递增这类"不需要表达物理运动感"的场景。

`QEasingCurve::InQuad` / `InCubic` / `InQuart` 是"慢启动"系列，动画开始时速度很慢然后逐渐加快。`In` 表示加速发生在动画的前半段，`Quad`/`Cubic`/`Quart` 是二次、三次、四次曲线，次数越高加速越猛烈。

`QEasingCurve::OutQuad` / `OutCubic` / `OutQuart` 是"快启慢停"系列，动画开始时速度很快然后逐渐减慢。这是界面动画中最常用的曲线族——控件从某个位置飞入并柔和地停在目标位置，给人的感觉非常自然。`OutCubic` 大概是你在各种应用里见过最多的缓动效果。

`QEasingCurve::InOutQuad` / `InOutCubic` 是"慢-快-慢"系列，两头慢中间快，像一个完整的加速-匀速-减速过程。适合尺寸变化、颜色渐变这类"两端都需要柔和"的过渡。

`QEasingCurve::OutBounce` 是弹跳效果，动画到达终点时会像弹球一样反弹几次再停下。视觉效果很活泼但用多了会显得花哨，适合游戏或者趣味性比较强的界面。

`QEasingCurve::InBack` / `OutBack` 是回弹效果，动画会在开始（`InBack`）或结束（`OutBack`）时稍微超过目标值然后回弹。`OutBack` 特别适合"弹出"效果——控件从某个方向飞入，略微超过目标位置然后弹回，看起来很有弹性。

```cpp
auto *anim = new QPropertyAnimation(button, "pos");
anim->setDuration(800);
anim->setStartValue(QPoint(0, 100));
anim->setEndValue(QPoint(400, 100));
anim->setEasingCurve(QEasingCurve::OutCubic);  // 快启慢停
anim->start(QAbstractAnimation::DeleteWhenStopped);
```

通过 `setEasingCurve()` 设置缓动曲线只需要一行代码，但对动画的视觉感受影响巨大。同一个动画，`Linear` 看起来像机器运动，`OutCubic` 看起来像物体自然减速，`OutBounce` 看起来像弹球。选择合适的缓动曲线是让动画"看起来舒服"的关键。

如果你觉得四十种预设都不够用，`QEasingCurve` 还支持自定义贝塞尔曲线和自定义函数指针。通过 `QEasingCurve(QEasingCurve::BezierSpline)` 可以设置贝塞尔控制点，通过 `setCustomType()` 可以传入一个 `qreal (*)(qreal)` 类型的函数。但在实际开发中，预设曲线几乎总是够用的。

### 3.4 QSequentialAnimationGroup：串行动画组合

当你需要多个动画按顺序依次播放时——比如先让按钮从左边飞入，然后改变颜色，最后弹出一个提示——手动用信号槽串联 `QPropertyAnimation` 的 `finished()` 信号虽然能实现，但代码会变得非常冗长。`QSequentialAnimationGroup` 就是为了解决这个问题而设计的：你把多个动画按顺序加到 group 里，group 会自动管理它们的串行播放。

```cpp
auto *group = new QSequentialAnimationGroup(this);

// 第一步：从左侧飞入
auto *slideIn = new QPropertyAnimation(button, "pos");
slideIn->setDuration(600);
slideIn->setStartValue(QPoint(-150, 100));
slideIn->setEndValue(QPoint(200, 100));
slideIn->setEasingCurve(QEasingCurve::OutCubic);
group->addAnimation(slideIn);

// 第二步：在目标位置停留 300ms
group->addPause(300);

// 第三步：向下移动
auto *slideDown = new QPropertyAnimation(button, "pos");
slideDown->setDuration(400);
slideDown->setStartValue(QPoint(200, 100));
slideDown->setEndValue(QPoint(200, 300));
slideDown->setEasingCurve(QEasingCurve::InOutQuad);
group->addAnimation(slideDown);

group->start(QAbstractAnimation::DeleteWhenStopped);
```

`addAnimation()` 把一个动画添加到组的末尾。`addPause(int msecs)` 插入一个指定时长的暂停——这在两个动画之间需要间隔时非常方便。整个 group 的播放是串行的：第一个动画播完之后自动开始第二个（或暂停），暂停结束后自动开始第三个，以此类推。

`QSequentialAnimationGroup` 本身继承自 `QAbstractAnimation`，所以它也有 `start()`、`stop()`、`pause()`、`resume()`、`setLoopCount()` 等方法。你可以像操作单个动画一样操作整个组。它还有 `currentAnimation()` 方法返回当前正在播放的子动画，以及 `animationAdded`、`currentAnimationChanged` 等信号用于监听组的状态变化。

与 `QSequentialAnimationGroup` 对应的是 `QParallelAnimationGroup`，它让所有子动画同时开始播放。两者可以嵌套使用——比如在一个串行组中嵌入一个并行组，实现"先做 A，然后同时做 B 和 C，最后做 D"这种复杂编排。

```cpp
auto *seqGroup = new QSequentialAnimationGroup(this);

// 先淡入
auto *fadeIn = new QPropertyAnimation(widget, "windowOpacity");
fadeIn->setDuration(500);
fadeIn->setStartValue(0.0);
fadeIn->setEndValue(1.0);
seqGroup->addAnimation(fadeIn);

// 同时：向右滑 + 放大
auto *parGroup = new QParallelAnimationGroup(this);
auto *slide = new QPropertyAnimation(widget, "pos");
slide->setDuration(800);
slide->setStartValue(QPoint(100, 200));
slide->setEndValue(QPoint(300, 200));
slide->setEasingCurve(QEasingCurve::OutCubic);
parGroup->addAnimation(slide);

auto *grow = new QPropertyAnimation(widget, "geometry");
grow->setDuration(800);
grow->setStartValue(QRect(100, 200, 100, 40));
grow->setEndValue(QRect(300, 200, 200, 60));
grow->setEasingCurve(QEasingCurve::OutCubic);
parGroup->addAnimation(grow);

seqGroup->addAnimation(parGroup);
seqGroup->start(QAbstractAnimation::DeleteWhenStopped);
```

你会发现嵌套组合的代码虽然看起来长，但逻辑非常清晰——串行组管理"先做什么后做什么"，并行组管理"同时做什么"。这种声明式的动画编排方式比手动用定时器和信号槽拼接要简洁得多，而且更容易维护和修改。

## 4. 踩坑预防

第一个坑是动画目标对象在动画播放期间被销毁。`QPropertyAnimation` 持有的是目标对象的指针，如果在动画播放过程中你 `delete` 了目标对象，动画在下一帧尝试访问它时就会触发野指针崩溃。解决方法是在删除目标对象之前先 `stop()` 所有关联的动画，或者用 `QPointer` 来安全地跟踪目标对象。

第二个坑是属性类型不匹配。`setStartValue` 和 `setEndValue` 传入的 `QVariant` 类型必须和 `Q_PROPERTY` 声明的类型完全一致。对一个 `QRect` 属性传入 `QPoint`，动画不会生效，也不会在编译期报错，只在运行时输出一条很容易被忽略的警告。调试"动画没效果"的问题时，先检查这个。

第三个坑是忘记让自定义属性的 WRITE 函数触发重绘。如果你对一个自定义的颜色属性做动画，WRITE 函数里只赋了值但没调 `update()`，那么虽然属性值确实在变化，但控件表面看起来什么都没发生。动画是在驱动属性值的变化，而界面更新需要你自己在 WRITE 函数里手动触发。

第四个坑是 `setDuration(0)`。把时长设为 0 不会让动画"瞬间完成"——它会让动画直接跳到 endValue 然后结束。如果你需要"瞬间设置"某个属性值，直接调用 setter 方法更直接，不需要用动画。

第五个坑是在 `QSequentialAnimationGroup` 中，子动画的 `start()` 和 `stop()` 不应该由你手动调用。组的生命周期管理是由 group 统一控制的，如果你手动 start 了一个已经加入 group 的子动画，行为是未定义的。你只需要操作 group 本身的 `start()` / `stop()` 即可。

## 5. 练习项目

我们来做一个综合练习：创建一个窗口，里面有三个色块（自定义 ColorWidget）和两个按钮。点击"播放动画"按钮后，三个色块依次从窗口左侧飞入到各自的目标位置（用 `QSequentialAnimationGroup` 串行编排），飞入时使用 `OutCubic` 缓动曲线。色块到位后自动进行颜色渐变（从白色渐变到各自的主题色）。点击"重置"按钮后所有色块回到初始位置和颜色，为下一次播放做准备。

几个提示：色块使用前面提到的 `ColorWidget` 类，声明 `Q_PROPERTY(QColor backgroundColor ...)`；位置动画用 `QPropertyAnimation` 对 `pos` 属性操作；颜色动画用 `QPropertyAnimation` 对 `backgroundColor` 属性操作；串行编排时，三个色块的位置动画可以分别加入串行组实现依次飞入效果；颜色渐变可以在位置动画之后，通过嵌套一个并行组让三个色块同时变色。

## 6. 官方文档参考链接

[Qt 文档 · QPropertyAnimation](https://doc.qt.io/qt-6/qpropertyanimation.html) -- 属性动画类，核心 API 说明

[Qt 文档 · QEasingCurve](https://doc.qt.io/qt-6/qeasingcurve.html) -- 缓动曲线，包含所有预设曲线的可视化图示

[Qt 文档 · QSequentialAnimationGroup](https://doc.qt.io/qt-6/qsequentialanimationgroup.html) -- 串行动画组

[Qt 文档 · QParallelAnimationGroup](https://doc.qt.io/qt-6/qparallelanimationgroup.html) -- 并行动画组

[Qt 文档 · The Animation Framework](https://doc.qt.io/qt-6/animation-overview.html) -- 动画框架总览，包含架构图和概念说明

[Qt 文档 · QAbstractAnimation](https://doc.qt.io/qt-6/qabstractanimation.html) -- 动画基类，定义了状态管理和循环控制

---

到这里，属性动画框架的基础你就掌握了。`QPropertyAnimation` 驱动单个属性从 A 到 B，`QEasingCurve` 控制加速减速的节奏，`QSequentialAnimationGroup` 和 `QParallelAnimationGroup` 让你把多个动画编排成复杂的播放序列。动画框架的 API 数量不多，但组合出来的可能性几乎是无限的——日常开发中遇到"让某个东西动起来"的需求，这套框架基本都能覆盖。下一篇我们换个方向，看看 Qt 的多文档界面（MDI）是怎么做的。
