---
title: "3.49 QTreeView 进阶"
description: "进阶篇把 QTreeView 配自定义模型的路走通：直接继承 QAbstractItemModel 要交五个纯虚函数（index、parent、rowCount、columnCount、data，qabstractitemmodel.h 第 271-280 行），树形的钥匙在 createIndex 的 internalPointer 与 parent() 的互逆换算，顶层一级的 parent 返回无效 QModelIndex，QAbstractTableModel 已替您实现 index 与 parent（parent 恒返回无效索引），setRootIndex 一进来就把表头根索引同步过去再走基类，排序开关 setSortingEnabled 做三件事（亮指示、列头可点、连 sortIndicatorChanged），点击列头后 QTreeViewPrivate::sortIndicatorChanged 直接调 model->sort，而 QAbstractItemModel::sort 默认实现什么都不做，自定义模型要排序就得自己重写，expandAll 清空 viewItems 后 layout(-1, true) 递归铺整树，行高一致时 uniformRowHeights 换优化，默认缩进从样式 PM_TreeViewIndentation 取（QCommonStyle 给 dpiScaled(20)），动画只有 setAnimated 布尔开关没有时长接口，QTreeView 构造的 initialize 把基类默认 SelectItems 改成 SelectRows 所以整行选中开箱即得，分支箭头绘制在 drawBranches 走 PE_IndicatorBranch，委托可整视图可单列（setItemDelegateForColumn），表头默认值被 initialize 调成可拖列加末列拉伸（QHeaderView 裸默认是 Interactive 不可拖不拉伸），QSortFilterProxyModel 加中间一层：setFilterRegExp 随 QRegExp 在 Qt 5 退役只剩 setFilterRegularExpression，filterAcceptsRow 默认按 filterKeyColumn（默认 0，-1 扫全列）contains 命中即收，自定义过滤改参数要调 invalidateFilter，树形两难靠 recursiveFilteringEnabled 把父链一路保到根（默认关），lessThan 按 sortRole 交给 isVariantLessThan，dynamicSortFilter 默认开，配套示例在 examples/advanced/03-qtwidgets/49-qtreeview-advanced/"
---

# 现代Qt开发教程（进阶篇）3.49——QTreeView 进阶

[3.48 QTreeWidget 进阶](./48-qtreewidget-advanced.md) 结尾把话递了过来：树的规模上了万、要过滤、要自带数据结构时，QTreeView 配自己的模型才是正路。本篇接的就是这一棒。模型换成您自己写的，QTreeWidget 那层壳没了，原本被壳挡住的事一件件露面。树形索引怎么造，父怎么找，排序谁干活，过滤加在哪一层，咱们挨个交代。48 篇讲过的视图层通用机制，还有 QTreeWidget 侧的延后排序与 itemWidget，本篇只挂链不重讲。

引用文件咱们交代清楚：视图侧以 qtreeview.cpp 为主，模型基类补 qabstractitemmodel.h 与 qabstractitemmodel.cpp，代理层补 qsortfilterproxymodel.h 与 qsortfilterproxymodel.cpp，默认缩进补 qcommonstyle.cpp，表头默认值补 qheaderview_p.h。配套示例的完整玩法放在结语，正文引用文件名加行号。

## 五个纯虚函数是模型的答卷，树形的钥匙在 internalPointer

咱们直接继承 QAbstractItemModel 时，必须重写的函数有五个，index、parent、rowCount、columnCount、data，声明处 qabstractitemmodel.h 第 271-280 行。列表和表格模型好写，难就难在树形，难点集中在 index 与 parent 这对互逆换算上。

咱们看示例的写法。index 负责把树节点换成 QModelIndex，createIndex 的第三个参数放节点指针，Qt 管它叫 internalPointer，示例在 tree_model.cpp 第 131-152 行实现，开头一句 hasIndex 查边界（第 134 行）。hasIndex 本身没什么魔法，就是 rowCount 与 columnCount 的组合判断，出处 qabstractitemmodel.cpp 第 1966-1972 行。parent 反过来，从子索引里的 internalPointer 找回节点，再拿它在父节点里的行号造新索引：

```cpp
// parent 的顶层出口（tree_model.cpp 第 154-169 行，摘录）
auto* childNode = static_cast<TreeNode*>(child.internalPointer());
TreeNode* parentNode = childNode ? childNode->parent() : nullptr;
if (!parentNode || parentNode == m_rootNode.get()) {
    return QModelIndex();
}
return createIndex(parentNode->row(), 0, parentNode);
```

顶层那一级为什么返回无效索引，您对照 48 篇就通了。QTreeWidget 里顶层项的 parent() 是空指针，因为挂进 rootItem 那一步 par 被置空。到了自定义模型，数据根节点就是示例里不可见的 m_rootNode，根下第一层的 parent() 必须交无效 QModelIndex，视图靠它识别“到顶了”。

rowCount 这边拿到父索引就换回节点，数它的孩子，出处 tree_model.cpp 第 83-92 行。data 咱们只让它认 DisplayRole 与 EditRole 两种角色，出处第 100-112 行。

有两个现成的捷径可以省事。sibling 的默认实现是同行同列直接回原索引，否则 index(row, column, parent(idx))，绕一圈 parent，出处 qabstractitemmodel.cpp 第 2003-2006 行，您的模型索引造得快就可以重写它做优化。二维表更省心，QAbstractTableModel 已经替您把 index 与 parent 都实现了，parent 恒返回无效索引，出处 qabstractitemmodel.cpp 第 3809-3824 行，表格模型永远只需要关注行与列。

模型的根与视图的视觉根是两回事。setRootIndex 一进来就把表头的根索引同步过去，再走基类的实现，出处 qtreeview.cpp 第 225-231 行。视图只画 rootIndex 之下的内容，您把这个索引指到某个子树，界面上就只剩那棵子树，m_rootNode 里的数据一点没动。文件浏览器里“进入文件夹”的效果，用的正是它。

## 排序的开关在视图，干活的在模型的 sort

setSortingEnabled 做三件事，出处 qtreeview.cpp 第 827-843 行：表头亮出排序指示，列头变得可点，再把表头的 sortIndicatorChanged 信号接到私有槽上。您点一下列头，调用链走到槽里，就一句话：

```cpp
// 点击列头后的排序分发（qtreeview.cpp 第 4062-4065 行，摘录）
void QTreeViewPrivate::sortIndicatorChanged(int column, Qt::SortOrder order)
{
    model->sort(column, order);
}
```

视图只发号施令，干活的在模型。而 QAbstractItemModel::sort 的默认实现笔者把源码翻出来看了，函数体就一句 `do nothing`，出处 qabstractitemmodel.cpp 第 2472-2477 行。自定义模型不重写 sort，点表头什么都不会发生，它是五个纯虚函数之外最容易被漏掉的一个义务。重写时您要重排自己的数据，前后包一对 layoutAboutToBeChanged 与 layoutChanged，48 篇里 QTreeModel 的 sortChildren 就是这个套路，可以回看。

程序化排序咱们走 sortByColumn，出处 qtreeview.cpp 第 2662-2673 行。它设好排序指示后，在未开排序或同列同序的情形下直接调 model->sort，其余情况等信号来分发。不想自己写 sort 也有出路，把活外包给代理，后面有一整节讲它。

## expandAll 把整棵树铺进视图，大树要给视图留提示

expandAll 的实现只有几行，出处 qtreeview.cpp 第 2731-2740 行，清空 viewItems，然后 layout(-1, true) 递归铺开整棵树。文档原文带着警告，模型项数多时这个函数要花不少时间。懒加载的树您更要当心，一口气全展等于把懒加载的设计整个绕过去。只想铺开几层的话用 expandToDepth（第 2819-2845 行），示例就用它展开前两层，出处 widget.cpp 第 149 行。

大树的另一个提示是 uniformRowHeights。所有行高一致时咱们把它置 true，视图按第一项的高度统一处理，省掉逐项量尺寸，出处 qtreeview.cpp 第 123-127 行的文档与第 395-410 行的属性说明。

视觉参数咱们逐个对源码说准。默认缩进值您别在视图里找，它存在样式那边，键是 PM_TreeViewIndentation，QCommonStyle 给 dpiScaled(20)，出处 qcommonstyle.cpp 第 4857-4859 行，所以高分辨率屏幕上它自己会放大。setIndentation 设自定义值，resetIndentation 恢复样式值（qtreeview.cpp 第 349-365 行）。rootIsDecorated 只管顶层那圈展开箭头画不画，关掉它顶层节点照样能展开，双击展开归 expandsOnDoubleClick 管，属性声明在 qtreeview.h 第 32 行，默认开。

动画这边您别去找 setAnimationDuration，笔者翻遍 qtreeview.h 与 qtreeview.cpp，动画相关的公开接口只有 setAnimated 一个布尔开关，出处 qtreeview.cpp 第 864-870 行，没有时长可调。初始值也不是简单的 false，构造里按样式提示 SH_Widget_Animation_Duration 是否大于零来定，出处 qtreeview.cpp 第 3119-3121 行。

## 列的排布交给 QHeaderView，QTreeView 已替您调过默认值

表头是一只 QHeaderView，列怎么排布全归它管。有意思的是默认值分两层，咱们分开看。QHeaderView 裸用时相当朴素，整体 resize 模式 Interactive，列不可拖动换位，末列不拉伸，出处 qheaderview_p.h 第 65-75 行的初始化列表。而 QTreeView 构造里的 initialize 把表头调成了另一副样子，setSectionsMovable(true) 加 setStretchLastSection(true)，默认对齐靠左，出处 qtreeview.cpp 第 3112-3116 行。所以咱们开箱看到的树，列头可以拖住换位，末列自动吃掉剩余宽度。

列宽策略用 setSectionResizeMode 设，两个重载一个管整体一个管单列，声明在 qheaderview.h 第 100-101 行。常见配法是首列 Stretch 吃满剩余空间，数据列 ResizeToContents 按内容收窄，窗口缩小时跟着变窄的只有首列。有个叠加处您留意：树的末列拉伸已经默认开了，再给末列设 Stretch 就是重复动作，设列宽策略时把 stretchLastSection 关掉更干净。

列顺序有业务含义的场景，您记得关掉可拖动，setSectionsMovable(false) 一句的事，声明在 qheaderview.h 第 88 行。表头更深的门道，尺寸精度、隐藏列与保存恢复状态，[3.52 QHeaderView 进阶](./52-qheaderview-advanced.md) 会展开讲，本篇不抢它的地盘。

## 整行选中开箱即得，画笔的活才轮到委托

选中行为有个容易记错的默认。QAbstractItemView 的出厂值是 SelectItems，点哪个格子选哪个格子，出处 qabstractitemview.cpp 构造的初始化列表第 51 行。但 QTreeView 构造里的 initialize 把它改成了 SelectRows，顺带把选择模式定为 SingleSelection，出处 qtreeview.cpp 第 3108-3109 行。所以树默认就是整行选中，您什么都不用设。要退回格子级选中，您调 setSelectionBehavior 传 SelectItems，一行就够。

整行选中的视觉是每列各画各的选中底色，横向连起来看着像一整条。示例的委托把画笔整个接管过来，咱们看它做了什么。FullRowDelegate 在第一列自己画三角形。要不要画，问模型的 hasChildren（widget.cpp 第 43 行）。朝下还是朝右，问视图的 isExpanded（第 59 行）。横向位置它按层级深度乘 indentation 累计，出处第 46-52 行。整行高亮拿 viewport 宽度 fillRect（第 30-32 行），hover 的浅色底需要 setMouseTracking(true) 才生效，出处第 146 行。委托还顺手把 sizeHint 的行高抬到至少 28，出处第 83-90 行。

挂委托有两档，setItemDelegate 管整个视图，setItemDelegateForColumn 只管单列，声明在 qabstractitemview.h 第 104、194 行。分支箭头的默认绘制在 drawBranches，最终走 PE_IndicatorBranch 原语，出处 qtreeview.cpp 第 1877、1933 行，QSS 的 ::branch 选择器改的就是这一层的皮肤。示例用 setRootIsDecorated(false) 关掉默认分支，出处 widget.cpp 第 143 行，三角形全部自己画。您要是只需要整行选中，别学这么长，默认行为已经够了，示例的价值在于画笔完全归您时的写法。

## QSortFilterProxyModel：过滤与排序加在中间一层

48 篇说过，树大了要真过滤，代理配 QTreeView 才是正路，现在兑现。接法三步，咱们一步步挂。代理的 setSourceModel 挂上您的模型，代理的 setFilterRegularExpression 设过滤正则，视图 setModel 挂代理。此后视图拿到的一切索引都是代理索引，咱们要回源模型就调 mapToSource，反向是 mapFromSource，出处 qsortfilterproxymodel.cpp 第 3278-3295 行，选择模型与持久索引的换算代理自动包办。

正则接口的名您记准。Qt 6 里只有 setFilterRegularExpression，声明在 qsortfilterproxymodel.h 第 105-106 行，带 QRegExp 的旧名 setFilterRegExp 随 Qt 5 一起退役，6.9.1 的头文件里已经搜不到它，老代码迁移记得换名。

过滤的裁决在 filterAcceptsRow，默认实现出处 qsortfilterproxymodel.cpp 第 3231-3245 行，正则模式为空全收，否则按 filterKeyColumn 取那一列的文本做 contains 命中，任一命中即收。filterKeyColumn 默认是 0，只看首列，您把它设成 -1 就扫全列，默认值出处第 196-203 行。判定用的角色由 filterRole 决定，默认 DisplayRole，名称列不想当过滤条件就把它换成 UserRole。要自定义过滤规则，重写 filterAcceptsRow，规则参数一变就得调 invalidateFilter 让代理重算（第 3122-3127 行），Qt 6 起配套的 beginFilterChange 建议调在参数变化之前，出处第 3096-3101 行。

树形过滤有个两难，孩子命中了，父没命中，孩子跟着父一起消失。recursiveFilteringEnabled 就是解这个的，孩子命中时父链一路保到根，裁决逻辑咱们在 filterAcceptsRowInternal 里读到，出处第 399-414 行，属性默认关（第 211-214 行）。您把树撑开做搜索联想时记得开它，48 篇教的手工递归 setHidden 那套可以退休了。

排序这边代理照样接活。视图点表头，sortIndicatorChanged 调的是代理的 sort，实现出处第 2513-2522 行。代理按 lessThan 重排，默认拿 sortRole（DisplayRole）的数据交给 isVariantLessThan 比较，出处第 3209-3218 行，您要自定义比较就重写 lessThan。dynamicSortFilter 默认开（第 220-222 行），源数据一变代理自动重排重滤。您写的模型一行不用改，排序过滤全在中间层消化。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过。QTreeView 与代理都属于现成组件，链接 Qt6::Widgets 即可，自定义模型在 Qt6::Core。

[Qt 文档 · QTreeView](https://doc.qt.io/qt-6/qtreeview.html) -- setRootIndex、setSortingEnabled 与 expandAll 的定义处

[Qt 文档 · QAbstractItemModel](https://doc.qt.io/qt-6/qabstractitemmodel.html) -- 五个纯虚函数与 sort 的模型侧义务

[Qt 文档 · QSortFilterProxyModel](https://doc.qt.io/qt-6/qsortfilterproxymodel.html) -- setFilterRegularExpression、filterAcceptsRow 与 mapToSource

[Qt 文档 · QHeaderView](https://doc.qt.io/qt-6/qheaderview.html) -- 列宽策略，52 篇会展开讲

[Qt 文档 · QStyledItemDelegate](https://doc.qt.io/qt-6/qstyleditemdelegate.html) -- 自定义绘制的基类

---

树形视图配自定义模型的路走通了。视图侧的机制咱们讲到此告一段落，下一站是二维表，[3.50 QTableWidget 进阶](./50-qtablewidget-advanced.md)，QTableWidget 的壳与 QTreeWidget 同族，48 篇攒下的读壳经验正好复用。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/49-qtreeview-advanced/，可执行文件 49-qtreeview-advanced）。三层样例树是系统、网络、存储，点任意节点，状态栏沿 parent 链拼出完整路径（widget.cpp 第 163-180 行）。顺手做个实验，把 FullRowDelegate 换成默认委托，整行选中还在，对比一下两套视觉的差别。做完再给 TreeModel 的 data 加一种 UserRole，套一层 QSortFilterProxyModel 上去试试递归过滤，示例没做这一层，正好留给您练手。

完成标准照旧。五个纯虚函数为什么缺一不可、index 与 parent 怎么互逆，您说得出。点表头之后调用链落到谁的 sort，您对得上。recursiveFilteringEnabled 解决什么问题、代理索引怎么回源，您答得上。
