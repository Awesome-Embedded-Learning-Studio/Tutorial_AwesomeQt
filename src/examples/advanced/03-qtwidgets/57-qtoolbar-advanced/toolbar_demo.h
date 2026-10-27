/// @file    toolbar_demo.h
/// @brief   演示 QToolBar 响应式工具栏（宽度不足时自动折叠为仅图标）。
///
/// 对应教程：进阶层 03-QtWidgets/57-QToolBar 进阶。
/// 展示通过 resizeEvent 检测窗口宽度变化，动态切换 toolButtonStyle，
/// 实现工具栏在窄窗口下自动折叠为图标模式。

#pragma once

#include <QMainWindow>

class QToolBar;
class QLabel;

/// @brief 主窗口，演示响应式 QToolBar。
class ToolbarDemo : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit ToolbarDemo(QWidget* parent = nullptr);

protected:
    /// @brief 窗口大小变化时检测宽度并切换工具栏样式。
    /// @param[in] event resize 事件对象。
    /// @note 窗口宽度 < 500px 时切换为 ToolButtonIconOnly，
    ///       >= 500px 时切换为 ToolButtonTextBesideIcon。
    void resizeEvent(QResizeEvent* event) override;

private:
    /// @brief 构建工具栏和菜单。
    void setupUI();

    /// @brief 根据当前宽度更新工具栏按钮样式。
    void updateToolbarStyle();

    QToolBar* m_mainToolbar;           ///< 主工具栏
    QLabel* m_sizeInfoLabel;           ///< 显示当前窗口尺寸和工具栏样式

    static const int kCompactThreshold = 500;  ///< 折叠阈值宽度（像素）
};
