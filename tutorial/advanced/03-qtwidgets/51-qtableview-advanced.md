---
title: "3.51 QTableView 进阶"
description: "入门篇我们用 QTableView 配合 QStandardItemModel 展示了基本的表格数据，掌握了 setModel/setCurrentIndex/clicked 信号、setShowGrid/setAlternatingRowColors 等视觉配置。"
---

# 现代Qt开发教程（进阶篇）3.51——QTableView 进阶

## 1. 前言 / 当表格要扛住百万行

入门篇我们用 QTableView 配合 QStandardItemModel 展示了基本的表格数据，掌握了 setModel / setCurrentIndex / clicked 信号、setShowGrid / setAlternatingRowColors 等视觉配置。和 QListView 一样，QTableView 的核心优势是虚拟化——它只渲染可见区域内的单元格，不管你的 model 有一百行还是一百万行。但 QTableView 的虚拟化比 QListView 复杂——它是二维的，需要同时计算可见的行范围和列范围，而且列宽可能各不相同，行高也可能不均匀。

今天我们把 QTableView 的虚拟滚动性能优化拆透。核心内容是三个方面：行高列宽的缓存与 uniform 计算、QAbstractTableModel 的高效实现要点，以及大数据场景下的内存优化策略。

## 2. 环境说明

本篇基于 Qt 6.5+ 版本，CMake 3.26+，C++17 标准。QTableView 和 QAbstractTableModel 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。所有行为在 Qt 6.9.1 上验证通过。

## 3. 核心概念讲解

### 3.1 行高列宽的缓存与 uniform 计算

QTableView 的虚拟滚动依赖于精确的行高和列宽计算。对于垂直滚动，它需要知道每行的像素高度来定位第 N 行在视口中的 Y 坐标。对于水平滚动，它需要知道每列的像素宽度来定位第 M 列在视口中的 X 坐标。

默认情况下 QTableView 通过 QHeaderView 管理行高和列宽。QHeaderView 为每个 section（行或列）维护一个独立的尺寸值。当用户拖动行边界或列边界时，QHeaderView 更新对应 section 的尺寸。

当所有行高度相同（这是绝大多数表格的情况），你可以通过 QHeaderView 的 setDefaultSectionSize 设置一个全局默认行高。这样 QHeaderView 不需要为每一行单独存储尺寸值，内存占用从 O(rows) 降到 O(1)。

```cpp
// 所有行高度固定 30 像素
tableView->verticalHeader()->setDefaultSectionSize(30);
// 隐藏行号（如果不需要的话）
tableView->verticalHeader()->hide();
```

隐藏 verticalHeader（行号列）有两个好处：省去了行号渲染的开销，同时消除了行号列的宽度计算。对于百万行数据，这个优化虽然小但有意义。

列宽方面，QHeaderView::setSectionResizeMode 是最关键的配置。

```cpp
auto *header = tableView->horizontalHeader();

// 性能最优的配置：固定列宽
header->setSectionResizeMode(QHeaderView::Fixed);
header->setDefaultSectionSize(120);

// 实用且性能好：第一列交互式调整，其余固定
header->setSectionResizeMode(0, QHeaderView::Interactive);
for (int i = 1; i < header->count(); ++i) {
    header->setSectionResizeMode(i, QHeaderView::Fixed);
}
```

避免使用 ResizeToContents 模式处理大数据表格——这个模式会让 QHeaderView 对每一列的每个单元格都调用 sizeHint 来计算该列的最佳宽度。对于百万行数据，这就是百万次 sizeHint 调用，每列一次，初始化阶段会严重卡顿。

### 3.2 QAbstractTableModel 的高效实现

百万行数据的表格必须用自定义 QAbstractTableModel 而不是 QStandardItemModel。QStandardItemModel 为每个单元格创建一个 QStandardItem 对象——百万行 × 5 列 = 五百万个 QStandardItem，每个对象有十几字节的成员变量，总内存轻松超过 200MB。而且初始化时五百万次 new 操作本身就可能需要几秒。

自定义 QAbstractTableModel 的核心原则是：data() 方法按需计算返回值，不预存所有数据。

```cpp
class LargeTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    int rowCount(const QModelIndex & = {}) const override
    {
        return m_row_count;
    }

    int columnCount(const QModelIndex & = {}) const override
    {
        return m_col_count;
    }

    QVariant data(const QModelIndex &index,
                  int role) const override
    {
        if (!index.isValid() || role != Qt::DisplayRole) {
            return {};
        }
        // 按需生成，不需要存储
        return generateCellData(index.row(), index.column());
    }

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const override
    {
        if (role != Qt::DisplayRole) return {};
        if (orientation == Qt::Horizontal) {
            return m_headers.value(section, QString("Col %1").arg(section));
        }
        return QString::number(section + 1);
    }

private:
    QString generateCellData(int row, int col) const
    {
        // 如果数据在数据库中，这里做按需查询
        // 如果数据可以计算生成，这里按需计算
        return QString("R%1C%2").arg(row + 1).arg(col + 1);
    }

    int m_row_count = 1000000;
    int m_col_count = 5;
    QStringList m_headers = {"ID", "名称", "类型", "状态", "更新时间"};
};
```

这个 model 的内存占用是 O(1)——不管 rowCount 有多大，model 只存储 m_row_count、m_col_count 和 m_headers 这几个变量。data() 方法在 QTableView 需要渲染某个单元格时被调用，返回对应的值。如果数据源是数据库，data() 中可以做 `SELECT value FROM table WHERE row=? AND col=?` 的单点查询。关键是每次查询要快——O(1) 或 O(log N)。

如果你的数据源确实需要预加载一部分数据到内存中，使用分页缓存策略：只缓存当前可见区域附近的数据（比如可见行 ± buffer 行），滚动时淘汰远离可见区域的数据、加载新进入缓冲区的数据。

### 3.3 大数据场景下的内存优化

除了 model 的内存占用，QTableView 本身也有一些内存相关的配置需要注意。

setSpan 在大数据表格中要谨慎使用。QTableView 内部维护一个 span 的 QVector，每个 span 需要存储行列位置和跨距。如果你有大量 span（比如每 10 行一个分组表头），内存占用会线性增长。更重要的是，setSpan 会影响虚拟滚动的行高计算——合并行的高度需要对被合并行的行高求和，这比固定行高的 O(1) 计算慢。

setWordWrap(true) 会让 QTableView 对每个可见单元格做文字换行计算——它需要调用 QFontMetrics 计算文字在固定宽度内需要几行，然后更新行高。对于百万行数据，如果列宽频繁变化（比如 ResizeToContents 模式），每次变化都可能触发大量文字换行重算。建议在大数据表格中关闭 wordWrap，用省略号代替。

```cpp
tableView->setWordWrap(false);
// 默认的 elideMode 是 ElideRight，超长文字右侧显示"..."
```

最后是 setAttribute(Qt::WA_OpaquePaintEvent, true)——这个属性告诉 Qt 这个 widget 的 paintEvent 会覆盖整个绘制区域，不需要在 paintEvent 之前先擦除背景。对于 QTableView 这类每帧都完整重绘的控件，省掉背景擦除可以减少一次完整的区域填充操作。但对于有透明背景或者部分区域不绘制的自定义 delegate，这个属性可能导致残影。

## 4. 踩坑预防

第一个坑是 QStandardItemModel 存储百万行数据导致内存爆炸。QStandardItemModel 为每个单元格创建一个 QStandardItem 对象。百万行 × 5 列 = 五百万个对象，每个对象几十字节，总计数百 MB。后果是启动慢（五百万次 new）、占用大、滚动时 model 的 data() 虽然是 O(1) 但内存压力导致系统整体变慢。解决方案是使用自定义 QAbstractTableModel，data() 按需生成或查询。

第二个坑是 ResizeToContents 模式在大数据表格中导致初始化卡顿。ResizeToContents 需要遍历每列的每个单元格来计算最大宽度。百万行 × 5 列 = 五百万次 sizeHint 调用。后果是 setModel 后 UI 冻结数秒甚至更久。解决方案是用 Fixed 或 Interactive 模式，手动设置合理的列宽。

第三个坑是垂直滚动条范围溢出。QScrollBar 的 maximum 值是 int 类型。如果你的行高 × 行数超过 INT_MAX（约 21 亿），滚动条范围会溢出。对于 30 像素行高，大约 7100 万行就会溢出。如果确实需要更大的范围，需要在 model 中做分页处理——rowCount 返回当前页的行数，而不是全部数据。

## 5. 练习项目

练习项目：百万行数据表格浏览器。我们要实现一个能流畅展示大量数据的表格视图。

完成标准是：自定义 LargeTableModel 继承 QAbstractTableModel，模拟 100 万行 × 4 列数据。列名："ID"、"名称"、"分类"、"状态"。data() 按行号和列号即时生成字符串（不需要存储）。QTableView 配置：uniform 行高 28px、隐藏行号、固定列宽（ID:80px, 名称:200px, 分类:120px, 状态:100px）、关闭 wordWrap。表格顶部一个 QLineEdit 搜索框，输入行号后按回车滚动到对应行（使用 scrollTo）。滚动到目标行后该行背景高亮 3 秒。窗口大小 800x600，帧率不低于 60fps。

提示几个关键点：verticalHeader()->setDefaultSectionSize(28) 设固定行高；scrollTo(model->index(target_row, 0), QAbstractItemView::PositionAtCenter) 滚动到目标行居中显示；高亮用自定义 delegate 的 paint 或者临时修改 backgroundRole。

## 6. 官方文档参考链接

[Qt 文档 · QTableView](https://doc.qt.io/qt-6/qtableview.html) -- 表格视图，包含 setWordWrap/setSpan/scrollTo 等接口

[Qt 文档 · QAbstractTableModel](https://doc.qt.io/qt-6/qabstracttablemodel.html) -- 表格模型基类，rowCount/columnCount/data/headerData

[Qt 文档 · QHeaderView](https://doc.qt.io/qt-6/qheaderview.html) -- 表头控件，setSectionResizeMode/setDefaultSectionSize 控制列宽行高策略

---

到这里，QTableView 的进阶内容就拆完了。百万行数据的核心就是：自定义 QAbstractTableModel、data() 按需返回、固定行高列宽避免逐项计算。QStandardItemModel 在大数据场景下是内存杀手，坚决不用。ResizeToContents 是性能陷阱，改用 Fixed 或 Interactive。掌握这些原则后，你就能让 QTableView 流畅地展示任何规模的数据。
