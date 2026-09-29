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

#ifndef CAMERAWINDOW_H
#define CAMERAWINDOW_H

#include <QMainWindow>
#include <QList>

#include <QCameraDevice>

class QCamera;
class QMediaCaptureSession;
class QImageCapture;
class QVideoWidget;
class QComboBox;
class QLabel;

class CameraWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CameraWindow(QWidget *parent = nullptr);

private:
    /// 切换到指定索引的摄像头设备
    void initCamera(int index);

    QList<QCameraDevice> devices_;
    QCamera *camera_ = nullptr;
    QMediaCaptureSession *session_ = nullptr;
    QImageCapture *capture_ = nullptr;
    QVideoWidget *video_widget_ = nullptr;
    QComboBox *camera_combo_ = nullptr;
    QLabel *status_label_ = nullptr;
};

#endif // CAMERAWINDOW_H
