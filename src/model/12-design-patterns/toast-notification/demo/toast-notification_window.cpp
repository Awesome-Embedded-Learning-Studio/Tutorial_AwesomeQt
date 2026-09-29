/**
 * @file toast-notification_window.cpp
 * @brief Toast Notification 演示实现
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "toast-notification_window.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "toast-notification.h"

ToastNotificationWindow::ToastNotificationWindow(QWidget* parent) : QMainWindow(parent) {
    setupButtons();

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    layout->addWidget(info_btn_);
    layout->addWidget(success_btn_);
    layout->addWidget(warning_btn_);
    layout->addWidget(error_btn_);
    layout->addWidget(burst_btn_);
    layout->addStretch();
    setCentralWidget(central);

    setWindowTitle("Toast Notification Demo");
    resize(360, 260);
}

void ToastNotificationWindow::setupButtons() {
    info_btn_ = new QPushButton("Show Info Toast", this);
    success_btn_ = new QPushButton("Show Success Toast", this);
    warning_btn_ = new QPushButton("Show Warning Toast", this);
    error_btn_ = new QPushButton("Show Error Toast", this);
    burst_btn_ = new QPushButton("Burst 5 Toasts (see stacking)", this);

    // 函数指针语法 connect，禁 SIGNAL/SLOT 宏
    connect(info_btn_, &QPushButton::clicked, this, [this]() {
        AwesomeQt::ToastManager::getInstance()->showToast(
            "这是一条普通提示", AwesomeQt::ToastType::kInfo, 3000,
            AwesomeQt::ToastCorner::kBottomRight, this);
    });
    connect(success_btn_, &QPushButton::clicked, this, [this]() {
        AwesomeQt::ToastManager::getInstance()->showToast(
            "保存成功", AwesomeQt::ToastType::kSuccess, 3000, AwesomeQt::ToastCorner::kBottomRight,
            this);
    });
    connect(warning_btn_, &QPushButton::clicked, this, [this]() {
        AwesomeQt::ToastManager::getInstance()->showToast(
            "磁盘空间不足", AwesomeQt::ToastType::kWarning, 3000,
            AwesomeQt::ToastCorner::kBottomRight, this);
    });
    connect(error_btn_, &QPushButton::clicked, this, [this]() {
        AwesomeQt::ToastManager::getInstance()->showToast(
            "网络连接失败", AwesomeQt::ToastType::kError, 3000,
            AwesomeQt::ToastCorner::kBottomRight, this);
    });
    // 连发 5 条：看堆叠——新条向上排、先入的先消失后重排
    connect(burst_btn_, &QPushButton::clicked, this, [this]() {
        auto* mgr = AwesomeQt::ToastManager::getInstance();
        mgr->showToast("连发第 1 条", AwesomeQt::ToastType::kInfo, 4000,
                       AwesomeQt::ToastCorner::kBottomRight, this);
        mgr->showToast("连发第 2 条", AwesomeQt::ToastType::kSuccess, 4000,
                       AwesomeQt::ToastCorner::kBottomRight, this);
        mgr->showToast("连发第 3 条", AwesomeQt::ToastType::kWarning, 4000,
                       AwesomeQt::ToastCorner::kBottomRight, this);
        mgr->showToast("连发第 4 条", AwesomeQt::ToastType::kError, 4000,
                       AwesomeQt::ToastCorner::kBottomRight, this);
        mgr->showToast("连发第 5 条", AwesomeQt::ToastType::kInfo, 4000,
                       AwesomeQt::ToastCorner::kBottomRight, this);
    });
}
