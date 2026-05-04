/**
 * QtMultimedia 摄像头采集基础示例
 *
 * 本示例演示 QtMultimedia 模块的摄像头采集管线核心用法：
 * 1. QMediaDevices::videoInputs() 枚举摄像头设备
 * 2. QCamera + QMediaCaptureSession + QVideoWidget 实时预览
 * 3. QImageCapture 截图保存
 * 4. 多摄像头切换
 * 5. 设备热插拔监听
 */

#include "CameraWindow.h"

#include <QApplication>
#include <QCamera>
#include <QCameraDevice>
#include <QComboBox>
#include <QHBoxLayout>
#include <QImageCapture>
#include <QLabel>
#include <QMainWindow>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVideoWidget>
#include <QWidget>

// ============================================================================
// 主窗口：摄像头预览与截图工具
// ============================================================================
CameraWindow::CameraWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QtMultimedia 摄像头采集示例");
    resize(800, 600);

    // ---- 检查可用设备 ----
    devices_ = QMediaDevices::videoInputs();
    if (devices_.isEmpty()) {
        QMessageBox::critical(this, "错误", "未检测到摄像头设备");
        return;
    }

    // ---- 创建管线组件 ----
    session_ = new QMediaCaptureSession(this);
    video_widget_ = new QVideoWidget(this);
    capture_ = new QImageCapture(this);

    // 组装管线：摄像头 → 会话 → {预览, 截图}
    session_->setVideoOutput(video_widget_);
    session_->setImageCapture(capture_);
    capture_->setQuality(QImageCapture::VeryHighQuality);

    // 使用第一个摄像头初始化
    initCamera(0);

    // ---- 控制面板 ----
    auto *camera_label = new QLabel("摄像头:", this);
    camera_combo_ = new QComboBox(this);
    for (const auto &dev : devices_) {
        camera_combo_->addItem(dev.description());
    }

    auto *capture_button = new QPushButton("截图", this);
    status_label_ = new QLabel("就绪", this);

    // ---- 布局 ----
    auto *central = new QWidget(this);
    auto *main_layout = new QVBoxLayout(central);

    // 视频预览区域
    main_layout->addWidget(video_widget_, 1);

    // 控制行
    auto *control_layout = new QHBoxLayout();
    control_layout->addWidget(camera_label);
    control_layout->addWidget(camera_combo_, 1);
    control_layout->addWidget(capture_button);
    control_layout->addStretch();
    main_layout->addLayout(control_layout);

    // 状态栏
    main_layout->addWidget(status_label_);

    setCentralWidget(central);

    // ---- 信号槽连接 ----

    // 截图按钮
    connect(capture_button, &QPushButton::clicked, this, [this]() {
        if (!camera_ || !camera_->isActive()) {
            status_label_->setText("摄像头未启动，无法截图");
            return;
        }
        capture_->capture();
        status_label_->setText("正在截图...");
    });

    // 截图完成
    connect(capture_, &QImageCapture::imageCaptured, this,
            [this](int id, const QImage &image) {
                QString filename = QString("capture_%1.png").arg(id);
                if (image.save(filename)) {
                    status_label_->setText("已保存: " + filename);
                } else {
                    status_label_->setText("保存失败: " + filename);
                }
            });

    // 截图错误
    connect(capture_, &QImageCapture::errorOccurred, this,
            [this](int id, QImageCapture::Error error,
                   const QString &error_string) {
                Q_UNUSED(id)
                Q_UNUSED(error)
                status_label_->setText("截图出错: " + error_string);
            });

    // 摄像头切换
    connect(camera_combo_, &QComboBox::currentIndexChanged, this,
            [this](int index) {
                if (index >= 0) {
                    initCamera(index);
                }
            });

    // 设备热插拔监听
    QMediaDevices *mediaDevices = new QMediaDevices(this);
    connect(mediaDevices, &QMediaDevices::videoInputsChanged,
            this, [this]() {
                devices_ = QMediaDevices::videoInputs();
                camera_combo_->blockSignals(true);
                camera_combo_->clear();
                for (const auto &dev : devices_) {
                    camera_combo_->addItem(dev.description());
                }
                camera_combo_->blockSignals(false);

                if (!devices_.isEmpty()) {
                    initCamera(0);
                } else {
                    status_label_->setText("没有可用的摄像头设备");
                    if (camera_) {
                        camera_->stop();
                    }
                }
            });
}

/// 切换到指定索引的摄像头设备
void CameraWindow::initCamera(int index)
{
    if (index < 0 || index >= devices_.size()) {
        return;
    }

    // 停止并销毁旧摄像头
    if (camera_) {
        camera_->stop();
        delete camera_;
        camera_ = nullptr;
    }

    // 创建新摄像头并挂到会话
    camera_ = new QCamera(devices_[index], this);
    session_->setCamera(camera_);

    // 监听摄像头错误
    connect(camera_, &QCamera::errorOccurred, this,
            [this](QCamera::Error, const QString &error_string) {
                status_label_->setText("摄像头错误: " + error_string);
            });

    // 启动
    camera_->start();
    status_label_->setText(
        "摄像头已启动: " + devices_[index].description());
}
