/// @file    checkable_group_box.cpp
/// @brief   CheckableGroupBox 类实现——paintEvent 重写、checkable 禁用级联、changeEvent 拦截。
///
/// 对应教程：进阶层 03-QtWidgets/38-QGroupBox 进阶。

#include "checkable_group_box.h"

#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QSpinBox>
#include <QStyleOptionGroupBox>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// 常量
// ─────────────────────────────────────────────────────────────────────────────

namespace
{
    constexpr int kCornerRadius = 8;         // 边框圆角半径
    constexpr int kTitlePadding = 8;         // 标题背景条左右内边距
    constexpr int kTitleVerticalPadding = 2; // 标题背景条上下内边距
    constexpr int kTitleLeftOffset = 12;     // 标题距左边框的距离
}

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

CheckableGroupBox::CheckableGroupBox(QWidget* parent)
    : QGroupBox(parent)
    , m_nameEdit(nullptr)
    , m_portSpin(nullptr)
    , m_protocolCombo(nullptr)
    , m_statusLabel(nullptr)
{
    setTitle(QStringLiteral("代理设置"));
    setCheckable(true);
    setChecked(true);

    setupChildWidgets();

    // toggled 信号连接——处理焦点转移和状态保留
    connect(this, &QGroupBox::toggled, this, &CheckableGroupBox::onToggled);

    setMinimumWidth(320);
}

// ─────────────────────────────────────────────────────────────────────────────
// paintEvent 重写——自定义边框和标题绘制
// ─────────────────────────────────────────────────────────────────────────────

void CheckableGroupBox::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 使用 initStyleOption 获取当前 QGroupBox 的完整状态信息
    QStyleOptionGroupBox option;
    initStyleOption(&option);

    // 1) 绘制自定义圆角边框——顶部留出标题区域
    const int titleHeight = fontMetrics().height();
    QRect frameRect = rect();
    frameRect.setTop(frameRect.top() + titleHeight / 2);

    // checked 状态决定边框颜色
    const QColor borderColor = isChecked() ? QColor("#5C9DC7") : QColor("#CCCCCC");
    painter.setPen(QPen(borderColor, 1.5));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(frameRect, kCornerRadius, kCornerRadius);

    // 2) 绘制标题背景条——用窗口背景色遮断边框线
    const QString titleText = title();
    QRect titleBoundingRect = fontMetrics().boundingRect(titleText);
    titleBoundingRect.adjust(-kTitlePadding, -kTitleVerticalPadding,
                             kTitlePadding, kTitleVerticalPadding);
    titleBoundingRect.moveLeft(kTitleLeftOffset);
    titleBoundingRect.moveTop(0);

    // 背景色与父控件一致，"遮断"边框线
    painter.fillRect(titleBoundingRect, palette().window().color());

    // 3) 绘制标题文字——彩色标题
    const QColor titleColor = isChecked() ? QColor("#2E75B6") : QColor("#999999");
    painter.setPen(titleColor);
    painter.setFont(font());
    painter.drawText(titleBoundingRect, Qt::AlignCenter, titleText);
}

// ─────────────────────────────────────────────────────────────────────────────
// changeEvent 重写——拦截 EnabledChange
// ─────────────────────────────────────────────────────────────────────────────

void CheckableGroupBox::changeEvent(QEvent* event)
{
    QGroupBox::changeEvent(event);

    if (event->type() == QEvent::EnabledChange) {
        // EnabledChange 触发时输出当前启用状态
        // 实际工程中可在此添加日志或通知外部组件
        const bool enabledNow = isEnabled();
        if (m_statusLabel) {
            m_statusLabel->setText(
                enabledNow ? QStringLiteral("状态：已启用 - 子控件可操作")
                           : QStringLiteral("状态：已禁用 - 子控件不可操作"));
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 私有方法实现
// ─────────────────────────────────────────────────────────────────────────────

void CheckableGroupBox::setupChildWidgets()
{
    auto* layout = new QVBoxLayout(this);

    // 名称输入行
    auto* nameRow = new QHBoxLayout;
    auto* nameLabel = new QLabel(QStringLiteral("代理地址:"));
    m_nameEdit = new QLineEdit(QStringLiteral("127.0.0.1"));
    nameRow->addWidget(nameLabel);
    nameRow->addWidget(m_nameEdit, 1);
    layout->addLayout(nameRow);

    // 端口和协议行
    auto* portRow = new QHBoxLayout;
    auto* portLabel = new QLabel(QStringLiteral("端口:"));
    m_portSpin = new QSpinBox;
    m_portSpin->setRange(1, 65535);
    m_portSpin->setValue(8080);

    auto* protocolLabel = new QLabel(QStringLiteral("协议:"));
    m_protocolCombo = new QComboBox;
    m_protocolCombo->addItem(QStringLiteral("HTTP"));
    m_protocolCombo->addItem(QStringLiteral("SOCKS5"));
    m_protocolCombo->addItem(QStringLiteral("HTTPS"));

    portRow->addWidget(portLabel);
    portRow->addWidget(m_portSpin);
    portRow->addWidget(protocolLabel);
    portRow->addWidget(m_protocolCombo, 1);
    layout->addLayout(portRow);

    // 状态标签
    m_statusLabel = new QLabel(QStringLiteral("状态：已启用 - 子控件可操作"));
    layout->addWidget(m_statusLabel);
}

void CheckableGroupBox::onToggled(bool checked)
{
    if (!checked) {
        // 取消勾选时将焦点转移到分组框自身
        // Qt 的 enabled 传播只改变 isEnabled() 返回值，不清除已有焦点
        setFocus();
    }
    // 不清空子控件内容——用户可能只是暂时关闭，再次勾选时应保留之前填写的内容
}
