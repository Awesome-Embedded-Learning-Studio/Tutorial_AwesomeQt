/// @file    custom_validator_demo.h
/// @brief   演示 QLineEdit 的自定义 QValidator、输入掩码与 QCompleter 集成。
///
/// 对应教程：进阶层 03-QtWidgets/22-QLineEdit 进阶。

#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;

/// QLineEdit 进阶用法演示控件。
///
/// 展示三个核心知识点：
/// - 自定义 IpAddressValidator 验证器逐段验证 IP 地址
/// - setInputMask 输入掩码的字符级格式控制
/// - QCompleter 自动补全与自定义词表集成
class CustomValidatorDemo : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化界面布局与信号槽连接。
    /// @param[in] parent 父控件指针。
    explicit CustomValidatorDemo(QWidget* parent = nullptr);

private:
    /// @brief 创建 IP 地址验证器演示区域。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createValidatorSection();

    /// @brief 创建输入掩码演示区域。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createInputMaskSection();

    /// @brief 创建 QCompleter 自动补全演示区域。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createCompleterSection();

    /// @brief 更新验证器状态标签。
    void updateValidatorState();

    // --- 验证器演示区成员 ---
    QLineEdit* m_validatorEdit;     // IP 地址输入框
    QLabel* m_validatorState;       // 验证状态显示标签

    // --- 输入掩码演示区成员 ---
    QLineEdit* m_maskEdit;          // 带输入掩码的电话号码输入框

    // --- 自动补全演示区成员 ---
    QLineEdit* m_completerEdit;     // 带自动补全的搜索框
};
