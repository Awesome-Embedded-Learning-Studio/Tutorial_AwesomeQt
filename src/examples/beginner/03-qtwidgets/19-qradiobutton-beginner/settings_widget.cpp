// QtWidgets 入门示例 19: QRadioButton 单选按钮
// 演示：自动互斥：同一 parent 下单选按钮天然互斥
//       QButtonGroup 跨 parent 实现互斥分组
//       toggled(bool) 信号监听状态变化
//       自定义样式 QSS 圆形按钮美化

#include "settings_widget.h"

#include <QFont>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

// ============================================================================
// SettingsWidget: QRadioButton 综合演示 — 应用设置窗口
// ============================================================================
SettingsWidget::SettingsWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QRadioButton 综合演示 — 应用设置");
    resize(600, 420);
    initUi();
}

/// @brief 初始化界面
void SettingsWidget::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ---- 标题 ----
    auto *titleLabel = new QLabel("应用设置");
    titleLabel->setFont(QFont("Arial", 15, QFont::Bold));
    mainLayout->addWidget(titleLabel);

    // ---- 上下两行布局 ----
    auto *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(16);

    // ================================================================
    // 左侧: 主题选择 — 同一 QGroupBox 内自动互斥
    // ================================================================
    auto *themeGroup = new QGroupBox("主题选择");
    auto *themeLayout = new QVBoxLayout(themeGroup);

    auto *lightRadio = new QRadioButton("浅色主题");
    auto *darkRadio = new QRadioButton("深色主题");
    auto *systemRadio = new QRadioButton("跟随系统");

    lightRadio->setChecked(true);  // 默认选中浅色

    // 这三个按钮的 parent 都是 themeGroup，自动互斥
    themeLayout->addWidget(lightRadio);
    themeLayout->addWidget(darkRadio);
    themeLayout->addWidget(systemRadio);
    themeLayout->addStretch();

    // 单独一个 QButtonGroup 来监听主题选择
    m_themeGroup = new QButtonGroup(this);
    m_themeGroup->addButton(lightRadio, 0);
    m_themeGroup->addButton(darkRadio, 1);
    m_themeGroup->addButton(systemRadio, 2);
    connect(m_themeGroup, &QButtonGroup::idClicked, this,
            [this](int /*id*/) { updateStatus(); });

    contentLayout->addWidget(themeGroup);

    // ================================================================
    // 右侧: 语言选择 + 字体大小 — 不同 QGroupBox，用 QButtonGroup 分组
    // ================================================================
    auto *rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(12);

    // 语言选择
    auto *langGroup = new QGroupBox("语言选择");
    auto *langLayout = new QVBoxLayout(langGroup);

    auto *cnRadio = new QRadioButton("中文");
    auto *enRadio = new QRadioButton("English");
    cnRadio->setChecked(true);

    langLayout->addWidget(cnRadio);
    langLayout->addWidget(enRadio);

    m_langGroup = new QButtonGroup(this);
    m_langGroup->addButton(cnRadio, 0);
    m_langGroup->addButton(enRadio, 1);
    connect(m_langGroup, &QButtonGroup::idClicked, this,
            [this](int /*id*/) { updateStatus(); });

    rightLayout->addWidget(langGroup);

    // 字体大小
    auto *fontGroup = new QGroupBox("字体大小");
    auto *fontLayout = new QVBoxLayout(fontGroup);

    auto *smallRadio = new QRadioButton("小 (12px)");
    auto *mediumRadio = new QRadioButton("中 (14px)");
    auto *largeRadio = new QRadioButton("大 (16px)");
    mediumRadio->setChecked(true);

    fontLayout->addWidget(smallRadio);
    fontLayout->addWidget(mediumRadio);
    fontLayout->addWidget(largeRadio);

    m_fontGroup = new QButtonGroup(this);
    m_fontGroup->addButton(smallRadio, 0);
    m_fontGroup->addButton(mediumRadio, 1);
    m_fontGroup->addButton(largeRadio, 2);
    connect(m_fontGroup, &QButtonGroup::idClicked, this,
            [this](int /*id*/) { updateStatus(); });

    rightLayout->addWidget(fontGroup);
    rightLayout->addStretch();

    contentLayout->addLayout(rightLayout);
    mainLayout->addLayout(contentLayout, 1);

    // ================================================================
    // toggled(bool) 信号演示区
    // ================================================================
    auto *signalGroup = new QGroupBox("toggled(bool) 信号监听");
    auto *signalLayout = new QVBoxLayout(signalGroup);

    m_toggleLabel = new QLabel("切换上方任意选项，此处显示 toggled 信号的变化");
    m_toggleLabel->setStyleSheet(
        "background-color: #F5F5F5;"
        "border: 1px solid #E0E0E0;"
        "border-radius: 4px;"
        "padding: 8px;"
        "color: #555;");
    m_toggleLabel->setWordWrap(true);
    signalLayout->addWidget(m_toggleLabel);

    // 连接几个 toggled 信号来演示: toggled 会在选中/取消选中时都触发
    connect(lightRadio, &QRadioButton::toggled, this,
            [this](bool checked) {
                appendToggleLog(
                    QString("浅色主题 %1").arg(checked ? "选中" : "取消选中"));
            });
    connect(darkRadio, &QRadioButton::toggled, this,
            [this](bool checked) {
                appendToggleLog(
                    QString("深色主题 %1").arg(checked ? "选中" : "取消选中"));
            });

    mainLayout->addWidget(signalGroup);

    // ================================================================
    // 底部状态栏 + 重置按钮
    // ================================================================
    auto *bottomLayout = new QHBoxLayout();

    m_statusLabel = new QLabel();
    m_statusLabel->setStyleSheet("color: #666; font-size: 12px;");
    bottomLayout->addWidget(m_statusLabel, 1);

    auto *resetBtn = new QPushButton("重置为默认值");
    resetBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #F5F5F5;"
        "  border: 1px solid #DDD;"
        "  border-radius: 4px;"
        "  padding: 6px 16px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #EEEEEE;"
        "}");
    connect(resetBtn, &QPushButton::clicked, this, [this]() {
        m_themeGroup->button(0)->setChecked(true);   // 浅色
        m_langGroup->button(0)->setChecked(true);    // 中文
        m_fontGroup->button(1)->setChecked(true);    // 中号
        m_toggleLog.clear();
        m_toggleLabel->setText("已重置为默认值");
        updateStatus();
    });
    bottomLayout->addWidget(resetBtn);

    mainLayout->addLayout(bottomLayout);

    // 初始状态
    updateStatus();
}

/// @brief 更新底部状态标签
void SettingsWidget::updateStatus()
{
    const QString themeNames[] = {"浅色", "深色", "跟随系统"};
    const QString langNames[] = {"中文", "English"};
    const QString fontNames[] = {"小", "中", "大"};

    int themeId = m_themeGroup->checkedId();
    int langId = m_langGroup->checkedId();
    int fontId = m_fontGroup->checkedId();

    m_statusLabel->setText(
        QString("主题: %1 | 语言: %2 | 字体: %3")
            .arg(themeId >= 0 ? themeNames[themeId] : "未选择")
            .arg(langId >= 0 ? langNames[langId] : "未选择")
            .arg(fontId >= 0 ? fontNames[fontId] : "未选择"));
}

/// @brief 追加 toggled 信号日志
void SettingsWidget::appendToggleLog(const QString &message)
{
    m_toggleLog.append(message);
    // 只保留最后 4 条
    while (m_toggleLog.size() > 4) {
        m_toggleLog.removeFirst();
    }
    m_toggleLabel->setText(m_toggleLog.join(" → "));
}
