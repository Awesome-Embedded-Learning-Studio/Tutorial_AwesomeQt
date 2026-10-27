#include "frameparsingdemo.h"

#include <QDebug>

void demoFrameParsing()
{
    qDebug() << "\n=== Demo 3: Frame Parsing Simulation ===";
    qDebug() << "  Frame format: AA 55 [len] [data...] [xor_checksum]\n";

    // 构造测试帧
    auto buildFrame = [](const QByteArray &payload) -> QByteArray {
        QByteArray frame;
        frame.append(static_cast<char>(0xAA));  // 帧头 1
        frame.append(static_cast<char>(0x55));  // 帧头 2
        frame.append(
            static_cast<char>(payload.size()));  // 数据长度

        // 数据区
        for (char byte : payload) {
            frame.append(byte);
        }

        // XOR 校验和
        quint8 checksum = 0;
        for (char byte : payload) {
            checksum ^= static_cast<quint8>(byte);
        }
        frame.append(static_cast<char>(checksum));

        return frame;
    };

    // 构造几个测试帧
    QByteArray frame1 = buildFrame("Hello");
    QByteArray frame2 = buildFrame("QtSerialPort");
    QByteArray frame3 = buildFrame("\x01\x02\x03\x04\x05");

    qDebug() << "  Frame 1:" << frame1.toHex(' ');
    qDebug() << "  Frame 2:" << frame2.toHex(' ');
    qDebug() << "  Frame 3:" << frame3.toHex(' ');

    // 模拟：把所有帧拼接在一起（模拟分片到达的情况）
    QByteArray allData = frame1 + frame2 + frame3;
    qDebug() << "\n  Combined data:" << allData.toHex(' ');
    qDebug() << "  Total:" << allData.size() << "bytes";

    // 手动模拟帧解析过程
    QByteArray rxBuffer;
    rxBuffer.append(allData);

    qDebug() << "\n  Parsing frames from buffer:";
    int frameCount = 0;

    while (rxBuffer.size() >= 4) {
        if (static_cast<quint8>(rxBuffer[0]) != 0xAA
            || static_cast<quint8>(rxBuffer[1]) != 0x55) {
            rxBuffer.remove(0, 1);
            continue;
        }

        quint8 dataLen = static_cast<quint8>(rxBuffer[2]);
        int frameLen = 2 + 1 + dataLen + 1;

        if (rxBuffer.size() < frameLen) {
            qDebug() << "  Incomplete frame, waiting for more data...";
            break;
        }

        QByteArray frame = rxBuffer.left(frameLen);
        rxBuffer.remove(0, frameLen);
        ++frameCount;

        quint8 checksum = 0;
        for (int i = 3; i < frameLen - 1; ++i) {
            checksum ^= static_cast<quint8>(frame[i]);
        }
        quint8 receivedChecksum =
            static_cast<quint8>(frame[frameLen - 1]);

        if (checksum == receivedChecksum) {
            QByteArray payload = frame.mid(3, dataLen);
            qDebug() << "  [Frame" << frameCount << "] Valid - Payload:"
                     << payload.toHex(' ')
                     << "ASCII:" << payload;
        } else {
            qDebug() << "  [Frame" << frameCount << "] CHECKSUM ERROR";
        }
    }

    qDebug() << "\n  Parsed" << frameCount << "frame(s),"
             << rxBuffer.size() << "bytes remaining in buffer.";
}
