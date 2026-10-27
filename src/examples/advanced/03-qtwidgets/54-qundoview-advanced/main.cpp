/// @file    main.cpp
/// @brief   程序入口，构建并展示 UndoEditorWindow。
///
/// 对应教程：进阶层 03-QtWidgets/54-QUndoView 完整撤销重做系统。

#include "undo_editor.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    UndoEditorWindow window;
    window.show();

    return app.exec();
}
