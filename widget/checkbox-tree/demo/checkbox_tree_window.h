/**
 * @file checkbox_tree_window.h
 * @brief CheckboxTree 控件演示主窗口
 * @copyright Copyright (c) 2026 AwesomeQt. Licensed under MIT.
 */
#pragma once

#include <QMainWindow>

class QCheckBox;
class QTextEdit;

namespace AwesomeQt {
class CheckboxTree;
}

class CheckboxTreeWindow : public QMainWindow {
  public:
    explicit CheckboxTreeWindow(QWidget* parent = nullptr);

  private:
    void setup_ui();
    void populate_sample_tree();
    void list_checked_items();

    AwesomeQt::CheckboxTree* tree_{nullptr};
    QCheckBox* propagation_toggle_{nullptr};
    QTextEdit* output_{nullptr};
};
