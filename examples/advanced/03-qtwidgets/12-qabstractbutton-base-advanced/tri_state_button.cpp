/// @file    tri_state_button.cpp
/// @brief   TriStateButton 类实现——三态按钮的状态管理与手动绘制。
///
/// 对应教程：进阶层 03-QtWidgets/12-QAbstractButton 基类进阶。

#include "tri_state_button.h"

#include <QPainter>
#include <QPaintEvent>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

TriStateButton::TriStateButton(const QString& text, QWidget* parent)
    : QAbstractButton(parent)
    , m_checkState(Qt::Unchecked)
{
    setText(text);
    setCheckable(true);
    // 正方形按钮，确保圆形绘制不被裁剪
    setFixedSize(120, 120);
}

// ─────────────────────────────────────────────────────────────────────────────
// 状态访问
// ─────────────────────────────────────────────────────────────────────────────

Qt::CheckState TriStateButton::checkState() const
{
    return m_checkState;
}

void TriStateButton::setCheckState(Qt::CheckState state)
{
    if (m_checkState == state) {
        return;
    }
    m_checkState = state;
    // 同步 QAbstractButton 的 checked bool：Checked 为 true，其余为 false
    QAbstractButton::setChecked(state == Qt::Checked);
    Q_EMIT stateChanged(static_cast<int>(state));
    update();
}

// ─────────────────────────────────────────────────────────────────────────────
// paintEvent——手动绘制三态圆形按钮
// ─────────────────────────────────────────────────────────────────────────────

void TriStateButton::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 三种状态分别用不同底色：灰、橙、绿
    QColor bgColor;
    QString stateText;
    switch (m_checkState) {
    case Qt::Unchecked:
        bgColor = QColor(0xBD, 0xBD, 0xBD);  // 浅灰
        stateText = QStringLiteral("Off");
        break;
    case Qt::PartiallyChecked:
        bgColor = QColor(0xFF, 0xA7, 0x26);  // 橙色
        stateText = QStringLiteral("Half");
        break;
    case Qt::Checked:
        bgColor = QColor(0x66, 0xBB, 0x6A);  // 绿色
        stateText = QStringLiteral("On");
        break;
    }

    // 按下时颜色加深，给用户按下反馈
    if (isDown()) {
        bgColor = bgColor.darker(130);
    }

    // 绘制圆形背景
    painter.setPen(Qt::NoPen);
    painter.setBrush(bgColor);
    painter.drawEllipse(rect().adjusted(4, 4, -4, -4));

    // 绘制状态文字
    painter.setPen(Qt::white);
    QFont f = font();
    f.setBold(true);
    painter.setFont(f);
    painter.drawText(rect(), Qt::AlignCenter, stateText);
}

// ─────────────────────────────────────────────────────────────────────────────
// nextCheckState——三态循环
// ─────────────────────────────────────────────────────────────────────────────

void TriStateButton::nextCheckState()
{
    // Unchecked -> PartiallyChecked -> Checked -> Unchecked 循环
    switch (m_checkState) {
    case Qt::Unchecked:
        setCheckState(Qt::PartiallyChecked);
        break;
    case Qt::PartiallyChecked:
        setCheckState(Qt::Checked);
        break;
    case Qt::Checked:
        setCheckState(Qt::Unchecked);
        break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// hitButton——圆形点击区域
// ─────────────────────────────────────────────────────────────────────────────

bool TriStateButton::hitButton(const QPoint& pos) const
{
    // 只响应圆形区域内的点击，角落区域忽略
    const int radius = width() / 2 - 4;
    const QPointF center(width() / 2.0, height() / 2.0);
    const QPointF diff = pos - center;
    // 到圆心的距离 <= 半径才算命中
    return (diff.x() * diff.x() + diff.y() * diff.y()) <= (radius * radius);
}
