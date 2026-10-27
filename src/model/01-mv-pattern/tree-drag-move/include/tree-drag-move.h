/**
 * @file tree-drag-move.h
 * @brief TreeDragMove——QTreeWidget 子类，演示 Qt 拖放协议（mimeTypes/mimeData/dropMimeData）
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QTreeWidget>

class QDropEvent;
class QMimeData;

namespace AwesomeQt {

/// @brief 支持节点内部拖拽移动的 QTreeWidget 子类。
///
/// 演示 Qt 拖放协议四件套（QTreeWidget 视图的拖放契约）：
/// - `mimeTypes()`：声明本树能产生的 MIME 类型，拖出时框架据此构造拖拽数据；
/// - `mimeData(items)`：把被拖节点序列化成 QMimeData（自定义格式，内含稳定 node id）；
/// - `supportedDropActions()`：声明接受的动作集，这里返回 MoveAction（内部移动语义）；
/// - `dropMimeData(parent,index,data,action)`：反序列化并落到目标位置——MoveAction 时
///   先复制源到目标，再删源，完成「移动」。
///
/// dropEvent 额外演示「落点判定」：`target_parent == null` = 落在根级空白（同级插入到根），
/// 否则 `target_index == childCount()` = 落在某节点上（成为其子节点），其它情况 = 同级插入。
///
/// 教学点对应（实例库 model/01-mv-pattern/tree-drag-move）：
/// ①setDragEnabled/setAcceptDrops/setDragDropMode(InternalMove) 在 demo 里设置；
/// ②本类重写四个协议方法；
/// ③InternalMove 下 MoveAction 落地后删源；
/// ④dropEvent 区分「成为子节点」与「同级插入」；
/// ⑤拖拽中选中态/展开态维护。
class TreeDragMove : public QTreeWidget {
    Q_OBJECT

  public:
    /// @brief 自定义拖拽数据的 MIME 类型（应用内私有格式）
    static const char* const kMimeType;

    explicit TreeDragMove(QWidget* parent = nullptr);

  protected:
    /// @brief 本树支持的拖放动作：仅 MoveAction（内部移动）
    Qt::DropActions supportedDropActions() const override;

    /// @brief 本树产生的拖拽数据类型：自定义 application/x-awesomeqt-treedragmove
    QStringList mimeTypes() const override;

    /// @brief 拖出时回调：把被选中的源节点序列化进 QMimeData（存 node id + 文本树）
    /// @param[in] items 用户正在拖动的源节点列表
    /// @return 非 nullptr 的 QMimeData（调用方接管所有权）
    QMimeData* mimeData(const QList<QTreeWidgetItem*>& items) const override;

    /// @brief 拖入时回调：把 mimeData 还原成节点，落到 (parent, index) 处。
    /// @param[in] parent 目标父节点，null 表示根级
    /// @param[in] index 在 parent 下的插入位置（childCount 表示追加为末子）
    /// @param[in] data 源端 mimeData
    /// @param[in] action 当前动作（MoveAction / CopyAction）
    /// @return true 表示已处理（框架不会再走默认行为）
    bool dropMimeData(QTreeWidgetItem* parent, int index, const QMimeData* data,
                      Qt::DropAction action) override;

    /// @brief dropEvent：在协议方法之上，演示落点判定与选中态/展开态维护。
    /// @param[in,out] event 拖放事件，可查询 targetParent/targetIndex/mousePos
    void dropEvent(QDropEvent* event) override;

  private:
    /// @brief 把一棵子树深拷贝成新节点（保留文本/图标/数据/子树）
    /// @param[in] src 源节点（只读）
    /// @return 新建节点（caller 接管所有权，通常挂到目标父节点下）
    QTreeWidgetItem* cloneItem(const QTreeWidgetItem* src) const;
};

} // namespace AwesomeQt
