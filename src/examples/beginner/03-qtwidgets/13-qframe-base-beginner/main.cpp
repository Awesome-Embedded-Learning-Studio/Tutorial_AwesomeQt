// QtWidgets 入门示例 13: QFrame 可视框架基类
// 演示：QFrame::Shape（Box / Panel / StyledPanel / HLine / VLine）
//       QFrame::Shadow（Raised / Sunken / Plain）
//       lineWidth / midLineWidth 边框宽度控制

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
        "QSpinBox {"
        "  padding: 4px 8px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  font-size: 12px;"
        "}"
        "QLabel {"
        "  font-size: 12px;"
        "}"
    );

    DemoWindow window;
    window.show();

    return app.exec();
}
