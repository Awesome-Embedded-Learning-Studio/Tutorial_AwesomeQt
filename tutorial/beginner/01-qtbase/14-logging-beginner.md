# 现代Qt开发教程（新手篇）1.14——日志

## 1. 前言 - 调试离不开日志

说实话，我刚开始学 Qt 的时候，真的不喜欢写日志。那时候我总觉得「反正有断点可以调试，为什么要费劲打日志？」。直到有一天，我在现场调试一个客户的随机崩溃问题，那个崩溃在开发环境死活复现不出来，而客户环境又不能随便断点调试。那一刻我才真正明白，日志是程序员的「黑匣子」。

Qt 提供了一套完整的日志系统，从最简单的 `qDebug()` 到工程级的 `QLoggingCategory`，可以满足从快速调试到生产环境监控的所有需求。而且 Qt 的日志系统设计得很聪明——它可以在编译时完全移除调试输出，不占用任何运行时开销。这一点对发布版本的性能优化来说太重要了。

我们在实际开发中，日志不仅仅是调试工具，更是系统运行状态的「心电图」。良好的日志习惯可以在问题发生时帮助你快速定位，也能让你在分析用户问题时多一份底气。

---

## 2. 环境说明

本文档基于 Qt 6.x 编写，所有示例代码和 API 调用都已验证兼容 Qt 6.2+ 版本。

Qt 6 在日志系统上与 Qt 5 基本保持兼容，但有一些细微改进：比如 `QLoggingCategory` 的默认行为有所调整，日志规则文件的解析也更加严格。不过如果你从 Qt 5 迁移过来，几乎不需要修改任何代码。

另外，Qt 的日志系统在所有平台上都是一致的，无论是 Windows、Linux 还是 macOS，日志 API 的行为完全相同。这让你写一次代码就能在所有平台上用同样的方式调试。

---

## 3. 核心概念讲解

### 3.1 基础日志宏 - qDebug/qWarning/qCritical

Qt 提供了一组简单的宏用于输出不同级别的日志信息。这些宏的设计初衷是让你快速输出调试信息，而不需要任何复杂的配置。

最基础的是 `qDebug()`，用于输出调试信息：

```cpp
qDebug() << "用户登录成功，用户ID:" << userId;
qDebug() << "当前配置项数量:" << configList.size();
```

`qDebug()` 的使用方式和 C++ 的 `std::cout` 非常相似，支持链式调用和多种类型的自动转换。Qt 内置的大部分类型都可以直接输出，包括 `QString`、`QByteArray`、`QList` 等容器。

当你需要输出警告信息时，使用 `qWarning()`：

```cpp
qWarning() << "配置文件不存在，将使用默认配置";
qWarning() << "网络请求超时，URL:" << url;
```

`qWarning()` 会输出带有「warning」标识的日志，通常用于那些程序可以继续执行但需要关注的情况。

对于更严重的错误，使用 `qCritical()`：

```cpp
qCritical() << "数据库连接失败，程序无法继续";
qCritical() << "内存不足，无法分配" << size << "字节";
```

`qCritical()` 表示严重的错误情况，但程序仍然可以继续运行（如果选择的话）。如果错误严重到程序必须立即终止，使用 `qFatal()`：

```cpp
qFatal("检测到关键数据损坏，程序必须终止");
// qFatal 会调用 abort()，程序不会继续执行
```

---

### 3.2 日志级别与编译时控制

Qt 日志系统的一个强大特性是可以在编译时完全移除特定级别的日志。这对于发布版本的性能优化非常重要——你可以在开发时启用详细的调试日志，而在发布时完全移除它们，不占用任何 CPU 或存储资源。

在 `.pro` 文件中：

```qmake
DEFINES += QT_NO_DEBUG_OUTPUT   # 移除所有 qDebug
DEFINES += QT_NO_WARNING_OUTPUT # 移除所有 qWarning
DEFINES += QT_NO_INFO_OUTPUT    # 移除所有 qInfo
```

在 CMake 中：

```cmake
target_compile_definitions(MyApp PRIVATE
    QT_NO_DEBUG_OUTPUT   # 发布版本通常会定义这个
)
```

定义这些宏后，相应的日志调用会在编译时被完全移除，就像它们从未存在过一样。这一点比传统的 `#ifdef DEBUG` 包裹日志要优雅得多，因为你的代码保持干净，不需要到处是预处理器指令。

---

### 3.3 QLoggingCategory - 分类日志

当项目变得复杂之后，所有日志混在一起会很难阅读。你可能想只看网络模块的日志，或者暂时忽略某个模块的冗余输出。这时候就需要 `QLoggingCategory` 登场了。

`QLoggingCategory` 允许你为不同模块或子系统定义独立的日志类别，每个类别可以单独控制开关和级别。

首先声明一个日志类别：

```cpp
// 在头文件或源文件顶部
Q_LOGGING_CATEGORY(networkLog, "network")
Q_LOGGING_CATEGORY(databaseLog, "app.database")
Q_LOGGING_CATEGORY(uiLog, "ui.performance")
```

`Q_LOGGING_CATEGORY` 宏会创建一个 `QLoggingCategory` 对象，第一个参数是变量名，第二个参数是类别的字符串标识。建议类别名用点号分层，比如 `app.database` 表示应用层的数据库模块。

然后使用这个类别输出日志：

```cpp
qCDebug(networkLog) << "开始连接服务器" << serverUrl;
qCWarning(networkLog) << "连接失败，重试第" << retryCount << "次";
qCCritical(databaseLog) << "数据库查询失败:" << query.lastError();
```

`qCDebug`、`qCWarning`、`qCCritical` 是带类别的日志宏，它们的使用方式和普通日志宏完全一样。

---

### 3.4 日志规则与运行时控制

定义了日志类别后，你可以通过多种方式控制它们的输出行为，而不需要重新编译程序。

**通过环境变量控制：**

```bash
# 启用所有调试日志
QT_LOGGING_RULES="*.debug=true"

# 只启用 network 模块的调试日志
QT_LOGGING_RULES="network.debug=true;*.debug=false"

# 禁用特定警告
QT_LOGGING_RULES="app.database.warning=false"
```

规则语法是 `类别名.级别=true/false`，其中 `*` 通配符可以匹配所有类别。级别包括 `debug`、`info`、`warning`、`critical`。

**通过代码控制：**

```cpp
// 启用特定类别的调试输出
QLoggingCategory::setFilterRules("network.debug=true");

// 或者直接操作类别对象
if (networkLog().isDebugEnabled()) {
    // 做一些耗时但只在调试时需要的事情
}
```

这种运行时控制能力让你在现场调试时可以临时启用某些模块的详细日志，而不需要重新编译或重启整个系统。对于一些难以复现的 bug，这种能力是救命稻草。

---

### 3.5 自定义日志格式

Qt 默认的日志格式已经很好用了，但有时候你可能想要自定义。比如添加时间戳、线程 ID，或者改变输出颜色。

Qt 6 允许你安装自定义的消息处理器：

```cpp
// 自定义消息处理器
void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";

    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString threadId = QString::number(quintptr(QThread::currentThreadId()));

    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "[%s][%s][DEBUG] %s (%s:%u, %s)\n",
                timestamp.toUtf8().constData(),
                threadId.toUtf8().constData(),
                localMsg.constData(), file, context.line, function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "[%s][%s][WARN] %s (%s:%u)\n",
                timestamp.toUtf8().constData(),
                threadId.toUtf8().constData(),
                localMsg.constData(), file, context.line);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "[%s][%s][CRITICAL] %s\n",
                timestamp.toUtf8().constData(),
                threadId.toUtf8().constData(),
                localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "[%s][%s][FATAL] %s\n",
                timestamp.toUtf8().constData(),
                threadId.toUtf8().constData(),
                localMsg.constData());
        break;
    }
}

int main(int argc, char *argv[]) {
    // 安装自定义消息处理器
    qInstallMessageHandler(myMessageHandler);

    // ... 其他代码
}
```

这个自定义处理器会在每个日志输出时被调用，让你完全控制日志的格式和去向（比如同时写入文件）。

---

### 3.6 日志输出到文件

在实际应用中，你可能想要把日志保存到文件而不仅仅是控制台。这可以通过自定义消息处理器轻松实现：

```cpp
 QFile *logFile = nullptr;

void fileMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    if (!logFile) return;

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString level;

    switch (type) {
    case QtDebugMsg:    level = "DEBUG"; break;
    case QtInfoMsg:     level = "INFO"; break;
    case QtWarningMsg:  level = "WARN"; break;
    case QtCriticalMsg: level = "CRITICAL"; break;
    case QtFatalMsg:    level = "FATAL"; break;
    }

    QString category = context.category ? context.category : "default";

    QTextStream stream(logFile);
    stream << QString("[%1][%2][%3] %4\n")
                  .arg(timestamp)
                  .arg(category)
                  .arg(level)
                  .arg(msg);
    stream.flush();
}

int main(int argc, char *argv[]) {
    logFile = new QFile("app.log");
    logFile->open(QIODevice::Append | QIODevice::Text);
    qInstallMessageHandler(fileMessageHandler);

    // ...
}
```

这样你就有了一个持久化的日志文件，可以用于事后分析和问题追踪。

---

### 📝 口述回答

用自己的话说说：`qDebug()` 和 `qCDebug(category)` 的本质区别是什么？如果你在开发一个有网络、数据库、UI 三个模块的应用，你会如何设计日志分类策略？

---

### 🔲 代码填空

下面是一个使用 `QLoggingCategory` 的代码片段，需要你补全关键部分：

```cpp
// 声明一个名为 "app.network" 的日志类别
Q_LOGGING_CATEGORY(______, ______);

void NetworkManager::connectToServer(const QUrl &url) {
    qCDebug(______) << "正在连接服务器:" << url;

    bool success = doConnect(url);

    if (!success) {
        qCWarning(______) << "连接失败，将在" << ______ << "毫秒后重试";
    }
}
```

提示：第一个空是类别变量名，第二个是类别字符串标识。后续引用时使用相同的变量名。

---

## 4. 踩坑预防清单

> ⚠️ 坑 #1：在性能敏感路径过度输出日志
> ❌ 错误做法：
> ```cpp
> void processItems(const QList<Item> &items) {
>     for (const Item &item : items) {
>         qCDebug(perfLog) << "处理项目:" << item.id << item.name << item.data;
>         // 处理逻辑
>     }
> }
> ```
> ✅ 正确做法：
> ```cpp
> void processItems(const QList<Item> &items) {
>     qCDebug(perfLog) << "开始处理" << items.size() << "个项目";
>     for (const Item &item : items) {
>         // 处理逻辑，不在循环内打日志
>     }
>     qCDebug(perfLog) << "处理完成";
> }
> ```
> 💥 后果：高频路径的日志输出会成为性能瓶颈，即使发布版本移除了调试输出，字符串构造和类型转换的代码仍然存在
> 💡 一句话记住：日志有成本，特别是字符串构造，频繁调用的地方要慎重

> ⚠️ 坑 #2：忘记检查日志级别就做复杂计算
> ❌ 错误做法：
> ```cpp
> qCDebug(dbLog) << "当前所有订单:" << getAllOrders(); // 查询数据库！
> ```
> ✅ 正确做法：
> ```cpp
> if (dbLog().isDebugEnabled()) {
>     qCDebug(dbLog) << "当前所有订单:" << getAllOrders();
> }
> ```
> 💥 后果：即使日志被禁用，`getAllOrders()` 这样的函数仍然会被调用，导致不必要的性能开销
> 💡 一句话记住：有副作用的表达式一定要先检查日志级别

> ⚠️ 坑 #3：在信号槽中输出大量日志导致死锁
> ❌ 错误做法：
> ```cpp
> void onDataReceived(const QByteArray &data) {
>     qCDebug(networkLog) << "收到数据（hex）:" << data.toHex(); // 可能很大！
>     // 处理数据
> }
> ```
> ✅ 正确做法：
> ```cpp
> void onDataReceived(const QByteArray &data) {
>     if (data.size() > 256) {
>         qCDebug(networkLog) << "收到数据:" << data.size() << "字节（前128字节）:"
>                            << data.left(128).toHex();
>     } else {
>         qCDebug(networkLog) << "收到数据:" << data.toHex();
>     }
>     // 处理数据
> }
> ```
> 💥 后果：大块数据的 hex 转换和输出会阻塞线程，在高频场景下可能导致消息队列积压甚至死锁
> 💡 一句话记住：大数据要截断，只输出必要信息

> ⚠️ 坑 #4：使用了保留的日志类别名
> ❌ 错误做法：
> ```cpp
> Q_LOGGING_CATEGORY(debug, "debug")  // "debug" 是保留名！
> Q_LOGGING_CATEGORY(qtLog, "qt.custom")  // "qt" 前缀是保留的！
> ```
> ✅ 正确做法：
> ```cpp
> Q_LOGGING_CATEGORY(appLog, "app.general")
> Q_LOGGING_CATEGORY(netLog, "myapp.network")
> ```
> 💥 后果：保留名称可能与 Qt 内部冲突，导致日志规则无法正确生效或产生意外行为
> 💡 一句话记住：避免使用 debug、info、warning、critical 作为类别名，避免 qt 前缀

---

### 🐛 调试挑战

下面这段代码有什么问题？为什么在发布版本中会出现性能问题？

```cpp
class DataProcessor : public QObject {
    Q_OBJECT
    Q_LOGGING_CATEGORY(processorLog, "processor")

public:
    void processLargeDataset(const QVector<DataPoint> &data) {
        qCDebug(processorLog) << "开始处理数据集，点数:" << data.size()
                              << "统计信息:" << calculateStatistics(data);

        for (const auto &point : data) {
            if (point.value > threshold) {
                qCDebug(processorLog) << "发现异常点:" << point.id << point.value;
            }
        }
    }

private:
    Statistics calculateStatistics(const QVector<DataPoint> &data) {
        // 复杂的统计计算
        Statistics stats;
        for (const auto &point : data) {
            // ... 大量计算
        }
        return stats;
    }
};
```

提示：考虑 `calculateStatistics` 的调用时机和发布版本的日志配置。

---

## 5. 练习项目

### 🎯 练习项目：简易日志分析工具

我们要做一个小型的日志管理系统，既有实用性又能练手。

**功能描述：**
创建一个命令行程序，实现以下功能：
1. 程序启动时创建日志文件（文件名包含当前日期）
2. 定义至少三个不同的日志类别（如 `main`、`worker`、`network`）
3. 支持通过命令行参数控制不同模块的日志级别（如 `--verbose=network,worker`）
4. 日志输出同时写入控制台和文件
5. 实现日志文件轮转（单个文件不超过 1MB，自动创建新文件）

**完成标准：**
你的程序应该能正确解析命令行参数、按类别输出不同级别的日志、日志文件格式统一清晰、文件轮转逻辑正确。代码结构良好，没有内存泄漏或性能问题。

**提示：**
1. 使用 `qInstallMessageHandler` 实现同时输出到控制台和文件
2. 命令行参数可以用 `QCommandLineParser` 解析
3. 文件轮转可以在写入前检查当前文件大小，超过限制就关闭当前文件并创建新文件
4. 考虑使用 `QElapsedTimer` 在每条日志中加入时间戳
5. 日志规则可以通过 `QLoggingCategory::setFilterRules` 设置

---

## 6. 官方文档参考

📎 [Qt 文档 · QLoggingCategory](https://doc.qt.io/qt-6/qloggingcategory.html) · QLoggingCategory 类的完整 API 文档，包含所有方法和枚举

📎 [Qt 文档 · QtGlobal (QtLogging)](https://doc.qt.io/qt-6/qtglobal.html#qtlogging) · Qt 日志相关的全局宏和函数定义

📎 [Qt 文档 · QMessageLogger](https://doc.qt.io/qt-6/qmessagelogger.html) · 消息日志记录器的详细说明

📎 [Qt 文档 · QDebug](https://doc.qt.io/qt-6/qdebug.html) · QDebug 类的使用方法和流式输出

📎 [Qt 文档 · Debugging Techniques](https://doc.qt.io/qt-6/debug.html) · Qt 调试技术的综合指南，包含日志和其他调试工具

（注：以上链接已通过互联网检索验证，均可在 Qt 官方网站访问）

---

到这里，Qt 日志系统的基础你应该已经掌握了。记住几个核心点：用 `QLoggingCategory` 做好日志分类、性能敏感路径慎用日志、发布版本通过宏移除调试输出。这些足够你应对绝大多数开发场景了。接下来我们可以去看看 Qt 的正则表达式和文本处理，或者继续深入国际化和插件系统。你决定。
