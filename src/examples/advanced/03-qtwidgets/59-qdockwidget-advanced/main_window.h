/// @file    main_window.h
/// @brief   演示 QDockWidget 布局持久化的主窗口类声明。
///
/// 对应教程：进阶层 03-QtWidgets/59-QDockWidget 布局持久化。
/// 本文件声明了带有三个 QDockWidget 的主窗口，展示如何使用
/// saveState()/restoreState() 与 QSettings 配合实现布局的保存与恢复。

#pragma once

#include <QMainWindow>

class QDockWidget;
class QTextEdit;
class QTreeWidget;
class QTableWidget;

/// @brief 带有可停靠面板的主窗口，支持布局持久化。
///
/// 创建三个 QDockWidget（文件树、属性表、输出面板），通过 QSettings
/// 在关闭时保存几何信息与停靠状态，在启动时自动恢复。
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化界面并尝试恢复上次的布局。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit MainWindow(QWidget* parent = nullptr);

    /// @brief 析构函数，保存当前窗口布局到 QSettings。
    ~MainWindow() override;

protected:
    /// @brief 关闭事件处理，在窗口关闭前持久化布局。
    /// @param[in,out] event 关闭事件对象。
    void closeEvent(QCloseEvent* event) override;

private:
    /// @brief 创建所有 QDockWidget 及其内容控件。
    void setupDockWidgets();

    /// @brief 创建菜单栏及其动作。
    void setupMenus();

    /// @brief 将当前窗口几何信息与停靠状态写入 QSettings。
    /// @note 使用 QSettings 是因为它是跨平台的持久化方案，
    ///       在 Windows 上写注册表，在 Linux/macOS 上写 INI 文件。
    void saveLayout();

    /// @brief 从 QSettings 读取并恢复窗口几何信息与停靠状态。
    /// @return 恢复成功返回 true，无保存数据时返回 false。
    bool restoreLayout();

    /// @brief 重置所有 QDockWidget 到默认位置。
    /// @note 当用户把面板拖乱后，可通过菜单触发此函数一键恢复。
    void resetLayout();

    QDockWidget* m_fileTreeDock;    ///< 文件树停靠面板
    QDockWidget* m_propertiesDock;  ///< 属性表停靠面板
    QDockWidget* m_outputDock;      ///< 输出日志停靠面板

    QTreeWidget* m_fileTree;        ///< 文件树控件
    QTableWidget* m_propertiesTable;///< 属性表格控件
    QTextEdit* m_outputLog;         ///< 输出日志文本框
};
