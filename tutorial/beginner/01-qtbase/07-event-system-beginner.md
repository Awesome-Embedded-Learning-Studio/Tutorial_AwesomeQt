# 现代Qt开发教程（新手篇）1.7——事件系统

## 1. 前言——事件是什么

说实话，刚学 Qt 的时候我对「事件」这个东西特别困惑。前面刚学了信号槽，感觉什么都能用信号槽解决，怎么又冒出来一个事件系统？这俩到底有啥区别？后来熬夜调试了几次鼠标点击没反应、键盘输入被吞掉的问题后，我才真正明白——事件是 Qt 整个 GUI 框架的神经末梢，而信号槽更像是对象之间的高层通信协议。事件是底层驱动的，信号槽是业务逻辑的。搞不清这个，写自定义控件的时候一定会踩坑。

这篇文章会带你从零搞懂 Qt 事件系统的核心概念：QEvent 是什么、事件循环怎么跑起来的、事件怎么在对象之间传播、怎么拦截事件、postEvent 和 sendEvent 有啥区别。学完之后，你写自定义控件就不会再对着 `mousePressEvent()` 发呆了。

## 2. 环境说明

本文基于 Qt 6.x，示例代码使用 CMake 构建。事件系统是 Qt Core 模块的核心部分，不依赖 GUI 模块，但 GUI 事件（如鼠标、键盘）需要 QtWidgets 或 QtGui 模块支持。

## 3. 核心概念

### 3.1 QEvent 与事件循环

Qt 的整个 GUI 应用程序其实就是一个大循环——这就是事件循环。当你写完 `return a.exec();` 之后，程序并没有结束，而是进入了一个永无止境的循环：等待事件 -> 处理事件 -> 等待事件。这个循环在 Qt 里叫 `QEventLoop`，而它处理的东西就是 `QEvent`。

`QEvent` 本身是个基类，真正干活的是它的子类：`QMouseEvent`、`QKeyEvent`、`QTimerEvent`、`QResizeEvent` 等等。当你在窗口上点一下鼠标，操作系统会捕获这个操作，Qt 把它包装成一个 `QMouseEvent`，然后扔进事件队列，事件循环再把它分发出去。

事件分发的大致流程是这样的：事件循环从队列取出事件 -> 调用 `QCoreApplication::notify()` -> `notify()` 把事件发给目标对象的 `event()` 方法 -> `event()` 根据事件类型分发给具体的事件处理函数（比如 `mousePressEvent()`）。

```cpp
// 伪代码：事件分发的简化流程
void QCoreApplication::processEvents() {
    while (!eventQueue.isEmpty()) {
        QEvent *event = eventQueue.dequeue();
        QObject *receiver = event->receiver;
        // 关键调用：notify 把事件发送给接收者
        notify(receiver, event);
    }
}

bool QObject::event(QEvent *e) {
    switch (e->type()) {
        case QEvent::MouseButtonPress:
            return mousePressEvent(static_cast<QMouseEvent*>(e));
        case QEvent::KeyPress:
            return keyPressEvent(static_cast<QKeyEvent*>(e));
        // ... 更多事件类型
    }
    return false;
}
```

### 3.2 事件处理函数

处理事件有两种方式：一种是重写 `event()` 虚函数，另一种是重写具体的事件处理函数，比如 `mousePressEvent()`、`keyPressEvent()`。大多数情况下，我们用第二种方式，因为更直接。

```cpp
class MyWidget : public QWidget {
protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            qDebug() << "左键点击在" << event->pos();
        }
        // 记得调用基类实现，否则默认行为可能丢失
        QWidget::mousePressEvent(event);
    }
};
```

这里有个重要细节：`event->accept()` 和 `event->ignore()`。这两个方法控制事件是否继续传播。`accept()` 表示「我已经处理了，事件到此为止」，`ignore()` 表示「我不处理，传给下一个」。对于鼠标点击，如果子 widget `ignore()` 了，事件可能会传给父 widget。

```cpp
void MyLabel::mousePressEvent(QMouseEvent *event) {
    if (isClickable) {
        event->accept();  // 我处理了
        emit clicked();
    } else {
        event->ignore();  // 让父组件处理
    }
}
```

说到这里，你可以想一想：Qt 的事件系统和信号槽机制有什么本质区别？什么时候该用事件，什么时候该用信号槽？其实答案就藏在前面的分析里——事件是 Qt 内部的分发机制，处理的是底层输入（鼠标、键盘、定时器），而信号槽是对象间的高层通信。自定义控件需要处理底层输入时重写事件处理函数，组件之间需要通信时用信号槽。两者并不冲突，信号槽的很多底层实现本身就依赖事件系统。

### 3.3 事件过滤器

事件过滤器是个很强大的机制——它让你可以在一个对象身上监听另一个对象的事件。这比继承更灵活，因为你可以动态安装和卸载过滤器。

使用场景比如：你有一个对话框，想禁用里面所有 `QLineEdit` 的回车键，但不想改每个 `QLineEdit` 的子类。这时就可以给对话框安装一个事件过滤器，过滤所有子控件的键盘事件。

```cpp
// 安装事件过滤器
lineEdit->installEventFilter(this);

// 在过滤对象中重写 eventFilter()
bool MyDialog::eventFilter(QObject *watched, QEvent *event) {
    if (watched == lineEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return) {
            qDebug() << "拦截了回车键";
            return true;  // true 表示事件已被处理，不再传播
        }
    }
    return QDialog::eventFilter(watched, event);
}
```

`eventFilter()` 返回 `true` 表示事件被拦截，返回 `false` 表示继续正常传播。事件过滤器链是有顺序的，后安装的过滤器先执行。

这里有个非常容易踩的坑：`eventFilter()` 的返回值一定要想清楚再写。返回 `true` 是拦截，事件到此为止；返回 `false` 是放行，事件继续传播。如果你总是无脑返回 `false`，或者干脆忘了写 `return` 语句，结果就是事件要么被意外拦截导致功能失效，要么继续传播触发了不该触发的行为。正确的做法是，想拦截时返回 `true`，否则返回基类的 `eventFilter()` 结果，让 Qt 的默认处理逻辑继续工作。

### 3.4 postEvent vs sendEvent

这是新手最容易混淆的两个函数。`QCoreApplication::postEvent()` 和 `sendEvent()` 都能发送事件，但行为完全不同。

`postEvent()` 是异步的——它把事件放入事件队列就立即返回，事件会在稍后被事件循环处理。这是线程安全的，可以跨线程发送事件。而且 `postEvent()` 只接受堆上分配的事件（用 `new` 创建的），因为 Qt 会在事件处理完后自动删除它。

```cpp
QKeyEvent *keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
QCoreApplication::postEvent(receiver, keyEvent);  // 异步，自动删除
```

`sendEvent()` 是同步的——它立即调用接收者的 `event()` 方法，等待处理完成后才返回。它不能跨线程使用，但可以接受栈上分配的事件，不会自动删除。

```cpp
QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
QCoreApplication::sendEvent(receiver, &keyEvent);  // 同步，不会删除
```

简单记：`postEvent()` 是「发个消息就走」，`sendEvent()` 是「等着对方处理完」。跨线程必须用 `postEvent()`，同线程内如果需要立即处理结果用 `sendEvent()`。

这里要特别提醒一下：如果你在工作线程中用 `sendEvent()` 向主线程的 widget 发送事件，轻则事件处理函数在错误的线程中执行导致崩溃，重则数据竞争引发各种诡异 bug。跨线程别用 `sendEvent()`，老老实实用 `postEvent()` 或者用信号槽。

### 3.5 常见事件类型

Qt 定义了几十种事件类型，这里列出几个最常用的：

| 事件类型 | 对应类 | 典型用途 |
|---------|--------|---------|
| `QEvent::MouseButtonPress` | `QMouseEvent` | 鼠标按下 |
| `QEvent::MouseMove` | `QMouseEvent` | 鼠标移动 |
| `QEvent::KeyPress` | `QKeyEvent` | 键盘按键 |
| `QEvent::Resize` | `QResizeEvent` | 窗口大小改变 |
| `QEvent::Timer` | `QTimerEvent` | 定时器触发 |
| `QEvent::Paint` | `QPaintEvent` | 需要重绘 |
| `QEvent::Close` | `QCloseEvent` | 窗口关闭 |

可以通过重写对应的事件处理函数来响应这些事件。需要注意的是，`QPaintEvent` 比较特殊，不能直接用 `postEvent()` 或 `sendEvent()` 发送，只能通过 `update()` 或 `repaint()` 触发。

你可以试着填一下下面这个练习：下面是一个自定义按钮的事件过滤器实现，看看你能不能把空填上——双击事件类型对应的枚举是什么？强制转换的目标类型是什么？左键枚举值怎么写？返回值应该填什么才能拦截事件、或者继续传播？

```cpp
bool MyFilter::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::______) {
        QMouseEvent *mouseEvent = static_cast<______>(event);
        if (mouseEvent->button() == Qt::______Button) {
            qDebug() << "检测到左键双击";
            return ______;  // 拦截事件
        }
    }
    return ______;  // 继续传播
}
```

答案依次是：`MouseButtonDblClick`、`QMouseEvent`、`Left`、`true`、基类的返回值（比如 `QObject::eventFilter(watched, event)`）。

## 4. 踩坑清单

除了前面提到的 `eventFilter()` 返回值问题，还有几个常见的坑值得说一说。

第一个是在事件处理函数中忘记调用基类实现。很多人在 `mousePressEvent()` 里处理完自己的逻辑就直接返回了，没有调用 `QWidget::mousePressEvent(event)`。后果是某些默认行为会失效，比如焦点切换、右键菜单等，而且排查起来非常困惑，因为表面上代码并没有写错。养成习惯就好：自定义事件处理完成后，记得让基类也处理一下。

第二个坑是事件过滤器安装后忘记卸载。给临时对象安装了事件过滤器，结果过滤器对象的生命周期结束了也没卸载。事件循环会尝试调用已经销毁的对象的 `eventFilter()`，直接导致崩溃或内存访问错误。安装和卸载要成对出现，确保在过滤器对象销毁前调用 `removeEventFilter()`。

接下来看一段有点意思的调试代码。下面这段代码想实现一个点击计数器，但点击没反应，你可以想想问题出在哪里：

```cpp
class ClickCounter : public QWidget {
    Q_OBJECT
public:
    ClickCounter(QWidget *parent = nullptr) : QWidget(parent), count(0) {
        installEventFilter(this);  // 监听自己的事件
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (event->type() == QEvent::MouseButtonPress) {
            count++;
            update();
            return true;
        }
        return false;
    }

    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.drawText(rect(), Qt::AlignCenter, QString("Clicks: %1").arg(count));
    }

private:
    int count;
};
```

问题出在 `installEventFilter(this)` 上——一个对象给自己安装事件过滤器，这意味着所有事件在到达 `event()` 之前都会先经过 `eventFilter()`。在这段代码里 `eventFilter()` 拦截了鼠标事件并返回 `true`，所以 `event()` 和后续的 `mousePressEvent()` 都不会被执行。表面上看这没问题，但 `return false` 分支没有调用基类的 `eventFilter()`，会导致其他事件也失去默认处理。更好的做法是不要让一个对象过滤自己的事件，或者在 `return false` 的地方改为 `return QWidget::eventFilter(watched, event)`。

## 5. 本层级练习项目

练习项目：事件拦截调试面板。

我们要创建一个简单的调试面板，包含一个 `QLineEdit`、一个 `QTextEdit`、几个按钮。核心任务是实现一个事件过滤器，能够记录并显示所有发生在这些控件上的键盘和鼠标事件。面板上需要显示事件的类型、时间戳、以及相关的详细信息（如按键码、鼠标坐标）。

完成标准是这样的：实现一个 `DebugEventFilter` 类，继承自 `QObject`；过滤器能够捕获 `KeyPress`、`KeyRelease`、`MouseButtonPress`、`MouseButtonRelease` 事件；在 `QTextEdit` 中实时显示捕获到的事件信息，格式为 `[HH:mm:ss.sss] EventType: Details`；提供一个启用/禁用过滤的复选框，动态安装/卸载事件过滤器；验证当过滤器禁用时，控件行为恢复正常。

几个实现提示：用 `QElapsedTimer` 或 `QTime::currentTime()` 获取时间戳；`QKeyEvent` 的 `text()` 可以获取按键对应的字符，`key()` 获取虚拟键码；动态卸载事件过滤器用 `removeEventFilter()`；记得在过滤时根据需要决定返回 `true` 还是 `false`。

## 6. 官方文档参考链接

[Qt 文档 · The Event System](https://doc.qt.io/qt-6/eventsandfilters.html) -- Qt 事件与过滤器完整文档，必读

[Qt 文档 · QEvent Class](https://doc.qt.io/qt-6/qevent.html) -- QEvent 类参考，包含所有事件类型枚举

[Qt 文档 · QEventLoop Class](https://doc.qt.io/qt-6/qeventloop.html) -- 事件循环类文档，理解 exec() 机制

[Qt 文档 · QCoreApplication::postEvent](https://doc.qt.io/qt-6/qcoreapplication.html#postEvent) -- postEvent 官方说明

[Qt 文档 · QCoreApplication::sendEvent](https://doc.qt.io/qt-6/qcoreapplication.html#sendEvent) -- sendEvent 官方说明

---
*本文档版本：v1.0 · 生成于 2026-03-17*
