/// @file    nonlinear_dial.h
/// @brief   演示 QDial 进阶用法：非线性角度到值的映射（对数刻度）与穿越点修正。
///
/// 本示例展示了以下进阶知识点：
/// - 将 QDial 内部的线性 0~99 范围映射为对数刻度输出（模拟音量旋钮）
/// - setWrapping(false) 下 QDial 的有效角度范围（约 270 度，底部有死角）
/// - setNotchesVisible 和 notchCount 的关系——notchTarget 只是建议值
/// - 穿越点修正：wrapping 模式下 0 和 max 之间的方向判断
///
/// 对应教程：进阶层 03-QtWidgets/33-QDial 进阶。

#pragma once

#include <QWidget>

class QDial;
class QLabel;

/// 非线性旋钮演示控件。
///
/// 内部使用一个 QDial（range 0~99，线性），通过对数映射将其转换为
/// 1~1000 的非线性输出值（模拟音频音量的对数感知曲线）。
/// 同时展示 wrapping 和 non-wrapping 模式下的行为差异。
class NonlinearDial : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化界面布局与信号槽连接。
    /// @param[in] parent 父控件指针。
    explicit NonlinearDial(QWidget* parent = nullptr);

private:
    /// @brief 创建旋钮和显示区域。
    void setupUI();

    /// @brief 将 QDial 的线性内部值（0~99）映射为对数刻度输出值。
    /// @param[in] dialValue QDial 内部线性值。
    /// @return 对数映射后的值（范围 1~1000）。
    ///
    /// 对数映射公式：output = 10^(dialValue / 99 * 3)
    /// - dialValue=0  -> 10^0   = 1
    /// - dialValue=50 -> 10^1.5 ~ 31
    /// - dialValue=99 -> 10^3   = 1000
    ///
    /// @note 对数映射模拟了人耳对音量的感知：低音量段更精细，高音量段更粗略。
    static int linearToLog(int dialValue);

    /// @brief QDial 值变化时的处理槽——更新对数输出和穿越点状态。
    /// @param[in] newValue QDial 的原始线性值。
    void handleValueChanged(int newValue);

    /// @brief 切换 wrapping 模式。
    void toggleWrapping();

    // ── 控件成员 ──
    QDial* m_dial;                    // 核心旋钮控件
    QLabel* m_rawValueLabel;          // 显示 QDial 内部线性值
    QLabel* m_logValueLabel;          // 显示对数映射后的值
    QLabel* m_wrappingInfoLabel;      // 显示当前 wrapping 模式说明
    QLabel* m_notchInfoLabel;         // 显示刻度系统信息

    // ── 穿越点修正状态 ──
    int m_lastRawValue;               // 上一次 QDial 的值，用于计算增量
    int m_accumulatedDelta;           // 累计旋转增量（模拟旋转编码器）
};

/// 对数映射的最小和最大输出值。
constexpr int kLogMin = 1;
constexpr int kLogMax = 1000;
