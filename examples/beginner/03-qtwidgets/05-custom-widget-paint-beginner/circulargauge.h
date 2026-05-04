#pragma once

#include <QPaintEvent>
#include <QWidget>

class CircularGauge : public QWidget
{
    Q_OBJECT
    // QPropertyAnimation 需要通过 Q_PROPERTY 驱动值渐变
    Q_PROPERTY(double value READ value WRITE setAnimatedValue NOTIFY valueChanged)

public:
    explicit CircularGauge(const QString &title, QWidget *parent = nullptr);

    /// @brief 布局系统查询推荐尺寸
    QSize sizeHint() const override;

    /// @brief 布局系统查询最小可用尺寸
    QSize minimumSizeHint() const override;

    double value() const;

    /// @brief 设置当前值，触发平滑动画过渡
    void setValue(double newValue);

    /// @brief 由 QPropertyAnimation 直接调用，驱动值的渐变
    void setAnimatedValue(double newValue);

signals:
    void valueChanged(double newValue);

protected:
    /// @brief 所有自定义绘制逻辑都在这里完成
    void paintEvent(QPaintEvent *event) override;

private:
    /// @brief 根据进度比例返回弧线颜色（绿 → 黄 → 红）
    QColor getArcColor(double ratio) const;

    QString m_title;
    double m_value;
};
