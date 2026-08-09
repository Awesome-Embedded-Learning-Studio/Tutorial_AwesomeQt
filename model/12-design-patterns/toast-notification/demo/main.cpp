/**
 * @file main.cpp
 * @brief Toast Notification 演示程序入口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "toast-notification_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    ToastNotificationWindow window;
    window.show();
    return app.exec();
}
