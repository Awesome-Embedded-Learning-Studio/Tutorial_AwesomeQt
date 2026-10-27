// QtWidgets 入门示例 09: 属性动画框架基础
// 演示：QPropertyAnimation 对 Q_PROPERTY 属性做动画

#ifndef COLORWIDGET_H
#define COLORWIDGET_H

#include <QColor>
#include <QWidget>

// ============================================================================
// ColorWidget: 支持 backgroundColor 属性动画的自定义控件
// ============================================================================
class ColorWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor backgroundColor READ backgroundColor
               WRITE setBackgroundColor NOTIFY backgroundColorChanged)

public:
    explicit ColorWidget(const QColor &color, const QString &text,
                         QWidget *parent = nullptr);

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &color);
    void reset();

protected:
    void paintEvent(QPaintEvent *event) override;

signals:
    void backgroundColorChanged(const QColor &color);

private:
    QColor m_targetColor;
    QColor m_bgColor;
    QString m_text;
};

#endif // COLORWIDGET_H
