// QtWidgets 入门示例 09: 属性动画框架基础
// 演示：QPropertyAnimation 对 Q_PROPERTY 属性做动画
//       setStartValue / setEndValue / setDuration 基础配置
//       QEasingCurve 缓动函数效果对比
//       QSequentialAnimationGroup 串行动画组合
//       自定义 ColorWidget 的 backgroundColor 属性动画

#include <QApplication>

#include "animationdemowindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QMainWindow { background-color: #FAFAFA; }"
        "QPushButton {"
        "  padding: 8px 20px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  background-color: #FFF;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #E8E8E8;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #DDD;"
        "}"
        "QComboBox {"
        "  padding: 5px 10px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  min-width: 150px;"
        "}"
    );

    AnimationDemoWindow window;
    window.show();

    return app.exec();
}
