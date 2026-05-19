/// @file    label_mode_demo.cpp
/// @brief   LabelModeDemo 类实现——QLabel setTextFormat、setBuddy、elidedText 演示。
///
/// 对应教程：进阶层 03-QtWidgets/34-QLabel 进阶。

#include "label_mode_demo.h"

#include <QComboBox>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

LabelModeDemo::LabelModeDemo(QWidget* parent)
    : QWidget(parent)
    , m_buddyHasMarker(true)
{
    auto* mainLayout = new QVBoxLayout(this);

    // 三个演示区域依次排列
    mainLayout->addWidget(createTextFormatSection());
    mainLayout->addWidget(createBuddySection());
    mainLayout->addWidget(createElideSection());
    mainLayout->addStretch();

    setWindowTitle(QStringLiteral("QLabel Advanced Demo"));
    resize(600, 500);
}

// ─────────────────────────────────────────────────────────────────────────────
// setTextFormat 三种模式对比
// ─────────────────────────────────────────────────────────────────────────────

QWidget* LabelModeDemo::createTextFormatSection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* sectionTitle = new QLabel(QStringLiteral("1. setTextFormat 三种模式对比"));
    sectionTitle->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14px;"));

    // 用户输入区
    auto* inputRow = new QHBoxLayout;
    auto* inputLabel = new QLabel(QStringLiteral("输入测试文本:"));
    m_formatInput = new QLineEdit(QStringLiteral("<b>粗体</b> 和 <i>斜体</i> 以及 <script>alert('xss')</script>"));
    m_formatCombo = new QComboBox;
    m_formatCombo->addItem(QStringLiteral("PlainText"), static_cast<int>(Qt::PlainText));
    m_formatCombo->addItem(QStringLiteral("RichText"), static_cast<int>(Qt::RichText));
    m_formatCombo->addItem(QStringLiteral("AutoText"), static_cast<int>(Qt::AutoText));

    inputRow->addWidget(inputLabel);
    inputRow->addWidget(m_formatInput, 1);
    inputRow->addWidget(m_formatCombo);

    // 三种模式并排展示，方便直观对比
    auto* labelsRow = new QHBoxLayout;

    // PlainText：永远不会解析 HTML，最安全
    auto* plainTitle = new QLabel(QStringLiteral("PlainText"));
    plainTitle->setAlignment(Qt::AlignCenter);
    plainTitle->setStyleSheet(QStringLiteral("font-weight: bold;"));
    m_plainLabel = new QLabel;
    m_plainLabel->setFrameShape(QFrame::Box);
    m_plainLabel->setWordWrap(true);
    m_plainLabel->setTextFormat(Qt::PlainText);

    auto* plainCol = new QVBoxLayout;
    plainCol->addWidget(plainTitle);
    plainCol->addWidget(m_plainLabel);

    // RichText：强制走 HTML 解析管线
    auto* richTitle = new QLabel(QStringLiteral("RichText"));
    richTitle->setAlignment(Qt::AlignCenter);
    richTitle->setStyleSheet(QStringLiteral("font-weight: bold;"));
    m_richLabel = new QLabel;
    m_richLabel->setFrameShape(QFrame::Box);
    m_richLabel->setWordWrap(true);
    m_richLabel->setTextFormat(Qt::RichText);

    auto* richCol = new QVBoxLayout;
    richCol->addWidget(richTitle);
    richCol->addWidget(m_richLabel);

    // AutoText：启发式检测，可能误判用户输入为 HTML
    auto* autoTitle = new QLabel(QStringLiteral("AutoText"));
    autoTitle->setAlignment(Qt::AlignCenter);
    autoTitle->setStyleSheet(QStringLiteral("font-weight: bold;"));
    m_autoLabel = new QLabel;
    m_autoLabel->setFrameShape(QFrame::Box);
    m_autoLabel->setWordWrap(true);
    m_autoLabel->setTextFormat(Qt::AutoText);

    auto* autoCol = new QVBoxLayout;
    autoCol->addWidget(autoTitle);
    autoCol->addWidget(m_autoLabel);

    labelsRow->addLayout(plainCol);
    labelsRow->addLayout(richCol);
    labelsRow->addLayout(autoCol);

    layout->addWidget(sectionTitle);
    layout->addLayout(inputRow);
    layout->addLayout(labelsRow);

    // 文本变化或格式切换时刷新所有标签
    connect(m_formatInput, &QLineEdit::textChanged, this, &LabelModeDemo::updateFormatDisplay);
    connect(m_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &LabelModeDemo::updateFormatDisplay);

    // 初始填充
    updateFormatDisplay();

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// setBuddy 快捷键演示
// ─────────────────────────────────────────────────────────────────────────────

QWidget* LabelModeDemo::createBuddySection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* sectionTitle = new QLabel(QStringLiteral("2. setBuddy 快捷键与 & 标记"));
    sectionTitle->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14px;"));

    // 带快捷键标记的标签 + 目标输入框
    m_buddyLabel = new QLabel(QStringLiteral("用户名(&U):"));
    m_buddyEdit = new QLineEdit;
    m_buddyLabel->setBuddy(m_buddyEdit);

    auto* buddyRow = new QHBoxLayout;
    buddyRow->addWidget(m_buddyLabel);
    buddyRow->addWidget(m_buddyEdit, 1);

    // 切换按钮：演示文本中有 & 标记和无标记两种状态
    m_toggleBuddyBtn =
        new QPushButton(QStringLiteral("切换标签文本（当前：含 &U 标记，Alt+U 有效）"));

    // 提示信息
    auto* hint = new QLabel(
        QStringLiteral("按 Alt+U 测试焦点跳转。点击按钮移除 & 标记后快捷键将失效。"));
    hint->setWordWrap(true);

    layout->addWidget(sectionTitle);
    layout->addLayout(buddyRow);
    layout->addWidget(m_toggleBuddyBtn);
    layout->addWidget(hint);

    connect(m_toggleBuddyBtn, &QPushButton::clicked, this, &LabelModeDemo::toggleBuddyText);

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// QFontMetrics::elidedText 文本截断
// ─────────────────────────────────────────────────────────────────────────────

QWidget* LabelModeDemo::createElideSection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* sectionTitle = new QLabel(QStringLiteral("3. QFontMetrics::elidedText 文本截断"));
    sectionTitle->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14px;"));

    // 用户输入长文本
    auto* inputRow = new QHBoxLayout;
    auto* inputLabel = new QLabel(QStringLiteral("长文本:"));
    m_elideInput =
        new QLineEdit(QStringLiteral("/home/charliechen/Tutorial_AwesomeQt/tutorial/advanced/"
                                     "03-qtwidgets/34-qlabel-advanced.md"));
    inputRow->addWidget(inputLabel);
    inputRow->addWidget(m_elideInput, 1);

    // 截断模式选择
    auto* modeRow = new QHBoxLayout;
    auto* modeLabel = new QLabel(QStringLiteral("截断方向:"));
    m_elideModeCombo = new QComboBox;
    m_elideModeCombo->addItem(QStringLiteral("ElideLeft（左侧省略）"),
                              static_cast<int>(Qt::ElideLeft));
    m_elideModeCombo->addItem(QStringLiteral("ElideMiddle（中间省略）"),
                              static_cast<int>(Qt::ElideMiddle));
    m_elideModeCombo->addItem(QStringLiteral("ElideRight（右侧省略）"),
                              static_cast<int>(Qt::ElideRight));
    modeRow->addWidget(modeLabel);
    modeRow->addWidget(m_elideModeCombo, 1);

    // 展示截断结果的标签
    m_elideLabel = new QLabel;
    m_elideLabel->setFrameShape(QFrame::Box);
    m_elideLabel->setFixedWidth(400);

    // 截断信息
    m_elideInfo = new QLabel;

    layout->addWidget(sectionTitle);
    layout->addLayout(inputRow);
    layout->addLayout(modeRow);
    layout->addWidget(m_elideLabel);
    layout->addWidget(m_elideInfo);

    // 文本或截断模式变化时重新计算
    connect(m_elideInput, &QLineEdit::textChanged, this, &LabelModeDemo::updateElideDisplay);
    connect(m_elideModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &LabelModeDemo::updateElideDisplay);

    // 初始填充
    updateElideDisplay();

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// 槽函数实现
// ─────────────────────────────────────────────────────────────────────────────

void LabelModeDemo::updateFormatDisplay()
{
    const QString text = m_formatInput->text();
    const Qt::TextFormat selectedFormat =
        static_cast<Qt::TextFormat>(m_formatCombo->currentData().toInt());

    // 三个标签始终使用固定格式，让用户直观看到同一文本在不同模式下的渲染差异
    m_plainLabel->setText(text);
    m_richLabel->setText(text);
    m_autoLabel->setText(text);

    // 高亮用户当前选择的模式
    const QString highlight = QStringLiteral("background-color: #e0f0ff;");
    const QString normal = QStringLiteral("background-color: transparent;");

    m_plainLabel->setStyleSheet(selectedFormat == Qt::PlainText ? highlight : normal);
    m_richLabel->setStyleSheet(selectedFormat == Qt::RichText ? highlight : normal);
    m_autoLabel->setStyleSheet(selectedFormat == Qt::AutoText ? highlight : normal);
}

void LabelModeDemo::toggleBuddyText()
{
    // 在含 & 标记和不含标记之间切换，演示快捷键的依赖关系
    if (m_buddyHasMarker) {
        // 移除 & 标记——setBuddy 不会报错但快捷键静默失效
        m_buddyLabel->setText(QStringLiteral("用户名:"));
        m_toggleBuddyBtn->setText(QStringLiteral("切换标签文本（当前：无 & 标记，Alt+U 已失效）"));
        m_buddyHasMarker = false;
    } else {
        // 恢复 & 标记——快捷键重新生效
        m_buddyLabel->setText(QStringLiteral("用户名(&U):"));
        m_toggleBuddyBtn->setText(QStringLiteral("切换标签文本（当前：含 &U 标记，Alt+U 有效）"));
        m_buddyHasMarker = true;
    }
}

void LabelModeDemo::updateElideDisplay()
{
    const QString text = m_elideInput->text();
    const Qt::TextElideMode mode =
        static_cast<Qt::TextElideMode>(m_elideModeCombo->currentData().toInt());

    // 使用标签的实际字体和宽度计算截断
    QFontMetrics fm(m_elideLabel->font());
    const int availableWidth = m_elideLabel->width();

    // elidedText 在指定宽度内截断文本，返回已含省略号的字符串
    const QString elided = fm.elidedText(text, mode, availableWidth);
    m_elideLabel->setText(elided);

    // 显示截断统计信息
    m_elideInfo->setText(QStringLiteral("原始长度: %1 字符 | 截断后: %2 字符 | 可用宽度: %3 px")
                             .arg(text.length())
                             .arg(elided.length())
                             .arg(availableWidth));
}
