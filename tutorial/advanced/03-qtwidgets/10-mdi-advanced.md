---
title: "3.10 MDI 进阶"
description: "入门篇的 QMdiArea 写 demo 够用，搬进真实项目就出状况，用户一关子窗口，编辑中的内容跟着 WA_DeleteOnClose 一起没了，菜单同步的槽函数还可能摸到正在析构的指针。本篇讲 closeEvent 的 accept 与 ignore 分岔怎么接保存确认，subWindowActivated 的 nullptr 与销毁时序怎么防，子窗口最大化后图标和按钮怎么进的菜单栏，末节用一张决策表回答 MDI 与多标签的取舍。配套示例在 examples/advanced/03-qtwidgets/10-mdi-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.10——MDI 进阶

[入门篇 MDI 基础](../../beginner/03-qtwidgets/10-mdi-beginner.md) 把 addSubWindow、级联平铺、subWindowActivated 的基本接法过了一遍，写一个能跑的多文档骨架确实够用。可咱们把 MDI 往真实项目里搬，麻烦跟着就来了。用户在子窗口里敲了半小时文字，手滑点了关闭按钮，挂了 `WA_DeleteOnClose` 的窗口连同它包着的编辑器一起销毁，内容一点找回的余地都没有。窗口菜单想跟着活动窗口同步，`subWindowActivated` 的槽函数里拿到的指针却可能是马上要析构的。子窗口最大化之后，主窗口菜单栏的两个角上还会突然多出一个图标和三个按钮。本篇把三件事补上：closeEvent 的 accept 与 ignore 分岔怎么接保存确认，subWindowActivated 的触发时机怎么防，子窗口最大化后菜单栏被接管是怎么回事，最后回答一个架构问题，多文档的活儿什么时候根本不该用 MDI。

## 关闭可以否决：保存确认落在 accept 与 ignore 的分岔上

入门篇提过 `Qt::WA_DeleteOnClose`，挂上它子窗口一关就销毁。销毁本身没什么不对，问题是它不问内容同不同意。您往子窗口里放的是一个文本编辑器，就得在真正关闭之前拦下关闭事件，给用户一次保存确认的机会。咱们假设自己写了一个 QMdiSubWindow 子类，就叫 EditorSubWindow（虚构的教学类名，配套示例里没有这个类，示例选了哪条路下文交代）。拦的位置是重写它的 `closeEvent(QCloseEvent *event)`，在函数里检查内容有没有未保存的修改，有就弹对话框让用户选保存、放弃还是取消：

```cpp
void EditorSubWindow::closeEvent(QCloseEvent *event)
{
    auto *editor = qobject_cast<QTextEdit*>(widget());
    if (editor->document()->isModified()) {
        auto result = QMessageBox::warning(
            this, "确认关闭",
            "文档有未保存的修改，是否保存？",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (result == QMessageBox::Save) {
            saveContent();  // 你的保存逻辑
            event->accept();
        } else if (result == QMessageBox::Discard) {
            event->accept();
        } else {
            event->ignore();  // 用户取消，不关闭
        }
    } else {
        event->accept();
    }
}
```

分岔全在 `event->accept()` 和 `event->ignore()` 两个调用上。accept 是放行，ignore 把关闭顶回去，子窗口原地继续存在。您要是图省事在函数末尾无条件 accept，前面的确认框就成了纯装饰，本节末尾的调试题咱们再看它怎么出的事故。脏标记不用自己维护，QTextEdit 底下的 QTextDocument 自带 `isModified()`，内容被改过就返回 true，保存完调 `setModified(false)` 复位。咱们自己另记一个 bool 的毛病是插入删除的路径一多总有漏改的地方，QTextDocument 的标记跟在编辑器内部机制上，不会漏。

接着要弄清一件事，确认框为什么会弹两次。咱们把关闭事件的真实走法从 Qt 6.9.1 源码里翻出来看（qmdisubwindow.cpp 的 QMdiSubWindow::closeEvent）。用户点子窗口标题栏的关闭按钮，关闭事件先送进 QMdiSubWindow::closeEvent，这个函数头一件事是调用内容控件的 close()，等于把一份关闭事件转发给内容控件。内容控件 ignore 的话 close() 返回 false，QMdiSubWindow 跟着 ignore，窗口留下。内容控件 accept，QMdiSubWindow 才继续收尾并最终 accept。所以内容控件和 QMdiSubWindow 两层各有一次过问关闭的机会，确认对话框在两层都写了，用户就得连着见两个框。上面片段是做在 QMdiSubWindow 子类一层的写法，同样可以只做在内容控件一层，纪律就一条，确认只做在一层，另一层别再弹。

配套示例选的是内容控件一层，咱们直接看 MdiChild::closeEvent 的写法，文件在 examples/advanced/03-qtwidgets/10-mdi-advanced/mdi_child.cpp。它检查自己的 `isModified()`，有修改就弹一个“放弃/取消”二选一的确认框，默认按钮是取消。QMdiSubWindow 保持原装不子类化，整个程序只会有一次弹窗。

### 一道调试题：警告框弹了，窗口照关不误

咱们看一段有问题的关闭处理，您先自己找找原因：

```cpp
void EditorSubWindow::closeEvent(QCloseEvent *event)
{
    if (hasUnsavedChanges()) {
        QMessageBox::warning(this, "提示", "有未保存的修改");
        // 忘了 event->ignore()，也没有 accept()
    }
    event->accept();  // 不管怎样都 accept
}
```

咱们把毛病找出来了，就在最后一行的无条件 `event->accept()`。警告框弹了，可不管用户看完是什么反应，窗口都会被关掉，那个框完全成了摆设。正确写法是按用户的选择分别走 accept 或 ignore，配套示例 mdi_child.cpp 的 closeEvent 就是标准形状，Discard 分支 accept，其余一律 ignore。

## subWindowActivated 的触发时机：激活在前，销毁在后

入门篇咱们拿 subWindowActivated 更新状态栏和窗口菜单，进阶要把它的三个行为特性看准，不然槽函数里迟早出事。咱们把表现和对策摆成表：

| 行为特性 | 表现 | 槽函数里的对策 |
| -------- | ---- | -------------- |
| 参数可为 nullptr | 最后一个子窗口关闭后，信号带着 nullptr 来 | 入口判空，走“无文档”分支 |
| 激活先于销毁 | 被关的子窗口销毁前，别的子窗口先被激活并发出信号 | 遍历 subWindowList() 前想清楚列表里有没有将析构的窗口 |
| closeAllSubWindows 期间连发 | 关闭过程中信号反复触发，全部关完以一次 nullptr 收尾 | 用标志位挡住耗时刷新，循环返回后手动补一次 |

nullptr 一条是信号文档明说的，参数为 nullptr 表示 QMdiArea 刚取消了最后一个活动窗口，工作区里已经没有活动窗口。咱们要是忘了判空直接调 windowTitle()，收获的就是一个段错误。

激活先于销毁一条，官方在 closeAllSubWindows 的文档注释里专门提醒过，关闭过程中可能先收到别的子窗口的激活信号。机制咱们顺着源码看（qmdiarea.cpp），子窗口销毁时 QMdiArea 会激活下一个可见子窗口，被关闭的那个随后才销毁。您要是在槽函数里遍历 `subWindowList()` 做操作，列表里可能还站着即将析构的窗口，对它们调用方法就是在摸野指针。

配套示例的槽函数咱们拿来当参照，文件在 examples/advanced/03-qtwidgets/10-mdi-advanced/mdi_manager.cpp，表格三条对策里，示例做进去的只有入口判空：

```cpp
void MdiManager::onSubWindowActivated(QMdiSubWindow* subWin)
{
    // 子窗口全部关闭后 subWin 为 nullptr
    if (!subWin) {
        m_statusLabel->setText(QStringLiteral("无打开文档"));
        rebuildWindowMenu();
        return;
    }

    auto* child = qobject_cast<MdiChild*>(subWin->widget());
    if (child) {
        // 状态栏更新为当前文档标题与打开文档总数（完整代码见示例）
        ...
    }
    rebuildWindowMenu();
}
```

判空之后示例取了什么，咱们看着它走一遍，qobject_cast 把内容控件取出来，取不到就不动。接着有一处偏离得照实交代。槽函数里调用的 rebuildWindowMenu() 遍历 subWindowList() 重建窗口菜单，菜单动作的 lambda 里捕获的是裸 subWin 指针，正是上表第二条提醒咱们当心的用法。示例为简洁起见这么写，工程上您按表格的纪律来：需要跨信号长期持有子窗口指针，比如缓存“上一个活动窗口”、把窗口指针存进菜单动作，请改用 `QPointer<QMdiSubWindow>` 弱引用，对象一析构它自动置空，比裸指针安全。

连发一条的麻烦在耗时操作。您触发关闭所有子窗口（示例“文件”菜单里的“关闭所有”）时，槽函数里要是有重建复杂窗口列表一类的重活，关闭过程中会被反复执行。惯用的挡法是关闭前设一个标志位，槽函数见到标志直接返回，等 closeAllSubWindows 返回后再清掉标志，手动补一次刷新。

## 子窗口最大化：图标进了菜单栏左角，按钮进了右角

咱们从现象看起。只要 QMdiArea 所在的顶层窗口是一个菜单栏可见的 QMainWindow，不必非得是中央控件，子窗口一最大化，观感就换了一副样子：子窗口自己的标题栏收起来，窗口图标出现在主窗口菜单栏的最左角，最小化、还原、关闭三个按钮出现在最右角，主窗口的标题也改写成“主窗口标题 - [子窗口标题]”的格式。点左角的图标弹出的是子窗口的系统菜单，还原、移动、大小、最小化、最大化、关闭六项都在里面，双击图标等于关闭。

您要是觉得这套动作眼熟，没错，这套布局行为与经典 Windows MDI 的观感一致。笔者把 Qt 6.9.1 的实现翻出来了（qmdisubwindow.cpp），干活的是 QMdiSubWindow 内部的 ControlContainer：最大化时它把图标标签塞进菜单栏左角（setCornerWidget 的 TopLeftCorner），把控件按钮塞进右角（TopRightCorner），还原时再原样摘下来。发生也有前提，QMdiArea 所在的顶层窗口得是菜单栏可见的 QMainWindow，TabbedView 模式下不做，实现里另有两道判断把关，一道叫 drawTitleBarWhenMaximized，另一道叫 isChildOfTabbedQMdiArea。

咱们去公开 API 里找开关，结果是找不到。Qt 给的 `DontMaximizeSubWindowOnActivation` 管的是另一件事，激活联动最大化。官方文档的原话大意是，活动子窗口处于最大化时，默认会把下一个被激活的子窗口也最大化，设这个选项就是取消联动。它管不到菜单栏接管。

真要拦下接管，路子有两条。一条是给 QMdiSubWindow 带上 `Qt::CustomizeWindowHint` 一类的自定义窗口标志。实现里只有在没带任何自定义标志时，才会自动补上系统菜单、最小化最大化按钮那一整套标准标志，您一带 CustomizeWindowHint，系统菜单标志不再自动补，菜单栏左角的图标就没了来处。代价是子窗口标题栏上的图标菜单和按钮也一起没了。另一条是干脆不用最大化，`showFullScreen()` 配自定义标题栏，观感自己说了算。

## MDI 还是标签页：从“要不要并排”问起

QMdiArea 自带一个 TabbedView 模式，`setViewMode(QMdiArea::TabbedView)` 一调，子窗口以标签页形式展示。顺着这个模式，咱们来掂量一个架构问题，多文档的界面，什么时候用 MDI，什么时候用 QTabWidget。判断就问一句，子窗口之间需不需要对比查看、互相重叠、自由拖拽。需要，MDI 是唯一选择，QTabWidget 同一时刻只显示一个标签页，做不到两个文档并排。不需要，只是“同时打开多个文档，一次看一个”，QTabWidget 更轻，交互也更符合当下习惯。

TabbedView 是个折中。底层仍然是 QMdiArea 的子窗口管理机制，外观切成标签页，运行时可以在 SubWindowView 和 TabbedView 之间来回切，让用户按自己的偏好选。代价也明摆着，标签页形态下子窗口自由拖拽的能力没了，QMdiArea 的生命周期管理和信号复杂度却一点没少。确定只要标签页的话，您直接用 QTabWidget 会简单很多。视图模式切换的更多行为，[3.71 QMdiArea 进阶](./71-qmdiarea-advanced.md) 里有专门一篇。

取舍咱们摆成表：

| 需求形态 | 典型应用 | 该用的方案 |
| -------- | -------- | ---------- |
| 多文档同时可见且可拖拽 | 图像编辑器、CAD、数据对比工具 | QMdiArea 的 SubWindowView |
| 多文档切换查看，一次看一个 | 文本编辑器、设置面板、浏览器 | QTabWidget |
| 布局模式想让用户自己选 | 偏好因人而异的工具类软件 | QMdiArea 的 TabbedView 与 SubWindowView 动态切换 |

最后劝您一句，别因为“觉得 MDI 功能更多”就上 MDI。MDI 的生命周期管理和信号处理复杂度远高于 QTabWidget，没有并排和拖拽的需求，这些复杂度全是白背的。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，QMdiArea 与 QMdiSubWindow 都在 QtWidgets 模块，示例可在任何支持 Qt6 的桌面平台上编译运行。

[Qt 文档 · QMdiArea](https://doc.qt.io/qt-6/qmdiarea.html) -- MDI 区域控件，subWindowActivated 信号、视图模式和排列方法

[Qt 文档 · QMdiSubWindow](https://doc.qt.io/qt-6/qmdisubwindow.html) -- MDI 子窗口类，closeEvent 和窗口标志配置

[Qt 文档 · QCloseEvent](https://doc.qt.io/qt-6/qcloseevent.html) -- 关闭事件类，accept/ignore 机制说明

[Qt 文档 · QTabWidget](https://doc.qt.io/qt-6/qtabwidget.html) -- 标签页控件，MDI 的替代方案之一

[Qt 文档 · QTabBar](https://doc.qt.io/qt-6/qtabbar.html) -- 标签栏控件，自定义标签页行为的底层类

---

MDI 的工程实践就补到这里。想再动手的，请您把配套示例（examples/advanced/03-qtwidgets/10-mdi-advanced/，可执行文件 10-mdi-advanced）完整玩一轮。新建三个文档，往其中一个敲几个字，看底部的修改指示和状态栏怎么跟着活动窗口走。带修改关掉它，确认框弹出来，点取消它留在原地。再点“文件”菜单的“关闭所有”，看窗口菜单怎么一个个收干净，状态栏最后停在“无打开文档”。胆子大一点的，把 mdi_child.cpp 里 QMessageBox::warning 的按钮参数从 `Discard | Cancel` 升级成 `Save | Discard | Cancel`，在 Save 分支里把 textContent() 写进文件再 accept()，示例就离一个真的多文档编辑器不远了。下一篇 [3.11 QWidget 基类进阶](./11-qwidget-base-advanced.md) 回到所有控件的共同基类，把 QWidget 的窗口属性与底层机制补齐。
