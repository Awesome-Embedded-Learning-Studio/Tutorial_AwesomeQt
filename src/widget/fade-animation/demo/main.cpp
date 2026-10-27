/**
 * @file main.cpp
 * @brief FadeWidget demo 入口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include <QApplication>

#include "fade_animation_window.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    AwesomeQt::FadeAnimationWindow w;
    w.show();

    return app.exec();
}
