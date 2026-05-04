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
};
