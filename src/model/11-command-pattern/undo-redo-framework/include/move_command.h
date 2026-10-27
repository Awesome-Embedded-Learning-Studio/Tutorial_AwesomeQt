/**
 * @file move_command.h
 * @brief MoveCommand——QUndoCommand 子类，model 栏命令模式 reference 范式样例
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QPointF>
#include <QUndoCommand>

class QGraphicsItem;

namespace AwesomeQt {

/// @brief 移动图形项的撤销命令：记录 old/new 位置，undo/redo 在两者间切换。
///
/// 范式要点（model 栏 reference，后续 model 照此复刻）：
/// - 库类放 `AwesomeQt::` 命名空间内；
/// - QUndoCommand 子类无自定义信号时不需 Q_OBJECT，靠虚函数 undo/redo 工作；
/// - push 到 QUndoStack 时自动调 redo()，undo() 做逆操作，QUndoView 自动可视化。
class MoveCommand : public QUndoCommand {
  public:
    MoveCommand(QGraphicsItem* item, const QPointF& old_pos, const QPointF& new_pos,
                QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

    /// @brief 命令 id，标识「移动」命令族，供 mergeWith 合并连续移动
    int id() const override;
    /// @brief 合并连续同向移动：仅当 next 的起点 == 本命令终点时，扩展终点
    bool mergeWith(const QUndoCommand* other) override;

  private:
    QGraphicsItem* item_;
    QPointF old_pos_;
    QPointF new_pos_;
};

} // namespace AwesomeQt
