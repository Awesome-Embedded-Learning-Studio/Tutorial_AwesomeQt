# 现代Qt开发教程（新手篇）3.15——QAbstractItemView：视图基类

## 1. 前言 / 为什么你需要搞懂 QAbstractItemView

我们在 Qt 的 Model/View 架构里已经摸过好几回了——QListView、QTableView、QTreeView 这三个视图控件你大概率都用过。但不知道你有没有翻过它们的继承关系：这三个控件全部直接继承自 QAbstractItemView。QAbstractItemView 把"视图如何跟模型通信""用户如何选择项目""如何把绘制和编辑委托给自定义类"这三件事全部封装在了一层基类里。你在 QListView 上调用的 `setModel()`，在 QTableView 上用的 `selectionModel()`，在 QTreeView 上设置的委托——它们的入口全部定义在 QAbstractItemView 上。

说实话，大部分初学 Qt 的朋友对 QAbstractItemView 的态度跟对 QAbstractButton 差不多——知道有这么个基类，但觉得"我直接用 QTableView 就行了，干嘛要关心基类"。这种想法在做简单表格的时候确实没什么问题，但当你需要控制选择行为、拦截编辑事件、甚至自定义一个完全不同的视图控件时，不了解 QAbstractItemView 提供的那些接口会让你寸步难行。这篇文章我们就把 QAbstractItemView 的四个核心维度讲清楚：模型绑定与选择模型、选择模式与行为、获取当前选中项的几个方法、以及委托机制的基本使用。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QAbstractItemView 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。QAbstractItemView 的选择行为在所有桌面平台上一致，不存在平台相关的特殊行为。有一点需要注意：本篇涉及到的 QItemSelectionModel 和 QStyledItemDelegate 也都在 QtWidgets 模块内，不需要额外链接其他模块。如果你在之前的章节已经跑通了一个 QtWidgets 项目，本篇的代码直接编译运行即可，零额外依赖。

## 3. 核心概念讲解

### 3.1 setModel() / setSelectionModel()：绑定模型与选择模型

QAbstractItemView 最核心的接口就是 `setModel(QAbstractItemModel *model)`。它把一个数据模型绑定到视图上，视图随后会从模型中读取数据来渲染内容。你可以传入任何继承自 QAbstractItemModel 的模型——QStringListModel、QStandardItemModel、QSqlTableModel，或者你自己实现的 custom model。调用 `setModel()` 之后，视图会自动连接模型的 `dataChanged`、`rowsInserted`、`rowsRemoved` 等信号，实时反映数据的变化。

```cpp
auto *model = new QStandardItemModel(this);
model->setHorizontalHeaderLabels({"姓名", "年龄", "城市"});
model->appendRow({new QStandardItem("张三"), new QStandardItem("25"), new QStandardItem("北京")});
model->appendRow({new QStandardItem("李四"), new QStandardItem("30"), new QStandardItem("上海")});
model->appendRow({new QStandardItem("王五"), new QStandardItem("28"), new QStandardItem("深圳")});

auto *tableView = new QTableView();
tableView->setModel(model);
```

`setModel()` 做的事情比你想象的多。它不仅把模型指针存下来，还会创建一个默认的 QItemSelectionModel 来管理选择状态。这个默认的选择模型可以通过 `selectionModel()` 获取。如果你对这个默认行为不满意——比如你想让两个不同的视图共享同一个选择状态，这样在一个视图中选中某一行，另一个视图里对应的行也自动高亮——你可以自己创建一个 QItemSelectionModel 然后通过 `setSelectionModel()` 传给视图。

```cpp
auto *listView = new QListView();
auto *tableView = new QTableView();

// 两个视图绑定同一个模型
listView->setModel(model);
tableView->setModel(model);

// 共享选择模型：一个视图中选中，另一个也同步高亮
auto *sharedSelection = new QItemSelectionModel(model, this);
listView->setSelectionModel(sharedSelection);
tableView->setSelectionModel(sharedSelection);
```

这里有一个细节值得说明：当你调用 `setModel()` 替换一个视图已有的模型时，视图会先断开与旧模型的所有信号连接，然后把旧的选择模型也替换掉。所以如果你的代码里在 `setModel()` 之前保存了 `selectionModel()` 的指针，在 `setModel()` 之后那个指针就指向一个被废弃的选择模型了。务必在 `setModel()` 之后重新获取 `selectionModel()`。

### 3.2 选择模式：SingleSelection / MultiSelection / ExtendedSelection

QAbstractItemView 通过 `setSelectionMode(QAbstractItemView::SelectionMode)` 控制用户能选中多少个项目。这个属性对 QListView、QTableView、QTreeView 全部生效。

`QAbstractItemView::SingleSelection` 是最常用的模式——同一时间只能选中一个项目。点击一个新项目会自动取消之前选中的项目。这是大多数列表和表格的默认行为，用户操作起来最直觉。

`QAbstractItemView::MultiSelection` 允许用户选中多个项目，每次点击一个项目都会切换它的选中状态——点一下选中，再点一下取消选中，不需要按住任何修饰键。这个模式在"标签选择器""多属性筛选"这类场景下很实用，但普通用户可能不太习惯——因为点击一个已经选中的项目会取消选中，而不是替换选中。

`QAbstractItemView::ExtendedSelection` 是 QAbstractItemView 的默认选择模式。它在 SingleSelection 的基础上加入了多选能力：按住 Ctrl 点击可以追加/取消选中单个项目，按住 Shift 点击可以选中从上一个选中项到当前项之间的所有项目。这个模式对桌面用户来说最自然，因为它跟 Windows 资源管理器、macOS Finder 的多选行为一致。

```cpp
auto *listView = new QListView();
listView->setModel(model);

// 单选模式：同一时间只能选中一个
listView->setSelectionMode(QAbstractItemView::SingleSelection);

// 多选模式：点击切换选中，不按修饰键
listView->setSelectionMode(QAbstractItemView::MultiSelection);

// 扩展选择模式（默认）：Ctrl 追加，Shift 范围选
listView->setSelectionMode(QAbstractItemView::ExtendedSelection);

// 不允许选择
listView->setSelectionMode(QAbstractItemView::NoSelection);
```

跟选择模式配合使用的还有一个属性叫 `setSelectionBehavior(QAbstractItemView::SelectionBehavior)`。它控制在表格视图中"选中"的粒度。`SelectItems` 表示选中单个单元格，`SelectRows` 表示选中整行，`SelectColumns` 表示选中整列。在 QListView 和 QTreeView 中这个属性几乎没影响——它们天然就是按行选中的。但在 QTableView 中，`SelectRows` 是特别常用的设置，因为大部分表格应用场景下用户想选中的是"一条记录"而不是"一个单元格"。

```cpp
auto *tableView = new QTableView();
tableView->setModel(model);
tableView->setSelectionBehavior(QAbstractItemView::SelectRows);  // 按行选中
```

### 3.3 currentIndex() / selectedIndexes()：获取选中项

获取用户当前选中了什么是视图交互中最频繁的操作。QAbstractItemView 提供了几个不同粒度的获取方法。

`currentIndex()` 返回一个 QModelIndex，表示当前"焦点项"。无论选择模式是什么，总有一个焦点项存在——它通常就是你最后一次点击的那个项目，在视图上会有一个虚线框标记（即便你没选中它）。`currentIndex()` 在键盘导航场景下特别重要：当用户通过方向键上下移动时，`currentIndex()` 会跟着变化，而选中项可能不会变（在 SingleSelection 模式下 currentIndex 和选中项是同步的，但在 MultiSelection 模式下它们可以不同步）。

```cpp
// 监听当前焦点项变化
connect(tableView->selectionModel(), &QItemSelectionModel::currentChanged,
    [](const QModelIndex &current, const QModelIndex &previous) {
        if (current.isValid()) {
            qDebug() << "焦点项:" << current.row() << "," << current.column()
                     << "数据:" << current.data().toString();
        }
    }
);
```

`selectedIndexes()` 返回一个 QModelIndexList，包含所有被选中的模型索引。这个方法在多选模式下特别常用——你在槽函数里遍历这个列表就能拿到所有选中项的数据。

```cpp
// 获取所有选中行的第一列数据
QModelIndexList selected = tableView->selectionModel()->selectedIndexes();
for (const auto &index : selected) {
    qDebug() << "选中项:" << index.row() << "," << index.column()
             << "=" << index.data().toString();
}
```

有一个实用技巧：当你只需要知道"选中了哪些行"而不关心具体选中了哪些列时，用 `selectedRows(int column = 0)` 比 `selectedIndexes()` 更方便。`selectedRows()` 返回的列表中每行的索引只包含你指定列的那一个，避免了同一行被多次返回的情况——因为在 ExtendedSelection + SelectItems 模式下，用户可能选中了同一行的多个单元格，`selectedIndexes()` 会把它们全部返回，你得自己做去重。

```cpp
// 获取选中行的第 0 列索引（去重后的行列表）
QModelIndexList selectedRows = tableView->selectionModel()->selectedRows(0);
for (const auto &index : selectedRows) {
    qDebug() << "选中行:" << index.row()
             << "姓名:" << index.data().toString();
}
```

类似地，`selectedColumns(int row = 0)` 返回选中列的索引列表。

最后需要提醒的是：`selectionModel()` 返回的指针在 `setModel()` 调用之后才有效。如果你在没有设置模型的情况下调用 `selectionModel()`，它会返回 nullptr。所以在构造函数里如果你需要连接选择模型的信号，一定要在 `setModel()` 之后做。

### 3.4 setItemDelegate()：设置自定义委托

委托（Delegate）是 Qt Model/View 架构中负责"怎么画"和"怎么编辑"的组件。QAbstractItemView 默认使用 QStyledItemDelegate 作为委托——它负责把模型数据以文字、图标等形式渲染到每个单元格上，并在用户双击时提供一个默认的编辑器（通常是 QLineEdit）。当你需要对渲染或编辑行为做定制时，就可以通过 `setItemDelegate()` 换上自己的委托。

最典型的使用场景是"在表格某一列中使用自定义编辑控件"。比如你有一列是日期，你不想让用户手动输入日期字符串，而是希望双击后弹出一个 QDateEdit 日历选择器。这时候你就可以继承 QStyledItemDelegate，重写 `createEditor()`、`setEditorData()`、`setModelData()` 和 `updateEditorGeometry()` 这四个方法。

```cpp
class DateDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    using QStyledItemDelegate::QStyledItemDelegate;

    /// @brief 创建编辑器控件
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &,
                           const QModelIndex &) const override
    {
        auto *editor = new QDateEdit(parent);
        editor->setCalendarPopup(true);
        editor->setDisplayFormat("yyyy-MM-dd");
        return editor;
    }

    /// @brief 把模型数据设到编辑器上
    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        QDate date = index.data(Qt::EditRole).toDate();
        auto *dateEdit = qobject_cast<QDateEdit *>(editor);
        if (dateEdit) {
            dateEdit->setDate(date.isValid() ? date : QDate::currentDate());
        }
    }

    /// @brief 把编辑器的值写回模型
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                       const QModelIndex &index) const override
    {
        auto *dateEdit = qobject_cast<QDateEdit *>(editor);
        if (dateEdit) {
            model->setData(index, dateEdit->date(), Qt::EditRole);
        }
    }

    /// @brief 调整编辑器位置和大小
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                               const QModelIndex &) const override
    {
        editor->setGeometry(option.rect);
    }
};
```

然后把它设到视图的特定列上：

```cpp
auto *tableView = new QTableView();
tableView->setModel(model);

// 第 2 列使用日期委托
tableView->setItemDelegateForColumn(2, new DateDelegate(this));
```

注意 `setItemDelegate(QAbstractItemDelegate *)` 会替换整个视图的委托——所有列都用你指定的委托。如果你只想定制某一列，用 `setItemDelegateForColumn(int column, QAbstractItemDelegate *)`；只想定制某一行，用 `setItemDelegateForRow(int row, QAbstractItemDelegate *)`。这两个方法在 Qt 6 中属于 QAbstractItemView，不在 QStyledItemDelegate 上。

委托机制还有一个不太显眼但很有用的功能：如果你重写了 `paint()` 方法，你就可以完全控制单元格的渲染效果——比如在单元格中画一个进度条、一个颜色块、甚至一个小图表。这就是为什么很多"高级表格"看起来不像普通表格的原因——它们用了自定义委托来绘制单元格内容。

## 4. 踩坑预防

第一个坑是在 `setModel()` 之前操作 `selectionModel()`。`setModel()` 会销毁旧的选择模型并创建新的，所以如果你在 `setModel()` 之前保存了 `selectionModel()` 的指针或者连接了它的信号，在 `setModel()` 之后全部失效。养成一个习惯：先调 `setModel()`，再操作选择模型。

第二个坑是 `selectedIndexes()` 的返回顺序不确定。在不同版本的 Qt 和不同的视图实现中，返回的索引列表的顺序可能不同——不一定按行号排序。如果你需要按行号处理选中项，务必对返回的列表做排序，或者在遍历时自己维护顺序。

第三个坑是选择模式设成了 `NoSelection` 之后，`selectedIndexes()` 返回空列表，但 `currentIndex()` 仍然可能有效。很多朋友以为 NoSelection 意味着"完全不响应点击"，实际上焦点项仍然会跟着点击移动，只是选中状态不会变化。如果你需要彻底禁止点击响应，还需要重写 `mousePressEvent` 或者设置 `setEditTriggers(QAbstractItemView::NoEditTriggers)` 配合其他手段。

第四个坑是委托编辑时 `setModelData()` 没有被调用。这通常是因为用户按了 Esc 取消了编辑——取消编辑时 Qt 不会调用 `setModelData()`，编辑器的值会被丢弃。这是正确的行为，但如果你期望"无论如何都保存编辑结果"，你需要在别的地方做处理——比如监听视图的 `closeEditor` 信号。

第五个坑是在 `currentChanged` 信号的槽函数里直接操作模型。`currentChanged` 在焦点项变化时触发，而焦点项变化可能发生在模型正在被修改的过程中（比如 `beginResetModel` 和 `endResetModel` 之间）。此时如果你在槽函数里去读取模型数据，可能拿到的是中间状态甚至无效的 QModelIndex。一个安全的做法是在槽函数里通过 `QTimer::singleShot(0, ...)` 延迟到事件循环的下一轮再处理。

## 5. 练习项目

我们来做一个综合练习：创建一个窗口，展示一个 QTableView，绑定一个 QStandardItemModel，模型有四列数据——姓名、年龄、城市、入职日期。窗口分为上下两部分。上部是表格视图，支持 ExtendedSelection + SelectRows 模式，第 3 列（入职日期）使用自定义的日期委托。下部是控制面板，包含三个功能区域。左侧区域有三个按钮分别切换 SingleSelection、MultiSelection、ExtendedSelection 三种选择模式，切换后下方的标签会实时显示当前选择模式名称。中间区域有一个"显示选中项"按钮，点击后在 QTextEdit 中列出所有选中行的完整信息。右侧区域有一个"共享选择"复选框，勾选后会在表格旁边出现一个 QListView，两者共享同一个模型和选择模型——在表格中选中某一行，列表中对应的项也自动高亮。

几个提示：选择模式切换用 `setSelectionMode()`，切换后用 `setSelectionMode` 的枚举值生成模式名称字符串；选中项信息用 `selectionModel()->selectedRows(0)` 获取选中行的第一列索引，然后通过 `index.siblingAtColumn(n)` 获取同一行其他列的数据；共享选择模型的实现是创建一个 QListView，调用 `setModel()` 绑定同一个模型，然后调用 `setSelectionModel()` 传入表格视图的 `selectionModel()`；日期委托继承 QStyledItemDelegate，重写 createEditor/setEditorData/setModelData/updateEditorGeometry 四个方法。

## 6. 官方文档参考链接

[Qt 文档 · QAbstractItemView](https://doc.qt.io/qt-6/qabstractitemview.html) -- 视图基类，所有 Item 视图的根基

[Qt 文档 · QTableView](https://doc.qt.io/qt-6/qtableview.html) -- 表格视图

[Qt 文档 · QListView](https://doc.qt.io/qt-6/qlistview.html) -- 列表视图

[Qt 文档 · QItemSelectionModel](https://doc.qt.io/qt-6/qitemselectionmodel.html) -- 选择模型

[Qt 文档 · QStyledItemDelegate](https://doc.qt.io/qt-6/qstyleditemdelegate.html) -- 默认委托类

---

到这里，QAbstractItemView 的核心机制你就搞定了。setModel 是视图与数据模型之间的桥梁，setSelectionModel 让多个视图共享选择状态成为可能，三种选择模式覆盖了从单选到多选的全部需求，currentIndex 和 selectedIndexes 给你不同粒度的选中项获取手段，而委托机制则把"怎么画"和"怎么编辑"的控制权完全交给了你。这些知识对 QListView、QTableView、QTreeView 全部适用——因为它们都是从 QAbstractItemView 继承来的。
