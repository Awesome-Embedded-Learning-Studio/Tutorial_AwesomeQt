#pragma once

#include <QStringList>
#include <QWidget>

class QLineEdit;
class QLabel;

// ============================================================================
// RegisterForm: 模拟用户注册表单
// ============================================================================
class RegisterForm : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterForm(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 追加信号日志（最多保留 8 条）
    void appendSignalLog(const QString &entry);

    /// @brief 注册按钮点击：完整验证所有字段
    void onRegister();

private:
    QLineEdit *m_usernameEdit = nullptr;
    QLineEdit *m_emailEdit = nullptr;
    QLineEdit *m_ageEdit = nullptr;
    QLineEdit *m_passwordEdit = nullptr;
    QLineEdit *m_confirmEdit = nullptr;
    QLabel *m_charCountLabel = nullptr;
    QLabel *m_resultLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QLabel *m_signalLog = nullptr;
    QStringList m_logLines;
};
