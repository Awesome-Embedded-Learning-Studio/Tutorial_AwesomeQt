# 现代Qt开发教程（新手篇）3.47——QListView：Model 驱动列表视图

## 1. 前言 / 当 QListWidget 不够用时

上一篇我们聊了 QListWidget——一个把 Model 和 View 合二为一的便捷控件。它能覆盖大部分简单列表场景，但一旦你的需求稍微复杂一点——比如同一个数据源需要在多个列表中展示、需要对数据进行排序过滤、需要完全自定义每个条目的渲染方式——QListWidget 就显得力不从心了。原因很简单：QListWidget 内部的 Model 被封装死了，你没法把它拿出来和别的 View 共享，也没法方便地做数据层的操作。

Qt 的 Model/View 架构把数据和显示彻底分离。Model 负责管理数据（增删改查），View 负责把 Model 中的数据渲染到界面上。QListView 就是这个架构里最基础的列表视图——它本身不持有任何数据，所有数据都来自外部传入的 Model。这种设计带来的好处是灵活：你可以给同一个 Model 同时挂多个 View（一个列表视图、一个表格视图、一个树视图），数据只存一份但展示形式可以完全不同。你可以自由替换 Model 的实现——用 QStringListModel 做简单的字符串列表、用 QStandardItemModel 做带图标和自定义数据的列表、用 QSortFilterProxyModel 做搜索过滤、甚至完全自己写一个 QAbstractItemModel 子类对接数据库。

今天的内容从四个方面展开。先用 QStringListModel 配合 QListView 跑通一个最基本的列表，再通过 setViewMode 切换列表模式和图标模式两种视图风格，然后调整 setSpacing 和 setGridSize 来控制图标模式下的布局细节，最后用自定义 QStyledItemDelegate 来完全接管条目的渲染方式。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QListView、QStringListModel、QStyledItemDelegate 都在 QtWidgets 模块中，链接 Qt6::Widgets 即可。示例代码涉及 QListView、QStringListModel、QStyledItemDelegate、QStyleOptionViewItem、QPainter、QLineEdit、QPushButton、QVBoxLayout 和 QHBoxLayout。

## 3. 核心概念讲解

### 3.1 与 QStringListModel 配合的完整示例

QStringListModel 是 Qt 提供的最简单的 Model 实现——它内部管理一个 QStringList，每个字符串对应列表中的一行。它是 QAbstractListModel 的子类（不是 QAbstractTableModel 也不是 QAbstractItemModel 的直接子类），只支持一列数据。对于只需要显示一组字符串的场景，QStringListModel 是最轻量的选择。

使用方式非常直接：创建一个 QStringListModel，传入初始数据（或者之后用 setStringList 设置），然后创建一个 QListView，调用 setModel 把 Model 挂上去。QListView 会自动从 Model 中读取数据并渲染。

```cpp
// 创建 Model 并设置初始数据
const QStringList items = {
    "Qt Core", "Qt GUI", "Qt Widgets", "Qt Network",
    "Qt SQL", "Qt XML", "Qt Test"
};
auto *model = new QStringListModel(items, &app);

// 创建 View 并挂载 Model
auto *listView = new QListView;
listView->setModel(model);
```

setModel(QAbstractItemModel *model) 是 QListView 的核心方法——它把一个 Model 绑定到 View 上。调用之后 QListView 会立刻连接 Model 的信号（dataChanged、rowsInserted、rowsRemoved 等），并在数据变化时自动刷新显示。一个 Model 可以同时绑定到多个 View 上——如果你把同一个 QStringListModel 设给了三个 QListView，那么在任何一处修改 Model 的数据，三个 View 都会同步更新。

```cpp
auto *sharedModel = new QStringListModel(
    {"Item 1", "Item 2", "Item 3"}, &app);

auto *view1 = new QListView;
view1->setModel(sharedModel);

auto *view2 = new QListView;
view2->setModel(sharedModel);

// 修改 model 数据后，view1 和 view2 同时更新
sharedModel->setStringList({"Updated 1", "Updated 2", "Updated 3"});
```

QStringListModel 提供了 setStringList(const QStringList &) 来整体替换数据，也支持通过 QAbstractItemModel 的接口做增删改。比如 setData 修改某个位置的数据，insertRows 插入新行，removeRows 删除行。这些操作会触发对应的信号，QListView 收到信号后自动刷新。

```cpp
// 修改第 2 行的数据
QModelIndex index = model->index(2);
model->setData(index, "Qt Multimedia", Qt::DisplayRole);

// 在末尾追加一行
model->insertRows(model->rowCount(), 1);
QModelIndex newIndex = model->index(model->rowCount() - 1);
model->setData(newIndex, "Qt 3D", Qt::DisplayRole);

// 删除第 0 行
model->removeRows(0, 1);
```

这里你需要理解一个核心概念：QModelIndex。在 Model/View 架构中，View 通过 QModelIndex 来定位数据——它是一个轻量级的"指针"，包含行号、列号和一个指向 Model 的内部指针。对于 QStringListModel 这种一维列表 Model，QModelIndex 的行号就是数据在 QStringList 中的位置，列号始终是 0。你通过 model->index(row) 获取指定行的 QModelIndex，然后用 model->data(index, role) 读取数据。role 参数决定你要读取哪种数据——Qt::DisplayRole 返回显示文本，Qt::DecorationRole 返回图标，Qt::ToolTipRole 返回工具提示。

QListView 的 clicked(QModelIndex) 信号和 doubleClicked(QModelIndex) 信号在用户点击/双击条目时发射，参数是被点击的 QModelIndex。你可以通过 index.row() 获取行号，通过 model->data(index) 获取数据。这比 QListWidget 直接给你 QListWidgetItem 指针的方式多了一层间接——但代价换来的是 Model 和 View 的解耦。

### 3.2 setViewMode 列表模式 vs 图标模式

QListView 默认使用 QListView::ListMode——条目从上到下排列成一列，每个条目占一行，文字在图标右侧。这是最常见的列表展示形式，和 QListWidget 的默认外观完全一样。

但 QListView 还支持 QListView::IconMode——在这种模式下，条目从左到右、从上到下排列成网格，文字在图标下方。这种模式适合展示文件、图片、应用图标等"大图标 + 短文字"的场景——Windows 资源管理器的大图标视图、Android 的应用列表、macOS 的 Launchpad，都是 IconMode 的典型应用。

```cpp
auto *listView = new QListView;

// 列表模式（默认）
listView->setViewMode(QListView::ListMode);
listView->setIconSize(QSize(24, 24));

// 图标模式
listView->setViewMode(QListView::IconMode);
listView->setIconSize(QSize(64, 64));
```

两种模式的核心区别在于布局策略。ListMode 下，条目只在垂直方向排列，水平方向占满整行宽度（或者只占 sizeHint 的宽度，取决于 resizeMode）。IconMode 下，条目在两个方向上自由排列，形成一个流式布局（flow layout）——一行排满了自动换行到下一行。Flow 方向由 setFlow 控制——默认是 QListView::LeftToRight（从左到右排列），设为 QListView::TopToBottom 则变成从上到下排列（此时效果接近 ListMode 但条目仍然可以有图标样式的布局）。

在 IconMode 下，条目的可拖动行为也有区别。默认情况下 IconMode 允许用户拖拽条目来重新排列它们的位置——这适合做"桌面图标"式的交互。如果你不希望用户拖拽，可以 setDragEnabled(false) 或者 setMovement(QListView::Static)（静态布局，不允许移动）。

```cpp
// 图标模式 + 禁止拖拽
listView->setViewMode(QListView::IconMode);
listView->setMovement(QListView::Static);
```

还有一个有用的属性是 setWrapping(bool)。在 IconMode 下默认开启换行（wrapping = true），条目排满一行后自动换行。如果你关闭换行，所有条目会在一行内水平排列，列表变成水平的滚动条——适合做"横向滚动选择栏"这种 UI。

```cpp
// 横向滚动列表（不换行）
listView->setViewMode(QListView::ListMode);
listView->setFlow(QListView::LeftToRight);
listView->setWrapping(false);
```

### 3.3 setSpacing / setGridSize 图标视图布局

当使用 IconMode 时，条目之间的间距和整体网格尺寸需要额外调整。默认情况下 IconMode 的条目间距比较紧凑，如果你展示的是大图标，条目之间可能挤在一起看起来不够清爽。

setSpacing(int) 设置条目之间的间距（像素），默认是 0。这个间距同时作用于水平方向和垂直方向——它会在每两个相邻条目之间插入指定像素的空白。

```cpp
listView->setViewMode(QListView::IconMode);
listView->setIconSize(QSize(64, 64));
listView->setSpacing(16);  // 条目之间留 16px 间距
```

setGridSize(QSize) 设置一个固定的网格大小——每个条目都会被放到这个大小的格子中。如果条目本身比格子小，条目在格子中居中显示；如果条目比格子大，条目会被裁切。setGridSize 的效果是让所有条目在网格中对齐——不管每个条目的实际 sizeHint 有多大，它们都会被均匀地排列在相同大小的格子里。

```cpp
listView->setViewMode(QListView::IconMode);
listView->setIconSize(QSize(48, 48));
listView->setGridSize(QSize(100, 100));  // 每个格子 100x100
listView->setSpacing(8);
```

setGridSize 和 setSpacing 可以同时使用——setGridSize 决定每个格子的大小，setSpacing 决定格子之间的间距。这两者的叠加效果就是每个条目的占据空间为 gridSize + spacing。

有一个细节值得注意：setGridSize 只在 IconMode 下有效果。在 ListMode 下调用 setGridSize 不会有任何视觉变化——ListMode 的条目始终是从上到下的线性排列，不受网格约束。如果你在 ListMode 下需要控制条目的行高，应该通过 QStyledItemDelegate 的 sizeHint 或者 QListView 的 setUniformItemSizes 来间接控制。

setUniformItemSizes(bool) 是一个性能优化属性。当你知道所有条目的 sizeHint 都相同（比如纯文本列表或者统一大小的图标列表），把它设为 true 可以让 QListView 跳过逐个计算条目大小的步骤，直接用第一个条目的 sizeHint 统一所有条目的大小。这在列表有上千个条目时能显著提升滚动性能。默认值是 false。

```cpp
// 性能优化：所有条目大小相同时开启
listView->setUniformItemSizes(true);
```

### 3.4 自定义 ItemDelegate 改变显示样式

QStyledItemDelegate（以及它的父类 QAbstractItemDelegate）是 Model/View 架构中控制"每个条目怎么画"的核心机制。默认情况下 QListView 使用一个内部的默认 delegate 来渲染每个条目——它把 Qt::DisplayRole 的文本画在条目区域，把 Qt::DecorationRole 的图标画在文本旁边，加上选中高亮和焦点框。如果你对这个默认渲染不满意——比如想在条目右侧画一个小标签、想让选中效果是圆角矩形而不是默认的蓝色高亮、想在条目上画一个进度条或者自定义的图形——就需要继承 QStyledItemDelegate 并重写 paint 和 sizeHint 方法。

paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) 是渲染的核心方法。QPainter 提供绘图画布，QStyleOptionViewItem 包含条目的几何信息和状态（是否选中、是否悬停、是否获得焦点等），QModelIndex 指向 Model 中的数据项。

```cpp
class ColorDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        // 先让基类画默认的背景和文本
        QStyledItemDelegate::paint(painter, option, index);

        // 在条目右侧画一个小色块标记
        QString colorName = index.data(Qt::DisplayRole).toString();
        QColor color(colorName);

        int tagSize = 12;
        int margin = 6;
        QRect tagRect(
            option.rect.right() - tagSize - margin,
            option.rect.top() + (option.rect.height() - tagSize) / 2,
            tagSize, tagSize);

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setBrush(color);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(tagRect, 2, 2);
        painter->restore();
    }
};
```

上面这个 delegate 在每个条目右侧画了一个小色块——色块的颜色来自条目的文本内容（假设文本是颜色名称如"red"、"green"）。先调用基类的 paint 方法画好默认的背景和文本，然后在上面叠加自定义的绘制内容。painter->save() 和 painter->restore() 是好习惯——确保你的自定义绘制不会污染 QPainter 的状态，影响后续条目的渲染。

sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) 返回条目的推荐大小。默认实现返回一个基于文本大小计算的 QSize。如果你想让条目更高（比如要在条目中画两行信息），或者更宽（要为自定义绘制留空间），就需要重写 sizeHint。

```cpp
QSize sizeHint(const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
{
    // 在默认高度基础上额外加 8 像素
    QSize baseSize =
        QStyledItemDelegate::sizeHint(option, index);
    return QSize(baseSize.width(), baseSize.height() + 8);
}
```

设置自定义 delegate 的方法是 QListView::setItemDelegate(QAbstractItemDelegate *delegate)。QListView 会接管 delegate 的所有权并在适当时候 delete 它——所以你通常用 new 创建 delegate 后不需要手动 delete。

```cpp
auto *delegate = new ColorDelegate(listView);
listView->setItemDelegate(delegate);
```

自定义 delegate 还可以重写 createEditor 和 setEditorData / setModelData 来提供原地编辑（in-place editing）的自定义控件。默认情况下如果 Model 的 flags 包含 Qt::ItemIsEditable，双击条目会弹出一个 QLineEdit 让用户编辑文本。如果你想让用户编辑时弹出一个 QSpinBox 或者 QDateTimeEdit，就需要在 createEditor 中返回你想要的编辑控件。

## 4. 踩坑预防

第一个坑是忘记调用 setModel。QListView 在没有 Model 的情况下什么都不显示——一个空白的矩形。这和 QListWidget 不同，QListWidget 内部自带 Model，不需要手动设置。如果你发现 QListView 是空的，第一件事就是检查 setModel 是否被调用了，以及 Model 中是否有数据（model->rowCount() > 0）。

第二个坑是 QModelIndex 的生命周期问题。QModelIndex 是一个轻量级的值类型，它不持有 Model 的引用计数。如果 Model 的结构发生了变化（比如删除了行、插入了行），之前保存的 QModelIndex 可能失效——它的内部指针指向的数据可能已经被移走或者删除了。如果你需要在异步操作中使用 QModelIndex，应该用 QPersistentModelIndex 来保存——它会自动跟踪 Model 的变化并更新内部的行号和列号。

第三个坑是在自定义 delegate 的 paint 方法中忘记处理选中状态的背景。如果你完全重写了 paint 而没有调用基类的 paint，那么条目在选中时不会自动高亮——你需要自己在 paint 中根据 option.state & QStyle::State_Selected 来绘制选中背景。如果你只是想在默认渲染的基础上叠加自定义内容，先调用基类的 paint 再画自定义部分是最安全的做法。

第四个坑是 setGridSize 在 ListMode 下不生效。如果你发现调了 setGridSize 但布局没有任何变化，检查一下 setViewMode 是否被设成了 IconMode。ListMode 下条目始终是线性排列的，setGridSize 会被忽略。

第五个坑是 QListView 的 setModel 在切换 Model 时不会自动 delete 旧的 Model。如果你多次调用 setModel(modelA)、setModel(modelB)，modelA 仍然在内存中。你需要在切换之前手动 delete 旧 Model，或者把旧 Model 的 parent 设为一个会在适当时机析构的 QObject。

## 5. 练习项目

我们来做一个综合练习：创建一个窗口，展示 QListView 的两种视图模式。窗口左侧是一个 QListView，使用 QStringListModel 管理一组颜色名称（如"Tomato"、"SteelBlue"、"SeaGreen"、"Gold"等 12 种颜色）。窗口右侧有一个控制面板，包含"列表模式"和"图标模式"两个 QPushButton 切换 setViewMode，一个 QSpinBox 控制 setSpacing（范围 0-30），以及一个 QSpinBox 控制 setGridSize（范围 60-200，同时设置宽高）。列表使用自定义 QStyledItemDelegate——在 ListMode 下，每个条目右侧画一个对应颜色的小圆角矩形色块；在 IconMode 下，每个条目区域中心画一个大的颜色圆，下方显示颜色名称。窗口底部有一个 QLineEdit 用于输入新的颜色名称，点击"添加"按钮后往 Model 中追加一行，QListView 自动刷新显示新条目。

提示：delegate 的 paint 方法中可以通过 index.data(Qt::DisplayRole).toString() 获取颜色名称，然后用 QColor(colorName) 构造颜色对象。QColor 的构造函数接受颜色名称字符串（如"red"、"tomato"、"steelblue"等），如果名称无效则返回无效颜色——可以用 QColor::isValid() 检查。

## 6. 官方文档参考链接

[Qt 文档 -- QListView](https://doc.qt.io/qt-6/qlistview.html) -- Model 驱动列表视图

[Qt 文档 -- QStringListModel](https://doc.qt.io/qt-6/qstringlistmodel.html) -- 字符串列表 Model

[Qt 文档 -- QStyledItemDelegate](https://doc.qt.io/qt-6/qstyleditemdelegate.html) -- 样式化的 Item Delegate

[Qt 文档 -- QAbstractItemModel](https://doc.qt.io/qt-6/qabstractitemmodel.html) -- 抽象 Model 基类

[Qt 文档 -- Model/View Programming](https://doc.qt.io/qt-6/model-view-programming.html) -- Model/View 编程指南

---

到这里，QListView 的核心用法就全部讲完了。QStringListModel 加 QListView 是 Model/View 架构最简单的入门组合——一个管数据，一个管显示，通过 setModel 一行代码把它们连起来。setViewMode 让你在列表和图标两种布局之间自由切换，setSpacing 和 setGridSize 精细控制图标布局的间距和网格，自定义 QStyledItemDelegate 让你完全掌控每个条目的渲染方式。当你理解了这一套机制，后面用 QTableView、QTreeView 配合更复杂的 Model 就会顺理成章——因为底层原理完全一样。
