// QtWidgets 入门示例 06: 对话框体系基础
// UserInfoDialog 实现

#include "userinfodialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>

UserInfoDialog::UserInfoDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("编辑个人信息");
    setMinimumWidth(400);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);

    // ---- 表单区域 ----
    auto *formLayout = new QFormLayout;
    formLayout->setSpacing(10);

    m_nameEdit = new QLineEdit;
    m_nameEdit->setPlaceholderText("请输入姓名");
    formLayout->addRow("姓名:", m_nameEdit);

    m_ageSpin = new QSpinBox;
    m_ageSpin->setRange(1, 150);
    m_ageSpin->setValue(25);
    formLayout->addRow("年龄:", m_ageSpin);

    m_emailEdit = new QLineEdit;
    m_emailEdit->setPlaceholderText("user@example.com");
    formLayout->addRow("邮箱:", m_emailEdit);

    m_bioEdit = new QTextEdit;
    m_bioEdit->setMaximumHeight(80);
    m_bioEdit->setPlaceholderText("简单介绍一下自己（最多 200 字）");
    formLayout->addRow("简介:", m_bioEdit);

    m_charCountLabel = new QLabel("0 / 200");
    m_charCountLabel->setStyleSheet("color: #888; font-size: 11px;");
    formLayout->addRow("", m_charCountLabel);

    mainLayout->addLayout(formLayout);

    // ---- 标准按钮盒 ----
    // QDialogButtonBox 会根据当前平台自动排列按钮顺序
    m_buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel
    );
    mainLayout->addWidget(m_buttonBox);

    // 连接按钮盒的信号到对话框的 accept/reject
    connect(m_buttonBox, &QDialogButtonBox::accepted,
            this, &UserInfoDialog::validateAndAccept);
    connect(m_buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    // 简介字数实时计数
    connect(m_bioEdit, &QTextEdit::textChanged, this, [this]() {
        int count = m_bioEdit->toPlainText().length();
        m_charCountLabel->setText(
            QString::number(count) + " / 200");
        if (count > 200) {
            m_charCountLabel->setStyleSheet(
                "color: red; font-size: 11px; font-weight: bold;");
        } else {
            m_charCountLabel->setStyleSheet(
                "color: #888; font-size: 11px;");
        }
    });
}

void UserInfoDialog::setData(const UserInfo &info)
{
    m_nameEdit->setText(info.name);
    m_ageSpin->setValue(info.age);
    m_emailEdit->setText(info.email);
    m_bioEdit->setPlainText(info.bio);
}

UserInfo UserInfoDialog::getData() const
{
    return UserInfo{
        m_nameEdit->text().trimmed(),
        m_ageSpin->value(),
        m_emailEdit->text().trimmed(),
        m_bioEdit->toPlainText().trimmed()
    };
}

void UserInfoDialog::validateAndAccept()
{
    // 姓名不能为空
    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "验证失败", "姓名不能为空");
        m_nameEdit->setFocus();
        m_nameEdit->selectAll();
        return;
    }

    // 邮箱基本格式验证
    QString email = m_emailEdit->text().trimmed();
    if (email.isEmpty()) {
        QMessageBox::warning(this, "验证失败", "邮箱不能为空");
        m_emailEdit->setFocus();
        return;
    }
    int atIndex = email.indexOf('@');
    if (atIndex < 1 || email.indexOf('.', atIndex) < atIndex + 2) {
        QMessageBox::warning(this, "验证失败",
                             "邮箱格式不正确，请检查");
        m_emailEdit->setFocus();
        m_emailEdit->selectAll();
        return;
    }

    // 简介字数限制
    if (m_bioEdit->toPlainText().length() > 200) {
        QMessageBox::warning(this, "验证失败",
                             "简介不能超过 200 个字符");
        m_bioEdit->setFocus();
        return;
    }

    // 全部验证通过，关闭对话框并返回 Accepted
    accept();
}
