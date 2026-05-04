# 现代Qt开发教程（新手篇）5.23--Qt Spatial Audio 空间音频

## 1. 前言：给声音加上三维坐标

传统的音频播放是"平面的"——你调用 QMediaPlayer 或者 QSoundEffect 播放一段音频，声音从扬声器里出来，本质上没有空间位置的概念。如果你在做游戏、虚拟现实、或者工业仿真这类应用，这种平面音频远远不够。现实世界中我们能分辨声音的方向和距离，是因为人有双耳——左耳和右耳接收到的声波在时间差、强度差、频率响应上存在微妙的差异，大脑根据这些差异推算出声源的空间位置。空间音频（Spatial Audio）就是用算法模拟这种双耳效应，让耳机里的声音听起来好像是从三维空间中某个特定位置发出来的。

Qt Spatial Audio 模块提供了完整的 3D 空间音频能力。你可以在一个虚拟的 3D 空间中放置多个音源（QSpatialSound），每个音源有自己的三维坐标；然后设置一个听者（QAudioListener），代表听众在空间中的位置和朝向；音频引擎（QAudioEngine）负责计算每个音源到听者的空间关系，应用 HRTF（头部相关传输函数）算法生成带有空间感的立体声输出。这个过程对开发者来说是完全透明的——你只需要设置坐标，剩下的交给引擎。

这篇我们要做的是初始化 QAudioEngine 音频引擎，放置几个 3D 音源并设置它们的空间位置，配置听者的位置和朝向，最后实现一个简单的空间音效演示程序。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 SpatialAudio 模块。CMake 配置如下：

```cmake
find_package(Qt6 REQUIRED COMPONENTS SpatialAudio)
```

Qt Spatial Audio 从 Qt 6.2 开始作为技术预览提供，在 Qt 6.5 之后逐步稳定。它底层依赖 Qt 的多媒体模块（QtMultimedia）做音频输出，同时在算法层面使用了 HRTF 数据库来模拟双耳听觉效果。模块支持房间声学模拟——通过 QAudioRoom 可以定义虚拟房间的尺寸、墙壁材质反射系数，引擎会自动计算早期反射和混响尾，让空间感更加真实。

工具链方面没有特殊要求：MSVC 2019+、GCC 11+、Clang 14+，C++17 标准，CMake 3.26+。运行时需要音频输出设备（声卡 + 耳机或扬声器）。空间音频效果在耳机上体验最好——因为耳机保证了左右声道完全隔离，HRTF 算法才能正确工作。如果你用外放扬声器，左右声道的串扰会大幅削弱空间感。

另外需要注意的是，QSpatialSound 需要实际的音频文件作为输入。支持的格式取决于平台的多媒体后端——通常 WAV、MP3、OGG 都没有问题。示例中我们使用程序生成的简单正弦波音频文件，避免需要额外准备素材。

## 3. 核心概念讲解

### 3.1 QAudioEngine 音频引擎

QAudioEngine 是整个空间音频系统的核心。它管理所有音源、听者和房间的状态，在每一帧音频输出时计算空间化效果。你可以把它类比为一个 3D 游戏的音频中间件——它不产生声音，但它决定每个声音在空间中听起来是什么样的。

```cpp
#include <QAudioEngine>

// 创建音频引擎，设置输出采样率
QAudioEngine engine;
engine.setSampleRate(44100);

// 启动引擎
engine.start();

// ... 使用完毕后停止
// engine.stop();
```

QAudioEngine 的构造函数接受一个可选的 QObject parent 参数。setSampleRate() 设置引擎的内部采样率——默认值通常是 44100Hz 或 48000Hz，取决于平台。如果你的音频文件采样率和引擎不一致，引擎会自动做采样率转换，但这会消耗一点 CPU。对于大多数应用使用默认值就行。

engine.start() 启动音频处理循环，它会申请音频输出设备并开始推送数据。如果音频设备不可用（比如没有声卡或者被其他程序独占），start() 不会报错但不会有声音输出——你可以通过检查 engine.outputDevice() 来确认设备状态。engine.stop() 停止处理循环，释放音频设备。通常在整个应用生命周期中只需要 start 一次，不需要反复启停。

引擎还提供 setOutputMode() 方法，支持三种输出模式：QAudioEngine::Surround（环绕声，适用于多声道扬声器）、QAudioEngine::Stereo（立体声，适用于耳机）、QAudioEngine::Mono（单声道，空间效果会被禁用）。使用耳机时应该设置为 Stereo。

### 3.2 QSpatialSound 设置 3D 音源位置

QSpatialSound 代表空间中的一个音源。它关联一个音频文件，有一个三维坐标表示音源在空间中的位置，还有一些参数控制声音的衰减行为。

```cpp
#include <QSpatialSound>

// 创建音源，关联到引擎
QSpatialSound* sound = new QSpatialSound(&engine);
sound->setSource(QUrl::fromLocalFile(QStringLiteral("/path/to/sound.wav")));

// 设置音源在 3D 空间中的位置（单位：米）
sound->setPosition(QVector3D(2.0f, 0.0f, 0.0f));  // 右侧 2 米处

// 设置音量
sound->setVolume(0.8f);

// 循环播放
sound->setLoops(QSoundEffect::Infinite);

// 开始播放
sound->play();
```

QSpatialSound 的坐标系使用右手坐标系，单位是米。X 轴指向右侧，Y 轴指向上方，Z 轴指向观察者的后方（也就是远离屏幕的方向）。所以 `QVector3D(2, 0, 0)` 表示音源在听者右侧 2 米处，`QVector3D(0, 0, -3)` 表示音源在听者前方 3 米处。

setPosition() 是动态可调的——你可以在每一帧更新音源的位置来实现移动音源的效果。比如一辆汽车从左向右驶过，你只需要用 QTimer 或者动画系统持续更新音源的 X 坐标，引擎会自动计算多普勒效应和距离衰减。setDistanceCutoff() 设置音源的最大可听距离——超过这个距离的音源不会播放。setSize() 设置音源的体积——体积为 0 表示点声源，体积大于 0 表示一个有体积的声源区域，音源体积内的距离衰减会被平滑处理。

QSpatialSound 还支持 directivity 参数（通过 setDirectivity()），控制音源的方向性——值越大音源越集中在某个方向上。比如模拟一个扬声器对着某个方向播放时可以用这个参数。

### 3.3 QAudioListener 听者位置与方向

QAudioListener 代表听者在空间中的位置和朝向。每个 QAudioEngine 只有一个活跃的听者，所有音源的空间化计算都是相对于这个听者进行的。听者的位置和朝向决定了"我站在哪里，我看向哪个方向"——同一个音源在同一个位置，如果听者转向了，声音的左右声道就会互换。

```cpp
#include <QAudioListener>

// 获取引擎的默认听者
QAudioListener* listener = engine.listener();

// 设置听者位置（站在原点）
listener->setPosition(QVector3D(0.0f, 0.0f, 0.0f));

// 设置听者朝向——面向 -Z 方向（"前方"）
// 参数分别是：front 向量、up 向量
listener->setRotation(QQuaternion::fromEulerAngles(0.0f, 0.0f, 0.0f));
```

听者的坐标系约定是这样的：默认情况下（不旋转），听者面向 -Z 方向，头顶朝 +Y 方向，右手边是 +X 方向。这和 OpenGL 的相机坐标系一致。setRotation() 接受一个 QQuaternion 表示听者的旋转姿态，你可以用 QQuaternion::fromEulerAngles() 从欧拉角构建，也可以用 QQuaternion::fromAxisAndAngle() 绕任意轴旋转。

在实际应用中，听者的位置通常和用户控制的"角色"绑定。比如在 FPS 游戏中，听者的位置就是角色的位置，听者的朝向就是相机的朝向——每一帧从渲染系统拿到角色的 transform 矩阵，更新到 QAudioListener 上就行了。在仿真或者 VR 应用中，听者的位置可能和头戴显示器的追踪数据绑定，实现头部转向时声音方向跟随变化的效果。

### 3.4 QAudioRoom 房间声学模拟

如果你需要在室内场景中使用空间音频，QAudioRoom 可以模拟房间的声学效果——墙壁的反射、空间的混响。默认情况下（没有添加任何 QAudioRoom），所有音源处于一个"自由场"环境中，只有直达声，没有反射和混响。这在室外场景是合理的，但在室内听起来会很不自然。

```cpp
#include <QAudioRoom>

QAudioRoom* room = new QAudioRoom(&engine);

// 设置房间尺寸（单位：米）
room->setDimensions(QVector3D(10.0f, 3.0f, 8.0f));  // 10x3x8 米的房间

// 设置墙壁材质（影响反射系数）
room->setWallMaterial(QAudioRoom::FrontWall, QAudioRoom::Concrete);
room->setWallMaterial(QAudioRoom::BackWall, QAudioRoom::Concrete);
room->setWallMaterial(QAudioRoom::LeftWall, QAudioRoom::Wood);
room->setWallMaterial(QAudioRoom::RightWall, QAudioRoom::Wood);
room->setWallMaterial(QAudioRoom::Floor, QAudioRoom::Tile);
room->setWallMaterial(QAudioRoom::Ceiling, QAudioRoom::AcousticCeilingTiles);
```

QAudioRoom 内置了几种常见的声学材质——Concrete（混凝土，高反射）、Wood（木材，中等反射）、Glass（玻璃，高反射但特定频率）、AcousticCeilingTiles（吸音板，低反射）、Tile（瓷砖，较高反射）等。引擎根据材质的反射系数计算早期反射声和混响尾。反射系数越高，混响时间越长，空间感越明显。你可以通过 setReflectionGain() 和 setReverbGain() 全局调节反射和混响的强度。

## 4. 综合示例：3D 空间音效演示

我们把前面的内容整合成一个完整的空间音频示例。程序在控制台输出说明，通过程序生成一个简单的 WAV 文件作为音源，然后在三维空间中放置三个音源分别位于听者的左、右、前方，依次播放来展示空间效果。由于空间音频需要实际的音频文件，我们在运行时动态生成一个简单的正弦波 WAV 文件。

CMake 配置：

```cmake
find_package(Qt6 REQUIRED COMPONENTS SpatialAudio)

qt_add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::SpatialAudio)
```

main.cpp 的完整代码：

```cpp
#include <QDebug>
#include <QFile>
#include <QVector3D>
#include <QAudioEngine>
#include <QAudioListener>
#include <QSpatialSound>

/// @brief 在内存中生成一个简单的正弦波 WAV 文件
/// @param frequency 正弦波频率（Hz）
/// @param durationSec 持续时间（秒）
/// @param sampleRate 采样率
/// @return 包含完整 WAV 文件数据的 QByteArray
static QByteArray generateSineWaveWav(int frequency, double durationSec,
                                      int sampleRate = 44100)
{
    int numSamples = static_cast<int>(sampleRate * durationSec);
    int dataSize = numSamples * 2;  // 16-bit mono

    // WAV 文件头
    QByteArray wav;
    wav.reserve(44 + dataSize);

    auto writeString = [&wav](const char* s, int len) {
        wav.append(s, len);
    };
    auto writeUint32 = [&wav](quint32 v) {
        wav.append(reinterpret_cast<const char*>(&v), 4);
    };
    auto writeUint16 = [&wav](quint16 v) {
        wav.append(reinterpret_cast<const char*>(&v), 2);
    };

    // RIFF 头
    writeString("RIFF", 4);
    writeUint32(36 + dataSize);  // 文件大小 - 8
    writeString("WAVE", 4);

    // fmt 子块
    writeString("fmt ", 4);
    writeUint32(16);          // 子块大小
    writeUint16(1);           // PCM 格式
    writeUint16(1);           // 单声道
    writeUint32(sampleRate);  // 采样率
    writeUint32(sampleRate * 2);  // 字节率
    writeUint16(2);           // 块对齐
    writeUint16(16);          // 位深度

    // data 子块
    writeString("data", 4);
    writeUint32(dataSize);

    // 生成正弦波采样数据
    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        double sample = 0.5 * qSin(2.0 * M_PI * frequency * t);
        qint16 intSample = static_cast<qint16>(sample * 32767.0);
        wav.append(reinterpret_cast<const char*>(&intSample), 2);
    }

    return wav;
}

/// @brief 将 WAV 数据写入临时文件并返回文件路径
static QString writeWavToTemp(const QByteArray &wavData,
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

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "Qt Spatial Audio 空间音频示例";
    qDebug() << "本示例演示 3D 音源定位 + 听者方向 + 空间化效果";
    qDebug() << "";
    qDebug() << "请佩戴耳机以获得最佳空间音频体验";
    qDebug() << "";

    // 生成三个不同频率的 WAV 文件
    // 左侧音源：440Hz（A4 音）
    QByteArray wavLeft
        = generateSineWaveWav(440, 3.0, 44100);
    QString pathLeft = writeWavToTemp(wavLeft, QStringLiteral("left.wav"));

    // 右侧音源：523Hz（C5 音）
    QByteArray wavRight
        = generateSineWaveWav(523, 3.0, 44100);
    QString pathRight = writeWavToTemp(wavRight, QStringLiteral("right.wav"));

    // 前方音源：660Hz（E5 音）
    QByteArray wavFront
        = generateSineWaveWav(660, 3.0, 44100);
    QString pathFront = writeWavToTemp(wavFront, QStringLiteral("front.wav"));

    // ========================================
    // 初始化音频引擎
    // ========================================

    QAudioEngine engine;
    engine.setSampleRate(44100);
    // 使用立体声输出模式（耳机推荐）
    engine.setOutputMode(QAudioEngine::Stereo);

    // 设置听者位于原点，面向 -Z 方向（默认朝向）
    QAudioListener* listener = engine.listener();
    listener->setPosition(QVector3D(0.0f, 0.0f, 0.0f));
    listener->setRotation(QQuaternion());

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
```

运行程序后，请戴上耳机。程序会在 1 秒后播放左侧音源（440Hz，你会听到声音从左边传来），4.5 秒后播放右侧音源（523Hz，声音从右边传来），8 秒后播放前方音源（660Hz，声音从前方传来），12 秒后自动退出。

这个示例刻意用了三个不同的频率（A4、C5、E5），这样即使空间感不够明显，至少能通过音高区分是哪个音源在播放。如果听下来觉得三个音源的位置感差不多，那很可能是用扬声器播放导致的——换成耳机再试一次，差异会非常明显。

几个实现细节说明一下。generateSineWaveWav() 函数在内存中构建了一个标准的 PCM WAV 文件——16-bit 单声道，44100Hz 采样率。这个函数的存在是为了让示例不依赖外部音频文件。实际项目中你当然应该用真正的音频素材文件。WAV 文件写在 /tmp 目录下，这在 Linux 和 macOS 上没有问题，Windows 上需要改成其他临时目录。QSpatialSound 的 setSource() 接受 QUrl 参数——本地文件用 QUrl::fromLocalFile()，远程文件用普通的 HTTP URL。音源位置使用 QVector3D，单位是米，坐标系和 OpenGL 一致。程序使用 QTimer::singleShot 来控制播放时序，这是因为 QSpatialSound 没有内置的"延时播放"接口，我们通过定时器手动触发 play()。

## 5. 练习项目

练习项目：移动音源模拟。

在示例基础上实现一个音源沿路径移动的效果：创建一个音源在 Y=0 平面上以听者为圆心做圆周运动（半径 3 米），播放一段循环音频。听者的位置和朝向可以通过键盘控制——WASD 控制前后左右移动，鼠标或方向键控制转向。

完成标准是这样的：音源做圆周运动时能明显听出声音从左到右再从右到左的周期性变化；听者转向后声音的空间方向随之改变；音源靠近时音量增大、远离时音量减小（距离衰减）。

几个实现提示：音源位置用参数方程 `x = R*cos(theta), z = -R*sin(theta)` 计算，theta 随时间线性增长。听者转向时需要更新 QAudioListener 的 rotation，可以用 QQuaternion::fromAxisAndAngle 绕 Y 轴旋转。如果你不想做 GUI 窗口，也可以纯控制台实现——用 QTimer 每帧更新音源位置，让音源自动转圈，不需要用户交互。

## 6. 官方文档参考

[Qt 文档 · Qt Spatial Audio 模块](https://doc.qt.io/qt-6/qtspatialaudio-index.html) -- 空间音频模块总览与 API 参考

[Qt 文档 · QAudioEngine](https://doc.qt.io/qt-6/qaudioengine.html) -- 空间音频引擎，管理音源和听者

[Qt 文档 · QSpatialSound](https://doc.qt.io/qt-6/qspatialsound.html) -- 3D 空间音源，设置位置和播放参数

[Qt 文档 · QAudioListener](https://doc.qt.io/qt-6/qlistener.html) -- 听者位置与朝向

[Qt 文档 · QAudioRoom](https://doc.qt.io/qt-6/qaudioroom.html) -- 房间声学模拟

---

到这里就大功告成了。Qt Spatial Audio 把 3D 空间音频的能力直接拉进了 Qt 生态——创建 QAudioEngine，用 QSpatialSound 放置带坐标的音源，用 QAudioListener 设置听者位置和朝向，引擎自动计算 HRTF 空间化效果。对于游戏、VR、仿真这类需要沉浸式音效的应用，这套 API 比自己对接 OpenAL 或者 FMOD 轻量得多，而且和 Qt 的事件循环、信号槽、3D 模块无缝集成。如果你正在做任何需要"声音有方向感"的项目，这个模块值得一试。
