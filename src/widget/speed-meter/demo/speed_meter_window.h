/**
 * @file speed_meter_window.h
 * @brief SpeedMeter 控件演示主窗口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#pragma once

#include <QMainWindow>

class SpeedMeterWindow : public QMainWindow {
  public:
    explicit SpeedMeterWindow(QWidget* parent = nullptr);

  private:
    void setupUi();
    QWidget* setupStaticLayout();      // 静态几档：0/60/120/180/220
    QWidget* setupInteractiveLayout(); // 大表 + Cycle
    QWidget* setupSliderLayout();      // QSlider 0..220 驱动
    QWidget* setupRandomLayout();      // 随机跳变（测 stop()/接力不跳变）
};
