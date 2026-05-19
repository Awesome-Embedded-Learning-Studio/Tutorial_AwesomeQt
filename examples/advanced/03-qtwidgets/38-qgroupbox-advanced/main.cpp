/// @file    main.cpp
/// @brief   QGroupBox 进阶演示程序入口。
///
/// 展示 CheckableGroupBox 的自定义绘制、checkable 禁用级联和 changeEvent 拦截。
///
/// 对应教程：进阶层 03-QtWidgets/38-QGroupBox 进阶。

#include "checkable_group_box.h"

#include <QApplication>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 主窗口使用 QScrollArea 包裹，确保窗口 resize 时内容可滚动
    auto* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);

    auto* container = new QWidget;
    auto* mainLayout = new QVBoxLayout(container);

    // 标题说明
    auto* title = new QLabel(QStringLiteral(
        "QGroupBox 进阶演示\n"
        "勾选/取消勾选「代理设置」观察：\n"
        "1. 子控件自动禁用/启用\n"
        "2. 状态标签实时更新\n"
        "3. 再次勾选时输入内容保留"));
    title->setWordWrap(true);
    mainLayout->addWidget(title);

    // 核心：自定义 CheckableGroupBox
    auto* groupBox = new CheckableGroupBox;
    mainLayout->addWidget(groupBox);

    // 添加一个外部控件用于对比——取消勾选分组框后可以 Tab 到这里
    auto* outsideLabel = new QLabel(
        QStringLiteral("我是分组框外部的控件，用于验证焦点转移。"));
    outsideLabel->setFrameShape(QFrame::Box);
    outsideLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(outsideLabel);

    mainLayout->addStretch();

    scrollArea->setWidget(container);
    scrollArea->setWindowTitle(QStringLiteral("QGroupBox Advanced Demo"));
    scrollArea->resize(450, 400);
    scrollArea->show();

    return app.exec();
}
