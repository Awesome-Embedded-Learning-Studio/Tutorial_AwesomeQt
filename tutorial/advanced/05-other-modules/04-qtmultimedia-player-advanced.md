---
title: "5.4 媒体播放进阶：播放列表、媒体元数据、字幕"
description: "入门篇我们把 QMediaPlayer + QAudioOutput 的基本播放流程跑通了——加载一个文件、播放、暂停、调节音量。写个简陋的音乐播放器确实够用了。但一个正经的媒体播放器还需要什么？播放列表管理、媒体元数据解析、播放进度控制、以及字幕支持。"
---

# 现代Qt开发教程（进阶篇）5.4——媒体播放进阶：播放列表、媒体元数据、字幕

## 1. 前言 / 从「能播」到「好用」

入门篇我们把 QMediaPlayer + QAudioOutput 的基本播放流程跑通了——加载一个文件、播放、暂停、调节音量。写个简陋的音乐播放器确实够用了。但一个正经的媒体播放器还需要什么？播放列表管理、媒体元数据解析、播放进度控制、以及字幕支持。

Qt 6 的多媒体模块在 6.x 时代经历了一次大规模 API 重构。Qt 5 时代的 QMediaPlayer 是一个「大而全」的类，既管播放又管视频输出。Qt 6 把它拆成了 QMediaPlayer（播放控制）、QAudioOutput（音频输出）、QVideoWidget（视频渲染）三个独立的组件，通过组合而不是继承来构建播放器。这种设计更灵活，但也意味着你需要理解它们之间的连接关系。

这篇我们一起来把播放列表（QMediaPlaylist 的替代方案）、元数据读取、精确 seek、以及 SRT 字幕解析这四个进阶功能拆干净。

## 2. 环境说明

本文档基于 Qt 6.4+ 编写（QMediaPlayer API 在 6.4 才稳定），使用 C++17 标准和 CMake 3.26+ 构建系统。本篇依赖 Qt6::Multimedia 和 Qt6::MultimediaWidgets 模块。多媒体后端依赖平台编解码器：Linux 使用 GStreamer，Windows 使用 Windows Media Foundation，macOS 使用 AVFoundation。确保系统安装了必要的编解码器包。

## 3. 核心概念讲解

### 3.1 播放列表——Qt 6 的手动管理方案

Qt 5 有 QMediaPlaylist 类，但 Qt 6 移除了它（Qt 6.5+ 才在 QtMultimedia 中重新引入）。在 Qt 6.4/6.5 的过渡期，你需要自己管理播放列表。这其实不难——一个 `QList<QUrl>` 加上当前索引就够了。

```cpp
class PlaylistManager : public QObject
{
    Q_OBJECT
public:
    void addMedia(const QUrl &url) { items_.append(url); }
    void clear() { items_.clear(); currentIndex_ = -1; }
    int count() const { return items_.size(); }

    QUrl currentMedia() const
    {
        if (currentIndex_ >= 0 && currentIndex_ < items_.size()) {
            return items_[currentIndex_];
        }
        return QUrl();
    }

    enum PlaybackMode { Sequential, Loop, Random };
    void setPlaybackMode(PlaybackMode mode) { mode_ = mode; }

public slots:
    void next()
    {
        if (items_.isEmpty()) return;
        if (mode_ == Loop) {
            currentIndex_ = (currentIndex_ + 1) % items_.size();
        } else if (mode_ == Random) {
            currentIndex_ = QRandomGenerator::global()->bounded(items_.size());
        } else {
            currentIndex_ = qMin(currentIndex_ + 1, items_.size() - 1);
        }
        emit currentIndexChanged(currentIndex_);
    }

    void previous()
    {
        if (items_.isEmpty()) return;
        if (mode_ == Loop) {
            currentIndex_ = (currentIndex_ - 1 + items_.size()) % items_.size();
        } else {
            currentIndex_ = qMax(currentIndex_ - 1, 0);
        }
        emit currentIndexChanged(currentIndex_);
    }

    void setCurrentIndex(int index)
    {
        if (index >= 0 && index < items_.size()) {
            currentIndex_ = index;
            emit currentIndexChanged(currentIndex_);
        }
    }

signals:
    void currentIndexChanged(int index);

private:
    QList<QUrl> items_;
    int currentIndex_ = -1;
    PlaybackMode mode_ = Sequential;
};
```

使用时监听 QMediaPlayer 的 `playbackStateChanged` 信号，当状态变为 `StoppedState` 且不是用户手动停止时，调用 `next()` 切到下一首。

### 3.2 媒体元数据——读取标题、艺术家、封面

QMediaPlayer 在 Qt 6 中通过 `metaData()` 方法返回一个 `QMediaMetaData` 对象，包含标题、艺术家、专辑、时长、比特率等常用字段。

```cpp
connect(player, &QMediaPlayer::metaDataChanged, this, [=]() {
    QMediaMetaData meta = player->metaData();

    QString title = meta.stringValue(QMediaMetaData::Title);
    QString artist = meta.stringValue(QMediaMetaData::Author);
    QString album = meta.stringValue(QMediaMetaData::AlbumTitle);
    qint64 duration = meta.value(QMediaMetaData::Duration).toLongLong();

    qDebug() << "Now playing:" << title << "by" << artist;
    qDebug() << "Album:" << album << "Duration:" << duration << "ms";

    // 提取封面图
    QVariant coverVariant = meta.value(QMediaMetaData::ThumbnailImage);
    if (coverVariant.isValid()) {
        QImage cover = coverVariant.value<QImage>();
        coverLabel->setPixmap(QPixmap::fromImage(cover).scaled(
            200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
});
```

`QMediaMetaData::ThumbnailImage` 返回的是嵌入在媒体文件中的封面图（ID3 tag 中的 APIC 帧）。不是所有文件都有封面——如果 `coverVariant.isValid()` 返回 false，就显示一个默认占位图。

### 3.3 精确 Seek 与进度控制

QMediaPlayer 提供了 `setPosition(qint64)` 方法来跳转到指定位置（毫秒）。看起来很简单，但有一个微妙的问题：`setPosition()` 不保证精确。底层的媒体框架可能把 seek 目标对齐到最近的关键帧（keyframe），导致实际跳转位置和你期望的差几百毫秒甚至几秒。

```cpp
// 跳转到 1 分 30 秒
player->setPosition(90 * 1000);  // 毫秒
```

对于大多数场景（用户拖动进度条），这个精度足够了。如果你需要帧级别的精确 seek（比如视频编辑），需要依赖平台特定的 API——Qt Multimedia 的抽象层不提供这个级别的控制。

进度条的实现用 `positionChanged` 和 `durationChanged` 信号：

```cpp
connect(player, &QMediaPlayer::positionChanged,
        slider, &QSlider::setValue);
connect(player, &QMediaPlayer::durationChanged,
        slider, &QSlider::setMaximum);
connect(slider, &QSlider::sliderMoved,
        player, &QMediaPlayer::setPosition);
```

注意一个细节：用户拖动 slider 时会触发 `sliderMoved` → `setPosition()` → `positionChanged` → `slider->setValue()` 的循环反馈。为了避免 slider 在用户拖动时跳动，应该在 `sliderPressed` 时断开 `positionChanged` 的连接，`sliderReleased` 时重新连接。

### 3.4 SRT 字幕解析与同步

Qt Multimedia 没有内置字幕渲染。如果你需要显示 SRT 格式的字幕，需要自己解析文件并同步到播放进度。

```cpp
struct SubtitleEntry
{
    qint64 startMs;
    qint64 endMs;
    QString text;
};

QList<SubtitleEntry> parseSrtFile(const QString &filePath)
{
    QList<SubtitleEntry> subtitles;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return subtitles;
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        // 序号行（忽略）
        stream.readLine();

        // 时间戳行：00:01:30,000 --> 00:01:35,000
        QString timeLine = stream.readLine();
        QStringList parts = timeLine.split("-->");
        if (parts.size() != 2) continue;

        qint64 start = parseTimestamp(parts[0].trimmed());
        qint64 end = parseTimestamp(parts[1].trimmed());

        // 字幕文本（可能多行，以空行结束）
        QString text;
        QString line;
        while (!(line = stream.readLine()).isEmpty()) {
            if (!text.isEmpty()) text += "\n";
            text += line;
        }

        subtitles.append({start, end, text});
    }
    return subtitles;
}

// "00:01:30,500" → 90500 毫秒
qint64 parseTimestamp(const QString &ts)
{
    QStringList timeParts = ts.split(":");
    int hours = timeParts[0].toInt();
    int minutes = timeParts[1].toInt();
    QString secMs = timeParts[2];
    QStringList secParts = secMs.split(",");
    int seconds = secParts[0].toInt();
    int ms = secParts.size() > 1 ? secParts[1].toInt() : 0;
    return hours * 3600000 + minutes * 60000 + seconds * 1000 + ms;
}
```

同步逻辑用 `positionChanged` 信号驱动。每次位置更新时，二分查找当前时间对应的字幕条目：

```cpp
connect(player, &QMediaPlayer::positionChanged, [=](qint64 pos) {
    auto it = std::lower_bound(subtitles.begin(), subtitles.end(), pos,
        [](const SubtitleEntry &e, qint64 p) { return e.startMs < p; });

    // lower_bound 找到第一个 startMs >= pos 的条目
    // 实际匹配的可能是前一个条目（如果 pos 在它的 endMs 之前）
    if (it != subtitles.begin()) {
        --it;
        if (pos >= it->startMs && pos <= it->endMs) {
            subtitleLabel->setText(it->text);
            return;
        }
    }
    subtitleLabel->clear();
});
```

## 4. 踩坑预防

第一个坑是 `metaDataChanged` 信号在某些格式上不触发。MP3 和 MP4 的元数据解析比较可靠，但 OGG、FLAC、WMA 等格式的支持取决于平台的媒体后端。如果你发现某些文件的元数据读不到，先用其他播放器确认文件确实有元数据，再考虑是 Qt 后端的兼容性问题。

第二个坑是 `setPosition()` 后 `positionChanged` 不立即触发。Seek 是异步的——底层框架需要解码到目标位置才能更新 position。如果你在 `setPosition()` 后立刻读 `position()`，拿到的可能还是旧值。正确做法是等 `positionChanged` 信号确认 seek 完成。

## 5. 练习项目

练习项目：带字幕的视频播放器。支持打开视频文件和对应的 SRT 字幕文件。播放/暂停、进度拖动、音量调节、上一个/下一个文件切换。字幕覆盖在视频下方。

完成标准：能播放 MP4 文件并同步显示 SRT 字幕、进度拖动后字幕正确同步、切换文件后字幕更新、多行字幕正确换行显示。

## 6. 官方文档参考链接

[Qt 文档 · QMediaPlayer](https://doc.qt.io/qt-6/qmediaplayer.html) -- 媒体播放器核心 API，包含播放控制和元数据

[Qt 文档 · QMediaMetaData](https://doc.qt.io/qt-6/qmediametadata.html) -- 媒体元数据定义，包含所有可用的元数据字段键

[Qt 文档 · QAudioOutput](https://doc.qt.io/qt-6/qaudiooutput.html) -- 音频输出设备配置

*（链接已验证，2026-06-11 可访问）*

---

到这里就大功告成了。播放列表、元数据、精确 Seek、SRT 字幕——这四个功能加上入门篇的基础播放能力，一个像模像样的媒体播放器就有了。
