# 现代Qt开发教程（新手篇）5.5——QtMultimedia 摄像头采集基础

## 1. 前言：摄像头采集没有想象中那么玄乎

说到摄像头采集，很多开发者的第一反应是"这玩意得用 DirectShow 或者 V4L2 吧"。确实，如果你直接操作底层 API，光是初始化设备、协商格式、管理缓冲区就能耗掉你一周时间。但 Qt 的 QtMultimedia 模块已经把这些脏活全包了——枚举设备、打开摄像头、实时预览、截图保存，整个流程加起来不超过五十行代码。

Qt 6 的摄像头架构和 Qt 5 完全不同。旧版的 `QCamera` 直接输出到 `QCameraViewfinder`，API 比较黑盒，想对视频帧做处理非常困难。新版把采集管线拆成了 `QCamera`（控制设备）、`QMediaCaptureSession`（管线管理）、`QVideoWidget`（预览显示）、`QImageCapture`（截图捕获）四个独立的类，每个类职责明确，你可以灵活地组装它们。这篇我们要做的是从枚举系统摄像头开始，一步步搭建预览 + 截图的完整采集管线。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 `Qt6::Multimedia`、`Qt6::MultimediaWidgets` 和 `Qt6::Widgets` 三个模块。CMake 配置和上一篇播放器完全一样：`find_package(Qt6 REQUIRED COMPONENTS Core Multimedia MultimediaWidgets Widgets)`。摄像头后端同样是平台相关的——Windows 使用 WMF，macOS 使用 AVFoundation，Linux 使用 V4L2。Linux 上需要确认当前用户有 `/dev/video*` 设备的读写权限（通常需要把用户加入 `video` 组）。

关于摄像头权限：Windows 和 Linux 上默认不需要额外的权限申请——只要设备可用就能直接打开。macOS 上需要在应用的 `Info.plist` 里声明 `NSCameraUsageDescription`，否则系统会直接拒绝访问。如果你打算把程序打包发布到 macOS，别忘了这一步。移动端（Android / iOS）的权限机制更复杂，但本篇聚焦桌面端，就不展开了。

## 3. 核心概念讲解

### 3.1 枚举摄像头设备——QMediaDevices::videoInputs

在做任何摄像头操作之前，你需要知道系统上有多少个摄像头、它们分别叫什么。`QMediaDevices` 提供了静态方法 `videoInputs()` 来枚举所有可用的视频输入设备：

```cpp
#include <QMediaDevices>

const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
for (const QCameraDevice &device : cameras) {
    qDebug() << "Camera:" << device.description()
             << "id:" << device.id();
}

if (cameras.isEmpty()) {
    qWarning() << "No camera found!";
    return;
}
```

`QCameraDevice` 封装了摄像头的设备信息。`description()` 返回人类可读的设备名称（比如"Integrated Webcam"或"USB Camera"），`id()` 返回设备的唯一标识符。`QMediaDevices::defaultVideoInput()` 返回系统默认的摄像头——通常是笔记本的内置摄像头。

`QCameraDevice` 还提供了 `photoResolutions()` 和 `videoFormats()` 两个方法，分别返回设备支持的拍照分辨率列表和视频格式列表。在高级场景中你可以用这些信息来选择特定的分辨率和帧率，但对于基础用法，Qt 会自动选择设备的默认配置。

有一个需要注意的地方：`QMediaDevices::videoInputs()` 是一次性的快照调用。如果用户在程序运行过程中插拔了 USB 摄像头，这个列表不会自动更新。你需要监听 `QMediaDevices::videoInputsChanged` 信号来响应设备变化：

```cpp
connect(QMediaDevices::instance(), &QMediaDevices::videoInputsChanged, this, [this]() {
    qDebug() << "Video input devices changed!";
    // 重新加载设备列表
});
```

### 3.2 摄像头预览——QCamera + QMediaCaptureSession + QVideoWidget

Qt 6 的摄像头管线由三个核心类组成：`QCamera` 负责控制摄像头设备（启动、停止、参数设置），`QMediaCaptureSession` 是管线的中枢——它把摄像头、预览控件、截图组件连接在一起，`QVideoWidget` 负责实时画面显示。

这三者的组装方式如下：

```cpp
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoWidget>

// 获取默认摄像头设备
const QCameraDevice cameraDevice = QMediaDevices::defaultVideoInput();

// 创建摄像头实例
QCamera *camera = new QCamera(cameraDevice, this);

// 创建采集会话（管线中枢）
QMediaCaptureSession *session = new QMediaCaptureSession(this);
session->setCamera(camera);  // 把摄像头挂到会话上

// 创建预览控件
QVideoWidget *viewfinder = new QVideoWidget(this);
session->setVideoOutput(viewfinder);  // 把预览挂到会话上

// 启动摄像头
camera->start();
```

`QMediaCaptureSession` 是整个管线的关键。你可以把它理解成一个"交换机"——摄像头采集的视频帧经过它分发到各个输出端。预览画面走 `setVideoOutput()`，截图走 `setImageCapture()`，视频录制走 `setRecorder()`。这种设计让你可以同时挂多个输出——比如一边预览一边录制一边截图，互不干扰。

`QCamera` 的 `start()` 和 `stop()` 控制摄像头的启停。`start()` 之后预览画面会立刻出现在 `QVideoWidget` 上。摄像头的启动需要一点时间（通常是几百毫秒），你可以通过 `QCamera::activeChanged` 信号来确认摄像头是否已经真正开始工作：

```cpp
connect(camera, &QCamera::activeChanged, this, [](bool active) {
    qDebug() << "Camera active:" << active;
});
```

如果摄像头启动失败（比如被其他程序占用、设备不存在），`QCamera::errorOccurred` 信号会触发：

```cpp
connect(camera, &QCamera::errorOccurred, this,
    [](QCamera::Error error, const QString &errorString) {
        qWarning() << "Camera error:" << error << errorString;
    });
```

### 3.3 截图保存——QImageCapture

截图功能通过 `QImageCapture` 实现。它需要挂到 `QMediaCaptureSession` 上，和摄像头、预览控件一起工作：

```cpp
#include <QImageCapture>

QImageCapture *capture = new QImageCapture(this);
session->setImageCapture(capture);  // 挂到采集会话上

// 触发截图
capture->capture();  // 拍一张照片

// 截图完成信号
connect(capture, &QImageCapture::imageCaptured, this,
    [](int id, const QImage &image) {
        // image 就是截取到的帧
        image.save("capture.png");
        qDebug() << "Image captured, id:" << id;
    });
```

`capture()` 是异步的——调用后它会从摄像头的下一帧中截取画面，截图完成后通过 `imageCaptured` 信号返回 `QImage` 对象。`id` 参数是一个递增的整数，用来标识每次截图请求，在你连续拍多张时可以区分哪张是哪张。

`QImageCapture` 还有一个 `imageSaved` 信号，它在图片保存到磁盘后触发（需要先用 `captureToFile()` 指定保存路径）。不过对于基础用法，用 `imageCaptured` 信号拿到 `QImage` 然后手动 `save()` 更灵活——你可以先在界面上显示预览，让用户确认后再保存。

截图的质量和分辨率可以通过 `QImageCapture` 的属性来控制：

```cpp
capture->setQuality(QImageCapture::VeryHighQuality);
capture->setFileFormat(QImageCapture::PNG);
```

`quality` 影响的是 JPEG 等有损格式的压缩质量（对 PNG 无效）。`fileFormat` 设置保存格式，支持 JPEG、PNG、WebP 等。如果你需要在代码里手动 `save()` QImage，可以忽略 `fileFormat`，在 `save()` 时指定：

```cpp
image.save("photo.jpg", "JPEG", 95);  // 格式 + 质量
```

### 3.4 多摄像头切换

如果你的系统有多个摄像头（比如笔记本有前置 + 外接 USB），可以通过 `QCameraDevice` 列表让用户选择。切换摄像头的方式是重新设置 `QMediaCaptureSession` 的 camera：

```cpp
void switchCamera(const QCameraDevice &device)
{
    camera_->stop();
    delete camera_;

    camera_ = new QCamera(device, this);
    session_->setCamera(camera_);
    camera_->start();
}
```

切换过程需要先停止旧摄像头，销毁旧的 `QCamera` 对象，然后创建新的 `QCamera` 并重新挂到 `QMediaCaptureSession` 上。`QMediaCaptureSession` 的 `setImageCapture()` 和 `setVideoOutput()` 不需要重新设置——它们是绑定在 session 上的，不随摄像头切换而变化。这就是 session 作为管线中枢的好处，输出端保持不变，只换输入源。

## 4. 综合示例：摄像头预览与截图工具

把前面学的串起来，我们写一个包含摄像头预览、设备切换、截图保存的完整工具。程序使用 `QCamera` + `QMediaCaptureSession` + `QVideoWidget` 构建采集管线，通过 `QImageCapture` 实现截图功能，使用 `QComboBox` 让用户选择摄像头设备。

完整代码见 `examples/beginner/05-other-modules/05-qtmultimedia-camera-beginner/`，下面是关键部分的讲解。

CMake 配置和上一篇一样：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Multimedia MultimediaWidgets Widgets)
# ...
target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Multimedia Qt6::MultimediaWidgets Qt6::Widgets)
```

主程序结构：顶部是 `QVideoWidget` 用于实时预览，底部是控制面板——包含摄像头选择下拉框、截图按钮、状态标签。

管线组装的核心代码：

```cpp
// 获取摄像头列表
const QList<QCameraDevice> devices = QMediaDevices::videoInputs();
if (devices.isEmpty()) {
    qWarning() << "No camera device found";
    return;
}

// 组装管线
camera_ = new QCamera(devices.first(), this);
session_ = new QMediaCaptureSession(this);
capture_ = new QImageCapture(this);

session_->setCamera(camera_);
session_->setVideoOutput(video_widget_);
session_->setImageCapture(capture_);

camera_->start();
```

截图保存的处理：

```cpp
connect(capture_, &QImageCapture::imageCaptured, this,
    [this](int id, const QImage &image) {
        QString filename = QString("capture_%1.png").arg(id);
        if (image.save(filename)) {
            status_label_->setText("已保存: " + filename);
        }
    });
```

摄像头切换的处理：

```cpp
connect(camera_combo_, &QComboBox::currentIndexChanged, this,
    [this, devices](int index) {
        if (index < 0 || index >= devices.size()) return;
        camera_->stop();
        delete camera_;

        camera_ = new QCamera(devices[index], this);
        session_->setCamera(camera_);
        camera_->start();
    });
```

设备热插拔的处理——监听 `videoInputsChanged` 信号更新下拉框：

```cpp
connect(QMediaDevices::instance(), &QMediaDevices::videoInputsChanged, this,
    [this]() {
        camera_combo_->clear();
        const QList<QCameraDevice> devs = QMediaDevices::videoInputs();
        for (const auto &dev : devs) {
            camera_combo_->addItem(dev.description());
        }
    });
```

运行程序后，`QVideoWidget` 区域会立刻显示摄像头预览画面。点击"截图"按钮，程序会从当前视频帧中截取一张图片并保存到当前目录。如果系统有多个摄像头，通过下拉框可以实时切换预览。

## 5. 练习项目

练习项目：带拍照倒计时的简易相机应用。

我们要做一个支持 3 秒倒计时拍照的相机应用。用户点击"拍照"后，界面上显示 3-2-1 倒计时，倒计时结束后自动截图并显示预览缩略图。

完成标准是这样的：使用 `QTimer` 实现 3 秒倒计时，倒计时数字叠加显示在视频预览区域上；倒计时期间禁用拍照按钮，防止重复触发；截图完成后在右侧面板显示最近 4 张照片的缩略图（使用 `QLabel` + `QPixmap::scaled()` 缩放显示）；缩略图可点击，点击后在独立窗口中显示原始大小的图片。

几个实现提示：倒计时叠加可以用 `QLabel` 覆盖在 `QVideoWidget` 上方（通过 `QVBoxLayout` 或手动 `setGeometry`），设置大字号和半透明背景；缩略图用 `QPixmap::fromImage(image).scaled(160, 120, Qt::KeepAspectRatio)` 缩放；点击缩略图弹出新 `QLabel` 窗口显示原图——用 `QLabel::setPixmap()` + `QLabel::show()` 即可。

## 6. 官方文档参考

[Qt 文档 · QtMultimedia 模块](https://doc.qt.io/qt-6/qtmultimedia-index.html) -- 多媒体模块总览

[Qt 文档 · QCamera](https://doc.qt.io/qt-6/qcamera.html) -- 摄像头控制类

[Qt 文档 · QMediaCaptureSession](https://doc.qt.io/qt-6/qmediacapturesession.html) -- 采集会话管线

[Qt 文档 · QImageCapture](https://doc.qt.io/qt-6/qimagecapture.html) -- 图片捕获类

[Qt 文档 · QMediaDevices](https://doc.qt.io/qt-6/qmediadevices.html) -- 媒体设备枚举

*（链接已验证，2026-04-23 可访问）*

---

到这里就大功告成了！Qt 6 的摄像头采集管线设计得比 Qt 5 优雅得多：QCamera 管设备，QMediaCaptureSession 做管线中枢，QVideoWidget 显示画面，QImageCapture 拍照。四个类各司其职，通过 session 串联起来。掌握了这套架构之后，后面想做视频录制（加 `QMediaRecorder`）、想加实时滤镜（自定义 `QVideoSink` 处理帧数据），都是在同一个管线上加组件，底层逻辑不变。

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
