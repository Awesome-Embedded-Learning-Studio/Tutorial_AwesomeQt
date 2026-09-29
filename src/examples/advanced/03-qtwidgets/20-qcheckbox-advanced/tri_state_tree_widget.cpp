/// @file    tri_state_tree_widget.cpp
/// @brief   TriStateTreeWidget 类实现——三态复选框父子联动。
///
/// 对应教程：进阶层 03-QtWidgets/20-QCheckBox 进阶。

#include "tri_state_tree_widget.h"

#include <QHeaderView>
#include <QTreeWidgetItem>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

TriStateTreeWidget::TriStateTreeWidget(QWidget* parent)
    : QTreeWidget(parent)
    , m_updating(false)
{
    setHeaderLabel(QStringLiteral("Permission Tree - Tri-state Check"));
    header()->setStretchLastSection(true);

    // 先构建完树，再连接信号——避免构建过程中触发 onItemChanged
    buildPermissionTree();

    // 连接 itemChanged 信号处理三态联动
    connect(this, &QTreeWidget::itemChanged, this, &TriStateTreeWidget::onItemChanged);

    setWindowTitle(QStringLiteral("Tri-state CheckState Propagation"));
    resize(450, 550);
}

// ─────────────────────────────────────────────────────────────────────────────
// 构建权限树
// ─────────────────────────────────────────────────────────────────────────────

void TriStateTreeWidget::buildPermissionTree()
{
    // 三个角色节点（顶层）
    const struct
    {
        QString name;
        QStringList modules;
    } kRoles[] = {
        {QStringLiteral("Administrator"),
         {QStringLiteral("User Management"), QStringLiteral("Content Management"),
          QStringLiteral("System Settings")}},
        {QStringLiteral("Editor"),
         {QStringLiteral("User Management"), QStringLiteral("Content Management"),
          QStringLiteral("System Settings")}},
        {QStringLiteral("Viewer"),
         {QStringLiteral("User Management"), QStringLiteral("Content Management"),
          QStringLiteral("System Settings")}},
    };

    // 每个模块下的权限项
    const QStringList kPermissions = {
        QStringLiteral("View"), QStringLiteral("Edit"), QStringLiteral("Delete")};

    for (const auto& role : kRoles) {
        auto* roleItem = new QTreeWidgetItem(this, QStringList{role.name});
        roleItem->setFlags(roleItem->flags() | Qt::ItemIsUserCheckable);
        roleItem->setCheckState(0, Qt::Unchecked);

        for (const auto& moduleName : role.modules) {
            auto* moduleItem = new QTreeWidgetItem(roleItem, QStringList{moduleName});
            moduleItem->setFlags(moduleItem->flags() | Qt::ItemIsUserCheckable);
            moduleItem->setCheckState(0, Qt::Unchecked);

            for (const auto& permName : kPermissions) {
                auto* permItem = new QTreeWidgetItem(moduleItem, QStringList{permName});
                permItem->setFlags(permItem->flags() | Qt::ItemIsUserCheckable);
                permItem->setCheckState(0, Qt::Unchecked);
            }
        }

        // 默认展开，方便用户直接操作
        roleItem->setExpanded(true);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// itemChanged 槽——三态联动入口
// ─────────────────────────────────────────────────────────────────────────────

void TriStateTreeWidget::onItemChanged(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column)

    // 防递归：传播过程中触发的 itemChanged 不再进入
    if (m_updating) {
        return;
    }
    m_updating = true;

    // 第一步：向下传播——把当前节点的状态传给所有子孙
    propagateToChildren(item, item->checkState(0));

    // 第二步：向上冒泡——从当前节点开始，逐层更新祖先节点状态
    bubbleToParents(item);

    m_updating = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// 向下传播：parent → children
// ─────────────────────────────────────────────────────────────────────────────

void TriStateTreeWidget::propagateToChildren(QTreeWidgetItem* parent, Qt::CheckState state)
{
    if (!parent) {
        return;
    }

    for (int i = 0; i < parent->childCount(); ++i) {
        QTreeWidgetItem* child = parent->child(i);

        // 最小作用域 blockSignals：只阻塞 setCheckState 的信号发射
        blockSignals(true);
        child->setCheckState(0, state);
        blockSignals(false);

        // 递归传播到更深层
        propagateToChildren(child, state);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 向上冒泡：children → parent
// ─────────────────────────────────────────────────────────────────────────────

void TriStateTreeWidget::bubbleToParents(QTreeWidgetItem* item)
{
    QTreeWidgetItem* parentItem = item->parent();
    if (!parentItem) {
        return;
    }

    // 计算父节点应有的状态
    Qt::CheckState newState = computeParentState(parentItem);

    // 最小作用域 blockSignals——只在 setCheckState 前后阻塞
    blockSignals(true);
    parentItem->setCheckState(0, newState);
    blockSignals(false);

    // 继续向上冒泡
    bubbleToParents(parentItem);
}

// ─────────────────────────────────────────────────────────────────────────────
// 计算父节点状态
// ─────────────────────────────────────────────────────────────────────────────

Qt::CheckState TriStateTreeWidget::computeParentState(QTreeWidgetItem* parent) const
{
    if (!parent || parent->childCount() == 0) {
        return parent ? parent->checkState(0) : Qt::Unchecked;
    }

    int checkedCount = 0;
    int uncheckedCount = 0;
    const int total = parent->childCount();

    for (int i = 0; i < total; ++i) {
        QTreeWidgetItem* child = parent->child(i);
        // 递归计算子节点的"有效状态"（考虑子节点自身可能也有子节点）
        Qt::CheckState childState = computeParentState(child);

        if (childState == Qt::Checked) {
            ++checkedCount;
        } else if (childState == Qt::Unchecked) {
            ++uncheckedCount;
        }
        // PartiallyChecked 既不算 checked 也不算 unchecked
    }

    // 全部 Checked → Checked；全部 Unchecked → Unchecked；其余 → PartiallyChecked
    if (checkedCount == total) {
        return Qt::Checked;
    }
    if (uncheckedCount == total) {
        return Qt::Unchecked;
    }
    return Qt::PartiallyChecked;
}
