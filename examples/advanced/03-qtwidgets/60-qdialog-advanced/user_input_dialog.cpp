/// @file    user_input_dialog.cpp
/// @brief   演示异步对话框（非阻塞）与结果回调的自定义 QDialog 实现。
///
/// 对应教程：进阶层 03-QtWidgets/60-QDialog 异步对话框与结果回调。
/// 实现了包含姓名/邮箱输入的自定义对话框，支持阻塞与非阻塞两种模式。

#include "user_input_dialog.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

UserInputDialog::UserInputDialog(QWidget* parent)
    : QDialog(parent)
    , m_nameEdit(nullptr)
    , m_emailEdit(nullptr)
{
    setWindowTitle(tr("User Information"));
    setMinimumWidth(350);
    setupUi();
}

QString UserInputDialog::name() const
{
    return m_nameEdit->text().trimmed();
}

QString UserInputDialog::email() const
{
    return m_emailEdit->text().trimmed();
}

void UserInputDialog::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);

    // 使用表单布局对齐标签和输入框
    auto* formLayout = new QFormLayout();

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText(tr("Enter your name"));
    formLayout->addRow(tr("Name:"), m_nameEdit);

    m_emailEdit = new QLineEdit(this);
    m_emailEdit->setPlaceholderText(tr("Enter your email"));
    formLayout->addRow(tr("Email:"), m_emailEdit);

    mainLayout->addLayout(formLayout);

    // 提示标签
    auto* hintLabel = new QLabel(
        tr("Both fields are required to submit."), this);
    hintLabel->setStyleSheet("color: gray;");
    mainLayout->addWidget(hintLabel);

    // 按钮布局
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    auto* okButton = new QPushButton(tr("OK"), this);
    auto* cancelButton = new QPushButton(tr("Cancel"), this);

    // @note setDefault 让用户按 Enter 时触发 OK 按钮，
    //       配合 validateAndAccept 做输入校验。
    okButton->setDefault(true);

    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    // 连接按钮信号
    connect(okButton, &QPushButton::clicked,
            this, &UserInputDialog::validateAndAccept);
    connect(cancelButton, &QPushButton::clicked,
            this, &QDialog::reject);
}

void UserInputDialog::validateAndAccept()
{
    // 简单的非空校验
    if (m_nameEdit->text().trimmed().isEmpty()
        || m_emailEdit->text().trimmed().isEmpty())
    {
        // @note 不调用 accept()，对话框保持打开，用户可以修正输入
        return;
    }

    accept();
}
