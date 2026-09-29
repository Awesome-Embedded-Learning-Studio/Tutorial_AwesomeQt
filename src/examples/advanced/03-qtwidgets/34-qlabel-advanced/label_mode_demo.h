/// @file    label_mode_demo.h
/// @brief   演示 QLabel 的 setTextFormat 三种模式、setBuddy 快捷键与文本截断。
///
/// 对应教程：进阶层 03-QtWidgets/34-QLabel 进阶。

#pragma once

#include <QLabel>
#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;

/// QLabel 进阶用法演示控件。
///
/// 展示三个核心知识点：
/// - setTextFormat 三种模式（PlainText / RichText / AutoText）的行为差异
/// - setBuddy 快捷键注册机制及其与文本中 & 标记的依赖关系
/// - QFontMetrics::elidedText 文本截断的正确用法
class LabelModeDemo : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化界面布局与信号槽连接。
    /// @param[in] parent 父控件指针。
    explicit LabelModeDemo(QWidget* parent = nullptr);

private:
    /// @brief 创建 setTextFormat 三种模式对比演示区域。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createTextFormatSection();

    /// @brief 创建 setBuddy 快捷键演示区域。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createBuddySection();

    /// @brief 创建 elidedText 文本截断演示区域。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createElideSection();

    /// @brief 根据用户选择的格式模式更新所有演示标签的显示。
    void updateFormatDisplay();

    /// @brief 演示 setBuddy 快捷键——在文本有 & 标记和无标记两种情况之间切换。
    void toggleBuddyText();

    /// @brief 根据当前标签宽度重新计算 elidedText 截断。
    void updateElideDisplay();

private:
    // --- setTextFormat 演示区成员 ---
    QLineEdit* m_formatInput;        // 用户输入的测试文本
    QComboBox* m_formatCombo;        // 格式选择下拉框
    QLabel* m_plainLabel;            // PlainText 模式标签
    QLabel* m_richLabel;             // RichText 模式标签
    QLabel* m_autoLabel;             // AutoText 模式标签

    // --- setBuddy 演示区成员 ---
    QLabel* m_buddyLabel;            // 带 buddy 关联的标签
    QLineEdit* m_buddyEdit;          // buddy 目标输入框
    QPushButton* m_toggleBuddyBtn;   // 切换标签文本（含 & 或不含）
    bool m_buddyHasMarker;           // 当前标签文本是否包含 & 快捷键标记

    // --- elidedText 演示区成员 ---
    QLabel* m_elideLabel;            // 被截断的标签
    QLabel* m_elideInfo;             // 显示截断方式信息的标签
    QLineEdit* m_elideInput;         // 用户输入的长文本
    QComboBox* m_elideModeCombo;     // 截断模式选择（左/中/右）
};
