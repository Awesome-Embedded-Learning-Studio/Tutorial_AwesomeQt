/// @file    validated_dialog.cpp
/// @brief   ValidatedDialog 类实现——带输入验证的对话框。
///
/// 对应教程：进阶层 03-QtWidgets/06-对话框进阶。

#include "validated_dialog.h"

#include <QDialogButtonBox>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

ValidatedDialog::ValidatedDialog(const QString& title, QWidget* parent)
    : QDialog(parent)
    , m_portEdit(nullptr)
    , m_validationHint(nullptr)
    , m_okButton(nullptr)
    , m_portValue(-1)
{
    setWindowTitle(title);

    auto* layout = new QVBoxLayout(this);

    // 标题说明
    auto* headerLabel = new QLabel(
        QStringLiteral("请输入端口号（1-65535）：\n"
                       "验证逻辑在 accept() 中执行，非法输入不会关闭对话框。"));
    headerLabel->setWordWrap(true);
    layout->addWidget(headerLabel);

    // 输入行
    auto* inputRow = new QHBoxLayout;
    auto* portLabel = new QLabel(QStringLiteral("端口:"));
    m_portEdit = new QLineEdit;

    // QIntValidator 限制只能输入 1-65535 范围的整数
    m_portEdit->setValidator(new QIntValidator(1, 65535, this));
    m_portEdit->setPlaceholderText(QStringLiteral("例如: 8080"));

    inputRow->addWidget(portLabel);
    inputRow->addWidget(m_portEdit, 1);
    layout->addLayout(inputRow);

    // 验证提示标签
    m_validationHint = new QLabel;
    m_validationHint->setStyleSheet(QStringLiteral("color: red;"));
    layout->addWidget(m_validationHint);

    // 标准按钮盒
    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_okButton = buttonBox->button(QDialogButtonBox::Ok);
    layout->addWidget(buttonBox);

    // 连接信号
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ValidatedDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ValidatedDialog::reject);

    // 输入变化时实时验证
    connect(m_portEdit, &QLineEdit::textChanged, this, &ValidatedDialog::validateInput);

    // 初始状态：输入为空，OK 按钮禁用
    validateInput();

    resize(350, 180);
}

// ─────────────────────────────────────────────────────────────────────────────
// 公有方法
// ─────────────────────────────────────────────────────────────────────────────

int ValidatedDialog::portValue() const
{
    return m_portValue;
}

// ─────────────────────────────────────────────────────────────────────────────
// 重写 accept——验证通过才真正关闭
// ─────────────────────────────────────────────────────────────────────────────

void ValidatedDialog::accept()
{
    bool ok = false;
    const int port = m_portEdit->text().toInt(&ok);

    if (!ok || port < 1 || port > 65535) {
        // 验证失败：不调 reject()，不调 QDialog::accept()，对话框保持打开
        // 这样调用方的 exec() 返回值不会被污染
        m_validationHint->setText(QStringLiteral("端口号必须在 1-65535 之间！"));
        return;
    }

    m_portValue = port;
    QDialog::accept();  // 验证通过，真正关闭
}

// ─────────────────────────────────────────────────────────────────────────────
// 实时验证
// ─────────────────────────────────────────────────────────────────────────────

void ValidatedDialog::validateInput()
{
    const QString text = m_portEdit->text().trimmed();

    if (text.isEmpty()) {
        m_okButton->setEnabled(false);
        m_validationHint->setText(QStringLiteral("请输入端口号。"));
        return;
    }

    bool ok = false;
    const int port = text.toInt(&ok);

    if (!ok || port < 1 || port > 65535) {
        m_okButton->setEnabled(false);
        m_validationHint->setText(QStringLiteral("端口号必须在 1-65535 之间。"));
    } else {
        m_okButton->setEnabled(true);
        m_validationHint->setText(QStringLiteral("输入有效。"));
    }
}
