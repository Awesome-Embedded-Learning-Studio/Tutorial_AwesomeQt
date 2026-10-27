---
title: "3.47 QListView 进阶"
description: "入门篇咱们用 QListView 配 QStringListModel 列过字符串，进阶篇把协作链读穿：setModel 一次调用断旧连接、接一整批模型信号、就地重建 QItemSelectionModel 并 reset（setSelectionModel 走在 setModel 前会被顶掉，换上的选择模型必须绑同一个模型），paintEvent 只把与刷新矩形相交的条目交给 itemDelegateForIndex 逐个 paint（默认委托 QStyledItemDelegate 经 multiData 一次取 DisplayRole、DecorationRole 等七个角色，SizeHintRole 在 sizeHint 里消费，QSS 的 ::item 由样式表样式接手 CE_ItemViewItem），插删任意行都触发整表重排而 uniformItemSizes 改取缓存尺寸全表复用（QListViewPrivate 默认 spacing 0、batchSize 100、layoutMode SinglePass、movement Static），懒加载触发点在视图侧 verticalScrollbarValueChanged 滚到最大值即调 fetchMore（updateGeometries 的零毫秒定时器负责首批，模型侧 rowCount 只报已加载数），setRootIndex 建立视觉根、wordWrap 经基类 initViewItemOption 置 WrapText、setViewMode 换内部布局基类并按掩码回退未显式设置的属性，滚动模式默认从样式提示 SH_ItemView_ScrollMode 取值而 QCommonStyle 给 ScrollPerItem，配套示例在 examples/advanced/03-qtwidgets/47-qlistview-advanced/"
---

# 现代Qt开发教程（进阶篇）3.47——QListView 进阶

[3.46 QListWidget 进阶](./46-qlistwidget-advanced.md) 结尾把话递了过来：QListWidget 把模型藏在壳里换省心，条目一多内存吃紧，把外壳翻到底、QListView 配自定义模型才是出路。本篇接的就是这一棒。模型换成您自己给的，视图侧的事一件件浮出来：setModel 之后选择模型归谁，绘制和编辑交给谁，插一行删一行视图怎么响应，十万行数据怎么懒加载。46 篇讲过的私有模型挂载、batchSize 分批布局、movement 的适用面，本篇只挂链不重讲。

引用文件咱们也交代清楚：以 qlistview.cpp 为主，模型连接与懒加载补 qabstractitemview.cpp，角色消费补 qstyleditemdelegate.cpp，样式表接手补 qstylesheetstyle.cpp。配套示例的完整玩法放在结语，正文引用文件名加行号。

## setModel 一落地，选择模型跟着就地换新

咱们从 setModel 读起，出处 qabstractitemview.cpp 第 715-764 行。它把旧模型上的连接全部断开，然后接一整批模型信号，从 dataChanged、rowsInserted 一直接到 modelReset、layoutChanged，出处第 727-753 行。您的模型发什么，视图就靠这批槽响应什么，本篇后面几节讲的都是这批槽里的活儿。

信号连接里有双线并行的值得认。rowsInserted 一路连公有槽、一路连私有槽（第 734-737 行），删行那边，rowsAboutToBeRemoved 连公有槽，rowsRemoved 连私有槽（第 738-741 行）。公有槽是留给子类重写的响应入口，私有槽做辅助的事，无障碍事件加几何刷新（第 3589-3605、3707-3719 行）。您子类化 QListView 时重写的是公有那一路，私有那一路照跑不冲突。

咱们接着看清单末尾，本节正主在那里。setModel 就地 new 一个 QItemSelectionModel 挂给视图（第 759 行），紧接 setSelectionModel 换上（第 762 行），收尾一个 reset()，把编辑器、根索引、布局全部重置（第 764 行）。

```cpp
// setModel 的收尾动作（qabstractitemview.cpp 第 759-764 行，摘录）
QItemSelectionModel *selection_model = new QItemSelectionModel(d->model, this);
connect(d->model, &QAbstractItemModel::destroyed,
        selection_model, &QItemSelectionModel::deleteLater);
setSelectionModel(selection_model);

reset(); // kill editors, set new root and do layout
```

调用时机有一处要留神。您自建了选择模型、提前调了 setSelectionModel，回头再 setModel，自建的选择模型会被顶掉，换成交给视图重建，源码注释明说了这一层（第 777-780 行）。顺序倒过来就稳：setModel 在前，setSelectionModel 在后。换上的选择模型还必须绑同一个模型，setSelectionModel 里有一句 qWarning 加提前返回守着（第 797-800 行），接好之后选择模型的 currentChanged 与 selectionChanged 两条信号由视图转发出去（第 820-823 行），您在视图上连的，源头就是它们。

## paintEvent 只把相交的条目交给委托

咱们把虚拟化的证据找出来，就在 paintEvent，出处 qlistview.cpp 第 989-1075 行。它拿刷新矩形加滚动偏移去问内部布局结构要一组索引，这组索引就是当帧要画的全部内容，出处第 998-999 行。接着逐个填 option 状态，选中的、悬停的、带焦点的，末尾交给委托去画（第 1074 行）。视口外的一个都不碰，十万行数据只有视口里那几十行在耗 paint，依据就在这几行。

```cpp
// paintEvent 的取材与派活（qlistview.cpp 第 998-999、1074 行，摘录）
const QList<QModelIndex> toBeRendered =
        d->intersectingSet(e->rect().translated(horizontalOffset(), verticalOffset()), false);
// ... 逐个填 option 状态 ...
itemDelegateForIndex(*it)->paint(&painter, option, *it);
```

还有一行值得您认下来：交替行底色画的是 PE_PanelItemViewRow（第 1067 行），源码注释写明它只管交替行的底色，其余背景由委托补齐。

indexAt 是同一套结构的另一面，示例里咱们也会用到它。它拿一个 1x1 的小矩形去问 intersectingSet，回来多个就取末位，再用 visualRect 复核点位确实落在条目里，出处 qlistview.cpp 第 1100-1111 行。示例拿它问视口顶和视口底，可见范围就是这么算出来的。

画的人是谁。基类的私有初始化里，默认装了一个 QStyledItemDelegate，qabstractitemview.cpp 第 101 行，您没换过委托，动手画的就是它。画什么由模型角色决定，multiData 一次取七样，FontRole、TextAlignmentRole 一组，ForegroundRole、CheckStateRole 一组，DecorationRole、DisplayRole，再加上 BackgroundRole，qstyleditemdelegate.cpp 第 66-74 行有清单，initStyleOption 把它们填进绘制选项（第 254-338 行），文字走 DisplayRole，图标走 DecorationRole，背景刷走 BackgroundRole。行高另走一路，sizeHint 里消费 SizeHintRole，模型没给就回退到样式算（第 389-393 行）。

您换自定义委托就一条 setItemDelegate。旧委托身上的三条信号，closeEditor、commitData，还有 sizeHintChanged，会被断开重接到新委托身上，出处 qabstractitemview.cpp 第 856-872 行，连接逻辑在第 223-232 行，换完还挂一次延后重排。编辑链也归委托：编辑器由 createEditor 建，收尾经 closeEditor 与 commitData 交回视图，视图只当中转。同一个视图里还能按行按列配不同的委托，setItemDelegateForRow 与 setItemDelegateForColumn（第 933、981 行），行委托压过列委托，取舍写在文档注释里（第 922-924 行），paintEvent 用的 itemDelegateForIndex，会按索引挑人。

QSS 的可达面也在这层。写 QListView::item 的样式表时，条目绘制会被样式表样式接手，接手位置是 CE_ItemViewItem，出处 qstylesheetstyle.cpp 第 4389 行。您要是换了自己的委托并整段重写 paint，样式表就管不到条目了，两套皮肤别混着用。

## 插一行删一行，布局整个重来

咱们看插行：模型发了 rowsInserted，QListView 的响应是清掉布局、挂一个延后重排，出处 qlistview.cpp 第 709-717 行，源码注释自己标了一句 be smarter about inserted items，到 6.9.1 增量重排还没做。删行走 rowsAboutToBeRemoved，基类那一趟走在前面，把删除区间里的编辑器与静态 widget 收掉，再清 hiddenRows 里被删的行号，收尾同样是整表重排（第 721-737 行）。dataChanged 转给内部布局基类，去更新受影响的条目（第 699-705 行）。

全量换血另有通道。模型发 modelReset（连接在第 752-753 行），视图走 reset 这一趟，QListView 在基类之外多清一份 hiddenRows，出处 qlistview.cpp 第 641-648 行。您大批量重建数据时，干脆在模型里换好容器再发 reset，视图只整表重排一回，比一千次 rowsInserted 各重排一回划算得多。

整表重排的代价靠开关压。uniformItemSizes 开成 true 之后，条目尺寸从逐项问 sizeHint 换成取缓存，缓存算一次全表复用，出处第 2484-2485 行。默认值咱们从私有构造里对一遍（第 1756-1772 行）：uniformItemSizes 默认 false，spacing 默认 0，layoutMode 默认 SinglePass，batchSize 默认 100，movement 默认 Static，resizeMode 默认 Fixed。spacing 动一下也会挂延后重排（第 337-341 行）。batchSize 配 Batched 分批布局的细节，46 篇已经讲过，本篇挂链不重讲。

## 懒加载的触发点在视图手里

canFetchMore 与 fetchMore 是模型的虚函数，什么时候调它们，却由视图定。笔者顺着源码把触发点找齐了，最常用的一处在 verticalScrollbarValueChanged：滚动条一到最大值，视图就问模型 canFetchMore，答是就立刻 fetchMore，出处 qabstractitemview.cpp 第 2877-2882 行。首批数据不用您动手，updateGeometries 会启动一只零毫秒的 fetchMoreTimer，等事件循环转一圈再取（第 2866-2871 行，派发在第 2594-2595 行），QListView::updateGeometries 末尾调了基类，出处 qlistview.cpp 第 1582 行。视图不可见时插入行也会启同一定时器（第 3477-3484 行），当前项落在末行时另有兜底（第 3859 行）。

模型侧的职责，咱们也就看清楚了。rowCount 只报已加载数，fetchMore 里用 beginInsertRows 与 endInsertRows 把新区间包住，视图自然接到 rowsInserted 走重排。示例的 LargeDataModel 正是这么写的：算本批数量、append、更新计数，出处 widget.cpp 第 59-83 行，beginInsertRows 在第 74 行，endInsertRows 在第 82 行。rowCount 只认 m_loadedCount（第 23-30 行），所以滚动条最大值反映的是已加载部分，滚到底再补一批，固有行为就是如此。

列表模型还有个 parent 要您守。示例的 canFetchMore 与 fetchMore 开头都查 parent，有效就直接返回，出处 widget.cpp 第 50-63 行。列表模型的行全挂在无效 parent 下，视图传来的若是有效索引，说明结构对不上，早退比硬撑安全。

## setRootIndex、wordWrap 与布局默认值

视觉根给咱们的便利是：setRootIndex 之后，视图显示的就是那个索引的子项。实现里按新根的列数把列号钳住，再走基类，收尾清布局与 hiddenRows，出处 qlistview.cpp 第 652-663 行。配套的 setRowHidden，您能拿它藏行不删数据，出处第 504-517 行。

藏行的判定，您也顺路看一眼：isIndexHidden 查的就是 hiddenRows 里存的行号，父索引等于视觉根、列号等于当前列才算藏，出处第 1605-1610 行，与 setRootIndex 那段清表的行为正好呼应。模型有多列的话，还有个 modelColumn 挑列显示，默认取第 0 列（第 1616-1617 行），setModelColumn 越界直接返回，合法才挂延后重排（第 1620-1627 行）。

wordWrap 的生效路径值得咱们跟一遍。setWordWrap 把私有成员 wrapItemText 置位（第 1676-1688 行），这个标志在基类的 initViewItemOption 里，变成绘制选项的 WrapText 特性，依据在 qabstractitemview.cpp，第 3941 到 3942 行，委托与布局两头都能读到。开了 wordWrap 之后窗口一变尺寸，resizeEvent 会挂一个延时十分之一秒的重排，连续拖拽窗口就不至于一路重排，出处 qlistview.cpp 第 828-853 行。

gridSize 与 spacing 是一对二选一的开关，您二选其一。设了非空 gridSize 就开网格布局，spacing 被忽略，属性文档写明了（第 370-389 行），绘制选项的 rect 尺寸也直接取网格（第 980-981 行），设置动作同样挂延后重排（第 390-396 行）。

setViewMode 值得多看一眼，换的远比长相多。它把内部布局基类整个换掉，ListMode 配 QListModeViewBase，IconMode 配 QIconModeViewBase，没被您显式设置过的属性全部回退到该模式的默认：ListMode 不换行、纵向流、movement Static、橡皮筋关，IconMode 换行、横向流、movement Free、橡皮筋开，出处第 419-458 行。您显式调过的 setter 记进了 modeProperties 掩码，换模式时不被回退。收尾还有一笔，movement 非 Static 时 dragEnabled 与 acceptDrops 跟着打开（第 461-464 行），46 篇说过的拖放联动就打这儿来。

滚动模式这里源稿说错了，笔者对着源码改准。verticalScrollMode 的默认值没有写死，是从样式提示 SH_ItemView_ScrollMode 取的，qabstractitemview.cpp 第 120-121 行，QCommonStyle 给的答案是 ScrollPerItem，qcommonstyle.cpp 第 5397-5400 行。默认一次滚一行，您换 ScrollPerPixel 才得到像素级滚动，代价是滚动条按像素计程，行高信息得跟得上。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为在 6.9.1 源码核对。QListView 属 QtWidgets，链接 Qt6::Widgets 即可。

[Qt 文档 · QListView](https://doc.qt.io/qt-6/qlistview.html) -- uniformItemSizes、wordWrap 与 setRootIndex 的定义处

[Qt 文档 · QAbstractItemView](https://doc.qt.io/qt-6/qabstractitemview.html) -- setModel、setItemDelegate 与滚动模式

[Qt 文档 · QStyledItemDelegate](https://doc.qt.io/qt-6/qstyleditemdelegate.html) -- 默认委托的角色消费与编辑链

[Qt 文档 · QAbstractItemModel](https://doc.qt.io/qt-6/qabstractitemmodel.html) -- canFetchMore、fetchMore 与 beginInsertRows

[Qt 文档 · QItemSelectionModel](https://doc.qt.io/qt-6/qitemselectionmodel.html) -- 选择模型的建立与替换

---

模型接入、委托绘制、整表重排、懒加载，都对上了号。QListWidget 的省心是别人替您管模型，QListView 把模型交还给您，换来的是协作链每一段都能自己定。数据还要往上长一层的话，下一篇 [3.48 QTreeWidget 进阶](./48-qtreewidget-advanced.md)，咱们看树形数据怎么分层。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/47-qlistview-advanced/，可执行文件 47-qlistview-advanced）。模型总量十万条，构造里一批都不加载，首批由 updateGeometries 的零毫秒定时器补齐。setModel 之后紧接 setUniformItemSizes(true)，全表共用一份缓存尺寸（widget.cpp 第 111-113 行）。往下滚，底部标签的已加载数一跳一跳往上走（第 154-160 行），可见范围拿 indexAt 问视口顶和视口底（第 148-150 行），行号总在十几条以内打转，虚拟化就这么演给您看。

您点按钮把数据一口气取完（第 168-174 行），while 循环里反复 canFetchMore 加 fetchMore，取尽后按钮自己停用。之后滚动手感没什么变化，绘制量本来就没跟总量挂钩，差距在内存那头。

完成标准照旧。setModel 之后选择模型从哪来、什么时候会被顶掉，您说得出。一帧里哪些条目会被 paint、由谁画，您数得出。插一行删一行视图做什么、uniformItemSizes 兜的是什么，您答得上。懒加载的触发点在视图的哪几处，您对得上。
