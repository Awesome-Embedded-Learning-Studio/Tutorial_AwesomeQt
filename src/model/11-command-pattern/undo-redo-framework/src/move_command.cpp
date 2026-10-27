/**
 * @file move_command.cpp
 * @brief MoveCommand 实现——QUndoCommand undo/redo + 合并连续移动
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "move_command.h"

#include <QGraphicsItem>

namespace AwesomeQt {

MoveCommand::MoveCommand(QGraphicsItem* item, const QPointF& old_pos, const QPointF& new_pos,
                         QUndoCommand* parent)
    : QUndoCommand(parent), item_(item), old_pos_(old_pos), new_pos_(new_pos) {
    setText("Move Item");
}

void MoveCommand::undo() {
    if (item_ != nullptr) {
        item_->setPos(old_pos_);
    }
}

void MoveCommand::redo() {
    if (item_ != nullptr) {
        item_->setPos(new_pos_);
    }
}

int MoveCommand::id() const {
    return 1001; // 固定 id，标识「移动」命令族
}

bool MoveCommand::mergeWith(const QUndoCommand* other) {
    if (other->id() != id()) {
        return false; // 不同命令族，不合并
    }
    const auto* next = static_cast<const MoveCommand*>(other);
    // 仅当 next 的起点 == 本命令终点时，扩展终点（连续拖动合并为一次）
    if (next->old_pos_ != new_pos_) {
        return false;
    }
    new_pos_ = next->new_pos_;
    return true;
}

} // namespace AwesomeQt
