/**
 * @file ip_edit_window.h
 * @brief IpEdit 控件演示主窗口
 * @copyright Copyright (c) 2026
 */

#pragma once

#include <QMainWindow>

class QLabel;

class IpEditWindow : public QMainWindow {
  public:
    explicit IpEditWindow(QWidget* parent = nullptr);

  private:
    void setup_ui();
    QWidget* setup_input_layout();
    QWidget* setup_presets_layout();
    QWidget* setup_edgecases_layout();
};
