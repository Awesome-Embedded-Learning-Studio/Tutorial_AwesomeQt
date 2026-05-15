/// @file    rich_text_widget.cpp
/// @brief   RichTextWidget 类实现——富文本格式控制与文档信息展示。
///
/// 对应教程：进阶层 02-QtGui/04-字体与文本高级用法。
/// 核心演示：QTextCursor + QTextCharFormat 修改字符属性，
/// QTextCursor + QTextBlockFormat 修改段落属性，
/// QTextDocument::blockCount() / characterCount() 获取结构信息。

#include "rich_text_widget.h"

#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextEdit>
#include <QVBoxLayout>

RichTextWidget::RichTextWidget(QWidget* parent)
    : QWidget(parent)
    , m_textEdit(nullptr)
    , m_infoLabel(nullptr)
    , m_boldButton(nullptr)
    , m_italicButton(nullptr)
{
    auto* mainLayout = new QVBoxLayout(this);

    // 工具栏：格式化按钮
    mainLayout->addLayout(createToolbarLayout());

    // 编辑区
    m_textEdit = new QTextEdit(this);
    m_textEdit->setPlaceholderText(
        QStringLiteral("在此输入文字，选中后可使用上方按钮进行格式化..."));
    mainLayout->addWidget(m_textEdit, /*stretch=*/1);

    // 底部文档信息
    m_infoLabel = createInfoLabel();
    mainLayout->addWidget(m_infoLabel);

    // 文档内容变化时刷新统计信息
    connect(m_textEdit, &QTextEdit::textChanged, this,
            &RichTextWidget::updateDocumentInfo);

    // 光标位置变化时也刷新（选中范围可能改变格式状态）
    connect(m_textEdit, &QTextEdit::cursorPositionChanged, this,
            &RichTextWidget::updateDocumentInfo);

    // 初始刷新一次
    updateDocumentInfo();
}

void RichTextWidget::toggleBold()
{
    auto cursor = m_textEdit->textCursor();

    // QTextCharFormat 用于描述字符级属性：字体、颜色、粗体、斜体等
    QTextCharFormat fmt;
    fmt.setFontWeight(cursor.charFormat().fontWeight() == QFont::Bold
                          ? QFont::Normal
                          : QFont::Bold);

    // mergeCharFormat 会将 fmt 中已设置的属性合并到选中区域，
    // 未设置的属性保持不变——这是"切换"语义的关键
    cursor.mergeCharFormat(fmt);
    m_textEdit->setTextCursor(cursor);
}

void RichTextWidget::toggleItalic()
{
    auto cursor = m_textEdit->textCursor();

    QTextCharFormat fmt;
    fmt.setFontItalic(!cursor.charFormat().fontItalic());
    cursor.mergeCharFormat(fmt);
    m_textEdit->setTextCursor(cursor);
}

void RichTextWidget::pickTextColor()
{
    auto cursor = m_textEdit->textCursor();
    QColor initial = cursor.charFormat().foreground().color();

    // QColorDialog 提供标准的颜色选择界面
    QColor chosen = QColorDialog::getColor(initial, this,
                                           QStringLiteral("选择文字颜色"));
    if (!chosen.isValid()) {
        return; // 用户取消
    }

    QTextCharFormat fmt;
    fmt.setForeground(chosen);
    cursor.mergeCharFormat(fmt);
    m_textEdit->setTextCursor(cursor);
}

void RichTextWidget::alignCenter()
{
    auto cursor = m_textEdit->textCursor();

    // QTextBlockFormat 用于段落级属性：对齐、缩进、行距等
    QTextBlockFormat blockFmt = cursor.blockFormat();
    blockFmt.setAlignment(Qt::AlignCenter);

    // mergeBlockFormat 将段落格式属性合并到当前块
    cursor.mergeBlockFormat(blockFmt);
    m_textEdit->setTextCursor(cursor);
}

void RichTextWidget::alignLeft()
{
    auto cursor = m_textEdit->textCursor();

    QTextBlockFormat blockFmt = cursor.blockFormat();
    blockFmt.setAlignment(Qt::AlignLeft);
    cursor.mergeBlockFormat(blockFmt);
    m_textEdit->setTextCursor(cursor);
}

void RichTextWidget::updateDocumentInfo()
{
    // QTextDocument 是 QTextEdit 底层的文档模型，
    // 持有全部文本内容、格式信息和文档结构
    auto* doc = m_textEdit->document();

    int blockCount = doc->blockCount();     // 段落数（每个 \n 分隔一个 block）
    int charCount  = doc->characterCount(); // 字符数（含不可见字符如段落分隔符）

    m_infoLabel->setText(
        QStringLiteral("段落数: %1  |  字符数: %2")
            .arg(blockCount)
            .arg(charCount));
}

auto RichTextWidget::createToolbarLayout() -> QHBoxLayout*
{
    auto* layout = new QHBoxLayout;

    m_boldButton = new QPushButton(QStringLiteral("B 粗体"), this);
    m_italicButton = new QPushButton(QStringLiteral("I 斜体"), this);
    auto* colorButton = new QPushButton(QStringLiteral("A 颜色"), this);
    auto* centerButton = new QPushButton(QStringLiteral("居中"), this);
    auto* leftButton = new QPushButton(QStringLiteral("左对齐"), this);

    // @note 按钮使用 setFlat(true) 去除边框，使其更像工具栏按钮
    m_boldButton->setFlat(true);
    m_italicButton->setFlat(true);
    colorButton->setFlat(true);
    centerButton->setFlat(true);
    leftButton->setFlat(true);

    layout->addWidget(m_boldButton);
    layout->addWidget(m_italicButton);
    layout->addWidget(colorButton);
    layout->addWidget(centerButton);
    layout->addWidget(leftButton);
    layout->addStretch(); // 按钮靠左，右侧弹性空白

    connect(m_boldButton, &QPushButton::clicked, this,
            &RichTextWidget::toggleBold);
    connect(m_italicButton, &QPushButton::clicked, this,
            &RichTextWidget::toggleItalic);
    connect(colorButton, &QPushButton::clicked, this,
            &RichTextWidget::pickTextColor);
    connect(centerButton, &QPushButton::clicked, this,
            &RichTextWidget::alignCenter);
    connect(leftButton, &QPushButton::clicked, this,
            &RichTextWidget::alignLeft);

    return layout;
}

auto RichTextWidget::createInfoLabel() -> QLabel*
{
    auto* label = new QLabel(this);
    label->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    return label;
}
