/// @file    mdi_manager.h
/// @brief   MDI 子窗口管理主窗口——QMdiArea 子窗口添加、级联、平铺、窗口菜单同步。
///
/// 对应教程：进阶层 03-QtWidgets/10-MDI 进阶。

#pragma once

#include <QMainWindow>

class QMdiArea;
class QMenu;
class QLabel;

/// MDI 管理器主窗口。
///
/// 基于 QMainWindow 内嵌 QMdiArea，提供：
/// - 新建子窗口（MdiChild 内含 QPlainTextEdit）
/// - 级联/平铺排列子窗口
/// - 动态更新的窗口菜单（列出所有子窗口，当前活动窗口打勾）
/// - 状态栏显示当前文档标题和打开文档总数
class MdiManager : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief 构造函数，创建菜单栏、工具栏、MDI 区域和状态栏。
    /// @param[in] parent 父控件指针。
    explicit MdiManager(QWidget* parent = nullptr);

private:
    /// @brief 创建一个新的 MDI 子窗口（内含文本编辑器）。
    void createChild();

    /// @brief 级联排列所有子窗口。
    void cascadeSubWindows();

    /// @brief 平铺排列所有子窗口。
    void tileSubWindows();

    /// @brief 子窗口激活时更新菜单栏和状态栏。
    /// @param[in] subWin 被激活的子窗口（可能为 nullptr）。
    void onSubWindowActivated(class QMdiSubWindow* subWin);

    /// @brief 重建窗口菜单，列出所有子窗口并标记当前活动窗口。
    void rebuildWindowMenu();

    QMdiArea* m_mdiArea;     // MDI 区域控件
    QMenu* m_windowMenu;     // 窗口菜单
    QLabel* m_statusLabel;   // 状态栏标签
    int m_childCounter;      // 子窗口递增编号计数器
};
