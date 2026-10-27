#include "mainwindow.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSettings>
#include <QSlider>
#include <QSplitter>
#include <QStatusBar>
#include <QSpinBox>
#include <QTextEdit>
#include <QToolBox>
#include <QVBoxLayout>
#include <QWidget>

// ============================================================================
// MainWindow: QToolBox 综合演示主窗口
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QToolBox 综合演示 — 系统控制面板");
    resize(800, 520);
    initUi();
    restoreToolBoxState();
}

MainWindow::~MainWindow() { saveToolBoxState(); }

/// @brief 初始化界面
void MainWindow::initUi()
{
    // ================================================================
    // 主布局：水平 QSplitter（左侧 QToolBox | 右侧日志区）
    // ================================================================
    auto *splitter = new QSplitter(Qt::Horizontal);

    // ---- 左侧：QToolBox 折叠面板 ----
    m_toolbox = new QToolBox;
    m_toolbox->setMinimumWidth(200);

    m_toolbox->addItem(createDisplayPage(), "显示设置");
    m_toolbox->addItem(createNetworkPage(), "网络配置");
    m_toolbox->addItem(createSoundPage(), "声音设置");
    m_toolbox->addItem(createAdvancedPage(), "高级选项");

    // 默认禁用"高级选项"面板（索引 3）
    m_toolbox->setItemEnabled(3, false);

    // 连接 currentChanged 信号
    connect(m_toolbox, &QToolBox::currentChanged,
            this, &MainWindow::onToolBoxCurrentChanged);

    // ---- 右侧：日志区域 ----
    m_logArea = new QTextEdit;
    m_logArea->setReadOnly(true);
    m_logArea->setPlaceholderText("面板切换日志会显示在这里...");

    splitter->addWidget(m_toolbox);
    splitter->addWidget(m_logArea);
    splitter->setSizes({240, 560});
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);

    setCentralWidget(splitter);

    // ================================================================
    // 底部：启用高级选项的复选框
    // ================================================================
    auto *advancedCheck = new QCheckBox("启用高级选项");
    connect(advancedCheck, &QCheckBox::toggled, this,
            &MainWindow::onAdvancedCheckToggled);
    statusBar()->addWidget(advancedCheck);

    log("QToolBox 综合演示已启动");
    log("左侧是 QToolBox 折叠面板，点击标题栏切换面板");
    log("高级选项面板当前已禁用，勾选底部复选框解锁");
}

/// @brief 创建"显示设置"面板
QWidget *MainWindow::createDisplayPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);

    auto *resLabel = new QLabel("分辨率:");
    auto *resCombo = new QComboBox;
    resCombo->addItems({"1920x1080", "2560x1440", "3840x2160"});

    auto *rateLabel = new QLabel("刷新率 (Hz):");
    auto *rateSpin = new QSpinBox;
    rateSpin->setRange(30, 240);
    rateSpin->setValue(60);

    layout->addWidget(resLabel);
    layout->addWidget(resCombo);
    layout->addWidget(rateLabel);
    layout->addWidget(rateSpin);
    layout->addStretch();

    return page;
}

/// @brief 创建"网络配置"面板
QWidget *MainWindow::createNetworkPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);

    auto *ipLabel = new QLabel("IP 地址:");
    auto *ipEdit = new QLineEdit;
    ipEdit->setPlaceholderText("192.168.1.100");
    ipEdit->setText("192.168.1.100");

    auto *maskLabel = new QLabel("子网掩码:");
    auto *maskEdit = new QLineEdit;
    maskEdit->setPlaceholderText("255.255.255.0");
    maskEdit->setText("255.255.255.0");

    layout->addWidget(ipLabel);
    layout->addWidget(ipEdit);
    layout->addWidget(maskLabel);
    layout->addWidget(maskEdit);
    layout->addStretch();

    return page;
}

/// @brief 创建"声音设置"面板
QWidget *MainWindow::createSoundPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);

    auto *volLabel = new QLabel("音量:");
    auto *volSlider = new QSlider(Qt::Horizontal);
    volSlider->setRange(0, 100);
    volSlider->setValue(75);

    auto *muteCheck = new QCheckBox("静音");

    layout->addWidget(volLabel);
    layout->addWidget(volSlider);
    layout->addWidget(muteCheck);
    layout->addStretch();

    return page;
}

/// @brief 创建"高级选项"面板
QWidget *MainWindow::createAdvancedPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);

    auto *infoLabel = new QLabel(
        "高级选项面板\n\n"
        "此面板需要勾选底部\"启用高级选项\"复选框\n"
        "才能展开访问。");
    infoLabel->setWordWrap(true);

    auto *debugCheck = new QCheckBox("启用调试模式");
    auto *logCheck = new QCheckBox("记录详细日志");

    layout->addWidget(infoLabel);
    layout->addWidget(debugCheck);
    layout->addWidget(logCheck);
    layout->addStretch();

    return page;
}

/// @brief 响应 QToolBox 面板切换
void MainWindow::onToolBoxCurrentChanged(int index)
{
    if (index < 0) {
        return;
    }
    QString name = m_toolbox->itemText(index);
    log(QString("用户切换到面板: %1 (索引 %2)").arg(name).arg(index));
}

/// @brief 响应"启用高级选项"复选框
void MainWindow::onAdvancedCheckToggled(bool checked)
{
    m_toolbox->setItemEnabled(3, checked);
    if (checked) {
        m_toolbox->setItemText(3, "高级选项");
        log("高级选项面板已解锁");
    } else {
        m_toolbox->setItemText(3, "高级选项 (已锁定)");
        log("高级选项面板已锁定");
    }
}

/// @brief 保存 QToolBox 当前展开的面板索引
void MainWindow::saveToolBoxState()
{
    QSettings settings("AwesomeQt", "QToolBoxDemo");
    settings.setValue("currentIndex", m_toolbox->currentIndex());
}

/// @brief 从 QSettings 恢复面板索引
void MainWindow::restoreToolBoxState()
{
    QSettings settings("AwesomeQt", "QToolBoxDemo");
    int savedIndex = settings.value("currentIndex", 0).toInt();
    if (savedIndex >= 0 && savedIndex < m_toolbox->count()) {
        m_toolbox->setCurrentIndex(savedIndex);
    }
}

/// @brief 在日志区域追加一行
void MainWindow::log(const QString &message)
{
    m_logArea->append(message);
}
