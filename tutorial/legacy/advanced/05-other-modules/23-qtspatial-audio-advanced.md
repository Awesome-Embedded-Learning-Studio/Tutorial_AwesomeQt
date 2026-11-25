---
title: "5.23 空间音频进阶：混响、距离衰减、头部追踪"
description: "入门篇我们让 QSpatialSound 发出了声音，也体验了 3D 定位效果——声音从左边来还是右边来。进阶篇要深入空间音频的物理模型：距离衰减（声音怎么随距离变弱）、房间混响（不同房间听起来不一样）、听者位置与朝向（头转了声音方向跟着变）。"
---

# 现代Qt开发教程（进阶篇）5.23——空间音频进阶：混响、距离衰减、头部追踪

## 1. 前言

入门篇我们把 Qt Spatial Audio 跑通了——`QSpatialSound` 放一个音源在 3D 空间中，`QAudioListener` 代表听者的位置，配合耳机可以感受到声音的方向感。但入门篇只是最基础的 3D 音频——声音定位正确了，但没有考虑环境因素。

在真实世界中，声音不只是有方向感。离得远声音就小，这是距离衰减。在空旷的大厅里声音有回声，在小隔间里声音很干，这是房间混响。你转个头，声音的左右关系会变化，这是头部朝向的影响。这些效果合在一起，才构成逼真的空间音频体验。

Qt Spatial Audio 模块提供了 `QAudioRoom` 来模拟房间声学效果，`QSpatialSound` 的距离衰减模型可以选择线性或对数，`QAudioListener` 支持设置位置和朝向。这篇我们逐个拆解这些参数。

## 2. 环境说明

本文档基于 Qt 6.5+ 编写，需要 Qt6::SpatialAudio 模块。CMake 中通过 `find_package(Qt6 REQUIRED COMPONENTS SpatialAudio)` 引入。Qt Spatial Audio 底层使用 Ambisonics 技术实现 3D 音频，需要立体声耳机才能获得正确的空间效果（扬声器因为声道串扰会导致定位不准）。在 Linux 上可能需要 PulseAudio 或 PipeWire 作为音频后端。

## 3. 核心概念讲解

### 3.1 QSpatialSound——3D 音源定位与配置

`QSpatialSound` 是 3D 空间中的音源。它的核心属性是 `position`（音源在 3D 空间中的位置）、`rotation`（音源的朝向，对有方向性的声源有意义）、`volume`（音量）以及距离衰减参数。

```cpp
/// @brief 创建并配置一个 3D 音源。
/// @param[in] engine 音频引擎。
/// @return 音源对象指针。
QSpatialSound* create_audio_source(QAudioEngine* engine)
{
    auto* sound = new QSpatialSound(engine);
    sound->setSource(QUrl::fromLocalFile("sounds/gunshot.wav"));
    sound->setPosition(QVector3D(5.0f, 0.0f, 0.0f));   // 右侧 5 米
    sound->setVolume(0.8f);

    // 距离衰减模型
    sound->setDistanceModel(QSpatialSound::DistanceModel::Logarithmic);
    sound->setManualAttenuation(1.0f);  // 手动衰减增益

    // 近场和远场距离（对数模型下有效）
    sound->setMinDistance(1.0f);   // 1 米内不衰减
    sound->setMaxDistance(50.0f);  // 50 米外静音

    return sound;
}
```

`setMinDistance` 设置了参考距离——在这个距离内声音不衰减（保持原始音量）。`setMaxDistance` 设置了最大可听距离——超过这个距离声音就听不到了。距离衰减模型的两个选项：`Linear` 是线性衰减（距离翻倍音量减半，听起来不自然），`Logarithmic` 是对数衰减（更接近真实世界的物理规律——声压与距离的平方成反比）。

对数衰减模型的公式大致是 `gain = minDistance / max(minDistance, distance)`。当 `distance` 等于 `minDistance` 时增益为 1.0（无衰减），距离增大后增益按反比关系减小。这比线性衰减听起来自然得多——在真实世界中，从 1 米走到 2 米的声音变化量，远大于从 10 米走到 11 米的变化量。

```cpp
/// @brief 模拟一个移动的音源（比如飞行中的子弹）。
/// @param[in] sound 音源对象。
void simulate_moving_source(QSpatialSound* sound)
{
    auto* timer = new QTimer(sound);
    float x = -20.0f;

    connect(timer, &QTimer::timeout, sound, [sound, &x]() {
        x += 0.5f;
        sound->setPosition(QVector3D(x, 0.0f, 0.0f));
        if (x > 20.0f) {
            x = -20.0f;  // 循环
        }
    });
    timer->start(50);  // 每 50ms 更新一次位置
}
```

这段代码模拟一个从左到右飞过的音源。由于使用了空间音频，配合耳机会听到声音从左耳移到右耳——非常有沉浸感。

### 3.2 距离衰减模型——线性与对数

距离衰减决定了声音随距离变小的规律。Qt Spatial Audio 提供两种模型，选择哪一种直接影响听感的自然程度。

线性衰减：`gain = 1.0 - (distance - minDistance) / (maxDistance - minDistance)`。距离从 `minDistance` 增加到 `maxDistance` 时，增益从 1.0 线性降到 0.0。这种模型计算简单，但听感不自然——在远处声音消失得太突然。

对数衰减：`gain = minDistance / max(minDistance, distance)`。距离增大时增益按反比关系缓慢减小，远处的声音不会突然消失而是逐渐变小。这正是真实世界中声音传播的规律——声压强度与距离的平方成反比，但人耳对音量的感知是对数关系，所以增益与距离成反比已经足够自然。

在实际项目中，几乎总是用对数衰减。线性衰减只在特殊场景下使用——比如你想让声音在特定距离内突然消失（模拟隔音墙）。

```cpp
/// @brief 配置多个音源使用不同的距离模型。
void configure_distance_models(QAudioEngine* engine)
{
    // 环境音——使用对数衰减，远处也能隐约听到
    auto* ambient = new QSpatialSound(engine);
    ambient->setSource(QUrl::fromLocalFile("sounds/wind.wav"));
    ambient->setPosition(QVector3D(0.0f, 10.0f, 0.0f));
    ambient->setDistanceModel(QSpatialSound::DistanceModel::Logarithmic);
    ambient->setMinDistance(5.0f);
    ambient->setMaxDistance(100.0f);
    ambient->setLoops(QSpatialSound::Infinite);  // 循环播放

    // 脚步声——使用线性衰减，远处的脚步声不应该被听到
    auto* footstep = new QSpatialSound(engine);
    footstep->setSource(QUrl::fromLocalFile("sounds/step.wav"));
    footstep->setDistanceModel(QSpatialSound::DistanceModel::Linear);
    footstep->setMinDistance(0.5f);
    footstep->setMaxDistance(10.0f);
}
```

### 3.3 QAudioRoom——房间混响效果

`QAudioRoom` 模拟了房间的声学效果。当音源和听者都在房间内部时，声音会经过墙壁反射产生混响（reverb）。不同大小、不同材质的房间混响特性不同——大理石大厅混响时间长，铺了地毯的小办公室混响几乎为零。

```cpp
/// @brief 创建一个模拟大厅的房间。
/// @param[in] engine 音频引擎。
/// @return 房间对象指针。
QAudioRoom* create_hall_room(QAudioEngine* engine)
{
    auto* room = new QAudioRoom(engine);
    room->setPosition(QVector3D(0.0f, 0.0f, 0.0f));     // 房间中心
    room->setDimensions(QVector3D(20.0f, 5.0f, 15.0f));  // 20m x 5m x 15m

    // 墙壁材质——影响声音反射率
    room->setWallMaterial(QAudioRoom::Wall::Front, QAudioRoom::Material::Concrete);
    room->setWallMaterial(QAudioRoom::Wall::Back, QAudioRoom::Material::Concrete);
    room->setWallMaterial(QAudioRoom::Wall::Left, QAudioRoom::Material::Brick);
    room->setWallMaterial(QAudioRoom::Wall::Right, QAudioRoom::Material::Brick);
    room->setWallMaterial(QAudioRoom::Wall::Floor, QAudioRoom::Material::Marble);
    room->setWallMaterial(QAudioRoom::Wall::Ceiling, QAudioRoom::Material::Wood);

    // 混响参数微调
    room->setReflectionGain(0.7f);     // 反射增益
    room->setReverbGain(0.5f);         // 混响增益
    room->setReverbTime(2.0f);         // 混响时间（秒）
    room->setReverbBrightness(0.8f);   // 混响亮度（高频衰减程度）

    return room;
}
```

房间的 6 面墙可以分别设置不同的材质。`QAudioRoom::Material` 枚举提供了常见的材质预设：`Transparent`（完全透明，不反射）、`AcousticCeilingTiles`（吸音板，弱反射）、`Brick`、`Concrete`、`Glass`、`Marble`（强反射）、`Wood` 等。材质决定了声音碰到墙壁后的反射率和吸收率。

混响时间和混响增益是最重要的两个参数。混响时间是从声音发出到混响衰减 60dB 所需的时间（即 RT60）。大教堂的 RT60 可能有 3-5 秒，普通办公室大约 0.5 秒，户外接近 0。混响增益控制混响的相对强度——0.0 表示完全干声（无混响），1.0 表示满强度混响。

一个场景中可以有多个房间，音源和听者在不同房间中的效果不同。当听者不在任何房间内时（或者房间没有设置），声音是「户外模式」——只有直接路径的距离衰减，没有混响。

### 3.4 QAudioListener——听者位置与朝向

`QAudioListener` 代表听者（通常是玩家或摄像机）在 3D 空间中的位置和朝向。空间音频的计算以听者为参考点——所有音源的位置、方向都是相对于听者来渲染的。

```cpp
/// @brief 配置听者并绑定到第一人称摄像机。
/// @param[in] engine 音频引擎。
/// @param[in] camera 摄像机对象（提供位置和朝向）。
void setup_listener(QAudioEngine* engine, Camera* camera)
{
    auto* listener = engine->listener();
    listener->setPosition(camera->position());
    listener->setRotation(camera->rotation());

    // 当摄像机移动/旋转时更新听者
    connect(camera, &Camera::position_changed, listener,
        [listener](const QVector3D& pos) {
            listener->setPosition(pos);
        });

    connect(camera, &Camera::rotation_changed, listener,
        [listener](const QQuaternion& rot) {
            listener->setRotation(rot);
        });
}
```

听者的朝向用 `QQuaternion` 表示。当听者转头时，空间音频引擎会重新计算所有音源相对于听者的方向，相应调整左右声道的音量。这就是「头部追踪」的基础——声音的方向跟着你的头转。

在 VR 或第一人称游戏中，头部追踪是沉浸感的核心。听者朝北时，东边的声音在右耳；听者转头朝东时，同一个声音跑到前面去了。`QAudioListener` 的 `setRotation` 会在每帧更新时触发重新计算。

现在有个思考题：如果听者在房间外面，音源在房间里面，听者能听到混响效果吗？

答案是听不到。`QAudioRoom` 的混响效果只在音源和听者都在房间内部时生效。如果只有音源在房间内，听者在房间外，音源的声音经过距离衰减后直接传到听者，不经过房间的反射计算。如果你需要模拟「从门外听到房间里的声音」，需要手动调整参数——在门外放一个低音量的音源模拟泄露的声音。

## 4. 踩坑预防

第一个坑是空间音频必须用耳机才能获得正确的方向感。扬声器输出的左右声道会互相串扰——左耳能听到右扬声器的声音，右耳也能听到左扬声器的声音。空间音频依赖精确的左右声道隔离来实现方向感，扬声器无法提供这种隔离。如果你的用户反馈「感觉不到方向感」，先确认他们是不是在用扬声器。

第二个坑是 `QAudioRoom` 的尺寸是矩形，不支持不规则形状。如果你的场景是一个 L 形的房间，只能用矩形近似（取外接矩形），或者用多个矩形房间拼接。不规则房间的混响模拟在学术研究中都是难题，Qt 的实现选择了简单实用的矩形模型。

第三个坑是听者的 `setRotation` 需要每帧更新才能实现实时头部追踪，但频繁的信号/槽调用可能有性能开销。在每帧渲染循环中直接调用 `setPosition` 和 `setRotation` 比通过信号槽更高效。如果你在 QML 中使用空间音频，可以用 `PropertyBinding` 直接绑定听者位置到摄像机位置，性能足够好。

## 5. 练习项目

练习项目是一个 3D 音频场景演示。场景中有 4 个音源和 1 个听者，听者通过键盘 WASD 控制移动，QE 控制转向。

场景是一个模拟的室内环境：一个 10m x 3m x 8m 的房间，使用 `QAudioRoom` 配置墙壁材质和混响参数。房间中央有一个循环播放的环境音乐音源。房间四个角落各有一个音效音源（比如滴水声、时钟声），使用对数距离衰减。听者通过 WASD 在房间内移动，通过 QE 左右转向，转向时声音方向跟着变化。

完成标准是用耳机能清晰感受到声音的方向感（左前、右后等）、移动时声音的距离衰减自然、转向时方向感正确变化、混响效果让声音有空间感（对比关闭房间混响时的干声）。不需要可视化 3D 场景，纯音频即可，控制台打印当前位置和朝向。

提示几个关键点：`QAudioEngine::listener()` 获取听者对象；键盘事件用 `QKeyEvent` 处理，每帧根据按键状态更新位置和朝向；房间混响参数可以从大教堂预设开始调参。

## 6. 官方文档参考链接

[Qt 文档 · QSpatialSound](https://doc.qt.io/qt-6/qspatialsound.html) -- 3D 音源配置

[Qt 文档 · QAudioRoom](https://doc.qt.io/qt-6/qaudioroom.html) -- 房间混响效果

[Qt 文档 · QAudioListener](https://doc.qt.io/qt-6/qaudiolistener.html) -- 听者位置与朝向

[Qt 文档 · QAudioEngine](https://doc.qt.io/qt-6/qaudioengine.html) -- 空间音频引擎

---

到这里空间音频的进阶用法就拆完了。距离衰减、房间混响、听者朝向——这三个维度的组合，足以构建从户外到室内的各种音频场景。如果你的项目需要更高级的音频效果（比如声音遮挡——墙壁挡住声音、声音传播延迟——远处的声音晚到），目前 Qt Spatial Audio 还不支持，需要用更专业的音频中间件（如 FMOD、Wwise）。
