/// @file    main.cpp
/// @brief   阴影卡片演示程序入口。
///
/// 展示 3 个 ShadowCard 实例，分别演示手动 QPainter 阴影和
/// QGraphicsDropShadowEffect 两种阴影配置的效果差异。
///
/// 对应教程：进阶层 03-QtWidgets/13-QFrame 基类进阶。

#include "shadow_card.h"

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

    // 标题
    auto* title = new QLabel(QStringLiteral("QFrame 阴影卡片演示"));
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: bold;"));
    mainLayout->addWidget(title);

    // 副标题——手动阴影
    auto* manualLabel = new QLabel(QStringLiteral("手动 QPainter 阴影（2 张卡片）："));
    manualLabel->setStyleSheet(QStringLiteral("font-weight: bold; color: #555;"));
    mainLayout->addWidget(manualLabel);

    auto* manualRow = new QHBoxLayout;
    // 布局留足 margin，防止阴影被裁剪
    manualRow->setContentsMargins(30, 10, 30, 10);

    auto* card1 = new ShadowCard(
        QStringLiteral("卡片 A - 手动阴影"),
        QStringLiteral("使用 QPainter 在 paintEvent 中逐层绘制半透明阴影。"
                       "无需 QGraphicsDropShadowEffect，性能更优。"),
        ShadowCard::ShadowMode::Manual);
    auto* card2 = new ShadowCard(
        QStringLiteral("卡片 B - 手动阴影"),
        QStringLiteral("与卡片 A 相同的阴影技术，演示多卡片共存时的视觉效果。"
                       "注意父布局 margin 留足阴影空间。"),
        ShadowCard::ShadowMode::Manual);

    manualRow->addWidget(card1);
    manualRow->addWidget(card2);
    manualRow->addStretch();
    mainLayout->addLayout(manualRow);

    // 副标题——DropShadowEffect
    auto* effectLabel = new QLabel(QStringLiteral("QGraphicsDropShadowEffect 阴影（1 张卡片）："));
    effectLabel->setStyleSheet(QStringLiteral("font-weight: bold; color: #555;"));
    mainLayout->addWidget(effectLabel);

    auto* effectRow = new QHBoxLayout;
    effectRow->setContentsMargins(30, 10, 30, 10);

    auto* card3 = new ShadowCard(
        QStringLiteral("卡片 C - DropShadow"),
        QStringLiteral("使用 QGraphicsDropShadowEffect 自动阴影。"
                       "配置 StyledPanel 框架 + blurRadius=20 + offset=(0,6)。"),
        ShadowCard::ShadowMode::DropEffect);

    effectRow->addWidget(card3);
    effectRow->addStretch();
    mainLayout->addLayout(effectRow);

    mainLayout->addStretch();

    window->setWindowTitle(QStringLiteral("ShadowCard Demo"));
    window->resize(900, 500);
    window->show();

    return app.exec();
}
