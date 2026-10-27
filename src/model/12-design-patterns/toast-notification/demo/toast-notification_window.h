/**
 * @file toast-notification_window.h
 * @brief Toast Notification 演示主窗口——四类按钮触发不同 Toast + 连发看堆叠
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QMainWindow>

class QPushButton;

/// @brief 演示窗口：四个按钮分别弹出 Info/Success/Warning/Error Toast，
///        另有一个「连发 5 条」按钮看堆叠动画。
class ToastNotificationWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit ToastNotificationWindow(QWidget* parent = nullptr);

  private:
    void setupButtons();

    QPushButton* info_btn_{nullptr};
    QPushButton* success_btn_{nullptr};
    QPushButton* warning_btn_{nullptr};
    QPushButton* error_btn_{nullptr};
    QPushButton* burst_btn_{nullptr};
};
