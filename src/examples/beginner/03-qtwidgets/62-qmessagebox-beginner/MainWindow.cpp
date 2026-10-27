#include "MainWindow.h"

// ============================================================================
// MainWindow: 演示四种 QMessageBox + 线程安全触发
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QMessageBox 消息对话框演示");
    resize(550, 500);

    auto *central = new QWidget;
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);

    // ---- 按钮区域 ----
    auto *btnLayout = new QHBoxLayout;

    auto *infoBtn = new QPushButton("信息提示");
    auto *warnBtn = new QPushButton("警告提示");
    auto *critBtn = new QPushButton("严重错误");
    auto *questionBtn = new QPushButton("确认操作");
    auto *bgBtn = new QPushButton("模拟后台任务");

    for (auto *btn : {infoBtn, warnBtn, critBtn,
                      questionBtn, bgBtn}) {
        btn->setMinimumHeight(36);
    }

    btnLayout->addWidget(infoBtn);
    btnLayout->addWidget(warnBtn);
    btnLayout->addWidget(critBtn);
    btnLayout->addWidget(questionBtn);
    btnLayout->addWidget(bgBtn);
    mainLayout->addLayout(btnLayout);

    // ---- 结果显示 ----
    auto *resultLabel = new QLabel("操作结果:");
    mainLayout->addWidget(resultLabel);

    m_resultEdit = new QTextEdit;
    m_resultEdit->setReadOnly(true);
    m_resultEdit->setPlaceholderText(
        "点击上方按钮触发不同的消息对话框...");
    mainLayout->addWidget(m_resultEdit);

    // ---- 信号连接 ----
    connect(infoBtn, &QPushButton::clicked,
            this, &MainWindow::onInformation);
    connect(warnBtn, &QPushButton::clicked,
            this, &MainWindow::onWarning);
    connect(critBtn, &QPushButton::clicked,
            this, &MainWindow::onCritical);
    connect(questionBtn, &QPushButton::clicked,
            this, &MainWindow::onQuestion);
    connect(bgBtn, &QPushButton::clicked,
            this, &MainWindow::onBackgroundTask);
}

// ====================================================================
// information: 纯信息提示
// ====================================================================
void MainWindow::onInformation()
{
    QMessageBox::information(
        this, "操作完成",
        "文件已成功保存到指定路径。");
    m_resultEdit->append("[信息] 用户已确认信息提示");
}

// ====================================================================
// warning: 警告 + 用户选择
// ====================================================================
void MainWindow::onWarning()
{
    auto result = QMessageBox::warning(
        this, "磁盘空间不足",
        "当前磁盘剩余空间低于 500MB，\n"
        "部分功能可能无法正常使用。\n\n"
        "是否继续操作？",
        QMessageBox::Ignore |
        QMessageBox::Abort,
        QMessageBox::Abort);  // 默认聚焦 Abort

    if (result == QMessageBox::Ignore) {
        m_resultEdit->append(
            "[警告] 用户选择继续操作（忽略警告）");
    } else {
        m_resultEdit->append(
            "[警告] 用户选择中止操作");
    }
}

// ====================================================================
// critical: 严重错误 + setDetailedText
// ====================================================================
void MainWindow::onCritical()
{
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowTitle("数据库连接失败");
    msgBox.setText("无法连接到数据库服务器。");
    msgBox.setInformativeText(
        "请检查网络连接和数据库配置，\n"
        "然后点击\"重试\"重新连接。");
    msgBox.setDetailedText(
        "连接详情:\n"
        "--------\n"
        "主机: db.example.com\n"
        "端口: 5432\n"
        "数据库名: production_db\n"
        "用户: app_user\n"
        "超时: 30000ms\n"
        "--------\n"
        "错误码: CONNECTION_TIMEOUT\n"
        "错误信息: Connection refused (ECONNREFUSED)\n"
        "OS 错误码: 111\n"
        "最后尝试时间: 2025-04-22 10:30:45\n"
        "--------\n"
        "已重试次数: 3/3");
    msgBox.setStandardButtons(
        QMessageBox::Retry | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);

    int result = msgBox.exec();
    if (result == QMessageBox::Retry) {
        m_resultEdit->append(
            "[严重] 用户选择重试连接");
    } else {
        m_resultEdit->append(
            "[严重] 用户取消连接");
    }
}

// ====================================================================
// question: 确认操作 + 自定义按钮文字
// ====================================================================
void MainWindow::onQuestion()
{
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle("确认删除");
    msgBox.setText("确定要删除选中的 5 个文件吗？");
    msgBox.setInformativeText(
        "此操作不可撤销，删除后无法恢复文件。");

    // 标准按钮 + 自定义文字
    msgBox.setStandardButtons(
        QMessageBox::Yes | QMessageBox::No);
    msgBox.button(QMessageBox::Yes)->setText("删除");
    msgBox.button(QMessageBox::No)->setText("保留");
    // 默认聚焦在"保留"——防止误操作
    msgBox.setDefaultButton(QMessageBox::No);

    int result = msgBox.exec();
    if (result == QMessageBox::Yes) {
        m_resultEdit->append(
            "[确认] 用户确认删除文件");
    } else {
        m_resultEdit->append(
            "[确认] 用户保留文件");
    }
}

// ====================================================================
// 模拟后台任务: QTimer + QMetaObject::invokeMethod
// ====================================================================
void MainWindow::onBackgroundTask()
{
    m_resultEdit->append(
        "[后台] 后台任务已启动，2 秒后完成...");
    bgBtnSetEnabled(false);

    // 用 QTimer 模拟工作线程
    QTimer::singleShot(2000, this, [this]() {
        // 模拟：在"工作线程"中通过 invokeMethod
        // 将 QMessageBox 投递到主线程
        // 这里本身就在主线程，所以直接弹出即可
        // 下面演示 invokeMethod 的用法
        QMetaObject::invokeMethod(
            qApp,
            [this]() {
                QMessageBox::information(
                    nullptr,
                    "后台任务完成",
                    "数据同步已完成，共处理 128 条记录。\n"
                    "成功: 126 条\n"
                    "跳过: 2 条（重复数据）");
            },
            Qt::QueuedConnection);

        m_resultEdit->append(
            "[后台] 任务完成，已通知用户");
        bgBtnSetEnabled(true);
    });
}

void MainWindow::bgBtnSetEnabled(bool enabled)
{
    // 查找"模拟后台任务"按钮
    auto buttons = centralWidget()->findChildren<
        QPushButton*>();
    for (auto *btn : buttons) {
        if (btn->text().contains("后台")) {
            btn->setEnabled(enabled);
        }
    }
}
