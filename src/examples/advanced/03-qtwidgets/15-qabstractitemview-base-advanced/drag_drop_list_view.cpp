/// @file    drag_drop_list_view.cpp
/// @brief   DragDropListView 类实现——拖拽排序列表视图。
///
/// 对应教程：进阶层 03-QtWidgets/15-QAbstractItemView 基类进阶。

#include "drag_drop_list_view.h"

#include <QStandardItem>
#include <QStandardItemModel>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

DragDropListView::DragDropListView(QWidget* parent)
    : QListView(parent)
{
    // 创建标准模型并填充示例数据
    auto* itemModel = new QStandardItemModel(this);

    // 填充 10 个可拖拽排序的条目
    for (int i = 0; i < 10; ++i) {
        auto* item = new QStandardItem(
            QStringLiteral("Task %1 - Priority %2").arg(i + 1).arg(10 - i));
        item->setFlags(item->flags() | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
        itemModel->appendRow(item);
    }

    setModel(itemModel);

    // 拖拽框架四个核心属性——缺一不可
    setDragEnabled(true);                                        // 允许从本视图拖出
    setAcceptDrops(true);                                        // 允许接收拖入
    setDragDropMode(QAbstractItemView::InternalMove);            // 内部移动模式
    setDefaultDropAction(Qt::MoveAction);                        // 默认拖拽动作为移动
    setDropIndicatorShown(true);                                 // 显示拖放位置指示器

    setWindowTitle(QStringLiteral("Drag & Drop Reorder - QListView"));
    resize(400, 500);
}

// ─────────────────────────────────────────────────────────────────────────────
// 模型访问
// ─────────────────────────────────────────────────────────────────────────────

QStandardItemModel* DragDropListView::model() const
{
    return qobject_cast<QStandardItemModel*>(QListView::model());
}
