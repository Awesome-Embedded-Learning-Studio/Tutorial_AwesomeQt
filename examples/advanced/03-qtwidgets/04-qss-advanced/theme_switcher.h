/// @file    theme_switcher.h
/// @brief   演示 QSS 动态主题切换、选择器特异性与样式级联。
///
/// 对应教程：进阶层 03-QtWidgets/04-QSS 进阶。

#pragma once

#include <QWidget>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;

/// QSS 主题切换与选择器特异性演示控件。
///
/// 展示三个核心知识点：
/// - 运行时动态切换亮色/暗色主题（全部通过全局 QSS，不在 C++ 中单独 setStyleSheet）
/// - QSS 选择器特异性对比：ID 选择器 > 类选择器 > 后代选择器
/// - 样式级联：父控件设置样式表后子控件如何继承与覆盖
class ThemeSwitcher : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化界面布局与信号槽连接。
    /// @param[in] parent 父控件指针。
    explicit ThemeSwitcher(QWidget* parent = nullptr);

private:
    /// @brief 创建主题切换演示区域。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createThemeSwitchSection();

    /// @brief 创建选择器特异性对比演示区域。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createSelectorSpecificitySection();

    /// @brief 创建样式级联演示区域。
    /// @return 包含完整演示控件的 QWidget 指针。
    QWidget* createCascadeSection();

    /// @brief 应用指定主题到 QApplication 全局样式表。
    /// @param[in] themeName 主题名称（"light" 或 "dark"）。
    void applyTheme(const QString& themeName);

    /// @brief 更新特异性对比区域的说明文字。
    void updateSpecificityInfo();

private:
    QComboBox* m_themeCombo;          // 主题选择下拉框
    QLabel* m_themeInfo;              // 当前主题说明

    QPushButton* m_specificityIdBtn;  // ID 选择器控制的按钮
    QPushButton* m_specificityClassBtn; // 类选择器控制的按钮
    QPushButton* m_specificityDescBtn;  // 后代选择器控制的按钮
    QLabel* m_specificityInfo;        // 特异性说明

    QWidget* m_cascadeParent;         // 级联演示父容器
    QLabel* m_cascadeParentLabel;     // 父容器标签
    QPushButton* m_cascadeChildBtn;   // 级联演示子按钮
    QLabel* m_cascadeInfo;            // 级联说明

    QString m_currentTheme;           // 当前主题名称
};
