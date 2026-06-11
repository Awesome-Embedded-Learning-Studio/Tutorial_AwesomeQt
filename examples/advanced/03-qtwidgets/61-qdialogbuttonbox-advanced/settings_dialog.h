/// @file    settings_dialog.h
/// @brief   演示 QDialogButtonBox 自定义按钮与帮助按钮的对话框声明。
///
/// 对应教程：进阶层 03-QtWidgets/61-QDialogButtonBox 自定义帮助按钮与提示。
/// 本文件声明了一个设置对话框，展示 QDialogButtonBox 的标准按钮、
/// 自定义帮助按钮、工具提示以及 buttonRole() 判断逻辑。

#pragma once

#include <QDialog>

class QAbstractButton;
class QDialogButtonBox;
class QLineEdit;
class QSpinBox;
class QLabel;

/// @brief 带 QDialogButtonBox 的设置对话框，演示按钮角色与自定义按钮。
///
/// 功能点：
/// - 添加 Ok/Cancel 标准按钮和 Help 自定义按钮
/// - 通过 buttonRole() 判断用户点击了哪个角色的按钮
/// - 为每个按钮设置独立的工具提示
/// - 演示自定义按钮文本和图标
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化界面和按钮盒。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit SettingsDialog(QWidget* parent = nullptr);

    /// @brief 获取用户名设置。
    /// @return 用户名文本。
    QString username() const;

    /// @brief 获取端口设置。
    /// @return 端口号。
    int port() const;

private:
    /// @brief 初始化界面布局。
    void setupUi();

    /// @brief 处理按钮盒中按钮的点击事件。
    /// @param[in] button 被点击的按钮指针。
    /// @note 通过 buttonRole() 识别按钮角色，而非比较按钮指针，
    ///       这样可以正确处理标准按钮和自定义按钮的混合使用。
    void handleButtonClicked(QAbstractButton* button);

    QDialogButtonBox* m_buttonBox; ///< 按钮盒控件
    QLineEdit* m_usernameEdit;     ///< 用户名输入框
    QSpinBox* m_portSpinBox;       ///< 端口号输入框
    QLabel* m_statusLabel;         ///< 状态提示标签
};
