/**
 * Qt SerialPort 基础示例
 *
 * 本示例演示串口通信的核心操作：
 * 1. QSerialPortInfo::availablePorts() 枚举系统串口
 * 2. QSerialPort 配置波特率/数据位/校验位/停止位
 * 3. open(ReadWrite) 打开串口
 * 4. readyRead 信号异步读取 + write 发送数据
 * 5. 错误处理与串口状态监控
 *
 * 核心要点：
 * - QtSerialPort 是独立模块，需 find_package(Qt6 SerialPort)
 * - Linux 需要当前用户在 dialout 组才能访问串口
 * - readyRead 不保证一次触发对应一个完整帧，需应用层做帧解析
 * - 串口同一时间只能被一个进程打开
 */

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

#include "serialportlist.h"
#include "serialportconfigdemo.h"
#include "serialmanager.h"
#include "frameparsingdemo.h"

// ========================================
// 主函数
// ========================================

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== Qt SerialPort Basic Example ===";

    // 示例 1: 列出系统可用串口
    listAvailablePorts();

    // 示例 2: 串口配置演示
    demoSerialConfiguration();

    // 示例 3: 帧解析模拟（不需要硬件）
    demoFrameParsing();

    // 延迟退出（如果打开了真实串口，给异步操作时间完成）
    QTimer::singleShot(1000, [&app]() {
        qDebug() << "\n=== Summary ===";
        qDebug() << "QtSerialPort provides async serial communication";
        qDebug() << "via the same readyRead/write pattern as QTcpSocket.";
        qDebug() << "Always implement frame parsing in your application layer.";
        qDebug() << "Demo finished.";
        QCoreApplication::quit();
    });

    return app.exec();
}
