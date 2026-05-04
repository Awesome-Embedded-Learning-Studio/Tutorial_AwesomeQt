#include "preferencepage.h"

#include <QGridLayout>
#include <QLabel>

// ============================================================================
// 第 3 页: 偏好设置
// ============================================================================
PreferencePage::PreferencePage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("偏好设置");
    setSubTitle("根据你的信息提供推荐选项");

    m_themeCombo = new QComboBox;
    m_themeCombo->addItems(
        {"系统默认", "浅色主题", "深色主题"});
    m_recommendLabel = new QLabel;
    m_recommendLabel->setWordWrap(true);
    m_recommendLabel->setStyleSheet(
        "color: #2196F3; padding: 8px;"
        "background: #E3F2FD; "
        "border-radius: 4px;");

    auto *layout = new QGridLayout;
    layout->addWidget(
        new QLabel("界面主题:"), 0, 0);
    layout->addWidget(m_themeCombo, 0, 1);
    layout->addWidget(m_recommendLabel, 1, 0, 1, 2);
    layout->setRowStretch(2, 1);
    setLayout(layout);

    registerField("theme", m_themeCombo);
}

/// @brief 每次进入该页时，根据邮箱域名给出推荐
void PreferencePage::initializePage()
{
    const QString email =
        field("email").toString();

    // 提取邮箱域名
    const QStringList parts =
        email.split('@');
    QString domain;
    if (parts.size() >= 2) {
        domain = parts.last().toLower();
    }

    QString recommendation;
    if (domain == "gmail.com") {
        recommendation =
            "检测到你使用 Gmail 邮箱，"
            "推荐开启 Google 服务集成。";
    } else if (domain == "outlook.com" ||
               domain == "hotmail.com") {
        recommendation =
            "检测到你使用 Microsoft 邮箱，"
            "推荐开启 Microsoft 365 集成。";
    } else if (domain == "qq.com" ||
               domain == "163.com") {
        recommendation =
            "检测到你使用国内邮箱，"
            "推荐开启本地化服务支持。";
    } else {
        recommendation =
            QString("你的邮箱域名为 %1，"
                    "暂无特殊推荐。")
                .arg(domain);
    }
    m_recommendLabel->setText(recommendation);
}
