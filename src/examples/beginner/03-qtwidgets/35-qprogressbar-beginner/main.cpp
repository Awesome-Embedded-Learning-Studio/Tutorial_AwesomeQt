// QtWidgets 入门示例 35: QProgressBar 进度条
// 演示：setRange + setValue 更新进度
//       无限进度 setRange(0, 0) 滚动动画
//       setFormat 自定义显示文字
//       跨线程安全更新进度条（Worker + moveToThread）

#include <QApplication>

#include "ProgressDemoWidget.h"

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
        "QProgressBar {"
        "  border: 1px solid #E0E0E0;"
        "  border-radius: 4px;"
        "  background-color: #F5F5F5;"
        "  text-align: center;"
        "  color: #333;"
        "}"
        "QProgressBar::chunk {"
        "  background-color: #1976D2;"
        "  border-radius: 3px;"
        "}"
    );

    ProgressDemoWidget demo;
    demo.show();

    return app.exec();
}
