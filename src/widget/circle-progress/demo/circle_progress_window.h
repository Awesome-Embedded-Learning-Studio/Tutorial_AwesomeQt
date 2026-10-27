/**
 * @file circle_progress_window.h
 * @brief CircleProgress 演示主窗口：静态多档 + Cycle 过渡 + Slider 驱动 + 配色变体
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QMainWindow>

class CircleProgressWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit CircleProgressWindow(QWidget* parent = nullptr);

  private:
    void setupUi();
    QWidget* setupStaticLayout();
    QWidget* setupInteractiveLayout();
    QWidget* setupVariantsLayout();
};
