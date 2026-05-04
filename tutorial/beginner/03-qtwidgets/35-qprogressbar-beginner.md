# 现代Qt开发教程（新手篇）3.35——QProgressBar：进度条

## 1. 前言 / 进度条比你想象的要复杂

如果你问我 Qt 里哪个控件看起来最简单但实际用起来最容易踩坑，QProgressBar 绝对排前三。大部分人第一次用进度条的套路都是一样的——建一个 QProgressBar，设 setRange(0, 100)，然后在某个循环里不停地 setValue(i)，最后发现进度条要么根本不动、要么卡在 0% 不走、要么整个窗口直接冻结。原因很简单：Qt 的事件循环被你的耗时操作阻塞了，QProgressBar 的重绘请求根本没机会被处理。这时候你才会意识到"在后台线程里更新进度条"不是一个可选项，而是一个必选项。

今天我们要把 QProgressBar 从头到尾讲透四个维度：setRange + setValue 的基本进度更新机制和常见错误用法，setRange(0, 0) 实现无限进度的滚动动画效果，setFormat 自定义进度条上显示的文字格式，以及最关键的——如何在工作线程中通过跨线程信号槽安全地更新进度条。最后一个话题涉及到 QThread 和信号槽的线程安全性，是本篇的重头戏。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QProgressBar 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。跨线程部分用到了 QThread（QtCore 模块），不需要额外链接——QtCore 已经被 QtWidgets 自动引入。示例代码中还用到了 QLabel、QPushButton、QVBoxLayout、QHBoxLayout、QGridLayout、QGroupBox 和 QProgressBar 来搭建界面。

## 3. 核心概念讲解

### 3.1 setRange + setValue 更新进度

QProgressBar 的核心接口非常简洁。setRange(int minimum, int maximum) 设定进度条的最小值和最大值，setValue(int value) 设定当前进度。进度条的百分比显示由 (value - minimum) / (maximum - minimum) 计算得出——也就是说 setRange(0, 100) 时 setValue(50) 显示 50%，setRange(0, 200) 时 setValue(50) 显示 25%。reset() 方法把值重置为 minimum，视觉上进度条回到起点。

```cpp
auto *progress = new QProgressBar;
progress->setRange(0, 100);
progress->setValue(0);    // 0%
progress->setValue(50);   // 50%
progress->setValue(100);  // 100%
progress->reset();        // 回到 0%
```

看起来很简单对吧？但问题在于 setValue 本身只改变了 QProgressBar 的内部状态并触发了一次重绘请求（repaint），而重绘请求只有在事件循环处理事件时才会真正执行。如果你的代码在主线程里跑一个密集的 for 循环，不停地调用 setValue(i)，整个事件循环就被这个循环霸占了——没有机会处理重绘请求，进度条就不会动。这是新手用 QProgressBar 最常遇到的"进度条不动"的问题。

一种不推荐的临时解决方案是在 setValue 之后调用 QApplication::processEvents()，强制处理一下事件队列里的重绘请求。这种方式确实能让进度条动起来，但它有很多问题：每次调用 processEvents 都可能触发其他事件处理（比如用户点了一个"取消"按钮），导致不可预期的递归调用和状态混乱；而且 processEvents 本身也有性能开销，频繁调用会拖慢整个循环。

```cpp
// 不推荐的方式：processEvents
for (int i = 0; i <= 100; ++i) {
    doExpensiveWork();
    progress->setValue(i);
    QApplication::processEvents();  // 强制刷新，但会带来副作用
}
```

正确的做法是把耗时操作放到工作线程里，通过信号槽把进度更新通知到主线程。这个话题我们在 3.4 节详细展开。

setRange 还有几个细节值得注意。如果 minimum 等于 maximum，进度条会进入"忙等待"模式——这个在下一节讲。如果 minimum 大于 maximum，行为是未定义的，不要这么干。默认的 range 是 (0, 100)，所以如果你直接 setValue(50) 不改 range，进度条会停在 50%。

setValue 在值没有变化的情况下不会触发重绘，这是一个小的性能优化。如果你连续两次调用 setValue(50)，第二次不会做任何事情。另外 setValue 会触发 valueChanged(int) 信号——这个信号携带的是新的值，你可以用它来同步更新一个 QLabel 显示百分比。

### 3.2 无限进度 setRange(0, 0) 滚动动画

不是所有的耗时操作都能提前知道"总共有多少工作量"。比如你调用一个网络请求去下载文件，服务器没有返回 Content-Length 头，你根本不知道文件有多大，也就没法算出百分比。这种场景下你需要的是一个"正在干活，请等待"的视觉提示，而不是一个精确的百分比数字。

QProgressBar 为这种场景提供了"无限进度"模式：当 setRange(0, 0) 时（minimum 等于 maximum 都等于 0），进度条不会显示百分比，而是在槽内显示一个来回滚动的动画块——类似于很多应用启动时那个左右来回移动的进度条。

```cpp
auto *busyProgress = new QProgressBar;
busyProgress->setRange(0, 0);  // 无限进度模式
busyProgress->setValue(0);     // 启动动画

// 当操作完成后恢复正常模式
busyProgress->setRange(0, 100);
busyProgress->setValue(100);
```

无限进度模式的动画是由 QProgressBar 的内部定时器驱动的，不需要你手动更新值。你只需要设 setRange(0, 0)，进度条就会自动开始滚动动画。当操作完成后，把 range 恢复为正常值，动画就停止了。

这个模式非常适合以下场景：应用启动时的加载画面、网络请求等待中、数据库查询执行中、文件解析中（不知道文件有多大）。它告诉用户"系统在忙着，没有卡死"，而不需要知道精确的进度百分比。

有一个小的视觉差异需要注意：无限进度模式在不同平台上的外观不一样。Windows 上是一个蓝色的小方块在槽内来回移动，macOS 上是一个带渐变的条状动画，Linux 上取决于你用的桌面主题。如果你用 QSS 自定义了进度条外观，无限模式的动画可能会受到影响——有些 QSS 样式会让无限模式的动画块变得不可见或者变形。如果你需要高度自定义的无限进度动画，可以考虑用 QPropertyAnimation 配合一个自定义控件来实现。

还有一个实用的技巧：你可以通过 setMinimum(0) 和 setMaximum(0) 来单独设置，效果和 setRange(0, 0) 完全一样。有些代码风格偏好分开调用，有些偏好合并调用，没有功能差异。

### 3.3 setFormat 自定义显示文字

QProgressBar 默认在进度条上方或者内部显示一个百分比字符串，比如 "50%"。这个格式可以通过 setFormat(const QString &) 来自定义。格式字符串中可以使用三个占位符：`%p%` 替换为百分比（比如 50），`%v` 替换为当前值（比如 50），`%m` 替换为总值（maximum - minimum，比如 100）。

```cpp
auto *progress = new QProgressBar;
progress->setRange(0, 1000);

// 默认格式：显示百分比
progress->setFormat("%p%");           // "50%"

// 显示当前值 / 总值
progress->setFormat("%v / %m");       // "500 / 1000"

// 自定义文字
progress->setFormat("已下载 %v KB，共 %m KB");  // "已下载 500 KB，共 1000 KB"

// 不显示文字
progress->setFormat("");
```

setFormat 对无限进度模式（setRange(0, 0)）也有效。在无限进度模式下，`%p%` 会替换为空字符串（因为没法算百分比），`%v` 替换为当前值（通常是你最后一次 setValue 的值），`%m` 替换为 0。所以无限进度模式下通常不需要特别设置 format，默认的空百分比显示就够了。

setAlignment(Qt::Alignment) 可以控制文字在进度条内的对齐方式——Qt::AlignCenter 是默认值，文字居中显示在进度条上方。如果你希望文字显示在进度条内部（嵌入在颜色条中），需要配合 QSS 来实现。

```cpp
progress->setAlignment(Qt::AlignCenter);  // 默认值，文字居中
progress->setAlignment(Qt::AlignLeft);    // 文字靠左
progress->setTextVisible(true);           // 显示文字（默认 true）
progress->setTextVisible(false);          // 隐藏文字，只显示进度条
```

setTextVisible(bool) 控制是否显示文字——有些设计风格偏好纯粹的进度条不带文字，设为 false 即可。文字的字体和颜色可以通过 QSS 的 color 和 font 属性来控制。

QSS 自定义进度条外观也是常见需求。QProgressBar 有两个 QSS 子控件：`chunk`（已填充部分）和整个控件本身（未填充部分）。常见的样式写法如下：

```css
QProgressBar {
    border: 1px solid #E0E0E0;
    border-radius: 4px;
    background-color: #F5F5F5;
    text-align: center;
    color: #333;
    height: 24px;
}

QProgressBar::chunk {
    background-color: #1976D2;
    border-radius: 3px;
}
```

text-align 控制 QProgressBar 上方文字的对齐方式（注意它和 setAlignment 是两个不同的东西——text-align 在 QSS 中控制文字位置，setAlignment 在 C++ 中控制文字在进度条矩形内的对齐）。chunk 是已填充的那段颜色条，background-color 控制填充色。如果你想让进度条有渐变效果，可以用 QLinearGradient：

```css
QProgressBar::chunk {
    background: qlineargradient(
        x1: 0, y1: 0, x2: 1, y2: 0,
        stop: 0 #1976D2, stop: 1 #42A5F5
    );
    border-radius: 3px;
}
```

### 3.4 跨线程安全更新进度条

这是本篇最重要的一节。如果你理解了前面讲的"为什么不能在主线程的循环里直接 setValue"，那么接下来的问题就是：怎么把耗时操作放到后台线程，同时又能安全地更新主线程上的进度条。

Qt 的答案是"跨线程信号槽"。Qt 的信号槽机制在跨线程时有两种连接方式：Qt::DirectConnection（直接调用，在发送信号的线程里执行槽函数）和 Qt::QueuedConnection（队列调用，把槽函数调用打包成事件投递到接收者所在的线程的事件循环中）。当信号发送者和槽函数接收者不在同一个线程时，默认使用 Qt::QueuedConnection——这意味着槽函数会在接收者所在的线程里执行，而不是发送者所在的线程。

这个机制天然地保证了线程安全：工作线程发送一个携带进度值的信号，Qt 内部把这个信号打包成一个事件投递到主线程的事件队列里，主线程的事件循环在空闲时取出这个事件并执行槽函数——槽函数里调用 setValue 更新 QProgressBar。整个过程工作线程不会直接触碰 GUI 控件，所有 GUI 操作都在主线程里完成。

下面是一个标准的跨线程进度更新模式的代码骨架：

```cpp
/// @brief 模拟耗时操作的工作类
class Worker : public QObject
{
    Q_OBJECT

public slots:
    void doWork()
    {
        for (int i = 0; i <= 100; ++i) {
            // 模拟耗时操作
            QThread::msleep(50);

            // 发送进度信号（在工作线程中发送）
            emit progressChanged(i);
        }
        emit finished();
    }

signals:
    void progressChanged(int percent);
    void finished();
};
```

```cpp
// 在主线程中创建 Worker 和 QThread
auto *worker = new Worker;
auto *thread = new QThread;
worker->moveToThread(thread);

// 连接信号槽
connect(thread, &QThread::started,
        worker, &Worker::doWork);
connect(worker, &Worker::progressChanged,
        progressBar, &QProgressBar::setValue);
connect(worker, &Worker::finished,
        thread, &QThread::quit);
connect(worker, &Worker::finished,
        worker, &Worker::deleteLater);
connect(thread, &QThread::finished,
        thread, &QThread::deleteLater);

// 启动工作线程
thread->start();
```

这段代码的执行流程是这样的：主线程创建 Worker 对象，通过 moveToThread 把它移到新的 QThread 中。调用 thread->start() 后，QThread 开始运行自己的事件循环。thread 的 started 信号触发 Worker::doWork 槽函数，doWork 在工作线程中执行耗时操作。每当 doWork 中 emit progressChanged(i) 时，Qt 把这个信号排队投递到主线程，主线程的事件循环取出事件后执行 QProgressBar::setValue(i)。doWork 结束后 emit finished()，finished 触发 thread->quit() 停止线程的事件循环，同时触发 worker->deleteLater() 和 thread->deleteLater() 清理内存。

这个模式有几个需要注意的地方。第一，Worker 对象必须通过 moveToThread 移到新线程之后再启动，不能在主线程中直接调用 Worker 的槽函数——否则槽函数还是在主线程里执行，耗时操作依然阻塞主线程。第二，Worker 中绝对不能直接调用任何 GUI 控件的方法——所有 GUI 操作必须通过信号槽投递到主线程。第三，线程清理要用 deleteLater 而不是直接 delete——因为 Worker 对象已经属于另一个线程，从主线程直接 delete 会导致竞态条件。deleteLater 会在对象所属线程的事件循环中安全地删除对象。

还有一个常见的变体是使用 QThread::create（Qt 5.10+ 引入的便捷方法），配合 lambda 来避免显式定义 Worker 类：

```cpp
auto *thread = QThread::create([progressBar]() {
    for (int i = 0; i <= 100; ++i) {
        QThread::msleep(50);
        // 不能直接调用 progressBar->setValue(i)
        // 需要通过信号槽
    }
});
```

但 QThread::create 的 lambda 里不能直接 emit 信号（lambda 不是 QObject），所以如果你需要通过信号更新进度条，还是要用传统的 Worker 类模式，或者用 QMetaObject::invokeMethod 来间接调用。对于初学者来说，掌握 Worker + moveToThread 的标准模式就足够了。

最后提一个关于线程数量的实践建议：如果你有多个独立的耗时任务需要并行执行，不要为每个任务创建一个新线程。使用 QThreadPool + QRunnable 或者 QtConcurrent 来管理线程池，让 Qt 帮你调度。每个任务通过 QRunnable::progressSignal 之类的自定义信号汇报进度。但这个话题超出了本篇的范围，后面讲到 QThread 和 QtConcurrent 的时候再详细展开。

## 4. 踩坑预防

第一个坑是主线程循环里直接 setValue 导致进度条不动。前面已经反复强调了——setValue 只发送重绘请求，事件循环被阻塞时重绘不会执行。解决方案是跨线程信号槽。

第二个坑是 processEvents 的滥用。虽然它能让进度条动起来，但会带来事件递归、状态混乱等副作用。只在确实无法避免主线程阻塞的场景下谨慎使用，正式代码不要依赖它。

第三个坑是 setRange(0, 0) 后忘记恢复。无限进度模式应该在操作开始时设置，操作结束后要恢复为正常的 range 并设最终值。如果一直停留在 setRange(0, 0)，用户永远看不到"完成"状态。

第四个坑是 Worker 对象中直接操作 GUI 控件。moveToThread 之后 Worker 的所有方法都在工作线程中执行，直接调用 QWidget 子类的方法是未定义行为——可能崩溃、可能什么都不做、可能看似正常但随时可能崩溃。所有 GUI 操作必须通过信号槽回到主线程。

第五个坑是线程清理不完整。创建的 QThread 和 Worker 对象必须在操作结束后正确清理。推荐的连接模式是 Worker::finished -> thread->quit + worker->deleteLater，thread::finished -> thread->deleteLater。忘记 deleteLater 会导致内存泄漏。

第六个坑是 setFormat 的占位符写法。`%p%` 中的两个 `%` 不是一个占位符 + 一个百分号——它是一个完整的占位符，Qt 在内部会替换为 "50" + "%"。如果你写成 "%p %%"，显示的会是 "50 %" 而不是 "50%"。

## 5. 练习项目

我们来做一个综合练习：创建一个"批量文件处理模拟器"窗口，覆盖 QProgressBar 的核心用法。窗口顶部有两个按钮："开始处理"和"取消"。中间是一个标准的 QProgressBar（setRange(0, 100)），用 setFormat 显示 "已处理 %v 个文件，共 %m 个"。进度条下方是一个 QLabel 显示当前状态文字（"空闲" / "处理中..." / "已完成" / "已取消"）。点击"开始处理"后，启动一个 Worker 线程模拟耗时操作（每次循环 sleep 100ms），通过 progressChanged 信号更新进度条。点击"取消"后，通过一个 bool 标志通知 Worker 停止。窗口底部还有一个无限进度条（setRange(0, 0)），在 Worker 运行时显示滚动动画，Worker 结束后隐藏。

提示：Worker 需要一个 std::atomic<bool> 或者通过信号槽传递的标志来支持取消操作；无限进度条通过 setVisible 控制显隐；所有 GUI 更新必须在主线程中完成。

## 6. 官方文档参考链接

[Qt 文档 -- QProgressBar](https://doc.qt.io/qt-6/qprogressbar.html) -- 进度条控件

[Qt 文档 -- QThread](https://doc.qt.io/qt-6/qthread.html) -- 线程类（跨线程信号槽的基础）

[Qt 文档 -- QObject::moveToThread](https://doc.qt.io/qt-6/qobject.html#moveToThread) -- 将对象移到指定线程

---

到这里，QProgressBar 的四个核心维度就全部讲完了。setRange + setValue 是进度条最基本的 API，但直接在主线程的循环里调用 setValue 是新手最常犯的错误——事件循环被阻塞导致进度条不动，正确的做法是跨线程信号槽。setRange(0, 0) 提供了一种不需要知道总量的"无限进度"视觉反馈，适合无法预估耗时的操作。setFormat 让你可以自定义进度条上显示的文字，`%p%` 是百分比，`%v` 是当前值，`%m` 是总值。跨线程安全更新进度条是本篇的重头戏——Worker + moveToThread + 信号槽的标准模式，保证耗时操作在工作线程执行、GUI 更新在主线程执行、线程间通信安全有序。把这些搞清楚后，无论是文件下载进度、数据处理进度还是批量操作进度，你都能写出流畅不卡顿的进度条了。
