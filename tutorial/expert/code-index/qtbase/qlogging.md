---
title: qDebug 与 QLoggingCategory 源码索引
description: qDebug 是宏展开成 QMessageLogger 成员函数指针取址、QMessageLogContext release 默认丢 file/line/function（QT_NO_MESSAGELOGCONTEXT）、qt_message_print 真正分流（thread_local 防自递归）+ qt_message_output 是带 isFatal 外层、qFatal 走 qAbort（Windows fastfail/TerminateProcess 其他 std::abort）不跑析构非 exit、qInstallMessageHandler fetchAndStoreOrdered 直接替换不 wrap 返回旧 handler nullptr restore、QDebug RAII 析构输出、QLoggingCategory 构造默认 0x01010101 全开（注释漏 info 爆点）+ defaultCategoryFilter 只关 qt.* 前缀、4 套规则集 QtConfig→Config→Api→Environment、qCDebug for+Holder 编译期短路、Qt6.9 Q_LOGGING_CATEGORY 改函数返回引用、QT_MESSAGE_PATTERN 默认 %{if-category}%{category}: %{endif}%{message}。
---

# qDebug 与 QLoggingCategory 源码索引

> 本索引收录 Qt 6.9.1 源码中 qDebug/qWarning/QLoggingCategory 的已验证证据。tr 用的 QMessageLogger 上下文机制与元对象系统关联见 [QMetaObject 静态元数据](./qmetaobject-static-metadata.md)。

## qDebug 宏链

源码文件：`qtbase/src/corelib/global/qlogging.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 【纠偏】qDebug 是宏非函数 | qlogging.h:165-169 | `#define qDebug QMessageLogger(QT_MESSAGELOG_FILE,QT_MESSAGELOG_LINE,QT_MESSAGELOG_FUNC).debug` | .debug 无括号=成员函数指针取址；`qDebug("x")`=`(QMessageLogger(...).debug)("x")`。 |
| QT_NO_DEBUG_OUTPUT 降级 | qlogging.h:171-184 | `#define QT_NO_QDEBUG_MACRO while(false) QMessageLogger().noDebug` | while(false) 保证 << 链仍编译；qCritical/qFatal 无降级永远启用。 |
| QtMsgType 不按严重性排序 | qlogging.h:29-39 | `QtDebugMsg, QT7_ONLY(QtInfoMsg,) QtWarningMsg, QtCriticalMsg, QtFatalMsg, QT6_ONLY(QtInfoMsg,)` | Qt6 中 QtInfoMsg 数值在 QtFatalMsg 后。 |

## QMessageLogContext

源码文件：`qtbase/src/corelib/global/qlogging.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 【关键纠偏】release 默认丢 file/line/function | qlogging.h:147-163 | `#if defined(QT_NO_DEBUG) #define QT_NO_MESSAGELOGCONTEXT` → `QT_MESSAGELOG_FILE nullptr` | release 默认 file/line/function 全 nullptr；要保留须定义 QT_MESSAGELOGCONTEXT。category 始终填充。 |

## 消息分发链

源码文件：`qtbase/src/corelib/global/qlogging.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| qt_message_print 真正分流 | qlogging.cpp:2096-2119 | defaultCategory 过滤 + grabMessageHandler thread_local 防自递归 + `(msgHandler?msgHandler:qDefaultMessageHandler)` | handler 内 qDebug 直接走 stderr 不重入。 |
| 【纠偏】stderr 是最后一跳 | qlogging.cpp:2155-2161+2053-2066+1993-2005 | qt_message_output 是带 isFatal 外层；stderr_message_handler fprintf+fflush | macOS/iOS AppleUnifiedLogger/Windows win_message_handler 可能短路 stderr。 |
| 【关键纠偏】qFatal 走 qAbort 非 exit | qlogging.cpp:2121-2150+qassert.cpp:24-51 | qt_message_fatal→qAbort；Windows fastfail/RaiseFailFast/TerminateProcess，其他 std::abort | 不跑析构；MSVC debug 弹 _CrtDbgReportW。 |
| isFatal 计数器 | qlogging.cpp:203-219 | QtFatalMsg 永远；QtCriticalMsg 受 QT_FATAL_CRITICALS；QtWarningMsg 受 QT_FATAL_WARNINGS 倒数 | =1 第 1 条 abort，=5 第 5 条；CI 抓 warning 常用。 |

## qInstallMessageHandler

源码文件：`qtbase/src/corelib/global/qlogging.cpp` / `qlogging.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 【关键纠偏】直接替换不 wrap | qlogging.cpp:2357-2364 | `fetchAndStoreOrdered(h)` + 返回 old（空返 qDefaultMessageHandler） | 您的 handler 完全接管；想 wrap 自己保存返回值。 |
| nullptr restore | qlogging.cpp:2114-2115 | `msgHandler ? msgHandler : qDefaultMessageHandler` | fetchAndStoreOrdered(nullptr) 后回退默认。 |
| messageHandler 原子指针 | qlogging.cpp:1748 | `QBasicAtomicPointer<...> messageHandler = Q_BASIC_ATOMIC_INITIALIZER(nullptr)` | 多线程 install + qDebug 并发安全；handler 自身要自同步。 |

## QDebug 流式（RAII）

源码文件：`qtbase/src/corelib/io/qdebug.h` / `qdebug.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| Stream 结构核心字段 | qdebug.h:53-79 | QTextStream ts + buffer + ref=1 + type + space + message_output=false + context | qDebug()<<x 临时对象，析构输出。 |
| ~QDebug 输出点 | qdebug.cpp:155-168 | ref-- 到 0 + 砍尾空格 + message_output 则 qt_message_output | message_output=false 是禁用开关（category 关 debug 时设）。 |
| debug(cat) 关 debug 禁输出 | qlogging.cpp:494-505 | `if (!cat.isDebugEnabled()) dbg.stream->message_output = false` | << 链仍编译但不输出。 |

## QLoggingCategory

源码文件：`qtbase/src/corelib/io/qloggingcategory.h` / `qloggingcategory.cpp` / `qloggingregistry.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 4 原子 bool union 共享 | qloggingcategory.h:44-53 | `union { AtomicBools bools; QBasicAtomicInt enabled; }` | 一次 storeRelaxed 设全 4 个。 |
| 【重大爆点】构造默认 0x01010101 全开 | qloggingcategory.cpp:172-185 | `enabled.storeRelaxed(0x01010101); // 注释只列 debug/warning/critical 漏 info` | 4 字节各 0x01 全 true；注释与行为不符。 |
| defaultCategoryFilter 三步 | qloggingregistry.cpp:436-489 | enableForLevel 起始 + 硬编码 qt/qt.* 关 debug（env 可 override）+ 4 规则集覆盖 | 用户自定义 category 默认开 debug。 |

## 规则系统

源码文件：`qtbase/src/corelib/io/qloggingregistry.cpp` / `qloggingregistry_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 自动加载 4 源 + API 第 5 源 | qloggingregistry.cpp:278-326 | QT_LOGGING_CONF / QT_LOGGING_RULES / DataPath ini / GenericConfig ini | 覆盖顺序 QtConfig→Config→Api→Environment，env 最高。 |
| 4 通配模式 | qloggingregistry.cpp:54-84 | FullText/LeftFilter/RightFilter/MidFilter，返回 +1/-1/0 | 中间 * 不支持（flags 清空=不匹配）。 |
| messageType 后缀决定 | qloggingregistry.cpp:95-133 | .debug/.info/.warning/.critical chopped；无后缀=-1 匹配所有 | `driver.usb=false`=整类全关。 |

## qCDebug / Q_LOGGING_CATEGORY 宏

源码文件：`qtbase/src/corelib/io/qloggingcategory.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| qCDebug 编译期短路 | qloggingcategory.h:153-161 | `for (Holder qt_category(cat()); qt_category; qt_category.control=false)` + Q_UNLIKELY | category 关时 for 一次不执行，零开销。 |
| 【Qt6.9 新】category 改函数返回引用 | qloggingcategory.h:104-151 | `const QLoggingCategory &name() { static const QLoggingCategory category(...); return category; }` | 避免 static init order fiasco；deprecation 警告仅 Qt 内部构建，用户代码无感。 |

## 格式化 + 线程

源码文件：`qtbase/src/corelib/global/qlogging.cpp` / `qloggingcategory.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 默认 pattern | qlogging.cpp:1138-1160 | `%{if-category}%{category}: %{endif}%{message}`（Android 只 %{message}） | qDebug 默认裸消息；qCDebug 带 category 前缀。 |
| 占位符全集 | qlogging.cpp:1111-1129 | category/type/message/file/line/function/pid/appname/threadid/qthreadptr/time/backtrace/if-*/endif | threadid=OS 线程号，qthreadptr=QThread* 指针值。 |
| handler 在产生线程跑 | qlogging.cpp:2077-2089 | thread_local msgHandlerGrabbed 防自递归 | 子线程 qDebug handler 在子线程；handler 内不能 UI 操作。 |
| installFilter 不能并发 | qloggingcategory.cpp:348-364 | 文档：called from different threads; never concurrently; cannot call QLoggingCategory static functions | filter 内禁调 installFilter/setFilterRules（死锁）。 |
