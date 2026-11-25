---
title: "3.42 QSplitter 进阶"
description: "入门篇用 QSplitter 搭过左右分栏，进阶篇把分割器当尺寸谈判机对着 6.9.1 读实现，手柄与子件在私有链表里一一配对且 0 号手柄永远隐藏、createHandle 是造手柄的工厂虚函数、拖动链从手柄按下经 adjustPos 的 40 像素吸附阈值落到 doMove 与 setGeo 再发 splitterMoved、setStretchFactor 写的是子件自身 sizePolicy 的水平垂直伸展且没人设时全员按伸展分地盘权重是当前尺寸乘因子、setSizes 的数值当权重用多退少补 0 值可折叠就收没、collapsible 字段三态 Default 回落 childrenCollapsible、收没的件靠挪到负坐标保住手柄（hide 未被调用）、handleWidth 默认走 PM_SplitterWidth 且不超过 3 时热区外扩补足抓取带、opaqueResize 默认经 SH_Splitter_OpaqueResize 由 QCommonStyle 返回真、saveState 连方向与折叠开关一起存而 restoreState 只校验魔数和版本数量不校验、childEvent 自动收编散落子件但拒绝 QLayout、setOrientation 转置策略全家转、sizeHint 顺轴求和横轴取最大，配套示例在 examples/advanced/03-qtwidgets/42-qsplitter-advanced/"
---

# 现代Qt开发教程（进阶篇）3.42——QSplitter 进阶

[3.41 QStackedWidget 进阶](./41-qstackedwidget-advanced.md) 收尾时咱们把话递到拖杆分尺寸，本篇的 QSplitter 正是管分地盘的。入门篇您搭过左右分栏，addWidget、setSizes、saveState 都摸过，进阶篇把它当一台尺寸谈判机来读，手柄从哪造出来，拖动走哪条链，地盘按什么规则分，状态存了些什么。源稿把重心压在自定义手柄与最小宽度上，机制层几乎空白，还有几处断言与 6.9.1 对不上，本篇按源码重排，改准的地方随文给出行号。

本篇引用以 qsplitter.cpp 为主，类与属性的定义补 qsplitter.h，折叠三态的私有结构补 qsplitter_p.h，样式默认值补 qcommonstyle.cpp 与 qfusionstyle.cpp。配套示例的完整玩法咱们放在结语，正文引用文件名加行号。

## createHandle 是工厂，handle() 按件取左

QSplitterHandle 是公开导出类（qsplitter.h 第 97 行），构造函数摆在明面上谁都能调，官方推荐的路子却是子类化 QSplitter 重写 createHandle，类注释写得很清楚（qsplitter.cpp 第 50-76 行）。工厂本体一行，new QSplitterHandle(d->orient, this)，出处（第 1197-1201 行）。私有链表里每个条目都握着两样东西，一个子件加一根手柄（qsplitter_p.h 第 29-45 行），新件插进来时工厂被调，手柄对象名拼成 qt_splithandle_ 加件名，方便您在对象树里对号（qsplitter.cpp 第 844-847 行）。

手柄与子件同数，handle(index) 取的是 index 号件左侧或上方那根，0 号手柄永远隐藏，recalc 用 setHidden(first || widgetHidden) 落实（第 368-378 行），接口文档把配对规则写得很直白（第 1203-1213 行）。入门材料常把手柄数记成子件数减一，可见手柄确实少一根，对象一根不少，indexOf 认件也认手柄（第 1179-1188 行）。您拿 handle(1) 到手的正是 0 号与 1 号件之间那根。示例里 CustomSplitter 重写 createHandle 返回带防滑点的 CustomHandle，出处（custom_splitter.cpp 第 13-16 行），两根可见手柄全部换成自绘外观。

## 拖杆的一条链：按下、吸附、落位、发信号

拖动咱们从头跟到尾。按下时手柄记下偏移并置 pressed（第 272-280 行），move 时 opaqueResize 为真就直接 moveSplitter，为假只画橡皮筋 setRubberBand（第 254-267 行），松手时收掉橡皮筋，再补一次 moveSplitter（第 285-300 行）。手柄侧的 moveSplitter 做完从右到左语言的坐标映射，转调 QSplitter::moveSplitter，出处（第 160-166 行）。正戏在后者，adjustPos 调位置，doMove 向两侧推，setGeo 落几何，storeSizes 把新尺寸写回 sizer（第 532-538 行），末尾 emit splitterMoved(pos, index)（第 1423 行）。您挂槽同步状态栏，拿到的 pos 是吸附调整之后的值，别当鼠标原始位置用。

吸附值得笔者停半拍。adjustPos 里 Threshold 定在 40（第 631 行），拖出常规极值后，位移超过剩余宽度一半且越过 qMin(40, 宽度)，位置就整段跳到折叠边界，去 farMin 或 farMax（第 629-658 行）。您把手柄往边上甩，面板要么停在最小尺寸要么整个收没，中间不悬停，干脆的手感来自这 40 像素。

## setStretchFactor 落在子件的 sizePolicy 上

笔者把 setStretchFactor 的落点翻出来核过，它把水平垂直两个伸展同时写进子件的 sizePolicy（第 1750-1761 行），离开分割器照样在，存的是子件自己的属性，越界索引静默返回（第 1753-1754 行）。分配在 doResize 里做，手柄与子件交替进 QLayoutStruct 数组，qGeomCalc 一把算完（第 449-530 行）。分地盘的资格有个总开关，咱们看摘录。

```cpp
// stretch 资格的取舍（qsplitter.cpp 第 492-499 行）：
bool stretch = noStretchFactorsSet;
if (!stretch) {
    QSizePolicy p = s->widget->sizePolicy();
    int sf = orient == Qt::Horizontal ? p.horizontalStretch() : p.verticalStretch();
    stretch = (sf != 0);
}
if (stretch) {
    a[j].stretch = s->getWidgetSize(orient);
```

没人设 stretch 时开关为真，全员按伸展对待、expansive 全开，剩余空间按各自当前尺寸的权重分（第 457-465 行）。一旦有人设了，只有设了的件保有资格，没设的按 sizeHint 夹在 min 与 max 之间。权重还带乘法，getWidgetSize 里伸展因子大于 1 时 sizer 乘上因子（第 337-341 行），函数注释写明，有效伸展由初始尺寸乘 stretch 得来，出处（第 1737-1743 行）。所以 setStretchFactor(0, 1) 配 setStretchFactor(1, 3) 分出来的远超 “一比三”，两件的初始尺寸也压在秤上。最小尺寸不靠 stretch 托底，doResize 填表时用 qSmartMinSize（第 488 行），您给面板 setMinimumWidth(200)，自动分配不会给到线下，用户拖拽是另一套规则，归折叠一节交代。

## setSizes 给的是权重，0 值直接收没

setSizes 转 setSizes_helper(list, true)（第 1596-1600 行）。helper 逐件填 sizer，您看摘录。

```cpp
// 0 值的归宿（qsplitter.cpp 第 686-698 行）：
int smartMinSize = pick(qSmartMinSize(s->widget));

// Make sure that we reset the collapsed state.
if (s->sizer == 0) {
    if (collapsible(s) && smartMinSize > 0) {
        s->collapsed = true;
    } else {
        s->sizer = smartMinSize;
    }
} else {
    if (s->sizer < smartMinSize)
        s->sizer = smartMinSize;
}
```

0 值时件可折叠就置 collapsed 收没，不可折叠抬回 hint，低于 hint 的值同样抬回。数组多给的值被循环忽略，给少了后来件兜默认 0 落进折叠分支（第 679-699 行）。函数文档点明整体尺寸不动，多退少补按相对权重分（第 1575-1594 行），您设 {230, 230, 240}，窗口够宽时大致照单收，缩到五百像素就按 23:23:24 重新分，绝对值不作数。sizes() 读的是 s->rect 的实占尺寸（第 1559-1573 行），setSizes 之后立刻读回来，值可能已被 doResize 重分，您别拿它当回声用。

## 折叠三态：Default 回落 childrenCollapsible，收没的件被挪到负坐标

咱们把 collapsible 字段翻出来看，两位宽，Default 值就是 2（qsplitter_p.h 第 27、34-35 行），setCollapsible 只往里写 0 或 1（qsplitter.cpp 第 1043 行），索引越界给一句 qWarning 就返回（第 1039-1042 行）。取值时看到 Default 就回落到 childrenCollapsible（第 660-667 行），后者私有初值真（qsplitter_p.h 第 56 行），要全局关折叠，调 setChildrenCollapsible(false)，出处（qsplitter.cpp 第 1010-1020 行）。

拖拽绕过最小尺寸的机关在 doMove，允许折叠时 ws 直接归 0，不再夹回 qSmartMinSize（第 776-783 行），接口文档也写明，用户能把设了 minimumSize 的件拖到 0，出处（第 1022-1032 行）。您要面板有底又拖不动，setMinimumWidth 与 setCollapsible(index, false) 得两样一起设。收没的实现笔者多看了一眼，它不调 hide，把件挪到 (-w-1, -h-1)，注释写明为了保住手柄可见（第 732-740 行）。手柄在，面板随时拖得回来，对象树里它也始终是可见状态。

## handleWidth 的热区补偿：视觉细与抓取宽解耦

默认宽度走 PM_SplitterWidth 样式度量，私有初值 -1 表示没设过（第 1613-1621 行，qsplitter_p.h 第 56 行），具体几像素随平台与样式走，源码不写死。手柄的 resizeEvent 里有一段补偿，handleMargin 拿 (5-宽)/2 算，宽不超过 3 时 margin 大于 0 进 tiny mode，置 WA_MouseNoMask 并撑开 contentsMargins，让抓取区外扩压到相邻件的地盘（第 203-228 行），属性文档写明，宽设成 0 或 1 时，抓取区会向外多占几像素（第 1609-1611 行）。您给触摸屏做界面，视觉上一根细线也留着 4 到 5 像素的热区（源码注释原话 at least 4 or 5 pixels，宽 0 得 4、宽 1 得 5），两件事互不牵扯。

绘制走 CE_Splitter，悬停与按下经 State_MouseOver 和 State_Sunken 传给样式（第 305-323 行），状态翻转在 event() 里做，HoverEnter 置真、HoverLeave 置假，出处（第 233-249 行）。悬停事件要有 WA_Hover 才会来，Fusion 的 polish 名单明确收了 QSplitterHandle，出处（qfusionstyle.cpp 第 2977-2989 行），别的样式不一定置，您做悬停高亮就在子类构造里自己 setAttribute(Qt::WA_Hover, true)，事件与状态位全都现成。

## opaqueResize 默认随样式，false 时走橡皮筋

私有 opaque 初值真，opaqueResizeSet 初值假，取值时转问样式 SH_Splitter_OpaqueResize，两级出处（qsplitter_p.h 第 55-56 行，qsplitter.cpp 第 1459-1475 行）。QCommonStyle 对该提示一律返回真（qcommonstyle.cpp 第 5393-5395 行），6.9.1 的样式目录里也没有谁改写它，常见平台默认都是拖动实时重排。您调 setOpaqueResize(false) 后，拖动期间只画一条 QRubberBand 线（第 1300-1326 行），松手才一次性落位，面板重排昂贵的大界面改用它更省心。

## saveState 连方向一起存，restoreState 只认魔数和版本

saveState 用 QDataStream 依次写魔数 0xff、版本、sizer 列表、childrenCollapsible、handleWidth、opaqueResize、orientation、opaqueResizeSet（第 1654-1677 行），存的远超尺寸一栏。restoreState 的校验只有魔数与版本两道（第 1709-1712 行），子件数量根本不看。数量对不上时的走向在 setSizes_helper，保存三件恢复两件，多余的值被循环忽略，保存两件恢复三件，后来件兜默认 0，可折叠就整个收没（第 679-699 行），函数照样返回真。源稿说数量不一致时静默失败，6.9.1 里查无此事，您做可选面板的恢复逻辑，返回值护不住，得自己核对 count。

嵌套分割器的恢复次序咱们自己排，内层恢复完再动外层。依据是外层分地盘时拿的是子件现时的 sizer 与 sizeHint，内层没就位，外层就拿旧值谈，落位后再被内层顶一下。Qt 本身没规定顺序，排错的代价也只是一个回合的闪跳，咱们顺手排对就是。

## childEvent 收编散落子件，setOrientation 全家跟着转

拿 QSplitter 当 parent 直接 new 出来的子件，childEvent 会自动收编追加到队尾（第 1262-1273 行），注释标明兼容用途，新代码还是走 addWidget。子件是 QLayout 时直接 qWarning 顶回，分割器不接待布局（第 1267-1268 行）。件被删时条目同步移除并 recalc（第 1280-1290 行）。方向切换在 setOrientation，尺寸策略转置、逐根手柄转方向连带换光标、recalc 重排（第 973-991 行，光标在第 106-113 行），restoreState 里 orientation 也原样写回（第 1726-1727 行），存好的状态跨方向恢复不怕错位。咱们顺带把尺寸聚合记一笔，sizeHint 顺轴求和、横轴取最大（第 1489-1506 行），minimumSizeHint 把手柄的建议尺寸也算进去（第 1513-1540 行），面板越加越多，窗口的最小建议宽度跟着水涨船高。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，全部行为在 6.9.1 源码核对。QSplitter 与 QSplitterHandle 同在 QtWidgets，链接 Qt6::Widgets 即可。

[Qt 文档 · QSplitter](https://doc.qt.io/qt-6/qsplitter.html) -- 分割容器，setSizes 的权重语义与 saveState/restoreState 的定义处

[Qt 文档 · QSplitterHandle](https://doc.qt.io/qt-6/qsplitterhandle.html) -- 分割手柄，paintEvent 与鼠标事件的默认实现

[Qt 文档 · QSplitter::createHandle](https://doc.qt.io/qt-6/qsplitter.html#createHandle) -- 手柄工厂虚函数，自定义外观的官方入口

[Qt 文档 · QSplitter::opaqueResize](https://doc.qt.io/qt-6/qsplitter.html#opaqueResize-prop) -- 拖动是否实时重排的属性，默认值交给样式提示

[Qt 文档 · QSizePolicy](https://doc.qt.io/qt-6/qsizepolicy.html) -- 伸展因子的存放地，setStretchFactor 的落点

---

分地盘的容器读穿，谈判规则全在 qGeomCalc 一张表里，41 篇递过来的话到这里对上了号。下一程收进一个抽屉里翻页，[3.43 QToolBox 进阶](./43-qtoolbox-advanced.md)，标签列配折叠页的机制咱们接着看。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/42-qsplitter-advanced/，可执行文件 42-qsplitter-advanced）。三块彩色面板的最小宽度各不相同（main.cpp 第 76-78 行），setSizes 给了 230、230、240 的初值（第 81 行），您可以拖右边的蓝面板试试，最小宽度 80 挡不住它整个收没，折叠默认开着。防滑点的半径、间距、数量在 custom_handle.cpp 第 50-52 行，改三个数换一版手感。想看热区补偿，给 CustomSplitter 补一句 setHandleWidth(1)，视觉上近乎消失，照样拖得动。

完成标准照旧。handle(1) 在哪两件之间，您说得清 0 号手柄去了哪。拖到边上的干脆吸附从哪来，您指得出 Threshold 在哪一行。setStretchFactor(0, 1) 配 (1, 3) 实际怎么分，您按 getWidgetSize 的乘法推得出来。三件的字节数组恢复进两件的分割器会发生什么，您答得出多余值的归宿。
