/// @file    main.cpp
/// @brief   程序入口，构建并展示 RichTextWidget。
///
/// 对应教程：进阶层 02-QtGui/04-字体与文本高级用法。

#include "rich_text_widget.h"

#include <QApplication>

auto main(int argc, char* argv[]) -> int
{
    QApplication app(argc, argv);

    RichTextWidget widget;
    widget.setWindowTitle(QStringLiteral("QTextDocument 富文本演示"));
    widget.resize(640, 480);
    widget.show();

    return app.exec();
}
