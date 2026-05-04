// QtWidgets 入门示例 08: 图形视图框架基础
// 演示：QGraphicsScene / QGraphicsView / QGraphicsItem 三角关系
//       标准图元：矩形、椭圆、文字
//       场景坐标 vs 视图坐标 vs 图元坐标转换
//       鼠标事件在 Scene 层拦截与响应
//       简易图形画板

#include <QApplication>
#include "mainwindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QToolBar {"
        "  spacing: 6px;"
        "  padding: 4px;"
        "  border-bottom: 1px solid #DDD;"
        "}"
        "QToolBar QToolButton {"
        "  padding: 5px 12px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  background-color: #FAFAFA;"
        "}"
        "QToolBar QToolButton:hover {"
        "  background-color: #E8E8E8;"
        "}"
        "QToolBar QToolButton:pressed {"
        "  background-color: #DDD;"
        "}"
        "QStatusBar {"
        "  border-top: 1px solid #DDD;"
        "}"
        "QGraphicsView {"
        "  border: none;"
        "  background-color: #FAFAFA;"
        "}"
    );

    MainWindow window;
    window.show();

    return app.exec();
}
