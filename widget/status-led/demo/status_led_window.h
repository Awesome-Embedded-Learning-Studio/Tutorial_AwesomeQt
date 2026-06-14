/**
 * @file status_led_window.h
 * @brief StatusLED 控件演示主窗口
 * @copyright Copyright (c) 2026
 */

#pragma once

#include <QMainWindow>

class StatusLEDWindow : public QMainWindow {
  public:
    explicit StatusLEDWindow(QWidget* parent = nullptr);

  private:
    void setup_ui();
    QWidget* setup_static_layout();
    QWidget* setup_dynamic_layout();
    QWidget* setup_sizes_layout();
    QWidget* setup_modes_layout();
};
