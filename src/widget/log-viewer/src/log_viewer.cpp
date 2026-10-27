/**
 * @file log_viewer.cpp
 * @brief LogViewer 控件实现——级别染色 + 自动滚底 + 行数上限裁旧
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "log_viewer.h"

#include <QFontDatabase>
#include <QPlainTextEdit>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTime>
#include <QVBoxLayout>

namespace AwesomeQt {

LogViewer::LogViewer(QWidget* parent) : QWidget(parent) {
    // view_ 由 this 父对象托管，构造即 new、放布局。
    view_ = new QPlainTextEdit(this);
    view_->setReadOnly(true);
    // 非等宽字体下日志列不齐，换等宽更利于阅读
    view_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    view_->setLineWrapMode(QPlainTextEdit::NoWrap);
    // 中心对齐滚动条：默认即可，这里不强制

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(view_);
}

void LogViewer::append(Level level, const QString& message) {
    // 拼装行：[HH:MM:SS] [LEVEL] message（时间戳可关）
    QString line;
    if (show_timestamp_) {
        line +=
            QStringLiteral("[%1] ").arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss")));
    }
    line += QStringLiteral("[%1] %2").arg(levelLabel(level), message);

    // 用 QTextCursor 在文末按 level 套前景色插入，QPlainTextEdit 支持按插入设色
    QTextCursor cursor(view_->document());
    cursor.movePosition(QTextCursor::End);

    QTextCharFormat fmt;
    const QColor color = colorForLevel(level);
    if (color.isValid()) {
        fmt.setForeground(color);
    }
    // 文末非空时先补一个换行，保证新行独立成块
    if (!cursor.atStart()) {
        cursor.insertText(QStringLiteral("\n"));
    }
    cursor.insertText(line, fmt);

    trimOldBlocks();

    if (auto_scroll_) {
        // 滚到底让最新行可见
        view_->moveCursor(QTextCursor::End);
        view_->ensureCursorVisible();
    }
}

void LogViewer::appendInfo(const QString& message) {
    append(Level::Info, message);
}

void LogViewer::appendWarning(const QString& message) {
    append(Level::Warning, message);
}

void LogViewer::appendError(const QString& message) {
    append(Level::Error, message);
}

void LogViewer::clear() {
    view_->clear();
}

int LogViewer::lineCount() const {
    return view_->blockCount();
}

void LogViewer::setMaxLines(int maxLines) {
    // 夹到 >=1，避免上限为 0 时裁成空或被绕过
    const int clamped = maxLines < 1 ? 1 : maxLines;
    if (clamped == max_lines_) {
        return;
    }
    max_lines_ = clamped;
    // 已有内容可能已超新上限，立即裁一次
    trimOldBlocks();
    emit maxLinesChanged(max_lines_);
}

int LogViewer::maxLines() const {
    return max_lines_;
}

void LogViewer::setAutoScroll(bool autoScroll) {
    if (autoScroll == auto_scroll_) {
        return;
    }
    auto_scroll_ = autoScroll;
    emit autoScrollChanged(auto_scroll_);
}

bool LogViewer::autoScroll() const {
    return auto_scroll_;
}

void LogViewer::setShowTimestamp(bool show) {
    if (show == show_timestamp_) {
        return;
    }
    show_timestamp_ = show;
    emit showTimestampChanged(show);
}

bool LogViewer::showTimestamp() const {
    return show_timestamp_;
}

QSize LogViewer::sizeHint() const {
    return QSize(380, 220);
}

QColor LogViewer::colorForLevel(Level level) const {
    switch (level) {
        case Level::Info:
            // 返回无效色：用 view 默认前景色
            return QColor();
        case Level::Warning:
            return QColor(180, 120, 0); // 暗黄
        case Level::Error:
            return QColor(200, 0, 0); // 红
    }
    return QColor();
}

QString LogViewer::levelLabel(Level level) {
    switch (level) {
        case Level::Info:
            return QStringLiteral("INFO");
        case Level::Warning:
            return QStringLiteral("WARN");
        case Level::Error:
            return QStringLiteral("ERROR");
    }
    return QStringLiteral("INFO");
}

void LogViewer::trimOldBlocks() {
    const int count = view_->blockCount();
    if (count <= max_lines_) {
        return;
    }
    const int toRemove = count - max_lines_;
    QTextCursor cursor(view_->document());
    cursor.movePosition(QTextCursor::Start);
    // 从文档头连续选中 toRemove 个块（含每块的行尾换行），整段删除
    for (int i = 0; i < toRemove; ++i) {
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
        // 连块尾换行一起选中
        if (cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor)) {
            // 已把行尾 \n 带进选区
        }
    }
    cursor.removeSelectedText();
}

} // namespace AwesomeQt
