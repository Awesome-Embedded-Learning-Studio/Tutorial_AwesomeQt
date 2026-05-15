/// @file    custom_handler.cpp
/// @brief   自定义消息处理器实现。
///
/// 对应教程：进阶层 01-QtBase/14-日志。

#include "custom_handler.h"

#include <QDebug>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>
#include <QThread>

#include <QDateTime>

// ========== 日志文件全局变量 ==========
// Qt 的消息处理器是全局回调函数，无法传递用户数据（闭包），
// 因此只能通过全局变量访问日志文件。
static QFile* g_logFile = nullptr;                       ///< 日志文件指针
static QMutex g_logMutex;                                ///< 保护并发写入的互斥锁
static QtMessageHandler g_oldHandler = nullptr;          ///< 保存旧的处理器，用于恢复

/// @brief 将日志级别枚举转换为可读字符串。
/// @param[in] type Qt 消息级别。
/// @return 对应的可读字符串。
static const char* msgTypeToString(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:    return "DEBUG";
    case QtInfoMsg:     return "INFO ";
    case QtWarningMsg:  return "WARN ";
    case QtCriticalMsg: return "CRIT ";
    case QtFatalMsg:    return "FATAL";
    default:            return "UNKN ";
    }
}

/// @brief 自定义消息处理器核心回调。
///
/// 每次调用 qDebug/qInfo/qWarning/qCritical/qFatal 都会经过此函数。
/// 格式：[时间戳][线程ID][级别][类别] 位置 - 消息
///
/// @param[in] type    消息级别。
/// @param[in] context 消息上下文（文件名、行号、函数名、日志类别）。
/// @param[in] msg     实际的日志消息内容。
static void customMessageHandler(QtMsgType type,
                                  const QMessageLogContext& context,
                                  const QString& msg)
{
    // 获取高精度时间戳（毫秒级）
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");

    // 多线程环境下线程 ID 帮助区分不同线程的日志输出
    QString threadId = QString::number(quintptr(QThread::currentThreadId()), 16);

    // context.category 是 Q_LOGGING_CATEGORY 定义的类别名
    QString category = context.category ? context.category : "default";

    // context.file, context.line 在 Debug 模式下自动填充
    // Release 模式需要编译时定义 QT_MESSAGELOGCONTEXT
    QString location;
    if (context.file && context.line > 0) {
        // 只取文件名部分，不显示完整路径
        QString filename = QString(context.file);
        int lastSlash = filename.lastIndexOf('/');
        if (lastSlash >= 0) {
            filename = filename.mid(lastSlash + 1);
        }
        location = QString("%1:%2").arg(filename).arg(context.line);
    } else {
        location = "<unknown>";
    }

    // 构造格式化的日志消息
    QString formattedMsg = QString("[%1][%2][%3][%4] %5 - %6")
                               .arg(timestamp)
                               .arg(threadId)
                               .arg(msgTypeToString(type))
                               .arg(category)
                               .arg(location)
                               .arg(msg);

    // Debug/Info 输出到 stdout，Warning/Critical/Fatal 输出到 stderr
    FILE* outputStream = (type >= QtWarningMsg) ? stderr : stdout;
    fprintf(outputStream, "%s\n", formattedMsg.toUtf8().constData());
    fflush(outputStream);

    // 多线程环境下需要互斥锁保护日志文件写入
    if (g_logFile && g_logFile->isOpen()) {
        QMutexLocker locker(&g_logMutex);    // RAII 自动加锁/解锁
        QTextStream stream(g_logFile);
        stream << formattedMsg << "\n";
        stream.flush();    // 立即刷新，确保崩溃时不丢失日志
    }

    // Qt 规定：qFatal() 会终止程序
    if (type == QtFatalMsg) {
        abort();
    }
}

/// @brief 安装自定义处理器并初始化日志文件。
/// @param[in] logFilePath 日志文件路径，为空则仅输出到控制台。
static void installCustomHandler(const QString& logFilePath = {})
{
    if (!logFilePath.isEmpty()) {
        g_logFile = new QFile(logFilePath);
        if (g_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            qDebug() << "日志文件已打开:" << logFilePath;
        } else {
            qWarning() << "无法打开日志文件:" << logFilePath
                       << "- 错误:" << g_logFile->errorString();
            delete g_logFile;
            g_logFile = nullptr;
        }
    }

    // qInstallMessageHandler 返回旧的处理函数指针
    g_oldHandler = qInstallMessageHandler(customMessageHandler);
}

/// @brief 恢复默认处理器。
///
/// 在单元测试中，可能需要在不同测试间切换处理器。
static void restoreDefaultHandler()
{
    if (g_logFile) {
        g_logFile->close();
        delete g_logFile;
        g_logFile = nullptr;
    }

    // 传入 nullptr 会恢复为 Qt 默认处理器
    qInstallMessageHandler(g_oldHandler);
    g_oldHandler = nullptr;
}

void demoCustomHandler()
{
    qDebug() << "  [安装前] 这条消息使用 Qt 默认格式输出";

    // 安装自定义处理器（同时写入文件）
    installCustomHandler("advanced_demo.log");

    qDebug() << "  [安装后] 这条消息使用自定义格式";
    qDebug() << "  格式包含：时间戳、线程 ID、级别、类别、文件名:行号";

    qInfo() << "  这是一条信息级别日志";
    qWarning() << "  这是一条警告级别日志";
    qCritical() << "  这是一条严重错误级别日志";

    qDebug() << "  [级别演示] 不同级别的日志会输出到不同的流";
    qDebug() << "    Debug/Info -> stdout";
    qDebug() << "    Warning/Critical/Fatal -> stderr";

    restoreDefaultHandler();

    qDebug() << "  [恢复后] 这条消息恢复为 Qt 默认格式";
}
