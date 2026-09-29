#include "mainwindow.h"

#include <QAction>
#include <QDate>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QStatusBar>
#include <QStyle>
#include <QTextEdit>
#include <QTime>
#include <QToolBar>
#include <QToolButton>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QToolButton 综合演示");
    resize(720, 520);
    initUi();
}

void MainWindow::initUi()
{
    // ---- 中央编辑区 ----
    m_textEdit = new QTextEdit();
    m_textEdit->setPlaceholderText("这里是文档编辑区，操作日志会在此处显示...");
    setCentralWidget(m_textEdit);

    // ================================================================
    // 工具栏
    // ================================================================
    m_toolbar = addToolBar("主工具栏");
    m_toolbar->setMovable(false);
    m_toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    // ---- 第 1 组: 文件操作 — MenuButtonPopup ----
    auto *saveAction = new QAction(
        style()->standardIcon(QStyle::SP_DialogSaveButton), "保存");
    connect(saveAction, &QAction::triggered, this, [this]() {
        log("执行: 保存文件（默认操作，点击按钮主体触发）");
    });

    auto *saveMenu = new QMenu();
    saveMenu->addAction("另存为...", this, [this]() {
        log("执行: 另存为...");
    });
    saveMenu->addAction("保存全部", this, [this]() {
        log("执行: 保存全部文件");
    });
    saveMenu->addAction("导出为 PDF...", this, [this]() {
        log("执行: 导出为 PDF");
    });

    auto *saveBtn = new QToolButton();
    saveBtn->setDefaultAction(saveAction);
    saveBtn->setMenu(saveMenu);
    saveBtn->setPopupMode(QToolButton::MenuButtonPopup);
    m_toolbar->addWidget(saveBtn);

    m_toolbar->addSeparator();

    // ---- 第 2 组: 编辑操作 — DelayedPopup ----
    auto *undoBtn = new QToolButton();
    undoBtn->setText("撤销");
    undoBtn->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    auto *undoMenu = new QMenu(undoBtn);
    undoMenu->addAction("撤销: 输入文字", this, [this]() {
        log("从历史撤销: 输入文字");
    });
    undoMenu->addAction("撤销: 删除行", this, [this]() {
        log("从历史撤销: 删除行");
    });
    undoMenu->addAction("撤销: 格式修改", this, [this]() {
        log("从历史撤销: 格式修改");
    });
    undoBtn->setMenu(undoMenu);
    undoBtn->setPopupMode(QToolButton::DelayedPopup);
    connect(undoBtn, &QToolButton::clicked, this, [this]() {
        log("执行: 撤销（快速点击触发）");
    });
    m_toolbar->addWidget(undoBtn);

    auto *redoBtn = new QToolButton();
    redoBtn->setText("重做");
    redoBtn->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
    auto *redoMenu = new QMenu(redoBtn);
    redoMenu->addAction("重做: 输入文字", this, [this]() {
        log("从历史重做: 输入文字");
    });
    redoMenu->addAction("重做: 删除行", this, [this]() {
        log("从历史重做: 删除行");
    });
    redoBtn->setMenu(redoMenu);
    redoBtn->setPopupMode(QToolButton::DelayedPopup);
    connect(redoBtn, &QToolButton::clicked, this, [this]() {
        log("执行: 重做（快速点击触发）");
    });
    m_toolbar->addWidget(redoBtn);

    m_toolbar->addSeparator();

    // ---- 第 3 组: 插入操作 — InstantPopup ----
    auto *insertBtn = new QToolButton();
    insertBtn->setText("插入");
    insertBtn->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    auto *insertMenu = new QMenu(insertBtn);
    insertMenu->addAction("插入当前时间", this, [this]() {
        log("插入: " + QTime::currentTime().toString("HH:mm:ss"));
    });
    insertMenu->addAction("插入当前日期", this, [this]() {
        log("插入: " + QDate::currentDate().toString("yyyy-MM-dd"));
    });
    insertMenu->addSeparator();
    insertMenu->addAction("插入模板文字", this, [this]() {
        log("插入: [模板文字已插入]");
    });
    insertBtn->setMenu(insertMenu);
    insertBtn->setPopupMode(QToolButton::InstantPopup);
    m_toolbar->addWidget(insertBtn);

    // ================================================================
    // 底部切换栏
    // ================================================================
    auto *bottomWidget = new QWidget();
    auto *bottomLayout = new QHBoxLayout(bottomWidget);
    bottomLayout->setContentsMargins(12, 6, 12, 6);

    auto *hintLabel = new QLabel("切换工具栏样式:");
    bottomLayout->addWidget(hintLabel);

    auto *toggleBtn = new QPushButton("切换: IconOnly / TextBesideIcon");
    connect(toggleBtn, &QPushButton::clicked, this, [this]() {
        if (m_toolbar->toolButtonStyle() == Qt::ToolButtonIconOnly) {
            m_toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            log("工具栏样式切换为: TextBesideIcon");
        } else {
            m_toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
            log("工具栏样式切换为: IconOnly");
        }
    });
    bottomLayout->addWidget(toggleBtn);
    bottomLayout->addStretch();

    // 把底部栏作为 status bar 的固定 widget
    statusBar()->addWidget(bottomWidget, 1);

    log("欢迎使用 QToolButton 综合演示");
    log("第 1 组 (保存): MenuButtonPopup — 点击主体执行保存，点击箭头弹出菜单");
    log("第 2 组 (撤销/重做): DelayedPopup — 快速点击执行操作，长按弹出历史");
    log("第 3 组 (插入): InstantPopup — 点击立刻弹出菜单");
}

void MainWindow::log(const QString &message)
{
    m_textEdit->append(
        "[" + QTime::currentTime().toString("HH:mm:ss") + "] " + message);
}
