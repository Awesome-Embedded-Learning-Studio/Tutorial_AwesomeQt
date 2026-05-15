/// @file    pixel_widget.cpp
/// @brief   QImage 像素级操作演示控件的实现。
///
/// 实现三种图像处理：渐变生成、颜色反转、灰度转换。分别使用
/// setPixelColor 和 scanLine 两种像素操作方式，并使用
/// convertToFormat 演示格式转换。
///
/// 对应教程：进阶层 02-QtGui/03-图像处理进阶。

#include "pixel_widget.h"

#include <QPainter>
#include <QRgb>

PixelWidget::PixelWidget(QWidget* parent) : QWidget(parent)
{
    setWindowTitle("QImage Pixel Manipulation Demo");
    // 窗口宽度容纳三张 400px 图像 + 间距，高度 300px + 标签区域
    setMinimumSize(kImageWidth * 3 + kPadding * 4, kImageHeight + 120);

    // 在构造函数中完成图像处理，避免每次 paintEvent 重复计算
    m_originalImage = generateGradientImage();
    m_invertedImage = invertColors(m_originalImage);
    m_grayscaleImage = convertToGrayscale(m_originalImage);
}

QImage PixelWidget::generateGradientImage()
{
    // 创建 RGB32 格式图像：每像素 4 字节（0xFFRRGGBB），最适合像素操作
    QImage image(kImageWidth, kImageHeight, QImage::Format_RGB32);

    // setPixelColor 逐像素写入：红通道水平渐变，绿通道垂直渐变
    for (int y = 0; y < kImageHeight; ++y)
    {
        const int green = static_cast<int>(
            255.0 * static_cast<double>(y) / static_cast<double>(kImageHeight - 1));

        for (int x = 0; x < kImageWidth; ++x)
        {
            const int red = static_cast<int>(
                255.0 * static_cast<double>(x) / static_cast<double>(kImageWidth - 1));
            // QColor 构造时 alpha 默认 255（不透明），无需显式指定
            image.setPixelColor(x, y, QColor(red, green, 0));
        }
    }

    return image;
}

QImage PixelWidget::invertColors(const QImage& source)
{
    // 深拷贝源图像，避免修改原始数据（QImage 使用 COW，此处触发实际复制）
    QImage result = source;

    // scanLine 返回指向第 y 行像素数据的指针，直接操作内存比 setPixelColor 快
    for (int y = 0; y < result.height(); ++y)
    {
        // QRgb 即 unsigned int，Format_RGB32 内存布局为 0xFFBBGGRR（小端序）
        auto* scanLine = reinterpret_cast<QRgb*>(result.scanLine(y));

        for (int x = 0; x < result.width(); ++x)
        {
            const QRgb pixel = scanLine[x];
            // 颜色反转：255 减去各通道原始值，alpha 保持不变
            const int r = 255 - qRed(pixel);
            const int g = 255 - qGreen(pixel);
            const int b = 255 - qBlue(pixel);
            scanLine[x] = qRgb(r, g, b);
        }
    }

    return result;
}

QImage PixelWidget::convertToGrayscale(const QImage& source)
{
    // 先用 convertToFormat 转为 Grayscale8 格式，演示格式转换 API
    // Format_Grayscale8 每像素 1 字节，比 RGB32 节省 75% 内存
    QImage gray8 = source.convertToFormat(QImage::Format_Grayscale8);

    // 再转回 RGB32 用于统一显示（灰度图仍可正常绘制，此步演示 convertToFormat）
    // 保持原始图像尺寸和格式，方便三面板统一绘制
    QImage result(kImageWidth, kImageHeight, QImage::Format_RGB32);

    for (int y = 0; y < kImageHeight; ++y)
    {
        const auto* srcScan = gray8.constScanLine(y);
        auto* dstScan = reinterpret_cast<QRgb*>(result.scanLine(y));

        for (int x = 0; x < kImageWidth; ++x)
        {
            // Grayscale8 格式：直接读取单字节灰度值
            const uchar grayValue = srcScan[x];
            // ITU-R BT.601 luminance 公式：Y = 0.299R + 0.587G + 0.114B
            // convertToFormat 内部已使用此公式，此处直接复制灰度值到三通道
            dstScan[x] = qRgb(grayValue, grayValue, grayValue);
        }
    }

    return result;
}

void PixelWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // 填充背景
    painter.fillRect(rect(), QColor(245, 245, 245));

    // 绘制标题
    painter.setPen(QColor(50, 50, 50));
    QFont titleFont("Sans", 14, QFont::Bold);
    painter.setFont(titleFont);
    painter.drawText(QPointF(kPadding, 25),
                     "QImage Pixel Manipulation: setPixelColor / scanLine / convertToFormat");

    // 三张图像的水平起始位置
    const int imageY = 40;  // 图像绘制起始 Y 坐标（标题下方）
    const int x1 = kPadding;
    const int x2 = kPadding + kImageWidth + kPadding;
    const int x3 = kPadding + (kImageWidth + kPadding) * 2;

    // 绘制三张图像及标签
    drawImageWithLabel(painter, m_originalImage, x1, imageY,
                       "Original (setPixelColor)");
    drawImageWithLabel(painter, m_invertedImage, x2, imageY,
                       "Inverted (scanLine)");
    drawImageWithLabel(painter, m_grayscaleImage, x3, imageY,
                       "Grayscale (convertToFormat)");

    // 在每张图像下方显示格式信息
    const int formatY = imageY + kImageHeight + 35;
    drawFormatInfo(painter, m_originalImage, x1, formatY);
    drawFormatInfo(painter, m_invertedImage, x2, formatY);
    drawFormatInfo(painter, m_grayscaleImage, x3, formatY);
}

void PixelWidget::drawImageWithLabel(QPainter& painter, const QImage& image,
                                     int x, int y, const QString& label)
{
    // 绘制图像边框
    QPen borderPen(QColor(180, 180, 180), 1);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(x - 1, y - 1, image.width() + 2, image.height() + 2);

    // 绘制图像
    painter.drawImage(x, y, image);

    // 绘制标签
    painter.setPen(QColor(70, 70, 70));
    QFont labelFont("Sans", 10);
    painter.setFont(labelFont);
    painter.drawText(QPointF(x, y + image.height() + 18), label);
}

void PixelWidget::drawFormatInfo(QPainter& painter, const QImage& image,
                                 int x, int y)
{
    // 将 QImage::Format 枚举值转为可读字符串
    QString formatName;
    switch (image.format())
    {
    case QImage::Format_RGB32:
        formatName = "Format_RGB32 (4 bytes/pixel)";
        break;
    case QImage::Format_Grayscale8:
        formatName = "Format_Grayscale8 (1 byte/pixel)";
        break;
    default:
        formatName = QString("Format %1").arg(static_cast<int>(image.format()));
        break;
    }

    painter.setPen(QColor(120, 120, 120));
    QFont infoFont("monospace", 9);
    painter.setFont(infoFont);
    painter.drawText(QPointF(x, y), formatName);

    // 显示像素位数（bitsPerPixel）和是否有 alpha 通道
    const bool hasAlpha = image.hasAlphaChannel();
    painter.drawText(
        QPointF(x, y + 16),
        QString("Depth: %1 bpp | Alpha: %2")
            .arg(image.depth())
            .arg(hasAlpha ? "yes" : "no"));
}
