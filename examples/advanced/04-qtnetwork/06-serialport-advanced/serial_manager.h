/// @file    serial_manager.h
/// @brief   Serial port manager combining QSerialPort with ProtocolParser.
///
/// @details Provides a high-level interface for sending and receiving
///          framed binary messages over a serial port. Tutorial reference:
///          advanced 04-qtnetwork/06-serialport-advanced.

#pragma once

#include <QObject>
#include <QSerialPort>

#include "protocol_parser.h"

/// @brief Manages a serial port connection with framed binary protocol support.
///
/// Encapsulates QSerialPort configuration, frame building, and incoming
/// data parsing. Tracks transmit/receive statistics for diagnostics.
class SerialManager : public QObject
{
    Q_OBJECT

public:
    /// @brief Construct a serial manager with parser and port setup.
    /// @param[in] parent  Parent QObject for ownership management.
    explicit SerialManager(QObject* parent = nullptr);

    /// @brief Destructor ensures the serial port is closed cleanly.
    ~SerialManager() override;

    // Disable copying: QObject-derived classes must not be copied
    SerialManager(const SerialManager&) = delete;
    SerialManager& operator=(const SerialManager&) = delete;

    /// @brief Open a serial port with standard 115200 8N1 configuration.
    /// @param[in] portName  System port name (e.g. "COM3", "/dev/ttyUSB0").
    /// @return true if the port was opened successfully, false otherwise.
    /// @note Hardcodes 115200 baud because this demo uses a fixed protocol;
    ///       production code should make baud rate configurable.
    bool openPort(const QString& portName);

    /// @brief Close the currently open serial port, if any.
    void closePort();

    /// @brief Build a protocol frame and write it to the serial port.
    /// @param[in] cmd      Command byte for the frame.
    /// @param[in] payload  Payload data to include.
    /// @return true if the full frame was written, false on error.
    /// @note Even if the port is closed, this returns false without crashing.
    bool sendFrame(quint8 cmd, const QByteArray& payload);

    /// @brief Get the number of successfully parsed incoming frames.
    /// @return Count of frames received since construction or last reset.
    int framesReceived() const;

    /// @brief Get the number of frames sent via sendFrame().
    /// @return Count of frames sent since construction or last reset.
    int framesSent() const;

    /// @brief Get the number of protocol errors detected.
    /// @return Count of errors (checksum mismatch, frame too long, etc.).
    int errorCount() const;

signals:
    /// @brief Emitted when a complete, verified frame arrives from the port.
    /// @param cmd      Command byte from the received frame.
    /// @param payload  Verified payload bytes.
    void frameReceived(quint8 cmd, const QByteArray& payload);

    /// @brief Emitted when a serial port error occurs.
    /// @param errorMessage  Human-readable error description.
    void portError(const QString& errorMessage);

private slots:
    /// @brief Handle incoming serial data by feeding it to the parser.
    /// @note Connected to QSerialPort::readyRead automatically.
    void handleReadyRead();

    /// @brief Handle QSerialPort error notifications.
    /// @param[in] error  The serial port error that occurred.
    void handleSerialError(QSerialPort::SerialPortError error);

private:
    QSerialPort m_serialPort;          ///< Underlying serial port instance
    ProtocolParser m_parser;           ///< State machine parser for incoming data
    int m_framesSent = 0;              ///< Counter for sent frames
    int m_framesReceived = 0;          ///< Counter for successfully parsed frames
    int m_errorCount = 0;              ///< Counter for protocol and port errors
};
