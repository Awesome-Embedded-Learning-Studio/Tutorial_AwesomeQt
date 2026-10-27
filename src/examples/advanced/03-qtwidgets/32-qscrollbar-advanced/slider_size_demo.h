/// @file    slider_size_demo.h
/// @brief   演示 QScrollBar 手柄大小计算公式及其与 range / pageStep 的联动关系。
///
/// 对应教程：进阶层 03-QtWidgets/32-QScrollBar 进阶。

#pragma once

#include <QWidget>

class QLabel;
class QScrollBar;
class QSpinBox;

/// QScrollBar 手柄大小公式可视化演示控件。
///
/// 核心知识点：
/// - 手柄大小公式：sliderSize = pageStep / (max - min + pageStep) * availableLength
/// - 交互式调整 range / pageStep 参数，实时观察手柄大小变化
/// - range / singleStep / pageStep 三者之间的语义约束关系
class SliderSizeDemo : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化界面布局与信号槽连接。
    /// @param[in] parent 父控件指针。
    explicit SliderSizeDemo(QWidget* parent = nullptr);

private:
    /// @brief 创建参数控制区域（min / max / pageStep / singleStep 输入框）。
    /// @return 包含所有参数控件的 QWidget 指针。
    QWidget* createParamSection();

    /// @brief 创建实时 QScrollBar 演示区域。
    /// @return 包含 QScrollBar 及其标注的 QWidget 指针。
    QWidget* createScrollBarSection();

    /// @brief 创建公式计算结果显示区域。
    /// @return 包含公式文本和计算结果的 QWidget 指针。
    QWidget* createFormulaSection();

    /// @brief 根据当前参数重新计算手柄大小并刷新所有显示。
    void updateDisplay();

private:
    // 参数输入控件
    QSpinBox* m_minSpin;           // range 最小值
    QSpinBox* m_maxSpin;           // range 最大值
    QSpinBox* m_pageStepSpin;      // pageStep 值
    QSpinBox* m_singleStepSpin;    // singleStep 值

    // 演示用滚动条
    QScrollBar* m_scrollBar;       // 实时反映参数的 QScrollBar

    // 公式结果显示
    QLabel* m_formulaLabel;        // 显示完整公式及代入数值
    QLabel* m_resultLabel;         // 显示计算结果和手柄占比
    QLabel* m_warningLabel;        // 参数异常时的警告提示
};
