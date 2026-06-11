/// @file    user_input_dialog.h
/// @brief   演示异步对话框（非阻塞）与结果回调的自定义 QDialog 声明。
///
/// 对应教程：进阶层 03-QtWidgets/60-QDialog 异步对话框与结果回调。
/// 本文件声明了一个包含姓名和邮箱输入框的自定义对话框，
/// 支持 exec()（阻塞）和 show()（非阻塞）两种使用模式。

#pragma once

#include <QDialog>

class QLineEdit;
class QLabel;

/// @brief 带有姓名和邮箱输入的自定义对话框。
///
/// 演示 QDialog 的两种使用方式：
/// 1. 阻塞模式（exec）：对话框关闭后才返回结果。
/// 2. 非阻塞模式（show）：对话框打开后主窗口仍可操作，
///    通过 accepted() 信号异步获取结果。
class UserInputDialog : public QDialog
{
    Q_OBJECT

public:
    /// @brief 构造函数，创建输入控件和按钮。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit UserInputDialog(QWidget* parent = nullptr);

    /// @brief 获取用户输入的姓名。
    /// @return 姓名字符串，若对话框被取消则返回空串。
    QString name() const;

    /// @brief 获取用户输入的邮箱。
    /// @return 邮箱字符串，若对话框被取消则返回空串。
    QString email() const;

private:
    /// @brief 初始化界面布局与信号槽连接。
    void setupUi();

    /// @brief 验证输入内容，决定是否接受对话框。
    /// @note 非空校验确保 accepted() 信号只在有效输入时才发射。
    void validateAndAccept();

    QLineEdit* m_nameEdit;  ///< 姓名输入框
    QLineEdit* m_emailEdit; ///< 邮箱输入框
};
