---
title: "3.71 QMdiArea 进阶"
description: "进阶篇把 QMdiArea 的机制对到源码行上：addSubWindow 给普通 widget 造一只 QMdiSubWindow 当壳并带上 WA_DeleteOnClose（qmdiarea.cpp 第 1963-1965 行），壳的 closeEvent 拿 baseWidget->close() 做一票否决（qmdisubwindow.cpp 第 2987-2996 行），setViewMode 切换不重建子窗口，只建删标签栏并动活动窗口的最大化（第 1521-1594 行），TabbedView 默认把活动窗口拉满视口（第 1561-1562 行），tabPosition 默认 North、tabsClosable 与 tabsMovable 默认 false（第 629-637 行），标签叉、鼠标中键、closeActiveSubWindow 全汇到 subWindow->close()（第 754-761、533-547、1829-1834 行），挪标签会改写 CreationOrder（第 765-771 行），subWindowActivated 的 nullptr 出自 resetActiveWindow（第 1066、1074 行），tile 前会收集最小尺寸并把排布区域扩到位（第 912-913、920 行），级联的水平步进写死 10 像素、窗口尺寸取 sizeHint（第 296、318-319 行），示例在 examples/advanced/03-qtwidgets/71-qmdiarea-advanced/"
---

# 现代Qt开发教程（进阶篇）3.71——QMdiArea 进阶

widgets 卷走到了容器类的地带，QMdiArea 是多文档界面的容器，Visual Studio、GIMP 这类一屏多文档的应用背后都有它。它跟普通容器的差别从一件小事就能看出来，您塞给它的 widget，传入时会被包进一只 QMdiSubWindow，之后才挂进区域。笔者把 Qt 6.9.1 的 QMdiArea 源码（qmdiarea.cpp）从头读到尾，qmdisubwindow.cpp 一并翻过，本篇从包装与所有权讲起，一路讲到两种视图的切换、标签页的关闭路径，落到 cascade 与 tile 的实际算法。网上与旧稿流传的几处说法，笔者逐一拿行号对过，对不上的当场改讲。

引用文件咱们交代清楚。笔者主读 qmdiarea.cpp（src/widgets/widgets），枚举定义在 qmdiarea.h，排布器与私有类声明在 qmdiarea_p.h，关闭行为还要借 qmdisubwindow.cpp 对证。入门篇走过的 addSubWindow 基本调用与 subWindowList 遍历，本篇不重讲，配套示例是一只带窗口菜单的多文档主窗口，咱们把完整玩法放结语。

## addSubWindow：您的 widget 多了一层壳

咱们从进场那句差别讲起。addSubWindow 拿到您的 widget 后，头一件事是问它自己是不是 QMdiSubWindow（qmdiarea.cpp 第 1952 行），是，就只把它的父对象改成 viewport（第 1960 行），文档注释写明，父对象落在 viewport 上（第 1927-1929 行）。不是，就轮到造壳。

```cpp
child = new QMdiSubWindow(viewport(), windowFlags);
child->setAttribute(Qt::WA_DeleteOnClose);
child->setWidget(widget);
```

（qmdiarea.cpp 第 1963-1965 行）三行各管一段。壳当场带上 WA_DeleteOnClose，setWidget 再把您的 widget 塞进壳里，内部走的是 setParent(this)（qmdisubwindow.cpp 第 2276 行）。所以所有权链是三层的，viewport 拥有壳，壳拥有您的 widget。您要是把 widget 指针存下来，打算留着自己删，就要当心了，壳被关闭删除的那天，您的 widget 作为孩子一起走，外部再碰这个指针就是悬空。

自己造 QMdiSubWindow 传进去，咱们接着看会是怎样。造壳分支不给补 WA_DeleteOnClose，文档把话说明白，想要关掉即删，就得您自己设属性，不设的话窗口关掉只是隐藏，区域也不会激活下一个子窗口（第 1933-1936 行）。removeSubWindow 则从不 delete 任何东西，传壳就把壳摘下来，传内部 widget 就只清空壳，壳照样留在区域里（第 1979-1982 行）。您把两个方向合起来看，这批窗口的生命周期怎么管，传入时就得想好。

关闭链上还有一票否决权，笔者把它单独拎出来。QMdiSubWindow::closeEvent 的头几行就调 baseWidget->close()，问您 widget 自己的 closeEvent 同不同意，不同意，整次关闭被 ignore 掉（qmdisubwindow.cpp 第 2987-2996 行）。您的未保存确认对话框写在 widget 的 closeEvent 里，那么从标签页的叉到标题栏的关闭钮，全都得过这一道，验证逻辑不会被绕开。

## setViewMode：切换不重建，动的是标签栏和最大化

构造私有对象时的一排默认值，咱们直接从源码抄给您，viewMode 是 SubWindowView（qmdiarea.cpp 第 629 行），documentMode 是 false，tabsClosable 与 tabsMovable 也是 false（第 631-633 行），tabShape 是 Rounded，tabPosition 是 North（第 636-637 行）。两套属性是分家的，标签相关那批在 SubWindowView 下没有视觉对应物，您随时可设，值会存着等 TabbedView 用。

切换的实现咱们跟着读一遍，在 QMdiAreaPrivate::setViewMode 里，函数范围第 1521-1594 行。切到 TabbedView 时，Qt 造一只 QMdiAreaTabBar（第 1533 行），把现有子窗口挨个 addTab 进去（第 1547-1549 行），再把当前活动子窗口 showNormal 一次，紧接着 showMaximized，两步落在第 1555-1556 与 1561-1562 行。切回 SubWindowView，就是删掉标签栏（第 1579 行），活动窗口若处于最大化，再 showNormal 一次。

这里笔者要当面驳回一条流传说法，称切换时子窗口的几何被存进内部状态、切回来再恢复。咱们把这个函数从头读到尾，里面没有任何存取几何的代码，子窗口的位置从头到尾没人碰。布局不丢的原因，用不着存副本，因为从头到尾没人去改它，真正被动的只有活动窗口的最大化状态。标签页模式的视觉来源也是那次最大化，活动窗口被拉满整个视口，其余窗口压在底下，看起来就像一次只剩一页。

那次最大化是有条件的。第 1561 行的判断查 DontMaximizeSubWindowOnActivation 选项，AreaOption 枚举里只有一名成员（qmdiarea.h 第 36-38 行），类文档写明它的用途，活动子窗口被最大化时，默认会把下一个激活的子窗口也最大化，设此选项关掉联动（qmdiarea.cpp 第 75-83 行）。您想让每个子窗口在标签页里保持自己的尺寸，就得设它。

新窗口的摆位，咱们顺带也查了。appendChild 给新来的壳找位置，用的是 MinOverlapPlacer，专门挑与现有窗口重叠最少的地方（第 803-805 行）。所以另一条说法，称标签页模式下新加的窗口切回子窗口视图会堆在原点与其他窗口重叠，笔者在源码里找不到支持，找位逻辑不分视图模式。标签文字另有个小细节，空标题会被换成 `(Untitled)`，标题里的 [*] 占位在窗口处于 modified 状态时换成星号（tabTextFor，第 223-236 行）。

## 标签页的叉与挪动：全汇到 subWindow->close()

setTabsClosable 与 setTabsMovable 的实现，咱们一眼就能看完，改个成员变量再 refreshTabBar 完事（第 2164-2196 行），它们只管标签栏长什么样，不碰关闭语义。您点中标签上的叉之后，tabBar 的 tabCloseRequested 被接到 _q_closeTab（第 1573 行），槽的实现就一行。

```cpp
QMdiSubWindow *subWindow = childWindows.at(index);
Q_ASSERT(subWindow);
subWindow->close();
```

（qmdiarea.cpp 第 759-761 行）叉关掉的就是子窗口本身，标签页并没有独立对象，前文 addSubWindow 一节讲的 closeEvent 否决与 WA_DeleteOnClose，在这里全部适用。closeActiveSubWindow 是同一条路的另一入口，实现是 active->close()（第 1829-1834 行）。closeAllSubWindows 咱们也看了一眼，它挨个发关闭事件，忽略事件的窗口留在原地，文档写得明白（第 1853-1859 行）。

鼠标的旁路咱们也数了。中键点标签，直接 close 对应子窗口（QMdiAreaTabBar::mousePressEvent，第 533-547 行），右键点标签，弹出的是子窗口自带的系统菜单（contextMenuEvent，第 553 行起），当前窗口处于最大化时，菜单里的移动与缩放两项会被藏掉，免得点了没处使（第 571-575 行）。

tabsMovable 打开后，您拖着标签换位，_q_moveTab 收到 tabMoved，就执行 childWindows.move(from, to)（第 765-771 行）。视觉换了序，subWindowList 的 CreationOrder 也跟着变了，因为那个列表就是 childWindows 本身。示例的 Window 菜单按 subWindowList() 生成，咱们拖着标签换完序，菜单顺序要等到下一次窗口激活才刷新，因为示例只在 subWindowActivated 与新建文档处重建菜单，挪标签不发激活信号。

## subWindowActivated：nullptr 是常规一态

信号的发射点笔者数全了，一共三处。正常换活动窗口在 emitWindowActivated 尾部（第 1049 行），同一个窗口反复激活，会被函数开头的判断挡掉，不重发（第 1007-1008 行）。nullptr 出现在 resetActiveWindow 的两个分支（第 1066、1074 行），类文档写明参数为空时，区域刚失去最后一个活动窗口（第 64-69 行）。最后一个子窗口关闭的路径也在，updateActiveWindow 一看 childWindows 空了，就调 resetActiveWindow（第 1099-1102 行）。您在槽里不判空就调 sub->widget()，接下来就是解引用空指针，崩溃点还离事发地很远，查起来费劲，槽函数开头判一下空，是您能做的最低配置。

参数给的是壳，您想拿里面的内容，得用 sub->widget()，这是常见的混淆点。setActiveSubWindow(nullptr) 的语义，咱们也顺便对清，传空指针等于撤销当前激活（第 1803-1809 行），区域就此没有活动窗口。

还有一处与最大化联动的细节。emitWindowActivated 里 showActiveWindowMaximized 标志为真时，新激活的窗口自动 showMaximized（第 1015-1020 行），注释写明条件，DontMaximizeSubWindowOnActivation 未设，且上一个活动窗口是最大化（第 1015 行）。标签页模式下咱们切标签，每一页都铺满视口，源头就在这里。

## cascade 与 tile：排布算法摊在源码里

两个槽都是纯转发，tileSubWindows 与 cascadeSubWindows 各自造一个排布器交给 rearrange，函数体一行逻辑都没有（第 2471-2490 行）。活分两层，咱们从外层讲起。

外层 rearrange 做筛选和预处理（第 881-927 行）。看不见的窗口跳过，最小化的跳过（第 902-903、908-909 行），最大化的被 showNormal 摊平了再排（第 910-911 行）。tile 用的列表还要倒序，您激活得越早，排得越靠前（第 898 行）。有一处笔者要为 tile 正名，流传说法称它无视子窗口的 minimumSize，硬把窗口压进格子。源码正相反，rearrange 一路收集所有参与窗口的最小尺寸，expandedTo(child->minimumSize()) 之后，再 expandedTo(internalMinimumSize)（第 912-913 行），internalMinimumSize 的出处咱们也查了，您没设最小尺寸时，它取 minimumSizeHint（qmdisubwindow.cpp 第 1105-1107 行）。随后 tile 的排布区域交给 resizeToMinimumTileSize，扩到容得下这些最小值（第 920 行，实现从第 1223 行起），注释写明区域放不下就给滚动条。子窗口不会被压破最小尺寸，放不下时区域被撑大或亮出滚动条，空间紧张转嫁给了滚动区。

内层就是排布器本身。RegularTiler 拿列数开方取上整，ncols = qCeil(qSqrt(n))，行数向上补齐（第 246-249 行），每格宽高是排布区域除以行列数的整数商（第 250-252 行），格子几何直接 setGeometry 写进窗口（第 276-278 行），首行贴顶，末行贴底，末列贴右，凑不整的余数摊在边上的格里。五个窗口排出三列两行的格局，缺的一格由首行最左列拉高占两行补齐，不留空格，这就是您在屏幕上看到的样子。

SimpleCascader 的参数，咱们在源码里看得一清二楚（第 286-308 行）。水平步进 dx 写死 10 像素（第 296 行），垂直步进拿标题栏高度的样式度量与字体高度合成（第 300-303 行），底部留白 50，右侧留白 100（第 292-295 行），窗口尺寸一律重设为它自己的 sizeHint（第 318-319 行）。两条流传说法到这里一起站不住了，一条称级联偏移 20 到 30 像素由平台风格决定，实际水平方向写死了 10 像素，风格只参与垂直步进，另一条称级联把窗口大小重置为区域的某个比例，源码给的答案是 sizeHint。

排布函数本身不看视图模式，几何照写，这是咱们读完源码的判断。但标签页模式下活动窗口默认被拉满视口，下次切标签又会被重新拉满（第 1015-1020 行），排好的布局站不住。示例因此在 Cascade 与 Tile 之前，都把视图切回 SubWindowView（widget.cpp 第 90、97 行），这是稳妥做法。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 Qt 6.9.1 源码核对过。

[Qt 文档 · QMdiArea](https://doc.qt.io/qt-6/qmdiarea.html) -- MDI 区域控件，ViewMode、AreaOption 与排布函数的定义处

[Qt 文档 · QMdiSubWindow](https://doc.qt.io/qt-6/qmdisubwindow.html) -- MDI 子窗口类，closeEvent 与最小尺寸的语义出处

[Qt 文档 · QTabBar](https://doc.qt.io/qt-6/qtabbar.html) -- 标签栏控件，TabbedView 模式内部的承载者

---

widgets 卷的容器地带走到收尾，下一站 [3.72 QPrinter 进阶](./72-qprinter-advanced.md)，打印设备与输出格式的控制在那边等咱们。

想动手的您把示例跑起来（examples/advanced/03-qtwidgets/71-qmdiarea-advanced/，可执行文件 71-qmdiarea-advanced）。窗口起步就是 TabbedView（widget.cpp 第 37 行）。您点几轮 New Document 造出三四个文档，标签能拖着换位，叉能关页，关掉一个后 Window 菜单的列表跟着少一项，因为示例把菜单刷新接在 subWindowActivated 上。切到 MDI View 后点 Cascade，窗口按各自的 sizeHint 摆开，横向一档只错 10 像素，标题栏一档错一行，比常见的直觉要挤。再点 Tile，窗口排成两列网格，您把主窗口压小，格子小到放不下最小尺寸时出的是滚动条，不是压扁的控件，这正是 resizeToMinimumTileSize 在起作用。

完成标准照旧。addSubWindow 给普通 widget 造的壳带不带 WA_DeleteOnClose，您说得出。切换视图时哪些东西被动了，您数得清。标签叉与中键关闭汇到哪一行，您答得准。tile 会不会压破最小尺寸、级联的水平步进是多少像素，您讲得明。
