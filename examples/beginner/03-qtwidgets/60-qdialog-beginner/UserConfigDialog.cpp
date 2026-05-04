#include "UserConfigDialog.h"

// ============================================================================
// UserConfigDialog: 模态对话框 — 采集用户配置信息
// ============================================================================
UserConfigDialog::UserConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("新建用户");
    setMinimumWidth(320);

    auto *form = new QFormLayout;

    m_nameEdit = new QLineEdit;
    m_nameEdit->setPlaceholderText("请输入用户名");
    form->addRow("用户名:", m_nameEdit);

    m_ageSpin = new QSpinBox;
    m_ageSpin->setRange(0, 150);
    m_ageSpin->setValue(25);
    form->addRow("年龄:", m_ageSpin);

    m_roleCombo = new QComboBox;
    m_roleCombo->addItems({"管理员", "编辑者", "观察者"});
    form->addRow("角色:", m_roleCombo);

    // 标准按钮盒
    m_buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(m_buttonBox, &QDialogButtonBox::accepted,
            this, &UserConfigDialog::tryAccept);
    connect(m_buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(form);
    mainLayout->addWidget(m_buttonBox);
}

/// @brief 带校验的 accept — 校验失败则不关闭
void UserConfigDialog::tryAccept()
{
    if (m_nameEdit->text().isEmpty()) {
        m_nameEdit->setFocus();
        return;
    }
    if (m_ageSpin->value() < 18) {
        m_ageSpin->setFocus();
        return;
    }
    QDialog::accept();
}
