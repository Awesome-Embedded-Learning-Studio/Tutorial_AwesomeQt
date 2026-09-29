/// @file    shadow_card.cpp
/// @brief   ShadowCard 类实现——QFrame 卡片控件的手动阴影与 DropShadowEffect。
///
/// 对应教程：进阶层 03-QtWidgets/13-QFrame 基类进阶。

#include "shadow_card.h"

#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QVBoxLayout>

namespace {
/// 圆角半径
constexpr int kBorderRadius = 12;
/// 手动阴影模糊半径（像素）
constexpr int kShadowBlur = 18;
/// 手动阴影偏移量
constexpr int kShadowOffsetY = 6;
}

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

ShadowCard::ShadowCard(const QString& title, const QString& desc,
                       ShadowMode mode, QWidget* parent)
    : QFrame(parent)
    , m_shadowMode(mode)
{
    if (m_shadowMode == ShadowMode::Manual) {
        // 手动阴影模式：设为 NoFrame，paintEvent 里自己画背景和阴影
        setFrameShape(QFrame::NoFrame);
    } else {
        // DropShadowEffect 模式：用 StyledPanel，QStyle 画边框
        setFrameShape(QFrame::StyledPanel);
        setupDropShadow();
    }

    setupContent(title, desc);
    setFixedSize(260, 160);
}

// ─────────────────────────────────────────────────────────────────────────────
// 界面初始化
// ─────────────────────────────────────────────────────────────────────────────

ShadowCard::ShadowMode ShadowCard::shadowMode() const
{
    return m_shadowMode;
}

void ShadowCard::setupContent(const QString& title, const QString& desc)
{
    // 手动阴影模式需要留出阴影空间作为内边距
    auto* layout = new QVBoxLayout(this);
    if (m_shadowMode == ShadowMode::Manual) {
        layout->setContentsMargins(kShadowBlur, kShadowBlur,
                                   kShadowBlur, kShadowBlur + kShadowOffsetY);
    } else {
        layout->setContentsMargins(16, 16, 16, 16);
    }

    auto* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 14px; font-weight: bold; color: #333;"));

    auto* descLabel = new QLabel(desc);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet(QStringLiteral("font-size: 12px; color: #666;"));

    layout->addWidget(titleLabel);
    layout->addWidget(descLabel, 1);
}

void ShadowCard::setupDropShadow()
{
    auto* effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(20);
    effect->setOffset(0, 6);
    effect->setColor(QColor(0, 0, 0, 50));
    setGraphicsEffect(effect);
}

// ─────────────────────────────────────────────────────────────────────────────
// paintEvent——手动阴影模式的自定义绘制
// ─────────────────────────────────────────────────────────────────────────────

void ShadowCard::paintEvent(QPaintEvent* event)
{
    if (m_shadowMode == ShadowMode::Manual) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // 绘制柔和阴影：多层半透明矩形逐渐扩散
        for (int i = kShadowBlur; i > 0; --i) {
            const int alpha = static_cast<int>(40.0 * (1.0 - static_cast<double>(i) / kShadowBlur));
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(0, 0, 0, alpha));
            painter.drawRoundedRect(
                rect().adjusted(i, i + kShadowOffsetY, -i, -i + kShadowOffsetY),
                kBorderRadius, kBorderRadius);
        }

        // 绘制白色卡片背景
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255));
        // 卡片本体不含阴影偏移，用固定内边距
        const QRect cardRect = rect().adjusted(kShadowBlur, kShadowBlur,
                                               -kShadowBlur,
                                               -kShadowBlur - kShadowOffsetY);
        painter.drawRoundedRect(cardRect, kBorderRadius, kBorderRadius);

        // 绘制淡灰色边框
        QPen pen(QColor(230, 230, 230), 1);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(cardRect.adjusted(1, 1, -1, -1),
                                kBorderRadius - 1, kBorderRadius - 1);
    }

    // 让 QFrame 基类处理 StyledPanel 模式的边框绘制
    QFrame::paintEvent(event);
}
