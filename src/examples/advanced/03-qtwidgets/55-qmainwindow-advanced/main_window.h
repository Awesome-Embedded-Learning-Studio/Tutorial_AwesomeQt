/// @file    main_window.h
/// @brief   演示 QMainWindow 多显示器适配与全屏模式切换。
///
/// 对应教程：进阶层 03-QtWidgets/55-QMainWindow 多显示器适配与全屏模式切换。
/// 展示如何使用 QScreen 检测显示器、在多屏之间移动窗口、
/// 以及 F11 全屏切换时保存/恢复窗口几何信息。

#pragma once

#include <QMainWindow>

class QLabel;
class QDockWidget;

/// @brief 演示多显示器适配与全屏切换的主窗口。
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化 UI 并连接多屏幕相关信号。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    /// @brief 键盘事件处理，用于捕获 F11 全屏切换。
    /// @param[in] event 键盘事件对象。
    /// @note 重写 keyPressEvent 而非使用快捷键，是因为 F11 是
    ///       全屏切换的惯例快捷键，直接拦截更可靠。
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    /// @brief 切换到指定显示器。
    /// @note 根据发送信号的 QAction 的 data() 中存储的屏幕索引，
    ///       将窗口移动到对应显示器。
    void moveToScreen();

    /// @brief 切换全屏/正常模式。
    /// @note 全屏时保存当前几何信息用于恢复，并隐藏菜单栏和状态栏
    ///       以获得真正的沉浸式体验。
    void toggleFullscreen();

    /// @brief 更新状态栏中的屏幕信息显示。
    /// @note 在屏幕添加/移除、窗口移动时都需要调用，
    ///       确保用户始终能看到当前屏幕的分辨率和 DPI。
    void updateScreenInfo();

private:
    /// @brief 构建菜单栏，包含视图切换和多显示器操作。
    void setupMenus();

    /// @brief 创建停靠窗口，展示全屏模式下 dock 的行为。
    void setupDockWidgets();

    /// @brief 重建「移动到显示器」子菜单项。
    /// @note 每次显示菜单前重建，因为用户可能随时热插拔显示器。
    void rebuildScreenMenu();

    QLabel* m_screenInfoLabel;    ///< 状态栏中显示当前屏幕信息的标签
    QDockWidget* m_dockLeft;      ///< 左侧停靠窗口
    QDockWidget* m_dockRight;     ///< 右侧停靠窗口
    QRect m_normalGeometry;       ///< 进入全屏前保存的正常模式窗口几何
    bool m_isFullscreen;          ///< 当前是否处于全屏模式
};
