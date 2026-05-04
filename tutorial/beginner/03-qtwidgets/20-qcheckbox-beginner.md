# 现代Qt开发教程（新手篇）3.20——QCheckBox：复选框

## 1. 前言 / 为什么 QCheckBox 比 QRadioButton 复杂

如果你觉得 QCheckBox 就是"可以同时选多个的 QRadioButton"，那只能说你对了一半。QCheckBox 确实不像 QRadioButton 那样有自动互斥的限制——每个 QCheckBox 都是独立的，选中一个不会影响其他的。但 QCheckBox 有一个 QRadioButton 完全不具备的能力：三态模式。三态复选框除了"选中"和"未选中"之外还有第三个状态——"部分选中"（`Qt::PartiallyChecked`），这个状态在表示"子项中有一部分被选中"这种层级关系时特别有用，比如你在一个文件管理器里选中了一个文件夹下的部分文件，文件夹的复选框就应该显示为半选中状态。

三态模式引入了一个新的信号选择问题：`toggled(bool)` 只能表达选中/未选中两种状态，无法处理"部分选中"。所以 QCheckBox 还有另一个信号 `checkStateChanged(Qt::CheckState)`，它的参数直接是 `Qt::CheckState` 枚举值（`Qt::Unchecked`、`Qt::PartiallyChecked`、`Qt::Checked`）。这两个信号的选择和配合是很多开发者会困惑的地方。再往深了走，复选框在层级结构中的应用——比如 QTreeWidget 中的父子节点联动——是一个在实际项目中高频出现但很少有教程系统讲解的话题。这篇文章我们就把 QCheckBox 的四个核心维度讲透：三态复选框的工作机制、两个信号的区别与选择、全选/全不选的批量操作逻辑、以及与 QTreeWidget 结合的层级复选实现。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QCheckBox 和 QTreeWidget 都属于 QtWidgets 模块，链接 Qt6::Widgets 即可。QCheckBox 在所有桌面平台上的行为一致，三态复选框的"部分选中"视觉表现（通常是一个灰色填充的方块或一条横线）由 QStyle 绘制。

## 3. 核心概念讲解

### 3.1 三态复选框：setTristate(true) 与 Qt::PartiallyChecked

默认情况下，QCheckBox 是两态的——它只有"选中"（`Qt::Checked`）和"未选中"（`Qt::Unchecked`）两种状态。用户点击复选框时在这两个状态之间来回切换。调用 `setTristate(true)` 之后，QCheckBox 进入三态模式，增加了第三个状态"部分选中"（`Qt::PartiallyChecked`），用户点击时在 未选中→部分选中→选中 三个状态之间循环。

```cpp
auto *checkbox = new QCheckBox("全选");
checkbox->setTristate(true);
```

三态复选框的典型使用场景是"父-子"层级选择。假设你有一组子项复选框（比如"选项 A""选项 B""选项 C"），还有一个"全选"复选框作为父项。当所有子项都被选中时，"全选"显示为选中状态；当所有子项都未选中时，"全选"显示为未选中状态；当部分子项被选中时，"全选"显示为部分选中状态。这个"部分选中"的视觉反馈让用户一眼就能看出不是所有子项都被选中了。

```cpp
// 设置三态复选框的状态
checkbox->setCheckState(Qt::Unchecked);          // 未选中
checkbox->setCheckState(Qt::PartiallyChecked);   // 部分选中
checkbox->setCheckState(Qt::Checked);            // 全部选中
```

`Qt::CheckState` 是一个枚举，定义在 `Qt` 命名空间中，有三个值：`Qt::Unchecked`（值为 0）、`Qt::PartiallyChecked`（值为 1）、`Qt::Checked`（值为 2）。注意 `setCheckState()` 和 `setChecked()` 的区别——`setChecked(bool)` 只能设置选中和未选中两个状态，而 `setCheckState(Qt::CheckState)` 可以设置所有三个状态。在两态模式下调用 `setCheckState(Qt::PartiallyChecked)` 实际上不会生效——你必须先调用 `setTristate(true)` 开启三态模式。

还有一个容易忽略的细节：在两态模式下，用户点击复选框会在选中和未选中之间切换。在三态模式下，用户点击的循环顺序是 未选中→部分选中→选中→未选中。但在大多数实际应用中，你不希望用户手动触发"部分选中"状态——这个状态应该由程序根据子项的选中情况自动计算。所以常见做法是在 `checkStateChanged` 信号的处理中拦截用户操作：如果用户点击了处于"部分选中"状态的复选框，根据点击后的状态来决定是全选还是全不选，然后把所有子项同步到对应状态。

```cpp
connect(parentCheck, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState state) {
    // 用户手动点击时，只处理"全选"和"全不选"两种情况
    // "部分选中"只由程序根据子项状态自动设置
    if (state == Qt::Checked) {
        setAllChildrenChecked(true);
    } else if (state == Qt::Unchecked) {
        setAllChildrenChecked(false);
    }
    // Qt::PartiallyChecked 由程序自动设置，不在这里处理
});
```

### 3.2 checkStateChanged(Qt::CheckState) vs toggled(bool) 信号区别

QCheckBox 从 QAbstractButton 继承了 `toggled(bool)` 信号，同时自己提供了 `checkStateChanged(Qt::CheckState)` 信号。这两个信号的区别在于它们对三态模式的支持程度不同。

`toggled(bool)` 只关心选中/未选中两种状态。参数 `true` 表示选中，`false` 表示未选中或部分选中——也就是说，它把 `Qt::PartiallyChecked` 和 `Qt::Unchecked` 混为一谈了。在两态模式下用 `toggled(bool)` 没有任何问题，但在三态模式下你会丢失"部分选中"这个信息。

```cpp
connect(checkbox, &QCheckBox::toggled, this, [](bool checked) {
    // 三态模式下: checked 为 true 表示 Checked
    // checked 为 false 表示 Unchecked 或 PartiallyChecked——你分不清
    qDebug() << "toggled:" << checked;
});
```

`checkStateChanged(Qt::CheckState)` 的参数直接是类型安全的 `Qt::CheckState` 枚举值，而不是旧的 `stateChanged(int)` 那样的整数。这个信号在三个状态之间切换时都会触发，你能精确知道当前是哪个状态。

```cpp
connect(checkbox, &QCheckBox::checkStateChanged, this, [](Qt::CheckState state) {
    switch (state) {
        case Qt::Unchecked:
            qDebug() << "未选中"; break;
        case Qt::PartiallyChecked:
            qDebug() << "部分选中"; break;
        case Qt::Checked:
            qDebug() << "全部选中"; break;
    }
});
```

实际项目中的选择建议是这样的：如果你的复选框是两态的（没有调用 `setTristate(true)`），用 `toggled(bool)` 更方便——你只需要一个 bool 值。如果你的复选框是三态的，或者你需要在信号处理中区分"部分选中"状态，就必须用 `checkStateChanged(Qt::CheckState)`。同一个复选框上这两个信号可以同时使用，但通常你只需要选一个。

有一个值得注意的细节：在两态模式下，`checkStateChanged` 和 `toggled(bool)` 的触发时机完全一致——它们都在 `setChecked()` 或用户点击改变状态时触发。但在三态模式下，从 `Checked` 切换到 `PartiallyChecked` 时，`toggled(bool)` 会收到 `false`（因为它把 PartiallyChecked 视为"非选中"），而 `checkStateChanged` 会收到 `Qt::PartiallyChecked`。所以如果你用 `toggled(bool)` 来监听三态复选框，当用户从"选中"切换到"部分选中"时，你的逻辑会误以为复选框被取消了——这可能不是你想要的行为。

另外，Qt 6.9 中旧的 `stateChanged(int)` 信号已被标记为 deprecated，推荐使用类型安全的 `checkStateChanged(Qt::CheckState)` 替代。两者的行为完全一致，只是参数类型不同——新信号的参数是枚举类型而不是整数，编译器可以帮你做类型检查。

### 3.3 复选框组实现"全选/全不选"逻辑

"全选/全不选"是复选框组最常见的交互模式。一个"全选"复选框控制下面所有子项复选框的选中状态，同时子项的变化也会反向影响"全选"的状态。这个双向同步逻辑看起来简单，但实现时有一些微妙的地方需要处理。

我们来拆解这个逻辑。"全选"复选框（假设叫 `selectAllCheck`）的 `checkStateChanged` 信号需要把所有子项设置为对应状态。子项复选框的 `checkStateChanged` 信号需要检查所有子项的选中情况，然后更新 `selectAllCheck` 的状态——如果所有子项都选中了，`selectAllCheck` 设为 `Checked`；如果所有子项都没选中，设为 `Unchecked`；否则设为 `PartiallyChecked`。

```cpp
// "全选"被点击：同步所有子项
connect(selectAllCheck, &QCheckBox::checkStateChanged, this,
        [this](Qt::CheckState state) {
    // 防止循环更新
    m_updating = true;

    if (state == Qt::Checked) {
        for (auto *child : m_childChecks) {
            child->setChecked(true);
        }
    } else if (state == Qt::Unchecked) {
        for (auto *child : m_childChecks) {
            child->setChecked(false);
        }
    }
    // PartiallyChecked 不处理——它只由 updateParentState() 设置

    m_updating = false;
});

// 每个子项变化时：更新"全选"的状态
for (auto *child : m_childChecks) {
    connect(child, &QCheckBox::checkStateChanged, this,
            [this](Qt::CheckState /*state*/) {
        if (!m_updating) {
            updateParentState();
        }
    });
}
```

这里面有一个关键点：防止循环更新。当"全选"被点击时，它会设置所有子项的状态，而每个子项的 `checkStateChanged` 信号又会尝试更新"全选"的状态——这就形成了循环调用。解决方案是引入一个 `m_updating` 标志位：当"全选"在更新子项时设为 true，子项的信号处理中检查这个标志位，如果为 true 就跳过更新"全选"的逻辑。

```cpp
/// @brief 根据 children 的选中情况更新 parent 的状态
void updateParentState()
{
    int checkedCount = 0;
    for (const auto *child : m_childChecks) {
        if (child->isChecked()) {
            ++checkedCount;
        }
    }

    m_updating = true;  // 防止 parent 变化再次触发 children 更新

    if (checkedCount == 0) {
        selectAllCheck->setCheckState(Qt::Unchecked);
    } else if (checkedCount == static_cast<int>(m_childChecks.size())) {
        selectAllCheck->setCheckState(Qt::Checked);
    } else {
        selectAllCheck->setCheckState(Qt::PartiallyChecked);
    }

    m_updating = false;
}
```

`updateParentState()` 中也需要设置 `m_updating = true`，因为修改 `selectAllCheck` 的 `setCheckState()` 会触发它的 `checkStateChanged` 信号，而这个信号的处理函数会遍历所有子项调用 `setChecked()`——如果不拦截，又是一个循环。这个双向同步逻辑写正确之后是非常稳固的，但第一次写的时候很容易在循环更新的坑里转半天。

### 3.4 与 QTreeWidget 结合的层级复选

QTreeWidget 中的层级复选是"全选/全不选"逻辑的升级版——它不是一层父子关系，而是多层嵌套的树形结构。父节点的选中状态取决于所有子节点（包括间接子节点）的选中情况，子节点的变化需要冒泡影响到所有祖先节点。

QTreeWidget 的每个 QTreeWidgetItem 都有内置的复选框支持——调用 `setCheckState(column, Qt::CheckState)` 就可以在指定列显示一个复选框。不需要手动创建 QCheckBox 控件。

```cpp
// 创建顶层项（父节点）
auto *parentItem = new QTreeWidgetItem({"项目组 A"});
parentItem->setCheckState(0, Qt::Unchecked);

// 创建子项
auto *child1 = new QTreeWidgetItem({"文件 1"});
child1->setCheckState(0, Qt::Unchecked);
parentItem->addChild(child1);

auto *child2 = new QTreeWidgetItem({"文件 2"});
child2->setCheckState(0, Qt::Unchecked);
parentItem->addChild(child2);

treeWidget->addTopLevelItem(parentItem);
```

用户点击树形控件中的复选框时，Qt 不会自动同步父子节点的状态——这个逻辑需要你自己在 `QTreeWidget::itemChanged` 信号的处理器中实现。信号处理器需要做两件事：第一，当某个节点被点击时，把它的所有子节点设置为相同的状态（向下传播）；第二，根据所有子节点的状态更新父节点的状态（向上冒泡）。

```cpp
connect(treeWidget, &QTreeWidget::itemChanged,
        this, [this](QTreeWidgetItem *item, int column) {
    if (column != 0) return;  // 只处理第 0 列
    updateChildren(item, item->checkState(0));
    updateParent(item->parent());
});
```

向下传播的逻辑比较直观：递归地把所有子节点设为和当前节点相同的状态。

```cpp
/// @brief 向下传播: 设置 item 及其所有子孙节点的 checkState
void updateChildren(QTreeWidgetItem *item, Qt::CheckState state)
{
    m_updating = true;
    for (int i = 0; i < item->childCount(); ++i) {
        auto *child = item->child(i);
        child->setCheckState(0, state);
        updateChildren(child, state);  // 递归处理更深层次
    }
    m_updating = false;
}
```

向上冒泡的逻辑跟前面"全选/全不选"的 `updateParentState()` 类似，但需要递归到顶层。从当前节点的直接父节点开始，统计它所有直接子节点的选中情况，计算父节点应该是什么状态，然后继续向上冒泡到祖父节点。

```cpp
/// @brief 向上冒泡: 根据 children 的选中情况更新 parent 的状态
void updateParent(QTreeWidgetItem *parent)
{
    if (parent == nullptr) return;

    int checked = 0;
    int partially = 0;
    int total = parent->childCount();

    for (int i = 0; i < total; ++i) {
        auto state = parent->child(i)->checkState(0);
        if (state == Qt::Checked) ++checked;
        else if (state == Qt::PartiallyChecked) ++partially;
    }

    m_updating = true;

    if (checked == total) {
        parent->setCheckState(0, Qt::Checked);
    } else if (checked == 0 && partially == 0) {
        parent->setCheckState(0, Qt::Unchecked);
    } else {
        parent->setCheckState(0, Qt::PartiallyChecked);
    }

    m_updating = false;

    // 继续向上冒泡
    updateParent(parent->parent());
}
```

这里同样需要 `m_updating` 标志位来防止循环更新——因为 `setCheckState()` 会再次触发 `itemChanged` 信号。在信号处理器中检查 `m_updating`，如果为 true 就直接 return 跳过处理。

```cpp
connect(treeWidget, &QTreeWidget::itemChanged,
        this, [this](QTreeWidgetItem *item, int column) {
    if (m_updating || column != 0) return;
    updateChildren(item, item->checkState(0));
    updateParent(item->parent());
});
```

层级复选的逻辑写起来代码量不小，但核心就是"向下传播 + 向上冒泡"这两个方向的递归。把这个模式掌握了，无论多深的树形结构都能正确处理。

## 4. 踩坑预防

第一个坑是 `itemChanged` 信号的循环触发。在 QTreeWidget 的层级复选实现中，`setCheckState()` 会触发 `itemChanged` 信号，而信号处理器中又会调用 `setCheckState()`——这会形成无限递归直到栈溢出。解决方案是 `m_updating` 标志位，在信号处理器入口处检查。

第二个坑是 `toggled(bool)` 在三态模式下把 `PartiallyChecked` 当作 `false` 处理。如果你用 `toggled(bool)` 来监听三态复选框，当状态从 Checked 变为 PartiallyChecked 时，你会收到 `false`，你的逻辑可能会错误地认为复选框被取消了。三态复选框务必使用 `checkStateChanged(Qt::CheckState)` 信号。

第三个坑是 `setTristate(true)` 后用户可以手动点击进入 `PartiallyChecked` 状态。如果你不希望用户手动触发这个状态，需要在信号处理中拦截——当检测到用户操作导致 `PartiallyChecked` 时，根据业务逻辑决定是改为 Checked 还是 Unchecked。或者另一个做法是：不在"全选"复选框上使用 `setTristate(true)`，而是自己手动管理——因为 `setCheckState(Qt::PartiallyChecked)` 即使没有 `setTristate(true)` 也能在代码中设置成功，只是用户点击时不会循环到这个状态。

第四个坑是 QTreeWidget 的 `itemChanged` 信号在 `addTopLevelItem()` 或 `addChild()` 时也会触发。当你在初始化阶段构建树形结构时，每一次 `setCheckState()` 调用都会触发 `itemChanged`。如果你的信号处理器在初始化阶段就被连接了，你需要在处理器中判断是否处于初始化状态，或者在构建完整个树之后再连接信号。

第五个坑是 QTreeWidget 层级复选中"部分选中"的统计逻辑需要同时考虑 `Checked` 和 `PartiallyChecked`。在 `updateParent()` 中，如果某个子节点是 `PartiallyChecked`，父节点就应该是 `PartiallyChecked`——即使其他子节点都是 `Checked`。所以判断条件应该是：所有子节点都是 `Checked` 才设为 `Checked`，所有子节点都是 `Unchecked` 才设为 `Unchecked`，其他情况都是 `PartiallyChecked`。

## 5. 练习项目

我们来做一个综合练习：创建一个窗口，上半部分演示"全选/全不选"逻辑，下半部分演示 QTreeWidget 层级复选。上半部分有一个三态的"全选"QCheckBox，下面有五个普通 QCheckBox（两态）作为子项。点击"全选"时所有子项同步选中，点击"全不选"时同步取消。子项的变化反向影响"全选"的状态（全部选中/部分选中/全不选中）。下半部分是一个 QTreeWidget，包含两级结构：顶层是两个"项目组"节点，每个项目组下有三个"文件"子节点。父子节点之间实现层级复选联动——点击父节点向下传播，子节点变化向上冒泡。窗口底部有一个 QLabel 实时显示选中统计信息（"共选中 X 个项目"）。

几个提示：三态的"全选"用 `setTristate(true)` 和 `checkStateChanged(Qt::CheckState)` 信号；QTreeWidget 用 `itemChanged` 信号配合 `m_updating` 防循环标志；向上冒泡递归调用 `updateParent(parent->parent())`；统计选中数量用 `treeWidget->selectedItems()` 并不合适（那是高亮选择，不是复选框），应该遍历所有顶层和子层 item 检查 `checkState()`。

## 6. 官方文档参考链接

[Qt 文档 · QCheckBox](https://doc.qt.io/qt-6/qcheckbox.html) -- 复选框

[Qt 文档 · QTreeWidget](https://doc.qt.io/qt-6/qtreewidget.html) -- 树形控件

[Qt 文档 · QTreeWidgetItem](https://doc.qt.io/qt-6/qtreewidgetitem.html) -- 树节点项（setCheckState）

[Qt 文档 · Qt::CheckState](https://doc.qt.io/qt-6/qt.html#CheckState-enum) -- 复选状态枚举

[Qt 文档 · QAbstractButton](https://doc.qt.io/qt-6/qabstractbutton.html) -- 按钮基类（toggled 信号）

---

到这里，QCheckBox 的四个核心维度我们就全部讲完了。三态复选框通过 `setTristate(true)` 引入了 `PartiallyChecked` 状态，让复选框能够表达"部分选中"这种层级语义；`checkStateChanged(Qt::CheckState)` 和 `toggled(bool)` 的选择取决于你是否需要处理三态；全选/全不选的批量逻辑核心是"向下传播 + 向上冒泡 + 防循环标志位"这个模式；而 QTreeWidget 的层级复选把这个模式扩展到了多层树形结构。QCheckBox 看起来是"勾一个框"这么简单的操作，但它在层级选择和批量操作场景下的实现复杂度远超表面。
