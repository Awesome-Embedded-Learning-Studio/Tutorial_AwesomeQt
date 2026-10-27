/// @file    main.cpp
/// @brief   自定义控件进阶演示程序入口。
///
/// 启动一个包含 StyledGauge 和 QSlider 的窗口，展示 QStylePainter 风格感知绘制
/// 以及 heightForWidth 宽高比约束的实际效果。
///
/// 对应教程：进阶层 03-QtWidgets/05-自定义控件进阶。

#include "styled_gauge.h"

#include <QApplication>
#include <QBoxLayout>
#include <QLabel>
#include <QSlider>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    auto* window = new QWidget;
    auto* layout = new QVBoxLayout(window);

    // 标题说明
    auto* title = new QLabel(
        QStringLiteral("StyledGauge —— QStylePainter + heightForWidth 演示"));
    layout->addWidget(title);

    // 环形仪表盘控件，heightForWidth 保证它始终是正方形
    auto* gauge = new StyledGauge;
    layout->addWidget(gauge, 1);

    // 滑块控制进度值
    auto* sliderLabel = new QLabel(QStringLiteral("拖动滑块调整进度:"));
    layout->addWidget(sliderLabel);

    auto* slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 100);
    slider->setValue(0);
    layout->addWidget(slider);

    // 滑块值变化 → 仪表盘进度
    QObject::connect(slider, &QSlider::valueChanged, gauge, &StyledGauge::setProgress);

    // 进度变化时同步窗口标题，方便观察
    QObject::connect(gauge, &StyledGauge::progressChanged, [&](int value) {
        window->setWindowTitle(
            QStringLiteral("StyledGauge — 进度: %1%").arg(value));
    });

    window->resize(300, 400);
    window->show();

    return app.exec();
}
