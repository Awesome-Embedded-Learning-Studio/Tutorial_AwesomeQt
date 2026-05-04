#include "file_dialog.h"

#include <QFont>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QStyle>
#include <QTime>
#include <QVBoxLayout>

FileDialog::FileDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("QPushButton 综合演示 — 文件操作对话框");
    resize(560, 440);
    initUi();
}

void FileDialog::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ---- 标题 ----
    auto *titleLabel = new QLabel("文件操作对话框");
    titleLabel->setFont(QFont("Arial", 15, QFont::Bold));
    mainLayout->addWidget(titleLabel);

    // ---- 操作结果展示区 ----
    m_resultLabel = new QLabel("等待操作...");
    m_resultLabel->setAlignment(Qt::AlignCenter);
    m_resultLabel->setMinimumHeight(80);
    m_resultLabel->setStyleSheet(
        "background-color: #FAFAFA;"
        "border: 1px solid #E0E0E0;"
        "border-radius: 6px;"
        "padding: 16px;"
        "font-size: 13px;"
        "color: #333;");
    mainLayout->addWidget(m_resultLabel, 1);

    // ---- 底部状态栏 ----
    m_statusLabel = new QLabel("最近操作: 无");
    m_statusLabel->setStyleSheet("color: #888; font-size: 11px;");
    mainLayout->addWidget(m_statusLabel);

    // ================================================================
    // 按钮区域 1: 带菜单的新建按钮
    // ================================================================
    auto *menuGroup = new QGroupBox("下拉菜单按钮");
    auto *menuLayout = new QHBoxLayout(menuGroup);

    auto *newBtn = new QPushButton("新建");
    auto *menu = new QMenu(newBtn);
    menu->addAction("新建文本文件", this, [this]() {
        setResult("创建了新的文本文件");
    });
    menu->addAction("新建文件夹", this, [this]() {
        setResult("创建了新的文件夹");
    });
    menu->addAction("新建快捷方式", this, [this]() {
        setResult("创建了新的快捷方式");
    });
    newBtn->setMenu(menu);
    newBtn->setAutoDefault(false);

    menuLayout->addWidget(newBtn);
    menuLayout->addStretch();

    auto *menuHint = new QLabel(
        "setMenu() 绑定下拉菜单，点击弹出");
    menuHint->setStyleSheet("color: #999; font-size: 11px;");
    menuLayout->addWidget(menuHint);

    mainLayout->addWidget(menuGroup);

    // ================================================================
    // 按钮区域 2: 图标按钮 + 默认按钮 + QSS 美化按钮
    // ================================================================
    auto *actionGroup = new QGroupBox("图标 / 默认 / QSS 按钮");
    auto *actionLayout = new QHBoxLayout(actionGroup);

    // 图标按钮（打开）
    auto *openBtn = new QPushButton("打开");
    openBtn->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    openBtn->setIconSize(QSize(20, 20));
    openBtn->setAutoDefault(false);
    connect(openBtn, &QPushButton::clicked, this, [this]() {
        setResult("打开了文件选择器");
    });

    // 默认按钮（保存）—— setDefault(true)，按 Enter 触发
    auto *saveBtn = new QPushButton("保存");
    saveBtn->setDefault(true);
    saveBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #1976D2;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 7px 20px;"
        "  font-size: 13px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #1565C0;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #0D47A1;"
        "}"
    );
    connect(saveBtn, &QPushButton::clicked, this, [this]() {
        setResult("文件已保存（默认按钮 / 按 Enter 触发）");
    });

    // QSS 红色警告按钮（关闭）
    auto *closeBtn = new QPushButton("关闭");
    closeBtn->setAutoDefault(false);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #D32F2F;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 7px 20px;"
        "  font-size: 13px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #C62828;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #B71C1C;"
        "}"
    );
    connect(closeBtn, &QPushButton::clicked, this, [this]() {
        setResult("确认关闭？当前操作已取消");
    });

    actionLayout->addWidget(openBtn);
    actionLayout->addWidget(saveBtn);
    actionLayout->addWidget(closeBtn);

    auto *actionHint = new QLabel(
        "保存 = setDefault(true) | 按 Enter 可直接触发");
    actionHint->setStyleSheet("color: #999; font-size: 11px;");
    auto *hintRow = new QHBoxLayout();
    hintRow->addWidget(actionHint);
    hintRow->addStretch();
    auto *actionVLayout = new QVBoxLayout();
    actionVLayout->addLayout(actionLayout);
    actionVLayout->addLayout(hintRow);
    actionGroup->setLayout(actionVLayout);

    mainLayout->addWidget(actionGroup);

    // ================================================================
    // 按钮区域 3: 扁平按钮（setFlat + QSS）
    // ================================================================
    auto *flatGroup = new QGroupBox("扁平按钮 (setFlat + QSS)");
    auto *flatLayout = new QHBoxLayout(flatGroup);

    auto *linkBtn = new QPushButton("高级选项");
    linkBtn->setFlat(true);
    linkBtn->setAutoDefault(false);
    linkBtn->setStyleSheet(
        "QPushButton {"
        "  color: #1565C0;"
        "  border: none;"
        "  padding: 4px 8px;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "  color: #0D47A1;"
        "  text-decoration: underline;"
        "}"
        "QPushButton:pressed {"
        "  color: #0A3D91;"
        "}"
    );
    connect(linkBtn, &QPushButton::clicked, this, [this]() {
        setResult("打开了高级选项面板（扁平按钮触发）");
    });

    auto *helpBtn = new QPushButton("帮助文档");
    helpBtn->setFlat(true);
    helpBtn->setAutoDefault(false);
    helpBtn->setStyleSheet(
        "QPushButton {"
        "  color: #6A1B9A;"
        "  border: none;"
        "  padding: 4px 8px;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "  color: #4A148C;"
        "  text-decoration: underline;"
        "}"
        "QPushButton:pressed {"
        "  color: #38006b;"
        "}"
    );
    connect(helpBtn, &QPushButton::clicked, this, [this]() {
        setResult("打开了帮助文档链接");
    });

    flatLayout->addWidget(linkBtn);
    flatLayout->addWidget(helpBtn);
    flatLayout->addStretch();

    auto *flatHint = new QLabel(
        "setFlat(true) + QSS: 无边框的文字链接风格");
    flatHint->setStyleSheet("color: #999; font-size: 11px;");
    flatLayout->addWidget(flatHint);

    mainLayout->addWidget(flatGroup);
}

void FileDialog::setResult(const QString &text)
{
    m_resultLabel->setText(text);
    m_statusLabel->setText(
        "最近操作: " + text + "  [" +
        QTime::currentTime().toString("HH:mm:ss") + "]");
}
