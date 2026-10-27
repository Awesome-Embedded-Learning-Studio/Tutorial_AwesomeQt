#include "drawingcanvas.h"

#include <QKeyEvent>
#include <QLine>
#include <QList>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QDebug>
#include <QResizeEvent>

// ============================================================================
// 构造函数
// ============================================================================
DrawingCanvas::DrawingCanvas(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(500, 350);
    setMouseTracking(true);  // 鼠标未按下时也追踪移动（用于坐标显示）
    setFocusPolicy(Qt::StrongFocus);  // 允许接收键盘焦点
    setCursor(Qt::CrossCursor);

    // 默认画笔颜色
    m_penColor = Qt::black;
    m_penWidth = 3;
}

// ============================================================================
// 公有方法
// ============================================================================
void DrawingCanvas::clearCanvas()
{
    m_lines.clear();
    m_lineColors.clear();
    update();
    qDebug() << "[Canvas] 画板已清空";
}

void DrawingCanvas::undoLastLine()
{
    if (!m_lines.isEmpty()) {
        m_lines.removeLast();
        m_lineColors.removeLast();
        update();
        qDebug() << "[Canvas] 撤销成功，剩余线条数:" << m_lines.size();
    }
}

void DrawingCanvas::setPenColor(const QColor &color)
{
    m_penColor = color;
}

QColor DrawingCanvas::penColor() const { return m_penColor; }
int DrawingCanvas::lineCount() const { return m_lines.size(); }

// ============================================================================
// 事件重写
// ============================================================================
void DrawingCanvas::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_drawing = true;
        m_lastPoint = event->pos();
        event->accept();  // 已处理，不传播
    } else if (event->button() == Qt::RightButton) {
        // 右键切换颜色，演示 accept
        cycleColor();
        event->accept();
    } else {
        event->ignore();  // 其他按键不处理，让父控件处理
    }
}

void DrawingCanvas::mouseMoveEvent(QMouseEvent *event)
{
    // 更新鼠标位置（供状态栏显示）
    m_mousePos = event->pos();

    if (m_drawing) {
        // 画线：把上一个点和当前点连成一条线
        QLine line(m_lastPoint, event->pos());
        m_lines.append(line);
        m_lineColors.append(m_penColor);
        m_lastPoint = event->pos();
        update();
        event->accept();
    } else {
        // 没有在画线，只是追踪鼠标位置
        // 更新父窗口的状态栏
        emit mousePositionChanged(event->pos());
        event->ignore();  // 让事件继续传播
    }
}

void DrawingCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_drawing = false;
        event->accept();
    }
}

void DrawingCanvas::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_C) {
        clearCanvas();
        event->accept();
    } else if (event->key() == Qt::Key_R) {
        cycleColor();
        event->accept();
    } else if (event->key() == Qt::Key_Plus
               || event->key() == Qt::Key_Equal) {
        m_penWidth = qMin(m_penWidth + 1, 20);
        qDebug() << "[Canvas] 画笔宽度:" << m_penWidth;
        event->accept();
    } else if (event->key() == Qt::Key_Minus) {
        m_penWidth = qMax(m_penWidth - 1, 1);
        qDebug() << "[Canvas] 画笔宽度:" << m_penWidth;
        event->accept();
    } else {
        // 其他按键我们不处理，传给基类
        // Ctrl+Z 等组合键会被事件过滤器拦截（见 MainWindow）
        QWidget::keyPressEvent(event);
    }
}

void DrawingCanvas::resizeEvent(QResizeEvent *event)
{
    qDebug() << "[Canvas] 尺寸变化:" << event->oldSize()
             << "->" << event->size();
    emit canvasResized(event->size());
    QWidget::resizeEvent(event);
}

void DrawingCanvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 白色背景
    painter.fillRect(rect(), Qt::white);

    // 画网格参考线
    painter.setPen(QPen(QColor(230, 230, 230), 1));
    for (int x = 0; x < width(); x += 50) {
        painter.drawLine(x, 0, x, height());
    }
    for (int y = 0; y < height(); y += 50) {
        painter.drawLine(0, y, width(), y);
    }

    // 画所有已保存的线条
    for (int i = 0; i < m_lines.size(); ++i) {
        painter.setPen(QPen(m_lineColors[i], m_penWidth,
                            Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(m_lines[i]);
    }

    // 右下角显示当前颜色
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_penColor);
    painter.drawEllipse(width() - 35, height() - 35, 25, 25);
    painter.setPen(Qt::gray);
    painter.drawRect(width() - 36, height() - 36, 27, 27);
}

// ============================================================================
// 私有方法
// ============================================================================
void DrawingCanvas::cycleColor()
{
    QList<QColor> colors = {Qt::black, Qt::red, Qt::blue,
                            Qt::darkGreen, Qt::darkMagenta,
                            QColor(255, 140, 0)};
    int idx = colors.indexOf(m_penColor);
    m_penColor = colors[(idx + 1) % colors.size()];
    qDebug() << "[Canvas] 切换颜色:" << m_penColor.name();
    update();
}
