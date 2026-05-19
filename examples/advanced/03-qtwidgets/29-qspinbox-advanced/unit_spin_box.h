/// @file    unit_spin_box.h
/// @brief   带动态单位的整数微调框——通过重写 textFromValue/valueFromText 实现。
///
/// 演示 QSpinBox 定制化的核心模式：
/// - textFromValue()：根据值大小自动选择合适的单位（px / kb / MB / GB）并格式化
/// - valueFromText()：反向解析带单位的文本，支持多种单位格式
/// - 不使用 prefix/suffix，所有格式化逻辑集中在虚函数中
///
/// 对应教程：进阶层 03-QtWidgets/29-QSpinBox 进阶。

#pragma once

#include <QSpinBox>

/// 带动态单位切换的整数微调框。
///
/// 内部值表示"字节数"，但显示时根据大小自动选择最合适的单位：
///   - < 1024           → "XXX B"   （字节）
///   - 1024 ~ 1048575   → "XXX KB"  （千字节，保留 1 位小数）
///   - 1048576 ~ 更大    → "XXX MB"  （兆字节，保留 1 位小数）
///
/// 用户也可以手动输入 "500 KB" 或 "2.5 MB" 这样的文本，
/// valueFromText 会正确解析回字节数。
/// 设计原则：textFromValue 和 valueFromText 互为逆运算。
class UnitSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针。
    explicit UnitSpinBox(QWidget* parent = nullptr);

protected:
    // ── QSpinBox 虚函数重写 ──

    /// @brief 将内部字节数值转换为带单位的显示文本。
    /// 根据值大小自动选择 B / KB / MB 单位。
    /// @param[in] value 内部值（字节数）。
    /// @return 格式化的显示文本。
    QString textFromValue(int value) const override;

    /// @brief 将用户输入的带单位文本解析回字节数值。
    /// 支持 "500 B"、"128 KB"、"2.5 MB" 等格式。
    /// @param[in] text 用户输入的文本。
    /// @return 解析后的字节数。
    int valueFromText(const QString& text) const override;

    /// @brief 验证输入文本。
    /// 对可解析的输入返回 Acceptable，否则让基类处理。
    QValidator::State validate(QString& text, int& pos) const override;
};
