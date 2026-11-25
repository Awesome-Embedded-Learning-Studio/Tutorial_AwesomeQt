---
title: "1.14 日志进阶：自定义处理器与分类过滤"
description: "入门篇我们聊了 qDebug/qWarning/qCritical 的基本用法和 QLoggingCategory 的概念。说实话，开发阶段 qDebug() << \"value:\" << x 确实够用了。"
---

# 现代Qt开发教程（进阶篇）1.14——日志进阶：自定义处理器与分类过滤

## 1. 前言 / 日志系统是生产环境的眼睛

入门篇我们聊了 qDebug/qWarning/qCritical 的基本用法和 QLoggingCategory 的概念。说实话，开发阶段 `qDebug() << "value:" << x` 确实够用了。但到了生产环境，你需要的是：日志写入文件而不是控制台、按模块分类过滤日志级别、日志文件自动轮转防止撑爆磁盘、关键操作的审计日志单独存储——这些入门篇一个都没提到。

我之前在一个 7x24 运行的数据采集系统里踩过一个坑：qDebug 输出全部写到 stdout，被 systemd 收集到了 journal 里，一个月后 journal 文件占了 50GB。更离谱的是排查问题时发现关键错误信息被大量调试日志淹没了，grep 了半天才找到。后来实现了分级日志 + 文件轮转 + 分类过滤，问题才彻底解决。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。日志相关类（QLoggingCategory）和函数（qDebug/qInstallMessageHandler）属于 QtCore 模块。示例只依赖 QtCore，控制台程序即可验证。

## 3. 核心概念讲解

### 3.1 qInstallMessageHandler——接管所有日志输出

`qInstallMessageHandler` 允许你安装一个自定义的消息处理函数，接管 Qt 所有日志通道（qDebug、qWarning、qCritical、qFatal）的输出。这是实现自定义日志系统的入口点。

```cpp
void customMessageHandler(QtMsgType type, const QMessageLogContext& context,
                           const QString& msg)
{
    // 根据 type 决定日志级别
    const char* level = "UNKNOWN";
    switch (type) {
    case QtDebugMsg:    level = "DEBUG"; break;
    case QtInfoMsg:     level = "INFO"; break;
    case QtWarningMsg:  level = "WARN"; break;
    case QtCriticalMsg: level = "ERROR"; break;
    case QtFatalMsg:    level = "FATAL"; break;
    }

    // context 包含文件名、行号、函数名
    fprintf(stderr, "[%s] [%s:%u] %s\n",
            level, context.file, context.line,
            msg.toLocal8Bit().constData());
}

// 安装自定义处理器
qInstallMessageHandler(customMessageHandler);
```

QMessageLogContext 提供了日志调用的位置信息：file（文件名）、line（行号）、function（函数名）、category（日志分类）。这些信息在 Release 构建中默认不可用——需要定义 `QT_MESSAGELOGCONTEXT` 宏才能在 Release 中保留位置信息。

### 3.2 QLoggingCategory——按模块分类控制日志级别

QLoggingCategory 允许你为不同的模块或子系统定义独立的日志分类，每个分类可以独立控制是否输出 Debug/Info/Warning/Critical 级别的日志。

```cpp
// 在头文件中声明分类
Q_DECLARE_LOGGING_CATEGORY(networkLog)
Q_DECLARE_LOGGING_CATEGORY(databaseLog)

// 在 cpp 文件中定义分类
Q_LOGGING_CATEGORY(networkLog, "network")
Q_LOGGING_CATEGORY(databaseLog, "database")

// 使用分类日志
qCDebug(networkLog) << "Sending request to" << url;
qCInfo(databaseLog) << "Query executed in" << elapsed << "ms";
```

日志分类的级别可以在运行时通过环境变量或配置文件控制：

```bash
# 只输出 network 分类的 debug 及以上级别
QT_LOGGING_RULES="network.debug=true"
# 关闭所有 debug 级别
QT_LOGGING_RULES="*.debug=false"
# 组合规则
QT_LOGGING_RULES="network.debug=true;database.warning=true"
```

### 3.3 日志文件轮转——防止日志撑爆磁盘

生产环境中日志文件必须自动轮转——当文件大小或时间达到阈值时，关闭当前文件，创建新文件继续写入。Qt 本身不提供日志轮转功能，你需要在自定义消息处理器中实现。

常见的轮转策略有两种：按大小轮转（单文件超过指定大小后切换）和按时间轮转（每天一个文件）。按大小轮转适合日志量不确定的场景，按时间轮转适合需要按日期查找日志的场景。

### 3.4 结构化日志——方便日志分析工具解析

结构化日志将日志信息格式化为机器可解析的格式（如 JSON），方便 ELK Stack、Splunk 等日志分析工具索引和检索。

```cpp
void structuredLogHandler(QtMsgType type, const QMessageLogContext& context,
                           const QString& msg)
{
    QJsonObject log;
    log["level"] = levelToString(type);
    log["message"] = msg;
    log["file"] = QString(context.file);
    log["line"] = context.line;
    log["function"] = QString(context.function);
    log["timestamp"] = QDateTime::currentDateTime().toIso8601String();

    QJsonDocument doc(log);
    fprintf(stderr, "%s\n", doc.toJson(QJsonDocument::Compact).constData());
}
```

现在有一道调试题。下面这段日志处理器代码有什么问题？

```cpp
void myHandler(QtMsgType type, const QMessageLogContext& ctx, const QString& msg)
{
    QFile file("app.log");
    file.open(QIODevice::Append);
    file.write(msg.toUtf8());
    file.close();
}
```

问题在于：每次日志调用都打开和关闭文件，性能极差。如果每秒 1000 条日志，就是每秒 1000 次 open/write/close 系统调用。解决方案是保持文件打开状态（用静态变量或 RAII 包装），只在轮转时关闭旧文件打开新文件。

## 4. 踩坑预防

第一个坑是 Release 构建中 QMessageLogContext 的文件名和行号为空。默认情况下 Release 构建会移除这些信息以减少二进制体积。后果是生产日志没有位置信息，排查问题时无法定位到具体代码行。解决方案是在 CMake 中为目标添加 `QT_MESSAGELOGCONTEXT` 定义：`target_compile_definitions(myapp PRIVATE QT_MESSAGELOGCONTEXT)`。

第二个坑是日志处理器中的死循环。如果你在自定义消息处理器中调用了任何可能触发 Qt 日志的函数（比如 QString 的 arg() 在某些错误情况下会触发 qWarning），就会形成无限递归。后果是栈溢出崩溃。解决方案是在消息处理器中只使用 C 标准库函数（fprintf、fwrite）和原始字节数组，不要调用任何 Qt 函数。

第三个坑是日志文件权限问题。在 Linux 上，如果你的程序以 systemd 服务运行，日志文件需要正确的权限设置。如果文件由 root 创建但你的程序以非 root 用户运行，后续写入会失败。后果是日志静默丢失。解决方案是在程序启动时检查日志文件的权限和所有者，或者用 umask 控制文件创建权限。

## 5. 练习项目

练习项目：分级日志框架。实现一个支持文件输出、分类过滤和大小轮转的日志框架。

具体要求是：LogManager 类在程序启动时安装自定义消息处理器，日志按级别写入不同文件（debug.log、error.log），支持 QLoggingCategory 按模块过滤，单个文件超过 10MB 自动轮转（保留最近 5 个文件）。完成标准是日志格式统一（时间戳 + 级别 + 位置 + 消息）、轮转不丢日志、分类过滤可通过环境变量控制。

提示几个关键点：用静态变量保持文件打开状态，轮转时用 rename 而不是 copy+truncate，QLoggingCategory 的规则可以用 QLoggingCategory::setFilterRules 在代码中设置。

## 6. 官方文档参考链接

[Qt 文档 · QLoggingCategory](https://doc.qt.io/qt-6/qloggingcategory.html) -- 日志分类类参考

[Qt 文档 · qInstallMessageHandler](https://doc.qt.io/qt-6/qtglobal.html#qInstallMessageHandler) -- 消息处理器安装函数

[Qt 文档 · Debugging Techniques](https://doc.qt.io/qt-6/debug.html) -- Qt 调试技术总览

---

到这里，日志系统的进阶知识就拆完了。自定义消息处理器、分类过滤、文件轮转、结构化输出——这些是让日志系统从「能用」变成「好用」的关键。下一篇我们来看正则表达式进阶。
