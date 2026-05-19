---
title: "3.3 Model/View 进阶"
description: "入门篇我们用 QListWidget/QTreeWidget/QTableWidget 做了一些增删改查的练习，这些便利类背后都封装了一个内置 Model。但在正式项目里，一旦数据源不是简单的内存列表——比如来自数据库、网络或者复杂的业务对象——你就必须绕过便利类，直接面对 QAbstractItemModel。"
---

# 现代Qt开发教程（进阶篇）3.3——Model/View 进阶

## 1. 前言 / 为什么要自己写 Model

入门篇我们用 QStringListModel 和 QStandardItemModel 做了增删改查，够用了。但我第一次在项目里被逼着继承 QAbstractTableModel，是因为后端给了一组复杂的业务对象，每个有十几个字段还要在界面上做实时状态标记和进度条渲染。QStandardItemModel 每个单元格都是一个堆上的对象，数据量一大就感受到了内存和更新延迟。更别提我的数据源本身就是 C++ struct，把它们逐个塞进 QStandardItem 再取出来改，整个流程多了一层完全没必要的中间映射。

这篇文章我们完整走一遍：继承 QAbstractTableModel 实现全套虚函数、数据变更通知的调用约定、QSortFilterProxyModel 代理排序过滤，最后用自定义 QStyledItemDelegate 把渲染和编辑拿到自己手里。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17。涉及 QtCore（QAbstractTableModel、QSortFilterProxyModel）和 QtWidgets（QStyledItemDelegate、QTableView），原理对 QListView/QTreeView 同样适用。

## 3. 核心概念讲解

### 3.1 QAbstractTableModel 全套虚函数——比你想的多几个

大部分人第一反应是重写 `rowCount()`、`columnCount()` 和 `data()` 就完事。但如果需要编辑、表头自定义或复选框，`setData()`、`headerData()` 和 `flags()` 也必须一并重写。我们先定义一个贯穿全文的业务结构：

```cpp
struct Task {
    QString name;
    int priority;
    int progress;  // 0-100
    bool completed;
};
```

`rowCount()` 返回行数，`columnCount()` 固定返回 4，不赘述。真正的核心是 `data()`——View 每次渲染单元格时调用它，传入 QModelIndex 和 role。同一个单元格 View 会带着不同 role 问很多遍：

```cpp
QVariant data(const QModelIndex &index, int role) const override
{
    if (!index.isValid()) return {};
    const auto &task = m_tasks.at(index.row());
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: return task.name;
        case 1: return QString::number(task.priority);
        case 2: return QString("%1%").arg(task.progress);
        case 3: return task.completed ? "Done" : "Pending";
        }
    }
    if (role == Qt::EditRole && index.column() == 0) return task.name;
    if (role == Qt::CheckStateRole && index.column() == 3)
        return task.completed ? Qt::Checked : Qt::Unchecked;
    if (role == Qt::TextAlignmentRole && index.column() == 2)
        return Qt::AlignCenter;
    return {};
}
```

`data()` 本质是 column x role 的二维分发器。`DisplayRole` 是显示文本，`EditRole` 是编辑初始值，`CheckStateRole` 控制复选框，`TextAlignmentRole` 控制对齐。每个 role 必须显式处理，否则 View 拿到空 QVariant，对应功能静默不工作。

`flags()` 控制交互能力——默认只返回 `ItemIsEnabled | ItemIsSelectable`，不重写的话 `setData()` 写得再漂亮用户也无法触发编辑：

```cpp
Qt::ItemFlags flags(const QModelIndex &index) const override
{
    auto f = QAbstractTableModel::flags(index);
    if (index.column() == 0) f |= Qt::ItemIsEditable;
    if (index.column() == 3) f |= Qt::ItemIsUserCheckable;
    return f;
}
```

`setData()` 是编辑的回写通道。极其关键的细节：修改完数据后必须 emit `dataChanged`，否则 View 不知道数据变了：

```cpp
bool setData(const QModelIndex &index, const QVariant &value, int role) override
{
    if (!index.isValid()) return false;
    auto &task = m_tasks[index.row()];
    if (role == Qt::EditRole && index.column() == 0) {
        task.name = value.toString();
        emit dataChanged(index, index, {role});
        return true;
    }
    if (role == Qt::CheckStateRole && index.column() == 3) {
        task.completed = (value == Qt::Checked);
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}
```

`headerData()` 控制表头文本，只处理 `Qt::Horizontal` + `DisplayRole` 即可。六个虚函数全部到齐，可编辑的表格 Model 就成型了。

### 3.2 数据变更通知——begin/end 函数对的调用约定

`dataChanged` 处理"改一个单元格"。插入行、删除行必须用 `beginInsertRows/endInsertRows` 和 `beginRemoveRows/endRemoveRows` 函数对。严格三步走：先 begin，改数据，最后 end：

```cpp
void addTask(const Task &task) {
    int row = m_tasks.size();
    beginInsertRows(QModelIndex(), row, row);
    m_tasks.append(task);
    endInsertRows();
}
```

`beginInsertRows` 参数：parent（表格 Model 永远传无效 QModelIndex）、first/last 表示新行在插入后模型中的索引范围。插一行则 first=last=目标位置。千万别理解成"从哪插、插几行"——那样 ASSERT 会直接炸。删除行对称，first/last 是要删的行在删除前的索引。如果需要整体替换数据（比如列数变了），用 `beginResetModel()/endResetModel()`。

现在有一道调试题。下面这段 `setData` 里，用户编辑了单元格但界面纹丝不动，问题在哪？

```cpp
bool setData(const QModelIndex &index, const QVariant &value, int role) override
{
    if (!index.isValid()) return false;
    if (role != Qt::EditRole) return false;
    m_tasks[index.row()].name = value.toString();
    return true;
}
```

答案：缺少 `emit dataChanged(index, index, {role})`。`setData` 返回 true 只表示"我接受了修改"，不会自动通知 View。

### 3.3 QSortFilterProxyModel——不碰 Model 的排序和过滤

用户想搜索关键字或按列排序，直接在 Model 里做会让职责膨胀。QSortFilterProxyModel 是插在 Model 和 View 之间的代理层，不持有数据，只做筛选和重排序：

```cpp
auto *proxy = new QSortFilterProxyModel(this);
proxy->setSourceModel(model);
tableView->setModel(proxy);  // View 连 proxy
```

重写 `filterAcceptsRow` 控制哪些行显示。重写 `lessThan` 自定义排序——对数字列尤其重要，默认字符串排序会把 "10" 排在 "2" 前面。关键的调用时机：过滤条件更新后必须调用 `invalidateFilter()` 让代理重新评估所有行，否则界面不更新。`invalidateFilter()` 只重过滤保留排序，`invalidate()` 完全重置两者。数据量大时前者比后者便宜很多。

### 3.4 自定义 QStyledItemDelegate——渲染和编辑的完全控制

需要把某列渲染成进度条或用自定义控件编辑时，就得继承 QStyledItemDelegate。四个核心虚函数：`paint()` 渲染、`createEditor()` 创建编辑器、`setEditorData()` 填数据、`setModelData()` 回写。看 `paint()` 的进度条实现：

```cpp
void paint(QPainter *painter, const QStyleOptionViewItem &option,
           const QModelIndex &index) const override
{
    if (index.column() != 2)
        return QStyledItemDelegate::paint(painter, option, index);
    painter->save();
    auto *style = option.widget ? option.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &option, painter, option.widget);
    int progress = index.data(Qt::EditRole).toInt();
    QStyleOptionProgressBar bar;
    bar.rect = option.rect.adjusted(4, 4, -4, -4);
    bar.minimum = 0; bar.maximum = 100; bar.progress = progress;
    bar.text = QString("%1%").arg(progress); bar.textVisible = true;
    style->drawControl(QStyle::CE_ProgressBar, &bar, painter);
    painter->restore();
}
```

必须先用 `drawControl(CE_ItemViewItem)` 画背景，否则选中行的蓝色高亮不会出现。如果 Model 的 `flags()` 没给对应列加 `ItemIsEditable`，三个编辑函数永远不会被调用。Delegate 的设计哲学是"按列定制"——只重写需要的列，其他回退默认实现。

## 4. 踩坑预防

第一个坑是 `data()` 只处理了 `DisplayRole` 而忽略其他 role。表格"显示正常"时最容易麻痹。后果：不处理 `EditRole` 则编辑初始值为空，不处理 `CheckStateRole` 则复选框不出现，不处理 `TextAlignmentRole` 则数字列全左对齐。`ToolTipRole` 和 `BackgroundRole` 不报错，只是静默不工作。解决方案：按"DisplayRole + EditRole + 该列需要的特殊 role"清单逐项实现。

第二个坑是 `beginInsertRows` 的 first/last 理解错误。这对参数是"新行在插入后模型中的索引范围"。Model 当前 5 行，末尾追加则 first=5 last=5。如果传 first=5 last=1，range 检查会 ASSERT "first cannot be larger than last"。某些边界条件 ASSERT 不触发但 View 内部状态不一致，表现为行数错乱或随机崩溃。解决方案：默念 first/last 是新行索引，插一行就是 first=last。

第三个坑是 QSortFilterProxyModel 过滤条件更新后忘调 `invalidateFilter()`。代码逻辑完全正确，但代理不知道条件变了还在用旧结果，用户输入关键字界面不变。解决方案：把条件更新和 `invalidateFilter()` 封装成一个 `setFilterText` 方法。

## 5. 练习项目

练习项目：带搜索过滤和自定义进度条 Delegate 的任务管理器。基于 QTableView，底层自定义 QAbstractTableModel 管理 Task 结构体（名称、优先级、进度、完成状态），顶层 QSortFilterProxyModel 提供搜索过滤。进度列用 Delegate 渲染为可视化进度条，优先级列用 QSpinBox 限制范围 1-10，状态列支持复选框，上方搜索框实时过滤。完成标准：编辑后视图即时更新，搜索流畅，进度条正确显示，复选框正确切换。

提示：搜索框 `textChanged` 连槽，槽里更新过滤文本并 `invalidateFilter()`；Delegate 的 `paint()` 用 `QStyleOptionProgressBar` 渲染，先 `drawControl(CE_ItemViewItem)` 画背景；`flags()` 给名称列加 `ItemIsEditable`、状态列加 `ItemIsUserCheckable`。

## 6. 官方文档参考链接

[Qt 文档 · QAbstractTableModel](https://doc.qt.io/qt-6/qabstracttablemodel.html) -- 表格 Model 基类，必须重写的虚函数列表

[Qt 文档 · QAbstractItemModel](https://doc.qt.io/qt-6/qabstractitemmodel.html) -- Model 基类完整 API，含 beginInsertRows 等变更通知

[Qt 文档 · QSortFilterProxyModel](https://doc.qt.io/qt-6/qsortfilterproxymodel.html) -- 代理排序过滤，filterAcceptsRow/lessThan 详解

[Qt 文档 · QStyledItemDelegate](https://doc.qt.io/qt-6/qstyleditemdelegate.html) -- 自定义 Delegate 基类，paint/createEditor/setModelData 接口

[Qt 文档 · Model/View Programming](https://doc.qt.io/qt-6/model-view-programming.html) -- 架构完整概述，含自定义 Model 和 Delegate 设计理念

[Qt 文档 · Qt::ItemDataRole](https://doc.qt.io/qt-6/qt.html) -- 数据角色枚举，理解 data() 的 role 参数必备

[Qt 文档 · Model/View Tutorial](https://doc.qt.io/qt-6/modelview.html) -- 官方渐进式教程

---

到这里，自定义 Model、数据变更通知、代理过滤排序、自定义 Delegate 这四块都走了一遍。搞清楚 `data()` 的 role 分发和 begin/end 函数对的调用约定后，剩下的就是按模板填内容。下一篇我们看 QSS 进阶——动态主题切换和复杂选择器。
