#include "dragsourcewidget.h"

#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>

DragSourceWidget::DragSourceWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(250, 300);
    m_text = "Drag me!";
    setToolTip("按住左键拖拽这段文字到右侧目标区域");
}

void DragSourceWidget::setText(const QString &text)
{
    m_text = text;
    update();
}

void DragSourceWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 背景
    painter.fillRect(rect(), QColor(245, 245, 255));
    QPen borderPen(QColor(100, 100, 200), 2, Qt::DashLine);
    painter.setPen(borderPen);
    painter.drawRect(rect().adjusted(1, 1, -2, -2));

    // 标题
    painter.setPen(QColor(80, 80, 180));
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(QRect(10, 10, width() - 20, 30),
                     Qt::AlignCenter, "拖拽源 (Drag Source)");

    // 中央大文字
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 20, QFont::Bold));
    painter.drawText(rect().adjusted(10, 50, -10, -10),
                     Qt::AlignCenter, m_text);

    // 底部提示
    painter.setPen(QColor(150, 150, 150));
    painter.setFont(QFont("Arial", 9));
    painter.drawText(QRect(10, height() - 30, width() - 20, 25),
                     Qt::AlignCenter, "也可以从外部拖入文件到右侧");
}

void DragSourceWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStartPos = event->pos();
    }
    QWidget::mousePressEvent(event);
}

void DragSourceWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton)) {
        return;
    }

    // 判断移动距离是否超过拖拽阈值
    if ((event->pos() - m_dragStartPos).manhattanLength()
         < QApplication::startDragDistance()) {
        return;
    }

    // 创建 QDrag 对象
    QDrag *drag = new QDrag(this);

    // 创建 MIME 数据，携带文本
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(m_text);
    drag->setMimeData(mimeData);

    // 创建一个简单的拖拽预览 pixmap
    QPixmap pixmap(120, 40);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(0, 0, 120, 40, QColor(100, 100, 200, 180));
    p.setPen(Qt::white);
    p.setFont(QFont("Arial", 11));
    p.drawText(QRect(0, 0, 120, 40), Qt::AlignCenter, m_text);
    p.end();
    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(60, 20));

    // 执行拖拽操作（阻塞直到拖放结束）
    Qt::DropAction result = drag->exec(Qt::CopyAction | Qt::MoveAction);

    if (result == Qt::CopyAction) {
        qDebug() << "[DragSource] 文本被复制";
    } else if (result == Qt::MoveAction) {
        qDebug() << "[DragSource] 文本被移动";
    } else {
        qDebug() << "[DragSource] 拖放被取消";
    }
}
