/// @file    protocol_frame.cpp
/// @brief   Implementation of ProtocolFrame encoder and FrameParser state machine.

#include "protocol_frame.h"

#include <QtGlobal>

// Magic bytes that mark the beginning of every frame.
static constexpr uint8_t kMagic1 = 0xAA;
static constexpr uint8_t kMagic2 = 0x55;

// ---------------------------------------------------------------------------
// ProtocolFrame
// ---------------------------------------------------------------------------

QByteArray ProtocolFrame::encode(const QByteArray& payload)
{
    const uint16_t length = static_cast<uint16_t>(payload.size());

    QByteArray frame;
    // Reserve: 2 magic + 2 length + N payload + 1 checksum
    frame.reserve(2 + 2 + length + 1);

    // Magic header
    frame.append(static_cast<char>(kMagic1));
    frame.append(static_cast<char>(kMagic2));

    // Length field (big-endian)
    frame.append(static_cast<char>((length >> 8) & 0xFF));
    frame.append(static_cast<char>(length & 0xFF));

    // Payload
    frame.append(payload);

    // XOR checksum over all payload bytes
    uint8_t checksum = 0;
    for (int i = 0; i < payload.size(); ++i) {
        checksum ^= static_cast<uint8_t>(payload[i]);
    }
    frame.append(static_cast<char>(checksum));

    return frame;
}

bool ProtocolFrame::verifyChecksum(const QByteArray& payload, uint8_t checksum)
{
    uint8_t computed = 0;
    for (int i = 0; i < payload.size(); ++i) {
        computed ^= static_cast<uint8_t>(payload[i]);
    }
    return computed == checksum;
}

// ---------------------------------------------------------------------------
// FrameParser
// ---------------------------------------------------------------------------

FrameParser::FrameParser(QObject* parent)
    : QObject(parent)
{
}

void FrameParser::reset()
{
    m_state = ParseState::kWaitingMagic1;
    m_payload.clear();
    m_expectedLength = 0;
    m_bytesReceived = 0;
    m_lengthHighByte = 0;
}

void FrameParser::parse(const QByteArray& data)
{
    for (int i = 0; i < data.size(); ++i) {
        processByte(static_cast<uint8_t>(data[i]));
    }
}

void FrameParser::processByte(uint8_t byte)
{
    switch (m_state) {
    case ParseState::kWaitingMagic1:
        if (byte == kMagic1) {
            m_state = ParseState::kWaitingMagic2;
        }
        // @note If not 0xAA we stay in this state — TCP stream may have noise
        //       or we may be resynchronising after a corrupt frame.
        break;

    case ParseState::kWaitingMagic2:
        if (byte == kMagic2) {
            m_state = ParseState::kWaitingLength;
        } else if (byte == kMagic1) {
            // Stay in kWaitingMagic2 — two consecutive 0xAA could mean
            // the second 0xAA is the real start of a new magic sequence.
        } else {
            // False magic — go back to hunting for the first byte.
            m_state = ParseState::kWaitingMagic1;
        }
        break;

    case ParseState::kWaitingLength:
        // First byte of the 2-byte big-endian length.
        m_lengthHighByte = byte;
        m_state = ParseState::kWaitingPayload;
        // @note We reuse kWaitingPayload state but use m_bytesReceived == 0
        //       to detect that we still need the low byte.
        //       Actually, we need a separate sub-step. We handle the second
        //       length byte immediately: if m_bytesReceived == 0 we are still
        //       reading length. This is handled below.
        {
            // We treat this state as "received high byte, need low byte".
            // Move to a combined state: we'll store the second byte next.
            // Using m_bytesReceived as a flag: set to 0xFFFF sentinel.
        }
        // Set a sentinel so processByte knows we need the low byte next.
        // We abuse the state machine slightly: after high byte we need one
        // more byte for the low byte before actual payload collection.
        // Store high byte, then transition to a pseudo-state.
        // For simplicity we handle the second length byte right here via a
        // flag encoded in m_bytesReceived.
        m_bytesReceived = 0xFFFF;  // Sentinel: waiting for low byte
        break;

    case ParseState::kWaitingPayload:
        if (m_bytesReceived == 0xFFFF) {
            // This is the low byte of the length field.
            m_expectedLength = (static_cast<uint16_t>(m_lengthHighByte) << 8) | byte;
            m_bytesReceived = 0;
            m_payload.clear();
            m_payload.reserve(m_expectedLength);

            if (m_expectedLength == 0) {
                // Zero-length payload — skip straight to checksum.
                m_state = ParseState::kWaitingChecksum;
            }
            // Otherwise stay in kWaitingPayload to collect payload bytes.
        } else {
            // Normal payload byte accumulation.
            m_payload.append(static_cast<char>(byte));
            ++m_bytesReceived;

            if (m_bytesReceived >= m_expectedLength) {
                m_state = ParseState::kWaitingChecksum;
            }
        }
        break;

    case ParseState::kWaitingChecksum:
        if (ProtocolFrame::verifyChecksum(m_payload, byte)) {
            emit frameReady(m_payload);
        }
        // @note On checksum failure we silently discard the frame and
        //       resume hunting — the sender must implement retransmission
        //       at the application layer if reliability is required.
        reset();
        break;
    }
}
