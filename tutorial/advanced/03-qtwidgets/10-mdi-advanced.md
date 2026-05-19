---
title: "3.10 MDI 进阶"
description: "入门篇我们学会了 QMdiArea 的基本用法，但 MDI 的工程实践难点在于：子窗口生命周期管理、菜单同步、以及 MDI 与多标签方案的选择。"
---

# 现代Qt开发教程（进阶篇）3.10——MDI 进阶

## 1. 前言 / 为什么 MDI 用起来总感觉差点意思

入门篇我们学会了 QMdiArea 的基本操作——创建子窗口、级联平铺、信号监听。说实话，写个 demo 够用了。但一旦你把这些东西搬到真实项目里，问题就来了：用户关了一个编辑中的文档，数据直接没了；菜单栏想跟着活动窗口的状态动态切换启用/禁用，结果槽函数里访问到的是一个正在析构的野指针；子窗口最大化后菜单栏突然多出一堆莫名其妙的条目，快捷键还跟主菜单冲突。这些问题入门篇一个都没提到，但它们才是 MDI 工程实践中真正让人头疼的部分。

这篇进阶篇我们聚焦四个核心议题：子窗口关闭前的数据保存确认机制、subWindowActivated 信号的工程级使用方式、最大化子窗口时的菜单栏合并行为与控制、以及 MDI 与多标签方案的选择策略。搞清楚这些，你的 MDI 应用才算是"能上生产"的。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。所有内容依赖 QtWidgets 模块中的 QMdiArea 和 QMdiSubWindow，不需要额外模块。示例可在任何支持 Qt6 的桌面平台上编译运行。

## 3. 核心概念讲解

### 3.1 子窗口关闭前的保存确认——拦截 QCloseEvent

入门篇我们提到过 `Qt::WA_DeleteOnClose`——设置之后子窗口关闭时自动销毁。但这里有一个致命的盲区：如果子窗口里是一个文本编辑器，用户敲了半小时的代码然后手滑点了关闭按钮，数据直接没了，没有任何撤销机会。所以我们在真正关闭之前，必须拦截关闭事件，给用户一个保存确认的机会。

拦截关闭事件的正确方式是继承 QMdiSubWindow（或者让内容 widget 自己处理），重写 `closeEvent(QCloseEvent *event)`。在这个函数里检查内容是否有未保存的修改，如果有就弹出对话框让用户选择保存、放弃或取消。

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

这里的关键在于 `event->ignore()`——调用 ignore 之后关闭操作会被取消，子窗口继续存在。千万别偷懒直接全部 accept，不然你就等着用户投诉数据丢失吧。另外一个细节是：`QTextEdit::document()->isModified()` 是 QTextDocument 自带的脏标记，只要内容被修改过就会返回 true，调用 `setModified(false)` 可以重置，这比自己维护一个 bool 标记要靠谱得多。

但这里有一个容易踩的坑——如果你同时在内容 widget 上也重写了 closeEvent，关闭事件会先发给内容 widget，再发给 QMdiSubWindow。如果你两边都弹对话框，用户就会看到两个确认弹窗。解决方案是只在一个层级处理 closeEvent，推荐在 QMdiSubWindow 子类中统一处理，内容 widget 不要重写 closeEvent。

### 3.2 subWindowActivated 信号的工程级使用

入门篇我们用 `subWindowActivated` 来更新状态栏和窗口菜单，这是基本用法。但进阶场景下，这个信号有几个非常容易踩坑的行为特性。

第一个特性是：当关闭一个子窗口时，Qt 会先激活另一个子窗口（发出 subWindowActivated），然后才销毁被关闭的那个。这意味着你的槽函数里拿到的"当前活动窗口"可能还没稳定下来。更糟糕的是，如果你在槽函数里遍历 `subWindowList()` 做操作，列表里可能还包含即将被销毁的窗口。

安全的做法是在槽函数里做一次有效性检查：

```cpp
void MainWindow::onSubWindowActivated(QMdiSubWindow *subWin)
{
    if (!subWin) {
        // 所有子窗口都关了，禁用文档相关操作
        updateMenuBarForNoDocument();
        return;
    }

    // 检查子窗口是否正在被销毁
    if (subWin->closeResult() == QDialog::Rejected) {
        return;  // 关闭被取消，不做处理
    }

    updateMenuBarForDocument(subWin);
    updateToolbarState(subWin);
}
```

第二个特性是：在调用 `closeAllSubWindows()` 时，subWindowActivated 会连续触发多次——每关闭一个子窗口就触发一次，最后一次触发时参数为 nullptr。如果你在槽函数里做耗时操作（比如刷新一个复杂的窗口列表），性能会很差。更好的做法是在 closeAllSubWindows 之前设置一个标志位，槽函数检测到这个标志后跳过处理，等 closeAllSubWindows 返回后手动触发一次刷新。

第三个特性是信号可能传入 nullptr。当最后一个子窗口被关闭后，subWindowActivated 的参数就是 nullptr。很多朋友忘了处理这个情况，直接对 nullptr 调 windowTitle()，收获一个漂亮的 segfault。上面代码里的 `if (!subWin)` 判断就是干这个的。

现在有一道调试题给大家。看下面这段关闭事件处理代码：

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

问题出在哪？这段代码弹了警告框告诉用户"有未保存的修改"，但随后无条件调用了 `event->accept()`。也就是说不管用户看到警告后是什么反应，窗口都会被关闭——那个警告框完全是个摆设。正确的做法是根据用户的选择分别调用 accept 或 ignore。另外还有一类更隐蔽的 bug：如果你在内容 widget 的 closeEvent 里做了处理（比如弹了保存确认），同时 QMdiSubWindow 子类也重写了 closeEvent 也弹了一次，关闭事件会逐层传递，两次对话框依次弹出。这就是为什么关闭确认对话框会弹出两次的原因——两个层级各自拦截了一次 closeEvent。解决方式是只在一个层级做关闭确认。

### 3.3 最大化子窗口的菜单合并与控制

QMdiArea 有一个让很多开发者困惑的行为：当一个子窗口处于最大化状态时，它的菜单栏会"合并"到主窗口的菜单栏上。具体表现是——子窗口最大化后，主窗口菜单栏上突然多出了子窗口的系统菜单项（还原、移动、大小、最小化、最大化、关闭），而且子窗口的图标也会出现在菜单栏左侧。

这个行为在某些场景下是有用的（比如传统的 MDI 文档编辑器），但在现代应用中往往显得突兀。如果你不想要这个行为，Qt 提供了一个选项来控制它：

```cpp
// 禁止激活子窗口时自动最大化
mdiArea->setOption(QMdiArea::DontMaximizeSubWindowOnActivation, true);
```

这个选项的作用是：当用户切换活动子窗口时，不会自动把新激活的窗口也最大化。默认行为是"如果当前最大化窗口被切换，新窗口也跟着最大化"，开了这个选项之后切换窗口会恢复到正常的子窗口大小。

但这个选项只控制"激活时是否自动最大化"，不控制菜单合并。菜单合并是 QMdiSubWindow 最大化时 QMdiArea 自动做的行为，目前没有直接的 API 可以关闭它。如果你确实需要禁止菜单合并，一个 workaround 是在 QMdiSubWindow 上设置 `Qt::CustomizeWindowHint` 标志来阻止系统菜单的注入，或者干脆不用最大化而用 `showFullScreen()` + 自定义标题栏来替代。

另外一个需要注意的点是：菜单合并后子窗口菜单项的快捷键可能与主窗口菜单的快捷键冲突。比如主窗口的"文件"菜单绑了 Ctrl+F，而子窗口的某个操作也绑了 Ctrl+F，合并之后两个快捷键同时存在，行为完全不可预测。解决方案是在设计菜单时就规划好快捷键的分配，或者在子窗口最大化时临时禁用冲突的快捷键。

### 3.4 MDI vs 多标签——什么时候该用什么

QMdiArea 在入门篇还提到了一个 TabView 模式——`setViewMode(QMdiArea::TabView)` 可以把子窗口以标签页形式展示。这引出一个架构层面的选择问题：什么场景下用 MDI，什么场景下用 QTabWidget/QTabBar？

选择的核心标准是"子窗口是否需要自由拖拽和重叠"。如果你的子窗口之间需要对比查看、互相重叠、自由调整位置和大小，MDI 是唯一的选择——QTabWidget 同一时间只能显示一个标签页，做不到"两个文档并排"。如果你的需求只是"同时打开多个文档，一次看一个"，QTabWidget 更轻量、更现代、交互上也更直观。

QMdiArea::TabView 模式是一个折中方案。它的底层仍然是 QMdiArea 的子窗口管理机制，但外观上切换为标签页。好处是你可以在运行时动态切换视图模式（`SubWindowView` 和 `TabView`），用户可以按偏好选择。但坏处是标签页模式下你丢失了子窗口自由拖拽的能力，同时获得了 QMdiArea 的全部复杂度——如果你确定只需要标签页，直接用 QTabWidget 会简单很多。

一个实用的决策框架是这样的：如果应用需要"多文档同时可见且可拖拽"（图像编辑器、CAD、数据对比工具），选 MDI 的 SubWindowView；如果只是"多文档切换查看"（文本编辑器、设置面板、浏览器），选 QTabWidget；如果想让用户自己选择布局模式，用 QMdiArea 的 TabView/SubWindowView 动态切换。千万不要"明明只需要标签页却用 MDI，只因为觉得 MDI 功能更多"——MDI 的生命周期管理和信号处理复杂度远高于 QTabWidget，没有需求就不要给自己找麻烦。

## 4. 踩坑预防

第一个坑是关闭子窗口时数据未保存直接 accept。这个问题在 3.1 节已经详细讲了。后果很明确：用户编辑的内容全部丢失，而且没有撤销机会——因为窗口已经被销毁了，QTextEdit 和它的 QTextDocument 都跟着没了。解决方案是重写 closeEvent，检查脏标记，弹保存确认对话框，根据用户选择调用 accept 或 ignore。

第二个坑是 subWindowActivated 信号在窗口销毁过程中触发。具体表现是槽函数里拿到一个"看起来还活着但实际正在析构"的子窗口指针，对它调任何方法都可能崩溃。这个坑在入门篇的踩坑预防里提到过但没有展开，现在我们把原因讲透：Qt 在关闭子窗口时会先激活另一个窗口，再销毁被关闭的窗口。如果你在 subWindowActivated 的槽函数里访问了即将被销毁的窗口（比如通过 subWindowList() 遍历），就会触发野指针访问。解决方案是在槽函数里用 `qobject_cast` 检查指针有效性，或者用 `QPointer<QMdiSubWindow>` 弱引用来安全地追踪子窗口。

第三个坑是菜单合并后子窗口菜单项与主菜单冲突。当子窗口最大化时，QMdiArea 会把子窗口的系统菜单注入到主窗口菜单栏，这可能和主窗口已有菜单项的快捷键冲突。后果是快捷键重复绑定，用户按下某个快捷键后行为不可预测——可能执行主菜单的操作，也可能执行子窗口的操作。解决方案是在设计阶段就规划好快捷键命名空间，避免主菜单和子窗口使用相同的快捷键组合，或者在子窗口最大化时通过代码动态调整快捷键绑定。

## 5. 练习项目

练习项目：多文档文本编辑器。我们要在 MDI 框架下实现一个简易的多文档编辑器，覆盖本篇所有核心知识点。

功能要求是：工具栏支持新建、打开、保存三个操作。新建创建一个子窗口（内含 QTextEdit），标题递增编号。打开弹出文件对话框，读取文件内容到新的子窗口。保存将当前活动子窗口的内容写回文件。每个子窗口关闭前检查 document()->isModified()，有修改则弹保存确认对话框。主菜单有一个"窗口"菜单，动态列出所有打开的子窗口，当前活动窗口打勾，点击菜单项切换活动窗口。状态栏显示当前文档标题和打开文档总数。

提示几个关键点：自定义一个 EditorSubWindow 继承 QMdiSubWindow，在 closeEvent 里做保存确认；用 QPointer 追踪活动子窗口，避免 subWindowActivated 槽函数里的野指针问题；窗口菜单在 subWindowActivated 槽里每次重建，用 setActiveSubWindow 切换；保存功能通过子窗口的自定义接口获取文件路径和编辑器内容。完成标准是关闭未保存文档时弹确认对话框且只弹一次，窗口菜单正确同步当前活动文档，切换和关闭操作无崩溃。

## 6. 官方文档参考链接

[Qt 文档 · QMdiArea](https://doc.qt.io/qt-6/qmdiarea.html) -- MDI 区域控件，subWindowActivated 信号、视图模式和排列方法

[Qt 文档 · QMdiSubWindow](https://doc.qt.io/qt-6/qmdisubwindow.html) -- MDI 子窗口类，closeEvent 和窗口标志配置

[Qt 文档 · QCloseEvent](https://doc.qt.io/qt-6/qcloseevent.html) -- 关闭事件类，accept/ignore 机制说明

[Qt 文档 · QTabWidget](https://doc.qt.io/qt-6/qtabwidget.html) -- 标签页控件，MDI 的替代方案之一

[Qt 文档 · QTabBar](https://doc.qt.io/qt-6/qtabbar.html) -- 标签栏控件，自定义标签页行为的底层类

---

到这里，MDI 的进阶内容我们就过了一遍。子窗口的关闭确认是保证用户数据安全的第一道防线，subWindowActivated 信号的坑点搞清楚了就不会在槽函数里踩野指针，菜单合并行为理解了就不会被突然多出来的菜单项吓一跳，MDI vs 多标签的选择策略能帮你在架构阶段做出正确的决策。下一篇我们来看动画系统——让控件动起来的那些事。
