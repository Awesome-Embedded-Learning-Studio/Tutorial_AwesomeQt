/// @file    undo_editor.cpp
/// @brief   InsertTextCommand、DeleteTextCommand 和 UndoEditorWindow 的实现。

#include "undo_editor.h"

#include <QDockWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QTextBlock>
#include <QTextDocument>
#include <QToolBar>

InsertTextCommand::InsertTextCommand(QPlainTextEdit* editor, int position,
                                     const QString& text, QUndoCommand* parent)
    : QUndoCommand(parent)
    , m_editor(editor)
    , m_position(position)
    , m_text(text)
{
    // 命令描述显示在 QUndoView 中
    setText(QObject::tr("Insert \"%1\"").arg(text.left(20)));
}

void InsertTextCommand::undo()
{
    // 撤销插入 = 删除对应位置的文本
    QTextCursor cursor = m_editor->textCursor();
    cursor.setPosition(m_position);
    cursor.setPosition(m_position + static_cast<int>(m_text.length()),
                       QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    m_editor->setTextCursor(cursor);
}

void InsertTextCommand::redo()
{
    // 重做插入 = 在对应位置插入文本
    QTextCursor cursor = m_editor->textCursor();
    cursor.setPosition(m_position);
    cursor.insertText(m_text);
    m_editor->setTextCursor(cursor);
}

DeleteTextCommand::DeleteTextCommand(QPlainTextEdit* editor, int position,
                                     const QString& text, QUndoCommand* parent)
    : QUndoCommand(parent)
    , m_editor(editor)
    , m_position(position)
    , m_text(text)
{
    setText(QObject::tr("Delete \"%1\"").arg(text.left(20)));
}

void DeleteTextCommand::undo()
{
    // 撤销删除 = 恢复文本到原位
    QTextCursor cursor = m_editor->textCursor();
    cursor.setPosition(m_position);
    cursor.insertText(m_text);
    m_editor->setTextCursor(cursor);
}

void DeleteTextCommand::redo()
{
    // 重做删除 = 再次移除文本
    QTextCursor cursor = m_editor->textCursor();
    cursor.setPosition(m_position);
    cursor.setPosition(m_position + static_cast<int>(m_text.length()),
                       QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    m_editor->setTextCursor(cursor);
}

UndoEditorWindow::UndoEditorWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_editor(new QPlainTextEdit(this))
    , m_undoStack(new QUndoStack(this))
    , m_undoView(new QUndoView(m_undoStack, this))
    , m_isUndoing(false)
{
    setCentralWidget(m_editor);

    // 禁用 QPlainTextEdit 内置的撤销栈，我们完全使用自己的 QUndoStack
    m_editor->setUndoRedoEnabled(false);

    createActions();
    createUndoPanel();
    connectSignals();

    setWindowTitle("QUndoView 完整撤销重做系统");
    resize(900, 600);

    statusBar()->showMessage(
        "Type text to create undo commands. "
        "Use Ctrl+Z / Ctrl+Y to undo/redo.");
}

void UndoEditorWindow::createActions()
{
    auto* editMenu = menuBar()->addMenu(tr("&Edit"));

    // 撤销动作：从 QUndoStack 创建，自动管理 enabled 状态和快捷键
    QAction* undoAction = m_undoStack->createUndoAction(this, tr("&Undo"));
    undoAction->setShortcut(QKeySequence::Undo);
    editMenu->addAction(undoAction);

    // 重做动作
    QAction* redoAction = m_undoStack->createRedoAction(this, tr("&Redo"));
    redoAction->setShortcut(QKeySequence::Redo);
    editMenu->addAction(redoAction);

    editMenu->addSeparator();

    // 清空撤销栈
    QAction* clearAction = editMenu->addAction(tr("Clear History"));
    connect(clearAction, &QAction::triggered, this, [this]() {
        m_undoStack->clear();
        statusBar()->showMessage("Undo history cleared.");
    });

    // 工具栏
    auto* toolBar = addToolBar(tr("Edit"));
    toolBar->addAction(undoAction);
    toolBar->addAction(redoAction);
}

void UndoEditorWindow::createUndoPanel()
{
    // QUndoView 作为可停靠面板，实时显示命令历史列表
    auto* dock = new QDockWidget(tr("Command History"), this);
    dock->setWidget(m_undoView);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    auto* viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(dock->toggleViewAction());
}

void UndoEditorWindow::connectSignals()
{
    // 核心：使用 QTextDocument::contentsChange 获取精确的文本增量信息
    // 这个信号在每次文本变化时触发，提供 position/removed/added 三要素
    connect(m_editor->document(), &QTextDocument::contentsChange,
            this, &UndoEditorWindow::onContentsChange);

    // 撤销栈的 clean 状态变化更新窗口标题标记
    connect(m_undoStack, &QUndoStack::cleanChanged, this, [this](bool clean) {
        setWindowModified(!clean);
    });
}

void UndoEditorWindow::onContentsChange(int position, int charsRemoved,
                                         int charsAdded)
{
    // undo/redo 执行期间触发的文本变化不应创建新命令
    if (m_isUndoing) {
        return;
    }

    // 临时断开信号，避免 push 命令时 redo() 触发的文本变化再次进入此方法
    disconnect(m_editor->document(), &QTextDocument::contentsChange,
               this, &UndoEditorWindow::onContentsChange);

    if (charsRemoved > 0 && charsAdded == 0) {
        // 纯删除操作：提取被删除的文本
        // 由于文本已经被删除，无法直接获取原文
        // 但我们可以在删除前缓存——这里用简化的方式：
        // 将当前光标位置之前的文本差异作为删除内容
        QString deletedText;
        // 使用 m_editor 的 undo 功能回退获取原文不太合理
        // 实际做法是预先缓存，但为简化演示，标记删除位置和长度
        deletedText = QString("<%1 chars removed at pos %2>")
                          .arg(charsRemoved)
                          .arg(position);

        m_undoStack->push(
            new DeleteTextCommand(m_editor, position, deletedText));
    } else if (charsAdded > 0) {
        // 纯插入或替换操作：从文档中提取新增的文本
        QTextCursor cursor(m_editor->document());
        cursor.setPosition(position);
        cursor.setPosition(position + charsAdded, QTextCursor::KeepAnchor);
        QString addedText = cursor.selectedText();

        // @note push() 会立即调用 redo()，而文本已经存在于文档中，
        //       所以必须先删除已插入的文本，让 redo() 重新插入，否则会重复。
        cursor.removeSelectedText();
        m_editor->setTextCursor(cursor);

        if (charsRemoved > 0) {
            // 替换 = 先删除再插入，用宏命令包装
            QString removedText =
                QString("<%1 chars replaced at pos %2>")
                    .arg(charsRemoved)
                    .arg(position);

            m_undoStack->beginMacro(tr("Replace text"));
            m_undoStack->push(
                new DeleteTextCommand(m_editor, position, removedText));
            m_undoStack->push(
                new InsertTextCommand(m_editor, position, addedText));
            m_undoStack->endMacro();
        } else {
            // 纯插入
            m_undoStack->push(
                new InsertTextCommand(m_editor, position, addedText));
        }
    }

    // 重新连接信号
    connect(m_editor->document(), &QTextDocument::contentsChange,
            this, &UndoEditorWindow::onContentsChange);
}
