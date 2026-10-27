// QtWidgets 入门示例 64: QColorDialog 颜色选择对话框
// 演示：getColor 模态选择
//       ShowAlphaChannel 透明度通道
//       currentColorChanged 实时预览
//       棋盘格可视化透明度效果

#ifndef COLORPREVIEWWIDGET_H
#define COLORPREVIEWWIDGET_H

#include <QColor>
#include <QFrame>
#include <QPainter>

// ============================================================================
// ColorPreviewWidget: 带棋盘格的颜色预览控件
// ============================================================================
class ColorPreviewWidget : public QFrame
{
    Q_OBJECT

public:
    explicit ColorPreviewWidget(QWidget *parent = nullptr)
        : QFrame(parent)
    {
        setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
        setMinimumSize(200, 150);
    }

    void setColor(const QColor &color)
    {
        m_color = color;
        update();  // 触发重绘
    }

    QColor color() const { return m_color; }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        const int w = width();
        const int h = height();
        const int gridSize = 12;

        // 1) 绘制棋盘格背景——用于可视化透明度
        for (int y = 0; y < h; y += gridSize) {
            for (int x = 0; x < w; x += gridSize) {
                bool isLight = ((x / gridSize) + (y / gridSize))
                               % 2 == 0;
                painter.fillRect(x, y, gridSize, gridSize,
                    isLight ? QColor(240, 240, 240)
                            : QColor(200, 200, 200));
            }
        }

        // 2) 在棋盘格之上绘制用户选择的颜色（带 Alpha）
        painter.setCompositionMode(
            QPainter::CompositionMode_Source);
        painter.fillRect(0, 0, w, h, m_color);
    }

private:
    QColor m_color{Qt::white};
};

#endif // COLORPREVIEWWIDGET_H
