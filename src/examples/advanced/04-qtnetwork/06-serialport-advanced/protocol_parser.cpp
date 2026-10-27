/// @file    protocol_parser.cpp
/// @brief   Implementation of the binary protocol frame parser state machine.
///
/// @details Corresponds to tutorial: advanced 04-qtnetwork/06-serialport-advanced.
///          Each byte drives the state machine forward; corrupted or partial
///          data is handled gracefully without crashing.

#include "protocol_parser.h"

ProtocolParser::ProtocolParser(QObject* parent) : QObject(parent)
{
    // State machine starts in the initial waiting state
    m_state = State::kWaitHeader1;
}

void ProtocolParser::feed(const QByteArray& data)
{
    for (int i = 0; i < data.size(); ++i)
    {
        // static_cast because QByteArray::operator[] returns char; we need unsigned
        auto byte = static_cast<quint8>(data.at(i));
        processByte(byte);
    }
}

void ProtocolParser::reset()
{
    m_state = State::kWaitHeader1;
    m_cmd = 0;
    m_payloadLen = 0;
    m_payload.clear();
    m_checksum = 0;
}

QByteArray ProtocolParser::buildFrame(quint8 cmd, const QByteArray& payload)
{
    QByteArray frame;

    // Header bytes identify the start of a new frame
    frame.append(static_cast<char>(kHeaderByte1));
    frame.append(static_cast<char>(kHeaderByte2));

    // Command byte identifies the message type
    frame.append(static_cast<char>(cmd));

    // Payload length in big-endian: high byte first, then low byte
    auto len = static_cast<quint16>(payload.size());
    frame.append(static_cast<char>((len >> 8) & 0xFF));
    frame.append(static_cast<char>(len & 0xFF));

    // Append the actual payload data
    frame.append(payload);

    // XOR checksum covers cmd + len_hi + len_lo + all payload bytes
    quint8 checksum = computeChecksum(cmd, len, payload);
    frame.append(static_cast<char>(checksum));

    return frame;
}

void ProtocolParser::processByte(quint8 byte)
{
    switch (m_state)
    {
        case State::kWaitHeader1:
            if (byte == kHeaderByte1)
            {
                m_state = State::kWaitHeader2;
            }
            // Silently discard non-header bytes: serial noise between frames
            break;

        case State::kWaitHeader2:
            if (byte == kHeaderByte2)
            {
                m_state = State::kWaitCmd;
            }
            else if (byte == kHeaderByte1)
            {
                // Stay in kWaitHeader2: this byte could be the start of a new
                // frame where 0xAA appears twice in a row
                m_state = State::kWaitHeader2;
            }
            else
            {
                // Second header byte mismatch, restart from scratch
                m_state = State::kWaitHeader1;
            }
            break;

        case State::kWaitCmd:
            m_cmd = byte;
            // Begin checksum accumulation from the command byte
            m_checksum = byte;
            m_state = State::kWaitLen1;
            break;

        case State::kWaitLen1:
            // Big-endian: high byte of payload length
            m_payloadLen = static_cast<quint16>(byte) << 8;
            m_checksum ^= byte;
            m_state = State::kWaitLen2;
            break;

        case State::kWaitLen2:
        {
            // Big-endian: low byte of payload length
            m_payloadLen |= byte;
            m_checksum ^= byte;

            if (m_payloadLen > kMaxPayloadSize)
            {
                emit error(ErrorCode::kFrameTooLong);
                reset();
                return;
            }

            // Zero-length payloads are valid (command-only frames)
            if (m_payloadLen == 0)
            {
                m_state = State::kWaitChecksum;
            }
            else
            {
                m_payload.clear();
                m_payload.reserve(m_payloadLen);
                m_state = State::kWaitPayload;
            }
            break;
        }

        case State::kWaitPayload:
            m_payload.append(static_cast<char>(byte));
            m_checksum ^= byte;

            // Check if we have collected the full payload
            if (m_payload.size() >= m_payloadLen)
            {
                m_state = State::kWaitChecksum;
            }
            break;

        case State::kWaitChecksum:
        {
            quint8 expected = m_checksum;
            if (byte == expected)
            {
                emit frameReady(m_cmd, m_payload);
            }
            else
            {
                emit error(ErrorCode::kChecksumMismatch);
            }
            // Always reset after a complete frame attempt (success or failure)
            reset();
            break;
        }
    }
}

quint8 ProtocolParser::computeChecksum(quint8 cmd, quint16 length,
                                       const QByteArray& payload)
{
    // XOR all bytes from cmd through payload: matches what the state machine
    // accumulates byte-by-byte in m_checksum
    quint8 checksum = cmd;
    checksum ^= static_cast<quint8>((length >> 8) & 0xFF);
    checksum ^= static_cast<quint8>(length & 0xFF);

    for (int i = 0; i < payload.size(); ++i)
    {
        checksum ^= static_cast<quint8>(payload.at(i));
    }

    return checksum;
}
