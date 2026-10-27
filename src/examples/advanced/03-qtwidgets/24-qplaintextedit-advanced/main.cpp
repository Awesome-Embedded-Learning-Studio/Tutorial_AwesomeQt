/// @file    main.cpp
/// @brief   QPlainTextEdit 进阶演示程序入口。
///
/// 启动 LineNumberEditor 窗口，展示基于 block 布局模型的
/// 行号侧边栏、blockCountChanged/updateRequest 信号联动。
///
/// 对应教程：进阶层 03-QtWidgets/24-QPlainTextEdit 进阶。

#include "line_number_editor.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 设置等宽字体，使行号和代码文本对齐更整齐
    QFont editorFont(QStringLiteral("Monospace"), 11);
    editorFont.setStyleHint(QFont::Monospace);

    LineNumberEditor editor;
    editor.setFont(editorFont);

    // 预填充一些示例文本
    editor.setPlainText(
        QStringLiteral("#include <QApplication>\n"
                       "#include <QPlainTextEdit>\n"
                       "\n"
                       "int main(int argc, char *argv[])\n"
                       "{\n"
                       "    QApplication app(argc, argv);\n"
                       "\n"
                       "    QPlainTextEdit editor;\n"
                       "    editor.setPlainText(\"Hello, Qt!\");\n"
                       "    editor.show();\n"
                       "\n"
                       "    return app.exec();\n"
                       "}\n"));

    editor.setWindowTitle(QStringLiteral("Line Number Editor Demo"));
    editor.resize(600, 400);
    editor.show();

    return app.exec();
}
