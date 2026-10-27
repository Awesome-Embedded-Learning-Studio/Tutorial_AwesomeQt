---
title: "3.50 QTableWidget 进阶"
description: "进阶篇把 QTableWidget 的壳读细：默认选中是 SelectItems 加 ExtendedSelection（qabstractitemview.cpp 第 50-51 行初始化列表，QTableWidgetPrivate::setup 与 QTableViewPrivate::init 都不改它，offscreen 实测对上），QTableViewPrivate::init 另加 AnyKeyPressed 编辑触发器（qtableview.cpp 第 581 行），QTableModel::setItem 会删掉旧件，排序开着且设的正是排序列时走有序插入再 moveRows 搬行（qtablewidget.cpp 第 157-213 行），dataChanged 里还有 ensureSorted 兜底（第 1707-1717 行），填表要填完再开 setSortingEnabled，cellChanged 与 itemChanged 同出 QTableWidgetPrivate::emitItemChanged（第 1676-1681 行），槽里回写同格会递归，表头 item 变化走 headerDataChanged 分支（第 784-793 行），setCellWidget 包装 QAbstractItemView::setIndexWidget，旧控件被 deleteLater，QTableWidget 没有 setItemWidget（那是 QTreeWidget 与 QListWidget 的名字），horizontalHeaderItem 一批函数住在另一排数组，setSpan 与 clearSpans 是 QTableView 的接口，span 记录在视图侧（qtableview_p.h 第 179 行），重叠合并被 qWarning 拒绝，对已有合并左上角调 1x1 是撤销，插删行列时 span 平移而排序不动 span，resizeRowsToContents 那批函数全体版调 QHeaderView::resizeContents，单行单列版取内容与表头提示的较大者，QSS ::item 落在 PE_PanelItemViewItem，gridline-color 管网格线，冻结方案叠第二只表格，挂 viewport 会被 viewport->scroll 带走，官方示例挂在视图本体并用滚动条 valueChanged 同步，配套示例在 examples/advanced/03-qtwidgets/50-qtablewidget-advanced/"
---

# 现代Qt开发教程（进阶篇）3.50——QTableWidget 进阶

[3.49 QTreeView 进阶](./49-qtreeview-advanced.md) 结尾把下一站指到二维表，说 QTableWidget 的壳与 QTreeWidget 同族，48 篇攒下的读壳经验正好复用。本篇接的就是这一棒。壳里的事咱们读细：默认选中跟树不一样，setItem 在排序开着时会自己搬行，cellChanged 与 itemChanged 是同一个出口的两个名字，setSpan 的记录放在视图手里。冻结首行首列压轴，它要第二只表格配合才做得出来。46-49 篇讲过的虚拟化、委托链、selectionModel 通用机制，本篇只挂链不重讲。

引用文件咱们交代清楚。QTableWidget 侧，咱们主要读 qtablewidget.cpp 与 qtablewidget.h，setSpan 与尺寸调整的几个函数在 qtableview.cpp，QSS 的落点在 qstylesheetstyle.cpp，位于 widgets/styles 目录，官方冻结示例在 qtbase/examples 里，路径在 widgets/itemviews 下，名字叫 frozencolumn/。配套示例完整玩法放结语，正文引用文件名加行号。

## 默认选中是格子级加扩展多选，树的默认搬过来不成立

49 篇咱们验过 QTreeView 构造把选择行为改成 SelectRows，整行选中开箱即得。QTableWidget 没有这一步。它的两个构造只建内部模型，再调 QTableWidgetPrivate::setup，出处 qtablewidget.cpp 第 1873-1890 行，setup 只做信号接线，不碰选择（第 1601-1626 行），中间隔着的 QTableViewPrivate::init 也不碰，出处 qtableview.cpp 第 577-603 行。出厂值停在 QAbstractItemView 构造的初始化列表，SelectItems 加 ExtendedSelection，出处 qabstractitemview.cpp 第 50-51 行。笔者在本机拿 offscreen 模式建了一只三行三列的空表，行为打出来是 SelectItems，模式是 ExtendedSelection，与源码对上了。想要整行选中，您自己调 setSelectionBehavior(SelectRows)，Ctrl 加拖选的 Excel 手感，ExtendedSelection 本来就在位。

编辑触发器倒是被动过一处。咱们翻 init 能看到，加了一句 AnyKeyPressed（qtableview.cpp 第 581 行），所以焦点在表格上时按一个可打印键就能进编辑，不必等双击。

## setItem 在排序开着时会自己搬行

咱们从 QTableModel::setItem 读起，出处 qtablewidget.cpp 第 157-213 行。旧 item 被 delete，第 163-169 行，格子里的老物件不会留给您，想拿走就用 takeItem。排序开着、您设的又正是排序列时，代码走进有序插入，算出落点后 moveRows 把整行挪过去（第 180-207 行），这一步才是要紧处。填表时每 setItem 一次行就跳一次，填完的表行序已经不是您写的顺序了。

排序的兜底还有一层，咱们看 QTableWidgetPrivate::dataChanged，它在数据变化时检查排序列，命中就 ensureSorted（qtablewidget.cpp 第 1707-1717 行），开排序之后连改一个数字都会引发重排。所以纪律是数据填完再 setSortingEnabled(true)，反过来写就要吃行序的亏。

## cellChanged 与 itemChanged 是同一个出口的两个名字

两组信号都声明在 qtablewidget.h 第 278-295 行，item 系带 QTableWidgetItem 指针，cell 系带行列号，您挑顺手的用。咱们追到实现里看，它们不是两件事，出口只有一个：

```cpp
// itemChanged 与 cellChanged 的唯一出口（qtablewidget.cpp 第 1676-1681 行，摘录）
void QTableWidgetPrivate::emitItemChanged(const QModelIndex &index)
{
    Q_Q(QTableWidget);
    if (QTableWidgetItem *item = tableModel()->item(index))
        emit q->itemChanged(item);
    emit q->cellChanged(index.row(), index.column());
}
```

触发链咱们顺着追一遍。QTableWidgetItem::setData 改完数据，去调 QTableModel::itemChanged，出处 qtablewidget.cpp 第 1390-1413 行，它发 dataChanged（第 780-802 行），setup 里接好的线把咱们引到 emitItemChanged，接线在第 1617-1618 行。您在槽里改同一个格子，信号再发一轮，没拦着就是无限循环，三样任选：加布尔闸，blockSignals，或者槽里判断值没变就返回。自动算总分的场景尤其容易中招，合计列的更新咱们盯着这根线。

表头 item 走的是岔路。咱们在 itemChanged 的实现里能看到，发现 item 是表头物件时，发的是 headerDataChanged（qtablewidget.cpp 第 784-793 行），所以行列表头的文字变化不进 cellChanged，监听表头得用 QHeaderView 的信号。

## setCellWidget 放进真控件，所有权规则要读清楚

setCellWidget 声明在 qtablewidget.h 第 248-249 行，咱们看实现，换成索引再调 QAbstractItemView::setIndexWidget，出处 qtablewidget.cpp 第 2319-2323 行。旧控件被 deleteLater（qabstractitemview.cpp 第 3332-3336 行），所有权整个交给表格，文档把话说得很直白，被替换的 QLineEdit 会被删掉。

一个名字上的事您记准。QTableWidget 没有 setItemWidget，那个名字属于 QTreeWidget（qtreewidget.h 第 276 行）和 QListWidget（qlistwidget.h 第 213 行），表格这边的对应物叫 setCellWidget，迁移代码别找错函数。代价也直说：每个格子一只 QWidget，控件盖在 item 上，item 本来的绘制和 QSS ::item 皮肤全都不管用，几千行的表这么铺，内存和布局都吃不消。小表和特殊交互格用它，大表用委托，46-47 篇讲过的路。

编辑相关咱们顺手说三个函数。editItem，openPersistentEditor，closePersistentEditor，都声明在 qtablewidget.h 第 242-246 行，实现都是换回索引再调基类，出处 qtablewidget.cpp 第 2244-2280 行。想给一列常驻编辑器，openPersistentEditor 配委托，比每格 setCellWidget 轻。

## 表头也是 QTableWidgetItem，住在另一排数组

表头的读取与设置，咱们用三个函数，horizontalHeaderItem，setHorizontalHeaderItem，takeHorizontalHeaderItem，声明在 qtablewidget.h 第 220-226 行。set 的时候 item 的 view 指针指到表上，take 的时候置回空（qtablewidget.cpp 第 2073-2096 行）。便利函数 setHorizontalHeaderLabels，内部就是逐列造 item 再 set。表头 item 与数据 item 分开存，所以表头也有一套 QTableWidgetItem 的本事，字体、对齐这些照设不误。

## setSpan 的记录放在视图手里，插删行列会挪、排序不挪

setSpan 与 clearSpans 是 QTableView 的接口，咱们从声明看起，qtableview.h 第 80、83 行，QTableWidget 靠继承拿到，类声明见 qtablewidget.h 第 192 行。合并的记录存在视图私有侧的 spans 集合（qtableview_p.h 第 179 行），与模型里的 item 是两本记录，后面的行为差异全从这儿起头。

重叠的合并会被当面拒绝，笔者把处理代码翻出来给您看：

```cpp
// 重叠 span 直接拒绝（qtableview.cpp 第 715-720 行，摘录）
QSpanCollection::Span *sp = spans.spanAt(column, row);
if (sp) {
    if (sp->top() != row || sp->left() != column) {
        qWarning("QTableView::setSpan: span cannot overlap");
        return;
    }
```

对没合并过的格子调 setSpan(row, col, 1, 1)，同样被 qWarning 拒掉（qtableview.cpp 第 729-732 行），您就当这次调用不存在。对已有合并的左上角调 1x1 才是撤销，宽高归零后 span 被删除（第 721-724 行，删除动作在 updateSpan 第 104-107 行）。一口气全清用 clearSpans（第 3457-3462 行）。

插删行列时，视图会替咱们维护这本记录。updateInsertedRows 把每个 span 的上下沿平移，出处 qtableview.cpp 第 187-213 行，接线在第 975-1003 行，所以在合并过的表里插一行，合并块会跟着挪对位置。排序就不管了，QTableModel::sort 干的活，是把模型里的 tableItems 数组换序，出处 qtablewidget.cpp 第 497-547 行，span 一个字没动，内容却在记录底下换了行。又要排序又要合并的表没有两全接口，排序前 clearSpans，排完按新的行号重设。

示例里两处合并咱们看写法。setSpan(0, 0, 2, 1) 把项目列的产品 A 和 B 竖着并成一块，setSpan(4, 6, 2, 1) 并备注列的产品 E 和 F（frozen_table.cpp 第 67、69 行）。被盖住的格子还在模型里，item(4, 6) 照常拿得到，示例还给它改了文字（第 71 行），只是不画。

## 尺寸与皮肤：resizeRowsToContents 这几个函数，还有 QSS 的 ::item

尺寸调整的四个函数，resizeRowToContents、resizeRowsToContents、resizeColumnToContents、resizeColumnsToContents，声明在 qtableview.h 第 93-96 行，咱们从全体版看起。全体版一句话完事，调的是 QHeaderView 的 resizeSections，参数给 ResizeToContents，出处 qtableview.cpp，行号第 3321-3328 行与第 3350-3357 行，单行单列版取两边提示的较大者，一边是委托给的内容，一边是表头给的，行号第 3307-3318 行与第 3336-3347 行，所以调完的列宽至少容得下表头文字。这几个函数要逐格向委托要提示，格子多时开销跟着涨，填完数据调一次就够，别挂在每个 cellChanged 里。

QSS 侧咱们换个角度，QTableWidget::item 选择器改的是每个格子的皮肤，落点在 PE_PanelItemViewItem 原语，出处 qstylesheetstyle.cpp，第 4896-4900 行。行背景有一段兼容处理，同一行的底色借 item 的背景规则来画（第 4864-4886 行），所以给 item 设背景色就行。网格线的颜色归 gridline-color 属性管（第 692 行登记，第 5721 行接线）。

## 冻结：叠第二只表格，父对象挂在哪决定它跟不跟着滚

QTableWidget 自己没有冻结接口。官方 Frozen Column Example 的路数是叠一只子视图盖在数据区上，仓库示例照着做了，但父对象的选择有一处值得咱们对照着学。

挂到 viewport 上是最直觉的写法，仓库示例就是把 m_frozenTable 挂到主表 viewport 上，见 frozen_table.cpp 第 81 行。问题出在滚动上。表格滚动的实现，是 QAbstractItemViewPrivate 的 scrollContentsBy 里那句 viewport->scroll(dx, dy)（qabstractitemview_p.h 第 217-220 行），QWidget::scroll 连子控件一起平移，主表一滚，冻结块跟着内容一起走了。示例又没有滚动后重摆的代码，冻结效果在滚动时就散了，您跑起来一滚动就能看见。

官方示例的解法是结构性的，咱们看它怎么挂。frozenTableView 以 QTableView 自身为父，见 freezetablewidget.cpp 第 13 行，跟 viewport 平级，viewport 的平移带不走它，垂直滚动再拿两只视图的滚动条，用 valueChanged 互相同步（第 23-26 行），冻结视图的内容跟着主表滚，位置却停在左上不动。选中联动靠 setSelectionModel 共享一套选择模型（第 52 行），拖列宽的联动靠 sectionResized 信号，行号第 18-21 行。仓库示例的选中联动与列宽联动也都做对了（frozen_table.cpp 第 86、99-102 行），位置摆法也一致：横坐标从竖表头右边起，纵坐标从横表头下边起，尺寸取冻结区的行高列宽（第 111-122 行）。

还有一个便宜可捡。主表的行列表头本来就是 viewport 之外的独立控件（QTableViewPrivate::init，qtableview.cpp 第 577-603 行），天生不随视口滚，咱们要手工补的只有数据区里的行或列。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过，默认选中值另在本机 offscreen 模式实测对上。QTableWidget 属现成组件，链接 Qt6::Widgets 即可。

[Qt 文档 · QTableWidget](https://doc.qt.io/qt-6/qtablewidget.html) -- setItem、setCellWidget 与两组 cell/item 信号的定义处

[Qt 文档 · QTableWidgetItem](https://doc.qt.io/qt-6/qtablewidgetitem.html) -- 格子物件的数据角色与 clone

[Qt 文档 · QTableView](https://doc.qt.io/qt-6/qtableview.html) -- setSpan、clearSpans 与几个 resize 函数，51 篇的主角

[Qt 文档 · QHeaderView](https://doc.qt.io/qt-6/qheaderview.html) -- resizeSections 与表头信号

[Qt 文档 · Frozen Column Example](https://doc.qt.io/qt-6/qtwidgets-itemviews-frozencolumn-example.html) -- 冻结列的官方实现

---

QTableWidget 的壳咱们读到这里，便利该给的都给了，代价也都摆上了台面：每格一个物件的模型，排序与合并不能两全，控件铺格子费内存。下一站 [3.51 QTableView 进阶](./51-qtableview-advanced.md)，脱掉壳直接配 QAbstractTableModel，万行级的表在那里才是正路。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/50-qtablewidget-advanced/，可执行文件 50-qtablewidget-advanced）。八行八列的表，项目列和备注列各有一处合并，左上角叠着一只一格的冻结表。顺手做两个实验：拖动主表第 0 列的边界，冻结块的宽度跟着变，这是 sectionResized 那对连接在干活。再垂直滚一下，看冻结块跟着内容走散，这就是正文说的滚动问题，修法您挑一个：滚动条 valueChanged 一响就把 updateFrozenTableGeometry 再调一遍，或者学官方示例把冻结表改挂到表格本体。示例没做这一步，正好留给您练手。

完成标准照旧。默认选中值是哪两样，您说得出。setItem 为什么在排序开着时搬行，您对得上调用链。cellChanged 的递归怎么断，您有数。setSpan 在插删与排序下各是什么表现，您答得上。冻结方案里父对象挂在哪决定什么，您讲得清。
