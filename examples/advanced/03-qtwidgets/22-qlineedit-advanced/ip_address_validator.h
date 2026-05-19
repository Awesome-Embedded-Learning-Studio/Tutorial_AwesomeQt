/// @file    ip_address_validator.h
/// @brief   自定义 QValidator 子类，用于 IPv4 地址逐段验证。
///
/// 演示 QValidator 的 validate 虚函数重写、State 返回值语义、
/// 以及 Intermediate 状态的正确使用。
///
/// 对应教程：进阶层 03-QtWidgets/22-QLineEdit 进阶。

#pragma once

#include <QValidator>

/// IPv4 地址验证器。
///
/// 逐段验证 IP 地址（四个 0-255 的十进制数，以点号分隔）。
/// 空字符串或部分输入返回 Intermediate，完整合法输入返回 Acceptable，
/// 非法字符或超范围值返回 Invalid。
class IpAddressValidator : public QValidator
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父对象指针。
    explicit IpAddressValidator(QObject* parent = nullptr);

    /// @brief 验证输入文本的合法性。
    /// @param[in,out] input 当前输入文本。
    /// @param[in,out] pos 光标位置（本验证器不修改）。
    /// @return Acceptable / Intermediate / Invalid。
    State validate(QString& input, int& pos) const override;

    /// @brief 修正输入文本（基类空实现，本验证器不需要 fixup）。
    /// @param[in,out] input 待修正的文本。
    void fixup(QString& input) const override;
};
