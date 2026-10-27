/**
 * @file checkbox_tree.cpp
 * @brief 勾选树控件 CheckboxTree 实现——三态勾选 + 父子自动联动
 * @copyright Copyright (c) 2026 AwesomeQt. Licensed under MIT.
 */

#include "checkbox_tree.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace {

/// @brief 深度优先收集 item 子树里所有 Checked 节点。
void collectChecked(QTreeWidgetItem* item, QList<QTreeWidgetItem*>& out) {
    if (item == nullptr) {
        return;
    }
    if (item->checkState(0) == Qt::Checked) {
        out.append(item);
    }
    const int n = item->childCount();
    for (int i = 0; i < n; ++i) {
        collectChecked(item->child(i), out);
    }
}

} // namespace

namespace AwesomeQt {

CheckboxTree::CheckboxTree(QWidget* parent) : QWidget(parent) {
    // 内含一个 QTreeWidget，构造期 new 出来、parent=this 由对象树托管，放进布局。
    tree_ = new QTreeWidget(this);
    tree_->setColumnCount(1);
    tree_->setHeaderHidden(true);
    tree_->setUniformRowHeights(true);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(tree_);

    // 用函数指针语法连接，避免 SIGNAL/SLOT 宏；column 参数暂不使用，但保留槽签名。
    connect(tree_, &QTreeWidget::itemChanged, this, &CheckboxTree::onItemChanged);
}

QTreeWidgetItem* CheckboxTree::addItem(QTreeWidgetItem* parent, const QString& text,
                                       Qt::CheckState state) {
    auto* item = new QTreeWidgetItem();
    item->setText(0, text);
    item->setCheckState(0, state);

    if (parent != nullptr) {
        parent->addChild(item);
        // 新增子项可能影响父的聚合态，回算一次（守卫信号，避免雪崩）。
        const bool was_blocked = tree_->blockSignals(true);
        item->setCheckState(0, state);
        tree_->blockSignals(was_blocked);
        recalcUp(parent);
    } else {
        // blockSignals 守卫：setCheckState 会触发 itemChanged，这里只做初始化，
        // 真正的联动放给后续用户操作或 setItemChecked。
        const bool was_blocked = tree_->blockSignals(true);
        item->setCheckState(0, state);
        tree_->blockSignals(was_blocked);
        tree_->addTopLevelItem(item);
    }
    return item;
}

QList<QTreeWidgetItem*> CheckboxTree::checkedItems() const {
    QList<QTreeWidgetItem*> result;
    if (tree_ == nullptr) {
        return result;
    }

    // 从顶层逐棵深度优先遍历，只收 Checked（不含 PartiallyChecked）。
    const int top_count = tree_->topLevelItemCount();
    for (int i = 0; i < top_count; ++i) {
        collectChecked(tree_->topLevelItem(i), result);
    }
    return result;
}

void CheckboxTree::setItemChecked(QTreeWidgetItem* item, Qt::CheckState state) {
    if (item == nullptr) {
        return; // 边界：空指针安全返回。
    }

    if (!propagation_enabled_) {
        // 关掉联动就是普通勾选树：只改自身，不向子孙传播、不回算祖先。
        const bool was_blocked = tree_->blockSignals(true);
        item->setCheckState(0, state);
        tree_->blockSignals(was_blocked);
        emit checkStateChanged(item);
        return;
    }

    // 走完整联动：先向下传播，再向上回算。
    is_propagating_ = true;
    const bool was_blocked = tree_->blockSignals(true);
    item->setCheckState(0, state);
    propagateDown(item, state);
    tree_->blockSignals(was_blocked);
    recalcUp(item->parent());
    is_propagating_ = false;

    emit checkStateChanged(item);
}

void CheckboxTree::checkAll() {
    if (tree_ == nullptr) {
        return;
    }
    is_propagating_ = true;
    const bool was_blocked = tree_->blockSignals(true);
    const int top_count = tree_->topLevelItemCount();
    for (int i = 0; i < top_count; ++i) {
        QTreeWidgetItem* top = tree_->topLevelItem(i);
        if (top != nullptr) {
            propagateDown(top, Qt::Checked);
            top->setCheckState(0, Qt::Checked);
        }
    }
    tree_->blockSignals(was_blocked);
    is_propagating_ = false;
    emit checkStateChanged(nullptr);
}

void CheckboxTree::uncheckAll() {
    if (tree_ == nullptr) {
        return;
    }
    is_propagating_ = true;
    const bool was_blocked = tree_->blockSignals(true);
    const int top_count = tree_->topLevelItemCount();
    for (int i = 0; i < top_count; ++i) {
        QTreeWidgetItem* top = tree_->topLevelItem(i);
        if (top != nullptr) {
            propagateDown(top, Qt::Unchecked);
            top->setCheckState(0, Qt::Unchecked);
        }
    }
    tree_->blockSignals(was_blocked);
    is_propagating_ = false;
    emit checkStateChanged(nullptr);
}

QTreeWidget* CheckboxTree::treeWidget() const {
    return tree_;
}

bool CheckboxTree::isPropagationEnabled() const {
    return propagation_enabled_;
}

void CheckboxTree::setPropagationEnabled(bool enabled) {
    if (propagation_enabled_ == enabled) {
        return;
    }
    propagation_enabled_ = enabled;
    emit propagationEnabledChanged(enabled);
}

int CheckboxTree::indentation() const {
    return tree_ != nullptr ? tree_->indentation() : 0;
}

void CheckboxTree::setIndentation(int pixels) {
    if (tree_ == nullptr || pixels < 0) {
        return; // 边界：负值无意义，clamp 掉。
    }
    tree_->setIndentation(pixels);
    emit indentationChanged(pixels);
}

QSize CheckboxTree::sizeHint() const {
    return {260, 320};
}

void CheckboxTree::onItemChanged(QTreeWidgetItem* item, int /*column*/) {
    if (item == nullptr) {
        return;
    }
    // 自身触发的程序化修改（checkAll / setItemChecked 等）已自行处理联动，
    // 这里只响应用户点击，避免二次递归。
    if (is_propagating_) {
        return;
    }

    if (!propagation_enabled_) {
        emit checkStateChanged(item);
        return;
    }

    const Qt::CheckState state = item->checkState(0);

    // 向下传播：只有确定的 Checked/Unchecked 才传播，PartiallyChecked 是回算产物，
    // 用户不可能直接点出 Partially（QTreeWidget 交互只产生两态），故无需处理。
    is_propagating_ = true;
    const bool was_blocked = tree_->blockSignals(true);
    if (state == Qt::Checked || state == Qt::Unchecked) {
        propagateDown(item, state);
    }
    tree_->blockSignals(was_blocked);
    recalcUp(item->parent());
    is_propagating_ = false;

    emit checkStateChanged(item);
}

void CheckboxTree::propagateDown(QTreeWidgetItem* item, Qt::CheckState state) {
    if (item == nullptr) {
        return;
    }
    // 递归把同一状态写到所有子孙；调用方负责 blockSignals 守卫，这里直接改。
    const int child_count = item->childCount();
    for (int i = 0; i < child_count; ++i) {
        QTreeWidgetItem* child = item->child(i);
        if (child != nullptr) {
            child->setCheckState(0, state);
            propagateDown(child, state); // 递归到叶子
        }
    }
}

void CheckboxTree::recalcUp(QTreeWidgetItem* item) {
    // 从 item（变更节点的父）起逐层向上：每层据子项分布重算自身 tri-state。
    QTreeWidgetItem* current = item;
    while (current != nullptr) {
        const Qt::CheckState agg = aggregateState(current);
        const bool was_blocked = tree_->blockSignals(true);
        current->setCheckState(0, agg);
        tree_->blockSignals(was_blocked);
        current = current->parent();
    }
}

Qt::CheckState CheckboxTree::aggregateState(const QTreeWidgetItem* item) const {
    if (item == nullptr || item->childCount() == 0) {
        return item != nullptr ? item->checkState(0) : Qt::Unchecked;
    }

    const int child_count = item->childCount();
    int checked = 0;
    int unchecked = 0;

    for (int i = 0; i < child_count; ++i) {
        const QTreeWidgetItem* child = item->child(i);
        if (child == nullptr) {
            continue;
        }
        const Qt::CheckState cs = child->checkState(0);
        if (cs == Qt::Checked) {
            ++checked;
        } else if (cs == Qt::Unchecked) {
            ++unchecked;
        } else {
            // 任一子为 PartiallyChecked 即整体混合。
            return Qt::PartiallyChecked;
        }
    }

    if (checked == child_count) {
        return Qt::Checked;
    }
    if (unchecked == child_count) {
        return Qt::Unchecked;
    }
    return Qt::PartiallyChecked; // 部分 Checked 部分 Unchecked
}

} // namespace AwesomeQt
