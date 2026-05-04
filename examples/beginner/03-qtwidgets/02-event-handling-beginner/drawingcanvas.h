#pragma once

#include <QColor>
#include <QList>
#include <QLine>
#include <QPoint>
#include <QWidget>

class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;

// ============================================================================
// 画板 Widget：重写鼠标、键盘、resize 事件
// ============================================================================
class DrawingCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit DrawingCanvas(QWidget *parent = nullptr);

    /// @brief 清空所有已画的线条
    void clearCanvas();

    /// @brief 撤销最后一条线
    void undoLastLine();

    void setPenColor(const QColor &color);
    QColor penColor() const;
    int lineCount() const;

signals:
    void mousePositionChanged(const QPoint &pos);
    void canvasResized(const QSize &size);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void cycleColor();

    QList<QLine> m_lines;
    QList<QColor> m_lineColors;
    QColor m_penColor;
    int m_penWidth;
    QPoint m_lastPoint;
    QPoint m_mousePos;
    bool m_drawing = false;
};
