# 现代Qt开发教程（新手篇）1.11——定时器

## 1. 前言：为什么需要定时器

说实话，我第一次需要在程序里做「定时任务」的时候，第一反应是写一个死循环里面加 `sleep()`。结果试完就后悔了——界面卡死，程序无响应，任务管理器直接显示「未响应」。

后来我才发现，Qt 早就给我们准备了一个优雅的解决方案：QTimer。它不仅能让你的程序在指定时间后执行某个任务，还能周期性地重复执行，而且最重要的是——它不会阻塞你的界面。

想象一下这些场景：你需要每秒更新一次时钟显示、需要延迟 3 秒后自动关闭提示框、需要每隔 100ms 检查一次传感器数据、需要做一个简单的动画效果。这些场景如果用 `sleep()` 或者系统原生定时器 API，你会陷入跨平台兼容性的地狱。而 QTimer 把这一切都封装好了，Windows、Linux、macOS 通用，而且和 Qt 的事件循环完美集成。

所以这一篇，我们不玩虚的，直接把 QTimer 的核心用法搞清楚。从最简单的单次定时，到重复定时、高精度定时，再到常见的坑，我们都一一覆盖。

## 2. 环境说明

本文档基于 Qt 6.x 编写，所有示例代码和 API 调用都已验证兼容 Qt 6.2+ 版本。

定时器功能位于 Qt Core 模块中，不需要额外链接 GUI 模块。你可以在命令行程序、GUI 程序，甚至自定义的事件循环中使用它。

## 3. 核心概念讲解

### 3.1 QTimer 基础用法

QTimer 的使用非常简单。它本质上是一个「倒计时器」，倒计时结束后会发射一个 `timeout()` 信号。你只需要把这个信号连接到你的槽函数，就可以实现定时执行任务。

```cpp
QTimer *timer = new QTimer(this);

// 设置定时间隔（毫秒）
timer->setInterval(1000);  // 1000 毫秒 = 1 秒

// 连接 timeout 信号到你的槽
connect(timer, &QTimer::timeout, []() {
    qDebug() << "定时器触发了！";
});

// 启动定时器
timer->start();
```

`setInterval()` 设置的是两次触发之间的时间间隔，单位是毫秒。`start()` 启动定时器，之后每隔指定的间隔，`timeout()` 信号就会被发射一次。当你不需要定时器时，调用 `stop()` 即可停止。QTimer 还有一个方便的构造方式，可以直接在启动时指定间隔：

```cpp
QTimer *timer = new QTimer(this);
timer->start(1000);  // 等价于 setInterval(1000) + start()
```

### 3.2 单次定时器

有些时候你只需要「延迟执行一次」，而不是周期性重复。QTimer 提供了 `setSingleShot(true)` 来实现这个功能：

```cpp
QTimer *timer = new QTimer(this);
timer->setSingleShot(true);  // 只触发一次
timer->setInterval(3000);    // 3 秒后触发

connect(timer, &QTimer::timeout, []() {
    qDebug() << "三秒后只执行这一次";
});

timer->start();
```

设置单次模式后，定时器只会在第一次触发后自动停止。这对于「延迟执行」的场景特别有用，比如延迟关闭提示框、延迟加载资源等。Qt 还提供了一个更简洁的静态方法 `singleShot()`，非常适合一次性任务：

```cpp
// 静态方法：3 秒后执行 Lambda
QTimer::singleShot(3000, []() {
    qDebug() << "三秒后执行";
});

// 也可以指定接收对象
QTimer::singleShot(3000, this, []() {
    qDebug() << "在 this 对象的上下文中执行";
});
```

`singleShot()` 的优势是你不需要自己管理定时器对象的生命周期，Qt 会自动处理。这对于简单的延迟任务非常方便。

### 3.3 重复定时器

默认情况下，QTimer 是重复触发的。这就是最常见的「周期性任务」场景：

```cpp
QTimer *timer = new QTimer(this);
timer->setInterval(100);  // 100 毫秒 = 0.1 秒

int counter = 0;
connect(timer, &QTimer::timeout, [&]() {
    counter++;
    qDebug() << "Tick" << counter;

    if (counter >= 10) {
        timer->stop();  // 执行 10 次后停止
        qDebug() << "定时器停止";
    }
});

timer->start();
```

重复定时器会一直运行，直到你调用 `stop()` 或者定时器对象被销毁。需要注意的是，定时器的间隔不是绝对精确的。如果事件循环很忙，或者你的槽函数执行时间过长，下一次触发可能会延迟。你可以通过 `isActive()` 检查定时器是否正在运行，通过 `remainingTime()` 获取距离下一次触发的剩余时间：

```cpp
if (timer->isActive()) {
    qDebug() << "还有" << timer->remainingTime() << "毫秒触发";
}
```

### 3.4 高精度定时需求

QTimer 的精度取决于操作系统和硬件。在大多数现代系统上，精度通常在 10-20 毫秒左右。这对于一般的应用足够了，但如果你需要更高精度的定时（比如游戏循环、音频处理），可能需要考虑其他方案。

如果你需要测量时间间隔（而不是定时触发），应该使用 `QElapsedTimer` 而不是 QTimer：

```cpp
QElapsedTimer elapsedTimer;
elapsedTimer.start();

// 执行一些操作
QThread::msleep(500);

qint64 elapsed = elapsedTimer.elapsed();  // 经过的毫秒数
qDebug() << "经过了" << elapsed << "毫秒";

// 也可以检查是否超时
if (elapsedTimer.hasExpired(1000)) {
    qDebug() << "已经超过 1 秒了";
}
```

`QElapsedTimer` 是一个高精度的计时器，用于测量时间间隔，而不是定时触发事件。它通常用于性能测量、超时检测等场景。很多朋友会在这里搞混，其实它们的区别很清楚：QTimer 是「定时触发事件」，会在指定时间后发射信号，适合周期性任务；QElapsedTimer 是「测量时间间隔」，像一个秒表，适合计算两个时刻之间的时间差。如果你要做一个「倒计时 10 秒」的功能，应该用 QTimer，因为它可以每秒触发一次更新显示，而 QElapsedTimer 可以用来测量实际经过了多少时间，作为辅助验证。

如果你真的需要高精度的周期性定时（比如游戏引擎的 60 FPS），可以考虑结合 `QTimer` 和 `QElapsedTimer`：

```cpp
class HighPrecisionTimer : public QObject {
    Q_OBJECT
public:
    HighPrecisionTimer(QObject *parent = nullptr) : QObject(parent) {
        m_timer.setInterval(16);  // 约 60 FPS
        connect(&m_timer, &QTimer::timeout, this, &HighPrecisionTimer::onTick);
    }

    void start() {
        m_elapsedTimer.start();
        m_timer.start();
    }

private slots:
    void onTick() {
        qint64 elapsed = m_elapsedTimer.restart();
        // elapsed 是上一帧到现在的实际时间
        // 可以用来做插值、补偿等
        updateFrame(elapsed);
    }

private:
    QTimer m_timer;
    QElapsedTimer m_elapsedTimer;

    void updateFrame(qint64 deltaMs);
};
```

## 4. 踩坑预防清单

定时器用起来很简单，但有些坑真的会让你血压拉满。这里集中列几个最常见的。

第一个坑是定时器对象被提前销毁。如果你在函数里 `new QTimer` 但没设置 parent，函数结束后 timer 就成了野指针，不但内存泄漏，定时器还可能被意外销毁导致程序崩溃。正确的做法永远是 `new QTimer(this)`，让它随 parent 一起销毁，或者手动管理生命周期。

第二个坑是在错误的线程使用定时器。定时器属于哪个线程，就在哪个线程触发。如果你在主线程创建了定时器，却想在工作线程里用它，那它根本不会触发，或者触发后槽函数在错误的线程执行。解决办法是用 `moveToThread()` 把定时器移到目标线程，或者直接在目标线程中创建。

第三个坑是槽函数执行时间过长。如果你在 `timeout` 的槽函数里搞了一个耗时操作（比如 `QThread::sleep(2)`），整个事件循环都会被阻塞，界面卡死不说，其他定时器也会变得不准。正确的做法是槽函数里只做标记，耗时操作丢给工作线程去处理。

第四个坑比较低级但真的很常见——忘记调用 `start()`。你配置了间隔、连接了信号槽，但就是忘了启动，然后困惑为什么连接了信号槽却什么都没发生。记住，配置完间隔后必须调用 `start()`。

接下来做一个代码填空练习，补全以下代码，实现一个每 500 毫秒打印一次计数、打印 5 次后停止的定时器：

```cpp
QTimer *timer = new QTimer(______);  // 设置 parent
timer->______(500);  // 设置间隔

int count = 0;
connect(timer, &QTimer::timeout, [&]() {
    count++;
    qDebug() << "Count:" << count;

    if (count >= 5) {
        timer->______();  // 停止定时器
        qDebug() << "Done!";
    }
});

timer->______();  // 启动定时器
```

提示：需要填入的分别是 `this`、`setInterval`、`stop`、`start`。参考答案如下：

```cpp
QTimer *timer = new QTimer(this);  // 设置 parent
timer->setInterval(500);  // 设置间隔

int count = 0;
connect(timer, &QTimer::timeout, [&]() {
    count++;
    qDebug() << "Count:" << count;

    if (count >= 5) {
        timer->stop();  // 停止定时器
        qDebug() << "Done!";
    }
});

timer->start();  // 启动定时器
```

再看一个调试挑战：下面这段代码有什么问题？为什么定时器可能不会按预期工作？

```cpp
class MyClass : public QObject {
    Q_OBJECT
public:
    MyClass() {
        QTimer *timer = new QTimer;
        timer->setInterval(1000);
        connect(timer, &QTimer::timeout, this, &MyClass::onTimeout);
        timer->start();
    }

private slots:
    void onTimeout() {
        qDebug() << "Timeout!";
    }
};
```

问题出在定时器没有设置 parent，会导致内存泄漏。更严重的是，如果 MyClass 被销毁，定时器仍然存在，但 `this` 已经无效，访问时会直接崩溃。正确写法是 `new QTimer(this)`，这样 MyClass 销毁时定时器也会一起被清理。

## 5. 练习项目

我们要做一个小型秒表程序，功能不多但正好练手。

创建一个命令行或简单 GUI 程序，实现启动、暂停、重置秒表的功能，每秒更新显示的当前时间（格式：MM:SS），并且可以设置倒计时模式，倒计时结束后发出提示。

你的程序应该能正确响应启动、暂停、重置操作，每秒准时更新显示，倒计时到达零时能触发提示信号。代码结构清晰，定时器管理合理，没有内存泄漏或崩溃风险。

提示几个方向：用一个 QTimer 作为计时核心，连接它的 `timeout` 信号到更新显示的槽；需要维护一个「当前秒数」的状态变量，暂停时停止计时器但不重置这个值；倒计时模式可以在更新显示的槽里检查是否到达零；QElapsedTimer 可以用来做高精度计时，减少误差累积。

## 6. 官方文档参考

- [Qt 文档 · QTimer 类](https://doc.qt.io/qt-6/qtimer.html) -- QTimer 的完整 API 参考，包含所有信号、槽和属性说明
- [Qt 文档 · QElapsedTimer 类](https://doc.qt.io/qt-6/qelapsedtimer.html) -- 高精度计时器类，用于测量时间间隔
- [Qt 文档 · QObject::startTimer](https://doc.qt.io/qt-6/qobject.html#startTimer) -- 低级定时器 API，直接使用定时器 ID

（注：以上链接已通过互联网检索验证，均可在 Qt 官方网站访问）

---

到这里就大功告成了。掌握了 QTimer，你就可以在 Qt 程序里自由地实现各种定时任务，这大大扩展了程序的交互能力和自动化程度。定时器是 Qt 开发中最常用的工具之一，多练习几次你就会发现它的强大之处。下一节我们会讲插件系统，看看如何让程序支持动态扩展。
