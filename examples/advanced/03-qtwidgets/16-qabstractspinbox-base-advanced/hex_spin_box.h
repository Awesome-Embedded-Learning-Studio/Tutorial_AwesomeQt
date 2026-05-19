/// @file    hex_spin_box.h
/// @brief   十六进制输入微调框——通过子类化 QAbstractSpinBox 实现自定义数值基。
///
/// 演示 QAbstractSpinBox 三个核心虚函数的协作：
/// - validate()：十六进制字符验证（0-9, a-f, A-F）
/// - fixup()：自动修正非法输入（去掉无效字符、钳位到范围）
/// - stepBy()：根据光标位置决定步进幅度（+0x1, +0x10, +0x100...）
///
/// 对应教程：进阶层 03-QtWidgets/16-QAbstractSpinBox 基类进阶。

#pragma once

#include <QAbstractSpinBox>

class QLabel;

/// 十六进制微调框，继承 QAbstractSpinBox 而非 QSpinBox。
///
/// 核心设计思路：
/// - 内部值以 quintptr 存储（无符号整数，足够容纳地址宽度）
/// - validate 只允许 0-9、a-f、A-F 以及可选的 "0x" 前缀
/// - stepBy 根据光标所在数位决定步进量，体验类似十六进制编辑器
class HexSpinBox : public QAbstractSpinBox
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针。
    explicit HexSpinBox(QWidget* parent = nullptr);

    /// @brief 获取当前十六进制值。
    /// @return 无符号整数值。
    quintptr hexValue() const;

    /// @brief 设置当前十六进制值。
    /// @param[in] value 要设置的值，会被钳位到 [m_min, m_max]。
    void setHexValue(quintptr value);

    /// @brief 设置允许的最小值。
    /// @param[in] min 最小值。
    void setMinimum(quintptr min);

    /// @brief 设置允许的最大值。
    /// @param[in] max 最大值。
    void setMaximum(quintptr max);

    /// @brief 同时设置最小值和最大值。
    /// @param[in] min 最小值。
    /// @param[in] max 最大值。
    void setRange(quintptr min, quintptr max);

Q_SIGNALS:
    /// @brief 值发生变化时发射。
    /// @param newValue 新的十六进制值。
    void hexValueChanged(quintptr newValue);

protected:
    // ── QAbstractSpinBox 虚函数重写 ──

    /// @brief 验证输入文本是否为合法的十六进制表示。
    /// 对可修正的输入返回 Intermediate（让 fixup 有机会介入），
    /// 对完全无法解析的输入返回 Invalid（直接拒绝）。
    QValidator::State validate(QString& text, int& pos) const override;

    /// @brief 修正 Intermediate 状态的文本，使其通过验证。
    /// 去掉非法字符后钳位到 [m_min, m_max]。
    void fixup(QString& text) const override;

    /// @brief 执行步进操作。
    /// 步进幅度取决于光标位置对应的十六进制数位（个位 +1，十位 +0x10，百位 +0x100...）。
    void stepBy(int steps) override;

    /// @brief 控制上下箭头按钮的可用状态。
    /// 值到达边界时禁用对应方向的箭头。
    StepEnabled stepEnabled() const override;

private:
    /// @brief 从显示文本中解析出十六进制数值。
    /// @param[in] text 输入文本。
    /// @return 解析结果，解析失败返回 0。
    quintptr textToValue(const QString& text) const;

    /// @brief 将数值格式化为十六进制显示文本。
    /// @param[in] value 数值。
    /// @return 带 "0x" 前缀的十六进制字符串。
    QString valueToText(quintptr value) const;

    /// @brief 根据当前光标位置计算步进单位。
    /// 光标在第 N 个十六进制数位上，步进单位 = 16^N。
    /// @return 步进单位（1, 0x10, 0x100, ...）。
    quintptr stepUnitFromCursor() const;

    quintptr m_min{0x00000000};   // 最小值
    quintptr m_max{0xFFFFFFFF};   // 最大值
    quintptr m_value{0x00000000}; // 当前值
};
