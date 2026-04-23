# 现代Qt开发教程（新手篇）1.10——进程

## 1. 前言：为什么需要调用外部程序

说实话，我第一次需要在 Qt 程序里调用外部命令的时候，第一反应是「用 system()」。结果试了之后就后悔了——system() 是阻塞的，调用的时候整个界面都卡死，而且拿不到输出，也不知道命令到底执行成功没有。

后来我才发现，Qt 早就给我们准备了一个强大的类：QProcess。它不仅能启动外部程序，还能读取 stdout 和 stderr，能异步等待进程结束，甚至能设置环境变量和工作目录。这简直是把进程控制做到了极致。

想象一下这些场景：你需要调用 ffmpeg 转码视频、调用 git 获取仓库信息、调用系统命令查看 CPU 占用，甚至需要和其他进程进行管道通信。这些场景如果用 system() 或者 platform-dependent API，你会陷入跨平台兼容性的地狱。而 QProcess 把这一切都封装好了，Windows、Linux、macOS 通用。

所以这一篇，我们不玩虚的，直接把 QProcess 的核心用法搞清楚。从最简单的启动命令，到读取输出、异步等待，再到常见的坑，我们都一一覆盖。

## 2. 环境说明

本篇假设你已经掌握了前面的基础知识，特别是信号槽机制。因为 QProcess 大量使用信号槽来处理异步事件。

我们主要使用 Qt6 的 Core 模块，QProcess 就在这个模块里。不需要额外链接 Widgets 或其他模块。

## 3. 核心概念讲解

### 3.1 QProcess 启动进程

最基础的用法就是启动一个外部程序。QProcess 提供了两种启动方式：

```cpp
// 方式一：分开设置程序和参数
QProcess process;
process.start("git", QStringList() << "status");
process.waitForFinished();

// 方式二：一次性传入完整命令（不推荐，后面会讲为什么）
QProcess process;
process.start("git status"); // Windows 下可能出问题
```

第一种方式更安全，因为参数是分开传递的，不用担心空格和转义的问题。第二种方式看起来简洁，但在 Windows 下可能会因为路径解析问题导致失败。你可能会问，为什么推荐用 `start(program, args)` 而不是 `start(command)`？原因很简单：`start(program, args)` 分别传递程序名和参数列表，Qt 会自动处理空格和转义，是跨平台的；而 `start(command)` 把命令当成一个字符串，不同平台的 shell 解析规则不同，容易出问题。

### 3.2 读取 stdout 和 stderr

启动进程之后，我们通常需要获取它的输出。QProcess 把标准输出和标准错误输出当成了两个 I/O 通道，可以像读文件一样读取：

```cpp
QProcess process;
process.start("ls", QStringList() << "-la");
process.waitForFinished();

// 读取标准输出
QByteArray stdoutData = process.readAllStandardOutput();
qDebug() << "STDOUT:" << stdoutData;

// 读取标准错误输出
QByteArray stderrData = process.readAllStandardError();
qDebug() << "STDERR:" << stderrData;
```

这里要注意，`waitForFinished()` 是一个阻塞调用，会一直等待进程结束。如果进程执行时间很长，你的界面就会卡住。后面我们会讲异步方式。另外，`readAllStandardOutput()` 和 `readAllStandardError()` 返回的是 QByteArray，这是因为输出可能是二进制数据。如果你确定是文本，可以直接转成 QString：

```cpp
QString output = QString::fromUtf8(process.readAllStandardOutput());
```

这里有一个非常常见的坑：很多人在 `start()` 之后立即调用 `readAllStandardOutput()`，结果读到的是空数据。原因在于进程是异步执行的，你必须先等它结束，才能读到完整输出。所以一定要先调用 `waitForFinished()`，再读取。

### 3.3 异步等待进程结束

上面讲的 `waitForFinished()` 是阻塞式的，对于短命令还好，如果命令需要几秒甚至更长时间，界面就会卡死。这时候就需要用异步方式。

QProcess 提供了几个信号，最重要的是 `finished(int exitCode, QProcess::ExitStatus exitStatus)`。当进程结束时，这个信号会被触发：

```cpp
QProcess *process = new QProcess(this);

// 连接 finished 信号
connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [](int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus == QProcess::NormalExit) {
        qDebug() << "Process finished with exit code:" << exitCode;
    } else {
        qDebug() << "Process crashed";
    }
});

// 连接 readyReadStandardOutput 信号，实时读取输出
connect(process, &QProcess::readyReadStandardOutput, [=]() {
    QByteArray data = process->readAllStandardOutput();
    qDebug() << "New output:" << data;
});

process->start("long-running-command");
```

这样写有几个好处：界面不会卡，输出是实时流式读取的，而且能检测到进程是否崩溃。不过这里有一个细节很多人会忽略——如果你用 `new QProcess()` 而不传 parent，那每次启动进程都会泄漏一块内存。正确的做法是 `new QProcess(this)`，让父对象在销毁时自动清理它。

### 3.4 start() vs startDetached()

QProcess 还有一个 `startDetached()` 方法，它的行为和 `start()` 完全不同：

```cpp
// start()：父进程和子进程关联，父进程结束会影响子进程
QProcess process;
process.start("some-command");

// startDetached()：子进程独立运行，父进程结束不影响子进程
bool success = QProcess::startDetached("some-command");
```

`startDetached()` 返回一个 bool，表示启动是否成功。但它不会给你一个 QProcess 对象，因为你无法控制一个已经脱离的进程。这个方法适合启动那些完全独立运行的程序，比如打开浏览器、打开记事本等。需要注意的是，`startDetached()` 之后你没法再读取输出，因为 detached 进程已经和你的程序断开了连接。简单来说：需要交互用 `start()`，只需要启动用 `startDetached()`。

### 3.5 环境变量和工作目录

有时候你需要给子进程设置特定的环境变量或者工作目录：

```cpp
QProcess process;

// 设置工作目录
process.setWorkingDirectory("/path/to/directory");

// 设置环境变量（会继承父进程的环境）
QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
env.insert("MY_VAR", "some_value");
process.setProcessEnvironment(env);

// 清空所有环境变量（一般不这么做）
process.setProcessEnvironment(QProcessEnvironment());

process.start("my-program");
```

这里要注意的是，`setProcessEnvironment()` 会完全替换环境，所以通常先获取系统环境，然后再修改。

## 4. 踩坑预防清单

这里集中列几个最容易被忽略的问题。

路径中如果包含空格，比如 `C:\Program Files\MyApp.exe`，在某些情况下路径会被空格截断，导致找不到程序。解决办法是用参数列表形式启动，Qt 会正确处理：`process.start("C:\\Program Files\\MyApp.exe", QStringList())`。

关于线程的问题：QProcess 虽然可以跨线程使用，但启动和事件处理最好在同一线程。如果在非 GUI 线程启动 QProcess 并同步等待，可能导致事件循环问题或者线程竞争。正确做法是在主线程启动 QProcess，用异步信号槽处理。

另外一件很多人偷懒不做的事情是检查进程是否启动成功。外部程序是你控制不了的，如果程序不存在或者启动失败，你还在傻等一个永远不会结束的进程。正确的做法是检查 `waitForStarted()` 的返回值，或者监听 `errorOccurred` 信号。

现在我们来做一个小的代码填空练习，检验一下前面的内容。补全以下代码，实现启动一个命令并读取其输出：

```cpp
QProcess process;
process.______("git", QStringList() << "--version");
if (process.__________()) {
    QByteArray output = process.____________________();
    QString version = QString::fromUtf8(output);
    qDebug() << "Git version:" << version;
} else {
    qDebug() << "Failed to start git";
}
```

提示：需要填入的方法分别是 `start`、`waitForFinished`、`readAllStandardOutput`。

参考答案如下：

```cpp
QProcess process;
process.start("git", QStringList() << "--version");
if (process.waitForFinished()) {
    QByteArray output = process.readAllStandardOutput();
    QString version = QString::fromUtf8(output);
    qDebug() << "Git version:" << version;
} else {
    qDebug() << "Failed to start git";
}
```

## 5. 练习项目

我们来做一个命令执行器，把这一节学的东西串起来。功能是创建一个简单的命令执行器程序，用户可以输入任意 shell 命令，程序会执行并显示输出。

完成标准：有一个 QLineEdit 用于输入命令，一个 QPushButton 用于执行命令，一个 QTextEdit 用于显示命令输出。执行长命令时界面不能卡，而且能区分标准输出和错误输出（用不同颜色显示）。

提示几个方向：用 `QProcess::start()` 启动命令，连接 `readyReadStandardOutput` 和 `readyReadStandardError` 信号来实时获取输出，连接 `finished` 信号来处理命令结束。命令字符串需要拆分成程序名和参数列表，可以用空格分割，但要注意引号里的空格。

再看一个调试挑战：以下代码有什么问题？会导致什么后果？

```cpp
void executeCommand(QString command) {
    QProcess process;
    process.start(command);
    process.waitForFinished();
    qDebug() << process.readAllStandardOutput();
}
```

这里面有几个隐患。首先，`start(command)` 把整个命令当成一个字符串，包含参数的话可能失败。其次，QProcess 是局部变量，函数结束后就被销毁，可能来不及完成异步操作。另外没有检查启动是否成功，如果命令不存在会静默失败。应该用 `start(command.split(' '))` 或更好的参数解析方式，同时检查 `waitForStarted()` 的返回值。

## 6. 官方文档参考

- [Qt 文档 · QProcess 类](https://doc.qt.io/qt-6/qprocess.html) -- QProcess 的完整 API 参考，包含所有信号和槽的说明
- [Qt 文档 · QProcessEnvironment 类](https://doc.qt.io/qt-6/qprocessenvironment.html) -- 环境变量处理的专用类

*（链接已验证，2026-03-17 可访问）*

---

到这里就大功告成了。掌握了 QProcess，你就可以在 Qt 程序里自由地调用外部程序，这大大扩展了 Qt 的能力边界。下一节我们会讲定时器，看看如何在 Qt 里实现周期性任务。
