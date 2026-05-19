/// @file    main.cpp
/// @brief   十六进制微调框演示程序入口。
///
/// 创建一个包含 HexSpinBox 和信息标签的窗口，展示 validate/fixup/stepBy
/// 三个核心虚函数的协作。下方标签实时显示当前值的十六进制和十进制表示。
///
/// 对应教程：进阶层 03-QtWidgets/16-QAbstractSpinBox 基类进阶。

#include "hex_spin_box.h"

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
    auto* title = new QLabel(QStringLiteral("HexSpinBox 演示 — validate / fixup / stepBy"));
    title->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14px;"));

    // ── HexSpinBox ──
    auto* spinBox = new HexSpinBox;
    spinBox->setRange(0x00000000, 0x00FFFFFF);
    spinBox->setHexValue(0x00FF00A0);

    // ── 信息标签：实时显示十六进制和十进制值 ──
    auto* hexLabel = new QLabel;
    auto* decLabel = new QLabel;
    auto* stepHintLabel = new QLabel(
        QStringLiteral("提示：点击输入框中不同数位位置，上下箭头的步进幅度会变化。\n"
                       "光标在个位 → ±0x1，在十位 → ±0x10，在百位 → ±0x100，以此类推。"));
    stepHintLabel->setWordWrap(true);

    // 值变化时更新标签
    const auto updateLabels = [&](quintptr value) {
        hexLabel->setText(QStringLiteral("十六进制: 0x%1")
                              .arg(value, 8, 16, QLatin1Char('0')).toUpper());
        decLabel->setText(QStringLiteral("十进制: %1").arg(value));
    };
    updateLabels(spinBox->hexValue());

    // ── 布局 ──
    auto* spinRow = new QHBoxLayout;
    auto* spinLabel = new QLabel(QStringLiteral("Hex SpinBox:"));
    spinRow->addWidget(spinLabel);
    spinRow->addWidget(spinBox, 1);

    mainLayout->addWidget(title);
    mainLayout->addLayout(spinRow);
    mainLayout->addWidget(hexLabel);
    mainLayout->addWidget(decLabel);
    mainLayout->addWidget(stepHintLabel);
    mainLayout->addStretch();

    // 连接信号
    QObject::connect(spinBox, &HexSpinBox::hexValueChanged, updateLabels);

    window->setWindowTitle(QStringLiteral("HexSpinBox - QAbstractSpinBox Advanced"));
    window->resize(500, 250);
    window->show();

    return app.exec();
}
