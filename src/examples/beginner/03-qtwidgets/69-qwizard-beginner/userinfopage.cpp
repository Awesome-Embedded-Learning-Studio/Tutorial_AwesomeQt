#include "userinfopage.h"

#include <QGridLayout>
#include <QLabel>

// ============================================================================
// 第 2 页: 用户信息
// ============================================================================
UserInfoPage::UserInfoPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("用户信息");
    setSubTitle("请填写你的基本信息");

    m_nameEdit = new QLineEdit;
    m_nameEdit->setPlaceholderText("请输入姓名");
    m_emailEdit = new QLineEdit;
    m_emailEdit->setPlaceholderText(
        "请输入邮箱地址");

    auto *layout = new QGridLayout;
    layout->addWidget(new QLabel("姓名:"), 0, 0);
    layout->addWidget(m_nameEdit, 0, 1);
    layout->addWidget(new QLabel("邮箱:"), 1, 0);
    layout->addWidget(m_emailEdit, 1, 1);
    setLayout(layout);

    // 注册 mandatory 字段: 姓名和邮箱为必填
    registerField("name*", m_nameEdit);
    registerField("email*", m_emailEdit);
}

/// @brief 离开页面时的校验: 邮箱必须包含 @
bool UserInfoPage::validatePage()
{
    const QString email = m_emailEdit->text().trimmed();
    if (!email.contains('@') || email.endsWith('@')) {
        m_emailEdit->setFocus();
        return false;
    }
    return true;
}
