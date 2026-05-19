/// @file    rich_text_navigator.cpp
/// @brief   RichTextNavigator 类实现——QTextDocument 操控与 QTextCursor 导航。
///
/// 对应教程：进阶层 03-QtWidgets/23-QTextEdit 进阶。

#include "rich_text_navigator.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextCursor>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

RichTextNavigator::RichTextNavigator(QWidget* parent)
    : QTextEdit(parent)
{
    setReadOnly(false);
    loadSampleContent();

    // 主布局：工具栏 + 编辑器 + 状态栏
    auto* wrapper = new QWidget(this);
    auto* outerLayout = new QVBoxLayout(wrapper);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    // 使用 QWidget::create() 方式嵌入工具栏和状态栏
    // 这里直接用窗口布局：把工具栏和状态栏放到主窗口上
    // QTextEdit 自身就是中心控件，我们在外部 QWidget 中组合
}

// ─────────────────────────────────────────────────────────────────────────────
// 因为 QTextEdit 本身就是编辑区，改用顶层 QWidget 来组合布局
// 这里直接在 main.cpp 中用外部 QWidget 组合，本类负责核心功能
// ─────────────────────────────────────────────────────────────────────────────

QWidget* RichTextNavigator::createToolBar()
{
    auto* bar = new QWidget;
    auto* layout = new QHBoxLayout(bar);
    layout->setContentsMargins(4, 4, 4, 4);

    m_btnStart = new QPushButton(QStringLiteral("跳到起始"));
    m_btnEnd = new QPushButton(QStringLiteral("跳到末尾"));
    m_btnSelectBlock = new QPushButton(QStringLiteral("选中段落"));
    m_btnSelectLine = new QPushButton(QStringLiteral("选中行"));
    m_btnSelectWord = new QPushButton(QStringLiteral("选中单词"));
    m_btnInsert = new QPushButton(QStringLiteral("插入富文本"));

    m_searchInput = new QLineEdit;
    m_searchInput->setPlaceholderText(QStringLiteral("输入搜索文本..."));
    m_btnSearch = new QPushButton(QStringLiteral("搜索"));

    layout->addWidget(m_btnStart);
    layout->addWidget(m_btnEnd);
    layout->addWidget(m_btnSelectBlock);
    layout->addWidget(m_btnSelectLine);
    layout->addWidget(m_btnSelectWord);
    layout->addWidget(m_btnInsert);
    layout->addStretch();
    layout->addWidget(m_searchInput, 1);
    layout->addWidget(m_btnSearch);

    // 连接导航按钮
    connect(m_btnStart, &QPushButton::clicked, this, &RichTextNavigator::moveToStart);
    connect(m_btnEnd, &QPushButton::clicked, this, &RichTextNavigator::moveToEnd);
    connect(m_btnSelectBlock, &QPushButton::clicked, this, &RichTextNavigator::selectBlock);
    connect(m_btnSelectLine, &QPushButton::clicked, this, &RichTextNavigator::selectLine);
    connect(m_btnSelectWord, &QPushButton::clicked, this, &RichTextNavigator::selectWord);
    connect(m_btnInsert, &QPushButton::clicked, this, &RichTextNavigator::insertRichText);
    connect(m_btnSearch, &QPushButton::clicked, this, &RichTextNavigator::performSearch);

    // 搜索框回车触发搜索
    connect(m_searchInput, &QLineEdit::returnPressed, this, &RichTextNavigator::performSearch);

    return bar;
}

QWidget* RichTextNavigator::createStatusBar()
{
    auto* bar = new QWidget;
    auto* layout = new QHBoxLayout(bar);
    layout->setContentsMargins(4, 2, 4, 2);

    m_cursorPosLabel = new QLabel;
    m_blockInfoLabel = new QLabel;
    m_docStatsLabel = new QLabel;

    layout->addWidget(m_cursorPosLabel);
    layout->addStretch();
    layout->addWidget(m_blockInfoLabel);
    layout->addStretch();
    layout->addWidget(m_docStatsLabel);

    // 光标位置变化时更新状态栏
    connect(this, &QTextEdit::cursorPositionChanged, this, &RichTextNavigator::updateStatusBar);

    return bar;
}

void RichTextNavigator::loadSampleContent()
{
    // 插入一段包含多种格式的演示 HTML
    const QString html = QStringLiteral(
        "<h2>QTextEdit 进阶演示</h2>"
        "<p>这段文本用于演示 <b>QTextDocument</b> 的底层操控能力。"
        "你可以使用工具栏按钮来体验 QTextCursor 的高级导航操作。</p>"
        "<h3>段落一：光标导航</h3>"
        "<p>QTextCursor 提供了 <i>movePosition</i> 方法来精确移动光标位置。"
        "你可以选中整行、整个段落或者单个单词。</p>"
        "<h3>段落二：富文本插入</h3>"
        "<p>使用 <u>insertHtml</u> 可以在光标位置插入 HTML 片段，"
        "新内容会被解析为富文本并融入现有文档结构。</p>"
        "<h3>段落三：文本搜索</h3>"
        "<p>QTextDocument::find 方法支持正向和反向搜索，"
        "配合 ExtraSelection 可以实现搜索高亮效果。"
        "试试在搜索框中输入 <font color='red'>QTextCursor</font> 看看效果。</p>"
        "<p>这是一段额外的测试文本，包含关键字 QTextDocument 和 QTextEdit，"
        "用于验证搜索功能是否能正确找到所有匹配项。</p>");

    setHtml(html);
}

// ─────────────────────────────────────────────────────────────────────────────
// 光标导航操作
// ─────────────────────────────────────────────────────────────────────────────

void RichTextNavigator::moveToStart()
{
    QTextCursor cursor = textCursor();
    // 移动到文档最前面，MoveMode 为 MoveAnchor 表示不保留选区
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    setTextCursor(cursor);
    ensureCursorVisible();
}

void RichTextNavigator::moveToEnd()
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    setTextCursor(cursor);
    ensureCursorVisible();
}

void RichTextNavigator::selectBlock()
{
    QTextCursor cursor = textCursor();
    // QTextCursor::BlockUnderCursor 选中光标所在的整个 block（段落）
    cursor.select(QTextCursor::BlockUnderCursor);
    setTextCursor(cursor);
}

void RichTextNavigator::selectLine()
{
    QTextCursor cursor = textCursor();
    // 先移到行首，再用 KeepAnchor 选到行尾
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    setTextCursor(cursor);
}

void RichTextNavigator::selectWord()
{
    QTextCursor cursor = textCursor();
    // WordUnderCursor 选中光标下方的整个单词
    cursor.select(QTextCursor::WordUnderCursor);
    setTextCursor(cursor);
}

// ─────────────────────────────────────────────────────────────────────────────
// 富文本插入
// ─────────────────────────────────────────────────────────────────────────────

void RichTextNavigator::insertRichText()
{
    QTextCursor cursor = textCursor();
    // insertHtml 在光标位置插入 HTML 片段，保留周围格式
    const QString htmlFragment = QStringLiteral(
        "<br/><table border='1' cellpadding='4'>"
        "<tr><td bgcolor='#e0f0ff'>单元格 A</td>"
        "<td bgcolor='#fff3e0'>单元格 B</td></tr>"
        "<tr><td bgcolor='#e8f5e9'>单元格 C</td>"
        "<td bgcolor='#fce4ec'>单元格 D</td></tr>"
        "</table><br/>");
    cursor.insertHtml(htmlFragment);
    setTextCursor(cursor);
    ensureCursorVisible();
}

// ─────────────────────────────────────────────────────────────────────────────
// 搜索与高亮
// ─────────────────────────────────────────────────────────────────────────────

void RichTextNavigator::performSearch()
{
    const QString keyword = m_searchInput->text().trimmed();
    if (keyword.isEmpty()) {
        // 清空高亮
        setExtraSelections({});
        return;
    }

    QList<QTextEdit::ExtraSelection> highlights;
    QTextDocument* doc = document();
    QTextCursor searchCursor(doc);

    // 从文档开头逐个搜索所有匹配项
    while (!searchCursor.isNull() && !searchCursor.atEnd()) {
        searchCursor = doc->find(keyword, searchCursor);
        if (!searchCursor.isNull()) {
            QTextEdit::ExtraSelection sel;
            sel.cursor = searchCursor;
            sel.format.setBackground(QColor(QStringLiteral("#FFF176")));  // 黄色高亮
            highlights.append(sel);
        }
    }

    setExtraSelections(highlights);

    // 如果有匹配项，跳转到第一个
    if (!highlights.isEmpty()) {
        setTextCursor(highlights.first().cursor);
        ensureCursorVisible();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 状态栏更新
// ─────────────────────────────────────────────────────────────────────────────

void RichTextNavigator::updateStatusBar()
{
    QTextCursor cursor = textCursor();
    QTextDocument* doc = document();

    // 光标绝对位置
    const int pos = cursor.position();

    // 当前 block 编号（从 0 开始）
    const int blockNum = cursor.blockNumber();

    // 文档统计
    const int totalBlocks = doc->blockCount();
    const int totalChars = doc->characterCount();

    m_cursorPosLabel->setText(QStringLiteral("光标位置: %1").arg(pos));
    m_blockInfoLabel->setText(
        QStringLiteral("段落: %1 / %2").arg(blockNum + 1).arg(totalBlocks));
    m_docStatsLabel->setText(
        QStringLiteral("总段落: %1 | 总字符: %2").arg(totalBlocks).arg(totalChars));
}
