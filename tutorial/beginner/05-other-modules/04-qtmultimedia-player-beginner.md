# 现代Qt开发教程（新手篇）5.4——QtMultimedia 音视频播放基础

## 1. 前言：音视频播放比你想象的要近

做桌面应用做到一定程度，迟早会遇到"播放一段音视频"的需求。可能是做个本地播放器，可能是产品介绍页里嵌一个宣传片，也可能是工业场景里需要回放录像文件。不管哪种场景，Qt 内置的 QtMultimedia 模块都能直接搞定，不需要你折腾 FFmpeg、GStreamer 这些底层库。

Qt 6 对多媒体模块做了一次彻底的重构。Qt 5 时代的 `QMediaPlayer` 依赖平台插件（Windows 上用 DirectShow / WMF，Linux 上用 GStreamer），API 碎片化严重，同一个功能在不同平台上的行为经常不一致。Qt 6.0 把整个模块砍掉重写，到 Qt 6.5 以后基本稳定了。新的架构把播放、录制、摄像头分成了独立的类，API 设计比旧版清晰得多。这篇我们要做的是把 QtMultimedia 的播放管线彻底搞清楚——从 `QMediaPlayer` 到 `QVideoWidget`，从播放控制到状态监听，一条线走到底。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 `Qt6::Multimedia`、`Qt6::MultimediaWidgets` 和 `Qt6::Widgets` 三个模块。CMake 配置需要 `find_package(Qt6 REQUIRED COMPONENTS Core Multimedia MultimediaWidgets Widgets)`。QtMultimedia 在不同平台上有不同的后端实现：Windows 上使用 Windows Media Foundation（WMF），macOS 上使用 AVFoundation，Linux 上使用 PulseAudio + GStreamer。大部分情况下你不需要关心后端差异，Qt 已经做了抽象。编译工具链方面，MSVC 2019+、GCC 11+ 均可，C++17 标准，CMake 3.26+ 构建系统。

有一点需要特别注意：Linux 上如果你用的是最小化安装的发行版，可能没有预装 GStreamer 的插件包。遇到"无法播放"的问题时，先检查一下系统是否安装了 `gstreamer1.0-plugins-good` 和 `gstreamer1.0-plugins-bad`。这一步很容易被忽略，笔者在这里踩过坑。

## 3. 核心概念讲解

### 3.1 播放管线的组成——QMediaPlayer + QAudioOutput + QVideoWidget

Qt 6 的多媒体播放管线由三个核心组件构成：`QMediaPlayer` 负责解码和控制播放流程，`QAudioOutput` 负责音频输出到声卡，`QVideoWidget` 负责视频画面渲染到界面。这三个组件通过 `setAudioOutput()` 和 `setVideoOutput()` 连接到 `QMediaPlayer` 上，形成一个完整的播放管线。

你可以把 `QMediaPlayer` 理解成一个播放引擎——它知道怎么解码音视频文件，但它自己不会发出声音也不会显示画面。音频必须通过 `QAudioOutput` 输出，视频必须通过 `QVideoWidget`（或者 `QGraphicsVideoItem`）输出。这种解耦设计比 Qt 5 那种"一个类全包"的方式灵活得多——比如你只想播放音频（做个音乐播放器），那只接 `QAudioOutput` 就够了，不需要创建视频组件。

最基本的播放代码：

```cpp
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>
#include <QUrl>

// 创建播放器和输出组件
QMediaPlayer *player = new QMediaPlayer(this);
QAudioOutput *audioOutput = new QAudioOutput(this);
QVideoWidget *videoWidget = new QVideoWidget(this);

// 组装管线
player->setAudioOutput(audioOutput);
player->setVideoOutput(videoWidget);

// 设置音量（0.0 到 1.0）
audioOutput->setVolume(0.8);

// 设置播放源
player->setSource(QUrl::fromLocalFile("/path/to/video.mp4"));

// 开始播放
player->play();
```

`QAudioOutput` 的 `setVolume()` 接受 0.0 到 1.0 之间的浮点数，0 是静音，1 是最大音量。`QVideoWidget` 就是一个普通的 QWidget，你可以把它放到任何布局里面，也可以直接作为独立窗口显示。

`setSource()` 接受 `QUrl` 类型参数，支持本地文件（`QUrl::fromLocalFile`）和网络流（`QUrl("http://...")`）。本地文件播放几乎零延迟，网络流则需要先缓冲——你可以通过 `bufferProgress` 属性监听缓冲进度。

### 3.2 播放控制——play / pause / stop / setPosition

`QMediaPlayer` 提供了一套完整的播放控制接口。`play()` 开始或恢复播放，`pause()` 暂停当前播放（保留当前位置），`stop()` 停止播放并将位置重置到开头。这三个操作对应三种播放状态：

```cpp
// 播放控制
player->play();     // 开始播放
player->pause();    // 暂停（位置不变）
player->stop();     // 停止（位置回到 0）

// 跳转位置（毫秒）
player->setPosition(30000);  // 跳到 30 秒处

// 查询状态
qint64 pos = player->position();         // 当前位置（毫秒）
qint64 dur = player->duration();         // 总时长（毫秒）
```

`setPosition()` 接受毫秒为单位的整数，可以用来实现进度条拖拽跳转。`position()` 和 `duration()` 也是毫秒值。对于视频文件来说，`duration()` 在 `setSource()` 之后不一定立刻有值——播放器需要先解析文件头部才能获取总时长。如果你在 `setSource()` 之后马上调用 `duration()`，大概率拿到的是 0。正确的做法是监听 `durationChanged` 信号，等它触发后再更新界面上的时长显示。

这里有一个实际开发中很常用的进度条更新模式：用 `QSlider` 作为进度条，`position()` 信号驱动滑块移动，用户拖拽滑块时通过 `setPosition()` 跳转：

```cpp
// 播放位置变化 → 更新滑块
connect(player, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
    if (!slider->isSliderDown()) {  // 用户没在拖拽时才更新
        slider->blockSignals(true);
        slider->setValue(static_cast<int>(position));
        slider->blockSignals(false);
    }
    // 更新时间标签
    timeLabel->setText(formatTime(position) + " / " + formatTime(player->duration()));
});

// 用户拖拽滑块 → 跳转位置
connect(slider, &QSlider::sliderMoved, player, &QMediaPlayer::setPosition);
```

`blockSignals(true)` 是个很关键的调用。如果你不阻断信号，`setValue()` 会触发 `valueChanged` 信号，而你在 `valueChanged` 的槽里又调了 `setPosition()`，就会形成信号死循环。`isSliderDown()` 判断用户是否正在拖拽滑块——如果用户正在拖拽，就不应该用播放位置去更新滑块，否则滑块会被"拽回去"，拖拽体验会非常糟糕。

### 3.3 播放状态信号——playbackStateChanged / errorOccurred

`QMediaPlayer` 的状态管理通过信号机制暴露给开发者。最重要的两个信号是 `playbackStateChanged` 和 `errorOccurred`。

`playbackStateChanged` 在播放状态切换时触发，三种状态分别是 `PlayingState`、`PausedState` 和 `StoppedState`：

```cpp
connect(player, &QMediaPlayer::playbackStateChanged, this,
    [](QMediaPlayer::PlaybackState state) {
        switch (state) {
        case QMediaPlayer::PlayingState:
            // 更新按钮图标为"暂停"
            playButton->setText("Pause");
            break;
        case QMediaPlayer::PausedState:
            playButton->setText("Play");
            break;
        case QMediaPlayer::StoppedState:
            playButton->setText("Play");
            // 停止时重置进度条
            break;
        }
    });
```

通过这个信号，你可以精确地同步 UI 状态和播放状态。很多新手会用 `playButton->clicked` 信号去切换按钮文字，但这是不对的——播放可能因为错误、因为到达末尾、因为调用了 `stop()` 而停止，这些情况下你都需要更新按钮。以播放状态信号为唯一真相来源才是正确的做法。

`errorOccurred` 信号在播放出错时触发，包括文件不存在、格式不支持、解码失败等：

```cpp
connect(player, &QMediaPlayer::errorOccurred, this,
    [](QMediaPlayer::Error error, const QString &errorString) {
        qWarning() << "Playback error:" << error << errorString;
        // 向用户显示错误信息
        statusBar()->showMessage("播放出错: " + errorString, 5000);
    });
```

`error` 是一个枚举值，告诉你错误类型（`ResourceError` 资源不存在、`FormatError` 格式不支持、`NetworkError` 网络问题等），`errorString` 是人类可读的错误描述。实际开发中，你一定要处理这个信号——不然用户遇到播放失败时界面毫无反应，会以为你的程序卡死了。

还有一个比较有用的信号是 `mediaStatusChanged`，它告诉你媒体文件的加载状态——`LoadingMedia` 正在加载、`LoadedMedia` 加载完成、`EndOfMedia` 播放到末尾。当 `mediaStatus` 变为 `EndOfMedia` 时，你可以选择自动重播（`setLoops(QMediaPlayer::Infinite)`）或者 `stop()` 后重置到开头。

### 3.4 QAudioOutput 音频输出配置

`QAudioOutput` 是 Qt 6 新引入的类，专门负责音频输出设备的选择和音量控制。在 Qt 5 里，音量控制在 `QMediaPlayer` 上，音频设备选择更是几乎没有 API。Qt 6 把它独立出来，逻辑更清晰。

```cpp
QAudioOutput *audioOutput = new QAudioOutput(this);
audioOutput->setVolume(0.8);  // 音量 0.0 ~ 1.0
audioOutput->setDevice(QMediaDevices::defaultAudioOutput());

player->setAudioOutput(audioOutput);
```

`QMediaDevices::defaultAudioOutput()` 返回系统默认的音频输出设备。如果你的电脑插了耳机和音箱，想指定输出到特定设备，可以用 `QMediaDevices::audioOutputs()` 获取所有可用设备列表，然后选择其中一个：

```cpp
const QList<QAudioDevice> devices = QMediaDevices::audioOutputs();
for (const auto &device : devices) {
    qDebug() << "Audio device:" << device.description();
}
if (!devices.isEmpty()) {
    audioOutput->setDevice(devices.first());
}
```

这个功能在做播放器应用时很有用——用户可能想在"播放设置"里选择输出到音箱还是蓝牙耳机。不过对于大部分应用场景来说，用默认设备就够了。

## 4. 综合示例：简易视频播放器

把前面学的串起来，我们写一个包含播放/暂停/停止、进度条、音量控制、状态显示的完整视频播放器。程序使用 `QMediaPlayer` + `QAudioOutput` + `QVideoWidget` 构建播放管线，通过信号槽同步 UI 状态。

完整代码见 `examples/beginner/05-other-modules/04-qtmultimedia-player-beginner/`，下面是关键部分的讲解。

CMake 配置需要引入 Multimedia 和 MultimediaWidgets：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Multimedia MultimediaWidgets Widgets)
# ...
target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Multimedia Qt6::MultimediaWidgets Qt6::Widgets)
```

主程序的结构很清晰：顶部是 `QVideoWidget` 用于视频显示，底部是控制面板——包含播放按钮、停止按钮、进度条、时间标签、音量滑块、文件选择按钮。

```cpp
// 管线组装
auto *player = new QMediaPlayer(this);
auto *audioOutput = new QAudioOutput(this);
auto *videoWidget = new QVideoWidget(this);

player->setAudioOutput(audioOutput);
player->setVideoOutput(videoWidget);
audioOutput->setVolume(0.8);
```

播放按钮需要根据当前状态切换 play/pause 行为：

```cpp
connect(playButton, &QPushButton::clicked, this, [this, player]() {
    if (player->playbackState() == QMediaPlayer::PlayingState) {
        player->pause();
    } else {
        player->play();
    }
});
```

进度条需要双向绑定——播放位置更新进度条，用户拖拽进度条跳转播放位置：

```cpp
// 播放位置 → 进度条
connect(player, &QMediaPlayer::positionChanged, this, [this](qint64 pos) {
    if (!progressSlider->isSliderDown()) {
        progressSlider->blockSignals(true);
        progressSlider->setValue(static_cast<int>(pos));
        progressSlider->blockSignals(false);
    }
    timeLabel->setText(formatTime(pos) + " / " + formatTime(player->duration()));
});

// 进度条拖拽 → 跳转位置
connect(progressSlider, &QSlider::sliderMoved, player, &QMediaPlayer::setPosition);
```

文件选择通过 `QFileDialog::getOpenFileName` 实现，支持常见音视频格式：

```cpp
connect(openButton, &QPushButton::clicked, this, [this, player]() {
    QString file = QFileDialog::getOpenFileName(
        this, "选择媒体文件", {},
        "Media Files (*.mp4 *.avi *.mkv *.mp3 *.wav *.flac);;All Files (*)");
    if (!file.isEmpty()) {
        player->setSource(QUrl::fromLocalFile(file));
    }
});
```

错误处理不能少：

```cpp
connect(player, &QMediaPlayer::errorOccurred, this,
    [this](QMediaPlayer::Error, const QString &errorString) {
        errorLabel->setText("Error: " + errorString);
        errorLabel->show();
    });
```

运行程序后，点击"打开文件"选择一个视频或音频文件，点播放按钮就能正常播放了。进度条会随播放位置自动移动，拖拽进度条可以跳转，音量滑块实时生效。如果你选择的是纯音频文件（mp3、wav），`QVideoWidget` 区域会是空白的，但声音正常输出——这就是管线解耦带来的好处。

## 5. 练习项目

练习项目：带播放列表的音频播放器。

我们要做一个支持多文件顺序播放的音频播放器，使用 QListWidget 作为播放列表，支持添加、删除、双击播放列表项切换歌曲，当前歌曲播放完后自动播放下一首。

完成标准是这样的：播放列表使用 `QListWidget`，通过"添加文件"按钮可以一次选多个音频文件加入列表；双击列表项切换播放对应文件；当 `mediaStatusChanged` 报告 `EndOfMedia` 时自动切换到下一首；正在播放的列表项用高亮标识；窗口标题显示当前正在播放的文件名。

几个实现提示：用 `QList<QUrl>` 维护播放列表的文件路径，`QListWidget` 只显示文件名（用 `QFileInfo::fileName()` 提取）；`EndOfMedia` 时计算下一首的索引并调用 `setSource()` + `play()`；最后一首播放完后可以选择回到第一首循环，或者停止。

## 6. 官方文档参考

[Qt 文档 · QtMultimedia 模块](https://doc.qt.io/qt-6/qtmultimedia-index.html) -- 多媒体模块总览

[Qt 文档 · QMediaPlayer](https://doc.qt.io/qt-6/qmediaplayer.html) -- 播放器核心类

[Qt 文档 · QAudioOutput](https://doc.qt.io/qt-6/qaudiooutput.html) -- 音频输出类

[Qt 文档 · QVideoWidget](https://doc.qt.io/qt-6/qvideowidget.html) -- 视频显示控件

[Qt 文档 · QMediaDevices](https://doc.qt.io/qt-6/qmediadevices.html) -- 媒体设备枚举

*（链接已验证，2026-04-23 可访问）*

---

到这里就大功告成了！QtMultimedia 在 Qt 6 里的播放管线设计得非常清晰：QMediaPlayer 做引擎，QAudioOutput 管声音，QVideoWidget 管画面，三者通过 set 方法组装到一起。掌握了这套管线之后，不管你是做简单的音视频播放，还是做复杂的播放器应用，底层逻辑都是一样的——创建组件、组装管线、设置源、控制播放、监听状态。后续如果你需要更高级的功能（比如播放速度调节、字幕叠加、视频滤镜），Qt 6.8+ 引入的 QtMultimedia 扩展 API 也都在这套管线上扩展的。

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
