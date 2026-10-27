#include "mainwindow.h"

#include <QLabel>
#include <QStatusBar>
#include <QVBoxLayout>

// ============================================================================
// 主窗口: 启动完成后显示
// ============================================================================
MainWindow::MainWindow(
    double elapsedSec,
    QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("My Application");
    resize(800, 600);

    auto *central = new QWidget;
    auto *layout = new QVBoxLayout(central);

    auto *title = new QLabel("应用已就绪");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(
        "font-size: 24px; font-weight: bold;"
        "color: #1E1E2E;");

    auto *info = new QLabel(
        QString("启动完成，共耗时 %1 秒")
            .arg(elapsedSec, 0, 'f', 1));
    info->setAlignment(Qt::AlignCenter);
    info->setStyleSheet("font-size: 14px; color: #585B70;");

    layout->addStretch();
    layout->addWidget(title);
    layout->addSpacing(10);
    layout->addWidget(info);
    layout->addStretch();

    setCentralWidget(central);

    statusBar()->showMessage("就绪");
}
