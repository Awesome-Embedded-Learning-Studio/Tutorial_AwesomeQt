---
title: "3.14 QAbstractScrollArea 基类进阶"
description: "入门篇把滚动条控制、ScrollBarPolicy、scrollContentsBy 和 viewport 过了一遍，进阶篇看 QAbstractScrollArea 替子类统管的滚动机制。两个滚动区联动要防信号绕环，视口坐标与内容坐标靠滚动偏移换算，滚动条显隐牵动视口宽度，重画的内容走缓存，配套示例在 examples/advanced/03-qtwidgets/14-qabstractscrollarea-base-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.14——QAbstractScrollArea 基类进阶

[入门篇 QAbstractScrollArea 基础](../../beginner/03-qtwidgets/14-qabstractscrollarea-base-beginner.md) 把滚动条控制、ScrollBarPolicy、scrollContentsBy、viewport 概念过了一遍，拿现成控件滚一滚，入门篇的内容够用。控件基类部分走到第四篇，咱们看 QScrollArea、QTextEdit、QPlainTextEdit、QGraphicsView、QAbstractItemView 共同的祖先 QAbstractScrollArea。水平垂直两个滚动条、显示策略、视口管理、内容滚动时的回调，全由它统一管着，咱们在子类上调的 verticalScrollBar()、scrollContentsBy() 也全是从它继承来的。

平时拿现成控件，内容一超，滚动条自己冒出来自己消失，咱们不必过问。一旦要让两个区域联动、要继承它画自己的内容，细节就都摆到眼前了。咱们把最容易把人问住的三个场面摆出来：两个滚动区互相连了信号槽，滚动抖动，信号绕成了环。自定义 paintEvent 里画的东西位置不对，或者被滚动条挡住，分不清自己站在哪套坐标系里。滚动条策略一切换，界面跳一下，视口宽度悄悄变了。三个场面病根各不相同，本篇把四件事讲明白：信号环怎么断，视口坐标与内容坐标怎么换算，滚动条显隐怎么牵动视口宽度，重画的内容怎么靠缓存与 viewport()->update() 省下来。

## 两个滚动区联动：信号环在 setValue 处断开

表格对比、对照查看、主从联动，经常要两个滚动区同步滚。思路不复杂，监听一个区域滚动条的 valueChanged，在槽里把值设给另一个区域。咱们要是把两边直接互连，麻烦就来了。A 的值变了触发 B 变，B 变了又反过来触发 A，信号绕成一个环。环倒不会无限转下去，笔者翻了 QAbstractSlider 的实现文件 qabstractslider.cpp，里面的 setValue 值没变它直接返回，valueChanged 根本不发，绕一圈碰到相同的值就停了。可这个环在某些边界条件下会让滚动不流畅，内容会抖。

```cpp
// 双向同步的正确做法：用 blockSignals 打破循环
void sync_vertical_scroll(QScrollArea *source, QScrollArea *target)
{
    auto *srcBar = source->verticalScrollBar();
    auto *tgtBar = target->verticalScrollBar();

    // 阻止 target 的信号回传，避免循环
    tgtBar->blockSignals(true);
    tgtBar->setValue(srcBar->value());
    tgtBar->blockSignals(false);
}

// 双向同步就连两条，设值一律走上面 blockSignals 包裹的 sync_vertical_scroll，
// 只需要单向同步时，把第二个 connect 删掉即可
connect(area_a->verticalScrollBar(), &QScrollBar::valueChanged,
    this, [this]() { sync_vertical_scroll(area_a, area_b); });

connect(area_b->verticalScrollBar(), &QScrollBar::valueChanged,
    this, [this]() { sync_vertical_scroll(area_b, area_a); });
```

咱们对着代码看这个环是怎么被断开的。blockSignals(true) 在 setValue 之前调，target 的 valueChanged 被按住，setValue 引不起回声，A 到 B 再回 A 的路就断了，设完值再用 blockSignals(false) 放回来。更省心的做法是只做单向同步，一个当主控、一个当从属，用户在主控区滚，从属区跟着动，从属区自己滚时不反向同步主控。业务上确实要双向，就用 blockSignals 方案，并测一测快速拖动滑块时抖不抖。

同步还有个量级问题要咱们提前算。两个区域的滚动条 maximum 得在同一个量级上，一边范围很大、一边很小，A 刚滚一小截 B 就到头了，用户只会更困惑。内容高度不同又要同步，别同步像素值，改同步比例，拿 `double ratio = double(value) / maximum` 换算过去。

配套示例把整套机制做成了能跑的工程，文件在 examples/advanced/03-qtwidgets/14-qabstractscrollarea-base-advanced/，可执行文件 14-qabstractscrollarea-base-advanced。咱们把界面数一遍：窗口里左右两个 SyncTextArea 并排，各自渲染 100 行文本，底部一条控制栏上摆着 Synchronize Scrolling 复选框（默认勾上）、Reset to Top 按钮、一个实时报位置的标签，百分比按 value 除以 maximum 乘 100 算。SyncTextArea 继承 QAbstractScrollArea，重写 scrollContentsBy，滚动时发出 scrollChanged 信号。静默设值走 setScrollValueSilently，blockSignals 包住 setValue，外面再拿 m_syncing 标志挡一层嵌套。

静默设值之后为什么还得自己调 viewport()->update()？咱们翻 qabstractscrollarea.cpp 的 QAbstractScrollAreaPrivate::init 就能看到，滚动条的 valueChanged 连着内部槽 _q_vslide，scrollContentsBy 正是从那里被调用的。blockSignals 一按，不只是环断了，目标区域自己的 scrollContentsBy 也被跳过，视口不会自动重画，所以 setScrollValueSilently 在设完值之后自己补了一次 viewport()->update()。

咱们跑一个实验。滚到中部，取消勾选 Synchronize Scrolling，左右两边各滚各的，互不牵扯。重新勾上，右面板立刻跳到左面板当前的位置。干这件事的是示例主窗口 sync_scroll_widget.cpp 里的 toggleSyncMode，它拿 setScrollValueSilently(m_leftArea->scrollValue()) 把两边拉回同一位置，同步恢复的头一步就是对齐。

## 内容坐标加滚动偏移：translate 移的是画笔，裁剪留在原地

入门篇说过视口坐标从 (0,0) 开始，不包含滚动偏移。咱们把边界画准。视口坐标系的原点在视口左上角，x 向右增长，y 向下增长。内容坐标系的原点在内容左上角。滚动偏移量就是两个坐标系之间的位移，内容坐标 = 视口坐标 + scrollOffset。三套容易搅在一起的空间，咱们列成表：

| 坐标系 | 原点 | 覆盖范围 |
| -------- | -------- | -------- |
| 视口坐标 | 视口左上角 | (0, 0) 到 (viewportWidth, viewportHeight) |
| 内容坐标 | 内容左上角 | (0, 0) 到 (contentWidth, contentHeight) |
| 控件自身坐标 | 控件左上角 | 整个 rect，滚动条与边角都在内 |

```cpp
// 在 paintEvent 中正确做坐标映射
void paintEvent(QPaintEvent *event) override
{
    QPainter painter(viewport());
    int scrollY = verticalScrollBar()->value();
    int scrollX = horizontalScrollBar()->value();

    // translate 把视口坐标原点平移到内容坐标的可见区域
    painter.translate(-scrollX, -scrollY);

    // 现在 painter 的坐标系就是内容坐标系
    // 画一条从 (0, 100) 到 (2000, 100) 的线
    painter.drawLine(0, 100, 2000, 100);

    // 但 translate 之后 clipRect 还是视口范围，
    // 所以只有视口可见区域内的线段会被实际绘制
}
```

代码里最容易想岔的一处：translate(-scrollX, -scrollY) 之后，painter 的逻辑坐标变成了内容坐标系，可 QPainter 的裁剪区域（clipRegion）仍然是视口的矩形范围。这恰好是咱们要的分工，不必自己算哪些内容落在视口里，把全部内容在内容坐标系里画出来，裁剪交给 Qt。但裁剪省的是绘制，不是遍历。绘制元素多到几万条线时，不可见的虽然一条都不画，paintEvent 里遍历它们的循环开销照样在，咱们得自己做粗粒度的可见性判断，只遍历落在视口范围内的元素。

表里第三行最容易让咱们画错位置。QAbstractScrollArea 自身的空间里，滚动条、角落、边框都占着位置，viewport() 返回的那个 QWidget 才是扣掉它们之后显示内容的区域。咱们在 paintEvent 里开画笔，要用 QPainter(viewport())，不是 QPainter(this)。算布局也一样，宽度从 viewport() 上取，viewport()->width() 才是内容真正可用的宽度。qabstractscrollarea.cpp 里 paintEvent 的文档注释专门提醒了一句，创建的 QPainter 必须作用在 viewport() 上。两套空间一混，内容要么画不到正确位置，要么被滚动条遮住。配套示例 SyncTextArea 的 paintEvent 解决的是同一个坐标换算问题，只是入口不同。它不平移坐标系，拿 scrollY 除以行高算出第一可见行，最后一行在视口能容纳的行数上再多取两行余量，每帧画的就是视口内那几十行，正是前面说的粗粒度可见性判断的现成实现。

咱们改一行看现象。sync_text_area.cpp 的 updateScrollBarRange 里，把 setRange(0, qMax(0, m_totalHeight - viewport()->height())) 的减法去掉，写成 setRange(0, m_totalHeight)，重新编译滚到底。最后一行继续上移滚出视口顶部，滚到最底时整个视口一片空白。滚动范围是内容高减视口高，多报了一个视口高，就多滚出一屏。看完记得改回去。

## 滚动条出现，视口就变窄：策略切换牵动的是布局

咱们拿最常见的可折叠面板来说，面板一展开，内容溢出，要滚动条。折叠回去，内容收进视口，滚动条该藏起来。切换本身一句 setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff) 就够，容易漏的是副作用。滚动条占的空间是从 QAbstractScrollArea 的 rect 中扣除的，滚动条出现，视口变窄，滚动条隐藏，视口变宽。这一变化会触发 resizeEvent，咱们写在 paintEvent 里、依赖 viewport()->width() 的布局计算就自动重排。重排不是错，宽度突然变化时用户看到的界面跳一下，才是要您当心的地方。

```cpp
// 平滑切换滚动条策略：先固定视口大小再切换
void set_scrollbar_shown(QScrollArea *area, bool show)
{
    // 保存当前视口宽度
    int vpWidth = area->viewport()->width();

    area->setVerticalScrollBarPolicy(
        show ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff
    );

    // 如果需要保持视口宽度不变，可以调整整个控件的尺寸
    // 但通常让 Qt 自动处理就行——内容会自适应新宽度
}
```

更实用的做法是在设计阶段把策略定下来，不频繁切换。确实要动态切，咱们可以配合 setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff) 固定水平滚动条的状态，至少水平方向的视口宽度是确定的。

被扣走的宽度，咱们也得心里有数。滚动条出现时占大约 16-20 像素的宽度，具体随平台和 QStyle 变，全都从视口里扣。内容的最小宽度要是恰好等于滚动区的宽度，垂直滚动条一出现视口变窄，内容就被压缩，或者干脆触发水平滚动。设计内容时给最小宽度留出滚动条的余量，或者禁用水平滚动，两个方向选一个。

内容是一个 widget 时（QScrollArea 的用法），视口宽度的变化还多一层牵连。setWidgetResizable 默认 false，widget 不会自动放大填满视口。想让它随视口缩放、不需要滚动，设 true，它和子控件尺寸策略的交互咱们放到 [3.44 QScrollArea 进阶](./44-qscrollarea-advanced.md) 展开。设了 true，widget 的最小大小约束（minimumSize）仍然生效，minimumWidth 大于视口宽度时 widget 照样被裁剪。本篇守在基类。

### 一道场景题：切换策略后，滚动位置还在吗

咱们的滚动区当前垂直策略是 AlwaysOff，内容高 2000 像素，视口高 400 像素。用户拿滚轮滚到了内容中间，此时把策略切成 ScrollBarAsNeeded，垂直滚动条出现了，视口变窄了。滚动条的 value 还是刚才滚到的值吗？答案是不一定。策略切换可能导致 Qt 内部重新计算滚动条的范围，value 可能被钳位到新的 maximum 之内。您要是把内容绘制挂在 value 上，切换策略后最好主动设置一次 value，确保位置正确。

咱们拿示例复现场景的前半段，也就是策略还挂在 AlwaysOff、用户单靠滚轮滚到中部的那一段。在 SyncTextArea 构造函数末尾加一句 setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff)，重新编译，滚动条不见了，滚轮照样滚。原因是 wheelEvent 把滚轮事件直接转发给垂直滚动条，paintEvent 读的还是它的 value，底部位置百分比标签也照常更新。策略管的是显隐，滚动条对象和它的值一直都在。看完把新加的一行删掉，恢复原状。

## 重活画进缓存：滚动一帧只是搬运像素

咱们继承 QAbstractScrollArea 做自定义控件，paintEvent 的调用频率不低，滚动时每一帧都来，resize 时也来。绘制逻辑重（渲染大量数据点、复杂图形）时，每次 paintEvent 都从头计算，瓶颈就出在这里。优化思路是把内容缓存进 QPixmap，内容数据变化时重新渲染缓存，滚动时把缓存里对应视口的区域贴到视口上，不重画。

```cpp
void paintEvent(QPaintEvent *event) override
{
    QPainter painter(viewport());
    int scrollY = verticalScrollBar()->value();

    // 把缓存的 pixmap 的对应区域绘制到视口
    // sourceRect 是 pixmap 中对应视口可见区域的矩形
    QRect source(0, scrollY, viewport()->width(), viewport()->height());
    painter.drawPixmap(0, 0, m_cache, source.x(), source.y(),
                       source.width(), source.height());
}

void rebuild_cache()
{
    // 在内容数据变化时调用
    m_cache = QPixmap(m_contentWidth, m_contentHeight);
    m_cache.fill(Qt::transparent);
    QPainter cachePainter(&m_cache);
    // 把全部内容画到缓存 pixmap 上
    render_all_content(&cachePainter);
    viewport()->update();
}
```

这个方案的代价是内存，缓存 pixmap 占的是 GPU 或系统内存，内容越大消耗越多。内容大到上万像素的画布，整块缓存不现实，咱们做分块缓存，把内容切成多个较小的 pixmap 块，只缓存当前可见区域附近的块，滚动时按需加载新块。QGraphicsView 处理超大场景靠的是另一招，它的 cacheMode 只缓存背景，每帧用 BSP 空间索引查询与暴露区域相交的图元，只画这些图元，思路正对应前面说的粗粒度可见性判断。

还有一对容易混的接口要您分清。viewport()->update() 只在视口区域触发重绘，update() 在整个 QAbstractScrollArea（包括滚动条区域）上触发重绘。绝大多数情况下咱们要前者，滚动条的重绘由 QScrollBar 自己管理，用不着咱们操心，调 update() 只换来不必要的滚动条重绘，浪费性能。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，QAbstractScrollArea 在 QtWidgets 模块，示例在桌面平台行为一致，macOS 的 overlay 滚动条在视觉效果上可能与 Windows/Linux 有差异。

[Qt 文档 · QAbstractScrollArea](https://doc.qt.io/qt-6/qabstractscrollarea.html) -- 滚动区域基类，viewport/scrollContentsBy 接口

[Qt 文档 · QScrollArea](https://doc.qt.io/qt-6/qscrollarea.html) -- 最常用的滚动区域，setWidgetResizable/setWidget 接口

[Qt 文档 · QScrollBar](https://doc.qt.io/qt-6/qscrollbar.html) -- 滚动条控件，valueChanged/setRange/pageStep 接口

[Qt 文档 · QWidget::scroll](https://doc.qt.io/qt-6/qwidget.html#scroll) -- 位块传输优化的内容滚动方法

---

viewport 加两个滚动条，就是 QAbstractScrollArea 交给子类的底子，本篇的联动、换算、缓存全是围着它们做的。想再动手的，请您把配套示例（examples/advanced/03-qtwidgets/14-qabstractscrollarea-base-advanced/，可执行文件 14-qabstractscrollarea-base-advanced）完整玩一轮。滚到中部，取消勾选 Synchronize Scrolling，左右各滚各的，重新勾上，右面板跳到左面板的位置，同步与独立的边界就摸到了。再把 sync_text_area.cpp 里 generateSampleText 的 i <= 100 改成 i <= 10000 重新编译，滚动依旧跟手，总行数翻了一百倍，paintEvent 每帧只画视口内那几十行，每帧的活没变，做完改回去。下一篇 [3.15 QAbstractItemView 基类进阶](./15-qabstractitemview-base-advanced.md) 讲 QListView、QTreeView、QTableView 的共同基类，它本身就继承自 QAbstractScrollArea，列表、树、表格的滚动正是从今天这套机制来的。
