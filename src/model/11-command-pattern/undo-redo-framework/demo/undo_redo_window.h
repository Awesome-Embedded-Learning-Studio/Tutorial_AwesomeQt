/**
 * @file undo_redo_window.h
 * @brief Undo/Redo Framework 演示主窗口——QUndoStack + QUndoView + 按钮触发 MoveCommand
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QMainWindow>

class QGraphicsRectItem;
class QGraphicsScene;
class QGraphicsView;
class QUndoStack;
class QUndoView;

/// @brief 演示窗口：点按钮右移矩形 push MoveCommand，菜单撤销/重做，右侧 QUndoView 可视化栈。
class UndoRedoWindow : public QMainWindow {
  public:
    explicit UndoRedoWindow(QWidget* parent = nullptr);

  private:
    void setup_scene();
    void setup_menu();
    void setup_undo_dock();
    void move_right();

    QGraphicsScene* scene_{nullptr};
    QGraphicsView* view_{nullptr};
    QGraphicsRectItem* rect_item_{nullptr};
    QUndoStack* undo_stack_{nullptr};
    QUndoView* undo_view_{nullptr};
};
