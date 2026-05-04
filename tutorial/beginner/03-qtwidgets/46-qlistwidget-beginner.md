# 现代Qt开发教程（新手篇）3.46——QListWidget：便捷列表控件

## 1. 前言 / 最快上手列表的方案

Qt 的 Model/View 架构把数据和显示分离得很好，但不可否认，当你只是想快速搞一个列表出来——显示几个选项、让用户点选、显示一些图标——直接上手 QStringListModel 加 QListView 那套东西确实有点重。QListWidget 就是为此而生的便捷封装：它把 Model 和 View 合并成一个控件，内部帮你管理了一个 QListModel，你不需要自己创建 Model、不需要理解 index、不需要操心 data role，直接 addItem 往里塞条目就行。

说白了，QListWidget 适合的场景是数据量不大、不需要跨控件共享数据、不需要自定义 Model 逻辑的情况。一个文件选择列表、一个选项配置面板、一个简单的收藏夹列表——这些场景下 QListWidget 的开发效率远高于自己搭建 Model/View。代价是灵活性受限——你不能给多个 View 共享同一个 Model，也没法很方便地在 Model 层做排序过滤。但对于大量实际项目中的简单列表需求来说，这点代价完全可以接受。

今天的内容围绕四个方面展开。先看 QListWidget 添加条目的三种方式——addItem、addItems、insertItem，再看获取当前选中和多项选中的 currentItem 和 selectedItems 方法，然后研究 QListWidgetItem 的图标、复选框和自定义数据能力，最后把 itemDoubleClicked 和 itemChanged 这两个最常用的信号串起来做一个完整的交互示例。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QListWidget 和 QListWidgetItem 都在 QtWidgets 模块中，链接 Qt6::Widgets 即可。示例代码涉及 QListWidget、QListWidgetItem、QLabel、QPushButton、QVBoxLayout、QHBoxLayout 和 QInputDialog。

## 3. 核心概念讲解

### 3.1 addItem / addItems / insertItem 添加条目

QListWidget 提供了三种方式往列表中添加条目，它们的区别在于插入位置和批量能力。

addItem 是最基础的单条添加方法。它接受一个 QListWidgetItem 指针或者一个 QString 字符串。传 QString 时 QListWidget 内部会自动创建一个 QListWidgetItem 对象，设置好文本后追加到列表末尾。传 QListWidgetItem 指针时你可以提前配置好图标、复选框状态、自定义数据等属性，然后一次性塞进去。不管哪种形式，addItem 总是把新条目追加到列表的最底部。

```cpp
auto *listWidget = new QListWidget;

// 方式一：传 QString，最简形式
listWidget->addItem("苹果");
listWidget->addItem("香蕉");
listWidget->addItem("橘子");

// 方式二：传 QListWidgetItem，可以附带更多属性
auto *item = new QListWidgetItem("西瓜");
item->setIcon(QIcon::fromTheme("document-open"));
item->setCheckState(Qt::Checked);
listWidget->addItem(item);
```

addItems 是批量添加方法，接受一个 QStringList。它内部遍历列表，对每个字符串调用一次 addItem。当你有一组纯文本数据需要快速填入列表时，addItems 是最简洁的写法——一行代码搞定。

```cpp
const QStringList fruits = {"苹果", "香蕉", "橘子", "葡萄", "芒果", "荔枝"};
listWidget->addItems(fruits);
```

insertItem 和 addItem 的区别在于它可以指定插入位置。它接受一个行号（int row）和一个 QListWidgetItem 或 QString，把新条目插入到指定行号的位置，原来该行及之后的条目依次后移。行号从 0 开始，如果行号等于 count()（即当前条目总数），效果等同于 addItem——追加到末尾。如果行号大于 count()，不会报错，Qt 会自动把它截断到 count()，等效于追加。

```cpp
// 在第 0 行插入一个条目（置顶）
auto *topItem = new QListWidgetItem("置顶条目");
listWidget->insertItem(0, topItem);

// 在第 2 行插入
listWidget->insertItem(2, "插入到第三行");
```

有一个细节需要了解：当你用 addItem 或 insertItem 传入 QListWidgetItem 指针时，QListWidget 会接管这个条目的所有权。这意味着你不需要手动 delete 这个 item——QListWidget 析构时会自动销毁它管理的所有 QListWidgetItem。如果你在 QListWidget 存活期间手动 delete 了一个 item，你需要先调用 takeItem 把它从列表中取出来（takeItem 返回 item 的指针并从列表中移除，但不 delete），然后再 delete。直接 delete 一个还在列表中的 item 会导致 QListWidget 内部状态不一致。

takeItem(int row) 是和 insertItem 对应的移除方法。它从列表中取出指定行号的条目，返回 QListWidgetItem 指针，调用者负责后续的 delete。如果行号越界，返回 nullptr。

```cpp
// 取出并删除第 3 行
QListWidgetItem *taken = listWidget->takeItem(3);
delete taken;
```

count() 方法返回当前列表中的条目总数，它等价于 model()->rowCount()——因为 QListWidget 内部就是靠一个 model 在管理数据。

### 3.2 currentItem / selectedItems 获取选中

QListWidget 的选中行为取决于它的 selectionMode 属性。默认是 QAbstractItemView::SingleSelection——同一时间只能选中一个条目。还有 ExtendedSelection（支持 Ctrl/Shift 多选）、MultiSelection（点击即切换选中状态，不需要修饰键）和 NoSelection（禁止选中）。我们最常用的是 SingleSelection 和 ExtendedSelection。

currentItem() 返回当前条目——也就是列表中当前获得焦点的那一个条目（通常有一个虚线框包围）。在 SingleSelection 模式下，当前条目就是选中条目。但在 ExtendedSelection 模式下，当前条目和选中条目可以不同——你可以按住 Ctrl 点击多个条目全部选中，但"当前条目"只有最后一个被点击的那个。currentRow() 是 currentItem() 的便捷版本，直接返回行号（int），不需要再调 row()。

```cpp
// SingleSelection 模式下获取选中条目
QListWidgetItem *current = listWidget->currentItem();
if (current) {
    qDebug() << "当前选中:" << current->text();
}

// 也可以用 currentRow() 获取行号
int row = listWidget->currentRow();
if (row >= 0) {
    qDebug() << "当前行号:" << row;
}
```

selectedItems() 返回所有被选中的条目列表（QList<QListWidgetItem *>）。在 SingleSelection 模式下这个列表最多有一个元素，和 currentItem() 返回的是同一个条目。在 ExtendedSelection 或 MultiSelection 模式下，这个列表可能包含多个条目。selectedItems() 是处理多选场景的核心方法。

```cpp
// 设置为多选模式
listWidget->setSelectionMode(
    QAbstractItemView::ExtendedSelection);

// 获取所有选中条目
const auto selected = listWidget->selectedItems();
for (const auto *item : selected) {
    qDebug() << "选中:" << item->text()
             << "行号:" << item->row();
}
```

有一个容易忽略的问题：当列表为空时，currentItem() 返回 nullptr，currentRow() 返回 -1。所以在使用返回值之前一定要做判空检查，否则对 nullptr 调用 text() 直接崩溃。

还有一个相关的信号是 currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)，它在当前条目变化时发射，参数分别是新的当前条目和之前的当前条目。这个信号在实现"选中条目变化时更新右侧详情面板"这种联动逻辑时非常有用。两个参数都可能为 nullptr——current 为 nullptr 表示没有条目被选中（比如列表被清空），previous 为 nullptr 表示之前没有选中任何条目。

### 3.3 QListWidgetItem 图标 / 复选框 / 自定义数据

QListWidgetItem 不只是一个文本容器——它支持图标、复选框和任意自定义数据，这让 QListWidget 的表现力远超纯文本列表。

setIcon(QIcon) 给条目设置一个图标，图标显示在文本左侧。图标来源可以是资源文件中的图标、系统主题图标（QIcon::fromTheme）、或者程序生成的 QPixmap。图标大小可以通过 QListWidget 的 setIconSize(QSize) 统一控制，默认是 16x16 像素。如果你设了 setIconSize(QSize(32, 32))，所有条目的图标都会被缩放到 32x32。

```cpp
auto *item = new QListWidgetItem("文档");
item->setIcon(QIcon::fromTheme("text-x-generic"));
listWidget->addItem(item);

// 统一设置图标大小
listWidget->setIconSize(QSize(24, 24));
```

setCheckState(Qt::CheckState) 给条目添加一个复选框。三个可选值是 Qt::Unchecked（未选中）、Qt::PartiallyChecked（半选中）和 Qt::Checked（选中）。复选框显示在图标左侧——如果同时有图标和复选框，从左到右的排列是：复选框、图标、文本。默认情况下条目没有复选框（setFlags 中不包含 Qt::ItemIsUserCheckable），只有当你调用了 setCheckState 之后复选框才会出现。

```cpp
auto *taskItem = new QListWidgetItem("完成教程写作");
taskItem->setCheckState(Qt::Unchecked);
listWidget->addItem(taskItem);

// 另一个已完成的任务
auto *doneItem = new QListWidgetItem("安装 Qt 环境");
doneItem->setCheckState(Qt::Checked);
listWidget->addItem(doneItem);
```

setData(int role, QVariant) 是 QListWidgetItem 最灵活的能力——它允许你往条目中存储任意类型的自定义数据。Qt 定义了一些标准的 Item Data Role，比如 Qt::DisplayRole（显示文本）、Qt::DecorationRole（图标）、Qt::CheckStateRole（复选框状态）等。但你可以用 Qt::UserRole（值为 256）及以上的值来存储自己的数据。

```cpp
auto *item = new QListWidgetItem("Alice");

// 存储自定义数据：用户 ID 和邮箱
item->setData(Qt::UserRole, 1001);                  // 用户 ID
item->setData(Qt::UserRole + 1, "alice@example.com"); // 邮箱
item->setData(Qt::UserRole + 2, 28);                 // 年龄

listWidget->addItem(item);

// 读取自定义数据
int userId = item->data(Qt::UserRole).toInt();
QString email = item->data(Qt::UserRole + 1).toString();
int age = item->data(Qt::UserRole + 2).toInt();
```

data() 返回 QVariant，你需要用 toInt()、toString()、toBool() 等方法提取具体类型的值。QVariant 支持所有 Qt 常用类型，甚至可以存储自定义的 QVariant-registered 类型（通过 Q_DECLARE_METATYPE + qRegisterMetaType）。自定义数据不会显示在界面上——它们纯粹是给程序逻辑用的。最常见的用法是在条目中存储数据库记录的 ID，用户点击条目时取出 ID 去查库。

还有一个有用的方法是 setFlags(Qt::ItemFlags)，它控制条目的交互行为。默认 flags 包含 Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled。如果你想让某个条目不可选中但仍然可见，去掉 ItemIsSelectable；如果你想让某个条目变灰显示（禁用状态），去掉 ItemIsEnabled；如果你想启用编辑，加上 ItemIsEditable——双击条目就能原地编辑文本。

```cpp
// 设置条目为不可选中但可见（类似分组标题）
auto *header = new QListWidgetItem("-- 水果 --");
header->setFlags(header->flags() & ~Qt::ItemIsSelectable);

// 设置条目可编辑
auto *editable = new QListWidgetitem("双击编辑我");
editable->setFlags(editable->flags() | Qt::ItemIsEditable);
```

### 3.4 itemDoubleClicked / itemChanged 信号

QListWidget 有大量信号，但日常开发中最常用的两个是 itemDoubleClicked 和 itemChanged。

itemDoubleClicked(QListWidgetItem *item) 在用户双击一个条目时发射。参数是被双击的条目指针。这个信号的典型应用场景是"双击打开"——双击文件列表中的文件来打开，双击联系人列表中的联系人来查看详情，双击歌曲列表中的歌曲来播放。它和 currentItemChanged 不一样——currentItemChanged 只要选中变了就触发，而 itemDoubleClicked 专门响应双击操作。

```cpp
connect(listWidget, &QListWidget::itemDoubleClicked,
        [](QListWidgetItem *item) {
    if (item) {
        qDebug() << "双击了:" << item->text();
        // 实际应用中这里执行"打开"操作
    }
});
```

itemChanged(QListWidgetItem *item) 在条目的数据发生变化时发射。触发它的操作包括：程序调用 item->setText()、item->setIcon()、item->setCheckState() 等方法改变了条目数据；用户双击编辑了条目文本（前提是 flags 包含 ItemIsEditable）；用户点击了条目的复选框切换了勾选状态。这个信号的参数是发生变化的条目指针。

这里有一个很常见的坑：如果你在 itemChanged 的槽函数中又修改了同一个 item 的属性（比如根据复选框状态改变条目的文本颜色），就会再次触发 itemChanged，导致无限递归。解决方案是在槽函数中临时 blockSignals。

```cpp
connect(listWidget, &QListWidget::itemChanged,
        [listWidget](QListWidgetItem *item) {
    if (!item) return;

    // 临时阻塞信号，避免递归
    listWidget->blockSignals(true);

    if (item->checkState() == Qt::Checked) {
        // 已完成的任务用灰色显示并加删除线
        QFont font = item->font();
        font.setStrikeOut(true);
        item->setFont(font);
        item->setForeground(QColor("#999"));
    } else {
        QFont font = item->font();
        font.setStrikeOut(false);
        item->setFont(font);
        item->setForeground(QColor("#333"));
    }

    listWidget->blockSignals(false);
});
```

blockSignals(true) 会阻止对象发射任何信号，blockSignals(false) 恢复。在这个场景下，我们在修改条目属性之前先 blockSignals(true)，修改完之后再 blockSignals(false)，这样修改操作就不会再次触发 itemChanged 信号了。

除了这两个信号，QListWidget 还有 itemClicked（单击）、itemPressed（鼠标按下）、itemEntered（鼠标进入条目区域，需要开启 MouseTracking）等信号，但使用频率远低于 itemDoubleClicked 和 itemChanged。currentRowChanged(int currentRow) 也很常用——它只传行号，比 currentItemChanged 更轻量，适合只需要知道行号的场景。

## 4. 踩坑预防

第一个坑是 addItem 接管了 QListWidgetItem 的所有权后不要手动 delete。如果你确实需要在运行时移除某个条目，用 takeItem(row) 先把它从列表中取出，拿到指针后再 delete。直接 delete 一个还在列表中的 item 会导致 QListWidget 内部的 model 持有悬空指针，后续访问可能崩溃。

第二个坑是 itemChanged 信号的递归触发。如上面提到的，在 itemChanged 槽函数中修改同一个 item 的属性会再次触发 itemChanged。这个坑非常隐蔽，因为程序不会崩溃也不会报错——只是无限递归调用直到栈溢出。养成习惯：在 itemChanged 槽函数中如果需要修改 item 属性，一律用 blockSignals 保护。

第三个坑是 selectedItems() 和 currentItem() 在 NoSelection 模式下永远返回空。如果你发现怎么点都拿不到选中条目，检查一下 selectionMode 是否被意外设成了 NoSelection。

第四个坑是 setIconSize 只影响显示大小不影响源图像。如果你给一个条目设了一个 256x256 的大图标，然后把 QListWidget 的 iconSize 设为 16x16，QListWidget 会把图标缩放到 16x16 来显示。但源图像的 256x256 像素数据仍然保存在内存中。如果你的列表有几百个条目，每个条目都有一个很大的图标，内存占用会非常可观。建议在添加图标之前就把图片缩放到合适的尺寸。

第五个坑是 QListWidget 的 sortItems() 方法在排序后会改变所有条目的行号。如果你在代码中通过行号来引用条目（比如存了一个 int 变量记住"当前选中的是第 3 行"），排序后这个行号就失效了。更好的做法是通过 item 的指针或者 data(Qt::UserRole) 中的 ID 来标识条目，而不是依赖行号。

## 5. 练习项目

我们来做一个综合练习：创建一个"待办任务管理器"窗口。中央是一个 QListWidget，显示任务列表。每个任务条目有复选框（表示完成状态）、任务名称和一个自定义数据（存储优先级 1-3）。窗口上方有一行输入框和"添加任务"按钮，点击后把输入框中的文字作为新任务追加到列表末尾，默认未选中、优先级为普通。右键点击条目弹出上下文菜单，包含"设为高优先级"（条目文字变红）、"设为普通优先级"（恢复正常色）、"删除"三个操作。双击条目弹出一个 QInputDialog 让用户修改任务名称。勾选/取消勾选复选框时，已完成的任务显示灰色文字加删除线，未完成的恢复正常样式（用 itemChanged 信号 + blockSignals 实现）。窗口底部有一个 QLabel 实时显示当前任务总数和已完成数量（用 itemChanged 信号驱动更新）。

## 6. 官方文档参考链接

[Qt 文档 -- QListWidget](https://doc.qt.io/qt-6/qlistwidget.html) -- 便捷列表控件

[Qt 文档 -- QListWidgetItem](https://doc.qt.io/qt-6/qlistwidgetitem.html) -- 列表条目

[Qt 文档 -- QAbstractItemView::SelectionMode](https://doc.qt.io/qt-6/qabstractitemview.html#SelectionMode-enum) -- 选中模式枚举

[Qt 文档 -- Qt::ItemDataRole](https://doc.qt.io/qt-6/qt.html#ItemDataRole-enum) -- 数据角色枚举（UserRole 在此）

---

到这里，QListWidget 的核心用法就全部讲完了。addItem 和 addItems 让你快速填充列表数据，currentItem 和 selectedItems 让你精确获取用户选中了什么，QListWidgetItem 的图标、复选框和 setData(Qt::UserRole, ...) 让每个条目都能承载远超纯文本的信息量，itemDoubleClicked 和 itemChanged 信号则覆盖了最常见的交互需求。当你的列表需求不涉及复杂的自定义 Model 逻辑时，QListWidget 就是效率最高的选择。
