/// 串口通信管理器：封装打开、配置、收发、帧解析
#pragma once

#include <QObject>
#include <QSerialPort>
#include <QByteArray>

class SerialManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialManager(QObject *parent = nullptr);

    /// 打开指定串口并配置参数
    bool openPort(const QString &portName,
                  qint32 baudRate = QSerialPort::Baud115200);

    /// 发送数据
    qint64 sendData(const QByteArray &data);

    /// 关闭串口
    void closePort();

    /// 返回统计信息
    void printStats() const;

    /// 返回串口是否已打开
    bool isOpen() const { return m_serial->isOpen(); }

private:
    /// 解析接收缓冲区中的完整帧
    /// 帧格式：0xAA 0x55 + 1字节长度(N) + N字节数据 + 1字节XOR校验
    void parseFrames();

    QSerialPort *m_serial;
    QByteArray m_rxBuffer;
    qint64 m_totalBytesSent;
    qint64 m_totalBytesReceived;
};
