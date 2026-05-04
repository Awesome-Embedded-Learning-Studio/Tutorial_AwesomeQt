# 现代Qt开发教程（新手篇）3.32——QScrollBar：滚动条

## 1. 前言 / 当你需要自己控制滚动逻辑

我们在讲 QAbstractScrollArea 的时候提到过，QScrollArea 内部自动管理了一对 QScrollBar——当内容超出可视区域时，滚动条自动出现，用户拖动滚动条就能滚动内容。这套机制在绝大多数场景下够用了。但总有些时候你需要直接操作滚动条来驱动自定义的内容偏移——比如你自己写了一个时间轴控件，内容是一排水平排列的事件卡片，你不想用 QScrollArea 把整个时间轴包起来（那样会破坏你对绘制区域的完全控制），而是想用一个独立的水平滚动条来控制时间轴的偏移量。又比如你在做一个图片查看器，需要用滚动条来做画布的平移，但同时还需要在同一个区域处理缩放和拖拽——QScrollArea 的封装反而成了限制。

QScrollBar 就是 Qt 为这种"自己控制滚动"的场景提供的原始控件。它和 QSlider 继承自同一个基类 QAbstractSlider，所以你会发现它们的 API 非常相似——setRange / setValue / setSingleStep / setPageStep 以及 valueChanged 信号都是同一套。但 QScrollBar 的定位和 QSlider 完全不同：QSlider 是"让用户调一个值"，QScrollBar 是"让用户控制内容的可见偏移"。今天我们要把 QScrollBar 的四个核心维度讲清楚：独立使用滚动条驱动自定义控件、setRange / setPageStep / setSingleStep 的配置、valueChanged 信号驱动内容偏移、以及 QScrollBar 和 QScrollArea 之间的关系。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QScrollBar 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。它继承自 QAbstractSlider（与 QSlider 和 QDial 同父），所以核心 API 都是共享的。示例代码中我们会自定义一个绘制控件来演示滚动条驱动内容偏移的效果，用到了 QPainter 的基础绘制能力和 QWidget::paintEvent，这些同样在 QtWidgets 中。

## 3. 核心概念讲解

### 3.1 独立使用滚动条驱动自定义控件

QScrollBar 最常见的独立使用场景就是把它和一个自定义绘制的控件绑定在一起。滚动条负责接收用户的滚动操作，自定义控件负责根据滚动条的 value 来决定绘制内容的偏移量。这种模式的本质是把"滚动控制"和"内容渲染"解耦——滚动条只是一个数值提供者，怎么用这个数值完全由你决定。

```cpp
// 创建一个水平滚动条和一个自定义绘制控件
auto *scrollBar = new QScrollBar(Qt::Horizontal);
auto *customWidget = new CustomTimelineWidget();

// 滚动条的值变化时，通知自定义控件更新偏移
connect(scrollBar, &QScrollBar::valueChanged,
        customWidget, &CustomTimelineWidget::setScrollOffset);
```

和 QScrollArea 的区别在于：QScrollArea 是一个"开箱即用的滚动容器"，你把内容 widget 塞进去，它自动帮你管理滚动条、视口、内容偏移。但 QScrollArea 要求内容是一个完整的 QWidget——它不支持你自定义绘制逻辑中的一部分区域可滚动、另一部分固定。而独立使用 QScrollBar 时，你拥有完全的控制权：滚动条的 value 映射到什么偏移量、偏移后怎么重绘、每次重绘绘制哪些内容，全由你自己决定。

典型的应用场景包括：自定义的时间轴控件（水平滚动）、自定义的画布（带缩放的平移）、表格控件中的固定表头和可滚动数据区、游戏地图编辑器中的视口控制。这些场景的共同特征是"内容的渲染逻辑非常定制化，无法简单地用一个 QWidget 包起来交给 QScrollArea 管理"。

### 3.2 setRange / setPageStep / setSingleStep 配置

QScrollBar 的配置参数和 QSlider 非常类似，但语义有所不同。对于 QScrollBar 来说，setRange 设定的不是"用户可以选什么值"，而是"内容的总范围有多大"。pageStep 对应的是"视口大小"，singleStep 对应的是"单次滚动步长"。

```cpp
auto *scrollBar = new QScrollBar(Qt::Horizontal);

// 假设内容总宽度 2000 像素，视口宽度 600 像素
int contentWidth = 2000;
int viewportWidth = 600;

// range 的 max = 内容总尺寸 - 视口尺寸
scrollBar->setRange(0, contentWidth - viewportWidth);
scrollBar->setPageStep(viewportWidth);    // 点击槽时跳转的距离
scrollBar->setSingleStep(20);            // 鼠标滚轮每次滚动的距离
scrollBar->setValue(0);                  // 初始位置在最左边
```

这里有一个核心公式：`range_max = content_size - viewport_size`。当内容的总尺寸小于等于视口尺寸时，range 的 max 应该是 0（或者负数，此时 range 会被设为 (0, 0)），表示不需要滚动。当内容尺寸大于视口尺寸时，range 的 max 就是内容"多出来"的那部分——滚动条的 value 可以从 0 变到 content_size - viewport_size，对应内容偏移量从 0 到最大偏移。

pageStep 设为视口尺寸的效果是：当用户点击滚动条的槽（手柄两侧的空白区域）时，内容恰好滚动一整屏。这和 QScrollArea 的默认行为一致。singleStep 通常设为一个较小的值（比如 20 或 30 像素），对应鼠标滚轮每格的滚动距离。

```cpp
void updateScrollBar(QScrollBar *bar, int contentSize, int viewportSize)
{
    if (contentSize <= viewportSize) {
        // 内容没超出视口，不需要滚动
        bar->setRange(0, 0);
        bar->setValue(0);
    } else {
        bar->setRange(0, contentSize - viewportSize);
        bar->setPageStep(viewportSize);
        bar->setSingleStep(qMax(1, viewportSize / 20));
    }
}
```

这里有一个容易出错的地方：如果你把 range 的 max 设成了 contentSize（而不是 contentSize - viewportSize），滚动条可以滚到最右端，但此时内容会偏移 contentSize 个像素——最右边会超出视口，显示出来的是一片空白。记住 range_max 永远是 content_size - viewport_size，这个公式是滚动条的数学基础。

当内容尺寸动态变化时（比如列表中新增了元素、画布缩放后逻辑尺寸变大），你需要重新调用 updateScrollBar 更新 range 和 pageStep。如果新的 range 比旧的 range 小，而当前 value 超出了新的 max，QScrollBar 会自动把 value 钳位到新的 max——这和 QSlider 的钳位行为一致。

### 3.3 valueChanged 信号驱动内容偏移

valueChanged(int) 是 QScrollBar 的核心信号。它的值就是当前的偏移量——你需要把这个偏移量传递给你的内容渲染逻辑。

```cpp
connect(m_hScrollBar, &QScrollBar::valueChanged,
        this, [this](int value) {
    m_offsetX = value;     // 记录水平偏移
    update();              // 触发重绘
});

connect(m_vScrollBar, &QScrollBar::valueChanged,
        this, [this](int value) {
    m_offsetY = value;     // 记录垂直偏移
    update();              // 触发重绘
});
```

在 paintEvent 中，你需要根据偏移量来决定绘制内容的起始位置。最简单的做法是用 QPainter::translate 把坐标系偏移——这样你的绘制代码不需要感知滚动，所有的坐标都是"逻辑坐标"，QPainter 的变换矩阵帮你处理偏移。

```cpp
void paintEvent(QPaintEvent *) override
{
    QPainter painter(this);
    // 先把坐标系平移到当前滚动位置
    painter.translate(-m_offsetX, -m_offsetY);
    // 之后所有的绘制操作都使用逻辑坐标
    drawContent(&painter);
}
```

注意 translate 的参数是负值——当你向右滚动时（value 增大），内容应该向左移动，所以坐标系的偏移是负方向。这是一个经常搞反的地方：如果你用了正值 translate，滚动方向就是反的。

如果你的内容包含很多元素（比如一个有 10000 行的列表），你不应该把所有元素都绘制出来——只绘制那些在当前视口范围内可见的元素。这需要根据偏移量和视口尺寸来计算"哪些元素是可见的"，然后只绘制这些元素。这种"裁剪"是性能优化的关键。

```cpp
void paintEvent(QPaintEvent *) override
{
    QPainter painter(this);

    int visibleTop = m_offsetY;
    int visibleBottom = m_offsetY + height();

    for (int i = 0; i < itemCount; ++i) {
        int itemY = i * itemHeight;
        int itemBottom = itemY + itemHeight;

        // 跳过不可见的项
        if (itemBottom < visibleTop || itemY > visibleBottom) {
            continue;
        }

        painter.save();
        painter.translate(0, itemY - m_offsetY);
        drawItem(&painter, i);
        painter.restore();
    }
}
```

这种模式在自定义列表、表格、时间轴控件中非常常见。它的本质就是"只在可见区域绘制可见内容"——虽然 QWidget 的 paintEvent 会自动裁剪超出 widget 边界的绘制，但如果你在 translate 之后绘制了 10000 个元素，即使有 9900 个被裁剪了不可见，QPainter 仍然需要执行这 9900 次绘制调用，开销不可忽视。手动裁剪把绘制调用从 O(N) 降到了 O(visible)。

还有一个细节：当用户使用鼠标滚轮时，QScrollBar 会自动处理滚轮事件并调整 value。你不需要自己重写 wheelEvent——只要你的自定义控件没有 accept 滚轮事件，滚轮事件会传播到 QScrollBar（前提是 QScrollBar 获得了焦点，或者你把 QScrollBar 的 focusPolicy 设为 Qt::WheelFocus）。但在实际开发中，更常见的做法是在自定义控件的 wheelEvent 中手动调用 QScrollBar::setValue。

```cpp
void wheelEvent(QWheelEvent *event) override
{
    int delta = event->angleDelta().y();
    int step = m_vScrollBar->singleStep();

    if (delta < 0) {
        m_vScrollBar->setValue(
            m_vScrollBar->value() + step);
    } else {
        m_vScrollBar->setValue(
            m_vScrollBar->value() - step);
    }
    event->accept();
}
```

这种方式让你可以精确控制滚轮的滚动行为——比如根据 Ctrl 是否按下来决定步长（按住 Ctrl 时慢速滚动），或者实现平滑滚动效果。

### 3.4 与 QScrollArea 的关系

讲到这里，你可能会问：既然 QScrollBar 可以独立使用，那 QScrollArea 内部到底做了什么？答案是：QScrollArea 本质上就是"一个 QWidget + 一对 QScrollBar + 一套自动管理的滚动逻辑"的封装。它帮你处理了上面提到的所有事情——内容尺寸变化时自动更新 range 和 pageStep，滚动条 value 变化时自动偏移内容 widget 的位置，滚轮事件自动转发给滚动条。

所以绝大多数情况下你应该优先使用 QScrollArea。只有当以下条件之一满足时，才需要考虑独立使用 QScrollBar：你的内容不是一个完整的 QWidget，而是自定义绘制逻辑；你需要部分区域可滚动、部分区域固定；你需要自定义滚动条的映射关系（比如非线性映射、缩放感知的滚动）；你正在实现一个自定义的可滚动容器控件。

QScrollArea 内部的滚动条可以通过 `horizontalScrollBar()` 和 `verticalScrollBar()` 方法获取。如果你需要对 QScrollArea 的滚动行为做一些微调（比如修改 singleStep、设置初始位置、监听滚动位置变化），直接操作这两个 QScrollBar 即可，不需要绕开 QScrollArea。

```cpp
auto *scrollArea = new QScrollArea();
scrollArea->setWidget(new MyContentWidget());

// 自定义滚动行为
scrollArea->verticalScrollBar()->setSingleStep(30);  // 加大滚轮步长

// 监听滚动位置
connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged,
        this, [](int value) {
    qDebug() << "滚动位置:" << value;
});
```

还有一个相关的类是 QAbstractScrollArea，它是 QScrollArea 的基类。如果你想实现自己的可滚动容器（比 QScrollArea 更定制化，但又不完全从零开始），可以继承 QAbstractScrollArea 并重写它的 `scrollContentsBy(int dx, int dy)` 方法。QAbstractScrollArea 帮你管理了滚动条和视口，你只需要实现"内容偏移后怎么更新"这一步。

## 4. 踩坑预防

第一个坑是 range 的 max 应该设为 content_size - viewport_size，而不是 content_size。如果你设成了 content_size，滚动到最右边时内容会多偏移一个视口宽度，右侧出现大片空白。

第二个坑是 QPainter::translate 的偏移方向是负值。value 增大（向右/向下滚动）时，内容的绘制起始位置应该向左/向上移动，所以 translate 的参数应该是 -value。写反了滚动方向就反了。

第三个坑是忘记手动裁剪不可见元素。虽然 Qt 会自动裁剪超出 widget 边界的绘制，但 QPainter 的绘制调用本身就有开销。如果内容有几千个元素，不做裁剪的话 paintEvent 会成为性能瓶颈。

第四个坑是鼠标滚轮事件不一定能自动转发给 QScrollBar。如果你的自定义控件 accept 了 wheelEvent，QScrollBar 就收不到滚轮事件了。最安全的做法是在自定义控件的 wheelEvent 中手动调用 QScrollBar::setValue。

第五个坑是内容尺寸动态变化时忘记更新 range 和 pageStep。如果你的列表可以增删元素，或者画布可以缩放，必须在尺寸变化后调用 updateScrollBar 重新配置。否则滚动条的范围和实际内容尺寸不匹配，用户会发现要么滚动不到底部，要么滚动到底部还有一大截空白。

## 5. 练习项目

我们来做一个综合练习：创建一个"自定义时间轴浏览器"窗口，覆盖 QScrollBar 独立使用的核心用法。窗口中央是一个自定义绘制的 QWidget，显示一条水平时间轴，上面分布着若干事件卡片（用不同颜色的矩形表示，每个卡片有标题文字）。窗口底部是一个水平 QScrollBar，用来控制时间轴的水平偏移。窗口右侧有一个垂直 QScrollBar，用来控制时间轴的垂直缩放（value 越大，卡片高度越高、间距越大）。时间轴的事件数据用一个简单的 QVector 存储（每个事件包含日期偏移、标题、颜色）。滚动条的 range 和 pageStep 应该根据缩放级别动态更新。滚轮操作应该驱动水平滚动条。

几个提示：缩放级别变化时，内容的逻辑宽度 = 事件数量 * 卡片宽度 * 缩放系数，你需要据此重新计算水平滚动条的 range；垂直滚动条的 value 不直接映射为偏移量，而是映射为缩放系数（比如 1.0 到 3.0）；在 paintEvent 中先 translate(-offsetX, 0)，然后按逻辑坐标绘制卡片。

## 6. 官方文档参考链接

[Qt 文档 -- QScrollBar](https://doc.qt.io/qt-6/qscrollbar.html) -- 滚动条控件

[Qt 文档 -- QAbstractSlider](https://doc.qt.io/qt-6/qabstractslider.html) -- 抽象滑动条基类（QSlider / QScrollBar / QDial 共享）

[Qt 文档 -- QScrollArea](https://doc.qt.io/qt-6/qscrollarea.html) -- 滚动区域容器

[Qt 文档 -- QAbstractScrollArea](https://doc.qt.io/qt-6/qabstractscrollarea.html) -- 滚动区域基类

---

到这里，QScrollBar 的四个核心维度就全部讲完了。独立使用滚动条驱动自定义控件是 QScrollBar 的核心用法——它把"滚动控制"和"内容渲染"解耦，让你在自定义绘制场景下也能拥有标准的滚动体验。setRange / setPageStep / setSingleStep 三个参数定义了滚动条的行为模型，核心公式 range_max = content_size - viewport_size 是一切滚动计算的基础。valueChanged 信号配合 QPainter::translate 构成了"滚动驱动的重绘"的经典模式，而手动裁剪不可见元素则是性能保障的关键。最后，QScrollBar 和 QScrollArea 的关系理清楚后，你就知道什么时候该用 QScrollArea（大部分时候），什么时候该直接用 QScrollBar（需要自定义渲染控制的时候）。掌握了这些内容，任何需要滚动功能的自定义控件都不在话下了。
