#include "MainWindow.h"

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

// ============================================================================
// MainWindow: 主窗口，演示按钮盒对话框
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QDialogButtonBox 标准按钮盒演示");
    resize(500, 400);

    auto *central = new QWidget;
    setCentralWidget(central);

    auto *layout = new QVBoxLayout(central);

    // 打开设置按钮
    auto *openBtn = new QPushButton("打开设置对话框");
    openBtn->setMinimumHeight(40);
    connect(openBtn, &QPushButton::clicked,
            this, &MainWindow::onOpenSettings);
    layout->addWidget(openBtn);

    // 结果显示区域
    auto *label = new QLabel("设置结果:");
    layout->addWidget(label);

    m_resultEdit = new QTextEdit;
    m_resultEdit->setReadOnly(true);
    m_resultEdit->setPlaceholderText(
        "点击上方按钮打开设置对话框...");
    layout->addWidget(m_resultEdit);
}

void MainWindow::onOpenSettings()
{
    SettingsDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        m_resultEdit->append(
            QString("保存设置:\n"
                    "  应用名称: %1\n"
                    "  最大连接数: %2\n"
                    "  日志级别: %3")
                .arg(dlg.appName())
                .arg(dlg.maxConnections())
                .arg(dlg.logLevel()));
        m_resultEdit->append("---");
    } else {
        m_resultEdit->append("用户取消了设置");
        m_resultEdit->append("---");
    }
}
