/// @file    widget.h
/// @brief   演示 QPrinter 自定义页眉页脚与分页逻辑。
///
/// 对应教程：进阶层 03-QtWidgets 打印高级用法。
/// 本示例展示如何使用 QPainter 在 QPrinter 上逐页绘制内容，
/// 包含自定义页眉（标题 + 日期）和页脚（页码），以及手动分页。

#pragma once

#include <QMainWindow>

class QTextEdit;

/// @brief 主窗口，提供文本编辑区和打印到 PDF 功能。
class PrintMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit PrintMainWindow(QWidget* parent = nullptr);

private slots:
    /// @brief 将 QTextEdit 中的内容打印到 PDF 文件。
    /// @note 手动逐页绘制页眉、正文、页脚，而非使用 QTextDocument::print()，
    ///       以便完全控制每页的布局和装饰元素。
    void printToPdf();

private:
    /// @brief 初始化 UI：文本编辑器和打印按钮。
    void setupUI();

    /// @brief 在给定页面上绘制页眉（标题 + 日期）。
    /// @param[in] painter 用于绘制的 QPainter 对象。
    /// @param[in] pageRect 页面可绘制区域。
    /// @param[in] title 页眉标题文本。
    /// @return 页眉占用的高度，用于后续内容偏移。
    /// @note 返回高度使正文绘制能正确跳过页眉区域。
    qreal drawHeader(QPainter& painter, const QRectF& pageRect,
                     const QString& title);

    /// @brief 在给定页面上绘制页脚（页码）。
    /// @param[in] painter 用于绘制的 QPainter 对象。
    /// @param[in] pageRect 页面可绘制区域。
    /// @param[in] pageNumber 当前页码（从 1 开始）。
    /// @param[in] totalPages 总页数。
    /// @return 页脚占用的高度，用于正文区域计算。
    qreal drawFooter(QPainter& painter, const QRectF& pageRect,
                     int pageNumber, int totalPages);

    QTextEdit* m_textEdit;       ///< 文本编辑器，用户输入待打印内容
};
