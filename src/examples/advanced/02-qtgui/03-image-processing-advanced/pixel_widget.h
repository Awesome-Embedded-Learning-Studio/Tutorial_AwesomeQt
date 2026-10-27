/// @file    pixel_widget.h
/// @brief   演示 QImage 像素级操作的控件声明。
///
/// 展示三种图像处理效果：原始渐变、颜色反转、灰度转换。
/// 使用 setPixelColor（高层 API）和 scanLine（底层指针）两种方式
/// 操作像素，演示 QImage::Format_RGB32 和 convertToFormat 用法。
///
/// 对应教程：进阶层 02-QtGui/03-图像处理进阶。

#pragma once

#include <QImage>
#include <QWidget>

/// @brief 像素级图像处理演示控件。
///
/// 创建 400x300 的 RGB32 图像，在三列面板中分别展示：
/// - 左侧：原始渐变图像（用 setPixelColor 逐像素写入）
/// - 中间：颜色反转图像（用 scanLine 直接操作内存）
/// - 右侧：灰度转换图像（使用 convertToFormat + luminance 公式）
class PixelWidget : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit PixelWidget(QWidget* parent = nullptr);

protected:
    /// @brief 重写绘制事件，将三张处理后的图像绘制到窗口。
    /// @param[in] event 绘制事件指针，由 Qt 事件循环分发。
    void paintEvent(QPaintEvent* event) override;

private:
    /// @brief 生成原始渐变图像。
/// 使用 setPixelColor 逐像素写入，红通道水平渐变、绿通道垂直渐变。
/// @return 生成的 RGB32 图像。
    /// @note setPixelColor 是高层 API，每次调用触发一次像素格式转换，
    ///       性能低于 scanLine，但代码可读性更好。
    QImage generateGradientImage();

    /// @brief 对源图像执行颜色反转。
    /// @param[in] source 源图像（必须为 Format_RGB32）。
    /// @return 反转后的图像。
    /// @note 使用 scanLine 直接操作 QRgb 内存，比 setPixelColor 快得多。
    QImage invertColors(const QImage& source);

    /// @brief 对源图像执行灰度转换。
    /// @param[in] source 源图像。
    /// @return 灰度格式的图像（Format_Grayscale8）。
    /// @note 使用 convertToFormat 转为灰度格式后用 luminance 公式计算。
    QImage convertToGrayscale(const QImage& source);

    /// @brief 在给定位置绘制图像及标签文字。
    /// @param[in] painter 已配置好的画笔对象。
    /// @param[in] image   要绘制的图像。
    /// @param[in] x       绘制位置的 X 坐标。
    /// @param[in] y       绘制位置的 Y 坐标。
    /// @param[in] label   图像下方的标签文字。
    void drawImageWithLabel(QPainter& painter, const QImage& image,
                            int x, int y, const QString& label);

    /// @brief 在面板下方绘制格式信息。
    /// @param[in] painter 已配置好的画笔对象。
    /// @param[in] image   图像（读取其 format）。
    /// @param[in] x       文本位置 X 坐标。
    /// @param[in] y       文本位置 Y 坐标。
    void drawFormatInfo(QPainter& painter, const QImage& image,
                        int x, int y);

    QImage m_originalImage;       ///< 原始渐变图像
    QImage m_invertedImage;       ///< 颜色反转图像
    QImage m_grayscaleImage;      ///< 灰度转换图像

    static constexpr int kImageWidth = 400;   ///< 图像宽度
    static constexpr int kImageHeight = 300;  ///< 图像高度
    static constexpr int kPadding = 20;       ///< 面板间距
};
