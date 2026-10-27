#pragma once

#include <QUrl>
#include <QWidget>

class DropTargetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DropTargetWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    QString m_receivedText;
    QList<QUrl> m_receivedFiles;
    bool m_dragHovering = false;
    int m_dropCount = 0;
};
