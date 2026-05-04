# 现代Qt开发教程（新手篇）3.3——Model/View 架构入门

## 1. 前言 / 为什么需要 Model/View

我最早写 Qt 界面的时候，展示一列数据的方式特别粗暴——直接往 QListWidget 里面 `addItem()`，一个个塞字符串。如果数据源变了，就手动清空列表再重新塞一遍。这样做十几个条目还行，一旦数据量上去了，或者你需要在同一个数据上展示不同的视图（比如列表视图和表格视图同步显示同一份数据），代码就会变得一团乱。更尴尬的是，QListWidget 把数据和显示绑死在一起了，你想换个展示方式就得把数据逻辑全改一遍。

后来我才搞明白，Qt 有一套专门的 Model/View 架构来解决这个问题。这套架构的核心思想其实是从经典的 MVC（Model-View-Controller）模式演变来的，只不过 Qt 把 Controller 的职责拆分到了 View 和一个叫 Delegate 的组件里。Model 负责持有和管理数据，View 负责把数据渲染到屏幕上，Delegate 负责控制每个数据项的编辑方式和外观。三者的职责边界非常清晰，你可以换 View 不动 Model，也可以换 Model 不动 View，这在做复杂界面的时候是一个巨大的优势。

我们这篇文章的目标很明确：搞清楚 Qt Model/View 架构的基本分工，然后用两个最常用的预置 Model——QStringListModel 和 QStandardItemModel——配合 QListView 和 QTableView 做出能跑的示例。最后我们会深入三个核心接口 `data()`、`setData()` 和 `rowCount()`，理解它们在整条数据链路里扮演的角色。

## 2. 环境说明

本篇代码基于 Qt 6.5+，CMake 3.26+，C++17 标准。所有 Model/View 相关的类都在 QtWidgets 模块中（QStringListModel 需要 QtCore），链接时加上 Qt6::Core 和 Qt6::Widgets 即可。示例代码在任何支持 Qt6 的桌面平台上都能编译运行。

## 3. 核心概念讲解

### 3.1 从 MVC 到 Qt 的 Model/View/Delegate

经典的 MVC 模式你应该在各种框架里见过：Model 持有数据，View 负责显示，Controller 处理用户输入并把变更同步给 Model。Qt 没有直接照搬这个三件套，而是做了一点调整——它把 Controller 的功能拆分到了 View 和 Delegate 两个组件里。

在 Qt 的设计里，Model 是一个继承自 QAbstractItemModel 的对象，它的职责就是提供数据。外部不管怎么问它要数据——"你有多少行"、"第 i 行第 j 列的值是什么"、"显示用的图标是什么"——Model 都通过统一的接口回答。View 是一个继承自 QAbstractItemView 的对象（比如 QListView、QTableView、QTreeView），它从 Model 那里拉取数据，然后按照自己的方式渲染。Delegate 则是夹在 Model 和 View 之间的一个中间层，负责定制每个数据项的绘制方式（怎么画）和编辑方式（怎么改）。

你会发现，这个设计的好处在于：同一份数据可以被多个 View 同时使用，你在 Model 里改了数据，所有关联的 View 都会自动刷新。反过来，你想换一种展示方式——比如把列表视图换成表格视图——只需要换 View，数据层完全不用动。这就是关注点分离带来的灵活性。

对于入门阶段，Qt 提供了几个预置的 Model 可以直接拿来用，不需要你自己去继承 QAbstractItemModel。最常用的是 QStringListModel（管理一组字符串）和 QStandardItemModel（管理二维表格数据）。我们先从最简单的 QStringListModel 开始。

### 3.2 QStringListModel + QListView：最简组合

QStringListModel 是 Qt 提供的预置 Model 中最简单的一个，它持有的数据就是一个 QStringList。我们用它配合 QListView，实现一个可以增删改的字符串列表。

先看最基本的使用方式——把一组字符串显示到列表里：

```cpp
auto *model = new QStringListModel(this);
model->setStringList({"苹果", "香蕉", "橘子", "西瓜", "葡萄"});

auto *listView = new QListView;
listView->setModel(model);
```

四行代码就搞定了。我们创建了一个 QStringListModel，塞进去一组水果名称，然后把它设置给 QListView。QListView 会自动调用 Model 的 `rowCount()` 拿到行数，然后逐行调用 `data()` 获取每一行的显示文本，最后把所有条目渲染成一个可滚动的列表。

接下来我们让这个列表支持编辑。QListView 默认是只读的，你需要调用 `listView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed)` 来启用编辑触发方式。启用之后，双击或按 F2 就能进入编辑状态。编辑完成后，QListView 会自动调用 Model 的 `setData()` 方法把新值写回 Model。

我们来看一个完整一点的例子，包含增删改操作：

```cpp
// 添加一行
void addItem(QStringListModel *model)
{
    int row = model->rowCount();
    model->insertRow(row);
    // insertRow 只是增加了一行空数据，还需要设置默认值
    QModelIndex index = model->index(row);
    model->setData(index, "新条目");
}

// 删除选中行
void removeItem(QListView *listView, QStringListModel *model)
{
    QModelIndex index = listView->currentIndex();
    if (index.isValid()) {
        model->removeRow(index.row());
    }
}
```

这里有个细节值得注意：`insertRow()` 只是在 Model 里插入了一行空数据，它不会帮你填默认值。你必须紧接着调用 `setData()` 来设置这行的显示文本，否则列表里会出现一个空白行。很多初学者会以为 `insertRow()` 会自动生成一个"新条目"之类的占位文本，但实际上它只是开辟了空间，内容需要你自己填。

`removeRow()` 就简单多了，传入行号直接删除。删除后 Model 会通知所有关联的 View 刷新，不需要你手动做任何事情。

### 3.3 QStandardItemModel + QTableView：表格数据

QStringListModel 只能处理一维的字符串列表。当你需要展示二维表格数据——比如一个学生名单，每行有姓名、年龄、成绩三列——就需要用 QStandardItemModel 了。

QStandardItemModel 的数据组织方式是：每个单元格都是一个 QStandardItem 对象，你可以给每个单元格设置文本、图标、字体、背景色、对齐方式等属性。这种设计比直接操作原始数据要重一些（每个单元格都是一个堆上的对象），但在数据量不大的场景下用起来非常方便。

```cpp
// 创建一个 4 行 3 列的表格 Model
auto *model = new QStandardItemModel(4, 3, this);
model->setHorizontalHeaderLabels({"姓名", "年龄", "成绩"});

// 填充数据
model->setItem(0, 0, new QStandardItem("张三"));
model->setItem(0, 1, new QStandardItem("20"));
model->setItem(0, 2, new QStandardItem("92"));

model->setItem(1, 0, new QStandardItem("李四"));
model->setItem(1, 1, new QStandardItem("21"));
model->setItem(1, 2, new QStandardItem("88"));

// ...更多行
```

`setHorizontalHeaderLabels()` 设置水平表头，`setItem(row, col, item)` 在指定行列放置一个 QStandardItem。QStandardItem 的构造函数接受 QString，所以你可以直接传字符串字面量。

然后用 QTableView 来展示这个 Model：

```cpp
auto *tableView = new QTableView;
tableView->setModel(model);
tableView->horizontalHeader()->setStretchLastSection(true);
```

`setStretchLastSection(true)` 让最后一列自动拉伸填满表格的剩余宽度，这样右侧不会出现一片空白。如果你希望所有列都均分宽度，可以逐列调用 `tableView->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch)`。

QStandardItemModel 也可以和 QTreeView 搭配，因为 QStandardItem 支持父子关系——你可以通过 `setItem()` 嵌套 QStandardItem 来构建树形结构。不过这个属于进阶用法，我们这篇文章先聚焦在一维列表和二维表格上。

### 3.4 三个核心接口：data()、setData()、rowCount()

到现在我们一直用的是 Qt 预置的 Model，调用的是它们封装好的高级 API（比如 `setStringList()`、`setItem()`）。但在 Qt Model/View 架构的底层，所有 Model 最终都要通过三个最基础的接口和 View 通信：`data()`、`setData()` 和 `rowCount()`。理解这三个接口是后续自定义 Model 的前置知识。

`rowCount(const QModelIndex &parent = QModelIndex())` 告诉 View "我这个 Model 有多少行数据"。对于一维列表 Model 来说，parent 参数始终是无效的 QModelIndex（也就是默认值），返回值就是列表的长度。对于树形 Model，parent 指定了你要查的是哪个父节点下的子节点数量。

`data(const QModelIndex &index, int role = Qt::DisplayRole)` 是 View 获取数据的唯一通道。index 告诉 Model "我要第几行第几列的数据"，role 告诉 Model "我要这份数据的哪个方面"。role 是一个非常关键的概念——同一个单元格的数据，View 可能会问很多遍，每次带着不同的 role。`Qt::DisplayRole` 是显示用的文本，`Qt::EditRole` 是编辑时用的文本（可以跟显示的不一样），`Qt::DecorationRole` 是图标，`Qt::ToolTipRole` 是悬停提示，`Qt::BackgroundRole` 是背景色，等等。Model 根据 role 返回不同的 QVariant。

`setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)` 是反向操作——View（或者 Delegate）在用户编辑完数据后，通过这个接口把新值写回 Model。默认的 role 是 `Qt::EditRole`，也就是写入编辑数据。

我们把这三个接口的协作流程串起来看一下。View 首先调用 `rowCount()` 知道要渲染多少行；然后对每一行的每一个可见列，调用 `data(index, Qt::DisplayRole)` 拿到显示文本并渲染出来；当用户双击某个单元格编辑完成后，View 调用 `setData(index, newValue, Qt::EditRole)` 把新值写回 Model；Model 内部更新数据后，发出 `dataChanged` 信号通知 View 刷新。这就是整条数据链路的完整闭环。

你可以试着直接调用这些底层接口来操作预置 Model。比如对 QStringListModel 调用 `model->setData(model->index(0), "改过的文本")`，效果和直接改底层 QStringList 是一样的。对 QStandardItemModel 调用 `model->setData(model->index(0, 0), "新名字")` 等价于重新 `setItem(0, 0, new QStandardItem("新名字"))`。底层走的是同一条路径，只不过预置 Model 在上面封装了一层更方便的 API。

## 4. 踩坑预防

第一个坑是忘记给 View 设置 Model。`QListView::setModel()` 不调用的话，View 显示为空白，不会报任何错误。这是一个很容易被忽略的步骤，尤其是当你把创建 Model 和创建 View 的代码分散在不同的函数里的时候。养成习惯：创建完 Model 立刻 `setModel()`，不要拖到后面。

第二个坑是对 `data()` 的 role 参数理解不到位。最常见的错误是自定义 Model 时只处理 `Qt::DisplayRole`，忽略了其他 role。这样做的后果是：列表确实能显示文本，但你可能会发现选中项的高亮不对、工具提示不显示、编辑行为异常——这些都是因为 Model 对其他 role 返回了空的 QVariant。正确的做法是至少处理 `Qt::DisplayRole`、`Qt::EditRole` 和 `Qt::DecorationRole`，其他 role 如果不需要特殊处理就让它走默认值。

第三个坑是 `insertRow()` 后忘记 `setData()`。前面已经提到了，`insertRow()` 只分配空间不填充内容。如果你在代码里连续 `insertRow()` 了若干行但忘了设置数据，列表里会出现一堆空白行，这些空白行不是 bug——而是你真的放了空字符串进去。调试的时候看到空白行，先检查一下是不是忘了 `setData()`。

第四个坑是 QStandardItemModel 的内存管理。`setItem()` 的内部实现是把传入的 QStandardItem 的所有权转移给 Model，所以你不需要手动 delete 这些 item。但如果你通过 `takeItem()` 或者 `takeRow()` 把 item 从 Model 里取出来了，所有权就回到了你手上，你必须在合适的时候 delete 它。这个"所有权跟着 Model 走"的规则和 Qt 的父子对象树是同一套机制，理解了这一点就不会在内存管理上踩坑。

第五个坑是在多 View 共享同一个 Model 的场景下，一个 View 的编辑操作会立刻反映到所有 View 上。这不是 bug，而是 Model/View 架构的设计意图。但如果你没有预期到这种行为，可能会觉得"我明明只改了一个表格，为什么另一个表格也变了"。多个 View 共享 Model 的时候，所有 View 读到的永远是同一份数据。

## 5. 练习项目

我们来做一个综合练习：用 QStringListModel 和 QStandardItemModel 构建一个简单的"学生成绩管理"界面。界面分上下两部分：上半部分是一个 QTableView，用 QStandardItemModel 展示一个 4 列的表格（姓名、语文、数学、英语），支持双击编辑和右键删除行；下半部分是一个 QListView，用 QStringListModel 展示所有学生的姓名列表，和表格的第一列保持同步——当你在表格里改了一个学生的姓名，列表里也要跟着变。

实现同步的关键是：监听 QStandardItemModel 的 `dataChanged` 信号，当第一列（姓名列）的数据发生变化时，从 Model 中提取所有姓名组成一个新的 QStringList，然后 `setStringList()` 刷新 QStringListModel。反过来，在 QListView 里添加一个新条目时，也要在 QStandardItemModel 里同步添加一行。

这个练习会逼着你想清楚两个 Model 之间的数据同步逻辑。虽然在实际工程中你不太可能用两个独立的 Model 来管理有关联的数据（更好的做法是自定义一个统一的 Model），但作为入门练习，它能帮你理解 Model 的 `data()`/`setData()`/`dataChanged` 信号这套机制是怎么运作的。

几个提示：QStandardItemModel 的 `dataChanged` 信号携带了 topLeft 和 bottomRight 两个 QModelIndex，你可以通过它们判断哪些单元格发生了变化；`setStringList()` 会整体替换 QStringListModel 的底层数据，所以每次同步都需要重新构造完整的 QStringList；右键菜单可以用 `setContextMenuPolicy(Qt::CustomContextMenu)` 配合 `customContextMenuRequested` 信号来实现。

## 6. 官方文档参考链接

[Qt 文档 · Model/View Programming](https://doc.qt.io/qt-6/model-view-programming.html) -- Qt Model/View 架构的完整概述，包含设计理念和各组件之间的关系

[Qt 文档 · QStringListModel](https://doc.qt.io/qt-6/qstringlistmodel.html) -- 字符串列表 Model 的 API 文档，setStringList / setData / insertRow 等方法

[Qt 文档 · QStandardItemModel](https://doc.qt.io/qt-6/qstandarditemmodel.html) -- 标准项 Model 的 API 文档，setItem / setHorizontalHeaderLabels 等方法

[Qt 文档 · QListView](https://doc.qt.io/qt-6/qlistview.html) -- 列表视图的 API 文档，setModel / setEditTriggers 等配置

[Qt 文档 · QTableView](https://doc.qt.io/qt-6/qtableview.html) -- 表格视图的 API 文档，表头配置、列宽策略、选中行为等

[Qt 文档 · Qt::ItemDataRole](https://doc.qt.io/qt-6/qt.html#ItemDataRole-enum) -- 所有数据角色的枚举定义，理解 role 参数的关键参考

---

到这里，Qt 的 Model/View 架构你就入门了。预置 Model 能覆盖大部分简单的数据展示场景，当你遇到更复杂的需求——比如从数据库拉数据、从网络 API 获取 JSON 再展示、或者需要自定义排序过滤逻辑——那才是真正需要继承 QAbstractItemModel 自己写的时候。但不管 Model 多复杂，底层走的一直是 `data()` / `setData()` / `rowCount()` 这三个接口和 View 通信。下一篇文章我们来聊 QSS 样式表，那是一套完全不同的能力：让界面好看起来。
