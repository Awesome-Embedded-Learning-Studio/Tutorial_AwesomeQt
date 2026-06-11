/// @file    wizard_demo.h
/// @brief   演示 QWizard 非线性跳转与动态页面生成。
///
/// 对应教程：进阶层 03-QtWidgets/69-QWizard 进阶。
/// 核心知识点：重写 nextId() 实现分支跳转、动态 addPage/removePage、
///             自定义 setButtonLayout()。

#pragma once

#include <QWizard>
#include <QWizardPage>

class QComboBox;
class QLabel;
class QLineEdit;
class QRadioButton;

// ============================================================================
// 自定义向导页面
// ============================================================================

/// @brief 第 1 页：选择类型（A 或 B），决定后续分支路径。
class ChoicePage : public QWizardPage
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针。
    explicit ChoicePage(QWidget* parent = nullptr);

    /// @brief 重写 nextId，根据用户选择返回不同的下一页 ID。
    /// @note 这是非线性跳转的核心机制：QWizard 调用此函数决定 "Next" 指向哪页。
    int nextId() const override;

    /// @brief 获取用户选择的类型索引。
    int selectedChoice() const;

private:
    QComboBox* m_choiceCombo;  ///< 类型选择下拉框
};

/// @brief 第 2A 页：路径 A 的配置页，仅在用户选择类型 A 时出现。
class PathAPage : public QWizardPage
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针。
    explicit PathAPage(QWidget* parent = nullptr);

    /// @brief 重写 nextId，跳转到汇总页。
    int nextId() const override;
};

/// @brief 第 2B 页：路径 B 的配置页，仅在用户选择类型 B 时出现。
class PathBPage : public QWizardPage
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针。
    explicit PathBPage(QWidget* parent = nullptr);

    /// @brief 重写 nextId，跳转到汇总页。
    int nextId() const override;
};

/// @brief 最终汇总页，显示用户的所有选择。
class SummaryPage : public QWizardPage
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针。
    explicit SummaryPage(QWidget* parent = nullptr);

    /// @brief 重写 initializePage，在页面显示前动态更新汇总文本。
    /// @note initializePage 在页面即将显示时调用，此时可读取前序页面数据。
    void initializePage() override;

    /// @brief 重写 nextId，返回 -1 表示这是最后一页。
    int nextId() const override;

private:
    QLabel* m_summaryLabel;  ///< 汇总信息标签
};

// ============================================================================
// 主向导窗口
// ============================================================================

/// @brief 页面 ID 常量，用于 nextId() 返回和页面注册。
enum class WizardPageId : int
{
    kChoicePage = 1,   ///< 类型选择页
    kPathAPage = 2,    ///< 路径 A 配置页
    kPathBPage = 3,    ///< 路径 B 配置页
    kSummaryPage = 4,  ///< 汇总页
};

/// @brief 主向导窗口，管理所有页面并演示非线性跳转。
class DemoWizard : public QWizard
{
    Q_OBJECT

public:
    /// @brief 构造函数，注册所有页面并配置按钮布局。
    /// @param[in] parent 父控件指针。
    /// @note 动态页面切换由各页面的 nextId() 驱动，
    ///       而非在构造函数中增删页面。
    explicit DemoWizard(QWidget* parent = nullptr);
};
