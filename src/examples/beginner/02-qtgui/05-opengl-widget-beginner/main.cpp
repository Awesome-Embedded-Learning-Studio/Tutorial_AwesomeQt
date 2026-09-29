// QtGui 入门示例 05: QOpenGLWidget 嵌入 OpenGL 基础
// 演示：OpenGL 三角形绘制、着色器编译链接、VAO/VBO 使用、旋转动画

#include <QApplication>
#include <QSurfaceFormat>

#include "mainwindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    // 设置 OpenGL 表面格式（请求 3.3 Core Profile）
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setSamples(4);  // 4x MSAA 抗锯齿
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
