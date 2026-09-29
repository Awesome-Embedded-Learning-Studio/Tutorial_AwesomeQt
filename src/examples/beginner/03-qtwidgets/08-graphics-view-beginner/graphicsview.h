// QtWidgets 入门示例 08: 图形视图框架基础
// GraphicsView: 自定义视图，支持滚轮缩放

#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>

class GraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit GraphicsView(QGraphicsScene *scene, QWidget *parent = nullptr);

protected:
    /// @brief 滚轮缩放视图
    void wheelEvent(QWheelEvent *event) override;
};

#endif // GRAPHICSVIEW_H
