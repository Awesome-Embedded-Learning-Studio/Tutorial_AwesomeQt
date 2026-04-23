#include "pixeldemowidget.h"

#include <QPainter>
#include <QPaintEvent>

#include <cmath>

PixelDemoWidget::PixelDemoWidget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("QImage 像素操作演示");
    resize(500, 300);
    generateDemoImage();
}

void PixelDemoWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_pixmap.isNull()) {
        painter.setPen(Qt::red);
        painter.drawText(rect(), Qt::AlignCenter, "图片生成失败");
        return;
    }

    // 居中显示处理后的图片
    int x = (width() - m_pixmap.width()) / 2;
    int y = (height() - m_pixmap.height()) / 2;
    painter.drawPixmap(x, y, m_pixmap);

    // 在下方标注说明
    painter.setPen(Qt::darkGray);
    painter.setFont(QFont("Sans", 10));
    painter.drawText(QRect(0, height() - 30, width(), 25),
                     Qt::AlignCenter,
                     "左: 原始渐变 | 中: 灰度转换 | 右: 像素替换");
}

void PixelDemoWidget::generateDemoImage()
{
    const int imgW = 150;
    const int imgH = 150;

    // ---- 原始图片：彩虹渐变 ----
    QImage original(imgW, imgH, QImage::Format_RGB32);
    for (int y = 0; y < imgH; ++y) {
        for (int x = 0; x < imgW; ++x) {
            // 用三角函数生成渐变色
            int r = static_cast<int>(127.5 * (1 + std::sin(x * 0.05)));
            int g = static_cast<int>(127.5 * (1 + std::sin(y * 0.05)));
            int b = static_cast<int>(127.5 * (1 + std::sin((x + y) * 0.03)));
            original.setPixel(x, y, qRgb(r, g, b));
        }
    }

    // ---- 灰度转换：逐像素读取 RGB，按加权公式转灰度 ----
    QImage grayscale = original.convertToFormat(QImage::Format_RGB32);
    for (int y = 0; y < grayscale.height(); ++y) {
        for (int x = 0; x < grayscale.width(); ++x) {
            QRgb pixel = grayscale.pixel(x, y);
            // 人眼对绿色最敏感，所以加权系数最高
            int gray = qGray(pixel);
            grayscale.setPixel(x, y, qRgb(gray, gray, gray));
        }
    }

    // ---- 像素替换：把偏红色的区域替换为青色 ----
    QImage replaced = original.convertToFormat(QImage::Format_RGB32);
    for (int y = 0; y < replaced.height(); ++y) {
        for (int x = 0; x < replaced.width(); ++x) {
            QRgb pixel = replaced.pixel(x, y);
            int r = qRed(pixel);
            // 红色分量超过 150 的像素，替换为青色
            if (r > 150) {
                replaced.setPixel(x, y, qRgb(0, 200, 200));
            }
        }
    }

    // ---- 把三张图拼成一张 ----
    int gap = 20;
    int totalW = imgW * 3 + gap * 4;
    int totalH = imgH + gap * 2;
    QImage combined(totalW, totalH, QImage::Format_RGB32);
    combined.fill(Qt::white);

    QPainter cp(&combined);
    cp.drawImage(gap, gap, original);
    cp.drawImage(gap * 2 + imgW, gap, grayscale);
    cp.drawImage(gap * 3 + imgW * 2, gap, replaced);

    // 转 QPixmap 用于高效显示
    m_pixmap = QPixmap::fromImage(combined);
}
