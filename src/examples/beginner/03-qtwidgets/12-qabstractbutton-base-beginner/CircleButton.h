// QtWidgets 入门示例 12: QAbstractButton 按钮基类机制
// 演示：setCheckable / setChecked / setAutoRepeat 核心属性
//       clicked / toggled / pressed / released 四个信号
//       QButtonGroup 单选互斥管理
//       继承 QAbstractButton 自定义圆形按钮

#pragma once

#include <QAbstractButton>

// ============================================================================
// CircleButton: 继承 QAbstractButton 的自定义圆形按钮
// ============================================================================
class CircleButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit CircleButton(const QString &text, QWidget *parent = nullptr);

protected:
    /// @brief 必须重写：绘制圆形按钮外观
    void paintEvent(QPaintEvent *event) override;

    /// @brief 精确判断点击是否在圆形区域内
    bool hitButton(const QPoint &pos) const override;

    /// @brief 告诉布局管理器按钮的期望大小
    QSize sizeHint() const override;
};
