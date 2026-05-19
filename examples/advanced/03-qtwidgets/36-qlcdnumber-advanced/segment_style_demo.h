/// @file    segment_style_demo.h
/// @brief   演示 QLCDNumber 的 SegmentStyle 切换、digitCount 溢出行为与 setMode 进制转换。
///
/// 对应教程：进阶层 03-QtWidgets/36-QLCDNumber 进阶。

#pragma once

#include <QWidget>

class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLCDNumber;
class QPushButton;
class QSpinBox;

/// QLCDNumber SegmentStyle 与进制模式演示控件。
///
/// 展示三个核心知识点：
/// - 三种 SegmentStyle（Outline / Filled / Flat）的视觉差异
/// - digitCount 精确计算规则：小数点、负号、smallDecimalPoint 对占位的影响
/// - setMode 进制切换：内部存储值与显示值的分离机制
class SegmentStyleDemo : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化界面布局与信号槽连接。
    /// @param[in] parent 父控件指针。
    explicit SegmentStyleDemo(QWidget* parent = nullptr);

private:
    /// @brief 创建 SegmentStyle 三种风格对比区域。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createStyleSection();

    /// @brief 创建 digitCount 溢出行为演示区域。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createOverflowSection();

    /// @brief 创建 setMode 进制切换演示区域。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createModeSection();

    /// @brief 更新 SegmentStyle 演示区的三个 LCD 显示。
    void updateStyleDisplay();

    /// @brief 检测当前值是否会导致溢出并更新提示。
    void checkOverflow();

    /// @brief 切换进制模式并重新显示当前值。
    void switchMode();

private:
    // --- SegmentStyle 演示区成员 ---
    QLCDNumber* m_outlineLcd;       // Outline 风格 LCD
    QLCDNumber* m_filledLcd;        // Filled 风格 LCD
    QLCDNumber* m_flatLcd;          // Flat 风格 LCD
    QSpinBox* m_styleValueSpin;     // SegmentStyle 区的值输入

    // --- digitCount 溢出演示区成员 ---
    QLCDNumber* m_overflowLcd;      // 用于观察溢出的 LCD
    QSpinBox* m_digitCountSpin;     // digitCount 调节
    QDoubleSpinBox* m_overflowValueSpin;  // 测试溢出的值输入
    QLabel* m_overflowInfo;         // 溢出状态提示标签

    // --- 进制切换演示区成员 ---
    QLCDNumber* m_modeLcd;          // 进制切换 LCD
    QSpinBox* m_modeValueSpin;      // 进制切换的值输入
    QLabel* m_modeInfo;             // 显示当前内部 value() 和 mode 的标签
    QComboBox* m_modeCombo;         // 进制选择下拉框
};
