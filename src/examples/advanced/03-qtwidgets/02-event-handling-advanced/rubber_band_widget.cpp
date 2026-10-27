/// @file    rubber_band_widget.cpp
/// @brief   RubberBandWidget 类实现——grabMouse 橡皮筋选区。
///
/// 对应教程：进阶层 03-QtWidgets/02-事件处理进阶。

#include "rubber_band_widget.h"

#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

RubberBandWidget::RubberBandWidget(QWidget* parent)
    : QWidget(parent)
    , m_isDragging(false)
    , m_infoLabel(nullptr)
{
    auto* layout = new QVBoxLayout(this);

    auto* hint = new QLabel(
        QStringLiteral("在下方区域按住左键拖拽绘制选区。\n"
                       "即使鼠标移出控件边界，grabMouse 也能保证事件不丢失。"));
    hint->setWordWrap(true);
    layout->addWidget(hint);

    // 信息提示标签
    m_infoLabel = new QLabel(QStringLiteral("等待操作..."));
    layout->addWidget(m_infoLabel);

    // 让控件有最小尺寸，方便拖拽测试
    setMinimumSize(400, 300);
    setWindowTitle(QStringLiteral("Rubber Band Selection Demo"));
}

// ─────────────────────────────────────────────────────────────────────────────
// 鼠标事件处理
// ─────────────────────────────────────────────────────────────────────────────

void RubberBandWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_origin = event->pos();
        m_current = event->pos();
        m_isDragging = true;

        // grabMouse: 强制捕获鼠标，后续所有鼠标事件都路由到此控件
        // 即使鼠标移到其他窗口也能收到事件
        grabMouse();

        m_infoLabel->setText(
            QStringLiteral("grabMouse() 已调用 —— 起点: (%1, %2)").arg(m_origin.x()).arg(m_origin.y()));
    }
}

void RubberBandWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isDragging) {
        m_current = event->pos();
        update();  // 触发重绘以更新选区矩形

        const QRect sel = normalizedSelection();
        m_infoLabel->setText(
            QStringLiteral("拖拽中 —— 选区: (%1, %2) %3x%4")
                .arg(sel.x())
                .arg(sel.y())
                .arg(sel.width())
                .arg(sel.height()));
    }
}

void RubberBandWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_isDragging) {
        // releaseMouse: 必须与 grabMouse 严格配对，否则整个应用鼠标锁死
        releaseMouse();

        m_isDragging = false;
        update();

        const QRect sel = normalizedSelection();
        m_infoLabel->setText(
            QStringLiteral("releaseMouse() 已调用 —— 最终选区: (%1, %2) %3x%4")
                .arg(sel.x())
                .arg(sel.y())
                .arg(sel.width())
                .arg(sel.height()));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 绘制
// ─────────────────────────────────────────────────────────────────────────────

void RubberBandWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制浅灰背景
    painter.fillRect(rect(), QColor(245, 245, 245));

    if (m_isDragging) {
        const QRect sel = normalizedSelection();

        // 半透明填充
        painter.fillRect(sel, QColor(100, 149, 237, 60));

        // 边框
        QPen pen(QColor(100, 149, 237));
        pen.setWidth(2);
        pen.setStyle(Qt::DashLine);
        painter.setPen(pen);
        painter.drawRect(sel);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 辅助方法
// ─────────────────────────────────────────────────────────────────────────────

QRect RubberBandWidget::normalizedSelection() const
{
    // normalized() 自动处理反向拖拽（右→左、下→上）产生的负宽高
    return QRect(m_origin, m_current).normalized();
}
