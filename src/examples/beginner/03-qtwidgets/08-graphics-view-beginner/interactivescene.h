// QtWidgets 入门示例 08: 图形视图框架基础
// InteractiveScene: 自定义场景，拦截鼠标事件并报告坐标

#ifndef INTERACTIVESCENE_H
#define INTERACTIVESCENE_H

#include <QGraphicsScene>
#include <QColor>

class InteractiveScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit InteractiveScene(QObject *parent = nullptr);

    /// @brief 添加一个随机颜色的矩形
    void addRandomRect();

    /// @brief 添加一个随机颜色的椭圆
    void addRandomEllipse();

    /// @brief 删除所有选中的图元
    void deleteSelectedItems();

signals:
    /// @brief 鼠标在场景中移动时发出，携带场景坐标
    void mouseScenePosChanged(const QPointF &scenePos);

    /// @brief 选中图元数量变化时发出
    void selectionCountChanged(int count);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    void setupPresetItems();
    QColor randomColor() const;
};

#endif // INTERACTIVESCENE_H
