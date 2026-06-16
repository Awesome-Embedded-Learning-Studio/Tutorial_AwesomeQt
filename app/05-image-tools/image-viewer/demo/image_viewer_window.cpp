/**
 * @file image_viewer_window.cpp
 * @brief 图片查看器应用主窗口实现（骨架）
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "image_viewer_window.h"

#include <QAction>
#include <QFileDialog>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollArea>

ImageViewerWindow::ImageViewerWindow(QWidget* parent) : QMainWindow(parent) {
    setup_menu();
    setup_central();
    setWindowTitle("Image Viewer (scaffold)");
    resize(800, 600);
}

void ImageViewerWindow::setup_menu() {
    // —— 文件菜单：打开（弹文件对话框）+ 退出 ——
    auto* file_menu = menuBar()->addMenu("&File");

    auto* open_action = file_menu->addAction("&Open...");
    open_action->setShortcut(QKeySequence::Open);
    connect(open_action, &QAction::triggered, this, [this]() {
        // 骨架：只把所选路径显示到占位 label，验证「菜单 → 文件对话框 → 中央区」链路通。
        // 不做 QImage 加载（成品阶段补缩放/旋转/幻灯片）。
        const QString path = QFileDialog::getOpenFileName(
            this, "Open Image", QString(),
            "Images (*.png *.jpg *.jpeg *.bmp *.gif *.svg);;All Files (*)");
        if (!path.isEmpty()) {
            placeholder_label_->setText("Selected: " + path);
        }
    });

    auto* quit_action = file_menu->addAction("&Quit");
    quit_action->setShortcut(QKeySequence::Quit);
    connect(quit_action, &QAction::triggered, this, &ImageViewerWindow::close);

    // —— 帮助菜单：关于 ——
    auto* help_menu = menuBar()->addMenu("&Help");
    auto* about_action = help_menu->addAction("&About");
    connect(about_action, &QAction::triggered, this, [this]() {
        QMessageBox::about(
            this, "About",
            "AwesomeQt Image Viewer\n"
            "app/ 栏骨架 · 验证 QMainWindow + 菜单 + 文件对话框范式");
    });
}

void ImageViewerWindow::setup_central() {
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);

    placeholder_label_ = new QLabel(
        "Open a file (Ctrl+O) to select an image.\n"
        "骨架：验证菜单 → 文件对话框 → 占位区链路。",
        scroll);
    placeholder_label_->setAlignment(Qt::AlignCenter);

    scroll->setWidget(placeholder_label_);
    setCentralWidget(scroll);
}
