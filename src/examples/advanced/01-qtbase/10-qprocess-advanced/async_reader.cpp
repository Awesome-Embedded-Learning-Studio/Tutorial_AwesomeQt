/// @file    async_reader.cpp
/// @brief   AsyncProcessReader 类实现，异步信号驱动读取进程输出。
///
/// 对应教程：进阶层 01-QtBase/10-QProcess 高级用法。

#include "async_reader.h"

#include <QDebug>
#include <QEventLoop>
#include <QTimer>

AsyncProcessReader::AsyncProcessReader(QObject* parent)
    : QObject(parent)
    , m_lineCount(0)
{
    // readyReadStandardOutput: stdout 有新数据可读时触发
    connect(&m_process, &QProcess::readyReadStandardOutput,
            this, &AsyncProcessReader::onReadyRead);

    // readyReadStandardError: stderr 有新数据可读时触发
    connect(&m_process, &QProcess::readyReadStandardError,
            this, &AsyncProcessReader::onReadyReadError);

    // finished: 进程正常结束或被终止时触发
    connect(&m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &AsyncProcessReader::onFinished);

    // errorOccurred: 进程启动失败或运行时发生错误
    connect(&m_process, &QProcess::errorOccurred,
            this, &AsyncProcessReader::onError);
}

void AsyncProcessReader::start(const QString& program,
                               const QStringList& arguments)
{
    m_lineCount = 0;

    // 合并 stdout 和 stderr：方便统一处理所有输出
    // 也可以使用 QProcess::SeparateChannels（默认）分别处理
    m_process.setProcessChannelMode(QProcess::MergedChannels);

    qDebug() << "[AsyncReader] 启动进程:" << program << arguments;
    m_process.start(program, arguments);
}

bool AsyncProcessReader::startAndWait(const QString& program,
                                      const QStringList& arguments,
                                      int timeoutMs)
{
    start(program, arguments);

    QEventLoop loop;
    bool success = false;

    connect(&m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            &loop, [&](int exitCode, QProcess::ExitStatus status) {
                success = (status == QProcess::NormalExit && exitCode == 0);
                loop.quit();
            });

    connect(&m_process, &QProcess::errorOccurred, &loop, [&]() {
        success = false;
        loop.quit();
    });

    // 设置超时保护
    QTimer::singleShot(timeoutMs, &loop, [&]() {
        qDebug() << "[AsyncReader] 等待超时，终止进程";
        m_process.terminate();
        // 如果 terminate 无效，3 秒后强制 kill
        QTimer::singleShot(3000, &loop, [&]() {
            m_process.kill();
            loop.quit();
        });
    });

    loop.exec();
    return success;
}

int AsyncProcessReader::lineCount() const
{
    return m_lineCount;
}

QString AsyncProcessReader::allOutput() const
{
    return m_outputBuffer;
}

void AsyncProcessReader::onReadyRead()
{
    // 逐行读取：更适合文本输出的场景
    while (m_process.canReadLine()) {
        QString line = QString::fromUtf8(m_process.readLine()).trimmed();
        if (!line.isEmpty()) {
            m_lineCount++;
            m_outputBuffer += line + "\n";
            qDebug() << "[AsyncReader] 行" << m_lineCount << ":" << line;
        }
    }

    // 也读取不完整的行（缓冲区中不以换行结尾的数据）
    QByteArray remaining = m_process.readAllStandardOutput();
    if (!remaining.isEmpty()) {
        QString text = QString::fromUtf8(remaining).trimmed();
        if (!text.isEmpty()) {
            m_lineCount++;
            m_outputBuffer += text + "\n";
            qDebug() << "[AsyncReader] 尾部数据:" << text;
        }
    }
}

void AsyncProcessReader::onReadyReadError()
{
    QByteArray data = m_process.readAllStandardError();
    qDebug() << "[AsyncReader] stderr:" << QString::fromUtf8(data).trimmed();
}

void AsyncProcessReader::onFinished(int exitCode,
                                    QProcess::ExitStatus exitStatus)
{
    qDebug() << "[AsyncReader] 进程结束"
             << "退出码:" << exitCode
             << "状态:"
             << (exitStatus == QProcess::NormalExit ? "正常退出" : "崩溃")
             << "总行数:" << m_lineCount;
}

void AsyncProcessReader::onError(QProcess::ProcessError error)
{
    QString errorMsg;
    switch (error) {
    case QProcess::FailedToStart: errorMsg = "进程启动失败"; break;
    case QProcess::Crashed:       errorMsg = "进程崩溃"; break;
    case QProcess::Timedout:      errorMsg = "等待超时"; break;
    case QProcess::WriteError:    errorMsg = "写入错误"; break;
    case QProcess::ReadError:     errorMsg = "读取错误"; break;
    default:                      errorMsg = "未知错误"; break;
    }
    qDebug() << "[AsyncReader] 错误:" << errorMsg;
}
