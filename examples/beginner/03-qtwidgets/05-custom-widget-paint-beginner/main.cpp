// QtWidgets 入门示例 05: 自定义绘制 Widget 基础
// 演示：paintEvent 完整流程 / update() vs repaint()
//       双缓冲防闪烁 / sizeHint + minimumSizeHint 配合布局系统
//       自定义圆形仪表盘控件 + QPropertyAnimation 平滑动画

#include <QApplication>

#include "mainwindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QGroupBox {"
        "  font-weight: bold;"
        "  border: 1px solid #DDD;"
        "  border-radius: 6px;"
        "  margin-top: 10px;"
        "  padding-top: 16px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 12px;"
        "  padding: 0 4px;"
        "}"
        "QPushButton {"
        "  padding: 6px 18px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  background-color: #FAFAFA;"
        "}"
        "QPushButton:hover { background-color: #E8E8E8; }"
        "QPushButton:pressed { background-color: #DDD; }"
        "QSlider::groove:horizontal {"
        "  height: 6px;"
        "  background: #E0E0E0;"
        "  border-radius: 3px;"
        "}"
        "QSlider::handle:horizontal {"
        "  width: 16px;"
        "  height: 16px;"
        "  margin: -5px 0;"
        "  background: #3498DB;"
        "  border-radius: 8px;"
        "}"
        "QSlider::handle:horizontal:hover { background: #2980B9; }"
        "QSlider::sub-page:horizontal {"
        "  background: #3498DB;"
        "  border-radius: 3px;"
        "}"
    );

    MainWindow window;
    window.show();

    return app.exec();
}
