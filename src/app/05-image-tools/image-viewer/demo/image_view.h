/**
 * @file image_view.h
 * @brief 图片显示画布——QImage 加载 + QTransform「缩放/旋转/居中」一次性渲染
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QImage>
#include <QWidget>

/// @brief 图片显示画布。
///
/// 持有原始 QImage，paintEvent 用一个 QTransform 把「平移到图像中心 → 缩放 →
/// 旋转 → 平移到画布中心」合成成一次绘制，不用 QLabel setPixmap——后者每次缩放/
/// 旋转都要重新生成 QPixmap，也讲不清坐标系变换。
///
/// 画布自报 sizeHint = 变换后图像外接尺寸（旋转 90/270 时宽高互换），外层 QScrollArea
/// 据此决定滚动范围：缩放变大 → 画布变大 → 滚动条出现。
class ImageView : public QWidget {
    Q_OBJECT
  public:
    explicit ImageView(QWidget* parent = nullptr);

    /// 加载图片。失败返回 false（错误信息来自 QImageReader::errorString）。
    bool loadImage(const QString& path);
    bool hasImage() const;
    QString errorString() const { return last_error_; }

    /// 缩放系数（1.0 = 原始尺寸，100%）。
    void setZoom(double factor);
    double zoom() const { return zoom_; }
    void zoomIn();  // ×1.25
    void zoomOut(); // ÷1.25

    /// 旋转 ±90 的倍数，内部归一化到 [0,360)。
    void rotateBy(int degrees);
    int rotation() const { return rotation_; }

    /// 适配给定区域（取 min(sx,sy) 保证完整可见）。
    void zoomToFit(const QSize& area);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override { return {40, 40}; }

  signals:
    void transformChanged(double zoom, int rotation);
    void imageLoaded(const QString& path, const QSize& imageSize);
    void loadFailed(const QString& message);

  protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

  private:
    /// 变换后（缩放+旋转）的图像外接尺寸——旋转 90/270 宽高互换。
    QSize transformedSize() const;
    void applyAndNotify();

    QImage image_;
    QString last_error_;
    double zoom_ = 1.0;
    int rotation_ = 0; // 仅 0/90/180/270
};
