# 现代Qt开发教程（新手篇）3.48——QTreeWidget：便捷树形控件

## 1. 前言 / 当你需要一个层级结构的时候

前面两篇我们聊了 QListWidget 和 QListView，它们处理的是一维的线性列表数据。但现实中的数据结构远不止"平铺一排"这么简单——文件系统的目录树、公司的组织架构、软件的菜单层级、XML/JSON 的节点嵌套，这些全都是树形结构。QTreeWidget 就是 Qt 为这类层级数据提供的便捷控件，和 QListWidget 一样，它把 Model 和 View 合二为一，内部帮你管理了一棵数据树，你不需要自己搭建 Model/View 架构，直接创建 QTreeWidgetItem 往里塞就行。

QTreeWidget 适合的场景和 QListWidget 非常类似——数据量不大、不需要跨控件共享数据、不需要自定义 Model 逻辑。一个设置面板的分类树、一个书签管理器、一个项目文件浏览面板——这些场景下 QTreeWidget 的开发效率远高于自己用 QTreeView 配 QAbstractItemModel。代价同样是灵活性受限：你不能在 Model 层做排序过滤，也不能给多个 View 共享同一份数据。但对于大量实际需求来说，这点代价完全可以接受。

今天的内容围绕四个方面展开。先看 QTreeWidgetItem 如何逐层构建出一棵层级树，然后通过 addTopLevelItem 和 insertChild 等方法来实现节点的增删操作，接着研究 setColumnCount 和 setHeaderLabels 让树形控件变成多列的树表，最后把 itemExpanded、itemCollapsed 和 itemClicked 这三个最常用的信号串起来做完整的交互演示。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QTreeWidget 和 QTreeWidgetItem 都在 QtWidgets 模块中，链接 Qt6::Widgets 即可。示例代码涉及 QTreeWidget、QTreeWidgetItem、QLabel、QPushButton、QLineEdit、QVBoxLayout、QHBoxLayout 和 QHeaderView。

## 3. 核心概念讲解

### 3.1 QTreeWidgetItem 构建层级树

QTreeWidget 的数据载体是 QTreeWidgetItem。每个 QTreeWidgetItem 就是树中的一个节点，它可以有若干个子节点，也可以有零个或一个父节点。没有父节点的节点被称为"顶层节点"（top-level item），它们直接挂在 QTreeWidget 的根上。一个 QTreeWidget 可以有多个顶层节点，形成一个"森林"结构（多棵树并排），最常见的做法是只设置一两个顶层节点作为根分类，然后在下面挂子节点。

创建一个 QTreeWidgetItem 非常简单——它有多个重载的构造函数，最常用的形式是传入一个父节点指针和一个字符串列表。字符串列表的每个元素对应一列的文本（如果 QTreeWidget 只有一列，列表中就只有一个元素）。如果不传父节点指针（即传入 QTreeWidget 指针作为构造参数），创建出来的就是顶层节点。

```cpp
auto *treeWidget = new QTreeWidget;

// 方式一：创建顶层节点（传 treeWidget 指针表示是顶层）
auto *rootItem = new QTreeWidgetItem(treeWidget);
rootItem->setText(0, "项目根目录");

// 方式二：创建子节点（传父节点指针）
auto *srcItem = new QTreeWidgetItem(rootItem);
srcItem->setText(0, "src");

auto *headerItem = new QTreeWidgetItem(srcItem);
headerItem->setText(0, "main.cpp");
```

你可以看到，构造 QTreeWidgetItem 时传入的父对象决定了它在树中的位置。传 QTreeWidget 指针就是顶层节点，传另一个 QTreeWidgetItem 指针就是那个节点的子节点。如果你不想在构造时就指定父对象，也可以用默认构造函数创建一个"游离"的节点，之后手动用 addTopLevelItem 或 addChild 把它挂上去。

setText(int column, const QString &) 设置节点在指定列上的文本。column 从 0 开始编号——如果你的 QTreeWidget 只有一列，那 column 永远是 0。如果有多列（后面会讲到 setColumnCount），那每个节点在不同列上可以有不同的文本内容。

QTreeWidgetItem 也支持图标。setIcon(int column, QIcon) 给指定列设置图标，图标显示在文本左侧。和使用 QListWidgetItem 一样，图标来源可以是资源文件、系统主题或者程序生成的 QPixmap。

```cpp
auto *folderItem = new QTreeWidgetItem(rootItem);
folderItem->setText(0, "resources");
folderItem->setIcon(0, QIcon::fromTheme("folder"));
```

还有一个经常被忽略但非常好用的方法是 setExpanded(bool)。默认情况下树节点的子节点是折叠起来的，用户需要手动点击展开箭头。如果你希望某个节点在创建时就处于展开状态，调用 setExpanded(true) 即可。这在初始化界面时很常见——比如你希望程序启动后默认展开根节点，让用户第一眼就能看到一级子目录。

```cpp
rootItem->setExpanded(true);  // 根节点默认展开
```

setExpanded 只控制初始展开状态，用户之后依然可以手动折叠/展开。如果你想禁用某个节点的展开箭头（让它永远不可折叠），可以用 setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator)——但这种情况比较少见，因为大多数时候你确实希望用户能自由折叠和展开子树。

### 3.2 addTopLevelItem / insertChild 增删节点

上一节我们在构造 QTreeWidgetItem 时直接通过构造函数指定了父对象，这确实是最方便的做法。但有些场景下你需要在运行时动态增删节点——比如用户点击"添加文件夹"按钮创建一个新的子目录，或者选中某个节点后点击"删除"把它从树中移除。这些操作需要用到 QTreeWidget 和 QTreeWidgetItem 的增删方法。

addTopLevelItem(QTreeWidgetItem *item) 是 QTreeWidget 的方法，它把一个节点添加为顶层节点。这个方法等价于在构造 QTreeWidgetItem 时传入 QTreeWidget 指针，区别在于它适用于"先创建节点、后添加到树"的场景——比如你先创建了一个节点，配置了文本、图标、自定义数据等属性，然后再一次性塞进树里。

```cpp
auto *newRoot = new QTreeWidgetItem;
newRoot->setText(0, "新建项目");
newRoot->setIcon(0, QIcon::fromTheme("folder"));

treeWidget->addTopLevelItem(newRoot);
```

insertTopLevelItem(int index, QTreeWidgetItem *item) 和 addTopLevelItem 类似，但可以指定插入位置。addTopLevelItem 总是追加到顶层节点列表的末尾，而 insertTopLevelItem 可以在指定位置插入。index 从 0 开始，如果 index 等于 topLevelItemCount()，效果等同于 addTopLevelItem。

在子节点层面，QTreeWidgetItem 提供了 addChild(QTreeWidgetItem *child) 和 insertChild(int index, QTreeWidgetItem *child) 两个方法。addChild 追加到子节点列表末尾，insertChild 可以指定插入位置。这两个方法和顶层的 addTopLevelItem/insertTopLevelItem 是完全对称的。

```cpp
auto *parent = treeWidget->currentItem();
if (parent) {
    auto *newChild = new QTreeWidgetItem;
    newChild->setText(0, "新建文件");
    newChild->setIcon(0, QIcon::fromTheme("text-x-generic"));
    parent->addChild(newChild);

    // 展开父节点让用户看到新增的子节点
    parent->setExpanded(true);
}
```

删除节点稍微需要注意一下。QTreeWidget 提供 takeTopLevelItem(int index) 来移除顶层节点，返回被移除的节点指针。QTreeWidgetItem 提供 takeChild(int index) 来移除子节点，同样返回指针。这两个方法都是"取出"而不是"删除"——它们把节点从树结构中断开，但不会 delete 这个节点。如果你确实要销毁它，需要手动 delete 返回的指针。如果你直接 delete 一个还在树中的 QTreeWidgetItem，它会自动从父节点的子节点列表中移除——这个行为和 QListWidgetItem 不同，QTreeWidgetItem 的析构函数会自己处理从树结构中摘除的逻辑。

```cpp
// 删除当前选中的节点
auto *current = treeWidget->currentItem();
if (current) {
    delete current;  // 析构函数自动从树中移除
}
```

如果你只是想移除所有子节点但保留父节点本身，可以用 QTreeWidgetItem 的 takeChildren() 方法——它返回子节点列表（QList<QTreeWidgetItem *>），调用者负责后续 delete。QTreeWidget 本身也有 clear() 方法，一次性清空整棵树的所有顶层节点及其子树。

```cpp
// 清空某个节点下的所有子节点
auto *parent = treeWidget->currentItem();
if (parent) {
    QList<QTreeWidgetItem *> children = parent->takeChildren();
    qDeleteAll(children);  // 批量删除
}

// 清空整棵树
treeWidget->clear();
```

topLevelItemCount() 返回顶层节点数量，topLevelItem(int index) 返回指定位置的顶层节点指针。对于一个节点，childCount() 返回它的子节点数量，child(int index) 返回指定位置的子节点指针。parent() 返回父节点指针，如果返回 nullptr 说明这是顶层节点。这些方法组合起来可以遍历整棵树——虽然在实际开发中你很少需要手写树的遍历，因为 QTreeWidget 自带的信号和交互已经覆盖了大部分需求。

### 3.3 setColumnCount / setHeaderLabels 多列树表

QTreeWidget 默认只有一列——每个节点就是一行文本加上可选的图标。但 QTreeWidget 的能力远不止于此：它支持多列显示，每个节点在不同列上可以有不同的文本。这种"树形结构 + 多列表格"的组合体通常被称为 TreeList 或者 TreeTable——Windows 资源管理器的"详细信息"视图就是一个典型的例子：左侧有目录树，右侧有多列文件属性（名称、大小、类型、修改日期）。

setColumnCount(int columns) 设置列数。调用 setColumnCount(3) 之后，每个节点就有 3 个"单元格"——分别可以通过 setText(0, ...)、setText(1, ...)、setText(2, ...) 来设置内容。

setHeaderLabels(const QStringList &labels) 设置表头标签。表头是 QTreeWidget 顶部的一行，显示每列的标题。如果列数是 3，传入的 QStringList 应该有 3 个元素。

```cpp
treeWidget->setColumnCount(3);
treeWidget->setHeaderLabels({"名称", "类型", "大小"});

auto *root = new QTreeWidgetItem(treeWidget);
root->setText(0, "项目根目录");
root->setText(1, "文件夹");
root->setText(2, "--");

auto *src = new QTreeWidgetItem(root);
src->setText(0, "src");
src->setText(1, "文件夹");
src->setText(2, "--");

auto *mainCpp = new QTreeWidgetItem(src);
mainCpp->setText(0, "main.cpp");
mainCpp->setText(1, "C++ 源文件");
mainCpp->setText(2, "4.2 KB");
```

树形展开箭头始终在第一列（column 0），后面的列只是纯文本。第一列通常用来显示层级结构的名称（文件名、分类名等），后面的列用来显示附属信息。这是一种非常自然的 UI 设计——用户在第一列看到层级关系，在同一行的后续列看到详细信息。

QTreeWidget 的表头实际上是一个 QHeaderView，你可以通过 header() 方法获取它的指针。QHeaderView 提供了很多控制表头行为的方法——setSectionResizeMode 控制列宽调整策略（Interactive 用户可拖拽、Stretch 自动拉伸占满、ResizeToContents 根据内容自适应），setStretchLastSection(bool) 控制最后一列是否自动占满剩余空间。

```cpp
// 第一列自适应内容，最后一列自动拉伸
treeWidget->header()->setSectionResizeMode(
    0, QHeaderView::ResizeToContents);
treeWidget->header()->setStretchLastSection(true);
```

如果你不想显示表头，可以调用 setHeaderHidden(true)——这在你的树不需要列标题时很有用，比如一个纯分类选择面板。如果只有一列并且不需要表头，设置 setHeaderHidden(true) 加 setColumnCount(1) 就是一个干净的纯树控件。

### 3.4 itemExpanded / itemCollapsed / itemClicked 信号

QTreeWidget 提供了大量的信号，其中和树形结构交互最相关的三个是 itemExpanded、itemCollapsed 和 itemClicked。

itemExpanded(QTreeWidgetItem *item) 在用户展开一个节点时发射。展开操作就是点击节点左侧的展开箭头（或者双击节点，取决于 QTreeWidget 的 expandsOnDoubleClick 属性）。参数是被展开的节点指针。这个信号的典型应用场景是"懒加载"——节点第一次展开时，你检查它下面是否已经有子节点，如果没有就从外部数据源（文件系统、数据库、网络接口）加载子节点数据。这样可以避免在程序启动时就一次性加载整棵树，对于数据量很大的场景能显著提升启动速度。

```cpp
connect(treeWidget, &QTreeWidget::itemExpanded,
        [](QTreeWidgetItem *item) {
    // 第一次展开时加载子节点
    if (item->childCount() == 0) {
        // 从数据源加载子节点...
        auto *child1 = new QTreeWidgetItem(item);
        child1->setText(0, "动态加载的节点 1");
        auto *child2 = new QTreeWidgetItem(item);
        child2->setText(0, "动态加载的节点 2");
    }
});
```

itemCollapsed(QTreeWidgetItem *item) 在用户折叠一个节点时发射。参数是被折叠的节点指针。这个信号使用频率低于 itemExpanded，但在某些场景下很有用——比如你想在节点折叠时释放子节点占用的资源（懒卸载），或者更新其他 UI 元素的状态。

itemClicked(QTreeWidgetItem *item, int column) 在用户单击一个节点时发射。参数是被点击的节点指针和被点击的列号。这个信号是最常用的——点击文件树中的文件来打开、点击设置面板中的分类来切换右侧内容、点击组织架构中的人员来显示详情。column 参数让你区分用户点击了哪一列——如果你有"名称"和"大小"两列，用户点击"名称"和点击"大小"会触发同一个信号但 column 不同。

```cpp
connect(treeWidget, &QTreeWidget::itemClicked,
        [](QTreeWidgetItem *item, int column) {
    if (!item) return;
    qDebug() << "点击了:" << item->text(0)
             << "列:" << column;
});
```

除了这三个信号，QTreeWidget 还有 currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)——当前选中节点变化时发射，用法和 QListWidget 的同名信号完全一致。itemDoubleClicked(QTreeWidgetItem *item, int column) 在双击节点时发射。itemChanged(QTreeWidgetItem *item, int column) 在节点数据变化时发射（比如你调用了 setText、setCheckState 等方法），和 QListWidget 的 itemChanged 一样需要注意 blockSignals 防止递归。

## 4. 踩坑预防

第一个坑是 delete QTreeWidgetItem 时它会自动从树中摘除，但这意味着如果你通过父节点的 child(index) 保存了一个指针然后 delete 了父节点，这个子节点指针就悬空了。QTreeWidgetItem 的析构是递归的——删除一个节点会同时删除它的所有子节点。所以如果你只想删除父节点但保留子节点，需要先用 takeChildren() 把子节点取出来。

第二个坑是 setHeaderLabels 传入的 QStringList 长度和 setColumnCount 设置的列数不匹配时不会报错。如果你 setColumnCount(3) 但 setHeaderLabels 只传了两个元素，第三列表头会是空的。反过来如果你传了四个元素但列数是 3，第四个元素会被忽略。养成习惯：setHeaderLabels 的列表长度始终和 setColumnCount 保持一致。

第三个坑是 itemChanged 信号在程序初始化填充数据时也会被触发。如果你在 itemChanged 槽函数中有比较重的逻辑（比如保存到数据库），在批量填充数据时应该先 blockSignals(true)，填充完再 blockSignals(false)，否则每添加一个节点都会触发一次槽函数。

第四个坑是 topLevelItem(index) 在 index 越界时返回 nullptr，和 QListWidget 的 item(index) 行为一致。同样地，QTreeWidgetItem 的 child(index) 在越界时也返回 nullptr。在遍历子节点时一定要做判空检查。

第五个坑是 QTreeWidget 的 clear() 方法会 delete 所有顶层节点及其子节点。如果你持有某个节点的裸指针，clear() 之后这个指针就悬空了。如果你需要在 clear 之后继续使用某些节点，先用 takeTopLevelItem 把它们取出来。

## 5. 练习项目

我们来做一个综合练习：创建一个"项目文件浏览器"窗口。中央是一个三列 QTreeWidget（名称、类型、大小），预先填充一个模拟的项目目录树——根节点"QtProject"，下面有"src"文件夹（内含 main.cpp、widget.h、widget.cpp）、"include"文件夹（内含 widget.h、helper.h）、"resources"文件夹（内含 icon.png、style.qss），以及根目录下的 CMakeLists.txt 和 README.md。窗口右侧有一个信息面板，用 QLabel 显示当前选中节点的完整路径（从根到当前节点逐层拼接）。窗口上方有"添加文件夹"和"添加文件"两个按钮——点击后在当前选中节点下创建子节点，文件名从 QLineEdit 中获取。双击文件节点弹出一个 QInputDialog 让用户修改文件名。点击"删除"按钮删除当前选中节点及其所有子节点。itemExpanded 信号用于在状态栏显示"已展开: xxx"，itemCollapsed 信号显示"已折叠: xxx"。

提示：获取节点的完整路径可以用循环不断调用 parent() 向上回溯，把每一层的 text(0) 拼接起来。QTreeWidgetItem::parent() 返回 nullptr 时说明已经到达顶层节点。

## 6. 官方文档参考链接

[Qt 文档 -- QTreeWidget](https://doc.qt.io/qt-6/qtreewidget.html) -- 便捷树形控件

[Qt 文档 -- QTreeWidgetItem](https://doc.qt.io/qt-6/qtreewidgetitem.html) -- 树节点条目

[Qt 文档 -- QTreeWidget::itemExpanded](https://doc.qt.io/qt-6/qtreewidget.html#itemExpanded) -- 展开信号

[Qt 文档 -- QHeaderView](https://doc.qt.io/qt-6/qheaderview.html) -- 表头控件

---

到这里，QTreeWidget 的核心用法就全部讲完了。QTreeWidgetItem 通过父子关系的嵌套构建出层级树，addTopLevelItem 和 addChild/insertChild 让你在运行时动态增删节点，setColumnCount 和 setHeaderLabels 把一棵纯树变成多列的树表，itemExpanded、itemCollapsed、itemClicked 三个信号覆盖了树形控件最常见的交互需求。当你的数据是层级结构且不需要复杂的自定义 Model 时，QTreeWidget 是效率最高的选择。
