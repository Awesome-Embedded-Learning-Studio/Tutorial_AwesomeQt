---
title: "3.40 QTabBar 进阶"
description: "入门篇用 QTabBar 配 QStackedWidget 搭过独立标签栏，进阶篇脱离 QTabWidget 单独读标签栏本身：selectionBehaviorOnRemove 默认 SelectRightTab 保位、SelectPreviousTab 靠每个标签自记的 lastTab 回指、没有全局历史栈、blockSignals 拦不住函数体内的赋值，拖拽重排跟手的是 QMovableTabWidget 截图、拖过相邻标签一半宽高才经 slide 调公开 moveTab、tabMoved 在途中逐次发出，setTabsClosable 对关闭侧被占的标签跳过不发内置按钮、地盘之争只往一个方向使劲，usesScrollButtons 关掉后 minimumSizeHint 变成全部标签 minRect 之和、滚轮归 SH_TabBar_AllowWheelScrolling 另行决定、普通滚轮切换标签，expanding 默认 true 让标签摊满整行、关掉才封宽，shape 只折算造型、被裁标签画 tear 提示、QSS 有 ::tab ::scroller ::tear ::close-button 四个伪元素，配套示例在 examples/advanced/03-qtwidgets/40-qtabbar-advanced/"
---

# 现代Qt开发教程（进阶篇）3.40——QTabBar 进阶

[3.39 QTabWidget 进阶](./39-qtabwidget-advanced.md) 收尾时咱们约好单独来会标签栏本身。QTabBar 脱了那层壳，删掉标签后谁上位、拖拽换位的时机、溢出往哪走，全要咱们自己拿主意，机制也就值得逐段读码。源稿的硬伤比 39 那篇还多几处，枚举名整组写错、默认档说反、setTabsClosable 的冲突方向说反、滚轮行为安错了开关，正文一律按 6.9.1 源码改准。

本篇引用以 qtabbar.cpp 为主，枚举与默认值落在 qtabbar.h、qtabbar_p.h，样式侧补 qcommonstyle.cpp 与 qstylesheetstyle.cpp。配套示例的完整玩法咱们放在结语里，正文引用文件名加行号。

## 删掉当前标签后谁上位，答案记在每个标签自己身上

咱们从 removeTab 删到当前标签的情形读起，私有数据里走一个 switch（qtabbar.cpp 第 1050-1064 行）。默认档就埋着头文件里，qtabbar_p.h 第 80 行写明 `selectionBehaviorOnRemove = QTabBar::SelectRightTab`，官方文档的同名属性说明也是原话。SelectRightTab 做的事是保位，`newIndex = qBound(d->firstVisible, index, d->lastVisible)`，把原索引钳在可见范围里按住不动（第 1057-1059 行）。

咱们拿一道调试题把语义验一遍，代码摆在下面。

```cpp
tabBar->setSelectionBehaviorOnRemove(QTabBar::SelectRightTab); // 默认档
// 标签顺序: 0, 1, 2, 3, 4，当前选中 2
tabBar->removeTab(2);
// 标签顺序: 0, 1, 3, 4，currentIndex 仍是 2，站进去的是原来的 3 号
```

removeTab(2) 之后选中的还是 2 号位，站进去的是原来的 3 号标签，因为后面的标签前移补位了。您想要原来的 1 号上位，要用 SelectLeftTab，它拿 index 减一去钳（第 1060-1062 行）。枚举名里的 Right 容易读成“选中原来右边那个”，保位与前移之后的结果恰好同一回事，您记保位就好。

SelectPreviousTab 走另一条路，`newIndex = removedTab->lastTab`（第 1032 行）。这个 lastTab 值得笔者单独停下看一眼，它没有全局的访问历史栈，是每个标签各自记一句“我上位之前谁在位”，setCurrentIndex 的函数体里写着 `tab->lastTab = oldIndex`（第 1425 行）。源稿说 blockSignals 加 setCurrentIndex 做静默切换不会记进历史，读码就知道拦不住，信号开关只管 emit，函数体内的赋值照常执行，lastTab 一样更新。目标无效或不可见时还有兜底，Q_FALLTHROUGH 落到保位分支（第 1051-1056 行）。拖拽换位时 lastTab 也跟着重映射，moveTab 里逐个标签过 calculateNewPosition（第 2065-2068 行），您拖完再删，“上一个”仍然指向拖后位置上的那个标签。

removeTab 还顺手把两侧控件 hide 加 deleteLater（第 1021-1031 行），您挂在标签上的自定义件随标签一起销毁，别在外面留着指针继续用。示例设了 SelectPreviousTab，关掉当前标签，界面回到上次访问的那页（drag_reorder_tab_bar.cpp 第 34 行）。

插入与删除对索引的扰动咱们一并记下。insertTab 插到当前标签前面时，currentIndex 抬一格（第 971-972 行），各标签的 lastTab 只要落在插入点之后，就跟着抬（第 990-992 行）。删到一根不剩，removeTab 直接 emit currentChanged(-1)（第 1074、1077 行），您在槽里要经得起负值。拖拽进行中删标签另有一道保险，removeTab 一开头就调 moveTabFinished，把拖拽收掉（第 1015-1016 行）。

## 拖拽重排：跟手的是一张截图，过半才换一次位

movable 打开后按下的链路 39 讲过，咱们补半句 39 没讲的。按下那一下选不选中听样式提示 SH_TabBar_SelectMouseType，按下即选中是默认，样式答 MouseButtonRelease 时按下只重绘，松手才切换（qtabbar.cpp 第 2147-2152、2338-2345 行）。

咱们接着读 mouseMoveEvent。拖动位移的曼哈顿长度超过 QApplication::startDragDistance，dragInProgress 才置位（第 2170-2177 行），随后 setupMovableTab 把标签画成一张 pixmap 交给跟随控件，挪动的是截图，标签本体按兵不动（第 2234-2277 行）。跟随控件叫 QMovableTabWidget，声明在私有头文件里，动画收尾时回调 moveTabFinished（qtabbar_p.h 第 38、155-171 行，qtabbar.cpp 第 2926 行）。

真换位的门槛在重叠过半。您拖着的标签矩形压到相邻标签上，超过那个标签宽度或高度的一半（needsToBeOver）才调私有 slide（第 2210-2218 行），slide 转手调公开的 moveTab，再给换位的标签挂一段动画（第 2095-2112 行）。

moveTab 是索引重排的正主，咱们把它的顺序摆一遍。tabList.move 挪数据结构，lastTab、currentIndex、pressedIndex 全部过 calculateNewPosition 重映射（第 2060-2072 行），拖拽进行中连 dragStartPosition 都按位移差修正，好让下一次过半判定不失准（第 2074-2085 行），随后 emit tabMoved（第 2089 行），currentIndex 变了再补一个 currentChanged，末尾还有 tabLayoutChange（第 2089-2093 行）。

所以 tabMoved 在拖拽途中逐次发出，每过半程发一次。槽里同步 QStackedWidget 的写法是老三样，widget(from) 取页面，removeWidget 加 insertWidget 放回 to，再把 currentIndex 对齐标签栏，咱们照示例摘出来看。

```cpp
connect(m_tabBar, &QTabBar::tabMoved,
        this, &DragReorderTabBar::syncStackedWidget);
// 槽内三步（drag_reorder_tab_bar.cpp 第 193-203 行）：
QWidget *page = m_stackedWidget->widget(from);
m_stackedWidget->removeWidget(page);
m_stackedWidget->insertWidget(to, page);
```

from 与 to 是 moveTab 的实参，逐次相邻交换时它俩每回只差一位。顺序只活在内存的 tabList 里，重启即丢，39 讲过同一个事实，写进 QSettings 的活儿仍归您。

## setTabButton 的地盘之争只往一个方向使劲

内置关闭按钮落在哪个侧别由样式提示 SH_TabBar_CloseButtonPosition 定，39 讲过，本篇补方向。setTabsClosable(true) 遍历已有标签时会查关闭侧有没有人，被占就 continue 跳过，内置按钮不发给您占过位的标签，您的控件原样保留（qtabbar.cpp 第 2578-2586 行）。源稿说谁后调谁覆盖，读码只有单方向成立，另一个方向压根没有覆盖这回事。

挤压发生在您主动动手的那侧。setTabButton 往已占的侧别塞新件，旧件只被 hide（第 2804-2812 行），39 讲过 closeTab 靠 sender 与两侧控件逐一比对找索引，按钮一藏就认不到，tabCloseRequested 也就停了。

新插的标签单独算，closeButtonOnTabs 开着时 insertTab 当场给它配一个内置 CloseButton（第 979-986 行），与您早前在别的标签上动过的手脚互不相干。稳妥的路子依旧是自定义件放 LeftSide，关闭侧留给内置按钮，示例的绿色状态点走的就是这一侧，出处（drag_reorder_tab_bar.cpp 第 158-168 行）。

子类化的人还有两个现成的挂点，tabInserted 与 tabRemoved 是 protected 虚函数，默认空实现，insertTab 末尾调前者，removeTab 末尾调后者（qtabbar.h 第 161 行，qtabbar.cpp 第 999、1094 行）。您想在标签增删时同步外部数据结构，覆写它们比连接信号更贴身，参数就是现成的索引。

## 溢出策略：expanding 摊开地盘，关掉滚动按钮改谈最小尺寸

独立使用 QTabBar 时您会看到标签自动摊满整行，来源是 expanding 默认 true，layoutTabs 的源码注释把这明写成 feature，标签会展开占满空位（qtabbar.cpp 第 433-441 行）。expanding 关掉后每个标签的 maximumSize 被封在 sizeHint（第 466-467、497-498 行），多余空位交给前后两个空槽吸收，分法听样式提示 SH_TabBar_Alignment 的，出处（第 425-430 行）。39 讲过 documentMode 会拨 expanding，独立使用时就您自己拨。

usesScrollButtons 的默认值问 SH_TabBar_PreferNoArrows（第 397-398 行），开着时 QTabWidget 给标签栏尺寸封顶 200 像素，那套在 39 讲过，挂链不重讲。本篇补关掉之后的几何后果，笔者把 minimumSizeHint 那段分支读了一遍，关掉按钮时它直接变成全部可见标签 minRect 之和（第 1508-1515 行），布局谈判里整条栏索要的地盘随标签数增长，给不够就裁切，被裁的标签由滚动按钮带回来。滚动按钮开着时同一函数只答两个按钮宽加 75 像素（第 1516-1519 行），两相对照，开关拨的其实是尺寸谈判的底牌。

滚轮与这个属性无关。咱们单看 wheelEvent，滚轮事件切不切标签由样式提示 SH_TabBar_AllowWheelScrolling 决定（第 2380-2383 行），QCommonStyle 默认允许（qcommonstyle.cpp 第 5415-5418 行）。普通滚轮走 setCurrentNextEnabledIndex 切换当前标签（第 2420-2433 行），只有带像素级滚动的触控板才会滚动标签栏（第 2388-2414 行）。源稿把关掉按钮说成改用滚轮滚动，两个行为根本不在一个开关上。

同属拖放阵营的还有 changeCurrentOnDrag 属性，外部数据拖过标签栏时自动切换当前页，两个拖放事件处理里都有它的检查，出处（第 1754、1758 行）。文档注释提醒，要配 acceptDrops 才生效（第 2753-2767 行）。您做接收文件拖放的面板时用得上它。

elideMode 默认问样式拿 ElideNone，39 讲过。本篇补一句量尺的内部，minimumTabSizeHint 会把标签文字换成极限省略形态再量，ElideRight 时留头两个字加省略号，ElideMiddle 留首尾各一个（qtabbar.cpp 第 1523-1546、1551-1560 行），所以最小宽度天生跟着您选的省略方位走。示例显式设了 Qt::ElideRight，超长标题右侧出省略号（drag_reorder_tab_bar.cpp 第 36 行）。

## shape 只折算造型，绘制链上有 tear 补台

咱们看 setShape 的两步，存值加 refresh（qtabbar.cpp 第 879-886 行），四个方向各配圆角与三角两款，共八种造型。绘制的活儿几乎都在样式侧，paintEvent 里 drawBase 开着才画 PE_FrameTabBarBase 底座（第 1828 行），每个标签逐个走 CE_TabBarTab。

选中标签压轴画在最上层，拖拽进行中它跟着 pressedIndex 走（第 1818-1819 行），压轴的那段绘制在同处（第 1886-1915 行）。被滚动按钮遮去一截的标签记成 cutLeft 或 cutRight，滚动按钮可见且确实有标签被裁时，画 PE_IndicatorTabTearLeft 或 Right 的撕裂提示（第 1866-1871、1916-1926 行），您看到标签边缘那道撕痕就是它。

滚动按钮自身若是半透明，会透出底下的标签，paintEvent 专门裁掉按钮区域防穿透，注释挂着 QTBUG-50866（第 1830-1842 行），咱们给按钮写半透明 QSS 时不用担心叠影。

QSS 的可达面也宽，伪元素从 ::tab 到 ::scroller、::tear、::close-button，在样式表引擎里各有专位（qstylesheetstyle.cpp 第 208-241 行），CE_TabBarTab 同样被引擎接管（第 4441 行），您想换造型不必子类化。autoHide 在标签数不超过一时整栏藏掉，39 讲过，挂链。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，QTabBar 在 QtWidgets 模块，链接 Qt6::Widgets 即可。

[Qt 文档 · QTabBar](https://doc.qt.io/qt-6/qtabbar.html) -- 标签栏控件，selectionBehaviorOnRemove、movable、expanding、usesScrollButtons 等属性定义处

[Qt 文档 · QTabBar::SelectionBehavior](https://doc.qt.io/qt-6/qtabbar.html#SelectionBehavior-enum) -- 删除当前标签后的自动选择策略枚举，默认值 SelectRightTab

[Qt 文档 · QTabWidget](https://doc.qt.io/qt-6/qtabwidget.html) -- 标签页控件，与 QTabBar 的分工在 3.39 讲过

[Qt 文档 · QStyle](https://doc.qt.io/qt-6/qstyle.html) -- 样式基类，SH_TabBar_CloseButtonPosition 与 SH_TabBar_AllowWheelScrolling 等提示的定义处

[Qt 文档 · QStackedWidget](https://doc.qt.io/qt-6/qstackedwidget.html) -- 堆叠页面容器，tabMoved 槽里同步页面顺序的对手方

---

标签栏到这就读穿了，下一程回到页面容器那半边，[3.41 QStackedWidget 进阶](./41-qstackedwidget-advanced.md)，堆叠页与标签栏怎么在各写各的代码里保持同构，咱们接着看。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/40-qtabbar-advanced/，可执行文件 40-qtabbar-advanced），入口在 main.cpp 第 13-21 行。构造里一口气设五个，从 setMovable 到 ElideRight 各就各位，出处（drag_reorder_tab_bar.cpp 第 32-36 行）。拖一下标签，底部信息条跟着 tabMoved 逐次跳动，关掉当前标签，看是否回到上次访问的那页（第 209-222 行）。第 82-118 行还有一段手写的鼠标事件演示，注释写明 setMovable 已内置同一套逻辑，您可以对照着看 moveTab 的调用方式，只是那段代码在父件里收不到栏内鼠标事件，实际跑不到，当阅读材料看。

完成标准照旧。removeTab(2) 之后 currentIndex 为什么还是 2、指向谁，您指得出那几行代码。拖拽途中 tabMoved 发几次、每次的 from 与 to 是什么，您按 needsToBeOver 推得出来。setTabsClosable(true) 遇到关闭侧被占的标签，内置按钮发不发，您答得出 continue 在哪一行。关掉 usesScrollButtons 后标签栏的 minimumSizeHint 怎么变，您说得出来。
