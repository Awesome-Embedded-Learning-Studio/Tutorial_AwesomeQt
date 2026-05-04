# 现代Qt开发教程（新手篇）3.67——QProgressDialog：进度对话框

## 1. 前言 / 长任务不给反馈就是耍流氓

上一节我们聊了 QFileDialog，用户选完文件后下一步通常就是对文件做操作——读取内容、解析格式、写入磁盘、上传到服务器。这些操作如果耗时很短（几百毫秒以内），界面卡一下用户不会在意。但如果操作要跑几秒甚至几分钟——比如处理几百兆的文件、批量转换图片格式、从网络下载大型资源——界面完全冻住不动，用户会觉得应用崩了。进度对话框就是解决这个问题的标准方案：告诉用户"我还在干活、目前干了多少、大概还要多久"。

QProgressDialog 是 Qt 提供的进度对话框组件，它的形态很简单——一个带进度条的模态对话框，上面显示一行描述文字（当前在做什么）、一个进度条（完成了百分之多少）、一个取消按钮（让用户可以中途放弃）。就这么三个元素，但它解决的问题非常实际：给用户一个明确的反馈通道，避免用户在等待过程中产生焦虑或者误操作。

从 API 角度看，QProgressDialog 的核心操作只有几个：setLabelText 设置描述文字、setValue 设置当前进度、setRange 设置进度范围。这三个方法组合起来就能驱动进度条的更新。除此之外，canceled 信号用于响应取消操作，setAutoClose 和 setAutoReset 控制进度达到终点时的自动行为。当进度对话框和 QThread 配合使用时，就构成了一个完整的"后台任务 + 前端进度反馈"的方案。

今天我们从四个方面展开。先看 setLabelText / setValue / setRange 的基础配置，然后讨论 canceled 信号响应取消操作的方式，接着研究 setAutoClose / setAutoReset 的自动关闭行为，最后实现一个与 QThread 配合的长任务进度汇报方案。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QProgressDialog 在 QtWidgets 模块中，QThread 在 QtCore 模块中，链接 Qt6::Widgets 即可（Qt6::Core 是隐式依赖）。示例代码涉及 QProgressDialog、QThread、QApplication、QMainWindow、QPushButton、QVBoxLayout 和 QDebug。跨线程信号使用了新式 connect 语法，依赖自动队列连接机制。

## 3. 核心概念讲解

### 3.1 setLabelText / setValue / setRange：基础三件套

QProgressDialog 的使用模式可以用一句话概括：设定范围、循环更新值、到达终点自动关闭。我们先看一个最简单的例子——模拟一个有 100 个步骤的处理过程：

```cpp
QProgressDialog progress("正在处理...", "取消", 0, 100, this);
progress.setWindowModality(Qt::WindowModal);
progress.setWindowTitle("处理进度");
progress.setMinimumDuration(0);  // 立即显示

for (int i = 0; i <= 100; ++i) {
    progress.setValue(i);
    if (progress.wasCanceled()) {
        break;
    }
    // 模拟每步耗时
    QThread::msleep(50);
}

progress.setValue(100);
```

这段代码的执行流程是这样的。构造函数的参数依次是：标签文字（显示在进度条上方）、取消按钮文字、最小值、最大值、父窗口。setWindowModality(Qt::WindowModal) 设置为窗口级模态——对话框弹出后，用户不能操作主窗口，但可以操作其他应用的窗口。setMinimumDuration(0) 让对话框立即显示——默认情况下 QProgressDialog 有一个 4 秒的最短等待时间，只有在操作超过 4 秒时对话框才会出现。这个默认行为的设计初衷是避免短操作弹出闪烁的对话框，但在我们的演示中需要立即看到效果，所以设为 0。

在循环中，每次调用 setValue 更新进度值。QProgressDialog 内部会在 setValue 中处理事件（调用 QCoreApplication::processEvents），所以即使你在主线程的循环中频繁调用 setValue，界面也不会完全冻住——用户可以看到进度条在走，也可以点击取消按钮。这是 QProgressDialog 和 QProgressBar 的一个重要区别：QProgressBar 只是一个控件，它不会主动处理事件；QProgressDialog 是一个独立的对话框窗口，它在 setValue 中内建了事件处理。

setValue 每次被调用时都会检查当前值是否达到了最大值。如果达到了并且 setAutoClose 为 true（默认就是 true），对话框会自动关闭。所以上面的代码中最后的 setValue(100) 其实是多余的——循环到 i=100 时对话框就已经自动关闭了。但显式写出来可以让代码意图更清晰。

setLabelText 用于更新描述文字。在处理过程的不同阶段，你可以更新标签文本来告诉用户当前正在做什么：

```cpp
progress.setLabelText("正在读取文件...");
// ... 读取文件 ...
progress.setLabelText("正在解析数据...");
// ... 解析数据 ...
progress.setLabelText("正在写入结果...");
// ... 写入结果 ...
```

setRange 用于设置进度的范围。上面的例子用的是 0~100，对应百分比。但很多时候你的进度不是按百分比计算的——比如你要处理 500 个文件，那范围就是 0~500：

```cpp
progress.setRange(0, fileCount);
for (int i = 0; i < fileCount; ++i) {
    progress.setLabelText(
        QString("正在处理: %1/%2")
            .arg(i + 1).arg(fileCount));
    progress.setValue(i);
    // ... 处理第 i 个文件 ...
}
progress.setValue(fileCount);
```

有一种特殊情况需要处理：如果你在开始处理之前不知道总共有多少步骤（比如从网络流式读取数据，事先不知道文件大小），可以调用 setRange(0, 0)——这会让进度条进入"忙碌模式"（busy indicator），进度条会来回滚动，表示"正在处理但不知道进度"。等你拿到总量后再调用 setRange(0, total) 切换回正常模式。

### 3.2 canceled 信号：响应用户取消

当用户点击进度对话框的取消按钮时，QProgressDialog 会发射 canceled 信号，同时内部的 wasCanceled 标志变为 true。有两种方式响应取消操作。

第一种方式是在循环中主动检查 wasCanceled()：

```cpp
for (int i = 0; i <= 100; ++i) {
    progress.setValue(i);
    if (progress.wasCanceled()) {
        qDebug() << "用户取消了操作";
        // 执行清理工作...
        break;
    }
    QThread::msleep(50);
}
```

这种方式最简单直观，适合处理步骤清晰、可以安全中断的场景。wasCanceled() 的检查频率取决于你调用 setValue 的频率——如果每步都调用 setValue，那每步都会检查一次，响应速度足够快。

第二种方式是连接 canceled 信号到自定义槽函数：

```cpp
QProgressDialog progress("正在处理...", "取消",
                          0, 100, this);

connect(&progress, &QProgressDialog::canceled,
        this, [&]() {
    qDebug() << "用户点击了取消按钮";
    // 设置标志位让后台任务停止
    m_cancelRequested = true;
});

// 在后台任务中检查 m_cancelRequested
```

信号槽方式更适合与 QThread 配合的场景——后台线程在执行长任务，主线程显示进度对话框。用户点击取消时，通过信号槽（或者原子标志位）通知后台线程停止工作。我们稍后在 3.4 节会看到完整的实现。

这里有一个容易忽略的细节需要注意：QProgressDialog 默认有自动关闭和自动重置行为。当用户点击取消按钮时，setValue 会检测到 wasCanceled 为 true，然后自动关闭对话框——即使进度没有达到最大值。这意味着用户点击取消后对话框会立即消失，你的循环中的 break 语句会执行，但此时对话框已经关了。如果你需要在取消后做一些清理工作（比如删除临时文件、恢复状态），需要在对话框关闭之前完成——或者在 canceled 信号的槽函数中处理。

另一个细节是取消按钮的文字。构造函数的第二个参数就是取消按钮的文字，你可以传空字符串来隐藏取消按钮——这样进度对话框就没有取消功能，用户只能等它跑完。这在某些不允许中途取消的操作中是有意义的，比如固件升级——中途取消可能导致设备变砖。

### 3.3 setAutoClose / setAutoReset：自动关闭行为

QProgressDialog 在进度达到最大值时有两个自动行为可以通过 setAutoClose 和 setAutoReset 分别控制。

setAutoClose 控制是否在进度达到最大值时自动关闭对话框。默认是 true——进度到 100% 时对话框自动消失。如果你希望在进度完成后保持对话框打开（比如让用户确认结果或者查看日志），需要手动关闭：

```cpp
progress.setAutoClose(false);
progress.setValue(progress.maximum());
// 对话框保持打开
// ... 用户查看结果 ...
progress.close();  // 手动关闭
```

setAutoReset 控制是否在进度达到最大值时自动将进度值重置为最小值。默认也是 true。这意味着 setValue(maximum) 之后，下一次 setValue 调用会从 minimum 开始。如果 autoReset 为 true 而 autoClose 为 false，对话框会在进度到顶后重置进度条到 0，看起来就像重新开始了——这通常不是你想要的效果。所以在 setAutoClose(false) 的场景下，通常也需要 setAutoReset(false)。

```cpp
progress.setAutoClose(false);
progress.setAutoReset(false);
progress.setValue(progress.maximum());
// 进度条停在 100%，对话框保持打开
```

autoClose 和 autoReset 的组合有四种情况。autoClose=true + autoReset=true 是默认行为，进度到顶后对话框关闭并重置。autoClose=true + autoReset=false 会让对话框关闭但不重置——但这个组合基本没用，因为关闭后重置不重置用户都看不到了。autoClose=false + autoReset=true 会让进度条弹回 0% 但对话框不关——通常是个 bug。autoClose=false + autoReset=false 是"完成后保持显示"的正确配置。

在实际项目中需要干预 autoClose/autoReset 的场景不多。大多数情况下默认行为就是对的——进度到顶，对话框关闭，一切自然。需要手动控制的场景通常是：处理完成后要显示一个摘要（比如 "处理了 150 个文件，成功 148 个，失败 2 个"），用户看完摘要后手动关闭对话框。这种情况下你可以在进度到顶后更新 labelText 显示摘要，设置 autoClose=false 让对话框不消失。

### 3.4 与 QThread 配合：后台任务进度汇报

前面三节讲的所有用法都是在主线程中操作的——QProgressDialog 在主线程创建和更新，耗时操作也在主线程中执行（通过 QThread::msleep 模拟）。这种方式有一个严重的问题：虽然 QProgressDialog 在 setValue 中处理了事件，但界面响应并不流畅——每次 setValue 调用之间的间隔是 50ms，在这 50ms 内界面是冻住的。对于真正的耗时操作（比如压缩一个大文件、网络下载），这 50ms 的间隔就是实际的处理时间，界面可能会明显卡顿。

正确的做法是把耗时操作放到后台线程（QThread），通过信号槽把进度更新从后台线程发送到主线程来更新 QProgressDialog。Qt 的信号槽机制天然支持跨线程通信——当信号从后台线程发出、槽函数连接到主线程的对象时，Qt 会自动使用队列连接（Queued Connection），把信号参数打包成一个事件投递到主线程的事件循环中，主线程在下一个事件循环迭代时处理这个信号。这一切都是自动的，你不需要手动加锁或者使用条件变量。

我们来写一个完整的示例。后台线程执行一个模拟的耗时任务（分 100 步完成），每完成一步就发射一个进度信号，主线程接收信号更新 QProgressDialog。用户点击取消时通知后台线程停止。

```cpp
class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(int steps)
        : m_totalSteps(steps), m_cancel(false) {}

public slots:
    void doWork()
    {
        for (int i = 0; i <= m_totalSteps; ++i) {
            if (m_cancel.load()) {
                emit finished(false);
                return;
            }

            // 模拟每步耗时操作
            QThread::msleep(50);

            emit progressChanged(i);
        }
        emit finished(true);
    }

    void cancel()
    {
        m_cancel.store(true);
    }

signals:
    void progressChanged(int value);
    void finished(bool completed);

private:
    int m_totalSteps;
    std::atomic<bool> m_cancel;
};
```

Worker 是一个 QObject 子类，它会在一个 QThread 中执行。doWork 是实际的工作函数，包含一个从 0 到 m_totalSteps 的循环。每完成一步，发射 progressChanged 信号通知主线程更新进度。m_cancel 是一个 std::atomic<bool> 标志位，主线程通过调用 cancel 方法设置这个标志来通知后台线程停止。

```cpp
void MainWindow::onStartTask()
{
    const int totalSteps = 100;

    // 创建 Worker 和 Thread
    auto *worker = new Worker(totalSteps);
    auto *thread = new QThread;
    worker->moveToThread(thread);

    // 创建进度对话框
    auto *progress = new QProgressDialog(
        "正在处理...", "取消", 0, totalSteps, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(0);

    // 连接信号
    connect(thread, &QThread::started,
            worker, &Worker::doWork);
    connect(worker, &Worker::progressChanged,
            progress, &QProgressDialog::setValue);
    connect(progress, &QProgressDialog::canceled,
            worker, &Worker::cancel);
    connect(worker, &Worker::finished,
            this, [progress](bool completed) {
        if (completed) {
            progress->setLabelText("处理完成!");
        } else {
            progress->setLabelText("已取消");
        }
    });
    connect(worker, &Worker::finished,
            thread, &QThread::quit);
    connect(worker, &Worker::finished,
            worker, &Worker::deleteLater);
    connect(thread, &QThread::finished,
            thread, &QThread::deleteLater);

    thread->start();
}
```

这段代码的信号连接逻辑需要仔细梳理。thread->started 触发 worker->doWork，这是后台线程的入口。worker->progressChanged 跨线程传递到 progress->setValue，Qt 自动使用队列连接确保 setValue 在主线程执行。progress->canceled 触发 worker->cancel，设置原子标志位让后台线程在下一个循环迭代中退出。worker->finished 触发两个操作：更新标签文字显示结果，以及停止线程并清理资源。

资源清理的顺序很重要。finished 信号触发 thread->quit 让线程的事件循环退出，然后 thread->finished 触发 thread->deleteLater 在主线程中安全删除 QThread 对象。worker->deleteLater 确保在信号处理完成后安全删除 Worker 对象。注意不能直接在 finished 槽中 delete worker——因为 finished 是从后台线程发出的，此时 worker 还在后台线程的栈帧中，直接 delete 会导致未定义行为。

这段代码有一个潜在的问题需要注意：如果主窗口在任务还没完成时被关闭，QProgressDialog 会被 Qt 的父子机制自动销毁，但后台线程还在运行。你需要确保主窗口关闭时通知后台线程停止并等待它退出。一个安全的做法是在主窗口的析构函数中调用 thread->quit() + thread->wait()。

## 4. 踩坑预防

第一个坑是 setMinimumDuration 的默认值。QProgressDialog 默认有 4 秒的最短等待时间——如果任务在 4 秒内完成，对话框根本不会出现。这个设计是合理的（避免短操作弹出闪烁的对话框），但在调试时经常让人困惑——"我的进度对话框为什么不出来？"。调试时设为 0 可以立即显示，上线时根据实际任务的耗时决定是否保留。

第二个坑是在主线程的循环中做耗时操作。QProgressDialog 的 setValue 确实会处理事件，但它不是万能的。如果你的每步操作耗时超过 100ms，用户会感觉到明显的卡顿——进度条的更新不流畅，取消按钮响应迟钝。正确的做法是把耗时操作移到 QThread，就像我们在 3.4 节中做的那样。QProgressDialog 只负责显示进度，不负责处理事件来保持界面流畅。

第三个坑是 setValue(0) 的行为。在 QProgressDialog 中，setValue(0) 不仅设置进度值为 0，还会触发对话框的重置逻辑——包括重置 wasCanceled 标志。这意味着如果你在循环开始前调用 setValue(0) 来初始化，然后用户在循环过程中点击了取消，wasCanceled 变为 true，但如果你在某个地方又调用了 setValue(0)，wasCanceled 会被重置为 false，你的取消检测就失效了。所以不要在循环过程中调用 setValue(0)。

第四个坑是跨线程直接调用 setValue。Worker 在后台线程执行，你不能在后台线程中直接调用 progress->setValue()——因为 QProgressDialog 是一个 QWidget，所有 QWidget 的操作都必须在主线程中执行。必须通过信号槽（自动队列连接）来跨线程更新。如果你直接调用，在调试模式下 Qt 会打印 "QObject::setParent: Cannot set parent, new parent is in a different thread" 之类的警告，在发布模式下行为未定义。

第五个坑是忘记处理 canceled 信号。如果你创建了进度对话框但没有连接 canceled 信号或者没有在循环中检查 wasCanceled()，用户点击取消按钮后对话框会关闭，但后台任务会继续运行——用户以为已经取消了，实际上任务还在后台消耗资源。必须在 canceled 的处理中真正停止后台任务。

## 5. 练习项目

我们来做一个综合练习：创建一个 QMainWindow 应用，中央是一个 QPushButton "开始处理"。点击按钮后启动一个后台 QThread 执行模拟任务（分 200 步，每步 30ms），同时弹出一个 QProgressDialog 显示进度。进度对话框的 labelText 随阶段变化——0~50 步显示"正在读取数据..."，51~120 步显示"正在解析数据..."，121~180 步显示"正在计算结果..."，181~200 步显示"正在写入..."。用户点击取消后，后台线程在下一步检测到取消标志并安全退出，进度对话框显示"已取消"。任务正常完成后，进度对话框的 autoClose 设为 false，labelText 显示"处理完成! 共耗时 X.X 秒"，用户手动关闭。

QPushButton 在任务执行期间被禁用，任务完成后恢复可用。QMainWindow 关闭时如果任务还在运行，需要等待后台线程退出后再关闭。

提示：使用 QElapsedTimer 计算耗时，std::atomic<bool> 作为取消标志。主窗口的 closeEvent 中检查线程是否在运行，如果是则调用 quit + wait。

## 6. 官方文档参考链接

[Qt 文档 -- QProgressDialog](https://doc.qt.io/qt-6/qprogressdialog.html) -- 进度对话框类

[Qt 文档 -- QProgressDialog::canceled](https://doc.qt.io/qt-6/qprogressdialog.html#canceled) -- 取消信号

[Qt 文档 -- QThread](https://doc.qt.io/qt-6/qthread.html) -- 线程类

[Qt 文档 -- QElapsedTimer](https://doc.qt.io/qt-6/qelapsedtimer.html) -- 高精度计时器

---

到这里，QProgressDialog 的核心用法就全部讲完了。setLabelText / setValue / setRange 三件套控制进度对话框的基本显示，canceled 信号和 wasCanceled 方法提供了用户取消操作的两种响应方式，setAutoClose / setAutoReset 控制进度到顶后的自动行为，与 QThread 配合则是长任务进度汇报的标准方案。掌握了这些内容，你的应用就再也不会在处理耗时操作时让用户对着冻住的界面发呆了。
