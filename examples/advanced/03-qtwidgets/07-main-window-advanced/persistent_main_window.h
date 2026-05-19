/// @file    persistent_main_window.h
/// @brief   演示 QMainWindow 的 Dock 布局持久化：saveGeometry/restoreGeometry、
///          saveState/restoreState 搭配 QSettings 实现跨会话布局恢复。
///
/// 对应教程：进阶层 03-QtWidgets/07-主窗口进阶。

#pragma once

#include <QMainWindow>

class QDockWidget;
class QPlainTextEdit;

/// 主窗口布局持久化演示。
///
/// 展示四个核心知识点：
/// - QDockWidget 多区域停靠、嵌套与 tabify
/// - saveGeometry/restoreGeometry 保存窗口几何信息
/// - saveState/restoreState 保存 Dock 和工具栏布局
/// - setCorner 配置角落区域归属，toggleViewAction 集成到"视图"菜单
class PersistentMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief 构造函数，创建 Dock、工具栏、菜单并恢复上次布局。
    /// @param[in] parent 父控件指针。
    explicit PersistentMainWindow(QWidget* parent = nullptr);

    /// @brief 从 QSettings 恢复窗口几何和 Dock 布局。
    /// @note restoreState 必须在 show() 之后调用才能正确还原停靠位置。
    void restoreLayout();

protected:
    /// @brief 关闭事件——保存窗口几何与 Dock 布局到 QSettings。
    /// @param[in,out] event 关闭事件对象。
    void closeEvent(QCloseEvent* event) override;

private:
    /// @brief 配置四个角落区域归属左侧和右侧 Dock 区域。
    /// @note 必须在 addDockWidget 之前调用，否则已有 Dock 不会重新计算布局。
    void setupCorners();

    /// @brief 创建四个 Dock 面板并添加到对应区域。
    void setupDockWidgets();

    /// @brief 创建工具栏并添加到主窗口。
    void setupToolBars();

    /// @brief 创建菜单栏（文件、视图）。
    void setupMenuBar();

    /// @brief 清除 QSettings 中保存的状态，将所有 Dock 恢复到代码中的初始位置。
    void resetToDefaultLayout();

    /// @brief 将指定 Dock 重新停靠到初始区域。
    /// @param[in] dock 目标 Dock 指针。
    /// @param[in] area 目标停靠区域。
    void reDock(QDockWidget* dock, Qt::DockWidgetArea area);

private:
    QPlainTextEdit* m_editor;          // 中央代码编辑区
    QDockWidget* m_fileExplorerDock;   // 左侧文件浏览器
    QDockWidget* m_propertyDock;       // 右侧属性面板
    QDockWidget* m_toolBoxDock;        // 右侧工具箱
    QDockWidget* m_outputDock;         // 底部编译输出
};
