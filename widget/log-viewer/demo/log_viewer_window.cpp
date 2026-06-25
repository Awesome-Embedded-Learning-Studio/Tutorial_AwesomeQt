/**
 * @file log_viewer_window.cpp
 * @brief LogViewer 演示主窗口实现
 * @copyright Copyright (c) 2026
 */

#include "log_viewer_window.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "log_viewer.h"

using AwesomeQt::LogViewer;

LogViewerWindow::LogViewerWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
}

void LogViewerWindow::setupUi() {
    auto* central = new QWidget(this);
    setCentralWidget(central);

    log_ = new LogViewer(central);

    // —— 控制面板：级别按钮 + 连发压测 + 清空 + autoScroll ——
    auto* controlGroup = new QGroupBox("Controls", central);
    auto* controlLayout = new QVBoxLayout(controlGroup);

    auto* levelRow = new QHBoxLayout();
    auto* infoBtn = new QPushButton("Append Info", controlGroup);
    auto* warnBtn = new QPushButton("Append Warning", controlGroup);
    auto* errorBtn = new QPushButton("Append Error", controlGroup);
    levelRow->addWidget(infoBtn);
    levelRow->addWidget(warnBtn);
    levelRow->addWidget(errorBtn);
    levelRow->addStretch();
    controlLayout->addLayout(levelRow);

    auto* stressRow = new QHBoxLayout();
    auto* stressBtn = new QPushButton("Burst 200 Info", controlGroup);
    stressBtn->setToolTip("连发 200 条 Info——测自动滚底 + 行数上限裁旧");
    auto* clearBtn = new QPushButton("Clear", controlGroup);
    auto_scroll_check_ = new QCheckBox("Auto Scroll", controlGroup);
    auto_scroll_check_->setChecked(log_->autoScroll());
    stressRow->addWidget(stressBtn);
    stressRow->addWidget(clearBtn);
    stressRow->addStretch();
    stressRow->addWidget(auto_scroll_check_);
    controlLayout->addLayout(stressRow);

    auto* statusRow = new QHBoxLayout();
    line_count_label_ = new QLabel("lines: 0", controlGroup);
    statusRow->addWidget(line_count_label_);
    statusRow->addStretch();
    controlLayout->addLayout(statusRow);

    // —— 信号连接 ——
    connect(infoBtn, &QPushButton::clicked, this,
            [this]() { log_->appendInfo("系统启动完成，监听端口 8080"); });
    connect(warnBtn, &QPushButton::clicked, this,
            [this]() { log_->appendWarning("配置文件缺失，使用默认值"); });
    connect(errorBtn, &QPushButton::clicked, this,
            [this]() { log_->appendError("无法连接数据库：connection refused"); });

    connect(stressBtn, &QPushButton::clicked, this, [this]() {
        // 连发 200 条：超过默认 maxLines=1000 不会爆，但底部应始终最新
        for (int i = 0; i < 200; ++i) {
            log_->appendInfo(QStringLiteral("心跳 #%1").arg(i));
        }
    });

    connect(clearBtn, &QPushButton::clicked, this, [this]() { log_->clear(); });

    connect(auto_scroll_check_, &QCheckBox::toggled, this,
            [this](bool checked) { log_->setAutoScroll(checked); });

    // 任何 append 后刷新行数显示
    connect(log_, &LogViewer::maxLinesChanged, this, [this]() {
        line_count_label_->setText(QStringLiteral("lines: %1").arg(log_->lineCount()));
    });

    auto* layout = new QVBoxLayout(central);
    layout->addWidget(log_);
    layout->addWidget(controlGroup);

    setWindowTitle("LogViewer Widget Demo");
    resize(560, 420);

    // 初始播几条
    log_->appendInfo("LogViewer 就绪");
    log_->appendWarning("这是一条警告示例");
    log_->appendError("这是一条错误示例");
    line_count_label_->setText(QStringLiteral("lines: %1").arg(log_->lineCount()));
}
