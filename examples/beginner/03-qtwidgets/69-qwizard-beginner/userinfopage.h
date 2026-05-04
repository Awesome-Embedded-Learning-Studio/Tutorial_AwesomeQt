#ifndef USERINFOPAGE_H
#define USERINFOPAGE_H

#include <QLineEdit>
#include <QWizardPage>

// ============================================================================
// 第 2 页: 用户信息
// ============================================================================
class UserInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit UserInfoPage(QWidget *parent = nullptr);

    /// @brief 离开页面时的校验: 邮箱必须包含 @
    bool validatePage() override;

private:
    QLineEdit *m_nameEdit;
    QLineEdit *m_emailEdit;
};

#endif // USERINFOPAGE_H
