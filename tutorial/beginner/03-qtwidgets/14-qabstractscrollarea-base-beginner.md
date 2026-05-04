# 现代Qt开发教程（新手篇）3.14——QAbstractScrollArea：滚动区域基类

## 1. 前言 / 为什么你需要搞懂 QAbstractScrollArea

我们在 Qt 开发中遇到的"可以滚动"的控件——QTextEdit、QPlainTextEdit、QTableWidget、QTreeWidget、QListWidget、QGraphicsView，还有直接使用的 QScrollArea——它们全部继承自同一个基类：QAbstractScrollArea。这个基类封装了一套完整的滚动机制：水平滚动条、垂直滚动条、滚动条策略、视口（viewport）管理，以及内容滚动时的回调。你在这些控件上调用的 `horizontalScrollBar()` 和 `verticalScrollBar()`，以及 `setHorizontalScrollBarPolicy()` 和 `setVerticalScrollBarPolicy()`，全部来自 QAbstractScrollArea 的继承。

说实话，滚动这件事看起来简单——内容超出了控件大小，出现滚动条让用户拖动查看更多内容——但底层实现并不简单。QAbstractScrollArea 把内容区域和滚动条之间的关系管理得很好：它使用一个 QWidget 作为"视口"来显示内容的一部分，两个 QScrollBar 控件让用户控制视口在内容中的位置，而内容本身的绘制由子类在视口上完成。理解这个架构对正确使用和扩展滚动控件至关重要，尤其是当你需要继承 QAbstractScrollArea 实现自定义的滚动控件时。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QAbstractScrollArea 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。QAbstractScrollArea 的滚动行为在所有桌面平台上一致——它使用 Qt 自己的 QScrollBar 控件而不是操作系统的原生滚动条（除非你通过 QStyle 做了特殊定制）。在触摸屏设备和高分辨率显示器上，滚动条的交互行为可能会有细微差异——比如在 HiDPI 下滚动条的像素宽度会被自动缩放，但这些差异由 Qt 内部处理，对应用层代码透明。

## 3. 核心概念讲解

### 3.1 滚动条控制：horizontalScrollBar() / verticalScrollBar()

QAbstractScrollArea 内部维护了两个 QScrollBar 对象——一个水平方向，一个垂直方向。你可以通过 `horizontalScrollBar()` 和 `verticalScrollBar()` 获取它们的指针，然后像操作普通 QScrollBar 一样操作它们。

QScrollBar 有四个核心属性你需要了解：`minimum`、`maximum`、`value` 和 `pageStep`。`minimum` 和 `maximum` 定义了滚动条的值域范围，`value` 是当前的滚动位置，`pageStep` 是点击滚动条滑块两侧空白区域时值的变化量（相当于"翻一页"）。对于 QAbstractScrollArea 的内部滚动条，这些值是由子类在内容大小变化时自动设置的——比如 QTextEdit 在文本内容改变时会重新计算滚动条的 maximum，QTableWidget 在行数变化时也会更新。

但作为使用者，你可以直接操作这些滚动条来实现一些实用功能。比如"滚动到顶部"就是 `verticalScrollBar()->setValue(0)`，"滚动到底部"就是 `verticalScrollBar()->setValue(verticalScrollBar()->maximum())`。再比如"滚动一页"就是 `verticalScrollBar()->setValue(verticalScrollBar()->value() + verticalScrollBar()->pageStep())`。

```cpp
auto *textEdit = new QTextEdit();

// 滚动到顶部
textEdit->verticalScrollBar()->setValue(0);

// 滚动到底部
textEdit->verticalScrollBar()->setValue(
    textEdit->verticalScrollBar()->maximum()
);

// 监听滚动位置变化
connect(textEdit->verticalScrollBar(), &QScrollBar::valueChanged,
    [](int value) {
        qDebug() << "当前滚动位置:" << value;
    }
);
```

你还可以通过滚动条的 `setSingleStep(int)` 来控制鼠标滚轮每滚一格时的滚动量。默认值通常是 1 个"行高"或等效单位，你可以根据需要调整。比如在一个图片查看器中，你可能希望鼠标滚轮每次滚动 20 像素而不是 1 像素。

```cpp
// 设置鼠标滚轮每次滚动的像素量
textEdit->verticalScrollBar()->setSingleStep(20);
textEdit->horizontalScrollBar()->setSingleStep(20);
```

有一个微妙的地方需要了解：QAbstractScrollArea 的滚动条是惰性的——它们只在需要时才被创建和显示。如果你从来没调过 `horizontalScrollBar()` 或设置过水平滚动条策略，水平滚动条可能根本不存在。所以你在获取滚动条指针后应该检查是否为 nullptr——虽然大部分情况下不会是 nullptr，但防御性编程不会有害。

### 3.2 滚动条策略：ScrollBarPolicy

`setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy)` 和 `setVerticalScrollBarPolicy(Qt::ScrollBarPolicy)` 控制滚动条的显示策略。有三个可选值。

`Qt::ScrollBarAsNeeded` 是默认策略——只有当内容超出了控件的可视区域时，滚动条才会出现；如果内容完全在可视区域内，滚动条自动隐藏。这是最常用的策略，适合大多数场景。在 macOS 上，`ScrollBarAsNeeded` 还有一个额外的行为：滚动条默认是半透明覆盖式的（overlay scrollbar），只有在用户滚动时才会短暂显示，不占用布局空间。这个行为由系统设置控制，Qt 会自动适配。

`Qt::ScrollBarAlwaysOff` 强制关闭滚动条，无论内容是否溢出。滚动条不会显示，也不会占用布局空间。但注意，关闭滚动条不等于关闭滚动功能——用户仍然可以用鼠标滚轮、触摸板手势或键盘方向键来滚动内容。如果你确实要禁止滚动，需要额外处理——比如重写 `wheelEvent` 并忽略事件。

`Qt::ScrollBarAlwaysOn` 强制显示滚动条，即使内容没有溢出。在这种模式下，当内容完全在可视区域内时，滚动条的滑块会填满整个滚动条（value == minimum == maximum，或者更准确地说 maximum 会被设为 0），视觉上呈现为"禁用"状态但始终可见。这个策略在你需要保持界面布局一致性的时候很有用——避免滚动条出现/消失导致的界面跳动。

```cpp
auto *area = new QScrollArea();

// 默认策略：需要时显示
area->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

// 强制关闭水平滚动条
area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

// 强制显示垂直滚动条
area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
```

在 QScrollArea 这个最常见的子类中，`ScrollBarAlwaysOff` 有一个经常被使用的场景：当你想让 QScrollArea 的内容自动换行以适应宽度，不允许水平滚动时，就把水平滚动条策略设为 `AlwaysOff`，然后在内容 widget 的布局中设置合适的换行策略。

### 3.3 scrollContentsBy()：内容滚动响应

`scrollContentsBy(int dx, int dy)` 是 QAbstractScrollArea 提供给子类的保护（protected）虚函数。当用户拖动滚动条或者程序改变了滚动条的值时，QAbstractScrollArea 会调用这个函数来通知子类"内容需要滚动"。参数 `dx` 和 `dy` 是水平和垂直方向的滚动偏移量（像素），正值表示向右/向下滚动，负值表示向左/向上滚动。

这个函数是子类实现自定义滚动的核心入口。默认实现会调用 `viewport()->scroll(dx, dy)` 来移动视口中的内容——这是 Qt 通过 `QWidget::scroll()` 实现的优化操作，它利用操作系统的位块传输（bitblt）来快速移动已经绘制好的像素，而不是重新绘制整个视口。只有新暴露出来的区域需要重新绘制，这对于性能来说是很重要的优化。

如果你继承 QAbstractScrollArea 实现自定义控件，通常需要重写 `scrollContentsBy()` 来更新你的内容绘制。最简单的做法是调用基类实现（让视口做位块传输），然后调用 `viewport()->update()` 刷新整个视口。如果你知道只需要刷新某些区域，可以调用 `viewport()->update(QRect(...))` 来做局部刷新以提高性能。

```cpp
class CustomScrollWidget : public QAbstractScrollArea
{
    Q_OBJECT

public:
    explicit CustomScrollWidget(QWidget *parent = nullptr)
        : QAbstractScrollArea(parent)
    {
        // 假设内容高度为 2000 像素
        m_contentHeight = 2000;
        // 配置垂直滚动条
        verticalScrollBar()->setRange(0, m_contentHeight - viewport()->height());
        verticalScrollBar()->setPageStep(viewport()->height());
        verticalScrollBar()->setSingleStep(20);
    }

protected:
    /// @brief 内容滚动时被调用
    void scrollContentsBy(int dx, int dy) override
    {
        // 调用基类实现：利用位块传输优化滚动
        QAbstractScrollArea::scrollContentsBy(dx, dy);
        // 刷新视口以绘制新暴露的区域
        viewport()->update();
    }

    /// @brief 绘制视口内容
    void paintEvent(QPaintEvent *event) override
    {
        QPainter painter(viewport());
        painter.setRenderHint(QPainter::Antialiasing);

        // 根据 verticalScrollBar()->value() 偏移绘制
        int scrollOffset = verticalScrollBar()->value();
        painter.translate(0, -scrollOffset);

        // 绘制你的内容（这里以水平线为例）
        painter.setPen(QPen(QColor("#CCCCCC"), 1));
        for (int y = 0; y < m_contentHeight; y += 30) {
            painter.drawLine(0, y, viewport()->width(), y);
        }
    }

private:
    int m_contentHeight = 2000;
};
```

这段代码展示了一个最简化的自定义滚动控件。核心逻辑是：在 `paintEvent` 中通过 `verticalScrollBar()->value()` 获取当前滚动偏移，然后对 painter 做对应的平移变换（`translate`），使得绘制的内容随滚动条的拖动而移动。`scrollContentsBy()` 调用基类实现后刷新视口，确保新暴露的区域被正确绘制。

有一个性能相关的细节值得说明：当滚动量很大时（比如用户快速拖动滚动条滑块），位块传输优化可能不如直接重绘整个视口高效，因为需要重绘的区域可能覆盖了整个视口。在这种情况下，`QWidget::scroll()` 的位块传输操作就是多余的。不过在实际应用中，这个性能差异几乎可以忽略，除非你在做高帧率的内容渲染（比如游戏或实时图表）。

### 3.4 视口（viewport）概念与绘制注意事项

视口（viewport）是 QAbstractScrollArea 架构中最核心的概念。你可以把它理解为一个"窗口"——内容是一个很大的区域，视口是你透过这个窗口看到的那一部分。滚动操作的本质就是移动这个窗口在内容中的位置。

在实现层面，视口就是 QAbstractScrollArea 内部持有的一个 QWidget 对象。你通过 `viewport()` 方法获取它的指针。子类的所有绘制操作都应该在视口上进行，而不是直接在 QAbstractScrollArea 自身上绘制。这意味着你在重写 `paintEvent` 时，QPaintEvent 的 region 是视口的区域，QPainter 的绘制目标也应该是 `viewport()`。

```cpp
void paintEvent(QPaintEvent *event) override
{
    // 注意：QPainter 的目标应该是 viewport()
    QPainter painter(viewport());
    // 而不是 QPainter painter(this)
    // ...
}
```

这可能是继承 QAbstractScrollArea 时最容易犯的错误——在 `paintEvent` 中把 `this` 作为 QPainter 的绘制目标。QAbstractScrollArea 自身的绘制区域包含了滚动条占据的空间，如果你在 `this` 上绘制，内容会被滚动条覆盖，而且坐标系统也不对。始终用 `viewport()` 作为绘制目标。

视口的大小由 QAbstractScrollArea 的布局自动管理——它占据了去掉滚动条之后的所有剩余空间。你可以通过 `viewport()->width()` 和 `viewport()->height()` 获取视口的当前大小。当控件被拉伸或收缩时，视口的大小会跟着变化，同时 QAbstractScrollArea 会自动调整滚动条的 range 和 pageStep。

```cpp
void resizeEvent(QResizeEvent *event) override
{
    QAbstractScrollArea::resizeEvent(event);
    // 视口大小变化后，需要更新滚动条的 range
    int visibleHeight = viewport()->height();
    verticalScrollBar()->setRange(0, m_contentHeight - visibleHeight);
    verticalScrollBar()->setPageStep(visibleHeight);
}
```

上面这段代码展示了在 `resizeEvent` 中更新滚动条范围的正确做法。当用户拉伸窗口导致视口大小变化时，滚动条的 `maximum` 和 `pageStep` 都需要重新计算——`maximum` 应该是"内容总大小减去视口大小"，`pageStep` 应该是视口的大小。

另一个需要注意的点是：视口的坐标系统从 (0, 0) 开始，不包含滚动偏移。滚动偏移由你自己在 `paintEvent` 中通过 `translate()` 或手动减去 `scrollBar()->value()` 来处理。这意味着视口坐标永远是"屏幕上看到的那块区域"的坐标，不是"内容中的绝对坐标"。

还有一个关于 `viewport()->update()` 的使用建议：当你需要刷新视口内容时，调用 `viewport()->update()` 而不是 `update()`。前者只刷新视口区域，后者会刷新整个 QAbstractScrollArea（包括滚动条区域），显然前者更高效。

## 4. 踩坑预防

第一个坑是在 `paintEvent` 中用 `QPainter(this)` 而不是 `QPainter(viewport())`。这个错误会导致内容绘制到错误的位置——被滚动条覆盖，或者坐标完全错乱。始终记住：QAbstractScrollArea 的绘制目标是视口，不是它自身。

第二个坑是忘记在 `resizeEvent` 中更新滚动条的范围。如果内容大小或视口大小变化后你没有重新设置 `scrollBar()->setRange()`，滚动条的滑块大小和行为就会不正确——可能出现滑块超出滚动条范围、或者滚动不到底部的情况。最佳实践是在任何可能影响内容大小或视口大小的操作之后，都调用一个统一的 `updateScrollBars()` 函数来重新计算所有滚动条参数。

第三个坑是 `scrollContentsBy()` 中忘记调用基类实现。默认实现做了位块传输优化，如果你不调用它，滚动时内容会出现撕裂或闪烁——因为旧的内容像素没有被移动到正确位置。除非你确实需要完全自定义的滚动渲染逻辑（比如每次滚动都重绘所有内容），否则应该先调用 `QAbstractScrollArea::scrollContentsBy(dx, dy)` 再做后续处理。

第四个坑是滚动条的 `maximum` 计算不正确。正确的公式是 `maximum = contentSize - viewportSize`，而不是 `contentSize`。如果你把 `maximum` 设成了内容总大小，那滚动到底部时视口显示的会是最底部一行的内容，但滚动条的滑块不会到达最右/最下端——因为 `value` 的最大值等于 `maximum`，此时视口的底边对齐内容的底边，但滚动条滑块没有到底。用 `contentSize - viewportSize` 可以确保滑块到达最底端时，内容的最后一行恰好显示在视口底边。

第五个坑是在高 DPI 显示器上的像素偏移问题。Qt 在 HiDPI 模式下会自动对坐标做缩放，QPainter 的绘制和滚动条的值都会受到影响。大部分情况下 Qt 内部处理得很好，但如果你在 `paintEvent` 中手动计算滚动偏移并且混用了物理像素和逻辑像素，可能会出现模糊或偏移半个像素的问题。建议始终使用逻辑坐标（`viewport()->width()` 等返回的就是逻辑坐标），不要手动做 DPI 缩放。

## 5. 练习项目

我们来做一个综合练习：继承 QAbstractScrollArea 实现一个自定义的时间轴控件。时间轴展示从 0 到 100 的时间刻度（每 10 个单位一个主刻度，每 2 个单位一个次刻度），内容总高度为 2000 像素（每 20 像素一个时间单位）。控件需要支持垂直滚动来浏览完整的时间轴。窗口分为上下两部分：上部是自定义的时间轴滚动控件，下部是控制面板。控制面板包含三个按钮和一个标签——"滚动到顶部"按钮把滚动条值设为 0，"滚动到底部"按钮把滚动条值设为 maximum，"切换滚动条策略"按钮在 `ScrollBarAsNeeded` 和 `ScrollBarAlwaysOn` 之间切换。标签实时显示当前滚动位置对应的"时间单位"（通过监听滚动条的 valueChanged 信号计算）。

几个提示：继承 QAbstractScrollArea 后，在构造函数中初始化滚动条的 range、pageStep 和 singleStep；重写 `paintEvent(QPaintEvent *)` 在 `viewport()` 上绘制刻度线，用 `translate(0, -verticalScrollBar()->value())` 做滚动偏移；重写 `scrollContentsBy(int, int)` 调用基类实现后刷新视口；重写 `resizeEvent(QResizeEvent *)` 更新滚动条的 range 和 pageStep。

## 6. 官方文档参考链接

[Qt 文档 · QAbstractScrollArea](https://doc.qt.io/qt-6/qabstractscrollarea.html) -- 滚动区域基类

[Qt 文档 · QScrollArea](https://doc.qt.io/qt-6/qscrollarea.html) -- 最常用的滚动区域实现

[Qt 文档 · QScrollBar](https://doc.qt.io/qt-6/qscrollbar.html) -- 滚动条控件

[Qt 文档 · QTextEdit](https://doc.qt.io/qt-6/qtextedit.html) -- 文本编辑器，继承自 QAbstractScrollArea

[Qt 文档 · QGraphicsView](https://doc.qt.io/qt-6/qgraphicsview.html) -- 图形视图，也继承自 QAbstractScrollArea

---

到这里，QAbstractScrollArea 的核心机制你就搞定了。horizontalScrollBar / verticalScrollBar 给你直接操作滚动条的入口，ScrollBarPolicy 控制滚动条的显示策略，scrollContentsBy 是子类响应滚动事件的核心回调，而 viewport 概念是整个滚动架构的基石——理解视口是"窗口"、内容是"大画布"、滚动是"移动窗口位置"，你就能在继承 QAbstractScrollArea 时做到心中有数。
