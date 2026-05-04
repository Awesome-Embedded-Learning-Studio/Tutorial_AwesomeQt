#include "tabwidgetdemowidget.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>

// ============================================================================
// TabWidgetDemoWidget: QTabWidget 综合演示窗口
// ============================================================================
TabWidgetDemoWidget::TabWidgetDemoWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QTabWidget 综合演示 — 标签页控件");
    resize(600, 500);
    initUi();
}

/// @brief 初始化界面
void TabWidgetDemoWidget::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // ================================================================
    // 顶部: 控制栏（标签栏位置、形状、可关闭、可拖动）
    // ================================================================
    auto *controlLayout = new QHBoxLayout;

    controlLayout->addWidget(new QLabel("标签栏位置:"));
    m_positionCombo = new QComboBox;
    m_positionCombo->addItems({"上方 (North)", "下方 (South)",
                               "左侧 (West)", "右侧 (East)"});
    controlLayout->addWidget(m_positionCombo);

    controlLayout->addWidget(new QLabel("标签形状:"));
    m_shapeCombo = new QComboBox;
    m_shapeCombo->addItems({"圆角 (Rounded)", "三角形 (Triangular)"});
    controlLayout->addWidget(m_shapeCombo);

    m_closableCheck = new QCheckBox("可关闭");
    m_closableCheck->setChecked(true);
    controlLayout->addWidget(m_closableCheck);

    m_movableCheck = new QCheckBox("可拖动");
    m_movableCheck->setChecked(true);
    controlLayout->addWidget(m_movableCheck);

    controlLayout->addStretch();
    mainLayout->addLayout(controlLayout);

    // ================================================================
    // 中央: QTabWidget
    // ================================================================
    m_tabWidget = new QTabWidget;
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);

    // 标签页 1: 常规设置
    auto *generalPage = createGeneralPage();
    m_tabWidget->addTab(generalPage, "常规");

    // 标签页 2: 网络设置
    auto *networkPage = createNetworkPage();
    m_tabWidget->addTab(networkPage, "网络");

    // 标签页 3: 外观设置
    auto *appearancePage = createAppearancePage();
    m_tabWidget->addTab(appearancePage, "外观");

    // 标签页 4: 高级设置（默认禁用）
    auto *advancedPage = createAdvancedPage();
    m_tabWidget->addTab(advancedPage, "高级");
    m_tabWidget->setTabEnabled(3, false);

    // 设置标签图标和提示
    m_tabWidget->setTabIcon(0, QIcon::fromTheme("preferences-system"));
    m_tabWidget->setTabToolTip(0, "常规设置：语言、启动选项");
    m_tabWidget->setTabIcon(1, QIcon::fromTheme("network-wired"));
    m_tabWidget->setTabToolTip(1, "网络设置：代理、DNS");
    m_tabWidget->setTabIcon(2, QIcon::fromTheme("preferences-desktop"));
    m_tabWidget->setTabToolTip(2, "外观设置：主题、字体");
    m_tabWidget->setTabIcon(3, QIcon::fromTheme("preferences-other"));
    m_tabWidget->setTabToolTip(3, "高级设置（已锁定）");

    mainLayout->addWidget(m_tabWidget, 1);

    // ================================================================
    // 底部: 解锁高级选项 + 新增标签页按钮 + 状态栏
    // ================================================================
    auto *bottomLayout = new QHBoxLayout;

    m_unlockCheck = new QCheckBox("解锁高级选项");
    bottomLayout->addWidget(m_unlockCheck);

    auto *addTabBtn = new QPushButton("新增标签页");
    addTabBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #388E3C; color: white;"
        "  border: none; border-radius: 4px;"
        "  padding: 6px 16px;"
        "}"
        "QPushButton:hover { background-color: #2E7D32; }");
    bottomLayout->addWidget(addTabBtn);

    bottomLayout->addStretch();

    m_statusLabel = new QLabel;
    m_statusLabel->setStyleSheet(
        "color: #555; font-size: 11px;"
        "padding: 4px 8px;"
        "background-color: #F5F5F5;"
        "border: 1px solid #E0E0E0;"
        "border-radius: 4px;");
    bottomLayout->addWidget(m_statusLabel, 1);

    mainLayout->addLayout(bottomLayout);

    // ================================================================
    // 信号连接
    // ================================================================

    // 标签切换 → 更新状态栏
    connect(m_tabWidget, &QTabWidget::currentChanged,
            this, [this](int index) {
        if (index < 0) {
            m_statusLabel->setText("所有标签页已关闭");
            return;
        }
        QString name = m_tabWidget->tabText(index);
        bool enabled = m_tabWidget->isTabEnabled(index);
        m_statusLabel->setText(
            QString("当前页面: %1 (索引 %2) %3")
                .arg(name)
                .arg(index)
                .arg(enabled ? "" : "[已禁用]"));
    });

    // 关闭标签页
    connect(m_tabWidget, &QTabWidget::tabCloseRequested,
            this, [this](int index) {
        QWidget *page = m_tabWidget->widget(index);
        QString name = m_tabWidget->tabText(index);

        auto result = QMessageBox::question(
            this, "关闭标签页",
            QString("确定关闭「%1」标签页吗？").arg(name),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

        if (result == QMessageBox::Yes) {
            m_tabWidget->removeTab(index);
            delete page;
        }
    });

    // 标签栏位置切换
    connect(m_positionCombo, &QComboBox::currentIndexChanged,
            this, [this](int index) {
        m_tabWidget->setTabPosition(
            static_cast<QTabWidget::TabPosition>(index));
    });

    // 标签形状切换
    connect(m_shapeCombo, &QComboBox::currentIndexChanged,
            this, [this](int index) {
        m_tabWidget->setTabShape(
            static_cast<QTabWidget::TabShape>(index));
    });

    // 可关闭切换
    connect(m_closableCheck, &QCheckBox::toggled,
            m_tabWidget, &QTabWidget::setTabsClosable);

    // 可拖动切换
    connect(m_movableCheck, &QCheckBox::toggled,
            m_tabWidget, &QTabWidget::setMovable);

    // 解锁高级选项
    connect(m_unlockCheck, &QCheckBox::toggled,
            this, [this](bool checked) {
        m_tabWidget->setTabEnabled(3, checked);
        m_tabWidget->setTabToolTip(
            3, checked ? "高级设置"
                       : "高级设置（已锁定）");
    });

    // 新增标签页
    connect(addTabBtn, &QPushButton::clicked,
            this, [this]() {
        int count = m_tabWidget->count();
        auto *newPage = new QWidget;
        auto *layout = new QVBoxLayout(newPage);

        auto *infoLabel = new QLabel(
            QString("这是动态添加的第 %1 个标签页").arg(count + 1));
        infoLabel->setAlignment(Qt::AlignCenter);
        infoLabel->setStyleSheet(
            "font-size: 14px; color: #666; padding: 20px;");
        layout->addWidget(infoLabel);

        auto *contentEdit = new QTextEdit;
        contentEdit->setPlaceholderText("在此输入内容...");
        layout->addWidget(contentEdit);

        QString tabName = QString("标签 %1").arg(count + 1);
        int idx = m_tabWidget->addTab(newPage, tabName);
        m_tabWidget->setTabToolTip(
            idx, QString("动态创建的标签页 #%1").arg(count + 1));

        // 自动切换到新标签页
        m_tabWidget->setCurrentIndex(idx);
    });

    // 初始化状态栏
    m_statusLabel->setText(
        "当前页面: 常规 (索引 0)");
}

/// @brief 创建"常规设置"页面
QWidget *TabWidgetDemoWidget::createGeneralPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setSpacing(10);

    auto *nameRow = new QHBoxLayout;
    auto *nameLabel = new QLabel("用户名:");
    nameLabel->setMinimumWidth(80);
    auto *nameEdit = new QLineEdit;
    nameEdit->setPlaceholderText("请输入用户名");
    nameRow->addWidget(nameLabel);
    nameRow->addWidget(nameEdit, 1);
    layout->addLayout(nameRow);

    auto *langRow = new QHBoxLayout;
    auto *langLabel = new QLabel("界面语言:");
    langLabel->setMinimumWidth(80);
    auto *langCombo = new QComboBox;
    langCombo->addItems({"简体中文", "English", "日本語"});
    langRow->addWidget(langLabel);
    langRow->addWidget(langCombo, 1);
    layout->addLayout(langRow);

    auto *autoStartCheck = new QCheckBox("开机自动启动");
    layout->addWidget(autoStartCheck);

    layout->addStretch();
    return page;
}

/// @brief 创建"网络设置"页面
QWidget *TabWidgetDemoWidget::createNetworkPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setSpacing(10);

    auto *proxyRow = new QHBoxLayout;
    auto *proxyLabel = new QLabel("代理地址:");
    proxyLabel->setMinimumWidth(80);
    auto *proxyEdit = new QLineEdit;
    proxyEdit->setPlaceholderText("例如: 127.0.0.1");
    proxyRow->addWidget(proxyLabel);
    proxyRow->addWidget(proxyEdit, 1);
    layout->addLayout(proxyRow);

    auto *portRow = new QHBoxLayout;
    auto *portLabel = new QLabel("端口号:");
    portLabel->setMinimumWidth(80);
    auto *portSpin = new QSpinBox;
    portSpin->setRange(1, 65535);
    portSpin->setValue(8080);
    portRow->addWidget(portLabel);
    portRow->addWidget(portSpin, 1);
    layout->addLayout(portRow);

    auto *timeoutRow = new QHBoxLayout;
    auto *timeoutLabel = new QLabel("超时(秒):");
    timeoutLabel->setMinimumWidth(80);
    auto *timeoutSpin = new QSpinBox;
    timeoutSpin->setRange(1, 300);
    timeoutSpin->setValue(30);
    timeoutRow->addWidget(timeoutLabel);
    timeoutRow->addWidget(timeoutSpin, 1);
    layout->addLayout(timeoutRow);

    layout->addStretch();
    return page;
}

/// @brief 创建"外观设置"页面
QWidget *TabWidgetDemoWidget::createAppearancePage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setSpacing(10);

    auto *darkCheck = new QCheckBox("深色模式");
    layout->addWidget(darkCheck);

    auto *sysThemeCheck = new QCheckBox("跟随系统主题");
    sysThemeCheck->setChecked(true);
    layout->addWidget(sysThemeCheck);

    auto *fontRow = new QHBoxLayout;
    auto *fontLabel = new QLabel("字体大小:");
    fontLabel->setMinimumWidth(80);
    auto *fontSpin = new QSpinBox;
    fontSpin->setRange(8, 36);
    fontSpin->setValue(14);
    fontSpin->setSuffix(" px");
    fontRow->addWidget(fontLabel);
    fontRow->addWidget(fontSpin, 1);
    layout->addLayout(fontRow);

    layout->addStretch();
    return page;
}

/// @brief 创建"高级设置"页面
QWidget *TabWidgetDemoWidget::createAdvancedPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setSpacing(10);

    auto *debugCheck = new QCheckBox("启用调试模式");
    layout->addWidget(debugCheck);

    auto *logCheck = new QCheckBox("启用详细日志");
    layout->addWidget(logCheck);

    auto *warnLabel = new QLabel(
        "高级选项仅供开发者使用，修改可能导致应用行为异常。");
    warnLabel->setStyleSheet(
        "color: #E65100; font-size: 12px; padding: 8px;"
        "background-color: #FFF3E0;"
        "border: 1px solid #FFCC80;"
        "border-radius: 4px;");
    warnLabel->setWordWrap(true);
    layout->addWidget(warnLabel);

    layout->addStretch();
    return page;
}
