/**
 * @file editable_table_window.h
 * @brief EditableTable 控件演示主窗口
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#pragma once

#include <QMainWindow>

class QCheckBox;
class QTextEdit;

namespace AwesomeQt {
class EditableTable;
}

class EditableTableWindow : public QMainWindow {
  public:
    explicit EditableTableWindow(QWidget* parent = nullptr);

  private:
    void setup_ui();
    void dump_data(); // 把 data() 格式化进 text_output_

    AwesomeQt::EditableTable* table_{nullptr};
    QCheckBox* editable_check_{nullptr};
    QTextEdit* text_output_{nullptr};
};
