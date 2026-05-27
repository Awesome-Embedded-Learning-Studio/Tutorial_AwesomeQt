---
title: "3.52 QHeaderView 进阶"
description: "入门篇我们用 QHeaderView 配合 QTableView/QTreeView 做了列头点击排序、列宽拖动调整，掌握了 setSectionResizeMode/setSortIndicatorShown/clicked 信号。"
---

# 现代Qt开发教程（进阶篇）3.52——QHeaderView 进阶

## 1. 前言 / 当表头不只是一行文字

入门篇我们用 QHeaderView 配合 QTableView/QTreeView 做了列头点击排序、列宽拖动调整，掌握了 setSectionResizeMode / setSortIndicatorShown / clicked 信号。大多数表格的表头就是一行文字，点击排序，拖动调宽——入门篇完全够用。但如果你需要"多级表头"（比如第一级是"上半年/下半年"，第二级是"Q1/Q2/Q3/Q4"），或者需要在表头中嵌入自定义控件（比如表头中放一个筛选下拉框），QHeaderView 的基础接口就不够了。

今天我们把 QHeaderView 的两个高级能力拆透：多级表头的实现方案和自定义排序逻辑。

## 2. 环境说明

本篇基于 Qt 6.5+ 版本，CMake 3.26+，C++17 标准。QHeaderView 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。所有行为在 Qt 6.9.1 上验证通过。

## 3. 核心概念讲解

### 3.1 多级表头的实现方案

QHeaderView 本身不支持多级表头——它只有一级 section，没有"section 的 section"这种嵌套概念。实现多级表头的标准方案是重写 QHeaderView 的 paintEvent，在现有表头上方手动绘制上级表头。

核心思路是这样的：创建一个 GroupHeaderView 继承自 QHeaderView，维护一个"分组"数据结构记录每个上级表头跨越哪些子 section。在 paintEvent 中先调 QHeaderView::paintEvent 画正常的单级表头，然后在表头上方额外画上级表头的文字和边框。

```cpp
struct HeaderGroup {
    QString title;
    int start_section;
    int span;
};

class GroupHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    explicit GroupHeaderView(Qt::Orientation orientation,
                             QWidget *parent = nullptr)
        : QHeaderView(orientation, parent)
    {
        // 上级表头高度
        m_group_height = 30;
    }

    void addGroup(const QString &title,
                  int start, int span)
    {
        m_groups.append({title, start, span});
        // 增加 header 的总高度以容纳上级表头
        int total_height = m_group_height + defaultSectionSize();
        setFixedHeight(total_height);
        viewport()->update();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QHeaderView::paintEvent(event);

        QPainter painter(viewport());
        painter.setRenderHint(QPainter::Antialiasing, false);

        // 在表头上方绘制分组
        int y = 0;
        int h = m_group_height;

        for (const auto &group : m_groups) {
            int x = sectionViewportPosition(group.start_section);
            int w = 0;
            for (int i = 0; i < group.span; ++i) {
                w += sectionSize(group.start_section + i);
            }

            // 绘制分组背景和边框
            painter.fillRect(x, y, w, h,
                QColor(240, 240, 240));
            painter.setPen(QPen(QColor(200, 200, 200)));
            painter.drawRect(x, y, w, h);

            // 绘制分组文字
            painter.setPen(QColor(60, 60, 60));
            painter.drawText(QRect(x, y, w, h),
                Qt::AlignCenter, group.title);
        }
    }

    void paintSection(QPainter *painter, const QRect &rect,
                      int logical_index) const override
    {
        // 将底层表头绘制位置下移
        QRect adjusted = rect.translated(0, m_group_height);
        QHeaderView::paintSection(painter, adjusted, logical_index);
    }

    QSize sizeHint() const override
    {
        QSize hint = QHeaderView::sizeHint();
        hint.setHeight(m_group_height
                       + defaultSectionSize());
        return hint;
    }

private:
    int m_group_height = 30;
    QVector<HeaderGroup> m_groups;
};
```

使用时创建 GroupHeaderView 替换默认的水平表头：

```cpp
auto *header = new GroupHeaderView(Qt::Horizontal);
tableView->setHorizontalHeader(header);

// 添加分组："上半年"覆盖列 0-2，"下半年"覆盖列 3-5
header->addGroup("上半年", 0, 3);
header->addGroup("下半年", 3, 3);

// 添加子列
model->setHorizontalHeaderLabels(
    {"Q1", "Q2", "Q3", "Q4", "Q5", "Q6"});
```

这个方案有一个局限：上级表头不支持交互——没有点击排序、没有拖动调宽。如果你需要上级表头也支持排序，需要在 paintEvent 中检测鼠标点击位置，判断落在哪个分组上，然后对该分组下的所有列做排序。

另一种实现多级表头的方案是在表格顶部插入额外的行作为表头——把表头数据作为普通数据行放在 model 的前几行。这种方案更简单但不灵活——表头行会和数据行一起滚动，表头行的样式需要通过 delegate 自定义，而且排序时表头行会被排到数据中去。

### 3.2 自定义排序逻辑

QHeaderView 的 sortIndicatorChanged(int logicalIndex, Qt::SortOrder) 信号在用户点击列头时触发。默认行为是调用 QAbstractItemModel::sort() 进行排序。但 model 的 sort() 默认实现是按字符串字典序排序——"100"排在"20"前面，因为"1" < "2"。如果你需要按数值排序，需要在 model 中重写 sort() 方法。

```cpp
void sort(int column, Qt::SortOrder order) override
{
    // 构建排序用的索引列表
    QVector<QPair<QVariant, int>> sort_data;
    sort_data.reserve(rowCount());
    for (int i = 0; i < rowCount(); ++i) {
        QVariant val = index(i, column).data(Qt::EditRole);
        sort_data.append({val, i});
    }

    // 根据数据类型选择排序方式
    std::sort(sort_data.begin(), sort_data.end(),
        [order](const auto &a, const auto &b) {
        if (a.first.userType() == QMetaType::Int) {
            return order == Qt::AscendingOrder
                ? a.first.toInt() < b.first.toInt()
                : a.first.toInt() > b.first.toInt();
        }
        return order == Qt::AscendingOrder
            ? a.first.toString() < b.first.toString()
            : a.first.toString() > b.first.toString();
    });

    // 重新排列数据
    // ... emit layoutChanged ...
}
```

对于 QSortFilterProxyModel，排序逻辑通过 lessThan 虚函数定制。QSortFilterProxyModel 的 sort() 方法内部会调用 lessThan 来比较两个 QModelIndex 的值。

```cpp
class NumericSortProxy : public QSortFilterProxyModel
{
    Q_OBJECT

protected:
    bool lessThan(const QModelIndex &left,
                  const QModelIndex &right) const override
    {
        QVariant left_data = left.data(Qt::EditRole);
        QVariant right_data = right.data(Qt::EditRole);

        bool ok_left = false, ok_right = false;
        double left_num = left_data.toDouble(&ok_left);
        double right_num = right_data.toDouble(&ok_right);

        if (ok_left && ok_right) {
            return left_num < right_num;
        }
        // 非数值按字符串比较
        return QSortFilterProxyModel::lessThan(left, right);
    }
};
```

使用 NumericSortProxy 替代默认的 proxy model 后，点击列头排序时会自动按数值大小排序而不是按字符串字典序。

## 4. 踩坑预防

第一个坑是多级表头的 sectionViewportPosition 在列被隐藏时返回负值。如果你在分组中引用的某个 section 被 hideSection 隐藏了，sectionViewportPosition 返回 -1 或者不正确的值。后果是分组的绘制位置错误，分组文字出现在表格外面。解决方案是在绘制分组时检查 section 的 visible 状态，跳过被隐藏的 section。

第二个坑是 QSortFilterProxyModel 的排序在大数据量下性能不够。QSortFilterProxyModel::sort() 内部对 model 的所有行做一次完整排序。对于百万行数据，排序需要 O(N log N) 次比较，每次比较调用 lessThan，lessThan 又调用 sourceModel->data()——总计数百万次 data() 调用。如果你的 data() 是 O(1) 的还好，如果是 O(log N) 的数据库查询，排序可能需要几分钟。解决方案是在 model 端直接排序（重写 model 的 sort()），避免 proxy 层的额外开销。

第三个坑是 setSortIndicatorShown(true) 但实际没有排序。sort indicator 只是一个视觉标记——在列头上画一个上/下箭头。它不会自动触发排序。如果你手动设了 setSortIndicator(column, order) 但没有调用 model->sort()，列头上会出现排序箭头但数据没排。解决方案是确保 sort indicator 和 model->sort() 总是同步调用。

## 5. 练习项目

练习项目：多级表头的学生成绩表。我们要实现一个包含两级表头的成绩展示表格。

完成标准是：QTableView 配合自定义 QAbstractTableModel。水平表头使用 GroupHeaderView，两级结构：上级"文科"覆盖"语文/历史/地理"三列，"理科"覆盖"数学/物理/化学"三列。子列可以按数值排序（使用 QSortFilterProxyModel 重写 lessThan）。数据包含 20 名学生的成绩，每列随机生成 60-100 的整数分数。点击子列排序时按分数数值排序而不是字符串排序。上级分组列不响应点击排序。

提示几个关键点：GroupHeaderView 的 addGroup 设置分组范围；NumericSortProxy 重写 lessThan 对数值列做数值比较；setSectionsClickable(true) 让子列可点击排序。

## 6. 官方文档参考链接

[Qt 文档 · QHeaderView](https://doc.qt.io/qt-6/qheaderview.html) -- 表头控件，包含 setSectionResizeMode/setSortIndicator/sectionResized 等接口

[Qt 文档 · QSortFilterProxyModel](https://doc.qt.io/qt-6/qsortfilterproxymodel.html) -- 排序过滤代理模型，lessThan 用于自定义排序比较

[Qt 文档 · QAbstractItemModel::sort](https://doc.qt.io/qt-6/qabstractitemmodel.html#sort) -- 模型排序虚函数

---

到这里，QHeaderView 的进阶内容就拆完了。多级表头没有内置支持，但通过重写 paintEvent 在表头上方额外绘制分组是标准方案。自定义排序的关键是区分字符串排序和数值排序——QSortFilterProxyModel::lessThan 是最干净的介入点。把这些搞透后，你就能做出专业的多级表头表格——而不只是"一行文字能点击排序"的简单表头。
