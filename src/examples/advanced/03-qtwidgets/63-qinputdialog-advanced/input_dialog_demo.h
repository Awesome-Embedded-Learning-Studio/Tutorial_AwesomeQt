/// @file    input_dialog_demo.h
/// @brief   演示 QInputDialog 结合自定义 QValidator 与输入范围约束。
///
/// 对应教程：进阶层 03-QtWidgets/63-QInputDialog 自定义验证器与输入范围。
/// 展示 Text/Int/Double/Item 四种输入模式，以及 QRegularExpressionValidator
/// 对文本输入的实时校验能力。

#pragma once

#include <QWidget>

class QLabel;
class QPushButton;
class QRegularExpressionValidator;

/// @brief 主窗口，演示 QInputDialog 的四种输入模式与自定义验证器。
class InputDialogDemo : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit InputDialogDemo(QWidget* parent = nullptr);

private slots:
    /// @brief 打开文本输入对话框，使用邮箱格式验证器。
    void openTextDialog();

    /// @brief 打开整数输入对话框，带范围和步长约束。
    void openIntDialog();

    /// @brief 打开浮点数输入对话框，带精度和范围约束。
    void openDoubleDialog();

    /// @brief 打开条目选择对话框，从预设列表中选择。
    void openItemDialog();

private:
    /// @brief 初始化界面布局与信号槽连接。
    /// @note 所有子控件通过 Qt 父对象机制管理，无需手动 delete。
    void setupUI();

    /// @brief 更新结果标签的显示文本。
    /// @param[in] text 要显示的结果文本。
    void updateResult(const QString& text);

    QLabel* m_resultLabel;                     ///< 显示对话框返回结果的标签
    QPushButton* m_textBtn;                    ///< 打开文本输入对话框的按钮
    QPushButton* m_intBtn;                     ///< 打开整数输入对话框的按钮
    QPushButton* m_doubleBtn;                  ///< 打开浮点数输入对话框的按钮
    QPushButton* m_itemBtn;                    ///< 打开条目选择对话框的按钮
    QRegularExpressionValidator* m_emailValidator;  ///< 邮箱格式验证器
};
