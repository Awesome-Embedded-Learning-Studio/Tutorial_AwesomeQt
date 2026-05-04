# 现代Qt开发教程（新手篇）3.58——QStatusBar：状态栏

## 1. 前言 / 状态栏是信息分层的第一道门

状态栏这个东西，说起来是桌面应用中最不起眼的组件——它永远趴在窗口最底下，默默显示一些辅助信息，用户甚至不会主动注意到它的存在。但恰恰因为它"一直在那里却不碍事"，状态栏成了应用向用户传递低优先级信息最自然的地方。光标位置、当前操作状态、进度反馈、字符统计、连接状态……这些信息如果用弹窗通知用户，那体验简直是一场灾难；如果完全不显示，用户又会觉得应用对当前状态"一无所知"。状态栏就是解决这个矛盾的方案——它提供一个低干扰的信息展示区域，让需要这些信息的用户随时可以瞥一眼，不需要的时候完全可以无视。

QStatusBar 是 QMainWindow 内置的状态栏组件。和 QToolBar、QMenuBar 一样，它通过 statusBar() 懒创建并获取。QStatusBar 的布局逻辑比较特殊：左侧是临时消息区域（showMessage 显示的文本），中间是通过 addWidget 添加的普通控件，右侧是通过 addPermanentWidget 添加的永久控件。这三块区域的显示优先级和空间分配规则各有不同，理解它们是正确使用状态栏的前提。

今天我们从四个方面展开。先看 showMessage(text, timeout) 的临时消息机制，然后研究 addWidget / addPermanentWidget 嵌入永久控件的方式和区别，接着实现进度条嵌入状态栏的典型模式，最后讨论 clearMessage 与消息优先级的交互行为。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QStatusBar 在 QtWidgets 模块中，链接 Qt6::Widgets 即可。示例代码涉及 QStatusBar、QMainWindow、QLabel、QProgressBar、QPushButton、QPlainTextEdit、QTimer、QTextEdit 和 QAction。

## 3. 核心概念讲解

### 3.1 showMessage(text, timeout)：临时消息机制

showMessage 是 QStatusBar 最基础的功能。调用 showMessage(const QString &text, int timeout = 0) 后，状态栏左侧会显示指定的文本。如果 timeout 大于 0，文本在 timeout 毫秒后自动消失；如果 timeout 为 0（默认值），文本一直显示直到被下一次 showMessage 调用覆盖或者 clearMessage 手动清除。

```cpp
// 显示一条 3 秒后自动消失的临时消息
statusBar()->showMessage("文件已保存", 3000);

// 显示一条持续存在的消息（需要手动清除）
statusBar()->showMessage("正在加载...");

// 手动清除当前消息
statusBar()->clearMessage();
```

临时消息的显示位置在状态栏的最左侧，占据 addWidget 添加的控件之外的所有剩余空间。当 showMessage 被调用时，临时消息会覆盖（而不是挤开）addWidget 添加的控件——准确地说是 addWidget 的控件会被隐藏，只显示临时消息文本。addPermanentWidget 添加的控件不受临时消息影响，它们始终显示在状态栏右侧。

这种行为意味着 showMessage 适合用来显示短暂的、一次性的反馈信息（"文件已保存""操作完成""连接成功"），而需要持续显示的信息（光标位置、字符计数、连接状态指示器）应该用 addWidget 或 addPermanentWidget 来实现。如果你把持续信息也用 showMessage 来显示，当一条临时消息覆盖它时，持续信息就消失了——这不是你想要的效果。

timeout 参数的实现原理是 QStatusBar 内部启动了一个 QTimer，在 timeout 毫秒后自动调用 clearMessage。这意味着 showMessage 的定时器行为和 QTimer 的精度一致——通常足够精确，但在高负载系统中可能有几十毫秒的偏差。对于"显示 3 秒"这种场景，这个偏差完全可以忽略。

```cpp
// 典型的操作反馈模式
void MainWindow::onSaveFile()
{
    // ... 保存逻辑 ...

    if (success) {
        statusBar()->showMessage("已保存: " + filePath, 3000);
    } else {
        statusBar()->showMessage("保存失败: " + errorString, 5000);
    }
}
```

多次调用 showMessage 会覆盖之前的消息——状态栏同一时刻只能显示一条临时消息。如果你在短时间内连续触发了多个操作（比如批量处理文件），每条操作反馈都会覆盖前一条。这在大多数情况下是期望行为（用户只关心最近的状态），但如果你需要累积显示多条消息，需要自己维护一个消息队列或者用 QLabel 来实现。

### 3.2 addWidget / addPermanentWidget：嵌入永久控件

QStatusBar 有两种嵌入永久控件的方式，它们的布局位置和交互行为截然不同。

addWidget(QWidget *widget, int stretch = 0) 把控件添加到状态栏的左侧区域。这个区域同时也是临时消息的显示区域——当 showMessage 被调用时，addWidget 添加的控件会被隐藏，临时消息文本占据整个左侧区域。当 clearMessage 或消息超时后，控件重新显示。这个"被临时消息隐藏"的行为是 addWidget 的核心特征。

addPermanentWidget(QWidget *widget, int stretch = 0) 把控件添加到状态栏的右侧区域。这个区域的控件永远不会被临时消息隐藏——无论 showMessage 显示什么内容，右侧的永久控件始终可见。stretch 参数控制控件在布局中的拉伸系数，和 QHBoxLayout 的 stretch 行为一致。

```cpp
// 左侧：光标位置（会被临时消息隐藏）
auto *positionLabel = new QLabel("行 1, 列 1");
statusBar()->addWidget(positionLabel);

// 右侧：字符计数（始终可见）
auto *charCountLabel = new QLabel("字符数: 0");
statusBar()->addPermanentWidget(charCountLabel);

// 右侧：编码信息（始终可见）
auto *encodingLabel = new QLabel("UTF-8");
statusBar()->addPermanentWidget(encodingLabel);
```

选择用 addWidget 还是 addPermanentWidget 的原则很简单：如果这个控件的信息可以在某些时刻被临时消息覆盖而不影响使用体验（比如光标位置——用户在执行操作时不一定需要看到它），用 addWidget；如果这个控件的信息在任何时候都需要可见（比如进度指示、连接状态、错误指示灯），用 addPermanentWidget。

一个常见的布局模式是：左侧 addWidget 放一个可拉伸的 QLabel 作为永久信息区（比如当前打开的文件名），右侧 addPermanentWidget 放若干固定宽度的信息标签（编码、光标位置、缩放比例）。这样临时消息出现时会覆盖左侧的文件名显示，但右侧的状态信息始终可见。

```cpp
// 左侧：当前文件名（stretch = 1 拉伸占据剩余空间）
m_fileNameLabel = new QLabel("未命名");
statusBar()->addWidget(m_fileNameLabel, 1);

// 右侧：状态信息（依次排列）
m_positionLabel = new QLabel("行 1, 列 1");
statusBar()->addPermanentWidget(m_positionLabel);

m_encodingLabel = new QLabel("UTF-8");
statusBar()->addPermanentWidget(m_encodingLabel);
```

addPermanentWidget 添加的控件从左到右依次排列在状态栏最右侧。addWidget 添加的控件从左到右依次排列在临时消息区域的右边（但在 addPermanentWidget 控件的左边）。整体布局是：[临时消息 / addWidget 控件] ... [addPermanentWidget 控件]。

控件的移除通过 removeWidget(QWidget *widget) 实现。注意 removeWidget 只是从布局中移除控件，不会 delete 它——如果你确定不再需要这个控件，需要手动 delete。

### 3.3 进度条嵌入状态栏

进度条嵌入状态栏是一个经典的 UI 模式——几乎所有桌面应用都在状态栏中显示文件加载、数据导出、网络请求等长时间操作的进度。在 QStatusBar 中嵌入 QProgressBar 非常直接：创建一个 QProgressBar，通过 addWidget 或 addPermanentWidget 添加到状态栏即可。

```cpp
// 创建进度条
m_progressBar = new QProgressBar;
m_progressBar->setRange(0, 100);
m_progressBar->setValue(0);
m_progressBar->setMaximumWidth(200);
m_progressBar->setTextVisible(true);
m_progressBar->setFormat("%p%");

// 添加到状态栏左侧
statusBar()->addWidget(m_progressBar);
```

这里有几个细节值得展开。setMaximumWidth(200) 限制了进度条的宽度——如果不限制，进度条会拉伸占据整个状态栏左侧的可用空间，把其他控件挤到看不见的地方。setFormat("%p%") 设置进度条上显示的文本格式，%p 会被替换为当前百分比。setTextVisible(true) 让这个文本显示在进度条上。

进度条的显示/隐藏控制是一个需要仔细处理的问题。通常的做法是：平时隐藏进度条（m_progressBar->hide()），在需要显示进度时才让它可见，操作完成后再次隐藏。

```cpp
void MainWindow::startLongOperation()
{
    m_progressBar->setValue(0);
    m_progressBar->show();
    statusBar()->showMessage("正在处理...", 0);

    // 模拟长时间操作
    m_operationTimer = new QTimer(this);
    m_operationProgress = 0;
    connect(m_operationTimer, &QTimer::timeout, this, [this]() {
        m_operationProgress += 5;
        m_progressBar->setValue(m_operationProgress);

        if (m_operationProgress >= 100) {
            m_operationTimer->stop();
            m_operationTimer->deleteLater();
            m_progressBar->hide();
            statusBar()->showMessage("操作完成", 3000);
        }
    });
    m_operationTimer->start(50);
}
```

这个例子用 QTimer 模拟了一个分步推进的操作。在实际项目中，进度更新通常来自工作线程（通过信号槽跨线程传递进度值）或者 QProcess / QNetworkReply 等异步操作的进度信号。无论进度来源是什么，核心模式都是一样的：更新 QProgressBar 的 value，操作完成后隐藏进度条。

一个值得注意的细节是进度条应该用 addWidget 还是 addPermanentWidget。大多数应用选择 addWidget——这样当 showMessage 显示临时消息时，进度条被隐藏、消息文本显示出来，不会出现进度条和消息文本挤在一起的尴尬布局。如果你的进度指示是一个"一直可见的状态指示器"（比如 IDE 底部常驻的构建进度条），可以用 addPermanentWidget 让它始终可见。

还有一种更轻量的进度指示方式：QStatusBar 本身不提供内置的进度指示，但你可以用一个小型 QLabel 显示旋转动画（通过 QMovie 加载 GIF）或者简单的"正在加载..."文本，这比完整的 QProgressBar 更节省空间。

### 3.4 clearMessage 与消息优先级

QStatusBar 内部维护了一个消息栈。每次调用 showMessage 时，新消息被推入栈顶；clearMessage 弹出栈顶消息，恢复到栈中的下一条消息（如果有的话）。这意味着 showMessage 的调用是可以嵌套的——这在一个操作触发了另一个需要显示状态的操作时非常有用。

```cpp
// 场景：保存文件时先显示"正在保存..."，完成后显示"已保存"
void MainWindow::onSave()
{
    statusBar()->showMessage("正在保存...", 0);

    // ... 执行保存操作 ...

    statusBar()->showMessage("已保存", 3000);
    // "已保存" 3 秒后消失，恢复到之前的消息状态
}
```

但实际上 QStatusBar 的消息栈只有一层——当前消息。clearMessage 之后恢复的不是"上一条消息"，而是恢复到 addWidget 控件的显示状态。所以消息的优先级逻辑是：临时消息 > addWidget 控件 > addPermanentWidget 控件。showMessage 会隐藏 addWidget 的控件来显示消息文本，clearMessage 或超时后控件恢复显示，addPermanentWidget 的控件始终不受影响。

如果你需要在 clearMessage 之后立即显示另一条消息，只需要再次调用 showMessage——不需要等 clearMessage 完成后再调用。

```cpp
// 错误理解：以为需要先 clearMessage 再 showMessage
statusBar()->clearMessage();
statusBar()->showMessage("新消息", 3000);

// 正确理解：showMessage 直接覆盖，不需要先 clear
statusBar()->showMessage("新消息", 3000);
```

当一条带 timeout 的消息正在显示时，如果你又调用了 showMessage 显示一条新消息，新消息会覆盖旧消息，但旧消息的定时器不会被取消——它仍然会在原来的 timeout 时间后触发 clearMessage，把你刚显示的新消息给清掉。这看起来像是一个 bug，但实际上是 QStatusBar 的设计行为。

```cpp
// 可能的问题场景
statusBar()->showMessage("消息 A", 5000);  // 5 秒后清除

// 1 秒后，用户执行了另一个操作
statusBar()->showMessage("消息 B", 3000);  // 3 秒后清除

// 问题：消息 A 的 5 秒定时器仍然在运行
// 在显示消息 B 的第 4 秒（距离消息 A 已经过去了 5 秒），
// 消息 A 的定时器触发 clearMessage，消息 B 被提前清除了
```

解决这个问题的方法是在每次 showMessage 之前先调用 clearMessage 来清除之前的定时器，或者在应用层面自己管理消息的显示逻辑。如果你确实需要多条消息按顺序显示（比如"步骤 1/3 完成" -> "步骤 2/3 完成" -> "全部完成"），建议自己实现一个消息队列而不是依赖 QStatusBar 内部的消息管理。

currentMessage() 返回当前正在显示的临时消息文本，如果没有临时消息则返回空字符串。你可以用这个方法来判断状态栏当前是否正在显示临时消息。

```cpp
if (statusBar()->currentMessage().isEmpty()) {
    // 状态栏没有显示临时消息，addWidget 的控件应该是可见的
    positionLabel->setText("行 5, 列 12");
}
```

isSizeGripEnabled() 控制状态栏右下角是否显示一个尺寸调整把手（Size Grip）。默认是启用的——在非最大化窗口中，用户可以拖拽这个把手来调整窗口大小。如果你不需要这个功能（比如窗口大小是固定的），可以禁用它：statusBar()->setSizeGripEnabled(false)。

## 4. 踩坑预防

第一个坑是 addWidget 的控件在 showMessage 时被隐藏这个问题。很多新手不知道这个行为，把关键的状态指示器（比如"未保存"提示、连接状态灯）通过 addWidget 添加，结果当一条临时消息出现时这些指示器就不见了。如果你的控件需要在任何时候都可见，务必使用 addPermanentWidget。

第二个坑是进度条的宽度管理。如果不给 QProgressBar 设置 maximumWidth，它会无限制地拉伸，把状态栏中其他控件的空间全部吃掉。建议给嵌入状态栏的进度条设置一个合理的 maximumWidth（通常 150-250 像素），或者在不需要时 hide() 掉。

第三个坑是 showMessage 的 timeout 定时器覆盖问题。连续调用多次 showMessage 时，旧消息的定时器不会被新消息取消——多个定时器可能同时存在，导致消息被意外清除。解决方法是在 showMessage 之前先 clearMessage，或者自己管理消息显示逻辑。

第四个坑是状态栏的高度。QStatusBar 的高度由其内部控件的 sizeHint 决定。如果你添加了一个很高的控件（比如一个没设固定高度的 QFrame），状态栏会被撑高，挤压中央区域的可用空间。建议所有嵌入状态栏的控件都设置合理的 fixedHeight 或 maximumHeight（通常 20-24 像素）。

第五个坑是 QStatusBar 在 QMainWindow 之外的使用。虽然 QStatusBar 可以脱离 QMainWindow 单独使用（它本质上就是一个 QWidget），但如果你把 QStatusBar 放到普通的 QWidget 布局中，它不会有 QMainWindow 中那样的自动管理行为（比如 statusBar() 的懒创建、状态栏自动占据底部等）。如果你不需要 QMainWindow 的完整功能但需要状态栏，可以手动 new QStatusBar 并用 QVBoxLayout 把它放到布局底部。

## 5. 练习项目

我们来做一个综合练习：创建一个 QMainWindow 应用，中央区域使用 QPlainTextEdit 作为文本编辑器。状态栏布局分为三个部分：左侧通过 addWidget 放置一个 QLabel 显示当前光标位置（行号、列号），中间通过 addWidget 放置一个 QProgressBar（平时隐藏，点击工具栏的"模拟加载"按钮时显示并推进到 100%），右侧通过 addPermanentWidget 放置两个 QLabel 分别显示字符总数和文件编码。工具栏包含"新建""保存""模拟加载"三个按钮。"新建"按钮清空编辑器并 showMessage("已新建文件", 2000)，"保存"按钮 showMessage("已保存", 3000)，"模拟加载"按钮启动 QTimer 模拟进度推进。编辑器的 cursorPositionChanged 信号更新光标位置标签，textChanged 信号更新字符计数。

提示：QPlainTextEdit 的 textCursor(). blockNumber() 返回从 0 开始的行号，positionInBlock() 返回从 0 开始的列号。使用时各加 1 转换为用户友好的显示。

## 6. 官方文档参考链接

[Qt 文档 -- QStatusBar](https://doc.qt.io/qt-6/qstatusbar.html) -- 状态栏类

[Qt 文档 -- QProgressBar](https://doc.qt.io/qt-6/qprogressbar.html) -- 进度条控件

[Qt 文档 -- QLabel](https://doc.qt.io/qt-6/qlabel.html) -- 标签控件

[Qt 文档 -- QMainWindow](https://doc.qt.io/qt-6/qmainwindow.html) -- 主窗口类

[Qt 文档 -- QTimer](https://doc.qt.io/qt-6/qtimer.html) -- 定时器

---

到这里，QStatusBar 的核心用法就全部讲完了。showMessage 显示临时反馈消息，addWidget 嵌入可被临时消息覆盖的信息控件，addPermanentWidget 嵌入始终可见的永久状态控件，进度条嵌入状态栏是长时间操作反馈的标准模式，clearMessage 与消息优先级决定了状态栏的信息显示策略。把这些组合起来，就能搭建出一个信息丰富、层次分明的状态栏系统。
