---
title: "3.65 QFontDialog 进阶"
description: "进阶篇顺着 60 篇点过的第 283 行 connect 走进字体对话框：非原生的 QFontDialog 是一台用 QFontDatabase 现场组装的字体浏览器（qfontdialog.cpp 第 136-299 行 init 全家福），当样张的 QLineEdit 就是 currentFont 的登记处（第 193、806 行），FontDialogOption 六枚两两成对当掩码使（qfontdialog.h 第 27-34 行），过滤执行点在 updateFamilies（qfontdialog.cpp 第 450-467 行），书写系统过滤是内置下拉框不是自建工程（第 196、223-229、651-656 行），getFont 取消时退还原字体（第 358-375 行），NoButtons 混进 getFont 会交回 ok 为真的空 QFont（第 367、993 行），fontSelected 只在 done 的 Accepted 岔发出（第 985-991 行），open 字符串连接关闭时自动断开（第 907-914、994-999 行），原生面板是 macOS 一家、过滤选项在那里可能失效（第 832-838 行），示例在 examples/advanced/03-qtwidgets/65-qfontdialog-advanced/"
---

# 现代Qt开发教程（进阶篇）3.65——QFontDialog 进阶

60 篇读 QDialog 与按钮盒的接线时，咱们点过一处细节，Qt 自家的 QFontDialog 在 qfontdialog.cpp 第 283 行手写了 connect(buttonBox, &QDialogButtonBox::accepted, q, &QDialog::accept)，本篇就顺着它往下读。论家门，QFontDialog 与 QColorDialog 是一路的，静态便捷函数加 Option 集，getFont 对应 getColor，FontDialogOption 对应那一套颜色选项。差别在数据形状：颜色是一组数值，字体是一叠元数据，family、style、size、删除线、下划线各占一格，所以 QFontDialog 肚子里装的是一台现场组装的小型字体浏览器，数据全部来自 QFontDatabase，拾色板在这里没有对应物。把它读明白，等宽过滤、书写系统过滤、getFont 的返回行为，入门篇没展开的问题全有出处。

引用文件咱们交代清楚。主战场是 Qt 6.9.1 的 QFontDialog 源码（qfontdialog.cpp 与 qfontdialog.h，src/widgets/dialogs），字体数据一侧还会进 qfontdatabase.cpp 与 qfontdatabase.h（src/gui/text）查证。入门篇的 getFont 基本用法不重讲，60 篇讲过的 exec 与 open 模态语义也不重讲。配套示例是一只静态函数与对话框对象两种拿字体方式并开的演示窗口，完整玩法放结语。

## 一台现场组装的字体浏览器：三张列表、两只勾选框、一条样张

咱们翻出生记录。私有类的 `init`（qfontdialog.cpp 第 136-299 行）把界面一次装齐：family 与 style 各配一只只读 QLineEdit 加一只 QFontListView，字号栏 sizeEdit 挂 QIntValidator(1, 512)，手输也出不了圈（第 168 行）。Effects 组里是删除线与下划线两只勾选框（第 179-184 行），Sample 组里一条居中的 QLineEdit 当样张，初始文字 “AaBbYyZz”，注释特意写明不走 tr()，因为这行字挑的就是字符本身（第 186-193 行）。窗口起手 500 乘 360（第 289 行），角落的拖拽手柄也开着（第 140 行）。

样张的地位比它看上去的高。currentFont() 在非原生路径上读的就是 sampleEdit->font()（第 798-807 行），咱们浏览时换 family、换 style、换字号，最后都汇到 updateSample 里拼一只新 QFont，QFontDatabase::font(family, style, pSize) 加上两只勾选框的删除线与下划线（第 625-636 行），再把字体设给样张。设字体时还有一道判断，新字体与样张手里的一样，currentFontChanged 一声不吭，不同才发（第 639-646 行）。

按钮盒的接线就是 60 篇点过的那两行。init 造出 QDialogButtonBox（第 278-279 行），加 Ok 之后写 connect(buttonBox, &QDialogButtonBox::accepted, q, &QDialog::accept)（第 283 行），Ok 顺手设成默认钮（第 284 行），再加 Cancel，把 rejected 接到 reject（第 286-287 行）。键盘行为归 eventFilter 管：光标在 sizeEdit 里按上下与翻页键，按键事件被转交给字号列表，列表替您翻（第 389-401 行），光标在 family 或 style 列表里按回车，对话框当场 accept 关闭（第 402-407 行）。

样式列表还有一处换词重试，笔者读着觉得贴心。setCurrentFont 带来的样式名在列表里找不到时，updateStyles 拿名字做替换再搜一轮，Italic 换 Oblique、Regular 换 Normal，四个方向各试一遍（第 550-568 行）。字体厂起名没有统一标准，Qt 用四个替换把常见别名兜住。

## MonospacedFonts 不是魔法：updateFamilies 里的两对掩码

FontDialogOption 全集六枚，值在 qfontdialog.h 第 27-34 行：NoButtons 0x00000001，DontUseNativeDialog 0x00000002，ScalableFonts 0x00000004，NonScalableFonts 0x00000008，MonospacedFonts 0x00000010，ProportionalFonts 0x00000020。笔者提醒您留意 NonScalableFonts，本篇源稿只数了五枚，显示不可缩放字体的那一枚被漏了。

过滤的执行点在 updateFamilies（qfontdialog.cpp 第 444-521 行）。函数进门把选项分成两对掩码，scalableMask 是 ScalableFonts 加 NonScalableFonts，spacingMask 是 ProportionalFonts 加 MonospacedFonts（第 450-451 行），接着遍历 QFontDatabase::families(writingSystem)，拿全量字体族（第 455 行）。掩码只亮一位时才动过滤：ScalableFonts 单亮，isSmoothlyScalable 不达标的跳过，MonospacedFonts 单亮，isFixedPitch 不达标的跳过（第 460-467 行）。两位一起亮等于掩码全开，过滤整段跳过，咱们想恢复全量列表，把选项清空比凑齐两位更直白。isPrivateFamily 标记的私有字体族一律不进列表（第 457-458 行），isFixedPitch 的实现在 qfontdatabase.cpp 第 1533-1546 行，读的是字体库里每个 family 自带的 fixedPitch 位。

列表装好后咱们再看一道兜底匹配。setCurrentFont 带进来的字体族在过滤后的列表里选不中时，updateFamilies 按 family 同名、foundry 同名逐级让步，再拿 helvetica 与应用当前字体垫底（第 484-514 行），保证列表里总有一项亮着。

平台边界咱们按文档读。枚举文档写明过滤选项可能在个别平台不被支持，文档举的例子是 Mac，而 Windows 与 Linux 上走的非原生对话框，回回都支持（qfontdialog.cpp 第 832-834 行），DontUseNativeDialog 的说明也只针对 Mac 上的 Apple 字体面板，别的平台没提（第 837-838 行）。本篇源稿说 Windows 原生对话框会静默忽略 MonospacedFonts，还配了一道 Windows 上的调试题，笔者在源码与文档里都找不到支撑，Windows 与 Linux 走的本来就是非原生实现，六枚选项原样生效，那道题不成立。

设置时机也有一条，您记一下。options 的属性文档写明要在显示前设置，显示中途改不保证当场生效（第 878-880 行）。setOptions 里顺手管了一件事，NoButtons 亮起时按钮盒整个藏掉（第 892 行）。

## 书写系统过滤是内置的：左下角的下拉框已经替咱们装好了

源稿用了半节讲想按支持中文过滤就得自己拿 QFontDatabase 搭面板，咱们翻 init 能翻出反例。非原生 QFontDialog 的左下角，自带一只 Writing System 下拉框（qfontdialog.cpp 第 196 行），init 遍历 QFontDatabase::WritingSystem 枚举，拿 writingSystemName 逐项填进去，名字为空就停（第 223-229 行）。私有类构造时把书写系统默认成 QFontDatabase::Any（第 76-77 行），正对下拉框的第一项。

选中哪一项，writingSystemHighlighted 接手：下拉框序号直接转成 WritingSystem（第 651-653 行），样张文字换成 writingSystemSample 给那套文字备的样张（第 654 行），再重跑 updateFamilies，families(writingSystem) 换了参数，字体族列表当场收窄（第 455、655 行）。咱们想只看支持简体中文的字体，在下拉框里挑中简体中文那一项就完事，一行自建代码都不用写。同一个查询咱们代码里也能直接用，QFontDatabase::families 是 public static，定义在 qfontdatabase.h 第 85 行。咱们写一行 QFontDatabase::families(QFontDatabase::SimplifiedChinese)，拿到的列表与对话框内部用的是同一份。

样张文字有个笔者对着源码数过才知道的小机关。init 摆的样张是 “AaBbYyZz”（qfontdialog.cpp 第 193 行），writingSystemSample 给 Any 与 Symbol 备的样张，却是 “AaBbzZ”（qfontdatabase.cpp 第 2091-2094 行）。咱们把下拉框切到 Cyrillic 再切回 Any，样张就从 AaBbYyZz 变成 AaBbzZ，没有谁手滑，切换前后样张来自两个不同的赋值来源。

边界也提醒您一句。书写系统下拉框是非原生实现的东西，macOS 的原生字体面板上没有它，六枚过滤选项同理（qfontdialog.cpp 第 832-838 行）。

## getFont 的返回链：ok 为真不等于用户做了选择

两只静态重载在 qfontdialog.h 第 58-60 行，带 initial 与不带，不带的内部用默认构造的 QFont 当 initial，出处 qfontdialog.cpp 第 352-356 行。私有实现共用一套（第 358-375 行）：QAutoPointer 拿住对话框（第 361 行），setOptions、setCurrentFont、设标题挨个做完再 exec（第 362-367 行），ok 指针非空就按 exec 的结果填写（第 368-369 行），结果为真返回 selectedFont()，为假返回 initial（第 370-374 行）。用户取消时，带 initial 的重载退回 initial，不带的退回 Qt 默认字体，两只重载的文档注释各自写明（第 313-320、341-347 行），咱们要紧的还是把 ok 标志查到位，别把取消交回的原字体当新选择用。

咱们停在 done 的两岔（第 982-1001 行）。Accepted 岔上 selectedFont 存下来、fontSelected 发出（第 985-991 行），Rejected 岔上，selectedFont 被清成默认构造的 QFont，值是空的（第 993 行）。把返回判定与 Rejected 岔连起来读，会看出一条怪链，源码咱们原样摘下来。

```cpp
// Qt 6.9.1 的 QFontDialog 源码（qfontdialog.cpp）
int ret = (dlg->exec() || (options & QFontDialog::NoButtons));
if (ok)
    *ok = !!ret;
if (ret && bool(dlg)) {
    return dlg->selectedFont();
} else {
    return initial;
}
```

咱们把 NoButtons 传给 getFont 会怎样。按钮盒被藏掉（第 892 行），Esc 与关窗都走 Rejected，selectedFont 随之清成默认 QFont（第 993 行）。回车不受影响，光标在 family 或 style 列表按回车照样 accept，init 末尾还把初始焦点给了 familyList，出处 setFocus 第 296 行。可摘录里第一行的 || 把 NoButtons 位当真值顶上去，exec 返回 0 也被顶成真，*ok 报 true，返回条件同样放行，交回一只默认构造的空 QFont。ok 为真、字体为空，两头都不能信。NoButtons 的正业写在枚举文档里，是给 open() 的活对话框用的（第 836 行），别把它混进 getFont。

## 两只信号分工与 open 的自动接线

currentFontChanged 是浏览态信号，family、style、字号、勾选框任何一样动了都汇进 updateSample（第 661-708 行），样张字体真变了才发（第 642-645 行），用户按住上下键扫列表时它是高频的，信号文档也提醒，选择过程中的字体可能与最终选择不同（第 916-926 行）。fontSelected 是定稿信号，只在 done 的 Accepted 岔发出（第 991 行），发出之前若最终字体与样张手里的字体不同，咱们还能看到一次补发的 currentFontChanged，出处第 987-989 行。咱们接实时预览挂 currentFontChanged，接定稿动作挂 fontSelected，别混。selectedFont() 与 currentFont() 的差别文档写得直白，用户可能反复换字体后挑定一只，两者不一定相同（第 809-815 行）。

`open(QObject *receiver, const char *member)`（qfontdialog.h 第 53-54 行）是 QFontDialog 自己补的便捷口。60 篇讲过 QDialog 的 Qt 5 字符串式 open 在 6.9 头文件里已经没了，字体对话框这边留了一份，实现里拿 SIGNAL 宏做连接，咱们自己写的槽接上 fontSelected（qfontdialog.cpp 第 907-914 行），连接对象存进 receiverToDisconnectOnClose，done 收尾时自动断开（第 994-999 行）。NoButtons 加 open 加 currentFontChanged，一只不落地的浮动字体面板就靠三样拼出来。

原生路径咱们接着读（第 1003-1017 行）。canBeNativeDialog 的否决清单，咱们挨个数：全局属性 AA_DontUseNativeDialogs、控件属性 WA_DontShowOnScreen、选项 DontUseNativeDialog，任一命中就否决（第 1010-1013 行），子类被类名比对排除在外（第 1016 行）。真走原生时私有类的 setVisible 给自己盖上 WA_DontShowOnScreen，控件版整棵不显示（第 957-971 行），平台助手的两只同名信号在 initHelper 里桥接到 QFontDialog 自己的信号上（第 428-431 行），窗口标题由 helperPrepareShow 转交，出处第 434-437 行。

## 想往列表里塞自己的字体集：Qt 没留口子

qfontdialog.h 的公开面笔者逐行数过（第 39-60 行），setCurrentFont、currentFont、selectedFont、setOption 这些加两只 getFont，没有 setFamilies 或同类的注入 API。updateFamilies 的注释倒是许了愿，说有特殊需求可以重实现（原文 can be reimplemented，qfontdialog.cpp 第 439-442 行），可它是 QFontDialogPrivate 的函数，咱们在应用侧够不着。想按自己的规则筛字体，出路是拿 QFontDatabase 自搭选择面板，查询面全是 public static，families、styles、pointSizes 这些，加上 isSmoothlyScalable 与 isFixedPitch，都在 qfontdatabase.h 第 85-97 行。Qt 自家对话框调的与咱们能调的是同一个库，自搭面板不缺数据，缺的只是三张列表的组装工夫，照 init 的骨架拼一遍就有。

示例还给了一条省事的路。咱们选中字体之后，用 QFontInfo 读实际匹配结果，出处 font_dialog_demo.cpp 第 111-112 行，QFontDialog 交回的 QFont 是请求值，QFontInfo 报的是 Qt 实际匹配到的字体，family 对不上时它当场能看出来。信息标签里的 Available styles 清单，就是拿 QFontDatabase::styles 补的，出处第 114-115 行。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过。

[Qt 文档 · QFontDialog](https://doc.qt.io/qt-6/qfontdialog.html) -- 字体对话框类，getFont、选项集与两只信号的定义处

[Qt 文档 · QFontDialog::FontDialogOption](https://doc.qt.io/qt-6/qfontdialog.html#FontDialogOption-enum) -- 六枚选项的取值与平台支持说明

[Qt 文档 · QFontDatabase](https://doc.qt.io/qt-6/qfontdatabase.html) -- 字体查询库，families、isFixedPitch 与书写系统接口的老家

[Qt 文档 · QFontInfo](https://doc.qt.io/qt-6/qfontinfo.html) -- 实际匹配字体的查询，请求与到手的差别它说了算

---

对话框一家咱们接着走，下一站 [3.66 QFileDialog 进阶](./66-qfiledialog-advanced.md)。QFileDialog 是标准对话框里块头最大的一只，Option 表比本篇六枚还多一枚，原生与非原生的分岔也更深，getFont 式的静态便捷函数，咱们到那边见最后一面。

想动手的您把示例跑起来（examples/advanced/03-qtwidgets/65-qfontdialog-advanced/，可执行文件 65-qfontdialog-advanced）。点 Pick Font (Static Method)，弹出标题 Select Font 的对话框，选一只字体确认，下方预览区换成新字体，信息标签报出 Family、Style、Size、Weight，还有该字体族的可用样式清单（font_dialog_demo.cpp 第 114-127 行），咱们改点 Cancel，预览区纹丝不动，ok 为假时示例不应用字体（第 78-80 行）。勾上 Monospace fonts only 再点 Pick Font (Dialog Object)，字体族列表收窄到等宽字体（第 94 行），不勾就是全量，确认后应用的是 currentFont()（第 97-98 行）。对话框左下角的 Writing System 下拉咱们切到 Cyrillic 试试，字体族列表当场换一批，样张文字换成西里尔字符，再切回 Any，样张变成 AaBbzZ，与初始的 AaBbYyZz 不同，两个来源的差别您现在对得上号。光标停在字体族或样式列表里按回车，对话框直接接受并关闭（qfontdialog.cpp 第 402-407 行）。

完成标准照旧。六枚选项哪两对组成掩码，两位同亮时过滤为什么整段跳过，您说得出。NoButtons 混进 getFont 交回什么，您讲得明。书写系统过滤在界面哪一处、走哪两个函数，您指得出。currentFontChanged 与 fontSelected 各在哪个时机发出，您数得清。
