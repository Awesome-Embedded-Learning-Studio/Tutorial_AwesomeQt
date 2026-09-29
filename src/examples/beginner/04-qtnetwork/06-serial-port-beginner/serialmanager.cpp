#include "serialmanager.h"

#include <QDebug>

SerialManager::SerialManager(QObject *parent)
    : QObject(parent),
      m_serial(new QSerialPort(this)),
      m_totalBytesSent(0),
      m_totalBytesReceived(0)
{
    // 异步读取：有新数据到达时处理
    connect(m_serial, &QSerialPort::readyRead, this, [this]() {
        QByteArray newData = m_serial->readAll();
        m_totalBytesReceived += newData.size();
        m_rxBuffer.append(newData);

        qDebug() << "  [RX]" << newData.size() << "bytes:"
                 << newData.toHex(' ');

        // 尝试解析完整的帧
        parseFrames();
    });

    // 错误处理
    connect(m_serial, &QSerialPort::errorOccurred, this,
            [this](QSerialPort::SerialPortError error) {
        if (error == QSerialPort::NoError) {
            return;
        }

        qDebug() << "  [Error]" << error
                 << m_serial->errorString();

        // 设备意外断开
        if (error == QSerialPort::ResourceError) {
            qDebug() << "  Device disconnected unexpectedly!";
            m_serial->close();
        }
    });

    // 发送完成通知
    connect(m_serial, &QSerialPort::bytesWritten, this,
            [](qint64 bytes) {
        qDebug() << "  [TX Complete]" << bytes << "bytes sent";
    });
}

bool SerialManager::openPort(const QString &portName,
                              qint32 baudRate)
{
    m_serial->setPortName(portName);
    m_serial->setBaudRate(baudRate);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial->open(QIODevice::ReadWrite)) {
        qDebug() << "  Failed to open" << portName
                 << ":" << m_serial->errorString();
        return false;
    }

    qDebug() << "  Port" << portName << "opened at"
             << baudRate << "baud (8N1)";
    return true;
}

qint64 SerialManager::sendData(const QByteArray &data)
{
    if (!m_serial->isOpen()) {
        qDebug() << "  Port not open, cannot send.";
        return -1;
    }

    qint64 written = m_serial->write(data);
    if (written > 0) {
        m_totalBytesSent += written;
        qDebug() << "  [TX]" << written << "bytes:"
                 << data.toHex(' ');
    }
    return written;
}

void SerialManager::closePort()
{
    if (m_serial->isOpen()) {
        m_serial->close();
        qDebug() << "  Port closed.";
    }
}

void SerialManager::printStats() const
{
    qDebug() << "\n  --- Serial Statistics ---";
    qDebug() << "  Bytes sent:" << m_totalBytesSent;
    qDebug() << "  Bytes received:" << m_totalBytesReceived;
    qDebug() << "  RX buffer remaining:" << m_rxBuffer.size();
}

void SerialManager::parseFrames()
{
    while (m_rxBuffer.size() >= 4) {
        // 查找帧头 0xAA 0x55
        if (static_cast<quint8>(m_rxBuffer[0]) != 0xAA
            || static_cast<quint8>(m_rxBuffer[1]) != 0x55) {
            m_rxBuffer.remove(0, 1);
            continue;
        }

        quint8 dataLen = static_cast<quint8>(m_rxBuffer[2]);
        int frameLen = 2 + 1 + dataLen + 1;

        if (m_rxBuffer.size() < frameLen) {
            break;  // 数据不完整，等待更多数据
        }

        // 提取完整帧
        QByteArray frame = m_rxBuffer.left(frameLen);
        m_rxBuffer.remove(0, frameLen);

        // XOR 校验：对数据区域逐字节异或
        quint8 checksum = 0;
        for (int i = 3; i < frameLen - 1; ++i) {
            checksum ^= static_cast<quint8>(frame[i]);
        }

        quint8 receivedChecksum =
            static_cast<quint8>(frame[frameLen - 1]);

        if (checksum == receivedChecksum) {
            QByteArray payload = frame.mid(3, dataLen);
            qDebug() << "  [Frame OK] Payload:"
                     << payload.toHex(' ')
                     << "(len:" << dataLen << ")";
        } else {
            qDebug() << "  [Frame ERROR] Checksum mismatch:"
                     << "expected" << Qt::hex << checksum
                     << "got" << receivedChecksum << Qt::dec;
        }
    }
}
