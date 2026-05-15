/// @file    async_reader.h
/// @brief   定义 AsyncProcessReader 类，演示 QProcess 的异步信号驱动读取。
///
/// 对应教程：进阶层 01-QtBase/10-QProcess 高级用法。
/// 与同步的 waitForFinished() 不同，异步模式通过 Qt 信号机制
/// 在数据到达时实时处理，适合长时间运行的进程（如日志监控、
/// 实时数据处理）。

#pragma once

#include <QObject>
#include <QProcess>

/// @brief 异步进程输出读取器。
///
/// 封装 QProcess 的异步信号驱动读取模式。将 readyReadStandardOutput
/// 信号连接到槽函数，实现流式数据处理。这种模式不会阻塞事件循环，
/// 可以同时管理多个进程。
class AsyncProcessReader : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化信号槽连接和计数器。
    /// @param[in] parent 父对象指针，Qt 对象树自动管理生命周期。
    /// @note 必须在构造函数中完成信号槽连接，否则首次触发可能丢失。
    explicit AsyncProcessReader(QObject* parent = nullptr);

    /// @brief 启动进程并异步读取输出。
    /// @param[in] program 要执行的程序路径。
    /// @param[in] arguments 参数列表，默认为空。
    /// @note 设置 MergedChannels 将 stdout 和 stderr 合并到同一个通道，
    ///       这样只需要监听 readyReadStandardOutput 就能获取所有输出。
    void start(const QString& program, const QStringList& arguments = {});

    /// @brief 启动进程并通过局部事件循环等待完成（带超时）。
    /// @param[in] program  要执行的程序路径。
    /// @param[in] arguments 参数列表，默认为空。
    /// @param[in] timeoutMs 超时毫秒数，默认 10000ms。
    /// @return true 表示进程正常退出且退出码为 0。
    /// @note 使用局部事件循环等待进程完成，同时保持信号驱动。
    bool startAndWait(const QString& program,
                      const QStringList& arguments = {},
                      int timeoutMs = 10000);

    /// @brief 获取累计读取的行数。
    /// @return 已读取的总行数。
    int lineCount() const;

    /// @brief 获取所有已读取的输出。
    /// @return 包含全部已读取文本的字符串。
    QString allOutput() const;

private slots:
    /// @brief stdout 数据就绪回调。
    /// @note 每次 readyReadStandardOutput 信号触发时调用。
    ///       逐行读取适合日志类输出，一次读取适合二进制数据。
    void onReadyRead();

    /// @brief stderr 数据就绪回调。
    void onReadyReadError();

    /// @brief 进程结束回调。
    /// @param[in] exitCode   进程退出码。
    /// @param[in] exitStatus 退出状态（NormalExit / CrashExit）。
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

    /// @brief 错误回调。
    /// @param[in] error 进程错误类型。
    void onError(QProcess::ProcessError error);

private:
    QProcess m_process;       ///< QProcess 实例
    int m_lineCount;          ///< 累计读取行数
    QString m_outputBuffer;   ///< 输出缓冲区
};
