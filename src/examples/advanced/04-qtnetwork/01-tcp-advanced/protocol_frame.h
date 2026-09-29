/// @file    protocol_frame.h
/// @brief   Binary protocol frame encoder/parser with length-prefixed framing.
///
/// @details 对应教程：进阶层 04-QtNetwork/01-TCP 高级。
///          演示基于魔术字节的二进制协议帧封装与状态机解析，
///          适用于 TCP 流式数据中的消息边界划分。

#pragma once

#include <QByteArray>
#include <QObject>

#include <cstdint>

/// @brief Static helper to encode/verify protocol frames.
///
/// Frame layout: [magic 0xAA 0x55][length 2B big-endian][payload NB][checksum 1B XOR]
class ProtocolFrame
{
public:
    /// @brief Encode a payload into a complete framed QByteArray.
    /// @param[in] payload Raw application data to wrap.
    /// @return Complete frame ready to write to the wire.
    static QByteArray encode(const QByteArray& payload);

    /// @brief Verify the XOR checksum of a complete frame buffer.
    /// @param[in] frame    Buffer containing [magic][length][payload][checksum].
    /// @param[in] checksum The expected checksum byte.
    /// @return True if the XOR of all payload bytes matches checksum.
    static bool verifyChecksum(const QByteArray& payload, uint8_t checksum);
};

// ---------------------------------------------------------------------------

/// @brief Parser states for the incremental frame receiver.
enum class ParseState : uint8_t
{
    kWaitingMagic1,   ///< Waiting for first magic byte 0xAA
    kWaitingMagic2,   ///< Waiting for second magic byte 0x55
    kWaitingLength,   ///< Accumulating 2-byte length field (big-endian)
    kWaitingPayload,  ///< Accumulating payload bytes
    kWaitingChecksum  ///< Waiting for final XOR checksum byte
};

// ---------------------------------------------------------------------------

/// @brief Incremental state-machine parser that feeds on raw TCP stream data.
///
/// @note Call parse() with whatever bytes arrive from the socket; the parser
///       buffers internally and emits frameReady() only when a complete,
///       checksum-verified frame has been assembled.
class FrameParser : public QObject
{
    Q_OBJECT

public:
    /// @brief Construct a fresh parser in the initial state.
    /// @param[in] parent Owner QObject for lifetime management.
    explicit FrameParser(QObject* parent = nullptr);

    /// @brief Reset the parser to its initial state, discarding partial data.
    void reset();

    /// @brief Feed raw bytes from the TCP socket into the state machine.
    /// @param[in] data Bytes freshly read from the socket.
    /// @note May emit frameReady() zero or more times per call depending on
    ///       how many complete frames are contained in the data.
    void parse(const QByteArray& data);

signals:
    /// @brief Emitted once for each complete, validated frame.
    /// @param payload The application-layer payload (framing stripped).
    void frameReady(const QByteArray& payload);

private:
    /// @brief Handle a single incoming byte through the state machine.
    /// @param[in] byte One byte from the TCP stream.
    void processByte(uint8_t byte);

    ParseState m_state{ParseState::kWaitingMagic1};  ///< Current FSM state
    QByteArray m_payload;                            ///< Accumulated payload buffer
    uint16_t m_expectedLength{0};                    ///< Length read from header
    uint16_t m_bytesReceived{0};                     ///< Payload bytes received so far
    uint8_t  m_lengthHighByte{0};                    ///< Temp storage for high byte of length
};
