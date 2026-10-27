/// @file    widget.h
/// @brief   演示 QPrintDialog 集成预览与打印范围选择。
///
/// 对应教程：进阶层 03-QtWidgets 打印对话框高级用法。
/// 本示例展示如何配置 QPrintDialog 的打印范围选项（PrintRange、
/// from-page/to-page），以及如何根据用户选择的范围只打印指定页面。

#pragma once

#include <QMainWindow>

class QPlainTextEdit;
class QPrinter;

/// @brief 主窗口，提供多页文本编辑区和打印对话框集成。
class PrintDialogMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit PrintDialogMainWindow(QWidget* parent = nullptr);

    /// @brief 析构函数，释放 QPrinter 资源。
    ~PrintDialogMainWindow() override;

private slots:
    /// @brief 打开 QPrintDialog 并根据用户选择执行打印。
    /// @note 对话框允许用户选择打印范围（全部/选区/页码范围），
    ///       打印时只输出用户选定的页面。
    void openPrintDialog();

private:
    /// @brief 初始化 UI：多页文本编辑器和打印按钮。
    void setupUI();

    /// @brief 生成用于演示的多页示例文本。
    /// @note 生成足够多的文本以跨越多个打印页面，方便演示打印范围选择。
    void populateSampleText();

    /// @brief 使用给定 QPrinter 执行实际打印。
    /// @param[in] printer 已配置好打印范围和页面参数的打印机对象。
    /// @note 根据 printRange 设置只绘制用户选定的页面。
    void performPrint(QPrinter* printer);

    QPlainTextEdit* m_textEdit;   ///< 纯文本编辑器，含多页示例内容
    QPrinter* m_printer;          ///< 持久化打印机对象，保留用户设置
};
