#include "MainWindow.h"

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

// ============================================================================
// MainWindow: 主窗口，演示模态和非模态对话框
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QDialog 对话框演示");
    resize(500, 400);

    auto *central = new QWidget;
    setCentralWidget(central);

    auto *layout = new QVBoxLayout(central);

    // 模态对话框按钮
    auto *newUserBtn = new QPushButton("新建用户 (模态对话框)");
    newUserBtn->setMinimumHeight(40);
    connect(newUserBtn, &QPushButton::clicked,
            this, &MainWindow::onNewUser);
    layout->addWidget(newUserBtn);

    // 非模态对话框按钮
    auto *findBtn = new QPushButton("查找 (非模态对话框)");
    findBtn->setMinimumHeight(40);
    connect(findBtn, &QPushButton::clicked,
            this, &MainWindow::onFind);
    layout->addWidget(findBtn);

    // 结果显示区域
    auto *label = new QLabel("操作结果:");
    layout->addWidget(label);

    m_resultEdit = new QTextEdit;
    m_resultEdit->setReadOnly(true);
    m_resultEdit->setPlaceholderText("用户配置信息将显示在这里...");
    layout->addWidget(m_resultEdit);

    // 窗口级模态对话框按钮
    auto *windowModalBtn = new QPushButton(
        "弹出窗口级模态对话框");
    windowModalBtn->setMinimumHeight(40);
    connect(windowModalBtn, &QPushButton::clicked,
            this, &MainWindow::onWindowModal);
    layout->addWidget(windowModalBtn);
}

void MainWindow::onNewUser()
{
    UserConfigDialog dlg(this);
    int result = dlg.exec();
    if (result == QDialog::Accepted) {
        m_resultEdit->append(
            QString("新建用户成功!\n"
                    "  用户名: %1\n"
                    "  年龄: %2\n"
                    "  角色: %3")
                .arg(dlg.username())
                .arg(dlg.age())
                .arg(dlg.role()));
    } else {
        m_resultEdit->append("用户取消了新建操作");
    }
}

void MainWindow::onFind()
{
    // 非模态：show() 立刻返回
    // WA_DeleteOnClose 已在 FindDialog 构造函数中设置
    auto *dlg = new FindDialog(this);
    dlg->show();
    m_resultEdit->append("已打开查找对话框（非模态）");

    // 注意：dlg 可能在任何时候被 delete
    // 不要在这里之后使用 dlg 指针
}

void MainWindow::onWindowModal()
{
    auto *dlg = new QDialog(this);
    dlg->setWindowTitle("窗口级模态对话框");
    dlg->setMinimumSize(300, 150);

    // 设置为窗口级模态
    dlg->setWindowModality(Qt::WindowModal);

    auto *layout = new QVBoxLayout(dlg);
    layout->addWidget(new QLabel(
        "这是一个窗口级模态对话框。\n"
        "你无法操作父窗口，但可以操作其他窗口。"));

    auto *closeBtn = new QPushButton("关闭");
    connect(closeBtn, &QPushButton::clicked,
            dlg, &QDialog::accept);
    layout->addWidget(closeBtn);

    // 用 show() + WindowModal 实现异步模态
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    connect(dlg, &QDialog::finished,
            this, [this](int result) {
                if (result == QDialog::Accepted) {
                    m_resultEdit->append(
                        "窗口级模态对话框已关闭（Accepted）");
                }
            });
    dlg->show();

    m_resultEdit->append(
        "已打开窗口级模态对话框（show + WindowModal）");
}
