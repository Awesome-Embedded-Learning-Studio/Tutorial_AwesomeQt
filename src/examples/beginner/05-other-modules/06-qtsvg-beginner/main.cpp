#include <QApplication>

#include "svgviewerwindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    SvgViewerWindow window;
    window.show();

    return app.exec();
}
