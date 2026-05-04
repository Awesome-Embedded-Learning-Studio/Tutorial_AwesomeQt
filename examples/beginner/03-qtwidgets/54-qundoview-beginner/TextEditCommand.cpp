#include "TextEditCommand.h"

#include <QPlainTextEdit>
#include <QTextCursor>

TextEditCommand::TextEditCommand(QPlainTextEdit *editor,
                                 const QString &removedText,
                                 const QString &insertedText,
                                 int position,
                                 QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_editor(editor)
    , m_removedText(removedText)
    , m_insertedText(insertedText)
    , m_position(position)
{
    // 根据操作类型生成描述文本（QUndoView 中显示）
    if (!insertedText.isEmpty() && removedText.isEmpty()) {
        setText(QString("插入 \"%1\"").arg(
            truncated(insertedText, 20)));
    } else if (insertedText.isEmpty()
               && !removedText.isEmpty()) {
        setText(QString("删除 \"%1\"").arg(
            truncated(removedText, 20)));
    } else {
        setText(QString("替换 \"%1\" -> \"%2\"")
                    .arg(truncated(removedText, 15),
                         truncated(insertedText, 15)));
    }
}

void TextEditCommand::undo()
{
    QTextCursor cursor = m_editor->textCursor();

    // 移除插入的文本
    cursor.setPosition(m_position);
    cursor.setPosition(
        m_position + m_insertedText.length(),
        QTextCursor::KeepAnchor);
    cursor.removeSelectedText();

    // 恢复被删除的文本
    if (!m_removedText.isEmpty()) {
        cursor.setPosition(m_position);
        cursor.insertText(m_removedText);
    }

    m_editor->setTextCursor(cursor);
}

void TextEditCommand::redo()
{
    QTextCursor cursor = m_editor->textCursor();

    // 移除旧文本
    if (!m_removedText.isEmpty()) {
        cursor.setPosition(m_position);
        cursor.setPosition(
            m_position + m_removedText.length(),
            QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
    }

    // 插入新文本
    cursor.setPosition(m_position);
    cursor.insertText(m_insertedText);

    m_editor->setTextCursor(cursor);
}

/// @brief 截断过长文本用于显示
QString TextEditCommand::truncated(const QString &text, int maxLen)
{
    return text.length() <= maxLen
               ? text
               : text.left(maxLen) + "...";
}
