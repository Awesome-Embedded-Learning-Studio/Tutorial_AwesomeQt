---
title: "3.61 QDialogButtonBox 进阶"
description: "进阶篇接住 60 篇交棒看按钮盒本体：标准按钮的角色映射定在 QPlatformDialogHelper::buttonRole（qplatformdialoghelper.cpp 第 946-987 行），Retry 与 Ignore 都算确认类落 AcceptRole，Abort 算拒绝类，Discard 独占 DestructiveRole，排列顺序由样式提示 SH_DialogButtonLayout 选平台队列（qdialogbuttonbox.cpp 第 141 行），Win 与 Kde 两副队列（qplatformdialoghelper.cpp 第 50-70 行），Mac 缺模态角色时切 modeless 排法（qdialogbuttonbox.cpp 第 195-218 行），点击时 clicked 打头，便利信号按角色补发 accepted 与 rejected，YesRole 也补 accepted（第 872-903 行），Action、Apply、Reset、Destructive 四类只有 clicked，removeButton 只摘不删（第 716-720 行），clear 才整排删（第 644-658 行），藏起来的按钮角色记在 hiddenButtons（第 914-945 行），显示时第一只 AcceptRole 按钮自动设为默认（第 998-1044 行），centerButtons 默认 false（第 118 行），示例在 examples/advanced/03-qtwidgets/61-qdialogbuttonbox-advanced/"
---

# 现代Qt开发教程（进阶篇）3.61——QDialogButtonBox 进阶

60 篇收尾时留了一句话，按钮盒不会替咱们接线，本篇就顺着这句话把 QDialogButtonBox 本体读完。咱们关心三件事，一排按钮的左右顺序谁说了算，点下去的信号从哪发出来，默认按钮怎么悄悄上位。笔者的读法照旧，把 Qt 6.9.1 的 QDialogButtonBox 源码，也就是 qdialogbuttonbox.cpp，共 1077 行，从头读到尾，再摸到 QtGui 一侧的角色映射表。

引用文件咱们交代清楚。主战场是 qdialogbuttonbox.cpp 与 qdialogbuttonbox.h（src/widgets/widgets），私有结构下探到 qdialogbuttonbox_p.h。标准按钮到角色的映射表不在本文件里，它住在 QtGui 一侧的 QPlatformDialogHelper 源码里，文件名 qplatformdialoghelper.cpp。配套示例是一只设置对话框，Ok 与 Cancel 两只标准按钮加 Help 与 Apply 两只自定义按钮，完整玩法放结语。

## 角色表是按钮盒的户口，标准按钮只是快捷方式

咱们从户口数起。ButtonRole 在 qdialogbuttonbox.h 第 27-41 行，一共十个值。第 28 行有条注释，要求它与 QMessageBox 的同名枚举，以及 QPlatformDialogHelper 的同名枚举，保持同步。每个角色点击后拿到什么信号，咱们整理成表，判定出处是点击分发函数的 switch，qdialogbuttonbox.cpp 第 888-901 行。

| 角色 | 点击后的便利信号 | 语义 |
|------|------|------|
| InvalidRole（-1） | 无 | 查不到角色时的返回值 |
| AcceptRole | accepted | 确认并收下结果 |
| RejectRole | rejected | 拒绝并放弃结果 |
| DestructiveRole | 无，只有 clicked | 破坏性操作，如放弃更改 |
| ActionRole | 无，只有 clicked | 不改变对话框状态的业务动作 |
| HelpRole | helpRequested | 请求帮助 |
| YesRole | accepted | 肯定回答 |
| NoRole | rejected | 否定回答 |
| ResetRole | 无，只有 clicked | 重置表单到初始值 |
| ApplyRole | 无，只有 clicked | 应用当前设置但不关窗 |

标准按钮是角色表的快捷方式。十八个值定义在 qdialogbuttonbox.h 第 43-69 行，每个值落到哪个角色，由 QtGui 一侧的 switch 决定，函数名 QPlatformDialogHelper::buttonRole，出处 qplatformdialoghelper.cpp 第 946-987 行。咱们逐行核对后按角色归堆列出。

| 标准按钮 | 值 | 所属角色 |
|------|------|------|
| Ok | 0x00000400 | AcceptRole |
| Save | 0x00000800 | AcceptRole |
| SaveAll | 0x00001000 | AcceptRole |
| Open | 0x00002000 | AcceptRole |
| Retry | 0x00080000 | AcceptRole |
| Ignore | 0x00100000 | AcceptRole |
| Yes | 0x00004000 | YesRole |
| YesToAll | 0x00008000 | YesRole |
| No | 0x00010000 | NoRole |
| NoToAll | 0x00020000 | NoRole |
| Cancel | 0x00400000 | RejectRole |
| Close | 0x00200000 | RejectRole |
| Abort | 0x00040000 | RejectRole |
| Discard | 0x00800000 | DestructiveRole |
| Help | 0x01000000 | HelpRole |
| Apply | 0x02000000 | ApplyRole |
| Reset | 0x04000000 | ResetRole |
| RestoreDefaults | 0x08000000 | ResetRole |

表里有两处反直觉，咱们停下多看一眼。Retry 与 Ignore 是确认类，和 Ok 同住 AcceptRole，出处 qplatformdialoghelper.cpp 第 949-955 行。Abort 是拒绝类，和 Cancel 同住 RejectRole，出处第 957-960 行。您给错误处理对话框配一只 Retry，用户点下去拿到的是 accepted，不存在专用的重试信号，别在信号列表里找。

标准按钮的文案也不是写死的字符串。createButton 造按钮时，qdialogbuttonbox.cpp 第 306-389 行，文字取自平台主题的 standardButtonText，第 371 行，图标看样式开关 SH_DialogButtonBox_ButtonsHaveIcons，第 373-374 行。连快捷键，都向平台主题要一份 standardButtonShortcut，出处第 384-386 行。所以同一只 Ok 在中文环境跑出来就是中文文案，语言切换事件到来时 retranslateStrings 挨个重取，出处第 427-434 行，触发点在 event 的 LanguageChange 分支，第 1065-1067 行。构造函数有四个，咱们数过，qdialogbuttonbox.h 第 83-87 行，带 StandardButtons 的重载最终都汇到 createStandardButtons，按位从 FirstButton 扫到 LastButton 逐个造，出处 qdialogbuttonbox.cpp 第 417-425 行。

## 顺序是平台说了算，角色只管排队

同一份代码，Ok 在 Windows 上站在 Cancel 左边，到 macOS 上就站到右边，这不是谁的失误。按钮盒初始化布局时，向样式要一个 SH_DialogButtonLayout，initLayout 里，qdialogbuttonbox.cpp 第 141 行。拿到的是五选一，WinLayout、MacLayout、KdeLayout、GnomeLayout、AndroidLayout，枚举在 qdialogbuttonbox.h 第 74-81 行。真正用来排队的角色顺序数组住在 QtGui 一侧，qplatformdialoghelper.cpp 第 50-85 行的 buttonRoleLayouts。咱们把水平方向的两副队列原文摆出来对比。

```cpp
// WinLayout（qplatformdialoghelper.cpp 第 55-58 行，摘录）
{ QPlatformDialogHelper::ResetRole, QPlatformDialogHelper::Stretch, QPlatformDialogHelper::YesRole, QPlatformDialogHelper::AcceptRole,
  QPlatformDialogHelper::AlternateRole, QPlatformDialogHelper::DestructiveRole, QPlatformDialogHelper::NoRole,
  QPlatformDialogHelper::ActionRole, QPlatformDialogHelper::RejectRole, QPlatformDialogHelper::ApplyRole,
  QPlatformDialogHelper::HelpRole, QPlatformDialogHelper::EOL, QPlatformDialogHelper::EOL, QPlatformDialogHelper::EOL },

// KdeLayout（qplatformdialoghelper.cpp 第 68-70 行，摘录）
{ QPlatformDialogHelper::HelpRole, QPlatformDialogHelper::ResetRole, QPlatformDialogHelper::Stretch, QPlatformDialogHelper::YesRole,
  QPlatformDialogHelper::NoRole, QPlatformDialogHelper::ActionRole, QPlatformDialogHelper::AcceptRole, QPlatformDialogHelper::AlternateRole,
  QPlatformDialogHelper::ApplyRole, QPlatformDialogHelper::DestructiveRole, QPlatformDialogHelper::RejectRole, QPlatformDialogHelper::EOL },
```

Win 队列把 Reset 打头，Accept 后面紧跟着 Alternate，Help 压尾。Kde 队列正相反，Help 与 Reset 站最左，中间一道 Stretch 弹簧，后面的角色从 Yes 到 Reject 依次向右排开，完整名单在上面的代码块里。样式提示返回 UnknownLayout 时，兜底逻辑按编译目标选队列，Unix 落 KdeLayout，出处 qplatformdialoghelper.cpp 第 991-1000 行，咱们在 Linux 桌面看到的按钮位置走的就是它。您若拿着 Windows 风格的设计稿验收 Linux 成品，Help 的位置就对不上，代码没写错，队列本来就有两副。

layoutButtons 负责照队列逐格摆按钮，出处 qdialogbuttonbox.cpp 第 187-304 行。咱们摆的过程中有三处细节值得停一下。

AcceptRole 格位只摆头一只，第二只起进 AlternateRole 格，出处第 234-245 行。您把 Ok 与 Save 放进同一个按钮盒，两只都是 AcceptRole，排队时 Ok 进正槽，Save 落在 Alternate 的位置。Win 队列里 Accept 与 Alternate 两格相邻，正是为 Ok 加 Save 的搭配准备的。

AcceptRole 一只都没有时，格位空着直接跳过，出处第 234-236 行的 isEmpty 检查。咱们只配 Help 与 Cancel 时，版面不会留出空位。

MacLayout 还有个变档，咱们接着看。layoutButtons 开头检查 Accept、Reject、Destructive、Yes、No 五类模态角色是否全部缺席，全不在场时队列临时换成第 4 号，也就是 Mac 的 modeless 排法，出处第 195-218 行，tmpPolicy 赋值 4。同一副 Mac 队列还自带间距常量，36 减 8，防止 Discard 这类破坏性按钮贴上邻居，出处第 190、246-271 行。

设计上咱们把角色给对就行，顺序交给平台。真要全平台统一顺序，SH_DialogButtonLayout 是从 style() 取的样式提示，第 141 行，自定义样式重写它就能换队列。笔者没在生产环境验证过，只告诉您机制存在，不敢替您下断言。

## 点击分发：clicked 打头，便利信号按角色补发

点击分发的源码，咱们直接摆出来。函数名 handleButtonClicked，出处 qdialogbuttonbox.cpp 第 872-903 行。

```cpp
// 点击分发（qdialogbuttonbox.cpp 第 880-891 行，摘录）
const QDialogButtonBox::ButtonRole buttonRole = q->buttonRole(button);
QPointer<QDialogButtonBox> guard(q);

emit q->clicked(button);

if (!guard)
    return;

switch (QPlatformDialogHelper::ButtonRole(buttonRole)) {
case QPlatformDialogHelper::AcceptRole:
case QPlatformDialogHelper::YesRole:
    emit q->accepted();
    break;
```

clicked 打头发出，便利信号随后按角色补发。YesRole 与 AcceptRole 一起走 accepted，出处第 888-892 行，NoRole 与 RejectRole 一起走 rejected，第 893-896 行，HelpRole 走 helpRequested，第 897-898 行。switch 的 default 分支什么都不补，出处第 900-901 行。所以 DestructiveRole、ActionRole、ApplyRole、ResetRole 四类按钮点下去只有 clicked 一个信号，您想接住 Apply 就必须连 clicked，再拿 buttonRole 或 standardButton 分辨，看被点的到底是哪一只。

有一段注释咱们别滑过去，第 876-879 行。角色必须在发 clicked 前取好，因为 clicked 的槽里可能把按钮删掉或者挪走角色，等补发便利信号时就查不着了。中间的 QPointer 守卫同理，第 881-885 行，槽里若把按钮盒整个销毁，后面的补发直接取消。

60 篇留下一句判断，说按钮盒不替任何人接线，咱们在这里把证据补齐。按钮盒不会调用 QDialog 的 accept 或 reject，它只发自己的三个便利信号，对话框那头的接线要您自己连。配套示例里点 Ok 能关窗，走的是示例在 clicked 槽的 AcceptRole 分支里自己调用的 accept()，没有按钮盒代劳。想少写代码，您拿 connect 把 accepted 接到 QDialog::accept，两行完事，但接线这步仍归您写。

buttonRole 的实现朴素，第 698-709 行，挨个角色队列扫过去，扫到就返回，藏起来的按钮去 hiddenButtons 备份表里查。咱们接着就讲藏起来是怎么回事。

## addButton 与 removeButton：接手、换队、只摘不删

addButton 有三个重载，各管一摊，出处 qdialogbuttonbox.h 第 93-95 行。文字版 addButton(text, role) 造一只新 QPushButton 归按钮盒所有，指针还给您，图标与提示随您设置，出处 qdialogbuttonbox.cpp 第 790-802 行。指针版 addButton(button, role) 接手现成按钮，进门做一次 removeButton 再重新入队，同一只按钮想换角色不必拆盒重装，出处第 771-781 行。角色值越界时，文字版与指针版都会 qWarning 然后拒收，出处第 774-777、793-796 行。

标准按钮版 addButton(StandardButton) 直通 createButton，出处第 808-813 行，这里没有查重。构造时已带 Ok，再调一次 addButton(Ok)，盒里就有两只文案相同的按钮，都进 AcceptRole 队列，点哪只都发 accepted。您想改 Ok 的文案，正确姿势是拿 button(Ok) 取出现成按钮，再 setText，出处第 849-858 行。button() 取不到时返回 `nullptr`，对没造过的按钮直接 setEnabled 就是空指针崩溃，您判空的一步省不得。传 NoButton 也走 `nullptr` 返回，出处 createButton 第 367-369 行。

removeButton 与 clear 一个只摘不删，一个整排删。removeButton 手动移除，按钮不删，setParent(nullptr) 加断开信号，按钮交还给您自己管，出处第 712、733-750 行。clear 才整排 delete，出处第 644-658 行。还有第三种退出方式是隐藏，构造函数装了一只事件过滤器，第 119-135 行，按钮收到 HideToParent 时退出角色队列，角色记进 hiddenButtons，ShowToParent 时按记录回队，出处第 923-938 行。所以 setVisible(false) 只让按钮离队待命，buttonRole 照样答得出它的角色，出处第 708 行。

## 默认按钮：头一只 AcceptRole 自动上位

类文档写明了规则，qdialogbuttonbox.cpp 第 108-113 行。您没设默认按钮时，按钮盒显示时会把头一只 AcceptRole 按钮设为默认，这么做是为了跨平台保住同一个默认身份。实现在 ensureFirstAcceptIsDefault，出处第 998-1044 行。咱们找到两个触发点，一个是按钮盒自己的 Show 事件，第 1061-1063 行，另一个是藏起来的 AcceptRole 按钮重新显示，出处第 936-938 行。

它动手前还查一遍在场名单。从按钮盒向上找到最近的 QDialog，出处第 1011-1016 行，再遍历整个对话框的 QPushButton，发现已有别的按钮被设了 default，就让位不抢，出处第 1021-1027 行。您想让 Cancel 当默认，您自己动手调它的 setDefault(true)，上位逻辑会让路。设置只有一行。

```cpp
// 头一只 AcceptRole 按钮设为默认（qdialogbuttonbox.cpp 第 1029 行，摘录）
firstAcceptButton->setDefault(true);
```

后面第 1031-1043 行的注释解释了一段焦点公案。对话框里的 QPushButton 默认 autoDefault 为真，焦点一进按钮盒就会被头一只按钮抢走默认身份，源码的对策是连 setFocus 一起安排。咱们读这段注释能带走的是，autoDefault 抢焦点是真的，遇到焦点莫名跳到按钮上的现象，到这里找答案。排版收尾还有一条链子，layoutButtons 按版面顺序挨个 setTabOrder，并把默认按钮设为按钮盒的焦点代理，出处第 285-299 行。

## 方向、居中与换装

setOrientation 传同方向直接返回，换方向才 resetLayout 重建布局对象，QHBoxLayout 与 QVBoxLayout 就地互换，出处第 629-637 行，initLayout 第 142-150 行。垂直按钮盒咱们平时少用，向导侧栏这类场景才出场，排队逻辑不变，数组换垂直那副，qplatformdialoghelper.cpp 第 88 行起。

居中开关咱们翻过来看。centerButtons 默认 false，构造函数里 center(false)，第 118 行，文档第 949-952 行说消息框才常居中。开了之后版面首尾各加一道 stretch，出处第 220-221、302-303 行，setCenterButtons 值没变不动作，变了就重排，出处第 957-964 行。

setStandardButtons 是推倒重来，旧的整排销毁，新的从头造，出处第 823-832 行。运行时换按钮组前，您手里存的旧 button(Ok) 指针全部作废，要重新取。样式变化走 changeEvent 的 StyleChange 分支，新样式传播给每只标准按钮并重排，出处第 973-991 行。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过。

[Qt 文档 · QDialogButtonBox](https://doc.qt.io/qt-6/qdialogbuttonbox.html) -- 按钮盒类，StandardButton 与 ButtonRole 的定义处

[Qt 文档 · QDialog](https://doc.qt.io/qt-6/qdialog.html) -- accept 与 reject，60 篇的主战场

[Qt 文档 · QStyle](https://doc.qt.io/qt-6/qstyle.html) -- SH_DialogButtonLayout 等样式提示的查询入口

[Qt 文档 · QMessageBox](https://doc.qt.io/qt-6/qmessagebox.html) -- 消息框类，下一站的主角

---

对话框三件套还差消息框，下一站 [3.62 QMessageBox 进阶](./62-qmessagebox-advanced.md)。它家的自定义按钮用的还是这套 ButtonRole，qdialogbuttonbox.h 第 28 行的同步注释就是两边关系的凭证，那边咱们看消息框把这套角色用出什么花样。

想动手的您把示例跑起来，examples/advanced/03-qtwidgets/61-qdialogbuttonbox-advanced/，可执行文件 61-qdialogbuttonbox-advanced。点 Help 弹一只说明框，对话框不关，点 Apply 底部状态行变蓝提示 Applied，窗也不关，点 Ok 状态行变绿随即关窗。再做两个实验。把 settings_dialog.cpp 里 `addButton(tr("&Help"), HelpRole)` 的角色改成 ActionRole 重编，按钮位置基本不动，Windows 队列下也只是从最右挪进中段，点它说明框不弹了，状态行提示 Unknown button role，因为示例的 switch 里，ActionRole 落进了 default 分支。再把 connect clicked 那句注释掉重编，四只按钮点下去全无反应，便利信号之外没有别的通道，接线要您自己动手。

完成标准照旧。Retry 按钮点下去发哪个信号，您答得出。DestructiveRole 点下去有没有便利信号，您说得出没有。removeButton 与 clear 对按钮分别做了什么，您分得清。头一只 AcceptRole 按钮凭什么当默认，您讲得明。
