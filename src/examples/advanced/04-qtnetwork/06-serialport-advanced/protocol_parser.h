/// @file    protocol_parser.h
/// @brief   Binary protocol frame parser using a finite state machine.
///
/// @details Demonstrates robust serial protocol parsing for Qt Network
///          advanced tutorial (04-qtnetwork/06-serialport-advanced).
///          Frame format: [0xAA][0x55][cmd 1B][len 2B BE][payload lenB][checksum 1B]

#pragma once

#include <QByteArray>
#include <QObject>

/// @brief Parses a custom binary protocol from a serial byte stream.
///
/// Uses a state machine to handle partial reads, burst data, and
/// frame integrity verification via XOR checksum.
class ProtocolParser : public QObject
{
    Q_OBJECT

public:
    /// @brief Parser state machine states.
    /// @note Using kPascalCase to distinguish protocol constants from Qt enums.
    enum class State
    {
        kWaitHeader1,   ///< Waiting for first header byte (0xAA)
        kWaitHeader2,   ///< Waiting for second header byte (0x55)
        kWaitCmd,       ///< Waiting for command byte
        kWaitLen1,      ///< Waiting for length high byte (big-endian)
        kWaitLen2,      ///< Waiting for length low byte (big-endian)
        kWaitPayload,   ///< Collecting payload bytes
        kWaitChecksum   ///< Waiting for checksum byte
    };

    /// @brief Error codes emitted when protocol parsing fails.
    enum class ErrorCode
    {
        kChecksumMismatch,  ///< Computed checksum does not match received
        kFrameTooLong,      ///< Payload length exceeds maximum allowed
        kUnexpectedByte     ///< Unexpected byte in header sequence
    };

    /// @brief Maximum allowed payload size to prevent memory exhaustion.
    /// @note A 1024-byte limit is typical for embedded serial protocols.
    static constexpr int kMaxPayloadSize = 1024;

    /// @brief First header byte of the protocol frame.
    static constexpr quint8 kHeaderByte1 = 0xAA;

    /// @brief Second header byte of the protocol frame.
    static constexpr quint8 kHeaderByte2 = 0x55;

    /// @brief Construct a parser with a fresh state machine.
    /// @param[in] parent  Parent QObject for ownership management.
    explicit ProtocolParser(QObject* parent = nullptr);

    /// @brief Feed received serial data into the state machine.
    /// @param[in] data  Raw bytes from the serial port (may be partial).
    /// @note Processes byte by byte so partial reads are handled correctly.
    void feed(const QByteArray& data);

    /// @brief Reset the parser to its initial waiting state.
    /// @note Call this after a port error or disconnect to avoid stale state.
    void reset();

    /// @brief Build a complete protocol frame ready for transmission.
    /// @param[in] cmd      Command byte identifying the message type.
    /// @param[in] payload  Payload data to include in the frame.
    /// @return A QByteArray containing the complete frame with header,
    ///         length, payload, and checksum.
    /// @note Static so it can be used without a parser instance.
    static QByteArray buildFrame(quint8 cmd, const QByteArray& payload);

signals:
    /// @brief Emitted when a complete, verified frame has been parsed.
    /// @param command  The command byte from the frame.
    /// @param payload  The payload bytes (checksum already verified).
    void frameReady(quint8 command, const QByteArray& payload);

    /// @brief Emitted when a protocol error is detected.
    /// @param error  The specific error that occurred.
    void error(ErrorCode error);

private:
    /// @brief Process a single byte through the state machine.
    /// @param[in] byte  One raw byte from the serial stream.
    void processByte(quint8 byte);

    /// @brief Compute XOR checksum over command + length + payload.
    /// @param[in] cmd      Command byte.
    /// @param[in] length   Payload length (big-endian pair).
    /// @param[in] payload  Payload bytes.
    /// @return The XOR checksum byte.
    static quint8 computeChecksum(quint8 cmd, quint16 length,
                                  const QByteArray& payload);

    State m_state = State::kWaitHeader1;  ///< Current state machine position
    quint8 m_cmd = 0;                     ///< Command byte being assembled
    quint16 m_payloadLen = 0;             ///< Expected payload length
    QByteArray m_payload;                 ///< Accumulated payload bytes
    quint8 m_checksum = 0;                ///< Running checksum accumulator
};
