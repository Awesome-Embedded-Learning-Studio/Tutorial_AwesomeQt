/**
 * @file tree-drag-move_window.h
 * @brief TreeDragMove 演示主窗口——单树改层级 + 双树转移，演示拖放协议全套
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QMainWindow>

class QLabel;
class QSplitter;

namespace AwesomeQt {
class TreeDragMove;
}

/// @brief 演示窗口：左右两棵 TreeDragMove + 底部说明栏。
/// - 左树「项目结构」：单树内拖动改层级（落点判定 ①成为子节点 / ②同级插入）；
/// - 右树「回收区」：双树之间拖拽转移（CopyAction 跨树保留 / 单树内 MoveAction 删源）；
/// - 底部说明：实时显示最近一次拖拽的落点类型与动作。
class TreeDragMoveWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit TreeDragMoveWindow(QWidget* parent = nullptr);

  private:
    void setup_trees();
    void setup_status_bar();
    void seed_left_tree();
    void seed_right_tree();

    AwesomeQt::TreeDragMove* left_tree_{nullptr};
    AwesomeQt::TreeDragMove* right_tree_{nullptr};
    QLabel* status_label_{nullptr};
};
