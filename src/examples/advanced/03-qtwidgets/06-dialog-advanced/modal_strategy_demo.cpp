/// @file    modal_strategy_demo.cpp
/// @brief   ModalStrategyDemo 类实现——模态范围差异演示。
///
/// 对应教程：进阶层 03-QtWidgets/06-对话框进阶。

#include "modal_strategy_demo.h"
#include "validated_dialog.h"

#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

ModalStrategyDemo::ModalStrategyDemo(QWidget* parent)
    : QWidget(parent)
    , m_toolWindow(nullptr)
    , m_statusLabel(nullptr)
{
    setWindowTitle(QStringLiteral("Modal Strategy Demo - Main Window"));

    auto* layout = new QVBoxLayout(this);

    // 标题说明
    auto* titleLabel = new QLabel(
        QStringLiteral("对比 WindowModal 与 ApplicationModal 的阻塞范围：\n"
                       "旁边的\"独立工具窗口\"不受 WindowModal 影响，但会被 ApplicationModal 冻结。"));
    titleLabel->setWordWrap(true);
    layout->addWidget(titleLabel);

    // 两个按钮并排
    auto* buttonRow = new QHBoxLayout;

    auto* windowModalBtn = new QPushButton(QStringLiteral("弹出 WindowModal 对话框"));
    auto* appModalBtn = new QPushButton(QStringLiteral("弹出 ApplicationModal 对话框"));

    buttonRow->addWidget(windowModalBtn);
    buttonRow->addWidget(appModalBtn);
    layout->addLayout(buttonRow);

    // 状态标签
    m_statusLabel = new QLabel(QStringLiteral("点击按钮测试不同模态策略。"));
    layout->addWidget(m_statusLabel);

    layout->addStretch();

    // 连接信号
    connect(windowModalBtn, &QPushButton::clicked, this, &ModalStrategyDemo::showWindowModal);
    connect(appModalBtn, &QPushButton::clicked, this, &ModalStrategyDemo::showApplicationModal);

    resize(500, 200);

    // 创建一个独立的工具窗口（parent 为 nullptr，与主窗口无父子关系）
    m_toolWindow = new QWidget;
    m_toolWindow->setWindowTitle(QStringLiteral("独立工具窗口（无父子关系）"));
    auto* toolLayout = new QVBoxLayout(m_toolWindow);
    auto* toolLabel = new QLabel(
        QStringLiteral("我是独立工具窗口。\n"
                       "WindowModal 不会冻结我——你可以继续点击我。\n"
                       "ApplicationModal 会冻结我——我完全无法接收输入。"));
    toolLabel->setWordWrap(true);
    toolLayout->addWidget(toolLabel);

    auto* toolClickBtn = new QPushButton(QStringLiteral("点击测试（如果我能响应说明没被冻结）"));
    toolLayout->addWidget(toolClickBtn);

    connect(toolClickBtn, &QPushButton::clicked, toolClickBtn, [this]() {
        m_statusLabel->setText(QStringLiteral("工具窗口按钮被点击了 —— 它没有被冻结。"));
    });

    m_toolWindow->resize(350, 150);
    m_toolWindow->show();
}

// ─────────────────────────────────────────────────────────────────────────────
// 槽函数
// ─────────────────────────────────────────────────────────────────────────────

void ModalStrategyDemo::showWindowModal()
{
    // WindowModal: 只阻塞父窗口及其祖先，独立工具窗口不受影响
    auto* dialog = new ValidatedDialog(QStringLiteral("WindowModal 对话框"), this);
    dialog->setWindowModality(Qt::WindowModal);

    m_statusLabel->setText(
        QStringLiteral("已弹出 WindowModal 对话框 —— 独立工具窗口仍可操作。"));

    if (dialog->exec() == QDialog::Accepted) {
        m_statusLabel->setText(
            QStringLiteral("WindowModal 对话框确认 —— 端口: %1").arg(dialog->portValue()));
    } else {
        m_statusLabel->setText(QStringLiteral("WindowModal 对话框已取消。"));
    }

    // dialog 的 parent 是 this，Qt 对象树自动析构
}

void ModalStrategyDemo::showApplicationModal()
{
    // ApplicationModal: 阻塞所有窗口，包括独立工具窗口
    auto* dialog = new ValidatedDialog(QStringLiteral("ApplicationModal 对话框"), this);
    dialog->setWindowModality(Qt::ApplicationModal);

    m_statusLabel->setText(
        QStringLiteral("已弹出 ApplicationModal 对话框 —— 所有窗口均被冻结。"));

    if (dialog->exec() == QDialog::Accepted) {
        m_statusLabel->setText(
            QStringLiteral("ApplicationModal 对话框确认 —— 端口: %1").arg(dialog->portValue()));
    } else {
        m_statusLabel->setText(QStringLiteral("ApplicationModal 对话框已取消。"));
    }
}
