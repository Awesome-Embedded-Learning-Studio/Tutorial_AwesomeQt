/**
 * @file hmi_dashboard_window.h
 * @brief HMI Dashboard 主窗口（骨架）——整机多区布局 + 占位仪表，验证 industrial 栏范式
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QMainWindow>

/// @brief HMI Dashboard 主窗口（骨架）。
/// 验证 industrial 栏范式：QSplitter 多区布局 + QDockWidget + 自绘占位仪表。
/// 骨架期仪表/趋势用自绘占位，成品期替换为 widget/speed-meter、circle-progress、line-chart。
class HmiDashboardWindow : public QMainWindow {
  public:
    explicit HmiDashboardWindow(QWidget* parent = nullptr);

  private:
    void setup_layout();
    void setup_status_bar();
};
