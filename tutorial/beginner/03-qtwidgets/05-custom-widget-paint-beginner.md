# 现代Qt开发教程（新手篇）3.5——自定义绘制 Widget 基础

## 1. 前言 / 为什么需要自定义绘制

QSS 能调颜色、改边框、设圆角，但它有天生的边界——你没法用它画一个圆形进度条、没法画一个带贝塞尔曲线的图表、没法画一个不规则的仪表盘。这些需求在正经的产品界面里并不罕见，当你真的需要"画"出一些标准控件提供不了的视觉效果时，唯一的出路就是继承 QWidget 并重写 `paintEvent()`，自己拿 QPainter 往屏幕上画。

说实话，第一次面对"自己画一个控件"这个需求的时候我是有点怵的——总觉得那是图形学大佬才干的事。但实际写下来你会发现，Qt 的绘制体系封装得相当友好：你不需要管帧缓冲区、不需要管显存、不需要管渲染管线，你只需要在一个叫 `paintEvent` 的函数里拿着 QPainter 这个"画笔"，按照从上到下的顺序一条一条地画就行。画笔、画刷、路径、文字、图片——QPainter 把这些东西统一成了非常直观的 API。

这篇文章我们只关注四件事：`paintEvent` 的完整流程和触发时机、`update()` 和 `repaint()` 的区别、双缓冲绘制防闪烁的原理、以及 `sizeHint()` / `minimumSizeHint()` 如何让自定义控件和布局系统和平共处。把这四件事搞清楚，自定义绘制的基础就扎实了。

## 2. 环境说明

本篇代码基于 Qt 6.5+，CMake 3.26+，C++17 标准。自定义绘制需要 QtGui 模块中的 QPainter 相关类和 QtWidgets 模块中的 QWidget，所以 CMake 里链接 Qt6::Gui 和 Qt6::Widgets 即可。所有绘制代码在任何支持 Qt6 的桌面平台上都能正常工作，QPainter 内部会自动处理不同平台的渲染后端差异（Windows 上用 Direct2D 或 GDI+、Linux 上用 XRender 或 Cairo、macOS 上用 CoreGraphics）。

## 3. 核心概念讲解

### 3.1 paintEvent 完整流程与触发时机

`paintEvent(QPaintEvent *event)` 是 QWidget 的一个虚函数，每当 Qt 认为某个控件需要被重绘时，就会调用这个函数。你不需要手动调用它——你只需要重写它，在里面写好绘制逻辑，Qt 会在合适的时机帮你调用。

paintEvent 的执行流程非常线性：首先你创建一个 QPainter 对象并绑定到当前 Widget（`QPainter painter(this)`），然后依次调用 QPainter 的绘制方法画背景、画内容、画前景，最后 QPainter 析构时自动把绘制结果提交到屏幕。整个流程是同步的——paintEvent 返回的时候，画面就已经更新了。

```cpp
void MyWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 开启抗锯齿

    // 画背景
    painter.fillRect(rect(), QColor("#F5F5F5"));

    // 画内容
    painter.setPen(QPen(Qt::blue, 2));
    painter.drawEllipse(20, 20, width() - 40, height() - 40);

    // 画文字
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 14));
    painter.drawText(rect(), Qt::AlignCenter, "自定义绘制");
}
```

这段代码展示了一个最简单的 paintEvent 结构。`rect()` 返回控件的整个矩形区域，`fillRect()` 用颜色填充整个背景，`drawEllipse()` 画一个椭圆，`drawText()` 居中绘制文字。注意 `setRenderHint(QPainter::Antialiasing)` 这一行——它开启了抗锯齿，否则画出来的圆会有锯齿。对于包含曲线、圆形、斜线等非水平垂直图形的绘制，抗锯齿几乎是必须的。

那么 paintEvent 什么时候会被触发？有三种情况。

第一种是系统触发的重绘。当窗口第一次显示、从最小化恢复、被其他窗口遮挡后暴露出来、或者窗口大小发生变化时，操作系统会告诉 Qt "这个区域需要重绘了"，Qt 会把事件投递到控件的事件队列中，最终调用 paintEvent。这种触发是你无法控制也不需要关心的——系统说画你就画。

第二种是你主动调用 `update()` 触发的重绘。当你的数据发生变化、需要刷新显示时，你调用 `update()`（或者带参数的 `update(x, y, w, h)` 指定只更新某个矩形区域）。`update()` 不会立刻调用 paintEvent，而是把一个重绘请求投递到事件队列里。如果连续多次调用 `update()`，Qt 会把多次重绘请求合并成一次——这是一个很重要的优化，避免了一帧之内多次重绘的浪费。

第三种是你主动调用 `repaint()` 触发的立即重绘。和 `update()` 不同，`repaint()` 会立刻同步调用 paintEvent，不经过事件队列。这意味着如果你在 `repaint()` 后面的代码里读控件的像素，你能保证读到的是最新的结果。

绝大多数情况下你应该用 `update()`。`repaint()` 的使用场景非常有限——主要是在实现一些需要精确帧同步的动画或实时渲染时才会用到。

### 3.2 update() vs repaint() 的区别与选用

这两个方法的区别看起来简单——一个是异步一个是同步——但实际影响比你想的要大。

`update()` 的核心行为是"延迟合并"。调用 `update()` 后，Qt 不会立刻重绘，而是在下一个事件循环迭代时统一处理。如果你在一帧之内对同一个控件调用了三次 `update()`（比如你先后改了三个数据属性），三次调用会被合并成一次 paintEvent。这意味着你的绘制逻辑只会执行一次，性能开销是原来的三分之一。对于频繁更新的场景（比如动画、实时数据展示），这种合并机制能显著减少不必要的重绘。

`update()` 还有几个重载版本。`update(int x, int y, int width, int height)` 只标记指定矩形区域为"脏区域"，paintEvent 中可以通过 `event->rect()` 获取这个区域，然后只重绘脏区域内的内容。`update(const QRect &rect)` 和 `update(const QRegion &region)` 是它的 QRect/QRegion 版本。这种"局部更新"在控件面积很大但只有一小部分需要重绘时非常有用——比如一个网格控件只有一个单元格变了，你只需要重绘那个单元格所在的矩形就行了。

```cpp
// 只更新某个区域，而不是整个控件
void MyWidget::onDataChanged(int row, int col)
{
    QRect cellRect = getCellRect(row, col);
    update(cellRect);  // 只标记这个单元格为脏区域
}

void MyWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    // event->rect() 返回需要重绘的区域
    // 如果只有一个小区域需要重绘，可以跳过其他区域的绘制
    QRect dirtyRect = event->rect();
    // ... 只绘制与 dirtyRect 相交的内容
}
```

`repaint()` 的行为完全不同——它是同步的、立即的。调用 `repaint()` 后，paintEvent 会在当前调用栈中立刻执行。这意味着如果你在 `repaint()` 后面紧跟着读取控件的像素（比如 `grab()` 截图），你能保证截到的是最新画面。但代价是它绕过了 Qt 的所有优化机制：不合并、不延迟、不裁剪。连续调用三次 `repaint()` 就会导致 paintEvent 执行三次。

实际开发中，99% 的场景用 `update()` 就对了。`repaint()` 的典型使用场景是在实现自定义动画的时候，你需要保证每一帧的绘制时机和外部帧同步逻辑严格一致，不允许出现一帧合并导致跳帧的情况。或者在单元测试中，你需要立刻看到绘制结果来做验证。除了这两种情况，`repaint()` 基本没有用武之地。

有一点需要特别注意：在 paintEvent 内部绝对不能调用 `update()` 或 `repaint()`。这会导致无限递归——paintEvent 调 update，update 触发下一次 paintEvent，然后又调 update……直到栈溢出崩溃。如果你需要在 paintEvent 中触发后续更新，用 `QTimer::singleShot(0, this, [this]() { update(); })` 这种方式把 update 调用延迟到下一个事件循环迭代。

### 3.3 双缓冲绘制防闪烁原理

如果你写过 Win32 的 GDI 绘制程序，一定对"闪烁"这个现象不陌生——控件在重绘的时候，先擦除旧内容（变成白色），然后画新内容。如果擦除和绘制之间有一个肉眼可见的时间差，用户就会看到一个白屏闪烁。窗口越大、绘制逻辑越复杂、机器越慢，闪烁越明显。

Qt 在 QWidget 层面默认启用了双缓冲机制，原理是：paintEvent 中所有的绘制操作并不是直接画到屏幕上的，而是先画到一个离屏的 QPixmap（后台缓冲区）上。等 paintEvent 执行完毕、所有绘制操作都完成后，Qt 才把整个后台缓冲区一次性拷贝到屏幕上。因为"擦除旧内容"和"绘制新内容"都是在后台缓冲区中完成的，用户看到的永远是最终结果——要么是旧画面，要么是新画面，不会出现中间状态。

这就是双缓冲的核心思想：用一个后台缓冲区做"草稿纸"，画完之后再整体展示。Qt 从 4.0 版本开始就默认为所有 QWidget 启用了双缓冲，所以你在正常使用 paintEvent 的时候基本不会遇到闪烁问题。

但有一个例外：如果你在代码中手动调用了 `setAttribute(Qt::WA_PaintOnScreen, true)`，Qt 就会绕过双缓冲机制，让你直接在屏幕上绘制。这个属性是为了支持一些需要直接操作底层渲染上下文的特殊场景（比如嵌入 OpenGL 或 Direct3D 的渲染结果），普通自定义绘制完全不需要碰它。如果你发现自己做的自定义控件出现了闪烁，第一件事就是检查有没有误设了 `WA_PaintOnScreen`。

另一个可能导致闪烁的原因是 `setAttribute(Qt::WA_OpaquePaintEvent, true)`。这个属性告诉 Qt "我的 paintEvent 会覆盖整个控件区域，你不需要帮我擦除背景"。如果你设了这个属性但 paintEvent 里又没有完全覆盖控件区域，没被覆盖的部分就会显示为垃圾像素。这个属性的正确用法是：当你的 paintEvent 确实会填满整个 rect() 的时候，开启它可以省掉一次背景擦除操作，略微提升性能。但如果你不确定，就别碰它，让 Qt 帮你擦背景就好。

另外，如果你在控件的构造函数里设置了 `setAutoFillBackground(true)`，Qt 会在 paintEvent 之前自动用 QPalette 的背景色填充控件区域，然后再让你的 paintEvent 在上面画。这相当于帮你做了一次底色填充，避免了你自己在 paintEvent 开头写 `painter.fillRect(rect(), backgroundColor)` 的麻烦。不过它会引入一次额外的绘制操作，对于性能敏感的场景可以关掉然后自己管理背景填充。

### 3.4 sizeHint() / minimumSizeHint() 告知布局系统尺寸

自定义控件如果只是能画出来还不够——你还需要让它和布局系统配合工作。布局管理器在安排控件位置和大小时，需要知道两件事：这个控件"想要多大"和"最小能缩到多小"。这两项信息分别由 `sizeHint()` 和 `minimumSizeHint()` 提供。

```cpp
class CircularProgressBar : public QWidget
{
    Q_OBJECT

public:
    QSize sizeHint() const override
    {
        return QSize(200, 200);  // 推荐尺寸 200x200
    }

    QSize minimumSizeHint() const override
    {
        return QSize(50, 50);  // 最小不能小于 50x50
    }

    // ...
};
```

`sizeHint()` 返回控件的"理想尺寸"——也就是在没有任何外部约束的情况下，控件最希望自己有多大。布局管理器会把 sizeHint 作为初始参考值。对于我们的圆形进度条来说，200x200 是一个既能清晰展示进度弧线又不会过分占空间的尺寸。

`minimumSizeHint()` 返回控件的"最小可用尺寸"——小于这个尺寸控件就没法正常显示了。布局管理器在空间紧张时会优先保证控件不小于 minimumSizeHint。如果可用空间比 minimumSizeHint 还小，布局管理器也无可奈何，控件会被裁剪。

当布局管理器分配给控件的实际尺寸和 sizeHint 不一样时（通常都会不一样），你需要在 paintEvent 里根据 `width()` 和 `height()` 动态计算绘制参数。比如圆形进度条的半径应该根据控件的实际尺寸来算，而不是写死 80 像素。

```cpp
void CircularProgressBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 根据实际尺寸动态计算半径
    int side = qMin(width(), height());
    int radius = side / 2 - 10;  // 留 10px 的边距

    QPointF center(width() / 2.0, height() / 2.0);

    // 画背景圆环
    painter.setPen(QPen(QColor("#E0E0E0"), 8));
    painter.drawEllipse(center, radius, radius);

    // 画进度弧
    painter.setPen(QPen(QColor("#3498DB"), 8));
    painter.drawArc(
        QRectF(center.x() - radius, center.y() - radius,
               radius * 2, radius * 2),
        90 * 16,                   // 起始角度（12 点钟方向）
        -m_progress * 3.6 * 16     // 扫过角度（根据进度百分比计算）
    );

    // 画中心文字
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", radius / 4));
    painter.drawText(rect(), Qt::AlignCenter,
                     QString::number(m_progress) + "%");
}
```

你会发现这里所有绘制参数都是相对于 `width()`、`height()` 和 `center` 计算的，没有写死任何绝对坐标。这就是自定义控件能适应不同尺寸的关键——所有几何参数都是动态计算的，而不是硬编码的。当布局管理器把控件从 200x200 拉大到 400x300 的时候，进度条的半径、线宽、字体大小都会自动调整。

还有一点需要提：`sizePolicy()` 和 sizeHint 是配合工作的。sizeHint 告诉布局"我想要多大"，sizePolicy 告诉布局"给了我不是想要的尺寸我会怎样反应"。默认的 sizePolicy 是 `(QSizePolicy::Preferred, QSizePolicy::Preferred)`，意思是"我有一个推荐的尺寸（sizeHint），但给我大一点小一点我都能接受"。如果你希望控件在某个方向上固定为 sizeHint 的大小不变，可以设置 `setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed)`。如果你的控件可以无限拉伸（比如一个水平进度条），设置 `setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed)`。

## 4. 踩坑预防

第一个坑是在 paintEvent 里做耗时的操作。paintEvent 是在主线程上同步执行的，如果你在 paintEvent 里做 IO 操作、网络请求、复杂的数据计算，会导致界面卡顿甚至无响应。所有耗时的计算都应该在 paintEvent 之外完成，paintEvent 只负责"把已经算好的结果画出来"。如果你发现界面的帧率很低，第一件事就是检查 paintEvent 里有没有不该有的计算逻辑。

第二个坑是忘记开启抗锯齿。`QPainter::Antialiasing` 默认是关闭的。如果你画了一个椭圆或一条斜线发现边缘有锯齿，不是你代码写错了，而是忘了调用 `painter.setRenderHint(QPainter::Antialiasing)`。如果你画的是纯水平和垂直的矩形或线条，抗锯齿开不开差别不大；但凡涉及曲线、圆弧、旋转、缩放，抗锯齿几乎是必须的。另一个有用的 renderHint 是 `QPainter::TextAntialiasing`，它控制文字的抗锯齿——不过这个默认就是开的，一般不需要额外设置。

第三个坑是 paintEvent 内部调用 `update()` 导致无限递归。前面已经详细说了原因，这里再强调一次：paintEvent 里绝对不能直接调 update() 或 repaint()。如果你需要持续刷新（比如动画），用 `QTimer` 定期在外面调 `update()`，或者在 paintEvent 结束后通过 `QTimer::singleShot(0, ...)` 延迟触发下一次 update。

第四个坑是没有正确实现 `sizeHint()`。如果你做了一个自定义控件但不提供 sizeHint，布局管理器会使用默认值 `(0, 0)` 或者一个很小的尺寸。结果就是你的控件在布局里被压成一条线或者完全看不见。养成习惯：自定义控件一定要实现 sizeHint()，返回一个合理的尺寸。同时如果控件有最小尺寸限制，也把 minimumSizeHint() 实现了。

第五个坑是在 paintEvent 中使用绝对坐标和硬编码尺寸。比如写死了 `painter.drawEllipse(50, 50, 100, 100)`，当控件被拉大后这个圆还是 100x100，不会跟着缩放。正确的做法是用 `width()`、`height()`、`rect()` 这些动态值来计算所有几何参数，确保绘制内容能自适应控件的实际大小。

## 5. 练习项目

我们来做一个综合练习：实现一个自定义的"圆形仪表盘"控件（CircularGauge）。这个控件需要展示一个半圆形的仪表盘，底色是灰色的弧线，上面叠加一条彩色的弧线表示当前值。弧线下方居中显示当前数值的文字。控件支持通过 `setValue(double)` 方法设置当前值（0 到 100），值改变时触发平滑的动画过渡。

具体要求是：重写 `paintEvent` 绘制半圆弧和数值文字，所有几何参数基于 `width()` 和 `height()` 动态计算；实现 `sizeHint()` 返回 `(250, 160)`，`minimumSizeHint()` 返回 `(100, 70)`；用 QPropertyAnimation 对值做动画过渡，动画持续 500ms，使用 `QEasingCurve::OutCubic` 缓动函数；每次值变化时调用 `update()` 触发重绘。

然后在主窗口中放两个仪表盘，一个显示"CPU 使用率"、一个显示"内存使用率"，用 QVBoxLayout 垂直排列。底部放两个 QSlider 分别控制两个仪表盘的值，这样你可以拖动滑块实时看到仪表盘的动画效果。

几个提示：半圆弧用 `drawArc()` 实现，起始角度是 180 度（9 点钟方向），扫过 180 度到 0 度（3 点钟方向），Qt 的角度单位是 1/16 度，所以起始角度是 `180 * 16`，扫过角度是 `-180 * 16 * (value / 100)`；QPropertyAnimation 需要你在类中声明一个 `Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)`，这样动画系统才能通过 property 系统驱动值的渐变；QPainter 的 `QPen` 设置 `Qt::RoundCap` 线帽样式可以让弧线的两端变成圆角，视觉效果更好。

## 6. 官方文档参考链接

[Qt 文档 · QPainter](https://doc.qt.io/qt-6/qpainter.html) -- Qt 绘制引擎的核心类，所有绘制操作的入口

[Qt 文档 · QWidget::paintEvent](https://doc.qt.io/qt-6/qwidget.html#paintEvent) -- paintEvent 的文档说明，包含重绘机制和事件参数

[Qt 文档 · QWidget::update](https://doc.qt.io/qt-6/qwidget.html#update) -- update() 的所有重载版本和延迟合并机制

[Qt 文档 · QWidget::repaint](https://doc.qt.io/qt-6/qwidget.html#repaint) -- repaint() 的文档，包含和 update() 的对比说明

[Qt 文档 · QWidget::sizeHint](https://doc.qt.io/qt-6/qwidget.html#sizeHint-prop) -- sizeHint 属性的文档，包含布局系统如何使用这个值的说明

[Qt 文档 · QPaintEvent](https://doc.qt.io/qt-6/qpaintevent.html) -- 绘制事件的文档，rect() 方法获取需要重绘的区域

---

到这里，自定义绘制 Widget 的基础你就掌握了。paintEvent 拿 QPainter 画东西、update() 异步触发重绘、双缓冲保证不闪烁、sizeHint 配合布局系统——这四件事组成了自定义控件绘制的核心骨架。后面不管你画的是进度条、图表还是游戏 UI，底层走的都是这套机制。下一篇我们进入对话框体系，那是一套完全不同的能力：模态与非模态对话框、标准按钮配置、数据返回机制。
