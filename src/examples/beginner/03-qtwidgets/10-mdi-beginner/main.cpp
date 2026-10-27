// QtWidgets 入门示例 10: QMdiArea 多文档界面基础
// 演示：QMdiArea 子窗口创建与管理
//       QMdiSubWindow 标题、图标与关闭行为
//       子窗口排列模式（级联 / 平铺）
//       激活子窗口与信号监听
//       简易多文档文本编辑器

#include <QApplication>

#include "mdimainwindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QMainWindow { background-color: #FAFAFA; }"
        "QMdiArea {"
        "  background-color: #F0F0F0;"
        "  border: 1px solid #DDD;"
        "}"
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
        "QTextEdit {"
        "  border: 1px solid #E0E0E0;"
        "  padding: 8px;"
        "  font-size: 13px;"
        "}"
        "QMdiSubWindow {"
        "  border: 1px solid #BBB;"
        "}"
    );

    MdiMainWindow window;
    window.show();

    return app.exec();
}
