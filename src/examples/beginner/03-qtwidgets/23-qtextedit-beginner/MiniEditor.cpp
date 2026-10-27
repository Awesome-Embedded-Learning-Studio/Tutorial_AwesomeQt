#include "MiniEditor.h"

#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QMessageBox>
#include <QCloseEvent>

MiniEditor::MiniEditor(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QTextEdit 综合演示 — 迷你文本编辑器");
    resize(640, 580);
    initUi();
}

void MiniEditor::closeEvent(QCloseEvent *event)
{
    if (m_editor->document()->isModified()) {
        auto ret = QMessageBox::question(
            this, "保存确认",
            "文档已修改但未保存，是否保存？",
            QMessageBox::Save | QMessageBox::Discard
                | QMessageBox::Cancel);

        if (ret == QMessageBox::Save) {
            // 模拟保存操作
            m_editor->document()->setModified(false);
            event->accept();
        } else if (ret == QMessageBox::Discard) {
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void MiniEditor::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ---- 标题 ----
    auto *titleLabel = new QLabel("迷你文本编辑器");
    titleLabel->setFont(QFont("Arial", 15, QFont::Bold));
    mainLayout->addWidget(titleLabel);

    // ================================================================
    // 工具栏: 格式按钮 + 功能按钮
    // ================================================================
    auto *toolbarGroup = new QGroupBox("格式工具栏");
    auto *toolbarLayout = new QHBoxLayout(toolbarGroup);
    toolbarLayout->setSpacing(6);

    // 加粗按钮
    auto *boldBtn = new QPushButton("B 加粗");
    boldBtn->setAutoDefault(false);
    boldBtn->setStyleSheet(
        "QPushButton { font-weight: bold; padding: 5px 12px; }");
    connect(boldBtn, &QPushButton::clicked, this, [this]() {
        QTextCharFormat fmt;
        fmt.setFontWeight(QFont::Bold);
        applyFormat(fmt);
    });

    // 斜体按钮
    auto *italicBtn = new QPushButton("I 斜体");
    italicBtn->setAutoDefault(false);
    italicBtn->setStyleSheet(
        "QPushButton { font-style: italic; padding: 5px 12px; }");
    connect(italicBtn, &QPushButton::clicked, this, [this]() {
        QTextCharFormat fmt;
        fmt.setFontItalic(true);
        applyFormat(fmt);
    });

    // 下划线按钮
    auto *underlineBtn = new QPushButton("U 下划线");
    underlineBtn->setAutoDefault(false);
    underlineBtn->setStyleSheet(
        "QPushButton { text-decoration: underline; padding: 5px 12px; }");
    connect(underlineBtn, &QPushButton::clicked, this, [this]() {
        QTextCharFormat fmt;
        fmt.setFontUnderline(true);
        applyFormat(fmt);
    });

    toolbarLayout->addWidget(boldBtn);
    toolbarLayout->addWidget(italicBtn);
    toolbarLayout->addWidget(underlineBtn);

    toolbarLayout->addSpacing(16);

    // 插入蓝色标题按钮
    auto *insertTitleBtn = new QPushButton("插入蓝色标题");
    insertTitleBtn->setAutoDefault(false);
    connect(insertTitleBtn, &QPushButton::clicked, this,
            &MiniEditor::insertBlueTitle);

    // 清除所有格式按钮
    auto *clearFmtBtn = new QPushButton("清除所有格式");
    clearFmtBtn->setAutoDefault(false);
    connect(clearFmtBtn, &QPushButton::clicked, this, [this]() {
        m_editor->setPlainText(m_editor->toPlainText());
    });

    toolbarLayout->addWidget(insertTitleBtn);
    toolbarLayout->addWidget(clearFmtBtn);
    toolbarLayout->addStretch();

    mainLayout->addWidget(toolbarGroup);

    // ================================================================
    // 编辑区域: QTextEdit + 初始 HTML 内容
    // ================================================================
    m_editor = new QTextEdit();
    m_editor->setHtml(
        "<h2>QTextEdit 演示</h2>"
        "<p>这是一个 <b>富文本</b> 编辑器。你可以选中文字后点击"
        "工具栏按钮来应用格式。</p>"
        "<p>支持的格式包括：<i>斜体</i>、<u>下划线</u>、"
        "<b>加粗</b>等。试试选中一些文字然后点击上面的按钮吧。</p>"
        "<p>你还可以点击「插入蓝色标题」按钮，在当前光标位置插入"
        "一段蓝色加粗的文字。</p>");
    m_editor->document()->setModified(false);

    mainLayout->addWidget(m_editor, 1);

    // ================================================================
    // 状态栏: 字符数 / 行数 / 修改状态
    // ================================================================
    auto *statusLayout = new QHBoxLayout();

    m_charCountLabel = new QLabel("字符: 0");
    m_lineCountLabel = new QLabel("行数: 0");
    m_modifiedLabel = new QLabel("状态: 未修改");
    m_modifiedLabel->setStyleSheet("color: #888;");

    m_charCountLabel->setStyleSheet("color: #666; font-size: 11px;");
    m_lineCountLabel->setStyleSheet("color: #666; font-size: 11px;");
    m_modifiedLabel->setStyleSheet("color: #888; font-size: 11px;");

    statusLayout->addWidget(m_charCountLabel);
    statusLayout->addWidget(m_lineCountLabel);
    statusLayout->addStretch();
    statusLayout->addWidget(m_modifiedLabel);

    mainLayout->addLayout(statusLayout);

    // ---- 信号连接 ----

    // textChanged: 更新字符数和行数
    connect(m_editor, &QTextEdit::textChanged, this,
            &MiniEditor::updateStatusBar);

    // modificationChanged: 更新修改状态标签和窗口标题
    connect(m_editor->document(), &QTextDocument::modificationChanged,
            this, &MiniEditor::onModificationChanged);

    // 初始化状态栏
    updateStatusBar();
}

void MiniEditor::applyFormat(const QTextCharFormat &format)
{
    QTextCursor cursor = m_editor->textCursor();

    if (cursor.hasSelection()) {
        // 有选中文字: 对选中部分应用格式
        cursor.mergeCharFormat(format);
    } else {
        // 没有选中文字: 设置光标位置的预格式（下次输入生效）
        cursor.mergeCharFormat(format);
    }

    m_editor->mergeCurrentCharFormat(format);
}

void MiniEditor::insertBlueTitle()
{
    QTextCursor cursor = m_editor->textCursor();

    QTextCharFormat titleFormat;
    titleFormat.setFontWeight(QFont::Bold);
    titleFormat.setForeground(QColor("#1976D2"));
    titleFormat.setFontPointSize(14);

    cursor.insertText("\n蓝色标题文字\n", titleFormat);

    // 插入后恢复默认格式
    QTextCharFormat normalFormat;
    normalFormat.setFontWeight(QFont::Normal);
    normalFormat.setForeground(QColor("#333333"));
    normalFormat.setFontPointSize(10);
    cursor.setCharFormat(normalFormat);
    m_editor->setTextCursor(cursor);
}

void MiniEditor::updateStatusBar()
{
    auto *doc = m_editor->document();
    m_charCountLabel->setText(
        QString("字符: %1").arg(doc->characterCount()));
    m_lineCountLabel->setText(
        QString("行数: %1").arg(doc->blockCount()));
}

void MiniEditor::onModificationChanged(bool modified)
{
    if (modified) {
        m_modifiedLabel->setText("状态: 已修改 *");
        m_modifiedLabel->setStyleSheet(
            "color: #D32F2F; font-size: 11px; font-weight: bold;");

        // 窗口标题加星号
        QString title = windowTitle();
        if (!title.endsWith(" *")) {
            setWindowTitle(title + " *");
        }
    } else {
        m_modifiedLabel->setText("状态: 未修改");
        m_modifiedLabel->setStyleSheet(
            "color: #888; font-size: 11px;");

        // 窗口标题移除星号
        QString title = windowTitle();
        title.remove(" *");
        setWindowTitle(title);
    }
}
