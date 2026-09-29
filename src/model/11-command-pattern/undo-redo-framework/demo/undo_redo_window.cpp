/**
 * @file undo_redo_window.cpp
 * @brief Undo/Redo Framework 演示实现
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "undo_redo_window.h"

#include <QAction>
#include <QBrush>
#include <QColor>
#include <QDockWidget>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QKeySequence>
#include <QMenuBar>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QUndoStack>
#include <QUndoView>
#include <QVBoxLayout>
#include <QWidget>

#include "move_command.h"

UndoRedoWindow::UndoRedoWindow(QWidget* parent) : QMainWindow(parent) {
    setup_scene();
    setup_menu();
    setup_undo_dock();

    auto* move_btn = new QPushButton("Move Right (push MoveCommand)", this);
    connect(move_btn, &QPushButton::clicked, this, &UndoRedoWindow::move_right);

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    layout->addWidget(view_);
    layout->addWidget(move_btn);
    setCentralWidget(central);

    setWindowTitle("Undo/Redo Framework Demo");
    resize(720, 500);
}

void UndoRedoWindow::setup_scene() {
    scene_ = new QGraphicsScene(this);
    scene_->setSceneRect(0, 0, 640, 360);

    rect_item_ = scene_->addRect(20, 150, 80, 60, QPen(QColor(30, 60, 120), 2),
                                 QBrush(QColor(60, 120, 220)));

    view_ = new QGraphicsView(scene_, this);
    view_->setRenderHint(QPainter::Antialiasing);
}

void UndoRedoWindow::setup_menu() {
    undo_stack_ = new QUndoStack(this);

    // QUndoStack 自带 undo/redo action：自动管 enabled + 快捷键，是 Qt 范式
    auto* edit_menu = menuBar()->addMenu("&Edit");

    auto* undo_action = undo_stack_->createUndoAction(this, "&Undo");
    undo_action->setShortcut(QKeySequence::Undo);
    edit_menu->addAction(undo_action);

    auto* redo_action = undo_stack_->createRedoAction(this, "&Redo");
    redo_action->setShortcut(QKeySequence::Redo);
    edit_menu->addAction(redo_action);
}

void UndoRedoWindow::setup_undo_dock() {
    undo_view_ = new QUndoView(undo_stack_, this);
    auto* dock = new QDockWidget("Command Stack", this);
    dock->setWidget(undo_view_);
    addDockWidget(Qt::RightDockWidgetArea, dock);
}

void UndoRedoWindow::move_right() {
    const QPointF old_pos = rect_item_->pos();
    const QPointF new_pos = old_pos + QPointF(50, 0);
    // push 会自动调 redo() 把矩形移到 new_pos，并把命令入栈（QUndoView 自动显示）
    undo_stack_->push(new AwesomeQt::MoveCommand(rect_item_, old_pos, new_pos));
}
