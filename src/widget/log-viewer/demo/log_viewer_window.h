/**
 * @file log_viewer_window.h
 * @brief LogViewer 控件演示主窗口
 * @copyright Copyright (c) 2026
 */

#pragma once

#include <QMainWindow>

class QCheckBox;
class QLabel;

namespace AwesomeQt {
class LogViewer;
}

class LogViewerWindow : public QMainWindow {
  public:
    explicit LogViewerWindow(QWidget* parent = nullptr);

  private:
    void setupUi();

    AwesomeQt::LogViewer* log_{nullptr};
    QCheckBox* auto_scroll_check_{nullptr};
    QLabel* line_count_label_{nullptr};
};
