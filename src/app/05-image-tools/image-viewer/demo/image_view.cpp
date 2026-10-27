/**
 * @file image_view.cpp
 * @brief ImageView 画布实现——加载 / QTransform 变换 / 滚轮缩放
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "image_view.h"

#include <algorithm>
#include <cmath>

#include <QImageReader>
#include <QPaintEvent>
#include <QPainter>
#include <QTransform>
#include <QWheelEvent>

ImageView::ImageView(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_OpaquePaintEvent); // paintEvent 自己填满背景，省一次系统擦除
}

// ============================================================================
// 加载
// ============================================================================
bool ImageView::loadImage(const QString& path) {
    QImageReader reader(path);
    reader.setAutoTransform(true); // 自动按 EXIF 方向校正（手机拍照常见）
    QImage img = reader.read();
    if (img.isNull()) {
        last_error_ = reader.errorString();
        emit loadFailed(last_error_);
        return false;
    }
    image_ = std::move(img);
    last_error_.clear();
    applyAndNotify();
    emit imageLoaded(path, image_.size());
    return true;
}

bool ImageView::hasImage() const {
    return !image_.isNull();
}

// ============================================================================
// 缩放 / 旋转
// ============================================================================
void ImageView::setZoom(double factor) {
    factor = std::clamp(factor, 0.02, 30.0);
    if (qFuzzyCompare(factor, zoom_)) {
        return;
    }
    zoom_ = factor;
    applyAndNotify();
}

void ImageView::zoomIn() {
    setZoom(zoom_ * 1.25);
}

void ImageView::zoomOut() {
    setZoom(zoom_ / 1.25);
}

void ImageView::rotateBy(int degrees) {
    // 归一化到 [0,360)，处理负角度（rotateBy(-90) 回退一格）
    int r = rotation_ + degrees;
    r %= 360;
    if (r < 0) {
        r += 360;
    }
    if (r == rotation_) {
        return;
    }
    rotation_ = r;
    applyAndNotify();
}

void ImageView::zoomToFit(const QSize& area) {
    if (image_.isNull() || area.isEmpty()) {
        return;
    }
    // 旋转后基准尺寸要随之转置（90/270 宽高互换）——基准算错 fit 就错（见踩坑②）
    const QSize base = (rotation_ % 180 == 0) ? image_.size() : image_.size().transposed();
    const double sx = static_cast<double>(area.width()) / base.width();
    const double sy = static_cast<double>(area.height()) / base.height();
    setZoom(std::min(sx, sy)); // 取较小者保证整张可见
}

QSize ImageView::transformedSize() const {
    if (image_.isNull()) {
        return {0, 0};
    }
    QSize s = image_.size();
    if (rotation_ % 180 != 0) {
        s.transpose(); // 原地转置
    }
    // QSize 与 qreal 相乘四舍五入到 int；expandedTo 兜底极小图/0
    return (s * zoom_).expandedTo(minimumSizeHint());
}

QSize ImageView::sizeHint() const {
    return transformedSize();
}

void ImageView::applyAndNotify() {
    updateGeometry(); // sizeHint 变了（zoom/rotation 变），通知外层布局
    update();         // 异步重绘（不 repaint）
    emit transformChanged(zoom_, rotation_);
}

// ============================================================================
// 自绘：一个 QTransform 合成「平移到图像中心 → 缩放 → 旋转 → 平移到画布中心」
// QTransform 的 translate/rotate/scale 是右乘累积，所以「书写顺序」与「作用顺序」相反
// （代码最后一行 translate 先作用于点）——见成品导览踩坑③
// ============================================================================
void ImageView::paintEvent(QPaintEvent*) {
    QPainter p(this);

    // 整片背景填一次（WA_OpaquePaintEvent 下系统不替我们擦）
    p.fillRect(rect(), QColor(30, 30, 30));

    if (image_.isNull()) {
        p.setPen(QColor(150, 150, 150));
        p.drawText(rect(), Qt::AlignCenter, "Open an image (Ctrl+O)");
        return;
    }

    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // 画布尺寸 = transformedSize（外层在 transformChanged 里 resize），画布中心即图像中心
    QTransform t;
    t.translate(width() / 2.0, height() / 2.0);                 // 4. 落到画布中心
    t.rotate(rotation_);                                        // 3. 绕原点旋转
    t.scale(zoom_, zoom_);                                      // 2. 缩放
    t.translate(-image_.width() / 2.0, -image_.height() / 2.0); // 1. 图像中心移到原点

    p.setTransform(t);
    p.drawImage(QPointF(0, 0), image_);
}

// ============================================================================
// 滚轮缩放（以画布中心为锚；「以鼠标位置为锚」留给手搓手册进阶挑战）
// ============================================================================
void ImageView::wheelEvent(QWheelEvent* event) {
    const int delta = event->angleDelta().y();
    if (delta == 0) {
        return;
    }
    const double steps = delta / 120.0;         // 滚轮每 120 = 一档
    const double factor = std::pow(1.1, steps); // 每档 ×1.1
    setZoom(zoom_ * factor);
    event->accept();
}
