---
title: "3.39 QTabWidget 进阶"
description: "入门篇用 addTab、removeTab、setTabPosition、currentChanged 把 QTabWidget 用了起来，进阶篇对着 6.9.1 源码把壳里的机制逐一看清：私有 init 建好 qt_tabwidget_tabbar 与 qt_tabwidget_stackedwidget 两个对象、连接全收在受保护的 setTabBar 里，QTabBar 的 currentChanged 进 QTabWidgetPrivate::showTab、堆叠页切完才轮到 QTabWidget 自己的 currentChanged，removeTab 由 QStackedWidget 的 widgetRemoved 回流驱动、插入才是双写、标签文字里的 & 被抓成快捷键，关闭按钮是 CloseButton 内置件、画的是 PE_IndicatorTabClose、侧别由 SH_TabBar_CloseButtonPosition 决定且 QSS 能改、点击只发 tabCloseRequested、setTabButton 同侧会把它挤到隐藏、关掉 tabsClosable 时关闭侧控件被 deleteLater，拖拽重排整套在 QTabBar 的鼠标事件里、tabMoved 在 QSignalBlocker 下重排堆叠页，usesScrollButtons 开着时 sizeHint 给标签栏封顶 200 像素、elideMode 默认问样式拿 ElideNone，setTabPosition 只折算成 shape、竖排文字旋转在 common style 的 CE_TabBarTabLabel，documentMode 连拨 setExpanding 与 setDrawBase、外框不画，cornerWidget 挂在本体且只保证 North 与 South，tabBarAutohide 在标签数不超过 1 时藏栏，setIconSize 是公开属性，配套示例在 examples/advanced/03-qtwidgets/39-qtabwidget-advanced/"
---

# 现代Qt开发教程（进阶篇）3.39——QTabWidget 进阶

[3.38 QGroupBox 进阶](./38-qgroupbox-advanced.md) 收尾时咱们约好来翻页签。QTabWidget 把一组组页面叠进同一块地盘，每层只露一个标题，用起来省心，壳底下的联动却值得咱们读一遍代码，看看页面堆叠与标签栏怎么接，关闭按钮为什么点了没动静，拖拽重排归谁管，标签多了往哪溢出。源稿有两处硬伤，iconSize 被说成没暴露、setTabBar 被说成不存在，正文按源码改准。

咱们把默认状态照源码记一遍。构造走 QTabWidgetPrivate::init，页面堆叠建在前面，对象名 qt_tabwidget_stackedwidget，标签栏随后落地，对象名 qt_tabwidget_tabbar，并且 setDrawBase(false)，出处（qtabwidget.cpp 第 190-218 行）。标签位置由构造里的样式提示决定，问的是 SH_TabWidget_DefaultTabPosition，North 保底，焦点代理也设给了标签栏，出处第 213-216 行。正文引用以 qtabwidget.cpp 为主，标签栏侧补 qtabbar.cpp，绘制与样式表侧补 qcommonstyle.cpp、qstylesheetstyle.cpp 的行号。

## 一根标签栏加一个堆叠页，连接全收在 setTabBar 里

壳里的组合关系，咱们从 setTabBar 读起。它把几路连接一次配好：QTabBar::currentChanged 接到私有槽 showTab，tabMoved 接到 tabMoved，tabBarClicked 与 tabBarDoubleClicked 原样中继，tabCloseRequested 只在 tabsClosable 已开时中继，出处（qtabwidget.cpp 第 711-736 行）。

setTabBar 本身是 protected，您想换成自定义的标签栏子类，机会在子类构造里、任何 addTab 之前，旧的内置栏会被 delete 掉，文档注释也明确要求在加标签之前调用，出处（qtabwidget.cpp 第 705-720 行，qtabwidget.h 第 135 行）。源稿说接口不存在，读漏了 protected 段。

查询类的接口也分头问两家，咱们拿 count() 与 indexOf() 对一下：count() 问标签栏，indexOf() 问堆叠页，出处（qtabwidget.cpp 第 688-692、1170-1174 行）。两侧靠回流机制保持同构，您随便问哪边，答案都一样。

切换的时序值得记牢。您调 setCurrentIndex，话落到 QTabBar::setCurrentIndex，同索引直接返回，拖拽进行中拒绝切换，出处（qtabbar.cpp 第 1401-1407 行）。

确实变了才发 QTabBar 的 currentChanged，进 showTab，那里调 stack->setCurrentIndex 把新页面立起来，随后才发 QTabWidget 自己的 currentChanged，出处（qtabwidget.cpp 第 755-761 行）。咱们在 currentChanged 槽里拿 currentWidget()，到手的已经是新页面，不用等下一轮事件循环。

增删的方向不对称，笔者读到这也停了半拍。insertTab 是双写，堆叠页插一条、标签栏插一条，出处（qtabwidget.cpp 第 435-446 行）。removeTab 却只调 stack->removeWidget，标签栏那边靠 QStackedWidget 的 widgetRemoved 信号回流到 QTabWidgetPrivate::removeTab，才删标签并重排布局，出处（qtabwidget.cpp 第 627-632、200、763-769 行）。

删页面从堆叠侧驱动，加页面在外层双写，咱们调试索引错位时按方向找源头，能少绕弯。

插入还带附赠行为。标签栏原本是空的，insertTab 落地第一根标签时自动把它设成当前项，插到当前项前面则只把当前索引往后顶一格、页面不动，出处（qtabbar.cpp 第 969-972 行）。标签文字里的 & 记号与 QGroupBox 的标题同路数，insertTab 拿 QKeySequence::mnemonic 抓成快捷键，出处第 964-966 行，您给标签写 “常规(&G)”，Alt 加 G 就能落到对应的标签上。

setTabEnabled 是又一处双写，标签栏那侧管视觉与点击，页面控件那侧挨个 setEnabled，出处（qtabwidget.cpp 第 524-530 行）。禁用标签只是点不进去，页面还躺在堆叠里，全禁用时窗口照样显示其中一页，咱们在官方文档 setTabEnabled 的说明里能读到原话。

## 关闭按钮是内置的 tabButton，点击只发信号

setTabsClosable(true) 之后您点关闭按钮没动静，原因在链路终点。QTabWidget::setTabsClosable 转发给标签栏，再把 QTabBar 的 tabCloseRequested 中继成自己的同名信号，出处（qtabwidget.cpp 第 1023-1036 行）。

标签栏那侧给每个标签 new 一个 CloseButton，QAbstractButton 的私有子类，出处（qtabbar.cpp 第 41-54 行）。它的绘制走 PE_IndicatorTabClose，尺寸问 PM_TabCloseIndicatorWidth 与 Height，工具提示写着 “Close Tab”，出处（qtabbar.cpp 第 2860-2878、2915 行）。点击之后 closeTab 拿 sender 与每个标签的 leftWidget、rightWidget 逐一比对找到索引，然后 emit tabCloseRequested，到这就停了，移除与销毁都在您手里，出处（qtabbar.cpp 第 771-792 行）。

关闭按钮落在哪一侧也不是写死的。侧别来自样式提示 SH_TabBar_CloseButtonPosition，common style 返回 RightSide，出处（qtabbar.cpp 第 2570 行，qcommonstyle.cpp 第 5349-5350 行）。样式表引擎也接管了同一条提示，您在 QSS 里给 close-button 规则写 position 对齐，靠左就答 LeftSide、靠右就答 RightSide，出处（qstylesheetstyle.cpp 第 5759-5767 行）。

侧别正是 setTabButton 冲突的根源。setTabButton 对同侧旧控件只 hide 不删，出处（qtabbar.cpp 第 2804-2812 行），您往 RightSide 塞进度条，内置关闭按钮被藏起来，closeTab 的比对从此找不到它，tabCloseRequested 也就停了。想在标签上带进度条又保留关闭，把自定义控件放 LeftSide，示例正是这么做的，出处（closable_tab_widget.cpp 第 179-192 行）。

还有一处连带的行为，关掉 tabsClosable 时，处在关闭侧的控件被无条件 deleteLater，代码分不清那是内置按钮还是您的自定义件，出处（qtabbar.cpp 第 2571-2581 行）。您把进度条放去了 RightSide，回头一关 tabsClosable，被清掉的就是那个进度条。

移除的写法有个顺序要守，咱们把次序摆正：widget(index) 要在 removeTab 之前取，移除之后同一个 index 指向的是下一页，您删错对象还算轻的，末尾标签时干脆拿回空指针。

示例的 handleCloseRequested 是范本，您照着它走：取指针、按需确认、removeTab、deleteLater，出处（closable_tab_widget.cpp 第 112-140 行）。连续关多个标签时索引会前移，您倒着遍历，或者攒一批索引再统一删。

源稿还有一处误传，咱们当场纠正。图标大小在 QTabWidget 上有公开的 setIconSize，属性挂在头文件里，实现是转给内部标签栏，出处（qtabwidget.cpp 第 1286-1304 行）。示例里写的 tabBar()->setIconSize(QSize(20, 20)) 与它等价，出处（closable_tab_widget.cpp 第 47-48 行），注释也写明了两条路等价。

## 拖拽重排整套在 QTabBar 手里

setMovable 在 QTabWidget 这边只有一行转发，出处（qtabwidget.cpp 第 1053-1056 行）。拖拽的实现，笔者把鼠标的按下与移动两个处理函数连起来读了一遍：按下时记下 dragStartPosition，位移的曼哈顿距离超过 QApplication::startDragDistance 才算开始拖，出处（qtabbar.cpp 第 2154-2156、2172-2177 行）。拖动过程中标签带着 dragOffset 跟手挪，松手才落位。

落位之后的联动在 moveTab。它重排内部列表并 emit tabMoved，出处（qtabbar.cpp 第 2087-2092 行），QTabWidgetPrivate::tabMoved 收到后，在 QSignalBlocker 里对堆叠页做一次 remove 加 insert，让页面顺序跟上标签，屏蔽信号是为了不给您塞多余的中继，出处（qtabwidget.cpp 第 771-777 行）。顺序只活在内存里，重启就丢，想跨重启保住顺序，得您自己写进设置。

## 溢出的两条路，滚动按钮或省略号

标签一多，咱们就得在长个儿与滚动之间选一边。usesScrollButtons 的默认值问样式提示 SH_TabBar_PreferNoArrows，出处（qtabbar.cpp 第 397-398 行）。

它还悄悄影响 sizeHint：滚动按钮开着时，QTabWidget 的 sizeHint 把标签栏那一维封顶在 200 像素，关着才允许随标签数长到屏幕尺寸，出处（qtabwidget.cpp 第 862-867、929-935 行）。您把按钮一开，窗口尺寸不再被标签栏绑架，代价是挤出去的标签要靠箭头找回来。

文字放不下归 elideMode 管。默认值同样问样式，common style 给 ElideNone，出处（qcommonstyle.cpp 第 5276-5278 行）。您一旦调过 setElideMode，用户标志 elideModeSetByUser 置位，之后样式变更也不再覆盖您的选择，出处（qtabbar.cpp 第 2510-2511、2458-2459 行）。示例里显式设了 Qt::ElideRight，超长标题右侧出省略号，出处（closable_tab_widget.cpp 第 41 行）。

## 位置、四角与自动藏栏

setTabPosition 看着管方向，咱们读实现只看到两步，存位置，折算成标签栏的 shape，出处（qtabwidget.cpp 第 960-1008 行）。East 与 West 的竖排文字，旋转的活儿不在标签栏手里，它发生在 common style 的 CE_TabBarTabLabel，East 转 90 度、West 转 -90 度，出处（qcommonstyle.cpp 第 1972-1997 行），您拿两侧页签对比着看，转向刚好相反。入门篇把竖排描述成图标在上、文字在下，与旋转出来的长相不同，竖排的呈现随样式走，您以本机样式为准。

键盘那边咱们也顺手试了，Ctrl 加 Tab 在页面间轮换，Shift 反着来，出处（qtabwidget.cpp 第 1111-1152 行）。

四角控件的归属咱们看准了，挂在 QTabWidget 本体，不挂标签栏，几何由 SE_TabWidgetLeftCorner 与 SE_TabWidgetRightCorner 两个子元素矩形决定，出处（qtabwidget.cpp 第 592-608、806-814 行）。文档注释提醒过，它只保证 North 与 South 下工作正常，East 与 West 属于没人管的地带，出处第 587-588 行，您真要用竖排页签，四角就别指望了。

tabBarAutohide 转发给标签栏的 autoHide，标签数不超过 1 时整根栏藏掉，出处（qtabwidget.cpp 第 1390-1400 行，qtabbar.cpp 第 763-769 行）。藏栏不影响尺寸测算，sizeHint 里查了 isAutoHidden，藏掉的栏不再往窗口尺寸里添贡献，出处（qtabwidget.cpp 第 163-167、861 行）。您做单文档界面时开着它，只剩一页的瞬间标签栏自己让位。

几何的重算节奏咱们也看了一眼。setUpLayout 在控件不可见时只标一个脏位就返回，等 showEvent 或 resizeEvent 再补算，出处（qtabwidget.cpp 第 784-797、955-958 行）。所以咱们在构造期连着 addTab 一串页面，布局活儿被攒着，显示那一刻一次算清。

## documentMode 连拨了一串开关

咱们把 setDocumentMode 的实现数了一遍，它连着调了标签栏的一串接口：setDocumentMode、setExpanding 取反、setDrawBase 同步开，出处（qtabwidget.cpp 第 1370-1377 行）。

expanding 默认是 true，标签会摊开占满空位，documentMode 把它关掉，咱们看到的标签就恢复自然宽度，出处（qtabbar.cpp 第 2651-2673 行）。外框那边，paintEvent 在 documentMode 下画完四角的底座线就提前返回，非 documentMode 才画整圈 PE_FrameTabWidget，出处（qtabwidget.cpp 第 1254-1283 行）。

外框一去，标签栏和页面容易糊在一起，您可以用 QSS 给 QTabWidget::pane 补一条顶线，示例就是这么处理的，出处（closable_tab_widget.cpp 第 43-45 行）。macOS 上 documentMode 会贴着原生样式走，跨平台验收时别拿某一个平台的效果当标准，要么统一开、要么统一关，再拿 QSS 兜住差异。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，QTabWidget 与 QTabBar、QStackedWidget 同在 QtWidgets，链接 Qt6::Widgets 即可。

[Qt 文档 · QTabWidget](https://doc.qt.io/qt-6/qtabwidget.html) -- 标签页控件，tabsClosable、documentMode、tabBarAutohide 等属性与 tabCloseRequested 信号说明

[Qt 文档 · QTabBar](https://doc.qt.io/qt-6/qtabbar.html) -- 标签栏控件，setTabButton、movable、elideMode、usesScrollButtons 定义在此

[Qt 文档 · QStackedWidget](https://doc.qt.io/qt-6/qstackedwidget.html) -- 堆叠页面容器，widgetRemoved 信号驱动 QTabWidget 删标签

[Qt 文档 · QStyle](https://doc.qt.io/qt-6/qstyle.html) -- 样式基类，SH_TabBar_CloseButtonPosition 等样式提示的定义处

[Qt 文档 · QStackedLayout](https://doc.qt.io/qt-6/qstackedlayout.html) -- 堆叠布局，setCurrentIndex 与 removeWidget 的实际实现层

---

页签控件到这就拆完了，壳里的两件东西您都见过真身，下一程咱们单独去会标签栏本身，QTabBar 脱离 QTabWidget 之后还有一整套拖拽、按钮与选择策略的活儿，入口在 [3.40 QTabBar 进阶](./40-qtabbar-advanced.md)。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/39-qtabwidget-advanced/，可执行文件 39-qtabwidget-advanced）。构造里四连开，setTabsClosable、setDocumentMode、setMovable、setElideMode 各就各位，出处（closable_tab_widget.cpp 第 38-41 行）。点 “当前标签加进度条”，进度条落在标签左侧，关闭按钮照常工作，出处第 179-192 行。往某个标签里敲几个字再点关闭，确认对话框会拦一下，出处第 112-140 行。入口在 main.cpp 第 13-21 行。

完成标准照旧。currentChanged 的槽里 currentWidget() 指向哪一页、依据哪几行代码，您指得出来。关闭按钮点下去为什么页面不动、链路在哪一行停住，您答得上来。RightSide 放了自定义控件后再关掉 tabsClosable，您的控件会遭遇什么，您不跑程序就能推。标签数翻倍之后窗口的 sizeHint 为什么不怎么长，您说得出来。
