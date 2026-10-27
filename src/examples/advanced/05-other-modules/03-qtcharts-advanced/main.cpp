/// @file    main.cpp
/// @brief   程序入口，构建主窗口并展示 RealtimeChart 实时数据可视化。
///
/// 对应教程：进阶层 05-其他模块/03-QtCharts 进阶。

#include "realtime_chart.h"

#include <QApplication>
#include <QMainWindow>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle(QStringLiteral("QtCharts 进阶：实时数据"));
    window.resize(800, 500);

    // RealtimeChart 作为中央控件，由 QMainWindow 管理生命周期
    auto* chart = new RealtimeChart(&window);
    window.setCentralWidget(chart);

    // 窗口显示后立即启动数据采集
    // @note 使用 QWidget::showEvent 更稳健，但此处为了简洁在 show() 后直接调用
    window.show();
    chart->start();

    return app.exec();
}
