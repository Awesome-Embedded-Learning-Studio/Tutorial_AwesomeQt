---
title: "5.5 摄像头进阶：视频录制、帧处理、滤镜"
description: "入门篇我们把 QCamera + QMediaCaptureSession 的基本预览流程跑通了——打开摄像头、显示预览画面、拍照保存。做视频通话预览或者扫码器的画面采集确实够用了。但如果你要做视频录制、实时帧处理（比如人脸检测、二维码识别）、或者给画面加滤镜，入门篇的那套方案就不够了。"
---

# 现代Qt开发教程（进阶篇）5.5——摄像头进阶：视频录制、帧处理、滤镜

## 1. 前言 / 从「预览」到「处理」

入门篇我们把 QCamera + QMediaCaptureSession 的基本预览流程跑通了——打开摄像头、显示预览画面、拍照保存。做视频通话预览或者扫码器的画面采集确实够用了。但如果你要做视频录制、实时帧处理（比如人脸检测、二维码识别）、或者给画面加滤镜，入门篇的那套方案就不够了。

Qt 6 的多媒体架构把功能拆得很细：QCamera 管设备、QMediaCaptureSession 管会话、QVideoWidget 管显示、QMediaRecorder 管录制、QVideoSink 管帧数据。这些组件通过组合连接，你可以根据需要灵活搭配。

这篇我们一起来把视频录制、QVideoSink 帧获取与处理、以及简单的实时滤镜这三个核心能力拆干净。

## 2. 环境说明

本文档基于 Qt 6.4+ 编写，使用 C++17 标准和 CMake 3.26+ 构建系统。本篇依赖 Qt6::Multimedia 和 Qt6::MultimediaWidgets 模块。摄像头设备在 Linux 上使用 V4L2（确保用户有 `/dev/video*` 的访问权限），Windows 使用 DirectShow/Media Foundation，macOS 使用 AVFoundation。WSL2 环境下摄像头访问需要 USB 设备透传配置。

## 3. 核心概念讲解

### 3.1 视频录制——QMediaRecorder 的配置与控制

QMediaRecorder 负责把 QMediaCaptureSession 中的音视频流编码写入文件。它和 QCamera 通过 CaptureSession 连接——你在 session 上设置 recorder，session 自动把摄像头的视频帧送给 recorder。

```cpp
class VideoRecorder : public QWidget
{
    Q_OBJECT
public:
    VideoRecorder(QWidget *parent = nullptr) : QWidget(parent)
    {
        // 初始化摄像头
        camera_ = new QCamera(this);
        captureSession_.setCamera(camera_);

        // 预览显示
        auto *videoWidget = new QVideoWidget(this);
        captureSession_.setVideoOutput(videoWidget);

        // 录制器
        recorder_ = new QMediaRecorder(this);
        captureSession_.setRecorder(recorder_);

        // 配置录制参数
        QMediaFormat format;
        format.setFileFormat(QMediaFormat::MPEG4);
        format.setVideoCodec(QMediaFormat::VideoCodec::H264);
        recorder_->setMediaFormat(format);

        recorder_->setQuality(QMediaRecorder::HighQuality);
        recorder_->setOutputLocation(
            QUrl::fromLocalFile("recording.mp4"));

        // UI 布局
        auto *layout = new QVBoxLayout(this);
        layout->addWidget(videoWidget);

        auto *btnLayout = new QHBoxLayout();
        auto *startBtn = new QPushButton("Start Recording");
        auto *stopBtn = new QPushButton("Stop");
        stopBtn->setEnabled(false);
        btnLayout->addWidget(startBtn);
        btnLayout->addWidget(stopBtn);
        layout->addLayout(btnLayout);

        // 信号连接
        connect(startBtn, &QPushButton::clicked, this, [=]() {
            recorder_->record();
            startBtn->setEnabled(false);
            stopBtn->setEnabled(true);
        });

        connect(stopBtn, &QPushButton::clicked, this, [=]() {
            recorder_->stop();
            startBtn->setEnabled(true);
            stopBtn->setEnabled(false);
        });

        connect(recorder_, &QMediaRecorder::recorderStateChanged,
                this, [=](QMediaRecorder::RecorderState state) {
            qDebug() << "Recorder state:" << state;
        });

        connect(recorder_, &QMediaRecorder::errorOccurred,
                this, [=](QMediaRecorder::Error error,
                          const QString &errorString) {
            qDebug() << "Recorder error:" << error << errorString;
        });

        camera_->start();
    }

private:
    QCamera *camera_;
    QMediaCaptureSession captureSession_;
    QMediaRecorder *recorder_;
};
```

`QMediaFormat` 控制输出格式。MPEG4 + H264 是兼容性最好的组合——几乎所有播放器都支持。如果你需要更小的文件体积，可以选 WebM + VP9。但注意不是所有平台的后端都支持所有编码器——`QMediaFormat::supportedFormats()` 可以查询当前平台支持的格式列表。

录制器还有音频支持——如果你要录制带声音的视频，需要在 CaptureSession 上额外设置一个 QAudioInput 连接到麦克风。`captureSession.setAudioInput(audioInput)` 即可。

### 3.2 QVideoSink——获取原始帧数据

QVideoSink 是 Qt 6 多媒体架构中新增的关键组件。它从视频管线中「窃取」原始帧数据，让你能在应用层处理每一帧。这对于人脸检测、二维码识别、运动检测等需要逐帧分析的场景至关重要。

```cpp
auto *sink = new QVideoSink(this);
captureSession_.setVideoSink(sink);

connect(sink, &QVideoSink::videoFrameChanged,
        this, &VideoProcessor::processFrame);
```

`videoFrameChanged` 信号在每个新帧到达时触发，参数是一个 `QVideoFrame`。你可以从中提取像素数据：

```cpp
void processFrame(const QVideoFrame &frame)
{
    // 把帧转换为 QImage（注意：这里会做格式转换，有性能开销）
    QImage image = frame.toImage();
    if (image.isNull()) return;

    // 处理 QImage：比如检测二维码、分析颜色分布等
    // ...

    // 如果需要把处理后的帧显示出来
    QVideoFrame processedFrame = QVideoFrame(
        QImage2QVideoFrame(processedImage));
    // 送到另一个 QVideoSink 或 QVideoWidget
}
```

`QVideoFrame::toImage()` 是最方便的帧提取方式，但它的性能开销不小——每次调用都做一次格式转换和内存拷贝。在 30fps 的视频流中，每秒 30 次 `toImage()` 是可以接受的；但如果摄像头分辨率很高（4K），每次转换可能要几十毫秒，会拖慢帧率。

更高性能的做法是直接访问帧的原始数据：

```cpp
if (frame.map(QVideoFrame::ReadOnly)) {
    // 获取原始像素数据指针
    const uchar *data = frame.bits(0);
    int bytesPerLine = frame.bytesPerLine(0);
    int width = frame.width();
    int height = frame.height();

    // 直接操作原始数据（格式可能是 YUV、NV12、BGRA 等）
    // ...

    frame.unmap();
}
```

`map()` 把帧数据映射到 CPU 可访问的内存，`bits()` 返回数据指针。你需要知道帧的像素格式（`frame.pixelFormat()`）来正确解析数据。BGRA 和 NV12 的解析逻辑完全不同。如果你的处理逻辑只依赖 QImage，用 `toImage()` 更简单；如果追求极致性能，直接操作原始数据更快。

### 3.3 实时滤镜——灰度化与边缘检测

有了 QVideoSink 的帧获取能力，我们就可以实现实时滤镜了。基本流程是：获取帧 → 转 QImage → 应用滤镜 → 转回 QVideoFrame → 显示。

```cpp
class FilterProcessor : public QObject
{
    Q_OBJECT
public:
    enum FilterType { None, Grayscale, EdgeDetect, Sepia };
    Q_ENUM(FilterType)

    void setFilter(FilterType type) { currentFilter_ = type; }

signals:
    void frameFiltered(const QVideoFrame &frame);

public slots:
    void processFrame(const QVideoFrame &frame)
    {
        QImage image = frame.toImage();
        if (image.isNull()) return;

        QImage result;
        switch (currentFilter_) {
        case Grayscale:
            result = applyGrayscale(image);
            break;
        case EdgeDetect:
            result = applyEdgeDetect(image);
            break;
        case Sepia:
            result = applySepia(image);
            break;
        default:
            emit frameFiltered(frame);
            return;
        }

        emit frameFiltered(QVideoFrame(result));
    }

private:
    QImage applyGrayscale(const QImage &src)
    {
        QImage dst(src.size(), QImage::Format_Grayscale8);
        for (int y = 0; y < src.height(); ++y) {
            for (int x = 0; x < src.width(); ++x) {
                QColor c = src.pixelColor(x, y);
                int gray = qGray(c.red(), c.green(), c.blue());
                dst.setPixelColor(x, y, QColor(gray, gray, gray));
            }
        }
        return dst;
    }

    QImage applySepia(const QImage &src)
    {
        QImage dst = src.convertToFormat(QImage::Format_RGB32);
        for (int y = 0; y < dst.height(); ++y) {
            QRgb *line = reinterpret_cast<QRgb*>(dst.scanLine(y));
            for (int x = 0; x < dst.width(); ++x) {
                int r = qRed(line[x]);
                int g = qGreen(line[x]);
                int b = qBlue(line[x]);
                int tr = qBound(0, int(r * 0.393 + g * 0.769 + b * 0.189), 255);
                int tg = qBound(0, int(r * 0.349 + g * 0.686 + b * 0.168), 255);
                int tb = qBound(0, int(r * 0.272 + g * 0.534 + b * 0.131), 255);
                line[x] = qRgba(tr, tg, tb, qAlpha(line[x]));
            }
        }
        return dst;
    }

    QImage applyEdgeDetect(const QImage &src)
    {
        // Sobel 算子边缘检测（简化版）
        QImage gray = src.convertToFormat(QImage::Format_Grayscale8);
        QImage dst(gray.size(), QImage::Format_Grayscale8);
        dst.fill(Qt::black);

        for (int y = 1; y < gray.height() - 1; ++y) {
            for (int x = 1; x < gray.width() - 1; ++x) {
                int gx = -qGray(gray.pixel(x-1, y-1))
                         - 2*qGray(gray.pixel(x-1, y))
                         - qGray(gray.pixel(x-1, y+1))
                         + qGray(gray.pixel(x+1, y-1))
                         + 2*qGray(gray.pixel(x+1, y))
                         + qGray(gray.pixel(x+1, y+1));
                int gy = -qGray(gray.pixel(x-1, y-1))
                         - 2*qGray(gray.pixel(x, y-1))
                         - qGray(gray.pixel(x+1, y-1))
                         + qGray(gray.pixel(x-1, y+1))
                         + 2*qGray(gray.pixel(x, y+1))
                         + qGray(gray.pixel(x+1, y+1));
                int mag = qBound(0, int(std::sqrt(gx*gx + gy*gy)), 255);
                dst.setPixel(x, y, mag);
            }
        }
        return dst;
    }

    FilterType currentFilter_ = None;
};
```

说实话，这种逐像素的 QImage 处理在高分辨率下性能很惨。一个 1280×720 的灰度化操作需要遍历 92 万个像素，在 CPU 上每帧可能要 10-20ms——加上 `toImage()` 的转换开销，帧率会从 30fps 掉到 10-15fps。更好的方案是用 GPU 着色器（QOpenGLShaderProgram）做图像处理，但这超出了本篇的范围。对于 640×480 或更低分辨率的场景，CPU 滤镜是可以接受的。

## 4. 踩坑预防

第一个坑是 QVideoFrame 格式兼容性。不同平台的摄像头输出不同的像素格式——Linux V4L2 通常输出 NV12 或 YUYV，Windows 可能输出 BGRA 或 NV12，macOS 输出 NV12。`toImage()` 会自动转换，但 `map()` + 直接访问需要你处理不同格式。如果你的滤镜代码只在特定平台上测试过，部署到其他平台可能因为格式不同而崩溃。

第二个坑是录制和帧处理不能同时高效进行。QVideoSink 和 QMediaRecorder 都从 CaptureSession 获取帧数据——两者同时工作会增加管线复杂度。某些平台后端（特别是 Linux GStreamer）在同时录制和处理帧时可能出现帧率下降。如果需要同时录制和处理，建议先录制原始帧，离线处理，而不是实时处理+实时录制。

第三个坑是摄像头权限。macOS 和 Windows 10+ 都要求应用获取摄像头和麦克风权限。如果你的应用没有在 Info.plist（macOS）或 appxmanifest（Windows UWP）中声明权限，`QCamera` 会报告权限错误。Linux 上确保用户在 `video` 用户组中。

## 5. 练习项目

练习项目：实时滤镜相机。打开摄像头显示预览画面，提供三个滤镜按钮（灰度、复古、边缘检测），点击切换滤镜。支持录制带滤镜效果的视频。支持截图保存当前画面（含滤镜）。

完成标准：三种滤镜实时应用无明显卡顿（640×480 分辨率下至少 15fps）、录制视频包含滤镜效果、截图保存为 PNG 格式、切换滤镜时画面不中断。

提示几个关键点：用 QVideoSink 获取帧，处理后送到另一个 QVideoSink 显示。录制时把处理后的帧送到 recorder 的管线。截图用 `QVideoFrame::toImage()` + `QImage::save()`。

## 6. 官方文档参考链接

[Qt 文档 · QCamera](https://doc.qt.io/qt-6/qcamera.html) -- 摄像头设备管理，包含设备枚举和格式配置

[Qt 文档 · QMediaRecorder](https://doc.qt.io/qt-6/qmediarecorder.html) -- 视频录制 API，包含格式配置和录制控制

[Qt 文档 · QVideoSink](https://doc.qt.io/qt-6/qvideosink.html) -- 视频帧获取，包含 videoFrameChanged 信号

[Qt 文档 · QVideoFrame](https://doc.qt.io/qt-6/qvideoframe.html) -- 视频帧数据结构，包含像素格式和 map/unmap 操作

*（链接已验证，2026-06-11 可访问）*

---

到这里就大功告成了。视频录制、QVideoSink 帧获取、实时滤镜——这三个能力组合起来，你的摄像头应用就从「预览拍照」升级为「视频处理平台」了。
