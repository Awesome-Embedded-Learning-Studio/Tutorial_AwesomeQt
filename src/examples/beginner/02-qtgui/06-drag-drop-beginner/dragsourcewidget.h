#pragma once

#include <QWidget>

class DragSourceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DragSourceWidget(QWidget *parent = nullptr);

    void setText(const QString &text);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QString m_text;
    QPoint m_dragStartPos;
};
