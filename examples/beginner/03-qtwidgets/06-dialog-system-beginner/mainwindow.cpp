// QtWidgets 入门示例 06: 对话框体系基础
// MainWindow 实现

#include "mainwindow.h"

#include "finddialog.h"
#include "userinfo.h"
#include "userinfodialog.h"

#include <QDebug>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("对话框体系基础演示");
    resize(520, 380);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // ---- 标题 ----
    auto *titleLabel = new QLabel("对话框体系基础");
    titleLabel->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #2C3E50;");
    mainLayout->addWidget(titleLabel);

    // ---- 当前信息展示 ----
    m_infoDisplay = new QLabel("尚未设置个人信息。点击下方按钮开始编辑。");
    m_infoDisplay->setWordWrap(true);
    m_infoDisplay->setStyleSheet(
        "background-color: #F8F9FA; border: 1px solid #DEE2E6;"
        "border-radius: 6px; padding: 16px; color: #495057;"
        "line-height: 1.6;");
    m_infoDisplay->setMinimumHeight(120);
    mainLayout->addWidget(m_infoDisplay, 1);

    // ---- 按钮区域 ----
    auto *buttonLayout = new QVBoxLayout;
    buttonLayout->setSpacing(10);

    auto *editBtn = new QPushButton("编辑个人信息（模态对话框 exec）");
    editBtn->setStyleSheet(
        "QPushButton { padding: 10px; font-size: 14px; }");
    connect(editBtn, &QPushButton::clicked,
            this, &MainWindow::onEditInfoClicked);
    buttonLayout->addWidget(editBtn);

    auto *findBtn = new QPushButton("打开查找面板（非模态对话框 show）");
    findBtn->setStyleSheet(
        "QPushButton { padding: 10px; font-size: 14px; }");
    connect(findBtn, &QPushButton::clicked,
            this, &MainWindow::onFindClicked);
    buttonLayout->addWidget(findBtn);

    mainLayout->addLayout(buttonLayout);

    // ---- 底部状态 ----
    auto *footerLabel = new QLabel(
        "exec() 阻塞等待返回值 | show() 非阻塞自由交互 | "
        "QDialogButtonBox 自动适配平台按钮排列");
    footerLabel->setStyleSheet("color: #AAA; font-size: 11px;");
    footerLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(footerLabel);
}

void MainWindow::onEditInfoClicked()
{
    UserInfoDialog dialog(this);

    // 预填当前已有的数据
    dialog.setData(m_userInfo);

    // exec() 会阻塞在这里，直到对话框被 accept() 或 reject() 关闭
    if (dialog.exec() == QDialog::Accepted) {
        // 用户点了确定，读取对话框中的数据
        m_userInfo = dialog.getData();
        updateInfoDisplay();
        qDebug() << "用户确认了信息:" << m_userInfo.name
                 << m_userInfo.age << m_userInfo.email;
    } else {
        qDebug() << "用户取消了编辑";
    }
}

void MainWindow::onFindClicked()
{
    if (m_findDialog == nullptr) {
        m_findDialog = new FindDialog(this);
        // WA_DeleteOnClose 会在对话框关闭时自动 delete
        // 关闭后指针不会自动置空，需要在 destroyed 信号中处理
        connect(m_findDialog, &QObject::destroyed,
                this, [this]() { m_findDialog = nullptr; });
        connect(m_findDialog, &FindDialog::findRequested,
                this, [this](const QString &text) {
                    qDebug() << "查找内容:" << text;
                    m_infoDisplay->setText(
                        m_infoDisplay->text() +
                        "\n\n[查找] " + text);
                });
    }

    // show() 不会阻塞，对话框弹出后代码立刻继续执行
    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
}

void MainWindow::updateInfoDisplay()
{
    QString html = QString(
        "<b>姓名:</b> %1<br>"
        "<b>年龄:</b> %2<br>"
        "<b>邮箱:</b> %3<br>"
        "<b>简介:</b> %4")
        .arg(m_userInfo.name.isEmpty() ? "（未设置）" : m_userInfo.name)
        .arg(m_userInfo.age)
        .arg(m_userInfo.email.isEmpty() ? "（未设置）" : m_userInfo.email)
        .arg(m_userInfo.bio.isEmpty() ? "（无）" : m_userInfo.bio);

    m_infoDisplay->setText(html);
}
