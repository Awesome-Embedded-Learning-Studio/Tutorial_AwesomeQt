/// @file    main.cpp
/// @brief   Console demo for the binary protocol parser and serial manager.
///
/// @details Runs four demos in software mode (no physical serial port needed):
///          1) Single frame byte-by-byte parsing
///          2) Multiple concatenated frames fed at once (burst simulation)
///          3) Corrupted frame error detection
///          4) List available serial ports via QSerialPortInfo
///          Tutorial reference: advanced 04-qtnetwork/06-serialport-advanced.

#include "protocol_parser.h"
#include "serial_manager.h"

#include <QCoreApplication>
#include <QSerialPortInfo>
#include <QTimer>

#include <cstdio>

/// @brief Print a separator line to the console for readability.
static void printSeparator()
{
    std::printf("------------------------------------------------------------\n");
}

/// @brief Print a QByteArray as hex values for debugging.
/// @param[in] data  The byte array to print.
static void printHex(const QByteArray& data)
{
    for (int i = 0; i < data.size(); ++i)
    {
        std::printf("%02X ", static_cast<unsigned char>(data.at(i)));
    }
    std::printf("\n");
}

/// @brief Demo 1: Build a single frame and feed it to the parser byte by byte.
///
/// This simulates the worst case where each byte arrives in a separate
/// read event, which is common with slow serial baud rates.
static void demoSingleFrameByteByByte()
{
    std::printf("\n=== Demo 1: Single frame, byte-by-byte feeding ===\n\n");

    ProtocolParser parser;

    // Track received frames via lambda
    int frameCount = 0;
    quint8 receivedCmd = 0;
    QByteArray receivedPayload;

    QObject::connect(&parser, &ProtocolParser::frameReady,
                     [&](quint8 cmd, const QByteArray& payload)
                     {
                         ++frameCount;
                         receivedCmd = cmd;
                         receivedPayload = payload;
                     });

    // Build a test frame: command 0x10 with "Hello" payload
    const quint8 cmd = 0x10;
    const QByteArray payload = QByteArray("Hello");
    QByteArray frame = ProtocolParser::buildFrame(cmd, payload);

    std::printf("  Built frame (%d bytes): ", static_cast<int>(frame.size()));
    printHex(frame);

    // Feed one byte at a time to stress the state machine
    for (int i = 0; i < frame.size(); ++i)
    {
        QByteArray oneByte;
        oneByte.append(frame.at(i));
        parser.feed(oneByte);
    }

    std::printf("  Frame parsed: cmd=0x%02X, payload=\"%s\"\n",
                receivedCmd, receivedPayload.constData());
    std::printf("  Frames received: %d\n", frameCount);
    printSeparator();
}

/// @brief Demo 2: Build multiple frames, concatenate, and feed all at once.
///
/// Simulates a burst read where the OS delivers multiple frames in a
/// single readyRead callback. The parser must correctly split them.
static void demoMultipleFramesBurst()
{
    std::printf("\n=== Demo 2: Multiple frames, burst feeding ===\n\n");

    ProtocolParser parser;

    struct FrameInfo
    {
        quint8 cmd;
        QByteArray payload;
    };

    std::vector<FrameInfo> receivedFrames;

    QObject::connect(&parser, &ProtocolParser::frameReady,
                     [&](quint8 cmd, const QByteArray& payload)
                     {
                         receivedFrames.push_back({cmd, payload});
                     });

    // Build three different frames
    QByteArray burst;
    burst.append(ProtocolParser::buildFrame(0x01, QByteArray("First")));
    burst.append(ProtocolParser::buildFrame(0x02, QByteArray("Second")));
    burst.append(ProtocolParser::buildFrame(0x03, QByteArray()));

    std::printf("  Concatenated %d bytes from 3 frames\n",
                static_cast<int>(burst.size()));
    std::printf("  Raw burst data: ");
    printHex(burst);

    // Feed the entire burst at once
    parser.feed(burst);

    std::printf("  Frames parsed: %zu\n", receivedFrames.size());
    for (size_t i = 0; i < receivedFrames.size(); ++i)
    {
        std::printf("  Frame %zu: cmd=0x%02X, payload=\"%s\"\n",
                    i, receivedFrames[i].cmd,
                    receivedFrames[i].payload.constData());
    }
    printSeparator();
}

/// @brief Demo 3: Inject a corrupted frame and verify error detection.
///
/// Modifies the checksum of a valid frame to simulate transmission
/// corruption and verifies the parser emits the correct error.
static void demoCorruptedFrameDetection()
{
    std::printf("\n=== Demo 3: Corrupted frame detection ===\n\n");

    ProtocolParser parser;

    int errorCount = 0;
    ProtocolParser::ErrorCode lastError =
        ProtocolParser::ErrorCode::kChecksumMismatch;

    QObject::connect(&parser, &ProtocolParser::error,
                     [&](ProtocolParser::ErrorCode code)
                     {
                         ++errorCount;
                         lastError = code;
                     });

    // Build a valid frame then corrupt its last byte (checksum)
    QByteArray frame = ProtocolParser::buildFrame(0x20, QByteArray("Test"));
    std::printf("  Original frame: ");
    printHex(frame);

    // Flip bits in the checksum byte (last byte)
    int lastIdx = frame.size() - 1;
    frame[lastIdx] = static_cast<char>(
        static_cast<unsigned char>(frame.at(lastIdx)) ^ 0xFF);

    std::printf("  Corrupted frame: ");
    printHex(frame);

    parser.feed(frame);

    std::printf("  Errors detected: %d\n", errorCount);
    std::printf("  Error type: %s\n",
                lastError == ProtocolParser::ErrorCode::kChecksumMismatch
                    ? "kChecksumMismatch"
                    : "other");
    printSeparator();
}

/// @brief Demo 4: List all available serial ports on this system.
static void demoListSerialPorts()
{
    std::printf("\n=== Demo 4: Available serial ports ===\n\n");

    const auto ports = QSerialPortInfo::availablePorts();

    if (ports.isEmpty())
    {
        std::printf("  No serial ports found on this system.\n");
    }
    else
    {
        for (const auto& port : ports)
        {
            std::printf("  Port:      %s\n",
                        port.portName().toUtf8().constData());
            std::printf("  System:    %s\n",
                        port.systemLocation().toUtf8().constData());
            std::printf("  Desc:      %s\n",
                        port.description().toUtf8().constData());
            std::printf("  Manufacturer: %s\n",
                        port.manufacturer().toUtf8().constData());
            if (port.hasProductIdentifier())
            {
                std::printf("  PID:       0x%04X\n", port.productIdentifier());
            }
            if (port.hasVendorIdentifier())
            {
                std::printf("  VID:       0x%04X\n", port.vendorIdentifier());
            }
            std::printf("\n");
        }
    }

    printSeparator();
}

/// @brief Entry point: runs all demos then exits via timer.
int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    std::printf("Qt6 Serial Port Advanced Demo\n");
    std::printf("Binary Protocol State Machine Parser\n");
    printSeparator();

    // Run all demos in sequence
    demoSingleFrameByteByByte();
    demoMultipleFramesBurst();
    demoCorruptedFrameDetection();
    demoListSerialPorts();

    std::printf("\nAll demos completed.\n");

    // Auto-quit after a short delay to allow event loop cleanup
    QTimer::singleShot(0, &app, &QCoreApplication::quit);

    return app.exec();
}
