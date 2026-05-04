#include <QByteArray>
#include <QString>

/// @brief 在内存中生成一个简单的正弦波 WAV 文件
/// @param frequency 正弦波频率（Hz）
/// @param durationSec 持续时间（秒）
/// @param sampleRate 采样率
/// @return 包含完整 WAV 文件数据的 QByteArray
QByteArray generateSineWaveWav(int frequency, double durationSec,
                               int sampleRate = 44100);

/// @brief 将 WAV 数据写入临时文件并返回文件路径
QString writeWavToTemp(const QByteArray &wavData,
                       const QString &fileName);
