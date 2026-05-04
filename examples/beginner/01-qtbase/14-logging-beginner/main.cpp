/*
 * 14-logging-beginner 示例
 *
 * 演示 Qt 日志系统的基础用法：
 * 1. 基础日志宏：qDebug、qInfo、qWarning、qCritical、qFatal
 * 2. QLoggingCategory 分类日志
 * 3. 日志级别控制与规则设置
 * 4. 自定义消息处理器
 * 5. 日志输出到文件
 * 6. 线程安全的日志输出
 */

#include <QCoreApplication>      // 应用程序核心类
#include <QDebug>                // 调试输出
#include <QLoggingCategory>      // 日志类别
#include <QFile>                 // 文件操作
#include <QTextStream>           // 文本流
#include <QDateTime>             // 日期时间
#include <QThread>               // 线程类
#include <QMutex>                // 互斥锁
#include <QMutexLocker>          // 互斥锁保护器
#include <QCommandLineParser>    // 命令行解析
#include <QElapsedTimer>         // 计时器
#include <QStringList>

#include "networkmanager.h"
#include "databasemanager.h"
#include "workerthread.h"

// ========== 定义日志类别 ==========
Q_LOGGING_CATEGORY(mainLog, "app.main")
Q_LOGGING_CATEGORY(networkLog, "app.network")
Q_LOGGING_CATEGORY(databaseLog, "app.database")
Q_LOGGING_CATEGORY(perfLog, "app.performance")

// ========== 全局变量（用于文件日志） ==========
QFile *g_logFile = nullptr;              // 日志文件指针
QMutex g_logMutex;                       // 保护文件写入的互斥锁

// ========== 自定义消息处理器 ==========

/**
 * 自定义消息处理器：同时输出到控制台和文件
 * @param type 消息类型（Debug、Info、Warning、Critical、Fatal）
 * @param context 消息上下文（文件名、行号、函数名、类别）
 * @param msg 消息内容
 */
void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // 获取当前时间戳
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    // 获取当前线程 ID
    QString threadId = QString::number(quintptr(QThread::currentThreadId()), 16);

    // 获取日志类别
    QString category = context.category ? context.category : "default";

    // 根据消息类型确定级别字符串
    QString level;
    FILE *outputStream = stderr;  // 默认输出到标准错误

    switch (type) {
    case QtDebugMsg:
        level = "DEBUG";
        break;
    case QtInfoMsg:
        level = "INFO ";
        break;
    case QtWarningMsg:
        level = "WARN ";
        break;
    case QtCriticalMsg:
        level = "ERROR";
        outputStream = stderr;  // 错误也输出到 stderr
        break;
    case QtFatalMsg:
        level = "FATAL";
        outputStream = stderr;
        break;
    }

    // 构造格式化的日志消息
    QString formattedMsg = QString("[%1][%2][%3][%4] %5\n")
                               .arg(timestamp)
                               .arg(threadId)
                               .arg(category)
                               .arg(level)
                               .arg(msg);

    // 输出到控制台
    fprintf(outputStream, "%s", formattedMsg.toUtf8().constData());
    fflush(outputStream);

    // 输出到文件（线程安全）
    if (g_logFile && g_logFile->isOpen()) {
        QMutexLocker locker(&g_logMutex);
        QTextStream stream(g_logFile);
        stream << formattedMsg;
        stream.flush();
    }

    // Fatal 消息会导致程序终止
    if (type == QtFatalMsg) {
        abort();
    }
}

// ========== 主函数 ==========

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // ========== 命令行参数解析 ==========
    QCommandLineParser parser;
    parser.setApplicationDescription("Qt 日志系统示例");
    parser.addHelpOption();
    parser.addVersionOption();

    // --verbose 参数：启用详细日志
    QCommandLineOption verboseOption(QStringList() << "v" << "verbose",
                                    "启用详细日志输出");
    parser.addOption(verboseOption);

    // --log-file 参数：指定日志文件路径
    QCommandLineOption logFileOption(QStringList() << "l" << "log-file",
                                    "指定日志文件路径",
                                    "file");
    parser.addOption(logFileOption);

    // --rules 参数：设置日志规则
    QCommandLineOption rulesOption("rules",
                                  "设置日志规则（如：'*.debug=true'）",
                                  "rules");
    parser.addOption(rulesOption);

    parser.process(app);

    // ========== 初始化日志文件 ==========
    QString logFilePath = parser.value(logFileOption);
    if (logFilePath.isEmpty()) {
        // 默认使用当前日期作为日志文件名
        logFilePath = QString("app_%1.log")
                          .arg(QDateTime::currentDateTime().toString("yyyyMMdd"));
    }

    g_logFile = new QFile(logFilePath);
    if (g_logFile->open(QIODevice::Append | QIODevice::Text)) {
        qInfo() << "日志文件:" << logFilePath;
    } else {
        qWarning() << "无法打开日志文件:" << logFilePath;
        delete g_logFile;
        g_logFile = nullptr;
    }

    // ========== 安装自定义消息处理器 ==========
    qInstallMessageHandler(customMessageHandler);

    qInfo() << "========== Qt 日志系统示例启动 ==========";
    qInfo() << "Qt 版本:" << QT_VERSION_STR;

    // ========== 应用日志规则 ==========
    QString rules = parser.value(rulesOption);
    if (parser.isSet(verboseOption)) {
        // --verbose 启用所有调试日志
        rules = "*.debug=true";
    }

    if (!rules.isEmpty()) {
        QLoggingCategory::setFilterRules(rules);
        qInfo() << "应用日志规则:" << rules;
    }

    // ========== 示例 1: 基础日志宏 ==========
    qDebug() << "--- 示例 1: 基础日志宏 ---";

    qDebug() << "这是调试信息";
    qInfo() << "这是普通信息";
    qWarning() << "这是警告信息";
    qCritical() << "这是严重错误";
    // qFatal() 会终止程序，所以这里不调用
    // qFatal("这是致命错误，程序将终止");

    qDebug() << "";

    // ========== 示例 2: 分类日志 ==========
    qDebug() << "--- 示例 2: 分类日志 ---";

    qCDebug(mainLog) << "主应用日志 - 调试级别";
    qCInfo(mainLog) << "主应用日志 - 信息级别";
    qCWarning(mainLog) << "主应用日志 - 警告级别";
    qCCritical(mainLog) << "主应用日志 - 错误级别";

    qCDebug(networkLog) << "网络模块日志 - 调试级别";
    qCDebug(databaseLog) << "数据库模块日志 - 调试级别";
    qCDebug(perfLog) << "性能日志 - 调试级别";

    qDebug() << "";

    // ========== 示例 3: 检查日志级别 ==========
    qDebug() << "--- 示例 3: 检查日志级别 ---";

    // 在执行耗时操作前检查日志级别
    if (mainLog().isDebugEnabled()) {
        qCDebug(mainLog) << "调试日志已启用，将输出详细信息";
    } else {
        qCInfo(mainLog) << "调试日志已禁用";
    }

    if (networkLog().isWarningEnabled()) {
        qCWarning(networkLog) << "警告日志已启用";
    }

    qDebug() << "";

    // ========== 示例 4: NetworkManager 演示 ==========
    qDebug() << "--- 示例 4: NetworkManager ---";

    NetworkManager networkMgr;
    networkMgr.connectToServer("https://example.com/api");
    networkMgr.connectToServer("https://example.com/api");
    networkMgr.connectToServer("https://example.com/api");

    networkMgr.downloadData(1024 * 100);  // 100KB

    qDebug() << "";

    // ========== 示例 5: DatabaseManager 演示 ==========
    qDebug() << "--- 示例 5: DatabaseManager ---";

    DatabaseManager dbMgr;
    dbMgr.connect();
    dbMgr.executeQuery("SELECT * FROM users");
    dbMgr.executeQuery("");  // 空查询，会产生警告
    dbMgr.executeQuery("DROP TABLE users");  // 危险操作，会产生错误

    qDebug() << "";

    // ========== 示例 6: 多线程日志 ==========
    qDebug() << "--- 示例 6: 多线程日志 ---";

    qDebug() << "主线程 ID:" << QString::number(quintptr(QThread::currentThreadId()), 16);

    WorkerThread worker1("Worker-1");
    WorkerThread worker2("Worker-2");

    worker1.start();
    worker2.start();

    worker1.wait();
    worker2.wait();

    qDebug() << "";

    // ========== 示例 7: 流式输出演示 ==========
    qDebug() << "--- 示例 7: 流式输出 ---";

    QString name = "Qt";
    int version = 6;
    double score = 98.5;

    qDebug() << "名称:" << name << "版本:" << version << "评分:" << score;

    // 链式调用
    qCDebug(mainLog) << "链式输出:" << "A" << 1 << 3.14 << QByteArray("data");

    qDebug() << "";

    // ========== 示例 8: 条件日志 ==========
    qDebug() << "--- 示例 8: 条件日志（性能优化） ---";

    // 正确做法：先检查再执行耗时操作
    if (perfLog().isDebugEnabled()) {
        // 假设这里有一些耗时的数据收集
        QStringList stats = QStringList() << "CPU: 50%" << "Memory: 200MB";
        qCDebug(perfLog) << "性能统计:" << stats.join(", ");
    }

    // 错误做法（不要这样）：无条件执行
    // qCDebug(perfLog) << "性能统计:" << collectExpensiveStats();

    qDebug() << "";

    // ========== 完成 ==========

    qInfo() << "========== 所有示例执行完毕 ==========";
    qInfo() << "日志文件保存在:" << logFilePath;

    // 清理
    if (g_logFile) {
        g_logFile->close();
        delete g_logFile;
        g_logFile = nullptr;
    }

    return 0;
}
