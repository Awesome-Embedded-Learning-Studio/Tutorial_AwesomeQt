/// @file    main.cpp
/// @brief   事件处理进阶演示程序入口。
///
/// 启动后展示橡皮筋选区控件和事件过滤器日志面板，
/// 演示事件传播链（eventFilter → event() → specific handler）和 grabMouse 机制。
///
/// 对应教程：进阶层 03-QtWidgets/02-事件处理进阶。

#include "event_filter_logger.h"
#include "rubber_band_widget.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QSplitter>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 使用 QSplitter 分割选区控件和日志面板
    auto* splitter = new QSplitter(Qt::Horizontal);

    // 左侧：橡皮筋选区控件
    auto* rubberBand = new RubberBandWidget;

    // 右侧：事件日志面板
    auto* logPanel = new QPlainTextEdit;
    logPanel->setReadOnly(true);
    logPanel->setMaximumBlockCount(200);  // 限制日志行数，避免内存无限增长
    logPanel->setWindowTitle(QStringLiteral("Event Log"));

    splitter->addWidget(rubberBand);
    splitter->addWidget(logPanel);
    splitter->setStretchFactor(0, 3);  // 选区控件占更多空间
    splitter->setStretchFactor(1, 2);  // 日志面板占较少空间

    // 创建事件过滤器并安装到橡皮筋选区控件
    // eventFilter 优先级高于 event() 和 specific handler
    EventFilterLogger logger(logPanel);
    logger.installOn(rubberBand);

    splitter->resize(900, 400);
    splitter->setWindowTitle(QStringLiteral("Event Handling Advanced Demo"));

    splitter->show();

    return app.exec();
}
