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

#include "pdfviewer.h"

#include <QApplication>

#include <QDebug>

// ========================================
// 主函数
// ========================================

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "QtPdf PDF 渲染基础示例";
    qDebug() << "本示例演示 QPdfDocument + QPdfPageRenderer + QPdfView";

    PdfViewer viewer;
    viewer.show();

    return app.exec();
}
