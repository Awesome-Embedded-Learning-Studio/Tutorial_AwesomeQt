#include "register_form.h"

#include <QApplication>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QVBoxLayout>

// ============================================================================
// RegisterForm: 模拟用户注册表单
// ============================================================================

RegisterForm::RegisterForm(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QLineEdit 综合演示 — 用户注册表单");
    resize(480, 560);
    initUi();
}

/// @brief 初始化界面
void RegisterForm::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    // ---- 标题 ----
    auto *titleLabel = new QLabel("用户注册");
    titleLabel->setFont(QFont("Arial", 16, QFont::Bold));
    mainLayout->addWidget(titleLabel);

    // ================================================================
    // 表单区域: 五个 QLineEdit
    // ================================================================
    auto *formGroup = new QGroupBox("注册信息");
    auto *formLayout = new QFormLayout(formGroup);
    formLayout->setSpacing(14);
    formLayout->setLabelAlignment(Qt::AlignRight);

    // 1. 用户名 — 正则验证 + 最大长度 + 字符计数
    m_usernameEdit = new QLineEdit();
    m_usernameEdit->setPlaceholderText("字母、数字、下划线，最长 20 字符");
    m_usernameEdit->setMaxLength(20);
    m_usernameEdit->setClearButtonEnabled(true);
    // 只允许字母、数字、下划线
    m_usernameEdit->setValidator(new QRegularExpressionValidator(
        QRegularExpression(R"(^[a-zA-Z0-9_]+$)"), this));

    auto *usernameRow = new QHBoxLayout();
    usernameRow->addWidget(m_usernameEdit);
    m_charCountLabel = new QLabel("0 / 20");
    m_charCountLabel->setStyleSheet(
        "color: #888; font-size: 11px; min-width: 50px;");
    usernameRow->addWidget(m_charCountLabel);
    formLayout->addRow("用户名:", usernameRow);

    // textChanged: 实时字符计数（包括 setText 也会触发）
    connect(m_usernameEdit, &QLineEdit::textChanged, this,
            [this](const QString &text) {
        m_charCountLabel->setText(
            QString("%1 / 20").arg(text.length()));
    });

    // 2. 邮箱 — 正则验证基本格式
    m_emailEdit = new QLineEdit();
    m_emailEdit->setPlaceholderText("example@domain.com");
    m_emailEdit->setClearButtonEnabled(true);
    // 宽松的正则：只排除明显非法的字符，完整验证在提交时做
    m_emailEdit->setValidator(new QRegularExpressionValidator(
        QRegularExpression(R"(^[a-zA-Z0-9_.@+-]+$)"), this));

    formLayout->addRow("邮箱:", m_emailEdit);

    // 3. 年龄 — 整数验证，范围 1-150
    m_ageEdit = new QLineEdit();
    m_ageEdit->setPlaceholderText("1 - 150");
    m_ageEdit->setValidator(new QIntValidator(1, 150, this));
    m_ageEdit->setMaxLength(3);

    formLayout->addRow("年龄:", m_ageEdit);

    // 4. 密码 — Password 回显模式
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setPlaceholderText("至少 6 个字符");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setClearButtonEnabled(true);

    formLayout->addRow("密码:", m_passwordEdit);

    // 5. 确认密码 — Password 回显 + editingFinished 检查一致性
    m_confirmEdit = new QLineEdit();
    m_confirmEdit->setPlaceholderText("再次输入密码");
    m_confirmEdit->setEchoMode(QLineEdit::Password);

    formLayout->addRow("确认密码:", m_confirmEdit);

    // editingFinished: 失焦时检查密码一致性
    connect(m_confirmEdit, &QLineEdit::editingFinished, this,
            [this]() {
        if (m_confirmEdit->text().isEmpty()) return;

        if (m_confirmEdit->text() != m_passwordEdit->text()) {
            m_resultLabel->setText("两次密码不一致，请重新输入");
            m_resultLabel->setStyleSheet(
                "color: #D32F2F; padding: 8px; font-size: 12px;");
        } else {
            m_resultLabel->setText("密码一致");
            m_resultLabel->setStyleSheet(
                "color: #388E3C; padding: 8px; font-size: 12px;");
        }
    });

    mainLayout->addWidget(formGroup);

    // ---- 结果展示区 ----
    m_resultLabel = new QLabel("填写完成后点击「注册」");
    m_resultLabel->setAlignment(Qt::AlignCenter);
    m_resultLabel->setMinimumHeight(40);
    m_resultLabel->setStyleSheet(
        "background-color: #FAFAFA;"
        "border: 1px solid #E0E0E0;"
        "border-radius: 6px;"
        "padding: 10px;"
        "font-size: 13px;"
        "color: #333;");
    mainLayout->addWidget(m_resultLabel);

    // ================================================================
    // 信号演示区: textChanged vs textEdited vs editingFinished
    // ================================================================
    auto *signalGroup = new QGroupBox("信号日志（textChanged / textEdited / editingFinished）");
    auto *signalLayout = new QVBoxLayout(signalGroup);

    auto *demoEdit = new QLineEdit();
    demoEdit->setPlaceholderText("在这里输入，观察三个信号的触发时机");
    demoEdit->setClearButtonEnabled(true);
    signalLayout->addWidget(demoEdit);

    m_signalLog = new QLabel("等待输入...");
    m_signalLog->setMinimumHeight(60);
    m_signalLog->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_signalLog->setStyleSheet(
        "background-color: #1E1E1E; color: #D4D4D4;"
        "border-radius: 4px; padding: 8px;"
        "font-family: monospace; font-size: 11px;");
    signalLayout->addWidget(m_signalLog);

    // textChanged: 任何内容变化（包括 setText）都触发
    connect(demoEdit, &QLineEdit::textChanged, this,
            [this](const QString &text) {
        appendSignalLog(
            QString("textChanged: \"%1\"").arg(text));
    });

    // textEdited: 只有用户手动编辑才触发
    connect(demoEdit, &QLineEdit::textEdited, this,
            [this](const QString &text) {
        appendSignalLog(
            QString("textEdited:   \"%1\"").arg(text));
    });

    // editingFinished: 按回车或失焦时触发
    connect(demoEdit, &QLineEdit::editingFinished, this,
            [this, demoEdit]() {
        appendSignalLog(
            QString("editingFinished: \"%1\"").arg(demoEdit->text()));
    });

    // 演示按钮: 用 setText 修改内容
    auto *setTextBtn = new QPushButton("程序 setText(\"hello\")");
    setTextBtn->setAutoDefault(false);
    connect(setTextBtn, &QPushButton::clicked, this,
            [demoEdit]() {
        demoEdit->setText("hello");
    });

    signalLayout->addWidget(setTextBtn);
    mainLayout->addWidget(signalGroup);

    // ---- 底部按钮行 ----
    auto *bottomLayout = new QHBoxLayout();

    m_statusLabel = new QLabel("");
    m_statusLabel->setStyleSheet("color: #888; font-size: 11px;");

    auto *registerBtn = new QPushButton("注册");
    registerBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #1976D2;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 8px 28px;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #1565C0; }"
        "QPushButton:pressed { background-color: #0D47A1; }");
    connect(registerBtn, &QPushButton::clicked, this,
            &RegisterForm::onRegister);

    bottomLayout->addWidget(m_statusLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(registerBtn);

    mainLayout->addLayout(bottomLayout);
}

/// @brief 追加信号日志（最多保留 8 条）
void RegisterForm::appendSignalLog(const QString &entry)
{
    m_logLines.append(entry);
    if (m_logLines.size() > 8) {
        m_logLines.removeFirst();
    }
    m_signalLog->setText(m_logLines.join("\n"));
}

/// @brief 注册按钮点击：完整验证所有字段
void RegisterForm::onRegister()
{
    QStringList errors;

    const auto username = m_usernameEdit->text().trimmed();
    const auto email = m_emailEdit->text().trimmed();
    const auto ageText = m_ageEdit->text().trimmed();
    const auto password = m_passwordEdit->text();
    const auto confirm = m_confirmEdit->text();

    if (username.isEmpty()) {
        errors << "用户名不能为空";
    } else if (username.length() < 3) {
        errors << "用户名至少 3 个字符";
    }

    if (email.isEmpty()) {
        errors << "邮箱不能为空";
    } else {
        // 提交时的完整邮箱格式验证
        QRegularExpression emailRegex(
            R"(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$)");
        if (!emailRegex.match(email).hasMatch()) {
            errors << "邮箱格式不正确";
        }
    }

    if (ageText.isEmpty()) {
        errors << "年龄不能为空";
    } else {
        bool ok = false;
        int age = ageText.toInt(&ok);
        if (!ok || age < 1 || age > 150) {
            errors << "年龄须在 1-150 之间";
        }
    }

    if (password.length() < 6) {
        errors << "密码至少 6 个字符";
    }

    if (password != confirm) {
        errors << "两次密码不一致";
    }

    if (errors.isEmpty()) {
        m_resultLabel->setText(
            QString("注册成功！用户: %1").arg(username));
        m_resultLabel->setStyleSheet(
            "color: #388E3C; padding: 10px; font-size: 13px;"
            "background-color: #E8F5E9;"
            "border: 1px solid #A5D6A7; border-radius: 6px;");
        m_statusLabel->setText(
            QString("注册完成 — %1").arg(username));
    } else {
        m_resultLabel->setText(errors.join("\n"));
        m_resultLabel->setStyleSheet(
            "color: #D32F2F; padding: 10px; font-size: 12px;"
            "background-color: #FFEBEE;"
            "border: 1px solid #EF9A9A; border-radius: 6px;");
    }
}
