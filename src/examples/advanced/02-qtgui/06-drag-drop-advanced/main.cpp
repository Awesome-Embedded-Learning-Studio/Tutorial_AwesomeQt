/// @file    main.cpp
/// @brief   程序入口，展示拖拽源列表与放下目标列表并排布局。
///
/// 对应教程：进阶层 02-QtGui/06-拖放系统高级用法。

#include "drag_list_widget.h"
#include "drop_list_widget.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

auto main(int argc, char* argv[]) -> int
{
    QApplication app(argc, argv);

    // 主窗口
    auto* window = new QWidget;
    window->setWindowTitle(QStringLiteral("自定义 MIME 拖放演示"));

    auto* mainLayout = new QHBoxLayout(window);

    // 左侧：拖拽源
    auto* leftLayout = new QVBoxLayout;
    leftLayout->addWidget(new QLabel(QStringLiteral("拖拽源（从此拖出）:")));
    auto* dragList = new DragListWidget;
    leftLayout->addWidget(dragList);

    // 右侧：放下目标
    auto* rightLayout = new QVBoxLayout;
    rightLayout->addWidget(new QLabel(QStringLiteral("放下目标（拖入此处）:")));
    auto* dropList = new DropListWidget;
    rightLayout->addWidget(dropList);

    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);

    window->resize(640, 400);
    window->show();

    return app.exec();
}
