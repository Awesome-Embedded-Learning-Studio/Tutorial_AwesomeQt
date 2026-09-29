/// @file    platform_style_demo.h
/// @brief   演示 QCommandLinkButton 的平台样式差异、description 文本与动态更新。
///
/// 对应教程：进阶层 03-QtWidgets/21-QCommandLinkButton 进阶。

#pragma once

#include <QWidget>

class QCommandLinkButton;
class QLabel;
class QPushButton;

/// QCommandLinkButton 平台样式与 description 行为演示控件。
///
/// 展示三个核心知识点：
/// - QCommandLinkButton 带 description 文本的基本用法
/// - 动态更新 description 文本对 sizeHint 和布局的影响
/// - 三种实现"标题+描述"按钮的方案对比（原生、QSS美化、自定义widget）
class PlatformStyleDemo : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化界面布局与信号槽连接。
    /// @param[in] parent 父控件指针。
    explicit PlatformStyleDemo(QWidget* parent = nullptr);

private:
    /// @brief 创建原生 QCommandLinkButton 演示区域。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createNativeSection();

    /// @brief 创建 QSS 美化的 QCommandLinkButton 演示区域。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createStyledSection();

    /// @brief 创建动态 description 切换演示区域。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createDynamicSection();

    /// @brief 处理用户选择结果，更新状态标签。
    /// @param[in] choice 用户选择的安装方式名称。
    void onChoiceSelected(const QString& choice);

private:
    QLabel* m_resultLabel;           // 显示用户选择结果的状态标签

    // --- 动态 description 区域成员 ---
    QCommandLinkButton* m_dynamicBtn;   // 动态切换描述的按钮
    QPushButton* m_toggleDescBtn;       // 触发描述文本切换的按钮
    int m_descIndex;                    // 当前描述文本索引
};
