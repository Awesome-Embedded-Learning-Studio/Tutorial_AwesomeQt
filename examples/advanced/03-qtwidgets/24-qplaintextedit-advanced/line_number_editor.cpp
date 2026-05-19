/// @file    line_number_editor.cpp
/// @brief   LineNumberEditor 类实现——行号编辑器与侧边栏联动。
///
/// 对应教程：进阶层 03-QtWidgets/24-QPlainTextEdit 进阶。

#include "line_number_editor.h"
#include "line_number_sidebar.h"

#include <QResizeEvent>
#include <QTextBlock>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

LineNumberEditor::LineNumberEditor(QWidget* parent)
    : QPlainTextEdit(parent)
{
    m_sidebar = new LineNumberSidebar(this);

    // blockCountChanged：行数变化时更新行号区域宽度
    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &LineNumberEditor::onBlockCountChanged);

    // updateRequest：编辑器内容滚动或重绘时同步更新行号
    connect(this, &QPlainTextEdit::updateRequest,
            this, &LineNumberEditor::onUpdateRequest);

    // 初始化行号区域宽度和视口边距
    updateSidebarWidth();
}

// ─────────────────────────────────────────────────────────────────────────────
// 保护方法
// ─────────────────────────────────────────────────────────────────────────────

void LineNumberEditor::resizeEvent(QResizeEvent* event)
{
    QPlainTextEdit::resizeEvent(event);

    // 编辑器大小变化时重新定位行号侧边栏
    const QRect cr = contentsRect();
    m_sidebar->setGeometry(cr.left(), cr.top(),
                           m_sidebar->calculateWidth(), cr.height());
}

// ─────────────────────────────────────────────────────────────────────────────
// 私有槽
// ─────────────────────────────────────────────────────────────────────────────

void LineNumberEditor::onBlockCountChanged(int newCount)
{
    Q_UNUSED(newCount)
    updateSidebarWidth();
}

void LineNumberEditor::onUpdateRequest(const QRect& rect, int dy)
{
    if (dy != 0) {
        // 垂直滚动：行号区域整体平移 dy 像素
        m_sidebar->scroll(0, dy);
    } else {
        // 非滚动重绘（如文本变化）：更新行号区域中对应矩形
        m_sidebar->update(0, rect.y(), m_sidebar->width(), rect.height());
    }

    // rect 包含编辑器完整宽度时，需要重绘整个行号区域
    if (rect.contains(viewport()->rect())) {
        updateSidebarWidth();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 私有方法
// ─────────────────────────────────────────────────────────────────────────────

void LineNumberEditor::updateSidebarWidth()
{
    // 通过 setViewportMargins 给行号区域留出左侧空间
    setViewportMargins(m_sidebar->calculateWidth(), 0, 0, 0);
}
