/// @file    serial_manager.cpp
/// @brief   Implementation of the serial port manager with protocol framing.
///
/// @details Corresponds to tutorial: advanced 04-qtnetwork/06-serialport-advanced.
///          Connects QSerialPort's asynchronous readyRead to the state machine
///          parser and provides statistics tracking.

#include "serial_manager.h"

#include <QSerialPortInfo>

SerialManager::SerialManager(QObject* parent)
    : QObject(parent), m_parser(this)
{
    // Wire parser signals to relay frames and count errors
    connect(&m_parser, &ProtocolParser::frameReady, this,
            [this](quint8 cmd, const QByteArray& payload)
            {
                ++m_framesReceived;
                emit frameReceived(cmd, payload);
            });

    connect(&m_parser, &ProtocolParser::error, this,
            [this](ProtocolParser::ErrorCode /*errorCode*/)
            {
                ++m_errorCount;
            });

    // Wire serial port signals
    connect(&m_serialPort, &QSerialPort::readyRead, this,
            &SerialManager::handleReadyRead);

    connect(&m_serialPort, &QSerialPort::errorOccurred, this,
            &SerialManager::handleSerialError);
}

SerialManager::~SerialManager()
{
    closePort();
}

bool SerialManager::openPort(const QString& portName)
{
    if (m_serialPort.isOpen())
    {
        closePort();
    }

    m_serialPort.setPortName(portName);
    m_serialPort.setBaudRate(QSerialPort::Baud115200);
    m_serialPort.setDataBits(QSerialPort::Data8);
    m_serialPort.setParity(QSerialPort::NoParity);
    m_serialPort.setStopBits(QSerialPort::OneStop);
    m_serialPort.setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serialPort.open(QIODevice::ReadWrite))
    {
        emit portError(m_serialPort.errorString());
        return false;
    }

    // Reset parser to clean state for the new connection
    m_parser.reset();
    return true;
}

void SerialManager::closePort()
{
    if (m_serialPort.isOpen())
    {
        m_serialPort.close();
    }
}

bool SerialManager::sendFrame(quint8 cmd, const QByteArray& payload)
{
    if (!m_serialPort.isOpen())
    {
        return false;
    }

    QByteArray frame = ProtocolParser::buildFrame(cmd, payload);
    qint64 written = m_serialPort.write(frame);

    if (written != frame.size())
    {
        return false;
    }

    // Ensure the data is flushed to the hardware buffer immediately
    m_serialPort.flush();
    ++m_framesSent;
    return true;
}

int SerialManager::framesReceived() const
{
    return m_framesReceived;
}

int SerialManager::framesSent() const
{
    return m_framesSent;
}

int SerialManager::errorCount() const
{
    return m_errorCount;
}

void SerialManager::handleReadyRead()
{
    QByteArray data = m_serialPort.readAll();
    m_parser.feed(data);
}

void SerialManager::handleSerialError(QSerialPort::SerialPortError error)
{
    // ResourceError is emitted on normal close, skip it to avoid false alarms
    if (error == QSerialPort::NoError || error == QSerialPort::ResourceError)
    {
        return;
    }

    ++m_errorCount;
    emit portError(m_serialPort.errorString());
}
