/**
 * @file checkbox_list_window.h
 * @brief CheckboxList 控件演示主窗口
 * @copyright Copyright (c) 2026 AwesomeQt. Licensed under MIT.
 */
#pragma once

#include <QMainWindow>

class QCheckBox;
class QTextEdit;

namespace AwesomeQt {
class CheckboxList;
}

class CheckboxListWindow : public QMainWindow {
  public:
    explicit CheckboxListWindow(QWidget* parent = nullptr);

  private:
    void setup_ui();
    void populate_sample_items();
    void list_checked_items();

    AwesomeQt::CheckboxList* list_{nullptr};
    QCheckBox* alternating_toggle_{nullptr};
    QTextEdit* output_{nullptr};
};
