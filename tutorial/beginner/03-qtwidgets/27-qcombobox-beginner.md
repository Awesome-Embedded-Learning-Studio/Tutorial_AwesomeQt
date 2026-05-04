# 现代Qt开发教程（新手篇）3.27——QComboBox：下拉选择框

## 1. 前言 / 下拉选择这种交互，几乎所有应用都在用

我们做桌面应用开发的时候，"让用户从一组预设选项里挑一个"这种需求几乎是避不开的——选择所在城市、选择字体大小、选择导出格式、选择网络协议。如果你用一组 QRadioButton 来做，当选项数量超过五六个的时候界面就会变得非常拥挤；如果你用 QListWidget 做一个弹出列表来选，又会占用过多的屏幕空间。QComboBox 的设计初衷就是解决这个矛盾：收起来的时候只占一行的高度，展开后显示一个下拉列表供用户点选。这种"折叠/展开"的交互方式是桌面 UI 中最高频出现的模式之一，你每天使用的软件里到处都是它的身影。

QComboBox 看起来简单——不就是一个下拉菜单嘛，有什么好讲的？但实际上它的能力远比你想象的要丰富。它支持普通的文本选项，也支持带图标、带自定义数据的选项；它支持只读模式，也支持让用户自己输入文本的可编辑模式；它的底层数据存储用的是 Model/Viewer 架构，你可以给它挂一个自定义的 QAbstractItemModel 来实现任意复杂的数据展示。我们今天把 QComboBox 的四个核心维度全部讲清楚：addItem / addItems / insertItem 添加选项的各种方式，currentIndex / currentText / currentData 三种获取当前选中值的方式，setEditable(true) 可编辑组合框的用法和注意事项，以及 setModel() 挂载自定义 Model 的高级玩法。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QComboBox 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。QComboBox 的 Model/Viewer 架构依赖 QtGui 模块中的 QStandardItemModel 和 QStandardItem——但因为 Widgets 依赖 Gui，所以不需要额外链接。如果你需要给选项加图标，还需要用到 QIcon（同样在 QtGui 中）。

## 3. 核心概念讲解

### 3.1 addItem / addItems / insertItem 添加选项

我们从最基础的操作开始——往 QComboBox 里塞选项。QComboBox 提供了几种不同的添加方式，每一种都有它适用的场景。

最直接的方式是 `addItem()`。它有几个重载版本，最常用的是传入一个 QString 作为显示文本：

```cpp
auto *combo = new QComboBox();
combo->addItem("北京");
combo->addItem("上海");
combo->addItem("广州");
combo->addItem("深圳");
```

每调用一次 `addItem()` 就在下拉列表的末尾追加一个选项。如果你有一组预设数据，逐个 addItem 当然可以，但 Qt 提供了更方便的 `addItems()`——一次调用传入一个 QStringList，批量添加：

```cpp
auto *combo = new QComboBox();
combo->addItems(QStringList{"北京", "上海", "广州", "深圳"});
```

`addItems()` 在内部就是对列表中的每个元素依次调用 `addItem()`，所以它的效率和手写循环是一样的，但代码简洁得多。

接下来是 `insertItem()`。和 `addItem()` / `addItems()` 只能往末尾追加不同，`insertItem()` 允许你把选项插入到指定位置。它的第一个参数是插入位置的索引——如果索引是 0，选项会出现在列表最前面；如果索引等于 count()，效果和 addItem() 一样是追加到末尾。如果索引超过了当前选项数量，Qt 会自动调整到末尾位置，不会越界崩溃：

```cpp
auto *combo = new QComboBox();
combo->addItems(QStringList{"上海", "广州", "深圳"});
// 把"北京"插入到最前面，现在顺序是：北京、上海、广州、深圳
combo->insertItem(0, "北京");
```

在实际项目中你会发现，很多时候我们不仅要显示文本，还需要给每个选项关联一个"内部值"。比如下拉框显示城市名称，但我们的业务逻辑需要用到城市编码。QComboBox 的 `addItem()` 有一个重载版本支持同时传入显示文本和用户数据（userData），这个用户数据是 QVariant 类型，可以存任何 Qt 支持的类型：

```cpp
combo->addItem("北京", "BJ");   // 显示"北京"，关联数据 "BJ"
combo->addItem("上海", "SH");   // 显示"上海"，关联数据 "SH"
combo->addItem("广州", "GZ");   // 显示"广州"，关联数据 "GZ"
```

这里第二个参数的类型是 QVariant，所以你传 QString、int、甚至自定义的 QVariant 包装值都可以。如果你需要给选项加一个图标，也有对应的重载：`addItem(const QIcon &icon, const QString &text, const QVariant &userData)`。

还有一个容易忽略的细节：当你往 QComboBox 添加第一个选项时，它的 currentIndex 会自动从 -1（无选中）变成 0（选中第一个），并且会触发 `currentIndexChanged` 信号。如果你在初始化阶段不想响应这个变化，要么先 blockSignals，要么在连接信号之前把选项添加完。

### 3.2 currentIndex() / currentText() / currentData() 获取当前值

当用户在下拉框中选择了一个选项（或者你通过代码设置了当前选项）后，我们有三种方式获取当前的状态。

`currentIndex()` 返回当前选中项的索引，类型是 int。如果没有选中任何项（比如一个空的 QComboBox），返回 -1。这个索引用于程序内部定位选项位置，比如你需要根据索引去其他数据结构里查对应的记录：

```cpp
int index = combo->currentIndex();
if (index >= 0) {
    qDebug() << "当前选中索引:" << index;
} else {
    qDebug() << "没有选中任何项";
}
```

`currentText()` 返回当前选中项的显示文本。在非可编辑模式下，它就是你用 addItem 时传入的那个字符串。在可编辑模式下，它返回的是用户实际输入的文本（可能不在下拉列表中）。这是一个很常用的方法，因为很多时候你只需要知道用户选了什么文字：

```cpp
QString city = combo->currentText();
qDebug() << "用户选择了:" << city;
```

`currentData()` 返回当前选中项关联的用户数据。如果你在 addItem 时传入了第三个参数，currentData() 就能把它取出来。如果没有关联数据，返回一个无效的 QVariant。这个方法有一个带参数的重载 `currentData(int role = Qt::UserRole)`，默认取的是 Qt::UserRole 角色的数据：

```cpp
// 假设之前 addItem("北京", "BJ")
QString code = combo->currentData().toString();  // 返回 "BJ"
```

这三者的关系是这样的：QComboBox 内部维护一个索引（currentIndex），通过索引可以找到对应的显示文本（currentText）和关联数据（currentData）。所以 currentIndex 是最基础的状态，currentText 和 currentData 都是从索引派生出来的。

当你需要通过代码设置当前选项时，用 `setCurrentIndex(int)` 或 `setCurrentText(const QString &)`。setCurrentIndex 直接设置索引值；setCurrentText 会在下拉列表中查找匹配的文本并设置为当前项——如果找到了匹配项，它的行为和 setCurrentIndex 一样；如果没找到匹配项，在非可编辑模式下不做任何事，在可编辑模式下会把用户文本设为输入值但不改变索引。这里有个容易踩坑的地方：setCurrentText 做的是精确匹配（大小写敏感），如果你的选项文本中有空格或者大小写不一致，可能匹配不上。

### 3.3 setEditable(true) 可编辑组合框

QComboBox 默认是不可编辑的——用户只能从下拉列表中选择已有的选项，不能自己输入内容。但有些场景下你需要允许用户自由输入，比如一个搜索框下面的"最近搜索"下拉列表，用户既可以从历史记录中选，也可以输入新的搜索词。这时候就需要 `setEditable(true)`：

```cpp
auto *combo = new QComboBox();
combo->addItems(QStringList{"Chrome", "Firefox", "Safari", "Edge"});
combo->setEditable(true);
```

设置为可编辑后，QComboBox 的外观会发生变化：它的显示区域变成了一个带光标的文本输入框，用户可以直接在里面打字。下拉列表仍然可用，用户点击下拉箭头展开列表选择一个已有项后，文本框会填入选中的内容。

可编辑模式下有几个值得注意的行为变化。首先，`currentText()` 不再保证返回的是下拉列表中的某一项——它返回的是用户实际输入的文本，可能是列表中已有的值，也可能是用户自己打的字。所以如果你需要"确保用户选了一个有效值"，就要自己做校验：

```cpp
connect(combo, &QComboBox::currentTextChanged,
        this, [combo]() {
    QString input = combo->currentText();
    int index = combo->findText(input);
    if (index < 0) {
        qDebug() << "用户输入了列表中不存在的值:" << input;
    }
});
```

其次，可编辑模式下 QComboBox 内部使用了一个 QLineEdit 来实现文本输入功能。你可以通过 `lineEdit()` 方法获取这个内部 QLineEdit 的指针，然后对它进行进一步的自定义——比如设置输入掩码、设置验证器、设置占位文本等：

```cpp
combo->setEditable(true);
combo->lineEdit()->setPlaceholderText("请输入或选择...");
// 限制只能输入数字
combo->lineEdit()->setValidator(new QIntValidator(0, 9999, this));
```

还有一个和可编辑模式密切相关的设置是 `setInsertPolicy()`。当用户在可编辑模式下输入了一个新值并按下回车时，QComboBox 可以自动把这个新值插入到下拉列表中。插入的位置由 InsertPolicy 决定，常用的策略有：QComboBox::InsertAtTop（插入到列表顶部）、QComboBox::InsertAtBottom（插入到列表底部）、QComboBox::NoInsert（不自动插入）。如果你不想让用户的输入"污染"你的预设列表，记得设为 NoInsert：

```cpp
combo->setInsertPolicy(QComboBox::NoInsert);
```

最后提一下 `clear()` 方法——它会清空所有选项，把 QComboBox 恢复到空白状态。在可编辑模式下，clear() 同时也会清空文本框的内容。

### 3.4 setModel() 用自定义 Model 填充选项

QComboBox 的底层实现用的是 Qt 的 Model/Viewer 架构——它内部持有一个 QAbstractItemModel，所有的选项数据都存在这个 Model 中。addItem、addItems、insertItem 这些方法本质上都是在操作这个内部 Model（默认是一个 QStandardItemModel）。当你需要更灵活的数据管理方式时，可以直接给 QComboBox 挂一个自定义的 Model。

最常见的情况是用 QStandardItemModel 手动构建选项树。QStandardItemModel 是 Qt 提供的一个通用 Model 实现，你可以为每一行设置多个角色的数据（显示文本、图标、工具提示、用户数据等）：

```cpp
auto *model = new QStandardItemModel(this);

auto *item1 = new QStandardItem("北京");
item1->setData("BJ", Qt::UserRole);         // 城市编码
item1->setData("华北", Qt::UserRole + 1);   // 地区
item1->setToolTip("中华人民共和国首都");

auto *item2 = new QStandardItem("上海");
item2->setData("SH", Qt::UserRole);
item2->setData("华东", Qt::UserRole + 1);
item2->setToolTip("中国最大的经济中心");

model->appendRow(item1);
model->appendRow(item2);

combo->setModel(model);
```

用 QStandardItemModel 的好处是每个选项可以携带多个角色的数据——Qt::DisplayRole 是显示文本，Qt::ToolTipRole 是工具提示，Qt::UserRole 到 Qt::UserRole + N 是自定义数据。你可以通过 `combo->currentData(Qt::UserRole)` 和 `combo->currentData(Qt::UserRole + 1)` 分别取出不同角色的值。

当你调用 `setModel()` 后，QComboBox 之前通过 addItem 添加的选项全部会被替换掉——因为 setModel 是替换整个数据源，不是追加。另外有一个非常重要的细节：QComboBox 不会接管 Model 的所有权。如果你传入的是一个在栈上创建的 Model 或者一个 parent 为 nullptr 的 Model，你需要自己保证 Model 的生命周期不短于 QComboBox，否则会出现悬垂指针导致崩溃。通常的做法是把 QComboBox（或者它的父 widget）设为 Model 的 parent。

如果你需要自定义每个选项的显示方式——比如在选项旁边显示一个颜色块或者一个小图标——你可以给 QComboBox 设置一个自定义的 `QAbstractItemDelegate` 或 `QStyledItemDelegate`，重写它的 `paint()` 方法来实现自定义绘制。不过这个话题比较深入，我们在 Model/View 系列教程里会专门讲。

还有一个实用的模式：如果你有一个现成的 QStringList 或其他数据源，想在不丢失类型信息的前提下填充到 QComboBox 中，可以用 QStandardItemModel 做中间层，把原始数据挂到自定义角色上：

```cpp
auto *model = new QStandardItemModel(this);
QStringList cities{"北京", "上海", "广州"};
QStringList codes{"BJ", "SH", "GZ"};

for (int i = 0; i < cities.size(); ++i) {
    auto *item = new QStandardItem(cities[i]);
    item->setData(codes[i], Qt::UserRole);
    model->appendRow(item);
}

combo->setModel(model);
```

## 4. 踩坑预防

第一个坑是添加第一个选项时 currentIndex 从 -1 变成 0 会触发信号。如果你的信号处理代码里有依赖"初始化完成"的逻辑，可能会在初始化阶段就被意外触发。解决方法是在添加完所有选项之后再连接信号，或者用 blockSignals(true) 包裹初始化代码。

第二个坑是 setCurrentText 在非可编辑模式下如果找不到匹配项就静默失败——不会报错，也不会改变当前选中项，但也不会给你任何提示。如果你依赖 setCurrentText 的副作用，一定要检查设置是否生效。

第三个坑是 setModel() 会替换掉所有之前添加的选项。不要以为 setModel 是"在现有选项基础上追加"——它是完全替换数据源。如果你想在 setModel 之后再加选项，需要通过 Model 的 API 来操作，而不是用 addItem。

第四个坑是可编辑模式下 currentText() 的语义变了。在只读模式下它总是返回下拉列表中某一项的文本；在可编辑模式下它返回的是用户实际输入的内容，可能不在列表中。如果你的业务逻辑假设 currentText() 一定是有效选项，在可编辑模式下就会出问题。

第五个坑是 insertItem 的索引越界行为。如果你传了一个超过 count() 的索引，Qt 会自动调整到末尾——这不会崩溃，但如果你期望选项出现在特定位置，可能会搞错顺序。

## 5. 练习项目

我们来做一个综合练习：创建一个"城市信息选择器"窗口，覆盖 QComboBox 的四种核心用法。窗口分为上下两个区域。上半部分是一个 QFormLayout 构建的配置面板：第一行是一个普通的 QComboBox（不可编辑），用 addItem 逐个添加六个城市（北京、上海、广州、深圳、杭州、成都），每个城市关联一个城市编码作为 userData。当用户选择某个城市时，在右侧的 QLabel 中显示城市编码（通过 currentData() 获取）。第二行是另一个 QComboBox（可编辑），用 addItems 批量添加相同的城市列表，设置 setEditable(true) 和 setInsertPolicy(NoInsert)，下方有一个 QLabel 显示用户输入的文本（通过 currentText() 获取）——如果输入的文本不在列表中，QLabel 用红色字体显示"自定义输入"。第三行是第三个 QComboBox，使用 QStandardItemModel 作为数据源，每个选项携带城市名称（DisplayRole）、城市编码（UserRole）、所在地区（UserRole+1）三个角色的数据，选中后在右侧的 QLabel 中同时显示编码和地区信息。下半部分是一个 QPlainTextEdit（只读），作为操作日志区域——每次用户切换选项时追加一条记录，格式为"[时间] 城市选择框：从 XX 切换到 YY"。

几个提示：三个 QComboBox 的 currentIndexChanged 信号各自连接到对应的更新槽函数；日志区域用 QTime::currentTime() 加时间戳；可编辑框的实时文本变化用 currentTextChanged 信号监控；自定义 Model 的 QStandardItem 不要忘记设置 parent 或通过 appendRow 加入 model 来确保内存管理正确。

## 6. 官方文档参考链接

[Qt 文档 · QComboBox](https://doc.qt.io/qt-6/qcombobox.html) -- 下拉选择框控件

[Qt 文档 · QComboBox::InsertPolicy](https://doc.qt.io/qt-6/qcombobox.html#InsertPolicy-enum) -- 可编辑模式下的插入策略

[Qt 文档 · QStandardItemModel](https://doc.qt.io/qt-6/qstandarditemmodel.html) -- 通用标准项模型

[Qt 文档 · QStandardItem](https://doc.qt.io/qt-6/qstandarditem.html) -- 标准项（Model 中的数据单元）

---

到这里，QComboBox 的四个核心维度我们就全部讲完了。addItem / addItems / insertItem 覆盖了各种添加选项的方式，currentIndex / currentText / currentData 三种获取当前值的方式各有适用场景，setEditable(true) 把 QComboBox 从纯选择器变成了选择+输入的组合控件，setModel() 则打开了 Model/Viewer 架构的完整能力。在日常开发中，掌握这四块内容基本上可以应对所有和下拉选择相关的需求了。
