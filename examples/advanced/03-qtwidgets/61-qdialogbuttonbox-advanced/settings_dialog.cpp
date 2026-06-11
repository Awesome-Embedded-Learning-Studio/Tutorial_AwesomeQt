/// @file    settings_dialog.cpp
/// @brief   演示 QDialogButtonBox 自定义按钮与帮助按钮的对话框实现。
///
/// 对应教程：进阶层 03-QtWidgets/61-QDialogButtonBox 自定义帮助按钮与提示。
/// 实现了带有 Ok/Cancel/Help 按钮的设置对话框，展示 buttonRole()
/// 判断逻辑、工具提示配置和自定义按钮文本。

#include "settings_dialog.h"

#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QMessageBox>
#include <QSpinBox>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
    , m_buttonBox(nullptr)
    , m_usernameEdit(nullptr)
    , m_portSpinBox(nullptr)
    , m_statusLabel(nullptr)
{
    setWindowTitle(tr("Settings (ButtonBox Demo)"));
    setMinimumWidth(400);
    setupUi();
}

QString SettingsDialog::username() const
{
    return m_usernameEdit->text().trimmed();
}

int SettingsDialog::port() const
{
    return m_portSpinBox->value();
}

void SettingsDialog::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);

    // --- 表单区域 ---
    auto* formLayout = new QFormLayout();

    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText(tr("e.g. admin"));
    m_usernameEdit->setToolTip(tr("Enter your login username"));
    formLayout->addRow(tr("Username:"), m_usernameEdit);

    m_portSpinBox = new QSpinBox(this);
    m_portSpinBox->setRange(1, 65535);
    m_portSpinBox->setValue(8080);
    m_portSpinBox->setToolTip(tr("Server port number (1-65535)"));
    formLayout->addRow(tr("Port:"), m_portSpinBox);

    mainLayout->addLayout(formLayout);

    // --- 状态标签 ---
    m_statusLabel = new QLabel(tr("Configure settings and click a button."), this);
    m_statusLabel->setStyleSheet("color: gray; font-style: italic;");
    mainLayout->addWidget(m_statusLabel);

    // --- 按钮盒 ---
    // @note 使用 Qt::StandardButton 枚举一次性创建多个标准按钮，
    //       每个标准按钮自动获得平台原生的文字和图标。
    m_buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this);

    // 添加一个自定义的帮助按钮，使用 HelpRole
    // @note HelpRole 确保帮助按钮出现在按钮盒的左侧（与 Ok/Cancel 分离），
    //       符合各平台对话框按钮的布局惯例。
    auto* helpButton = m_buttonBox->addButton(
        tr("&Help"), QDialogButtonBox::HelpRole);

    // 添加一个自定义的"应用"按钮，使用 ApplyRole
    auto* applyButton = m_buttonBox->addButton(
        tr("&Apply"), QDialogButtonBox::ApplyRole);

    // 为每个按钮设置工具提示
    // @note 标准按钮也可以通过 button() 获取后设置工具提示
    m_buttonBox->button(QDialogButtonBox::Ok)
        ->setToolTip(tr("Save changes and close the dialog"));
    m_buttonBox->button(QDialogButtonBox::Cancel)
        ->setToolTip(tr("Discard changes and close the dialog"));
    helpButton->setToolTip(tr("Open the help documentation"));
    applyButton->setToolTip(tr("Apply changes without closing the dialog"));

    mainLayout->addWidget(m_buttonBox);

    // --- 连接信号 ---
    // clicked 信号携带被点击的按钮指针，可在槽中通过 buttonRole 判断
    connect(m_buttonBox, &QDialogButtonBox::clicked,
            this, &SettingsDialog::handleButtonClicked);
}

void SettingsDialog::handleButtonClicked(QAbstractButton* button)
{
    // @note buttonRole() 返回按钮在按钮盒中的角色，
    //       这比直接比较按钮指针更灵活，因为同一角色可能有多个按钮。
    QDialogButtonBox::ButtonRole role = m_buttonBox->buttonRole(button);
    QString buttonText = button->text();

    switch (role)
    {
    case QDialogButtonBox::AcceptRole:
        // Ok 按钮属于 AcceptRole
        m_statusLabel->setText(
            tr("Accepted with Username=%1, Port=%2")
                .arg(username())
                .arg(port()));
        m_statusLabel->setStyleSheet("color: green;");
        accept();
        break;

    case QDialogButtonBox::RejectRole:
        // Cancel 按钮属于 RejectRole
        m_statusLabel->setText(tr("Rejected."));
        m_statusLabel->setStyleSheet("color: red;");
        reject();
        break;

    case QDialogButtonBox::HelpRole:
        // Help 按钮不会关闭对话框
        QMessageBox::information(
            this, tr("Help"),
            tr("This dialog demonstrates QDialogButtonBox features:\n\n"
               "- Standard Ok/Cancel buttons\n"
               "- Custom Help button with HelpRole\n"
               "- Custom Apply button with ApplyRole\n"
               "- Tooltips on each button\n"
               "- buttonRole() to identify clicked button"));
        break;

    case QDialogButtonBox::ApplyRole:
        // Apply 按钮不关闭对话框，只应用更改
        m_statusLabel->setText(
            tr("Applied: Username=%1, Port=%2 (dialog stays open)")
                .arg(username())
                .arg(port()));
        m_statusLabel->setStyleSheet("color: blue;");
        break;

    default:
        m_statusLabel->setText(
            tr("Unknown button role for: %1").arg(buttonText));
        break;
    }
}
