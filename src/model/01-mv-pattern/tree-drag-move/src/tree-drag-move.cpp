/**
 * @file tree-drag-move.cpp
 * @brief TreeDragMove 实现——拖放协议四件套 + dropEvent 落点判定与状态维护
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "tree-drag-move.h"

#include <QByteArray>
#include <QDataStream>
#include <QDropEvent>
#include <QHeaderView>
#include <QIODevice>
#include <QList>
#include <QMimeData>
#include <QStringList>
#include <QTreeWidgetItem>
#include <Qt>

namespace AwesomeQt {

// 自定义 MIME 类型：application/x-awesomeqt-treedragmove
// 进程内拖拽用指针定位源节点最稳；文本走 text/plain 便于跨进程/日志观察
const char* const TreeDragMove::kMimeType = "application/x-awesomeqt-treedragmove";

TreeDragMove::TreeDragMove(QWidget* parent) : QTreeWidget(parent) {
    // 选中态维护的基础：单选模式，拖拽时清晰追踪当前被拖节点
    setSelectionMode(QAbstractItemView::SingleSelection);
}

Qt::DropActions TreeDragMove::supportedDropActions() const {
    // 教学点③：内部移动语义——只认 MoveAction，拖完源位置删，目标位置有
    return Qt::MoveAction;
}

QStringList TreeDragMove::mimeTypes() const {
    // 教学点②：声明本树能产出/接受的 MIME 类型
    // 自定义格式优先；text/plain 给跨进程或日志观察留一条可读通道
    return {QString::fromLatin1(kMimeType), QStringLiteral("text/plain")};
}

QMimeData* TreeDragMove::mimeData(const QList<QTreeWidgetItem*>& items) const {
    // 教学点②：拖出时把被拖源节点序列化进 QMimeData
    if (items.isEmpty()) {
        return nullptr;
    }

    // 进程内拖拽：把源节点指针编码进自定义 MIME（QDataStream 写 quintptr）
    QByteArray payload;
    QDataStream stream(&payload, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_0);

    QStringList text_preview; // text/plain 通道：仅人类可读
    for (const QTreeWidgetItem* item : items) {
        if (item == nullptr) {
            continue;
        }
        // 写指针——dropMimeData 端据此把 QTreeWidgetItem* 取回来定位源
        stream << reinterpret_cast<quintptr>(item);
        text_preview << item->text(0);
    }

    auto* data = new QMimeData;
    data->setData(QString::fromLatin1(kMimeType), payload);
    data->setText(text_preview.join(QStringLiteral(" | ")));
    return data;
}

bool TreeDragMove::dropMimeData(QTreeWidgetItem* parent, int index, const QMimeData* data,
                                Qt::DropAction action) {
    // 教学点②③：协议级落地。QTreeWidget 的默认 dropMimeData 会无脑复制文本，
    // 这里完全接管：反序列化 → 克隆源子树 → 挂到目标 (parent, index)
    if (data == nullptr || !data->hasFormat(QString::fromLatin1(kMimeType))) {
        return false;
    }
    if (action != Qt::MoveAction && action != Qt::CopyAction) {
        return false; // 只处理移动 / 复制
    }

    QByteArray payload = data->data(QString::fromLatin1(kMimeType));
    QDataStream stream(&payload, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_6_0);

    // 决定落地容器：parent == nullptr 表示拖到根级空白处，否则挂到该父节点下
    QTreeWidget* target_tree = parent != nullptr ? parent->treeWidget() : this;
    if (parent != nullptr && parent->treeWidget() != this) {
        // 跨树拖拽（demo 双树场景）：源指针在另一棵树，克隆逻辑仍成立，
        // 但源删除要回到源树做——这里只负责复制，删源交给 dropEvent 的 MoveAction 分支
        target_tree = parent->treeWidget();
    }

    // index 的语义：parent==null 时 index 是 topLevelItem 下标；否则是 parent 的子下标
    if (index < 0) {
        index = 0;
    }

    int inserted = 0;
    while (!stream.atEnd()) {
        quintptr src_ptr_raw = 0;
        stream >> src_ptr_raw;
        auto* src = reinterpret_cast<QTreeWidgetItem*>(src_ptr_raw);
        if (src == nullptr) {
            continue;
        }
        // 教学点③：克隆源子树（深拷贝，保留文本/图标/data/子树结构）
        // 注意：协议层只负责「复制」——源删除统一放在 dropEvent 的 MoveAction 分支，
        // 集中处理避免「删早了导致后续取不到源」的时序坑（见 dropEvent）
        QTreeWidgetItem* clone = cloneItem(src);

        if (parent != nullptr) {
            parent->insertChild(index, clone);
        } else {
            target_tree->insertTopLevelItem(index, clone);
        }
        ++index; // 多选拖拽时依次顺延插入
        ++inserted;
    }

    return inserted > 0;
}

void TreeDragMove::dropEvent(QDropEvent* event) {
    // 教学点④⑤：在协议方法之上做落点判定 + 选中态/展开态维护。
    //
    // QTreeWidget::dropEvent 内部会调 dropMimeData（我们已接管）。
    // 但默认实现会「先删源、后插副本」，在 InternalMove 下产生节点闪烁，
    // 且不会展开目标父节点。这里：
    // 1. 记下拖拽前的源指针（从 mimeData 反解），先备好；
    // 2. 调基类 dropEvent 走 dropMimeData 复制；
    // 3. MoveAction 下手动删源（dropMimeData 里故意没删，集中在这）；
    // 4. 落点是某父节点 → 展开它、选中新克隆；落点是空白 → 选中新顶级项。

    if (event == nullptr) {
        return;
    }
    const QMimeData* data = event->mimeData();
    if (data == nullptr || !data->hasFormat(QString::fromLatin1(kMimeType))) {
        QTreeWidget::dropEvent(event);
        return;
    }

    // 落点判定（教学点④）：event->targetParent()/targetIndex() 在 Qt6 已弃用精确语义，
    // 改用 currentItem()/itemAt(pos) 双通道判断更稳
    QTreeWidgetItem* target_parent = itemAt(event->position().toPoint());
    bool drop_on_item = (target_parent != nullptr);

    // —— 备份源指针（dropMimeData 之后源可能已被默认实现删除，提前取）——
    QList<QTreeWidgetItem*> sources;
    {
        QByteArray payload = data->data(QString::fromLatin1(kMimeType));
        QDataStream stream(&payload, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_6_0);
        while (!stream.atEnd()) {
            quintptr raw = 0;
            stream >> raw;
            auto* src = reinterpret_cast<QTreeWidgetItem*>(raw);
            if (src != nullptr) {
                sources.append(src);
            }
        }
    }

    // 调基类 dropEvent → 触发我们重写的 dropMimeData 完成复制
    QTreeWidget::dropEvent(event);

    // 教学点③：InternalMove 下删源。dropMimeData 只复制了，这里统一删源。
    // 注意顺序：先复制（上一步）后删源，避免删早了导致 dropMimeData 取不到源。
    if (event->dropAction() == Qt::MoveAction) {
        for (QTreeWidgetItem* src : sources) {
            if (src == nullptr) {
                continue;
            }
            QTreeWidgetItem* p = src->parent();
            if (p != nullptr) {
                int i = p->indexOfChild(src);
                if (i >= 0) {
                    QTreeWidgetItem* taken = p->takeChild(i);
                    delete taken; // MoveAction：源真正消失（不是隐藏）
                }
            } else {
                int i = indexOfTopLevelItem(src);
                if (i >= 0) {
                    QTreeWidgetItem* taken = takeTopLevelItem(i);
                    delete taken;
                }
            }
        }
    }

    // 教学点⑤：选中态 / 展开态维护
    // —— 落在某节点上 → 展开它（让新子节点可见）；选中态：QTreeWidget 在 dropEvent 后
    //    默认选中是旧的，这里清掉再选第一个顶层克隆（根级落点）或父节点（落在节点上）。
    if (drop_on_item && target_parent != nullptr) {
        target_parent->setExpanded(true);
        setCurrentItem(target_parent);
    } else if (topLevelItemCount() > 0) {
        setCurrentItem(topLevelItem(topLevelItemCount() - 1));
    }
}

QTreeWidgetItem* TreeDragMove::cloneItem(const QTreeWidgetItem* src) const {
    if (src == nullptr) {
        return nullptr;
    }
    auto* clone = new QTreeWidgetItem;
    const int col_count = src->columnCount();
    for (int c = 0; c < col_count; ++c) {
        clone->setText(c, src->text(c));
        clone->setIcon(c, src->icon(c));
        clone->setToolTip(c, src->toolTip(c));
        clone->setData(c, Qt::UserRole, src->data(c, Qt::UserRole));
    }
    clone->setCheckState(0, src->checkState(0));
    // 深拷贝整棵子树
    for (int i = 0; i < src->childCount(); ++i) {
        QTreeWidgetItem* child_clone = cloneItem(src->child(i));
        if (child_clone != nullptr) {
            clone->addChild(child_clone);
        }
    }
    return clone;
}

} // namespace AwesomeQt
