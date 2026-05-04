# 现代Qt开发教程（新手篇）3.52——QHeaderView：表头控件

## 1. 前言 / 表头远不止一行文字

前面几篇我们在用 QTableWidget、QTableView、QTreeView 的时候已经频繁接触到了 QHeaderView——horizontalHeader() 和 verticalHeader() 返回的就是它。但我们之前只是把它当作一个"设置列宽策略"和"隐藏行号"的工具来用，并没有真正深入它本身的能力。实际上 QHeaderView 是 Qt Model/View 架构中一个非常独立的控件，它负责在 QTableView 的顶部和左侧、QTreeView 的顶部渲染那一排表头，同时承载了列宽调整、列排序指示、列显示/隐藏、甚至自定义绘制等一系列功能。如果你在做数据密集型的界面——比如报表系统、数据分析面板、配置管理器——对表头的精细控制几乎是刚需。

QHeaderView 的设计哲学和 Qt 的其他 View 类一脉相承：它不持有数据，而是通过 Model 的 headerData 方法来获取每个表头分区的文本、图标、对齐方式等信息。这使得它天生就支持动态更新——当 Model 的 headerData 发生变化时，QHeaderView 会自动刷新显示。它也天生支持和 QSortFilterProxyModel 配合——排序状态的变化会自动反映在表头的排序指示器上。

今天的内容从四个方面展开。先看 setSectionResizeMode 的四种列宽策略以及它们的组合使用方式，然后研究 setSortIndicator 和排序指示器，接着用 hideSection 和 showSection 控制列的显示与隐藏，最后通过继承 QHeaderView 并重写 paintSection 实现自定义表头绘制。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QHeaderView 在 QtWidgets 模块中，链接 Qt6::Widgets 即可。示例代码涉及 QHeaderView、QTableView、QStandardItemModel、QPainter、QStyleOptionHeader、QPushButton、QLabel、QComboBox、QVBoxLayout 和 QHBoxLayout。

## 3. 核心概念讲解

### 3.1 setSectionResizeMode 四种列宽策略

setSectionResizeMode 是 QHeaderView 最核心的方法之一，它决定了每个表头分区（section）的宽度调整策略。理解这四种模式的区别和组合使用方式，是做好表格布局的基础。

Interactive 是默认模式。在这种模式下，用户可以通过鼠标拖拽表头分区之间的分隔线来手动调整列宽，程序也可以通过 resizeSection(logicalIndex, size) 在代码中设置列宽。列宽不会根据内容自动变化——一旦用户或者代码设定了一个值，它就保持不变，直到下一次被手动改变。这种模式给用户最大的自由度，适合大部分通用场景。

Fixed 模式把列宽锁定为一个固定值。你需要先通过 resizeSection 或者 setSectionResizeMode 之后调用 resizeSection 来设定宽度，之后用户不能拖拽调整，内容超出的部分会被截断。这种模式适合宽度已经确定的列——比如状态列（固定 80 像素）、操作按钮列（固定 100 像素）。它在 UI 布局中的角色类似于 HTML 的 `width: 120px; min-width: 120px; max-width: 120px;`。

```cpp
auto *header = tableView->horizontalHeader();

// 第一列固定 100 像素，不可拖拽
header->setSectionResizeMode(0, QHeaderView::Fixed);
header->resizeSection(0, 100);
```

ResizeToContents 模式根据列中所有条目的内容宽度自动计算列宽——它遍历 Model 中该列的所有数据项，计算每个数据项的文本宽度（考虑 delegate 的 sizeHint），取最大值作为列宽。同时它也会考虑表头文本自身的宽度——取"内容最大宽度"和"表头文本宽度"中的较大者。这种模式适合数据长度变化不大、但你想让每列恰好容纳内容的场景——比如 ID 列、状态列。

```cpp
// 第二列根据内容自适应
header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
```

Stretch 模式让列自动拉伸以占满 QHeaderView 的可用空间。如果有多个列被设为 Stretch，它们会平分剩余空间。这种模式适合"弹性布局"——让表格的宽度被充分利用，不会在右侧留出一块空白。最常见的做法是让一个描述性的长文本列设为 Stretch，其他列用 Fixed 或者 ResizeToContents 保持紧凑。

```cpp
// 最后一列拉伸占满剩余空间
header->setSectionResizeMode(3, QHeaderView::Stretch);
```

在实际项目中，混合使用多种策略是最常见的做法。下面这个组合在员工管理表格中非常典型：编号列和状态列用 ResizeToContents 保持紧凑，部门列用 Fixed 固定宽度防止太宽，描述列用 Stretch 弹性填充剩余空间。这样无论窗口怎么缩放，布局都始终合理。

```cpp
header->setSectionResizeMode(0, QHeaderView::ResizeToContents);  // 编号
header->setSectionResizeMode(1, QHeaderView::ResizeToContents);  // 姓名
header->setSectionResizeMode(2, QHeaderView::Fixed);             // 部门
header->resizeSection(2, 120);
header->setSectionResizeMode(3, QHeaderView::ResizeToContents);  // 职位
header->setSectionResizeMode(4, QHeaderView::ResizeToContents);  // 工龄
header->setSectionResizeMode(5, QHeaderView::Stretch);           // 备注
```

setSectionResizeMode 有两个重载。单参数版本 setSectionResizeMode(ResizeMode mode) 把所有列设为同一种模式。双参数版本 setSectionResizeMode(int logicalIndex, ResizeMode mode) 只影响指定列。如果你先调用单参数版本设了一个全局默认，再对某些列单独调用双参数版本覆盖，后面的调用会覆盖前面的设置——顺序很重要。

还有一个容易忽略的辅助方法 setStretchLastSection(bool)。当它设为 true 时，最后一列自动拉伸占满剩余空间——效果等同于对最后一列单独设 Stretch。这是一个快捷方式，在你不确定最后一列的索引时用起来很方便。

```cpp
// 快捷方式：让最后一列自动拉伸
header->setStretchLastSection(true);
```

setMinimumSectionSize(int size) 和 setMaximumSectionSize(int size) 设置所有列的最小和最大宽度约束。即使列的模式是 Stretch 或者用户手动拖拽，列宽也不会小于最小值或者大于最大值。这在防止某些列被挤压到不可读，或者被拉伸到离谱的宽度时非常有用。

```cpp
header->setMinimumSectionSize(50);   // 最窄 50px，保证能看见
header->setDefaultSectionSize(120);  // 默认宽度 120px
```

### 3.2 setSortIndicator 排序指示

当 QTableView 或者 QTreeView 配合 QSortFilterProxyModel 使用排序功能时，表头上会显示一个排序指示器——一个小箭头，告诉用户当前按哪一列排序、是升序还是降序。这个指示器由 QHeaderView 自动管理，你通常不需要手动控制它。但在某些场景下——比如你需要编程触发排序、或者需要自定义排序指示器的行为——就需要了解 setSortIndicator 的用法。

setSortIndicator(int logicalIndex, Qt::SortOrder order) 手动设置排序指示器。logicalIndex 是排序列的逻辑索引（注意是逻辑索引，不是视觉位置），order 是 Qt::AscendingOrder（升序）或者 Qt::DescendingOrder（降序）。调用这个方法后，QHeaderView 会在对应列的表头右侧画一个方向箭头，同时发射 sortIndicatorChanged(int logicalIndex, Qt::SortOrder order) 信号。

```cpp
auto *header = tableView->horizontalHeader();

// 手动设置排序指示器：按第三列升序
header->setSortIndicator(2, Qt::AscendingOrder);
header->setSortIndicatorShown(true);
```

sortIndicatorChanged 信号是连接排序逻辑的关键。如果你使用 QSortFilterProxyModel，通常的做法是把这个信号连接到 proxy model 的 sort 方法上。但实际上 QTableView 的 setSortingEnabled(true) 已经帮你做了这件事——它会自动连接信号并调用 proxy model 的 sort。只有在你不使用 setSortingEnabled 而是手动管理排序时，才需要直接操作 setSortIndicator。

```cpp
// 常规做法：一行代码开启排序，表头自动处理指示器
tableView->setSortingEnabled(true);

// 等价的手动做法
connect(header, &QHeaderView::sortIndicatorChanged,
        [proxyModel](int column, Qt::SortOrder order) {
    proxyModel->sort(column, order);
});
```

setSortIndicatorShown(bool) 控制排序指示器是否可见。默认是 true。如果你不想在表头上显示排序箭头（但仍然保持排序功能），可以把它设为 false。某些 UI 设计风格中，排序状态可能通过其他方式传达给用户（比如在状态栏显示"按 X 列升序排列"），此时隐藏表头上的排序指示器会更清爽。

sortIndicatorSection() 返回当前排序指示器所在的列索引，sortIndicatorOrder() 返回当前的排序方向。这两个方法在保存/恢复表格的排序状态时很有用——你可以把它们记录到 QSettings 中，下次打开程序时恢复。

```cpp
// 保存排序状态
int sortCol = header->sortIndicatorSection();
Qt::SortOrder sortOrder = header->sortIndicatorOrder();

// 恢复排序状态
header->setSortIndicator(sortCol, sortOrder);
```

有一个细节需要注意：setSortIndicator 并不会实际触发 Model 的排序——它只是在视觉上更新了表头的箭头。真正的排序操作需要你自己在 sortIndicatorChanged 的槽函数中调用 Model 的 sort 方法，或者用 setSortingEnabled(true) 让 Qt 自动帮你连接。如果你只调了 setSortIndicator 而没有排序逻辑，箭头会变但数据不会重排——这是一个常见的困惑点。

### 3.3 hideSection / showSection 列的显示与隐藏

QHeaderView 提供了 hideSection(int logicalIndex) 和 showSection(int logicalIndex) 来控制单个表头分区的显示与隐藏。当一个列被隐藏后，它在 QTableView 中完全不可见——列数据仍然存在于 Model 中，只是 View 不渲染它。隐藏列不会影响 Model 的列索引——Model 中的第 3 列永远 是第 3 列，不管第 1 列是否被隐藏。

```cpp
auto *header = tableView->horizontalHeader();

// 隐藏第二列（索引 1）
header->hideSection(1);

// 之后如果需要显示
header->showSection(1);
```

isSectionHidden(int logicalIndex) 返回指定列是否被隐藏。这在你需要遍历所有可见列做某些操作时很有用——先检查 isSectionHidden，跳过被隐藏的列。

```cpp
// 遍历所有可见列
for (int col = 0; col < model->columnCount(); ++col) {
    if (!header->isSectionHidden(col)) {
        // 处理可见列
    }
}
```

sectionsHidden() 返回是否有任何列被隐藏。这是一个快速检查——如果你从来没有隐藏过任何列，这个方法返回 false。

这里有一个非常重要的概念区分：逻辑索引（logical index）和视觉索引（visual index）。逻辑索引是 Model 中列的固定编号——Model 的第 0 列永远是第 0 列。视觉索引是列在 QHeaderView 中从左到右的实际显示位置。当用户通过拖拽表头来重新排列列的顺序时，逻辑索引不会变，但视觉索引会变。当一个列被隐藏时，它的逻辑索引仍然存在，但它没有视觉索引（因为它不可见）。

```cpp
// 逻辑索引 <-> 视觉索引 转换
int visualPos = header->visualIndex(2);  // 第 2 列在视觉上的位置
int logicalIdx = header->logicalIndex(1);  // 视觉上第 1 个位置的逻辑列号
```

logicalIndex(int visualIndex) 把视觉位置转换成逻辑索引，visualIndex(int logicalIndex) 做反向转换。这两个方法在处理用户拖拽重排列顺序后、需要知道"用户看到的第一列对应 Model 中的哪一列"时非常关键。

sectionsMovable() 控制用户是否可以拖拽表头来重新排列列的顺序。默认是 false。如果你把它设为 true，用户可以拖拽表头分区来交换列的显示位置——这在数据分析类应用中是一个非常实用的交互方式，让用户自己决定哪些列放在最前面。

```cpp
header->setSectionsMovable(true);  // 允许拖拽重排列顺序
```

moveSection(int from, int to) 在代码中移动列的位置——from 是当前的视觉位置，to 是目标视觉位置。swapSections(int first, int second) 交换两个视觉位置的列。这些方法不会改变 Model 中的数据排列，只改变 View 中的显示顺序。

### 3.4 自定义表头绘制（继承 + paintSection）

当 QHeaderView 默认的渲染效果无法满足你的 UI 设计需求时，可以通过继承 QHeaderView 并重写 paintSection 来完全接管表头的绘制。常见的自定义绘制需求包括：在表头中画一个小的筛选图标、给表头加特殊的背景渐变、在排序箭头旁边画一个额外的状态指示器、或者完全改变表头的布局方式。

paintEvent 和 paintSection 是两个可以重写的绘制入口。paintSection(QPainter *painter, const QRect &rect, int logicalIndex) 负责绘制单个表头分区。QPainter 是绘图画布，QRect 是这个分区的矩形区域，logicalIndex 是这个分区对应的逻辑列号。你可以在 paintSection 中读取 Model 的 headerData 获取表头文本和其他属性，然后用 QPainter 自由绘制。

```cpp
class ColoredHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    explicit ColoredHeaderView(Qt::Orientation orientation,
                                QWidget *parent = nullptr)
        : QHeaderView(orientation, parent)
    {
        // 启用鼠标悬停追踪
        setAttribute(Qt::WA_Hover);
    }

protected:
    void paintSection(QPainter *painter, const QRect &rect,
                      int logicalIndex) const override
    {
        if (!rect.isValid()) return;

        painter->save();

        // 自定义渐变背景
        QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
        gradient.setColorAt(0.0, QColor("#F8F9FA"));
        gradient.setColorAt(1.0, QColor("#E9ECEF"));
        painter->fillRect(rect, gradient);

        // 底部分隔线
        painter->setPen(QPen(QColor("#DEE2E6"), 1));
        painter->drawLine(rect.bottomLeft(), rect.bottomRight());

        // 右侧分隔线
        painter->drawLine(rect.topRight(), rect.bottomRight());

        // 获取表头文本
        QString text = model()->headerData(
            logicalIndex, orientation(), Qt::DisplayRole).toString();

        // 绘制文本
        painter->setPen(QColor("#212529"));
        painter->setFont(QFont("sans-serif", 10, QFont::Bold));
        painter->drawText(rect.adjusted(8, 0, -8, 0),
                          Qt::AlignVCenter | Qt::AlignLeft,
                          text);

        // 如果有排序指示器，画一个小箭头
        if (isSortIndicatorShown()
            && sortIndicatorSection() == logicalIndex) {
            drawSortIndicator(painter, rect);
        }

        painter->restore();
    }

private:
    /// @brief 绘制排序指示箭头
    void drawSortIndicator(QPainter *painter,
                           const QRect &rect) const
    {
        int arrowSize = 6;
        int x = rect.right() - 16;
        int y = rect.center().y();

        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor("#495057"));

        QPolygon triangle;
        if (sortIndicatorOrder() == Qt::AscendingOrder) {
            triangle << QPoint(x, y + arrowSize)
                     << QPoint(x - arrowSize, y - arrowSize / 2)
                     << QPoint(x + arrowSize, y - arrowSize / 2);
        } else {
            triangle << QPoint(x, y - arrowSize)
                     << QPoint(x - arrowSize, y + arrowSize / 2)
                     << QPoint(x + arrowSize, y + arrowSize / 2);
        }
        painter->drawPolygon(triangle);
    }
};
```

上面这个自定义表头做了几件事情：用线性渐变替代了默认的纯色背景，画了自定义的分隔线样式，用指定的字体绘制了表头文本，还手动绘制了排序指示箭头。整体效果比默认的表头更精致。

这里有几个需要注意的点。首先，painter->save() 和 painter->restore() 是必须的——QHeaderView 在调用 paintSection 时传入的 painter 可能处于某种特定状态（裁剪区域、变换矩阵等），你不应该在 paintSection 结束时留下任何状态污染。其次，rect 参数已经考虑了排序指示器、复选框等附加元素的预留空间——你需要在这个矩形内完成所有绘制。最后，如果你需要处理鼠标事件（比如在表头上画一个筛选按钮，点击后弹出筛选菜单），还需要重写 mousePressEvent 和 mouseReleaseEvent。

将自定义的 QHeaderView 安装到 QTableView 上需要在创建 QTableView 之后手动替换——QTableView 默认创建的水平和垂直 QHeaderView 是内部对象，你需要 new 一个自定义的 QHeaderView 然后通过 setHorizontalHeader 设进去。

```cpp
auto *tableView = new QTableView;
auto *customHeader = new ColoredHeaderView(Qt::Horizontal, tableView);
tableView->setHorizontalHeader(customHeader);
```

setHorizontalHeader(QHeaderView *header) 会替换掉 QTableView 默认的水平表头。QTableView 会接管新 header 的所有权，并在析构时自动 delete 它。同理，setVerticalHeader(QHeaderView *header) 替换垂直表头。替换后，之前通过 horizontalHeader() 设置的属性（比如列宽策略、排序指示器）全部丢失——你需要在新 header 上重新设置。

## 4. 踩坑预防

第一个坑是 paintSection 中忘记调用 painter->save() 和 painter->restore()。QHeaderView 在绘制每个分区时复用同一个 QPainter 对象，如果你在 paintSection 中修改了 QPainter 的状态（画笔、画刷、字体、裁剪区域）而没有恢复，后续分区的绘制会被你的状态污染，导致显示异常。养成习惯：paintSection 的第一行 save，最后一行 restore，中间随意折腾。

第二个坑是 setSectionResizeMode 的单参数版本会覆盖所有列的模式。如果你先对某些列单独设了策略，然后不小心调了一次 setSectionResizeMode(QHeaderView::Interactive)，之前所有的单独设置都会被清掉。双参数版本是安全的——它只影响指定列。

第三个坑是拖拽重排列顺序后，QTableView 的 setColumnHidden 的行为可能和你预期的不一样。setColumnHidden 接受的是逻辑索引——不管用户怎么拖拽列的顺序，逻辑索引永远是 Model 中的列号。如果你用视觉位置去调用 setColumnHidden，隐藏的可能是错误的列。需要用 logicalIndex(visualPos) 做转换。

第四个坑是自定义 QHeaderView 时如果重写了 paintEvent 而不是 paintSection，你需要自己处理所有的表头绘制逻辑——包括每个分区的位置计算、分隔线绘制、排序指示器绘制等。除非你真的需要改变整个表头的布局方式，否则重写 paintSection 更安全也更简单。

第五个坑是 hideSection 之后 Model 的 columnCount 不会变。被隐藏的列仍然存在于 Model 中，index(row, hiddenColumn) 仍然有效。如果你在遍历 Model 数据时忘了考虑隐藏列的逻辑，可能会读取到不应该展示给用户的数据。在 View 层面遍历可见列时用 logicalIndex + isSectionHidden 过滤。

## 5. 练习项目

我们来做一个综合练习：创建一个窗口，中央是一个 QTableView 配合 QStandardItemModel，展示一个 6 列的商品清单数据（编号、名称、分类、单价、库存、备注）。窗口右侧是一个控制面板，包含三组控件。第一组是四个 QPushButton："固定模式"、"自适应模式"、"拉伸模式"、"交互模式"，点击后通过 setSectionResizeMode 切换所有列的列宽策略，方便对比四种模式的效果差异。第二组是六个 QCheckBox，每个对应一列，勾选/取消控制对应列的 hideSection/showSection。第三组是一个 QComboBox 显示所有列名，选中某列后点击旁边的"按此列排序"按钮，调用 setSortIndicator 设置排序指示器并触发 QSortFilterProxyModel 的 sort。表头使用自定义 QHeaderView 子类——在表头文本左侧画一个小的彩色方块，颜色根据列索引从预设的调色板中取（第一列蓝色、第二列绿色、第三列橙色，以此类推）。

提示：自定义 paintSection 中，先用 rect.adjusted() 在左侧预留出 20 像素的方块空间，用 QPainter::drawRoundedRect 画圆角矩形色块，再用剩余空间绘制文本。调色板可以用一个 QColor 数组来维护。

## 6. 官方文档参考链接

[Qt 文档 -- QHeaderView](https://doc.qt.io/qt-6/qheaderview.html) -- 表头控件

[Qt 文档 -- QTableView](https://doc.qt.io/qt-6/qtableview.html) -- Model 驱动表格视图

[Qt 文档 -- QStyleOptionHeader](https://doc.qt.io/qt-6/qstyleoptionheader.html) -- 表头绘制选项

[Qt 文档 -- QAbstractItemView](https://doc.qt.io/qt-6/qabstractitemview.html) -- 抽象 View 基类

---

到这里，QHeaderView 的核心用法就全部讲完了。setSectionResizeMode 提供了四种灵活的列宽策略和它们的混合组合方式，setSortIndicator 和排序指示器让用户直观地看到当前的排序状态，hideSection/showSection 配合逻辑索引和视觉索引的概念实现了列的动态显示隐藏，自定义 paintSection 让你完全掌控表头的渲染效果。当你需要对表格的表头做精细控制时，QHeaderView 提供的能力远比你想象的丰富。
