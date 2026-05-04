# 现代Qt开发教程（新手篇）5.2——QSqlTableModel 数据库表格视图

## 1. 前言：当数据库遇上 Model/View

上一篇我们用 `QSqlQuery` 手动执行 SQL、手动遍历结果集、手动把数据填到控件里。这在逻辑层面没什么问题，但如果你的需求就是在界面上展示一张数据库表、让用户直接编辑里面的数据，那手写这些代码就很冗余了。Qt 的 Model/View 架构恰好为这种场景提供了一个现成的解决方案——`QSqlTableModel`。

`QSqlTableModel` 把一张数据库表直接映射成一个 Qt Model，配合 `QTableView` 就能在界面上展示出来，而且自带编辑、排序、过滤功能。你甚至不需要写一行 SQL——增删改查全部由 Model 在底层帮你完成。这篇我们要搞清楚的就是这个机制的工作原理，以及怎么在实际项目中用好它。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要用到两个 Qt 模块：`QtSql`（提供 `QSqlTableModel`）和 `QtWidgets`（提供 `QTableView` 及相关 GUI 组件）。CMake 配置需要同时引入 `Qt6::Sql` 和 `Qt6::Widgets`。编译工具链方面，MSVC 2019+、GCC 11+ 均可，C++17 标准，CMake 3.26+ 构建系统。示例继续使用 SQLite 作为后端数据库，因为它不需要额外安装，非常适合学习和原型开发。

## 3. 核心概念讲解

### 3.1 最简表格——三行代码展示数据库内容

先看一个最小可运行的例子。假设数据库里已经有一张 `employees` 表，我们用三行代码把它显示出来：

```cpp
// 1. 设置数据库连接
QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
db.setDatabaseName("company.db");
db.open();

// 2. 创建 Model 并绑定表名
QSqlTableModel *model = new QSqlTableModel;
model->setTable("employees");
model->select();   // 从数据库加载数据

// 3. 创建 View 并绑定 Model
QTableView *view = new QTableView;
view->setModel(model);
view->show();
```

运行之后你会看到一个完整的表格界面，列名是数据库表的字段名，每一行是一条记录。用户可以直接双击某个单元格进行编辑，修改完按回车或者点击其他行就能保存——没错，`QSqlTableModel` 默认就支持编辑，不需要你写任何额外代码。

这个"三行代码展示数据库"的背后，`QSqlTableModel` 做了大量工作：它自动生成了 `SELECT * FROM employees` 查询语句，把查询结果转换成了 Model 的行列数据结构，并且在用户编辑单元格时自动生成对应的 `UPDATE` 语句提交到数据库。

### 3.2 数据提交与回滚——select / submitAll / revertAll

上一节说到默认就支持编辑，那编辑之后数据是怎么回到数据库的呢？这取决于 `EditStrategy`，我们在下一小节详细讨论。但不管哪种策略，提交和回滚的 API 是统一的。

`select()` 是数据加载的入口。调用它时，Model 会根据当前的表名、过滤条件和排序规则生成一条 SELECT 语句，执行查询，然后把结果集填充到 Model 的内部缓存中。你在 `QTableView` 里看到的每一行数据，都是从这个缓存里来的。

`submitAll()` 把 Model 中所有未保存的修改一次性提交到数据库。这包括新增的行、修改的单元格、删除的行——所有挂起的变更都会在这一步变成真正的 SQL 语句发往数据库。如果提交成功返回 `true`，否则返回 `false`，你可以通过 `lastError()` 查看失败原因。

`revertAll()` 则是放弃所有未保存的修改，把 Model 恢复到上次 `select()` 时的状态。这在用户改了一堆数据之后突然反悔的时候特别有用。

```cpp
// 提交所有修改
if (!model->submitAll()) {
    qWarning() << "提交失败:" << model->lastError().text();
}

// 或者放弃所有修改
model->revertAll();
```

这里有一个容易踩的坑：`submitAll()` 和 `revertAll()` 操作的是整个 Model 的全部修改，不是单行。如果你只想提交某一行的修改，用 `submitRow()` 在 Qt 6 里并没有直接提供，你需要通过 `EditStrategy` 来控制提交时机。

### 3.3 EditStrategy——三种编辑策略的选择

`QSqlTableModel` 提供了三种编辑策略，通过 `setEditStrategy()` 设置，它们决定了用户在界面上编辑数据之后，修改何时被提交到数据库：

```cpp
// 策略一：行切换时自动提交（默认值）
model->setEditStrategy(QSqlTableModel::OnRowChange);

// 策略二：手动提交
model->setEditStrategy(QSqlTableModel::OnManualSubmit);

// 策略三：字段变更时立即提交
model->setEditStrategy(QSqlTableModel::OnFieldChange);
```

`OnFieldChange` 是最激进的方式——用户每修改一个单元格，修改立即写入数据库。好处是你永远不需要手动提交，坏处是没有撤销的机会，而且频繁的数据库写入在数据量大的时候会严重影响性能。另外，如果用户正在编辑某条重要的财务数据，手滑敲错一个数字就直接入库了，这种体验是很糟糕的。

`OnRowChange` 是默认策略——当用户编辑完一行后，光标移到其他行时自动提交上一行的修改。这种方式在提交频率和编辑体验之间取得了一个不错的平衡。用户在一行内的多个字段可以反复修改，只有离开这行的时候才会提交。但它也有一个隐含的问题：如果你的 `QTableView` 只有一行数据，用户编辑完之后根本没地方"切换到其他行"，那修改就一直不会提交。解决办法是在界面上加一个"保存"按钮，手动调用 `submitAll()`。

`OnManualSubmit` 是最保守也最安全的方式——所有修改都只在内存中累积，直到你显式调用 `submitAll()` 才写入数据库。这是实际工程中最推荐的方式，因为它允许你在提交之前做数据校验、允许用户撤销操作、也方便做事务管理。代价是你需要自己处理提交逻辑。

选择策略的原则其实很简单：对于需要数据校验或者允许撤销的场景用 `OnManualSubmit`，对于简单的数据浏览和快速编辑用 `OnRowChange`，尽量不要用 `OnFieldChange`——除非你真的需要在每次字段变更时触发某些业务逻辑。

### 3.4 过滤与排序——setFilter / setSort

数据库表往往有几千甚至几万条记录，全部加载到界面上既慢又不实用。`QSqlTableModel` 提供了在 SQL 层面进行过滤和排序的能力。

`setFilter()` 设置 WHERE 子句的条件：

```cpp
// 只显示 Engineering 部门的员工
model->setFilter("department = 'Engineering'");
model->select();  // 设置 filter 后必须重新 select 才能生效

// 组合条件
model->setFilter("department = 'Engineering' AND salary > 70000");
model->select();

// 模糊搜索
model->setFilter("name LIKE '%ali%'");
model->select();
```

注意 `setFilter()` 的参数是纯 SQL 的 WHERE 条件，不带 `WHERE` 关键字。这意味着你可以使用 SQL 的全部条件语法——AND、OR、LIKE、IN、BETWEEN 都可以。但正因为如此，这里存在 SQL 注入的风险。如果你的过滤条件包含用户输入，一定要做参数化处理或者至少做字符串转义。`setFilter()` 本身不支持预编译参数绑定，所以安全起见，对于用户输入的过滤值，建议用 `QString` 拼接时对特殊字符做转义，或者用 `QSqlQuery::prepare()` 先做一层参数化查询。

`setSort()` 设置 ORDER BY 子句：

```cpp
// 按薪资降序排列
model->setSort(3, Qt::DescendingOrder);  // 第 3 列（salary），降序
model->select();

// 按部门升序排列
model->setSort(2, Qt::AscendingOrder);   // 第 2 列（department），升序
model->select();
```

`setSort()` 的第一个参数是列索引（从 0 开始），第二个参数是排序方向。列索引对应的是 `SELECT *` 查询结果的列顺序，也就是数据库表的字段定义顺序。你也可以用 `QSqlTableModel::setSort(int column, Qt::SortOrder order)` 来实现点击表头排序——`QTableView` 本身支持点击表头触发排序信号，你只需要在槽函数里调用 `setSort()` 并 `select()`。

每次修改了 filter 或 sort 之后，都必须调用 `select()` 才能让新的查询生效。这是因为 filter 和 sort 只是修改了 Model 内部的 SQL 语句模板，真正的查询是在 `select()` 时执行的。

### 3.5 自定义表头与列可见性

默认情况下，`QSqlTableModel` 会用数据库表的字段名作为列标题。这在开发和调试阶段够用，但给最终用户看的话就不太友好了——"hire_date" 这种名字普通用户根本看不懂。

```cpp
// 修改表头显示文本
model->setHeaderData(0, Qt::Horizontal, "编号");
model->setHeaderData(1, Qt::Horizontal, "姓名");
model->setHeaderData(2, Qt::Horizontal, "部门");
model->setHeaderData(3, Qt::Horizontal, "薪资");

// 隐藏不需要显示的列（比如 id 列）
view->hideColumn(0);

// 设置列宽
view->setColumnWidth(1, 150);
view->setColumnWidth(2, 120);
view->setColumnWidth(3, 100);
```

`setHeaderData()` 修改的是 Model 层的数据，所以即使你换了 View（比如从 `QTableView` 换成 `QListView`），表头文本也会保持一致。`hideColumn()` 则是 View 层的操作，它只是让某一列不可见，数据仍然在 Model 里。

### 3.6 新增与删除行

`QSqlTableModel` 提供了 `insertRow()` 和 `removeRow()` 方法来操作数据行：

```cpp
// 在末尾新增一行
int row = model->rowCount();
model->insertRow(row);

// 设置新行的数据
model->setData(model->index(row, 1), "New Employee");
model->setData(model->index(row, 2), "Sales");
model->setData(model->index(row, 3), 50000.0);

// 如果是 OnManualSubmit 策略，需要手动提交
model->submitAll();

// 删除当前选中行
int selectedRow = view->currentIndex().row();
model->removeRow(selectedRow);
model->submitAll();
```

`insertRow()` 在 Model 的缓存中创建一个空行，`setData()` 填充各列的值。如果策略是 `OnFieldChange` 或 `OnRowChange`，修改会在合适的时机自动提交；如果是 `OnManualSubmit`，你需要手动 `submitAll()`。

`removeRow()` 标记指定行为"待删除"，实际删除发生在提交时。这意味着在 `OnManualSubmit` 策略下，你可以先删除几行，然后 `revertAll()` 全部撤销——数据不会真的从数据库里消失。

## 4. 综合示例：可编辑的员工管理表格

把前面学的串起来，我们写一个完整的 GUI 程序——一个带工具栏按钮的员工管理表格，支持新增、删除、筛选、撤销和保存：

```cpp
// 创建 Model 并配置
QSqlTableModel *model = new QSqlTableModel;
model->setTable("employees");
model->setEditStrategy(QSqlTableModel::OnManualSubmit);
model->setSort(3, Qt::DescendingOrder);  // 默认按薪资降序
model->setHeaderData(0, Qt::Horizontal, "ID");
model->setHeaderData(1, Qt::Horizontal, "姓名");
model->setHeaderData(2, Qt::Horizontal, "部门");
model->setHeaderData(3, Qt::Horizontal, "薪资");
model->select();

// 创建 View
QTableView *view = new QTableView;
view->setModel(model);
view->hideColumn(0);        // 隐藏 ID 列
view->setAlternatingRowColors(true);  // 交替行背景色

// 工具栏按钮
// 添加 -> model->insertRow(model->rowCount())
// 删除 -> model->removeRow(view->currentIndex().row())
// 保存 -> model->submitAll()
// 撤销 -> model->revertAll()
// 刷新 -> model->select()
```

这套组合覆盖了日常数据管理的所有基本操作，而且代码量非常少。`QSqlTableModel` 的优势就在于此——它把数据库操作和界面展示之间的桥梁搭好了，你只需要关注业务逻辑。

## 5. 练习项目

练习项目：带筛选功能的学生成绩管理界面。

我们要做一个基于 `QSqlTableModel` + `QTableView` 的 GUI 程序，管理一张学生成绩表。表结构包含学号、姓名、科目、成绩四个字段。

完成标准是这样的：使用 `OnManualSubmit` 策略，界面包含"新增"、"删除"、"保存"、"撤销"、"刷新"五个按钮；顶部有一个 `QComboBox` 做科目筛选（全部、数学、英语、物理），一个 `QLineEdit` 做姓名模糊搜索；表头显示中文名称，ID 列隐藏；点击"保存"时做基础数据校验（成绩在 0-100 之间），校验不通过弹出提示并阻止提交。

几个实现提示：筛选功能通过 `setFilter()` 拼接科目和姓名条件实现，每次筛选条件变化时调用 `select()`；数据校验可以在 `submitAll()` 之前遍历 Model 的所有行，检查 `data()` 返回的成绩值是否合法；`QMessageBox::warning()` 用来弹出校验失败提示。

## 6. 官方文档参考

[Qt 文档 · QSqlTableModel](https://doc.qt.io/qt-6/qsqltablemodel.html) -- 数据库表映射 Model

[Qt 文档 · QTableView](https://doc.qt.io/qt-6/qtableview.html) -- 表格视图组件

[Qt 文档 · Model/View Programming](https://doc.qt.io/qt-6/model-view-programming.html) -- Qt Model/View 架构总览

[Qt 文档 · QSqlDatabase](https://doc.qt.io/qt-6/qsqldatabase.html) -- 数据库连接管理

*（链接已验证，2026-04-23 可访问）*

---

到这里就大功告成了！`QSqlTableModel` 把数据库和界面之间的那层胶水代码全部封装好了，对于"展示一张表并允许编辑"这种最常见的数据库 GUI 需求，它几乎是最高效的实现方式。掌握了 `EditStrategy` 的选择和 `setFilter` / `setSort` 的用法，日常开发中大部分数据管理界面都能快速搞定。下一篇我们换个方向，看看 Qt 的图表模块怎么把数据变成漂亮的可视化图形。

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
