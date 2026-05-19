/// @file    validated_dialog.h
/// @brief   带输入验证的对话框——演示 accept() 中拦截非法输入的正确模式。
///
/// 对应教程：进阶层 03-QtWidgets/06-对话框进阶。

#pragma once

#include <QDialog>

class QLineEdit;
class QLabel;
class QPushButton;

/// 带验证的输入对话框。
///
/// 用户输入一个端口号（1-65535），通过重写 accept() 在对话框关闭前执行验证。
/// 验证不通过时对话框保持打开，不调用 reject() 以避免污染 exec() 返回值。
/// OK 按钮的启用/禁用状态根据输入合法性实时更新。
class ValidatedDialog : public QDialog
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] title 对话框标题。
    /// @param[in] parent 父控件指针。
    explicit ValidatedDialog(const QString& title, QWidget* parent = nullptr);

    /// @brief 获取用户输入的端口号。
    /// @return 合法的端口号（1-65535），若用户取消返回 -1。
    int portValue() const;

protected:
    /// @brief 重写 accept——验证通过才真正关闭对话框。
    /// @note 验证失败时直接 return，不调 reject() 也不调 QDialog::accept()。
    void accept() override;

private:
    /// @brief 实时验证输入内容并更新 UI 状态。
    void validateInput();

    QLineEdit* m_portEdit;      // 端口输入框
    QLabel* m_validationHint;   // 验证提示标签
    QPushButton* m_okButton;    // 确定按钮（根据验证状态启用/禁用）
    int m_portValue;            // 最终验证通过的端口号
};
