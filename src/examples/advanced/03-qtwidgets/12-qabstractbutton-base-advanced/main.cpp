/// @file    main.cpp
/// @brief   三态按钮演示程序入口。
///
/// 展示 3 个 TriStateButton 实例，点击后循环切换
/// Unchecked -> PartiallyChecked -> Checked 三态，并通过 QLabel 实时显示每个按钮的状态。
///
/// 对应教程：进阶层 03-QtWidgets/12-QAbstractButton 基类进阶。

#include "tri_state_button.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

/// @brief 将 Qt::CheckState 转为可读字符串。
/// @param[in] state 三态枚举值。
/// @return 可读状态文本。
static QString stateToString(Qt::CheckState state)
{
    switch (state) {
    case Qt::Unchecked:
        return QStringLiteral("Unchecked");
    case Qt::PartiallyChecked:
        return QStringLiteral("PartiallyChecked");
    case Qt::Checked:
        return QStringLiteral("Checked");
    }
    return QStringLiteral("Unknown");
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    auto* window = new QWidget;
    auto* mainLayout = new QVBoxLayout(window);

    // 标题
    auto* title = new QLabel(QStringLiteral("QAbstractButton 三态按钮演示"));
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: bold;"));
    mainLayout->addWidget(title);

    // 三个按钮 + 状态标签
    auto* buttonsRow = new QHBoxLayout;

    const QStringList names = {QStringLiteral("选项 A"), QStringLiteral("选项 B"),
                               QStringLiteral("选项 C")};

    // 每个按钮下方各一个状态标签
    QLabel* stateLabels[3];
    TriStateButton* buttons[3];

    for (int i = 0; i < 3; ++i) {
        auto* col = new QVBoxLayout;

        buttons[i] = new TriStateButton(names[i]);
        stateLabels[i] = new QLabel(QStringLiteral("状态: Unchecked"));
        stateLabels[i]->setAlignment(Qt::AlignCenter);

        col->addWidget(buttons[i], 0, Qt::AlignCenter);
        col->addWidget(stateLabels[i]);
        buttonsRow->addLayout(col);
    }

    mainLayout->addLayout(buttonsRow);

    // 底部汇总标签
    auto* summary = new QLabel;
    summary->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(summary);

    // 用 lambda 捕获数组指针，连接 stateChanged 信号更新对应标签
    for (int i = 0; i < 3; ++i) {
        QObject::connect(buttons[i], &TriStateButton::stateChanged, stateLabels[i],
                         [stateLabels, buttons, summary, i](int state) {
                             stateLabels[i]->setText(
                                 QStringLiteral("状态: %1").arg(stateToString(static_cast<Qt::CheckState>(state))));

                             // 汇总：统计三种状态各有多少按钮
                             int unchecked = 0;
                             int partial = 0;
                             int checked = 0;
                             for (int j = 0; j < 3; ++j) {
                                 switch (buttons[j]->checkState()) {
                                 case Qt::Unchecked:
                                     ++unchecked;
                                     break;
                                 case Qt::PartiallyChecked:
                                     ++partial;
                                     break;
                                 case Qt::Checked:
                                     ++checked;
                                     break;
                                 }
                             }
                             summary->setText(
                                 QStringLiteral("汇总 — Unchecked: %1 | PartiallyChecked: %2 | Checked: %3")
                                     .arg(unchecked)
                                     .arg(partial)
                                     .arg(checked));
                         });
    }

    window->setWindowTitle(QStringLiteral("TriStateButton Demo"));
    window->resize(500, 300);
    window->show();

    return app.exec();
}
