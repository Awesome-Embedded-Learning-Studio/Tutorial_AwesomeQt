/// @file    widget.h
/// @brief   演示 QPrintPreviewDialog 自定义工具栏操作。
///
/// 对应教程：进阶层 03-QtWidgets 打印预览对话框高级用法。
/// 本示例展示如何在 QPrintPreviewDialog 的工具栏中添加自定义 Action，
/// 例如"添加水印"开关，以及如何在 paintRequested 回调中绘制内容。

#pragma once

#include <QMainWindow>

class QTextEdit;
class QPrinter;

/// @brief 主窗口，提供文本编辑区和打印预览功能。
class PreviewMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit PreviewMainWindow(QWidget* parent = nullptr);

private slots:
    /// @brief 打开 QPrintPreviewDialog 并添加自定义工具栏操作。
    /// @note 每次打开预览时都重新配置对话框，确保水印状态正确。
    void openPrintPreview();

    /// @brief 响应预览对话框的 paintRequested 信号，渲染内容到打印机。
    /// @param[in] printer 预览对话框传入的打印机对象。
    /// @note 根据 m_watermarkEnabled 决定是否在每页绘制对角线水印。
    void renderPages(QPrinter* printer);

private:
    /// @brief 初始化 UI：文本编辑器和打印预览按钮。
    void setupUI();

    /// @brief 生成用于演示的多页示例文本。
    void populateSampleText();

    /// @brief 在给定页面上绘制"DRAFT"对角线水印。
    /// @param[in] painter 用于绘制的 QPainter 对象。
    /// @param[in] pageRect 页面可绘制区域。
    /// @note 水印使用半透明灰色、大号字体、45 度旋转绘制，
    ///       位于页面正中央，不影响正文可读性。
    void drawWatermark(QPainter& painter, const QRectF& pageRect);

    QTextEdit* m_textEdit;          ///< 文本编辑器，含多页示例内容
    bool m_watermarkEnabled;        ///< 水印开关状态
};
