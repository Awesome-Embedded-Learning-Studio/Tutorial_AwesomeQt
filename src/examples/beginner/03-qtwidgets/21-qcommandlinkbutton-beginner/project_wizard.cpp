#include "project_wizard.h"

#include <QApplication>
#include <QCommandLinkButton>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QTime>
#include <QVBoxLayout>

ProjectWizard::ProjectWizard(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QCommandLinkButton 综合演示 — 项目创建向导");
    resize(560, 520);
    initUi();
}

void ProjectWizard::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    // ---- 标题 ----
    auto *titleLabel = new QLabel("选择项目类型");
    titleLabel->setFont(QFont("Arial", 16, QFont::Bold));
    mainLayout->addWidget(titleLabel);

    auto *hintLabel = new QLabel(
        "请选择要创建的项目模板，点击选项进入下一步配置");
    hintLabel->setStyleSheet("color: #666; font-size: 12px; margin-bottom: 8px;");
    mainLayout->addWidget(hintLabel);

    // ================================================================
    // 三个 QCommandLinkButton 作为功能选项
    // ================================================================
    auto *optionsGroup = new QGroupBox("项目模板");
    auto *optionsLayout = new QVBoxLayout(optionsGroup);
    optionsLayout->setSpacing(6);

    // 统一的 QSS 样式（跨平台视觉一致性）
    QString linkStyle =
        "QCommandLinkButton {"
        "  color: #0066CC;"
        "  border: 1px solid transparent;"
        "  border-radius: 6px;"
        "  text-align: left;"
        "  padding: 10px 14px;"
        "  font-size: 14px;"
        "}"
        "QCommandLinkButton:hover {"
        "  color: #004499;"
        "  background-color: #F0F6FF;"
        "  border: 1px solid #C8DFFF;"
        "}"
        "QCommandLinkButton:pressed {"
        "  color: #003366;"
        "  background-color: #DCE8F8;"
        "  border: 1px solid #A0C0E8;"
        "}";

    // 选项 1: 空项目
    m_emptyBtn = new QCommandLinkButton(
        "空项目",
        "创建一个空白项目，手动添加源文件和配置。"
        "适合有经验的开发者或需要完全自定义的场景。");
    m_emptyBtn->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    m_emptyBtn->setStyleSheet(linkStyle);
    m_emptyBtn->setAutoDefault(false);
    connect(m_emptyBtn, &QCommandLinkButton::clicked, this,
            [this]() { onOptionSelected("空项目"); });

    // 选项 2: Qt Widgets 应用
    m_widgetsBtn = new QCommandLinkButton(
        "Qt Widgets 应用",
        "使用 QMainWindow 或 QDialog 作为主窗口的传统桌面应用。"
        "适合需要复杂菜单、工具栏、多文档界面的程序。");
    m_widgetsBtn->setIcon(
        style()->standardIcon(QStyle::SP_ComputerIcon));
    m_widgetsBtn->setStyleSheet(linkStyle);
    m_widgetsBtn->setAutoDefault(false);
    connect(m_widgetsBtn, &QCommandLinkButton::clicked, this,
            [this]() { onOptionSelected("Qt Widgets 应用"); });

    // 选项 3: Qt Quick 应用
    m_quickBtn = new QCommandLinkButton(
        "Qt Quick 应用",
        "基于 QML 和 Qt Quick 的现代声明式 UI 应用。"
        "适合需要流畅动画、触摸交互和动态界面的场景。");
    m_quickBtn->setIcon(
        style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    m_quickBtn->setStyleSheet(linkStyle);
    m_quickBtn->setAutoDefault(false);
    connect(m_quickBtn, &QCommandLinkButton::clicked, this,
            [this]() { onOptionSelected("Qt Quick 应用"); });

    optionsLayout->addWidget(m_emptyBtn);
    optionsLayout->addWidget(m_widgetsBtn);
    optionsLayout->addWidget(m_quickBtn);

    mainLayout->addWidget(optionsGroup, 1);

    // ---- 结果展示区 ----
    m_resultLabel = new QLabel("等待选择...");
    m_resultLabel->setAlignment(Qt::AlignCenter);
    m_resultLabel->setMinimumHeight(50);
    m_resultLabel->setStyleSheet(
        "background-color: #FAFAFA;"
        "border: 1px solid #E0E0E0;"
        "border-radius: 6px;"
        "padding: 12px;"
        "font-size: 13px;"
        "color: #333;");
    mainLayout->addWidget(m_resultLabel);

    // ---- 底部按钮行 ----
    auto *bottomLayout = new QHBoxLayout();

    m_statusLabel = new QLabel("最近操作: 无");
    m_statusLabel->setStyleSheet("color: #888; font-size: 11px;");

    auto *exitBtn = new QPushButton("退出");
    exitBtn->setAutoDefault(false);
    exitBtn->setStyleSheet(
        "QPushButton {"
        "  padding: 6px 20px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  background-color: #FFF;"
        "}"
        "QPushButton:hover {"
        "  background-color: #E8E8E8;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #DDD;"
        "}");
    connect(exitBtn, &QPushButton::clicked, this, &QWidget::close);

    bottomLayout->addWidget(m_statusLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(exitBtn);

    mainLayout->addLayout(bottomLayout);
}

void ProjectWizard::onOptionSelected(const QString &projectType)
{
    m_resultLabel->setText(
        QString("已选择: %1\n点击「下一步」继续配置").arg(projectType));
    m_statusLabel->setText(
        QString("最近操作: 选择「%1」  [%2]")
            .arg(projectType,
                 QTime::currentTime().toString("HH:mm:ss")));
}
