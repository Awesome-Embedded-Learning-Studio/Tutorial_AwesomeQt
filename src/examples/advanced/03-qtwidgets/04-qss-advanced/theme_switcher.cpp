/// @file    theme_switcher.cpp
/// @brief   ThemeSwitcher 类实现——动态主题切换、选择器特异性、样式级联演示。
///
/// 对应教程：进阶层 03-QtWidgets/04-QSS 进阶。

#include "theme_switcher.h"

#include <QApplication>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace {

/// @brief 亮色主题 QSS。
/// @note 所有样式写在全局 QSS 中，通过选择器区分控件，不在 C++ 中单独 setStyleSheet。
const QString kLightThemeQss = QStringLiteral(
    // ── 全局基础 ──
    "QWidget {"
    "   background-color: #FFFFFF;"
    "   color: #2C3E50;"
    "   font-family: 'Segoe UI', 'Noto Sans CJK SC', sans-serif;"
    "}"
    // ── QGroupBox 标题 ──
    "QGroupBox {"
    "   border: 1px solid #BDC3C7;"
    "   border-radius: 4px;"
    "   margin-top: 12px;"
    "   padding-top: 16px;"
    "   font-weight: bold;"
    "}"
    "QGroupBox::title {"
    "   subcontrol-origin: margin;"
    "   left: 10px;"
    "   padding: 0 4px;"
    "}"
    // ── QPushButton 类选择器 ──
    "QPushButton {"
    "   background-color: #3498DB;"
    "   color: white;"
    "   border: none;"
    "   border-radius: 4px;"
    "   padding: 8px 16px;"
    "   min-height: 20px;"
    "}"
    "QPushButton:hover {"
    "   background-color: #2980B9;"
    "}"
    "QPushButton:pressed {"
    "   background-color: #21618C;"
    "}"
    // ── ID 选择器：#deleteButton（特异性高于 QPushButton） ──
    "QPushButton#deleteButton {"
    "   background-color: #E74C3C;"
    "}"
    "QPushButton#deleteButton:hover {"
    "   background-color: #C0392B;"
    "}"
    // ── ID 选择器：#navButton ──
    "QPushButton#navButton {"
    "   background-color: #2ECC71;"
    "   text-align: left;"
    "   padding-left: 12px;"
    "}"
    "QPushButton#navButton:hover {"
    "   background-color: #27AE60;"
    "}"
    // ── 后代选择器：QGroupBox #descendantButton ──
    "QGroupBox QPushButton#descendantButton {"
    "   background-color: #9B59B6;"
    "   border: 2px dashed #8E44AD;"
    "}"
    // ── QLineEdit ──
    "QLineEdit {"
    "   border: 1px solid #BDC3C7;"
    "   border-radius: 4px;"
    "   padding: 6px 10px;"
    "   background-color: #FAFAFA;"
    "}"
    "QLineEdit:focus {"
    "   border-color: #3498DB;"
    "}"
    // ── QComboBox ──
    "QComboBox {"
    "   border: 1px solid #BDC3C7;"
    "   border-radius: 4px;"
    "   padding: 6px 10px;"
    "   background-color: #FAFAFA;"
    "}"
    // ── 级联演示：#cascadeParent 容器 ──
    "#cascadeParent {"
    "   background-color: #EBF5FB;"
    "   border: 2px solid #3498DB;"
    "   border-radius: 6px;"
    "}"
    // ── QLabel 样式 ──
    "QLabel {"
    "   background-color: transparent;"
    "   border: none;"
    "}"
);

/// @brief 暗色主题 QSS。
/// @note 与亮色主题选择器结构完全相同，只替换颜色值，确保主题切换的一致性。
const QString kDarkThemeQss = QStringLiteral(
    // ── 全局基础 ──
    "QWidget {"
    "   background-color: #1E1E2E;"
    "   color: #CDD6F4;"
    "   font-family: 'Segoe UI', 'Noto Sans CJK SC', sans-serif;"
    "}"
    // ── QGroupBox 标题 ──
    "QGroupBox {"
    "   border: 1px solid #45475A;"
    "   border-radius: 4px;"
    "   margin-top: 12px;"
    "   padding-top: 16px;"
    "   font-weight: bold;"
    "}"
    "QGroupBox::title {"
    "   subcontrol-origin: margin;"
    "   left: 10px;"
    "   padding: 0 4px;"
    "   color: #89B4FA;"
    "}"
    // ── QPushButton 类选择器 ──
    "QPushButton {"
    "   background-color: #45475A;"
    "   color: #CDD6F4;"
    "   border: none;"
    "   border-radius: 4px;"
    "   padding: 8px 16px;"
    "   min-height: 20px;"
    "}"
    "QPushButton:hover {"
    "   background-color: #585B70;"
    "}"
    "QPushButton:pressed {"
    "   background-color: #6C7086;"
    "}"
    // ── ID 选择器：#deleteButton（特异性高于 QPushButton） ──
    "QPushButton#deleteButton {"
    "   background-color: #F38BA8;"
    "   color: #1E1E2E;"
    "}"
    "QPushButton#deleteButton:hover {"
    "   background-color: #EBA0AC;"
    "}"
    // ── ID 选择器：#navButton ──
    "QPushButton#navButton {"
    "   background-color: #A6E3A1;"
    "   color: #1E1E2E;"
    "   text-align: left;"
    "   padding-left: 12px;"
    "}"
    "QPushButton#navButton:hover {"
    "   background-color: #94E2D5;"
    "}"
    // ── 后代选择器：QGroupBox #descendantButton ──
    "QGroupBox QPushButton#descendantButton {"
    "   background-color: #CBA6F7;"
    "   color: #1E1E2E;"
    "   border: 2px dashed #B4BEFE;"
    "}"
    // ── QLineEdit ──
    "QLineEdit {"
    "   border: 1px solid #45475A;"
    "   border-radius: 4px;"
    "   padding: 6px 10px;"
    "   background-color: #313244;"
    "   color: #CDD6F4;"
    "}"
    "QLineEdit:focus {"
    "   border-color: #89B4FA;"
    "}"
    // ── QComboBox ──
    "QComboBox {"
    "   border: 1px solid #45475A;"
    "   border-radius: 4px;"
    "   padding: 6px 10px;"
    "   background-color: #313244;"
    "   color: #CDD6F4;"
    "}"
    "QComboBox QAbstractItemView {"
    "   background-color: #313244;"
    "   color: #CDD6F4;"
    "   selection-background-color: #45475A;"
    "}"
    // ── 级联演示：#cascadeParent 容器 ──
    "#cascadeParent {"
    "   background-color: #313244;"
    "   border: 2px solid #89B4FA;"
    "   border-radius: 6px;"
    "}"
    // ── QLabel 样式 ──
    "QLabel {"
    "   background-color: transparent;"
    "   border: none;"
    "}"
);

}  // namespace

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

ThemeSwitcher::ThemeSwitcher(QWidget* parent) : QWidget(parent), m_currentTheme("light")
{
    auto* mainLayout = new QVBoxLayout(this);

    // 三个演示区域依次排列
    mainLayout->addWidget(createThemeSwitchSection());
    mainLayout->addWidget(createSelectorSpecificitySection());
    mainLayout->addWidget(createCascadeSection());
    mainLayout->addStretch();

    setWindowTitle(QStringLiteral("QSS Advanced - Theme Switcher"));
    resize(700, 600);

    // 初始应用亮色主题
    applyTheme(QStringLiteral("light"));
}

// ─────────────────────────────────────────────────────────────────────────────
// 主题切换区域
// ─────────────────────────────────────────────────────────────────────────────

QWidget* ThemeSwitcher::createThemeSwitchSection()
{
    auto* group = new QGroupBox(QStringLiteral("1. 动态主题切换"));
    auto* layout = new QVBoxLayout(group);

    auto* descLabel = new QLabel(
        QStringLiteral("通过 QApplication::setStyleSheet() 全局切换亮色/暗色主题。\n"
                       "所有样式集中在全局 QSS 中，不使用控件级别的 setStyleSheet()。"));
    descLabel->setWordWrap(true);

    // 主题选择行
    auto* themeRow = new QHBoxLayout;
    auto* themeLabel = new QLabel(QStringLiteral("选择主题:"));

    m_themeCombo = new QComboBox;
    m_themeCombo->addItem(QStringLiteral("Light（亮色）"), QStringLiteral("light"));
    m_themeCombo->addItem(QStringLiteral("Dark（暗色）"), QStringLiteral("dark"));

    themeRow->addWidget(themeLabel);
    themeRow->addWidget(m_themeCombo, 1);

    // 信息标签
    m_themeInfo = new QLabel;
    m_themeInfo->setWordWrap(true);

    // 演示控件：验证主题切换后各类控件是否正确刷新
    auto* demoRow = new QHBoxLayout;
    auto* demoEdit = new QLineEdit(QStringLiteral("QLineEdit 文本输入框"));
    auto* demoBtn = new QPushButton(QStringLiteral("QPushButton 按钮"));
    auto* demoNavBtn = new QPushButton(QStringLiteral("Nav Button"));
    demoNavBtn->setObjectName(QStringLiteral("navButton"));

    demoRow->addWidget(demoEdit, 1);
    demoRow->addWidget(demoBtn);
    demoRow->addWidget(demoNavBtn);

    layout->addWidget(descLabel);
    layout->addLayout(themeRow);
    layout->addWidget(m_themeInfo);
    layout->addLayout(demoRow);

    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this]() { applyTheme(m_themeCombo->currentData().toString()); });

    return group;
}

// ─────────────────────────────────────────────────────────────────────────────
// 选择器特异性对比
// ─────────────────────────────────────────────────────────────────────────────

QWidget* ThemeSwitcher::createSelectorSpecificitySection()
{
    auto* group = new QGroupBox(QStringLiteral("2. 选择器特异性（ID > 后代 > 类）"));
    auto* layout = new QVBoxLayout(group);

    auto* descLabel = new QLabel(
        QStringLiteral("三个按钮分别由不同特异性的选择器控制颜色：\n"
                       "  - 左：QPushButton（类选择器）→ 默认蓝色\n"
                       "  - 中：QGroupBox QPushButton#descendantButton（后代+ID）→ 紫色虚线边框\n"
                       "  - 右：QPushButton#deleteButton（ID 选择器）→ 红色"));
    descLabel->setWordWrap(true);

    auto* btnRow = new QHBoxLayout;

    // 类选择器按钮：只匹配 QPushButton {}
    m_specificityClassBtn = new QPushButton(QStringLiteral("QPushButton（类选择器）"));

    // 后代+ID 选择器按钮：匹配 QGroupBox QPushButton#descendantButton {}
    m_specificityDescBtn = new QPushButton(QStringLiteral("后代+ID 选择器"));
    m_specificityDescBtn->setObjectName(QStringLiteral("descendantButton"));

    // ID 选择器按钮：匹配 QPushButton#deleteButton {}
    m_specificityIdBtn = new QPushButton(QStringLiteral("#deleteButton（ID 选择器）"));
    m_specificityIdBtn->setObjectName(QStringLiteral("deleteButton"));

    btnRow->addWidget(m_specificityClassBtn, 1);
    btnRow->addWidget(m_specificityDescBtn, 1);
    btnRow->addWidget(m_specificityIdBtn, 1);

    m_specificityInfo = new QLabel;
    m_specificityInfo->setWordWrap(true);

    layout->addWidget(descLabel);
    layout->addLayout(btnRow);
    layout->addWidget(m_specificityInfo);

    // 按钮点击时显示其匹配的选择器说明
    connect(m_specificityClassBtn, &QPushButton::clicked, this, [this]() {
        m_specificityInfo->setText(
            QStringLiteral("你点击了「类选择器」按钮。\n"
                           "匹配规则：QPushButton { background-color: ... }\n"
                           "特异性最低，当 ID 选择器或后代选择器也匹配时会被覆盖。"));
    });
    connect(m_specificityDescBtn, &QPushButton::clicked, this, [this]() {
        m_specificityInfo->setText(
            QStringLiteral("你点击了「后代+ID 选择器」按钮。\n"
                           "匹配规则：QGroupBox QPushButton#descendantButton { ... }\n"
                           "后代选择器 + ID 选择器组合，特异性介于类选择器和纯 ID 选择器之间。\n"
                           "@note 此按钮必须位于 QGroupBox 内部才能匹配后代选择器。"));
    });
    connect(m_specificityIdBtn, &QPushButton::clicked, this, [this]() {
        m_specificityInfo->setText(
            QStringLiteral("你点击了「ID 选择器」按钮。\n"
                           "匹配规则：QPushButton#deleteButton { background-color: #E74C3C; }\n"
                           "ID 选择器特异性最高，覆盖 QPushButton 类选择器的蓝色。\n"
                           "@note objectName 在同一窗口内应保持唯一，否则多个控件同色。"));
    });

    return group;
}

// ─────────────────────────────────────────────────────────────────────────────
// 样式级联演示
// ─────────────────────────────────────────────────────────────────────────────

QWidget* ThemeSwitcher::createCascadeSection()
{
    auto* group = new QGroupBox(QStringLiteral("3. 样式级联（父容器与子控件）"));
    auto* layout = new QVBoxLayout(group);

    auto* descLabel = new QLabel(
        QStringLiteral("父容器 #cascadeParent 有独立的背景色和边框。\n"
                       "子控件默认继承父控件的 background-color，但 QPushButton 的 QSS 会覆盖继承。\n"
                       "QLabel 的 background-color 设为 transparent 以透出父容器的背景色。"));
    descLabel->setWordWrap(true);

    // 父容器
    m_cascadeParent = new QWidget;
    m_cascadeParent->setObjectName(QStringLiteral("cascadeParent"));
    auto* innerLayout = new QVBoxLayout(m_cascadeParent);

    m_cascadeParentLabel =
        new QLabel(QStringLiteral("我是 QLabel（transparent 背景，透出父容器底色）"));
    m_cascadeChildBtn =
        new QPushButton(QStringLiteral("我是 QPushButton（QSS 覆盖了继承的背景色）"));

    innerLayout->addWidget(m_cascadeParentLabel);
    innerLayout->addWidget(m_cascadeChildBtn);

    m_cascadeInfo = new QLabel;
    m_cascadeInfo->setWordWrap(true);

    layout->addWidget(descLabel);
    layout->addWidget(m_cascadeParent);
    layout->addWidget(m_cascadeInfo);

    connect(m_cascadeChildBtn, &QPushButton::clicked, this, [this]() {
        m_cascadeInfo->setText(
            QStringLiteral("样式级联规则：\n"
                           "1. 全局 QSS 中 #cascadeParent 设置了独立背景色和边框\n"
                           "2. 子 QLabel 的 background-color: transparent 透出父容器底色\n"
                           "3. 子 QPushButton 的 QPushButton {} 规则覆盖了继承的背景色\n"
                           "4. 切换主题时，全局 QSS 中 #cascadeParent 的颜色随之改变"));
    });

    return group;
}

// ─────────────────────────────────────────────────────────────────────────────
// 应用主题
// ─────────────────────────────────────────────────────────────────────────────

void ThemeSwitcher::applyTheme(const QString& themeName)
{
    m_currentTheme = themeName;

    // 全部通过 QApplication 全局样式表，不在 C++ 中单独 setStyleSheet
    if (themeName == QStringLiteral("dark")) {
        qApp->setStyleSheet(kDarkThemeQss);
        m_themeInfo->setText(
            QStringLiteral("当前主题：Dark（暗色）\n"
                           "已通过 QApplication::setStyleSheet() 设置全局暗色 QSS。"));
    } else {
        qApp->setStyleSheet(kLightThemeQss);
        m_themeInfo->setText(
            QStringLiteral("当前主题：Light（亮色）\n"
                           "已通过 QApplication::setStyleSheet() 设置全局亮色 QSS。"));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 更新特异性信息
// ─────────────────────────────────────────────────────────────────────────────

void ThemeSwitcher::updateSpecificityInfo()
{
    m_specificityInfo->setText(
        QStringLiteral("QSS 选择器特异性（从高到低）：\n"
                       "  1. ID 选择器  — QPushButton#deleteButton\n"
                       "  2. 后代选择器 — QGroupBox QPushButton#descendantButton\n"
                       "  3. 类选择器   — QPushButton\n"
                       "切换主题后观察颜色变化——三条规则的特异性关系在两种主题下保持一致。"));
}
