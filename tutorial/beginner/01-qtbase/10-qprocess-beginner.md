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

第一种方式更安全，因为参数是分开传递的，不用担心空格和转义的问题。第二种方式看起来简洁，但在 Windows 下可能会因为路径解析问题导致失败。

> 📝 **随堂测验：口述回答**
> 用自己的话说说：为什么推荐用 `start(program, args)` 而不是 `start(command)`？
>
> *(请先自己想一下，再往下滑看答案)*
>
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
>
> **答案参考**：
> - `start(program, args)` 分别传递程序名和参数列表，Qt 会自动处理空格和转义
> - `start(command)` 把命令当成一个字符串，不同平台的 shell 解析规则不同，容易出问题
> - 前者是跨平台的，后者依赖平台特定的 shell 行为
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

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

这里有几个关键点需要注意。`waitForFinished()` 是一个阻塞调用，会一直等待进程结束。如果进程执行时间很长，你的界面就会卡住。后面我们会讲异步方式。

另外，`readAllStandardOutput()` 和 `readAllStandardError()` 返回的是 QByteArray，这是因为输出可能是二进制数据。如果你确定是文本，可以直接转成 QString：

```cpp
QString output = QString::fromUtf8(process.readAllStandardOutput());
```

> ⚠️ **坑 #1：忘记 waitForFinished() 就读输出**
> ❌ 错误做法：`start()` 后立即 `readAllStandardOutput()`
> ✅ 正确做法：先 `waitForFinished()` 等进程结束，再读取输出
> 💥 后果：读到的是空数据，因为进程还没执行完
> 💡 一句话记住：进程是异步的，必须等结束才能读到完整输出

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

这样写有几个好处：界面不会卡，输出是实时流式读取的，而且能检测到进程是否崩溃。

> ⚠️ **坑 #2：忘记给 QProcess 设置 parent**
> ❌ 错误做法：`QProcess *process = new QProcess();` 后面忘记 delete
> ✅ 正确做法：`QProcess *process = new QProcess(this);` 或手动 delete
> 💥 后果：内存泄漏，每次启动进程都泄漏一块内存
> 💡 一句话记住：有 parent 的 QProcess 会在父对象销毁时自动清理

### 3.4 start() vs startDetached()

QProcess 还有一个 `startDetached()` 方法，它的行为和 `start()` 完全不同：

```cpp
// start()：父进程和子进程关联，父进程结束会影响子进程
QProcess process;
process.start("some-command");

// startDetached()：子进程独立运行，父进程结束不影响子进程
bool success = QProcess::startDetached("some-command");
```

`startDetached()` 返回一个 bool，表示启动是否成功。但它不会给你一个 QProcess 对象，因为你无法控制一个已经脱离的进程。这个方法适合启动那些完全独立运行的程序，比如打开浏览器、打开记事本等。

> ⚠️ **坑 #3：startDetached() 后尝试读取输出**
> ❌ 错误做法：用 `startDetached()` 后试图 `readAllStandardOutput()`
> ✅ 正确做法：`startDetached()` 适合启动独立程序，不需要交互的场景
> 💥 后果：读不到任何输出，因为 detached 进程已经和你的程序断开连接
> 💡 一句话记住：需要交互用 start()，只需要启动用 startDetached()

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

> ⚠️ **坑 #4：路径中的空格问题**
> ❌ 错误做法：`process.start("C:\\Program Files\\MyApp.exe");`
> ✅ 正确做法：`process.start("C:\\Program Files\\MyApp.exe", QStringList());`
> 💥 后果：在某些情况下，路径会被空格截断，导致找不到程序
> 💡 一句话记住：路径有空格时，用参数列表形式启动，Qt 会正确处理

> ⚠️ **坑 #5：在错误的线程启动进程**
> ❌ 错误做法：在非 GUI 线程启动 QProcess 并同步等待
> ✅ 正确做法：在主线程启动 QProcess，用异步信号槽处理
> 💥 后果：可能导致事件循环问题，或者线程竞争
> 💡 一句话记住：QProcess 虽然可以跨线程，但启动和事件处理最好在同一线程

> ⚠️ **坑 #6：忘记检查进程启动成功与否**
> ❌ 错误做法：`start()` 后直接假设进程在运行
> ✅ 正确做法：检查 `waitForStarted()` 返回值，或监听 `errorOccurred` 信号
> 💥 后果：程序不存在或启动失败时，你还在傻等一个永远不会结束的进程
> 💡 一句话记住：外部程序是你控制不了的，必须检查启动是否成功

> 🔲 **随堂测验：代码填空**
> 补全以下代码，实现启动一个命令并读取其输出：
>
> ```cpp
> QProcess process;
> process.______("git", QStringList() << "--version");
> if (process.__________()) {
>     QByteArray output = process.____________________();
>     QString version = QString::fromUtf8(output);
>     qDebug() << "Git version:" << version;
> } else {
>     qDebug() << "Failed to start git";
> }
> ```
>
> *(提示：start、waitForFinished、readAllStandardOutput)*
>
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
>
> **答案参考**：
> ```cpp
> QProcess process;
> process.start("git", QStringList() << "--version");
> if (process.waitForFinished()) {
>     QByteArray output = process.readAllStandardOutput();
>     QString version = QString::fromUtf8(output);
>     qDebug() << "Git version:" << version;
> } else {
>     qDebug() << "Failed to start git";
> }
> ```
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 5. 练习项目

🎯 **练习项目：命令执行器**

📋 **功能描述**：
创建一个简单的命令执行器程序，用户可以输入任意 shell 命令，程序会执行并显示输出。

✅ **完成标准**：
- 有一个 QLineEdit 用于输入命令
- 有一个 QPushButton 用于执行命令
- 有一个 QTextEdit 用于显示命令输出
- 执行长命令时界面不卡
- 能区分标准输出和错误输出（用不同颜色显示）

💡 **提示**：
- 用 `QProcess::start()` 启动命令
- 连接 `readyReadStandardOutput` 和 `readyReadStandardError` 信号来实时获取输出
- 连接 `finished` 信号来处理命令结束
- 命令字符串需要拆分成程序名和参数列表，可以用空格分割（注意引号里的空格）

> 🐛 **随堂测验：调试挑战**
>
> 以下代码有什么问题？会导致什么后果？
>
> ```cpp
> void executeCommand(QString command) {
>     QProcess process;
>     process.start(command);
>     process.waitForFinished();
>     qDebug() << process.readAllStandardOutput();
> }
> ```
>
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
>
> **答案参考**：
> - `start(command)` 把整个命令当成一个字符串，包含参数的话可能失败
> - QProcess 是局部变量，函数结束后就被销毁，可能来不及完成异步操作
> - 没有检查启动是否成功，如果命令不存在会静默失败
> - 应该用 `start(command.split(' '))` 或更好的参数解析方式
> - 应该检查 `waitForStarted()` 的返回值
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 6. 官方文档参考

📎 [Qt 文档 · QProcess 类](https://doc.qt.io/qt-6/qprocess.html) · QProcess 的完整 API 参考，包含所有信号和槽的说明
📎 [Qt 文档 · QProcessEnvironment 类](https://doc.qt.io/qt-6/qprocessenvironment.html) · 环境变量处理的专用类

*（链接已验证，2026-03-17 可访问）*

---

到这里就大功告成了。掌握了 QProcess，你就可以在 Qt 程序里自由地调用外部程序，这大大扩展了 Qt 的能力边界。下一节我们会讲定时器，看看如何在 Qt 里实现周期性任务。

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
