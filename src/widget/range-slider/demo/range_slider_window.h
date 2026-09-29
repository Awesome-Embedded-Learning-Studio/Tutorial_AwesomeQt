/**
 * @file range_slider_window.h
 * @brief RangeSlider 演示主窗口
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QMainWindow>

class RangeSliderWindow : public QMainWindow {
  public:
    explicit RangeSliderWindow(QWidget* parent = nullptr);

  private:
    void setupUi();
    QWidget* setupStaticLayout();
    QWidget* setupInteractiveLayout();
    QWidget* setupProgrammaticLayout();
    QWidget* setupThemedLayout();
};
