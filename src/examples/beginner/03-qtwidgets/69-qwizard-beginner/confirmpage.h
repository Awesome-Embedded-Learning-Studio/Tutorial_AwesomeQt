#ifndef CONFIRMPAGE_H
#define CONFIRMPAGE_H

#include <QLabel>
#include <QVBoxLayout>
#include <QWizardPage>

// ============================================================================
// 第 4 页: 确认页
// ============================================================================
class ConfirmPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ConfirmPage(QWidget *parent = nullptr);

    /// @brief 进入确认页时汇总所有字段
    void initializePage() override;

    /// @brief 完成时打印到调试输出
    bool validatePage() override;

private:
    QLabel *m_summaryLabel;
};

#endif // CONFIRMPAGE_H
