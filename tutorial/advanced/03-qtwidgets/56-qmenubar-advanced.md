---
title: "3.56 QMenuBar 进阶"
description: "进阶篇把 QMenuBar 的机制读明白：平台给了原生菜单栏这只部件就被 hide（qmenubar.cpp 第 673-677 行），macOS 与 Android 恒原生、Linux 看 D-Bus 服务、Windows 的 QApplication 默认自绘，原生菜单栏生效时尺寸函数跳过条目几何只剩角部件（第 1528、1578、1629 行），setVisible 的 show 是空操作（第 943-951 行），setNativeMenuBar 随时换向且旧项由 actionEvent 镜像补同步（第 1783-1798、1190-1231 行），insertMenu 插的是 menuAction（第 805-811 行），QMenuBar::clear 只摘不删（第 846-852 行）而 QMenu::clear 会删 parent 归属且未共享的动作（qmenu.cpp 第 2213-2225 行），装不下的项进 hiddenActions 塞进 extension 按钮的 QMenu（第 126-212 行），setCornerWidget 只认两只上角（第 1713-1745 行），setDefaultUp 默认向下弹（qmenubar_p.h 第 34-35 行），键盘导航经 getNextAction 跳过空矩形与禁用项（第 688-716 行），aboutToShow 在部件与原生两条路径都发出（qmenu.cpp 第 2366、196、1549-1551 行，macOS 桥头在 qcocoansmenu.mm 第 168-177 行），最近文件列表的 aboutToShow 重建与示例玩法在 examples/advanced/03-qtwidgets/56-qmenubar-advanced/"
---

# 现代Qt开发教程（进阶篇）3.56——QMenuBar 进阶

上一篇收尾时留了一句，menuBar() 懒创建，头一次调用才 new 出一只 QMenuBar 挂上主窗口。懒创建回答的是部件出生的时机，本篇接着往下问，出生以后它在窗口里占不占地方，占的话运行时增删菜单它怎么应付。QMenuBar 与 QMenu 的分工一句话说清，前者管窗口顶上的横条和顶层菜单项，后者管弹出来的菜单清单，咱们本篇两条腿都走。

咱们把引用文件交代清楚。主战场是 qmenubar.cpp 与 qmenubar_p.h（src/widgets/widgets），菜单侧下探 qmenu.cpp，removeAction 的出处要翻到 qwidget.cpp（src/widgets/kernel），平台插件只取要紧的几行。配套示例是一只动态菜单加最近文件列表的演示窗口（examples/advanced/03-qtwidgets/56-qmenubar-advanced/），完整玩法放结语。

## 出生那天就可能被平台收走：原生菜单栏与窗口里的净高

咱们从出生现场读起：

```cpp
// 平台交出原生菜单栏，这只部件就被藏起来（qmenubar.cpp 第 673-677 行，摘录）
if (!QCoreApplication::testAttribute(Qt::AA_DontUseNativeMenuBar))
    platformMenuBar = QGuiApplicationPrivate::platformTheme()->createPlatformMenuBar();

if (platformMenuBar)
    q->hide();
```

平台上谁会交货，笔者挨个翻过插件。macOS 恒给，QCocoaTheme 每次都 new 一只 QCocoaMenuBar，出处 qcocoatheme.mm 第 539 行。Android 同样恒给，出处 qandroidplatformtheme.cpp 第 400 行。Linux 上通用、KDE、GNOME 三套主题都要看 D-Bus 会话总线，找得到 com.canonical.AppMenu 服务才交出 QDBusMenuBar，出处 qgenericunixthemes.cpp 第 468-473 行，另两处同款。Windows 上 useNativeMenus() 不开就返 nullptr，出处 qwindowstheme.cpp 第 1211-1215 行，QApplication 程序默认不开。所以咱们平时在 Windows 和 Linux 桌面上看到的是 QMenuBar 自己画的部件，macOS 上您在窗口里根本找不到它，藏它的就是那句 q->hide()，窗口里的净高是 0。

净高 0 在尺寸函数里还有一层保险。sizeHint、minimumSizeHint、heightForWidth 三个尺寸函数开头都取同一个开关，as_gui_menubar = !isNativeMenuBar()，出处第 1578、1528、1629 行，原生菜单栏生效时条目几何整段跳过，尺寸从 (0,0) 起步，只有角部件能往上叠。setVisible 同款脾气（第 943-951 行），原生形态下您调 setVisible(true) 它直接 return，显隐归平台管，您叫不动它。

setNativeMenuBar 随时可以改主意，出处第 1783-1798 行。值没变它不动手，true 改 false 时 delete 掉平台菜单栏、置脏几何标记，有父窗口就 setVisible(true)，菜单栏当场在窗口里重新出现。false 改 true 时向主题再要一只，已经 addMenu 进去的项由 actionEvent 的镜像逻辑补同步（第 1190-1231 行），下一节咱们就读它。全局开关是 Qt::AA_DontUseNativeMenuBar，全文件只在 init 里判断一次（第 673 行），属性说明写明默认行为跟随它，您显式调过 setNativeMenuBar 之后，就以显式值为准，出处第 1767-1780 行。在不支持的平台上调这属性没有效果，读回去恒是 false，因为主题压根交不出平台菜单栏。

## 运行时增删：insertMenu、removeAction 与两只 clear 的不同下场

想把一只 QMenu 插进菜单栏中间，insertMenu（第 805-811 行）拿的是 menu->menuAction()，插到您指定的 before 动作之前，返回值就是 menuAction 本身，往后想挪位置，操作对象也是它。insertSeparator（第 783-793 行）同款路数。

removeAction 是 QWidget 层的通用接口，出处 qwidget.cpp 第 3215-3227 行，从动作表里摘掉，发一个 ActionRemoved 事件，不 delete。咱们跟着事件流走到 QMenuBar::actionEvent（第 1190-1241 行），几何标记当场置脏，原生形态下再镜像调 platformMenuBar->removeMenu（第 1210-1213 行），两边保持一致。ActionAdded 进门时还有一步接线，triggered 与 hovered 连到菜单栏自己的转发槽（第 1236-1239 行），菜单栏上的 triggered(QAction*) 信号，源头就是这些接线。

动作属性变化也走同一个门。ActionChanged 进门时，原生形态下查当前动作关联的平台菜单，跟它名下该挂的菜单对不对得上，对不上就 removeMenu 换成 insertMenu，对得上就把文本、可见、可用三样抄过去再 syncMenu（第 1214-1231 行，抄写动作在 copyActionToPlatformMenu，第 1177-1186 行）。您 setEnabled 一只顶层动作，窗口里它变灰，macOS 顶栏上同步变灰，跑的就是整条链。

两只 clear 的下场不同，咱们摆在一起看。QMenuBar::clear（第 846-852 行）只循环 removeAction，一只动作都不删，它们的 QObject 父亲仍是菜单栏，等析构才收走。QMenu::clear 就下手了：

```cpp
// QMenu::clear 摘之外还删，带条件（qmenu.cpp 第 2213-2225 行，摘录）
void QMenu::clear()
{
    QList<QAction*> acts = actions();

    for(int i = 0; i < acts.size(); i++) {
        removeAction(acts[i]);
        if (acts[i]->parent() == this && acts[i]->d_func()->associatedObjects.isEmpty())
            delete acts[i];
    }
}
```

您看判据有两个，parent 得是菜单本尊，而且它没出现在任何别的部件里。同一只 QAction 同时挂在工具栏上的，clear 之后还活着。危险的是您把动作指针长期存在 QMap 里做状态更新，菜单侧一次 clear，没有共享关系的指针全悬空。示例的做法是每次重建都 clear（menu_bar_demo.cpp 第 119 行），动作当场建当场用完就丢，没人长期持有，这个用法就安全。

## 装不下的项去哪儿：几何重算与溢出按钮

actionEvent 头一行 itemsDirty = true（第 1192 行），重算的活推迟到 updateGeometries（第 126-212 行），脏标记守门，不脏直接返回。calcActionRects（第 381-473 行）拿字体逐项量尺寸，不可见的项跳过，分给每只动作一个矩形。窗口压窄后 menuRect 装不下的项进 hiddenActions（第 178-196 行），整批塞进 extension 按钮自带的一只 QMenu（第 198-208 行），按钮亮在菜单栏右端。您把示例窗口拖窄，顶层菜单会自己躲进那枚箭头按钮，机制就在这里。重算还顺带把 shortcutIndexMap 重建一遍（第 162-172 行），Alt 加字母的助记符每次都重新 grab。

置脏的入口不止 actionEvent 一处，resizeEvent 同样把标记打脏（第 880-885 行），窗口宽度一变布局就得跟着来。您想拿到某只动作的矩形，公开接口 actionGeometry 返回的就是私有缓存里的矩形，出处第 1519-1524 行。绘制在 paintEvent，逐项交给风格画 CE_MenuBarItem（第 890-910 行），QMenuBar 自己不写字不画框，画法全听当前风格的，这也是同一份代码在各平台长相不同的来路。

## 角落部件与弹向：setCornerWidget、setDefaultUp

setCornerWidget（第 1713-1745 行）只认 TopLeftCorner 与 TopRightCorner，其余枚举值 qWarning 拒收（第 1728 行）。收养走 setParent(this)，再装事件过滤器（第 1732-1734 行）。换角部件时旧件只被摘掉过滤器，仍是菜单栏的可见子部件（属性说明第 1706-1711 行），它不会消失，只是丢了边角排位，收留或挪走得您自己动手。

几何上角部件全程参与。咱们看 updateGeometries 量它们的建议尺寸并扣掉对应宽度（第 133-153 行），sizeHint 再把高度叠进总尺寸（第 1601-1614 行）。上一篇读过 QMainWindow::setMenuBar，换栏时它会把旧栏两只角的部件搬去新栏，再销毁旧栏，出处 qmainwindow.cpp 第 482-506 行，角部件的寿命跨菜单栏，那层衔接正好对上。

setDefaultUp（第 865-873 行）管弹出方向。私有点 defaultPopDown 初值是 1（qmenubar_p.h 第 34-35 行），所以默认向下弹，isDefaultUp 返 false。您置 true 后，从菜单栏弹出的菜单改向上弹，屏幕放不下时自动换方向（属性说明第 851-864 行）。弹向的实算在 popupAction（第 272-336 行），fitUp 与 fitDown 拿菜单高度对屏幕上下边缘各量一遍，第 328-329 行的判据里 defaultPopDown 参与决策，两头都放不下时还会平移菜单。菜单长在内容下方的应用会需要它。

## 高亮与键盘导航：currentAction 走到哪，哪儿亮

咱们把三条路放在一起看，鼠标划过、键盘移动、菜单弹出，高亮都汇进同一个函数，QMenuBarPrivate::setCurrentAction，出处第 338 行。移动时的挑选逻辑在 getNextAction（第 688-716 行），它逐个看动作矩形，矩形为空的项（躲进溢出按钮的那些）跳过，禁用项默认也跳过，例外得靠风格提示放行，提示名叫 SH_Menu_AllowActiveAndDisabled。咱们回到公开接口，activeAction 与 setActiveAction 在第 816-837 行，后者转手就调 setCurrentAction。

鼠标离开也有收尾。leaveEvent（第 1141-1147 行）在菜单栏没有焦点又不在弹开状态时把高亮清空，唯一例外是当前项挂着菜单的场合，弹着的菜单不收。所以您鼠标扫过菜单栏再移开，高亮自己退场，不用您管。

keyPressEvent（第 1013-1055 行）咱们按按键走。Tab 折算成 Right（第 1024-1027 行），从右到左的布局里左右互换（第 1018-1023 行）。Enter、Return、Space、Down 在当前项有菜单时把它弹出（第 1031-1037 行），Left 与 Right 沿 getNextAction 挪动高亮（第 1048-1055 行），Esc 撤掉高亮，单个字符再去对助记符（第 1064 行起）。您按 Alt 再按 F 敲开 File 菜单，走的就是重算时重建的 shortcutIndexMap。

想用代码打开某只顶层菜单也有现成入口。setActiveAction 转手调 setCurrentAction 时把 popup 传了 true（第 827-837 行），后者一路走到 popupAction 把菜单弹开（第 367-368 行），所以一句 setActiveAction 就能替您完成高亮加弹出两件事。

## 最近文件列表：aboutToShow 重建，两条路径都通

aboutToShow 的发出有两条路，咱们都验过。部件路径在 QMenuPrivate::popup，摆位置之前 emit（qmenu.cpp 第 2304、2366 行）。原生路径靠转发，setPlatformMenu 把平台菜单的 aboutToShow 连到 _q_platformMenuAboutToShow（第 196 行），它再替 QMenu 把自己的 aboutToShow 发出去，出处第 1549-1551 行。macOS 侧的入口在 QCocoaNSMenu 的 menuWillOpen，NSMenu 一开就 emit platformMenu 的 aboutToShow 信号，出处 qcocoansmenu.mm 第 168-177 行。笔者特意核对了整条转发链，因为不少老资料断言 mac 上 aboutToShow 不可靠，6.9 的源码里转发链已经接通，您放心把重建逻辑挂在它上面。

示例的最近文件就是这么挂的，咱们顺着代码走一遍。Recent Files 子菜单出生在 setupMenus，出处 menu_bar_demo.cpp 第 44-46 行，aboutToShow 连到 rebuildRecentFilesMenu，对应第 117-135 行。重建时 clear（第 119 行），空列表给一只禁用的占位项（第 122-124 行），有列表就逐项建动作，带编号前缀与 setData（第 127-133 行），点击后 sender() 回取，再用 data 拿回路径（第 108-113 行）。入列在 openFile，removeAll 加 prepend 完成去重置顶（第 89-90 行），while 截到上限（第 93-94 行），上限 5 写在 menu_bar_demo.h 第 56 行。Edit 菜单里的 Add Dynamic Item 是运行时增删的最小现场（第 59-71 行），每点一次 lambda 里 addAction 追加一项，actionEvent 收到事件当场重算几何，菜单上的项肉眼可见地多起来。

咱们顺手问一句，为什么挂在 aboutToShow 上，不在每次 openFile 时实时维护清单？菜单绝大部分时间不可见，弹开那一刻照着数据重建一次，两边必然一致，还省掉了维护一份常驻动作表的力气，惰性更新是动态菜单的常规做法，状态灰显也能如法炮制。

持久化示例没做，咱们通常用 QSettings 存下整列路径，启动时读回，入列之后写回，细节属于 QSettings 自己的章节，本篇不铺开。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过。

[Qt 文档 · QMenuBar](https://doc.qt.io/qt-6/qmenubar.html) -- 菜单栏类，addMenu、insertMenu、setNativeMenuBar 与平台行为的定义处

[Qt 文档 · QMenu](https://doc.qt.io/qt-6/qmenu.html) -- 菜单类，addAction、removeAction、clear 与 aboutToShow 信号

[Qt 文档 · QAction](https://doc.qt.io/qt-6/qaction.html) -- 动作类，menuRole、enabled、visible 与触发语义

[Qt 文档 · QSettings](https://doc.qt.io/qt-6/qsettings.html) -- 持久化最近文件列表的常规去处

[Qt 文档 · Menus Example](https://doc.qt.io/qt-6/qtwidgets-mainwindows-menus-example.html) -- 官方菜单示例，完整的菜单构建流程

---

菜单栏读完，工具栏接戏。下一篇咱们去 [3.57 QToolBar 进阶](./57-qtoolbar-advanced.md)，响应式布局与溢出机制在那边等着，正好与本篇的溢出按钮对照着读。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/56-qmenubar-advanced/，可执行文件 56-qmenubar-advanced）。点 Open File 四五次再开 Recent Files，清单每次都是最新的，超过五条最旧的被挤掉，编号跟着重排。Edit 菜单里连点 Add Dynamic Item 十来次，清单再长也只撑开 Edit 自己的下拉，菜单栏顶层照旧三只。再把窗口拖到很窄，躲进右端箭头按钮的是顶层菜单本身：File、Edit、Help 谁装不下谁进溢出 QMenu，点开它能看到躲起来的顶层项，动态项长在 Edit 的 QMenu 里，永远轮不到这一层。给 Help 菜单加一只带 AboutRole 的关于项，Windows 上毫无动静，macOS 上它会归位到应用菜单，menuRole 的规则上一篇已经读过。

完成标准照旧。QMenu::clear 与 QMenuBar::clear 的差别落在哪两个判据上，您说得出。原生菜单栏下您为什么叫不动菜单栏的显隐，您指得出 setVisible 那几行。装不下的顶层菜单项去了哪儿，您讲得明。setDefaultUp 默认弹向哪边，您答得上。
