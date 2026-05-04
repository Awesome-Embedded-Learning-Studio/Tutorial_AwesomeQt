#ifndef PREFERENCEPAGE_H
#define PREFERENCEPAGE_H

#include <QComboBox>
#include <QLabel>
#include <QWizardPage>

// ============================================================================
// 第 3 页: 偏好设置
// ============================================================================
class PreferencePage : public QWizardPage
{
    Q_OBJECT

public:
    explicit PreferencePage(QWidget *parent = nullptr);

    /// @brief 每次进入该页时，根据邮箱域名给出推荐
    void initializePage() override;

private:
    QComboBox *m_themeCombo;
    QLabel *m_recommendLabel;
};

#endif // PREFERENCEPAGE_H
