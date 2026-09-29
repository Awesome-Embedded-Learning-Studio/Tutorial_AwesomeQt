// QtWidgets 入门示例 14: QAbstractScrollArea 滚动区域基类
// 演示：horizontalScrollBar() / verticalScrollBar() 滚动条控制
//       setHorizontalScrollBarPolicy / setVerticalScrollBarPolicy
//       scrollContentsBy() 内容滚动响应
//       视口（viewport）概念与绘制注意事项

#include <QApplication>

#include "demo_window.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QPushButton {"
        "  padding: 6px 14px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  background-color: #FFF;"
        "  font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #E8E8E8;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #DDD;"
        "}"
        "QGroupBox {"
        "  font-weight: bold;"
        "  border: 1px solid #DDD;"
        "  border-radius: 4px;"
        "  margin-top: 8px;"
        "  padding-top: 16px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 12px;"
        "  padding: 0 4px;"
        "}"
    );

    DemoWindow window;
    window.show();

    return app.exec();
}
