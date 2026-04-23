# 现代Qt开发教程（新手篇）1.9——多线程基础

## 1. 前言：多线程的挑战

老实说，多线程是我学 Qt 时踩坑最多的地方。一开始觉得很简单嘛——开个线程跑后台任务，完了把结果发回来显示。结果呢？界面卡死、数据竞争、莫名其妙的崩溃，各种经典的并发问题我都遇了个遍。

后来我才发现，Qt 的多线程机制虽然设计得很优雅，但前提是你得理解它的工作方式。特别是 GUI 编程这个场景，有一条铁律你必须记住：GUI 操作必须在主线程进行。这条规则违反一次，程序就可能给你来个随机崩溃，而且还是那种间歇性的，最难调试。

这一篇我们会从最基础的 QThread 用法讲起，然后介绍更高级的 QThreadPool 和 QtConcurrent。你会发现，Qt 其实把很多复杂的事情都替你做好了——只要你用对姿势。

## 2. 环境说明

本教程基于 Qt 6.9.1，适用于 Windows 10/11（MSVC 2019+ 或 MinGW 11.2+）、Linux（GCC 11+）、WSL2 + WSLg（GUI 支持）。所有示例代码均使用 C++17 标准和 CMake 3.26+ 构建系统。

## 3. 核心概念讲解

### 3.1 为什么需要多线程

想象一下，你要写一个图片处理软件。用户选了一张 4K 图片，点击"应用滤镜"按钮，然后你就开始在主线程里搞事情——读图片、遍历像素、应用滤镜算法。如果这个操作需要 3 秒，那这 3 秒内整个界面就是死的，按钮点不动，窗口拖不动，用户体验极其糟糕。

这时候多线程就派上用场了。你可以把耗时的图片处理扔到后台线程，主线程继续响应用户操作。处理完了，后台线程发个信号告诉主线程："结果好了，来拿吧"。这样界面始终流畅，用户也开心。

但这里有个前提：任何 GUI 操作（更新控件、重绘界面）都必须在主线程进行。这是 Qt GUI 编程的铁律，违反必究。

### 3.2 QThread 的正确用法

QThread 是 Qt 中最基本的线程类，但说实话，它有两个用法，而且其中一个特别容易误导人。

先看错误但很常见的用法——继承 QThread：

```cpp
class WorkerThread : public QThread
{
    Q_OBJECT
protected:
    void run() override {
        // 这里写后台任务的代码
        for (int i = 0; i < 100; ++i) {
            // 做一些耗时操作...
        }
    }
};

// 使用方式
WorkerThread *thread = new WorkerThread;
thread->start();  // 启动线程，run() 会在新线程中执行
```

这个用法的问题在于，你把整个类都和 QThread 绑死了，而且很容易搞混哪些代码在哪个线程运行。

推荐的做法是使用 moveToThread：

```cpp
class Worker : public QObject
{
    Q_OBJECT
public slots:
    void doWork() {
        // 这里写后台任务的代码
        for (int i = 0; i < 100; ++i) {
            // 做一些耗时操作...
        }

        // 完成后发送信号
        emit workFinished(result);
    }

signals:
    void workFinished(const Result &result);
};

// 使用方式
QThread *thread = new QThread;
Worker *worker = new Worker;
worker->moveToThread(thread);  // 把 worker 移到新线程

// 连接信号槽
connect(thread, &QThread::started, worker, &Worker::doWork);
connect(worker, &Worker::workFinished, this, [this](const Result &r) {
    // 这里在主线程，可以安全更新 GUI
    updateUI(r);
});
connect(worker, &Worker::workFinished, thread, &QThread::quit);
connect(thread, &QThread::finished, thread, &QThread::deleteLater);

thread->start();  // 启动线程
```

第二种方式的优势很明显：Worker 是个独立的类，职责清晰；通过信号槽机制通信，线程安全；线程生命周期管理也更清晰。

你可能会问：为什么 `moveToThread` 之后，worker 的槽函数会在新线程执行？原因在于 `moveToThread` 改变了对象的线程依附性（thread affinity），之后发给该对象的信号（如果使用 `Qt::AutoConnection` 或 `Qt::QueuedConnection`）会以队列方式投递，在对象所依附的线程中执行槽函数。这就是为什么信号槽能安全地实现跨线程通信——Qt 在底层帮你做了线程切换。

### 3.3 QThreadPool 线程池

每次要跑后台任务都创建新线程其实挺浪费的。线程创建有开销，而且线程太多也会让系统调度压力变大。QThreadPool 就是为此设计的——它维护了一组可重用的线程，任务来了就分配一个空闲线程去执行。

```cpp
class Task : public QRunnable
{
public:
    Task(const QString &data) : m_data(data) {}

    void run() override {
        // 这里写任务代码
        process(m_data);

        // QRunnable 执行完会自动删除（如果 setAutoDelete(true)）
    }

private:
    QString m_data;
};

// 使用方式
QThreadPool::globalInstance()->start(new Task("some data"));
```

QThreadPool 的优点在于线程复用、减少创建开销，可以限制最大线程数避免资源耗尽，API 也简单，适合短暂的、独立的任务。但要注意，QRunnable 不是 QObject，不能发送信号。如果你需要结果通知，还是得用 QObject + moveToThread 的方式。

### 3.4 QtConcurrent 便捷 API

QtConcurrent 是更高级的封装，可以用一行代码启动并发任务。它内部使用 QThreadPool，但提供了更简洁的接口。

```cpp
// 在后台线程运行函数
QFuture<int> future = QtConcurrent::run([]() {
    // 耗时计算
    int result = heavyCalculation();
    return result;
});

// 处理结果（可选）
QFutureWatcher<int> *watcher = new QFutureWatcher<int>;
connect(watcher, &QFutureWatcher<int>::finished, [watcher]() {
    int result = watcher->result();
    // 更新 GUI
    updateUI(result);
    watcher->deleteLater();
});
watcher->setFuture(future);
```

QtConcurrent 还提供了一些常用算法的并发版本：`QtConcurrent::mapped()` 对容器中每个元素应用函数，`QtConcurrent::filtered()` 过滤容器元素，`QtConcurrent::reduce()` 做归约操作。

```cpp
QList<int> numbers = {1, 2, 3, 4, 5};

// 并发计算每个数的平方
QFuture<int> squared = QtConcurrent::mapped(numbers, [](int x) {
    return x * x;
});

// 并发过滤大于2的数
QFuture<int> filtered = QtConcurrent::filtered(numbers, [](int x) {
    return x > 2;
});
```

### 3.5 QFuture 异步结果

QFuture 代表一个异步操作的结果。你可以用它来检查操作是否完成、等待结果、或者取消操作。

```cpp
QFuture<int> future = QtConcurrent::run(heavyCalculation);

// 检查是否完成
if (future.isFinished()) {
    int result = future.result();
}

// 阻塞等待结果（小心别在主线程用）
int result = future.result();

// 取消操作
future.cancel();

// 暂停/恢复（需要底层支持）
future.pause();
future.resume();
```

配合 QFutureWatcher，你可以用信号槽方式监听异步操作：

```cpp
QFutureWatcher<int> *watcher = new QFutureWatcher<int>(this);

connect(watcher, &QFutureWatcher<int>::finished, [watcher]() {
    qDebug() << "Result:" << watcher->result();
});

connect(watcher, &QFutureWatcher<int>::progressValueChanged,
        [](int value) {
    qDebug() << "Progress:" << value;
});

watcher->setFuture(future);
```

这里有个坑真的坑了我半天：在主线程调用 `future.result()` 或 `future.waitForFinished()` 会阻塞主线程，界面直接卡死，和没用多线程一样。正确做法是使用 QFutureWatcher 监听完成信号，在槽函数里获取结果。永远不要在主线程阻塞等待异步结果，用信号槽通知——这条规则和"GUI 操作必须在主线程"一样重要。

### 3.6 跨线程信号槽的线程安全

Qt 的信号槽机制在跨线程调用时会自动使用队列连接（Qt::QueuedConnection），这保证了线程安全。但前提是你得正确使用。

```cpp
// 后台线程发送信号
class Worker : public QObject
{
    Q_OBJECT
public:
    void doWork() {
        // ... 耗时操作 ...
        emit progressChanged(50);     // 跨线程发送信号
        emit workFinished(result);    // 安全！
    }

signals:
    void progressChanged(int percent);
    void workFinished(const Result &result);
};

// 主线程接收信号
connect(worker, &Worker::progressChanged,
        this, &MyClass::updateProgress);
connect(worker, &Worker::workFinished,
        this, &MyClass::handleResult);
```

这里的关键是：当信号发送者和接收者在不同线程时，Qt 会自动把信号参数拷贝到事件队列，然后在接收者所在线程执行槽函数。你不需要手动加锁。

但要注意，如果你通过引用传递参数（比如 `const Result&`），Qt 会强制拷贝。如果想避免拷贝，确保类型是可共享的，或者显式注册为元类型。

另一个必须强调的坑：绝对不能在后台线程直接操作 GUI 对象。比如你在后台线程的 `doWork()` 里写了 `label->setText("Processing...")`，这就是危险操作——程序可能崩溃、界面闪烁、出现数据竞争和各种奇怪的行为。GUI 操作永远在主线程，跨线程通信用信号槽，没有任何例外。

### 3.7 QMutex 基础保护

虽然信号槽能解决大部分问题，但有时候你还是需要在多线程间共享数据。这时候就需要互斥锁了。

```cpp
class SharedData : public QObject
{
    Q_OBJECT
public:
    void addData(int value) {
        QMutexLocker locker(&m_mutex);  // RAII 风格，作用域结束自动解锁
        m_data.append(value);
    }

    QList<int> getData() const {
        QMutexLocker locker(&m_mutex);
        return m_data;  // 返回副本
    }

private:
    mutable QMutex m_mutex;
    QList<int> m_data;
};
```

QMutexLocker 是 RAII 风格的锁管理器，构造时加锁，析构时解锁，即使发生异常也能正确释放锁。比你手动 lock/unlock 安全得多。

说到手动 lock/unlock，这真的是一个经典死锁来源——你手动调了 `m_mutex.lock()`，然后在某个 `if` 分支里直接 `return` 了，忘了 `unlock()`，结果锁就永远卡在那了。优先用 QMutexLocker，让编译器帮你管理锁的生命周期，`return` 也好、异常也好，析构函数都会帮你解锁。

你可以试着想一下：下面这个安全的计数器类，两个空应该填什么？

```cpp
class SafeCounter {
public:
    void increment() {
        QMutexLocker ______(&m_mutex);
        ++m_count;
    }

    int value() const {
        QMutexLocker ______(&m_mutex);
        return m_count;
    }

private:
    mutable QMutex m_mutex;
    int m_count = 0;
};
```

两个空都填 `locker` 就对了——`QMutexLocker locker(&m_mutex);` 是标准写法，作用域结束时自动解锁。

再看一个更有意思的调试场景。下面这段代码有什么问题？

```cpp
class MyWidget : public QWidget {
    Q_OBJECT
public:
    MyWidget() {
        QPushButton *btn = new QPushButton("Process", this);
        connect(btn, &QPushButton::clicked, this, &MyWidget::onProcess);
    }

    void onProcess() {
        QThread *thread = QThread::create([&]() {
            heavyOperation();
            label->setText("Done");  // label 是成员变量
        });
        thread->start();
    }

private:
    QLabel *label = new QLabel(this);
};
```

问题出在 lambda 里直接操作了 GUI 对象——`label->setText("Done")` 是在后台线程执行的，而 label 是一个 GUI 控件，必须在主线程操作。后果是程序可能崩溃或出现不可预测的行为。解决办法是通过信号槽通知主线程更新，或者用 `QMetaObject::invokeMethod` 把调用安全地投递到主线程。

## 4. 多线程方案选择指南

到这里你可能会有点晕——这么多方案，到底该用哪个？我们简单梳理一下：单个长期运行的后台任务推荐用 `QObject + moveToThread`，因为你能完整控制线程生命周期，信号槽通信也很方便；多个短期的独立任务推荐用 `QRunnable + QThreadPool`，线程复用效率高，API 也简单；并发处理容器数据用 `QtConcurrent::mapped/filtered`，高层 API 代码最简洁；简单异步函数调用用 `QtConcurrent::run`，一行搞定；需要取消或暂停的任务用 `QFuture + QFutureWatcher`，提供完整的异步控制；简单的数据共享用 `QMutex/QMutexLocker`，保证原子操作。

记住一个原则：从简单的开始。`QtConcurrent::run` 能解决的问题，就不要搞复杂的 `moveToThread`。

## 5. 练习项目

练习项目：多线程图片加载器。

我们要创建一个图片浏览程序，支持从本地加载大量图片。加载过程必须在后台线程进行，主线程显示加载进度，加载完成后更新缩略图。如果用户点击了"取消"按钮，能够中断加载过程。

完成标准是这样的：使用 QThreadPool 或 QtConcurrent 实现后台加载；主界面有进度条显示当前加载进度；缩略图加载完成后自动刷新显示；取消按钮能正确中断加载过程；程序运行流畅，界面不卡顿。

几个实现提示：可以用 `QtConcurrent::run` 配合 `QFutureWatcher` 实现可取消的任务；图片加载用 `QImage::load()`，然后用 `QPixmap::fromImage()` 转换（这个转换必须在主线程）；进度更新可以通过自定义信号槽实现；取消操作使用 `QFuture::cancel()`。

## 6. 官方文档参考

[Qt 6 Thread Support](https://doc.qt.io/qt-6/thread.html) -- Qt 多线程编程概述，必读基础

[QThreadPool Class](https://doc.qt.io/qt-6/qthreadpool.html) -- 线程池管理类文档

[Qt Concurrent Module](https://doc.qt.io/qt-6/qtconcurrent-index.html) -- QtConcurrent 高层 API 详解

[QFuture Class](https://doc.qt.io/qt-6/qfuture.html) -- 异步结果处理类文档

[QMutex Class](https://doc.qt.io/qt-6/qmutex.html) -- 互斥锁文档，包含 QMutexLocker 说明

*（链接已验证，2026-03-17 可访问）*

---

到这里就大功告成了！多线程是个大话题，入门篇我们先掌握这些核心概念和正确用法。实际工程中你还会遇到更多问题——比如线程池调优、死锁排查、性能分析等，那些我们留到进阶层再深入。

记住最关键的两条铁律：GUI 必须在主线程操作，跨线程通信用信号槽。遵守这两条，你的多线程 Qt 程序就已经成功了一半。

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
