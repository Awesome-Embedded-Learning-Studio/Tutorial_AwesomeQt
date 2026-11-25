---
title: "3.52 QHeaderView 进阶"
description: "进阶篇把 QHeaderView 当正主读细，列有两套编号，logicalIndex 是模型户口，visualIndex 是屏幕位置，没人拖过列时映射数组根本不建、visualIndex 原样返回自身（qheaderview.cpp 第 1083-1088 行），moveSection 一来才建映射并 emit sectionMoved（第 750、781 行），出厂 movableSections 是 false（qheaderview_p.h 第 67 行），调宽模式五个值里 Custom 只是 Fixed 的别名（qheaderview.h 第 40-47 行），全局 setSectionResizeMode 重置两个私有计数器（qheaderview.cpp 第 1231-1236 行），单列版逐列增减（第 1264-1290 行），末列自己的模式会被 stretchLastSection 盖过（第 1256-1258 行），私有分配算法把剩余长度均分再拿余数逐列加一（第 3735-3740、3769-3771 行），defaultSectionSize 出厂跟样式走（第 4033-4042 行），minimumSectionSize 出厂 -1、真取值时按字体最宽字符加两倍 PM_HeaderMargin（第 1613-1621 行），resizeSection 是唯一写入口，程序与拖动同一条路（第 860-933、2643-2707 行），按内容调宽的精度旋钮出厂一千、0 只看可见区、-1 全量（qheaderview_p.h 第 87 行），消费方其实在视图的 sizeHintForColumn（qheaderview.cpp 第 4138-4144 行、qtableview.cpp 第 2499-2507 行），隐藏列的尺寸记忆存在 hiddenSectionSize 映射里（qheaderview.cpp 第 1023-1036 行），sectionSize 对隐藏列报 0（第 633-638 行），点击翻转排序箭头只看 clickableSections、不看箭头显示开关（第 2848-2851、3853-3883 行），setSortIndicator 只发信号不排序，接线在 QTableView 的 setSortingEnabled（qtableview.cpp 第 1020-1023、2730-2745 行），saveState 用 QDataStream 存下整套私有状态、restoreState 兼容两种流版本并在收尾代发一次 sortIndicatorChanged（qheaderview.cpp 第 1808-1858、4199-4240 行），setOffset 滚动并在拖动顶到边缘时把光标顶回 handle（第 423-448 行），配套示例在 examples/advanced/03-qtwidgets/52-qheaderview-advanced/"
---

# 现代Qt开发教程（进阶篇）3.52——QHeaderView 进阶

[3.51 QTableView 进阶](./51-qtableview-advanced.md) 结尾说，49 篇预告过的表头深水区在本篇，51 自己只摸了表头最外层的开关。深水区咱们这就潜下去。49 留的原话是尺寸精度、隐藏列与保存恢复状态，51 又补了一句藏列的记录跟着表头走，两句预告本篇都兑现。51 讲过的换头五接线、藏列转手、角钮，本篇只挂链不重讲。

引用文件咱们交代清楚。表头本体读 qheaderview.cpp 与 qheaderview.h，私有默认值在 qheaderview_p.h，视图侧的接线落到 qtableview.cpp。配套示例是一只分组表头 GroupHeaderView，完整玩法放结语，正文引用文件名加行号。

## 列号有两套，没人拖过列时连映射表都不建

两套编号咱们说定。模型给列发的编号叫 logicalIndex，屏幕上排第几个叫 visualIndex，表头管的就是这两套编号的换算。有人想当然觉得表头随时维护着一张映射表，实现比这懒得多，也聪明得多：

```cpp
// visualIndex 的直通捷径（qheaderview.cpp 第 1083-1088 行，摘录）
if (d->visualIndices.isEmpty()) { // nothing has been moved, so we have no mapping
    if (logicalIndex < d->sectionCount())
        return logicalIndex;
} else if (logicalIndex < d->visualIndices.size()) {
```

映射数组为空就把 logical 原样奉还，两套编号此时是同一个数。moveSection 一来才 initializeIndexMapping（第 750 行），visualIndices 与 logicalIndices 一对数组互相反查，搬完 emit sectionMoved（第 781 行）。您在代码里换列调 moveSection 或 swapSections，用户拖列换位得开 setSectionsMovable(true)，出厂是 false（qheaderview_p.h 第 67 行），树侧默认打开的那层 49 篇讲过。拖动还有个第 0 列豁免，私有的 allowUserMoveOfSection0 注释写明树视图为假、表格为真（第 73 行），所以树的第一列默认拖不动，放开它靠 setFirstSectionMovable。想问用户动没动过列，sectionsMoved 的实现，就是查映射数组空不空（第 1740-1744 行）。

查坐标咱们有三件套，sectionSize 问宽，sectionPosition 给内容坐标，sectionViewportPosition 给视口坐标，声明在 qheaderview.h 第 68-70 行。位置是内容坐标，视口坐标等于位置减 offset（qheaderview.cpp 第 678-697 行）。示例的分组表头就靠它们现算跨度，起点取首列的 sectionViewportPosition，宽度把组内各列 sectionSize 逐个加起来，出处 group_header_view.cpp 第 64-69 行。分组的记录用的是模型列号，用户一旦拖列换位，分组还指着旧列，视觉就错了，示例干脆 setSectionsMovable(false) 关掉换位，出处第 19 行。

## 调宽模式五个值，Stretch 的分配算法值得读一遍

枚举咱们翻 qheaderview.h 第 40-47 行，Interactive 手动可拖，Stretch 平分剩余宽度，Fixed 固定不可拖，ResizeToContents 按内容算，Custom 只是 Fixed 的别名。全局 setSectionResizeMode 重置两个私有计数器，拉伸列数与按内容列数各归各（qheaderview.cpp 第 1231-1236 行），几何一变，私有侧靠计数决定要不要重排。单列版 setSectionResizeMode(logical, mode) 逐列增减计数（第 1264-1290 行），可以让一列 Stretch 配其余 Interactive。有一处例外，stretchLastSection 开着时，末列自己设的模式被无视（第 1256-1258 行的文档注释），这个属性出厂 false（qheaderview_p.h 第 70 行）。

Stretch 怎么分宽度，笔者把私有 resizeSections 的分配段翻给您看：

```cpp
// 拉伸列的分配（qheaderview.cpp 第 3735-3739 行，摘录）
if (numberOfStretchedSections > 0 && lengthToStretch > 0) {
    int hintLengthForEveryStretchedSection = lengthToStretch / numberOfStretchedSections;
    stretchSectionLength = qMax(hintLengthForEveryStretchedSection, q->minimumSectionSize());
    pixelReminder = lengthToStretch % numberOfStretchedSections;
}
```

咱们把分配读完。非拉伸列的尺寸从可用长度里逐个扣掉，剩下长度除以拉伸列数得基准，每列下限守 minimumSectionSize，取余的零头在后面的循环里逐列加一像素（第 3769-3771 行）。三列分十像素，基准三像素，最前面的列多拿那一像素。零头不走四舍五入，排在前面的列吃加成。

出厂尺寸有两个来路。defaultSectionSize 跟样式走，横表头问 PM_HeaderDefaultSectionSizeHorizontal，竖表头问 Vertical（第 4033-4042 行），您调 setDefaultSectionSize 就立起自定义标志，resetDefaultSectionSize 把裁定权交还样式，出处第 1581-1601 行。minimumSectionSize 出厂 -1，真取值时横表头按字体最宽字符加两倍 PM_HeaderMargin，竖表头按字高加边距（第 1613-1621 行），所有调宽模式都守住下限。

## resizeSection 是唯一写入口，程序与拖动同一条路

您在代码里调 resizeSection，进来就被 qBound 夹在最小与最大尺寸之间（qheaderview.cpp 第 860-871 行）。列若正藏着，尺寸不进条目表，改存 hiddenSectionSize 映射（第 871-874 行），等它重见天日再放回去。末列若在被拉伸，尺寸记进 lastSectionSize（第 892-893 行），视口一变它接着吃剩余宽度。收尾 emit sectionResized（第 933 行），分组表头正是接它得知列宽变了，一接到就重绘（group_header_view.cpp 第 21-22、89-90 行）。

用户拖动没有旁门，mouseMoveEvent 里那段非级联分支咱们看一眼：

```cpp
// 拖动调宽的落点（qheaderview.cpp 第 2704-2707 行，摘录）
int delta = d->reverse() ? d->firstPos - pos : pos - d->firstPos;
int newsize = qBound(minimumSectionSize(), d->originalSize + delta, maximumSectionSize());
resizeSection(d->section, newsize);
```

走起来也有门槛，mousePressEvent 要命中列间 handle 且该列模式是 Interactive，才进 ResizeSection 状态（第 2643-2670 行），Fixed 连 handle 都不认。手势拿按下时记下的 originalSize 加位移，再夹一次上下限，最后还是调 resizeSection。所以您接管 sectionResized 重算分组时，分不清这次调用来自代码还是手势，也不需要分，两条路在同一个函数会合。

## 精度旋钮拧在表头，用在视图

ResizeToContents 的精度出厂一千，出处 qheaderview_p.h 第 87 行，51 篇改准过百万行的老说法。旋钮咱们拧过三个档，正数就是最多考虑多少行，0 只看可见区，-1 全量（qheaderview.cpp 第 1315-1331 行）。

有意思的地方笔者留到现在，表头自己不扫模型，算尺寸时它把问题转给所在视图的 sizeHintForColumn 或 sizeHintForRow（viewSectionSizeHint，第 4138-4144 行）。数行数的活落在 QTableView::sizeHintForColumn，循环里数到精度值就 break（qtableview.cpp 第 2499-2507 行起）。您重写视图的 sizeHintForColumn，旋钮就没了着力点，官方文档也写明了。

## 隐藏列的尺寸有记忆，存在一张映射里

头文件里的 hideSection 与 showSection，只是 inline 转手，出处 qheaderview.h 第 227-230 行，真活在 setSectionHidden。藏列那几行笔者抄在下面：

```cpp
// 藏列时把尺寸存起来（qheaderview.cpp 第 1023-1026 行，摘录）
int size = d->headerSectionSize(visual);
if (!d->hasAutoResizeSections())
    resizeSection(logicalIndex, 0);
d->hiddenSectionSize.insert(logicalIndex, size);
```

条目随后按 0 长计入总长（第 3754-3756 行），sectionSize 对隐藏列直接报 0（第 633-638 行），length() 因此不含隐藏列。showSection 时把存的尺寸放回去，查无此记录就用 defaultSectionSize 兜底，出处第 1033-1036 行。想问表头有没有藏列，sectionsHidden 查的就是映射空不空，出处第 1752-1757 行。51 讲过 QTableView 那批藏列函数全是转手到这层，模型侧的世界您去看，一个字都不改。

## 箭头是显示件，排序的接线在视图手里

showSortIndicator 出厂 false，出处 qheaderview_p.h 第 55 行。点击翻转的判定咱们翻 mouseReleaseEvent 看，门槛是 clickableSections 加按下同一列，并不检查箭头显示开关（qheaderview.cpp 第 2848-2851 行），翻转逻辑 flipSortIndicator 默认升序起跳，模型 headerData 里的 InitialSortOrderRole 能改每列的初值，出处第 3853-3883 行。

setSortIndicator 只改内部状态再 emit sortIndicatorChanged（第 1404-1438 行），传 -1 可以把箭头收掉。咱们看到接信号去排序的是视图，QTableViewPrivate::sortIndicatorChanged 就一句 model->sort，出处 qtableview.cpp 第 1020-1023 行，接线只在 setSortingEnabled(true) 时建立，出处第 2730-2745 行。

配套示例正好停在这个断层上。GroupHeaderView 自己开了箭头显示，出处 group_header_view.cpp 第 18 行，窗口却没调 setSortingEnabled，您点表头，箭头会翻，数据纹丝不动。补一句 tableView->setSortingEnabled(true)，接线一通，排序才接活。

## saveState 存下整套状态，恢复时还会代发一次排序信号

saveState 返回 QByteArray，里面是 QDataStream 序列化的私有数据，开头一个 VersionMarker 加版本号 0，出处 qheaderview.cpp 第 1808-1821 行。写进去的不止列宽，write 函数咱们数过（第 4199-4240 行），视觉映射、隐藏列的位图与尺寸记忆、总长、可拖可点、末列拉伸、两个计数器、默认与最小尺寸、对齐、全局调宽模式、每列条目、精度，全在。

restoreState 依次尝试 Qt_5_0 与 Qt_6_0 两种流版本，标记对不上直接返回 false（第 1827-1858 行）。有一处行为您得心里有数，恢复完成时它 emit 一次 sortIndicatorChanged（第 1849 行），视图若已接着排序线，恢复状态当场触发一次排序，想只还原外观的场合记得留意时序。

## offset 是表头自己的滚动刻度

表头也会滚，咱们从总长看起。length() 给内容方向的总长（qheaderview.cpp 第 489-497 行），offset 出厂 0（qheaderview_p.h 第 52 行），setOffset 用 viewport()->scroll 挪内容，出处 qheaderview.cpp 第 423-441 行，setOffsetToLastSection 拿总长减视口尺寸，出处第 475-485 行。滚动条联动在私有 setScrollOffset，逐项滚动模式拿 setOffsetToSectionPosition 对齐到列起点，出处第 4163-4173 行。

一个冷细节，拖动调宽顶到边缘触发自动滚动时，setOffset 会把鼠标光标反向顶回 handle 上（第 442-448 行），不然您拖着的列会从光标底下溜走。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过。QHeaderView 在 QtWidgets，应用链接 Qt6::Widgets 即可。

[Qt 文档 · QHeaderView](https://doc.qt.io/qt-6/qheaderview.html) -- 本篇主角，调宽模式、视觉映射与状态存取的定义处

[Qt 文档 · QTableView](https://doc.qt.io/qt-6/qtableview.html) -- setSortingEnabled 的接线与 sizeHintForColumn 的精度消费

[Qt 文档 · QSortFilterProxyModel](https://doc.qt.io/qt-6/qsortfilterproxymodel.html) -- 排序比较的定制入口，49 篇讲过 lessThan

[Qt 文档 · QDataStream](https://doc.qt.io/qt-6/qdatastream.html) -- saveState 字节数组的序列化格式

[Qt 文档 · QStyle](https://doc.qt.io/qt-6/qstyle.html) -- PM_HeaderDefaultSectionSize 系列像素度量的出处

---

表头的深水区咱们潜完了。下一站 [3.53 QColumnView 进阶](./53-qcolumnview-advanced.md)，条目视图家族的最后一位，浏览器式的分栏形态在那里等着。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/52-qheaderview-advanced/，可执行文件 52-qheaderview-advanced），四列两组的分组表头。做三个实验。拖动列宽，分组条跟着伸缩，sectionResized 到重绘的路径就在眼前。点表头，箭头翻了数据没动，补一句 setSortingEnabled(true) 再点，排序接活，断层自己补上了。把 Contact 组的 Email 列用 horizontalHeader()->hideSection(2) 藏起来，分组条缩到只剩 Phone 那一列宽，再把 saveState 的返回值存下来，hideSection 与 restoreState 走一个来回，尺寸记忆与状态恢复都能亲眼看到。

完成标准照旧。logical 与 visual 什么时候是同一个数，您说得出。三列 Stretch 分十像素各得多少，您算得开。程序调宽与用户拖动在哪里会合，您指得出。点表头翻箭头为什么数据不动，您讲得清。恢复状态为什么可能触发一次排序，您有数。
