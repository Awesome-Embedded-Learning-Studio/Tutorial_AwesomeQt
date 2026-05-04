#include "SettingsDialog.h"

// ============================================================================
// SettingsDialog: 标准按钮盒 + QDialog 的完整模板
// ============================================================================
SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("应用设置");
    setMinimumWidth(380);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 8);

    // ---- 内容区域：表单布局 ----
    auto *form = new QFormLayout;

    m_nameEdit = new QLineEdit;
    m_nameEdit->setPlaceholderText("请输入应用名称");
    form->addRow("应用名称:", m_nameEdit);

    m_maxConnSpin = new QSpinBox;
    m_maxConnSpin->setRange(1, 100);
    m_maxConnSpin->setValue(kDefaultMaxConn);
    form->addRow("最大连接数:", m_maxConnSpin);

    m_logLevelCombo = new QComboBox;
    m_logLevelCombo->addItems({"调试", "信息", "警告", "错误"});
    m_logLevelCombo->setCurrentIndex(kDefaultLogLevel);
    form->addRow("日志级别:", m_logLevelCombo);

    mainLayout->addLayout(form, 1);

    // ---- 状态提示 ----
    m_statusLabel = new QLabel("修改设置后点击\"应用\"预览，"
                               "或点击\"确定\"保存并关闭");
    m_statusLabel->setWordWrap(true);
    mainLayout->addWidget(m_statusLabel);

    // ---- 分隔线 ----
    auto *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(separator);

    // ---- 标准按钮盒 ----
    m_buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok |
        QDialogButtonBox::Cancel |
        QDialogButtonBox::Apply |
        QDialogButtonBox::Reset);

    // Ok 按钮：初始禁用，名称不为空时启用
    m_buttonBox->button(QDialogButtonBox::Ok)
        ->setEnabled(false);
    m_buttonBox->button(QDialogButtonBox::Ok)
        ->setToolTip("保存设置并关闭");
    m_buttonBox->button(QDialogButtonBox::Apply)
        ->setToolTip("应用当前设置但不关闭");
    m_buttonBox->button(QDialogButtonBox::Reset)
        ->setToolTip("恢复为默认值");

    // 信号连接
    connect(m_buttonBox, &QDialogButtonBox::accepted,
            this, &SettingsDialog::tryAccept);
    connect(m_buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);
    connect(m_buttonBox, &QDialogButtonBox::clicked,
            this, &SettingsDialog::onButtonClicked);

    // 输入变化时重新校验
    connect(m_nameEdit, &QLineEdit::textChanged,
            this, &SettingsDialog::validate);
    connect(m_maxConnSpin, &QSpinBox::valueChanged,
            this, &SettingsDialog::validate);

    mainLayout->addWidget(m_buttonBox);
}

/// @brief 带校验的 accept
void SettingsDialog::tryAccept()
{
    if (!validate()) return;
    applySettings();
    QDialog::accept();
}

/// @brief 处理非关闭按钮的点击
void SettingsDialog::onButtonClicked(QAbstractButton *btn)
{
    auto standard = m_buttonBox->standardButton(btn);
    switch (standard) {
        case QDialogButtonBox::Apply:
            if (validate()) {
                applySettings();
            }
            break;
        case QDialogButtonBox::Reset:
            resetToDefaults();
            break;
        default:
            break;
    }
}

/// @brief 校验输入并控制 OK 按钮状态
bool SettingsDialog::validate()
{
    bool valid = !m_nameEdit->text().trimmed().isEmpty();
    m_buttonBox->button(QDialogButtonBox::Ok)
        ->setEnabled(valid);
    return valid;
}

/// @brief 应用当前设置（不关闭对话框）
void SettingsDialog::applySettings()
{
    m_statusLabel->setText(
        QString("设置已应用: 名称=%1, 连接数=%2, "
                "日志级别=%3")
            .arg(m_nameEdit->text())
            .arg(m_maxConnSpin->value())
            .arg(m_logLevelCombo->currentText()));
}

/// @brief 恢复为默认值
void SettingsDialog::resetToDefaults()
{
    // 阻止信号以避免重复校验
    m_nameEdit->blockSignals(true);
    m_maxConnSpin->blockSignals(true);
    m_logLevelCombo->blockSignals(true);

    m_nameEdit->clear();
    m_maxConnSpin->setValue(kDefaultMaxConn);
    m_logLevelCombo->setCurrentIndex(kDefaultLogLevel);

    m_nameEdit->blockSignals(false);
    m_maxConnSpin->blockSignals(false);
    m_logLevelCombo->blockSignals(false);

    // 手动触发一次校验
    validate();
    m_statusLabel->setText("已恢复为默认值");
}
