/// @file    widget.h
/// @brief   演示 QMdiArea 标签页模式与子窗口菜单集成。
///
/// 对应教程：进阶层 03-QtWidgets/10-MDI 高级用法。
/// 本示例展示 QMdiArea 的 TabbedView 模式切换、
/// 动态子窗口菜单以及窗口级联/平铺操作。

#pragma once

#include <QMainWindow>

class QMdiArea;
class QMenu;

/// @brief 主窗口，内嵌 QMdiArea 并提供窗口管理菜单。
class MdiMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit MdiMainWindow(QWidget* parent = nullptr);

private slots:
    /// @brief 创建新的 MDI 子窗口，内含一个 QTextEdit。
    /// @note 每次调用都创建独立子窗口，标题自动递增编号。
    void createSubWindow();

    /// @brief 将所有子窗口级联排列。
    void cascadeSubWindows();

    /// @brief 将所有子窗口平铺排列。
    void tileSubWindows();

    /// @brief 在 SubWindowView 和 TabbedView 之间切换。
    /// @note TabbedView 模式下用户通过标签页切换子窗口，
    ///       SubWindowView 下子窗口可自由拖拽和重叠。
    void toggleViewMode();

    /// @brief 刷新 Window 菜单中的子窗口列表。
    /// @note 每次子窗口增减或焦点变化时调用，保证菜单与实际状态同步。
    void updateWindowMenu();

private:
    /// @brief 初始化菜单栏、工具栏和信号槽连接。
    void setupMenus();

    /// @brief 创建中央 QMdiArea 并设置基础属性。
    void setupMdiArea();

    QMdiArea* m_mdiArea;         ///< 多文档区域，作为中央控件
    QMenu* m_windowMenu;         ///< Window 菜单，包含子窗口操作和列表
    int m_windowCounter;         ///< 子窗口递增计数器，用于生成唯一标题
};
