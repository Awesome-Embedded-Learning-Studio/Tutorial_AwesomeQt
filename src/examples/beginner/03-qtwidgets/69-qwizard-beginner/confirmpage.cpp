#include "confirmpage.h"

#include <QDebug>

// ============================================================================
// 第 4 页: 确认页
// ============================================================================
ConfirmPage::ConfirmPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("确认注册信息");
    setSubTitle("请确认以下信息无误后完成注册");

    m_summaryLabel = new QLabel;
    m_summaryLabel->setWordWrap(true);
    m_summaryLabel->setStyleSheet(
        "padding: 12px; background: #F5F5F5; "
        "border: 1px solid #DDD; "
        "border-radius: 4px;");

    auto *layout = new QVBoxLayout;
    layout->addWidget(m_summaryLabel);
    layout->addStretch();
    setLayout(layout);
}

/// @brief 进入确认页时汇总所有字段
void ConfirmPage::initializePage()
{
    const QString name = field("name").toString();
    const QString email =
        field("email").toString();
    const QString theme =
        field("theme").toString();
    const bool agreed =
        field("agree").toBool();

    m_summaryLabel->setText(
        QString("姓名: %1\n"
                "邮箱: %2\n"
                "主题偏好: %3\n"
                "已同意协议: %4")
            .arg(name)
            .arg(email)
            .arg(theme)
            .arg(agreed ? "是" : "否"));
}

/// @brief 完成时打印到调试输出
bool ConfirmPage::validatePage()
{
    qDebug() << "=== 注册完成 ===";
    qDebug() << "姓名:" << field("name").toString();
    qDebug() << "邮箱:" << field("email").toString();
    qDebug() << "主题:" << field("theme").toString();
    qDebug() << "同意协议:"
             << field("agree").toBool();
    return true;
}
