/**
 * @file custom-model_window.h
 * @brief CustomTableModel 演示主窗口——QTableView + 增删行 + 列代理编辑
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QMainWindow>

class QTableView;
class QSpinBox;
class QPushButton;

namespace AwesomeQt {
class CustomTableModel;
}

/// @brief 演示窗口：用 QTableView 展示 CustomTableModel 的全部能力——
/// 多角色显示、可编辑单元格、增删行、列表头。
class CustomModelWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit CustomModelWindow(QWidget* parent = nullptr);

  private:
    void setup_toolbar();
    void add_row();
    void remove_selected_rows();
    void seed_extra();

    QTableView* table_view_{nullptr};
    AwesomeQt::CustomTableModel* model_{nullptr};
    QPushButton* add_btn_{nullptr};
    QPushButton* remove_btn_{nullptr};
    QPushButton* seed_btn_{nullptr};
};
