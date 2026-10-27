/// @file    main_window.h
/// @brief   演示 QDialog 阻塞与非阻塞模式的主窗口类声明。
///
/// 对应教程：进阶层 03-QtWidgets/60-QDialog 异步对话框与结果回调。
/// 本文件声明了主窗口，包含两个按钮分别触发阻塞（exec）和
/// 非阻塞（show）对话框，以及一个文本区域显示对话框返回的结果。

#pragma once

#include <QMainWindow>

class QTextEdit;

/// @brief 主窗口，演示 QDialog 的两种打开模式。
///
/// 提供两个按钮：
/// - "Open Blocking"：调用 exec()，对话框关闭后才继续。
/// - "Open Non-Blocking"：调用 show()，主窗口保持可操作，
///   通过连接 accepted() 信号异步获取对话框结果。
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化界面。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit MainWindow(QWidget* parent = nullptr);

private:
    /// @brief 以阻塞模式（exec）打开用户输入对话框。
    /// @note exec() 会阻塞主窗口的事件循环直到对话框关闭，
    ///       适合需要用户立即响应的场景。
    void openBlockingDialog();

    /// @brief 以非阻塞模式（show）打开用户输入对话框。
    /// @note show() 不会阻塞，主窗口仍可操作。
    ///       通过 accepted() 信号异步获取结果。
    void openNonBlockingDialog();

    /// @brief 处理非阻塞对话框的 accepted 信号，提取并显示结果。
    /// @note 使用 QPointer 棔查对话框是否已被删除，
    ///       避免访问已析构对象的悬空指针问题。
    void onDialogAccepted();

    /// @brief 向日志区域追加一条消息。
    /// @param[in] message 要显示的消息文本。
    void appendLog(const QString& message);

    QTextEdit* m_logOutput; ///< 日志输出区域
};
