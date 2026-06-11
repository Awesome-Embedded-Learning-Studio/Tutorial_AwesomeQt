/// @file    input_dialog_demo.cpp
/// @brief   InputDialogDemo 类的实现，四种 QInputDialog 模式与自定义验证器。
///
/// 对应教程：进阶层 03-QtWidgets/63-QInputDialog 自定义验证器与输入范围。
/// 重点演示 setIntRange/setIntStep、setDecimals、QRegularExpressionValidator。

#include "input_dialog_demo.h"

#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QStringList>
#include <QVBoxLayout>

InputDialogDemo::InputDialogDemo(QWidget* parent)
    : QWidget(parent)
    , m_resultLabel(nullptr)
    , m_textBtn(nullptr)
    , m_intBtn(nullptr)
    , m_doubleBtn(nullptr)
    , m_itemBtn(nullptr)
    , m_emailValidator(nullptr)
{
    // 邮箱正则：至少一个字符@至少一个字符.至少两个字母
    const QRegularExpression emailRegex(
        QStringLiteral("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"));
    m_emailValidator = new QRegularExpressionValidator(emailRegex, this);
    // @note 验证器设置父对象为 this，随窗口销毁自动释放

    setupUI();
}

void InputDialogDemo::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);

    // 按钮区域
    m_textBtn = new QPushButton(tr("Text Input (Email Validator)"), this);
    m_intBtn = new QPushButton(tr("Int Input (Range & Step)"), this);
    m_doubleBtn = new QPushButton(tr("Double Input (Decimals & Range)"), this);
    m_itemBtn = new QPushButton(tr("Item Selection"), this);

    // @note 使用垂直布局保持按钮顺序清晰，便于演示对比
    mainLayout->addWidget(m_textBtn);
    mainLayout->addWidget(m_intBtn);
    mainLayout->addWidget(m_doubleBtn);
    mainLayout->addWidget(m_itemBtn);

    // 结果显示区域
    m_resultLabel = new QLabel(tr("Result: (none)"), this);
    m_resultLabel->setWordWrap(true);
    // 最小宽度确保长文本有足够展示空间
    m_resultLabel->setMinimumWidth(300);
    mainLayout->addWidget(m_resultLabel);

    // 信号槽连接
    connect(m_textBtn, &QPushButton::clicked,
            this, &InputDialogDemo::openTextDialog);
    connect(m_intBtn, &QPushButton::clicked,
            this, &InputDialogDemo::openIntDialog);
    connect(m_doubleBtn, &QPushButton::clicked,
            this, &InputDialogDemo::openDoubleDialog);
    connect(m_itemBtn, &QPushButton::clicked,
            this, &InputDialogDemo::openItemDialog);

    setWindowTitle(tr("QInputDialog - Advanced"));
    resize(400, 250);
}

void InputDialogDemo::openTextDialog()
{
    // @note 使用栈上 QInputDialog 而非静态函数，以便设置自定义验证器
    QInputDialog dialog(this);
    dialog.setWindowTitle(tr("Email Input"));
    dialog.setLabelText(tr("Enter your email address:"));
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setTextValue(QStringLiteral("user@example.com"));

    // 设置自定义正则验证器，仅接受邮箱格式
    dialog.setTextEchoMode(QLineEdit::Normal);
    // @note 只能通过访问内部 QLineEdit 来设置验证器
    auto* lineEdit = dialog.findChild<QLineEdit*>();
    if (lineEdit != nullptr) {
        lineEdit->setValidator(m_emailValidator);
    }

    if (dialog.exec() == QDialog::Accepted) {
        updateResult(tr("Text: %1").arg(dialog.textValue()));
    } else {
        updateResult(tr("Text: (cancelled)"));
    }
}

void InputDialogDemo::openIntDialog()
{
    // @note 静态 getInt 支持直接设置范围和步长，适合简单场景
    bool ok = false;
    const int value = QInputDialog::getInt(
        this,                                // parent
        tr("Integer Input"),                 // title
        tr("Select a port number (1024-65535):"),  // label
        8080,                                // default value
        1024,                                // minimum
        65535,                               // maximum
        1,                                   // step
        &ok);

    if (ok) {
        updateResult(tr("Int: %1").arg(value));
    } else {
        updateResult(tr("Int: (cancelled)"));
    }
}

void InputDialogDemo::openDoubleDialog()
{
    // 演示 QInputDialog 对象方式设置精度和范围
    QInputDialog dialog(this);
    dialog.setWindowTitle(tr("Double Input"));
    dialog.setLabelText(tr("Enter temperature (°C):"));
    dialog.setInputMode(QInputDialog::DoubleInput);
    dialog.setDoubleValue(36.5);
    dialog.setDoubleMinimum(-273.15);        // 绝对零度下限
    dialog.setDoubleMaximum(1000.0);
    dialog.setDoubleDecimals(2);             // 保留两位小数
    // @note 静态 getDouble 也支持这些参数，此处展示对象式用法便于对比

    if (dialog.exec() == QDialog::Accepted) {
        updateResult(tr("Double: %1 °C").arg(dialog.doubleValue(), 0, 'f', 2));
    } else {
        updateResult(tr("Double: (cancelled)"));
    }
}

void InputDialogDemo::openItemDialog()
{
    const QStringList items = {
        QStringLiteral("Qt Widgets"),
        QStringLiteral("Qt Quick / QML"),
        QStringLiteral("Qt Network"),
        QStringLiteral("Qt SQL"),
        QStringLiteral("Qt Test")
    };

    bool ok = false;
    // @note getItem 的 editable=false 禁止用户输入不在列表中的值
    const QString item = QInputDialog::getItem(
        this,
        tr("Module Selection"),
        tr("Select a Qt module:"),
        items,
        0,                                    // current index
        false,                                // not editable
        &ok);

    if (ok && !item.isEmpty()) {
        updateResult(tr("Item: %1").arg(item));
    } else {
        updateResult(tr("Item: (cancelled)"));
    }
}

void InputDialogDemo::updateResult(const QString& text)
{
    m_resultLabel->setText(tr("Result: %1").arg(text));
}
