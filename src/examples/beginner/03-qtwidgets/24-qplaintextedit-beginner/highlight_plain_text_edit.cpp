#include "highlight_plain_text_edit.h"

#include <QTextEdit>
#include <QTextCursor>

HighlightPlainTextEdit::HighlightPlainTextEdit(QWidget *parent)
    : QPlainTextEdit(parent)
{
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, &HighlightPlainTextEdit::highlightCurrentLine);
}

void HighlightPlainTextEdit::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> selections;

    QTextEdit::ExtraSelection lineSelection;
    lineSelection.format.setBackground(QColor("#E8F0FE"));
    lineSelection.format.setProperty(
        QTextCharFormat::FullWidthSelection, true);
    lineSelection.cursor = textCursor();
    lineSelection.cursor.clearSelection();

    selections.append(lineSelection);
    setExtraSelections(selections);
}
