/// @file    pipe_demo.cpp
/// @brief   实现 QProcess 管道连接、崩溃检测与超时管理的演示函数。
///
/// 对应教程：进阶层 01-QtBase/10-QProcess 高级用法。

#include "pipe_demo.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QProcess>
#include <QTimer>

void demoProcessPipe()
{
    qDebug() << "\n--- 进程管道演示（类似 ls | grep） ---";

    QProcess grepProcess;
    QProcess lsProcess;

    // 关键操作：将 ls 的 stdout 连接到 grep 的 stdin
    // 这使得 ls 的输出自动成为 grep 的输入，无需手动中转
    lsProcess.setStandardOutputProcess(&grepProcess);

    // grep 进程：筛选包含特定关键字的行
    grepProcess.start("grep", QStringList() << "-E" << "\\.txt$");
    // ls 进程：列出 /tmp 目录内容
    lsProcess.start("ls", QStringList() << "/tmp");

    // 等待两个进程都完成
    grepProcess.waitForFinished();
    lsProcess.waitForFinished();

    // 读取 grep 的最终输出（经过管道过滤后的结果）
    QString output = QString::fromUtf8(grepProcess.readAllStandardOutput());
    if (!output.isEmpty()) {
        qDebug() << "[管道] ls | grep '\\.txt$' 结果:";
        qDebug().noquote() << output;
    } else {
        qDebug() << "[管道] 没有匹配 .txt 的文件（/tmp 目录下）";
    }

    qDebug() << "[管道] ls 退出码:" << lsProcess.exitCode();
    qDebug() << "[管道] grep 退出码:" << grepProcess.exitCode();
}

void demoCrashDetection()
{
    qDebug() << "\n--- 进程崩溃检测演示 ---";

    // 测试 1：正常运行并退出的进程
    {
        QProcess process;
        process.start("echo", QStringList() << "正常执行");
        process.waitForFinished();

        qDebug() << "[崩溃检测] echo 进程:"
                 << "退出码:" << process.exitCode()
                 << "状态:"
                 << (process.exitStatus() == QProcess::NormalExit ? "正常" : "崩溃");
    }

    // 测试 2：用错误参数导致进程失败
    {
        QProcess process;
        // ls 一个不存在的路径会返回非零退出码（不是崩溃，是正常退出但报告错误）
        process.start("ls", QStringList() << "/nonexistent_path_12345");
        process.waitForFinished();

        qDebug() << "[崩溃检测] ls 不存在路径:"
                 << "退出码:" << process.exitCode()
                 << "状态:"
                 << (process.exitStatus() == QProcess::NormalExit ? "正常退出" : "崩溃");
        // 注意：非零退出码不等于崩溃，进程是正常退出的
        QString stderrOutput =
            QString::fromUtf8(process.readAllStandardError()).trimmed();
        if (!stderrOutput.isEmpty()) {
            qDebug() << "[崩溃检测] stderr:" << stderrOutput;
        }
    }

    // 测试 3：启动不存在的程序（FailedToStart 错误）
    {
        QProcess process;

        // 使用信号捕获错误
        QObject::connect(&process, &QProcess::errorOccurred,
            [](QProcess::ProcessError error) {
                qDebug() << "[崩溃检测] errorOccurred 信号，错误码:" << error;
                switch (error) {
                case QProcess::FailedToStart:
                    qDebug() << "[崩溃检测] -> 进程启动失败（程序不存在或无权限）";
                    break;
                case QProcess::Crashed:
                    qDebug() << "[崩溃检测] -> 进程崩溃";
                    break;
                default:
                    qDebug() << "[崩溃检测] -> 其他错误";
                    break;
                }
            });

        // 尝试启动一个不存在的程序
        process.start("nonexistent_program_xyz", QStringList());
        process.waitForFinished();

        qDebug() << "[崩溃检测] 不存在程序:"
                 << "错误信息:" << process.errorString();
    }

    // 测试 4：使用 sleep 模拟一个被 kill 的进程
    {
        QProcess process;
        process.start("sleep", QStringList() << "60");

        if (process.waitForStarted()) {
            qDebug() << "[崩溃检测] sleep 进程已启动 (PID:"
                     << process.processId() << ")";

            // 立即终止它
            process.kill();
            process.waitForFinished();

            qDebug() << "[崩溃检测] 被 kill 的 sleep:"
                     << "退出码:" << process.exitCode()
                     << "状态:"
                     << (process.exitStatus() == QProcess::NormalExit
                             ? "正常"
                             : "被信号终止");
        }
    }
}

void demoTimeoutManagement()
{
    qDebug() << "\n--- 进程超时管理演示 ---";

    // 测试 1：正常进程，在超时前完成
    {
        qDebug() << "[超时] 测试 1：快速完成的进程";
        QProcess process;
        QElapsedTimer timer;
        timer.start();

        process.start("echo", QStringList() << "快速完成");
        process.waitForFinished();

        qint64 elapsed = timer.elapsed();
        qDebug() << "[超时] 进程在" << elapsed
                 << "ms 内完成（无需超时干预）";
        qDebug() << "[超时] 输出:"
                 << QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    }

    // 测试 2：长时间进程 + 超时终止
    {
        qDebug() << "\n[超时] 测试 2：长时间进程 + 超时终止";
        QProcess process;
        QElapsedTimer timer;
        timer.start();

        process.start("sleep", QStringList() << "30");

        if (process.waitForStarted()) {
            qDebug() << "[超时] sleep 进程已启动，设置 1 秒超时";

            // 阶段一：发送 SIGTERM（礼貌终止）
            QTimer::singleShot(1000, &process, [&]() {
                qDebug() << "[超时] 1 秒超时，发送 terminate()...";
                process.terminate();
            });

            // 阶段二：如果 3 秒后仍未退出，强制 kill
            QTimer::singleShot(4000, &process, [&]() {
                if (process.state() != QProcess::NotRunning) {
                    qDebug() << "[超时] terminate 后进程仍在运行，发送 kill()...";
                    process.kill();
                }
            });

            QEventLoop loop;
            QObject::connect(
                &process,
                QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                &loop,
                &QEventLoop::quit);
            loop.exec();

            qDebug() << "[超时] 进程已终止，总耗时:" << timer.elapsed()
                     << "ms"
                     << "状态:"
                     << (process.exitStatus() == QProcess::NormalExit ? "正常"
                                                                     : "被信号终止");
        } else {
            qDebug() << "[超时] 进程启动失败:" << process.errorString();
        }
    }

    // 测试 3：事件循环中的超时管理（更实用的模式）
    {
        qDebug() << "\n[超时] 测试 3：事件循环超时管理模式";
        QProcess process;
        process.start("cat", QStringList() << "/etc/hostname");

        // 使用 waitForFinished 带超时参数
        bool finished = process.waitForFinished(5000);
        if (finished) {
            qDebug() << "[超时] 进程在超时前完成";
            qDebug() << "[超时] 输出:"
                     << QString::fromUtf8(process.readAllStandardOutput())
                            .trimmed();
        } else {
            qDebug() << "[超时] 进程超时，执行两阶段终止";
            process.terminate();
            if (!process.waitForFinished(3000)) {
                process.kill();
                process.waitForFinished();
            }
            qDebug() << "[超时] 进程已被终止";
        }
    }
}
