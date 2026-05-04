# 现代Qt开发教程（新手篇）3.51——QTableView：Model 驱动表格视图

## 1. 前言 / 当 QTableWidget 不够用时

上一篇我们聊了 QTableWidget，一个把 Model 和 View 合二为一的便捷表格控件。如果你只是想快速搞一个成绩单、配置参数表或者 CSV 预览面板，它确实够用。但现实项目里的需求往往没这么简单——你可能会碰到同一个数据源需要同时用表格和表单两种方式展示，可能会需要对接数据库做实时查询，可能会需要对表格数据做排序和过滤。这些场景下 QTableWidget 就显得捉襟见肘了，原因和我们之前聊 QListWidget 时完全一样：QTableWidget 内部的 Model 被封装死了，你没法把它拿出来和别的 View 共享，也没法在数据层灵活地做操作。

QTableView 是 Qt Model/View 架构中专门处理二维表格数据的视图类。和 QListView、QTreeView 一样，它本身不持有任何数据——所有数据来自外部传入的 Model。你可以给同一个 Model 同时挂一个 QTableView 和一个 QTreeView，数据只存一份但展示形式完全不同。你可以自由替换 Model 的实现——用 QStandardItemModel 做通用表格数据、用 QSqlTableModel 直接对接数据库表、甚至完全自己写一个 QAbstractTableModel 子类对接你的业务数据源。这种灵活性是 QTableWidget 永远给不了的。

今天的内容围绕四个方面展开。先看 QTableView 如何与 QStandardItemModel 和 QSqlTableModel 配合工作，然后通过 horizontalHeader 和 verticalHeader 来精确控制表头的行为，接着用 setSpan 实现合并单元格，最后用 resizeColumnsToContents 做自动列宽适配。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QTableView 和 QStandardItemModel 在 QtWidgets 模块中，QSqlTableModel 在 QtSql 模块中。基础示例只需要链接 Qt6::Widgets，涉及数据库的示例需要额外链接 Qt6::Sql。示例代码涉及 QTableView、QStandardItemModel、QSqlTableModel、QSqlDatabase、QHeaderView、QLabel、QPushButton、QVBoxLayout、QHBoxLayout 和 QSortFilterProxyModel。

## 3. 核心概念讲解

### 3.1 与 QStandardItemModel 配合构建通用表格

QStandardItemModel 是 Qt 提供的最灵活的标准 Model 实现，我们在 QListView 和 QTreeView 篇里已经和它打过交道了。在表格场景下，QStandardItemModel 的用法更直观——行和列就是二维平面上的坐标，不存在树形结构那样的父子嵌套关系。每个 QStandardItem 对应表格中的一个单元格，通过 setItem(row, column, item) 把它放到指定的行列位置上。

创建一个基于 QStandardItemModel 的 QTableView 基本流程非常清晰：先 new 一个 QStandardItemModel，然后通过 setHorizontalHeaderLabels 设置列标题，接着逐行逐列创建 QStandardItem 并 setItem 到对应位置，最后调用 QTableView 的 setModel 把 Model 挂上去。你会发现这个流程和 QTableWidget 的操作方式非常相似，区别在于 Model 和 View 是分离的两个对象——这意味着你可以在不改动 View 的前提下替换整个 Model，也可以把同一个 Model 同时喂给多个 View。

```cpp
auto *model = new QStandardItemModel(this);
model->setHorizontalHeaderLabels({"姓名", "部门", "工龄", "绩效"});

// 填充数据
model->setItem(0, 0, new QStandardItem("陈一"));
model->setItem(0, 1, new QStandardItem("研发部"));
model->setItem(0, 2, new QStandardItem("3"));
model->setItem(0, 3, new QStandardItem("A"));

model->setItem(1, 0, new QStandardItem("刘二"));
model->setItem(1, 1, new QStandardItem("产品部"));
model->setItem(1, 2, new QStandardItem("5"));
model->setItem(1, 3, new QStandardItem("S"));

auto *tableView = new QTableView;
tableView->setModel(model);
```

这里有一个细节值得注意：QStandardItemModel 的 setItem 和 QTableWidget 的 setItem 虽然名字一样，但语义完全不同。QTableWidget 的 setItem 是直接操作 View 内部持有的 Model，而 QStandardItemModel 的 setItem 是操作 Model 自身的数据——View 只是数据的被动展示者。这意味着你可以在任何时候修改 Model 的数据，QTableView 会通过 Model 发出的 dataChanged 信号自动刷新显示，不需要手动调用任何 update 方法。

QStandardItemModel 也支持批量操作。appendRow(const QList<QStandardItem *> &items) 一次性追加一整行的数据——传入的 QList 中每个 QStandardItem 对应一列。这比逐个 setItem 更高效也更不容易出错，因为你不需要手动跟踪列号。

```cpp
// 批量追加一行
QList<QStandardItem *> rowItems;
rowItems << new QStandardItem("张三")
         << new QStandardItem("设计部")
         << new QStandardItem("2")
         << new QStandardItem("B+");
model->appendRow(rowItems);
```

和 QTableWidget 一样，QStandardItem 支持丰富的属性——setText 设置文本，setIcon 设置图标，setData(Qt::UserRole, value) 存储自定义数据，setFlags 控制交互行为（可编辑、可选中等）。这些属性的用法在 QTableWidget 篇里已经详细讲过，这里不再重复。核心区别在于：在 QTableWidget 中你直接操作 View 的方法，在 QTableView 中你通过 Model 的方法来操作数据。

### 3.2 与 QSqlTableModel 对接数据库

如果说 QStandardItemModel 是"手动搭建"的方案，那 QSqlTableModel 就是"自动对接"的方案。QSqlTableModel 直接绑定到数据库中的一张表，自动读取数据、自动监听变更、自动提交修改。你不需要手写 SQL 查询语句，不需要手动把查询结果转成 Model 数据——设置好表名之后它就把整张表映射成 Model 的行和列。

```cpp
// 先建立数据库连接
QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
db.setDatabaseName(":memory:");  // 内存数据库，适合演示
if (!db.open()) {
    qDebug() << "数据库打开失败:" << db.lastError().text();
    return;
}

// 创建 QSqlTableModel 并绑定表
auto *sqlModel = new QSqlTableModel(this, db);
sqlModel->setTable("employees");
sqlModel->setEditStrategy(QSqlTableModel::OnFieldChange);
sqlModel->select();  // 执行查询，加载数据

auto *tableView = new QTableView;
tableView->setModel(sqlModel);
```

QSqlTableModel 的 select() 方法执行一次 SELECT * FROM table 查询，把整张表的数据加载到 Model 中。加载完成后 QTableView 就能正常显示数据了。select() 的返回值告诉你查询是否成功——如果返回 false，你可以通过 lastError() 获取错误信息。

setEditStrategy 控制用户在 QTableView 中编辑单元格后，修改何时被提交到数据库。QSqlTableModel::OnFieldChange 是最激进的策略——每次修改一个单元格就立刻提交到数据库，适合单用户场景。QSqlTableModel::OnRowChange 在用户切换到另一行时提交上一行的修改——这是一种"行级事务"的策略，用户可以修改一行的多个字段后一起提交。QSqlTableModel::OnManualSubmit 最保守——所有修改都暂存在内存中，只有你手动调用 submitAll() 才会提交到数据库，调用 revertAll() 则放弃所有未提交的修改。在多用户并发场景下，OnManualSubmit 是最安全的选择。

QSqlTableModel 默认会用数据库表的列名作为表头标签。如果你想自定义表头文本，可以通过 setHeaderData 方法来覆盖。

```cpp
sqlModel->setHeaderData(0, Qt::Horizontal, "员工编号");
sqlModel->setHeaderData(1, Qt::Horizontal, "姓名");
sqlModel->setHeaderData(2, Qt::Horizontal, "部门");
sqlModel->setHeaderData(3, Qt::Horizontal, "薪资");
```

QSqlTableModel 还支持 setFilter(const QString &) 来添加 WHERE 条件——比如 sqlModel->setFilter("salary > 5000") 只显示薪资大于 5000 的记录。setSort(int column, Qt::SortOrder) 设置默认排序——比如 sqlModel->setSort(3, Qt::DescendingOrder) 按第四列降序排列。修改 filter 或 sort 之后需要重新调用 select() 来刷新数据。

QTableView 配合 QSqlTableModel 的一个强大之处在于：用户可以直接在表格中编辑数据，修改会自动同步到数据库。你不需要写任何"保存"逻辑——这是 Model/View 架构带来的直接好处，View 负责展示和捕获用户输入，Model 负责存储和持久化，两者各司其职。

### 3.3 horizontalHeader / verticalHeader 表头控制

QTableView 的水平表头和垂直表头都是 QHeaderView 对象，通过 horizontalHeader() 和 verticalHeader() 获取。QHeaderView 提供了比 QTableWidget 更丰富的表头控制能力——因为 QTableView 是 Model 驱动的，表头的行为需要和 Model 的数据结构配合。

最常用的表头操作是控制列宽策略。QHeaderView::setSectionResizeMode 提供了四种模式。Interactive 是默认模式——用户可以拖拽表头分隔线手动调整列宽。Stretch 让所有列等比例拉伸占满整个表格宽度，用户不能手动调整。ResizeToContents 根据每列的内容自动调整列宽，每列的宽度刚好容纳最长的内容。Fixed 需要配合 setSectionWidth(logicalIndex, size) 使用——列宽固定为指定值，用户不能拖拽。

```cpp
auto *header = tableView->horizontalHeader();

// 混合策略：前两列自适应内容，最后一列拉伸占满
header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
header->setSectionResizeMode(2, QHeaderView::Stretch);
```

setSectionResizeMode 可以接受两种参数形式。如果传入一个逻辑索引加一个模式，它只影响那一列。如果只传入一个模式，它影响所有列。混合使用不同的策略是实际开发中最常见的做法——ID 列和状态列用 ResizeToContents 保证紧凑，描述性的长文本列用 Stretch 占满剩余空间。

表头的显示和隐藏也有独立的控制。horizontalHeader()->hide() 隐藏整个水平表头，verticalHeader()->hide() 隐藏整个垂直表头（行号）。如果你只想隐藏某一列而不是整个表头，应该用 QTableView 的 setColumnHidden(int column, bool hide)——这会在 View 层面隐藏列，但 Model 的数据不受影响。

```cpp
tableView->verticalHeader()->hide();          // 隐藏行号
tableView->setColumnHidden(2, true);           // 隐藏第三列
```

verticalHeader 的 setDefaultSectionSize(int) 可以设置所有行的默认高度。这在表格数据量较大时很有用——把行高从默认的 30 像素缩小到 24 像素，同样的视口面积能多显示好几行数据。horizontalHeader 也有对应的 setDefaultSectionSize。

```cpp
// 缩小行高，让表格更紧凑
tableView->verticalHeader()->setDefaultSectionSize(24);
```

还有一个实用的方法是 horizontalHeader()->setStretchLastSection(bool)。当设为 true 时，最后一列会自动拉伸占满剩余的表格宽度——这比把最后一列单独设为 Stretch 模式更方便，因为你不需要知道最后一列的索引。

### 3.4 setSpan 合并单元格

合并单元格是表格控件中一个相对小众但有时又必不可少的功能——比如在报表中把"大分类"横跨多列显示，或者在日程表中把一个跨越多个时间段的事件合并成一个大的单元格。QTableView::setSpan(int row, int column, int rowSpan, int columnSpan) 就是干这个的：从 (row, column) 位置开始，把 rowSpan 行 columnSpan 列的单元格合并成一个。合并后只有左上角的单元格会显示内容，其他被合并的单元格内容被隐藏。

```cpp
auto *model = new QStandardItemModel(6, 4, this);
model->setHorizontalHeaderLabels({"季度", "产品A", "产品B", "产品C"});

// 填充数据...

auto *tableView = new QTableView;
tableView->setModel(model);

// 合并第一行的 1-3 列，让"上半年汇总"横跨三列
model->setItem(0, 0, new QStandardItem("Q1-Q2"));
tableView->setSpan(0, 1, 1, 3);  // 第 0 行，从第 1 列开始，跨 1 行 3 列
```

setSpan 的参数含义需要搞清楚。row 和 column 是合并区域的起始位置，rowSpan 是合并的行数，columnSpan 是合并的列数。所以 setSpan(2, 0, 3, 1) 表示从第 2 行第 0 列开始，纵向合并 3 行 1 列——这在"一个分类跨越多行"的场景下很常见。

```cpp
// 纵向合并：第 2-4 行的第一列合并为一个"研发部"标签
model->setItem(2, 0, new QStandardItem("研发部"));
tableView->setSpan(2, 0, 3, 1);  // 起始(2,0)，跨3行1列
```

有一个非常容易踩的坑：setSpan 是 QTableView 的方法而不是 Model 的方法。这意味着合并信息存储在 View 层面，不在 Model 层面。如果你给同一个 Model 挂了两个 QTableView，你需要在每个 View 上分别调用 setSpan——它们不会自动同步合并状态。这也意味着如果你替换了 Model（重新 setModel），之前的合并信息会丢失，你需要重新设置。

clearSpans() 清除所有合并状态，恢复为独立的单元格。rowSpan(int row, int column) 和 columnSpan(int row, int column) 分别返回指定位置的行跨度数和列跨度数——如果该位置没有被合并，返回 1；如果被合并了但不是左上角起始位置，也返回实际的跨度值。isSpanning(int row, int column) 返回该位置是否处于某个合并区域中。

```cpp
// 检查合并状态
int rs = tableView->rowSpan(2, 0);     // 3
int cs = tableView->columnSpan(0, 1);  // 3
```

合并单元格后，被合并区域的边框绘制会自动调整——Qt 会把合并后的大单元格画成一个完整的矩形，而不是在内部显示分隔线。合并区域内的选中行为也会变化——点击合并后的大单元格时，整个合并区域都会被高亮。如果你自定义了 delegate 的 paint 方法，需要通过 option.index 获取正确的 QModelIndex 来读取数据，而不是通过鼠标位置推算行列号。

### 3.5 resizeColumnsToContents 自动列宽

手动设置每列的列宽在很多场景下是一件费力不讨好的事情——尤其是当数据是动态加载的时候，你很难提前知道每列的最优宽度。QTableView 提供了一组自动列宽方法来解决这个问题。

resizeColumnsToContents() 是最直接的方法——调用一次后，所有列的宽度会根据内容自动调整到刚好容纳最宽条目的程度。它的工作方式是遍历 Model 中的每一列，对每列计算所有行的内容宽度（包括表头文本的宽度），取最大值作为该列的宽度。

```cpp
tableView->resizeColumnsToContents();
```

resizeColumnToContents(int column) 只调整指定列的宽度。如果你只想让某些列自适应而其他列保持固定宽度，用这个单列版本更合适。

```cpp
// 只调整前两列的宽度
tableView->resizeColumnToContents(0);
tableView->resizeColumnToContents(1);
```

resizeColumnsToContents 有一个性能问题需要注意：它会遍历 Model 中所有行的所有可见数据来计算最优宽度。如果你的 Model 有几万行数据，调用一次可能会触发明显的卡顿。在大数据量场景下，有两种应对策略。第一种是用 setSectionResizeMode(column, QHeaderView::ResizeToContents) 代替 resizeColumnsToContents——它只计算当前可见行的内容宽度，不会扫描全部数据，性能好得多。第二种是分批处理——先只加载前 100 行数据，调用 resizeColumnsToContents 计算出列宽，然后手动 setSectionResizeMode(column, QHeaderView::Fixed) 把列宽固定住，再加载剩余数据。

```cpp
// 性能友好的做法：用 ResizeToContents 模式代替一次性调整
tableView->horizontalHeader()
    ->setSectionResizeMode(QHeaderView::ResizeToContents);
```

还有一个经常配合使用的方法是 horizontalHeader()->setMinimumSectionSize(int)。它设置所有列的最小宽度——即使内容很窄，列宽也不会小于这个值。这在某些列的数据很短（比如状态列只有"是"或"否"）但你不想让列被挤得太窄时非常有用。

```cpp
// 列宽最小 60 像素，即使内容只有两个字符
tableView->horizontalHeader()->setMinimumSectionSize(60);
tableView->resizeColumnsToContents();
```

在实际项目中，一个比较推荐的列宽策略组合是这样的：文本较短的列（ID、状态、类型）用 ResizeToContents 保持紧凑，文本较长的列（描述、备注）用 Stretch 占满剩余空间，表头用 setMinimumSectionSize(60) 防止列被挤到看不见。这种组合既能充分利用空间，又能保证内容不被截断。

## 4. 踩坑预防

第一个坑是忘记调用 setModel。QTableView 在没有 Model 的情况下什么都不显示——一个空白的矩形。这和 QTableWidget 不同，QTableWidget 内部自带 Model。如果你发现 QTableView 是空的，第一件事就是检查 setModel 是否被调用了，以及 Model 的 rowCount() 和 columnCount() 是否大于 0。

第二个坑是 setSpan 的合并区域重叠。如果你在同一个位置或者有交叉的区域调用了两次 setSpan，后一次的设置会覆盖前一次，可能导致你预期的合并效果被破坏。如果你需要复杂的合并布局，建议在纸上画好合并区域的坐标，确认没有重叠后再编码。

第三个坑是 QSqlTableModel 的 select() 可能因为 SQL 错误而静默失败。select() 返回 false 表示查询失败，但不会抛异常。如果你在 select() 之后发现 Model 没有数据（rowCount() == 0），先检查 lastError() 的输出，看看是不是表名写错了或者数据库连接断了。

第四个坑是 resizeColumnsToContents 在大数据量下的性能问题。如果你的 Model 有上万行数据，一次性调用 resizeColumnsToContents 可能会导致明显的界面卡顿。用 setSectionResizeMode(column, QHeaderView::ResizeToContents) 替代会好很多，因为它只计算可见行的内容宽度。

第五个坑是 QSqlTableModel 默认会把数据库表的列名原样显示为表头。如果你的数据库列名是 employee_id、dept_name 这种英文风格，直接显示给中国用户看就不太友好。记得用 setHeaderData 来覆盖表头文本。

## 5. 练习项目

我们来做一个综合练习：创建一个"员工管理"窗口。中央是一个 QTableView，使用 QStandardItemModel 管理 10 条员工数据（编号、姓名、部门、职位、工龄、绩效评级共 6 列）。水平表头使用 setHorizontalHeaderLabels 设置中文列标题，垂直表头隐藏。窗口上方有搜索框和部门筛选下拉框——通过 QSortFilterProxyModel 实现实时过滤，搜索框对姓名列做模糊匹配，下拉框对部门列做精确匹配。表格支持按列排序（setSortingEnabled(true)），表头点击切换升序/降序。用 setSpan 合并同一个部门的行，让部门名称纵向跨行显示。窗口底部有一个 QLabel 实时显示"共 X 人，平均工龄 Y.Y 年"。表格列宽策略为混合模式：编号和绩效列用 ResizeToContents，备注列（如果加了的话）用 Stretch。

提示：QSortFilterProxyModel 的 setFilterKeyColumn 设置过滤目标列，setFilterCaseSensitivity(Qt::CaseInsensitive) 忽略大小写。setFilterFixedString 做精确匹配，setFilterRegularExpression 做模糊匹配。排序只需要调用 tableView->setSortingEnabled(true) 并把 proxyModel 设为 tableView 的 Model 即可。

## 6. 官方文档参考链接

[Qt 文档 -- QTableView](https://doc.qt.io/qt-6/qtableview.html) -- Model 驱动表格视图

[Qt 文档 -- QStandardItemModel](https://doc.qt.io/qt-6/qstandarditemmodel.html) -- 标准 Model 实现

[Qt 文档 -- QSqlTableModel](https://doc.qt.io/qt-6/qsqltablemodel.html) -- 数据库表 Model

[Qt 文档 -- QHeaderView](https://doc.qt.io/qt-6/qheaderview.html) -- 表头控件

[Qt 文档 -- QSortFilterProxyModel](https://doc.qt.io/qt-6/qsortfilterproxymodel.html) -- 排序过滤代理 Model

[Qt 文档 -- Model/View Programming](https://doc.qt.io/qt-6/model-view-programming.html) -- Model/View 编程指南

---

到这里，QTableView 的核心用法就全部讲完了。QStandardItemModel 配合 QTableView 构建通用的二维表格数据展示，QSqlTableModel 提供了直接对接数据库表的开箱即用方案，horizontalHeader 和 verticalHeader 让你精确控制表头行为，setSpan 实现了合并单元格，resizeColumnsToContents 和 setSectionResizeMode 的组合提供了灵活的自动列宽策略。当你理解了 Model/View 架构在表格控件中的应用方式，QTableView 的灵活性和可扩展性就远非 QTableWidget 能比了。
