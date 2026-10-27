---
title: "3.3 Model/View 进阶"
description: "用户在单元格里改完名字回车确认，表格还是旧值。优先级列排一次序，10 跑到了 2 前面。进度列画上了进度条，选中行的高亮却不见了。这些现象的根子分别在 dataChanged 的调用约定、字符串排序的默认行为和委托绘制的前置步骤里。本篇从继承 QAbstractTableModel 起步，把六个虚函数、begin/end 变更通知、QSortFilterProxyModel 代理过滤排序和自定义 QStyledItemDelegate 完整走一遍。配套示例在 examples/advanced/03-qtwidgets/03-model-view-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.3——Model/View 进阶

[入门篇 Model/View 架构入门](../../beginner/03-qtwidgets/03-model-view-beginner.md) 用 QStringListModel 和 QStandardItemModel 做过增删改查，简单场景确实够用。可正式项目里，数据源往往是一组 C++ 业务对象，每个有十几个字段，界面上还要做实时状态标记和进度条渲染，便利类的代价到这时候就露出来了。QStandardItemModel 的每个单元格都是一个堆上的对象，数据量一大，内存和更新延迟都能感受到。数据本体是现成的 C++ struct，咱们把它们逐个塞进 QStandardItem，要改还得一个个取出来，整个流程多了一层完全没必要的中间映射。

绕开便利类的路只有一条，继承 QAbstractTableModel，把数据留在自己的 struct 里，让 View 隔着一层虚函数来问。咱们把四个环节走完整：全套虚函数怎么写，数据变更通知有什么调用约定，QSortFilterProxyModel 怎么在不碰 Model 的前提下做排序过滤，最后用自定义 QStyledItemDelegate，把渲染和编辑拿到自己手里。

## 六个虚函数：data() 按 column 和 role 分发，漏一个 role 就静默失效

大部分人第一反应是重写 `rowCount()`、`columnCount()` 和 `data()` 就完事。可一旦需要编辑、自定义表头或复选框，`setData()`、`headerData()` 和 `flags()` 也必须一并重写。咱们定义一个贯穿全篇的业务结构：

```cpp
struct Task {
    QString name;
    int priority;
    int progress;  // 0-100
    bool completed;
};
```

`rowCount()` 返回行数，`columnCount()` 固定返回 4，都好理解。咱们把注意力放在 `data()` 上，View 每次渲染单元格都会调它，参数是 QModelIndex 和 role，同一个格子 View 会带着不同 role 问上很多遍：

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
    if (role == Qt::EditRole) {
        switch (index.column()) {
        case 0: return task.name;
        case 1: return task.priority;
        case 2: return task.progress;
        }
    }
    if (role == Qt::CheckStateRole && index.column() == 3)
        return task.completed ? Qt::Checked : Qt::Unchecked;
    if (role == Qt::TextAlignmentRole && index.column() == 2)
        return Qt::AlignCenter;
    return {};
}
```

代码读下来，`data()` 的形状就出来了，一张 column 和 role 的二维分发表，View 拿着坐标来查，没登记的组合返回空 QVariant，对应功能安静地不工作。咱们把最常用的几个 role 摆成一张表：

| role                  | 管的事           | 没处理时的后果     |
| --------------------- | ---------------- | ------------------ |
| Qt::DisplayRole       | 单元格显示文本   | 格子显示为空       |
| Qt::EditRole          | 编辑器的初始值   | 编辑框打开时是空的 |
| Qt::CheckStateRole    | 复选框勾选状态   | 复选框根本不出现   |
| Qt::TextAlignmentRole | 文本对齐方式     | 数字列全左对齐     |
| Qt::ToolTipRole       | 鼠标悬停提示     | 静默失效，不报错   |
| Qt::BackgroundRole    | 单元格背景色     | 静默失效，不报错   |

表格显示正常的时候最容易麻痹。DisplayRole 一通，咱们往往就以为 `data()` 写完了，其余 role 的缺口要等编辑、勾选这些操作真的发生才暴露，暴露的方式还全都不带报错。笔者的做法是按清单逐项过：DisplayRole、EditRole，加上该列需要的特殊 role，写一个勾一个。

`flags()` 管交互能力，基类默认只返回 `ItemIsEnabled | ItemIsSelectable`。咱们不重写它的话，`setData()` 写得再漂亮，用户也没有入口触发编辑：

```cpp
Qt::ItemFlags flags(const QModelIndex &index) const override
{
    auto f = QAbstractTableModel::flags(index);
    if (index.column() == 0) f |= Qt::ItemIsEditable;
    if (index.column() == 3) f |= Qt::ItemIsUserCheckable;
    return f;
}
```

`setData()` 是编辑的回写通道，用户在编辑框里敲完字回车，View 就调它把新值送回来。有个细节您得记牢，改完数据必须 emit `dataChanged`，否则 View 不知道数据变了：

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

`headerData()` 管表头文本，只处理 `Qt::Horizontal` 加 `DisplayRole` 就够用。您把这六个虚函数在同一个类里写齐，一个可编辑的表格 Model 就能跑起来了。

配套示例在 `examples/advanced/03-qtwidgets/03-model-view-advanced/`，`cmake -B build && cmake --build build` 构建后运行。示例里的 TaskTableModel 管三列数据（任务名称、优先级、进度），比本篇的 Task 少一个完成状态列，复选框的功能在示例里没有落地，其余机制都在。咱们双击任务名称列，编辑框打开就带着当前名字，这是 `data()` 里 EditRole 分支的功劳。开场四条任务里，优先级 8 和 9 的两行带浅红背景，鼠标悬停还能看到整条任务的摘要提示。背景和提示两路都出自示例 TaskTableModel 的 BackgroundRole 和 ToolTipRole 分支，正文前面的 data() 片段没收录它们，您要看全就得翻示例源码。数字列的居中对齐则来自 TextAlignmentRole 分支，正文片段只给进度列设了居中，示例把优先级列也一起设了。

## 变更通知是一份调用约定：改格子发 dataChanged，增删行走 begin/end

改一个单元格，靠 `setData()` 里 emit 的 `dataChanged`。插入行、删除行是另一回事，行结构要动，View 得提前知道，必须用 `beginInsertRows/endInsertRows` 和 `beginRemoveRows/endRemoveRows` 函数对把变更包起来。咱们按固定三步走，begin 打头，中间改数据，end 收尾，一步都不能乱：

```cpp
void addTask(const Task &task) {
    int row = m_tasks.size();
    beginInsertRows(QModelIndex(), row, row);
    m_tasks.append(task);
    endInsertRows();
}
```

`beginInsertRows` 的第一个参数 parent，表格 Model 永远传无效 QModelIndex。first 和 last 表示新行在插入后模型中的索引范围，插一行就是 first=last=目标位置。咱们拿具体数字过一遍：Model 当前 5 行，末尾追加，first 和 last 都是 5，因为插入之后新行占住的就是 5 号位。您要是把它理解成“从哪插、插几行”，传成 first=5、last=1，调试构建里的范围检查断言会直接把程序拦下。更要当心的是某些边界条件断言不触发，View 内部状态却已经不一致，表象是行数错乱或随机崩溃。记法就一句，first/last 说的是新行的行号，插一行就是两个相等。

删除行对称，first/last 是要删的行在删除前的索引。您要是需要整体替换数据，比如列数都变了，就换用 `beginResetModel()` 和 `endResetModel()`。

示例里有一组现成的对照实验。点“添加任务”，一条默认优先级 5、进度 0 的新任务当场出现在表格末尾，底部任务总数同步加一。总数标签连着 rowsInserted 和 rowsRemoved 信号，Model 每次增删行，标签都跟着刷新，变更通知的链路看得见。删多行也有一处值得您留意。示例的 View 连的还不是 Model 本身，中间隔着一层代理，下一节咱们细讲它，这里先借用它的一个接口，mapToSource 负责把代理里的行号换算回源模型的行号。main.cpp 把选中行的代理索引用它换回源模型行号，按降序排好再逐行删。删掉一行，排在后面的行号会整体前移，升序删到后面就删错行了，降序删把问题避开。

### 一道调试题：编辑成功，界面纹丝不动

咱们拿一道调试题把 dataChanged 的分量验一遍。`setData` 写成下面这样，用户编辑了单元格，代码返回了 true，界面却纹丝不动，问题出在哪？

```cpp
bool setData(const QModelIndex &index, const QVariant &value, int role) override
{
    if (!index.isValid()) return false;
    if (role != Qt::EditRole) return false;
    m_tasks[index.row()].name = value.toString();
    return true;
}
```

答案：缺了 emit `dataChanged(index, index, {role})`。`setData` 返回 true 只表示“我接受了修改”，它不会自动通知 View，View 拿不到信号，自然继续画旧数据。咱们把缺掉的 emit 补回去，View 才拿得到通知。

## QSortFilterProxyModel：排序过滤在中间层做，Model 一行不改

用户想按关键字搜索、按列排序，直接写进 Model 会让职责膨胀，咱们不想让数据类背上这些界面职责。QSortFilterProxyModel 是插在 Model 和 View 之间的代理层，自己不持有数据，只做筛选和重排序：

```cpp
auto *proxy = new QSortFilterProxyModel(this);
proxy->setSourceModel(model);
tableView->setModel(proxy);  // View 连 proxy
```

上一节删行时借用过的 mapToSource，就是这个类提供的接口。重写 `filterAcceptsRow` 控制哪些行显示，重写 `lessThan` 自定义排序。`lessThan` 对数字列尤其要重写，代理默认按字符串比大小，“10” 会排在 “2” 前面，优先级 10 的任务排到优先级 2 的前面，咱们一眼就能看出不对。

咱们自己维护过滤条件时，还有一条调用约定要记牢。`filterAcceptsRow` 读的条件存在子类的成员变量里，条件更新后必须手动调 `invalidateFilter()`，让代理重新评估所有行，否则代理不知道条件变了，界面继续用旧结果，用户输入关键字毫无反应。笔者建议把条件更新和 `invalidateFilter()` 封装进同一个 setFilterText 方法，想忘都忘不掉。两个重置函数的区别也顺手分清：`invalidateFilter()` 只重过滤、保留排序，`invalidate()` 连排序一起完全重置，数据量大时前者比后者便宜很多。

示例的搜索框走的是另一条路。main.cpp 没有重写 filterAcceptsRow，用的是 Qt 自带的 `setFilterFixedString`，过滤列固定为第 0 列（任务名称列），大小写不敏感。咱们在搜索框里敲两个字，不匹配的行当场消失，清空搜索框，被滤掉的行又全部回来。示例代码里注释特意说明，setFilterFixedString 自己会触发重过滤，不用手动补 invalidateFilter()，手动补的场景属于咱们自己维护成员变量的自定义路线。再点一下表头，还能按列排序（setSortingEnabled(true)），排序同样不区分大小写。示例用的是原装代理，没有重写 lessThan，您点优先级列表头，走的就是前面 lessThan 段警告过的字符串比大小。示例数据里优先级都是个位数，字符串序和数值序暂时分不出差别，优先级一旦出现两位数，10 就会排到 2 前面。

## 自定义 QStyledItemDelegate：渲染和编辑都拿到自己手里

需要把某列渲染成进度条，或者用自定义控件做编辑器时，咱们就得继承 QStyledItemDelegate。四个核心虚函数各管一段：

| 函数            | 职责                    |
| --------------- | ----------------------- |
| paint()         | 渲染单元格              |
| createEditor()  | 创建编辑器控件          |
| setEditorData() | 把 Model 数据填进编辑器 |
| setModelData()  | 把编辑结果写回 Model    |

示例里真正动手写的只有 paint()，咱们把它画进度条的完整实现读下来：

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

咱们留意进度值的取法，`index.data(Qt::EditRole)` 拿的是原始数值，`data()` 对进度列的 DisplayRole 返回的是拼好百分号的字符串，toInt() 要的就是前者。

`paint()` 里必须调 `drawControl(CE_ItemViewItem)` 把单元格背景画出来，否则选中行的蓝色高亮不会出现，进度条画得再漂亮，用户也看不出哪一行正被选中。编辑函数能被调用还有个前提，Model 的 `flags()` 没给对应列加 `ItemIsEditable` 的话，您把三个编辑函数写得再全，createEditor、setEditorData、setModelData 也永远不会被调用，paint() 照常工作，编辑却悄无声息。

Delegate 的设计哲学是“按列定制”，只重写需要的列，其余列回退默认实现，示例的 ProgressDelegate 就是活例子，编辑相关的三个函数整段回退默认行为。装配也只针对一列，main.cpp 里 `tableView->setItemDelegateForColumn(2, progressDelegate)`，进度列从此画成带文字的原生风格进度条。咱们可以做两个实验。一是点住表头按优先级排一次序，进度条跟着各自的行移动。委托每次绘制都从 index 重新取数，排序把行挪到哪儿，进度条就跟到哪儿。二是把某一行选中，进度条所在的格子照样带高亮背景，底色出自 drawControl(CE_ItemViewItem)。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，内容分布在 QtCore（QAbstractTableModel、QSortFilterProxyModel）和 QtWidgets（QStyledItemDelegate、QTableView），原理对 QListView、QTreeView 同样适用。

[Qt 文档 · QAbstractTableModel](https://doc.qt.io/qt-6/qabstracttablemodel.html) -- 表格 Model 基类，必须重写的虚函数列表

[Qt 文档 · QAbstractItemModel](https://doc.qt.io/qt-6/qabstractitemmodel.html) -- Model 基类完整 API，含 beginInsertRows 等变更通知

[Qt 文档 · QSortFilterProxyModel](https://doc.qt.io/qt-6/qsortfilterproxymodel.html) -- 代理排序过滤，filterAcceptsRow/lessThan 详解

[Qt 文档 · QStyledItemDelegate](https://doc.qt.io/qt-6/qstyleditemdelegate.html) -- 自定义 Delegate 基类，paint/createEditor/setModelData 接口

[Qt 文档 · Model/View Programming](https://doc.qt.io/qt-6/model-view-programming.html) -- 架构完整概述，含自定义 Model 和 Delegate 设计理念

[Qt 文档 · Qt::ItemDataRole](https://doc.qt.io/qt-6/qt.html) -- 数据角色枚举，理解 data() 的 role 参数必备

[Qt 文档 · Model/View Tutorial](https://doc.qt.io/qt-6/modelview.html) -- 官方渐进式教程

---

Model/View 的进阶就到这里。想再动手的，请您把示例的任务表完整玩一轮：双击名称列改名，追一遍 EditRole 给初始值、setData 回写、dataChanged 通知 View 的全链路，点“添加任务”再选中删掉两行，对照底部总数标签验证 begin/end 函数对，最后在搜索框敲关键字，看代理层怎么把不匹配的行收走。上一篇 [3.2 事件处理进阶](./02-event-handling-advanced.md) 讲事件的分发与传播，下一篇 [3.4 QSS 进阶](./04-qss-advanced.md) 讲动态主题切换和复杂选择器。
