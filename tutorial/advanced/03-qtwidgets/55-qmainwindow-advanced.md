---
title: "3.55 QMainWindow 进阶"
description: "入门篇搭了 QMainWindow 骨架、上篇做了 Dock 布局持久化——这次我们解决多显示器适配、全屏模式切换、窗口状态完整保存恢复、以及核心区域的动态替换。"
---

# 现代Qt开发教程（进阶篇）3.55——QMainWindow 进阶

## 1. 前言 / 当你的窗口需要"跨屏作战"

入门篇我们把 QMainWindow 五大区域过了一遍，上一篇我们在 Dock 布局持久化上做了深入讲解。这次我们面对的是另一类工程问题：多显示器环境。现在开发者用双屏甚至三屏已经是常态，你的应用如果只会在主屏幕上老老实实待着，用户一定会吐槽。具体来说，我们这次要解决四个问题——怎么检测并定位窗口到指定显示器、怎么正确实现全屏切换而不丢菜单栏、怎么在重启后完整恢复窗口状态（包括在哪个屏幕上）、以及怎么在运行时动态替换菜单栏/状态栏/中央控件。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。涉及 QtGui 模块（QScreen、QGuiApplication）和 QtWidgets 模块（QMainWindow、QSettings）。多显示器 API 在所有桌面平台行为一致，但全屏模式在 macOS 上有特殊行为——系统会把整个应用（而非单个窗口）推入全屏桌面空间，这一点我们后面会详细讲。

## 3. 核心概念讲解

### 3.1 QScreen 多显示器检测与窗口定位

Qt 6 中每个物理显示器由 QScreen 对象表示。QGuiApplication::screens() 返回当前连接的所有屏幕列表，列表中的第一个元素就是主屏幕（primaryScreen）。每个 QScreen 持有 geometry() 返回该屏幕在整个虚拟桌面中的矩形区域，availableGeometry() 则扣除了系统任务栏/ Dock 栏占用的区域。

我们把窗口放到副屏幕上的核心思路就是：拿到目标 QScreen 的 geometry，然后用 setGeometry 把窗口移过去。这里有一个重要前提——Qt 的虚拟桌面坐标系是所有屏幕拼接在一起的。比如主屏 1920x1080 在左边，副屏 1920x1080 在右边，那副屏的 geometry 就是 (1920, 0, 1920, 1080)。直接把窗口的 x 坐标设到副屏范围内就能跨屏定位了。

```cpp
void MainWindow::move_to_screen(int screen_index)
{
    auto screens = QGuiApplication::screens();
    if (screen_index < 0 || screen_index >= screens.size()) {
        return;
    }
    QScreen* target = screens.at(screen_index);
    // availableGeometry 已扣除任务栏区域
    QRect available = target->availableGeometry();
    // 保持窗口当前大小，移到目标屏幕中央
    QSize current_size = size();
    int x = available.x() + (available.width() - current_size.width()) / 2;
    int y = available.y() + (available.height() - current_size.height()) / 2;
    setGeometry(x, y, current_size.width(), current_size.height());
}
```

还有一个场景是热插拔——用户在应用运行时拔掉或接上显示器。QGuiApplication 提供了 screenAdded(QScreen*) 和 screenRemoved(QScreen*) 信号。如果你的窗口正站在一个被拔掉的屏幕上，Qt 会自动把它移到主屏幕，但如果你自己维护了"当前屏幕索引"之类的状态，就必须监听 screenRemoved 及时更新，否则索引就失效了。

```cpp
// 监听屏幕变化
connect(qApp, &QGuiApplication::screenRemoved, this, [this](QScreen* removed) {
    QScreen* current = screen();  // QWidget::screen() 返回窗口当前所在屏幕
    if (current == removed) {
        move_to_screen(0);  // 退回主屏幕
    }
});
```

现在有一道调试题给大家。下面这段代码试图把窗口放到第二个显示器上，但运行后窗口出现在了主屏幕的右边缘之外——完全看不到。问题出在哪里？

```cpp
auto screens = QGuiApplication::screens();
if (screens.size() > 1) {
    QRect geo = screens[1]->geometry();
    move(geo.x(), geo.y());  // 只移动了位置
}
```

问题出在窗口的 DPI 缩放上。如果副屏的 devicePixelRatio 和主屏不同（比如主屏是 1x，副屏是 2x Retina），Qt 的坐标系统在高 DPI 模式下可能需要额外处理。另外，move() 只设置了位置没有设置大小——如果窗口之前是最大化的，move 可能不会如预期工作。更稳健的做法是用 setGeometry 同时设置位置和大小，或者在 move 前先调用 setWindowState(windowState() & ~Qt::WindowMaximized) 确保窗口不在最大化状态。

### 3.2 全屏模式切换

QWidget 提供了 showFullScreen() 进入全屏模式和 showNormal() 退出全屏。全屏模式下窗口会覆盖整个屏幕，包括任务栏区域。这里有一个常见的工程需求：按 F11 切换全屏，类似浏览器的行为。

实现 F11 切换的核心是判断当前窗口状态。Qt::WindowState 是一个组合标志，windowState() 返回的值可能同时包含 Qt::WindowFullScreen 和 Qt::WindowMaximized 等标志。判断是否处于全屏时要用按位与测试，而不是直接比较。

```cpp
void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_F11) {
        if (windowState() & Qt::WindowFullScreen) {
            showNormal();
        } else {
            showFullScreen();
        }
        return;
    }
    QMainWindow::keyPressEvent(event);
}
```

这里有一个很容易踩的坑——全屏模式下菜单栏怎么办？在 Windows 和 Linux 上，showFullScreen() 不会自动隐藏菜单栏，用户还能正常操作菜单。但在 macOS 上，全屏模式是系统级的行为，整个应用进入一个独立的桌面空间，菜单栏会自动隐藏到屏幕顶部（鼠标移上去才会显示）。这不是 Qt 的行为，是 macOS 的全屏协议就是这么设计的。

如果你希望在全屏时自动隐藏菜单栏来获得更大的显示区域（比如图片查看器、视频播放器），可以自己控制 menuBar 的可见性。但要注意，QMainWindow::menuBar() 返回的 QMenuBar 在 Qt 内部有自己的布局管理，直接调用 hide()/show() 是可行的。

```cpp
void MainWindow::toggle_fullscreen()
{
    if (windowState() & Qt::WindowFullScreen) {
        showNormal();
        menuBar()->show();
    } else {
        menuBar()->hide();  // 全屏前先隐藏菜单栏
        showFullScreen();
    }
}
```

但这里有个细节——如果用户在全屏状态下通过系统快捷键（比如 macOS 的 Ctrl+Cmd+F）退出全屏，你不会收到 F11 的 keyPressEvent，菜单栏就不会恢复。解决方案是重写 changeEvent，监听 QEvent::WindowStateChange 事件。

```cpp
void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange) {
        bool is_fullscreen = windowState() & Qt::WindowFullScreen;
        menuBar()->setVisible(!is_fullscreen);
    }
    QMainWindow::changeEvent(event);
}
```

另外一个要记住的点是：showFullScreen() 和 showMaximized() 是互斥的。如果窗口当前是最大化状态，调用 showFullScreen() 会先退出最大化再进入全屏。调用 showNormal() 退出全屏时，窗口回到 normal 状态，不是回到之前的最大化状态。如果你希望"全屏 → 最大化 → normal"这样的三态切换，需要自己维护一个状态栈来记住之前的状态。

### 3.3 窗口状态保存与恢复

上一篇我们讲了 saveState/restoreState 处理 Dock 和工具栏布局。这次我们把状态保存做得更完整——除了 Dock 布局，还要保存窗口几何信息、全屏/最大化状态、以及当前所在的屏幕。

saveGeometry() 和 restoreGeometry() 是 QMainWindow（继承自 QWidget）提供的一对 API。saveGeometry 序列化窗口的位置和大小，restoreGeometry 从字节数组中恢复。这对 API 比手动保存 pos()/size() 好在它内置了"屏幕外窗口"保护——如果保存时的显示器已经不存在了，restoreGeometry 会自动把窗口移回主屏幕的可见区域。

```cpp
// 窗口关闭时保存完整状态
void MainWindow::closeEvent(QCloseEvent* event)
{
    QSettings settings("MyCompany", "MyApp");
    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("window_state", saveState());
    // 记住是否最大化或全屏
    settings.setValue("maximized",
                      bool(windowState() & Qt::WindowMaximized));
    settings.endGroup();
    QMainWindow::closeEvent(event);
}
```

恢复逻辑的调用顺序很重要。正确顺序是：先 restoreGeometry（恢复窗口位置和大小），再 restoreState（恢复 Dock 和工具栏布局）。如果还需要恢复最大化/全屏状态，要在 restoreGeometry 之后调用 showMaximized() 或 showFullScreen()。

```cpp
void MainWindow::restore_layout()
{
    QSettings settings("MyCompany", "MyApp");
    settings.beginGroup("MainWindow");

    // 先恢复几何信息
    bool has_geometry = restoreGeometry(
        settings.value("geometry").toByteArray());

    // 再恢复 Dock/ToolBar 布局
    restoreState(
        settings.value("window_state").toByteArray());

    // 恢复窗口状态（最大化/全屏）
    if (has_geometry && settings.value("maximized", false).toBool()) {
        showMaximized();
    }

    settings.endGroup();
}
```

一个值得注意的细节是 restoreGeometry 的版本兼容性。不同 Qt 版本序列化的字节数组格式可能不同。如果你的用户从 Qt 5 升级到了 Qt 6，旧的 geometry 数据可能无法正确恢复。restoreGeometry 在失败时返回 false，你可以在 false 时设置一个合理的默认窗口大小和居中位置。

### 3.4 statusBar / menuBar / centralWidget 的动态替换

入门篇我们讲过 QMainWindow 的五大区域是"一个坑位一个控件"——一个菜单栏、一个状态栏、一个中央控件。那如果运行时需要换掉中央控件怎么办？比如一个 IDE 从欢迎页切换到代码编辑器，再切换到差异对比视图。

QMainWindow::setCentralWidget(widget) 会把当前的中央控件替换掉。但这里有一个关键行为：setCentralWidget 会取得传入 widget 的所有权，并且会把之前的中央控件删除（delete）。没错，是直接 delete，不是 hide。如果你需要保留旧的中央控件以便稍后切回来，必须先调用 takeCentralWidget() 把它取出来。

```cpp
void MainWindow::switch_to_editor()
{
    // 取出欢迎页，保存起来
    QWidget* old_central = takeCentralWidget();
    if (old_central) {
        m_welcome_page = old_central;
        m_welcome_page->hide();
    }

    // 创建或显示编辑器
    if (!m_editor) {
        m_editor = new CodeEditor(this);
    }
    setCentralWidget(m_editor);
    m_editor->show();
}
```

takeCentralWidget() 是 Qt 5.2 引入的 API，它会断开中央控件和 QMainWindow 的父子关系并返回指针，但不会 delete 它。调用之后你获得了一个"自由的" widget，可以安全地保存起来或者放到别的地方。

菜单栏的动态替换比较少见但也不是没有场景。比如一个图像编辑器在打开不同格式的文件后，菜单项需要不同。QMainWindow::setMenuBar(menuBar) 会替换当前菜单栏——和 setCentralWidget 类似，它也会 delete 旧的菜单栏。如果你想保留旧菜单栏，Qt 没有提供 takeMenuBar，你需要用 removeAction 把菜单从旧菜单栏一个个移出来，或者干脆不用替换，而是用 QMenu 的 clear() + 重新 addAction 来动态更新菜单内容。

状态栏的动态替换用 setStatusBar(statusBar)。场景比较少，大多数时候我们是在同一个 QStatusBar 上动态添加/移除子控件。

## 4. 踩坑预防

第一个坑是 restoreGeometry 恢复后窗口出现在屏幕外。这个现象在用户拔掉副显示器后最容易复现——QSettings 里存的是副显示器的坐标，下次启动时副显示器不在了，窗口就跑到了一个不存在的坐标上。你可能会觉得 restoreGeometry 不是会自动处理这个情况吗？是的，大部分情况它会处理，但有一种例外：如果保存的坐标恰好在虚拟桌面范围内但实际不可见（比如多显示器排列方式被用户改了），restoreGeometry 可能认为坐标"有效"而不做修正。解决方案是在 restoreGeometry 之后加一个后检查——判断窗口的中心点是否在任何 QScreen 的 availableGeometry 内，如果不在就手动居中到主屏幕。

第二个坑是全屏切换丢菜单栏。这个坑我们在 3.2 节提到过，核心原因是用户没有通过你写的 F11 快捷键退出全屏（比如用 macOS 系统手势、Alt+Tab 切走再切回来、或者按 Esc），导致你的菜单栏显隐逻辑没有触发。解决方案是用 changeEvent 监听 WindowStateChange 事件来同步菜单栏状态，而不是只依赖 keyPressEvent。上面 3.2 已经给出了正确代码，这里不重复。

第三个坑是 setCentralWidget 静默删除旧控件。这是很多新手栽跟头的地方。setCentralWidget 的文档明确说了"the previous central widget is deleted"，但这行字很容易被忽略。如果你的中央控件里持有大量数据（比如一个加载了百 MB 文档的编辑器），调用 setCentralWidget(new_widget) 后旧控件连同数据全部被 delete，而且没有任何提示。如果你需要保留旧控件，必须先 takeCentralWidget()。

第四个坑是多显示器 DPI 不一致导致布局错乱。这在 Windows 上最常见——主屏 150% 缩放，副屏 100% 缩放。窗口从主屏拖到副屏时，Qt 会触发 changeEvent(QEvent::ScreenChange)，控件的 logicalDpiX/Y 会变化。如果你的布局中有硬编码的像素值（比如固定宽度 800px），在 DPI 切换后可能看起来过宽或过窄。解决方案是使用基于字体的尺寸（QFontMetrics）或布局的 stretch factor，避免硬编码像素值。

## 5. 练习项目

练习项目：多显示器窗口管理器。我们要做一个小工具，功能是展示当前所有显示器的信息，并能把窗口在显示器之间移动和切换全屏。

我们要实现的功能是：列出所有屏幕的名称、分辨率、DPI 和可用区域，点击某个屏幕可以把主窗口移到该屏幕上居中显示，按 F11 切换全屏，窗口关闭后记住位置和大小，下次启动恢复到上次的屏幕和位置。完成标准是程序能正确响应屏幕热插拔（插拔时列表更新），全屏切换流畅且菜单栏状态正确，重启后窗口回到上次的屏幕上，如果该屏幕不存在则回到主屏幕。

提示几个关键点：用 QGuiApplication::screens() 获取屏幕列表，用 QLabel 或 QTableWidget 展示信息，监听 screenAdded/screenRemoved 信号动态更新列表，用 QSettings 保存窗口几何信息，changeEvent 中处理 WindowStateChange 同步菜单栏显隐。

## 6. 官方文档参考链接

[Qt 文档 · QMainWindow](https://doc.qt.io/qt-6/qmainwindow.html) -- 主窗口类，包含 saveState/restoreState/saveGeometry/restoreGeometry/setCentralWidget

[Qt 文档 · QScreen](https://doc.qt.io/qt-6/qscreen.html) -- 屏幕类，提供 geometry/availableGeometry/devicePixelRatio

[Qt 文档 · QGuiApplication](https://doc.qt.io/qt-6/qguiapplication.html) -- GUI 应用类，screens()/primaryScreen/screenAdded/screenRemoved 信号

[Qt 文档 · QWidget](https://doc.qt.io/qt-6/qwidget.html) -- showFullScreen/showNormal/windowState/setGeometry

[Qt 文档 · QSettings](https://doc.qt.io/qt-6/qsettings.html) -- 跨平台持久化设置，用于保存窗口状态

---

到这里我们把 QMainWindow 的多显示器适配和全屏切换讲完了。多显示器这个话题看起来简单，但真正能在各种 DPI 组合和热插拔场景下都稳定工作，需要对 QScreen 坐标系和 Qt 窗口状态机制有扎实的理解。下一篇我们继续 QMainWindow 的周边组件——QMenuBar 的动态构建与最近文件列表，这也是 IDE 类应用的标配功能。
