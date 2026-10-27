---
title: "3.60 QDialog 进阶"
description: "进阶篇接住 59 篇交棒进入对话框本体：exec 停的是调用方代码，事件分发照常跑（qdialog.cpp 第 573-575 行局部 QEventLoop，第 839-842 行隐藏时 exit），WA_DeleteOnClose 在 exec 期间被临时摘掉由它自己补删（第 558-559、586-587 行），Esc 与关闭钮都汇到 reject（第 696-698、733-737 行），Enter 找 default 按钮代点（第 703-714 行），三种模态粒度在 qnamespace.h 第 1556-1560 行，exec 置的是 WA_ShowModal 而 ApplicationModal 是 qwidget.cpp 第 11462-11476 行属性分支的默认提升，open() 借 WindowModal 用完由 resetModalitySetByOpen 还回（第 509-526、458-471 行），Qt 5 的 open(receiver, member) 重载在 6.9 头文件已不存在（qdialog.h 第 51-56 行），三信号发放现场在 done()（第 608-626 行），done(3) 自定义码只走 finished，hide() 不发信号，QDialogButtonBox 到 QDialog 没有自动接线，Qt 自家 QFontDialog 第 283 行与 QInputDialog 第 216 行都是手动 connect，示例在 examples/advanced/03-qtwidgets/60-qdialog-advanced/"
---

# 现代Qt开发教程（进阶篇）3.60——QDialog 进阶

59 篇收队时咱们约好，从常驻窗口转入对话框，exec 的模态循环与三种模态粒度在这边等咱们。本篇兑现这句话。笔者把 Qt 6.9.1 的 QDialog 源码（qdialog.cpp）从头读到尾，exec 的嵌套事件循环、模态三档、open() 的异步路线、三个结果信号的发放次序，全在 qdialog.cpp 一份文件里有据可查。中间还要纠正一个流传很广的说法，那个得到按钮盒那边对证。

主战场是 qdialog.cpp 与 qdialog.h（src/widgets/dialogs），私有结构下探到 qdialog_p.h，咱们引用文件交代清楚。模态属性的本体在 QWidget 一侧，咱们要借 QWidget 源码（qwidget.cpp）的两段代码作证。配套示例是 examples/advanced/03-qtwidgets/60-qdialog-advanced/ 的一只双按钮演示窗，完整玩法放结语。

## exec：代码停在原地，事件照常分发

入门篇咱们写过那样的代码，new 一只对话框，exec 等返回值，按返回值分流。它到底停在哪。咱们去读 Qt 6.9.1 的 QDialog 源码（qdialog.cpp 第 549-589 行）。进门头一件事是查递归，同一只对话框 exec 两次，收到的是 qWarning 加 -1（第 553-556 行）。随后两行有点意外，exec 把 WA_DeleteOnClose 临时摘掉了（第 558-559 行），循环结束后再由它自己补 delete（第 586-587 行），删除时机收归 exec 一手管。

咱们接着看，模态与结果码的准备各一句（第 563-565 行），show（第 567 行），然后是本篇的主角：

```cpp
// exec 的本体：栈上一只 QEventLoop（qdialog.cpp 第 573-575 行，摘录）
QEventLoop eventLoop;
d->eventLoop = &eventLoop;
(void) eventLoop.exec(QEventLoop::DialogExec);
```

栈上的局部循环。您的代码停在 eventLoop.exec 那一行等它返回，可 QApplication 的主循环没有停，它只是被叠了一层。定时器超时、网络回复、排在队列里的删除事件，都在这一层里照常分发。这正是类文档第 539-544 行劝咱们少用 exec 的理由，原文把 “deleting the dialog's parent while the dialog is open via exec()” 写成事故例子，对话框还开着，父窗口已经没了。

循环怎么退出。QDialogPrivate::setVisible 在对话框藏起来的分支里调 eventLoop->exit()（第 839-842 行），所以任何让对话框 hide 的路径都能放行 exec。返回值 rescode 在私有类构造时是 0（qdialog_p.h 第 49 行），DialogCode 枚举 Rejected 打头（qdialog.h 第 30 行），没走到 accept 的关闭一律算拒收。用户按 Esc 是一条，keyPressEvent 里 QKeySequence::Cancel 一命中，就调 reject（第 696-698 行）。点标题栏关闭钮是另一条，closeEvent 在可见状态下调 reject，拒完还可见就 ignore 掉关闭事件（第 733-737 行）。Enter 键另有一路，它去找可见的 default 按钮，替您 click 一下（第 703-714 行）。

您若拿 exec 写主流程，得清楚这一层循环里谁都能插队。exec 自己有 QPointer guard 兜底（第 569、577-578 行，对象没了直接返回 Rejected），业务对象可没这层保险，等待 exec 返回的调用方握着裸指针，就赌上运气了。文档把它列成劝退语，咱们没有理由不当真。

## 三种模态粒度：谁被冻结，谁还活着

咱们把枚举数一遍，qnamespace.h 第 1556-1560 行，NonModal、WindowModal、ApplicationModal 三档，冻结范围递进。NonModal 谁都不冻，WindowModal 冻父窗口链，ApplicationModal 冻整个应用的窗口。类文档第 226-230 行讲窗口级模态，只挡住对话框一系的父窗口，其余顶层窗口照常用，工具面板要保持可用的多窗口应用，选这一档。

有种说法是 exec 内部调了 setWindowModality(Qt::ApplicationModal)，源码里没有这一句。exec 置的是 WA_ShowModal 属性（qdialog.cpp 第 563-564 行），属性的本体处理在 QWidget 源码（qwidget.cpp 第 11462-11476 行），置位时若 windowModality 还没被显式设过，属性分支自动把它提为 ApplicationModal。“exec 默认冻结全应用”这话对，机制却绕了一道，咱们报出处要报准。

setModal(true) 是同一条通道的便捷写法（qdialog.cpp 第 1010-1013 行，一行 setAttribute），文档注明它等价于把 windowModality 设为 ApplicationModal（第 996-1008 行），而且 exec 不看 modal 属性，进门一律按模态弹（第 1004-1005 行）。您手动 setWindowModality(Qt::WindowModal) 再 show()，得到窗口级模态加非阻塞，主窗口冻着，代码继续跑。open() 内部走的就是这一档，下一节咱们看它怎么走。

## open()：借来的窗口模态，收场还回去

open() 的实现在 qdialog.cpp 第 509-526 行，咱们逐行看。当前模态不是 WindowModal 时，记下旧值（第 515-516 行），随后 setWindowModality(Qt::WindowModal)，顺手把 WA_SetWindowModality 摘掉（第 517-518 行），装作模态生来如此。然后 setResult(0) 清结果码（第 524 行），show() 完就返回（第 525 行），全程不起事件循环。

借的模态要还。对话框收场时 QDialogPrivate::close 调 resetModalitySetByOpen（第 149 行），实现（第 458-471 行）里只要您事后没自己动过模态，旧值原样恢复。exec 进门也调一次它（第 561 行），免得上一场 open() 借的模态串场。借了要还的设计平时感知不到，您排查“对话框关了别的窗口还冻着”一类现象时，它是必经的检查点。

结果不再从返回值拿，从信号拿。`open(QObject *receiver, const char *member)` 这个一次性回调重载，6.9 的 qdialog.h 里已经没有了，槽清单（第 51-56 行）仅剩无参 open，Qt 5 迁移过来的代码见着它，换成 connect 加 lambda 是正路。另有一句警告咱们要带上，qdialog.cpp 类文档第 249-251 行写着，open() 或 show() 弹的模态对话框别建在栈上，控制权立刻回到调用方，栈帧一散对话框就成了悬空对象。

## accept、reject、done：三个信号的老家

咱们看三个信号，accepted、rejected、finished(int)，声明在 qdialog.h 第 46-49 行，发放现场全在 done()。done(int r)（qdialog.cpp 第 608-626 行）开头走 QDialogPrivate::close，把 r 写进 rescode（第 122 行 setResult）并隐藏对话框，随后按 dialogCode() 分流（qdialog_p.h 第 93 行，它就是 rescode 的内联读法），Accepted 发 accepted，Rejected 发 rejected（第 618-622 行），收尾的是 finished(r)，带着原始参数（第 624-625 行）。您要是 done(3) 传自定义码，accepted 与 rejected 都不出声，只有 finished 应答。

咱们再看 accept() 与 reject()，它们只是 done(Accepted) 与 done(Rejected) 的一行转发（第 634-648 行），而且五个槽在头文件里全是 virtual（qdialog.h 第 51-56 行），类文档（qdialog.cpp 第 314-318 行）也写明，可以重写 accept、reject、done 改关闭行为。想在关门前做校验，重写 accept() 是一条路，Enter 键、代码调用、按钮点击最终都汇到这里，一处拦住全部。另有一条信号细节值得记，hide() 或 setVisible(false) 不发任何信号，三个信号的文档各自注明了（第 1074-1085 行起），信号只认 done 家族的关闭。

关闭时的删除也讲究顺序。WA_DeleteOnClose 的 delete 走 deleteLater，出处 qwidget.cpp 第 8594-8600 行，排进事件循环稍后执行，所以 accepted 槽跑的时候对话框对象还活着，取数据来得及。但排队终究要执行，您在槽里再开一只模态对话框或再跑一轮事件处理，删除就可能插进来。要跨过这段时间用数据，把数据拷出来，别攥对话框指针。

## 按钮盒不会替您接线

有种流传的说法是，QDialogButtonBox 放进 QDialog，AcceptRole 的按钮点了自动调 accept()，不用 connect。笔者翻过 QDialogButtonBox 的源码，文件是 qdialogbuttonbox.cpp，没有那层自动接线。点击的路径在 handleButtonClicked（第 872-904 行），头一个发的是 clicked(button)（第 883 行），随后按按钮角色分流，AcceptRole 与 YesRole 发 accepted（第 891 行），RejectRole 与 NoRole 发 rejected（第 895 行），HelpRole 发 helpRequested（第 898 行）。到这一步，信号还停在按钮盒自己身上。

那 QDialog 那头谁接。Qt 自家对话框全是手动连的，QFontDialog 与 QInputDialog 都写了 connect(buttonBox, &QDialogButtonBox::accepted, q, &QDialog::accept)，出处分别在 qfontdialog.cpp 第 283 行与 qinputdialog.cpp 第 216 行，QMessageBox 则走 clicked 逐钮分发，出处 qmessagebox.cpp 第 274 行。按钮盒的 show 事件里确实有事发生，但那是 ensureFirstAcceptIsDefault，出处 qdialogbuttonbox.cpp 第 1061-1062 行，实现在第 998-1043 行，把第一只 AcceptRole 按钮设为默认钮，与 accept 调用无关。所以您写自己的对话框，connect(buttons, &QDialogButtonBox::accepted, dlg, &QDialog::accept) 这一行得自己敲，忘了它，按钮点了对话框纹丝不动。按钮角色的整套系统，61 篇展开。

## 示例：同一只对话框，两条取结果的路径

示例咱们走一遍（examples/advanced/03-qtwidgets/60-qdialog-advanced/，主窗口两只按钮加一块日志区）。阻塞的一路（main_window.cpp 第 57-77 行）里对话框是栈对象（第 63 行），exec 返回（第 64 行）后还要读 name() 与 email()（第 68-69 行），栈对象此刻还活着，取数安全，这是 exec 顺手的地方。非阻塞的一路（第 79-112 行）换成 new（第 88 行）配 WA_DeleteOnClose（第 89 行），accepted 的 lambda 用 QPointer 捕获（第 94 行），槽里判了空再取数（第 96-103 行），最后 show() 立即返回（第 111 行）。

咱们再看校验，它长在按钮转发上。UserInputDialog 把 OK 钮连到 validateAndAccept，出处 user_input_dialog.cpp 第 74-75 行，两栏输入有一栏空着就原样 return，对话框不关（第 83-88 行），两栏都有内容 accept 才被调到（第 90 行）。示例没有重写 accept()，靠的是按钮转发，与前面重写 accept() 的路线互为印证，两条都拦得住空输入。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过。

[Qt 文档 · QDialog](https://doc.qt.io/qt-6/qdialog.html) -- 对话框基类，exec、open、done 与模态行为的定义处

[Qt 文档 · QDialogButtonBox](https://doc.qt.io/qt-6/qdialogbuttonbox.html) -- 按钮盒类，ButtonRole 与 accepted、rejected 信号的出处

[Qt 文档 · QWidget](https://doc.qt.io/qt-6/qwidget.html) -- setWindowModality 与 WA_DeleteOnClose 的属性文档

[Qt 文档 · QEventLoop](https://doc.qt.io/qt-6/qeventloop.html) -- 事件循环类，exec 嵌套的本体

---

主窗口一家讲完，对话框这边刚开门。下一站 [3.61 QDialogButtonBox 进阶](./61-qdialogbuttonbox-advanced.md)，按钮角色的跨平台布局差异、自定义按钮的角色分配都在那边，本篇“按钮盒不会替您接线”的判断，到 61 篇还能看到更多手动接线的现成范本。

想动手的您把示例跑起来（examples/advanced/03-qtwidgets/60-qdialog-advanced/，可执行文件 60-qdialog-advanced）。点 Open Non-Blocking (show)，对话框开着时主窗口按钮照样点得动，日志区持续追加，谁都没被冻结，这就是 NonModal 配 show() 的现场。再点 Open Blocking (exec)，对话框关掉之前您点主窗口任何地方都没反应，日志停在打开那一行，您的代码停在 exec 那一行等返回。两栏空着点 OK，对话框不动，填一栏还是不动，两栏都填了才收下。按 Esc 与点关闭钮走同一条 reject 路，日志都记 Rejected。

完成标准照旧。exec 停的是谁的代码、不停的是谁，您说得出。WindowModal 与 ApplicationModal 冻结范围差在哪，您讲得明。open() 借的模态什么时候还，您答得上。按钮盒的 accept 要不要自己连，您手里现在有源码出处了。
