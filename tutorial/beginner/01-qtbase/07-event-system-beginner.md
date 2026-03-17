━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
入门 · QtBase · 07 · 事件系统
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

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

---

📝 口述回答：用自己的话说说，Qt 的事件系统和信号槽机制有什么本质区别？什么时候该用事件，什么时候该用信号槽？

---

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

---

🔲 代码填空：下面是一个自定义按钮的事件过滤器实现，请填空：

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

*提示：双击事件类型、强制转换的目标类型、左键枚举值、返回值含义*

---

## 4. 踩坑清单

> ⚠️ 坑 #1：忘记在 eventFilter 中正确返回值
> ❌ 错误做法：总是返回 `false`，或者忘记 `return` 语句
> ✅ 正确做法：想拦截事件时返回 `true`，否则返回基类的 `eventFilter()` 结果
> 💥 后果：事件要么被意外拦截导致功能失效，要么继续传播导致触发不应该触发的行为
> 💡 一句话记住：`true` 是拦截，`false` 是放行，想清楚再返回

> ⚠️ 坑 #2：在事件处理函数中忘记调用基类实现
> ❌ 错误做法：在 `mousePressEvent()` 中处理完后直接返回
> ✅ 正确做法：处理后调用 `QWidget::mousePressEvent(event)`
> 💥 后果：某些默认行为会失效，比如焦点切换、右键菜单等，排查起来非常困惑
> 💡 一句话记住：自定义事件处理后，记得让基类也处理一下

> ⚠️ 坑 #3：用 sendEvent 跨线程发送事件
> ❌ 错误做法：在工作线程中用 `sendEvent()` 向主线程的 widget 发送事件
> ✅ 正确做法：跨线程必须用 `postEvent()` 或者用信号槽
> 💥 后果：轻则事件处理函数在错误的线程中执行导致崩溃，重则数据竞争引发各种诡异 bug
> 💡 一句话记住：跨线程别用 `sendEvent()`，老老实实用 `postEvent()` 或信号槽

> ⚠️ 坑 #4：事件过滤器安装后忘记卸载
> ❌ 错误做法：给临时对象安装事件过滤器后，过滤器对象生命周期结束了也没卸载
> ✅ 正确做法：确保在过滤器对象销毁前调用 `removeEventFilter()`
> 💥 后果：事件循环会调用已销毁对象的 `eventFilter()`，导致崩溃或内存访问错误
> 💡 一句话记住：安装和卸载要成对，对象销毁前先卸载

---

🐛 调试挑战：下面这段代码有什么问题？它想实现一个点击计数器，但点击没反应。

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

*提示：思考 `installEventFilter(this)` 之后事件流向发生了什么变化*

---

## 5. 本层级练习项目

🎯 练习项目：事件拦截调试面板

📋 功能描述：创建一个简单的调试面板，包含一个 `QLineEdit`、一个 `QTextEdit`、几个按钮。实现一个事件过滤器，能够记录并显示所有发生在这些控件上的键盘和鼠标事件。面板上显示事件的类型、时间戳、以及相关的详细信息（如按键码、鼠标坐标）。

✅ 完成标准：
- 实现一个 `DebugEventFilter` 类，继承自 `QObject`
- 过滤器能够捕获 `KeyPress`、`KeyRelease`、`MouseButtonPress`、`MouseButtonRelease` 事件
- 在 `QTextEdit` 中实时显示捕获到的事件信息，格式为 `[HH:mm:ss.sss] EventType: Details`
- 提供一个「启用/禁用过滤」的复选框，动态安装/卸载事件过滤器
- 验证当过滤器禁用时，控件行为恢复正常

💡 提示：
- 用 `QElapsedTimer` 或 `QTime::currentTime()` 获取时间戳
- `QKeyEvent` 的 `text()` 可以获取按键对应的字符，`key()` 获取虚拟键码
- 动态卸载事件过滤器用 `removeEventFilter()`
- 记得在过滤时根据需要决定返回 `true` 还是 `false`

## 6. 官方文档参考链接

📎 [Qt 文档 · The Event System](https://doc.qt.io/qt-6/eventsandfilters.html) · Qt 事件与过滤器完整文档，必读

📎 [Qt 文档 · QEvent Class](https://doc.qt.io/qt-6/qevent.html) · QEvent 类参考，包含所有事件类型枚举

📎 [Qt 文档 · QEventLoop Class](https://doc.qt.io/qt-6/qeventloop.html) · 事件循环类文档，理解 exec() 机制

📎 [Qt 文档 · QCoreApplication::postEvent](https://doc.qt.io/qt-6/qcoreapplication.html#postEvent) · postEvent 官方说明

📎 [Qt 文档 · QCoreApplication::sendEvent](https://doc.qt.io/qt-6/qcoreapplication.html#sendEvent) · sendEvent 官方说明

---
*本文档版本：v1.0 · 生成于 2026-03-17*
