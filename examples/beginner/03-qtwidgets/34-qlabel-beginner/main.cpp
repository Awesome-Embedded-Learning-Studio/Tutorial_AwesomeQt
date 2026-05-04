// QtWidgets 入门示例 34: QLabel 文本与图像显示
// 演示：显示文本/HTML 富文本/图片
//       setAlignment 对齐与 setWordWrap 自动换行
//       setBuddy 关联快捷键到伙伴控件
//       linkActivated 信号处理超链接点击

#include <QApplication>

#include "labeldemowidget.h"

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
        "QLineEdit, QSpinBox {"
        "  padding: 4px 8px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "}"
    );

    LabelDemoWidget demo;
    demo.show();

    return app.exec();
}
