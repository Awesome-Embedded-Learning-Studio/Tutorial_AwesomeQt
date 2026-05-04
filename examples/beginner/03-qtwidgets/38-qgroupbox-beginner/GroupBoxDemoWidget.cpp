// QtWidgets 入门示例 38: QGroupBox 分组框
// 演示：带标题边框的控件分组容器
//       setCheckable(true) 可勾选分组框（整组启用/禁用）
//       setAlignment 标题对齐
//       嵌套布局的正确姿势

#include "GroupBoxDemoWidget.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

// ============================================================================
// GroupBoxDemoWidget: QGroupBox 综合演示窗口
// ============================================================================
GroupBoxDemoWidget::GroupBoxDemoWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QGroupBox 综合演示 — 分组框");
    resize(520, 620);
    initUi();
}

/// @brief 初始化界面
void GroupBoxDemoWidget::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // ================================================================
    // 区域 1: 基本设置分组框（普通分组，不可勾选）
    // ================================================================
    auto *basicGroup = new QGroupBox("基本设置");
    auto *basicLayout = new QVBoxLayout(basicGroup);
    basicLayout->setSpacing(8);

    // 用户名
    auto *nameRow = new QHBoxLayout;
    auto *nameLabel = new QLabel("用户名:");
    nameLabel->setMinimumWidth(80);
    m_usernameEdit = new QLineEdit;
    m_usernameEdit->setPlaceholderText("请输入用户名");
    nameRow->addWidget(nameLabel);
    nameRow->addWidget(m_usernameEdit, 1);
    basicLayout->addLayout(nameRow);

    // 语言选择
    auto *langRow = new QHBoxLayout;
    auto *langLabel = new QLabel("界面语言:");
    langLabel->setMinimumWidth(80);
    m_langCombo = new QComboBox;
    m_langCombo->addItems(
        {"简体中文", "English", "日本語", "Deutsch"});
    langRow->addWidget(langLabel);
    langRow->addWidget(m_langCombo, 1);
    basicLayout->addLayout(langRow);

    // 自动启动
    m_autoStartCheck = new QCheckBox("开机自动启动");
    basicLayout->addWidget(m_autoStartCheck);

    mainLayout->addWidget(basicGroup);

    // ================================================================
    // 区域 2: 网络配置分组框（可勾选）
    // ================================================================
    m_networkGroup = new QGroupBox("网络配置（使用代理）");
    m_networkGroup->setCheckable(true);
    m_networkGroup->setChecked(false);

    auto *networkLayout = new QVBoxLayout(m_networkGroup);
    networkLayout->setSpacing(8);

    // 代理地址
    auto *proxyRow = new QHBoxLayout;
    auto *proxyLabel = new QLabel("代理地址:");
    proxyLabel->setMinimumWidth(80);
    m_proxyEdit = new QLineEdit;
    m_proxyEdit->setPlaceholderText("例如: 127.0.0.1");
    proxyRow->addWidget(proxyLabel);
    proxyRow->addWidget(m_proxyEdit, 1);
    networkLayout->addLayout(proxyRow);

    // 端口
    auto *portRow = new QHBoxLayout;
    auto *portLabel = new QLabel("端口号:");
    portLabel->setMinimumWidth(80);
    m_portSpin = new QSpinBox;
    m_portSpin->setRange(1, 65535);
    m_portSpin->setValue(8080);
    portRow->addWidget(portLabel);
    portRow->addWidget(m_portSpin, 1);
    networkLayout->addLayout(portRow);

    mainLayout->addWidget(m_networkGroup);

    // 网络分组框勾选状态联动
    connect(m_networkGroup, &QGroupBox::toggled, this,
            [this](bool checked) {
        if (checked) {
            m_proxyEdit->setText("127.0.0.1");
            m_portSpin->setValue(8080);
            m_statusLabel->setText(
                "代理已启用，默认地址 127.0.0.1:8080");
            m_statusLabel->setStyleSheet(
                "color: #388E3C; font-size: 11px;"
                "padding: 6px;"
                "background-color: #E8F5E9;"
                "border: 1px solid #A5D6A7;"
                "border-radius: 4px;");
        } else {
            m_proxyEdit->clear();
            m_statusLabel->setText("代理已禁用");
            m_statusLabel->setStyleSheet(
                "color: #888; font-size: 11px;"
                "padding: 6px;"
                "background-color: #FAFAFA;"
                "border: 1px solid #E0E0E0;"
                "border-radius: 4px;");
        }
    });

    // ================================================================
    // 区域 3: 界面偏好（嵌套分组框）
    // ================================================================
    auto *uiGroup = new QGroupBox("界面偏好");
    uiGroup->setAlignment(Qt::AlignHCenter);  // 标题居中
    auto *uiOuterLayout = new QVBoxLayout(uiGroup);
    uiOuterLayout->setSpacing(10);

    // --- 子分组 1: 主题设置 ---
    auto *themeGroup = new QGroupBox("主题设置");
    themeGroup->setContentsMargins(6, 14, 6, 6);
    auto *themeLayout = new QVBoxLayout(themeGroup);
    themeLayout->setSpacing(6);

    m_darkModeCheck = new QCheckBox("深色模式");
    m_systemThemeCheck = new QCheckBox("跟随系统主题");
    m_systemThemeCheck->setChecked(true);
    themeLayout->addWidget(m_darkModeCheck);
    themeLayout->addWidget(m_systemThemeCheck);

    uiOuterLayout->addWidget(themeGroup);

    // --- 子分组 2: 字体设置 ---
    auto *fontGroup = new QGroupBox("字体设置");
    fontGroup->setContentsMargins(6, 14, 6, 6);
    auto *fontLayout = new QVBoxLayout(fontGroup);
    fontLayout->setSpacing(6);

    auto *fontSizeRow = new QHBoxLayout;
    auto *fontSizeLabel = new QLabel("字体大小:");
    fontSizeLabel->setMinimumWidth(70);
    m_fontSizeSpin = new QSpinBox;
    m_fontSizeSpin->setRange(8, 36);
    m_fontSizeSpin->setValue(14);
    m_fontSizeSpin->setSuffix(" px");
    fontSizeRow->addWidget(fontSizeLabel);
    fontSizeRow->addWidget(m_fontSizeSpin, 1);
    fontLayout->addLayout(fontSizeRow);

    auto *fontFamilyRow = new QHBoxLayout;
    auto *fontFamilyLabel = new QLabel("字体族:");
    fontFamilyLabel->setMinimumWidth(70);
    m_fontCombo = new QComboBox;
    m_fontCombo->addItems(
        {"Sans Serif", "Serif", "Monospace"});
    fontFamilyRow->addWidget(fontFamilyLabel);
    fontFamilyRow->addWidget(m_fontCombo, 1);
    fontLayout->addLayout(fontFamilyRow);

    uiOuterLayout->addWidget(fontGroup);

    mainLayout->addWidget(uiGroup);

    // ================================================================
    // 底部: 状态标签 + 应用按钮
    // ================================================================
    m_statusLabel = new QLabel("修改设置后点击「应用」生效");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet(
        "color: #888; font-size: 11px;"
        "padding: 6px;"
        "background-color: #FAFAFA;"
        "border: 1px solid #E0E0E0;"
        "border-radius: 4px;");
    mainLayout->addWidget(m_statusLabel);

    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();

    auto *applyBtn = new QPushButton("应用");
    applyBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #1976D2; color: white;"
        "  border: none; border-radius: 6px;"
        "  padding: 8px 32px; font-size: 13px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #1565C0; }"
        "QPushButton:pressed { background-color: #0D47A1; }");

    auto *resetBtn = new QPushButton("重置");
    resetBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #757575; color: white;"
        "  border: none; border-radius: 6px;"
        "  padding: 8px 32px; font-size: 13px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #616161; }");

    btnLayout->addWidget(applyBtn);
    btnLayout->addWidget(resetBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    // 应用按钮
    connect(applyBtn, &QPushButton::clicked, this,
            [this]() {
        QString summary;
        summary += QString("用户名: %1\n")
                       .arg(m_usernameEdit->text().isEmpty()
                                ? "(未设置)"
                                : m_usernameEdit->text());
        summary += QString("语言: %1\n")
                       .arg(m_langCombo->currentText());
        summary += QString("自动启动: %1\n")
                       .arg(m_autoStartCheck->isChecked()
                                ? "是"
                                : "否");
        summary += QString("代理: %1\n")
                       .arg(m_networkGroup->isChecked()
                                ? "启用"
                                : "禁用");
        if (m_networkGroup->isChecked()) {
            summary += QString("  地址: %1:%2\n")
                           .arg(m_proxyEdit->text())
                           .arg(m_portSpin->value());
        }
        summary += QString("深色模式: %1\n")
                       .arg(m_darkModeCheck->isChecked()
                                ? "是"
                                : "否");
        summary += QString("字体: %1 %2px")
                       .arg(m_fontCombo->currentText())
                       .arg(m_fontSizeSpin->value());

        m_statusLabel->setText("设置已应用:\n" + summary);
        m_statusLabel->setStyleSheet(
            "color: #1565C0; font-size: 11px;"
            "padding: 6px;"
            "background-color: #E3F2FD;"
            "border: 1px solid #90CAF9;"
            "border-radius: 4px;");
    });

    // 重置按钮
    connect(resetBtn, &QPushButton::clicked, this,
            [this]() {
        m_usernameEdit->clear();
        m_langCombo->setCurrentIndex(0);
        m_autoStartCheck->setChecked(false);
        m_networkGroup->setChecked(false);
        m_darkModeCheck->setChecked(false);
        m_systemThemeCheck->setChecked(true);
        m_fontSizeSpin->setValue(14);
        m_fontCombo->setCurrentIndex(0);
        m_statusLabel->setText(
            "所有设置已重置为默认值");
        m_statusLabel->setStyleSheet(
            "color: #F57C00; font-size: 11px;"
            "padding: 6px;"
            "background-color: #FFF3E0;"
            "border: 1px solid #FFCC80;"
            "border-radius: 4px;");
    });
}
