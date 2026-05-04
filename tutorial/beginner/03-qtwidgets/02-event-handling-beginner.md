# 现代Qt开发教程（新手篇）3.2——事件处理与传播基础

## 1. 前言 / 理解 Qt 的事件驱动模型

在 Qt 中，几乎所有的用户交互和系统通知都是通过事件来传递的——鼠标点击是 QMouseEvent，键盘按下是 QKeyEvent，窗口大小改变是 QResizeEvent，定时器触发是 QTimerEvent。你写的每一个 Qt 程序，底层都有一个事件循环在不停地跑：`QApplication::exec()` 启动事件循环，操作系统把各种输入事件投递到 Qt 的事件队列，Qt 再根据事件的类型和目标对象把它们分发出去。

理解事件处理机制，是从"能用 Qt 写界面"到"真正理解 Qt 在干什么"的关键一步。很多看起来莫名其妙的问题——为什么我的键盘事件没触发？为什么子控件的鼠标事件被父控件吞了？为什么重写了 paintEvent 但画面不更新？——归根结底都是事件传播的问题。

这篇文章我们先把最常用的几种事件（鼠标、键盘、resize）的重写方法搞清楚，然后深入讨论 `accept()` 和 `ignore()` 如何控制事件在父子控件之间的传播链，最后看看事件过滤器怎么让你在不修改子控件代码的情况下拦截它的事件。

## 2. 环境说明

本篇代码适用于 Qt 6.5+ 版本，CMake 3.26+，C++17 或更高标准。所有事件类分布在 QtGui（QMouseEvent、QKeyEvent、QResizeEvent 等）和 QtCore（QEvent 基类、QCoreApplication 的事件投递方法）模块中，但因为我们的示例需要 QWidget 作为事件接收对象，所以需要链接 Widgets 和 Gui 两个模块。桌面平台均可正常编译运行。

## 3. 核心概念讲解

### 3.1 重写 mousePressEvent、keyPressEvent 和 resizeEvent

事件处理最基本的做法就是重写 QWidget 的虚函数。当 Qt 把事件分发到一个 Widget 时，它会调用这个 Widget 对应的虚函数。你只需要在子类中 override 这些函数，就能捕获到你关心的事件。

鼠标事件有几个相关的虚函数：`mousePressEvent` 在鼠标按下时触发，`mouseReleaseEvent` 在松开时触发，`mouseMoveEvent` 在按住鼠标移动时触发，`mouseDoubleClickEvent` 在双击时触发。最常用的是 press 和 move。

```cpp
class ClickWidget : public QWidget
{
    Q_OBJECT

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            qDebug() << "左键点击位置:" << event->pos();
        } else if (event->button() == Qt::RightButton) {
            qDebug() << "右键点击位置:" << event->pos();
        }
        // 调用基类实现，保证默认行为仍然生效
        QWidget::mousePressEvent(event);
    }
};
```

这里有几个要点。`event->button()` 返回触发这次事件的鼠标按键，`event->pos()` 返回鼠标相对于当前 Widget 的坐标。还有一个容易搞混的地方：`mouseMoveEvent` 默认只在鼠标按住的时候才会触发。如果你想在鼠标没按下的时候也能追踪鼠标位置，需要先调用 `setMouseTracking(true)`。

键盘事件的重写方式类似。`keyPressEvent` 在按键按下时触发，`keyReleaseEvent` 在松开时触发。你通过 `event->key()` 获取按下的键值，通过 `event->modifiers()` 获取修饰键状态（Ctrl、Shift、Alt 等）。

```cpp
void keyPressEvent(QKeyEvent *event) override
{
    if (event->key() == Qt::Key_Escape) {
        close();  // ESC 关闭窗口
    } else if (event->key() == Qt::Key_Space) {
        qDebug() << "空格键被按下";
    } else if (event->modifiers() & Qt::ControlModifier
               && event->key() == Qt::Key_S) {
        qDebug() << "Ctrl+S 保存";
    }
    QWidget::keyPressEvent(event);
}
```

处理组合键的时候，先检查 `modifiers()` 再检查 `key()` 的顺序很重要。`event->modifiers()` 返回的是一个位掩码，用 `&` 按位与来检查某个修饰键是否按下。

`resizeEvent` 在 Widget 大小改变时触发。这个事件在窗口初始化、用户拖拽窗口边框、或者布局系统重新分配空间的时候都会被调用。

```cpp
void resizeEvent(QResizeEvent *event) override
{
    qDebug() << "旧尺寸:" << event->oldSize()
             << "新尺寸:" << event->size();
    QWidget::resizeEvent(event);
}
```

注意我们在所有重写函数的最后都调用了 `QWidget::xxxEvent(event)`。这不仅仅是一个好习惯，它和事件传播机制直接相关——我们在下一节详细讲。

### 3.2 accept 和 ignore：控制事件传播链

这是 Qt 事件系统中最核心也最容易被误解的概念。Qt 的事件不是只发给一个 Widget 就结束了——它们会沿着父子关系形成的对象树传播。传播的方向取决于事件类型：大部分输入事件（鼠标、键盘）是从子到父传播的，也就是先发给最内层的子控件，如果子控件不处理，就传给它的父控件，再传给父控件的父控件，一直往上冒泡。

控制这个传播行为的就是 `event->accept()` 和 `event->ignore()`。

调用 `event->accept()` 表示"这个事件我已经处理了，不需要继续传播"。调用 `event->ignore()` 表示"这个事件我不处理，请传给我的父对象"。

默认情况下，当你重写了一个事件处理函数且没有调用 accept 或 ignore 时，QWidget 的基类实现会自动调用 accept（对大部分事件类型而言）。但如果你在重写的函数中调用了基类实现 `QWidget::mousePressEvent(event)`，而基类实现发现你并没有真正处理这个事件（比如鼠标点击的位置不在任何子控件上），它可能会调用 ignore 把事件传给父控件。

这个机制的实际意义是：你可以在父控件中处理子控件没有处理的事件。比如一个自定义的面板，面板上有很多按钮和输入框，但面板的空白区域点击时你想弹出一个上下文菜单。这时候你不需要重写每个子控件的事件——子控件的点击事件被它们自己 accept 了不会传上来，但空白区域的点击事件会冒泡到面板层，你在面板的 `mousePressEvent` 里就能捕获到。

```cpp
// 子控件：点击按钮被处理，事件不会传播
void ButtonWidget::mousePressEvent(QMouseEvent *event)
{
    // 处理按钮点击逻辑...
    event->accept();  // 明确标记已处理，阻止传播
}

// 父控件：只收到子控件没有处理的点击事件
void PanelWidget::mousePressEvent(QMouseEvent *event)
{
    // 这里只会收到子控件 ignore 的事件（比如空白区域的点击）
    if (event->button() == Qt::RightButton) {
        showContextMenu(event->globalPos());
    }
    QWidget::mousePressEvent(event);
}
```

反过来，如果你想确保事件一定会传播到父控件（即使你自己在子控件中也处理了它），可以在处理完你的逻辑之后显式调用 `event->ignore()`。但这种情况比较少见，大部分时候 accept/ignore 的默认行为就是对的。

有一个特殊情况需要注意：`QKeyEvent` 的传播。如果你的 Widget 上有按钮之类的控件，按钮本身会 accept 键盘事件，所以你的 Widget 的 `keyPressEvent` 可能收不到某些按键。这时候你需要确认：你的 Widget 是否有焦点（`hasFocus()`），或者你是否需要调用 `setFocusPolicy(Qt::StrongFocus)` 来让你的 Widget 可以接收键盘焦点。

### 3.3 installEventFilter：拦截子控件事件

事件过滤器是 Qt 提供的一种更灵活的事件拦截机制。它允许你在一个对象上监视另一个对象的所有事件，而不需要修改那个对象的代码。这在很多场景下非常有用——比如你想给多个不同的控件统一添加某种行为，或者你想在一个容器层面拦截所有子控件的事件做统一处理。

使用事件过滤器分两步：先在目标对象上调用 `installEventFilter()`，指定谁来过滤它的事件；然后在过滤器对象的 `eventFilter()` 方法中实现过滤逻辑。

```cpp
class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow()
    {
        auto *lineEdit = new QLineEdit(this);
        // 在 lineEdit 上安装事件过滤器，this（MainWindow）是过滤器
        lineEdit->installEventFilter(this);
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        // 检查被监视的对象和事件类型
        if (watched == m_lineEdit && event->type() == QEvent::KeyPress) {
            auto *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Return) {
                qDebug() << "回车键被拦截!";
                return true;  // 返回 true 表示事件被消费，不再传递
            }
        }
        // 返回 false 表示不拦截，继续正常的事件处理流程
        return QWidget::eventFilter(watched, event);
    }

private:
    QLineEdit *m_lineEdit = nullptr;
};
```

`eventFilter` 的返回值是理解事件过滤器的关键。返回 `true` 表示这个事件被你消费了，它不会继续传递到目标对象的事件处理函数。返回 `false` 表示你不处理这个事件，让它继续正常传递。

事件过滤器的执行顺序是这样的：当一个事件到达目标对象之前，Qt 会先调用该对象上安装的所有事件过滤器的 `eventFilter()`。只有所有过滤器都返回 false（都不拦截），事件才会到达目标对象自身的 `xxxEvent()` 处理函数。这意味着事件过滤器的优先级比对象自身的事件处理函数更高。

一个常见的使用场景是给多个控件统一添加快捷键或者输入验证。比如你有五个 QLineEdit，想限制它们只能输入数字。与其给每个 QLineEdit 写一个子类，不如在父窗口上给它们全部安装事件过滤器，统一在 `eventFilter` 里判断按键是否合法：

```cpp
// 安装过滤器
for (auto *edit : m_numericEdits) {
    edit->installEventFilter(this);
}

// 统一拦截逻辑
bool eventFilter(QObject *watched, QEvent *event) override
{
    if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        // 允许退格、删除、方向键、Ctrl+A 等控制键
        if (keyEvent->key() == Qt::Key_Backspace
            || keyEvent->key() == Qt::Key_Delete
            || keyEvent->key() == Qt::Key_Tab) {
            return false;  // 放行
        }
        // 允许数字键
        if (keyEvent->text().at(0).isDigit()) {
            return false;  // 放行
        }
        // 其他按键拦截
        return true;
    }
    return QWidget::eventFilter(watched, event);
}
```

你还可以给一个对象安装多个事件过滤器，它们的调用顺序是后安装的先调用（栈式顺序）。在不需要过滤器的时候，调用 `removeEventFilter()` 卸载即可。

### 3.4 sendEvent 和 postEvent 的区别

Qt 提供了两种手动向事件队列投递事件的方式：`QCoreApplication::sendEvent()` 和 `QCoreApplication::postEvent()`。它们的区别非常关键。

`sendEvent` 是同步的——它直接调用目标对象的 `event()` 方法，在当前线程中立即执行。调用返回的时候，事件已经被处理完了。你可以把它理解成一次直接的函数调用，只不过走的是 Qt 的事件分发通道。

```cpp
// 同步投递：立即处理
QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
QCoreApplication::sendEvent(targetWidget, &keyPress);
// 到这里事件已经处理完了
```

`postEvent` 是异步的——它把事件放到事件队列中，等当前的事件处理完成之后，事件循环才会从队列中取出并分发。`postEvent` 返回的时候，事件还没被处理。

```cpp
// 异步投递：稍后处理
QKeyEvent *keyPress = new QKeyEvent(QEvent::KeyPress, Qt::Key_A,
                                     Qt::NoModifier);
QCoreApplication::postEvent(targetWidget, keyPress);
// 事件在队列中排队，当前函数返回后才可能被处理
```

注意一个重要的区别：`sendEvent` 接收事件对象的指针（不拥有所有权），`postEvent` 接收事件对象的指针并且会自动 delete 它（拥有所有权）。所以 `sendEvent` 可以用栈上的事件对象，而 `postEvent` 必须 new 一个堆上的对象。

日常开发中你需要手动投递事件的场景并不多。最常见的用途是自动化测试——用 `sendEvent` 模拟用户的键盘和鼠标操作，来测试你的界面逻辑。另一个用途是在多线程编程中，工作线程通过 `postEvent` 向主线程发送自定义事件来通知状态变化（不过更推荐用信号槽，代码更清晰）。

到这里你可以想一个问题：当用户在一个按钮上点击鼠标时，事件经历了怎样的旅程？从操作系统的原始输入到你的 `mousePressEvent` 被调用，中间经过了哪些步骤？如果你在按钮的父 Widget 上安装了事件过滤器，这个过滤器什么时候被调用？把这些环节串起来，Qt 事件系统的工作方式你就真正理解了。

## 4. 踩坑预防

第一个坑是重写事件处理函数但忘了调用基类实现。很多人在重写 `resizeEvent` 的时候只写了自己的逻辑，忘了 `QWidget::resizeEvent(event)` 这一行。在大多数简单场景下这似乎没什么问题，因为 QWidget 的默认 resizeEvent 不做什么特别的事。但在复杂的继承层次中（比如你继承了一个自定义控件），基类可能在自己的 resizeEvent 里做了重要的布局更新。不调用基类实现就会跳过这些逻辑，导致界面不更新或者布局错乱。养成习惯：重写事件函数的时候，总是在末尾调用 `QWidget::xxxEvent(event)`。

第二个坑是 `mouseMoveEvent` 不触发。默认情况下，Qt 只在鼠标按下状态下才发送 mouseMoveEvent。如果你需要在鼠标没按下的时候也追踪鼠标位置（比如实现一个跟随鼠标的提示效果），必须在构造函数里调用 `setMouseTracking(true)`。这个设置太容易被忽略了。

第三个坑是键盘事件不触发。键盘事件只发给当前拥有焦点的 Widget。如果你的 Widget 上有按钮、文本框等控件，焦点通常在那些控件上。你需要在你的 Widget 上调用 `setFocusPolicy(Qt::StrongFocus)` 并且 `setFocus()` 才能收到键盘事件。如果你的 Widget 只是一个普通的面板（而不是输入控件），Qt 不会自动把焦点给它。

第四个坑是事件过滤器中忘记检查 `watched` 对象。`eventFilter` 会对所有被监视的对象的所有事件调用，如果你不先判断 `watched` 是哪个对象，你的过滤逻辑可能会作用到错误的控件上。养成习惯：eventFilter 的第一行永远是判断 `watched` 和 `event->type()`。

## 5. 练习项目

我们来做一个综合练习：创建一个自定义的画板 Widget，能够响应鼠标和键盘事件，并且父窗口通过事件过滤器给画板添加额外的行为。

完成标准是：画板 Widget 重写 `mousePressEvent` 记录起始点、`mouseMoveEvent` 实时画线（需要设置 `setMouseTracking(true)` 或在按下状态下追踪）、`keyPressEvent` 响应 C 键清空画板、R 键切换画笔颜色；画板被一个 MainWindow 包裹，MainWindow 通过 `installEventFilter` 监听画板的键盘事件，在画板收到 Ctrl+Z 时撤销最后一条线；MainWindow 底部显示一个状态栏，通过重写画板的 `resizeEvent` 在状态栏中实时显示画板尺寸。

几个提示：画线可以用 `QPainter` 在 `paintEvent` 里画，维护一个 `QList<QLine>` 存储所有已画的线段；撤销功能就是从列表中移除最后一个线段然后 `update()`；事件过滤器和画板自身的 `keyPressEvent` 不冲突——过滤器只拦截 Ctrl+Z，其他按键正常传递到画板。

## 6. 官方文档参考链接

[Qt 文档 · The Event System](https://doc.qt.io/qt-6/eventsandfilters.html) -- Qt 事件系统的完整概述，涵盖事件分发、传播、过滤的全部机制

[Qt 文档 · QMouseEvent](https://doc.qt.io/qt-6/qmouseevent.html) -- 鼠标事件文档，包含 button()、pos()、globalPos() 等坐标和按键信息

[Qt 文档 · QKeyEvent](https://doc.qt.io/qt-6/qkeyevent.html) -- 键盘事件文档，包含 key()、modifiers()、text() 等属性

[Qt 文档 · QResizeEvent](https://doc.qt.io/qt-6/qresizeevent.html) -- 尺寸变化事件文档，包含 size() 和 oldSize()

[Qt 文档 · QObject::installEventFilter](https://doc.qt.io/qt-6/qobject.html#installEventFilter) -- 事件过滤器安装方法，以及 eventFilter 的返回值语义

[Qt 文档 · QCoreApplication::postEvent](https://doc.qt.io/qt-6/qcoreapplication.html#postEvent) -- 异步事件投递文档，包含事件队列和所有权说明

---

到这里，Qt 事件处理的机制你算是有一个整体认识了。重写事件函数是最直接的处理方式，accept 和 ignore 控制传播方向，事件过滤器提供了不修改子控件代码就能拦截事件的能力。掌握了这三层，后面遇到任何事件相关的需求你都能找到合适的切入点。下一篇我们会进入 Model/View 架构，那才是 Qt 数据展示和编辑的核心设计模式。
