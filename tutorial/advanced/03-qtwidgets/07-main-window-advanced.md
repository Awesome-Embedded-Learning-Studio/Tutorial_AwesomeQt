---
title: "3.7 主窗口进阶"
description: "用户拖拽调好的 Dock 布局、工具栏位置和窗口大小，关掉程序再打开全部回到默认状态。本篇讲 saveState/restoreState 搭配 QSettings 持久化 Dock 和工具栏布局，saveGeometry/restoreGeometry 记住窗口自身的几何信息，toggleViewAction 让视图菜单和 Dock 显隐自动同步，setCorner 划清四个角落的归属，restoreState 的返回值口径、corner 配置和存档的先后也当场讲清。配套示例在 examples/advanced/03-qtwidgets/07-main-window-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.7——主窗口进阶

[入门篇主窗口基础](../../beginner/03-qtwidgets/07-main-window-system-beginner.md) 把 QMainWindow 的五大区域过了一遍，菜单栏、工具栏、状态栏、停靠面板、中央控件怎么搭，照着文档走一遍就能立起一个功能完整的骨架。骨架不难搭，难的是留不住。您要是做过用户每天都要打开的软件，代码编辑器、数据分析工具、图像处理软件一类的 IDE 式应用，迟早会收到这么一串反馈：用户辛辛苦苦拖拽调整好的 Dock 布局、工具栏位置、窗口大小，关掉程序再打开，全部回到默认状态，调好的界面一点没留下。

咱们这一篇就补上状态记忆，用户调好的 Dock 和工具栏布局、窗口自己的位置大小，关了再开原样回来。视图菜单和 Dock 显隐的自动同步、四个角落的归属，也一并弄清。讲究藏在细节里，corner 配置和存档谁说了算、Dock 靠什么名字对上号，咱们讲到跟前当场弄清。

## saveState / restoreState：把整套 Dock 布局装进一个字节数组

QMainWindow 给了咱们一对现成的 API。saveState() 返回一个 QByteArray，把当前整套 Dock 和工具栏的布局编码进去。restoreState() 接收同一个字节数组，把布局还原到保存时的样子。标准用法是搭配 QSettings，窗口关闭时保存，构造完成后恢复：

```cpp
void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings("MyCompany", "MyApp");
    settings.setValue("main_window/geometry", saveGeometry());
    settings.setValue("main_window/state", saveState(kStateVersion));
    QMainWindow::closeEvent(event);
}
```

恢复就放在构造函数里做，Qt 官方文档给的 readSettings() 模式就是构造期恢复、show() 之前完成，这条路是官方支持的。片段里 restoreState 传的 kStateVersion 是布局版本号常量，配套示例（examples/advanced/03-qtwidgets/07-main-window-advanced/，程序叫 PersistentMainWindow）里定义为 1。咱们照官方模式把恢复写进构造函数：

```cpp
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupMenuBar();
    setupDockWidgets();  // 创建并 addDockWidget
    setupToolBars();

    QSettings settings("MyCompany", "MyApp");
    restoreGeometry(settings.value("main_window/geometry").toByteArray());
    restoreState(settings.value("main_window/state").toByteArray(), kStateVersion);
    show();  // 窗口显示时，布局引擎把保留的恢复结果应用上去
}
```

构造期恢复常被怀疑是 Dock 全部浮动的元凶，咱们把 Qt 6.9.1 的源码翻出来看（qmainwindowlayout.cpp），QMainWindowLayout::restoreState 对窗口还不可见的情况有专门分支：恢复结果先由布局引擎保留，等窗口显示时应用，最大化、全屏的场合还留一份重试副本，等多次 resize 落定后再试。调用顺序在这里解释不了全部浮动。病根在存档本身，Dock 的浮动状态是以 Floating 标记序列化进存档的，restoreState 只是如实还原，读到的存档在保存那一刻是浮动的，恢复出来就是浮动的。排查方向应该转向存档，先确认保存的时机和读到的到底是哪份数据。配套示例的 main.cpp 选择了先 window.show() 再调 window.restoreLayout() 的顺序，两种顺序都成立，它演示的是后一种。

QSettings 的键值存储是跨平台的，Windows 上走注册表，macOS 上走 CFPreferences（.plist 文件），Linux 上是 INI 格式的配置文件。代码里这些差异全透明，调试时您得知道去哪里找配置数据。

closeEvent 里两行 setValue 各存一份，geometry 和 state 是两份不同的数据，咱们把分工摆成表，写恢复代码时对着看：

| API                               | 管的范围 | 字节数组里编码的内容                                                             |
| --------------------------------- | -------- | -------------------------------------------------------------------------------- |
| saveGeometry / restoreGeometry    | 窗口自身 | 屏幕上的位置（x, y）、大小（width, height）、最大化/全屏状态                    |
| saveState / restoreState          | 窗口内部 | Dock 的位置、大小、堆叠顺序、浮动状态、是否隐藏，工具栏的位置和行号             |

restoreState 的第二个参数用来区分不同版本的布局数据。您在新版本里改了 Dock 的结构，比如新增了一个 Dock，旧版本的 state 数据就可能无法正确恢复。做法是给 version 递增一个数字，restoreState 在版本不匹配时直接返回 false，主窗口保持原状，咱们拿返回值走默认布局兜底就行。false 还有另一个来源，从来没有存过档，restoreState 收到空字节数组同样返回 false。两种 false 单看返回值分不开，您想区分，要么自己用 settings.contains() 查 key，要么单独存一个版本号，toggleViewAction 一节的代码片段用的就是前一种。

## objectName：restoreState 认 Dock 的唯一标识

saveState 存下的堆叠信息，恢复时要一项项对回各个 Dock，靠的就是 objectName。QMainWindow 内部用 objectName 来标识每个 Dock。多个 Dock 堆叠在同一区域时对它依赖最重，比如咱们让三个 Dock 都停在左侧、以 tab 形式排列，restoreState 恢复的不只是位置，还有 tab 的堆叠顺序和当前激活的 tab，匹配全按 objectName 来。

您要是忘了设置，或者几个 Dock 的 objectName 重复了，restoreState 就无法正确匹配保存的状态，tab 顺序错乱，或者错误的 Dock 被激活，用户看到的排列顺序和上次保存时不一致。所以 objectName 必须在 addDockWidget 和 restoreState 之前设置好：

```cpp
// 必须在 addDockWidget 和 restoreState 之前设置
file_dock_->setObjectName("fileExplorer");
output_dock_->setObjectName("outputPanel");
property_dock_->setObjectName("propertyEditor");
```

配套示例里四个 Dock 各有名字，fileExplorer、propertyEditor、toolBox、outputPanel，创建完紧跟一行 setObjectName，工具栏也没落下，fileToolBar 同样设了名字。片段里的成员变量以 file_dock_ 打头，示例里对应的以 m_fileExplorerDock 打头，两套名字指的是同一类角色。咱们把示例里属性面板和工具箱的当前 tab 切到工具箱，关闭再打开，激活的仍是工具箱，替咱们对上号的正是 objectName。

## saveGeometry / restoreGeometry：窗口自身的位置大小，别手存 pos()

saveGeometry 和 saveState 经常成对出现，保存的内容却是两码事。saveGeometry 序列化的是窗口在屏幕上的位置（x, y）和大小（width, height），外加最大化/全屏状态，restoreGeometry 从字节数组里把这些信息还原回来。咱们在 [3.6 对话框进阶](./06-dialog-advanced.md) 里给对话框配几何记忆时用的就是同一对 API，主窗口这边待遇相同。

多显示器的屏幕有效性检查，[3.6 对话框进阶](./06-dialog-advanced.md) 讲对话框时已经看过，副屏被拔掉后，保存的位置不在任何有效屏幕上，restoreGeometry 会自动把窗口拉回主显示器。手动存 pos() 和 size() 拿不到这层保护，您只会收获一个开在屏幕外、看不见的窗口。

restoreGeometry 返回 bool，表示是否成功恢复。您要判断恢复有没有成功，比如失败时给窗口设默认大小，看返回值就够了。

## toggleViewAction：一个现成的开关，菜单和 Dock 两头同步

入门篇提过 QDockWidget::toggleViewAction() 能拿到一个现成的 QAction 控制显示隐藏，进阶咱们把集成做完整：怎么把它正确挂进“视图”菜单，以及默认隐藏的 Dock 怎么处理。

toggleViewAction 返回的 QAction 就是一个带 checked 状态的开关。Dock 可见时 Action 被勾选，隐藏时取消勾选。用户点它，Dock 跟着显示或隐藏，用户直接点 Dock 标题栏的关闭按钮，勾选状态也会自动跟上。双向绑定由 Qt 内部通过信号槽实现，咱们不用手动处理一行同步代码。

工程里的做法是创建每个 Dock 之后，把它的 toggleViewAction 添加到“视图”菜单。有些 Dock 默认不显示，比如“输出”面板，有编译输出时才用得上，咱们创建它的时候调用 dock->hide()，toggleViewAction 会自动反映为未勾选状态，用户随时可以从“视图”菜单勾选打开。

不过 hide() 的时机要躲开 restoreState，后者恢复的是保存时的状态，保存时面板是显示的，恢复完它就又显示出来了，您在 restoreState 之前调用的 hide() 会被覆盖。想让某些 Dock 首次运行默认隐藏，得挑 restoreState 返回 false 的时候调 hide()。false 的两个来源 saveState 一节刚说过，没存过档和版本不匹配，单看返回值分不开，所以下面的代码改用 settings.contains() 直接查 key，把“从来没有存过档”从两种 false 里挑出来：

```cpp
// 把所有 Dock 的 toggleViewAction 添加到视图菜单
auto *viewMenu = menuBar()->addMenu("视图(&V)");
viewMenu->addAction(file_dock_->toggleViewAction());
viewMenu->addAction(output_dock_->toggleViewAction());
viewMenu->addAction(property_dock_->toggleViewAction());
viewMenu->addAction(tools_dock_->toggleViewAction());

// 首次运行时隐藏输出面板
QSettings settings("MyCompany", "MyApp");
if (!settings.contains("main_window/state")) {
    output_dock_->hide();
}
```

配套示例的 restoreLayout() 用的正是返回值分支，const bool restored = restoreState(...) 之后，restored 为 false 时 m_outputDock->hide()，状态栏报“首次运行——使用默认布局”，为 true 时报“已恢复上次的窗口布局”。示例不区分 false 的来源，版本不匹配也会落进同一个分支。咱们首次运行示例，编译输出面板默认收着，状态栏把原因也明说了。示例的“视图”菜单里，四个 toggleViewAction 后面还跟了一个“重置为默认布局”，它把 QSettings 里存的两个 key 清掉，再把四个 Dock 逐个重新停靠回初始区域，方便您验证整条持久化链路。

## setCorner：四个角落的重叠区，默认全归了上下两边

QMainWindow 的 Dock 区域分上、下、左、右四个方向，四个方向在四个角落会产生重叠，左上角既是 TopDockWidgetArea 的一部分，也是 LeftDockWidgetArea 的一部分。默认情况下 Qt 把角落区域分配给水平方向，上、下区域占据整个角落宽度。带来的直接效果是，您在左侧停靠了一个 Dock，它不会延伸到窗口的最顶部，顶部的一小段空间被顶部 Dock 区域占了。

setCorner 就是改写归属用的。它接受两个参数，一个角落（Qt::TopLeftCorner / Qt::TopRightCorner / Qt::BottomLeftCorner / Qt::BottomRightCorner），一个 Dock 区域。咱们拿左上角举例，调用 setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea) 意味着左上角的重叠区域归属左侧 Dock 区域，左侧的 Dock 就能一直延伸到窗口顶部：

```cpp
// 让左侧 Dock 占据左上角和左下角
setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
// 让右侧 Dock 占据右上角和右下角
setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
```

setCorner 的调用顺序倒是自由，放在 addDockWidget 之前之后都行。它在实现里末尾就调 invalidate() 把布局标记成待重算，角落归属在每一轮布局计算时现读，后设也能生效。corner 配置会跟着 saveState 一起序列化进存档，restoreState 恢复时也用存档值覆盖当前的 corner 设置，您在 restoreState 之前调的 setCorner，恢复一完成就被存档值盖掉了。想让程序里的配置说了算，把 setCorner 放到 restoreState 之后执行。配套示例把四个 setCorner 调用封装在 setupCorners() 里，构造函数中先于 setupDockWidgets() 调用，存进去的 corner 值和配置值本来就是同一套，转一圈回来还是它。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，界面部分在 QtWidgets 模块（QMainWindow、QDockWidget、QToolBar），持久化用到 QtCore 的 QSettings。

[Qt 文档 · QMainWindow](https://doc.qt.io/qt-6/qmainwindow.html) -- 主窗口类，包含 saveState/restoreState/saveGeometry/restoreGeometry/setCorner 全套 API

[Qt 文档 · QDockWidget](https://doc.qt.io/qt-6/qdockwidget.html) -- 停靠面板类，包含 toggleViewAction 和 features 配置

[Qt 文档 · QSettings](https://doc.qt.io/qt-6/qsettings.html) -- 跨平台持久化键值存储，Dock 状态保存的后端

[Qt 文档 · Application Main Window](https://doc.qt.io/qt-6/mainwindow.html) -- 主窗口编程概述，讲 QMainWindow、QDockWidget、QToolBar 怎么协作，含 setCorner 用法片段

---

主窗口的布局持久化就到这里。想再动手的，请您把示例完整玩一轮：运行起来把属性面板和工具箱的当前 tab 切到工具箱，再拖两个 Dock 出去浮动，关闭重开，看激活的 tab 和浮动位置是不是原样回来。然后把 persistent_main_window.cpp 顶部的 kStateVersion 改成 2，重新编译运行，存档里的版本 1 和新版本号对不上，restoreState 返回 false，落进和首次运行同一个默认布局分支，状态栏报的也是“首次运行——使用默认布局”，两种 false 的口径在这里看得清清楚楚。最后点“视图”菜单里的“重置为默认布局”，看 settings.remove 之后四个 Dock 怎么被 reDock 回初始区域、属性面板和工具箱怎么重新合成一个 tab 组。下一篇 [3.8 图形视图进阶](./08-graphics-view-advanced.md) 离开主窗口，去图形视图框架里看自定义 Item 和碰撞检测，坐标系统整个换了一套。
