// QProcess 入门示例 - 演示启动外部进程、读取输出、异步处理
// 本示例展示 QProcess 的核心用法：启动命令、读取 stdout/stderr、异步等待

#include <QCoreApplication>         // 应用程序核心类，非 GUI 程序用这个代替 QApplication
#include <QProcess>                 // 进程管理类，用于启动外部程序并通信
#include <QDebug>                   // 调试输出流
#include <QByteArray>               // 字节数组类，用于存储进程输出
#include <QString>                  // Qt 字符串类
#include <QTimer>                   // 定时器，用于演示异步处理

// 示例 1：同步启动进程并读取输出（阻塞方式）
void example1_SyncProcess()
{
    qDebug() << "=== 示例 1：同步启动进程 ===";

    QProcess process;                // 创建进程对象
    process.start("echo", QStringList() << "Hello from QProcess!");  // 启动命令

    // 等待进程结束（最多等待 3 秒）
    if (process.waitForFinished(3000)) {
        // 读取标准输出
        QByteArray stdoutData = process.readAllStandardOutput();
        QString output = QString::fromUtf8(stdoutData).trimmed();
        qDebug() << "输出:" << output;

        // 读取标准错误输出（如果有）
        QByteArray stderrData = process.readAllStandardError();
        if (!stderrData.isEmpty()) {
            qDebug() << "错误:" << stderrData;
        }

        // 获取退出码
        int exitCode = process.exitCode();
        qDebug() << "退出码:" << exitCode;
    } else {
        qDebug() << "进程执行超时或启动失败";
    }
}

// 示例 2：异步启动进程并实时读取输出（非阻塞方式）
void example2_AsyncProcess()
{
    qDebug() << "\n=== 示例 2：异步启动进程 ===";

    QProcess *process = new QProcess;  // 使用指针，因为需要异步访问

    // 连接信号：当有标准输出可读时触发
    QObject::connect(process, &QProcess::readyReadStandardOutput, [=]() {
        QByteArray data = process->readAllStandardOutput();
        qDebug() << "实时输出:" << QString::fromUtf8(data).trimmed();
    });

    // 连接信号：当进程结束时触发
    QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                    [=](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::NormalExit) {
            qDebug() << "进程正常结束，退出码:" << exitCode;
        } else {
            qDebug() << "进程崩溃";
        }
        process->deleteLater();  // 异步删除对象
    });

    // 连接信号：当进程启动失败时触发
    QObject::connect(process, &QProcess::errorOccurred, [=](QProcess::ProcessError error) {
        qDebug() << "进程错误:" << error << process->errorString();
    });

    // 启动进程（执行 sleep 命令模拟长时间运行）
    // Windows 下用 timeout，Linux/Mac 下用 sleep
#ifdef Q_OS_WIN
    process->start("timeout", QStringList() << "/t" << "2");
#else
    process->start("sleep", QStringList() << "2");
#endif
}

// 示例 3：分离式启动进程（独立运行）
void example3_DetachedProcess()
{
    qDebug() << "\n=== 示例 3：分离式启动进程 ===";

    // startDetached 启动的进程独立于父进程运行
    // 这里以启动记事本/文本编辑器为例
    QString program;
#ifdef Q_OS_WIN
    program = "notepad.exe";  // Windows 记事本
#elif defined(Q_OS_MACOS)
    program = "open";  // macOS open 命令
#else
    program = "xdg-open";  // Linux xdg-open 命令
#endif

    QStringList arguments;
#ifndef Q_OS_WIN
    arguments << "/tmp";  // 打开临时目录
#endif

    bool success = QProcess::startDetached(program, arguments);
    if (success) {
        qDebug() << "已独立启动程序:" << program;
    } else {
        qDebug() << "启动失败";
    }
}

// 示例 4：设置工作目录和环境变量
void example4_ProcessEnvironment()
{
    qDebug() << "\n=== 示例 4：设置工作目录和环境变量 ===";

    QProcess process;

    // 设置工作目录
    process.setWorkingDirectory("/tmp");

    // 获取系统环境变量并修改
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("MY_CUSTOM_VAR", "Hello from Qt!");
    process.setProcessEnvironment(env);

    // 在 Linux/Mac 上打印环境变量
#ifdef Q_OS_UNIX
    process.start("sh", QStringList() << "-c" << "echo $MY_CUSTOM_VAR");
    process.waitForFinished();
    qDebug() << "环境变量输出:" << QString::fromUtf8(process.readAllStandardOutput()).trimmed();
#else
    qDebug() << "此示例在 Linux/Mac 上运行效果最佳";
#endif
}

// 示例 5：通过管道传递数据给进程
void example5_WriteToProcess()
{
    qDebug() << "\n=== 示例 5：向进程写入数据 ===";

    QProcess process;

    // 启动一个会读取 stdin 的命令
#ifdef Q_OS_WIN
    process.start("findstr", QStringList() << "Hello");
#else
    process.start("grep", QStringList() << "Hello");
#endif

    // 向进程的标准输入写入数据
    process.write("Hello World\n");
    process.write("Goodbye World\n");
    process.closeWriteChannel();  // 关闭写入通道，表示输入结束

    process.waitForFinished();
    qDebug() << "过滤结果:" << QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);  // 创建控制台应用程序

    // 运行同步示例（阻塞方式）
    example1_SyncProcess();

    // 运行异步示例（非阻塞方式）
    example2_AsyncProcess();

    // 运行分离式启动示例
    example3_DetachedProcess();

    // 运行环境变量示例
    example4_ProcessEnvironment();

    // 运行管道示例
    example5_WriteToProcess();

    // 设置定时器，3 秒后退出（给异步示例足够时间完成）
    QTimer::singleShot(3000, &app, &QCoreApplication::quit);

    // 进入事件循环，处理异步信号
    return app.exec();
}

/*
 * 编译与运行说明：
 *
 * 1. 创建构建目录：
 *    mkdir build && cd build
 *
 * 2. 配置 CMake（请替换你的 Qt 路径）：
 *    cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt/6.9.1/gcc_64
 *
 * 3. 编译：
 *    cmake --build .
 *
 * 4. 运行：
 *    ./10-qprocess-beginner
 *
 * 预期输出：
 * - 同步执行 echo 命令并打印输出
 * - 异步执行 sleep/timeout 命令，2 秒后打印结束信息
 * - 启动一个记事本/文本编辑器窗口（独立运行）
 * - 打印自定义环境变量的值
 * - 通过 grep/findstr 过滤包含 "Hello" 的行
 */
