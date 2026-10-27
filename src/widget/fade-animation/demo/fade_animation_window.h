/**
 * @file fade_animation_window.h
 * @brief FadeWidget 控件演示主窗口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#pragma once

#include <QMainWindow>

namespace AwesomeQt {

class FadeWidget;

class FadeAnimationWindow : public QMainWindow {
  public:
    explicit FadeAnimationWindow(QWidget* parent = nullptr);

  private:
    void setup_ui();
    QWidget* build_fade_group();
    QWidget* build_duration_group();
    QWidget* build_opacity_group();

    FadeWidget* fade_widget_{nullptr};
};

} // namespace AwesomeQt
