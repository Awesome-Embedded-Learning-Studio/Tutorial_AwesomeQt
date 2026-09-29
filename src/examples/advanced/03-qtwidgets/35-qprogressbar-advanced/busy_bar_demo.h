/// @file    busy_bar_demo.h
/// @brief   演示 QProgressBar 的 busy 模式动画驱动与 paintEvent 自定义文本。
///
/// 对应教程：进阶层 03-QtWidgets/35-QProgressBar 进阶。

#pragma once

#include <QProgressBar>
#include <QWidget>

class QPushButton;

/// 自定义进度条——在填充区域内部绘制白色文字。
///
/// 重写 paintEvent，先让父类画好进度条外观，
/// 再用 clipRect 限制文字只在填充区域内显示，
/// 解决深浅色交界处文字可读性问题。
class OverlayTextBar : public QProgressBar
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针。
    explicit OverlayTextBar(QWidget* parent = nullptr);

    /// @brief 设置覆盖显示的自定义文本。
    /// @param[in] text 要在填充区域显示的文字。
    void setOverlayText(const QString& text);

protected:
    /// @brief 先绘制父类外观，再在填充区域内叠加白色文字。
    void paintEvent(QPaintEvent* event) override;

private:
    QString m_overlayText;    // 自定义覆盖文本
};

// ─────────────────────────────────────────────────────────────────────────────

/// QProgressBar 进阶演示主窗口。
///
/// 展示三个知识点：
/// - busy 模式（setRange(0,0)）的动画驱动——用 QTimer 定期 setValue 强制刷新
/// - 子类化 QProgressBar 重写 paintEvent 实现自定义文本
/// - textFromValue 子类化覆盖模式（显示自定义文本而非百分比）
class BusyBarDemo : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，搭建演示界面。
    /// @param[in] parent 父控件指针。
    explicit BusyBarDemo(QWidget* parent = nullptr);

private:
    /// @brief 构建 busy 模式演示区域。
    /// @param[in] parent 布局所属的容器控件。
    QWidget* createBusySection(QWidget* parent);

    /// @brief 构建自定义 paintEvent 文本覆盖演示区域。
    /// @param[in] parent 布局所属的容器控件。
    QWidget* createOverlaySection(QWidget* parent);

    OverlayTextBar* m_overlayBar;    // 自定义文本覆盖进度条
    int m_stepCount;                 // 模拟步骤计数
};
