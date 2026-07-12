---
title: QProcess 源码索引
description: QProcess 继承 QIODevice 的不对称 IO 契约、ProcessState 三态（setProcessState 是 protected）、start→startProcess→startupNotification 调用链、Unix 用 vforkfd（非 posix_spawn）+ pidfd 收割、Windows CreateProcess+createProcessArgumentsModifier 钩子、MergedChannels 的 dup2/同句柄实现、forkfd 事件化死亡收割（无 SIGCHLD 处理器）、terminate/kill 跨平台语义不对称（Windows terminate 是 EnumWindows+WM_CLOSE）、errorOccurred 取代 error 信号、waitFor* 默认 30000ms、Windows CrashExit 的 NT STATUS 范围判定、startDetached double-fork+setsid。
---

# QProcess 源码索引

> 本索引收录 Qt 6.9.1 源码中 QProcess 的已验证证据。QProcess 继承 QIODevice，IO 契约的不对称设计见 [QIODevice 与 QFile](./qiodevice-fileio.md)。

## 继承 QIODevice 与不对称 IO 契约

源码文件：`qtbase/src/corelib/io/qprocess.h` / `qprocess.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 公开继承 QIODevice | qprocess.h:81 | `class Q_CORE_EXPORT QProcess : public QIODevice` | 子进程抽象成「流设备」，read/write/waitForReadyRead 语义直接复用。 |
| readData：NotRunning 返 -1(EOF)，否则返 0 | qprocess.cpp:2124-2132 | `if (d->processState == NotRunning) return -1; return 0;` | 真正读管道在 tryReadFromChannel，readData 只报 EOF；writeData 平台相关故 .cpp 无跨平台实现。 |
| 双套读信号 | qprocess.cpp:1123-1188 | readyRead（仅当前读通道==本通道时发）+ readyReadStandardOutput/Error（无论当前通道都发） | emittedReadyRead 标志防一次 readable 触发多次 readyRead 嵌套。 |

## ProcessState 三态（setProcessState 是 protected）

源码文件：`qtbase/src/corelib/io/qprocess.h` / `qprocess.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 三态枚举 | qprocess.h:98-102 | `enum ProcessState { NotRunning, Starting, Running };` | Starting=已 fork/CreateProcess 但未确认 execve 成功；Running=已换装目标程序。 |
| setProcessState 是 protected | qprocess.h:261-262 | `protected: void setProcessState(ProcessState state);` | 【纠偏】普通用户代码不能直接 setState；只能由内部 start/startupNotification/finished 驱动。 |
| setProcessState 只改 d + emit stateChanged | qprocess.cpp:2103-2109 | `if (d->processState == state) return; d->processState = state; emit stateChanged(state, QPrivateSignal());` | 无业务校验，纯状态写入 + 信号广播。 |

## start 调用链与 startupNotification 枢纽

源码文件：`qtbase/src/corelib/io/qprocess.cpp` / `qprocess_unix.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| start→d->start→startProcess | qprocess.cpp:2212-2228 | 运行中再 start 打 qWarning；空 program 直接 FailedToStart | 真正干活在 startProcess()（平台相关分派）。 |
| QProcessPrivate::start 预处理 | qprocess.cpp:2387-2424 | MergedChannels 时 setReadChannelCount(1) 否则 2；重置 exitCode/exitStatus/processError；末尾调 startProcess() | 末尾分派到 unix/win 实现。 |
| _q_startupNotification 状态枢纽 | qprocess.cpp:1270-1291 | 成功→setRunning+emit started；失败→setNotRunning+setErrorAndEmit(FailedToStart) | Unix 失败还 waitForDeadChild 收掉中间 vfork 子进程。 |
| processStarted(Unix)读 childStartedPipe 判定 | qprocess_unix.cpp:974-1004 | 读到 EOF(ret==0) 视为 execve 成功；读到 ChildError 即失败 | 成功后 stateNotifier 从监听 childStartedPipe 切到监听 forkfd。 |

## Unix 进程创建：vforkfd（非 posix_spawn）

源码文件：`qtbase/src/corelib/io/qprocess_unix.cpp` / `qcore_unix_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 【关键纠偏】走 vforkfd 非 posix_spawn | qprocess_unix.cpp:331-335 | `int ffdflags = FFD_CLOEXEC \| (isUsingVfork ? 0 : FFD_USE_FORK); return ::vforkfd(ffdflags, pid, &QChildProcess::startProcess, this);` | posix_spawn 整个文件零出现；FFD_USE_FORK 时才退化 fork。 |
| vfork/fork 运行时抉择 | qprocess_unix.cpp:641-667 | globalUsingVfork 在 ASan/TSan/Linux 关 pidfd/Darwin/运行时检测 libasan 五类环境返 false | 其余（主要 Linux+pidfd）走 vfork 省 fork 开销。 |
| 子进程换装逻辑 | qprocess_unix.cpp:928-972 | commitChannels→关 childStartedPipe 读端→fchdir→qt_safe_execv/execve→失败 failChildProcess | qt_safe_execve 是 EINTR 重试包装（qcore_unix_p.h:381-385）。 |
| 启动失败写回父进程 | qprocess_unix.cpp:802-816 | failChildProcess 把 ChildError{函数名,errno} 写回 childStartedPipe[1] 再 _exit(-1) | _exit 避免运行析构（vfork 上下文不能跑 atexit）。 |

## Windows 进程创建：CreateProcess + modifier

源码文件：`qtbase/src/corelib/io/qprocess_win.cpp` / `qprocess.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| CreateProcess 全流程 | qprocess_win.cpp:531-604 | 无控制台加 CREATE_NO_WINDOW + CREATE_UNICODE_ENVIRONMENT；bInheritHandles=true | 真正 CreateProcess 调用在 callCreateProcess 内 511 行；成功后立刻 setRunning。CreateProcess 同步返回即知成败（与 Unix 异步 startupNotification 不同）。 |
| createProcessArgumentsModifier 钩子 | qprocess_win.cpp:507-515 | callCreateProcess 内 CreateProcess 前执行用户 modifier | 改 STARTUPINFO/dwCreationFlags/environment 的官方后门。 |
| 【纠偏】modifier 是 Windows 专属 | qprocess.h:160-207 | createProcessArgumentsModifier 在 `#if Q_OS_WIN\|\|Q_QDOC`；Unix 对等物 childProcessModifier | 非 Win 平台编译期不存在；qdoc 模式下文档看着跨平台。Qt5 虚函数 setupChildProcess 在 Qt6 是 deprecated 桩（:272-279，Qt7 移除）。 |

## 管道与 ChannelMode 五态

源码文件：`qtbase/src/corelib/io/qprocess_unix.cpp` / `qprocess_win.cpp` / `qprocess.h` / `qprocess.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| MergedChannels(Unix)用 dup2 | qprocess_unix.cpp:583-599 | commitChannels：stderr 没建管时 `qt_safe_dup2(STDOUT_FILENO, STDERR_FILENO, 0)` | stderr 接到 stdout。 |
| MergedChannels(Windows)用同句柄 | qprocess_win.cpp:485-505 | STARTUPINFOW 的 hStdError 直接填成 hStdOutput 同一 pipe | dwFlags 必含 STARTF_USESTDHANDLES。 |
| ChannelMode 五态 + Channel 四类型 | qprocess.h:108-115, qprocess_p.h:196-243 | SeparateChannels/MergedChannels/ForwardedChannels/ForwardedOutputChannel/ForwardedErrorChannel；Channel 类型 Normal/PipeSource/PipeSink/Redirect | PipeSource/PipeSink 支撑 setStandardOutputProcess 串联；Redirect 支撑 setStandardOutputFile。 |
| setProcessChannelMode 只赋值 | qprocess.cpp:1359-1361 | `d->processChannelMode = mode;` 无副作用 | 必须在 start 前调；运行中调无效不报错。 |

## 死亡收割：forkfd 事件化（无 SIGCHLD 处理器）

源码文件：`qtbase/src/corelib/io/qprocess_unix.cpp` / `qprocess_win.cpp` / `src/3rdparty/forkfd/forkfd.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 【架构事实】Qt6 无 QProcessManager/SIGCHLD 处理器 | qprocess_unix.cpp:1293-1310 | `QT_EINTR_LOOP(ret, forkfd_wait(forkfd, &info, nullptr));` | forkfd 把子进程死亡包装成可读 fd（Linux pidfd），QSocketNotifier 监听；避免多 QProcess 抢 SIGCHLD。 |
| Unix CrashExit 判定 | qprocess_unix.cpp:1303-1304, forkfd.h:50-53 | `exitCode = info.status; exitStatus = info.code == CLD_EXITED ? NormalExit : CrashExit;` | forkfd_info{code, status}；code 是 CLD_EXITED/CLD_KILLED 等 waitid 风格值。 |
| 【Windows 玄学】CrashExit 靠 NT STATUS 范围猜 | qprocess_win.cpp:796-808 | `theExitCode >= 0x80000000 && theExitCode < 0xD0000000` 或 ==KillProcessExitCode | 0xC0000005 ACCESS_VIOLATION 等算 CrashExit；Windows 无 exit/signal 区分。 |
| KillProcessExitCode=0xf291 | qprocess_win.cpp:31 | `constexpr UINT KillProcessExitCode = 0xf291;` | Qt kill() 用此码，findExitCode 见到即知「我们自己杀的」。 |
| Windows 用 QWinEventNotifier 监听 hProcess | qprocess_win.cpp:598-600 | `new QWinEventNotifier(pid->hProcess, q); connect(..., _q_processDied());` | 等价 Unix forkfd notifier；内核对象 signaled 时触发。 |

## terminate / kill：跨平台语义不对称

源码文件：`qtbase/src/corelib/io/qprocess_unix.cpp` / `qprocess_win.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| Unix：terminate=SIGTERM，kill=SIGKILL | qprocess_unix.cpp:1121-1137 | `::kill(pid, SIGTERM)` / `::kill(pid, SIGKILL)` | 都是 ::kill() 系统调用，信号不同。 |
| 【关键纠偏】Windows terminate 不是 TerminateProcess | qprocess_win.cpp:631-653 | terminateProcess: `EnumWindows(qt_terminateApp, pid) + PostThreadMessage(WM_CLOSE)`；killProcess: `TerminateProcess(hProcess, KillProcessExitCode)` | Windows terminate 对 GUI 程序有效（弹保存框），对无消息循环的 console 程序完全无效；kill() 才硬终止。 |

## 错误体系与 waitFor* 超时

源码文件：`qtbase/src/corelib/io/qprocess.h` / `qprocess.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| ProcessError 六态 | qprocess.h:85-92 | `enum ProcessError { FailedToStart, Crashed, Timedout, ReadError, WriteError, UnknownError };` | UnknownError 是初始默认值。 |
| 【纠偏】error() 是函数不是信号 | qprocess.h:217, :252-256 | `QProcess::ProcessError error() const;`（函数）/ signals 区段只有 errorOccurred（\\since 5.6） | Qt4/5 老 error() 信号已移除；connect 到 error() 信号是迁移坑。 |
| setErrorAndEmit 是 errorOccurred 唯一发射点 | qprocess.cpp:1023-1028 | `setError(...); emit q->errorOccurred(processError);` | waitFor* 超时只调 setError 不 emit（同步路径不发信号）。 |
| 【纠偏】waitFor* 默认 30000ms 非 -1 | qprocess.h:222-225 | 四个 `msecs = 30000` | 传 -1 才无限等；close() 内部用 -1。 |
| close() 无限等待刷干净 | qprocess.cpp:1873-1882 | `while (waitForBytesWritten(-1)) ; kill(); waitForFinished(-1);` | close 返回时进程必死。 |

## finished 时序与 startDetached

源码文件：`qtbase/src/corelib/io/qprocess.cpp` / `qprocess.h` / `qprocess_unix.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| processFinished 全流程 | qprocess.cpp:1236-1265 | 平台收尸→cleanup→CrashExit 则 setErrorAndEmit(Crashed)→readChannelFinished→finished | CrashExit 先发 errorOccurred 再发 finished。 |
| _q_processDied 先抽干管道余数 | qprocess.cpp:1210-1231 | `_q_canReadStandardOutput(); _q_canReadStandardError();` 在 processFinished 前 | 保证 finished 前拿到进程死前最后一批输出。 |
| finished 带 (exitCode, ExitStatus) | qprocess.h:253, qprocess.cpp:1260 | `void finished(int exitCode, QProcess::ExitStatus exitStatus = NormalExit);` | exitStatus 默认值只是声明默认，实际由 d->exitStatus 决定。 |
| startDetached(Unix) double-fork+setsid | qprocess_unix.cpp:1318-1354 | `::setsid(); ... startChild(&doubleForkPid);` | 不用 forkfd（父不收尸）；孙进程被 init 收养，脱离 Qt 生命周期。 |
| doFork 模板 | qprocess_unix.cpp:318-329 | isUsingVfork 时 vfork() 否则 fork()；子进程 _exit | vfork 下不能 return（弹栈破坏父进程）。 |
| 【源码彩蛋】setArguments warning 笔误 | qprocess.cpp:2532-2539 | setArguments 函数体内 `qWarning("QProcess::setProgram: Process is already running");` | copy-paste bug，文案应为 setArguments。 |
