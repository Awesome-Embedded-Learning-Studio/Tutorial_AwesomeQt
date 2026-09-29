/// @file    main_window.cpp
/// @brief   演示 QMessageBox 自定义图标与详情区域的主窗口类实现。
///
/// 对应教程：进阶层 03-QtWidgets/62-QMessageBox 自定义图标与详情区域。
/// 实现了四种不同配置的 QMessageBox：自定义图标、详细文本、
/// 自定义按钮，以及不同严重级别的消息框。

#include "main_window.h"

#include <QFont>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_logOutput(nullptr)
{
    setWindowTitle(tr("QMessageBox Advanced Demo"));
    resize(550, 400);

    auto* centralWidget = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(centralWidget);

    // --- 按钮区域 ---
    auto* buttonLayout = new QHBoxLayout();

    auto* infoBtn = new QPushButton(tr("Information"), this);
    auto* warningBtn = new QPushButton(tr("Warning"), this);
    auto* criticalBtn = new QPushButton(tr("Critical"), this);
    auto* questionBtn = new QPushButton(tr("Question"), this);

    buttonLayout->addWidget(infoBtn);
    buttonLayout->addWidget(warningBtn);
    buttonLayout->addWidget(criticalBtn);
    buttonLayout->addWidget(questionBtn);
    mainLayout->addLayout(buttonLayout);

    // --- 日志区域 ---
    m_logOutput = new QTextEdit(this);
    m_logOutput->setReadOnly(true);
    m_logOutput->setPlaceholderText(
        tr("Click a button above to see different QMessageBox configurations."));
    mainLayout->addWidget(m_logOutput);

    setCentralWidget(centralWidget);

    // 连接按钮信号
    connect(infoBtn, &QPushButton::clicked,
            this, &MainWindow::showInformationBox);
    connect(warningBtn, &QPushButton::clicked,
            this, &MainWindow::showWarningBox);
    connect(criticalBtn, &QPushButton::clicked,
            this, &MainWindow::showCriticalBox);
    connect(questionBtn, &QPushButton::clicked,
            this, &MainWindow::showQuestionBox);
}

void MainWindow::showInformationBox()
{
    appendLog(tr("Showing Information MessageBox with custom icon..."));

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("Information"));
    msgBox.setText(tr("The operation completed successfully."));
    msgBox.setInformativeText(
        tr("All files have been saved to the specified directory."));

    // 设置程序化绘制的自定义图标
    // @note setIconPixmap() 会覆盖 setIcon() 设置的标准图标，
    //       两者不应同时使用。
    msgBox.setIconPixmap(createInfoIcon(48));

    msgBox.exec();

    appendLog(tr("Information box closed."));
}

void MainWindow::showWarningBox()
{
    appendLog(tr("Showing Warning MessageBox with detailed text..."));

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("Warning"));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(tr("Some configuration items are invalid."));
    msgBox.setInformativeText(
        tr("Would you like to use default values instead?"));

    // setDetailedText 创建可折叠的详情区域
    // @note 详细文本默认是折叠的，用户点击"Show Details"按钮展开。
    //       这适合放置技术细节（如错误日志），主文本保持简洁。
    msgBox.setDetailedText(
        tr("Invalid configuration items:\n\n"
           "  - Port: -1 (valid range: 1-65535)\n"
           "  - Timeout: 0 (must be positive)\n"
           "  - Retry count: 999 (max: 10)\n\n"
           "Default values:\n"
           "  - Port: 8080\n"
           "  - Timeout: 3000ms\n"
           "  - Retry count: 3"));

    auto result = msgBox.exec();

    if (result == QMessageBox::Ok)
    {
        appendLog(tr("User chose to use default values."));
    }
    else
    {
        appendLog(tr("User cancelled."));
    }
}

void MainWindow::showCriticalBox()
{
    appendLog(tr("Showing Critical MessageBox with custom buttons..."));

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("Critical Error"));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(tr("Failed to connect to the database server."));
    msgBox.setInformativeText(
        tr("The application cannot function without a database connection."));

    // 添加自定义按钮，每个按钮指定一个角色
    // @note 按钮角色影响布局顺序和默认行为：
    //       AcceptRole 的按钮在右侧，RejectRole 在左侧（平台相关）。
    auto* retryBtn = msgBox.addButton(
        tr("Retry"), QMessageBox::AcceptRole);
    auto* configBtn = msgBox.addButton(
        tr("Change Settings"), QMessageBox::ActionRole);
    auto* quitBtn = msgBox.addButton(
        tr("Quit Application"), QMessageBox::RejectRole);

    // 设置默认按钮（按 Enter 时触发）
    // @note 必须在 addButton 之后调用，因为需要按钮已存在。
    msgBox.setDefaultButton(retryBtn);

    msgBox.exec();

    // 通过比较按钮指针判断用户点击了哪个
    if (msgBox.clickedButton() == retryBtn)
    {
        appendLog(tr("User chose: Retry"));
    }
    else if (msgBox.clickedButton() == configBtn)
    {
        appendLog(tr("User chose: Change Settings"));
    }
    else if (msgBox.clickedButton() == quitBtn)
    {
        appendLog(tr("User chose: Quit Application"));
    }

    // 避免 unused-variable 警告
    (void)retryBtn;
    (void)configBtn;
    (void)quitBtn;
}

void MainWindow::showQuestionBox()
{
    appendLog(tr("Showing Question MessageBox with custom icon..."));

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("Confirm Action"));
    msgBox.setText(tr("Are you sure you want to delete all temporary files?"));
    msgBox.setInformativeText(
        tr("This action cannot be undone. 42 files will be permanently removed."));

    // 使用程序化绘制的问号图标
    msgBox.setIconPixmap(createWarningIcon(48));

    // 标准的 Yes/No 按钮
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    auto result = msgBox.exec();

    if (result == QMessageBox::Yes)
    {
        appendLog(tr("User confirmed: Delete all temporary files."));
    }
    else
    {
        appendLog(tr("User cancelled the deletion."));
    }
}

QPixmap MainWindow::createInfoIcon(int size) const
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 蓝色圆形背景
    painter.setBrush(QColor(52, 152, 219));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(2, 2, size - 4, size - 4);

    // 白色 "i" 字母
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPixelSize(static_cast<int>(size * 0.6));
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(QRect(0, 0, size, size), Qt::AlignCenter, "i");

    return pixmap;
}

QPixmap MainWindow::createWarningIcon(int size) const
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 黄色三角形背景
    QPainterPath triangle;
    int margin = 4;
    int centerX = size / 2;
    triangle.moveTo(centerX, margin);
    triangle.lineTo(size - margin, size - margin);
    triangle.lineTo(margin, size - margin);
    triangle.closeSubpath();

    painter.setBrush(QColor(241, 196, 15));
    painter.setPen(QPen(QColor(180, 150, 10), 2));
    painter.drawPath(triangle);

    // 深色 "!" 感叹号
    painter.setPen(QColor(80, 60, 0));
    QFont font = painter.font();
    font.setPixelSize(static_cast<int>(size * 0.45));
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(QRect(0, 0, size, size - 4), Qt::AlignCenter, "!");

    return pixmap;
}

void MainWindow::appendLog(const QString& message)
{
    m_logOutput->append(message);
}
