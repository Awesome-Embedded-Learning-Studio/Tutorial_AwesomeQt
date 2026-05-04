// QtWidgets 入门示例 63: QInputDialog 输入对话框
// 演示：getText / getInt / getDouble / getItem 四种静态方法
//       自定义 QDialog 多字段输入
//       重写 accept() 做校验，阻止无效提交

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QTextEdit>

// ============================================================================
// UserLoginDialog: 自定义多字段输入对话框 + accept() 校验
// ============================================================================
class UserLoginDialog : public QDialog
{
public:
    explicit UserLoginDialog(QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("用户登录");
        setMinimumWidth(320);

        auto *formLayout = new QFormLayout(this);

        m_usernameEdit = new QLineEdit;
        m_usernameEdit->setPlaceholderText("至少 4 个字符");
        formLayout->addRow("用户名:", m_usernameEdit);

        m_passwordEdit = new QLineEdit;
        m_passwordEdit->setEchoMode(QLineEdit::Password);
        m_passwordEdit->setPlaceholderText("至少 6 个字符");
        formLayout->addRow("密码:", m_passwordEdit);

        m_rememberCheck = new QCheckBox("记住密码");
        formLayout->addRow(m_rememberCheck);

        auto *buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel);
        formLayout->addRow(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted,
                this, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected,
                this, &QDialog::reject);

        // 输入时清除红色边框提示
        connect(m_usernameEdit, &QLineEdit::textChanged,
                this, [this]() {
                    m_usernameEdit->setStyleSheet("");
                });
        connect(m_passwordEdit, &QLineEdit::textChanged,
                this, [this]() {
                    m_passwordEdit->setStyleSheet("");
                });
    }

    QString username() const { return m_usernameEdit->text(); }
    QString password() const { return m_passwordEdit->text(); }
    bool rememberPassword() const
    {
        return m_rememberCheck->isChecked();
    }

protected:
    /// @brief 校验通过才允许关闭对话框
    void accept() override
    {
        bool valid = true;

        if (m_usernameEdit->text().length() < 4) {
            m_usernameEdit->setStyleSheet(
                "border: 2px solid red;");
            valid = false;
        }

        if (m_passwordEdit->text().length() < 6) {
            m_passwordEdit->setStyleSheet(
                "border: 2px solid red;");
            valid = false;
        }

        if (!valid) {
            return;  // 阻止关闭
        }

        QDialog::accept();
    }

private:
    QLineEdit *m_usernameEdit = nullptr;
    QLineEdit *m_passwordEdit = nullptr;
    QCheckBox *m_rememberCheck = nullptr;
};

// ============================================================================
// MainWindow: 演示四种 QInputDialog + 自定义登录对话框
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    // ====================================================================
    // getText: 文本输入
    // ====================================================================
    void onGetText();

    // ====================================================================
    // getInt: 整数输入 (1 ~ 72)
    // ====================================================================
    void onGetInt();

    // ====================================================================
    // getDouble: 浮点数输入 (0.1 ~ 5.0)
    // ====================================================================
    void onGetDouble();

    // ====================================================================
    // getItem: 列表选择
    // ====================================================================
    void onGetItem();

    // ====================================================================
    // 自定义登录对话框: 多字段输入 + accept() 校验
    // ====================================================================
    void onLogin();

    QTextEdit *m_resultEdit = nullptr;
};
