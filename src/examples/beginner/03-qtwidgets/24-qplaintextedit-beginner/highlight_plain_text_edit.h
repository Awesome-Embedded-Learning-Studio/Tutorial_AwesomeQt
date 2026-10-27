// QtWidgets 入门示例 24: QPlainTextEdit 纯文本高性能编辑器
// 演示：appendPlainText 追加日志的正确用法
//       setMaximumBlockCount 限制行数防内存溢出
//       highlightCurrentLine 实现行高亮效果

#pragma once

#include <QPlainTextEdit>

// ============================================================================
// HighlightPlainTextEdit: 带当前行高亮的纯文本编辑器
// ============================================================================
class HighlightPlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit HighlightPlainTextEdit(QWidget *parent = nullptr);

private:
    /// @brief 高亮当前光标所在行
    void highlightCurrentLine();
};
