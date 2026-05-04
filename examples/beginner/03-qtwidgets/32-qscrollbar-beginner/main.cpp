// QtWidgets 入门示例 32: QScrollBar 滚动条
// 演示：独立使用滚动条驱动自定义控件
//       setRange / setPageStep / setSingleStep 配置
//       valueChanged 驱动内容偏移
//       QPainter::translate 偏移绘制 + 可见区域裁剪

#include <QApplication>

#include "TimelineBrowser.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TimelineBrowser browser;
    browser.show();

    return app.exec();
}
