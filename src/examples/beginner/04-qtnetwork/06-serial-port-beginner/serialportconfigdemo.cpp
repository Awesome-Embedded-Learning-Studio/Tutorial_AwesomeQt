#include "serialportconfigdemo.h"

#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>

void demoSerialConfiguration()
{
    qDebug() << "=== Demo 2: Serial Port Configuration ===";

    // 尝试找到第一个可用的串口设备
    const QList<QSerialPortInfo> ports =
        QSerialPortInfo::availablePorts();

    if (ports.isEmpty()) {
        qDebug() << "  No serial ports available for configuration demo.";
        qDebug() << "  Showing configuration API usage instead:\n";

        // 即使没有真实设备，也展示配置 API 的用法
        QSerialPort serial;
        serial.setPortName("COM3");  // 示例端口名
        serial.setBaudRate(QSerialPort::Baud115200);
        serial.setDataBits(QSerialPort::Data8);
        serial.setParity(QSerialPort::NoParity);
        serial.setStopBits(QSerialPort::OneStop);
        serial.setFlowControl(QSerialPort::NoFlowControl);

        qDebug() << "  Configured port:" << serial.portName();
        qDebug() << "  Baud rate:" << serial.baudRate();
        qDebug() << "  Data bits:" << serial.dataBits();
        qDebug() << "  Parity:" << serial.parity();
        qDebug() << "  Stop bits:" << serial.stopBits();
        qDebug() << "  Flow control:" << serial.flowControl();
        return;
    }

    // 使用第一个可用端口进行配置演示
    QSerialPortInfo firstPort = ports.first();
    qDebug() << "  Using port:" << firstPort.portName();

    QSerialPort serial;
    serial.setPort(firstPort);

    // 配置通信参数（标准 115200 8N1）
    serial.setBaudRate(QSerialPort::Baud115200);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    // 尝试打开串口
    if (serial.open(QIODevice::ReadWrite)) {
        qDebug() << "  Port opened successfully!";
        qDebug() << "  Baud rate:" << serial.baudRate();
        qDebug() << "  Data bits:" << serial.dataBits();
        qDebug() << "  Parity:" << serial.parity();
        qDebug() << "  Stop bits:" << serial.stopBits();
        serial.close();
    } else {
        qDebug() << "  Failed to open:" << serial.errorString();
        qDebug() << "  Possible causes:";
        qDebug() << "    - Port is in use by another program";
        qDebug() << "    - Permission denied (Linux: add user to dialout)";
        qDebug() << "    - Device disconnected";
    }
}
