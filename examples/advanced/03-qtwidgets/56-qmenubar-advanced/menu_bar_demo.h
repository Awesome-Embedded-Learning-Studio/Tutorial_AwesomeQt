/// @file    menu_bar_demo.h
/// @brief   演示 QMenuBar 动态构建菜单与最近文件列表。
///
/// 对应教程：进阶层 03-QtWidgets/56-QMenuBar 进阶。
/// 展示运行时动态构建菜单、"最近文件"子菜单的 aboutToShow 重建模式，
/// 以及 QMenu::addAction 的多种重载用法。

#pragma once

#include <QMainWindow>

class QLabel;
class QMenu;

/// @brief 主窗口，演示动态菜单与最近文件列表。
///
/// 核心演示点：
/// - 运行时动态添加/移除菜单项
/// - "最近文件"子菜单在 aboutToShow 时动态重建
/// - 最近文件列表的最大条目限制（最多 5 条）
class MenuBarDemo : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit MenuBarDemo(QWidget* parent = nullptr);

private slots:
    /// @brief 打开一个新文件（模拟），将其加入最近文件列表。
    void openFile();

    /// @brief 清空最近文件列表。
    void clearRecentFiles();

    /// @brief 从最近文件列表中打开指定文件。
    void openRecentFile();

private:
    /// @brief 构建菜单栏。
    void setupMenus();

    /// @brief 重建最近文件子菜单项。
    /// @note 每次 aboutToShow 时调用，确保列表与 m_recentFiles 同步。
    void rebuildRecentFilesMenu();

    /// @brief 更新状态栏显示。
    /// @param[in] message 要显示的消息。
    void updateStatus(const QString& message);

    QLabel* m_statusLabel;              ///< 状态栏标签
    QMenu* m_recentMenu;                ///< "最近文件"子菜单
    QStringList m_recentFiles;          ///< 最近打开的文件列表

    static const int kMaxRecentFiles = 5;  ///< 最近文件列表最大条目数
};
