#include "StackedWidgetDemoWidget.h"

// ============================================================================
// StackedWidgetDemoWidget: QStackedWidget 综合演示窗口
// ============================================================================
StackedWidgetDemoWidget::StackedWidgetDemoWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("系统偏好设置");
    resize(700, 480);
    initUi();
}

/// @brief 初始化界面
void StackedWidgetDemoWidget::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // ================================================================
    // 顶部: QComboBox 快速跳转
    // ================================================================
    auto *topLayout = new QHBoxLayout;
    topLayout->addWidget(new QLabel("快速跳转:"));

    m_jumpCombo = new QComboBox;
    m_jumpCombo->addItems(
        {"个人信息", "网络设置", "外观偏好", "快捷键", "关于"});
    topLayout->addWidget(m_jumpCombo, 1);
    topLayout->addStretch();

    mainLayout->addLayout(topLayout);

    // ================================================================
    // 中央: QListWidget 导航 + QStackedWidget 内容
    // ================================================================
    auto *contentLayout = new QHBoxLayout;
    contentLayout->setSpacing(0);

    // 左侧导航列表
    m_navList = new QListWidget;
    m_navList->setMaximumWidth(180);
    m_navList->setMinimumWidth(140);
    m_navList->addItems(
        {"个人信息", "网络设置", "外观偏好", "快捷键", "关于"});

    // 导航列表项的图标
    m_navList->item(0)->setIcon(QIcon::fromTheme("user-available"));
    m_navList->item(1)->setIcon(QIcon::fromTheme("network-wired"));
    m_navList->item(2)->setIcon(QIcon::fromTheme("preferences-desktop"));
    m_navList->item(3)->setIcon(QIcon::fromTheme("preferences-desktop-keyboard"));
    m_navList->item(4)->setIcon(QIcon::fromTheme("help-about"));

    contentLayout->addWidget(m_navList);

    // 右侧堆叠页面
    m_stackedWidget = new QStackedWidget;
    m_stackedWidget->addWidget(createPersonalPage());
    m_stackedWidget->addWidget(createNetworkPage());
    m_stackedWidget->addWidget(createAppearancePage());
    m_stackedWidget->addWidget(createShortcutPage());
    m_stackedWidget->addWidget(createAboutPage());

    contentLayout->addWidget(m_stackedWidget, 1);

    mainLayout->addLayout(contentLayout, 1);

    // ================================================================
    // 底部: 状态栏
    // ================================================================
    m_statusLabel = new QLabel;
    m_statusLabel->setStyleSheet(
        "color: #555; font-size: 11px; padding: 4px 8px;"
        "background-color: #F5F5F5;"
        "border: 1px solid #E0E0E0;"
        "border-radius: 4px;");
    mainLayout->addWidget(m_statusLabel);

    // ================================================================
    // 信号连接
    // ================================================================

    // QListWidget 导航 → 切换页面 + 同步 QComboBox
    connect(m_navList, &QListWidget::currentRowChanged,
            this, [this](int row) {
        if (row < 0) return;

        m_stackedWidget->setCurrentIndex(row);

        // 同步 QComboBox（屏蔽信号避免循环）
        m_jumpCombo->blockSignals(true);
        m_jumpCombo->setCurrentIndex(row);
        m_jumpCombo->blockSignals(false);

        updateWindowTitle(row);
        updateStatus(row);
    });

    // QComboBox 快速跳转 → 切换页面 + 同步 QListWidget
    connect(m_jumpCombo, &QComboBox::currentIndexChanged,
            this, [this](int index) {
        if (index < 0) return;

        m_stackedWidget->setCurrentIndex(index);

        // 同步 QListWidget（屏蔽信号避免循环）
        m_navList->blockSignals(true);
        m_navList->setCurrentRow(index);
        m_navList->blockSignals(false);

        updateWindowTitle(index);
        updateStatus(index);
    });

    // QStackedWidget 页面切换 → 更新状态
    connect(m_stackedWidget, &QStackedWidget::currentChanged,
            this, [this](int index) {
        if (index < 0) return;
        updateStatus(index);
    });

    // 默认选中第一项
    m_navList->setCurrentRow(0);
}

/// @brief 创建"个人信息"页面
QWidget *StackedWidgetDemoWidget::createPersonalPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setSpacing(12);
    layout->setContentsMargins(20, 16, 20, 16);

    auto *titleLabel = new QLabel("个人信息");
    titleLabel->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #1976D2;"
        "border-bottom: 2px solid #1976D2; padding-bottom: 6px;");
    layout->addWidget(titleLabel);

    // 用户名
    auto *nameRow = new QHBoxLayout;
    auto *nameLabel = new QLabel("用户名:");
    nameLabel->setMinimumWidth(80);
    auto *nameEdit = new QLineEdit;
    nameEdit->setPlaceholderText("请输入用户名");
    nameRow->addWidget(nameLabel);
    nameRow->addWidget(nameEdit, 1);
    layout->addLayout(nameRow);

    // 邮箱
    auto *emailRow = new QHBoxLayout;
    auto *emailLabel = new QLabel("邮箱地址:");
    emailLabel->setMinimumWidth(80);
    auto *emailEdit = new QLineEdit;
    emailEdit->setPlaceholderText("example@email.com");
    emailRow->addWidget(emailLabel);
    emailRow->addWidget(emailEdit, 1);
    layout->addLayout(emailRow);

    layout->addStretch();
    return page;
}

/// @brief 创建"网络设置"页面
QWidget *StackedWidgetDemoWidget::createNetworkPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setSpacing(12);
    layout->setContentsMargins(20, 16, 20, 16);

    auto *titleLabel = new QLabel("网络设置");
    titleLabel->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #1976D2;"
        "border-bottom: 2px solid #1976D2; padding-bottom: 6px;");
    layout->addWidget(titleLabel);

    auto *proxyCheck = new QCheckBox("启用代理服务器");
    layout->addWidget(proxyCheck);

    // 代理地址
    auto *proxyRow = new QHBoxLayout;
    auto *proxyLabel = new QLabel("代理地址:");
    proxyLabel->setMinimumWidth(80);
    auto *proxyEdit = new QLineEdit;
    proxyEdit->setPlaceholderText("127.0.0.1");
    proxyRow->addWidget(proxyLabel);
    proxyRow->addWidget(proxyEdit, 1);
    layout->addLayout(proxyRow);

    // 端口
    auto *portRow = new QHBoxLayout;
    auto *portLabel = new QLabel("端口号:");
    portLabel->setMinimumWidth(80);
    auto *portSpin = new QSpinBox;
    portSpin->setRange(1, 65535);
    portSpin->setValue(8080);
    portRow->addWidget(portLabel);
    portRow->addWidget(portSpin, 1);
    layout->addLayout(portRow);

    // 超时
    auto *timeoutRow = new QHBoxLayout;
    auto *timeoutLabel = new QLabel("连接超时:");
    timeoutLabel->setMinimumWidth(80);
    auto *timeoutSpin = new QSpinBox;
    timeoutSpin->setRange(1, 300);
    timeoutSpin->setValue(30);
    timeoutSpin->setSuffix(" 秒");
    timeoutRow->addWidget(timeoutLabel);
    timeoutRow->addWidget(timeoutSpin, 1);
    layout->addLayout(timeoutRow);

    layout->addStretch();
    return page;
}

/// @brief 创建"外观偏好"页面
QWidget *StackedWidgetDemoWidget::createAppearancePage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setSpacing(12);
    layout->setContentsMargins(20, 16, 20, 16);

    auto *titleLabel = new QLabel("外观偏好");
    titleLabel->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #1976D2;"
        "border-bottom: 2px solid #1976D2; padding-bottom: 6px;");
    layout->addWidget(titleLabel);

    auto *darkCheck = new QCheckBox("启用深色模式");
    layout->addWidget(darkCheck);

    auto *sysCheck = new QCheckBox("跟随系统主题");
    sysCheck->setChecked(true);
    layout->addWidget(sysCheck);

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

/// @brief 创建"快捷键"页面
QWidget *StackedWidgetDemoWidget::createShortcutPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setSpacing(12);
    layout->setContentsMargins(20, 16, 20, 16);

    auto *titleLabel = new QLabel("快捷键设置");
    titleLabel->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #1976D2;"
        "border-bottom: 2px solid #1976D2; padding-bottom: 6px;");
    layout->addWidget(titleLabel);

    auto *noticeLabel = new QLabel(
        "此功能正在开发中，将在后续版本中提供。\n\n"
        "计划支持的功能：\n"
        "- 自定义全局快捷键绑定\n"
        "- 快捷键冲突检测\n"
        "- 导入/导出快捷键配置");
    noticeLabel->setStyleSheet(
        "color: #666; font-size: 13px; padding: 12px;"
        "background-color: #FFF3E0;"
        "border: 1px solid #FFCC80;"
        "border-radius: 4px;");
    noticeLabel->setWordWrap(true);
    layout->addWidget(noticeLabel);

    layout->addStretch();
    return page;
}

/// @brief 创建"关于"页面
QWidget *StackedWidgetDemoWidget::createAboutPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setSpacing(12);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setAlignment(Qt::AlignCenter);

    auto *titleLabel = new QLabel("系统偏好设置");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 20px; font-weight: bold; color: #1976D2;");
    layout->addWidget(titleLabel);

    auto *versionLabel = new QLabel("版本 1.0.0");
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setStyleSheet(
        "font-size: 14px; color: #666;");
    layout->addWidget(versionLabel);

    auto *descLabel = new QLabel(
        "QStackedWidget 综合演示程序\n\n"
        "基于 Qt 6.9.1 / CMake 3.26+ / C++17 构建\n"
        "展示了 QStackedWidget 与导航控件的组合使用方式");
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet(
        "font-size: 12px; color: #888; padding: 16px;"
        "background-color: #F5F5F5;"
        "border: 1px solid #E0E0E0;"
        "border-radius: 6px;");
    layout->addWidget(descLabel);

    auto *copyLabel = new QLabel(
        "Copyright 2025 Tutorial AwesomeQt");
    copyLabel->setAlignment(Qt::AlignCenter);
    copyLabel->setStyleSheet("font-size: 11px; color: #AAA;");
    layout->addWidget(copyLabel);

    return page;
}

/// @brief 根据当前页面更新窗口标题
void StackedWidgetDemoWidget::updateWindowTitle(int index)
{
    static const QStringList kPageNames = {
        "个人信息", "网络设置", "外观偏好", "快捷键", "关于"};
    if (index >= 0 && index < kPageNames.size()) {
        setWindowTitle(
            QString("系统偏好设置 — %1").arg(kPageNames[index]));
    }
}

/// @brief 更新底部状态栏
void StackedWidgetDemoWidget::updateStatus(int index)
{
    static const QStringList kPageNames = {
        "个人信息", "网络设置", "外观偏好", "快捷键", "关于"};
    static const QStringList kPageDescs = {
        "管理用户名和邮箱地址",
        "配置代理服务器和网络参数",
        "选择主题风格和字体大小",
        "自定义键盘快捷键（开发中）",
        "版本信息和版权声明"};
    if (index >= 0 && index < kPageNames.size()) {
        m_statusLabel->setText(
            QString("当前页面: %1 — %2 | 第 %3/%4 页")
                .arg(kPageNames[index])
                .arg(kPageDescs[index])
                .arg(index + 1)
                .arg(m_stackedWidget->count()));
    }
}
