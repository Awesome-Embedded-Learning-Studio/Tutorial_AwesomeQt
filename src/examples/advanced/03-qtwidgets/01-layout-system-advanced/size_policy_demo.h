/// @file    size_policy_demo.h
/// @brief   演示 QSizePolicy 六种策略的空间分配行为与 stretch 因子。
///
/// 对应教程：进阶层 03-QtWidgets/01-布局系统进阶。

#pragma once

#include <QFrame>
#include <QWidget>

class QComboBox;
class QLabel;
class QSpinBox;

/// QSizePolicy 六种策略可视化演示控件。
///
/// 展示三个核心知识点：
/// - 六种 sizePolicy（Fixed/Minimum/Maximum/Preferred/Expanding/Ignored）在水平布局中的空间分配差异
/// - 动态切换控件的 sizePolicy，观察布局如何重新分配空间
/// - stretch 因子对可膨胀控件的按比例分配效果
class SizePolicyDemo : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化界面布局与信号槽连接。
    /// @param[in] parent 父控件指针。
    explicit SizePolicyDemo(QWidget* parent = nullptr);

private:
    /// @brief 创建六种策略并排对比演示区域。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createPolicyComparisonSection();

    /// @brief 创建动态切换 sizePolicy 演示区域。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createDynamicSwitchSection();

    /// @brief 创建 stretch 因子演示区域。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createStretchSection();

    /// @brief 根据下拉框选择动态切换目标控件的 sizePolicy。
    void updateDynamicPolicy();

    /// @brief 更新 stretch 演示区域的 stretch 值并刷新布局。
    void updateStretchValues();

    /// @brief 为策略名和枚举值创建一个带颜色的色条控件。
    /// @param[in] policyName 策略名称标签。
    /// @param[in] color 色条的背景色。
    /// @param[in] policy 对应的 QSizePolicy::Policy。
    /// @param[in] parent 父控件。
    /// @return 创建的色条 QFrame 指针。
    QFrame* createPolicyBar(const QString& policyName, const QString& color,
                            QSizePolicy::Policy policy, QWidget* parent = nullptr);

private:
    // --- 动态切换演示区成员 ---
    QFrame* m_dynamicBar;              // 被动态切换策略的色条
    QComboBox* m_policyCombo;          // 策略选择下拉框
    QLabel* m_dynamicInfo;             // 显示当前策略信息的标签

    // --- stretch 演示区成员 ---
    QFrame* m_stretchBarA;             // stretch 演示用色条 A
    QFrame* m_stretchBarB;             // stretch 演示用色条 B
    QFrame* m_stretchBarC;             // stretch 演示用色条 C
    QSpinBox* m_spinA;                 // 色条 A 的 stretch 值
    QSpinBox* m_spinB;                 // 色条 B 的 stretch 值
    QSpinBox* m_spinC;                 // 色条 C 的 stretch 值
    QLabel* m_stretchInfo;             // 显示 stretch 分配信息

    // --- 策略对比演示区色条（需要 resize 时更新信息） ---
    QFrame* m_barFixed;                // Fixed 策略色条
    QFrame* m_barMinimum;              // Minimum 策略色条
    QFrame* m_barMaximum;              // Maximum 策略色条
    QFrame* m_barPreferred;            // Preferred 策略色条
    QFrame* m_barExpanding;            // Expanding 策略色条
    QFrame* m_barIgnored;              // Ignored 策略色条
    QLabel* m_comparisonInfo;          // 六种策略对比信息
};
