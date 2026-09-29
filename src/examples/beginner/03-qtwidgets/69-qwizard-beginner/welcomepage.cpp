#include "welcomepage.h"

#include <QLabel>

// ============================================================================
// 第 1 页: 欢迎 + 用户协议
// ============================================================================
WelcomePage::WelcomePage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("欢迎使用注册向导");
    setSubTitle("本向导将引导你完成账号注册");

    auto *intro = new QLabel(
        "欢迎使用我们的服务！\n\n"
        "本向导将分步引导你完成以下操作:\n"
        "  1. 阅读并同意用户协议\n"
        "  2. 填写基本的用户信息\n"
        "  3. 设置个人偏好\n"
        "  4. 确认并完成注册\n\n"
        "请勾选下方复选框以继续。");
    intro->setWordWrap(true);

    m_agreeCheck = new QCheckBox(
        "我已阅读并同意《用户服务协议》");

    auto *layout = new QVBoxLayout;
    layout->addWidget(intro);
    layout->addSpacing(20);
    layout->addWidget(m_agreeCheck);
    layout->addStretch();
    setLayout(layout);

    // 注册 agree 字段，绑定 checked 属性
    // 星号表示 mandatory: 勾选后"下一步"才可用
    registerField("agree*", m_agreeCheck);
}
