#include "wavutils.h"

#include <QDebug>
#include <QFile>

QByteArray generateSineWaveWav(int frequency, double durationSec,
                               int sampleRate)
{
    const int numSamples = static_cast<int>(sampleRate * durationSec);
    const int dataSize = numSamples * 2;  // 16-bit mono

    QByteArray wav;
    wav.reserve(44 + dataSize);

    auto writeUint32 = [&wav](quint32 v) {
        wav.append(reinterpret_cast<const char*>(&v), 4);
    };
    auto writeUint16 = [&wav](quint16 v) {
        wav.append(reinterpret_cast<const char*>(&v), 2);
    };

    // RIFF 头
    wav.append("RIFF", 4);
    writeUint32(36 + dataSize);
    wav.append("WAVE", 4);

    // fmt 子块：PCM 格式、单声道、16-bit
    wav.append("fmt ", 4);
    writeUint32(16);          // 子块大小
    writeUint16(1);           // PCM 格式
    writeUint16(1);           // 单声道
    writeUint32(sampleRate);  // 采样率
    writeUint32(sampleRate * 2);  // 字节率（sampleRate * channels * bits/8）
    writeUint16(2);           // 块对齐（channels * bits/8）
    writeUint16(16);          // 位深度

    // data 子块
    wav.append("data", 4);
    writeUint32(dataSize);

    // 生成正弦波采样数据
    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        double sample = 0.5 * qSin(2.0 * M_PI * frequency * t);
        auto intSample = static_cast<qint16>(sample * 32767.0);
        wav.append(reinterpret_cast<const char*>(&intSample), 2);
    }

    return wav;
}

QString writeWavToTemp(const QByteArray &wavData,
                       const QString &fileName)
{
    QString tempPath
        = QStringLiteral("/tmp/qt_spatial_%1").arg(fileName);
    QFile file(tempPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(wavData);
        file.close();
        qDebug() << "已生成音频文件:" << tempPath;
    }
    return tempPath;
}
