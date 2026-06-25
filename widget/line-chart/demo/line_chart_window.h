/**
 * @file line_chart_window.h
 * @brief LineChart 控件演示主窗口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#pragma once

#include <QMainWindow>

class LineChartWindow : public QMainWindow {
  public:
    explicit LineChartWindow(QWidget* parent = nullptr);

  private:
    void setup_ui();
    QWidget* setup_static_layout();
    QWidget* setup_dynamic_layout();
    QWidget* setup_options_layout();
};
