---
title: "3.48 QTreeWidget 进阶"
description: "进阶篇把 QTreeWidget 的壳读穿：构造里 new 一个 QTreeModel(1, this)，setModel 被 Q_ASSERT 拦死，QTreeWidgetItem 不是 QObject，par 与 children 串成树形链，顶层项是 invisibleRootItem 的孩子，列数归 headerItem 全树共享，setColumnCount 改的是表头项的 values，展开箭头由 hasChildren 消费项上的 childIndicatorPolicy（默认 DontShowIndicatorWhenChildless），itemExpanded 是 QTreeView::expanded 的转译，collapseAll 不发 itemCollapsed，data 的 DisplayRole 走 d->display 快路径其余角色扫 values，sortItems 走 setSortIndicator 加递归 sortChildren 逐层重排，operator< 交给 variantLessThan（数字按数值、字符串 localeAwareCompare，非字典序），开排序后 addChild 挂零毫秒 sortPendingTimer 延后整树重排，批量 insertChildren 在开排序时退化为逐项插入，clear 是一对 beginResetModel 与 endResetModel，setItemWidget 换皮 setIndexWidget 只放静态内容，InternalMove 的 dropEvent 亲手 takeChild 与 insertChild 搬指针，配套示例在 examples/advanced/03-qtwidgets/48-qtreewidget-advanced/"
---

# 现代Qt开发教程（进阶篇）3.48——QTreeWidget 进阶

[3.47 QListView 进阶](./47-qlistview-advanced.md) 结尾把话递了过来：数据还要往上长一层，看树形数据怎么分层。本篇接棒。层级一出现，列表答不了的问题就冒出来了：模型怎么回答“某行底下还有没有孩子”，视图怎么画展开箭头和缩进。QTreeWidget 把这些全包了，代价是模型被封进壳里，您换不了。47 篇讲过的视图层通用机制，委托绘制链、selectionModel 的建立、虚拟化绘制，本篇只挂链不重讲。

引用文件咱们也交代清楚：以 qtreewidget.cpp 为主，私有结构补 qtreewidget_p.h 与 qtreewidget.h，排序开关补 qtreeview.cpp，比较函数补 qabstractitemmodel.cpp。配套示例的完整玩法放在结语，正文引用文件名加行号。

## 壳里是一棵 item 树，QTreeModel 只做翻译

咱们从构造读起。QTreeWidget 的构造函数里把一个私有类 QTreeModel 挂上基类，初始一列，出处 qtreewidget.cpp 第 2549-2553 行。您要是想沿用 47 篇那套 setModel 换自定义模型，路是堵死的：setModel 的重载里有一句 Q_ASSERT，断言文本明说换模型不被允许，出处第 3312-3315 行。

数据的真身不在模型里，在 QTreeWidgetItem 身上。它不是 QObject，成员咱们在 qtreewidget.h 第 178-190 行对过：par 指向父项，children 是一个 QList，view 反指所属控件，values 按列存角色值，外加一份 itemFlags。默认值是 Selectable 加 UserCheckable 加 Enabled，再带上 DragEnabled 与 DropEnabled，出处第 187-188 行。父子关系就是 item 之间互相指针对，模型只负责把它翻译成视图听得懂的话，rowCount 拿到无效父索引就返回 rootItem 的 childCount，出处 qtreewidget.cpp 第 269-278 行。

顶层项的秘密在 rootItem。您调 QTreeWidgetItem(treeWidget) 构造时，源码走的是 rootItem 的 addChild（第 1395-1403 行），而挂在 rootItem 名下的孩子，par 会被置成空指针（第 1940-1943 行）。所以顶层项的 parent() 返回 nullptr，源头就是插进去那一步的赋值。invisibleRootItem() 返回的就是 rootItem（第 2626-2630 行），topLevelItemCount() 数的也是它的孩子，出处第 2654-2658 行。您写递归函数时拿 invisibleRootItem 当统一起点，顶层和下层就一个写法了。

列归整棵树共享，由 headerItem 统一管。columnCount 无视传入索引，只看 headerItem（第 288-294 行），表头自己也是一个 QTreeWidgetItem，藏在模型里。setColumnCount 改的是 headerItem 的 values，加列走 beginInsertColumns 与 endInsertColumns，新列默认标题是 QString::number(i + 1)，也就是 “1”“2”，出处第 140-166 行。还有个捷径您可以记下：对 headerItem 的 setText 写到越界列，setData 会自动把列数扩上去（第 1735-1737、1780-1781 行），setHeaderLabels 借的正是同一机制，出处第 2784-2792 行。

子树还能离线搭。insertChild 在项还没进树时走的是无模型分支，只动 par 和 children，不发任何信号（第 1963-1966 行），等整棵子树挂进视图，view 指针再沿子树逐个补齐（第 1950-1959 行）。咱们要批量建一棵深树时，把它在树外搭好再一口气挂进去，比逐项往活树里 addChild 少走许多信号。

摘取的方向也值得看一眼。takeChild 摘下孩子后，view 指针沿子树逐个清空（第 1996-2013 行），takeChildren 干的是同一件事，只是整窝端走（第 2082-2113 行）。摘下来的项归您管，delete 也好，换棵树挂回去也好，模型不再过问。示例里 qDeleteAll 接住 takeChildren 的返回值再统一销毁，出处 widget.cpp 第 79-81 行，写法与源码语义正好咬合。

## 展开箭头吃的是 policy，懒加载可以占位也可以亮指示

分支画不画箭头，视图问的是模型的 hasChildren，而 QTreeModel 的答案就存在每个项身上。咱们看 switch，出处第 296-313 行：

```cpp
// hasChildren 只认项上的 policy（qtreewidget.cpp 第 296-313 行，摘录）
QTreeWidgetItem *itm = item(parent);
switch (itm->d->policy) {
case QTreeWidgetItem::ShowIndicator:
    return true;
case QTreeWidgetItem::DontShowIndicator:
    return false;
case QTreeWidgetItem::DontShowIndicatorWhenChildless:
    return (itm->childCount() > 0);
}
```

默认值 DontShowIndicatorWhenChildless 咱们在 QTreeWidgetItemPrivate 的构造里找到，出处 qtreewidget_p.h 第 156-157 行，有孩子才给箭头。setChildIndicatorPolicy 改完 d->policy，收尾只挂一次延后布局（qtreewidget.cpp 第 1589-1599 行）。懒加载因此可以给空节点设 ShowIndicator 让箭头凭空出现，也可以塞一个占位子项让 childCount 大于零，默认 policy 自然亮箭头。配套示例走的是占位路线，每个目录项出生就带一个 “Loading...” 孩子，UserRole 里存 “__loading__” 当暗号，出处 widget.cpp 第 52-57 行。

信号这头笔者把皮掀开看了一眼，itemExpanded 不是新发明。构造里把基类的 expanded 连到私有槽 emitItemExpanded（第 2565-2566 行），实现只是把 QModelIndex 换回 item 再发一遍，出处第 2321-2326 行。它和 47 篇讲过的 QTreeView::expanded 是同一个动作的两层皮，折叠同理。文档里还有条注记值得您留意：collapseAll 收拢整树时不发 itemCollapsed（第 2495-2501 行），您的逻辑要是依赖逐项收拢通知，会漏。

懒加载的时机也只能选在这里。47 篇那套 canFetchMore 与 fetchMore 的触发机制，前提是模型归您写，而 QTreeModel 是私有类，私有头里没有留给您的重写入口，树形懒加载就落在 itemExpanded 这一下。示例的加载动作长在它的槽里，onItemExpanded 调 loadChildren，函数开头查第一个孩子的 UserRole 带没带占位暗号，带了才动手，加载过的项直接跳过（widget.cpp 第 69-75 行），随后 takeChildren 把占位项摘下来手动 qDeleteAll，再按 UserRole 里存的目录类型补真实孩子（第 79-97 行），名字带 folder 的孩子再挂一层占位，往下就能继续展开（第 92-96 行）。提醒一句，itemExpanded 是直连同步执行，槽里做磁盘遍历或网络请求会当场卡住界面，重活得挪去后台线程，回来再补孩子。

expandItem 与 collapseItem 是给您准备的正面入口，实现里把项换成索引再走基类的 expand 与 collapse，进出都套一个 SkipSorting 守卫，那是 qtreewidget_p.h 第 131-138 行定义的 RAII 开关，防止展开收拢动作误触发延后排序（qtreewidget.cpp 第 3108-3126 行）。

## currentItem 信号组，data 走两条存放路径

currentItemChanged 的源头，在 47 篇讲过的 selectionModel。咱们把构造里那批接线摘一段出来看，出处第 2565-2578 行：

```cpp
// 信号转译的接线（qtreewidget.cpp 第 2565-2578 行，摘录）
QObjectPrivate::connect(this, &QTreeWidget::expanded,
                        d, &QTreeWidgetPrivate::emitItemExpanded),
QObjectPrivate::connect(this, &QTreeWidget::collapsed,
                        d, &QTreeWidgetPrivate::emitItemCollapsed),
QObjectPrivate::connect(model(), &QAbstractItemModel::dataChanged,
                        d, &QTreeWidgetPrivate::emitItemChanged),
QObjectPrivate::connect(selectionModel(), &QItemSelectionModel::currentChanged,
                        d, &QTreeWidgetPrivate::emitCurrentItemChanged),
```

expanded 与 collapsed 来自视图，dataChanged 来自模型，currentChanged 来自选择模型，三条来源汇进私有槽，换成 item 再发出去。currentItemChanged 那一路的槽把新旧两个索引换回 item，出处第 2333-2339 行。您调 setCurrentItem，最终落到 setCurrentIndex（第 2829-2843 行）。示例的状态栏就是连 currentItemChanged 更新选中项文字的，连接在 widget.cpp 第 34-35 行，槽在第 154-161 行。itemChanged 信号同构，从模型的 dataChanged 转译而来（qtreewidget.cpp 第 2569-2570、2314-2320 行）。

项上的数据有两条存放路径。data() 里 DisplayRole 与 EditRole 走 d->display 的快路径，直接按列取（第 1804-1811 行），其余角色才去 values 里按列扫描（第 1817-1825 行）。setData 写入相同的值会直接返回，不发信号（第 1748、1771 行），值变了才走 emitDataChanged 把 dataChanged 发给模型，出处第 1788-1798 行。视图消费那头，QTreeModel 的 data 与 flags 都只是把索引换回 item 再问它本人（第 323-331、531-538 行），flags 转发的正是 qtreewidget.h 里声明的 itemFlags，默认自带 DragEnabled 与 DropEnabled，树上能不能拖起来，一半答案在项自己的 flags 里。您用 setFlags 关掉 ItemIsEnabled，禁用状态会沿子树传播下去（第 1620-1652 行）。

过滤咱们一句带过。QTreeWidget 没有内置的代理过滤，hidden 是项私有数据里的一位（qtreewidget_p.h 第 165 行），要搜索就得自己递归遍历 setHidden，子项命中时各级父项不能藏，否则命中的孩子跟着父项一起看不见。树大了要真过滤，QSortFilterProxyModel 配 QTreeView 是正路，那是 49 篇的地盘。

## 开了排序，插入就变成延后的整树重排

排序的比较函数笔者对源码改准了一处，源稿说默认按字符串字典序，不对。operator< 取 view->sortColumn() 那一列的 DisplayRole，出处 qtreewidget.cpp 第 1834-1840 行，交给 QAbstractItemModelPrivate::variantLessThan，两侧都是数字类型才按数值比，其余一律退回 toString 加 localeAwareCompare，出处 qabstractitemmodel.cpp 第 827-837 行。所以数值序有前提：值得是 setData 存进去的 int 或 double，setText 存的是字符串，9 和 “10” 排起来是 “10” 在前，数字与文字混排也退回字符串比较，逐字节的字典序同样不是它的行为。

整树排序的路径咱们顺一遍。QTreeWidget::sortItems 是两步，header 的 setSortIndicator 加 model 的 sort（第 2917-2922 行），开关本身在 qtreeview.cpp 第 827-843 行，开了表头才可点。QTreeModel::sort 调 rootItem 的 sortChildren 并且 climb 传 true（第 547-557 行），私有实现沿孩子逐层往下爬（第 2116-2130 行），每层前后包 layoutAboutToBeChanged 与 layoutChanged（第 2139-2153 行），持久索引由模型侧的 sortItems 换新（第 820-857 行）。列被删掉时还会补一次排序（第 2573-2574 行）。

再改准第二处。源稿说开了排序之后新项会被自动插到正确位置，机制上不是。您看 insertChild 的源码，插入照常落在给定下标，随后启动一只零毫秒的 sortPendingTimer：

```cpp
// 插入后的延后排序（qtreewidget.cpp 第 1944-1948 行，摘录）
if (view->isSortingEnabled()) {
    // do a delayed sort instead
    if (!model->sortPendingTimer.isActive())
        model->sortPendingTimer.start(0, model);
}
```

事件循环转一圈，executePendingSort 整树重排，出处 qtreewidget_p.h 第 171-182 行。位置最终是对的，代价是每次插入都排一整棵树。索引查询也会主动结算这次排序，indexOfTopLevelItem 开头就调 executePendingSort，出处 qtreewidget.cpp 第 2703-2708 行。批量的差别笔者也核过：insertChildren 在没开排序时一对 beginInsertItems 与 endInsertItems 包住整批（第 2060-2076 行），开了排序就退化成逐个 insertChild（第 2036-2040 行），批量优势当场清零。源稿教的招（关排序、批量加、加完再开）源码依据就在那两行。

clear 的对照咱们顺路说完。QTreeWidget::clear 把选择清掉再走模型的 clear，后者一对 beginResetModel 与 endResetModel 包住整棵树的删除（第 3137-3142、119-132 行），比逐个 takeTopLevelItem 便宜得多。删单个项倒是随时安全，析构函数自己从父项摘身并递归删掉孩子（第 1498-1541 行）。

## setItemWidget 与树上的拖放

setItemWidget 是 setIndexWidget 的换皮，出处第 3008-3012 行，47 篇讲过的 index widget 机制原样生效。文档附的几条限制您记一下：它只放静态内容，动态内容该交给委托，widget 要开 autoFillBackground，否则模型数据和 widget 会叠着透出来，项得已在树里才有索引可挂，所有权归树（第 2987-3006 行）。

拖放在树上多一层语义。默认 flags 自带拖与放，supportedDropActions 在模型默认之上补了 MoveAction，出处第 3211-3214 行。InternalMove 的 dropEvent 咱们读下来是亲手搬指针：把选中项 takeTopLevelItem 或 takeChild 取出，再按落点 insertChild 或 insertTopLevelItem 放回（第 3242-3305 行），搬的是 QTreeWidgetItem 本尊，孩子和数据跟着指针一起走，没有序列化还原那一层。不走 InternalMove 的普通放下则交给模型的 dropMimeData，row 与 column 都是 -1 时，它把落点解释成追加到该父项末尾（第 727-733 行）。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过。QTreeWidget 与 QTreeWidgetItem 属 QtWidgets，链接 Qt6::Widgets 即可。

[Qt 文档 · QTreeWidget](https://doc.qt.io/qt-6/qtreewidget.html) -- addTopLevelItems、itemExpanded 与 sortItems 的定义处

[Qt 文档 · QTreeWidgetItem](https://doc.qt.io/qt-6/qtreewidgetitem.html) -- setChildIndicatorPolicy、data 与 flags

[Qt 文档 · QTreeView](https://doc.qt.io/qt-6/qtreeview.html) -- expanded 信号与 setSortingEnabled

[Qt 文档 · QAbstractItemView](https://doc.qt.io/qt-6/qabstractitemview.html) -- setIndexWidget，setItemWidget 的底层

---

树形独有的事都对上号了。QTreeWidget 的省心是把模型封在壳里换来的，树的规模上了万、要过滤、要自定义数据结构时，QTreeView 配自己的模型才是正路，下一篇 [3.49 QTreeView 进阶](./49-qtreeview-advanced.md)，咱们从树形视图这头把模型的事接着讲。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/48-qtreewidget-advanced/，可执行文件 48-qtreewidget-advanced）。五个顶层目录出生各带一个 “Loading...” 占位孩子，展开 Documents，占位项被摘下换成真实子项，状态栏报出已展开的目录和子项数（widget.cpp 第 144-152 行）。名字带 folder 的子项又挂了新占位，继续往下展开就能一层层走。顺手做个实验：您把 createTopLevelItem 里的占位路线换成 setChildIndicatorPolicy(ShowIndicator)，箭头照样在，占位与 policy 各自都能点亮它，亲眼验一遍。

完成标准照旧。顶层项为什么 parent() 是空指针，您说得出。展开箭头由谁决定、itemExpanded 从哪转译来，您对得上。开了排序之后插入走什么路径、批量为什么失效，您答得上。setItemWidget 的限制与 InternalMove 搬的对象，您讲得清。
