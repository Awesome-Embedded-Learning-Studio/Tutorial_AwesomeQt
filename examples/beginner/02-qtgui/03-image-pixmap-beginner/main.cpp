// QtGui 入门示例 03: QImage、QPixmap、QIcon 图像处理基础
// 演示：QImage 像素操作、QPixmap 显示与缩放、QIcon 动态生成、简易图片查看器

#include <QApplication>

#include "pixeldemowidget.h"
#include "simpleimageviewer.h"

// ============================================================================
// 主函数：创建两个演示窗口
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 演示 1: QImage 像素操作
    PixelDemoWidget pixelDemo;
    pixelDemo.show();

    // 演示 2: 简易图片查看器
    SimpleImageViewer viewer;
    viewer.show();

    return app.exec();
}
