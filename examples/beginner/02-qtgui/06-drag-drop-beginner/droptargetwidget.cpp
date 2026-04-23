#include "droptargetwidget.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QPainter>
#include <QUrl>
#include <QDebug>

DropTargetWidget::DropTargetWidget(QWidget *parent) : QWidget(parent)
{
    setAcceptDrops(true);  // 这一步绝对不能忘
    setMinimumSize(250, 300);
    setToolTip("把文本或文件拖放到这里");
}

void DropTargetWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 根据拖拽悬停状态改变背景色
    QColor bgColor = m_dragHovering ? QColor(230, 255, 230)
                                    : QColor(255, 250, 240);
    painter.fillRect(rect(), bgColor);

    QPen borderPen(m_dragHovering ? QColor(60, 180, 60) : QColor(200, 150, 80),
                   m_dragHovering ? 3 : 2);
    painter.setPen(borderPen);
    painter.drawRect(rect().adjusted(1, 1, -2, -2));

    // 标题
    painter.setPen(m_dragHovering ? QColor(30, 130, 30) : QColor(180, 120, 50));
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(QRect(10, 10, width() - 20, 30),
                     Qt::AlignCenter, "接收目标 (Drop Target)");

    if (m_receivedText.isEmpty() && m_receivedFiles.isEmpty()) {
        // 没有数据时显示提示
        painter.setPen(QColor(150, 150, 150));
        painter.setFont(QFont("Arial", 11));
        painter.drawText(rect().adjusted(10, 50, -10, -10),
                         Qt::AlignCenter, "将文本或文件拖到这里\n释放鼠标即可接收");
    } else {
        // 显示接收到的文本
        if (!m_receivedText.isEmpty()) {
            painter.setPen(Qt::black);
            painter.setFont(QFont("Arial", 14, QFont::Bold));
            painter.drawText(QRect(10, 50, width() - 20, 40),
                             Qt::AlignCenter,
                             QString("文本: %1").arg(m_receivedText));
        }

        // 显示接收到的文件列表
        if (!m_receivedFiles.isEmpty()) {
            painter.setPen(QColor(80, 80, 80));
            painter.setFont(QFont("Arial", 10));
            int y = 100;
            for (int i = 0; i < m_receivedFiles.size() && i < 5; ++i) {
                QString fileName = m_receivedFiles[i].toLocalFile();
                // 只显示文件名，不显示完整路径
                QUrl url(m_receivedFiles[i]);
                QString display = url.fileName().isEmpty()
                                      ? fileName
                                      : url.fileName();
                painter.drawText(QRect(15, y, width() - 30, 22),
                                 Qt::AlignLeft | Qt::AlignVCenter,
                                 QString("File: %1").arg(display));
                y += 24;
            }
            if (m_receivedFiles.size() > 5) {
                painter.drawText(QRect(15, y, width() - 30, 22),
                                 Qt::AlignLeft | Qt::AlignVCenter,
                                 QString("... 以及其他 %1 个文件")
                                     .arg(m_receivedFiles.size() - 5));
            }
        }
    }

    // 底部状态
    painter.setPen(QColor(150, 150, 150));
    painter.setFont(QFont("Arial", 9));
    int dropCount = m_dropCount;
    painter.drawText(QRect(10, height() - 30, width() - 20, 25),
                     Qt::AlignCenter,
                     QString("已接收 %1 次拖放").arg(dropCount));
}

void DropTargetWidget::dragEnterEvent(QDragEnterEvent *event)
{
    // 接受文本或文件类型的拖放
    if (event->mimeData()->hasText() || event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        m_dragHovering = true;
        update();  // 刷新显示拖拽悬停效果
    }
}

void DropTargetWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void DropTargetWidget::dragLeaveEvent(QDragLeaveEvent *)
{
    m_dragHovering = false;
    update();  // 恢复正常背景色
}

void DropTargetWidget::dropEvent(QDropEvent *event)
{
    m_dragHovering = false;
    m_dropCount++;

    // 提取文本数据
    if (event->mimeData()->hasText()) {
        m_receivedText = event->mimeData()->text();
        qDebug() << "[DropTarget] 收到文本:" << m_receivedText;
    }

    // 提取文件 URL 数据
    if (event->mimeData()->hasUrls()) {
        m_receivedFiles = event->mimeData()->urls();
        for (const QUrl &url : m_receivedFiles) {
            if (url.isLocalFile()) {
                qDebug() << "[DropTarget] 收到文件:" << url.toLocalFile();
            }
        }
    }

    event->acceptProposedAction();
    update();
}
