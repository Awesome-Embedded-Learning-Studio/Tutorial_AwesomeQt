---
title: "3.51 QTableView 进阶"
description: "进阶篇把 QTableView 配自定义 QAbstractTableModel 的路读细，树形的枝干在基类源码里就被剪掉（QAbstractTableModel::parent 恒返回无效索引，hasChildren 对有效 parent 返回 false，qabstractitemmodel.cpp 第 3822-3840 行），自定义模型只剩 rowCount、columnCount、data 三件加可选的 headerData，data() 按需生成，内存与行数无关，出厂选中是 SelectItems 加 ExtendedSelection（qabstractitemview.cpp 第 50-51 行，QTableViewPrivate::init 不碰选择，第 577-603 行），init 只加 AnyKeyPressed 编辑触发器（第 581 行），又建两只表头（第 583-592 行）并把 tabKeyNavigation 打开（第 593 行，Tab 走 moveCursor(MoveNext)，qabstractitemview.cpp 第 2430-2433 行），左上角钮画 CE_Header 接 selectAll（第 553-576、596-601 行），任一表头隐藏时角钮自动隐藏（第 2315-2322 行），setCornerButtonEnabled 只是 setEnabled（第 2847-2851 行），换表头走 setHorizontalHeader，旧的被 delete、接线五条重建、双击列分隔线接 resizeColumnToContents（第 1368-1400 行），裸 QHeaderView 的整体调宽默认 Interactive（qheaderview_p.h 第 85 行），hideColumn、showColumn、setColumnHidden、hideRow 全是转手给表头（qtableview.cpp 第 2703-2709、3262-3300 行），列留在模型里不减，setWordWrap 默认开（qtableview_p.h 第 114 行），改值后 invokeMethod 请两只表头 resizeSections（第 2820-2828 行），WrapText 特性从 initViewItemOption 来（qabstractitemview.cpp 第 3942 行），行高不会自动撑，放不下按 textElideMode 截断，默认 ElideRight（第 80 行），撑开靠 resizeRowsToContents（挂链 50），按内容调宽每列最多问 resizeContentsPrecision 默认的一千行（qheaderview_p.h 第 87 行），showGrid 默认 true 加 SolidLine（qtableview_p.h 第 109 行），paintEvent 画线拿 QPen(gridColor, 1, gridStyle)（qtableview.cpp 第 1511 行），setItemDelegateForColumn 换列委托会移出旧委托并触发重排（qabstractitemview.cpp 第 981-1000 行），配套示例在 examples/advanced/03-qtwidgets/51-qtableview-advanced/"
---

# 现代Qt开发教程（进阶篇）3.51——QTableView 进阶

[3.50 QTableWidget 进阶](./50-qtablewidget-advanced.md) 结尾把这一棒交到本篇手里：壳的便利读完了，代价也摆上了，万行级的表要脱壳，直接拿 QTableView 配 QAbstractTableModel。咱们本篇就干这件事，顺带把 QTableView 自己的几个开关读细，网格、换行、左上角钮、藏列、列级委托挨个过。46-49 篇讲过的虚拟化、委托链、selectionModel、排序分工，本篇只挂链不重讲。

引用文件咱们交代清楚。视图侧读 qtableview.cpp 与 qtableview_p.h，通用行为落到 qabstractitemview.cpp 与 qheaderview_p.h，QAbstractTableModel 的实现在 corelib 的 itemmodels 目录，文件是 qabstractitemmodel.cpp。配套示例完整玩法放结语，正文引用文件名加行号。

## 树形的枝干在基类源码里就被剪掉了，表格模型只剩三件活

49 篇咱们直接继承 QAbstractItemModel，交了五个纯虚函数，树形的钥匙在 index 与 parent 的互逆换算。QAbstractTableModel 把这两件都替咱们做了，做法相当决绝。笔者把实现翻给您看：

```cpp
// 表格模型的父链（qabstractitemmodel.cpp 第 3822-3840 行，摘录）
QModelIndex QAbstractTableModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

bool QAbstractTableModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return rowCount(parent) > 0 && columnCount(parent) > 0;
    return false;
}
```

parent 恒返回无效索引，index 也不带 internalPointer，边界查过直接 createIndex（第 3809-3812 行）。层级在这层基类里连存在的口子都没留，咱们要写的只剩 rowCount、columnCount、data，表头想自定义再补 headerData。49 篇树模型里那根挑着父子关系的 internalPointer，在表格模型里根本没有存放的地方。

咱们看示例的写法。BigTableModel 声明在 big_table_model.h 第 19 行，rowCount 与 columnCount 遇到有效的 parent 返回 0（big_table_model.cpp 第 24-39 行），data 在 DisplayRole 下按行列号现拼一个字符串（第 47-52 行），不存也不缓存。模型对外声明一百万行乘十列，内存里却只有两个 int。QTableWidget 与 QStandardItemModel 那边每格一件物件，千万格就是千万件，自定义模型把数据留在您自己的结构里，被问到才算，格子数再涨也涨不到模型的内存上。

想亲眼看见虚拟化，您在示例的 data() 里加一行计数打印，滚到第一百万行附近再滚回来，计数涨的幅度跟着可见区的格数走，跟总行数没关系。46 篇讲过的那套口径，表格侧一样成立。

## 出厂点选是格子级，树那套默认搬不过来

49 篇咱们验过 QTreeView 构造把选择行为改成 SelectRows，整行选中开箱即得。QTableView 没有这一步，出厂值停在 QAbstractItemViewPrivate 构造的初始化列表：

```cpp
// 出厂值（qabstractitemview.cpp 第 50-51 行，摘录）
selectionMode(QAbstractItemView::ExtendedSelection),
selectionBehavior(QAbstractItemView::SelectItems),
```

咱们再翻中间的 QTableViewPrivate::init，它不碰选择，只加了一句 AnyKeyPressed 编辑触发器（qtableview.cpp 第 581 行），焦点在表上时按一个可打印键就能进编辑，跟 50 篇读 QTableWidget 时看到的是同一句。

所以您点一个格子，currentIndex 与选中都落在那个格子上，整行高亮不会自己出现。想要它，您调 setSelectionBehavior(QAbstractItemView::SelectRows)，示例正是这么设的（big_table_model.cpp 第 88 行）。反过来，您做的表若要精确到格子的复制粘贴，格子级默认反而顺手，Ctrl 加拖选的多选手感 ExtendedSelection 本来就在位。

## 两只表头与左上角钮，都生在视图手里

init 里建了两只 QHeaderView，横竖各一，都调了 setSectionsClickable(true) 与 setHighlightSections(true)，出处 qtableview.cpp 第 583-592 行。调宽的默认没动过，裸 QHeaderView 的整体模式是 Interactive（qheaderview_p.h 第 85 行），行高列宽由您定。常用口径两个，行高统一时 verticalHeader()->setDefaultSectionSize(28) 一句全表同高，行号不要就 verticalHeader()->hide()。示例还把末列拉伸开了（big_table_model.cpp 第 89 行）。

不满意自带的表头，您换一只就是，setHorizontalHeader 的实现值得咱们看一眼：

```cpp
// 换一只横表头（qtableview.cpp 第 1374-1380 行，摘录）
for (const QMetaObject::Connection &connection : d->horHeaderConnections)
    disconnect(connection);
if (d->horizontalHeader && d->horizontalHeader->parent() == this)
    delete d->horizontalHeader;
d->horizontalHeader = header;
d->horizontalHeader->setParent(this);
d->horizontalHeader->setFirstSectionMovable(true);
```

旧的表头只要父对象是视图，当场 delete，新表头过继过来，模型与选择模型也顺手带过去（第 1381-1385 行）。五条信号接线随即重建，其中一条把 sectionHandleDoubleClicked 接到 resizeColumnToContents（第 1394-1395 行），所以您双击列分隔线，被双击的列按内容调宽，出处就在这里。setFirstSectionMovable(true) 是放开第 0 列的禁拖豁免，真要拖还得开 setSectionsMovable(true)，树侧那两层默认 49 篇讲过，表格这边同一个道理。

还有一个跟键盘走的小默认，咱们顺手看掉。QAbstractItemView 出厂 tabKeyNavigation 是关的（qabstractitemview.cpp 第 64 行），Tab 键把焦点送出控件。QTableViewPrivate::init 把它打开（qtableview.cpp 第 593 行），按键处理里 Key_Tab 命中时走 moveCursor(MoveNext)（qabstractitemview.cpp 第 2430-2433 行），所以在表格里按 Tab，当前格走到下一格，焦点不逃。

左上角的角钮也在 init 里出生。QTableCornerButton 是个小按钮，paintEvent 里画的是 CE_Header 原语（qtableview.cpp 第 553-576 行），clicked 接到 selectAll 上（第 596-601 行）。有一处行为您得当心，任一表头藏起来时角钮自己跟着藏（第 2315-2322 行），而 setCornerButtonEnabled 只是 setEnabled（第 2847-2851 行），置灰不等于不在，藏了表头再去找角钮是找不到的。表头自身还有一整套尺寸精度、隐藏列与状态存取的门道，都归 [3.52 QHeaderView 进阶](./52-qheaderview-advanced.md)，本篇不抢它的地盘。

## 藏列藏行全是转手给表头，模型毫不知情

hideColumn、showColumn、setColumnHidden 这批函数，实现都是一句转手，交给 horizontalHeader 的 hideSection、showSection、setSectionHidden，出处 qtableview.cpp 第 2703-2709 行与第 3273-3300 行，行版的 hideRow 同理去找 verticalHeader（第 3262 行）。所以列藏起来后 columnCount 一个不减，模型那侧的世界毫无变化，索引照常有效，只是不画。您想按可见列做运算，得自己去问表头的 isSectionHidden，视图不替您过滤。表头状态的保存与恢复也归 52 篇，藏列的记录跟着表头走。

## setWordWrap 管换行，行高要您另外撑

换行默认是开的，初值在 qtableview_p.h 第 114 行。咱们看 setWordWrap 做了什么：

```cpp
// setWordWrap 的实现（qtableview.cpp 第 2820-2828 行，摘录）
void QTableView::setWordWrap(bool on)
{
    Q_D(QTableView);
    if (d->wrapItemText == on)
        return;
    d->wrapItemText = on;
    QMetaObject::invokeMethod(d->verticalHeader, "resizeSections");
    QMetaObject::invokeMethod(d->horizontalHeader, "resizeSections");
}
```

翻转标志之外，它请横竖两只表头各跑一遍 resizeSections，因为换行会改委托算出来的高度。咱们再看绘制侧的接线，它埋在通用层，initViewItemOption 发现 wrapItemText 就给 option 挂上 WrapText 特性（qabstractitemview.cpp 第 3942 行），委托的 sizeHint 与绘制都看它。

行高不会自己撑开。表头默认 Interactive，行高停在您设的值上，换出来的多行文字放不下时按 textElideMode 截断，出厂是 ElideRight（qabstractitemview.cpp 第 80 行），超出的部分收成 “…”。想要多行全显示，您调 resizeRowsToContents，或者把竖表头调 ResizeToContents，那批函数 50 篇读过，逐格向委托要提示，万行级的表别挂在数据变化里反复调。

笔者顺带改准一个流行说法。ResizeToContents 不会扫全表，QHeaderView 的 resizeContentsPrecision 出厂是一千（qheaderview_p.h 第 87 行），按内容调宽时每列最多问前一千行的提示。一千行乘以列数的 sizeHint 依然不是白给的，大表还是该用固定列宽，但百万行就等于百万次调用的说法在 Qt 6 不成立，52 篇讲尺寸精度时还会碰到它。

## 网格线与合并格，视图自己的开关

咱们看网格。showGrid 出厂 true，线型 SolidLine（qtableview_p.h 第 109 行）。setShowGrid 只翻标志再重画视口（qtableview.cpp 第 2777-2784 行），画线的地方在 paintEvent，笔是 QPen(gridColor, 1, d->gridStyle)（第 1511 行），所以 setGridStyle 传入的 Qt::PenStyle 直接变成画线用的笔（第 2798-2805 行）。线的颜色归 QSS 的 gridline-color 管，50 篇读过接线。

合并格的 setSpan 与 clearSpans 也住在 QTableView 这边，QTableWidget 靠继承拿到。记录放在视图私有侧的集合里，插删行列会挪、排序不挪，这些 50 篇全讲过，咱们挂链。本篇要说的只有一句，QTableView 自己就是 setSpan 的正主，配自定义模型时它照样可用，与壳无关。

## 一列一个编辑器，setItemDelegateForColumn 的落点

想让某一列只许下拉选，您给那一列单独配委托，QStyledItemDelegate 子类的 createEditor 返回 QComboBox，再拿 setItemDelegateForColumn 挂上去，声明在 qabstractitemview.h 第 194 行。实现笔者带您看一眼，列委托存在 columnDelegates 表里，替换时旧委托被移出，按引用计数决定要不要断开它与视图的信号线，随后视口重画、布局重排（qabstractitemview.cpp 第 981-1000 行）。委托链的通用机制 46-47 篇讲过，本篇只认表格侧的入口，整只视图换委托走 setItemDelegate，单列的编辑器场景走本篇这一对。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过。QTableView 在 QtWidgets，QAbstractTableModel 反而住在 QtCore 的 itemmodels 里，应用链接 Qt6::Widgets 即可。

[Qt 文档 · QTableView](https://doc.qt.io/qt-6/qtableview.html) -- 表格视图，setWordWrap、setShowGrid 与两只表头的入口

[Qt 文档 · QAbstractTableModel](https://doc.qt.io/qt-6/qabstracttablemodel.html) -- index 与 parent 默认实现的出处

[Qt 文档 · QHeaderView](https://doc.qt.io/qt-6/qheaderview.html) -- setDefaultSectionSize 与调宽模式，52 篇的主角

[Qt 文档 · QAbstractItemView](https://doc.qt.io/qt-6/qabstractitemview.html) -- setItemDelegateForColumn 与选择行为的定义处

---

表格脱壳的路咱们走到这里。模型按需喂格子，视图按格点选，藏列、换行、调宽各归其位。下一站 [3.52 QHeaderView 进阶](./52-qheaderview-advanced.md)，49 篇预告过的表头深水区正在那里，本篇两只表头只摸了最外层的开关。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/51-qtableview-advanced/，可执行文件 51-qtableview-advanced），一百万行乘十列，滚动跟手。做三个实验。在 data() 里加计数打印，看着只有可见格被问到。把 big_table_model.cpp 第 88 行的 SelectRows 注掉再点格子，选中从整行缩回被点中的格子，出厂的 SelectItems 直接可见。把 data() 的返回值换成长文字再试 setWordWrap(false)，省略号从右侧出现，ElideRight 的出厂值就摆在眼前。

完成标准照旧。QAbstractTableModel 为什么写不成树，您指得出那三行源码。出厂点选是哪两样，您说得出。藏列后模型那边变没变，您有数。换行开着行高为什么不撑，您讲得清。给一列换委托时旧委托发生了什么，您答得上。
