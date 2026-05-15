/// @file    main.cpp
/// @brief   程序入口，构建并展示 GlWidget。
///
/// 对应教程：进阶层 02-QtGui/05-OpenGL 集成基础。

#include "gl_widget.h"

#include <QApplication>

auto main(int argc, char* argv[]) -> int
{
    QApplication app(argc, argv);

    GlWidget widget;
    widget.setWindowTitle(QStringLiteral("QOpenGLWidget 着色器演示"));
    widget.resize(640, 480);
    widget.show();

    return app.exec();
}
