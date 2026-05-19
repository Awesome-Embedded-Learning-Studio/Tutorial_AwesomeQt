/// @file    main.cpp
/// @brief   QSlider 进阶演示程序入口。
///
/// 展示 CustomTickSlider 的点击跳转、自定义刻度标注，以及
/// sliderPosition 与 value 的双值机制差异。
///
/// 对应教程：进阶层 03-QtWidgets/31-QSlider 进阶。

#include "custom_tick_slider.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    auto* window = new QWidget;
    auto* mainLayout = new QVBoxLayout(window);

    // ── 标题 ──
    auto* titleLabel = new QLabel(QStringLiteral("QSlider Advanced Demo"));
    mainLayout->addWidget(titleLabel);

    // ── 自定义滑块 ──
    auto* slider = new CustomTickSlider(Qt::Horizontal);
    slider->setRange(0, 100);
    slider->setValue(50);

    mainLayout->addWidget(slider);

    // ── 实时数值显示区 ──
    // 展示 sliderPosition（拖拽中实时）和 value（松开后确定）的差异
    auto* infoLayout = new QHBoxLayout;

    auto* posLabel = new QLabel(QStringLiteral("sliderPosition: 50"));
    auto* valLabel = new QLabel(QStringLiteral("value: 50"));

    infoLayout->addWidget(posLabel);
    infoLayout->addWidget(valLabel);
    mainLayout->addLayout(infoLayout);

    // sliderMoved 携带的是 sliderPosition，在拖拽过程中每帧触发
    QObject::connect(slider, &QSlider::sliderMoved, slider,
                     [posLabel](int pos)
                     { posLabel->setText(QStringLiteral("sliderPosition: %1").arg(pos)); });

    // valueChanged 在松开鼠标后触发（tracking=true 时拖拽中也会触发）
    QObject::connect(slider, &QSlider::valueChanged, slider,
                     [valLabel](int val)
                     { valLabel->setText(QStringLiteral("value: %1").arg(val)); });

    mainLayout->addStretch();

    window->setWindowTitle(QStringLiteral("QSlider Advanced Demo"));
    window->resize(500, 150);
    window->show();

    return app.exec();
}
