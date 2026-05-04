# 现代Qt开发教程（新手篇）3.50——QTableWidget：便捷表格控件

## 1. 前言 / 二维数据的便捷容器

前面我们聊了列表控件（QListWidget/QListView）和树形控件（QTreeWidget/QTreeView），它们分别处理一维线性数据和层级树形数据。现实中还有一大类数据是二维的表格——成绩单、商品清单、配置参数表、CSV 数据导入预览——这些数据的行和列都有明确含义，需要用表格的形式展示和编辑。QTableWidget 就是 Qt 为这类二维数据提供的便捷控件，和 QListWidget、QTreeWidget 一样走的是"Model 和 View 合二为一"的路线：内部帮你管理了一个表格 Model，你不需要自己搭建 Model/View 架构，直接通过行号和列号操作单元格就行。

QTableWidget 适合的场景和它的"便捷系兄弟"完全一致——数据量不大、不需要跨控件共享数据、不需要自定义 Model 逻辑。一个学生成绩管理界面、一个系统参数配置表、一个 CSV 文件预览面板——这些场景下 QTableWidget 的开发效率远高于自己用 QTableView 配 QAbstractTableModel。代价同样也很熟悉：你不能在 Model 层做排序过滤，也不能给多个 View 共享同一份数据。但对于大量实际项目中的简单表格需求，这点代价完全不是问题。

今天的内容围绕四个方面展开。先看 setRowCount 和 setColumnCount 如何定义表格的尺寸，然后通过 setItem 和 item 来读写单元格的内容，接着研究 setHorizontalHeaderLabels 和 setVerticalHeaderLabels 来自定义表头，最后把 cellChanged、cellClicked 和 currentCellChanged 这三个核心信号串起来做一个完整的交互示例。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QTableWidget 和 QTableWidgetItem 都在 QtWidgets 模块中，链接 Qt6::Widgets 即可。示例代码涉及 QTableWidget、QTableWidgetItem、QLabel、QPushButton、QLineEdit、QVBoxLayout、QHBoxLayout、QHeaderView 和 QInputDialog。

## 3. 核心概念讲解

### 3.1 setRowCount / setColumnCount 设置行列数

QTableWidget 的表格尺寸通过行数和列数来定义。setRowCount(int rows) 设置行数，setColumnCount(int columns) 设置列数。这两个方法可以在任何时候调用——创建 QTableWidget 后设置初始尺寸，或者在运行时动态调整表格大小。如果新设置的行数比当前少，多余的行会被删除（连同行中的 QTableWidgetItem 一起销毁）；如果新设置的行数比当前多，新增的行是空的。

```cpp
auto *tableWidget = new QTableWidget;

// 设置一个 5 行 3 列的表格
tableWidget->setRowCount(5);
tableWidget->setColumnCount(3);
```

QTableWidget 的行号从 0 开始，列号也从 0 开始。行号显示在表格左侧的垂直表头中（默认显示 1、2、3...），列号显示在表格顶部的水平表头中。如果你不想显示行号，可以调用 verticalHeader()->hide() 把垂直表头隐藏掉——这在行号没有实际含义的场景下很常见。

setRowCount 和 setColumnCount 有一个经常被忽略的细节：调用它们会清除表格中所有现有的单元格内容。如果你想把一个 10 行 3 列的表格重置为 5 行 3 列，setRowCount(5) 会保留前 5 行，删除后 5 行——这部分没问题。但如果你想"重新填充数据"，正确的做法是先 setRowCount(0) 清空所有行，再 setRowCount(newSize) 设置新行数，然后逐个填充。或者直接调用 clear() 清空所有内容后重新设置尺寸。

```cpp
// 清空并重新设置表格尺寸
tableWidget->clear();
tableWidget->setRowCount(10);
tableWidget->setColumnCount(4);
```

clear() 方法清空表格的所有内容——包括所有 QTableWidgetItem、表头标签和选中状态。clearContents() 只清空单元格内容但保留表头标签。如果你自定义了表头标签并且想保留它们，用 clearContents() 而不是 clear()。

rowCount() 和 columnCount() 分别返回当前的行数和列数。在遍历表格时这两个方法是基础。

### 3.2 setItem / item 单元格读写

QTableWidget 的每个单元格由一个 QTableWidgetItem 对象来承载数据。setItem(int row, int column, QTableWidgetItem *item) 把一个 item 放到指定行号和列号的位置上。如果那个位置已经有一个 item，旧的 item 会被替换并自动 delete。item(int row, int column) 返回指定位置的 item 指针，如果该位置没有设置过 item 则返回 nullptr。

```cpp
// 创建 item 并放入单元格
auto *nameItem = new QTableWidgetItem("Alice");
tableWidget->setItem(0, 0, nameItem);

auto *ageItem = new QTableWidgetItem("28");
tableWidget->setItem(0, 1, ageItem);

auto *cityItem = new QTableWidgetItem("北京");
tableWidget->setItem(0, 2, cityItem);
```

QTableWidgetItem 和 QListWidgetItem、QTreeWidgetItem 一样支持丰富的属性。setText(const QString &) 设置文本，setIcon(const QIcon &) 设置图标，setCheckState(Qt::CheckState) 设置复选框，setData(int role, QVariant) 存储自定义数据，setFlags(Qt::ItemFlags) 控制交互行为。和 QListWidgetItem 一样，setText 是最常见的操作，setIcon 和 setData 根据需要使用。

```cpp
auto *item = new QTableWidgetItem("Bob");
item->setIcon(QIcon::fromTheme("user-available"));
item->setData(Qt::UserRole, 1002);  // 存储 ID
item->setFlags(item->flags() | Qt::ItemIsEditable);  // 可编辑
tableWidget->setItem(1, 0, item);
```

读取单元格数据同样简单——先通过 item(row, col) 拿到 QTableWidgetItem 指针，然后调用 text()、data(role) 等方法获取内容。注意 item(row, col) 可能返回 nullptr，使用前必须判空。

```cpp
QTableWidgetItem *cell = tableWidget->item(0, 0);
if (cell) {
    QString name = cell->text();
    int userId = cell->data(Qt::UserRole).toInt();
    qDebug() << "姓名:" << name << "ID:" << userId;
}
```

QTableWidget 也提供了直接操作文本的便捷方法。setItem(row, col, new QTableWidgetItem("text")) 是设置文本的完整写法。如果你只是想快速设置某个单元格的文本内容而不需要配置其他属性，可以用 QTableWidgetItem 的构造函数直接传文本。反过来，如果你只是想读取文本，用 item(row, col)->text() 就够了。没有"setCellText"这种一步到位的方法——你必须先创建 QTableWidgetItem，然后 setItem。这是 QTableWidget 设计上的一个特点，初学者可能会觉得不够便捷，但好处是每个单元格都是独立的 item 对象，可以承载比纯文本丰富得多的信息。

QTableWidgetItem 的 setData(Qt::UserRole, value) 和 QListWidgetItem 的用法完全一致——用 Qt::UserRole 及以上的值存储自定义数据，通过 data(Qt::UserRole).toInt() 等方法读取。最常见的用法是在单元格中存储数据库记录的 ID，用户点击或编辑单元格时取出 ID 去做后续操作。

```cpp
// 写入自定义数据
auto *item = new QTableWidgetItem("高级工程师");
item->setData(Qt::UserRole, 3);  // 职级编号
tableWidget->setItem(0, 2, item);

// 读取自定义数据
QTableWidgetItem *cell = tableWidget->item(0, 2);
if (cell) {
    int level = cell->data(Qt::UserRole).toInt();
}
```

还有一个值得了解的方法是 takeItem(int row, int column)——它从表格中取出指定位置的 item 并返回指针，但不会 delete 它。这和 QListWidget 的 takeItem、QTreeWidgetItem 的 takeChild 是同一个模式。取出后你可以修改 item 的内容然后放回原位，或者放到另一个位置，或者手动 delete。

### 3.3 setHorizontalHeaderLabels / setVerticalHeaderLabels 表头

QTableWidget 默认的水平表头显示列号（1、2、3...），垂直表头显示行号（1、2、3...）。这对于实际应用来说通常不够友好——用户更希望看到"姓名"、"年龄"、"城市"这样的列标题，而不是"A列"、"B列"。

setHorizontalHeaderLabels(const QStringList &labels) 设置水平表头（列标题）。传入的 QStringList 中每个元素对应一列的标题。如果列表长度小于列数，多余的列保持默认标题；如果大于列数，多余的标题被忽略。

```cpp
tableWidget->setHorizontalHeaderLabels(
    {"姓名", "年龄", "城市", "职业"});
```

setVerticalHeaderLabels(const QStringList &labels) 设置垂直表头（行标题）。用法和 setHorizontalHeaderLabels 完全一致，每个元素对应一行的标题。垂直表头在实际开发中使用频率远低于水平表头——大部分表格只需要有意义的列标题，行号用默认的 1、2、3... 就够了。

```cpp
// 给前几行设置有意义的行标题
tableWidget->setVerticalHeaderLabels(
    {"第一行", "第二行", "第三行"});
```

QTableWidget 的水平表头和垂直表头都是 QHeaderView 对象，你可以通过 horizontalHeader() 和 verticalHeader() 获取它们的指针。QHeaderView 提供了丰富的列宽控制方法——setSectionResizeMode 控制列宽策略，setStretchLastSection(bool) 控制最后一列是否占满剩余空间。

QHeaderView::Interactive 是默认模式——用户可以通过拖拽表头分隔线来手动调整列宽，列宽不会自动变化。QHeaderView::Stretch 让所有列等比例拉伸占满整个表格宽度，用户不能手动调整。QHeaderView::ResizeToContents 根据内容自动调整列宽，每列的宽度刚好容纳最长的内容。QHeaderView::Fixed 需要配合 setSectionWidth(int logicalIndex, int size) 使用——列宽固定为指定值，用户不能拖拽。

```cpp
// 第一列根据内容自适应，最后一列自动拉伸
tableWidget->horizontalHeader()->setSectionResizeMode(
    0, QHeaderView::ResizeToContents);
tableWidget->horizontalHeader()->setStretchLastSection(true);
```

如果你不想显示某个方向的表头，调用 horizontalHeader()->hide() 或者 verticalHeader()->hide()。隐藏垂直表头在大部分场景下都是合理的——行号通常没有实际含义，隐藏后表格看起来更清爽。

你还可以单独设置某个表头单元格的属性——horizontalHeaderItem(int column) 返回指定列的水平表头 QTableWidgetItem 指针，你可以修改它的文本、字体、对齐方式等。setHorizontalHeaderItem(int column, QTableWidgetItem *item) 替换整个表头单元格。这在需要让某一列的表头显示图标或者特殊样式时很有用。

```cpp
// 给第一列的表头加一个图标
auto *headerItem = new QTableWidgetItem("姓名");
headerItem->setIcon(QIcon::fromTheme("user-available"));
tableWidget->setHorizontalHeaderItem(0, headerItem);
```

### 3.4 cellChanged / cellClicked / currentCellChanged 信号

QTableWidget 提供了大量的信号来响应表格的各类交互事件。其中在日常开发中使用频率最高的三个是 cellChanged、cellClicked 和 currentCellChanged。

cellChanged(int row, int column) 在单元格数据发生变化时发射。触发它的操作包括程序调用 item->setText() 等方法修改了单元格数据、用户双击编辑了单元格文本（前提是 flags 包含 ItemIsEditable）。参数是发生变化的单元格的行号和列号。这个信号和 QListWidget 的 itemChanged、QTreeWidget 的 itemChanged 是同一个性质——你在槽函数中需要注意 blockSignals 防止递归。

```cpp
connect(tableWidget, &QTableWidget::cellChanged,
        [tableWidget](int row, int col) {
    QTableWidgetItem *item = tableWidget->item(row, col);
    if (item) {
        qDebug() << "单元格变化: (" << row << "," << col
                 << ") ->" << item->text();
    }
});
```

cellClicked(int row, int column) 在用户单击一个单元格时发射。参数是被点击的单元格的行号和列号。这个信号是最常用的——点击表格中的某一行来选中并执行某个操作（比如点击学生列表中的某一行来显示该学生的详细信息）。它和 cellPressed 不同——cellPressed 在鼠标按下时就发射，cellClicked 在鼠标释放时发射（完整的一次点击操作）。

```cpp
connect(tableWidget, &QTableWidget::cellClicked,
        [tableWidget](int row, int col) {
    QTableWidgetItem *item = tableWidget->item(row, col);
    if (item) {
        qDebug() << "点击: (" << row << "," << col
                 << ") " << item->text();
    }
});
```

currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn) 在当前单元格发生变化时发射。参数是新的当前行号、当前列号，以及之前的行号和列号。这个信号和 cellClicked 的区别在于：cellClicked 只响应鼠标点击，而 currentCellChanged 在任何导致当前单元格变化的操作后都会触发——包括鼠标点击、键盘导航（方向键、Tab 键、PageUp/PageDown）和程序调用 setCurrentCell。如果你需要"无论用户怎么操作，当前高亮单元格变了就要响应"的逻辑，用 currentCellChanged。

```cpp
connect(tableWidget, &QTableWidget::currentCellChanged,
        [](int curRow, int curCol,
           int prevRow, int prevCol) {
    qDebug() << "当前单元格: (" << curRow << "," << curCol
             << ") 之前: (" << prevRow << "," << prevCol << ")";
});
```

除了这三个核心信号，QTableWidget 还有 cellDoubleClicked(int row, int column)——双击单元格时发射；cellActivated(int row, int column)——在单元格上按回车或双击时发射（取决于激活策略）；cellEntered(int row, int column)——鼠标移入单元格时发射（需要开启 MouseTracking）。currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous) 也在当前 item 变化时发射，它传的是 item 指针而不是行列号。

## 4. 踩坑预防

第一个坑是 item(row, col) 返回 nullptr 时直接调用 text() 会崩溃。这种情况在单元格从未被 setItem 设置过时会发生——QTableWidget 的单元格默认是"空"的，不包含任何 QTableWidgetItem。即使你 setRowCount(5) 了，5 行 20 列一共 100 个单元格默认都是 nullptr。你需要对每个要操作的单元格先 setItem 创建 item，之后才能用 item(row, col) 读取。如果你想让表格的所有单元格在创建时都有默认的空 item，可以在 setRowCount/setColumnCount 后遍历所有单元格并 setItem(row, col, new QTableWidgetItem(""))。

第二个坑是 cellChanged 信号在初始化填充数据时也会被触发。和 QTreeWidget 一样，如果你在 cellChanged 槽函数中有保存到数据库等重量级操作，批量填充数据时应该先 blockSignals(true)，填充完再 blockSignals(false)。

第三个坑是 setItem 会接管 QTableWidgetItem 的所有权，不要对一个已经 setItem 过的 item 再次 setItem 到另一个位置。如果你想移动一个 item，先用 takeItem 取出来，再 setItem 到新位置。直接把同一个 item 指针 setItem 到两个位置会导致内存错误。

第四个坑是 setHorizontalHeaderLabels 传入的列表长度和 columnCount 不匹配时不会报错。多余的标题被忽略，不够的列保持默认。养成习惯：setHorizontalHeaderLabels 的列表长度始终和 columnCount 保持一致。

第五个坑是 QTableWidget 的 clear() 会同时清除表头标签。如果你自定义了表头标签并且想保留它们，用 clearContents() 只清空单元格内容。如果用 clear() 后又发现表头没了，你需要重新调用 setHorizontalHeaderLabels。

## 5. 练习项目

我们来做一个综合练习：创建一个"学生信息管理表"窗口。中央是一个 6 列 QTableWidget（学号、姓名、性别、年龄、专业、成绩），预先填充 5-8 条学生数据。水平表头使用 setHorizontalHeaderLabels 设置有意义的列标题，垂直表头隐藏。表格支持行选择模式（setSelectionBehavior(QAbstractItemView::SelectRows)），点击某一行时在右侧面板用 QLabel 显示该行的完整信息。窗口上方有"添加学生"按钮和"删除选中行"按钮——添加时弹出一个 QInputDialog 逐项输入学生信息（或者用多个 QLineEdit 组成的输入面板），删除时删除当前选中的整行。双击单元格可以原地编辑（默认行为，flags 包含 ItemIsEditable）。cellChanged 信号用于在状态栏实时显示"已修改: (行, 列)"。currentCellChanged 信号用于在右侧面板同步更新当前行的详情。底部有一个 QLabel 显示"总计 X 名学生，平均成绩 Y.Y 分"。

提示：遍历所有行取成绩列的值来计算平均分。添加行时先 setRowCount(rowCount + 1)，然后为新行的每列创建 QTableWidgetItem 并 setItem。删除行时用 removeRow(int row) 一次性删除整行。

## 6. 官方文档参考链接

[Qt 文档 -- QTableWidget](https://doc.qt.io/qt-6/qtablewidget.html) -- 便捷表格控件

[Qt 文档 -- QTableWidgetItem](https://doc.qt.io/qt-6/qtablewidgetitem.html) -- 表格单元格条目

[Qt 文档 -- QHeaderView](https://doc.qt.io/qt-6/qheaderview.html) -- 表头控件

[Qt 文档 -- QAbstractItemView::SelectionBehavior](https://doc.qt.io/qt-6/qabstractitemview.html#SelectionBehavior-enum) -- 选中行为枚举

---

到这里，QTableWidget 的核心用法就全部讲完了。setRowCount 和 setColumnCount 定义了表格的基本骨架，setItem 和 item 让你通过行列号精确操作每个单元格的数据，setHorizontalHeaderLabels 和 setVerticalHeaderLabels 给表格加上有意义的表头，cellChanged、cellClicked 和 currentCellChanged 三个信号覆盖了表格控件最常见的交互需求。当你的数据是二维表格且不需要复杂的自定义 Model 时，QTableWidget 就是效率最高的选择。
