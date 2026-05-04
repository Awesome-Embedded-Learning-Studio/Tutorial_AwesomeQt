// QtWidgets 入门示例 31: QSlider 滑动条
// 演示：水平/垂直方向
//       setRange / setValue / setSingleStep / setPageStep
//       valueChanged / sliderMoved / sliderReleased 信号区别
//       QSS 自定义滑块外观（handle / groove）

#include <QApplication>

#include "color_palette_panel.h"

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
        "}");

    ColorPalettePanel panel;
    panel.show();

    return app.exec();
}
