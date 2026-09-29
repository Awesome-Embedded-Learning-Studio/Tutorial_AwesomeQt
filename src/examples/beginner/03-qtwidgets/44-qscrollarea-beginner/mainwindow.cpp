#include "mainwindow.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QTime>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

// ============================================================================
// MainWindow: QScrollArea 综合演示主窗口（消息日志查看器）
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QScrollArea 综合演示 — 消息日志查看器");
    resize(520, 420);
    initUi();
}

/// @brief 初始化界面
void MainWindow::initUi()
{
    auto *centralWidget = new QWidget;
    auto *mainLayout = new QVBoxLayout(centralWidget);

    // ================================================================
    // 顶部输入栏
    // ================================================================
    auto *inputLayout = new QHBoxLayout;

    m_inputEdit = new QLineEdit;
    m_inputEdit->setPlaceholderText("输入消息内容，按回车或点击发送...");

    auto *sendBtn = new QPushButton("发送");
    sendBtn->setFixedWidth(80);

    inputLayout->addWidget(m_inputEdit);
    inputLayout->addWidget(sendBtn);

    // ================================================================
    // 中间滚动区域
    // ================================================================
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff);

    m_contentWidget = new QWidget;
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setAlignment(Qt::AlignTop);

    // 底部锚点：确保滚动到最底部
    m_anchor = new QWidget;
    m_anchor->setFixedHeight(1);
    m_contentLayout->addWidget(m_anchor);

    m_scrollArea->setWidget(m_contentWidget);

    // ================================================================
    // 底部操作栏
    // ================================================================
    auto *bottomLayout = new QHBoxLayout;

    auto *clearBtn = new QPushButton("清空所有消息");
    auto *countLabel = new QLabel("消息数: 0");
    m_countLabel = countLabel;

    auto *addBatchBtn = new QPushButton("批量添加 20 条");
    connect(addBatchBtn, &QPushButton::clicked, this,
            &MainWindow::onAddBatch);

    bottomLayout->addWidget(clearBtn);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_countLabel);
    bottomLayout->addWidget(addBatchBtn);

    // 组装主布局
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(m_scrollArea, 1);
    mainLayout->addLayout(bottomLayout);

    setCentralWidget(centralWidget);

    // ================================================================
    // 信号连接
    // ================================================================
    connect(sendBtn, &QPushButton::clicked, this,
            &MainWindow::onSendMessage);
    connect(m_inputEdit, &QLineEdit::returnPressed, this,
            &MainWindow::onSendMessage);
    connect(clearBtn, &QPushButton::clicked, this,
            &MainWindow::onClearMessages);

    // 初始欢迎消息
    appendSystemMessage("QScrollArea 综合演示已启动");
    appendSystemMessage("输入文字点击发送，消息会自动追加到底部并滚动");
    appendSystemMessage("滚动条样式已通过 QSS 自定义");
}

/// @brief 发送用户输入的消息
void MainWindow::onSendMessage()
{
    QString text = m_inputEdit->text().trimmed();
    if (text.isEmpty()) {
        return;
    }
    appendUserMessage(text);
    m_inputEdit->clear();
}

/// @brief 批量添加 20 条测试消息
void MainWindow::onAddBatch()
{
    for (int i = 1; i <= 20; ++i) {
        appendUserMessage(
            QString("批量测试消息 #%1 — "
                    "验证 QScrollArea 在快速追加内容时的滚动行为")
                .arg(i));
    }
}

/// @brief 追加一条用户消息
void MainWindow::appendUserMessage(const QString &text)
{
    auto *label = new QLabel(
        QString("[%1] %2")
            .arg(QTime::currentTime().toString("HH:mm:ss"))
            .arg(text));
    label->setWordWrap(true);
    label->setStyleSheet(
        "padding: 6px 10px;"
        "margin: 2px 0;"
        "background-color: #E8F0FE;"
        "border-radius: 4px;"
        "color: #333;");
    // 插入到锚点前面
    m_contentLayout->insertWidget(
        m_contentLayout->count() - 1, label);
    ++m_messageCount;
    updateCountLabel();
    scrollToBottom();
}

/// @brief 追加一条系统消息
void MainWindow::appendSystemMessage(const QString &text)
{
    auto *label = new QLabel(text);
    label->setWordWrap(true);
    label->setStyleSheet(
        "padding: 4px 10px;"
        "margin: 2px 0;"
        "color: #888;"
        "font-style: italic;");
    m_contentLayout->insertWidget(
        m_contentLayout->count() - 1, label);
    scrollToBottom();
}

/// @brief 清空所有消息标签
void MainWindow::onClearMessages()
{
    // 从后往前遍历，只删除 QLabel，保留锚点 QWidget
    QLayoutItem *item = nullptr;
    int i = m_contentLayout->count() - 1;
    while ((item = m_contentLayout->takeAt(i)) != nullptr) {
        if (auto *label = qobject_cast<QLabel *>(item->widget())) {
            // 跳过系统消息也一并清除，只要是 QLabel 就删
            label->deleteLater();
        } else if (item->widget() == m_anchor) {
            // 锚点控件不删除，但需要重新加回去
            m_contentLayout->addWidget(m_anchor);
        }
        delete item;
        --i;
    }
    m_messageCount = 0;
    updateCountLabel();
    appendSystemMessage("所有消息已清空");
}

/// @brief 滚动到底部（使用 ensureWidgetVisible）
void MainWindow::scrollToBottom()
{
    // 延迟确保布局更新后再滚动
    QTimer::singleShot(0, this, [this]() {
        m_scrollArea->ensureWidgetVisible(m_anchor);
    });
}

/// @brief 更新消息计数标签
void MainWindow::updateCountLabel()
{
    m_countLabel->setText(
        QString("消息数: %1").arg(m_messageCount));
}
