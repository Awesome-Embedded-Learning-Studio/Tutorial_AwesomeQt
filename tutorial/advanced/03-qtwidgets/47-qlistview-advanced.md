---
title: "3.47 QListView 进阶"
description: "入门篇我们用 QListView 搭了基本的列表视图，配合 QStringListModel 展示数据，掌握了 setModel/setCurrentIndex/clicked 信号、setViewMode 切换列表和图标模式，以及 setAlternatingRowColors 交替行着色。"
---

# 现代Qt开发教程（进阶篇）3.47——QListView 进阶

## 1. 前言 / 当列表要扛住百万行数据

入门篇我们用 QListView 搭了基本的列表视图，配合 QStringListModel 展示数据，掌握了 setModel / setCurrentIndex / clicked 信号、setViewMode 切换列表和图标模式，以及 setAlternatingRowColors 交替行着色。对于几十到几百条数据的简单列表，入门篇那一套完全够用。但 QListView 真正的实力在于它的虚拟化机制——它不会为列表中的每一项都创建对应的 widget 或者做一次完整渲染，而是只为当前可见区域内的项调用 delegate 的 paint 方法。这意味着即使你的 model 里有一百万行数据，QListView 也只需要渲染屏幕上可见的那几十行。

今天我们把 QListView 的虚拟列表机制和大数据优化拆透。核心内容是四个方面：QListView 的虚拟化原理与可见区域计算、setUniformItemSizes 对布局性能的影响、QAbstractItemModel 的 canFetchMore/fetchMore 延迟加载机制，以及 batchSize 和 layoutMode 对初始化性能的调优。

## 2. 环境说明

本篇基于 Qt 6.5+ 版本，CMake 3.26+，C++17 标准。QListView 和 QAbstractItemModel 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。所有行为在 Qt 6.9.1 上验证通过。

## 3. 核心概念讲解

### 3.1 虚拟化原理与可见区域计算

QListView 的虚拟化核心是一个简单但关键的设计：它只为可见区域内的 QModelIndex 调用 delegate 的 paint() 方法。当用户滚动列表时，QListView 计算新的可见范围，回收离开可见区域的项的渲染资源，为新进入可见区域的项调用 paint。

这个计算过程大致是这样的：QListView 内部维护了一个从 model index 到像素位置的映射。对于垂直滚动（LeftToRight flow，TopToBottom layout），它需要知道每个项的高度来计算第 N 个项在什么 Y 坐标。如果所有项高度相同（uniformItemSizes = true），计算就是 `index * itemHeight`，O(1) 复杂度。如果项高度不同，QListView 需要从第一个项开始逐个累加高度来确定第 N 个项的位置——这就是为什么非均匀高度列表的滚动定位比均匀高度列表慢。

```cpp
// 模拟 QListView 内部的可见区域计算
int viewport_height = viewport()->height();
int scroll_offset = verticalScrollBar()->value();

// 找到第一个可见项
int first_visible = scroll_offset / uniform_height;
// 找到最后一个可见项
int last_visible = (scroll_offset + viewport_height) / uniform_height + 1;
last_visible = qMin(last_visible, model()->rowCount() - 1);

// 只对可见项调用 delegate->paint()
for (int i = first_visible; i <= last_visible; ++i) {
    // 获取 index，调用 paint
}
```

这意味着不管你的 model 有多少行数据——10 行也好、10 万行也好、100 万行也好——QListView 每帧渲染的项数量是固定的（取决于视口大小和项高度）。这就是虚拟化的核心优势：渲染复杂度是 O(可见项数) 而不是 O(总项数)。

但有一个前提：model 的 rowCount() 必须 O(1) 返回总数，data() 方法在接收到请求时必须快速返回。如果你的 rowCount() 每次都要遍历一个巨大数据结构来计数，或者 data() 做了重量级的磁盘 IO，虚拟化的优势就被抵消了。

### 3.2 setUniformItemSizes 的性能影响

setUniformItemSizes(bool) 是 QListView 最重要的性能开关。当设为 true 时，QListView 假设所有列表项的 sizeHint 返回值完全相同。这带来两个优化：第一，滚动定位不再需要逐项累加高度，直接用 `index * fixedSize` 计算；第二，QListView 可以缓存一个项的 sizeHint 结果复用给所有项，而不是每个项都调一次 sizeHint。

```cpp
// 最简单的优化：所有项大小相同时开启
listView->setUniformItemSizes(true);
listView->setGridSize(QSize(200, 40));  // 固定网格大小
```

setGridSize 为列表设置一个固定的网格大小，每个项占据一个格子。这比 uniformItemSizes 更进一步——不仅高度固定，宽度也固定，布局计算更简单。setGridSize 和 setViewMode(QListView::IconMode) 配合使用可以实现图标网格布局。

当 uniformItemSizes = false 时，QListView 需要维护一个完整的项位置索引表。每次 model 发生变化（行插入、删除、重置），QListView 都要重新计算整个索引表。对于百万行数据，这个重算过程可能需要几百毫秒，表现为界面在数据更新后短暂卡顿。

### 3.3 canFetchMore/fetchMore 延迟加载

如果你的数据源本身很大——比如数据库查询结果有 100 万行——一次性加载到内存是不现实的。QAbstractItemModel 提供了 canFetchMore / fetchMore 机制来实现延迟加载：model 初始只加载一部分数据，当用户滚动到列表底部时 QListView 自动调用 fetchMore 追加更多数据。

```cpp
class LazyModel : public QAbstractListModel
{
    Q_OBJECT

public:
    int rowCount(const QModelIndex & = {}) const override
    {
        return m_loaded_count;
    }

    bool canFetchMore(const QModelIndex & = {}) const override
    {
        return m_loaded_count < m_total_count;
    }

    void fetchMore(const QModelIndex & = {}) override
    {
        int remaining = m_total_count - m_loaded_count;
        int to_fetch = qMin(remaining, kBatchSize);
        beginInsertRows({}, m_loaded_count,
                        m_loaded_count + to_fetch - 1);
        m_loaded_count += to_fetch;
        endInsertRows();
    }

    QVariant data(const QModelIndex &index,
                  int role) const override
    {
        if (role != Qt::DisplayRole) return {};
        // 从数据源读取第 index.row() 项
        return fetchData(index.row());
    }

private:
    static constexpr int kBatchSize = 100;
    int m_total_count = 0;
    int m_loaded_count = 0;
};
```

canFetchMore 返回 true 表示还有更多数据可以加载。QListView 在滚动到接近底部时检查 canFetchMore，如果为 true 就调用 fetchMore。beginInsertRows / endInsertRows 通知 QListView 有新行插入。

这个机制的优势是内存占用可控——不管数据源有多大，内存中始终只有 `m_loaded_count` 条数据。劣势是 rowCount() 返回的不是真实总数，而是已加载数量，这会影响滚动条的范围——用户看到的滚动条最大位置只反映已加载的数据，而不是全部数据。如果你想让滚动条反映真实总数，可以在 model 的 rowCount 中直接返回 m_total_count，但在 data 中只处理已加载范围内的 index——未加载的 index 返回空数据。代价是滚动到未加载区域时会触发大量 fetchMore 调用。

### 3.4 batchSize 和 layoutMode 调优

QListView 在初始化时（model 第一次 setModel 或者 model 重置后）需要计算所有项的布局位置。对于大数据量的 model，这个过程可能很耗时。QListView 提供了两个属性来优化这个过程。

setLayoutMode(QListView::LayoutMode) 控制布局计算的时机。Batched 模式下 QListView 不会一次性计算所有项的位置，而是分批次进行——先计算第一批（由 setBatchSize 控制），渲染出来，然后在空闲时继续计算后续批次。这样用户不需要等所有项都布局完就能开始交互。

```cpp
listView->setLayoutMode(QListView::Batched);
listView->setBatchSize(100);  // 每批处理 100 项
```

默认的 layoutMode 是 SinglePass——一次性计算所有项。对于几千项以内的列表，SinglePass 和 Batched 的体感差异不大。但当项数超过一万时，SinglePass 会在 setModel 时卡住 UI 主线程数百毫秒甚至数秒（取决于项的复杂度），而 Batched 能让界面几乎立即响应。

另一个相关属性是 setViewportMargins——如果你的列表头部或底部有固定的装饰区域（比如一个搜索栏），需要通过 setViewportMargins 预留空间，否则 QListView 的虚拟化计算会把装饰区域也算进可见区域，导致可见项数量计算偏少。

## 4. 踩坑预防

第一个坑是非均匀项高度导致滚动定位性能下降。当 uniformItemSizes = false 时，QListView 需要维护完整的项位置索引。每次 model 变化（insert/remove/reset）都要重算这个索引。对于 10 万项以上的列表，重算可能需要上百毫秒。后果是用户添加/删除一个项后界面短暂卡顿。解决方案是尽量使用 uniformItemSizes = true。如果必须用非均匀高度，确保你的 model 的 data() 和 sizeHint 都 O(1) 返回。

第二个坑是 canFetchMore 的批量大小设太小导致滚动时频繁触发。如果 batchSize = 10，用户快速滚动时每滚过一个批次就触发一次 fetchMore，每次 fetchMore 触发 beginInsertRows/endInsertRows，引发一次布局重算。频繁的布局重算会导致滚动卡顿。建议 batchSize 设为 200-500 之间。

第三个坑是 QListView 的 scrollMode 默认是 ScrollPerPixel 而不是 ScrollPerItem。ScrollPerPixel 允许像素级精确滚动（视觉更流畅），但需要精确的项高度计算。如果项高度不准确（比如 sizeHint 和实际渲染大小不一致），ScrollPerPixel 模式下会出现项之间的间隙或重叠。确保 sizeHint 返回的值和 delegate 实际绘制的大小一致。

## 5. 练习项目

练习项目：百万行数据浏览器。我们要实现一个能流畅展示大量数据的列表视图。

完成标准是：创建一个 LazyModel 继承 QAbstractListModel，模拟 100 万行数据（data() 返回"行号: 内容-{hash}"格式的字符串，按需生成不需要预存）。QListView 配置 uniformItemSizes=true、layoutMode=Batched、batchSize=200。LazyModel 初始加载 500 行，canFetchMore/fetchMore 每次追加 200 行。窗口顶部一个 QLabel 实时显示"已加载: {count} / 1,000,000"。QListView 右侧滚动条能正确反映已加载数据的范围。点击任意列表项在底部状态栏显示该项的完整信息。

提示几个关键点：LazyModel 的 data() 用 index.row() 生成字符串，不需要 QVector 存储；fetchMore 里的 beginInsertRows/endInsertRows 参数是连续范围；uniformItemSizes=true 配合固定行高让滚动定位 O(1)。

## 6. 官方文档参考链接

[Qt 文档 · QListView](https://doc.qt.io/qt-6/qlistview.html) -- 列表视图，包含 setUniformItemSizes/setBatchSize/setLayoutMode 等接口

[Qt 文档 · QAbstractItemModel](https://doc.qt.io/qt-6/qabstractitemmodel.html) -- 抽象模型基类，包含 canFetchMore/fetchMore/rowCount/data

[Qt 文档 · QAbstractItemView](https://doc.qt.io/qt-6/qabstractitemview.html) -- 抽象视图基类，包含 setModel/scrollTo 等通用视图接口

---

到这里，QListView 的进阶内容就拆完了。虚拟化的核心就是只为可见项做渲染，不管 model 有多少数据。setUniformItemSizes 是最重要的性能开关——能用就用。canFetchMore/fetchMore 让大数据集的内存占用可控。Batched 布局模式让大数据初始化不卡界面。掌握了这些，你就能用 QListView 扛住百万级的数据展示——而不只是一个"几十行数据的列表"。
