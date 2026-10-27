---
title: "3.64 QColorDialog 进阶"
description: "进阶篇从取值转入取色：getColor 内部用 QAutoPointer 包一只临时实例走 exec（qcolordialog.cpp 第 2199-2215 行），取消时的空颜色由 done 的拒绝分支写进 selectedQColor（第 2310-2312 行），currentColorChanged 与 colorSelected 的分工也在 done 那条链上（第 2307-2309 行），Option 选项集一共四枚且默认全灭、要在对话框亮相以前设好（qcolordialog.h 第 26-31 行、qcolordialog.cpp 第 2066-2072 行），NoButtons 会挖掉唯一的 accept 入口（第 1852-1856 行），原生颜色对话框这条路 Windows 压根不走（qwindowsdialoghelpers.cpp 第 42、2027 行），macOS 与 GTK 的助手都认 ShowAlphaChannel（qcocoacolordialoghelper.mm 第 371-383 行、qgtk3dialoghelpers.cpp 第 206-211 行），open 只接 colorSelected（第 2182-2189 行），吸管取色按 Esc 原路退回（第 2284-2287 行），十六格自定义颜色全局共享（qplatformdialoghelper.cpp 第 239-255 行），示例在 examples/advanced/03-qtwidgets/64-qcolordialog-advanced/"
---

# 现代Qt开发教程（进阶篇）3.64——QColorDialog 进阶

63 篇篇尾递过来的话是从取值转入取色，这一篇咱们接着看取色。主角 QColorDialog，笔者把 Qt 6.9.1 的 QColorDialog 源码（qcolordialog.cpp）从头翻到尾，翻完的第一手印象是：它的静态函数比 QInputDialog 那拨更薄，一行 getColor 背后就是一只临时实例加一次 exec，而所有有意思的东西，两根信号、四枚选项、吸管、色库，全得您自己构造实例才碰得到。

咱们把引用文件交代清楚。主战场是 qcolordialog.cpp 与 qcolordialog.h（src/widgets/dialogs），牵连的还有 qplatformdialoghelper.cpp 里的全局色库与各家平台助手。入门篇的 getColor 基本用法不重讲，63 篇讲过的 open 自动挑信号与断连不重讲，本篇只补 QColorDialog 自己的特性。配套示例是一块预览色板加九宫格预设色，完整玩法放结语。

## getColor：一行调用的底气，是一只临时实例加一次 exec

咱们直接看函数体（qcolordialog.cpp 第 2199-2215 行）。getColor 用 QAutoPointer 把 new 出来的对话框包住，标题非空才 setWindowTitle，接着 setOptions、setCurrentColor，然后 exec，收尾拿 selectedColor() 交差。默认标题不归它管，init 里拿 tr 造好的 Select Color，才是出厂标题（第 1692 行）。QAutoPointer 意味着函数一返回对话框就销毁，实例是一次性的，您想留都留不住。

```cpp
QAutoPointer<QColorDialog> dlg(new QColorDialog(parent));
if (!title.isEmpty())
    dlg->setWindowTitle(title);
dlg->setOptions(options);
dlg->setCurrentColor(initial);

// If the dlg was deleted with a parent window,
// dlg == nullptr after leaving the exec().
dlg->exec();
if (bool(dlg))
    return dlg->selectedColor();
else
    return QColor();
```

取消时拿到的空 QColor 也不是谁特判出来的。done 的拒绝分支把 selectedQColor 写成 QColor()（第 2310-2312 行），getColor 只管把这个空颜色递还给咱们。您要掂量的是 exec 本身：嵌套事件循环一开，画布就冻住了，绘图工具的用户选色时看不见画面，选错了只能关掉重来。这个结咱们留到讲信号时解开。

还有一条更硬的边界：静态函数把实例的生命周期整个包走，咱们连对话框本体都摸不到，currentColorChanged 自然无处可连。要实时预览，自己构造实例是唯一的路。

## 两根信号的分工：“正在挑”归 currentColorChanged，“定下了”归 colorSelected

两根信号在头文件里挨着（qcolordialog.h 第 66-68 行），咱们还看到 currentColor 兼任属性，NOTIFY 挂的，就是 currentColorChanged（第 21-22 行）。自绘路径里它由 QColorShower 的 updateQColor 发出，新旧颜色不同才发（qcolordialog.cpp 第 1483-1489 行），再由 initWidgets 里的一根 connect，转接到对话框自己的信号上（第 1840 行）。原生路径也有，initHelper 把平台助手的同名信号接进来，出处第 1866-1872 行。两条路汇进同一根信号，GTK 助手的 notify::rgba 回调，就是这么把拖动中的颜色实时递出来的。出处 qgtk3dialoghelpers.cpp 第 201-204 行。

colorSelected 冷淡得多。自绘路径全类只有一处发它，咱们顺着 done 的接受分支看。同一行里，selectedQColor 被填成 currentQColor()，出处 qcolordialog.cpp 第 2307-2309 行。文档注解把两者的差别写明白了：selectedColor 不总等于 currentColor，用户可以挑过好几个颜色，最后才定一个（第 2025-2031 行）。

这个差别正是实时预览出问题的地方。咱们连 currentColorChanged 把预览跟着拖，用户挑了半天点取消，done 走拒绝分支（第 2310-2312 行），selectedColor 是空，可咱们的预览早就变了样，画面停在人家没确认的颜色上。补救是把 rejected 也连上，把打开对话框那一刻的原色描回去。

```cpp
// 伪代码：实时预览加取消回退
QColor saved = canvas->color();
auto* dlg = new QColorDialog(saved, this);
dlg->setOption(QColorDialog::DontUseNativeDialog);
connect(dlg, &QColorDialog::currentColorChanged,
        canvas, &Canvas::setColor);   // 拖动时预览实时变
connect(dlg, &QDialog::rejected, canvas, [canvas, saved] {
    canvas->setColor(saved);          // 用户取消，描回原色
});
connect(dlg, &QColorDialog::colorSelected,
        canvas, &Canvas::setColor);   // 点了 OK，定稿
dlg->open();                          // 窗体模态，不阻塞
```

## Option 选项集：四枚开关，默认全灭，加晚了不算数

枚举本体在头文件（qcolordialog.h 第 26-31 行），咱们按位值排一张表，语义出处是枚举文档（qcolordialog.cpp 第 2103-2115 行）。

| 选项 | 官方语义与落点 |
|------|------|
| ShowAlphaChannel | 允许用户选 alpha 分量（第 2108 行），自绘路径落在 alpha 标签与数值框的显隐（第 2090、1093-1094 行） |
| NoButtons | 不显示 OK 与 Cancel，文档注明 Useful for “live dialogs”（第 2109 行） |
| DontUseNativeDialog | 用 Qt 自绘版本替换系统原生对话框（第 2111-2112 行） |
| NoEyeDropperButton | 藏掉吸管按钮，文档写明 Qt 6.6 加入（第 2110 行） |

属性文档另有两句叮嘱：默认所有选项关闭，选项应当在对话框显示以前设好，显示中途设了不保证立刻见效（第 2066-2072 行）。setOption 的实现是一个异或翻位（第 2045-2050 行），翻的还是整组 options，您以为只动一枚，底下走的是 setOptions。

NoButtons 的官方用意是 live dialog，可按钮一没，接受就没了入口。accept 只接在 OK 按钮的 clicked 上，reject 接在 Cancel 上（第 1852-1856 行），NoButtons 把整排按钮藏掉（第 2089 行），用户关窗走的就是拒绝分支，selectedColor 是空。所以您做 live dialog，正确读法是拿 currentColor() 当产出，colorSelected 在这套玩法里等不来。

还有一处您留意：setOptions 对界面动手的前提是非原生路径（第 2088-2093 行），原生路径上 options 由 initHelper 转交给平台助手（第 1874 行），四枚开关里哪几枚生效，就得看各家助手认几枚了，下一节咱们挨家数。

## 原生对话框：原生阵营比传说的窄，Windows 压根不走

默认态度写在 init 里，原话是 use the native dialog if possible，判据是平台助手造不造得出来（qcolordialog.cpp 第 1694-1695 行）。否决清单在 canBeNativeDialog（第 1915-1925 行）：AA_DontUseNativeDialogs、WA_DontShowOnScreen、DontUseNativeDialog 选项，任何一条命中就回到自绘。清单末尾还有一条拿类名比对识别子类（第 1928 行），您要是继承 QColorDialog，原生资格就没了。

真有助手的平台，笔者数下来是两家。Linux 这边，GTK3 平台主题出一只 QGtk3ColorDialogHelper，出处 qgtk3theme.cpp 第 208 行，它的 applyOptions 拿 ShowAlphaChannel，去调 gtk_color_chooser_set_use_alpha，出处 qgtk3dialoghelpers.cpp 第 206-211 行。macOS 的可可助手用系统共享的 NSColorPanel，show 里同样拿这枚选项去 setShowsAlpha，注释里还留着一条绕法说明，对着苹果 rdar 编号，出处 qcocoacolordialoghelper.mm 第 371-383 行。Windows 那边就有意思了，USE_NATIVE_COLOR_DIALOG，在源码里是一行被注释掉的测试宏，出处 qwindowsdialoghelpers.cpp 第 42 行，useHelper 的 ColorDialog 分支因此落空（第 2027 行），Qt 6.9 的 Windows 上，QColorDialog 永远是 Qt 自绘那版。网上流传的说法把 Windows 也算进原生阵营，还说原生对话框会忽略 alpha 选项，笔者拿 6.9.1 源码对过，两条都站不住：Windows 根本没有原生路径，GTK 与可可两家助手都明明白白给 alpha 转发。

原生与自绘之间还留着一条切换通道，咱们试试中途切换。对话框已经显示，补 DontUseNativeDialog 也来得及，setOptions 检测到这枚选项就把原生助手撤下，转手 initWidgets 造出自绘界面（qcolordialog.cpp 第 2084-2087 行）。反方向的交接也有讲究，setCustomColor 与 setStandardColor 的文档都注明，macOS 原生对话框上用不了它们，要用，就配 DontUseNativeDialog，两处文档在第 529-531 行与第 551-553 行。

## open()：接线只认 colorSelected，模态是窗体级

QColorDialog 自己的 open(QObject*, const char*) 是字符串式接法（qcolordialog.cpp 第 2182-2189 行）：connect 把 colorSelected 接到您给的 SLOT 宏槽上，槽签名第一个参数得是 QColor，然后记下收件人，调 QDialog::open()。断线是自动的，done 收尾时把这笔连接断开再清空记录（第 2314-2319 行）。

这里笔者要替 open 纠正一句流传的说法，63 篇顺嘴也叫过它非模态，这话一并收回：它不是非模态。QDialog::open 的实现，是把对话框调成 Qt::WindowModal，再 show（qdialog.cpp 第 509-526 行），调用线程不被阻塞，但父窗口照样被按住。想要一块不挡画布的自由取色面板，用 show() 配 NoButtons，生命周期您自己管。头文件里那行 using QDialog::open，出处 qcolordialog.h 第 50 行，把无参版本带了进来，两只 open 并存，别混。

## 吸管与十六格色库：全局共享的两样配角

吸管按钮不是处处有，咱们看它什么时候才会被造出来。initWidgets 里 supportsColorPicking() 过了关，才会造出按钮（qcolordialog.cpp 第 1746-1757 行），判据是平台能力 ScreenWindowGrabbing，或者取色服务（第 1908-1913 行）。按钮文字拿 tr 出来，是 Pick Screen Color（第 1901-1902 行）。挑色期间对话框抢下键盘，按钮整排变灰（第 1645、1651-1657 行），旁边还实时报着光标坐标与 Press ESC to cancel 的提示（第 1661-1665 行），Windows 上另立一只一像素的透明窗口，专门接点击（第 1702-1705 行）。退出的方式有两种。点鼠标或回车，取光标下的颜色（第 2273-2278、2289-2292 行）。按 Esc，原路退回进吸管模式以前选中的颜色（第 2284-2287 行）。NoEyeDropperButton 这枚选项，就是给吸管按钮的隐身开关（第 2091-2092 行）。

色库那头，咱们来数格子。自绘对话框左边的基本色六行八列，自定义色两行八列（第 1521-1525 行）。底层数据住在 QColorDialogStaticData 里，枚举值固定，CustomColorCount 是 16，StandardColorCount 是 6 乘 8，四十八格基本色开机就按 RGB 立方体填好，十六格自定义色全白等着（qplatformdialoghelper.cpp 第 239-255 行）。类文档把后果交代清楚：自定义颜色由所有颜色对话框共享，程序运行期内一直记得（qcolordialog.cpp 第 1956-1958 行）。咱们在设置页存过一色，画板里再开对话框，颜色还在原地。读写入口是一组静态函数，customCount 问格数，customColor 取色，setCustomColor 写色（qcolordialog.h 第 60-64 行）。界面上的 Add to Custom Colors 按钮也走这里，把当前颜色写进 nextCust 指的格子，再前进一步（qcolordialog.cpp 第 1882-1888 行）。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过。

[Qt 文档 · QColorDialog](https://doc.qt.io/qt-6/qcolordialog.html) -- 颜色对话框类，getColor 与两根信号的定义处

[Qt 文档 · QColor](https://doc.qt.io/qt-6/qcolor.html) -- 颜色类，isValid 与 HexArgb 十六进制输出

[Qt 文档 · QDialog](https://doc.qt.io/qt-6/qdialog.html) -- exec 与 open 的模态语义，本篇替 open 纠偏的依据

[Qt 文档 · QColorDialog::ColorDialogOption](https://doc.qt.io/qt-6/qcolordialog.html#ColorDialogOption-enum) -- 四枚选项的官方说明，NoEyeDropperButton 标注了 6.6

---

标准对话框的取色部分收队，下一站 [3.65 QFontDialog 进阶](./65-qfontdialog-advanced.md)。取字体的 QFontDialog 把本篇的路数整个继承下来，NoButtons 与 DontUseNativeDialog，两枚原样在列（qfontdialog.h 第 27-30 行），静态函数换成 getFont，取消的判定从 isValid 换成一只 ok 指针，那边等咱们。

想动手的您把示例跑起来（examples/advanced/03-qtwidgets/64-qcolordialog-advanced/，可执行文件 64-qcolordialog-advanced）。点九宫格里任一色块，预览区与下方的 Hex、RGB、Alpha 三行信息当场换新。再点 Custom Color...，对话框带着 alpha 的调入口出场。原因是示例给 getColor 传了 ShowAlphaChannel，出处 color_dialog_demo.cpp 第 81-83 行，咱们把 alpha 调到 0，预览色块的背景整个透掉，信息行的 Alpha 归零。选了半截点取消，面板保留原色，示例拿 isValid 把空颜色挡在外面（第 85-87 行）。再把第 83 行的选项参数删掉重新编译，alpha 的调入口当场消失，ShowAlphaChannel 的落点您亲眼对上。

完成标准照旧。getColor 取消时的空颜色在哪一行写进去的，您说得出。currentColorChanged 与 colorSelected 各自的发出条件，您讲得明。NoButtons 模式下为什么等不来 colorSelected，accept 挂在哪只按钮上，您数得清。Windows 走不走原生颜色对话框，判据在哪两行，您答得上。
