/// @file    main_window.h
/// @brief   演示 QMessageBox 自定义图标与详情区域的主窗口类声明。
///
/// 对应教程：进阶层 03-QtWidgets/62-QMessageBox 自定义图标与详情区域。
/// 本文件声明了主窗口，包含多个按钮分别触发不同级别的 QMessageBox，
/// 展示自定义图标、详细文本、自定义按钮等功能。

#pragma once

#include <QMainWindow>

class QTextEdit;

/// @brief 主窗口，演示 QMessageBox 的各种高级用法。
///
/// 展示的功能点：
/// - 程序化绘制自定义图标并设置到 QMessageBox
/// - setDetailedText() 创建可展开的详情区域
/// - addButton() 添加自定义按钮
/// - 不同严重级别的消息框（Information/Warning/Critical/Question）
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化界面。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit MainWindow(QWidget* parent = nullptr);

private:
    /// @brief 显示带有自定义图标的信息消息框。
    /// @note 使用程序化绘制的 QPixmap 作为图标，
    ///       避免依赖外部资源文件。
    void showInformationBox();

    /// @brief 显示带有详细文本的警告消息框。
    /// @note setDetailedText() 会创建一个"Show Details"按钮，
    ///       点击后展开显示完整的详细内容。
    void showWarningBox();

    /// @brief 显示带有自定义按钮的严重错误消息框。
    /// @note addButton() 的第三个参数指定按钮角色，
    ///       QMessageBox 根据角色决定按钮顺序和默认行为。
    void showCriticalBox();

    /// @brief 显示带有自定义图标的询问消息框。
    /// @note 程序化绘制问号图标，展示不依赖系统图标的方案。
    void showQuestionBox();

    /// @brief 程序化绘制一个信息图标（蓝色圆圈 + i 字母）。
    /// @param[in] size 图标的像素尺寸。
    /// @return 绘制完成的 QPixmap。
    QPixmap createInfoIcon(int size) const;

    /// @brief 程序化绘制一个警告图标（黄色三角 + ! 感叹号）。
    /// @param[in] size 图标的像素尺寸。
    /// @return 绘制完成的 QPixmap。
    QPixmap createWarningIcon(int size) const;

    /// @brief 向日志区域追加一条消息。
    /// @param[in] message 要显示的消息文本。
    void appendLog(const QString& message);

    QTextEdit* m_logOutput; ///< 日志输出区域
};
