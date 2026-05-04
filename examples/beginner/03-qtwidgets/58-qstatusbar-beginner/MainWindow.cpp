#include "MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QSize>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QToolBar>

// ============================================================================
// MainWindow: 演示状态栏的完整配置
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QStatusBar 状态栏演示");
    resize(800, 560);

    // ---- 中央控件 ----
    m_editor = new QPlainTextEdit;
    m_editor->setPlaceholderText("在这里输入文本...");
    setCentralWidget(m_editor);

    // ---- 工具栏 ----
    setupToolBar();

    // ---- 状态栏 ----
    setupStatusBar();

    // ---- 信号连接 ----
    connect(m_editor, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::updateCursorPosition);
    connect(m_editor, &QPlainTextEdit::textChanged,
            this, &MainWindow::updateCharCount);

    // 初始状态
    updateCursorPosition();
    updateCharCount();

    // 欢迎消息（临时消息，2 秒后消失，期间会隐藏 positionLabel）
    statusBar()->showMessage("就绪 — 开始输入文本吧", 2000);
}

// ====================================================================
// 工具栏
// ====================================================================
void MainWindow::setupToolBar()
{
    auto *toolBar = addToolBar("操作");
    toolBar->setIconSize(QSize(20, 20));

    // 新建
    auto *newAction = new QAction("新建", this);
    newAction->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    connect(newAction, &QAction::triggered, this, [this]() {
        m_editor->clear();
        // 临时消息覆盖 addWidget 控件
        statusBar()->showMessage("已新建文件", 2000);
    });
    toolBar->addAction(newAction);

    // 保存
    auto *saveAction = new QAction("保存", this);
    saveAction->setIcon(
        style()->standardIcon(QStyle::SP_DialogSaveButton));
    connect(saveAction, &QAction::triggered, this, [this]() {
        statusBar()->showMessage("文件已保存", 3000);
    });
    toolBar->addAction(saveAction);

    toolBar->addSeparator();

    // 模拟加载
    auto *loadAction = new QAction("模拟加载", this);
    loadAction->setIcon(
        style()->standardIcon(QStyle::SP_BrowserReload));
    connect(loadAction, &QAction::triggered,
            this, &MainWindow::startSimulatedLoad);
    toolBar->addAction(loadAction);
}

// ====================================================================
// 状态栏
// ====================================================================
void MainWindow::setupStatusBar()
{
    // 左侧：光标位置（addWidget，会被临时消息隐藏）
    m_positionLabel = new QLabel("行 1, 列 1");
    m_positionLabel->setMinimumWidth(120);
    statusBar()->addWidget(m_positionLabel);

    // 左侧：进度条（addWidget，平时隐藏）
    m_progressBar = new QProgressBar;
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setMaximumWidth(200);
    m_progressBar->setMaximumHeight(18);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFormat("%p%");
    m_progressBar->hide();
    statusBar()->addWidget(m_progressBar);

    // 右侧：字符计数（addPermanentWidget，始终可见）
    m_charCountLabel = new QLabel("字符数: 0");
    m_charCountLabel->setMinimumWidth(80);
    statusBar()->addPermanentWidget(m_charCountLabel);

    // 右侧：编码信息（addPermanentWidget，始终可见）
    auto *encodingLabel = new QLabel("UTF-8");
    encodingLabel->setMinimumWidth(50);
    statusBar()->addPermanentWidget(encodingLabel);
}

// ====================================================================
// 模拟加载：演示进度条嵌入状态栏
// ====================================================================
void MainWindow::startSimulatedLoad()
{
    m_progressBar->setValue(0);
    m_progressBar->show();
    statusBar()->showMessage("正在加载...", 0);

    auto *timer = new QTimer(this);
    int progress = 0;

    connect(timer, &QTimer::timeout, this, [this, timer, progress]()
            mutable {
        progress += 2;
        m_progressBar->setValue(progress);

        if (progress >= 100) {
            timer->stop();
            timer->deleteLater();
            m_progressBar->hide();
            // 加载完成，显示临时消息
            statusBar()->showMessage("加载完成", 3000);
        }
    });
    timer->start(30);
}

// ====================================================================
// 状态更新
// ====================================================================
void MainWindow::updateCursorPosition()
{
    auto cursor = m_editor->textCursor();
    int line = cursor.blockNumber() + 1;
    int col = cursor.positionInBlock() + 1;
    m_positionLabel->setText(
        QString("行 %1, 列 %2").arg(line).arg(col));
}

void MainWindow::updateCharCount()
{
    int count = m_editor->toPlainText().length();
    m_charCountLabel->setText("字符数: " + QString::number(count));
}
