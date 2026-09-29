#ifndef WELCOMEPAGE_H
#define WELCOMEPAGE_H

#include <QCheckBox>
#include <QVBoxLayout>
#include <QWizardPage>

// ============================================================================
// 第 1 页: 欢迎 + 用户协议
// ============================================================================
class WelcomePage : public QWizardPage
{
    Q_OBJECT

public:
    explicit WelcomePage(QWidget *parent = nullptr);

private:
    QCheckBox *m_agreeCheck;
};

#endif // WELCOMEPAGE_H
