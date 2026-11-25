---
title: "3.59 QDockWidget 进阶"
description: "进阶篇从主窗口换到停靠部件本体：标题栏是私有 QDockWidgetLayout 里的一个角色（qdockwidget_p.h 第 142 行），setTitleBarWidget 只换角色不删旧栏，自定义栏一上场浮动钮关闭钮跟着退场（qdockwidget.cpp 第 682-684 行），未处理的鼠标事件 ignore 后冒泡给本体，拖拽双击停靠照旧（文档第 1827-1832 行），setFeatures 三开关加竖排，Closable 关掉连视图菜单里的开关一并失能（第 1422 行），closeEvent 拒关但主窗口隐藏时放行（第 1589-1601 行），setWidget 对旧内容件只藏不删（第 407-424 行），与 setCentralWidget 的销毁是两种脾气，floating 切换由 setWindowState 重设窗口旗标引发整场重开（第 1183-1247 行），topLevelChanged 只在浮动真变时发出，Qt 6.9 新增 setDockLocation 直接指派区域（第 1880-1890 行），显隐同步走 event 的 Hide/Show 分支（第 1647-1667 行），ZOrderChange 让 tab 组顶上的部件 raise（第 1676-1687 行），尺寸由私有布局代算且标题栏按钮计入最小宽度（第 436-477 行），示例在 examples/advanced/03-qtwidgets/59-qdockwidget-advanced/"
---

# 现代Qt开发教程（进阶篇）3.59——QDockWidget 进阶

56 到 58 三篇，咱们把菜单栏、工具栏、状态栏挨个读完，这篇回到停靠部件本体。55 篇里咱们站在主窗口主人的视角，读过 addDockWidget 的区域校验、setCorner 的角落归属、createPopupMenu 收编显隐开关，还有 saveState 拿 objectName 当钥匙存布局。本篇换到部件自己的视角。

笔者把 qdockwidget.cpp 从头读到尾，发现 QDockWidget 自己几乎不排版，标题栏、浮动钮、关闭钮、内容件全是私有 QDockWidgetLayout 里的角色，连拖拽都在 event() 里接鼠标事件亲手处理。这个观察是本篇的骨架。

引用文件咱们交代清楚。主战场是 qdockwidget.cpp 与 qdockwidget.h（src/widgets/widgets），私有结构下探到 qdockwidget_p.h，布局序列化那一头 55 篇读过了，本篇挂链不重讲。配套示例是一只带三只停靠面板与布局持久化的演示窗口，完整玩法放结语。

## 标题栏是布局里的一个角色，换它不必赔上拖拽

咱们去看出生记录。init()（qdockwidget.cpp 第 601-632 行）里两只 QDockWidgetTitleButton 一出生就挂进布局，objectName 一个叫 qt_dockwidget_floatbutton，另一个叫 qt_dockwidget_closebutton，出处第 608-617 行。角色表在 qdockwidget_p.h 第 142 行，Content、CloseButton、FloatButton、TitleBar 四个槽位，标题栏与内容件平起平坐，都是布局项。

两只按钮的个头也定得细。QDockWidgetTitleButton::sizeHint（第 140-151 行）拿 PM_DockWidgetTitleBarButtonMargin 与图标实际尺寸算一个正方形，图标尺寸另有讲究，样式说 SH_DockWidget_ButtonsHaveFrame 时图标还要折一圈，16 像素缩到 10（第 130-138 行）。sizeHint 您别急着跳过，后面算标题栏最小宽度时要拿它用。

您调 setTitleBarWidget（第 1855-1867 行），实际动作一行，把部件塞进 TitleBar 槽位，随后 updateButtons。传 nullptr 会把自定义栏摘下来但不删它，默认标题栏回来顶班（文档第 1817-1820 行）。updateButtons（第 674-709 行）里有这么一句：

```cpp
// 自定义栏或原生装饰在场，两只按钮退场（qdockwidget.cpp 第 682-684 行，摘录）
bool customTitleBar = dwLayout->widgetForRole(QDockWidgetLayout::TitleBar) != nullptr;
bool nativeDeco = dwLayout->nativeWindowDeco();
bool hideButtons = nativeDeco || customTitleBar;
```

随后两只按钮 setVisible(false)，出处第 692、701 行。浮动状态下待遇更彻底，nativeWindowDeco（第 245-248 行）要求 TitleBar 槽位空着才让窗口管理器画原生装饰，自定义栏一上场，浮动后也是 Qt 自己画的框，paintEvent 里画 PE_FrameDockWidget 与 CE_DockWidgetTitle（第 1604-1635 行），此时连默认标题都不画了。您想彻底不要标题栏，文档留了后门，拿一只默认构造的 QWidget 顶上去就行，出处第 1843-1845 行。

默认标题画什么，咱们让 initStyleOption 说了算（第 641-661 行），closable、movable、floatable、verticalTitleBar 一并交给样式，标题文字用的是洗过一遍的 fixedWindowTitle（第 654 行，init 时算好存下，第 625 行）。所以特性变了标题栏会跟着换脸，链条是 setFeatures 调 updateButtons，再触发重画。

咱们把竖排特性开起来，标题栏就挪到部件左侧。setGeometry（第 487 行起）里摆内容区时有这么一个分支，竖排时内容让到标题栏右侧，横排时让到下方（第 541、544 行）。您拿着窄面板想给内容多留横向空间，靠的就是它。

有种流传的说法是换了自定义标题栏，拖拽就没了，得自己写鼠标处理。源码不支持这个说法。咱们去查文档，第 1827-1832 行写明，标题栏没处理的鼠标事件调 QMouseEvent::ignore()，事件冒泡给部件本体，本体的 event() 在第 1700-1715 行接住 MouseButtonPress 一路到 Release，拖动、双击停靠照旧。

会丢拖拽的场景只有一种，您在自定义栏里把鼠标事件吃掉了。文档另有两句叮嘱，标题栏得给有效的 sizeHint 与 minimumSizeHint（第 1839-1841 行），开着竖排特性时，还要在它自己的 resizeEvent 里查方向，出处第 1834-1837 行。

## setFeatures：三只开关加一条竖排，关掉 Closable 连视图菜单一起哑

枚举咱们在 qdockwidget.h 第 40-50 行挨个数过，DockWidgetClosable 0x01、DockWidgetMovable 0x02、DockWidgetFloatable 0x04、DockWidgetVerticalTitleBar 0x08，全零叫 NoDockWidgetFeatures。默认值不在头文件里，在 qdockwidget_p.h 第 83-84 行，前三只全开。

咱们看 setFeatures（qdockwidget.cpp 第 1410-1434 行），进门套一层 DockWidgetFeatureMask 滤掉保留位，值没变直接回。VerticalTitleBar 位转给布局的 setVerticalTitleBar（第 1420 行，实现在第 552-559 行，只改标志重画）。第 1422 行是容易被漏看的一句：

```cpp
// 不可关闭的部件，视图菜单里的开关一并失能（qdockwidget.cpp 第 1422 行，摘录）
d->toggleViewAction->setEnabled((d->features & DockWidgetClosable) == DockWidgetClosable);
```

您把 Closable 关掉，视图菜单里对应开关整只灰掉，用户连重新唤出的入口都没有，设计固定面板时，得把它跟 NoDockWidgetFeatures 放在一起想。

咱们再看 closeEvent（第 1589-1601 行），同款脾气，不可关闭的部件拒收关闭事件，但父主窗口已经隐藏时放行，不然应用都没法退出。变更通知叫 featuresChanged（qdockwidget.h 第 81 行）。closable 位变了且浮动带原生装饰时，还会重设一次窗口状态（第 1425-1433 行），让装饰上的关闭钮跟着增减。

## setWidget：旧内容件只藏不删，换中央部件那套销毁这里没有

setWidget 只有一行（第 1394-1398 行），转给布局的 setWidgetForRole。您猜旧内容件什么下场？咱们看实现（第 407-424 行），旧件 hide 一句、removeWidget 一句，到此为止。removeWidget 删的是布局里的包装项，部件本体还挂在对象树上（qlayout.cpp 第 1297-1314 行）。

对照 55 篇读过的 setCentralWidget，旧中央部件是 hide 加 deleteLater 排队销毁，一家人两种脾气。想把停靠部件的内容件换下来复用，setWidget 本身就是安全的，旧件藏在那里等您收走。新件走 addChildWidget，父挂对了就直接 show（第 415-418 行），widget() 取回的，也正是 Content 槽位上的部件（第 1377-1381 行）。

## floating 切换：窗口旗标重设引发整场重开

咱们从 isFloating() 看起，它是 isWindow() 的内联，出处 qdockwidget.h 第 58 行，判定就这一句。setFloating（qdockwidget.cpp 第 1459-1463 行）转给私有实现（第 1468-1519 行），进门若正在拖拽就当场叫停（第 1472-1473 行），头一次拔出时把当前几何存进 undockedGeometry（第 1475-1478 行），从浮动 tab 组里单拔一只还要换父（第 1492-1497 行）。

真干活的是 setWindowState（第 1183-1247 行）。笔者数了它一只函数里做的事，本体 hide，算旗标，setWindowFlags，摆几何，再 show。setWindowFlags 会重建平台窗口，那套 hide 与 show 是它的代价。

咱们看旗标表，第 1204-1215 行。浮动的基底是 Qt::Tool，停靠的基底是 Qt::Widget，窗口管理器肯画原生装饰就补 CustomizeWindowHint 与 WindowTitleHint，Closable 开着再加 WindowCloseButtonHint，不肯画就换 FramelessWindowHint，平台拖拽中途，还会短暂加 X11BypassWindowManagerHint，出处第 1220-1221 行。

信号的发放有节制，咱们接着数。topLevelChanged 只在浮动状态真变了才发（第 1235-1236 行），同一场里 dockLocationChanged 也发，回停靠给实际区域，拔出给 NoDockWidgetArea（第 1237-1243 行）。Resize 事件另有一手，浮动中顺手把几何记回 undockedGeometry（第 1725-1728 行），下次拔出就能回老位置。

Qt 6.9 还给咱们添了一对区域接口。setDockLocation（第 1880-1890 行）直接给部件指派停靠区，浮动中的部件下一次 setFloating(false) 会落到指定区，传 NoDockWidgetArea，等于 setFloating(true)。dockLocation（第 1899-1912 行）回读时浮动状态恒返 NoDockWidgetArea，头文件里它已经是带 NOTIFY 的属性（qdockwidget.h 第 28-29 行）。

## 显隐、层级与尺寸：事件表接暗线，私有布局管算法

显隐的同步咱们到 event() 的 Hide 与 Show 分支里找（第 1647-1667 行），部件藏起来时 toggleViewAction 置未勾选并发 visibilityChanged(false)，亮出来时反向同步。Show 分支还多一道检查，几何的右下角要是落在屏幕可用区左上角之外，信号照样给 false（第 1657-1667 行），部件被挪到屏幕负坐标外时，isVisible() 与这个信号就分家了。叠进 tab 组的非激活页也发 false，文档第 1790-1801 行提醒过。

开关本身咱们在 55 篇读过出生记录（第 622-627 行），勾选调 show、取消调 close（第 663-672 行），窗口标题变了它还跟着换字，出处 changeEvent 第 1570-1571 行。changeEvent 里还有一小段，部件浮动时标题由平台窗口扛着，WindowTitleChange 分支会把平台标题同步回内部（第 1560-1565 行），省得两边各写各的。

ZOrderChange 分支在第 1676-1687 行，管的是 tab 组里谁在上。部件成为主窗口孩子列表的最后一个且未浮动时，布局就调 `raise` 把它顶上去，您在 tab 组里点谁谁浮上来，接的就是这一下。

尺寸这块 QDockWidget 自己不插手。笔者把头文件的护法清单数了一遍（qdockwidget.h 第 88-92 行），changeEvent、closeEvent、paintEvent、event 四个，没有 sizeHint 也没有 minimumSizeHint。建议尺寸与最小尺寸全由私有 QDockWidgetLayout 代算，sizeHint 第 363-372 行，minimumSize 第 385-394 行，都走 sizeFromContent 叠上标题栏的开销。

开销单独有一份算法。minimumTitleWidth（第 436-462 行）把关闭钮浮动钮的 sizeHint、标题高与样式边距全加进去，Closable 或 Floatable 关掉就少算一只按钮，titleHeight 在第 464-477 行，对自定义标题栏取它 sizeHint 的垂边。您关掉特性，部件能缩到的最小宽度真的会跟着缩。主窗口那头来问底线时，还要经过 QDockWidgetItem 一层包装（第 570-576 行），内容件的最小值叠上标题栏开销才交出去。

## 示例：持久化的整套流程装在一只窗口里

示例咱们走一遍（examples/advanced/03-qtwidgets/59-qdockwidget-advanced/，一只主窗口带三只停靠面板）。setupDockWidgets（main_window.cpp 第 64-121 行）造三只面板，三只都补了 objectName（saveState 的识别钥匙），第 113-120 行统一设 AllDockWidgetAreas，三特性全开，addDockWidget 进左区。View 菜单（第 123-136 行）收编三只 toggleViewAction，再挂一条 Reset Layout。

存取咱们交给 QSettings。main.cpp 里 setOrganizationName 与 setApplicationName（第 15-16 行）跟 saveLayout 里 QSettings 的构造参数对上，两处名字一致，存和取才落在同一条路径。saveLayout（第 138-150 行）存 saveGeometry 与 saveState，closeEvent 在第 58-62 行，析构在第 52-56 行，各存一遍。restoreLayout（第 152-170 行）在构造函数里就调（第 46 行），没存过就走 resetLayout（第 172-189 行），三只面板 show 回来，重新 addDockWidget 挪位，tabifyDockWidget 叠第 187 行，raise 第 188 行。resetLayout 的注释说 addDockWidget 会把已在布局里的部件挪到新位置，正是 55 篇读过的行为，进门发现部件已在布局里，摘出来重放。

saveState 与 restoreState 的机制 55 篇读过了，钥匙是 objectName。示例的早期版本在这里摔过一跤：三只面板都没设 objectName，序列化时写空名，qdockarealayout.cpp 第 1864 行的 qWarning 会替您提醒，恢复端拿名字认部件，三只无名部件各回各位就成了碰运气。现在的示例已把三句 setObjectName 补齐，出处 main_window.cpp 第 68、84、101 行。您把这三行注释掉再跑一轮持久化，warning 与错位就都回来了。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过。

[Qt 文档 · QDockWidget](https://doc.qt.io/qt-6/qdockwidget.html) -- 停靠部件类，标题栏定制、features 与 floating 的定义处

[Qt 文档 · QMainWindow](https://doc.qt.io/qt-6/qmainwindow.html) -- addDockWidget 与 saveState、restoreState，55 篇的主战场

[Qt 文档 · QSettings](https://doc.qt.io/qt-6/qsettings.html) -- 跨平台持久化后端，示例存布局的容器

[Qt 文档 · QLayout](https://doc.qt.io/qt-6/qlayout.html) -- removeWidget 与 addChildWidget 的通用语义

---

主窗口一家到这里收队，下一站 [3.60 QDialog 进阶](./60-qdialog-advanced.md)，从常驻窗口转入对话框，exec 模态循环与三种模态粒度在那边等咱们。

想动手的您把示例跑起来（examples/advanced/03-qtwidgets/59-qdockwidget-advanced/，可执行文件 59-qdockwidget-advanced）。把 Output 面板拖出主窗口浮动，关掉再从 View 菜单勾回来，它仍以浮动窗口现身，因为旗标从没被动过。拖乱三只面板的位置，关掉程序重开，布局回到上次的样子，把三句 setObjectName 注释掉再试一轮，对比恢复的可靠程度。View 菜单里取消勾选再勾回 File Tree，开关与显隐的同步，就是 event() 的 Hide 与 Show 两条分支。再改一处代码，把第 116-118 行特性清单里的 DockWidgetClosable 删掉重编，View 菜单里 File Tree 的开关当场变灰，标题栏的关闭钮也不见了，两处联动都出自 setFeatures。

完成标准照旧。setWidget 对旧内容件做了什么，跟 setCentralWidget 差在哪，您说得出。Closable 关掉后视图菜单里的开关会怎样，您讲得明。setFloating 切换时窗口经历哪几步，您数得清。自定义标题栏怎么保住拖拽，您答得上。
