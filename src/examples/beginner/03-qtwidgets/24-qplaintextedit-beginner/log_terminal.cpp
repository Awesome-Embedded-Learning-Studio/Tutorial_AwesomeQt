#include "log_terminal.h"

#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTimer>
#include <QTime>
#include <QRandomGenerator>
#include <QTextCursor>

LogTerminal::LogTerminal(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QPlainTextEdit 综合演示 — 模拟日志终端");
    resize(680, 520);
    initUi();

    // 初始追加几条日志
    appendLogEntry("[INFO] 日志终端已就绪");
    appendLogEntry("[INFO] 点击「开始日志」按钮启动模拟输出");
}

/// @brief 初始化界面
void LogTerminal::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ---- 标题 ----
    auto *titleLabel = new QLabel("模拟日志终端");
    titleLabel->setFont(QFont("Arial", 15, QFont::Bold));
    mainLayout->addWidget(titleLabel);

    // ================================================================
    // 工具栏: 控制按钮 + 行数限制选择
    // ================================================================
    auto *toolbarGroup = new QGroupBox("控制面板");
    auto *toolbarLayout = new QHBoxLayout(toolbarGroup);
    toolbarLayout->setSpacing(8);

    // 开始日志按钮
    m_startBtn = new QPushButton("开始日志");
    m_startBtn->setAutoDefault(false);
    connect(m_startBtn, &QPushButton::clicked, this,
            &LogTerminal::startLogging);

    // 停止日志按钮
    m_stopBtn = new QPushButton("停止日志");
    m_stopBtn->setAutoDefault(false);
    m_stopBtn->setEnabled(false);
    connect(m_stopBtn, &QPushButton::clicked, this,
            &LogTerminal::stopLogging);

    // 清空日志按钮
    auto *clearBtn = new QPushButton("清空日志");
    clearBtn->setAutoDefault(false);
    connect(clearBtn, &QPushButton::clicked, this, [this]() {
        m_logView->clear();
        updateLineCount();
    });

    // 行数限制选择
    auto *limitLabel = new QLabel("行数上限:");
    m_limitCombo = new QComboBox();
    m_limitCombo->addItem("100 行", 100);
    m_limitCombo->addItem("500 行", 500);
    m_limitCombo->addItem("1000 行", 1000);
    m_limitCombo->addItem("无限制", 0);
    m_limitCombo->setCurrentIndex(2);  // 默认 1000 行
    connect(m_limitCombo, &QComboBox::currentIndexChanged, this,
            &LogTerminal::onLimitChanged);

    toolbarLayout->addWidget(m_startBtn);
    toolbarLayout->addWidget(m_stopBtn);
    toolbarLayout->addWidget(clearBtn);
    toolbarLayout->addSpacing(20);
    toolbarLayout->addWidget(limitLabel);
    toolbarLayout->addWidget(m_limitCombo);
    toolbarLayout->addStretch();

    mainLayout->addWidget(toolbarGroup);

    // ================================================================
    // 日志显示区域: QPlainTextEdit + 当前行高亮
    // ================================================================
    m_logView = new HighlightPlainTextEdit();
    m_logView->setReadOnly(true);
    m_logView->setMaximumBlockCount(1000);
    m_logView->setLineWrapMode(QPlainTextEdit::NoWrap);

    // 等宽字体，适合日志显示
    QFont monoFont("Monospace", 10);
    monoFont.setStyleHint(QFont::Monospace);
    m_logView->setFont(monoFont);

    mainLayout->addWidget(m_logView, 1);

    // ================================================================
    // 状态栏: 当前行数 / 最大行数
    // ================================================================
    auto *statusLayout = new QHBoxLayout();

    m_lineCountLabel = new QLabel("行数: 0");
    m_limitLabel = new QLabel("上限: 1000 行");
    m_limitLabel->setStyleSheet("color: #888; font-size: 11px;");
    m_lineCountLabel->setStyleSheet(
        "color: #666; font-size: 11px;");

    statusLayout->addWidget(m_lineCountLabel);
    statusLayout->addWidget(m_limitLabel);
    statusLayout->addStretch();

    mainLayout->addLayout(statusLayout);

    // ---- 信号连接 ----
    connect(m_logView, &QPlainTextEdit::textChanged, this,
            &LogTerminal::updateLineCount);

    // 初始应用行数限制
    onLimitChanged(m_limitCombo->currentIndex());
}

/// @brief 开始模拟日志输出
void LogTerminal::startLogging()
{
    m_startBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);

    // 定时器每 300ms 追加一行日志
    if (!m_timer) {
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this,
                &LogTerminal::onTimerTick);
    }
    m_timer->start(300);
}

/// @brief 停止模拟日志输出
void LogTerminal::stopLogging()
{
    m_startBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);

    if (m_timer) {
        m_timer->stop();
    }

    appendLogEntry("[INFO] 日志输出已暂停");
}

/// @brief 定时器回调: 随机生成一条日志并追加
void LogTerminal::onTimerTick()
{
    // 随机选择日志级别
    const char *levels[] = {"INFO", "WARN", "ERROR"};

    const char *messages[] = {
        "数据库查询完成 (耗时 12ms)",
        "接收到新的 TCP 连接请求",
        "缓存命中率: 94.2%",
        "定时任务 heartbeat 执行成功",
        "配置热更新已生效",
        "GC 回收了 128KB 内存",
        "磁盘使用率达到 78%",
        "请求队列中有 3 个待处理任务",
        "SSL 证书将在 30 天后过期",
        "用户会话已超时，自动注销",
    };

    int levelIdx = QRandomGenerator::global()->bounded(3);
    int msgIdx = QRandomGenerator::global()->bounded(10);

    QString timestamp = QTime::currentTime().toString("HH:mm:ss.zzz");
    QString logLine = QString("[%1] [%2] %3")
        .arg(timestamp)
        .arg(levels[levelIdx])
        .arg(messages[msgIdx]);

    appendLogEntry(logLine);
}

/// @brief 追加一条日志并自动滚动到底部
void LogTerminal::appendLogEntry(const QString &text)
{
    m_logView->appendPlainText(text);

    // 自动滚动到文档底部
    QTextCursor cursor = m_logView->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_logView->setTextCursor(cursor);
}

/// @brief 行数限制选择变化时更新 QPlainTextEdit 的上限
void LogTerminal::onLimitChanged(int index)
{
    int limit = m_limitCombo->itemData(index).toInt();
    m_logView->setMaximumBlockCount(limit);

    if (limit == 0) {
        m_limitLabel->setText("上限: 无限制");
    } else {
        m_limitLabel->setText(QString("上限: %1 行").arg(limit));
    }

    updateLineCount();
}

/// @brief 更新状态栏的当前行数显示
void LogTerminal::updateLineCount()
{
    int blockCount = m_logView->document()->blockCount();
    m_lineCountLabel->setText(QString("行数: %1").arg(blockCount));
}
