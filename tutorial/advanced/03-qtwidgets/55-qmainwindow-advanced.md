---
title: "3.55 QMainWindow 进阶"
description: "进阶篇把 QMainWindow 的内部机制读明白：布局粗活全外包给私有 QMainWindowLayout，setCentralWidget 对旧中央部件 hide 后 deleteLater（qmainwindow.cpp 第 594-602 行），takeCentralWidget 以 setParent(nullptr) 交还所有权（第 611-620 行，Qt 5.2 起），menuBar 与 statusBar 懒创建（第 463-473、545-553 行），setMenuBar 换栏时旧栏角落部件随行移交再销毁旧栏（第 482-506 行），addDockWidget 校验区域、按区定取向、区域变化发 dockLocationChanged（第 1026-1045 行），setCorner 只收合法组合、非法组合 qWarning 拒收（第 629-651 行），四角默认上区下区各归其主（qdockarealayout.cpp 第 2382-2385 行），右键菜单经 contextMenuEvent 只认直接子级、createPopupMenu 收编每只部件的 toggleViewAction（qmainwindow.cpp 第 1387-1500 行），toggleViewAction 出生即 checkable 且 menuRole 为 NoRole（qdockwidget.cpp 第 622-627 行），menuRole 是 QAction 属性、与 setWindowIcon 无关，saveState 用 QDataStream 固定版本存布局与四角、识别靠 objectName（qmainwindow.cpp 第 1206-1214 行），restoreState 验 marker 与版本、不符则布局原样不动（第 1233-1250 行），干粗活的是私有 QMainWindowLayout，DockOptions 六值（qmainwindow.h 第 47-54 行）除 AnimatedDocks 与 VerticalTabs 外应在加停靠部件之前设好，setDockNestingEnabled 等价 AllowNestedDocks（qmainwindow.cpp 第 896-906 行），setUnifiedTitleAndToolBarOnMac 非 macOS 上整只空壳（第 1332-1362 行），QMainWindow 不重写 sizeHint，建议尺寸由 QMainWindowLayout::sizeHint 叠加状态栏得出（qmainwindowlayout.cpp 第 2372-2381 行），示例的 F11 全屏与跨屏移动在 examples/advanced/03-qtwidgets/55-qmainwindow-advanced/"
---

# 现代Qt开发教程（进阶篇）3.55——QMainWindow 进阶

54 篇收队时说好，本篇把 QMainWindow 的机制讲清楚。咱们用了整整一篇的壳子，内部其实是一副外包结构，qmainwindow.cpp 里几乎找不到几何计算，布局的粗活全交给一只私有的 QMainWindowLayout，它住在 qmainwindowlayout_p.h，不在公开头文件里。笔者把 qmainwindow.cpp 从头翻到尾，四类入口函数，setCentralWidget、setMenuBar、setStatusBar、addDockWidget，进门都是问布局要人，自己只管交接与销毁。这个观察是本篇的骨架，咱们顺它把每个区域读一遍。

引用文件咱们交代清楚。主战场是 qmainwindow.cpp 与 qmainwindow.h（src/widgets/widgets），布局细节下探到 qmainwindowlayout.cpp 与 qdockarealayout.cpp，停靠部件的动作到 qdockwidget.cpp 里核对。配套示例是一只带双停靠区、F11 全屏与跨屏移动的演示窗口，完整玩法放结语。

## 换中央部件：旧件被藏起来，排队销毁

咱们直接读源码，setCentralWidget 全文只有这么几行：

```cpp
// 旧中央部件被藏起来，排队销毁（qmainwindow.cpp 第 594-602 行，摘录）
void QMainWindow::setCentralWidget(QWidget *widget)
{
    Q_D(QMainWindow);
    if (d->layout->centralWidget() && d->layout->centralWidget() != widget) {
        d->layout->centralWidget()->hide();
        d->layout->centralWidget()->deleteLater();
    }
    d->layout->setCentralWidget(widget);
}
```

您拿一只新部件换进来，旧件当场 hide，随后 deleteLater 排进事件循环，控制权一回到事件循环它就销毁。文档里 the previous central widget is deleted 说的是结局，机制上是延迟删除，中间连一个抢救的公开接口都没有。想留着旧部件稍后切回来，唯一的路是 takeCentralWidget（第 611-620 行，Qt 5.2 起引入），它只做 setParent(nullptr)，把所有权交还给您，指针原样返回。顺序上有讲究，您得在换新件之前把旧件取走，晚了它就进销毁队列了。

菜单栏与状态栏是同一副脾气。menuBar() 与 statusBar() 都懒创建（第 463-473、545-553 行），头一次调用时 new 一只挂给自己，所以示例里那句 statusBar()->addPermanentWidget 您尽管放开手用，出处 main_window.cpp 第 39 行。setMenuBar 换栏时多一步人情（第 482-506 行），旧栏两只角上的 cornerWidget 会被搬到新栏，然后旧栏才 hide，setParent(nullptr) 打头，deleteLater 收尾。setStatusBar 与 setCentralWidget 同款处置，出处第 566-575 行。取回接口只有 takeCentralWidget 一只，takeMenuBar 与 takeStatusBar 压根没有，咱们把 qmainwindow.h 从头 grep 到尾确认过。真要收任意部件当菜单条，setMenuWidget（第 527-536 行）留了一条通道。

## 四个停靠区与四只角：角落归谁，setCorner 说了算

咱们看 addDockWidget（qmainwindow.cpp 第 1026-1045 行），进门头一件事是区域校验，checkDockWidgetArea（第 933 行）逐项验参数，非法区域直接打回。取向由区域定死，上下两区 horizontal，左右两区 vertical。部件若已在布局里，removeWidget 把它摘出来重新放，区域确实变了就发一次 dockLocationChanged。示例的两只停靠部件就是这么进的门（main_window.cpp 第 162-168 行），左区 File Tree，右区 Properties。

四只角是左右区与上下区的重叠地带，setCorner（第 629-651 行）用一张 switch 表裁定合法组合，左上角只认上区或左区，右上角只认上区或右区，下面两角对称。组合不合法就 qWarning 拒收，不替您猜。默认归属在 qdockarealayout.cpp 第 2382-2385 行，上面两只角默认给上区，下面两只角默认给下区，想让左上角改归左区，一句 setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea) 就换了主人。

咱们想让两只停靠部件叠着放，调 tabifyDockWidget（qmainwindow.cpp 第 1110-1113 行）就行，实现只有一行，转给布局，tabifiedDockWidgets 反过来问谁跟谁叠在一起，出处第 1125-1130 行。每只停靠部件出生时自带一只显隐开关，它就是下一节的主角。

## 右键菜单是白送的：createPopupMenu 收编显隐开关

您右键点在主窗口上，contextMenuEvent（第 1387-1437 行）从点击处的子部件一路向上找，只认 parentWidget 是自己的 QMenuBar、QDockWidget、QToolBar 三类直接子级，鼠标落在停靠部件的内容区里则整场忽略。命中后调 createPopupMenu（第 1456-1500 行），findChildren 把停靠部件捞出来，滤掉不在布局里的，逐个 addAction(toggleViewAction())，工具栏同样待遇，中间一道分隔线。窗口里一只可显隐的部件都没有时返回 nullptr，空菜单在事件路径上直接 delete（第 1426-1436 行）。您想要自己的右键菜单，重写 createPopupMenu 返回新造的 QMenu 即可，所有权归调用方。

toggleViewAction 长什么样，咱们去 qdockwidget.cpp 第 622-627 行看出生记录：

```cpp
// 停靠部件的显隐开关，出生时配齐（qdockwidget.cpp 第 622-627 行，摘录）
toggleViewAction = new QAction(q);
toggleViewAction->setCheckable(true);
toggleViewAction->setMenuRole(QAction::NoRole);
toggleViewAction->setText(fixedWindowTitle);
```

您看它可勾选，文字取窗口标题，部件显隐时勾选态自动跟着走，出处是第 1653、1658 行的 setChecked。menuRole 是 QAction 的属性（qaction.h 第 52 行），管 macOS 上动作往应用菜单哪个槽位归位，跟 setWindowIcon 是两码事，后者的处理在平台层，不劳 QAction。

## saveState 与 restoreState：布局的家当，钥匙是 objectName

saveState（qmainwindow.cpp 第 1206-1214 行）把工具栏与停靠部件的布局写进 QByteArray，开一只 QDataStream，流版本固定为 Qt_5_0，写入 QMainWindowLayout::VersionMarker 与您给的 version 号，剩下的序列化全由私有布局完成。识别每只部件的钥匙是 objectName，文档在第 1192-1194 行叮嘱过，重名会互相覆盖，所以 QToolBar 与 QDockWidget 每一只的名字，您都得亲自安排。四只角的归属也一并入袋，出处 qdockarealayout.cpp 第 2412、2447 行。restoreState（第 1233-1250 行）进门第一个动作是验货，数组为空返 false，VersionMarker 或 version 号对不上也返 false，而且布局原样不动，不存在半恢复。停靠部件创建得晚也不怕，restoreDockWidget 会给晚到的部件补状态，出处第 1055-1058 行。

存取的常规搭配是 QSettings，窗口几何另有 QWidget 的 saveGeometry 与 restoreGeometry 一对，咱们本篇不抢 QWidget 的戏。54 篇的历史面板能停靠能隐藏，想让它重启后回到原位，靠的也是 saveState 与 restoreState 这一对存取。

## setDockOptions 与 Mac 统一工具栏：一组开关加一只平台特例

DockOptions 有六个值，咱们在 qmainwindow.h 第 47-54 行挨个数过。AnimatedDocks 0x01 管动画，AllowNestedDocks 0x02 管嵌套，AllowTabbedDocks 0x04 管叠放。ForceTabbedDocks 0x08 强制叠放，VerticalTabs 0x10 让标签竖排，GroupedDragging 0x20 允许整组标签一起拖。setDockOptions（qmainwindow.cpp 第 395-399 行）一行转给布局。有一处时机要留神，文档在第 388-392 行写明，这些开关只管停靠部件往哪儿落，不重排已有的部件，所以除了 AnimatedDocks 与 VerticalTabs 随时能改，其余几个应当在加停靠部件之前设好。setDockNestingEnabled 是 AllowNestedDocks 的别名（第 896-906 行），内部改的就是同一个开关位。

setUnifiedTitleAndToolBarOnMac（第 1332-1362 行）在咱们这些非苹果平台上是一只空壳，整个函数体被 Q_OS_MACOS 宏关在门外，参数 Q_UNUSED，取值函数恒返 false。macOS 上它设 WA_NativeWindow 与 WA_TranslucentBackground，又调 QCocoaWindow 的 setContentBorderEnabled，把工具栏画进标题栏。文档还提醒 Qt 5 实现的两条限制，含 OpenGL 内容的窗口与可停靠工具栏都不推荐开。

## sizeHint 不在自己手里，全屏切换里菜单栏也只是子部件

笔者在 qmainwindow.cpp 里 grep sizeHint，一处重写都没有。建议尺寸来自私有布局，QMainWindowLayout::sizeHint 拿 layoutState 的建议值，出处 qmainwindowlayout.cpp 第 2372-2381 行，再把状态栏的高度叠上去，宽度取两者较大：

```cpp
// 建议尺寸由布局与状态栏合成（qmainwindowlayout.cpp 第 2372-2381 行，摘录）
QSize QMainWindowLayout::sizeHint() const
{
    if (!szHint.isValid()) {
        szHint = layoutState.sizeHint();
        const QSize sbHint = statusbar ? statusbar->sizeHint() : QSize(0, 0);
        szHint = QSize(qMax(sbHint.width(), szHint.width()),
                        sbHint.height() + szHint.height());
    }
    return szHint;
}
```

minimumSize 同款算法（第 2383-2393 行）。您 resize 主窗口时的底线，其实是布局里所有停靠部件与状态栏商量出来的。

示例把 QMainWindow 的子部件当普通部件使。您按下 F11，事件在 keyPressEvent 里被拦下，出处 main_window.cpp 第 58-65 行，toggleFullscreen（第 87-104 行）进全屏前把 menuBar 与 statusBar 都 hide 掉，退回时 showNormal 再逐个 show 回来，几何信息用 m_normalGeometry 存取。跨屏移动走 moveToScreen（第 67-85 行），QGuiApplication::screens() 列表按 QAction 的 data 拿索引，目标屏的 availableGeometry 取到手，直接交给 setGeometry。屏幕菜单每次 aboutToShow 前重建（第 142-157 行），热插拔之后打开菜单见到的就是新清单。状态栏里的 QLabel 定时刷新屏幕信息（第 50-53 行），窗口当前在哪块屏，screen() 一问便知（第 108 行）。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过。

[Qt 文档 · QMainWindow](https://doc.qt.io/qt-6/qmainwindow.html) -- 主窗口类，区域管理与 saveState、restoreState 的定义处

[Qt 文档 · QDockWidget](https://doc.qt.io/qt-6/qdockwidget.html) -- 停靠部件，toggleViewAction 与 features

[Qt 文档 · QAction](https://doc.qt.io/qt-6/qaction.html) -- menuRole 属性，macOS 上菜单动作的归位规则

[Qt 文档 · QScreen](https://doc.qt.io/qt-6/qscreen.html) -- 屏幕几何与 availableGeometry，示例跨屏移动的来源

[Qt 文档 · QGuiApplication](https://doc.qt.io/qt-6/qguiapplication.html) -- screens 列表与 screenAdded、screenRemoved 信号

---

主窗口的机制读到这里，该轮到菜单栏接戏了。下一站 [3.56 QMenuBar 进阶](./56-qmenubar-advanced.md)，正好顺着本篇 menuBar() 的懒创建往下走，动态构建与最近文件列表都在那边等咱们。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/55-qmainwindow-advanced/，可执行文件 55-qmainwindow-advanced）。做三个实验。右键点左侧 File Tree 的标题栏，弹出菜单里两项都可勾选，取消勾选部件就藏起来，再勾上又回来。按 F11 进全屏，菜单栏与状态栏一起消失，再按 F11 退回，两个部件原样回来，窗口大小也没变。双屏环境下从 View 菜单把窗口挪到第二块屏，状态栏里的屏幕信息跟着换。

完成标准照旧。setCentralWidget 对旧件的处置走哪两步，您指得出。右键菜单里的可勾选动作是谁造的，您说得出。restoreState 拒绝恢复时布局动没动，您讲得明。DockOptions 里哪两个值随时能改，您数得清。
