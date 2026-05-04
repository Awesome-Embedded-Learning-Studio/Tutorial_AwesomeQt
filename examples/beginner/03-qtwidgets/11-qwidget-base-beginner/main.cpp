// QtWidgets 入门示例 11: QWidget 基类基础
// 演示：窗口属性 resize / move / setWindowTitle / setWindowIcon
//       show() / hide() / setVisible() / raise() / lower()
//       尺寸策略 setSizePolicy / setFixedSize / setMinimumSize
//       窗口标志 Qt::WindowFlags（无边框、置顶、工具窗口）

#include <QApplication>

#include "DraggableWidget.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QPushButton {"
        "  padding: 7px 16px;"
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

    DraggableWidget window;
    window.show();

    return app.exec();
}
