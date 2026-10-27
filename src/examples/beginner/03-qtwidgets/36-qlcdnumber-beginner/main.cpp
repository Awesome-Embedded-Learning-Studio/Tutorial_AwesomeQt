// QtWidgets 入门示例 36: QLCDNumber 液晶数字显示
// 演示：display(int/double/QString) 三种显示方式
//       setDigitCount 位数控制 + setSmallDecimalPoint 小数点策略
//       setMode 十进制/十六进制/八进制/二进制切换
//       QTimer + QLCDNumber 仪表盘计时器

#include <QApplication>

#include "lcddemowidget.h"

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
        "  border-radius: 4px;"
        "  margin-top: 8px;"
        "  padding-top: 16px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 12px;"
        "  padding: 0 4px;"
        "}"
        "QLCDNumber {"
        "  background-color: #1a1a2e;"
        "  color: #00ff88;"
        "  border: 2px solid #333;"
        "  border-radius: 6px;"
        "}"
    );

    LcdDemoWidget demo;
    demo.show();

    return app.exec();
}
