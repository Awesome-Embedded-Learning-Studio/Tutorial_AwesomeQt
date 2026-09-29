#include "imagedisplaywidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QLinearGradient>

ImageDisplayWidget::ImageDisplayWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(200, 150);
    // 默认生成一张演示图片，不至于打开就是空白
    generateDefaultImage();
}

void ImageDisplayWidget::setPixmap(const QPixmap &pixmap)
{
    m_original = pixmap;
    updateScaledCache();
    update();
}

void ImageDisplayWidget::setFitMode(bool fit)
{
    m_fitMode = fit;
    updateScaledCache();
    update();
}

void ImageDisplayWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 浅灰背景，方便看到图片边界
    painter.fillRect(rect(), QColor(245, 245, 245));

    if (m_original.isNull()) {
        painter.setPen(Qt::gray);
        painter.setFont(QFont("Sans", 12));
        painter.drawText(rect(), Qt::AlignCenter, "点击「打开文件」加载图片");
        return;
    }

    if (m_fitMode) {
        // 自适应模式：使用缓存的缩放结果
        if (m_scaled.isNull()) {
            updateScaledCache();
        }
        int x = (width() - m_scaled.width()) / 2;
        int y = (height() - m_scaled.height()) / 2;
        painter.drawPixmap(x, y, m_scaled);
    } else {
        // 原始尺寸模式：居中显示，可能超出边界
        int x = (width() - m_original.width()) / 2;
        int y = (height() - m_original.height()) / 2;
        painter.drawPixmap(x, y, m_original);
    }
}

void ImageDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateScaledCache();
}

void ImageDisplayWidget::generateDefaultImage()
{
    const int size = 300;
    QImage img(size, size, QImage::Format_RGB32);

    // 画棋盘格
    int tileSize = 30;
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            bool isLight = ((x / tileSize) + (y / tileSize)) % 2 == 0;
            if (isLight) {
                img.setPixel(x, y, qRgb(230, 230, 230));
            } else {
                img.setPixel(x, y, qRgb(200, 200, 200));
            }
        }
    }

    // 叠加一个渐变覆盖层
    QImage overlay(size, size, QImage::Format_ARGB32);
    overlay.fill(Qt::transparent);
    QPainter p(&overlay);
    QLinearGradient gradient(0, 0, size, size);
    gradient.setColorAt(0.0, QColor(74, 144, 217, 100));
    gradient.setColorAt(1.0, QColor(142, 68, 173, 100));
    p.fillRect(0, 0, size, size, gradient);

    // 合成
    QPainter cp(&img);
    cp.drawImage(0, 0, overlay);
    cp.setPen(Qt::white);
    cp.setFont(QFont("Sans", 16, QFont::Bold));
    cp.drawText(QRect(0, 0, size, size), Qt::AlignCenter,
                 "AwesomeQt\n图片查看器演示");

    m_original = QPixmap::fromImage(img);
    updateScaledCache();
}

void ImageDisplayWidget::updateScaledCache()
{
    if (m_original.isNull()) {
        m_scaled = QPixmap();
        return;
    }
    m_scaled = m_original.scaled(width() - 20, height() - 20,
                                  Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation);
}
