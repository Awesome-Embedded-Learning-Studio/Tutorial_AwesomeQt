// QtWidgets 入门示例 72: QPrinter 打印机抽象类
// 演示：页面配置 (setPageSize/setOrientation/setPageMargins)
//       QPainter + QPrinter 自定义打印内容
//       QPrintPreviewDialog 打印预览
//       导出 PDF (QPrinter::PdfFormat)

#include <QMainWindow>

class QComboBox;
class QTextEdit;
class QLabel;
class QPrinter;

// ============================================================================
// MainWindow: 模拟简单文档编辑器 + 打印/导出功能
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    // ====================================================================
    // 配置 QPrinter 公共参数
    // ====================================================================
    void configurePrinter(QPrinter *printer);

    // ====================================================================
    // 打印预览
    // ====================================================================
    void onPrintPreview();

    // ====================================================================
    // 导出 PDF
    // ====================================================================
    void onExportPdf();

    // ====================================================================
    // 核心绘制函数——打印和导出 PDF 共用
    // ====================================================================
    void paintDocument(QPrinter *printer);

    // ====================================================================
    // 绘制页码（页脚居中）
    // ====================================================================
    void drawPageNumber(QPainter *painter, QPrinter *printer,
                        int pageNum);

    // ---- 成员变量 ----
    QComboBox *m_pageSizeCombo = nullptr;
    QComboBox *m_orientationCombo = nullptr;
    QTextEdit *m_textEdit = nullptr;
    QLabel *m_statusLabel = nullptr;
};
