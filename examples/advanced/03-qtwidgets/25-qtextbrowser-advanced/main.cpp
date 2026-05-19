/// @file    main.cpp
/// @brief   QTextBrowser 进阶演示程序入口。
///
/// 启动 CustomResourceBrowser 窗口，展示自定义协议资源加载、
/// 历史栈安全导航与 anchorClicked 手动链接处理。
///
/// 对应教程：进阶层 03-QtWidgets/25-QTextBrowser 进阶。

#include "custom_resource_browser.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 使用顶层 QWidget 组合导航栏和浏览器
    auto* window = new QWidget;
    auto* mainLayout = new QVBoxLayout(window);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 创建浏览器实例
    auto* browser = new CustomResourceBrowser;

    // 创建导航工具栏（按钮和标签由浏览器内部控制逻辑）
    auto* navBar = browser->createNavigationToolBar();

    // 组装布局：导航栏 + 浏览器
    mainLayout->addWidget(navBar);
    mainLayout->addWidget(browser, 1);

    window->setWindowTitle(QStringLiteral("QTextBrowser Advanced - Custom Resource Browser"));
    window->resize(700, 500);
    window->show();

    return app.exec();
}
