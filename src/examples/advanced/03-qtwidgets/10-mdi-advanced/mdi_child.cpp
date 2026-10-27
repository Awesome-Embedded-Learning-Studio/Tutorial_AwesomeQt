/// @file    mdi_child.cpp
/// @brief   MdiChild 类实现——MDI 子窗口内容控件。
///
/// 对应教程：进阶层 03-QtWidgets/10-MDI 进阶。

#include "mdi_child.h"

#include <QCloseEvent>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

MdiChild::MdiChild(QWidget* parent)
    : QWidget(parent)
    , m_editor(nullptr)
    , m_modLabel(nullptr)
{
    auto* layout = new QVBoxLayout(this);

    // 文本编辑器
    m_editor = new QPlainTextEdit;
    layout->addWidget(m_editor);

    // 底部修改状态指示
    m_modLabel = new QLabel(QStringLiteral("无修改"));
    layout->addWidget(m_modLabel);

    // 文档修改状态变化时更新指示标签
    connect(m_editor->document(), &QTextDocument::modificationChanged, this, [this](bool changed) {
        m_modLabel->setText(changed ? QStringLiteral("[已修改]") : QStringLiteral("无修改"));
    });
}

// ─────────────────────────────────────────────────────────────────────────────
// 公有方法
// ─────────────────────────────────────────────────────────────────────────────

void MdiChild::setDocumentTitle(const QString& title)
{
    setWindowTitle(title);
}

QString MdiChild::textContent() const
{
    return m_editor->toPlainText();
}

bool MdiChild::isModified() const
{
    return m_editor->document()->isModified();
}

// ─────────────────────────────────────────────────────────────────────────────
// 关闭事件拦截——保存确认
// ─────────────────────────────────────────────────────────────────────────────

void MdiChild::closeEvent(QCloseEvent* event)
{
    if (isModified()) {
        const auto result = QMessageBox::warning(
            this,
            QStringLiteral("确认关闭"),
            QStringLiteral("文档 \"%1\" 有未保存的修改，是否关闭？").arg(windowTitle()),
            QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Cancel);

        if (result == QMessageBox::Discard) {
            // 用户选择放弃修改，允许关闭
            event->accept();
        } else {
            // 用户取消，阻止关闭
            event->ignore();
        }
    } else {
        event->accept();
    }
}
