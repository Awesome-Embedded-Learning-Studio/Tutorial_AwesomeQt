// QtWidgets 入门示例 74: QPrintPreviewDialog 打印预览
// 演示：paintRequested 信号中执行绘制
//       翻页与缩放操作（内置）
//       自定义预览工具栏（添加页面设置快捷按钮）
//       QPageSetupDialog 页面参数配置

#include <QMainWindow>

#include <QMarginsF>
#include <QPageLayout>
#include <QPageSize>

class QTextEdit;
class QLabel;
class QPrinter;
class QPainter;

// ============================================================================
// MainWindow: 打印预览 + 页面设置 + 打印 三合一演示
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    // ====================================================================
    // 页面设置: QPageSetupDialog
    // ====================================================================
    void onPageSetup();

    // ====================================================================
    // 打印预览: QPrintPreviewDialog + 自定义工具栏
    // ====================================================================
    void onPrintPreview();

    // ====================================================================
    // 直接打印: QPrintDialog
    // ====================================================================
    void onDirectPrint();

    // ====================================================================
    // 核心绘制函数——预览和打印共用
    // ====================================================================
    void paintDocument(QPrinter *printer);

    // ====================================================================
    // 绘制页脚: 页码居中
    // ====================================================================
    void drawPageFooter(QPainter *painter, int pageNum,
                        const QRectF &pageRect);

    // ====================================================================
    // 更新页面配置信息标签
    // ====================================================================
    void updateInfoLabel();

    // ====================================================================
    // 填充示例文本
    // ====================================================================
    void fillSampleText();

    // ---- 成员变量 ----
    QTextEdit *m_textEdit = nullptr;
    QLabel *m_infoLabel = nullptr;

    // 页面配置（可被 QPageSetupDialog 修改）
    QPageSize::PageSizeId m_pageSize = QPageSize::A4;
    QPageLayout::Orientation m_orientation =
        QPageLayout::Portrait;
    QMarginsF m_margins{20, 20, 20, 20};
};
