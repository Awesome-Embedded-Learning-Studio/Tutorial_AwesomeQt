/**
 * Qt Spatial Audio 空间音频示例
 *
 * 本示例演示 Qt SpatialAudio 模块的核心功能：
 * - QAudioEngine 音频引擎初始化与输出配置
 * - QSpatialSound 设置 3D 音源位置
 * - QAudioListener 听者位置与朝向
 * - 程序生成 WAV 文件作为音源素材
 *
 * 启动后依次播放三个空间音源：
 *   - 左侧（440Hz，X=-3m）
 *   - 右侧（523Hz，X=+3m）
 *   - 前方（660Hz，Z=-4m）
 *
 * 请佩戴耳机以获得最佳空间音频体验。
 */

#include <QDebug>
#include <QQuaternion>
#include <QTimer>
#include <QUrl>
#include <QVector3D>
#include <QCoreApplication>

#include <QAudioEngine>
#include <QAudioListener>
#include <QSpatialSound>

#include "wavutils.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "Qt Spatial Audio 空间音频示例";
    qDebug() << "本示例演示 3D 音源定位 + 听者方向 + 空间化效果";
    qDebug() << "";
    qDebug() << "请佩戴耳机以获得最佳空间音频体验";
    qDebug() << "";

    // 生成三个不同频率的 WAV 文件作为音源素材
    QByteArray wavLeft
        = generateSineWaveWav(440, 3.0, 44100);
    QString pathLeft = writeWavToTemp(wavLeft, QStringLiteral("left.wav"));

    QByteArray wavRight
        = generateSineWaveWav(523, 3.0, 44100);
    QString pathRight = writeWavToTemp(wavRight, QStringLiteral("right.wav"));

    QByteArray wavFront
        = generateSineWaveWav(660, 3.0, 44100);
    QString pathFront = writeWavToTemp(wavFront, QStringLiteral("front.wav"));

    // ========================================
    // 初始化音频引擎
    // ========================================

    QAudioEngine engine(44100);
    // 使用立体声输出模式（耳机推荐）
    engine.setOutputMode(QAudioEngine::Stereo);

    // 设置听者位于原点，面向 -Z 方向（默认朝向）
    QAudioListener listener(&engine);
    listener.setPosition(QVector3D(0.0f, 0.0f, 0.0f));
    listener.setRotation(QQuaternion());

    // 启动引擎
    engine.start();
    qDebug() << "音频引擎已启动";

    // ========================================
    // 放置三个 3D 音源
    // ========================================

    // 左侧音源（X=-3, Y=0, Z=0）→ 听者左边 3 米处
    QSpatialSound soundLeft(&engine);
    soundLeft.setSource(QUrl::fromLocalFile(pathLeft));
    soundLeft.setPosition(QVector3D(-3.0f, 0.0f, 0.0f));
    soundLeft.setVolume(0.7f);

    // 右侧音源（X=3, Y=0, Z=0）→ 听者右边 3 米处
    QSpatialSound soundRight(&engine);
    soundRight.setSource(QUrl::fromLocalFile(pathRight));
    soundRight.setPosition(QVector3D(3.0f, 0.0f, 0.0f));
    soundRight.setVolume(0.7f);

    // 前方音源（X=0, Y=0, Z=-4）→ 听者前方 4 米处
    QSpatialSound soundFront(&engine);
    soundFront.setSource(QUrl::fromLocalFile(pathFront));
    soundFront.setPosition(QVector3D(0.0f, 0.0f, -4.0f));
    soundFront.setVolume(0.7f);

    // ========================================
    // 依次播放三个音源，展示空间定位效果
    // ========================================

    qDebug() << "";
    qDebug() << "即将依次播放三个音源：";
    qDebug() << "  1. 左侧（440Hz，X=-3m）";
    qDebug() << "  2. 右侧（523Hz，X=+3m）";
    qDebug() << "  3. 前方（660Hz，Z=-4m）";
    qDebug() << "";

    // 延迟 1 秒后播放左侧音源
    QTimer::singleShot(1000, [&soundLeft]() {
        qDebug() << "[播放] 左侧音源（440Hz）";
        soundLeft.play();
    });

    // 延迟 4.5 秒后播放右侧音源
    QTimer::singleShot(4500, [&soundRight]() {
        qDebug() << "[播放] 右侧音源（523Hz）";
        soundRight.play();
    });

    // 延迟 8 秒后播放前方音源
    QTimer::singleShot(8000, [&soundFront]() {
        qDebug() << "[播放] 前方音源（660Hz）";
        soundFront.play();
    });

    // 12 秒后退出程序
    QTimer::singleShot(12000, [&engine]() {
        qDebug() << "";
        qDebug() << "空间音频演示结束";
        engine.stop();
        QCoreApplication::quit();
    });

    return app.exec();
}
