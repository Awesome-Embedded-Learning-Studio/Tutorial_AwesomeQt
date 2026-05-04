# 现代Qt开发教程（新手篇）3.49——QTreeView：Model 驱动树视图

## 1. 前言 / 当 QTreeWidget 不够用时

上一篇我们聊了 QTreeWidget——一个把 Model 和 View 合二为一的便捷树形控件。它能覆盖大部分简单的层级数据展示场景，但一旦你的需求稍微复杂一点——比如同一个数据源需要在树和表格两种视图中展示、需要对接文件系统或者数据库、需要做排序过滤——QTreeWidget 就显得力不从心了。原因和 QListWidget 一样：QTreeWidget 内部的 Model 被封装死了，你没法把它拿出来和别的 View 共享，也没法方便地在数据层做操作。

QTreeView 是 Qt Model/View 架构中专门处理树形数据的视图类。和 QListView 一样，它本身不持有任何数据——所有数据来自外部传入的 Model。你可以给同一个 Model 同时挂一个 QTreeView 和一个 QTableView，数据只存一份但展示形式完全不同。你可以自由替换 Model 的实现——用 QStandardItemModel 做通用的树形数据、用 QFileSystemModel 直接展示文件系统、甚至完全自己写一个 QAbstractItemModel 子类对接你的数据源。

今天的内容从四个方面展开。先看 QStandardItemModel 和 QTreeView 配合构建一棵通用树，然后通过 QFileSystemModel 直接把文件系统映射成目录树，接着研究 setRootIndex 如何控制树的显示根节点，最后用 expand、collapse 和 expandAll 来编程控制节点的展开状态。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QTreeView、QStandardItemModel、QFileSystemModel 都在 QtWidgets 模块中，链接 Qt6::Widgets 即可。示例代码涉及 QTreeView、QStandardItemModel、QFileSystemModel、QLabel、QPushButton、QLineEdit、QVBoxLayout、QHBoxLayout 和 QHeaderView。

## 3. 核心概念讲解

### 3.1 与 QStandardItemModel 配合树结构展示

QStandardItemModel 是 Qt 提供的最灵活的标准 Model 实现。它可以同时用于列表、表格和树——在树形场景下，每个 QStandardItem 可以有子节点，形成一棵任意深度的层级树。QStandardItem 本身支持文本、图标、复选框、工具提示、自定义字体和前景/背景色，表达能力很强。

创建一棵树的基本流程是：先创建一个 QStandardItemModel，然后创建若干 QStandardItem，通过 setChild 或者 appendRow 建立父子关系，最后调用 QTreeView 的 setModel 把 Model 挂上去。

```cpp
auto *model = new QStandardItemModel(this);
model->setHorizontalHeaderLabels({"名称", "描述"});

// 顶层节点
auto *rootItem = new QStandardItem("项目根目录");
rootItem->setIcon(QIcon::fromTheme("folder"));

// 子节点
auto *srcItem = new QStandardItem("src");
srcItem->setIcon(QIcon::fromTheme("folder"));
rootItem->appendRow(srcItem);

auto *mainItem = new QStandardItem("main.cpp");
mainItem->setIcon(QIcon::fromTheme("text-x-generic"));
srcItem->appendRow(mainItem);

// 把顶层节点加入 model
model->appendRow(rootItem);

// 挂载到 view
auto *treeView = new QTreeView;
treeView->setModel(model);
treeView->expandAll();
```

这里有一个非常关键的细节需要注意：appendRow(QStandardItem *item) 的行为和 QTreeWidget 中的 addChild 不同。QStandardItemModel 的 appendRow 是在当前节点下追加一行——如果模型有多列，这一行的第一列（column 0）是你传入的 QStandardItem，后续列需要通过 setChild(row, column, item) 单独设置。这在刚开始用 QStandardItemModel 构建多列树时是一个常见的困惑点。

```cpp
// 多列树：第一列是名称，第二列是描述
auto *nameItem = new QStandardItem("main.cpp");
auto *descItem = new QStandardItem("C++ 源文件");

// 先 appendRow 添加第一列
srcItem->appendRow(nameItem);

// 再通过 setChild 设置第二列（行号，列号，item）
// nameItem 在 srcItem 中是第 0 行
srcItem->setChild(0, 1, descItem);
```

你会发现这比 QTreeWidget 的 setText(1, "xxx") 多了一步操作，原因是 Model/View 架构中每一列的数据都是一个独立的 QStandardItem，而不是同一个 item 的不同属性。这种设计的好处是每一列都可以有独立的图标、字体、前景色等属性，代价是写法稍微繁琐一些。如果只有一列数据，用 appendRow 就够了，不需要操心 setChild。

QStandardItem 支持和 QTreeWidgetItem 类似的丰富属性。setText(const QString &) 设置文本，setIcon(const QIcon &) 设置图标，setCheckable(bool) 启用复选框，setCheckState(Qt::CheckState) 设置勾选状态，setEditable(bool) 控制是否可编辑，setData(const QVariant &, int role) 存储自定义数据。setToolTip(const QString &) 设置悬停提示。大部分方法和 QTreeWidgetItem 是一一对应的，迁移成本很低。

### 3.2 QFileSystemModel + QTreeView 文件树

QFileSystemModel 是 Qt 提供的一个"开箱即用"的 Model 实现——它直接读取操作系统的文件系统，把目录和文件映射成一棵树。配合 QTreeView 使用时，你不需要手动创建任何 QStandardItem，只需要创建一个 QFileSystemModel，设置一个根路径，然后把它挂到 QTreeView 上，就能得到一个完整的文件浏览器。

```cpp
auto *fsModel = new QFileSystemModel(this);
fsModel->setRootPath(QDir::homePath());

auto *treeView = new QTreeView;
treeView->setModel(fsModel);

// 只显示第一列（文件名），隐藏其他列
treeView->setColumnHidden(1, true);  // 大小
treeView->setColumnHidden(2, true);  // 类型
treeView->setColumnHidden(3, true);  // 修改日期
```

QFileSystemModel 默认提供四列数据：文件名称（列 0）、文件大小（列 1）、文件类型（列 2）和最后修改日期（列 3）。如果你只需要文件名这一列，可以用 setColumnHidden 把其他三列隐藏掉，也可以只显示你关心的列。

setRootPath(const QString &) 是 QFileSystemModel 的核心方法。它告诉 Model 从哪个路径开始监控文件系统。注意这里的措辞是"监控"而不是"读取"——QFileSystemModel 内部使用了一个后台线程来监听文件系统的变化（文件创建、删除、重命名、修改），当文件系统发生变化时 Model 会自动更新，QTreeView 会自动刷新显示。这意味着你的文件浏览器是"实时"的——如果用户在操作系统的文件管理器中创建了一个新文件，你的 QTreeView 会自动多出一个节点，不需要手动刷新。

setRootPath 设置的路径只是监控的起点，不影响 QTreeView 显示的根节点。QFileSystemModel 实际上会读取整个文件系统的结构（从根目录开始），只是通过 setRootPath 告诉后台线程优先加载哪个路径下的内容。这就是为什么你还需要用 setRootIndex 来控制 QTreeView 的显示范围——下一节会详细讲。

QFileSystemModel 还有一些有用的过滤方法。setFilter(QDir::Filters) 控制显示哪些类型的文件系统条目——比如 QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot 表示显示所有目录和文件但不显示 . 和 ..。setNameFilters(const QStringList &) 设置文件名过滤器——比如 {"*.cpp", "*.h", "*.txt"} 只显示这三种扩展名的文件。setNameFilterDisables(bool) 控制不匹配的文件是被隐藏（false）还是变灰显示（true，默认）。

```cpp
fsModel->setFilter(QDir::AllDirs | QDir::Files
                   | QDir::NoDotAndDotDot);
fsModel->setNameFilters({"*.cpp", "*.h", "*.txt", "*.md"});
fsModel->setNameFilterDisables(false);  // 不匹配的直接隐藏
```

### 3.3 setRootIndex 设置显示根节点

QTreeView 默认会显示 Model 中的所有顶层节点。对于 QFileSystemModel 来说，这意味着它会从文件系统的根目录（Linux 上是 /，Windows 上是各个盘符）开始显示，展示整个文件系统的目录树——这通常不是你想要的。

setRootIndex(const QModelIndex &index) 是 QTreeView 的方法，它设置一个"显示根节点"——QTreeView 只显示这个节点及其子树，其他部分全部隐藏。这对于 QFileSystemModel 来说是必须的操作，否则你会看到从 / 开始的整个文件系统。

```cpp
auto *fsModel = new QFileSystemModel(this);
fsModel->setRootPath(QDir::homePath());

auto *treeView = new QTreeView;
treeView->setModel(fsModel);

// 只显示用户主目录下的内容
QModelIndex rootIndex =
    fsModel->index(QDir::homePath());
treeView->setRootIndex(rootIndex);
```

fsModel->index(const QString &path) 把一个文件系统路径转换成对应的 QModelIndex。QFileSystemModel 内部为每个文件和目录都维护了一个 QModelIndex，index 方法根据路径查找并返回它。注意这个方法可能返回一个无效的 QModelIndex（如果路径不存在），所以在实际代码中应该先检查 index.isValid()。

对于 QStandardItemModel，setRootIndex 同样有效——你可以把一棵大树的某个子节点设为显示根，让 QTreeView 只展示那棵子树。这种能力在实现"浏览历史"或者"面包屑导航"时非常有用——用户点击不同的路径节点，你通过 setRootIndex 切换显示的子树，实现"钻入/钻出"的浏览体验。

```cpp
// QStandardItemModel 中设置显示根节点
// 假设 treeRoot 下面有一棵多层级的树
QModelIndex subRoot = model->index(0, 0, treeRoot);
treeView->setRootIndex(subRoot);
```

有一个细节值得注意：setRootIndex 只是改变了显示范围，不影响 Model 的数据。Model 仍然持有完整的树结构，你只是让 QTreeView "聚焦"到了某个子树上。如果你需要"回到上层"，重新 setRootIndex 到父节点的 index 即可。

QTreeView 的 clicked(const QModelIndex &index) 信号在用户点击节点时发射。对于 QFileSystemModel，你可以通过 fsModel->filePath(index) 获取被点击节点对应的文件系统路径——这比手动拼接路径方便得多，也是使用 QFileSystemModel 的核心优势之一。

```cpp
connect(treeView, &QTreeView::clicked,
        [fsModel](const QModelIndex &index) {
    QString path = fsModel->filePath(index);
    QFileInfo info(path);
    if (info.isDir()) {
        qDebug() << "目录:" << path;
    } else {
        qDebug() << "文件:" << path
                 << "大小:" << info.size();
    }
});
```

### 3.4 expand / collapse / expandAll 节点展开控制

QTreeView 提供了一组方法让你在代码中控制节点的展开和折叠状态。expand(const QModelIndex &index) 展开指定节点，collapse(const QModelIndex &index) 折叠指定节点，expandAll() 展开所有节点，collapseAll() 折叠所有节点。

expandAll() 是最暴力的展开方式——它递归展开整棵树的所有节点。如果你的文件树有几十层嵌套，expandAll 会一次性展开所有层级，可能会比较慢（尤其是使用 QFileSystemModel 时，它会触发大量的文件系统 I/O）。在数据量不大时用 expandAll 很方便，在数据量大时应该避免。

```cpp
treeView->expandAll();    // 展开全部
treeView->collapseAll();  // 折叠全部
```

expand(const QModelIndex &) 展开单个节点。它只展开你指定的那个节点的一级子节点——不会递归展开子节点的子节点。这比 expandAll 精确得多，适合用在"懒加载"场景：用户展开一个目录时，你检查它的子目录是否已经加载，如果没有就触发加载，然后调用 expand 展开它。

```cpp
// 展开特定的节点
QModelIndex srcIndex = model->index(0, 0);
treeView->expand(srcIndex);
```

isExpanded(const QModelIndex &) 返回某个节点当前是否处于展开状态。这在保存/恢复树的展开状态时很有用——你可以遍历所有可见节点，记录哪些是展开的，下次打开程序时恢复。

```cpp
// 保存展开状态
QModelIndex root = model->index(0, 0);
if (treeView->isExpanded(root)) {
    // 记住这个节点是展开的
}
```

还有一个实用的方法是 scrollTo(const QModelIndex &, ScrollHint)。它让 QTreeView 滚动到指定节点位置。ScrollHint 参数控制滚动后的位置：QAbstractItemView::EnsureVisible 让节点可见即可（可能在视口边缘），PositionAtCenter 把节点滚动到视口中心，PositionAtTop 放到视口顶部。这在"搜索并定位到某个文件"的场景下很有用。

```cpp
// 滚动到某个节点并居中显示
treeView->scrollTo(targetIndex,
                   QAbstractItemView::PositionAtCenter);
treeView->setCurrentIndex(targetIndex);
```

QTreeView 的 expanded(const QModelIndex &) 和 collapsed(const QModelIndex &) 信号分别在节点展开和折叠时发射。和 QTreeWidget 的 itemExpanded/itemCollapsed 对应，区别是参数类型——QTreeView 的信号传的是 QModelIndex 而不是 QTreeWidgetItem 指针。通过 model->data(index, Qt::DisplayRole) 就能获取节点的文本数据。

## 4. 踩坑预防

第一个坑是 QFileSystemModel 的 setRootPath 和 QTreeView 的 setRootIndex 必须配合使用。setRootPath 只是告诉 Model 从哪里开始监控文件系统，不设置 setRootIndex 的话 QTreeView 会显示从根目录开始的完整文件系统树。如果你只想显示用户主目录，两步缺一不可。

第二个坑是 QFileSystemModel 在后台线程中加载文件系统数据，首次展开一个大目录时可能会有短暂的延迟。Model 的 directoryLoaded(const QString &path) 信号在一个目录加载完成时发射，你可以用这个信号来更新加载状态指示器。如果你在 setRootPath 之后立刻调用 expandAll()，很可能在数据还没加载完时就尝试展开了——此时节点还没有子数据，expandAll 实际上展开的是一棵空树。解决方案是连接 directoryLoaded 信号，在数据加载完后再调用 expandAll。

第三个坑是 QStandardItemModel 构建 多列树时 setChild 的行号问题。appendRow 之后新行在父节点中的行号是 parent->rowCount() - 1。如果你连续 appendRow 多个子节点，每个子节点的第二列需要用 setChild(row, 1, descItem) 来设置——row 必须是正确的行号。如果你不确定行号，可以用 appendRow 的重载版本，一次传入一整行的 items：parent->appendRow({nameItem, descItem})——这比先 appendRow 再 setChild 更不容易出错。

第四个坑是 QTreeView 的 setModel 在切换 Model 时不会自动 delete 旧的 Model。如果你多次调用 setModel(modelA)、setModel(modelB)，modelA 仍然在内存中。你需要在切换之前手动 delete 旧 Model，或者把旧 Model 的 parent 设为一个会在适当时机析构的 QObject。

第五个坑是 QFileSystemModel 的 index(path) 在路径不存在时返回无效 QModelIndex。如果你用了一个不存在的路径，setRootIndex 接收到无效 index 后 QTreeView 会显示空。在设置之前先检查路径是否存在是一个好习惯。

## 5. 练习项目

我们来做一个综合练习：创建一个"文件浏览器"窗口，左侧是 QTreeView 配合 QFileSystemModel 显示用户主目录下的文件树（只显示文件名列），右侧是信息面板显示选中文件/文件夹的详细信息（路径、大小、类型、修改时间）。窗口顶部有一个 QLineEdit 作为路径导航栏，显示当前显示根目录的路径，用户可以输入新路径后按回车切换 setRootIndex。下方有四个按钮："展开全部"、"折叠全部"、"展开选中"和"折叠选中"，分别调用 expandAll、collapseAll、expand 和 collapse。"展开选中"和"折叠选中"操作当前高亮的节点。窗口底部有一个 QLabel 显示当前选中文件的完整路径，用 clicked 信号驱动更新。程序启动时默认展开用户主目录的第一层子目录。

提示：QFileSystemModel 提供了 filePath(index)、fileName(index)、fileInfo(index) 等便捷方法来获取文件信息。QFileInfo 有 isDir()、size()、lastModified()、suffix() 等方法用于获取文件属性详情。

## 6. 官方文档参考链接

[Qt 文档 -- QTreeView](https://doc.qt.io/qt-6/qtreeview.html) -- Model 驱动树视图

[Qt 文档 -- QStandardItemModel](https://doc.qt.io/qt-6/qstandarditemmodel.html) -- 标准 Model 实现

[Qt 文档 -- QFileSystemModel](https://doc.qt.io/qt-6/qfilesystemmodel.html) -- 文件系统 Model

[Qt 文档 -- Model/View Programming](https://doc.qt.io/qt-6/model-view-programming.html) -- Model/View 编程指南

---

到这里，QTreeView 的核心用法就全部讲完了。QStandardItemModel 配合 QTreeView 构建通用的树形数据展示，QFileSystemModel 提供了直接对接文件系统的开箱即用方案，setRootIndex 精确控制显示范围，expand/collapse/expandAll 让你在代码中灵活控制节点的展开状态。当你理解了 Model/View 架构在树形控件中的应用方式，QTreeView 的灵活性和可扩展性就远非 QTreeWidget 能比了。
