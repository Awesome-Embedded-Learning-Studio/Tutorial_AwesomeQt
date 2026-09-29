/**
 * QtPdf PDF 渲染基础示例
 *
 * 本示例演示 QtPdf 模块的三个核心组件：
 * - QPdfDocument：加载 PDF 文件，查询页数和元信息
 * - QPdfPageRenderer：将指定页面渲染为 QImage
 * - QPdfView：开箱即用的 PDF 显示控件，支持多页滚动和缩放
 *
 * 工具栏提供：打开文件、上一页/下一页、放大/缩小
 */

#include <QWidget>

class QPdfDocument;
class QPdfView;
class QLabel;
class QVBoxLayout;

// ========================================
// PDF 查看器主窗口
// ========================================

class PdfViewer : public QWidget
{
    Q_OBJECT

public:
    explicit PdfViewer(QWidget *parent = nullptr);

private:

    void setupToolbar(QVBoxLayout *mainLayout);
    void setupPdfView(QVBoxLayout *mainLayout);

private slots:

    void openFile();
    void prevPage();
    void nextPage();
    void zoomIn();
    void zoomOut();

private:

    void updatePageLabel();
    void updateZoomLabel();

    QPdfView *m_pdfView = nullptr;
    QPdfDocument *m_pdfDoc = nullptr;
    QLabel *m_pageLabel = nullptr;
    QLabel *m_zoomLabel = nullptr;
    int m_currentPage = 0;
    double m_zoomFactor = 1.0;
};
