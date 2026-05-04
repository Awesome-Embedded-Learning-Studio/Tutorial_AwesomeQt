// QtWidgets 入门示例 12: QAbstractButton 按钮基类机制
// 演示：setCheckable / setChecked / setAutoRepeat 核心属性
//       clicked / toggled / pressed / released 四个信号
//       QButtonGroup 单选互斥管理
//       继承 QAbstractButton 自定义圆形按钮

#include "CircleButton.h"

#include <QPainter>

CircleButton::CircleButton(const QString &text, QWidget *parent)
    : QAbstractButton(parent)
{
    setText(text);
    setFixedSize(80, 80);
    setFocusPolicy(Qt::StrongFocus);
}

void CircleButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor bgColor;
    if (isDown()) {
        bgColor = QColor("#1565C0");
    } else if (isChecked()) {
        bgColor = QColor("#42A5F5");
    } else if (underMouse()) {
        bgColor = QColor("#64B5F6");
    } else {
        bgColor = QColor("#90CAF9");
    }

    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(rect());

    painter.setPen(Qt::white);
    painter.setFont(font());
    painter.drawText(rect(), Qt::AlignCenter, text());
}

bool CircleButton::hitButton(const QPoint &pos) const
{
    return QLineF(rect().center(), pos).length() <= width() / 2.0;
}

QSize CircleButton::sizeHint() const
{
    return QSize(80, 80);
}
