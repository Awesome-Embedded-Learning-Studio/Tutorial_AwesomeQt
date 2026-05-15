/// @file    main.cpp
/// @brief   QProcess 高级示例入口，依次执行全部演示。
///
/// 对应教程：进阶层 01-QtBase/10-QProcess 高级用法。
/// 演示 QProcess 的高级用法：
/// 1. setProcessChannelMode(MergedChannels) 合并输出
/// 2. readyReadStandardOutput 信号驱动异步读取
/// 3. setStandardOutputProcess() 管道连接（ls | grep）
/// 4. 进程崩溃检测：errorOccurred + exitStatus
/// 5. QTimer 超时终止（terminate -> kill 两阶段）

#include <QCoreApplication>
#include <QDebug>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTimer>

#include "async_reader.h"
#include "pipe_demo.h"

/// @brief 演示 setProcessChannelMode(MergedChannels) 合并输出。
/// @note 默认情况下 stdout 和 stderr 是分离的。使用 MergedChannels
///       可以将两者合并，这在需要统一处理所有输出的场景很有用。
static void demo1_mergedChannels()
{
    qDebug() << "\n[演示 1] setProcessChannelMode(MergedChannels) 合并输出";

    QProcess process;

    // 设置通道模式：合并 stdout 和 stderr
    process.setProcessChannelMode(QProcess::MergedChannels);

    // 执行一个同时输出 stdout 和 stderr 的命令
    process.start("sh",
                  QStringList()
                      << "-c"
                      << "echo 'stdout: 正常输出'; "
                         "echo 'stderr: 错误输出' >&2; "
                         "echo 'stdout: 再次正常输出'");

    process.waitForFinished();

    // 因为合并了通道，所有输出都可以通过 readAllStandardOutput 获取
    QString output = QString::fromUtf8(process.readAllStandardOutput());
    qDebug() << "[合并输出] 合并后的输出:\n" << output;

    // 对比：使用 SeparateChannels（默认模式）
    QProcess process2;
    process2.setProcessChannelMode(QProcess::SeparateChannels);
    process2.start("sh",
                   QStringList() << "-c"
                                 << "echo 'stdout line'; echo 'stderr line' >&2");
    process2.waitForFinished();

    QString stdoutOutput =
        QString::fromUtf8(process2.readAllStandardOutput()).trimmed();
    QString stderrOutput =
        QString::fromUtf8(process2.readAllStandardError()).trimmed();
    qDebug() << "[分离输出] stdout:" << stdoutOutput;
    qDebug() << "[分离输出] stderr:" << stderrOutput;
}

/// @brief 演示 readyReadStandardOutput 信号驱动异步读取。
/// @note 异步模式适合长时间运行的进程，通过信号实时接收数据，
///       不会阻塞事件循环。AsyncProcessReader 封装了这种模式。
static void demo2_asyncRead()
{
    qDebug() << "\n[演示 2] readyReadStandardOutput 信号驱动异步读取";

    AsyncProcessReader reader;

    // 使用异步读取器执行命令
    bool success = reader.startAndWait(
        "sh",
        QStringList() << "-c"
                      << "for i in 1 2 3 4 5; do echo \"异步读取 - 行 $i\"; done",
        5000);

    qDebug() << "[异步读取] 执行结果:" << (success ? "成功" : "失败");
    qDebug() << "[异步读取] 总共读取" << reader.lineCount() << "行";
}

/// @brief 演示管道连接和崩溃检测。
static void demo3_pipeAndCrash()
{
    demoProcessPipe();
    demoCrashDetection();
}

/// @brief 演示 QTimer 超时终止（terminate -> kill 两阶段）。
static void demo4_timeoutManagement()
{
    demoTimeoutManagement();
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "========== QProcess 高级示例 ==========";
    qDebug() << "Qt 版本:" << QT_VERSION_STR;

    // 依次执行所有演示
    demo1_mergedChannels();
    demo2_asyncRead();
    demo3_pipeAndCrash();
    demo4_timeoutManagement();

    qDebug() << "\n========== 所有演示执行完毕 ==========";

    // 使用 QTimer::singleShot 延迟退出，确保所有进程清理完成
    QTimer::singleShot(0, &app, &QCoreApplication::quit);

    return app.exec();
}
