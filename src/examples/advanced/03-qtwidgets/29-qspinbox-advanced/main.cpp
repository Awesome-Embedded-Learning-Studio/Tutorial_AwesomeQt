/// @file    main.cpp
/// @brief   UnitSpinBox 演示程序入口。
///
/// 创建一个包含 UnitSpinBox 和信息标签的窗口，展示 textFromValue/valueFromText
/// 的自定义格式化。用户可输入带单位的文本（如 "2.5 MB"），控件自动解析。
///
/// 对应教程：进阶层 03-QtWidgets/29-QSpinBox 进阶。

#include "unit_spin_box.h"

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
    auto* title = new QLabel(QStringLiteral("UnitSpinBox 演示 — textFromValue / valueFromText"));
    title->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14px;"));

    // ── UnitSpinBox ──
    auto* spinBox = new UnitSpinBox;
    spinBox->setValue(512 * 1024);  // 初始值 512 KB

    // ── 信息标签 ──
    auto* displayLabel = new QLabel;
    auto* valueLabel = new QLabel;
    auto* hintLabel = new QLabel(
        QStringLiteral("提示：尝试直接输入带单位的文本，如 \"2.5 MB\"、\"512 KB\"、\"1024 B\"。\n"
                       "也可以直接输入纯数字（视为字节）。上下箭头步进 1 KB。"));
    hintLabel->setWordWrap(true);

    // 值变化时更新标签
    const auto updateLabels = [&]() {
        const int val = spinBox->value();
        displayLabel->setText(QStringLiteral("显示文本: %1").arg(spinBox->text()));
        valueLabel->setText(QStringLiteral("内部值（字节）: %1").arg(val));
    };
    updateLabels();

    // ── 布局 ──
    auto* spinRow = new QHBoxLayout;
    auto* spinLabel = new QLabel(QStringLiteral("Unit SpinBox:"));
    spinRow->addWidget(spinLabel);
    spinRow->addWidget(spinBox, 1);

    mainLayout->addWidget(title);
    mainLayout->addLayout(spinRow);
    mainLayout->addWidget(displayLabel);
    mainLayout->addWidget(valueLabel);
    mainLayout->addWidget(hintLabel);
    mainLayout->addStretch();

    // 连接信号
    QObject::connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), updateLabels);

    window->setWindowTitle(QStringLiteral("UnitSpinBox - QSpinBox Advanced"));
    window->resize(500, 250);
    window->show();

    return app.exec();
}
