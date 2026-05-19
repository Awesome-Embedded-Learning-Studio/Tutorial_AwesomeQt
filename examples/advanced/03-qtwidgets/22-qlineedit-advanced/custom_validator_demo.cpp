/// @file    custom_validator_demo.cpp
/// @brief   CustomValidatorDemo 类实现——QLineEdit 验证器、掩码、补全演示。
///
/// 对应教程：进阶层 03-QtWidgets/22-QLineEdit 进阶。

#include "custom_validator_demo.h"
#include "ip_address_validator.h"

#include <QCompleter>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

CustomValidatorDemo::CustomValidatorDemo(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);

    // 三个演示区域依次排列
    mainLayout->addWidget(createValidatorSection());
    mainLayout->addWidget(createInputMaskSection());
    mainLayout->addWidget(createCompleterSection());
    mainLayout->addStretch();

    setWindowTitle(QStringLiteral("QLineEdit Advanced Demo"));
    resize(600, 450);
}

// ─────────────────────────────────────────────────────────────────────────────
// 自定义 IP 地址验证器演示
// ─────────────────────────────────────────────────────────────────────────────

QWidget* CustomValidatorDemo::createValidatorSection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* sectionTitle = new QLabel(
        QStringLiteral("1. Custom IpAddressValidator"));
    sectionTitle->setStyleSheet(
        QStringLiteral("font-weight: bold; font-size: 14px;"));

    auto* inputRow = new QHBoxLayout;
    auto* inputLabel = new QLabel(QStringLiteral("IP Address:"));
    m_validatorEdit = new QLineEdit;
    m_validatorEdit->setPlaceholderText(
        QStringLiteral("Enter IPv4 address, e.g. 192.168.1.1"));

    // 安装自定义 IP 验证器
    m_validatorEdit->setValidator(new IpAddressValidator(m_validatorEdit));

    inputRow->addWidget(inputLabel);
    inputRow->addWidget(m_validatorEdit, 1);

    // 状态标签：实时显示验证器的判定结果
    m_validatorState = new QLabel(QStringLiteral("State: Empty (Intermediate)"));
    m_validatorState->setWordWrap(true);

    auto* hint = new QLabel(
        QStringLiteral("Each segment must be 0-255, no leading zeros. "
                       "Empty or partial input = Intermediate."));
    hint->setWordWrap(true);

    layout->addWidget(sectionTitle);
    layout->addLayout(inputRow);
    layout->addWidget(m_validatorState);
    layout->addWidget(hint);

    // 文本变化时更新验证状态
    connect(m_validatorEdit, &QLineEdit::textChanged,
            this, &CustomValidatorDemo::updateValidatorState);

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// 输入掩码演示
// ─────────────────────────────────────────────────────────────────────────────

QWidget* CustomValidatorDemo::createInputMaskSection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* sectionTitle = new QLabel(
        QStringLiteral("2. Input Mask (Phone Number)"));
    sectionTitle->setStyleSheet(
        QStringLiteral("font-weight: bold; font-size: 14px;"));

    auto* inputRow = new QHBoxLayout;
    auto* inputLabel = new QLabel(QStringLiteral("Phone:"));
    m_maskEdit = new QLineEdit;

    // 9 = 必须输入数字，不允许为空
    // 括号和空格作为固定分隔符自动显示
    m_maskEdit->setInputMask(QStringLiteral("(999) 999-9999"));
    m_maskEdit->setPlaceholderText(QStringLiteral("(XXX) XXX-XXXX"));

    inputRow->addWidget(inputLabel);
    inputRow->addWidget(m_maskEdit, 1);

    auto* hint = new QLabel(
        QStringLiteral("Mask: (999) 999-9999. '9' = required digit. "
                       "Parentheses and hyphen are fixed separators."));
    hint->setWordWrap(true);

    layout->addWidget(sectionTitle);
    layout->addLayout(inputRow);
    layout->addWidget(hint);

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// QCompleter 自动补全演示
// ─────────────────────────────────────────────────────────────────────────────

QWidget* CustomValidatorDemo::createCompleterSection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* sectionTitle = new QLabel(
        QStringLiteral("3. QCompleter (Custom Word List)"));
    sectionTitle->setStyleSheet(
        QStringLiteral("font-weight: bold; font-size: 14px;"));

    auto* inputRow = new QHBoxLayout;
    auto* inputLabel = new QLabel(QStringLiteral("Search:"));
    m_completerEdit = new QLineEdit;
    m_completerEdit->setPlaceholderText(
        QStringLiteral("Type to search (e.g. 'signal', 'thread')..."));

    // 创建 QCompleter 并设置自定义词表
    const QStringList words = {
        QStringLiteral("QAbstractScrollArea"),
        QStringLiteral("QApplication"),
        QStringLiteral("QByteArray"),
        QStringLiteral("QCheckBox"),
        QStringLiteral("QCompleter"),
        QStringLiteral("QDateTime"),
        QStringLiteral("QEvent"),
        QStringLiteral("QFontMetrics"),
        QStringLiteral("QHBoxLayout"),
        QStringLiteral("QIODevice"),
        QStringLiteral("QJsonDocument"),
        QStringLiteral("QKeyEvent"),
        QStringLiteral("QLineEdit"),
        QStringLiteral("QMainWindow"),
        QStringLiteral("QMetaObject"),
        QStringLiteral("QNetworkAccessManager"),
        QStringLiteral("QObject"),
        QStringLiteral("QPainter"),
        QStringLiteral("QRegularExpression"),
        QStringLiteral("QSignalMapper"),
        QStringLiteral("QString"),
        QStringLiteral("QThread"),
        QStringLiteral("QTimer"),
        QStringLiteral("QUndoStack"),
        QStringLiteral("QValidator"),
        QStringLiteral("QWidget"),
    };

    auto* completer = new QCompleter(words, this);
    // 中间匹配：关键词出现在任意位置即可匹配
    completer->setFilterMode(Qt::MatchContains);
    // 忽略大小写
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completerEdit->setCompleter(completer);

    inputRow->addWidget(inputLabel);
    inputRow->addWidget(m_completerEdit, 1);

    auto* hint = new QLabel(
        QStringLiteral("Completer: MatchContains + CaseInsensitive. "
                       "Try typing 'signal', 'paint', or 'thread'."));
    hint->setWordWrap(true);

    layout->addWidget(sectionTitle);
    layout->addLayout(inputRow);
    layout->addWidget(hint);

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// 槽函数
// ─────────────────────────────────────────────────────────────────────────────

void CustomValidatorDemo::updateValidatorState()
{
    const QString text = m_validatorEdit->text();
    int pos = 0;
    QString mutableText = text;

    // 手动调用验证器获取当前状态
    // validator() 返回 const 指针，validate 需要非 const 引用参数
    const auto* validator =
        qobject_cast<const IpAddressValidator*>(m_validatorEdit->validator());
    if (!validator) {
        return;
    }

    const QValidator::State state = const_cast<IpAddressValidator*>(validator)->validate(mutableText, pos);

    QString stateText;
    switch (state) {
    case QValidator::Acceptable:
        stateText = QStringLiteral("Acceptable (valid IPv4 address)");
        break;
    case QValidator::Intermediate:
        stateText = QStringLiteral("Intermediate (incomplete or partial)");
        break;
    case QValidator::Invalid:
        stateText = QStringLiteral("Invalid (illegal characters or out of range)");
        break;
    }

    m_validatorState->setText(QStringLiteral("State: %1 | Input: \"%2\"")
                                  .arg(stateText, text));
}
