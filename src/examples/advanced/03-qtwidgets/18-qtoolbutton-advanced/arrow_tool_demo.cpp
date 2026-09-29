/// @file    arrow_tool_demo.cpp
/// @brief   ArrowToolDemo 类实现——ArrowType、PopupMode、QToolBar 集成演示。
///
/// 对应教程：进阶层 03-QtWidgets/18-QToolButton 进阶。

#include "arrow_tool_demo.h"

#include <QAction>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QStatusBar>
#include <QTextEdit>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

ArrowToolDemo::ArrowToolDemo(QWidget* parent)
    : QMainWindow(parent)
    , m_historyCount(0)
    , m_iconOnlyMode(true)
{
    // 创建顶部工具栏
    createToolBar();

    // 中央区域：弹出模式演示 + 日志
    auto* centralWidget = new QWidget;
    auto* mainLayout = new QVBoxLayout(centralWidget);

    mainLayout->addWidget(createPopupModeSection());

    // 日志区域
    auto* logTitle =
        new QLabel(QStringLiteral("操作日志（观察不同 PopupMode 的信号触发时机）"));
    logTitle->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14px;"));
    m_logEdit = new QTextEdit;
    m_logEdit->setReadOnly(true);
    m_logEdit->setMaximumHeight(200);

    mainLayout->addWidget(logTitle);
    mainLayout->addWidget(m_logEdit);
    mainLayout->addStretch();

    setCentralWidget(centralWidget);
    setWindowTitle(QStringLiteral("QToolButton Advanced Demo"));
    resize(700, 550);

    statusBar()->showMessage(QStringLiteral("点击工具栏上的按钮进行测试"));
}

// ─────────────────────────────────────────────────────────────────────────────
// 工具栏创建
// ─────────────────────────────────────────────────────────────────────────────

void ArrowToolDemo::createToolBar()
{
    m_toolBar = addToolBar(QStringLiteral("Navigation"));
    // 初始为 IconOnly 模式，稍后可通过按钮切换
    m_toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    // --- 第一组：四个 ArrowType 方向按钮 ---
    // setArrowType 让 QStyle 绘制方向箭头，无需 QIcon
    // ArrowType 优先级高于 setIcon，所以不要同时设置两者
    m_upArrowBtn = new QToolButton;
    m_upArrowBtn->setArrowType(Qt::UpArrow);
    m_upArrowBtn->setAutoRaise(true);
    m_upArrowBtn->setToolTip(QStringLiteral("导航 - 上"));
    m_upArrowBtn->setFixedSize(32, 32);
    m_toolBar->addWidget(m_upArrowBtn);

    m_downArrowBtn = new QToolButton;
    m_downArrowBtn->setArrowType(Qt::DownArrow);
    m_downArrowBtn->setAutoRaise(true);
    m_downArrowBtn->setToolTip(QStringLiteral("导航 - 下"));
    m_downArrowBtn->setFixedSize(32, 32);
    m_toolBar->addWidget(m_downArrowBtn);

    m_leftArrowBtn = new QToolButton;
    m_leftArrowBtn->setArrowType(Qt::LeftArrow);
    m_leftArrowBtn->setAutoRaise(true);
    m_leftArrowBtn->setToolTip(QStringLiteral("导航 - 左"));
    m_leftArrowBtn->setFixedSize(32, 32);
    m_toolBar->addWidget(m_leftArrowBtn);

    m_rightArrowBtn = new QToolButton;
    m_rightArrowBtn->setArrowType(Qt::RightArrow);
    m_rightArrowBtn->setAutoRaise(true);
    m_rightArrowBtn->setToolTip(QStringLiteral("导航 - 右"));
    m_rightArrowBtn->setFixedSize(32, 32);
    m_toolBar->addWidget(m_rightArrowBtn);

    m_toolBar->addSeparator();

    // --- 第二组：历史记录按钮（DelayedPopup）---
    // DelayedPopup：快速点击触发 clicked，长按弹出菜单
    // 必须先 setMenu 再 setPopupMode，否则菜单无处可弹
    m_historyBtn = new QToolButton;
    m_historyBtn->setText(QStringLiteral("History"));
    m_historyBtn->setToolTip(
        QStringLiteral("快速点击 = 撤销 | 长按 = 弹出历史记录"));
    m_historyBtn->setAutoRaise(true);

    m_historyMenu = new QMenu(m_historyBtn);
    m_historyBtn->setMenu(m_historyMenu);
    m_historyBtn->setPopupMode(QToolButton::DelayedPopup);
    m_toolBar->addWidget(m_historyBtn);

    m_toolBar->addSeparator();

    // --- 第三组：导出按钮（MenuButtonPopup）---
    // MenuButtonPopup：按钮主体触发 clicked，箭头区域弹出菜单
    // 主体和箭头是两个独立的点击区域
    m_exportBtn = new QToolButton;
    m_exportBtn->setText(QStringLiteral("Export"));
    m_exportBtn->setToolTip(
        QStringLiteral("点击主体 = 默认导出 | 点击箭头 = 选择格式"));
    m_exportBtn->setAutoRaise(true);

    m_exportMenu = new QMenu(m_exportBtn);
    m_exportMenu->addAction(QStringLiteral("PDF"), this, &ArrowToolDemo::onExportPdf);
    m_exportMenu->addAction(QStringLiteral("PNG"), this, &ArrowToolDemo::onExportPng);
    m_exportMenu->addAction(QStringLiteral("SVG"), this, &ArrowToolDemo::onExportSvg);

    m_exportBtn->setMenu(m_exportMenu);
    m_exportBtn->setPopupMode(QToolButton::MenuButtonPopup);
    m_toolBar->addWidget(m_exportBtn);

    m_toolBar->addSeparator();

    // --- 风格切换按钮 ---
    // 使用 addAction 方式添加到工具栏，对比 addWidget 的区别
    // addAction 返回的 QToolButton 由 QToolBar 自动管理 autoRaise
    auto* toggleStyleAction =
        new QAction(QStringLiteral("Toggle Style"), this);
    toggleStyleAction->setToolTip(
        QStringLiteral("在 IconOnly 和 TextBesideIcon 之间切换工具栏风格"));
    m_toolBar->addAction(toggleStyleAction);

    // --- 信号连接 ---
    // ArrowType 按钮使用 clicked 信号（pressed 在 DelayedPopup 下可能不安全）
    connect(m_upArrowBtn, &QToolButton::clicked, this,
            [this]() { onArrowClicked(QStringLiteral("上")); });
    connect(m_downArrowBtn, &QToolButton::clicked, this,
            [this]() { onArrowClicked(QStringLiteral("下")); });
    connect(m_leftArrowBtn, &QToolButton::clicked, this,
            [this]() { onArrowClicked(QStringLiteral("左")); });
    connect(m_rightArrowBtn, &QToolButton::clicked, this,
            [this]() { onArrowClicked(QStringLiteral("右")); });

    // 历史记录按钮：clicked 只在快速点击时触发
    connect(m_historyBtn, &QToolButton::clicked, this,
            &ArrowToolDemo::onHistoryClicked);
    // aboutToShow 在菜单弹出前触发，用于动态更新菜单内容
    connect(m_historyMenu, &QMenu::aboutToShow, this,
            &ArrowToolDemo::onHistoryMenuAboutToShow);

    // 导出按钮：MenuButtonPopup 下主体点击触发 clicked
    connect(m_exportBtn, &QToolButton::clicked, this,
            &ArrowToolDemo::onExportClicked);

    // 风格切换
    connect(toggleStyleAction, &QAction::triggered, this,
            &ArrowToolDemo::onToggleStyle);
}

// ─────────────────────────────────────────────────────────────────────────────
// 独立 PopupMode 对比演示区
// ─────────────────────────────────────────────────────────────────────────────

QWidget* ArrowToolDemo::createPopupModeSection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* sectionTitle =
        new QLabel(QStringLiteral("三种 PopupMode 独立对比（工具栏外）"));
    sectionTitle->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14px;"));

    // --- DelayedPopup ---
    // 快速点击触发 clicked，长按（约 600ms）弹出菜单
    auto* delayedRow = new QHBoxLayout;
    auto* delayedLabel =
        new QLabel(QStringLiteral("DelayedPopup: 快速点击 = clicked | 长按 = 菜单"));
    m_delayedBtn = new QToolButton;
    m_delayedBtn->setText(QStringLiteral("Delayed"));
    auto* delayedMenu = new QMenu(m_delayedBtn);
    delayedMenu->addAction(QStringLiteral("延迟菜单项 1"));
    delayedMenu->addAction(QStringLiteral("延迟菜单项 2"));
    m_delayedBtn->setMenu(delayedMenu);
    m_delayedBtn->setPopupMode(QToolButton::DelayedPopup);

    delayedRow->addWidget(m_delayedBtn);
    delayedRow->addWidget(delayedLabel, 1);

    // --- InstantPopup ---
    // 点击直接弹出菜单，pressed 和 clicked 都不会触发
    auto* instantRow = new QHBoxLayout;
    auto* instantLabel =
        new QLabel(QStringLiteral("InstantPopup: 点击直接弹菜单 | clicked 不触发"));
    m_instantBtn = new QToolButton;
    m_instantBtn->setText(QStringLiteral("Instant"));
    auto* instantMenu = new QMenu(m_instantBtn);
    instantMenu->addAction(QStringLiteral("即时菜单项 1"));
    instantMenu->addAction(QStringLiteral("即时菜单项 2"));
    m_instantBtn->setMenu(instantMenu);
    m_instantBtn->setPopupMode(QToolButton::InstantPopup);

    instantRow->addWidget(m_instantBtn);
    instantRow->addWidget(instantLabel, 1);

    // --- MenuButtonPopup ---
    // 按钮分成两个区域：主体 + 箭头
    auto* menuRow = new QHBoxLayout;
    auto* menuLabel =
        new QLabel(QStringLiteral(
            "MenuButtonPopup: 主体 = clicked | 箭头 = 弹菜单"));
    m_menuBtn = new QToolButton;
    m_menuBtn->setText(QStringLiteral("MenuBtn"));
    auto* menuBtnMenu = new QMenu(m_menuBtn);
    menuBtnMenu->addAction(QStringLiteral("菜单按钮项 1"));
    menuBtnMenu->addAction(QStringLiteral("菜单按钮项 2"));
    m_menuBtn->setMenu(menuBtnMenu);
    m_menuBtn->setPopupMode(QToolButton::MenuButtonPopup);

    menuRow->addWidget(m_menuBtn);
    menuRow->addWidget(menuLabel, 1);

    // 行为说明标签
    m_popupInfoLabel = new QLabel;
    m_popupInfoLabel->setWordWrap(true);
    m_popupInfoLabel->setStyleSheet(
        QStringLiteral("color: gray; font-style: italic;"));

    layout->addWidget(sectionTitle);
    layout->addLayout(delayedRow);
    layout->addLayout(instantRow);
    layout->addLayout(menuRow);
    layout->addWidget(m_popupInfoLabel);

    // --- 信号连接：观察各模式的信号触发差异 ---
    // DelayedPopup：快速点击触发 clicked，长按不触发
    connect(m_delayedBtn, &QToolButton::clicked, this, [this]() {
        appendLog(QStringLiteral("[DelayedPopup] clicked 触发（快速点击）"));
    });
    connect(m_delayedBtn, &QToolButton::pressed, this, [this]() {
        appendLog(QStringLiteral("[DelayedPopup] pressed 触发"));
    });

    // InstantPopup：clicked 和 pressed 都不触发，只有菜单弹出
    connect(m_instantBtn, &QToolButton::clicked, this, [this]() {
        appendLog(QStringLiteral("[InstantPopup] clicked 触发（不应出现）"));
    });
    connect(m_instantBtn, &QToolButton::pressed, this, [this]() {
        appendLog(QStringLiteral("[InstantPopup] pressed 触发（不应出现）"));
    });

    // MenuButtonPopup：只有主体区域点击触发 clicked
    connect(m_menuBtn, &QToolButton::clicked, this, [this]() {
        appendLog(QStringLiteral("[MenuButtonPopup] clicked 触发（主体区域）"));
    });

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// 槽函数实现
// ─────────────────────────────────────────────────────────────────────────────

void ArrowToolDemo::appendLog(const QString& message)
{
    m_logEdit->append(message);
}

void ArrowToolDemo::onArrowClicked(const QString& direction)
{
    const QString msg =
        QStringLiteral("导航方向: %1").arg(direction);
    statusBar()->showMessage(msg);
    appendLog(QStringLiteral("[ArrowType] 点击方向: %1").arg(direction));
}

void ArrowToolDemo::onHistoryClicked()
{
    // DelayedPopup 模式下，快速点击触发此槽函数
    ++m_historyCount;
    appendLog(QStringLiteral("[DelayedPopup] 快速点击 -> 撤销操作 #%1")
                  .arg(m_historyCount));
}

void ArrowToolDemo::onHistoryMenuAboutToShow()
{
    // 每次弹出前动态更新菜单内容，模拟最近操作记录
    m_historyMenu->clear();
    const int kMaxHistoryItems = 10;
    const int startIdx =
        std::max(1, m_historyCount - kMaxHistoryItems + 1);

    for (int i = m_historyCount; i >= startIdx; --i) {
        m_historyMenu->addAction(
            QStringLiteral("操作记录 #%1").arg(i),
            this, [this, i]() {
                appendLog(QStringLiteral("[历史] 选择操作记录 #%1").arg(i));
            });
    }

    if (m_historyCount == 0) {
        // 禁用空菜单项，避免用户困惑
        auto* emptyAction = m_historyMenu->addAction(
            QStringLiteral("（暂无历史记录）"));
        emptyAction->setEnabled(false);
    }
}

void ArrowToolDemo::onExportClicked()
{
    // MenuButtonPopup 模式下，主体区域点击触发此槽函数
    appendLog(QStringLiteral("[MenuButtonPopup] 默认导出（主体区域点击）"));
}

void ArrowToolDemo::onExportPdf()
{
    appendLog(QStringLiteral("[导出] 选择格式: PDF（箭头区域菜单）"));
}

void ArrowToolDemo::onExportPng()
{
    appendLog(QStringLiteral("[导出] 选择格式: PNG（箭头区域菜单）"));
}

void ArrowToolDemo::onExportSvg()
{
    appendLog(QStringLiteral("[导出] 选择格式: SVG（箭头区域菜单）"));
}

void ArrowToolDemo::onToggleStyle()
{
    // 在 IconOnly 和 TextBesideIcon 之间切换，观察工具栏按钮外观变化
    // QToolBar::setToolButtonStyle 全局覆盖所有工具栏内按钮的显示风格
    if (m_iconOnlyMode) {
        m_toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        appendLog(QStringLiteral("[风格] 切换为 TextBesideIcon"));
        m_iconOnlyMode = false;
    } else {
        m_toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
        appendLog(QStringLiteral("[风格] 切换为 IconOnly"));
        m_iconOnlyMode = true;
    }

    // ArrowType 按钮在 TextBesideIcon 下仍只显示箭头（没有 text）
    // 非 ArrowType 按钮会显示 text + icon
}
