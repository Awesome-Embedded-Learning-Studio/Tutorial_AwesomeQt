/// @file    frameless_widget.cpp
/// @brief   FramelessWidget 类实现——无边框半透明窗口与手动拖动。
///
/// 对应教程：进阶层 03-QtWidgets/11-QWidget 基类进阶。

#include "frameless_widget.h"

#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPushButton>
#include <QVBoxLayout>

namespace {
/// 圆角半径
constexpr int kBorderRadius = 16;
/// 背景色：85% 不透明度的白色
constexpr int kBgAlpha = 217;  // 255 * 0.85 ≈ 217
}

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

FramelessWidget::FramelessWidget(QWidget* parent)
    : QWidget(parent)
{
    // 去掉系统边框和标题栏
    setWindowFlags(Qt::FramelessWindowHint);

    // WA_TranslucentBackground：让窗口支持半透明
    setAttribute(Qt::WA_TranslucentBackground);
    // WA_NoSystemBackground：阻止系统预填充不透明背景色
    setAttribute(Qt::WA_NoSystemBackground);

    // 界面布局：顶部标题栏 + 中间内容区
    auto* mainLayout = new QVBoxLayout(this);
    // 外边距留出圆角空间
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 标题栏区域（可拖动）
    auto* titleBar = new QWidget;
    auto* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(0, 0, 0, 0);

    auto* titleLabel = new QLabel(QStringLiteral("Frameless Widget Demo"));
    titleLabel->setStyleSheet(QStringLiteral("font-size: 14px; font-weight: bold; color: #333;"));

    auto* closeButton = new QPushButton(QStringLiteral("X"));
    closeButton->setFixedSize(28, 28);
    closeButton->setStyleSheet(
        QStringLiteral("QPushButton { border: none; font-size: 16px; color: #666; }"
                       "QPushButton:hover { color: #e53935; }"));
    QObject::connect(closeButton, &QPushButton::clicked, this, &QWidget::close);

    titleLayout->addWidget(titleLabel, 1);
    titleLayout->addWidget(closeButton);

    // 内容区域
    auto* contentLabel = new QLabel(
        QStringLiteral("这是一个无边框半透明窗口。\n\n"
                       "核心知识点：\n"
                       "1. WA_TranslucentBackground + WA_NoSystemBackground\n"
                       "2. FramelessWindowHint 去除系统边框\n"
                       "3. mousePressEvent + mouseMoveEvent 手动拖动\n"
                       "4. paintEvent 自定义圆角半透明背景\n\n"
                       "拖动标题栏移动窗口，点击 X 关闭。"));
    contentLabel->setWordWrap(true);
    contentLabel->setStyleSheet(QStringLiteral("color: #555; font-size: 13px;"));

    mainLayout->addWidget(titleBar);
    mainLayout->addWidget(contentLabel, 1);

    resize(480, 360);
}

// ─────────────────────────────────────────────────────────────────────────────
// paintEvent——圆角半透明背景
// ─────────────────────────────────────────────────────────────────────────────

void FramelessWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制圆角半透明白色背景
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, kBgAlpha));
    painter.drawRoundedRect(rect(), kBorderRadius, kBorderRadius);

    // 绘制淡灰色边框
    QPen pen(QColor(200, 200, 200, 180), 1);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), kBorderRadius, kBorderRadius);
}

// ─────────────────────────────────────────────────────────────────────────────
// 鼠标拖动
// ─────────────────────────────────────────────────────────────────────────────

void FramelessWidget::mousePressEvent(QMouseEvent* event)
{
    // 只响应左键拖动，且限定在顶部 40px 的标题栏区域
    if (event->button() == Qt::LeftButton && event->position().y() < 40) {
        // 偏移量 = 鼠标全局坐标 - 窗口左上角坐标
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
    }
    QWidget::mousePressEvent(event);
}

void FramelessWidget::mouseMoveEvent(QMouseEvent* event)
{
    // 左键按住时更新窗口位置
    if (event->buttons() & Qt::LeftButton) {
        // 窗口新位置 = 鼠标全局坐标 - 偏移量
        move(event->globalPosition().toPoint() - m_dragPosition);
    }
    QWidget::mouseMoveEvent(event);
}
