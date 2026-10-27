// QtWidgets 入门示例 09: 属性动画框架基础
// AnimationDemoWindow: 动画演示主窗口

#ifndef ANIMATIONDEMOWINDOW_H
#define ANIMATIONDEMOWINDOW_H

#include <QMainWindow>
#include <QPoint>

class ColorWidget;
class QComboBox;
class QLabel;

// ============================================================================
// AnimationDemoWindow: 动画演示主窗口
// ============================================================================
class AnimationDemoWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AnimationDemoWindow(QWidget *parent = nullptr);

private:
    /// @brief 获取当前选择的缓动曲线
    QEasingCurve currentCurve() const;

    /// @brief 播放串行位移动画：三个色块依次从左侧飞入
    void playAnimation();

    /// @brief 播放颜色渐变动画：三个色块同时从白色渐变到目标色
    void playColorAnimation();

    /// @brief 重置所有色块到初始位置和颜色
    void resetAll();

private:
    ColorWidget *m_block1 = nullptr;
    ColorWidget *m_block2 = nullptr;
    ColorWidget *m_block3 = nullptr;
    QComboBox *m_curveCombo = nullptr;
    QLabel *m_statusLabel = nullptr;
    QPoint m_pos1;
    QPoint m_pos2;
    QPoint m_pos3;
};

#endif // ANIMATIONDEMOWINDOW_H
