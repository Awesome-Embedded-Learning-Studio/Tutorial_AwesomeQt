/**
 * @file toggle_switch_window.h
 * @brief ToggleSwitch 演示主窗口
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QMainWindow>

class ToggleSwitchWindow : public QMainWindow {
  public:
    explicit ToggleSwitchWindow(QWidget* parent = nullptr);

  private:
    void setup_ui();
    QWidget* setup_basic_layout();
    QWidget* setup_interactive_layout();
    QWidget* setup_custom_layout();
};
