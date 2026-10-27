/// @file    wizard_demo.cpp
/// @brief   所有向导页面类及 DemoWizard 的实现。
///
/// 对应教程：进阶层 03-QtWidgets/69-QWizard 进阶。

#include "wizard_demo.h"

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QVBoxLayout>

// ============================================================================
// ChoicePage
// ============================================================================

ChoicePage::ChoicePage(QWidget* parent)
    : QWizardPage(parent)
    , m_choiceCombo(nullptr)
{
    setTitle(tr("Choose Configuration Type"));
    setSubTitle(tr("Select a type to determine which options are available."));

    auto* layout = new QVBoxLayout(this);

    auto* label = new QLabel(tr("Configuration type:"), this);
    m_choiceCombo = new QComboBox(this);
    m_choiceCombo->addItem(tr("Type A — Quick Setup"), 0);
    m_choiceCombo->addItem(tr("Type B — Advanced Setup"), 1);

    layout->addWidget(label);
    layout->addWidget(m_choiceCombo);

    // 注册字段以便后续页面访问
    registerField("choiceIndex*", m_choiceCombo, "currentIndex", "currentIndexChanged");
}

int ChoicePage::nextId() const
{
    // @note 根据用户选择返回不同页面 ID，实现非线性跳转。
    // QWizard 会跳转到 nextId() 返回的页面，跳过中间不相关的页面。
    const int choice = m_choiceCombo->currentData().toInt();
    if (choice == 0) {
        return static_cast<int>(WizardPageId::kPathAPage);
    }
    return static_cast<int>(WizardPageId::kPathBPage);
}

int ChoicePage::selectedChoice() const
{
    return m_choiceCombo->currentData().toInt();
}

// ============================================================================
// PathAPage
// ============================================================================

PathAPage::PathAPage(QWidget* parent) : QWizardPage(parent)
{
    setTitle(tr("Type A — Quick Setup"));
    setSubTitle(tr("Enter a name for your quick configuration."));

    auto* layout = new QVBoxLayout(this);

    auto* label = new QLabel(tr("Project name:"), this);
    auto* nameEdit = new QLineEdit(this);

    layout->addWidget(label);
    layout->addWidget(nameEdit);

    // @note registerField 让 SummaryPage 可以通过 field() 读取此页的输入值。
    registerField("projectNameA*", nameEdit);
}

int PathAPage::nextId() const
{
    // @note 无论哪条路径，最终都汇合到汇总页。
    return static_cast<int>(WizardPageId::kSummaryPage);
}

// ============================================================================
// PathBPage
// ============================================================================

PathBPage::PathBPage(QWidget* parent) : QWizardPage(parent)
{
    setTitle(tr("Type B — Advanced Setup"));
    setSubTitle(tr("Configure advanced parameters."));

    auto* layout = new QVBoxLayout(this);

    auto* label = new QLabel(tr("Module name:"), this);
    auto* moduleEdit = new QLineEdit(this);

    layout->addWidget(label);
    layout->addWidget(moduleEdit);

    registerField("moduleNameB*", moduleEdit);
}

int PathBPage::nextId() const
{
    return static_cast<int>(WizardPageId::kSummaryPage);
}

// ============================================================================
// SummaryPage
// ============================================================================

SummaryPage::SummaryPage(QWidget* parent)
    : QWizardPage(parent)
    , m_summaryLabel(nullptr)
{
    setTitle(tr("Summary"));
    setSubTitle(tr("Review your choices before finishing."));

    auto* layout = new QVBoxLayout(this);
    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setWordWrap(true);

    layout->addWidget(m_summaryLabel);
}

void SummaryPage::initializePage()
{
    // @note initializePage 在页面显示前调用，此时前序页面的字段值已经可用。
    // 通过 field() 读取注册的字段，动态构建汇总文本。
    const int choice = field("choiceIndex").toInt();
    QString summary;

    if (choice == 0) {
        const QString name = field("projectNameA").toString();
        summary = tr("Type: A (Quick Setup)\n"
                     "Project name: %1")
                      .arg(name);
    } else {
        const QString mod = field("moduleNameB").toString();
        summary = tr("Type: B (Advanced Setup)\n"
                     "Module name: %1")
                      .arg(mod);
    }

    m_summaryLabel->setText(summary);
}

int SummaryPage::nextId() const
{
    // @note 返回 -1 表示这是最后一页，QWizard 将显示 "Finish" 按钮。
    return -1;
}

// ============================================================================
// DemoWizard
// ============================================================================

DemoWizard::DemoWizard(QWidget* parent) : QWizard(parent)
{
    setWindowTitle(tr("QWizard — Non-linear Navigation"));
    setOption(QWizard::HaveHelpButton, false);

    // @note 以指定的 ID 注册各页面，nextId() 返回这些 ID 实现跳转。
    setPage(static_cast<int>(WizardPageId::kChoicePage), new ChoicePage(this));
    setPage(static_cast<int>(WizardPageId::kPathAPage), new PathAPage(this));
    setPage(static_cast<int>(WizardPageId::kPathBPage), new PathBPage(this));
    setPage(static_cast<int>(WizardPageId::kSummaryPage), new SummaryPage(this));

    setStartId(static_cast<int>(WizardPageId::kChoicePage));

    // @note 自定义按钮布局：仅保留 Back、Next/Finish、Cancel，
    //       移除默认的 Help 和 Commit 按钮。
    setButtonLayout({QWizard::BackButton, QWizard::Stretch,
                     QWizard::NextButton, QWizard::FinishButton,
                     QWizard::CancelButton});
}
