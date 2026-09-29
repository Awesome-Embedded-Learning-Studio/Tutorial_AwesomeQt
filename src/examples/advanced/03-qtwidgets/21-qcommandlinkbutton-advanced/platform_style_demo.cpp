/// @file    platform_style_demo.cpp
/// @brief   PlatformStyleDemo 类实现——QCommandLinkButton 平台样式与 description 演示。
///
/// 对应教程：进阶层 03-QtWidgets/21-QCommandLinkButton 进阶。

#include "platform_style_demo.h"

#include <QCommandLinkButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

PlatformStyleDemo::PlatformStyleDemo(QWidget* parent)
    : QWidget(parent)
    , m_descIndex(0)
{
    auto* mainLayout = new QVBoxLayout(this);

    // 顶部标题
    auto* title = new QLabel(QStringLiteral("选择安装方式"));
    title->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: bold;"));
    title->setAlignment(Qt::AlignCenter);

    // 状态标签——显示用户点击了哪个按钮
    m_resultLabel = new QLabel(QStringLiteral("尚未选择安装方式"));
    m_resultLabel->setAlignment(Qt::AlignCenter);
    m_resultLabel->setFrameShape(QFrame::StyledPanel);

    mainLayout->addWidget(title);
    mainLayout->addWidget(createNativeSection());
    mainLayout->addWidget(createStyledSection());
    mainLayout->addWidget(createDynamicSection());
    mainLayout->addStretch();
    mainLayout->addWidget(m_resultLabel);

    setWindowTitle(QStringLiteral("QCommandLinkButton Advanced Demo"));
    resize(500, 550);
}

// ─────────────────────────────────────────────────────────────────────────────
// 原生 QCommandLinkButton（无 QSS）
// ─────────────────────────────────────────────────────────────────────────────

QWidget* PlatformStyleDemo::createNativeSection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* sectionTitle =
        new QLabel(QStringLiteral("1. 原生 QCommandLinkButton（Windows 上显示原生风格）"));
    sectionTitle->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 13px;"));

    // 完整安装——带 description 的典型用法
    auto* fullBtn = new QCommandLinkButton(
        QStringLiteral("完整安装"),
        QStringLiteral("安装所有组件，包括文档、示例和开发工具。推荐大多数用户选择此方式。"),
        container);
    connect(fullBtn, &QCommandLinkButton::clicked, this,
            [this]() { onChoiceSelected(QStringLiteral("完整安装（原生风格）")); });

    // 自定义安装——短描述
    auto* customBtn = new QCommandLinkButton(
        QStringLiteral("自定义安装"),
        QStringLiteral("选择需要安装的组件，适合有经验的开发者。"),
        container);
    connect(customBtn, &QCommandLinkButton::clicked, this,
            [this]() { onChoiceSelected(QStringLiteral("自定义安装（原生风格）")); });

    layout->addWidget(sectionTitle);
    layout->addWidget(fullBtn);
    layout->addWidget(customBtn);

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// QSS 美化的 QCommandLinkButton（跨平台统一样式）
// ─────────────────────────────────────────────────────────────────────────────

QWidget* PlatformStyleDemo::createStyledSection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* sectionTitle =
        new QLabel(QStringLiteral("2. QSS 美化（跨平台统一样式，description 颜色无法通过 QSS 控制）"));
    sectionTitle->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 13px;"));
    sectionTitle->setWordWrap(true);

    // QSS 只能控制标题颜色、背景、边框；description 的字号和颜色由 QStyle 绘制，QSS 无法覆盖
    const QString btnStyle =
        QStringLiteral("QCommandLinkButton {"
                       "  color: #0066CC;"
                       "  background: transparent;"
                       "  border: 1px solid #cccccc;"
                       "  border-radius: 4px;"
                       "  padding: 8px 16px;"
                       "}"
                       "QCommandLinkButton:hover {"
                       "  color: #004499;"
                       "  background: #f0f6ff;"
                       "  border: 1px solid #88aacc;"
                       "}");

    auto* minBtn = new QCommandLinkButton(
        QStringLiteral("最小安装"),
        QStringLiteral("仅安装运行时必需的核心组件，占用磁盘空间最小。"),
        container);
    minBtn->setStyleSheet(btnStyle);
    connect(minBtn, &QCommandLinkButton::clicked, this,
            [this]() { onChoiceSelected(QStringLiteral("最小安装（QSS美化）")); });

    auto* portableBtn = new QCommandLinkButton(
        QStringLiteral("便携安装"),
        QStringLiteral("不写入注册表，可放在 U 盘中使用。适合需要移动办公的用户。"),
        container);
    portableBtn->setStyleSheet(btnStyle);
    connect(portableBtn, &QCommandLinkButton::clicked, this,
            [this]() { onChoiceSelected(QStringLiteral("便携安装（QSS美化）")); });

    layout->addWidget(sectionTitle);
    layout->addWidget(minBtn);
    layout->addWidget(portableBtn);

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// 动态 description 切换——演示 sizeHint 随描述长度变化
// ─────────────────────────────────────────────────────────────────────────────

QWidget* PlatformStyleDemo::createDynamicSection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* sectionTitle = new QLabel(
        QStringLiteral("3. 动态 description 切换（观察 sizeHint 变化导致布局重排）"));
    sectionTitle->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 13px;"));
    sectionTitle->setWordWrap(true);

    // 初始描述较短
    m_dynamicBtn = new QCommandLinkButton(
        QStringLiteral("高级选项"),
        QStringLiteral("短描述：配置高级参数。"),
        container);

    connect(m_dynamicBtn, &QCommandLinkButton::clicked, this,
            [this]() { onChoiceSelected(QStringLiteral("高级选项（动态描述）")); });

    // 切换按钮：在短描述和长描述之间循环
    m_toggleDescBtn = new QPushButton(QStringLiteral("切换描述文本（短 -> 长）"));

    layout->addWidget(sectionTitle);
    layout->addWidget(m_dynamicBtn);
    layout->addWidget(m_toggleDescBtn);

    connect(m_toggleDescBtn, &QPushButton::clicked, this, [this]() {
        // 动态修改 description 会改变 sizeHint，布局自动重新计算按钮高度
        if (m_descIndex == 0) {
            m_dynamicBtn->setDescription(
                QStringLiteral("这是一个非常长的描述文字，用于演示 setDescription 修改描述文本后"
                               "sizeHint 发生变化导致布局自动重排的行为。观察按钮高度的变化。"));
            m_toggleDescBtn->setText(QStringLiteral("切换描述文本（长 -> 短）"));
            m_descIndex = 1;
        } else {
            m_dynamicBtn->setDescription(QStringLiteral("短描述：配置高级参数。"));
            m_toggleDescBtn->setText(QStringLiteral("切换描述文本（短 -> 长）"));
            m_descIndex = 0;
        }
    });

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// 槽函数
// ─────────────────────────────────────────────────────────────────────────────

void PlatformStyleDemo::onChoiceSelected(const QString& choice)
{
    m_resultLabel->setText(QStringLiteral("已选择: %1").arg(choice));
}
