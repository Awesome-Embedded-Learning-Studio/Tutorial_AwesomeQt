// QtWidgets 入门示例 40: QTabBar 独立标签栏
// 演示：QTabBar 与 QTabWidget 的区别（可独立使用）
//       自定义标签栏 + 自定义内容区域组合
//       tabCloseRequested 信号实现可关闭标签页
//       setMovable(true) 可拖动标签排序

#include <QApplication>

#include "TabBarDemoWidget.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QTabBar::tab {"
        "  background-color: #F5F5F5;"
        "  border: 1px solid #BDBDBD;"
        "  border-bottom: none;"
        "  border-top-left-radius: 6px;"
        "  border-top-right-radius: 6px;"
        "  padding: 6px 16px;"
        "  margin-right: 2px;"
        "  font-size: 13px;"
        "}"
        "QTabBar::tab:selected {"
        "  background-color: white;"
        "  border-bottom: 2px solid #1976D2;"
        "  font-weight: bold;"
        "  color: #1976D2;"
        "}"
        "QTabBar::tab:hover {"
        "  background-color: #E3F2FD;"
        "}"
        "QSplitter::handle {"
        "  background-color: #E0E0E0;"
        "  width: 3px;"
        "}"
        "QTextEdit {"
        "  border: 1px solid #E0E0E0;"
        "  border-radius: 4px;"
        "  padding: 6px;"
        "  font-family: monospace;"
        "  font-size: 13px;"
        "}"
    );

    TabBarDemoWidget demo;
    demo.show();

    return app.exec();
}
